#include "parser_declaration_local.hpp"

#include <cassert>

#include "ast/ast_base.hpp"
#include "ast/ast_expression.hpp"
#include "compiler/compilation_unit.hpp"
#include "nexus/ast/data.hpp"
#include "nexus/ast/definition.hpp"
#include "nexus/ast/forward.hpp"
#include "nexus/ids.hpp"
#include "nexus/lexer/token.hpp"
#include "nexus/forward.hpp"

#include "ast/ast_declaration_local.hpp"

#include "parser_base.hpp"
#include "parser_context.hpp"
#include "parser_expression.hpp"
#include "parser_literal.hpp"
#include "parser_type.hpp"

ast::ID parser::Parser_Declaration_Local::parse_local(bool silent_error)
{
  auto& tok  = p.peek();
  auto  kind = tok.kind;
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
    p.add_error(40, std::format("Illegal instruction '{}' in local.", p.tok_to_str(p.peek().tokid)),
                "you can define in local: variable, lambda, call, operation, assignation, statement");
    THROW_BAD_NODE;
  }

  return BAD_NODE_ID;
}

ast::ID parser::Parser_Declaration_Local::pattern_mapping(ast::ECapability p_parent_capa)
{
  auto& node = p.add_get_node<ast::Local_Pattern_Element>(p.peek().tokid);

  if (p.match(token::ETokenKind::UNDERSCORE)) {
    node.kind = ast::Local_Pattern_Element::Kind::Ignore;
    return node.nodeid();
  }


  // literal case
  if (auto lit = p.p_lit->try_literal(true)) {
    node.kind    = ast::Local_Pattern_Element::Kind::Literal;
    node.literal = lit;
    return node.nodeid();
  }

  node.kind = ast::Local_Pattern_Element::Kind::Binding;

  auto& bind      = p.add_get_node<ast::Local_Binding>(p.peek().tokid);
  bind.capability = p_parent_capa;

  if (p.match_any(token::k_capability)) {
    bind.capability = ast::ETokenKind_to_ECapability(p.peek(-1).kind);
    (void)p.expect(257, token::ETokenKind::TICK, "Expected ''' tick after a pass mode prefix", "");
  }

  bind.name = p.parse_name();

  node.bind = bind.nodeid();

  return bind.nodeid();
}

ast::ID parser::Parser_Declaration_Local::parse_evaluator(ast::ID comparison_expr)
{
  if (p.match_any({token::ETokenKind::CAPA_MUT, token::ETokenKind::CAPA_REF})) {
    return parse_pattern(comparison_expr);
  }

  return p.p_expr->parse_expression();
}

ast::ID parser::Parser_Declaration_Local::parse_pattern(ast::ID comparison_expr)
{
  ast::ECapability capa = ast::ETokenKind_to_ECapability(p.peek(-1).kind);

  if (p.match(token::ETokenKind::L_PAREN)) return tuple_pattern(capa, comparison_expr);


  auto id = p.p_base->identifier(false, true);

  // enum pattern : if ref Some(a) = val {...}
  if (p.match(token::ETokenKind::L_PAREN)) return enum_pattern(capa, id, comparison_expr);

  // form pattern / facet pattern
  if (p.check_chain({token::ETokenKind::STATIC_ACCESS, token::ETokenKind::L_ANGLE}))
    return form_pattern(capa, id, comparison_expr);

  if (p.match(token::ETokenKind::L_CURLY)) return facet_pattern(capa, id, comparison_expr);


  p.add_error(41, "Expected pattern.", "define auto inferred variable like `let myName = expression;`");
  THROW_BAD_NODE;
}

