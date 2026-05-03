#include "parser_declaration_cop.hpp"

#include <vector>

#include "ast/ast_base.hpp"
#include "compiler/compiler.hpp"
#include "nexus/ast/ast.hpp"
#include "nexus/lexer/token.hpp"

#include "ast/ast_declaration_cop.hpp"
#include "ast/ast_declaration_local.hpp"
#include "ast/ast_expression.hpp"
#include "ast/ast_literal.hpp"

#include "nexus/forward.hpp"
#include "nexus/lexer/token_viewer.hpp"
#include "nexus/script.hpp"

#include "nexus/symbol.hpp"
#include "nexus/type.hpp"
#include "parser/parser_base.hpp"
#include "parser_context.hpp"
#include "parser_expression.hpp"
#include "parser_literal.hpp"
#include "parser_declaration_local.hpp"
#include "parser_type.hpp"
#include "ast/ast_statement.hpp"


ast::_gnid parser::Parser_Declaration_COP::component()
{
  constexpr std::string_view hint =
      "define component declaration like:"
      "\n  - multi filed `comp name { var field_name: type = value, ... }`"
      "\n  - no field `comp name {}";

  p.match(token::ETokenKind::COMPONENT);

  parser_add_node(comp, COP_Component, p.peek().id);
  // not handled if (auto where = ctx.p_meta->metacode_where()) comp->gen_where;

  comp->name  = p.parse_name("", hint);
  auto sym_id = p.add_symbol(comp->node_id.get_node_id());
  p.expect(14, token::ETokenKind::OPEN_BRACE, "Expected start code block '{' after component declaration.", hint);

  // is no typed component
  if (p.match(token::ETokenKind::CLOSE_BRACE)) return comp->node_id;

  std::vector<type::_id> factory_types;

  while (!p.tok_v->is_end()) {
    parser_add_node(field, COP_Component_Field, p.peek().id);
    field->is_no_default = p.metablock_contains(p.peek().begin, "nodefault");

    if (p.match(token::ETokenKind::CAPA_REF))
      field->capability = ast::ECapability::Ref;
    else if (p.match(token::ETokenKind::CAPA_MUT))
      field->capability = ast::ECapability::Mut;

    field->name = p.parse_name("", hint);
    p.expect(15, token::ETokenKind::COLON, "Expected type defintion symbol ':' after field name", hint);

    field->type = p.p_type->parse_type();
    factory_types.push_back(field->type);

    p.add_symbol(field->node_id.get_node_id());

    if (!field->is_no_default) {
      p.expect(16, token::ETokenKind::ASSIGN, "Expected default value assignation '=' after field declaration", hint);

      field->default_value = p.p_expr->parse_expression();
    }

    comp->fields.push_back(field->node_id);

    if (p.match_field_separator(token::ETokenKind::COMMA, token::ETokenKind::CLOSE_BRACE)) break;
  }

  parser_type_factory.make_component(factory_types, sym_id);

  return comp->node_id;
}

ast::_gnid parser::Parser_Declaration_COP::role()
{
  constexpr std::string_view hint = "define role like: `role name { comp1, comp2, ... }`";

  p.match(token::ETokenKind::ROLE);

  auto tok = p.peek();

  parser_add_node(role, COP_Role, tok.id);
  role->name = p.parse_name("", hint);

  auto sym_id = p.add_symbol(role->node_id.get_node_id());
  p.enter_scope(*role, "role " + std::string(role->name));

  p.expect(17, token::ETokenKind::OPEN_BRACE, "Expected start definition '{' after role declaration.", hint);

  while (!p.tok_v->is_end()) {
    auto [gnid, ty_id] = p.p_base->identifier_typed();
    role->components.push_back(ty_id);

    if (p.match_field_separator(token::ETokenKind::COMMA, token::ETokenKind::CLOSE_BRACE)) break;
  }

  p.exit_scope();

  parser_type_factory.make_role(role->components, sym_id);

  return role->node_id;
}

