
#include "parser_declaration_global.hpp"

#include <string>
#include <string_view>
#include <utility>
#include <iostream>

#include "ast/ast_declaration_global.hpp"
#include "compiler/compiler.hpp"
#include "nexus/ast/ast.hpp"
#include "nexus/forward.hpp"
#include "nexus/lexer/token.hpp"
#include "nexus/type.hpp"
#include "nexus/script.hpp"
#include "nexus/symbol.hpp"
#include "nexus/metacode/metacode.hpp"

#include "parser_context.hpp"
#include "parser_type.hpp"

#include "ast/ast_declaration_local.hpp"
#include "ast/ast_declaration_cop.hpp"
#include "ast/ast_generic.hpp"

#include "ast/ast_declaration_global.hpp"
#include "parser_expression.hpp"
#include "parser_type.hpp"
#include "parser_declaration_local.hpp"
#include "parser_declaration_cop.hpp"
#include "parser_base.hpp"

#include "nexus/lexer/token.hpp"

#include "nexus/metacode/metacode.hpp"
#include "nexus/symbol.hpp"

ast::_gnid parser::Parser_Declaration::parse_declaration()
{
  auto tok = p.peek();

  switch (tok.kind) {
  case token::ETokenKind::MOD:       return _module();
  case token::ETokenKind::UNION:     return _union();
  case token::ETokenKind::FLAG:      return flag();
  case token::ETokenKind::ENUM:      return enumeration();
  case token::ETokenKind::VAR:
  case token::ETokenKind::LET:
  case token::ETokenKind::CONST:     return global_variable();
  case token::ETokenKind::FUNCTION:  return function();
  case token::ETokenKind::GENERIC:   return generic();
  case token::ETokenKind::TYPE:      return type_alias();
  case token::ETokenKind::COMPONENT: return p.p_cop->component();
  case token::ETokenKind::SYSTEM:    return p.p_cop->system();
  case token::ETokenKind::ENTITY:    return p.p_cop->entity();
  case token::ETokenKind::EXPORT:    return p.p_base->parse_export();
  case token::ETokenKind::EXTERN:    return p.p_base->parse_extern();
  case token::ETokenKind::IMPORT:    return p.p_base->parse_import();
  default:                           break;
  }

  p.add_error(60, "Illegal instruction '" + std::string(p.tok_to_str(tok.id)) + "' in global.",
              "you can define in global: namespace, variable, function, entity, component, system");

  p.next();

  return BAD_NODE_ID;
}

ast::_gnid parser::Parser_Declaration::_module()
{
  constexpr std::string_view hint =
      R"(define module like:"
  - module `mod myName { ... }`"
  - module alias `mod Vec = core::container::vector`"
  - module to root `mod root = core::container::vector`)";

  p.match(token::ETokenKind::MOD);

  auto name = p.parse_name("", hint);

  if (p.match(token::ETokenKind::ASSIGN)) {
    parser_add_node(node, Global_Alias_Module, p.peek().id);
    node->alias = name;
    node->regex = p.p_base->identifier();


    p.add_symbol(node->node_id.get_node_id());

    return node->node_id;
  } else if (p.match(token::ETokenKind::OPEN_BRACE)) {
    parser_add_node(node, Global_Module, p.peek(-2).id);

    p.enter_scope(*node, name);
    node->name = name;

    node->codeblock = p.p_loc->parse_codeblock();

    p.exit_scope();

    return node->node_id;
  }

  p.add_error_tok(61, p.peek(), "Expected '{' or '=' after module name.", hint);

  return BAD_NODE_ID;
}

