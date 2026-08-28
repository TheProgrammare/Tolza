#include "parser_expression.hpp"

#include "ast/ast_expression.hpp"
#include "ast/ast_literal.hpp"
#include "ast/ast_operation.hpp"
#include "nexus/ast/data.hpp"
#include "nexus/ast/definition.hpp"
#include "nexus/ast/forward.hpp"
#include "nexus/forward.hpp"
#include "nexus/lexer/token.hpp"
#include "parser/parser_base.hpp"
#include "parser_context.hpp"
#include "parser_declaration_local.hpp"
#include "parser_literal.hpp"
#include "parser_operation.hpp"
#include "parser_type.hpp"

#include <vector>


ast::ID parser::Parser_Expression::parse_expression()
{
  if (p.match(token::ETokenKind::UNDERSCORE)) return BAD_NODE_ID;
  if (p.check(token::ETokenKind::IF)) return if_ternary();
  if (p.check(token::ETokenKind::NEW)) return new_ptr();
  if (p.check_val("move") && p.check_at(1, token::ETokenKind::TICK)) return move();
  if (p.check_val("mut") && p.check_at(1, token::ETokenKind::TICK)) return mut_of();
  if (p.check_val("ref") && p.check_at(1, token::ETokenKind::TICK)) return ref_of();
  if (p.check_val("copy") && p.check_at(1, token::ETokenKind::TICK)) return copy_of();
  if (p.check_val("size") && p.check_at(1, token::ETokenKind::TICK)) return size_of();
  if (p.check_val("addr") && p.check_at(1, token::ETokenKind::TICK)) return addr_of();
  if (p.check(token::ETokenKind::AMPERSAND)) return addr_of();
  if (p.check_val("val") && p.check_at(1, token::ETokenKind::TICK)) return ptr_val();
  if (p.check(token::ETokenKind::OP_MULTIPLY)) return ptr_val();

  auto op = p.p_op->try_operation();

  while (!p.is_end()) {
    if (!p.check_any(token::k_operator)) return op;

    auto& node = p.add_get_node<ast::Operation_Binary>(p.peek().tokid);
    node.left  = op;
    node.op_ty = ast::ETokenKind_to_EOp_Bin(p.next().kind);
    node.right = parse_expression();
    op         = node.nodeid();
  }

  return op;
}
ast::ID parser::Parser_Expression::parse_expression_term()
{
  if (auto term = base_expression()) return suffix_expression(term);

  THROW_BAD_NODE;
}

ast::ID parser::Parser_Expression::base_expression()
{
  constexpr std::string_view hint =
      "(define a term with literal, identifier, ternary if, tuple, nested expression '()', literal array '{}' or "
      "nothing '_'.)";

  ast::ID base_expr;

  ast::EExprPassMode pass_mode = ast::ETokenKind_to_EExprPassMode(p.peek().kind);
  if (pass_mode != ast::EExprPassMode::NONE) {
    (void)p.next();
    (void)p.expect(259, token::ETokenKind::TICK, "Expected ''' tick after a pass mode prefix", hint);
  }

  if (p.match(token::ETokenKind::L_PAREN)) {
    base_expr = parse_expression();
    (void)p.expect(78, token::ETokenKind::R_PAREN, "Expected end of nested expression ')'", "");
    return base_expr;
  }

  if (auto lit = p.p_lit->try_literal(true)) return lit;

  if (p.check_any(token::k_start_identifier)) {
    auto id = p.p_base->identifier();
    if (p.check_chain({token::ETokenKind::L_CURLY, token::ETokenKind::DOT})) return p.p_lit->literal_facet(id);
    if (p.check_chain({token::ETokenKind::L_CURLY, token::ETokenKind::AT})) return p.p_lit->literal_form(id);
    return id;
  }


  p.add_error(79, std::format("Unexpected '{}' keyword.", p.tok_to_str(p.peek().tokid)), hint);

  THROW_BAD_NODE;
}

ast::ID parser::Parser_Expression::assign(ast::ID left)
{
}


