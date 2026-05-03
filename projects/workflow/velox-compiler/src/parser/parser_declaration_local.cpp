#include "parser_declaration_local.hpp"


#include "ast/ast_expression.hpp"
#include "compiler/compiler.hpp"
#include "nexus/ast/ast.hpp"
#include "nexus/ids.hpp"
#include "nexus/lexer/token.hpp"
#include "nexus/forward.hpp"
#include "nexus/script.hpp"
#include "nexus/symbol.hpp"

#include "ast/ast_declaration_local.hpp"

#include "parser_base.hpp"
#include "parser_context.hpp"
#include "parser_declaration_global.hpp"
#include "parser_declaration_local.hpp"
#include "parser_expression.hpp"
#include "parser_literal.hpp"
#include "parser_statement.hpp"
#include "parser_type.hpp"
#include <cassert>

ast::_gnid parser::Parser_Declaration_Local::parse_local(bool silent_error)
{
  auto tok  = p.peek();
  auto kind = tok.kind;
  switch (kind) {
  case token::ETokenKind::VAR:
  case token::ETokenKind::LET:
  case token::ETokenKind::CONST:    return variable();
  case token::ETokenKind::LAMBDA:   return lambda();
  case token::ETokenKind::CAPA_REF:
  case token::ETokenKind::CAPA_MUT: return capability();
  default:                          break;
  }

  if (!silent_error) {
    p.add_error(40, "Illegal instruction '" + std::string(p.tok_to_str(p.peek().id)) + "' in local.",
                "you can define in local: variable, lambda, call, operation, assignation, statement");
  }

  return BAD_NODE_ID;
}

ast::_gnid parser::Parser_Declaration_Local::pattern_mapping(ast::ECapability p_parent_capa)
{
  parser_add_node(node, Local_Pattern_Element, p.peek().id);

  if (p.match(token::ETokenKind::UNDERSCORE)) {
    node->kind = ast::Local_Pattern_Element::Kind::Ignore;
    return node->node_id;
  }


  // literal case
  if (auto lit = p.p_lit->try_literal(true)) {
    node->kind    = ast::Local_Pattern_Element::Kind::Literal;
    node->literal = lit;
    return node->node_id;
  }

  node->kind = ast::Local_Pattern_Element::Kind::Binding;

  parser_add_node(bind, Local_Binding, p.peek().id);
  bind->capability = p_parent_capa;

  if (p.match_any(token::k_expression_passmode)) {
    bind->capability = ast::ETokenKind_to_ECapability(p.peek(-1).kind);
  }

  bind->name = p.parse_name();

  node->bind = bind->node_id;

  return bind->node_id;
}

ast::_gnid parser::Parser_Declaration_Local::parse_evaluator(ast::_gnid comparison_expr)
{
  if (p.match_any({token::ETokenKind::CAPA_MUT, token::ETokenKind::CAPA_REF})) {
    return parse_pattern(comparison_expr);
  }

  return p.p_expr->parse_expression();
}

ast::_gnid parser::Parser_Declaration_Local::parse_pattern(ast::_gnid comparison_expr)
{
  ast::ECapability capa = ast::ETokenKind_to_ECapability(p.peek(-1).kind);

  if (p.match(token::ETokenKind::OPEN_PAREN)) {
    return tuple_pattern(capa, comparison_expr);
  }

  auto id = p.p_base->identifier(false, true);

  // enum pattern : if ref Some(a) = val {...}
  if (p.match(token::ETokenKind::OPEN_PAREN)) {
    return enum_pattern(capa, std::move(id), comparison_expr);
  }
  // entity pattern / component pattern
  else if (p.check(token::ETokenKind::STATIC_ACCESS) && p.check_at(1, token::ETokenKind::OPEN_BRACKETS)) {
    return entity_pattern(capa, std::move(id), comparison_expr);
  } else if (p.match(token::ETokenKind::OPEN_BRACE)) {
    return component_pattern(capa, std::move(id), comparison_expr);
  }

  p.add_error(41, "Expected pattern.", "define auto inferred variable like `let myName = expression;`");
  return BAD_NODE_ID;
}

ast::_gnid parser::Parser_Declaration_Local::variable()
{
  parser_add_node(var, Local_Variable, p.peek().id);
  var->kind     = ast::ETokenKind_to_EVariableKind(p.next().kind);
  var->name     = p.parse_name();
  var->isStatic = p.metablock_contains(p.tok_to_pos(var->node_token_id), "static");

  p.add_symbol(var->node_id.get_node_id());

  bool is_inferred_ty = false;

  // explicit type case
  if (p.match(token::ETokenKind::COLON)) var->type = p.p_type->parse_type();
  // auto deduce type case
  else
    is_inferred_ty = true;

  // check affectation
  var->assignment = ast::ETokenKind_to_ETransfertType(p.peek().kind);

  if (var->assignment == ast::ETransfertType::NONE) {
    if (is_inferred_ty)
      p.add_error(42, "Expected assignation '=' in auto inferred variable type.",
                  "define auto inferred variable like `let myName = expression;`");

    return var->node_id;
  }

  p.next();

  auto expr = p.p_expr->parse_expression();

  // if (isAutoTy) var->type = resolve_type(expr.get());

  var->expression = std::move(expr);

  return var->node_id;
}

