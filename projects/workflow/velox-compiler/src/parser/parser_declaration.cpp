
#include "parser_declaration.hpp"

#include <utility>
#include <iostream>

#include "ast/ast_data.hpp"
#include "ast/ast_declaration.hpp"
#include "ast/ast_declaration_local.hpp"
#include "ast/ast_declaration_cop.hpp"
#include "ast/ast_generic.hpp"

#include "ast/ast_type.hpp"
#include "parser_context.hpp"
#include "parser_expression.hpp"
#include "parser_type.hpp"
#include "parser_declaration_local.hpp"
#include "parser_declaration_cop.hpp"
#include "parser_base.hpp"

#include "lexer/token.hpp"

#include "misc/metacode.hpp"
#include "misc/symbol_manager.hpp"

std::shared_ptr<ast::ADeclaration> parser::Parser_Declaration::parse_declaration()
{
  auto tok = ctx.tok_v.peek();
  switch (tok.type) {
  case TokTy::MOD:       return _module();
  case TokTy::UNION:     return _union();
  case TokTy::FLAG:      return flag();
  case TokTy::ENUM:      return enumeration();
  case TokTy::VAR:
  case TokTy::LET:
  case TokTy::CONST:     return global_variable();
  case TokTy::FUNCTION:  return function();
  case TokTy::GENERIC:   return generic();
  case TokTy::TYPE:      return type_alias();
  case TokTy::COMPONENT: return std::static_pointer_cast<ast::ADeclaration>(ctx.p_cop->component());
  case TokTy::SYSTEM:    return std::static_pointer_cast<ast::ADeclaration>(ctx.p_cop->system());
  case TokTy::ENTITY:    return std::static_pointer_cast<ast::ADeclaration>(ctx.p_cop->entity());
  case TokTy::EXPORT:    return ctx.p_base->parse_export();
  case TokTy::EXTERN:    return ctx.p_base->parse_extern();
  case TokTy::IMPORT:    {
    auto ignore = ctx.p_base->parse_import();
    return nullptr;
  }
  default: break;
  }

  ctx.tok_v.add_error(60, "Illegal instruction '" + tok.val + "' in global.",
                      "you can define in global: namespace, variable, function, entity, component, system");

  return nullptr;
}

std::shared_ptr<ast::ADeclaration> parser::Parser_Declaration::_module()
{
  static const std::string hint =
      "define module like:"
      "\n  - module `mod myName { ... }`"
      "\n  - module alias `mod Vec = core::container::vector`"
      "\n  - module to root `mod root = core::container::vector`";

  ctx.tok_v.match(TokTy::MOD);

  auto name = ctx.parse_name("", hint);

  if (ctx.tok_v.match(TokTy::ASSIGN)) {
    auto node              = ctx.Create_Decl<ast::declaration::Mod_Alias>(ctx.tok_v.peek());
    node->declaration_name = std::move(name);
    node->module           = ctx.p_expr->identifier();

    ctx.current_module->add_item(node);

    return node;
  } else if (ctx.tok_v.match(TokTy::OPEN_BRACE)) {
    auto node = ctx.Create_Decl<ast::declaration::Mod>(ctx.tok_v.peek());

    ctx.enter_scope(node, name);
    node->declaration_name = std::move(name);

    while (!ctx.tok_v.is_end()) {
      if (ctx.tok_v.check_any({TokTy::IMPORT, TokTy::EXPORT})) {
        ctx.tok_v.add_error_tok(62, ctx.tok_v.peek(), "Illegal nested module export/import instruction.", hint);
      }

      node->declarations.push_back(ctx.p_decl->parse_declaration());

      if (ctx.match_field_separator(TokTy::S_END_OF_FILE, TokTy::CLOSE_BRACE)) break;
    }

    ctx.exit_scope();

    return node;
  }

  ctx.tok_v.add_error_tok(61, ctx.tok_v.peek(), "Expected '{' or '=' after module name.", hint);

  return nullptr;
}