ast::ID parser::Parser_Expression::suffix_expression(ast::ID p_base_expr)
{
  if (!p_base_expr) THROW_BAD_NODE;

  // is a literal expression, no suffix allowed
  if (ast::ENodeKind_is_literal(p_base_expr.kind())) {
    // only range and cast suffix allowed
    if (p.check_any({token::ETokenKind::RANGE, token::ETokenKind::RANGE_INCLUSIVE})) {
      p_base_expr = p.p_lit->literal_range(p_base_expr);
    }

    // if cast
    if (p.check(token::ETokenKind::AS)) p_base_expr = cast_as(p_base_expr);

    return p_base_expr;
  }


  // supported access operators:
  // member access : a.b
  // function call : a(b)
  // expand call : a.b()
  // run rule : a->b()
  // ptr at : a'at(b)
  // ptr offset : a'offset(b)
  // table access : a[i]

  while (!p.is_end()) {
    // member access
    if (p.check(token::ETokenKind::DOT)) {
      p_base_expr = member_access(p_base_expr);
    }
    // function call
    else if (p.check(token::ETokenKind::L_PAREN)) {
      assert(ast::ENodeKind_is_symbol(p_base_expr.kind()));
      p_base_expr = function_call(p_base_expr);
    }
    // run rule
    else if (p.check(token::ETokenKind::ARROW)) {
      p_base_expr = rule_call(p_base_expr);
    }
    // table access
    else if (p.match_any({token::ETokenKind::INTERROGATIVE, token::ETokenKind::L_SQUARE})) {
      p_base_expr = table_access(p_base_expr);
    }
    // end of access operator
    else {
      break;
    }
  }

  // final access operator:
  // no more access allowed after this operations
  // specials cases

  // get bits
  // a~[0..8]
  if (p.check(token::ETokenKind::TILDE)) {
    p_base_expr = getbits(p_base_expr);
    // table access on bits
    // a~[0..8][5]
    if (p.check(token::ETokenKind::L_SQUARE)) {
      p_base_expr = table_access(p_base_expr);
    }
  }
  // it's a range expression !
  else if (p.check_any({token::ETokenKind::RANGE, token::ETokenKind::RANGE_INCLUSIVE})) {
    p_base_expr = p.p_lit->literal_range(p_base_expr);
  }

  // if cast
  if (p.check(token::ETokenKind::AS)) p_base_expr = cast_as(p_base_expr);

  return p_base_expr;
}


ast::ID parser::Parser_Expression::cast_as(ast::ID p_expr)
{
  auto& cast_as = p.add_get_node<ast::Operation_Cast_As>(p.peek().tokid);

  assert(p.check(token::ETokenKind::AS) && "Unexpected token encounted in casting");
  (void)p.next(); // consume as

  if (p.match(token::ETokenKind::EXCLAMATION))
    cast_as.cast_type = ast::Operation_Cast_As::ECastType::AS_REINTERPRET;
  else if (p.match(token::ETokenKind::INTERROGATIVE))
    cast_as.cast_type = ast::Operation_Cast_As::ECastType::AS_SAFE;

  cast_as.expression = p_expr;
  cast_as.type       = p.p_type->parse_type();

  return cast_as.nodeid();
}

