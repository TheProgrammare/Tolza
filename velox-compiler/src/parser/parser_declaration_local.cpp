#include "parser_declaration_local.hpp"

#include <memory>
#include <vector>

#include "ast/ast_base.hpp"
#include "ast/ast_data.hpp"
#include "ast/ast_declaration_local.hpp"
#include "parser_base.hpp"
#include "parser_context.hpp"
#include "parser_declaration.hpp"
#include "parser_declaration_local.hpp"
#include "parser_expression.hpp"
#include "parser_literal.hpp"
#include "parser_statement.hpp"
#include "parser_type.hpp"
#include "visitor/symbol_manager.hpp"

std::shared_ptr<ast::ALocal> parser::Parser_Declaration_Local::parse_local(bool silent_error)
{
  auto type = ctx.tok_v.peek().type;
  switch (type) {
  case TokTy::VAR:
  case TokTy::LET:
  case TokTy::CONST: {
    if (ctx.tok_v.peek(1).type == TokTy::OPEN_PAREN) return variable_unpack();
    return variable();
  }
  case TokTy::LAMBDA:   return lambda();
  case TokTy::CAPA_REF:
  case TokTy::CAPA_MUT: return capability();
  default:              break;
  }

  if (!silent_error) {
    ctx.tok_v.add_error<40>("Illegal instruction '" + ctx.tok_v.peek().val + "' in local.",
                            "you can define in local: variable, lambda, call, operation, assignation, statement");
  }

  return nullptr;
}

ast::declaration::local::Pattern_Element parser::Parser_Declaration_Local::pattern_mapping(ECapability capa)
{
  if (auto lit = ctx.p_lit->try_literal(true)) {
    return ast::declaration::local::Pattern_Element(std::move(lit));
  } else {
    auto bind        = ctx.Create_Decl<ast::declaration::local::Variable_Binding>(ctx.tok_v.peek());
    bind->capability = capa;

    if (ctx.tok_v.match_any(kCapabilityKind)) {
      bind->capability = TokTy_to_ECapability(ctx.tok_v.peek(-1).type);
    }

    bind->name = ctx.parse_name();

    return ast::declaration::local::Pattern_Element(bind);
  }
}

ast::Evaluator parser::Parser_Declaration_Local::parse_evaluator(std::shared_ptr<ast::AExpression> comparison_expr)
{
  if (ctx.tok_v.match_any({TokTy::CAPA_MUT, TokTy::CAPA_REF})) {
    return ast::Evaluator(parse_pattern(comparison_expr));
  }

  return ast::Evaluator(ctx.p_expr->parse_expression());
}

std::unique_ptr<ast::declaration::local::Pattern>
parser::Parser_Declaration_Local::parse_pattern(std::shared_ptr<ast::AExpression> comparison_expr)
{
  ECapability capa = TokTy_to_ECapability(ctx.tok_v.peek(-1).type);

  if (ctx.tok_v.match(TokTy::OPEN_PAREN)) {
    return tuple_pattern(capa, comparison_expr);
  }

  auto id = ctx.p_expr->identifier(false, true);

  // enum pattern : if ref Some(a) = val {...}
  if (ctx.tok_v.match(TokTy::OPEN_PAREN)) {
    return enum_pattern(capa, std::move(id), comparison_expr);
  }
  // entity pattern / component pattern
  else if (ctx.tok_v.match(TokTy::ENTITY_START_LIT)) {
    return entity_pattern(capa, std::move(id), comparison_expr);
  } else if (ctx.tok_v.match(TokTy::OPEN_BRACE)) {
    return component_pattern(capa, std::move(id), comparison_expr);
  }

  ctx.tok_v.add_error<41>("Expected pattern.", "define auto inferred variable like `let myName = expression;`");
  return nullptr;
}

