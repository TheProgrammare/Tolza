
#include "Parser_Declaration.hpp"

#include "AST/AST_Data.hpp"
#include "AST/AST_Declaration.hpp"
#include "AST/AST_Headers.hpp"
#include "Parser_Headers.hpp"

#include "Metacode.hpp"
#include "Visitor/Symbol_Manager.hpp"

std::shared_ptr<AST::ADeclaration> PAR::Parser_Declaration::parse_declaration()
{
  auto tok = ctx.tok_v.peek();
  switch (tok.type) {
  case TokTy::MOD:       return module();
  case TokTy::ENUM:      return enumeration();
  case TokTy::VAR:
  case TokTy::LET:
  case TokTy::CONST:     return global_variable();
  case TokTy::FUNCTION:  return function();
  case TokTy::GENERIC:   return generic();
  case TokTy::TYPE:      return type_alias();
  case TokTy::COMPONENT: return std::static_pointer_cast<AST::ADeclaration>(ctx.p_cop->component());
  case TokTy::SYSTEM:    return std::static_pointer_cast<AST::ADeclaration>(ctx.p_cop->system());
  case TokTy::ENTITY:    return std::static_pointer_cast<AST::ADeclaration>(ctx.p_cop->entity());
  case TokTy::EXPORT:    return ctx.p_base->parse_export();
  case TokTy::IMPORT:    {
    auto ignore = ctx.p_base->parse_import();
    return nullptr;
  }
  default: break;
  }

  ctx.tok_v.add_error<60>("Illegal instruction '" + tok.val + "' in global.",
                          "you can define in global: namespace, variable, function, entity, component, system");

  return nullptr;
}

std::shared_ptr<AST::ADeclaration> PAR::Parser_Declaration::module()
{
  static const std::string hint =
      "define module like:"
      "\n  - module `mod myName { ... }`"
      "\n  - module alias `mod Vec = core::container::vector`"
      "\n  - module to root `mod root = core::container::vector`";

  ctx.tok_v.match(TokTy::MOD);

  auto name = ctx.parse_name("", hint);

  if (ctx.tok_v.match(TokTy::ASSIGN)) {
    auto node    = ctx.Create_Decl<AST::Declaration::Mod_Alias>(ctx.tok_v.peek());
    node->name   = std::move(name);
    node->module = ctx.p_expr->identifier();

    ctx.m_sym->add_decl(node);

    return node;
  } else if (ctx.tok_v.match(TokTy::OPEN_BRACE)) {
    auto node = ctx.Create_Decl<AST::Declaration::Mod>(ctx.tok_v.peek());

    ctx.m_sym->enter_scope(name, EScopeType::Mod);
    node->name = std::move(name);

    while (!ctx.tok_v.is_end()) {
      if (ctx.tok_v.check_any({TokTy::IMPORT, TokTy::EXPORT})) {
        ctx.tok_v.add_error_tok<62>(ctx.tok_v.peek(), "Illegal nested module export/import instruction.", hint);
      }

      node->elements.push_back(ctx.p_decl->parse_declaration());

      if (ctx.match_field_separator(TokTy::S_END_OF_FILE, TokTy::CLOSE_BRACE)) break;
    }

    ctx.m_sym->exit_scope();

    return node;
  }

  ctx.tok_v.add_error_tok<61>(ctx.tok_v.peek(), "Expected '{' or '=' after module name.", hint);

  return nullptr;
}

