#include "parser_expression.hpp"

#include <memory>
#include <utility>

#include "compiler/ast/ast_base.hpp"
#include "compiler/ast/ast_data.hpp"
#include "compiler/ast/ast_expression.hpp"
#include "compiler/ast/ast_operation.hpp"

#include "compiler/lexer/token.hpp"
#include "parser_context.hpp"
#include "parser_literal.hpp"
#include "parser_declaration_local.hpp"
#include "parser_type.hpp"
#include "parser_operation.hpp"

#include "compiler/lexer/token_viewer.hpp"

std::unique_ptr<ast::AExpression> parser::Parser_Expression::parse_expression()
{
  if (ctx.tok_v.check(TokTy::IF)) return if_ternary();
  if (ctx.tok_v.check(TokTy::NEW)) return new_ptr();
  if (ctx.tok_v.check(TokTy::CAPA_MOVE_OF)) return move();

  return ctx.p_op->try_operation();
}

std::unique_ptr<ast::AExpression> parser::Parser_Expression::parse_expression_term()
{
  if (ctx.tok_v.check(TokTy::VAL_OF)) return ptr_val();
  if (ctx.tok_v.check(TokTy::ADDR_OF)) return addr_of();
  if (ctx.tok_v.check(TokTy::SIZE_OF)) return size_of();

  std::unique_ptr<ast::AExpression> term;

  EExprPassMode pass_mode = TokTy_to_EExprPassMode(ctx.tok_v.peek().type);
  if (pass_mode != EExprPassMode::NONE) ctx.tok_v.next();

  if (ctx.tok_v.check(TokTy::OPEN_PAREN)) {
    term = parse_expression();
    ctx.tok_v.expect<78>(TokTy::CLOSE_PAREN, "Expected end of nested expression ')'", "");
  }

  if (auto lit = ctx.p_lit->try_literal(true)) {
    term = std::move(lit.value());
  } else if (ctx.tok_v.check_any(kStartIdentifier)) {
    auto id = identifier();
    if (ctx.tok_v.check(TokTy::OPEN_PAREN)) {
      auto call = function_call();

      ctx.m_sym->add_external_symbol(*id, Extern_Item::Kind::Function);

      call->callee = std::move(id);
      term         = std::move(call);
    } else {
      term = std::move(id);
    }

    if (dynamic_cast<ast::Expr_ID*>(id.get())) {
      while (!ctx.tok_v.is_end()) {
        if (ctx.tok_v.check(TokTy::DOT)) {
          auto access  = member_access();
          access->left = std::move(term);
          term         = std::move(access);
        } else if (ctx.tok_v.match(TokTy::RUN_SYSTEM)) {
          auto base_tok   = ctx.tok_v.peek(-1);
          auto sys_callee = identifier();

          ctx.m_sym->add_external_symbol(*sys_callee, Extern_Item::Kind::System);

          auto sys_call           = ctx.Create_Node<ast::expression::Call_System>(base_tok);
          sys_call->callee        = std::move(sys_callee);
          sys_call->target_entity = std::move(term);
          term                    = std::move(sys_call);
        } else {
          break;
        }
      }
    } else {
      ctx.m_sym->add_external_symbol(*id, Extern_Item::Kind::Global);
    }


    if (ctx.tok_v.check(TokTy::TILDE))
      term = getbits(std::move(term));
    else if (ctx.tok_v.check(TokTy::PTR_AT))
      term = ptr_at(std::move(term));
    else if (ctx.tok_v.match(TokTy::PTR_OFFSET))
      term = ptr_offset(std::move(term));
  }

  if (!term)
    ctx.tok_v.add_error<79>("Unexpected '" + ctx.tok_v.peek().val + "' keyword.",
                            "define a term with literal, identifier, ternary if, tuple, nested "
                            "expression '()', literal array '{}' or nothing '_'.");

  if (ctx.tok_v.check_any({TokTy::RANGE, TokTy::RANGE_INCLUSIVE})) {
    term = ctx.p_lit->literal_range(std::move(term));
  }

  // if cast
  if (ctx.tok_v.check_any(kCastType)) {
    term = cast_as(std::move(term));
  }

  return term;
}