ast::ID parser::Parser_Declaration_Local::variable()
{
  auto& var     = p.add_get_node<ast::Local_Variable>(p.peek().tokid);
  var.kind      = ast::ETokenKind_to_EVariableKind(p.next().kind);
  var.name      = p.parse_name();
  var.is_static = p.metablock_contains(p.tok_to_pos(var.header.start_tokid), "static");

  (void)p.add_definition(var.nodeid());

  bool is_inferred_ty = false;

  // explicit type case
  if (p.match(token::ETokenKind::COLON)) var.type = p.p_type->parse_type();
  // auto deduce type case
  else
    is_inferred_ty = true;

  // check affectation
  var.assignment = ast::ETokenKind_to_ETransfertType(p.peek().kind);

  if (var.assignment == ast::ETransfertType::NONE) {
    if (is_inferred_ty)
      p.add_error(42, "Expected assignation '=' in auto inferred variable type.",
                  "define auto inferred variable like `let myName = expression;`");

    return var.nodeid();
  }

  (void)p.next();

  if (p.match(token::ETokenKind::L_UNINIT)) {
    var.is_uninit = true;
  } else {
    auto expr      = p.p_expr->parse_expression();
    var.expression = expr;
  }

  if (var.is_uninit && var.kind != ast::EVariableKind::_var) {
    p.add_error(64, "Illegal uninit variable with a const or immutable status", "");
  }

  return var.nodeid();
}

ast::ID parser::Parser_Declaration_Local::tuple_destructuring()
{
  constexpr std::string_view hint =
      R"(define unpack like:
  - `var (a, b, c) = myFunction()`
  - with ignored values `var (a, _, c) = myFunction()`)";

  const auto varKind = ast::ETokenKind_to_EVariableKind(p.next().kind);
  auto&      unpack  = p.add_get_node<ast::Local_Tuple_Destructuring>(p.peek(-1).tokid);
  unpack.kind        = varKind;

  while (!p.is_end()) {
    // ignore variable
    if (!p.match(token::ETokenKind::UNDERSCORE)) {
      const auto capa = ast::ETokenKind_to_ECapability(p.peek().kind);

      auto& loc = p.add_get_node<ast::Local_Binding>(p.peek().tokid);
      if (capa != ast::ECapability::NONE) {
        (void)p.next();
        loc.capability = capa;
      }
      loc.name = p.parse_name("", hint);
      unpack.bindings.emplace_back(loc.nodeid());

      (void)p.add_definition(loc.nodeid());
    } else {
      unpack.bindings.emplace_back(ast::ID::make(p.cuid, WILCARD_ID));
    }

    if (p.match_field_separator(token::ETokenKind::COMMA, token::ETokenKind::ASSIGN)) break;
  }

  unpack.expression = p.p_expr->parse_expression();

  return unpack.nodeid();
}

