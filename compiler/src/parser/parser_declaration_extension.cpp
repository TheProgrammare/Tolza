#include "parser_declaration_extension.hpp"

#include <string_view>

#include "Neargye/magic_enum.hpp"
#include "ast/ast_declaration_extension.hpp"

#include "ast/ast_declaration_local.hpp"
#include "nexus/ast/data.hpp"
#include "nexus/extension.hpp"
#include "nexus/ids.hpp"
#include "nexus/lexer/token.hpp"
#include "nexus/type/type.hpp"

#include "compiler/compilation_unit.hpp"

#include "parser/parser_declaration_local.hpp"
#include "parser/parser_base.hpp"
#include "parser/parser_type.hpp"
#include "parser_context.hpp"


constexpr std::string_view hint = R"(define a extension like:
  - ref function:     `extend T fn name(ref self, args...) {...}`
  - mut function:     `extend T fn name(mut self, args...) {...}`
  - static function:  `extend T fn name(args...) {...}`
  - operator:         `extend T op + {...}`
  - cast:             `extend T cast U {...}`)";

ast::ID parser::Parser_Declaration_Extension::parse_extension() noexcept
{
  (void)p.match(token::ETokenKind::EXTENSION);

  const auto tyid = p.p_type->parse_type();

  const auto tokkind = p.peek().kind;

  switch (tokkind) {
  case token::ETokenKind::FUNCTION: return extend_fn(tyid);
  case token::ETokenKind::AS:       return extend_cast(tyid);
  case token::ETokenKind::OP:       {
    (void)p.next();

    const auto  tok_op  = p.peek().kind;
    const auto& tok_str = p.peek().tokid.str();

    if (auto op = ast::ETokenKind_to_EOp_Unary(tok_op); op != ast::EOp_Unary::NONE) return extend_op_un(tyid);
    if (auto op = ast::ETokenKind_to_EOp_Bin(tok_op); op != ast::EOp_Bin::NONE) return extend_op_bin(tyid);
    if (auto op = ast::ETokenKind_to_ETransfertType(tok_op); op != ast::ETransfertType::NONE)
      return extend_op_transfert(tyid);
    if (tok_op == token::ETokenKind::L_SQUARE || tok_op == token::ETokenKind::INTERROGATIVE
        || tok_op == token::ETokenKind::TILDE)
      return extend_op_subscript(tyid);
    if (auto op = ast::ETokenStr_to_EOp_Other(tok_str); op != ast::EOp_Other::NONE) return extend_op_other(tyid);

    p.add_error(264, "Invalid operator", hint);
    return NO_ID;
  }
  default: p.add_error(263, "Invalid extend specification keyword.", hint); return NO_ID;
  }
}