ast::_gnid parser::Parser_Declaration_Local::tuple_destructuring()
{
  constexpr std::string_view hint =
      R"(define unpack like:
  - `var (a, b, c) = myFunction()`
  - with ignored values `var (a, _, c) = myFunction()`)";

  const auto varKind = p.next().kind;
  parser_add_node(unpack, Local_Tuple_Destructuring, p.peek(-1).id);

  while (!p.is_end()) {
    // ignore variable
    if (!p.match(token::ETokenKind::UNDERSCORE)) {
      parser_add_node(loc, Local_Binding, p.peek().id);
      loc->name = p.parse_name("", hint);
      unpack->bindings.push_back(loc->node_id);

      p.add_symbol(loc->node_id.get_node_id());
    } else {
      unpack->bindings.push_back(ast::_gnid(WILCARD_ID));
    }

    if (p.match_field_separator(token::ETokenKind::COMMA, token::ETokenKind::ASSIGN)) break;
  }

  unpack->expression = p.p_expr->parse_expression();

  return unpack->node_id;
}

ast::_gnid parser::Parser_Declaration_Local::lambda_capture()
{
  constexpr std::string_view hint =
      R"(define capture like:
  - modify all variables `[mut]` or `[mut, copy a, ...]`
  - copy all variables `[copy] or `[copy, mut a, ...]`      
  - get instance `[..., self, ...]`)";

  parser_add_node(capture, Local_Lambda_Capture, p.peek().id);

  // all by ref
  if (p.check_val("mut") && p.check_at(1, token::ETokenKind::CLOSE_SQUARE)) {
    capture->is_all_ref = true;
    p.next();
    return capture->node_id;
  }
  // all by copy
  else if (p.check_val("copy") && p.check_at(1, token::ETokenKind::CLOSE_SQUARE)) {
    capture->is_all_ref = false;
    p.next();
    return capture->node_id;
  }

  if (p.match(token::ETokenKind::SELF)) {
    capture->is_capture_self = true;
  }

  while (!p.is_end()) {
    auto tok_capa = p.expect_any(44, token::k_capability, "Expected capture capability kind.", hint);
    auto capa     = ast::ETokenKind_to_ECapability(tok_capa.kind);

    ast::_gnid id;

    switch (capa) {
    case ast::ECapability::NONE:
    case ast::ECapability::Ref:  {
      parser_add_node(R, Expression_Ref_Of, p.peek(-1).id);
      id = R->node_id;
    }
    case ast::ECapability::Mut: {
      parser_add_node(M, Expression_Mut_Of, p.peek(-1).id);
      id = M->node_id;
    }
    case ast::ECapability::Copy: {
      parser_add_node(C, Expression_Copy_Of, p.peek(-1).id);
      id = C->node_id;
    }
    case ast::ECapability::Move: {
      parser_add_node(M, Expression_Move_Of, p.peek(-1).id);
      id = M->node_id;
    }
    }

    capture->capture_members.push_back(id);
  }

  return capture->node_id;
}

ast::_gnid parser::Parser_Declaration_Local::lambda()
{
  parser_add_node(lam, Local_Lambda, p.peek().id);

  if (p.check(token::ETokenKind::IDENTIFIER)) {
    lam->name = p.parse_name();

    p.add_symbol(lam->node_id.get_node_id());
    p.enter_scope(*lam, "lambda \"" + std::string(lam->name) + "\"");
  } else {
    p.enter_scope(*lam, "lambda");
  }

  lam->is_const = p.metablock_contains(p.tok_to_pos(lam->node_token_id), "const");
  lam->is_pure  = p.metablock_contains(p.tok_to_pos(lam->node_token_id), "pure");

  // check capture
  if (p.match(token::ETokenKind::OPEN_SQUARE)) lam->capture = lambda_capture();

  auto proto = p.p_type->parse_and_mount_local_callable(lam->prototype, lam->is_explicit_ret_type);

  lam->codeblock = p.p_loc->parse_codeblock();

  p.exit_scope();

  return lam->node_id;
}

ast::_gnid parser::Parser_Declaration_Local::parse_codeblock()
{
  p.expect_any(46, {token::ETokenKind::OPEN_BRACE, token::ETokenKind::INJECT},
               "Expected start code block '{' or linecode '=>'.", "");

  bool inline_code = p.peek(-1).kind == token::ETokenKind::INJECT;
  parser_add_node(cb, Local_CodeBlock, p.peek().id);

  while (!p.is_end()) {
    if (p.match_field_separator(token::ETokenKind::S_END_OF_FILE, token::ETokenKind::CLOSE_BRACE)) break;

    if (auto instruction = p.p_base->parse_instruction(); instruction) cb->elements.push_back(instruction);

    // one instruction
    if (inline_code) break;
  }

  return cb->node_id;
}

ast::_gnid parser::Parser_Declaration_Local::capability()
{
  constexpr std::string_view hint =
      R"(define capability like:
  - reference (read only) `ref a = lvalue`
  - mutable (read/write) `mut a = lvalue`)";

  auto capa_tok_kind = p.expect_any(47, token::k_capability, "Expected capability kind.", hint);
  parser_add_node(capa, Local_Capability, p.peek(-1).id);
  capa->kind = ast::ETokenKind_to_ECapability(capa_tok_kind.kind);

  capa->name = p.parse_name("", hint);

  p.expect(48, token::ETokenKind::ASSIGN, "Expected classic assignation '=' after capability declaration.", hint);

  capa->expression = p.p_expr->parse_expression();

  return capa->node_id;
}

ast::_gnid parser::Parser_Declaration_Local::component_pattern(ast::ECapability p_capa, ast::_gnid p_comp_id,
                                                               ast::_gnid p_comparison_ref)
{
  constexpr std::string_view hint =
      R"(define component mapping like:
  - `[ref/mut] CComponent{.field1: [ref/mut/copy]'a, .field2: 10} = val`)";

  auto& comp_node = p.scr_info.nodes->get(p_comp_id.get_node_id());

  assert(ast::ENodeKind_is_ID(comp_node.kind()));
  // Illegal identifier, impossible to use a type

  parser_add_node(comp_pat, Local_Pattern_Comp, p.peek().id);

  comp_pat->capability = p_capa;
  comp_pat->name       = std::move(p_comp_id);

  while (!p.is_end()) {
    if (p.match_field_separator(token::ETokenKind::COMMA, token::ETokenKind::CLOSE_BRACE)) break;

    p.expect(241, token::ETokenKind::COMMA, "Component field local access '.' expected in component mapping.", hint);

    ast::Local_Pattern_Comp::Field field;
    field.name = p.parse_name("", hint);

    p.expect(49, token::ETokenKind::COLON, "Expected field mapping association ':'.", hint);

    auto mapping = pattern_mapping(p_capa);

    field.mapping = mapping;
    comp_pat->mapping.push_back(field);
  }

  if (!p_comparison_ref) {
    p.expect(50, token::ETokenKind::ASSIGN, "Expected assignation on pattern.", hint);
    comp_pat->expression = p.p_expr->parse_expression();
  } else {
    comp_pat->expression = p_comparison_ref;
  }

  // to add ?
  // if (p.match(token::ETokenKind::IF)) comp_pat->additive_evaluator = p.p_expr->parse_expression();

  return comp_pat->node_id;
}

ast::_gnid parser::Parser_Declaration_Local::entity_pattern(ast::ECapability p_capa, ast::_gnid p_entity_id,
                                                            ast::_gnid p_comparison_ref)
{
  constexpr std::string_view hint =
      R"(define entity mapping like:
  - component general mapping `[ref/mut] MyEntity{CComponent{.field1: [ref/mut/copy] a, .field2: 10}}`
  - component field mapping `[ref/mut] MyEntity{CComponent.field1: [ref/mut/copy] a, CComponent.field2: 10}`)";

  p.match(token::ETokenKind::STATIC_ACCESS);
  p.match(token::ETokenKind::OPEN_BRACKETS);

  auto& entity_node = p.scr_info.nodes->get(p_entity_id.get_node_id());

  assert(ast::ENodeKind_is_ID(entity_node.kind()));
  // Illegal identifier, impossible to use a type

  parser_add_node(entity_pat, Local_Pattern_Entity, p.peek().id);
  entity_pat->name       = std::move(p_entity_id);
  entity_pat->capability = p_capa;

  while (!p.is_end()) {
    auto comp_identifier = p.p_base->identifier();

    if (p.match(token::ETokenKind::OPEN_BRACE)) {
      auto comp = component_pattern(p_capa, comp_identifier, p_comparison_ref);
      entity_pat->pattern_components.push_back(comp);
    } else {
      parser_add_node(pattern_comp, Local_Pattern_Comp, p.peek(-1).id);
      pattern_comp->name       = comp_identifier;
      pattern_comp->capability = p_capa;

      ast::Local_Pattern_Comp::Field field;

      p.expect(52, token::ETokenKind::DOT,
               "Expected component field mapping '.' or start component general mapping '{'.", hint);
      field.name = p.parse_name("", hint);

      p.expect(53, token::ETokenKind::COLON, "Expected field mapping association ':'.", hint);
      field.mapping = pattern_mapping(p_capa);

      pattern_comp->mapping = {field};
      entity_pat->pattern_components.push_back(pattern_comp->node_id);
    }

    if (p.match_field_separator(token::ETokenKind::COMMA, token::ETokenKind::CLOSE_BRACE)) break;
  }

  if (!p_comparison_ref) {
    p.expect(54, token::ETokenKind::ASSIGN, "Expected assignation on pattern.", hint);
    entity_pat->expression = p.p_expr->parse_expression();
  } else {
    entity_pat->expression = p_comparison_ref;
  }

  // if (p.match(token::ETokenKind::IF)) entity_pat->additive_evaluator = p.p_expr->parse_expression();

  return entity_pat->node_id;
}