ast::ID parser::Parser_Declaration_Local::lambda_capture()
{
  constexpr std::string_view hint =
      R"(define capture like:
  - modify all variables `[mut]` or `[mut, copy a, ...]`
  - copy all variables `[copy] or `[copy, mut a, ...]`      
  - get instance `[..., self, ...]`)";

  auto& capture = p.add_get_node<ast::Local_Lambda_Capture>(p.peek().tokid);

  // all by ref
  if (p.check_val("mut") && p.check_at(1, token::ETokenKind::R_SQUARE)) {
    capture.is_all_ref = true;
    (void)p.next();
    return capture.nodeid();
  }

  // all by copy
  if (p.check_val("copy") && p.check_at(1, token::ETokenKind::R_SQUARE)) {
    capture.is_all_ref = false;
    (void)p.next();
    return capture.nodeid();
  }

  if (p.match(token::ETokenKind::SELF)) {
    capture.is_capture_self = true;
  }

  while (!p.is_end()) {
    auto& tok_capa = p.expect_any(44, token::k_capability, "Expected capture capability kind.", hint);
    auto  capa     = ast::ETokenKind_to_ECapability(tok_capa.kind);

    ast::ID nodeid;

    switch (capa) {
    case ast::ECapability::NONE:
    case ast::ECapability::ref:  {
      auto& R = p.add_get_node<ast::Expression_Ref_Of>(p.peek(-1).tokid);
      nodeid  = R.nodeid();
    }
    case ast::ECapability::mut: {
      auto& M = p.add_get_node<ast::Expression_Mut_Of>(p.peek(-1).tokid);
      nodeid  = M.nodeid();
    }
    case ast::ECapability::copy: {
      auto& C = p.add_get_node<ast::Expression_Copy_Of>(p.peek(-1).tokid);
      nodeid  = C.nodeid();
    }
    case ast::ECapability::move: {
      auto& M = p.add_get_node<ast::Expression_Move_Of>(p.peek(-1).tokid);
      nodeid  = M.nodeid();
    }
    }

    capture.capture_members.emplace_back(nodeid);
  }

  return capture.nodeid();
}

ast::ID parser::Parser_Declaration_Local::lambda()
{
  auto&      lam       = p.add_get_node<ast::Local_Lambda>(p.peek().tokid);
  const auto old_ret   = p.current_returnable;
  p.current_returnable = lam.nodeid();

  if (p.check(token::ETokenKind::IDENTIFIER)) {
    lam.name = p.parse_name();

    (void)p.add_definition(lam.nodeid());
    p.enter_scope(lam.nodeid(), std::format("lambda \"{}\"", lam.name));
  } else {
    p.enter_scope(lam.nodeid(), "lambda");
  }

  lam.is_pure = p.metablock_contains(p.tok_to_pos(lam.header.start_tokid), "pure");

  // check capture
  if (p.match(token::ETokenKind::L_SQUARE)) lam.capture = lambda_capture();

  auto [protoid, params] = p.p_type->prototype_from_declaration();
  lam.prototype          = protoid;
  lam.parameters         = params;

  lam.codeblock = p.p_loc->parse_codeblock_instruction();

  p.exit_scope();

  p.current_returnable = old_ret;

  return lam.nodeid();
}

ast::ID parser::Parser_Declaration_Local::parse_codeblock_instruction()
{
  (void)p.expect_any(46, {token::ETokenKind::L_CURLY, token::ETokenKind::INJECT},
                     "Expected start code block '{' or linecode '=>'.", "");

  bool  inline_code = p.peek(-1).kind == token::ETokenKind::INJECT;
  auto& cb          = p.add_get_node<ast::CodeBlock>(p.peek().tokid);

  while (!p.is_end()) {
    if (p.match_field_separator(token::ETokenKind::S_END_OF_FILE, token::ETokenKind::R_CURLY)) break;

    if (auto instruction = p.p_base->parse_instruction())
      cb.elements.emplace_back(instruction);
    else
      p.add_error(255, std::format("Unexpected token '{}' inside codeblock.", p.peek().tokid.str()),
                  "A codeblock is ended by '}' or a new line if linecode.");

    // one instruction
    if (inline_code) break;
  }

  return cb.nodeid();
}

ast::ID parser::Parser_Declaration_Local::capability()
{
  constexpr std::string_view hint =
      R"(define capability like:
  - reference (read only) `ref a = lvalue`
  - mutable (read/write) `mut a = lvalue`)";

  auto& capa_tok_kind = p.expect_any(47, token::k_capability, "Expected capability kind.", hint);
  auto& capa          = p.add_get_node<ast::Local_Capability>(p.peek(-1).tokid);
  capa.kind           = ast::ETokenKind_to_ECapability(capa_tok_kind.kind);

  capa.name = p.parse_name("", hint);

  (void)p.expect(48, token::ETokenKind::ASSIGN, "Expected classic assignation '=' after capability declaration.", hint);

  capa.expression = p.p_expr->parse_expression();

  return capa.nodeid();
}

ast::ID parser::Parser_Declaration_Local::facet_pattern(ast::ECapability p_capa, ast::ID p_facet_id,
                                                        ast::ID p_comparison_ref)
{
  constexpr std::string_view hint =
      R"(define facet mapping like:
  - `[ref/mut] CFacet{.field1: [ref/mut/copy]'a, .field2: 10} = val`)";

  assert(ast::ENodeKind_is_symbol(p_facet_id.kind()));
  // Illegal identifier, impossible to use a type

  auto& facet_pat = p.add_get_node<ast::Local_Pattern_Facet>(p.peek().tokid);

  facet_pat.capability = p_capa;
  facet_pat.name       = p_facet_id;

  while (!p.is_end()) {
    if (p.match_field_separator(token::ETokenKind::COMMA, token::ETokenKind::R_CURLY)) break;

    (void)p.expect(241, token::ETokenKind::COMMA, "Facet field local access '.' expected in facet mapping.", hint);

    ast::Local_Pattern_Facet::Field field;
    field.name = p.parse_name("", hint);

    (void)p.expect(49, token::ETokenKind::COLON, "Expected field mapping association ':'.", hint);

    auto mapping = pattern_mapping(p_capa);

    field.mapping = mapping;
    facet_pat.mapping.emplace_back(field);
  }

  if (!p_comparison_ref) {
    (void)p.expect(50, token::ETokenKind::ASSIGN, "Expected assignation on pattern.", hint);
    facet_pat.expression = p.p_expr->parse_expression();
  } else {
    facet_pat.expression = p_comparison_ref;
  }

  // to add ?
  // if (p.match(token::ETokenKind::IF)) facet_pat->additive_evaluator = p.p_expr->parse_expression();

  return facet_pat.nodeid();
}

ast::ID parser::Parser_Declaration_Local::form_pattern(ast::ECapability p_capa, ast::ID p_form_id,
                                                       ast::ID p_comparison_ref)
{
  constexpr std::string_view hint =
      R"(define form mapping like:
  - facet general mapping `[ref/mut] MyForm{CFacet{.field1: [ref/mut/copy] a, .field2: 10}}`
  - facet field mapping `[ref/mut] MyForm{CFacet.field1: [ref/mut/copy] a, CFacet.field2: 10}`)";

  (void)p.match(token::ETokenKind::STATIC_ACCESS);
  (void)p.match(token::ETokenKind::L_ANGLE);

  assert(ast::ENodeKind_is_symbol(p_form_id.kind()));
  // Illegal identifier, impossible to use a type

  auto& form_pat      = p.add_get_node<ast::Local_Pattern_Form>(p.peek().tokid);
  form_pat.name       = p_form_id;
  form_pat.capability = p_capa;

  while (!p.is_end()) {
    auto facet_identifier = p.p_base->identifier();

    if (p.match(token::ETokenKind::L_CURLY)) {
      auto facet = facet_pattern(p_capa, facet_identifier, p_comparison_ref);
      form_pat.pattern_facets.emplace_back(facet);
    } else {
      auto& pattern_facet      = p.add_get_node<ast::Local_Pattern_Facet>(p.peek(-1).tokid);
      pattern_facet.name       = facet_identifier;
      pattern_facet.capability = p_capa;

      ast::Local_Pattern_Facet::Field field;

      (void)p.expect(52, token::ETokenKind::DOT, "Expected facet field mapping '.' or start facet general mapping '{'.",
                     hint);
      field.name = p.parse_name("", hint);

      (void)p.expect(53, token::ETokenKind::COLON, "Expected field mapping association ':'.", hint);
      field.mapping = pattern_mapping(p_capa);

      pattern_facet.mapping = {field};
      form_pat.pattern_facets.emplace_back(pattern_facet.nodeid());
    }

    if (p.match_field_separator(token::ETokenKind::COMMA, token::ETokenKind::R_CURLY)) break;
  }

  if (!p_comparison_ref) {
    (void)p.expect(54, token::ETokenKind::ASSIGN, "Expected assignation on pattern.", hint);
    form_pat.expression = p.p_expr->parse_expression();
  } else {
    form_pat.expression = p_comparison_ref;
  }

  // if (p.match(token::ETokenKind::IF)) form_pat->additive_evaluator = p.p_expr->parse_expression();

  return form_pat.nodeid();
}