std::unique_ptr<ast::operation::Cast_As> parser::Parser_Expression::cast_as(std::unique_ptr<ast::AExpression> expr)
{
  auto asCast = ctx.Create_Node<ast::operation::Cast_As>(ctx.tok_v.peek());

  switch (ctx.tok_v.peek().type) {
  case TokTy::AS:             asCast->cast_type = ast::operation::Cast_As::ECastType::AS; break;
  case TokTy::AS_REINTERPRET: asCast->cast_type = ast::operation::Cast_As::ECastType::AS_REINTERPRET; break;
  case TokTy::AS_SAFE:        asCast->cast_type = ast::operation::Cast_As::ECastType::AS_SAFE; break;
  default:                    ctx.tok_v.add_error<1000>("Unexpected token encounted in casting", "");
  }

  ctx.tok_v.next(); // consume as
  asCast->valueCasted = std::move(expr);
  asCast->typeCasted  = ctx.p_type->parse_type();

  return asCast;
}

std::vector<std::unique_ptr<ast::expression::Call_Argument>> parser::Parser_Expression::call_arguments()
{
  static const std::string hint =
      "define call argument like:"
      "\n  - ordinal `call(10)`  "
      "\n  - named `call(param_name: 10)`"
      "\n  - variadic `call(... 10, 20, 30)`";

  if (ctx.tok_v.match(TokTy::CLOSE_PAREN)) return {};

  std::vector<std::unique_ptr<ast::expression::Call_Argument>> params;

  while (!ctx.tok_v.is_end()) {
    auto param = ctx.Create_Node<ast::expression::Call_Argument>(ctx.tok_v.peek());

    // if parameter invocation
    if (ctx.tok_v.check(TokTy::IDENTIFIER) && ctx.tok_v.peek(1).type == TokTy::ASSIGN) {
      param->name = ctx.tok_v.next().val;

      ctx.tok_v.expect<111>(TokTy::COLON,
                            "Expected parameter assignation ':' after a parameter argument name invocation.", hint);

      param->expression = ctx.p_expr->parse_expression();
      params.push_back(std::move(param));
    }
    // ordered parameter affectation
    else {
      param->expression = ctx.p_expr->parse_expression();

      params.push_back(std::move(param));
    }

    // stop when an unexpected token is encounted permit to avoid ;
    // for pipe calls args are always separated by comma or ... so other token indicates a terminaison
    if (ctx.tok_v.match_any({TokTy::COMMA, TokTy::VARIADIC})) continue;
    ctx.tok_v.match(TokTy::CLOSE_PAREN);
    break;
  }

  return params;
}

std::unique_ptr<ast::expression::Call> parser::Parser_Expression::function_call()
{
  ctx.tok_v.match(TokTy::OPEN_PAREN);

  auto call        = ctx.Create_Node<ast::expression::Call>(ctx.tok_v.peek());
  call->param_args = call_arguments();

  return call;
}

ModuleImportation* parser::Parser_Expression::get_external_source(const std::string&              name,
                                                                  const std::vector<std::string>& path)
{
  if (path.empty()) return nullptr;
  const std::string& base = path[0];
  return ctx.scr_info.get_import_module(base);
}

std::unique_ptr<ast::expression::If_Ternary> parser::Parser_Expression::if_ternary()
{
  auto ternary = ctx.Create_Node<ast::expression::If_Ternary>(ctx.tok_v.peek());

  ternary->evaluator = ctx.p_loc->parse_evaluator(nullptr);
  ternary->true_line = ctx.p_loc->code_block_instruction();

  if (ctx.tok_v.match(TokTy::ELSE)) {
    ternary->false_line = ctx.p_loc->code_block_instruction();
  }

  return ternary;
}