std::shared_ptr<AST::Declaration::Enum> PAR::Parser_Declaration::enumeration()
{
  static const std::string hint =
      "define enum like:"
      "\n  - typed enum `enum name { field_name1(T), field_name2 }"
      "\n  - typed enum `enum Option { Valid(T), Invalid }";
  ctx.tok_v.match(TokTy::ENUM);

  auto enu  = ctx.Create_Decl<AST::Declaration::Enum>(ctx.tok_v.peek());
  enu->name = ctx.parse_name("", hint);
  ctx.m_sym->add_decl(enu);
  ctx.m_sym->enter_scope(enu->name, EScopeType::Enum);

  ctx.tok_v.expect<63>(TokTy::OPEN_BRACE, "Expected start enum block '{' after enum name declaration.", hint);

  while (!ctx.tok_v.is_end()) {
    auto elem  = ctx.Create_Node<AST::Declaration::Enum_Element>(ctx.tok_v.peek());
    elem->name = ctx.parse_name("", hint);

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

  ctx.m_sym->exit_scope();

  return enu;
}

std::shared_ptr<AST::Declaration::Global> PAR::Parser_Declaration::global_variable()
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

  auto          kind_tok = ctx.tok_v.expect_any<65>({TokTy::LET, TokTy::VAR, TokTy::CONST},
                                                    "Expected global variable declaration token", hint);
  EVariableKind kind     = TokTy_to_EVariableKind(kind_tok.type);

  auto var  = ctx.Create_Decl<AST::Declaration::Global>(ctx.tok_v.peek());
  var->kind = kind;
  var->name = ctx.parse_name("", hint);

  var->isExtern = ctx.metablock_contains(*var, "extern");

  ctx.m_sym->add_decl(var);

  if (var->name.empty()) {
    ctx.tok_v.add_error<66>("Invalid Identifier !", "");
  }

  // type definition no expression
  if (var->isExtern) {
    ctx.tok_v.expect<67>(TokTy::COLON, "Expected type definition for an global variable marked external.", hint);
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
  var->assignment  = TokTy_to_EAssignmentType(assign_tok.type);

  if (var->assignment == EAssignmentType::NONE && isAutoTy)
    ctx.tok_v.add_error<68>("Expected assignation '=' in auto inferred variable type.",
                            "define auto inferred variable like `let myName = expression;`");

  auto expr = ctx.p_expr->parse_expression();

  // if (isAutoTy) var->type = resolve_type(expr.get());

  var->expression = std::move(expr);

  return var;
}

std::shared_ptr<AST::Declaration::Function> PAR::Parser_Declaration::function()
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

  auto fn = ctx.Create_Decl<AST::Declaration::Function>(ctx.tok_v.peek());

  fn->name    = ctx.parse_name("", hint);
  fn->isConst = ctx.metablock_contains(*fn, "const");
  fn->isPure  = ctx.metablock_contains(*fn, "pure");

  fn->isExtern = ctx.metablock_contains(*fn, "extern");
  if (auto pattern = ctx.get_instruct(*fn, {"extern", "<*>"})) {
    fn->extern_call_convention = pattern->at_str(1, 0);
  }

  ctx.m_sym->add_decl(fn);
  ctx.m_sym->enter_scope(fn->name, EScopeType::Function);

  fn->prototype = ctx.p_type->explicit_function_proto(false);
  for (auto& param : fn->prototype->parameters) param->parent_function = fn;

  // if extern : no definition
  if (fn->isExtern && ctx.tok_v.check(TokTy::OPEN_BRACE))
    ctx.tok_v.add_error<69>("Unexpected start code block '{' after a extern function declaration", hint);

  if (!fn->isExtern) fn->codeblock = ctx.p_loc->code_block_instruction();

  ctx.m_sym->exit_scope();

  return fn;
}

