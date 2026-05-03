#include "parser_expression.hpp"

#include <memory>
#include <stdexcept>
#include <utility>
#include <iostream>
#include <vector>

#include "nexus/ast/ast.hpp"
#include "ast/ast_expression.hpp"
#include "ast/ast_operation.hpp"
#include "ast/ast_literal.hpp"

#include "nexus/lexer/token.hpp"
#include "nexus/forward.hpp"
#include "parser/parser_base.hpp"
#include "parser_context.hpp"
#include "parser_literal.hpp"
#include "parser_declaration_local.hpp"
#include "parser_type.hpp"
#include "parser_operation.hpp"

#include "nexus/lexer/token_viewer.hpp"
#include "nexus/script.hpp"

ast::_gnid parser::Parser_Expression::parse_expression()
{
  if (p.match(token::ETokenKind::UNDERSCORE)) return BAD_NODE_ID;
  if (p.check(token::ETokenKind::IF)) return if_ternary();
  if (p.check(token::ETokenKind::NEW)) return new_ptr();
  if (p.check(token::ETokenKind::CAPA_MOVE_OF)) return move();
  if (p.check(token::ETokenKind::CAPA_MUT_OF)) return mut_of();
  if (p.check(token::ETokenKind::CAPA_REF_OF)) return ref_of();
  if (p.check(token::ETokenKind::CAPA_COPY_OF)) return copy_of();

  auto op = p.p_op->try_operation();

  while (!p.is_end()) {
    if (!p.check_any({token::k_operator})) return op;

    parser_add_node(node, Operation_Binary, p.peek().id);
    node->left  = std::move(op);
    node->op_ty = ast::ETokenKind_to_EBinOpType(p.next().kind);
    node->right = parse_expression();
    op          = node->node_id;
  }

  return op;
}
ast::_gnid parser::Parser_Expression::parse_expression_term()
{
  if (p.check(token::ETokenKind::VAL_OF)) return ptr_val();
  if (p.check(token::ETokenKind::ADDR_OF)) return addr_of();
  if (p.check(token::ETokenKind::SIZE_OF)) return size_of();

  if (auto term = base_expression()) return suffix_expression(term);

  return BAD_NODE_ID;
}

ast::_gnid parser::Parser_Expression::base_expression()
{
  constexpr std::string_view hint =
      "(define a term with literal, identifier, ternary if, tuple, nested expression '()', literal array '{}' or "
      "nothing '_'.)";

  ast::_gnid base_expr;

  ast::EExprPassMode pass_mode = ast::ETokenKind_to_EExprPassMode(p.peek().kind);
  if (pass_mode != ast::EExprPassMode::NONE) p.next();

  if (p.match(token::ETokenKind::OPEN_PAREN)) {
    base_expr = parse_expression();
    p.expect(78, token::ETokenKind::CLOSE_PAREN, "Expected end of nested expression ')'", "");
    return base_expr;
  }

  if (auto lit = p.p_lit->try_literal(true)) {
    return lit;
  } else if (p.check_any(token::k_start_identifier)) {
    return p.p_base->identifier();
  }

  p.add_error(79, "Unexpected '" + std::string(p.tok_to_str(p.peek().id)) + "' keyword.", hint);

  return BAD_NODE_ID;
}