ast::_gnid parser::Parser_Declaration_COP::entity()
{
  constexpr std::string_view hint =
      "define entity like:"
      "\n  - `entity MyName { ... }`"
      "\n  - with parent `entity MyName : MyParent { ... }`";

  p.match(token::ETokenKind::ENTITY);

  parser_add_node(def_entity, COP_Entity, p.peek().id);

  auto tok_pos = p.peek().begin;

  // metacode
  def_entity->isCastable     = !p.metablock_contains(tok_pos, "nocast");
  def_entity->isExtCastable  = !p.metablock_contains(tok_pos, "no_extern_cast");
  def_entity->isMoveable     = !p.metablock_contains(tok_pos, "no_move");
  def_entity->isDestructible = !p.metablock_contains(tok_pos, "no_destruct");
  def_entity->name           = p.parse_name("", hint);

  p.add_symbol(def_entity->node_id.get_node_id());
  p.enter_scope(*def_entity, "entity " + std::string(def_entity->name));

  p.expect(18, token::ETokenKind::OPEN_BRACE, "Expected start code block '{' after entity declaration.", hint);

  while (!p.tok_v->is_end()) {
    parse_entity_declaration(*def_entity);

    if (p.match_field_separator(token::ETokenKind::S_END_OF_FILE, token::ETokenKind::CLOSE_BRACE)) break;
  }

  p.exit_scope();

  return def_entity->node_id;
}

void parser::Parser_Declaration_COP::parse_entity_declaration(ast::COP_Entity& entity)
{
  constexpr std::string_view hint =
      R"(define comp usage like:
  - `use name { field1: val1, field2: val2 }`
  - `use name`)";
  constexpr std::string_view new_hint = "define new entity def like: `new(params) { ... }`";
  constexpr std::string_view del_hint = "define del entity def like: `del { ... }`";

  if (p.match(token::ETokenKind::USE)) {
    ast::_gnid comp_id;

    auto comp_name = p.p_base->identifier();

    if (auto [gnid, ty] = p.p_base->identifier_typed(); ty) {
      auto n_ty  = p.scr_info.nodes->get_as<ast::ID_Typed>(gnid.get_node_id());
      n_ty->name = comp_name;
      comp_id    = gnid;
    } else {
      comp_id = comp_name;
    }
    auto comp = p.p_lit->literal_component(comp_id);

    if (auto ptr = p.scr_info.nodes->get_as<ast::Literal_Structured_Data>(comp.get_node_id())) {
      entity.components.push_back(comp);
    } else {
      p.add_error(19, "Expected Literal component after 'use' instruction", hint);
    }

    return;
  } else if (p.match(token::ETokenKind::NEW)) {

    parser_add_node(e_new, COP_Entity_New, p.peek().id);
    p.enter_scope(*e_new, "new");

    p.p_type->parse_and_mount_local_callable(e_new->prototype, e_new->is_explicit_ret_type);

    p.expect(20, token::ETokenKind::OPEN_BRACE, "Expected start code '{'.", new_hint);

    e_new->codeblock = p.p_loc->parse_codeblock();

    p.exit_scope();
    entity.constructors.push_back(e_new->node_id);
    return;
  } else if (p.match(token::ETokenKind::DEL)) {

    parser_add_node(e_del, COP_Entity_Del, p.peek().id);
    p.enter_scope(*e_del, "del");
    p.expect(20, token::ETokenKind::OPEN_BRACE, "Expected start code '{'.", del_hint);

    e_del->codeblock = p.p_loc->parse_codeblock();

    p.exit_scope();
    entity.constructors.push_back(e_del->node_id);
    return;
  } else if (p.match(token::ETokenKind::OP)) {
    if (p.tok_v->check_any(
            {token::ETokenKind::INTERROGATIVE, token::ETokenKind::OPEN_SQUARE, token::ETokenKind::TILDE})) {

      auto access = _entity_access_op();

      entity.accessors.push_back(access);
    } else {
      auto op = _entity_op();
      entity.operators.push_back(op);
    }

    return;
  } else if (p.match(token::ETokenKind::CAST)) {
    auto cast = _entity_cast(entity.node_id);
    entity.casters.push_back(cast);
    return;
  }

  p.add_error(21,
              "Unexpected '" + std::string(p.tok_to_str(p.peek().id)) + "' keyword not allowed in entity code block.",
              "you can define in functions: atribute, method, typealias, operator "
              "overloading, trait implementation.");
}

