#include "type/type.hpp"

#include "ast/data.hpp"
#include "ast/dumper.hpp"
#include "ast/forward.hpp"
#include "ast/node/base.hpp"
#include "ast/node/declaration_local.hpp"
#include "ast/node/expression.hpp"
#include "ast/node/literal.hpp"
#include "ast/node/numeric_128_bits.hpp"
#include "ast/pool.hpp"
#include "ast/tool.hpp"
#include "id/base.hpp"
#include "id/nodeid.hpp"
#include "id/typeid.hpp"
#include "lexer/data.hpp"
#include "lexer/pool.hpp"
#include "lexer/token_viewer.hpp"
#include "nexus/forward.hpp"
#include "parser/base.hpp"
#include "parser/context.hpp"
#include "parser/declaration_global.hpp"
#include "parser/expression.hpp"
#include "parser/type.hpp"
#include "type/data.hpp"
#include "type/pool.hpp"
#include "type/tool.hpp"

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <format>
#include <llvm/ADT/APInt.h>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>


parser::Parser_Type::Parser_Type(Parser_Context& p_ctx)
  : p(p_ctx)
{
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
  if (p.match_chain({token::ETokenKind::SEMICOLON, token::ETokenKind::RANGE, token::ETokenKind::R_SQUARE})) {
    get_qualifier(dec);
    return parser_type_factory.make_slice(inner, dec);
  }
  if (p.match_chain({token::ETokenKind::SEMICOLON, token::ETokenKind::UNDERSCORE, token::ETokenKind::R_SQUARE})) {
    get_qualifier(dec);
    return parser_type_factory.make_dynamic_array(inner, dec);
  }
  if (p.check(token::ETokenKind::SEMICOLON) && p.peek(1).tokid.str() == "c"
      && p.check_at(2, token::ETokenKind::R_SQUARE)) {
    (void)p.next(); // consume ;
    (void)p.next(); // consume c
    (void)p.next(); // consume ]
    get_qualifier(dec);
    return parser_type_factory.make_slice(inner, dec, true);
  }
  if (p.match(token::ETokenKind::SEMICOLON)) {
    size      = p.p_expr->parse_expression();
    size_t sz = 0;
    if (const auto* size_node = size.as<ast::Literal_Integral>()) {
      size_val = size_node->val;
      sz       = size_node->val.val->getZExtValue();
    }

    (void)p.expect(672, token::ETokenKind::R_SQUARE, "Expected ']' after a static array size.", "");

    get_qualifier(dec);

    return parser_type_factory.make_static_array(inner, sz, size, dec);
  }

  assert(false);
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

  common::compiler::DEBUG_TOLZA_ICE("bad token interpreted as type before primitive parsing");

  return NO_ID;
}

type::ID parser::Parser_Type::id_type(const type::Qualifier& qualifier)
{
  auto& base_tok = p.peek();

  auto id = p.p_base->identifier();

  if (p.match_any({token::ETokenKind::L_ANGLE, token::ETokenKind::TURBO_FISH})) {
    auto old_id        = id;
    auto [nodeid, _ty] = p.p_base->identifier_typed();

    id            = nodeid;
    auto* id_node = id.as<ast::Symbol_Type>();
    id_node->name = old_id;
  }

  auto dec = qualifier;
  get_qualifier(dec);

  return parser_type_factory.make_identifier(ast::get_decl_name(id), id, definition::ID::invalid(), dec);
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
  auto  protoid = prototype_from_type();
  auto* proto   = protoid.as<type::Prototype>();
  assert(proto);

  proto->header.qualifier = qualifier;

  return protoid;
}

type::ID parser::Parser_Type::parse_type()
{
  constexpr std::string_view hint =
      R"(define type like:
  - primitives `i32`, `f32`, `bool`, `char`, `addr`, ...
  - user type `identifier`, `_id32`, `T`, ...
  - generic args `T<i32, U>`, `T<gen_args>`, `T<gen_args>::U`, ...
  - from module/namespace `A::B::T`, `A::B<U, V>`, `A::B<U, V>::T`, ...)";

  type::Qualifier qua;
  get_qualifier(qua);

  switch (p.peek().kind) {
  case token::ETokenKind::T_OPAQUE: {
    (void)p.next();
    return type::TYPEID_opaque;
  }
  case token::ETokenKind::L_SQUARE: return table(qua);
  case token::ETokenKind::PTR:      return pointer(qua);
  case token::ETokenKind::L_PAREN:  return tuple(qua);
  case token::ETokenKind::FUNCTION: return function_proto(qua);
  default:                          break;
  }

  if (p.check_any(token::k_type_primitive)) return primitive(qua);

  if (p.check(token::ETokenKind::IDENTIFIER)) return id_type(qua);

  p.add_error(126, std::format("Unexpected type definition '{}'.", p.tok_to_str(p.peek().tokid)), hint);
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