ast::ID parser::Parser_Declaration_Local::tuple_pattern(ast::ECapability p_capa, ast::ID p_comparison_ref)
{
  constexpr std::string_view hint =
      R"(define enum pattern on condition like:                                                               
  - binding pattern `[if/elif/while] [ref/mut] ([copy/mut/ref] a, _, 10) = tuple {...}`
  Rules: - ref -> bind primitives by `copy`, bind complex types by `ref`
         - mut -> bind primitives and complex types by `mut`
         - override binding rule by explicit binding mode `([copy/mut/ref] a)`)";

  auto& pat      = p.add_get_node<ast::Local_Pattern_Tuple>(p.peek().tokid);
  pat.capability = p_capa;

  if (p.check(token::ETokenKind::R_PAREN)) p.add_error(55, "Unexpected void tuple.", hint);

  while (!p.is_end()) {
    pat.pattern_elements.emplace_back(pattern_mapping(p_capa));
    if (p.match_field_separator(token::ETokenKind::COMMA, token::ETokenKind::R_PAREN)) break;
  }

  if (!p_comparison_ref) {
    (void)p.expect(56, token::ETokenKind::ASSIGN, "Expected assignation on pattern.", hint);
    pat.expression = p.p_expr->parse_expression();
  } else {
    pat.expression = p_comparison_ref;
  }

  // if (p.match(token::ETokenKind::IF)) pat->additive_evaluator = p.p_expr->parse_expression();

  return pat.nodeid();
}