ast::_gnid parser::Parser_Declaration_COP::_entity_op()
{
  constexpr std::string_view hint = "define entity operator overloading like `op + { ... }`.";

  auto tok = p.peek();

  // other operator case op + - / * ...
  parser_add_node(entity_op, COP_Entity_Op, p.peek().id);

  p.enter_scope(*entity_op, "op");
  auto op_tok      = p.expect_any(27, token::k_operator, "Expected operator in entity operator overloading.", hint);
  entity_op->op_ty = ast::ETokenKind_to_EBinOpType(op_tok.kind);

  if (p.check(token::ETokenKind::ARROW))
    p.add_error(28,
                "Unexpected retrun type definition '-> T' after a entity operator '"
                    + std::string(EBinOpType_to_str(entity_op->op_ty)) + "'.",
                hint);

  p.expect(29, token::ETokenKind::OPEN_BRACE, "Expected start code block '{' after entity operator overloading.", hint);

  entity_op->codeblock = p.p_loc->parse_codeblock();

  p.exit_scope();

  return entity_op->node_id;
}


ast::_gnid parser::Parser_Declaration_COP::_entity_access_op()
{
  constexpr std::string_view hint =
      R"(define entity index overloading like:"
  - index `op [a] -> T {...}`"
  - range `op [r..] -> Slice<T> {...}`.)";

  auto tok = p.peek();

  // if index operator case op [] -> T { ... }
  parser_add_node(_access_op, COP_Entity_Access_Op, p.peek().id);
  p.enter_scope(*_access_op, "access_op");
  _access_op->op_ty =
      p.peek(-1).kind == token::ETokenKind::INTERROGATIVE ? ast::EAccessOpType::IndexBound : ast::EAccessOpType::Index;
  p.match(token::ETokenKind::OPEN_SQUARE); // if on bounded index
  _access_op->parameter_name = p.parse_name("Expected index name binding", hint);

  if (p.check(token::ETokenKind::DOT) && p.check_at(1, token::ETokenKind::DOT)) {
    _access_op->op_ty = _access_op->op_ty == ast::EAccessOpType::IndexBound ? ast::EAccessOpType::SliceBound
                                                                            : ast::EAccessOpType::Slice;
  }

  p.expect(23, token::ETokenKind::CLOSE_SQUARE, "Expected closed index operator ']'", hint);
  p.expect(24, token::ETokenKind::ARROW, "Expected explicit return type '-> T'", hint);

  // return type expected for index operator
  p.expect(25, token::ETokenKind::ARROW, "Expected return definition '-> T' after index operator '[]' overload.", hint);

  _access_op->ret = p.p_type->parse_type();


  p.expect(29, token::ETokenKind::OPEN_BRACE, "Expected start code block '{' after entity operator overloading.", hint);


  _access_op->codeblock = p.p_loc->parse_codeblock();

  p.exit_scope();

  return _access_op->node_id;
}

ast::_gnid parser::Parser_Declaration_COP::_entity_cast(ast::_gnid entity)
{
  constexpr std::string_view hint =
      R"(define entity cast overloading like:"
  - `cast self as T { ... }`."
  - `cast T as self { ... }`.)";

  parser_add_node(cast, COP_Entity_Cast, p.peek().id);

  p.enter_scope(*cast, "cast");

  auto key_self_case = [&]() {
    parser_add_node(self, Expression_Self, p.peek().id);
    return self->node_id;
  };

  auto key_other_case = [&]() { return p.p_type->parse_type(); };

  if (p.match(token::ETokenKind::SELF)) {
    auto self            = key_self_case();
    cast->source         = self;
    cast->source_is_self = true;
  } else {
    auto id      = key_other_case();
    cast->target = id;
  }

  p.expect(30, token::ETokenKind::AS, "Expected cast linker 'as' after casting source.", hint);

  if (p.match(token::ETokenKind::SELF)) {
    if (cast->source_is_self) {
      p.add_error(31,
                  "Expected other type than 'self' in target cast after 'self' in "
                  "source cast, cast can't be with himself.",
                  hint);
      return BAD_NODE_ID;
    }

    auto self    = key_self_case();
    cast->source = self;
  } else {
    if (!cast->source_is_self) {
      p.add_error(32,
                  "Expected 'self' in target cast after type casted in source cast.\n"
                  "  Cast can't be extern to the concerned entity.",
                  hint);
      return BAD_NODE_ID;
    }

    auto id      = key_other_case();
    cast->target = id;
  }

  p.expect(33, token::ETokenKind::OPEN_BRACE, "Expected start code block '{' after casting definition.", hint);

  cast->codeblock = p.p_loc->parse_codeblock();

  p.exit_scope();
  return cast->node_id;
}

