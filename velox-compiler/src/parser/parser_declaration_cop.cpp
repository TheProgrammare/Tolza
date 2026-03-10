#include "parser_declaration_cop.hpp"

#include <memory>
#include <vector>

#include "ast/ast_declaration_cop.hpp"
#include "ast/ast_declaration_local.hpp"
#include "ast/ast_expression.hpp"
#include "ast/ast_statement.hpp"
#include "ast/ast_type.hpp"
#include "ast/ast_literal.hpp"

#include "parser_context.hpp"
#include "parser_expression.hpp"
#include "parser_literal.hpp"
#include "parser_declaration_local.hpp"
#include "parser_type.hpp"

#include "visitor/symbol_manager.hpp"

std::shared_ptr<ast::declaration::cop::Component> parser::Parser_Declaration_COP::component()
{
  static const std::string hint =
      "define component declaration like:"
      "\n  - multi filed `comp name { var field_name: type = value, ... }`"
      "\n  - no field `comp name {}";

  ctx.tok_v.match(TokTy::COMPONENT);

  auto comp = ctx.Create_Decl<ast::declaration::cop::Component>(ctx.tok_v.peek());

  // not handled if (auto where = ctx.p_meta->metacode_where()) comp->gen_where;

  comp->name = ctx.parse_name("", hint);
  ctx.tok_v.expect(14, TokTy::OPEN_BRACE, "Expected start code block '{' after '" + comp->debug_str() + "'.", hint);

  // is no typed component
  if (ctx.tok_v.match(TokTy::CLOSE_BRACE)) return comp;

  while (!ctx.tok_v.is_end()) {
    auto field         = ctx.Create_Decl<ast::declaration::cop::Component_Field>(ctx.tok_v.peek());
    field->isNoDefault = ctx.metablock_contains(*field, "nodefault");

    field->name = ctx.parse_name("", hint);
    ctx.tok_v.expect(15, TokTy::COLON, "Expected type defintion symbol ':' after field name", hint);

    field->type = ctx.p_type->parse_type();

    ctx.m_sym->add_decl(field);

    if (!field->isNoDefault) {
      ctx.tok_v.expect(16, TokTy::ASSIGN, "Expected default value assignation '=' after field declaration", hint);

      field->default_value = ctx.p_expr->parse_expression();
    }

    field->parent_component = comp;

    comp->fields.push_back(field);

    if (ctx.match_field_separator(TokTy::COMMA, TokTy::CLOSE_BRACE)) break;
  }

  return comp;
}

std::shared_ptr<ast::declaration::cop::Role> parser::Parser_Declaration_COP::role()
{
  static const std::string hint = "define role like: `role name { comp1, comp2, ... }`";

  ctx.tok_v.match(TokTy::ROLE);

  Token tok = ctx.tok_v.peek();

  auto role  = ctx.Create_Decl<ast::declaration::cop::Role>(tok);
  role->name = ctx.parse_name("", hint);
  ctx.m_sym->add_decl(role);
  ctx.m_sym->enter_scope(role->name, EScopeType::Role);

  ctx.tok_v.expect(17, TokTy::OPEN_BRACE, "Expected start definition '{' after role declaration.", hint);

  while (!ctx.tok_v.is_end()) {
    role->components.push_back(ctx.p_expr->parse_expression());

    if (ctx.match_field_separator(TokTy::COMMA, TokTy::CLOSE_BRACE)) break;
  }

  ctx.m_sym->exit_scope();

  return role;
}