std::vector<ast::ID> parser::Parser_Expression::call_arguments()
{
  constexpr std::string_view hint =
      R"(define call argument like:
  - ordinal `call(10)`  
  - named `call(param_name: 10)`
  - variadic `call(... 10, 20, 30)`)";

  if (p.match(token::ETokenKind::R_PAREN)) return {};

  std::vector<ast::ID> args;

  while (!p.is_end()) {


    auto& arg = p.add_get_node<ast::Expression_Invocation_Arg>(p.peek().tokid);

    // if parameter invocation
    if (p.check_chain({token::ETokenKind::IDENTIFIER, token::ETokenKind::ASSIGN})) {
      arg.explicit_name = p.tok_to_str(p.next().tokid);

      (void)p.expect(111, token::ETokenKind::COLON,
                     "Expected parameter assignation ':' after a parameter argument name invocation.", hint);

      arg.expression = p.p_expr->parse_expression();
      args.emplace_back(arg.nodeid());
    }
    // ordered parameter affectation
    else {
      arg.expression = p.p_expr->parse_expression();

      args.emplace_back(arg.nodeid());
    }

    // stop when an unexpected token is encounted permit to avoid ;
    // for pipe calls args are always separated by comma or ... so other token indicates a terminaison
    if (p.match_any({token::ETokenKind::COMMA, token::ETokenKind::VARIADIC})) continue;
    if (p.match(token::ETokenKind::R_PAREN)) break;
  }

  return args;
}

ast::ID parser::Parser_Expression::function_call(ast::ID p_callee)
{
  (void)p.match(token::ETokenKind::L_PAREN);

  auto& call     = p.add_get_node<ast::Expression_Invocation>(p.peek().tokid);
  call.callee    = p_callee;
  call.arguments = call_arguments();
  if (call.arguments.empty()) call.invocation_kind = ast::EInvocationKind::fn_call;

  return call.nodeid();
}

ast::ID parser::Parser_Expression::rule_call(ast::ID p_target_form)
{
  (void)p.match(token::ETokenKind::ARROW);

  auto& base_tok = p.peek(-1);

  auto& rule_call       = p.add_get_node<ast::Expression_Invocation_Rule>(p.peek().tokid);
  rule_call.target_form = p_target_form;
  rule_call.callee      = p.p_base->identifier();
  rule_call.arguments   = call_arguments();

  return rule_call.nodeid();
}


ast::ID parser::Parser_Expression::if_ternary()
{
  constexpr std::string_view hint =
      "define ternary if like: if <condition> => <true_expression> [else => <false_expression>]";

  auto& ternary     = p.add_get_node<ast::Expression_If_Ternary>(p.peek().tokid);
  ternary.evaluator = p.p_loc->parse_evaluator();

  (void)p.expect(209, token::ETokenKind::INJECT, "Expected inject token '=>' after condition.", hint);
  ternary.statement_true = p.p_expr->parse_expression();

  if (p.match(token::ETokenKind::ELSE)) {
    (void)p.match(token::ETokenKind::INJECT);
    ternary.statement_false = p.p_expr->parse_expression();
  }

  return ternary.nodeid();
}


ast::ID parser::Parser_Expression::member_access(ast::ID p_left)
{
  (void)p.match(token::ETokenKind::DOT);

  auto& access            = p.add_get_node<ast::Expression_Member_Access>(p.peek().tokid);
  access.left_expression  = p_left;
  access.right_identifier = p.p_base->identifier(true);

  return access.nodeid();
}

ast::ID parser::Parser_Expression::table_access(ast::ID p_target)
{
  const bool is_bound = p.check_at(-1, token::ETokenKind::INTERROGATIVE);
  (void)p.match(token::ETokenKind::L_SQUARE);

  auto& table_access    = p.add_get_node<ast::Expression_Table_Access>(p.peek().tokid);
  table_access.bounded  = is_bound;
  table_access.target   = p_target;
  table_access.selector = p.p_expr->parse_expression();

  (void)p.expect(115, token::ETokenKind::R_SQUARE, "Expected end of table access '[' after expression.", "");

  return table_access.nodeid();
}


ast::ID parser::Parser_Expression::ptr_val()
{
  auto& node = p.add_get_node<ast::Expression_Ptr_Val>(p.peek().tokid);

  (void)p.match_val("val");
  (void)p.match(token::ETokenKind::TICK);
  (void)p.match(token::ETokenKind::OP_MULTIPLY);
  node.target = p.p_expr->parse_expression();

  return node.nodeid();
}