std::shared_ptr<ast::declaration::Enum> parser::Parser_Declaration::enumeration()
{
  static const std::string hint =
      "define enum like:"
      "\n  - typed enum `enum name { field_name1(T), field_name2 }"
      "\n  - typed enum `enum Option { Valid(T), Invalid }";
  ctx.tok_v.match(TokTy::ENUM);

  auto enu              = ctx.Create_Decl<ast::declaration::Enum>(ctx.tok_v.peek());
  enu->declaration_name = ctx.parse_name("", hint);
  ctx.current_module->add_item(enu);
  ctx.enter_scope(enu, enu->declaration_name);

  ctx.tok_v.expect(63, TokTy::OPEN_BRACE, "Expected start enum block '{' after enum name declaration.", hint);

  size_t count = 0;
  while (!ctx.tok_v.is_end()) {
    auto elem      = ctx.Create_Node<ast::declaration::Enum_Element>(ctx.tok_v.peek());
    elem->name     = ctx.parse_name("", hint);
    elem->position = count++;

    if (ctx.tok_v.match(TokTy::OPEN_PAREN)) {
      while (!ctx.tok_v.is_end()) {
        elem->types.push_back(ctx.p_type->parse_type());

        if (ctx.match_field_separator(TokTy::COMMA, TokTy::CLOSE_PAREN)) break;
      }
    }

    elem->parent_enum = enu;
    enu->variants.push_back(std::move(elem));
    if (ctx.match_field_separator(TokTy::COMMA, TokTy::CLOSE_BRACE)) break;
  }

  ctx.exit_scope();

  return enu;
}

std::shared_ptr<ast::declaration::Union> parser::Parser_Declaration::_union()
{
  static const std::string hint = "define union like: `union name { field_name1: T, field_name2: U, ... }";
  ctx.tok_v.match(TokTy::UNION);

  auto _union              = ctx.Create_Decl<ast::declaration::Union>(ctx.tok_v.peek());
  _union->declaration_name = ctx.parse_name("", hint);
  ctx.current_module->add_item(_union);
  ctx.enter_scope(_union, _union->declaration_name);

  ctx.tok_v.expect(182, TokTy::OPEN_BRACE, "Expected start union block '{' after union name declaration.", hint);

  while (!ctx.tok_v.is_end()) {
    auto field_name = ctx.parse_name("", hint);

    ctx.tok_v.expect(183, TokTy::COLON, "Expected colon ':' after union field name declaration.", hint);

    auto filed_ty = ctx.p_type->parse_type();

    _union->fields.emplace_back(field_name, std::move(filed_ty));

    if (ctx.match_field_separator(TokTy::COMMA, TokTy::CLOSE_BRACE)) break;
  }

  ctx.exit_scope();

  return _union;
}

std::shared_ptr<ast::declaration::Flag> parser::Parser_Declaration::flag()
{
  static const std::string hint = "define flag like: `flag name { flag1, flag2, ... }";
  ctx.tok_v.match(TokTy::FLAG);

  auto flag              = ctx.Create_Decl<ast::declaration::Flag>(ctx.tok_v.peek());
  flag->declaration_name = ctx.parse_name("", hint);

  ctx.current_module->add_item(flag);
  ctx.enter_scope(flag, flag->declaration_name);

  ctx.tok_v.expect(184, TokTy::OPEN_BRACE, "Expected start flag block '{' after flag name declaration.", hint);

  while (!ctx.tok_v.is_end()) {
    auto field_name = ctx.parse_name("", hint);

    flag->fields.push_back(field_name);

    if (ctx.match_field_separator(TokTy::COMMA, TokTy::CLOSE_BRACE)) break;
  }

  ctx.exit_scope();

  return flag;
}

