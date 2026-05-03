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
#include "nexus/script.hpp"
#include "nexus/symbol.hpp"
#include "nexus/type.hpp"

#include "ast/ast_declaration_local.hpp"


#include "parser_context.hpp"
#include "parser_expression.hpp"
#include "parser_base.hpp"

parser::Parser_Type::Parser_Type(Parser_Context& p_ctx)
  : p(p_ctx)
{
}


std::vector<type::_id> parser::Parser_Type::Params::to_type_params() const
{
  std::vector<type::_id> out;
  out.reserve(params.size());

  for (auto elem : params) {
    out.push_back(elem.type);
  }

  return out;
}


// const, optional, volatile
void parser::Parser_Type::get_decorator(type::Decorator& decorator)
{
  for (size_t i = 0; i < 3; i++) {
    if (p.match(token::ETokenKind::DOLLAR)) decorator.is_optional = true;
    if (p.match(token::ETokenKind::INTERROGATIVE)) decorator.is_volatile = true;
    if (p.match(token::ETokenKind::OP_NOT)) decorator.is_constant = true;
  }
}

type::_id parser::Parser_Type::table(const type::Decorator& decorator)
{
  p.match(token::ETokenKind::OPEN_SQUARE);

  auto       dec   = decorator;
  auto       inner = p.p_type->parse_type();
  ast::_gnid size;
  Int128     size_val;

  // static table (sized)
  if (p.match(token::ETokenKind::ARROW)) {
    size = p.p_expr->parse_expression();
    if (auto size_node = p.scr_info.nodes->get_as<ast::Literal_Integral>(size.get_node_id())) {
      size_val = size_node->val;
    }
  }

  if (p.match(token::ETokenKind::CLOSE_SQUARE)) {
    get_decorator(dec);
  }

  if (size_val.val)
    return parser_type_factory.make_static_array(inner, size_val.val->getZExtValue(), dec);
  else
    return parser_type_factory.make_dynamic_array(inner, dec);
}

type::_id parser::Parser_Type::pointer(const type::Decorator& decorator)
{
  auto ptr = ast::ETokenKind_to_EBinOpType(p.peek().kind);

  p.next(); // consume ptr

  auto dec = decorator;
  get_decorator(dec);

  p.expect(124, token::ETokenKind::TICK, "Expected tick ''' after pointer specification.",
           "define pointer like:"
           "  - `ptr'T` `std::unique_ptr'T` `std::shared_ptr'T` `std::weak_ptr'T`");

  auto inner = p.p_type->parse_type();


  return parser_type_factory.make_ptr(inner);
}

type::_id parser::Parser_Type::primitive(const type::Decorator& decorator)
{
  auto tok = p.next();

  auto prim = type::ETokenKind_to_EPrimitiveTypeKind(tok.kind);

  auto dec = decorator;
  get_decorator(dec);

  return parser_type_factory.make_primitive(prim, dec);
}

type::_id parser::Parser_Type::id_type(const type::Decorator& decorator)
{
  auto base_tok = p.peek();

  auto gnid = p.p_base->identifier();

  if (p.match_any({token::ETokenKind::OPEN_BRACKETS, token::ETokenKind::TURBO_FISH})) {
    auto old_id       = gnid;
    auto [_gnid, _ty] = p.p_base->identifier_typed();
    gnid              = _gnid;
    auto id_node      = p.scr_info.nodes->get_as<ast::ID_Typed>(gnid.get_node_id());
    id_node->name     = old_id;
  }

  auto dec = decorator;
  get_decorator(dec);

  return parser_type_factory.make_identifier(gnid, symbol::_id(), dec);
}

type::_id parser::Parser_Type::tuple(const type::Decorator& decorator)
{
  auto elems = explicit_tuple();

  auto dec = decorator;
  get_decorator(dec);

  return parser_type_factory.make_tuple(elems, dec);
}

type::_id parser::Parser_Type::function_proto(const type::Decorator& decorator)
{
  auto proto = explicit_function_proto();

  auto dec = decorator;
  get_decorator(dec);

  return parser_type_factory.make_prototype(proto.params.to_type_params(), proto.ret, proto.params.is_variadic,
                                            symbol::_id(), dec);
}

