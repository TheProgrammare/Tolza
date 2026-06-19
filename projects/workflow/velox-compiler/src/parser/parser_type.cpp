#include "parser_type.hpp"

#include <cstddef>
#include <llvm/ADT/APInt.h>
#include <string_view>
#include <sys/types.h>
#include <vector>

#include "ast/ast_base.hpp"
#include "ast/ast_expression.hpp"
#include "ast/ast_literal.hpp"
#include "ast/ast_numeric_128_bits.hpp"
#include "compiler/compiler.hpp"
#include "nexus/ast/ast.hpp"
#include "nexus/forward.hpp"
#include "nexus/ids.hpp"
#include "nexus/lexer/token.hpp"
#include "nexus/lexer/token_viewer.hpp"
#include "nexus/type/type.hpp"

#include "ast/ast_declaration_local.hpp"


#include "parser_context.hpp"
#include "parser_expression.hpp"
#include "parser_base.hpp"

parser::Parser_Type::Parser_Type(Parser_Context& p_ctx)
  : p(p_ctx)
{
}


std::vector<type::ID> parser::Parser_Type::Params::to_type_params() const
{
  std::vector<type::ID> out;
  out.reserve(params.size());

  for (const auto& elem : params) {
    out.emplace_back(elem.type);
  }

  return out;
}


// const, optional, volatile
void parser::Parser_Type::get_qualifier(type::Qualifier& qualifier)
{
  for (size_t i = 0; i < 3; i++) {
    if (p.match(token::ETokenKind::DOLLAR)) qualifier.is_optional = true;
    if (p.match(token::ETokenKind::INTERROGATIVE)) qualifier.is_volatile = true;
    if (p.match(token::ETokenKind::OP_NOT)) qualifier.is_constant = true;
  }
}

type::ID parser::Parser_Type::table(const type::Qualifier& qualifier)
{
  (void)p.match(token::ETokenKind::L_SQUARE);

  auto    dec   = qualifier;
  auto    inner = p.p_type->parse_type();
  ast::ID size;
  Int128  size_val;

  // static table (sized)
  if (p.match(token::ETokenKind::ARROW)) {
    size = p.p_expr->parse_expression();
    if (const auto* size_node = size.as<ast::Literal_Integral>()) {
      size_val = size_node->val;
    }
  }

  if (p.match(token::ETokenKind::R_SQUARE)) {
    get_qualifier(dec);
  }

  if (size_val.val) return parser_type_factory.make_static_array(inner, size_val.val->getZExtValue(), dec);

  return parser_type_factory.make_dynamic_array(inner, dec);
}

type::ID parser::Parser_Type::pointer(const type::Qualifier& qualifier)
{
  (void)p.next(); // consume ptr

  auto dec = qualifier;
  get_qualifier(dec);

  (void)p.expect(124, token::ETokenKind::TICK, "Expected tick ''' after pointer specification.",
                 "define pointer like:"
                 "  - `ptr'T`" /*"`uptr'T` `sptr'T` `wptr'T`"*/);

  auto inner = p.p_type->parse_type();

  return parser_type_factory.make_ptr(inner);
}

type::ID parser::Parser_Type::primitive(const type::Qualifier& qualifier)
{
  auto& tok = p.next();
  auto  dec = qualifier;
  get_qualifier(dec);

  if (auto prim = type::ETokenKind_to_EPrimitiveTypeKind(tok.kind); prim != type::EPrimitiveTypeKind::NONE) {
    return parser_type_factory.make_primitive(prim, dec);
  }

  if (auto txt = type::ETokenKind_to_ETextType(tok.kind); txt != type::ETextType::NONE) {
    return parser_type_factory.make_string(txt, dec);
  }

  common::compiler::DEBUG_VELOX_ICE("bad token interpreted as type before primitive parsing");

  return NO_ID;
}