std::tuple<type::ID, std::vector<ast::ID>, ast::ID>
parser::Parser_Type::prototype_from_declaration(bool start_at_params)
{
  constexpr std::string_view hint =
      R"(define function like:
  - `fn myName() { ... }`
  - with return `fn myName() -> (copy i32, ...) { ... }`
  - with contract `fn div(a: i32, b: i32) pre b != 0 { ... }`)";

  (void)p.match_any({token::ETokenKind::FUNCTION, token::ETokenKind::LAMBDA});

  type::Prototype proto;

  // check parameters
  if (!start_at_params)
    (void)p.expect(131, token::ETokenKind::L_PAREN,
                   "Expected start parameter definition '(' after function declaration.", hint);

  auto [is_variadic, params] = parameters();

  proto.params = type::to_proto_params(params);

  // check return
  if (p.match(token::ETokenKind::ARROW)) {
    proto.is_explicit_ret = true;
    proto.ret             = p.p_type->parse_type();
  } else {
    proto.is_explicit_ret = false;
    proto.ret             = type::TYPEID_u0;
  }

  proto.is_variadic = is_variadic;

  // check contract
  auto contract =
      p.check_any({token::ETokenKind::PRE, token::ETokenKind::POST}) ? p.p_decl->call_contract() : ast::ID::invalid();

  auto tyid = parser_type_factory.make_prototype(proto.params, proto.ret, proto.is_variadic);

  return {tyid, params, contract};
}


type::ID parser::Parser_Type::prototype_from_type()
{
  constexpr std::string_view hint =
      R"(define function like:
  - `fn myName() { ... }`
  - with return `fn myName() -> (copy i32, ...) { ... }`)";

  (void)p.match_any({token::ETokenKind::FUNCTION, token::ETokenKind::LAMBDA});
  (void)p.match(token::ETokenKind::L_PAREN);

  type::Prototype proto;

  if (!p.match(token::ETokenKind::R_PAREN)) {
    while (!p.is_end()) {
      if (p.match(token::ETokenKind::VARIADIC)) {
        proto.is_variadic = true;

        (void)p.expect(154, token::ETokenKind::R_PAREN,
                       "Unexpected token after a variadic mark, the variadic must be the last parameter.", hint);
        break;
      }

      type::Prototype_Param param;
      param.passmode = ast::ETokenKind_to_EPassMode(p.next().kind);
      if (param.passmode == ast::EPassMode::NONE)
        p.add_error(132, "Expected parameter pass mode before the parameter name.", hint);

      param.type = p.p_type->parse_type();

      proto.params.emplace_back(param);

      if (p.match_field_separator(token::ETokenKind::COMMA, token::ETokenKind::R_PAREN)) break;
    }
  }

  // check return
  if (p.match(token::ETokenKind::ARROW)) {
    proto.is_explicit_ret = true;
    proto.ret             = p.p_type->parse_type();
  } else {
    proto.is_explicit_ret = false;
    proto.ret             = type::TYPEID_u0;
  }

  auto tyid = parser_type_factory.make_prototype(proto.params, proto.ret, proto.is_variadic);

  return tyid;
}

std::pair<bool, std::vector<ast::ID>> parser::Parser_Type::parameters()
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

  std::vector<ast::ID> params;
  bool                 is_variadic = false;

  size_t count = 0;
  while (!p.is_end()) {
    if (p.match(token::ETokenKind::VARIADIC)) {
      is_variadic = true;

      (void)p.expect(154, token::ETokenKind::R_PAREN,
                     "Unexpected token after a variadic mark, the variadic must be the last parameter.", hint);
      break;
    }

    ast::Local_Parameter param;
    param.passmode = ast::ETokenKind_to_EPassMode(p.next().kind);
    if (param.passmode == ast::EPassMode::NONE)
      p.add_error(132, "Expected parameter pass mode before the parameter name.", hint);

    param.name = p.parse_name("", hint);
    auto tok   = p.peek(-1).tokid;
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
      param.default_value = p.p_expr->parse_expression();
    }

    auto& out = p.p_base->inject_parameter(p.current_returnable, count++, param.name, param.passmode, param.type);
    out.default_value = param.default_value;
    out.is_restrict   = param.is_restrict;

    params.emplace_back(out.nodeid());

    if (p.match_field_separator(token::ETokenKind::COMMA, token::ETokenKind::R_PAREN)) break;
  }

  return {is_variadic, params};
}