ast::_gnid parser::Parser_Declaration_Local::tuple_pattern(ast::ECapability p_capa, ast::_gnid p_comparison_ref)
{
  constexpr std::string_view hint =
      R"(define enum pattern on condition like:                                                               
  - binding pattern `[if/elif/while] [ref/mut] ([copy/mut/ref] a, _, 10) = tuple {...}`
  Rules: - ref -> bind primitives by `copy`, bind complex types by `ref`
         - mut -> bind primitives and complex types by `mut`
         - override binding rule by explicit binding mode `([copy/mut/ref] a)`)";

  parser_add_node(pat, Local_Pattern_Tuple, p.peek().id);
  pat->capability = p_capa;

  if (p.check(token::ETokenKind::CLOSE_PAREN)) p.add_error(55, "Unexpected void tuple.", hint);

  while (!p.is_end()) {
    pat->pattern_elements.push_back(pattern_mapping(p_capa));
    if (p.match_field_separator(token::ETokenKind::COMMA, token::ETokenKind::CLOSE_PAREN)) break;
  }

  if (!p_comparison_ref) {
    p.expect(56, token::ETokenKind::ASSIGN, "Expected assignation on pattern.", hint);
    pat->expression = p.p_expr->parse_expression();
  } else {
    pat->expression = p_comparison_ref;
  }

  // if (p.match(token::ETokenKind::IF)) pat->additive_evaluator = p.p_expr->parse_expression();

  return pat->node_id;
}