std::shared_ptr<ast::declaration::local::Variable> parser::Parser_Declaration_Local::variable()
{
  auto var      = ctx.Create_Decl<ast::declaration::local::Variable>(ctx.tok_v.peek());
  var->kind     = TokTy_to_EVariableKind(ctx.tok_v.next().type);
  var->name     = ctx.parse_name();
  var->isStatic = ctx.metablock_contains(*var, "static");

  ctx.m_sym->add_decl(var);

  bool isAutoTy = false;

  // explicit type case
  if (ctx.tok_v.match(TokTy::COLON)) var->type = ctx.p_type->parse_type();
  // auto deduce type case
  else
    isAutoTy = true;

  // check affectation
  var->assignment = TokTy_to_EAssignmentType(ctx.tok_v.peek().type);

  if (var->assignment == EAssignmentType::NONE) {
    if (isAutoTy)
      ctx.tok_v.add_error<42>("Expected assignation '=' in auto inferred variable type.",
                              "define auto inferred variable like `let myName = expression;`");

    return var;
  }

  ctx.tok_v.next();

  auto expr = ctx.p_expr->parse_expression();

  // if (isAutoTy) var->type = resolve_type(expr.get());

  var->expression = std::move(expr);

  return var;
}

std::shared_ptr<ast::declaration::local::Variable_Unpack> parser::Parser_Declaration_Local::variable_unpack()
{
  static const std::string hint =
      "define unpack like:"
      "\n  - `var (a, b, c) = myFunction()`"
      "\n  - with ignored values `var (a, _, c) = myFunction()`";

  const auto varKind = ctx.tok_v.next().type;
  auto       unpack  = ctx.Create_Decl<ast::declaration::local::Variable_Unpack>(ctx.tok_v.peek());
  unpack->kind       = TokTy_to_EVariableKind(ctx.tok_v.next().type);
  unpack->isStatic   = ctx.metablock_contains(*unpack, "static");

  while (!ctx.tok_v.is_end()) {
    // ignore
    // variable
    if (!ctx.tok_v.match(TokTy::UNDERSCORE)) {
      auto loc  = ctx.Create_Decl<ast::declaration::local::Variable_Binding>(ctx.tok_v.peek(-1));
      loc->name = ctx.parse_name("", hint);
      unpack->elements.push_back(loc);
      ctx.m_sym->add_decl(loc);
    } else {
      unpack->elements.push_back(nullptr);
    }

    if (ctx.match_field_separator(TokTy::COMMA, TokTy::ASSIGN)) break;
  }

  unpack->right = ctx.p_expr->parse_expression();

  return unpack;
}

std::unique_ptr<ast::declaration::local::Lambda_Capture> parser::Parser_Declaration_Local::lambda_capture()
{
  static const std::string hint =
      "define capture like:"
      "\n  - modify all variables `[mut]` or `[mut, copy a, ...]`"
      "\n  - copy all variables `[copy] or `[copy, mut a, ...]`"
      "\n  - get instance `[..., self, ...]`";

  auto capture = ctx.Create_Node<ast::declaration::local::Lambda_Capture>(ctx.tok_v.peek());

  // all by ref
  if (ctx.tok_v.check_val("mut") && ctx.tok_v.peek(1).type == TokTy::CLOSE_SQUARE) {
    capture->isAllRef = true;
    ctx.tok_v.next();
    return capture;
  }
  // all by copy
  else if (ctx.tok_v.check_val("copy") && ctx.tok_v.peek(1).type == TokTy::CLOSE_SQUARE) {
    capture->isAllRef = false;
    ctx.tok_v.next();
    return capture;
  }

  if (ctx.tok_v.match(TokTy::SELF)) {
    capture->isCaptureSelf = true;
  }

  while (!ctx.tok_v.is_end()) {
    auto elem = ctx.Create_Node<ast::declaration::local::Capture_Member>(ctx.tok_v.peek());

    auto tok_capa    = ctx.tok_v.expect_any<44>(kCapabilityKind, "Expected capture capability kind.", hint);
    elem->capability = TokTy_to_ECapability(tok_capa.type);

    elem->name = ctx.p_expr->parse_expression_term();
    capture->elements.push_back(std::move(elem));
  }

  return capture;
}