std::shared_ptr<ast::declaration::cop::Entity> parser::Parser_Declaration_COP::entity()
{
  static const std::string hint =
      "define entity like:"
      "\n  - `entity MyName { ... }`"
      "\n  - with parent `entity MyName : MyParent { ... }`";

  ctx.tok_v.match(TokTy::ENTITY);

  auto def_entity = ctx.Create_Decl<ast::declaration::cop::Entity>(ctx.tok_v.peek());

  // metacode
  def_entity->isCastable     = !ctx.metablock_contains(*def_entity, "nocast");
  def_entity->isExtCastable  = !ctx.metablock_contains(*def_entity, "no_extern_cast");
  def_entity->isMoveable     = !ctx.metablock_contains(*def_entity, "no_move");
  def_entity->isDestructible = !ctx.metablock_contains(*def_entity, "no_destruct");
  def_entity->name           = ctx.parse_name("", hint);
  auto entity_sym            = ctx.m_sym->add_decl(def_entity);
  ctx.m_sym->enter_scope(def_entity->name, EScopeType::Entity);

  ctx.tok_v.expect(18, TokTy::OPEN_BRACE, "Expected start code block '{' after entity declaration.", hint);

  while (!ctx.tok_v.is_end()) {
    parse_entity_declaration(def_entity);

    if (ctx.match_field_separator(TokTy::S_END_OF_FILE, TokTy::CLOSE_BRACE)) break;
  }

  ctx.m_sym->exit_scope();

  return def_entity;
}

void parser::Parser_Declaration_COP::parse_entity_declaration(
    std::shared_ptr<ast::declaration::cop::Entity> parent_entity)
{
  static const std::string hint =
      "define comp usage like:"
      "\n  - `use name { field1: val1, field2: val2 }`"
      "\n  - `use name`";
  static const std::string new_hint = "define new entity def like: `new(params) { ... }`";

  if (ctx.tok_v.match(TokTy::USE)) {
    std::unique_ptr<ast::AIdentifier> comp_id;

    auto comp_name = ctx.p_expr->identifier();

    if (auto ty = ctx.p_expr->identifier_typed()) {
      ty->name = std::move(comp_name);
      comp_id  = std::move(ty);
    } else {
      comp_id = std::move(comp_name);
    }
    auto comp = ctx.p_lit->literal_component(std::move(comp_id));

    if (auto ptr = dynamic_cast<ast::literal::Component*>(comp.get())) {
      // Transfert ownership directement en downcast
      parent_entity->comps.push_back(
          std::unique_ptr<ast::literal::Component>(static_cast<ast::literal::Component*>(comp.release())));
    } else {
      ctx.tok_v.add_error(19, "Expected Literal component after 'use' instruction", hint);
    }

    return;
  } else if (ctx.tok_v.match(TokTy::NEW)) {
    ctx.m_sym->enter_scope("new", EScopeType::Entity_New);
    auto new_fn_type = ctx.p_type->explicit_function_proto();
    ctx.tok_v.expect(20, TokTy::OPEN_BRACE, "Expected start code '{'.", new_hint);

    auto cb = ctx.p_loc->code_block_instruction();

    ctx.m_sym->exit_scope();
    parent_entity->constructors.push_back({std::move(new_fn_type), std::move(cb)});
    return;
  } else if (ctx.tok_v.match(TokTy::OP)) {
    auto op = _entity_op(parent_entity);
    parent_entity->operators.push_back(std::move(op));
    return;
  } else if (ctx.tok_v.match(TokTy::CAST)) {
    auto cast = _entity_cast(parent_entity);
    parent_entity->casts.push_back(std::move(cast));
    return;
  }

  ctx.tok_v.add_error(21, "Unexpected '" + ctx.tok_v.peek().val + "' keyword not allowed in entity code block.",
                      "you can define in functions: atribute, method, typealias, operator "
                      "overloading, trait implementation.");
}