ast::_gnid parser::Parser_Declaration_COP::system()
{
  constexpr std::string_view hint =
      R"(define system like:
  - single behaviour
   `sys Move() {
      Position(pos) + Velocity(vel) => update_physic(pos, vel)
  - multiple behaviour
   `sys Speak(ref msg: str) {
      other => print(\"say \")
      ID(id) => {
        print(\"(\" + id.name + \"): \" + msg)
        return
      }
      other => print(msg)
    }`)";

  p.match(token::ETokenKind::SYSTEM);

  auto tok = p.peek();

  parser_add_node(system, COP_System, p.peek().id);

  // not handled if (auto where = ctx.p_meta->metacode_where()) system->generic = where.value();

  system->name = p.parse_name("", hint);

  p.add_symbol(system->node_id.get_node_id());
  p.enter_scope(*system, "system " + std::string(system->name));

  p.p_type->parse_and_mount_local_callable(system->prototype, system->is_explicit_ret_type);

  bool is_no_comp_used = true;

  p.expect(34, token::ETokenKind::OPEN_BRACE, "Expected start code block '{' after system declaration.", hint);

  while (!p.tok_v->is_end()) {
    p.expect(35, token::ETokenKind::WITH, "Expected behaviour block 'with' in system code block.", hint);

    auto sys_case   = _system_case();
    auto n_sys_case = p.scr_info.nodes->get_as<ast::COP_System_Case>(sys_case.get_node_id());

    if (!n_sys_case->bindings.empty()) is_no_comp_used = false;
    system->cases.push_back(sys_case);

    if (p.check(token::ETokenKind::WITH)) continue;
    if (p.match(token::ETokenKind::CLOSE_BRACE)) break;
    p.add_error(36, "Expected behaviour block 'with' in system code block.", hint);
  }

  // pre semantic checking system form is useful or a function is prefered ? (case of no components specifed)
  if (is_no_comp_used) {
    p.add_error_tok(37, tok,
                    "Expected function instead of system given the behaviour of the code "
                    "block: no component specified in any where statement.",
                    hint);
  }

  p.exit_scope();

  return system->node_id;
}

ast::_gnid parser::Parser_Declaration_COP::_system_case()
{
  constexpr std::string_view hint =
      R"(define system case like:
  - no component (for universal/last/default behaviour)
   `_ => { Println(\"NPC don't have any item!\") }`
  - single component (for single operation most of the time)
   `CCounter(c) => add_count(c)`
  - multiple components (for interaction between components most of the time)
   `CPosition(pos) + CVelocity(vel) => update_move(pos, vel))";

  parser_add_node(sys_case, COP_System_Case, p.peek().id);
  p.match(token::ETokenKind::WITH);
  sys_case->is_default = p.match(token::ETokenKind::UNDERSCORE);

  if (!sys_case->is_default) {
    while (!p.tok_v->is_end()) {
      p.expect(38, token::ETokenKind::OPEN_PAREN, "Expected start binding '(' after component name pattern.", hint);

      parser_add_node(bind, Local_Binding, p.peek().id);
      bind->name = p.parse_name("", hint);
      p.add_symbol(bind->node_id.get_node_id());
      sys_case->bindings.push_back(bind->node_id);

      p.expect(39, token::ETokenKind::OPEN_PAREN, "Expected end binding ')' after component name pattern.", hint);

      if (p.match_field_separator(token::ETokenKind::OP_PLUS, token::ETokenKind::OPEN_BRACE)) break;
    }
  }

  sys_case->codeblock = p.p_loc->parse_codeblock();
  auto n_cb           = p.scr_info.nodes->get_as<ast::Local_CodeBlock>(sys_case->codeblock.get_node_id());
  for (auto& i_id : n_cb->elements) {
    if (auto ret = p.scr_info.nodes->get_as<ast::Statement_Return>(i_id.get_node_id())) {
      sys_case->have_return = true;
      break;
    }
  }

  return sys_case->node_id;
}