ast::ID parser::Parser_Declaration_Local::enum_pattern(ast::ECapability p_capa, ast::ID p_enum_id,
                                                       ast::ID p_comparison_ref)
{
  constexpr std::string_view hint =
      R"(define enum pattern on condition like:
  - binding pattern `[if/elif/while] [ref/mut] Some(a) = value {...}`
  - binding pattern with more condition `if [ref/mut] Some(a) = value if a > 10 {...}`
  - binding pattern conditionnal `if [ref/mut] Some(10) = value {...}`                            
  - check only indexation `[if/elif/while] value == Some`)";

  assert(ast::ENodeKind_is_symbol(p_enum_id.kind()));
  // Illegal identifier, impossible to use a type

  auto& pat = p.add_get_node<ast::Local_Pattern_Enum>(p.peek().tokid);
  pat.name  = p_enum_id;

  (void)p.expect(57, token::ETokenKind::L_PAREN, "Expected start binding on enum types '('.", hint);

  if (p.check(token::ETokenKind::R_PAREN))
    p.add_error(58,
                "Unexpected end of binding on enum types ')'. A enum pattern on condition must have at "
                "least one binding. Else, use a check indexation.",
                hint);

  ast::EExprPassMode pass_mode = ast::ETokenKind_to_EExprPassMode(p.peek().kind);
  if (pass_mode != ast::EExprPassMode::NONE) {
    (void)p.next();
    (void)p.expect(258, token::ETokenKind::TICK, "Expected ''' tick after a pass mode prefix", hint);
  }

  while (!p.is_end()) {
    pat.pattern_elements.emplace_back(pattern_mapping(p_capa));
    if (p.match_field_separator(token::ETokenKind::COMMA, token::ETokenKind::R_PAREN)) break;
  }

  if (!p_comparison_ref) {
    (void)p.expect(59, token::ETokenKind::ASSIGN, "Expected assignation on pattern.", hint);
    pat.expression = p.p_expr->parse_expression();
  } else {
    pat.expression = p_comparison_ref;
  }

  // if (p.match(token::ETokenKind::IF)) pat->additive_evaluator = p.p_expr->parse_expression();

  return pat.nodeid();
}