std::shared_ptr<ast::declaration::Global> parser::Parser_Declaration::global_variable()
{
  static const std::string hint =
      "define global variable like:"
      "\n  - mutable : `var name: type = expression` `var name = expression`"
      "\n  - immutable : `let name: type = expression` `let name = expression`"
      "\n  - constant (compiletime value): `const name: type = expression` `const name = expression`"
      "\n  - external mutable :"
      "\n   `# extern"
      "\n    var name: type`"
      "\n  - external immutable :"
      "\n   `# extern"
      "\n    let name: type";

  auto          kind_tok = ctx.tok_v.expect_any(65, {TokTy::LET, TokTy::VAR, TokTy::CONST},
                                                "Expected global variable declaration token", hint);
  EVariableKind kind     = TokTy_to_EVariableKind(kind_tok.type);

  auto var              = ctx.Create_Decl<ast::declaration::Global>(ctx.tok_v.peek());
  var->kind             = kind;
  var->declaration_name = ctx.parse_name("", hint);

  ctx.current_module->add_item(var);

  if (var->declaration_name.empty()) {
    ctx.tok_v.add_error(66, "Invalid Identifier !", "");
  }

  // type definition no expression
  if (var->declaration_is_external) {
    ctx.tok_v.expect(67, TokTy::COLON, "Expected type definition for an global variable marked external.", hint);
    var->type = ctx.p_type->parse_type();
    ctx.tok_v.match(TokTy::SEMICOLON);
    return var;
  }

  bool isAutoTy = false;

  // explicit type case
  if (ctx.tok_v.match(TokTy::COLON)) var->type = ctx.p_type->parse_type();
  // auto deduce type case
  else
    isAutoTy = true;

  // check affectation
  Token assign_tok = ctx.tok_v.next();
  var->assignment  = TokTy_to_ETransfertType(assign_tok.type);

  if (var->assignment == ETransfertType::NONE && isAutoTy)
    ctx.tok_v.add_error(68, "Expected assignation '=' in auto inferred variable type.",
                        "define auto inferred variable like `let myName = expression;`");

  auto expr       = ctx.p_expr->parse_expression();
  var->expression = std::move(expr);
  return var;
}

std::shared_ptr<ast::declaration::Function> parser::Parser_Declaration::function()
{
  static const std::string hint =
      "define function like:"
      "\n  - definition `fn myName() { ... }`"
      "\n  - definition with return `fn myName() -> i32 { ... }`."
      "\n  - extern declaration\n   `# extern"
      "\n    fn myName();`."
      "\n  - extern declaration with return"
      "\n   `# extern"
      "\n    fn myName() -> i32;`.";

  ctx.tok_v.match(TokTy::FUNCTION);

  auto fn = ctx.Create_Decl<ast::declaration::Function>(ctx.tok_v.peek());

  fn->declaration_name = ctx.parse_name("", hint);
  fn->is_const         = ctx.metablock_contains(*fn, "const");
  fn->is_pure          = ctx.metablock_contains(*fn, "pure");

  if (auto pattern = ctx.get_instruct(*fn, {"extern", "<*>"})) {
    fn->extern_call_convention = pattern->at_str(1, 0);
  }

  ctx.current_function = fn.get();

  ctx.current_module->add_item(fn);
  ctx.enter_scope(fn, fn->declaration_name);

  fn->prototype = ctx.p_type->explicit_function_proto(false);
  for (auto& param : fn->prototype->parameters) {
    param->parent_function = fn;
    ctx.current_module->add_item(param);
  }

  // if extern : no definition
  if (fn->declaration_is_external && ctx.tok_v.check(TokTy::OPEN_BRACE))
    ctx.tok_v.add_error(69, "Unexpected start code block '{' after a extern function declaration", hint);

  if (!fn->declaration_is_external) fn->codeblock = ctx.p_loc->code_block_instruction();

  ctx.exit_scope();
  ctx.current_function = nullptr;

  return fn;
}