ast::_gnid parser::Parser_Declaration_Local::enum_pattern(ast::ECapability p_capa, ast::_gnid p_enum_id,
                                                          ast::_gnid p_comparison_ref)
{
  constexpr std::string_view hint =
      R"(define enum pattern on condition like:
  - binding pattern `[if/elif/while] [ref/mut] Some(a) = value {...}`
  - binding pattern with more condition `if [ref/mut] Some(a) = value if a > 10 {...}`
  - binding pattern conditionnal `if [ref/mut] Some(10) = value {...}`                            
  - check only indexation `[if/elif/while] value == Some`)";

  auto& enum_node = p.scr_info.nodes->get(p_enum_id.get_node_id());

  assert(ast::ENodeKind_is_ID(enum_node.kind()));
  // Illegal identifier, impossible to use a type

  parser_add_node(pat, Local_Pattern_Enum, p.peek().id);
  pat->name = std::move(p_enum_id);

  p.expect(57, token::ETokenKind::OPEN_PAREN, "Expected start binding on enum types '('.", hint);

  if (p.check(token::ETokenKind::CLOSE_PAREN))
    p.add_error(58,
                "Unexpected end of binding on enum types ')'. A enum pattern on condition must have at "
                "least one binding. Else, use a check indexation.",
                hint);

  ast::EExprPassMode pass_mode = ast::ETokenKind_to_EExprPassMode(p.peek().kind);
  if (pass_mode != ast::EExprPassMode::NONE) p.next();

  while (!p.is_end()) {
    pat->pattern_elements.push_back(pattern_mapping(p_capa));
    if (p.match_field_separator(token::ETokenKind::COMMA, token::ETokenKind::CLOSE_PAREN)) break;
  }

  if (!p_comparison_ref) {
    p.expect(59, token::ETokenKind::ASSIGN, "Expected assignation on pattern.", hint);
    pat->expression = p.p_expr->parse_expression();
  } else {
    pat->expression = p_comparison_ref;
  }

  // if (p.match(token::ETokenKind::IF)) pat->additive_evaluator = p.p_expr->parse_expression();

  return pat->node_id;
}