ast::_gnid parser::Parser_Declaration::enumeration()
{
  constexpr std::string_view hint =
      "define enum like:"
      "\n  - typed enum `enum name { field_name1(T), field_name2 }"
      "\n  - typed enum `enum Option { Valid(T), Invalid }";
  p.match(token::ETokenKind::ENUM);

  parser_add_node(enu, Global_Enum, p.peek().id);
  enu->name   = p.parse_name("", hint);
  auto sym_id = p.add_symbol(enu->node_id.get_node_id());
  p.enter_scope(*enu, "enum " + std::string(enu->name));

  p.expect(63, token::ETokenKind::OPEN_BRACE, "Expected start enum block '{' after enum name declaration.", hint);
  std::vector<std::vector<type::_id>> factory_variants;

  size_t count = 0;
  while (!p.is_end()) {
    ast::Global_Enum::Enum_Field field;
    field.name     = p.parse_name("", hint);
    field.position = count++;
    factory_variants.push_back({});

    if (p.match(token::ETokenKind::OPEN_PAREN)) {
      while (!p.is_end()) {
        auto ty = p.p_type->parse_type();
        field.types.push_back(ty);
        factory_variants.back().push_back(ty);

        if (p.match_field_separator(token::ETokenKind::COMMA, token::ETokenKind::CLOSE_PAREN)) break;
      }
    }

    enu->variants.push_back(field);
    if (p.match_field_separator(token::ETokenKind::COMMA, token::ETokenKind::CLOSE_BRACE)) break;
  }

  p.exit_scope();

  parser_type_factory.make_enum(factory_variants, sym_id);

  return enu->node_id;
}

ast::_gnid parser::Parser_Declaration::_union()
{
  constexpr std::string_view hint = "define union like: `union name { field_name1: T, field_name2: U, ... }";
  p.match(token::ETokenKind::UNION);

  parser_add_node(_union, Global_Union, p.peek().id);
  _union->name = p.parse_name("", hint);
  auto sym_id  = p.add_symbol(_union->node_id.get_node_id());
  p.enter_scope(*_union, "union " + std::string(_union->name));

  p.expect(182, token::ETokenKind::OPEN_BRACE, "Expected start union block '{' after union name declaration.", hint);

  std::vector<type::_id> factory_types;

  while (!p.is_end()) {
    auto field_name = p.parse_name("", hint);

    p.expect(183, token::ETokenKind::COLON, "Expected colon ':' after union field name declaration.", hint);

    auto field_ty = p.p_type->parse_type();

    _union->fields.emplace_back(field_name, field_ty);
    factory_types.push_back(field_ty);

    if (p.match_field_separator(token::ETokenKind::COMMA, token::ETokenKind::CLOSE_BRACE)) break;
  }

  p.exit_scope();

  parser_type_factory.make_union(factory_types, sym_id);

  return _union->node_id;
}

ast::_gnid parser::Parser_Declaration::flag()
{
  constexpr std::string_view hint = "define flag like: `flag name { flag1, flag2, ... }";
  p.match(token::ETokenKind::FLAG);

  parser_add_node(flag, Global_Flag, p.peek().id);
  flag->name = p.parse_name("", hint);

  auto sym_id = p.add_symbol(flag->node_id.get_node_id());
  p.enter_scope(*flag, "flag " + std::string(flag->name));

  p.expect(184, token::ETokenKind::OPEN_BRACE, "Expected start flag block '{' after flag name declaration.", hint);

  while (!p.is_end()) {
    auto field_name = p.parse_name("", hint);

    flag->fields.push_back(field_name);

    if (p.match_field_separator(token::ETokenKind::COMMA, token::ETokenKind::CLOSE_BRACE)) break;
  }

  p.exit_scope();

  parser_type_factory.make_flag(flag->fields.size(), sym_id);

  return flag->node_id;
}