ast::_gnid parser::Parser_Expression::suffix_expression(ast::_gnid& p_base_expr)
{
  auto& node = p.scr_info.nodes->get(p_base_expr.get_node_id());

  // is a literal expression, no suffix allowed
  if (ast::ENodeKind_is_literal(node.kind())) {
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
  // run system : a::>b()
  // ptr at : a'at(b)
  // ptr offset : a'offset(b)
  // table access : a[i]
  // These operators can be chained:
  // a.b'at(1).c'offset(2)[5].e::>run_sys(1, 2).u(10).v
  // (highly not recommended :( )

  while (!p.is_end()) {
    // member access
    if (p.check(token::ETokenKind::DOT)) {
      p_base_expr = member_access(p_base_expr);
    }
    // function call
    else if (p.check(token::ETokenKind::OPEN_PAREN)) {
      p_base_expr = function_call(p_base_expr);
    }
    // run system
    else if (p.check(token::ETokenKind::RUN_SYSTEM)) {
      p_base_expr = system_call(p_base_expr);
    }
    // table access
    else if (p.match_any({token::ETokenKind::INTERROGATIVE, token::ETokenKind::OPEN_SQUARE})) {
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
    if (p.check(token::ETokenKind::OPEN_SQUARE)) {
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


ast::_gnid parser::Parser_Expression::cast_as(ast::_gnid& p_expr)
{
  parser_add_node(cast_as, Operation_Cast_As, p.peek().id);

  assert(p.check(token::ETokenKind::AS) && "Unexpected token encounted in casting");
  p.next(); // consume as

  if (p.match(token::ETokenKind::EXCLAMATION))
    cast_as->cast_type = ast::Operation_Cast_As::ECastType::AS_REINTERPRET;
  else if (p.match(token::ETokenKind::INTERROGATIVE))
    cast_as->cast_type = ast::Operation_Cast_As::ECastType::AS_SAFE;

  cast_as->expression = std::move(p_expr);
  cast_as->type       = p.p_type->parse_type();

  return cast_as->node_id;
}

std::vector<ast::_gnid> parser::Parser_Expression::call_arguments()
{
  constexpr std::string_view hint =
      R"(define call argument like:
  - ordinal `call(10)`  
  - named `call(param_name: 10)`
  - variadic `call(... 10, 20, 30)`)";

  if (p.match(token::ETokenKind::CLOSE_PAREN)) return {};

  std::vector<ast::_gnid> args;

  while (!p.is_end()) {
    parser_add_node(arg, Expression_Call_Argument, p.peek().id);

    // if parameter invocation
    if (p.check(token::ETokenKind::IDENTIFIER) && p.check_at(1, token::ETokenKind::ASSIGN)) {
      arg->explicit_name = p.tok_to_str(p.next().id);

      p.expect(111, token::ETokenKind::COLON,
               "Expected parameter assignation ':' after a parameter argument name invocation.", hint);

      arg->expression = p.p_expr->parse_expression();
      args.push_back(arg->node_id);
    }
    // ordered parameter affectation
    else {
      arg->expression = p.p_expr->parse_expression();

      args.push_back(arg->node_id);
    }

    // stop when an unexpected token is encounted permit to avoid ;
    // for pipe calls args are always separated by comma or ... so other token indicates a terminaison
    if (p.match_any({token::ETokenKind::COMMA, token::ETokenKind::VARIADIC})) continue;
    p.match(token::ETokenKind::CLOSE_PAREN);
    break;
  }

  return args;
}

ast::_gnid parser::Parser_Expression::function_call(ast::_gnid& p_callee)
{
  p.match(token::ETokenKind::OPEN_PAREN);

  parser_add_node(call, Expression_Call, p.peek().id);
  call->callee    = p_callee;
  call->arguments = call_arguments();

  // if (auto ptr = dynamic_cast<ast::AIdentifier*>(call->callee.get())) p.sym_m->check_if_unresolved_extern_sym(*ptr);

  return call->node_id;
}

ast::_gnid parser::Parser_Expression::system_call(ast::_gnid& p_target_entity)
{
  p.match(token::ETokenKind::RUN_SYSTEM);

  auto base_tok = p.peek(-1);

  parser_add_node(sys_call, Expression_Call_System, p.peek().id);
  sys_call->target_entity = p_target_entity;
  sys_call->callee        = p.p_base->identifier();
  sys_call->arguments     = call_arguments();

  return sys_call->node_id;
}


ast::_gnid parser::Parser_Expression::if_ternary()
{
  constexpr std::string_view hint =
      "define ternary if like: if <condition> => <true_expression> [else => <false_expression>]";

  parser_add_node(ternary, Expression_If_Ternary, p.peek().id);
  ternary->evaluator = p.p_loc->parse_evaluator();

  p.expect(209, token::ETokenKind::INJECT, "Expected inject token '=>' after condition.", hint);
  ternary->statement_true = p.p_expr->parse_expression();

  if (p.match(token::ETokenKind::ELSE)) {
    p.match(token::ETokenKind::INJECT);
    ternary->statement_false = p.p_expr->parse_expression();
  }

  return ternary->node_id;
}


ast::_gnid parser::Parser_Expression::member_access(ast::_gnid& p_left)
{
  p.match(token::ETokenKind::DOT);

  parser_add_node(access, Expression_Member_Access, p.peek().id);
  access->left_expression  = p_left;
  access->right_identifier = p.p_base->identifier(true);

  return access->node_id;
}

ast::_gnid parser::Parser_Expression::table_access(ast::_gnid& p_target)
{
  const bool is_bound = p.check_at(-1, token::ETokenKind::INTERROGATIVE);
  p.match(token::ETokenKind::OPEN_SQUARE);

  parser_add_node(table_access, Expression_Table_Access, p.peek().id);
  table_access->bounded  = is_bound;
  table_access->target   = p_target;
  table_access->selector = p.p_expr->parse_expression();

  p.expect(115, token::ETokenKind::CLOSE_SQUARE, "Expected end of table access '[' after expression.", "");

  return table_access->node_id;
}


ast::_gnid parser::Parser_Expression::ptr_val()
{
  parser_add_node(node, Expression_Ptr_Val, p.peek().id);

  p.match(token::ETokenKind::VAL_OF);
  node->target = p.p_expr->parse_expression();

  return node->node_id;
}

ast::_gnid parser::Parser_Expression::ref_of()
{
  parser_add_node(node, Expression_Ref_Of, p.peek().id);

  p.match(token::ETokenKind::CAPA_REF_OF);
  node->target = p.p_expr->parse_expression();

  return node->node_id;
}
ast::_gnid parser::Parser_Expression::mut_of()
{
  parser_add_node(node, Expression_Mut_Of, p.peek().id);

  p.match(token::ETokenKind::CAPA_MUT_OF);
  node->target = p.p_expr->parse_expression();

  return node->node_id;
}
ast::_gnid parser::Parser_Expression::copy_of()
{
  parser_add_node(node, Expression_Copy_Of, p.peek().id);

  p.match(token::ETokenKind::CAPA_COPY_OF);
  node->target = p.p_expr->parse_expression();

  return node->node_id;
}

ast::_gnid parser::Parser_Expression::addr_of()
{
  parser_add_node(node, Expression_Addr_Of, p.peek().id);

  p.match(token::ETokenKind::ADDR_OF);
  node->target = p.p_expr->parse_expression();

  return node->node_id;
}

ast::_gnid parser::Parser_Expression::getbits(ast::_gnid& p_expr)
{
  constexpr std::string_view hint = "define get bit like: `target~[0..8]` get first octect on target.";

  p.match(token::ETokenKind::TILDE);
  p.expect(98, token::ETokenKind::OPEN_SQUARE, "Expected start slice block '[' after a get bit operator '~'", hint);


  parser_add_node(get_bit, Expression_GetBits, p.peek().id);
  get_bit->target = p_expr;
  get_bit->range  = p.p_expr->parse_expression();

  p.expect(99, token::ETokenKind::CLOSE_SQUARE, "Expected end slice block ']' after range expression", hint);

  return get_bit->node_id;
}

ast::_gnid parser::Parser_Expression::size_of()
{
  constexpr std::string_view hint = "define value Expression size like: `size'a`";
  parser_add_node(node, Expression_Size_Of, p.peek().id);

  p.expect(104, token::ETokenKind::OPEN_PAREN, "Expected start arg '('.", hint);
  node->target = p.p_expr->parse_expression();
  p.expect(105, token::ETokenKind::CLOSE_PAREN, "Expected end arg ')'.", hint);

  return node->node_id;
}

ast::_gnid parser::Parser_Expression::move()
{
  p.match(token::ETokenKind::CAPA_MOVE_OF);

  parser_add_node(node, Expression_Move_Of, p.peek().id);
  node->target = p.p_expr->parse_expression();

  return node->node_id;
}

ast::_gnid parser::Parser_Expression::new_ptr()
{
  constexpr std::string_view hint =
      R"(define a dynamic Expression allocation:
  - primitive `var myPtr = new ptr'i32(10)`
  - array `var myPtr = new ptr'[i32 -> 3]({ 1, 2, 3 })`
  - array on all `var myPtr = new ptr'[i32 -> 3](0)`
  - entity `var myPtr = new ptr'Person(Person{ CIdentity{ name: \"Zagreus\", age: 25 } })`)";

  p.match(token::ETokenKind::NEW);

  parser_add_node(node, Expression_New_Ptr, p.peek().id);
  p.expect(100, token::ETokenKind::PTR, "Expected pointer specification after 'new' token.", hint);
  p.expect(101, token::ETokenKind::TICK, "Expected tick ' between pointer and type", hint);

  node->type = p.p_type->parse_type();

  p.expect(102, token::ETokenKind::OPEN_PAREN, "Expected start value '(' after type", hint);
  node->expression = p.p_expr->parse_expression();
  p.expect(103, token::ETokenKind::CLOSE_PAREN, "Expected end value ')'", hint);

  return node->node_id;
}