std::shared_ptr<ast::declaration::local::Lambda> parser::Parser_Declaration_Local::lambda()
{
  auto lam = ctx.Create_Decl<ast::declaration::local::Lambda>(ctx.tok_v.peek());

  if (ctx.tok_v.check(TokTy::IDENTIFIER)) {
    lam->name = ctx.parse_name();
    ctx.m_sym->add_decl(lam);
    ctx.m_sym->enter_scope(lam->name, EScopeType::Lambda);
  } else {
    ctx.m_sym->enter_scope("lam", EScopeType::Lambda);
  }

  lam->isConst           = ctx.metablock_contains(*lam, "const");
  lam->isMutable         = ctx.metablock_contains(*lam, "mutable");
  lam->isPure            = ctx.metablock_contains(*lam, "pure");
  lam->isNoexcept        = ctx.metablock_contains(*lam, "noexcept");
  lam->isConstexpr       = ctx.metablock_contains(*lam, "constexpr");
  lam->isLambdaConstexpr = ctx.metablock_contains(*lam, "lam_constexpr");

  // check capture
  if (ctx.tok_v.match(TokTy::OPEN_SQUARE)) lam->capture = lambda_capture();

  lam->prototype = ctx.p_type->explicit_function_proto(true);

  for (auto& param : lam->prototype->parameters) param->parent_function = lam;

  lam->codeblock = ctx.p_loc->code_block_instruction();

  ctx.m_sym->exit_scope();

  return lam;
}

std::unique_ptr<ast::declaration::local::CodeBlock> parser::Parser_Declaration_Local::code_block(bool is_silent_error,
                                                                                                 proto_cb in_function)
{
  ctx.tok_v.expect_any<45>({TokTy::OPEN_BRACE, TokTy::INJECT}, "Expected start code block '{' or linecode '=>'.", "");

  bool inline_code = ctx.tok_v.peek(-1).type == TokTy::INJECT;
  auto cb          = ctx.Create_Node<ast::declaration::local::CodeBlock>(ctx.tok_v.peek(-1));

  while (!ctx.tok_v.is_end()) {
    cb->elements.push_back(in_function());

    // one instruction
    if (inline_code) break;
    if (ctx.match_field_separator(TokTy::S_END_OF_FILE, TokTy::CLOSE_BRACE)) break;
  }

  return cb;
}

std::unique_ptr<ast::declaration::local::CodeBlock> parser::Parser_Declaration_Local::code_block_instruction()
{
  ctx.tok_v.expect_any<46>({TokTy::OPEN_BRACE, TokTy::INJECT}, "Expected start code block '{' or linecode '=>'.", "");

  bool inline_code = ctx.tok_v.peek(-1).type == TokTy::INJECT;
  auto cb          = ctx.Create_Node<ast::declaration::local::CodeBlock>(ctx.tok_v.peek(-1));

  while (!ctx.tok_v.is_end()) {
    if (auto instruction = ctx.p_base->parse_instruction()) cb->elements.push_back(std::move(instruction.value()));

    // one instruction
    if (inline_code) break;
    if (ctx.match_field_separator(TokTy::S_END_OF_FILE, TokTy::CLOSE_BRACE)) break;
  }

  return cb;
}

std::shared_ptr<ast::declaration::local::Capability> parser::Parser_Declaration_Local::capability()
{
  static const std::string hint =
      "define capability like:"
      "\n  - reference (read only) `ref a = lvalue`"
      "\n  - mutable (read/write) `mut a = lvalue`";

  Token capa_tok_kind = ctx.tok_v.expect_any<47>(kCapabilityKind, "Expected capability kind.", hint);
  auto  capa          = ctx.Create_Decl<ast::declaration::local::Capability>(ctx.tok_v.peek(-1));
  capa->kind          = TokTy_to_ECapability(capa_tok_kind.type);

  capa->name = ctx.parse_name("", hint);

  ctx.tok_v.expect<48>(TokTy::ASSIGN, "Expected classic assignation '=' after capability declaration.", hint);

  capa->right = ctx.p_expr->parse_expression();

  return capa;
}

