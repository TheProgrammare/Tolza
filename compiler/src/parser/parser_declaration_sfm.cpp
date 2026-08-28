#include "parser_declaration_sfm.hpp"

#include "ast/ast_base.hpp"
#include "ast/ast_declaration_local.hpp"
#include "ast/ast_declaration_sfm.hpp"
#include "ast/ast_expression.hpp"
#include "ast/ast_literal.hpp"
#include "ast/ast_statement.hpp"
#include "compiler/compilation_unit.hpp"
#include "compiler/compiler.hpp"
#include "nexus/ast/ast.hpp"
#include "nexus/ast/data.hpp"
#include "nexus/ast/definition.hpp"
#include "nexus/ast/forward.hpp"
#include "nexus/definition.hpp"
#include "nexus/forward.hpp"
#include "nexus/lexer/token.hpp"
#include "nexus/lexer/token_viewer.hpp"
#include "nexus/type/type.hpp"
#include "parser/parser_base.hpp"
#include "parser_context.hpp"
#include "parser_declaration_local.hpp"
#include "parser_expression.hpp"
#include "parser_literal.hpp"
#include "parser_type.hpp"

#include <vector>


ast::ID parser::Parser_Declaration_SFM::facet()
{
  constexpr std::string_view hint =
      "define facet declaration like:"
      "\n  - multi filed `facet name { var field_name: type = value, ... }`"
      "\n  - no field `facet name {}";

  (void)p.match(token::ETokenKind::FACET);

  auto& facet = p.add_get_node<ast::SFM_Facet>(p.peek().tokid);
  // not handled if (auto where = ctx.p_meta->metacode_where()) facet.gen_where;

  facet.name = p.parse_name("", hint);
  auto defid = p.add_definition(facet.nodeid());
  (void)p.expect(14, token::ETokenKind::L_CURLY, "Expected start code block '{' after facet declaration.", hint);

  // is no typed facet
  if (p.match(token::ETokenKind::R_CURLY)) return facet.nodeid();

  std::vector<type::ID> factory_types;

  while (!p.tok_v->is_end()) {
    auto& field         = p.add_get_node<ast::SFM_Facet_Field>(p.peek().tokid);
    field.is_no_default = p.metablock_contains(p.peek().begin, "nodefault");

    if (p.match(token::ETokenKind::CAPA_REF))
      field.capability = ast::ECapability::ref;
    else if (p.match(token::ETokenKind::CAPA_MUT))
      field.capability = ast::ECapability::mut;

    field.name = p.parse_name("", hint);
    (void)p.expect(15, token::ETokenKind::COLON, "Expected type definition symbol ':' after field name", hint);

    field.type = p.p_type->parse_type();
    factory_types.emplace_back(field.type);

    (void)p.add_definition(field.nodeid());

    if (!field.is_no_default) {
      (void)p.expect(16, token::ETokenKind::ASSIGN, "Expected default value assignation '=' after field declaration",
                     hint);

      field.default_value = p.p_expr->parse_expression();
    }

    facet.fields.emplace_back(field.nodeid());

    if (p.match_field_separator(token::ETokenKind::COMMA, token::ETokenKind::R_CURLY)) break;
  }

  (void)parser_type_factory.make_facet(factory_types, defid);

  return facet.nodeid();
}

ast::ID parser::Parser_Declaration_SFM::view()
{
  constexpr std::string_view hint = "define view like: `view name { comp1, comp2, ... }`";

  (void)p.match(token::ETokenKind::VIEW);

  auto& tok = p.peek();

  auto& view = p.add_get_node<ast::SFM_View>(tok.tokid);
  view.name  = p.parse_name("", hint);

  auto defid = p.add_definition(view.nodeid());
  p.enter_scope(view.nodeid(), "view " + std::string(view.name));

  (void)p.expect(17, token::ETokenKind::L_CURLY, "Expected start definition '{' after view declaration.", hint);

  while (!p.tok_v->is_end()) {
    auto id_id = p.p_base->identifier();

    auto [id, tyid] = p.p_base->identifier_typed();

    tyid.as<type::Identifier>()->forward_name = ast::get_decl_name(id_id);
    id.as<ast::Symbol_Type>()->name           = id_id;

    view.facets.emplace_back(tyid);

    if (p.match_field_separator(token::ETokenKind::COMMA, token::ETokenKind::R_CURLY)) break;
  }

  p.exit_scope();

  (void)parser_type_factory.make_view(view.facets, defid);

  return view.nodeid();
}

ast::ID parser::Parser_Declaration_SFM::form()
{
  constexpr std::string_view hint =
      "define form like:"
      "\n  - `form MyName { ... }`";

  (void)p.match(token::ETokenKind::FORM);

  auto& def_form = p.add_get_node<ast::SFM_Form>(p.peek().tokid);

  auto tok_pos = p.peek().begin;

  // metacode
  def_form.isCastable     = !p.metablock_contains(tok_pos, "nocast");
  def_form.isExtCastable  = !p.metablock_contains(tok_pos, "no_extern_cast");
  def_form.isMoveable     = !p.metablock_contains(tok_pos, "no_move");
  def_form.isDestructible = !p.metablock_contains(tok_pos, "no_destruct");
  def_form.name           = p.parse_name("", hint);

  (void)p.add_definition(def_form.nodeid());
  p.enter_scope(def_form.nodeid(), "form " + std::string(def_form.name));

  (void)p.expect(18, token::ETokenKind::L_CURLY, "Expected start code block '{' after form declaration.", hint);

  while (!p.tok_v->is_end()) {
    parse_form_declaration(def_form);

    if (p.match_field_separator(token::ETokenKind::S_END_OF_FILE, token::ETokenKind::R_CURLY)) break;
  }

  p.exit_scope();

  return def_form.nodeid();
}