type::ID parser::Parser_Type::id_type(const type::Qualifier& qualifier)
{
  auto& base_tok = p.peek();

  auto id = p.p_base->identifier();

  if (p.match_any({token::ETokenKind::L_ANGLE, token::ETokenKind::TURBO_FISH})) {
    auto old_id         = id;
    auto [_nodeid, _ty] = p.p_base->identifier_typed();

    id            = _nodeid;
    auto* id_node = id.as<ast::ID_Typed>();
    id_node->name = old_id;
  }

  auto dec = qualifier;
  get_qualifier(dec);

  return parser_type_factory.make_identifier(ast::get_decl_name(id), id, symbol::ID::invalid(), dec);
}

type::ID parser::Parser_Type::tuple(const type::Qualifier& qualifier)
{
  auto elems = explicit_tuple();

  auto dec = qualifier;
  get_qualifier(dec);

  return parser_type_factory.make_tuple(elems, dec);
}

type::ID parser::Parser_Type::function_proto(const type::Qualifier& qualifier)
{
  auto proto = explicit_function_proto();

  std::vector<type::Prototype::Param> ty_params;
  ty_params.reserve(proto.params.params.size());

  for (const auto& param : proto.params.params) {
    type::Prototype::Param out;
    out.type        = param.type;
    out.is_restrict = param.is_restrict;
    out.passmode    = param.passmode;
    ty_params.emplace_back(out);
  }

  auto dec = qualifier;
  get_qualifier(dec);

  return parser_type_factory.make_prototype(ty_params, proto.ret, proto.params.is_variadic, dec);
}

type::ID parser::Parser_Type::parse_type()
{
  constexpr std::string_view hint =
      R"(define type like:
  - primitives `i32`, `f32`, `bool`, `char`, `addr`, ...
  - user type `identifier`, `_id32`, `T`, ...
  - generic args `T<i32, U>`, `T<gen_args>`, `T<gen_args>::U`, ...
  - from module/namespace `A::B::T`, `A::B<U, V>`, `A::B<U, V>::T`, ...)";

  type::Qualifier dec;
  get_qualifier(dec);

  switch (p.peek().kind) {
  case token::ETokenKind::L_SQUARE: return table(dec);
  case token::ETokenKind::PTR:      return pointer(dec);
  case token::ETokenKind::L_PAREN:  return tuple(dec);
  case token::ETokenKind::FUNCTION: return function_proto(dec);
  default:                          break;
  }

  if (p.check_any(token::k_type_primitive)) return primitive(dec);

  if (p.check(token::ETokenKind::IDENTIFIER)) return id_type(dec);

  p.add_error(126, "Unexpected type definition '" + std::string(p.tok_to_str(p.peek().tokid)) + "'.", hint);
  return type::BAD_TYPE_ID;
};

std::vector<type::ID> parser::Parser_Type::explicit_tuple()
{
  constexpr std::string_view hint = "define named tuple like `(filed1: i32, ...)`.";

  std::vector<type::ID> tuple;

  bool endByParen = p.match(token::ETokenKind::L_PAREN);

  while (!p.is_end()) {
    tuple.emplace_back(p.p_type->parse_type());

    if (!endByParen && p.match_field_any_separator(token::ETokenKind::COMMA, token::k_args_ending)) {
      p.rewind(p.tok_v->position() - 1);
      break;
    }
    if (p.match_field_separator(token::ETokenKind::COMMA, token::ETokenKind::R_PAREN)) break;
  }

  return tuple;
}

ast::ID parser::Parser_Type::get_type()
{
  auto& get = p.add_get_node<ast::Expression_Get_Type>(p.peek(-1).tokid);

  (void)p.expect(129, token::ETokenKind::L_PAREN, "Expected start arg '('.",
                 "define get type at compilation time like: `comptime::type(var)`");

  get.target = p.p_expr->parse_expression();

  (void)p.expect(130, token::ETokenKind::R_PAREN, "Expected end arg ')'.",
                 "define get type at compilation time like: `comptime::type(var)`");

  return get.nodeid;
}