ast::ID parser::Parser_Declaration_Extension::extend_fn(type::ID extended_type) noexcept
{
  (void)p.match(token::ETokenKind::FUNCTION);

  auto&      n         = p.add_get_node<ast::Global_Extend_Fn>(p.peek(-1).tokid);
  const auto old_ret   = p.current_returnable;
  p.current_returnable = n.nodeid();

  n.extended_type = extended_type;
  n.is_pure       = p.metablock_contains(p.peek().begin, "pure");

  n.name = p.parse_name();

  (void)p.add_definition(n.nodeid());
  p.enter_scope(n.nodeid(), "extend " + extended_type.dump() + " fn " + n.name);

  (void)p.expect(265, token::ETokenKind::L_PAREN, "Expected start of extension arguments '('", hint);

  if (p.check_any({token::ETokenKind::CAPA_REF, token::ETokenKind::CAPA_MUT})
      && p.check_at(1, token::ETokenKind::SELF)) {
    n.is_self_const = p.check(token::ETokenKind::CAPA_REF);

    {
      auto& self = p.p_base->inject_parameter(
          n.nodeid(), 0, "self", n.is_self_const ? ast::EPassMode::ref : ast::EPassMode::mut, extended_type);
      n.self       = self.nodeid();
      current_self = self.nodeid();
    }

    (void)p.next(); // consume ref/mut
    (void)p.next(); // consume self

    n.is_static = false;
  } else {
    n.is_static = true;
  }

  // some parameters to init
  (void)p.match(token::ETokenKind::COMMA);

  auto [protoid, params] = p.p_type->prototype_from_declaration(true);
  n.prototype            = protoid;
  n.parameters           = params;

  n.codeblock = p.p_loc->parse_codeblock_instruction();

  p.CU.extensions->add(extended_type, n.nodeid());

  p.exit_scope();
  p.current_returnable = old_ret;

  return n.nodeid();
}
ast::ID parser::Parser_Declaration_Extension::extend_cast(type::ID extended_type) noexcept
{
  (void)p.match(token::ETokenKind::AS);

  auto&      n         = p.add_get_node<ast::Global_Extend_Cast>(p.peek(-1).tokid);
  const auto old_ret   = p.current_returnable;
  p.current_returnable = n.nodeid();

  n.extended_type = extended_type;
  n.as_type       = p.p_type->parse_type();

  (void)p.add_definition(n.nodeid());
  p.enter_scope(n.nodeid(), "extend " + extended_type.dump() + " as " + n.as_type.dump());

  {
    auto& self   = p.p_base->inject_parameter(n.nodeid(), 0, "self", ast::EPassMode::ref, extended_type);
    n.self       = self.nodeid();
    current_self = self.nodeid();

    (void)p.add_definition(self.nodeid());
  }

  n.codeblock = p.p_loc->parse_codeblock_instruction();

  p.CU.extensions->add(extended_type, n.nodeid());

  p.exit_scope();
  p.current_returnable = old_ret;

  return n.nodeid();
}
ast::ID parser::Parser_Declaration_Extension::extend_op_bin(type::ID extended_type) noexcept
{
  const auto op = ast::ETokenKind_to_EOp_Bin(p.next().kind);

  auto&      n         = p.add_get_node<ast::Global_Extend_Op_Bin>(p.peek(-2).tokid);
  const auto old_ret   = p.current_returnable;
  p.current_returnable = n.nodeid();

  n.extended_type = extended_type;
  n.bin_op        = op;


  (void)p.add_definition(n.nodeid());
  p.enter_scope(n.nodeid(), "extend " + extended_type.dump() + " op " + std::string(ast::EOp_Bin_to_str(n.bin_op)));

  {
    auto& self   = p.p_base->inject_parameter(n.nodeid(), 0, "self", ast::EPassMode::ref, extended_type);
    n.self       = self.nodeid();
    current_self = self.nodeid();
  }
  {
    auto& other   = p.p_base->inject_parameter(n.nodeid(), 1, "other", ast::EPassMode::ref, extended_type);
    n.other       = other.nodeid();
    current_other = other.nodeid();
  }

  n.codeblock = p.p_loc->parse_codeblock_instruction();

  p.CU.extensions->add(extended_type, n.nodeid());

  p.exit_scope();
  p.current_returnable = old_ret;

  return n.nodeid();
}
ast::ID parser::Parser_Declaration_Extension::extend_op_un(type::ID extended_type) noexcept
{
  const auto op = ast::ETokenKind_to_EOp_Unary(p.next().kind);

  auto&      n         = p.add_get_node<ast::Global_Extend_Op_Un>(p.peek(-2).tokid);
  const auto old_ret   = p.current_returnable;
  p.current_returnable = n.nodeid();

  n.extended_type = extended_type;
  n.unary_op      = op;

  (void)p.add_definition(n.nodeid());
  p.enter_scope(n.nodeid(), "extend " + extended_type.dump() + " op " + std::string(ast::EOp_Unary_to_str(n.unary_op)));

  {
    auto& self   = p.p_base->inject_parameter(n.nodeid(), 0, "self", ast::EPassMode::ref, extended_type);
    n.self       = self.nodeid();
    current_self = self.nodeid();
  }

  n.codeblock = p.p_loc->parse_codeblock_instruction();

  p.CU.extensions->add(extended_type, n.nodeid());

  p.exit_scope();
  p.current_returnable = old_ret;

  return n.nodeid();
}
ast::ID parser::Parser_Declaration_Extension::extend_op_subscript(type::ID extended_type) noexcept
{
  ast::EOp_Subscript op = ast::EOp_Subscript::NONE;

  std::string index_name;
  std::string start_name;
  std::string end_name;

  if (p.match_chain({token::ETokenKind::L_SQUARE, token::ETokenKind::IDENTIFIER, token::ETokenKind::R_SQUARE})) {
    op         = ast::EOp_Subscript::_index;
    index_name = p.peek(-2).tokid.str();
  } else if (p.match_chain({token::ETokenKind::INTERROGATIVE, token::ETokenKind::L_SQUARE,
                            token::ETokenKind::IDENTIFIER, token::ETokenKind::R_SQUARE})) {
    op         = ast::EOp_Subscript::_index_bound;
    index_name = p.peek(-2).tokid.str();
  } else if (p.match_chain({token::ETokenKind::L_SQUARE, token::ETokenKind::IDENTIFIER, token::ETokenKind::RANGE,
                            token::ETokenKind::IDENTIFIER, token::ETokenKind::R_SQUARE})) {
    op         = ast::EOp_Subscript::_slice;
    start_name = p.peek(-2).tokid.str();
    end_name   = p.peek(-2).tokid.str();
  } else if (p.match_chain({token::ETokenKind::INTERROGATIVE, token::ETokenKind::L_SQUARE,
                            token::ETokenKind::IDENTIFIER, token::ETokenKind::RANGE, token::ETokenKind::IDENTIFIER,
                            token::ETokenKind::R_SQUARE}))
    op = ast::EOp_Subscript::_slice_bound;
  else if (p.match_chain({token::ETokenKind::TILDE, token::ETokenKind::L_SQUARE, token::ETokenKind::IDENTIFIER,
                          token::ETokenKind::RANGE, token::ETokenKind::IDENTIFIER, token::ETokenKind::R_SQUARE}))
    op = ast::EOp_Subscript::_b_slice;

  auto&      n         = p.add_get_node<ast::Global_Extend_Op_Subscript>(p.peek(-2).tokid);
  const auto old_ret   = p.current_returnable;
  p.current_returnable = n.nodeid();

  n.extended_type = extended_type;
  n.subscript_op  = op;

  // check return
  if (p.match(token::ETokenKind::ARROW)) {
    n.is_explicit_ret = true;
    n.ret             = p.p_type->parse_type();
  } else {
    n.is_explicit_ret = false;
    n.ret             = type::TYPEID_u0; // managed by resolver type
  }

  (void)p.add_definition(n.nodeid());
  p.enter_scope(n.nodeid(),
                "extend " + extended_type.dump() + " op " + std::string(ast::EOp_Subscript_to_str(n.subscript_op)));

  {
    auto& self   = p.p_base->inject_parameter(n.nodeid(), 0, "self", ast::EPassMode::ref, extended_type);
    n.self       = self.nodeid();
    current_self = self.nodeid();
  }

  n.codeblock = p.p_loc->parse_codeblock_instruction();

  p.CU.extensions->add(extended_type, n.nodeid());

  p.exit_scope();
  p.current_returnable = old_ret;

  return n.nodeid();
}
ast::ID parser::Parser_Declaration_Extension::extend_op_transfert(type::ID extended_type) noexcept
{
  ast::ETransfertType op = ast::ETokenKind_to_ETransfertType(p.peek().kind);

  auto&      n         = p.add_get_node<ast::Global_Extend_Op_Transfert>(p.peek(-2).tokid);
  const auto old_ret   = p.current_returnable;
  p.current_returnable = n.nodeid();

  n.extended_type = extended_type;
  n.transfert_op  = op;

  (void)p.add_definition(n.nodeid());
  p.enter_scope(n.nodeid(),
                "extend " + extended_type.dump() + " op " + std::string(ast::ETransfertType_to_str(n.transfert_op)));

  n.codeblock = p.p_loc->parse_codeblock_instruction();

  p.CU.extensions->add(extended_type, n.nodeid());

  p.exit_scope();
  p.current_returnable = old_ret;

  return n.nodeid();
}
ast::ID parser::Parser_Declaration_Extension::extend_op_other(type::ID extended_type) noexcept
{
  ast::EOp_Other op = ast::ETokenStr_to_EOp_Other(p.peek().tokid.str());

  auto&      n         = p.add_get_node<ast::Global_Extend_Op_Other>(p.peek(-2).tokid);
  const auto old_ret   = p.current_returnable;
  p.current_returnable = n.nodeid();

  n.extended_type = extended_type;
  n.other_op      = op;

  (void)p.add_definition(n.nodeid());
  p.enter_scope(n.nodeid(),
                "extend " + extended_type.dump() + " op " + std::string(magic_enum::enum_name(n.other_op)).substr(1));

  // UNUSED
  // forward future grammar
  // check return
  if (p.match(token::ETokenKind::ARROW)) {
    n.is_explicit_ret = true;
    n.ret             = p.p_type->parse_type();
  } else {
    n.is_explicit_ret = false;
    n.ret             = type::TYPEID_u0; // managed by resolver type
  }

  n.codeblock = p.p_loc->parse_codeblock_instruction();

  p.CU.extensions->add(extended_type, n.nodeid());

  p.exit_scope();
  p.current_returnable = old_ret;

  return n.nodeid();
}