std::shared_ptr<ast::declaration::cop::Entity_Op>
parser::Parser_Declaration_COP::_entity_op(std::shared_ptr<ast::declaration::cop::Entity> parent_entity)
{
  static const std::string hint = "define entity operator overloading like `op + { ... }`.";
  static const std::string hint_index =
      "define entity index overloading like:"
      "\n  - index `op [a] -> T {...}`"
      "\n  - range `op [r..] -> Slice<T> {...}`.";
  auto tok = ctx.tok_v.peek();

  ctx.m_sym->enter_scope("op", EScopeType::Entity_Op);

  std::shared_ptr<ast::declaration::cop::Entity_Op> entity_op;

  // if index operator case op [] -> T { ... }
  if (ctx.tok_v.match(TokTy::OPEN_SQUARE)) {
    auto _op_index            = ctx.Create_Decl<ast::declaration::cop::Entity_OpIndex>(tok);
    _op_index->operatorType   = EBinOpType::Index;
    _op_index->parameter_name = ctx.parse_name("Expected index name binding", hint_index);

    if (ctx.tok_v.peek().type == TokTy::DOT && ctx.tok_v.peek(1).type == TokTy::DOT) {
      _op_index->operatorType = EBinOpType::Slice;
    }

    ctx.tok_v.expect(23, TokTy::CLOSE_SQUARE, "Expected closed index operator ']'", hint_index);
    ctx.tok_v.expect(24, TokTy::ARROW, "Expected explicit return type '-> T'", hint_index);

    // return type expected for index operator
    ctx.tok_v.expect(25, TokTy::ARROW, "Expected return definition '-> T' after index operator '[]' overload.",
                     hint_index);

    _op_index->return_type = ctx.p_type->parse_type();

    if (_op_index->operatorType == EBinOpType::Slice) {
      // ctx.parse_name("", hint_index);
    }

    entity_op = _op_index;
  }
  // other operator case op + - / * ...
  else {
    auto _op    = ctx.Create_Decl<ast::declaration::cop::Entity_Op>(tok);
    auto op_tok = ctx.tok_v.expect_any(27, kOperatorTokens, "Expected operator in entity operator overloading.", hint);
    _op->operatorType = TokTy_to_EBinOpType(op_tok.type);

    if (ctx.tok_v.check(TokTy::ARROW))
      ctx.tok_v.add_error(28,
                          "Unexpected retrun type definition '-> T' after a entity operator '"
                              + EBinOpType_to_str(_op->operatorType) + "'.",
                          hint);

    entity_op = _op;
  }

  ctx.tok_v.expect(29, TokTy::OPEN_BRACE, "Expected start code block '{' after entity operator overloading.", hint);

  entity_op->parent_entity = parent_entity;

  entity_op->codeblock = ctx.p_loc->code_block_instruction();

  ctx.m_sym->exit_scope();

  return entity_op;
}

std::shared_ptr<ast::declaration::cop::Entity_Cast>
parser::Parser_Declaration_COP::_entity_cast(std::shared_ptr<ast::declaration::cop::Entity> parent_entity)
{
  static const std::string hint =
      "define entity cast overloading like:"
      "\n  - `cast self as T { ... }`."
      "\n  - `cast T as self { ... }`.";

  auto cast = ctx.Create_Decl<ast::declaration::cop::Entity_Cast>(ctx.tok_v.peek());

  ctx.m_sym->enter_scope("cast", EScopeType::Entity_Cast);

  auto key_self_case = [&]() {
    auto self             = ctx.Create_Node<ast::expression::Self>(ctx.tok_v.peek());
    self->self_definition = parent_entity;

    return self;
  };

  auto key_other_case = [&]() {
    auto type = ctx.p_type->parse_type();
    return type;
  };

  if (ctx.tok_v.match(TokTy::SELF)) {
    auto self          = key_self_case();
    cast->source       = std::move(self);
    cast->isSourceSelf = true;
  } else {
    auto id      = key_other_case();
    cast->target = std::move(id);
  }

  ctx.tok_v.expect(30, TokTy::AS, "Expected cast linker 'as' after casting source.", hint);

  if (ctx.tok_v.match(TokTy::SELF)) {
    if (cast->isSourceSelf) {
      ctx.tok_v.add_error(31,
                          "Expected other type than 'self' in target cast after 'self' in "
                          "source cast, cast can't be with himself.",
                          hint);
      return nullptr;
    }

    auto self    = key_self_case();
    cast->source = std::move(self);
  } else {
    if (!cast->isSourceSelf) {
      ctx.tok_v.add_error(32,"Expected 'self' in target cast after '" + cast->source->debug_str() +
                                  "' in source cast.\n  Cast can't be extern "
                                  "to the concerned entity.",
                              hint);
      return nullptr;
    }

    auto id      = key_other_case();
    cast->target = std::move(id);
  }

  ctx.tok_v.expect(33, TokTy::OPEN_BRACE, "Expected start code block '{' after casting definition.", hint);

  cast->codeblock = ctx.p_loc->code_block_instruction();

  ctx.m_sym->exit_scope();
  return cast;
}