void parser::Parser_Declaration_SFM::parse_form_declaration(ast::SFM_Form& form)
{
  constexpr std::string_view hint =
      R"(define facet usage like:
  - `use MyFacet {.field1= val1, .field2= val2 }`
  - `use MyFacet`)";

  if (p.match(token::ETokenKind::USE)) {
    auto facet_name = p.p_base->identifier();

    if (auto [id, ty] = p.p_base->identifier_typed(); ty) {
      ty.as<type::Identifier>()->forward_name = ast::get_decl_name(facet_name);
      id.as<ast::Symbol_Type>()->name         = facet_name;
      facet_name                              = id;
    }

    ast::ID facet;
    if (p.check(token::ETokenKind::L_CURLY))
      facet = p.p_lit->literal_record(facet_name);
    else
      facet = facet_name;

    form.facets.emplace_back(facet);

    if (!facet.as<ast::Literal_Record>()) p.add_error(19, "Expected Literal facet after 'use' instruction", hint);

    return;
  }

  p.add_error(21, std::format("Unexpected '{}' keyword not allowed in form code block.", p.tok_to_str(p.peek().tokid)),
              "you can define in functions: atribute, method, typealias, operator "
              "overloading, trait extension.");
}

ast::ID parser::Parser_Declaration_SFM::rule()
{
  constexpr std::string_view hint =
      R"(define rule like:
  - single behaviour
   `rule Move() {
      Position(pos) + Velocity(vel) => update_physic(pos, vel)
  - multiple behaviour
   `rule Speak(ref msg: str) {
      other => print(\"say \")
      ID(id) => {
        print(\"(\" + id.name + \"): \" + msg)
        return
      }
      other => print(msg)
    }`)";

  (void)p.match(token::ETokenKind::RULE);

  auto& tok = p.peek();

  auto& n = p.add_get_node<ast::SFM_Rule>(p.peek().tokid);

  // not handled if (auto where = ctx.p_meta->metacode_where()) rule->generic = where.value();

  n.name               = p.parse_name("", hint);
  const auto old_ret   = p.current_returnable;
  p.current_returnable = n.nodeid();

  (void)p.add_definition(n.nodeid());
  p.enter_scope(n.nodeid(), "rule " + n.name);

  auto [protoid, params] = p.p_type->prototype_from_declaration();
  n.prototype            = protoid;
  n.parameters           = params;

  bool is_no_facet_used = true;

  (void)p.expect(34, token::ETokenKind::L_CURLY, "Expected start code block '{' after rule declaration.", hint);

  while (!p.tok_v->is_end()) {
    (void)p.expect(35, token::ETokenKind::WITH, "Expected behaviour block 'with' in rule code block.", hint);

    auto        rule_case   = _rule_case();
    const auto* n_rule_case = rule_case.as<ast::SFM_Rule_Case>();

    if (!n_rule_case->bindings.empty()) is_no_facet_used = false;
    n.cases.emplace_back(rule_case);

    if (p.check(token::ETokenKind::WITH)) continue;
    if (p.match(token::ETokenKind::R_CURLY)) break;
    p.add_error(36, "Expected behaviour block 'with' in rule code block.", hint);
  }

  // pre semantic checking rule form is useful or a function is prefered ? (case of no facets specifed)
  if (is_no_facet_used) {
    p.add_error_tok(37, tok,
                    "Expected function instead of rule given the behaviour of the code "
                    "block: no facet specified in any where statement.",
                    hint);
  }

  p.exit_scope();
  p.current_returnable = old_ret;

  return n.nodeid();
}

ast::ID parser::Parser_Declaration_SFM::_rule_case()
{
  constexpr std::string_view hint =
      R"(define rule case like:
  - no facet (for universal/last/default behaviour)
   `_ => { Println(\"NPC don't have any item!\") }`
  - single facet (for single operation most of the time)
   `CCounter(c) => add_count(c)`
  - multiple facets (for interaction between facets most of the time)
   `CPosition(pos) + CVelocity(vel) => update_move(pos, vel))";

  auto& rule_case = p.add_get_node<ast::SFM_Rule_Case>(p.peek().tokid);
  (void)p.match(token::ETokenKind::WITH);
  rule_case.is_default = p.match(token::ETokenKind::UNDERSCORE);

  if (!rule_case.is_default) {
    while (!p.tok_v->is_end()) {
      (void)p.expect(38, token::ETokenKind::L_PAREN, "Expected start binding '(' after facet name pattern.", hint);

      auto& bind = p.add_get_node<ast::Local_Binding>(p.peek().tokid);
      bind.name  = p.parse_name("", hint);
      (void)p.add_definition(bind.nodeid());
      rule_case.bindings.emplace_back(bind.nodeid());

      (void)p.expect(39, token::ETokenKind::L_PAREN, "Expected end binding ')' after facet name pattern.", hint);

      if (p.match_field_separator(token::ETokenKind::OP_PLUS, token::ETokenKind::L_CURLY)) break;
    }
  }

  rule_case.codeblock = p.p_loc->parse_codeblock_instruction();
  const auto* n_cb    = rule_case.codeblock.as<ast::CodeBlock>();
  for (const auto& i_id : n_cb->elements) {
    if (i_id.as<ast::Statement_Return>()) {
      rule_case.have_return = true;
      break;
    }
  }

  return rule_case.nodeid();
}