ast::_gnid parser::Parser_Declaration::global_variable()
{
  constexpr std::string_view hint =
      R"(define global variable like:"
  - mutable : `var name: type = expression` `var name = expression`"
  - immutable : `let name: type = expression` `let name = expression`"
  - constant (compiletime value): `const name: type = expression` `const name = expression`"
  - external mutable :"
   `# extern"
    var name: type`"
  - external immutable :"
   `# extern"
    let name: type)";

  auto               tok  = p.expect_any(65, {token::ETokenKind::LET, token::ETokenKind::VAR, token::ETokenKind::CONST},
                                         "Expected global variable declaration token", hint);
  ast::EVariableKind kind = ast::ETokenKind_to_EVariableKind(tok.kind);

  parser_add_node(var, Global_Variable, p.peek().id);
  var->kind = kind;
  var->name = p.parse_name("", hint);

  p.add_symbol(var->node_id.get_node_id());

  if (var->name.empty()) {
    p.add_error(66, "Invalid Identifier !", "");
  }

  bool is_inferred_type = false;

  // explicit type case
  if (p.match(token::ETokenKind::COLON)) var->type = p.p_type->parse_type();
  // auto deduce type case
  else
    is_inferred_type = true;

  // check affectation
  auto assign_tok = p.next();
  var->assignment = ast::ETokenKind_to_ETransfertType(assign_tok.kind);

  if (var->assignment == ast::ETransfertType::NONE && is_inferred_type)
    p.add_error(68, "Expected assignation '=' in auto inferred variable type.",
                "define auto inferred variable like `let myName = expression;`");

  auto expr       = p.p_expr->parse_expression();
  var->expression = expr;
  return var->node_id;
}

ast::_gnid parser::Parser_Declaration::function()
{
  constexpr std::string_view hint =
      R"(define function like:"
  - definition `fn myName() { ... }`"
  - definition with return `fn myName() -> i32 { ... }`."
  - extern declaration\n   `# extern"
    fn myName();`."
  - extern declaration with return"
   `# extern"
    fn myName() -> i32;`.)";

  p.match(token::ETokenKind::FUNCTION);

  auto tok_pos = p.tok_to_pos(p.peek().id);

  parser_add_node(fn, Global_Function, p.peek().id);
  fn->name     = p.parse_name("", hint);
  fn->is_const = p.metablock_contains(tok_pos, "const");
  fn->is_pure  = p.metablock_contains(tok_pos, "pure");

  if (!extern_abi.empty()) fn->extern_abi = extern_abi;

  if (auto pattern = p.get_instruction(tok_pos, {"extern", pattern_constants::wildcard})) {
    fn->extern_abi = std::string(pattern->at_str(1, 0));
  }

  p.add_symbol(fn->node_id.get_node_id());
  p.enter_scope(*fn, "function " + std::string(fn->name));

  auto proto = p.p_type->parse_and_mount_local_callable(fn->prototype, fn->is_explicit_ret_type);

  // if extern : no definition
  if (!fn->extern_abi.empty() && p.check(token::ETokenKind::OPEN_BRACE))
    p.add_error(69, "Unexpected start code block '{' after a extern function declaration", hint);

  if (fn->extern_abi.empty()) fn->codeblock = p.p_loc->parse_codeblock();

  p.exit_scope();

  return fn->node_id;
}

ast::_gnid parser::Parser_Declaration::generic()
{
  constexpr std::string_view kHint_gen = "define geneneric like `gen name<T, ...> { ... }`.";
  constexpr std::string_view kHint_filter =
      R"(define geneneric filter like:"
  - alone operator `T use op +;`"
  - role `T use role Printable;`"
  - system `T use sys Move;`"
  - component `T use comp Position;`"
  - typealias `T use type len;`"
  - nested filter `T is gen::base_of<Animal>;`)";

  parser_add_node(gen, Global_Generic, p.peek().id);
  p.match(token::ETokenKind::GENERIC);

  gen->name = p.parse_name("", kHint_gen);
  p.add_symbol(gen->node_id.get_node_id());
  p.enter_scope(*gen, "generic " + std::string(gen->name));
  p.expect(70, token::ETokenKind::OPEN_BRACKETS, "Expected start type '<' after generic name.", kHint_gen);

  while (!p.is_end()) {
    gen->typenames.push_back(p.parse_name("", kHint_gen));

    if (p.match_field_separator(token::ETokenKind::S_END_OF_FILE, token::ETokenKind::CLOSE_BRACKETS)) break;
  }

  p.expect(72, token::ETokenKind::OPEN_BRACE, "Expected start code block '{' after generic declaration.", kHint_gen);

  while (!p.is_end()) {
    if (p.match(token::ETokenKind::CLOSE_BRACE)) break;
    auto firstok = p.parse_name("", kHint_filter);

    // case: T op ...
    if (p.match(token::ETokenKind::OP)) {
      parser_add_node(gen_op, Generic_Have_Op, p.peek().id);
      gen_op->target_gen_sym = firstok;
      auto tok_op            = p.expect_any(74, token::k_operator,
                                            "Expected operator after 'op' keyword in generic filter argument.", kHint_filter);
      gen_op->op_ty          = ast::ETokenKind_to_EBinOpType(tok_op.kind);

      gen->gen_conds.push_back(gen_op->node_id);
      p.match(token::ETokenKind::SEMICOLON);
    }
    // case: T comp ...
    else if (p.match(token::ETokenKind::COMPONENT)) {
      parser_add_node(comp, Generic_Use_Component, p.peek().id);
      comp->target_gen_sym = firstok;
      comp->component      = p.p_expr->parse_expression();

      gen->gen_conds.push_back(comp->node_id);
    }
    // case: T role ...
    else if (p.match(token::ETokenKind::ROLE)) {
      parser_add_node(role, Generic_Have_Role, p.peek().id);
      role->target_gen_sym = firstok;
      role->role           = p.p_expr->parse_expression();

      gen->gen_conds.push_back(role->node_id);
    }
    // case: T sys ...
    else if (p.match(token::ETokenKind::SYSTEM)) {
      parser_add_node(sys, Generic_Compatible_System, p.peek().id);
      sys->target_gen_sym = firstok;
      sys->system         = p.p_expr->parse_expression();

      gen->gen_conds.push_back(sys->node_id);
    }
    // case: T is i32 | type::floating | ...
    else if (p.match(token::ETokenKind::IS)) {
      parser_add_node(nested, Generic_Is_Type, p.peek().id);
      nested->source_typename = firstok;

      while (!p.is_end()) {
        nested->in_type.push_back(p.p_type->parse_type());

        if (p.match(token::ETokenKind::PIPE)) continue;
        break;
      }

      gen->gen_conds.push_back(nested->node_id);
    }
    // case: T cast to/from ...
    else if (p.match(token::ETokenKind::CAST)) {
      parser_add_node(castNode, Generic_Can_Cast, p.peek().id);
      castNode->source_typename = firstok;

      if (!p.check_val("to") && !p.check_val("from")) {
        p.add_error(75, "Expected cast way 'to' or 'from' in generic filter argument.", kHint_filter);
      }

      castNode->target = p.p_type->parse_type();

      gen->gen_conds.push_back(castNode->node_id);
    } else
      p.add_error(76, "Expected generic condition 'use' or 'is' in generic filter argument.", kHint_filter);

    p.match(token::ETokenKind::SEMICOLON);
  }

  p.exit_scope();
  return gen->node_id;
}

ast::_gnid parser::Parser_Declaration::type_alias()
{
  constexpr std::string_view hint =
      R"(define type alias like:
  - type `type myAlias = i32`.
  - generic `type Vec<T> = core::container::vector<T>`.
  - generic `type StrList = core::container::vector<str>`.)";

  parser_add_node(type_alias, Global_Alias_Type, p.peek().id);

  p.match(token::ETokenKind::TYPE);

  type_alias->alias = p.parse_name();

  p.expect(77, token::ETokenKind::ASSIGN, "Expected '=' after type alias.", hint);

  type_alias->type = p.p_type->parse_type();

  p.add_symbol(type_alias->node_id.get_node_id());
  return type_alias->node_id;
}