std::unique_ptr<ast::declaration::local::Pattern_Component>
parser::Parser_Declaration_Local::component_pattern(ECapability capa, std::unique_ptr<ast::AIdentifier> comp_id,
                                                    std::shared_ptr<ast::AExpression> comparison_ref)
{
  static const std::string hint =
      "define component mapping like:"
      "\n  - `[ref/mut] CComponent{field1: [ref/mut/copy/clone] a, field2: 10} = val`";

  if (dynamic_cast<ast::AType*>(comp_id.get()) != nullptr)
    throw std::runtime_error("Illegal identifier, impossible to use a type ");

  auto comp_pat = ctx.Create_Node<ast::declaration::local::Pattern_Component>(ctx.tok_v.peek());

  comp_pat->name       = std::move(comp_id);
  comp_pat->capability = capa;

  while (!ctx.tok_v.is_end()) {
    std::string field_name = ctx.parse_name("", hint);

    ctx.tok_v.expect<49>(TokTy::COLON, "Expected field mapping association ':'.", hint);

    comp_pat->mapping.push_back({field_name, pattern_mapping(capa)});

    if (ctx.match_field_separator(TokTy::COMMA, TokTy::CLOSE_BRACE)) break;
  }

  if (!comparison_ref) {
    ctx.tok_v.expect<50>(TokTy::ASSIGN, "Expected assignation on pattern.", hint);
    comp_pat->right = std::shared_ptr<ast::AExpression>(ctx.p_expr->parse_expression().release());
  } else {
    comp_pat->right = comparison_ref;
  }

  if (ctx.tok_v.match(TokTy::IF)) comp_pat->additive_evaluator = ctx.p_expr->parse_expression();

  return comp_pat;
}

std::unique_ptr<ast::declaration::local::Pattern_Entity>
parser::Parser_Declaration_Local::entity_pattern(ECapability capa, std::unique_ptr<ast::AIdentifier> entity_id,
                                                 std::shared_ptr<ast::AExpression> comparison_ref)
{
  static const std::string hint =
      "define entity mapping like:"
      "\n  - component general mapping `[ref/mut] MyEntity{CComponent{field1: [ref/mut/copy/clone] a, field2: 10}}`"
      "\n  - component field mapping `[ref/mut] MyEntity{CComponent.field1: [ref/mut/copy/clone] a, "
      "CComponent.field2: 10}`";

  if (dynamic_cast<ast::AType*>(entity_id.get()) != nullptr)
    throw std::runtime_error("Illegal identifier, impossible to use a type ");

  auto entity_pat        = ctx.Create_Node<ast::declaration::local::Pattern_Entity>(ctx.tok_v.peek());
  entity_pat->name       = std::move(entity_id);
  entity_pat->capability = capa;

  while (!ctx.tok_v.is_end()) {
    auto pattern_comp = ctx.Create_Node<ast::declaration::local::Pattern_Component>(ctx.tok_v.peek(-2));
    auto comp_id      = ctx.p_expr->identifier();

    pattern_comp->name = std::move(comp_id);

    if (ctx.tok_v.match(TokTy::OPEN_BRACE)) {
      while (!ctx.tok_v.is_end()) {
        const std::string field_name = ctx.parse_name("", hint);

        ctx.tok_v.expect<51>(TokTy::COLON, "Expected field mapping association ':'.", hint);

        pattern_comp->mapping.push_back({field_name, pattern_mapping(capa)});

        if (ctx.match_field_separator(TokTy::COMMA, TokTy::CLOSE_BRACE)) break;
      }
    } else {
      ctx.tok_v.expect<52>(TokTy::DOT, "Expected component field mapping '.' or start component general mapping '{'.",
                           hint);

      const std::string field_name = ctx.parse_name("", hint);

      ctx.tok_v.expect<53>(TokTy::COLON, "Expected field mapping association ':'.", hint);

      pattern_comp->mapping.push_back({field_name, pattern_mapping(capa)});
    }

    entity_pat->mapping.push_back(std::move(pattern_comp));

    if (ctx.match_field_separator(TokTy::COMMA, TokTy::CLOSE_BRACE)) break;
  }

  if (!comparison_ref) {
    ctx.tok_v.expect<54>(TokTy::ASSIGN, "Expected assignation on pattern.", hint);
    entity_pat->right = std::shared_ptr<ast::AExpression>(ctx.p_expr->parse_expression().release());
  } else {
    entity_pat->right = comparison_ref;
  }


  if (ctx.tok_v.match(TokTy::IF)) entity_pat->additive_evaluator = ctx.p_expr->parse_expression();

  return entity_pat;
}