std::shared_ptr<ast::declaration::cop::System> parser::Parser_Declaration_COP::system()
{
  static const std::string hint =
      "define system like:"
      "\n  - single behaviour"
      "\n   `sys Move() {"
      "\n      Position(pos) + Velocity(vel) => update_physic(pos, vel)"
      "\n  - multiple behaviour"
      "\n   `sys Speak(ref msg: str) {"
      "\n      other => print(\"say \")"
      "\n      ID(id) => {"
      "\n        print(\"(\" + id.name + \"): \" + msg)"
      "\n        return"
      "\n      }"
      "\n      other => print(msg)"
      "\n    }`";

  ctx.tok_v.match(TokTy::SYSTEM);

  auto system = ctx.Create_Decl<ast::declaration::cop::System>(ctx.tok_v.peek());

  // not handled if (auto where = ctx.p_meta->metacode_where()) system->generic = where.value();

  system->name      = ctx.parse_name("", hint);
  system->prototype = ctx.p_type->explicit_function_proto();
  for (auto& param : system->prototype->parameters) param->parent_function = system;

  auto sys_sym      = ctx.m_sym->add_decl(system);
  bool isNoCompUsed = true;

  ctx.tok_v.expect(34, TokTy::OPEN_BRACE, "Expected start code block '{' after system declaration.", hint);

  while (!ctx.tok_v.is_end()) {
    ctx.tok_v.expect(35, TokTy::WITH, "Expected behaviour block 'with' in system code block.", hint);

    auto sys_case           = _system_case();
    sys_case->parent_system = system;
    if (!sys_case->bindings.empty()) isNoCompUsed = false;
    system->cases.push_back(sys_case);

    if (ctx.tok_v.check(TokTy::WITH)) continue;
    if (ctx.tok_v.match(TokTy::CLOSE_BRACE)) break;
    ctx.tok_v.add_error(36, "Expected behaviour block 'with' in system code block.", hint);
  }

  // pre semantic checking system form is useful or a function is prefered ? (case of no components specifed)
  if (isNoCompUsed) {
    ctx.tok_v.add_error_tok(37, system->_token,
                            "Expected function instead of system given the behaviour of the code "
                            "block: no component specified in any where statement.",
                            hint);
  }

  return system;
}

std::shared_ptr<ast::declaration::cop::System_Case> parser::Parser_Declaration_COP::_system_case()
{
  static const std::string hint =
      "define system case like:"
      "\n  - no component (for universal/last/default behaviour)"
      "\n   `_ => { Println(\"NPC don't have any item!\") }`"
      "\n  - single component (for single operation most of the time)"
      "\n   `CCounter(c) => add_count(c)`"
      "\n  - multiple components (for interaction between components most of the time)"
      "\n   `CPosition(pos) + CVelocity(vel) => update_move(pos, vel)";

  auto sys_case = ctx.Create_Decl<ast::declaration::cop::System_Case>(ctx.tok_v.peek());
  ctx.tok_v.match(TokTy::WITH);
  sys_case->isDefault = ctx.tok_v.match(TokTy::UNDERSCORE);

  if (!sys_case->isDefault) {
    while (!ctx.tok_v.is_end()) {
      ctx.tok_v.expect(38, TokTy::OPEN_PAREN, "Expected start binding '(' after component name pattern.", hint);

      auto bind  = ctx.Create_Decl<ast::declaration::local::Variable_Binding>(ctx.tok_v.peek());
      bind->name = ctx.parse_name("", hint);
      ctx.m_sym->add_decl(bind);
      sys_case->bindings.push_back(bind);

      ctx.tok_v.expect(39, TokTy::OPEN_PAREN, "Expected end binding ')' after component name pattern.", hint);

      if (ctx.match_field_separator(TokTy::OP_PLUS, TokTy::OPEN_BRACE)) break;
    }
  }

  sys_case->codeblock = ctx.p_loc->code_block_instruction();

  for (auto& instruction : sys_case->codeblock->elements) {
    if (dynamic_cast<ast::statement::Return*>(instruction.node())) {
      sys_case->isReturn = true;
      break;
    }
  }

  return sys_case;
}