std::unique_ptr<ast::AIdentifier> parser::Parser_Expression::identifier(bool no_qualified_id, bool keyword_allowed)
{
  static const std::string hint =
      "define identifier like:"
      "\n  - classic `name` -> name"
      "\n  - with scope path `mod A { name }` -> A_name"
      "\n  - with qualified id `A::B::C` -> A_B_C";

  bool root_scope    = false;
  bool parent_scope  = false;
  bool current_scope = false;

  if (ctx.tok_v.match(TokTy::STATIC_ACCESS)) {
    root_scope = true;
  } else if (ctx.tok_v.match(TokTy::SUPER_MOD)) {
    parent_scope = true;
  } else if (ctx.tok_v.match(TokTy::SELF_MOD)) {
    current_scope = true;
  }
  // it's a simple id with no path
  else if (ctx.tok_v.peek(1).type != TokTy::STATIC_ACCESS) {
    auto id = ctx.Create_Node<ast::Expr_ID>(ctx.tok_v.peek());
    if (!keyword_allowed)
      id->name = ctx.parse_name("", hint);
    else
      id->name = ctx.tok_v.next().val;

    return id;
  }

  // it's qualified id
  if (no_qualified_id) ctx.tok_v.add_error_tok<113>(ctx.tok_v.peek(-1), "Unexpected qualified id.", hint);

  auto   id    = ctx.Create_Node<ast::Expr_ID_Qualified>(ctx.tok_v.peek());
  size_t count = 0;
  while (!ctx.tok_v.is_end()) {
    id->path.push_back(ctx.tok_v.next().val);

    ctx.tok_v.match(TokTy::STATIC_ACCESS);

    // no more path : the last element is the name
    if (ctx.tok_v.peek(1).type != TokTy::STATIC_ACCESS) {
      id->name = ctx.tok_v.next().val;
      break;
    }

    count++;
    if (count > 12) {
      ctx.tok_v.add_error<114>("Explicit path for identifier is too long (> " + std::to_string(12) + ")", hint);
      break;
    }
  }

  return id;
}

std::unique_ptr<ast::Expr_ID_Generic> parser::Parser_Expression::identifier_typed()
{
  ctx.tok_v.match_any({TokTy::TURBO_FISH, TokTy::OPEN_BRACE});

  auto id_type = ctx.Create_Node<ast::Expr_ID_Generic>(ctx.tok_v.peek(-2));

  if (ctx.tok_v.match(TokTy::CLOSE_BRACKETS)) return id_type;

  while (!ctx.tok_v.is_end()) {
    id_type->gen_args.push_back(ctx.p_type->parse_type());

    if (ctx.match_field_separator(TokTy::COMMA, TokTy::CLOSE_BRACKETS)) break;
  }

  return id_type;
}

std::unique_ptr<ast::expression::Member_Access> parser::Parser_Expression::member_access()
{
  ctx.tok_v.match(TokTy::DOT);

  auto access   = ctx.Create_Node<ast::expression::Member_Access>(ctx.tok_v.peek(-2));
  access->right = identifier(true);

  return access;
}

std::unique_ptr<ast::expression::Table_Access> parser::Parser_Expression::table_access()
{
  ctx.tok_v.match(TokTy::OPEN_SQUARE);

  auto table_access      = ctx.Create_Node<ast::expression::Table_Access>(ctx.tok_v.peek(-1));
  table_access->selector = ctx.p_expr->parse_expression();

  ctx.tok_v.expect<115>(TokTy::CLOSE_SQUARE, "Expected end of table access '[' after expression.", "");

  return table_access;
}

std::unique_ptr<ast::expression::Ptr_At> parser::Parser_Expression::ptr_at(std::unique_ptr<ast::AExpression> expr)
{
  ctx.tok_v.match(TokTy::PTR_AT);

  auto ptr_at    = ctx.Create_Node<ast::expression::Ptr_At>(ctx.tok_v.peek());
  ptr_at->target = std::move(expr);
  ptr_at->index  = ctx.p_expr->parse_expression();
  ctx.tok_v.expect<108>(TokTy::CLOSE_PAREN, "Expected end of pointer at ')'.",
                        "define pointer at like: `my_ptr'at(i)`.");

  return ptr_at;
}

std::unique_ptr<ast::expression::Ptr_Offset>
parser::Parser_Expression::ptr_offset(std::unique_ptr<ast::AExpression> expr)
{
  ctx.tok_v.match(TokTy::PTR_OFFSET);

  auto ptr_offset    = ctx.Create_Node<ast::expression::Ptr_Offset>(ctx.tok_v.peek());
  ptr_offset->target = std::move(expr);
  ptr_offset->offset = ctx.p_expr->parse_expression();
  ctx.tok_v.expect<109>(TokTy::CLOSE_PAREN, "Expected end of pointer offset ')'.",
                        "define pointer offset like: `my_ptr'offset(i)`.");

  return ptr_offset;
}