std::shared_ptr<AST::Declaration::Generic> PAR::Parser_Declaration::generic()
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

  auto gen = ctx.Create_Decl<AST::Declaration::Generic>(ctx.tok_v.peek());

  ctx.tok_v.match(TokTy::GENERIC);

  gen->name = ctx.parse_name("", kHint_gen);
  ctx.m_sym->add_decl(gen);
  ctx.m_sym->enter_scope(gen->name, EScopeType::Generic);
  ctx.tok_v.expect<70>(TokTy::OPEN_BRACKETS, "Expected start type '<' after generic name.", kHint_gen);

  while (!ctx.tok_v.is_end()) {
    gen->targetGenericSymbols.insert(ctx.parse_name("", kHint_gen));

    if (ctx.match_field_separator(TokTy::S_END_OF_FILE, TokTy::CLOSE_BRACKETS)) break;
  }

  ctx.tok_v.expect<72>(TokTy::OPEN_BRACE, "Expected start code block '{' after generic declaration.", kHint_gen);

  while (!ctx.tok_v.is_end()) {
    if (ctx.tok_v.match(TokTy::CLOSE_BRACE)) break;
    std::string firstok = ctx.parse_name("", kHint_filter);

    // case: T op ...
    if (ctx.tok_v.match(TokTy::OP)) {
      auto gen_op          = ctx.Create_Node<AST::Generic::Have_Op>(ctx.tok_v.peek());
      gen_op->targetGenSym = firstok;
      auto tok_op          = ctx.tok_v.expect_any<74>(
          kOperatorTokens, "Expected operator after 'op' keyword in generic filter argument.", kHint_filter);
      gen_op->operatorType   = TokTy_to_EBinOpType(tok_op.type);
      gen_op->parent_generic = gen;

      gen->conditions.push_back(std::move(gen_op));
      ctx.tok_v.match(TokTy::SEMICOLON);
    }
    // case: T comp ...
    else if (ctx.tok_v.match(TokTy::COMPONENT)) {
      auto comp            = ctx.Create_Node<AST::Generic::Use_Component>(ctx.tok_v.peek());
      comp->targetGenSym   = firstok;
      comp->component      = ctx.p_expr->parse_expression();
      comp->parent_generic = gen;

      gen->conditions.push_back(std::move(comp));
    }
    // case: T role ...
    else if (ctx.tok_v.match(TokTy::ROLE)) {
      auto role            = ctx.Create_Node<AST::Generic::Have_Role>(ctx.tok_v.peek());
      role->targetGenSym   = firstok;
      role->role           = ctx.p_expr->parse_expression();
      role->parent_generic = gen;

      gen->conditions.push_back(std::move(role));
    }
    // case: T sys ...
    else if (ctx.tok_v.match(TokTy::SYSTEM)) {
      auto sys            = ctx.Create_Node<AST::Generic::Compatible_System>(ctx.tok_v.peek());
      sys->targetGenSym   = firstok;
      sys->system         = ctx.p_expr->parse_expression();
      sys->parent_generic = gen;

      gen->conditions.push_back(std::move(sys));
    }
    // case: T is i32 | type::floating | ...
    else if (ctx.tok_v.match(TokTy::IS)) {
      auto nested            = ctx.Create_Node<AST::Generic::Is_Type>(ctx.tok_v.peek());
      nested->srcTypename    = firstok;
      nested->parent_generic = gen;

      while (!ctx.tok_v.is_end()) {
        nested->inType.push_back(ctx.p_type->parse_type());

        if (ctx.tok_v.match(TokTy::PIPE)) continue;
        break;
      }

      gen->conditions.push_back(std::move(nested));
    }
    // case: T cast to/from ...
    else if (ctx.tok_v.match(TokTy::CAST)) {
      auto castNode            = ctx.Create_Node<AST::Generic::Can_Cast>(ctx.tok_v.peek());
      castNode->srcTypename    = firstok;
      castNode->parent_generic = gen;

      if (ctx.tok_v.peek(0).val != "to" && ctx.tok_v.peek(0).val != "from") {
        ctx.tok_v.add_error<75>("Expected cast way 'to' or 'from' in generic filter argument.", kHint_filter);
      }

      castNode->target = ctx.p_type->parse_type();

      gen->conditions.push_back(std::move(castNode));
    } else
      ctx.tok_v.add_error<76>("Expected generic condition 'use' or 'is' in generic filter argument.", kHint_filter);

    ctx.tok_v.match(TokTy::SEMICOLON);
  }

  ctx.m_sym->exit_scope();
  return gen;
}

std::shared_ptr<AST::Declaration::Type_Alias> PAR::Parser_Declaration::type_alias()
{
  auto tyAlias = ctx.Create_Decl<AST::Declaration::Type_Alias>(ctx.tok_v.peek());

  ctx.tok_v.match(TokTy::TYPE);

  tyAlias->name = ctx.parse_name();

  ctx.tok_v.expect<77>(TokTy::COLON, "Expected '=' after type alias.",
                       "define type alias like:"
                       "\n  - type `type myAlias = i32`."
                       "\n  - generic `type Vec<T> = core::container::vector<T>`."
                       "\n  - generic `type StrList = core::container::vector<str>`.");

  tyAlias->type = ctx.p_type->parse_type();

  ctx.m_sym->add_decl(tyAlias);
  return tyAlias;
}