type::_id parser::Parser_Type::parse_type()
{
  constexpr std::string_view hint =
      R"(define type like:
  - primitives `i32`, `f32`, `bool`, `char`, `addr`, ...
  - user type `identifier`, `_id32`, `T`, ...
  - generic args `T<i32, U>`, `T<gen_args>`, `T<gen_args>::U`, ...
  - from module/namespace `A::B::T`, `A::B<U, V>`, `A::B<U, V>::T`, ...)";

  type::Decorator dec;
  get_decorator(dec);

  switch (p.peek().kind) {
  case token::ETokenKind::OPEN_SQUARE: return table(dec);
  case token::ETokenKind::PTR:         return pointer(dec);
  case token::ETokenKind::OPEN_PAREN:  return tuple(dec);
  case token::ETokenKind::FUNCTION:    return function_proto(dec);
  default:                             break;
  }

  if (p.check_any(token::k_type_primitive)) return primitive(dec);

  if (p.check(token::ETokenKind::IDENTIFIER)) return id_type(dec);

  p.add_error(126, "Unexpected type definition '" + std::string(p.tok_to_str(p.peek().id)) + "'.", hint);
  return type::BAD_TYPE_ID;
};

std::vector<type::_id> parser::Parser_Type::explicit_tuple()
{
  constexpr std::string_view hint = "define named tuple like `(filed1: i32, ...)`.";

  std::vector<type::_id> tuple;

  bool endByParen = p.match(token::ETokenKind::OPEN_PAREN);

  while (!p.is_end()) {
    tuple.push_back(p.p_type->parse_type());

    if (!endByParen && p.match_field_any_separator(token::ETokenKind::COMMA, token::k_args_ending)) {
      p.rewind(p.tok_v->position() - 1);
      break;
    }
    if (p.match_field_separator(token::ETokenKind::COMMA, token::ETokenKind::CLOSE_PAREN)) break;
  }

  return tuple;
}

ast::_gnid parser::Parser_Type::get_type()
{
  parser_add_node(get, Expression_Get_Type, p.peek(-1).id);

  p.expect(129, token::ETokenKind::OPEN_PAREN, "Expected start arg '('.",
           "define get type at compilation time like: `comptime::type(var)`");

  get->target = p.p_expr->parse_expression();

  p.expect(130, token::ETokenKind::CLOSE_PAREN, "Expected end arg ')'.",
           "define get type at compilation time like: `comptime::type(var)`");

  return get->node_id;
}

parser::Parser_Type::Proto parser::Parser_Type::parse_and_mount_local_callable(type::_id& prototype_id,
                                                                               bool&      is_explicit_ret)
{
  auto proto = p.p_type->explicit_function_proto();

  prototype_id =
      parser_type_factory.make_prototype(proto.params.to_type_params(), proto.ret, proto.params.is_variadic, NO_ID);

  for (auto& param : proto.params.params) {
    parser_add_node(n_param, Local_Parameter, param.name_tok);

    n_param->name          = param.name;
    n_param->passmode      = param.passmode;
    n_param->default_value = param.default_val;
    n_param->type          = param.type;

    p.add_symbol(n_param->node_id.get_node_id());
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

  p.match_any({token::ETokenKind::FUNCTION, token::ETokenKind::LAMBDA});

  Proto proto;

  // check
  // parameters
  p.expect(131, token::ETokenKind::OPEN_PAREN, "Expected start parameter defintion '(' after function declaration.",
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

  if (p.match(token::ETokenKind::CLOSE_PAREN)) return {};

  Params params;

  while (!p.is_end()) {
    if (p.match(token::ETokenKind::VARIADIC)) {
      params.is_variadic = true;

      p.expect(154, token::ETokenKind::CLOSE_PAREN,
               "Unexpected token after a variadic mark, the variadic must be the last parameter.", hint);
      break;
    }

    Param param;
    param.passmode = ast::ETokenKind_to_EPassMode(p.next().kind);
    if (param.passmode == ast::EPassMode::NONE)
      p.add_error(132, "Expected parameter pass mode before the parameter name.", hint);

    param.name     = p.parse_name("", hint);
    param.name_tok = p.peek(-1).id;
    // check pointer parameter type
    p.expect(133, token::ETokenKind::COLON, "Expected type definition ':' after parameter name.", hint);
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

    if (p.match_field_separator(token::ETokenKind::COMMA, token::ETokenKind::CLOSE_PAREN)) break;
  }

  return params;
}