std::shared_ptr<ast::declaration::Generic> parser::Parser_Declaration::generic()
{
  static const std::string kHint_gen = "define geneneric like `gen name<T, ...> { ... }`.";
  static const std::string kHint_filter =
      "define geneneric filter like:"
      "\n  - alone operator `T use op +;`"
      "\n  - role `T use role Printable;`"
      "\n  - system `T use sys Move;`"
      "\n  - component `T use comp Position;`"
      "\n  - typealias `T use type len;`"
      "\n  - nested filter `T is gen::base_of<Animal>;`";

  auto gen = ctx.Create_Decl<ast::declaration::Generic>(ctx.tok_v.peek());
  ctx.tok_v.match(TokTy::GENERIC);

  gen->declaration_name = ctx.parse_name("", kHint_gen);
  ctx.current_module->add_item(gen);
  ctx.enter_scope(gen, gen->declaration_name);
  ctx.tok_v.expect(70, TokTy::OPEN_BRACKETS, "Expected start type '<' after generic name.", kHint_gen);

  while (!ctx.tok_v.is_end()) {
    gen->target_gen_sym.insert(ctx.parse_name("", kHint_gen));

    if (ctx.match_field_separator(TokTy::S_END_OF_FILE, TokTy::CLOSE_BRACKETS)) break;
  }

  ctx.tok_v.expect(72, TokTy::OPEN_BRACE, "Expected start code block '{' after generic declaration.", kHint_gen);

  while (!ctx.tok_v.is_end()) {
    if (ctx.tok_v.match(TokTy::CLOSE_BRACE)) break;
    std::string firstok = ctx.parse_name("", kHint_filter);

    // case: T op ...
    if (ctx.tok_v.match(TokTy::OP)) {
      auto gen_op            = ctx.Create_Node<ast::generic::Have_Op>(ctx.tok_v.peek());
      gen_op->target_gen_sym = firstok;
      auto tok_op            = ctx.tok_v.expect_any(
          74, k_operator, "Expected operator after 'op' keyword in generic filter argument.", kHint_filter);
      gen_op->op_ty          = TokTy_to_EBinOpType(tok_op.type);
      gen_op->parent_generic = gen;

      gen->conditions.push_back(std::move(gen_op));
      ctx.tok_v.match(TokTy::SEMICOLON);
    }
    // case: T comp ...
    else if (ctx.tok_v.match(TokTy::COMPONENT)) {
      auto comp            = ctx.Create_Node<ast::generic::Use_Component>(ctx.tok_v.peek());
      comp->target_gen_sym = firstok;
      comp->component      = ctx.p_expr->parse_expression();
      comp->parent_generic = gen;

      gen->conditions.push_back(std::move(comp));
    }
    // case: T role ...
    else if (ctx.tok_v.match(TokTy::ROLE)) {
      auto role            = ctx.Create_Node<ast::generic::Have_Role>(ctx.tok_v.peek());
      role->target_gen_sym = firstok;
      role->role           = ctx.p_expr->parse_expression();
      role->parent_generic = gen;

      gen->conditions.push_back(std::move(role));
    }
    // case: T sys ...
    else if (ctx.tok_v.match(TokTy::SYSTEM)) {
      auto sys            = ctx.Create_Node<ast::generic::Compatible_System>(ctx.tok_v.peek());
      sys->target_gen_sym = firstok;
      sys->system         = ctx.p_expr->parse_expression();
      sys->parent_generic = gen;

      gen->conditions.push_back(std::move(sys));
    }
    // case: T is i32 | type::floating | ...
    else if (ctx.tok_v.match(TokTy::IS)) {
      auto nested             = ctx.Create_Node<ast::generic::Is_Type>(ctx.tok_v.peek());
      nested->source_typename = firstok;
      nested->parent_generic  = gen;

      while (!ctx.tok_v.is_end()) {
        nested->in_type.push_back(ctx.p_type->parse_type());

        if (ctx.tok_v.match(TokTy::PIPE)) continue;
        break;
      }

      gen->conditions.push_back(std::move(nested));
    }
    // case: T cast to/from ...
    else if (ctx.tok_v.match(TokTy::CAST)) {
      auto castNode             = ctx.Create_Node<ast::generic::Can_Cast>(ctx.tok_v.peek());
      castNode->source_typename = firstok;
      castNode->parent_generic  = gen;

      if (ctx.tok_v.peek(0).val != "to" && ctx.tok_v.peek(0).val != "from") {
        ctx.tok_v.add_error(75, "Expected cast way 'to' or 'from' in generic filter argument.", kHint_filter);
      }

      castNode->target = ctx.p_type->parse_type();

      gen->conditions.push_back(std::move(castNode));
    } else
      ctx.tok_v.add_error(76, "Expected generic condition 'use' or 'is' in generic filter argument.", kHint_filter);

    ctx.tok_v.match(TokTy::SEMICOLON);
  }

  ctx.exit_scope();
  return gen;
}

std::shared_ptr<ast::declaration::Type_Alias> parser::Parser_Declaration::type_alias()
{
  auto tyAlias = ctx.Create_Decl<ast::declaration::Type_Alias>(ctx.tok_v.peek());

  ctx.tok_v.match(TokTy::TYPE);

  tyAlias->declaration_name = ctx.parse_name();

  ctx.tok_v.expect(77, TokTy::ASSIGN, "Expected '=' after type alias.",
                   "define type alias like:"
                   "\n  - type `type myAlias = i32`."
                   "\n  - generic `type Vec<T> = core::container::vector<T>`."
                   "\n  - generic `type StrList = core::container::vector<str>`.");

  tyAlias->type = ctx.p_type->parse_type();

  ctx.current_module->add_item(tyAlias);
  return tyAlias;
}