std::unique_ptr<ast::declaration::local::Pattern_Tuple>
parser::Parser_Declaration_Local::tuple_pattern(ECapability capa, std::shared_ptr<ast::AExpression> comparison_ref)
{
  static const std::string hint =
      "define enum pattern on condition like:"
      "\n  - binding pattern `[if/elif/while] [ref/mut] ([copy/clone/mut/ref] a, _, 10) = tuple {...}`"
      "\n  Binding rules: - ref -> bind primitives by `copy`, bind complex types by `ref`"
      "\n                 - mut -> bind primitives and complex types by `mut`"
      "\n                 - override binding rule by explicit binding mode `([copy/clone/mut/ref] a)`";

  auto pat        = ctx.Create_Node<ast::declaration::local::Pattern_Tuple>(ctx.tok_v.peek());
  pat->capability = capa;

  if (ctx.tok_v.check(TokTy::CLOSE_PAREN)) ctx.tok_v.add_error<55>("Unexpected void tuple.", hint);

  while (!ctx.tok_v.is_end()) {
    pat->mapping.push_back(pattern_mapping(capa));
    if (ctx.match_field_separator(TokTy::COMMA, TokTy::CLOSE_PAREN)) break;
  }

  if (!comparison_ref) {
    ctx.tok_v.expect<56>(TokTy::ASSIGN, "Expected assignation on pattern.", hint);
    pat->right = std::shared_ptr<ast::AExpression>(ctx.p_expr->parse_expression().release());
  } else {
    pat->right = comparison_ref;
  }

  if (ctx.tok_v.match(TokTy::IF)) pat->additive_evaluator = ctx.p_expr->parse_expression();

  return pat;
}

std::unique_ptr<ast::declaration::local::Pattern_Enum>
parser::Parser_Declaration_Local::enum_pattern(ECapability capa, std::unique_ptr<ast::AIdentifier> enum_id,
                                               std::shared_ptr<ast::AExpression> comparison_ref)
{
  static const std::string hint =
      "define enum pattern on condition like:"
      "\n  - binding pattern `[if/elif/while] [ref/mut] Some(a) = value {...}`"
      "\n  - binding pattern with more condition `if [ref/mut] Some(a) = value if a > 10 {...}`"
      "\n  - binding pattern conditionnal `if [ref/mut] Some(10) = value {...}`"
      "\n  - check only indexation `[if/elif/while] value == Some`";


  if (dynamic_cast<ast::AType*>(enum_id.get()) != nullptr)
    throw std::runtime_error("Illegal identifier, impossible to use a type ");

  auto pat  = ctx.Create_Node<ast::declaration::local::Pattern_Enum>(ctx.tok_v.peek());
  pat->name = std::move(enum_id);

  ctx.tok_v.expect<57>(TokTy::OPEN_PAREN, "Expected start binding on enum types '('.", hint);

  if (ctx.tok_v.check(TokTy::CLOSE_PAREN))
    ctx.tok_v.add_error<58>(
        "Unexpected end of binding on enum types ')'. A enum pattern on condition must have at "
        "least one binding. Else, use a check indexation.",
        hint);

  EExprPassMode pass_mode = TokTy_to_EExprPassMode(ctx.tok_v.peek().type);
  if (pass_mode != EExprPassMode::NONE) ctx.tok_v.next();

  while (!ctx.tok_v.is_end()) {
    pat->mapping.push_back(pattern_mapping(capa));
    if (ctx.match_field_separator(TokTy::COMMA, TokTy::CLOSE_PAREN)) break;
  }

  if (!comparison_ref) {
    ctx.tok_v.expect<59>(TokTy::ASSIGN, "Expected assignation on pattern.", hint);
    pat->right = std::shared_ptr<ast::AExpression>(ctx.p_expr->parse_expression().release());
  } else {
    pat->right = comparison_ref;
  }

  if (ctx.tok_v.match(TokTy::IF)) pat->additive_evaluator = ctx.p_expr->parse_expression();

  return pat;
}