ast::ID parser::Parser_Expression::ref_of()
{
  auto& node = p.add_get_node<ast::Expression_Ref_Of>(p.peek().tokid);

  (void)p.match_val("ref");
  (void)p.match(token::ETokenKind::TICK);
  node.target = p.p_expr->parse_expression();

  return node.nodeid();
}
ast::ID parser::Parser_Expression::mut_of()
{
  auto& node = p.add_get_node<ast::Expression_Mut_Of>(p.peek().tokid);

  (void)p.match_val("mut");
  (void)p.match(token::ETokenKind::TICK);
  node.target = p.p_expr->parse_expression();

  return node.nodeid();
}
ast::ID parser::Parser_Expression::copy_of()
{
  auto& node = p.add_get_node<ast::Expression_Copy_Of>(p.peek().tokid);

  (void)p.match_val("copy");
  (void)p.match(token::ETokenKind::TICK);
  node.target = p.p_expr->parse_expression();

  return node.nodeid();
}

ast::ID parser::Parser_Expression::addr_of()
{
  auto& node = p.add_get_node<ast::Expression_Addr_Of>(p.peek().tokid);

  (void)p.match_val("addr");
  (void)p.match(token::ETokenKind::TICK);
  (void)p.match(token::ETokenKind::AMPERSAND);
  node.target = p.p_expr->parse_expression();

  return node.nodeid();
}

ast::ID parser::Parser_Expression::getbits(ast::ID p_expr)
{
  constexpr std::string_view hint = "define get bit like: `target~[0..8]` get first octect on target.";

  (void)p.match(token::ETokenKind::TILDE);
  (void)p.expect(98, token::ETokenKind::L_SQUARE, "Expected start slice block '[' after a get bit operator '~'", hint);


  auto& get_bit  = p.add_get_node<ast::Expression_GetBits>(p.peek().tokid);
  get_bit.target = p_expr;
  get_bit.range  = p.p_expr->parse_expression();

  (void)p.expect(99, token::ETokenKind::R_SQUARE, "Expected end slice block ']' after range expression", hint);

  return get_bit.nodeid();
}

ast::ID parser::Parser_Expression::size_of()
{
  constexpr std::string_view hint = "define value Expression size like: `size'a`";
  auto&                      node = p.add_get_node<ast::Expression_Size_Of>(p.peek().tokid);

  (void)p.expect(104, token::ETokenKind::L_PAREN, "Expected start arg '('.", hint);
  node.target = p.p_expr->parse_expression();
  (void)p.expect(105, token::ETokenKind::R_PAREN, "Expected end arg ')'.", hint);

  return node.nodeid();
}

ast::ID parser::Parser_Expression::move()
{
  (void)p.match_val("move");
  (void)p.match(token::ETokenKind::TICK);

  auto& node  = p.add_get_node<ast::Expression_Move_Of>(p.peek().tokid);
  node.target = p.p_expr->parse_expression();

  return node.nodeid();
}

ast::ID parser::Parser_Expression::new_ptr()
{
  constexpr std::string_view hint =
      R"(define a dynamic Expression allocation:
  - primitive `var myPtr = new ptr'i32(10)`
  - array `var myPtr = new ptr'[i32 -> 3]({ 1, 2, 3 })`
  - array on all `var myPtr = new ptr'[i32 -> 3](0)`
  - form `var myPtr = new ptr'Person(Person{ CIdentity{ name: \"Zagreus\", age: 25 } })`)";

  (void)p.match(token::ETokenKind::NEW);

  auto& node = p.add_get_node<ast::Expression_New_Ptr>(p.peek().tokid);
  (void)p.expect(100, token::ETokenKind::PTR, "Expected pointer specification after 'new' token.", hint);
  (void)p.expect(101, token::ETokenKind::TICK, "Expected tick ' between pointer and type", hint);

  node.type = p.p_type->parse_type();

  (void)p.expect(102, token::ETokenKind::L_PAREN, "Expected start value '(' after type", hint);
  node.expression = p.p_expr->parse_expression();
  (void)p.expect(103, token::ETokenKind::R_PAREN, "Expected end value ')'", hint);

  return node.nodeid();
}