std::unique_ptr<ast::expression::Ptr_Val> parser::Parser_Expression::ptr_val()
{
  auto node = ctx.Create_Node<ast::expression::Ptr_Val>(ctx.tok_v.peek());
  ctx.tok_v.match(TokTy::VAL_OF);
  node->target = ctx.p_expr->parse_expression();

  return node;
}

std::unique_ptr<ast::expression::Addr_Of> parser::Parser_Expression::addr_of()
{
  auto node = ctx.Create_Node<ast::expression::Addr_Of>(ctx.tok_v.peek());
  ctx.tok_v.match(TokTy::ADDR_OF);
  node->target = ctx.p_expr->parse_expression();

  return node;
}

std::unique_ptr<ast::expression::GetBits> parser::Parser_Expression::getbits(std::unique_ptr<ast::AExpression> expr)
{
  static const std::string hint = "define get bit like: `target~[0..8]` get first octect on target.";

  ctx.tok_v.match(TokTy::TILDE);

  ctx.tok_v.expect<98>(TokTy::OPEN_SQUARE, "Expected start slice block '[' after a get bit operator '~'", hint);

  auto get_bit    = ctx.Create_Node<ast::expression::GetBits>(ctx.tok_v.peek(-2));
  get_bit->target = std::move(expr);
  get_bit->range  = ctx.p_expr->parse_expression();

  ctx.tok_v.expect<99>(TokTy::CLOSE_SQUARE, "Expected end slice block ']' after range expression", hint);

  return get_bit;
}

std::unique_ptr<ast::expression::Size_Of> parser::Parser_Expression::size_of()
{
  static const std::string hint = "define value Expression size like: `size'a`";
  auto                     node = ctx.Create_Node<ast::expression::Size_Of>(ctx.tok_v.peek(-1));

  ctx.tok_v.expect<104>(TokTy::OPEN_PAREN, "Expected start arg '('.", hint);
  node->target = ctx.p_expr->parse_expression();
  ctx.tok_v.expect<105>(TokTy::CLOSE_PAREN, "Expected end arg ')'.", hint);

  return node;
}

std::unique_ptr<ast::expression::Move> parser::Parser_Expression::move()
{
  ctx.tok_v.match(TokTy::CAPA_MOVE_OF);
  auto node    = ctx.Create_Node<ast::expression::Move>(ctx.tok_v.peek());
  node->target = ctx.p_expr->parse_expression();
  return node;
}

std::unique_ptr<ast::expression::New_Ptr> parser::Parser_Expression::new_ptr()
{
  static const std::string hint =
      "define a dynamic Expression allocation:"
      "\n  - primitive `var myPtr = new ptr'i32(10)`"
      "\n  - array `var myPtr = new ptr'[i32 -> 3]({ 1, 2, 3 })`"
      "\n  - array on all `var myPtr = new ptr'[i32 -> 3](0)`"
      "\n  - entity `var myPtr = new ptr'Person(Person{ CIdentity{ name: \"Zagreus\", age: 25 } })`";

  ctx.tok_v.match(TokTy::NEW);

  auto node = ctx.Create_Node<ast::expression::New_Ptr>(ctx.tok_v.peek());
  ctx.tok_v.expect_any<100>(kPointerTokens, "Expected pointer specification after 'new' token.", hint);
  node->pointer = TokTy_to_EPtrType(ctx.tok_v.peek(-1).type);

  ctx.tok_v.expect<101>(TokTy::TICK, "Expected tick ' between pointer and type", hint);

  node->type = ctx.p_type->parse_type();

  ctx.tok_v.expect<102>(TokTy::OPEN_PAREN, "Expected start value '(' after type", hint);
  node->expression = ctx.p_expr->parse_expression();
  ctx.tok_v.expect<103>(TokTy::CLOSE_PAREN, "Expected end value ')'", hint);

  return node;
}