parser::Parser_Type::Proto parser::Parser_Type::parse_and_mount_local_callable(type::ID& prototype_id,
                                                                               bool&     is_explicit_ret)
{
  auto proto = p.p_type->explicit_function_proto();

  std::vector<type::Prototype::Param> ty_params;
  ty_params.reserve(proto.params.params.size());

  for (const auto& param : proto.params.params) {
    type::Prototype::Param out;
    out.type        = param.type;
    out.is_restrict = param.is_restrict;
    out.passmode    = param.passmode;
    ty_params.emplace_back(out);
  }

  prototype_id = parser_type_factory.make_prototype(ty_params, proto.ret, proto.params.is_variadic, NO_ID);

  for (const auto& param : proto.params.params) {
    auto& n_param = p.add_get_node<ast::Local_Parameter>(param.name_tok);

    n_param.name          = param.name;
    n_param.passmode      = param.passmode;
    n_param.default_value = param.default_val;
    n_param.type          = param.type;

    (void)p.add_symbol(n_param.nodeid);
  }

  is_explicit_ret = proto.is_explicit_ret;

  return proto;
}

parser::Parser_Type::Proto parser::Parser_Type::explicit_function_proto(bool p_is_lam)
{
  constexpr std::string_view hint =
      R"(define function like:
  - `fn myName() { ... }`
  - with return `fn myName() -> (i32, ...) { ... }`)";

  (void)p.match_any({token::ETokenKind::FUNCTION, token::ETokenKind::LAMBDA});

  Proto proto;

  // check
  // parameters
  (void)p.expect(131, token::ETokenKind::L_PAREN, "Expected start parameter defintion '(' after function declaration.",
                 hint);

  proto.params = parameters();

  // check
  // return
  if (p.match(token::ETokenKind::ARROW)) {
    proto.is_explicit_ret = true;
    proto.ret             = p.p_type->parse_type();
  } else {
    proto.is_explicit_ret = false;
    proto.ret             = type::TYPEID_u0;
  }

  return proto;
}

parser::Parser_Type::Params parser::Parser_Type::parameters()
{
  constexpr std::string_view hint =
      R"(define parameter like:
  Rule: <pass_mode> <name>: <type> [= <default_value>]
  - with a specific pass mode `[mut/copy/move/addr] myName: i32`.
  - Note: copy pass mode can have a default value like `copy myName: i32 = 0`.
  - variadic (only in externs for interop): `...`)";

  constexpr std::string_view hint_passmode =
      "Parameters default by pass mode:\n  - Allowed default: `copy`, `ref`\n  - Prohibied default: `mut`, "
      "`move`";

  if (p.match(token::ETokenKind::R_PAREN)) return {};

  Params params;

  while (!p.is_end()) {
    if (p.match(token::ETokenKind::VARIADIC)) {
      params.is_variadic = true;

      (void)p.expect(154, token::ETokenKind::R_PAREN,
                     "Unexpected token after a variadic mark, the variadic must be the last parameter.", hint);
      break;
    }

    Param param;
    param.passmode = ast::ETokenKind_to_EPassMode(p.next().kind);
    if (param.passmode == ast::EPassMode::NONE)
      p.add_error(132, "Expected parameter pass mode before the parameter name.", hint);

    param.name     = p.parse_name("", hint);
    param.name_tok = p.peek(-1).tokid;
    // check pointer parameter type
    (void)p.expect(133, token::ETokenKind::COLON, "Expected type definition ':' after parameter name.", hint);
    param.type = p.p_type->parse_type();

    // check parameter default value
    if (p.match(token::ETokenKind::ASSIGN)) {
      if (!EPassMode_Can_Default(param.passmode))
        p.add_error_tok(134, p.peek(-1),
                        "Unexpected defaut value for pass mode '" + std::string(EPassMode_to_str(param.passmode))
                            + "'.",
                        hint_passmode);
      param.default_val = p.p_expr->parse_expression();
    }

    params.params.emplace_back(param);

    if (p.match_field_separator(token::ETokenKind::COMMA, token::ETokenKind::R_PAREN)) break;
  }

  return params;
}
