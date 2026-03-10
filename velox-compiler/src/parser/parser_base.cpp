
#include "parser_base.hpp"

#include "ast/ast_base.hpp"
#include "ast/ast_data.hpp"
#include "ast/ast_declaration.hpp"
#include "ast/ast_expression.hpp"
#include "ast/ast_operation.hpp"
#include "ast/ast_memory.hpp"

#include "parser_context.hpp"
#include "parser_declaration.hpp"
#include "parser_declaration_cop.hpp"
#include "parser_declaration_local.hpp"
#include "parser_context.hpp"
#include "parser_expression.hpp"
#include "parser_literal.hpp"
#include "parser_memory.hpp"
#include "parser_operation.hpp"
#include "parser_statement.hpp"
#include "parser_type.hpp"

#include "script_info.hpp"
#include "visitor/symbol_manager.hpp"

#include "metacode.hpp"
#include <memory>

parser::Parser_Base::Parser_Base(ScriptInfo& scr_info)
{
  ctx = new Parser_Context(scr_info);

  Parser_Declaration_COP*   p_cop   = new Parser_Declaration_COP(*ctx);
  Parser_Declaration*       p_decl  = new Parser_Declaration(*ctx);
  Parser_Expression*        p_expr  = new Parser_Expression(*ctx);
  Parser_Literal*           p_lit   = new Parser_Literal(*ctx);
  Parser_Declaration_Local* p_loc   = new Parser_Declaration_Local(*ctx);
  Parser_Memory*            p_mem   = new Parser_Memory(*ctx);
  Parser_Operator*          p_op    = new Parser_Operator(*ctx);
  Parser_Statement*         p_state = new Parser_Statement(*ctx);
  Parser_Type*              p_type  = new Parser_Type(*ctx);

  ctx->p_cop   = p_cop;
  ctx->p_decl  = p_decl;
  ctx->p_expr  = p_expr;
  ctx->p_lit   = p_lit;
  ctx->p_loc   = p_loc;
  ctx->p_mem   = p_mem;
  ctx->p_op    = p_op;
  ctx->p_state = p_state;
  ctx->p_type  = p_type;

  ctx->p_base = this;
}

parser::Parser_Base::~Parser_Base()
{
}

std::vector<std::string> parser::Parser_Base::start_parsing()
{
  // generate and enter in global scope
  if (!ctx->m_sym) ctx->m_sym = new Symbols_Manager(ctx->scr_info);
  ctx->scr_info.rootNode = new ast::Root;

  try {
    while (!ctx->tok_v.is_end()) {
      auto line = ctx->p_decl->parse_declaration();
      if (line) ctx->scr_info.rootNode->global_nodes.push_back(line);
      if (ctx->tok_v.match(TokTy::S_END_OF_FILE)) break;
    }
  } catch (const std::runtime_error& e) {
    // std::cerr << e.what() << std::endl; context.tokView.synchronize(); attempt_recovery();
  }

  return ctx->tok_v.errors;
}

ModuleImportation* parser::Parser_Base::parse_import()
{
  static const std::string hint =
      R"(define import module like:
  - import `import <name>[::*]`
  - import standard `import @<name>[::*]`
  - import user (default) `import $<name>[::*]`
  - import external `import extern <lang>::<lib>`
  - export/import module only declared in the root scope)";

  auto extract_id = [](const ast::AIdentifier& id, std::string& input_name, std::vector<std::string>& input_path) {
    input_name = id.get_base_name();
    if (auto ptr = dynamic_cast<const ast::Expr_ID_Qualified*>(&id)) input_path = ptr->path;
  };

  ctx->tok_v.match(TokTy::IMPORT);

  ModuleImportation mod_imp;

  // import std: @
  if (ctx->tok_v.match_any({TokTy::AT, TokTy::STD_LIB})) {
    mod_imp.import_source = ModuleImportation::EImportSource::StandardLib;
    auto id               = ctx->p_expr->identifier();
    extract_id(*id, mod_imp.name, mod_imp.path);
  }
  // import usr: $
  else if (ctx->tok_v.match_any({TokTy::DOLLAR, TokTy::USR_LIB})) {
    mod_imp.import_source = ModuleImportation::EImportSource::User;
    auto id               = ctx->p_expr->identifier();
    extract_id(*id, mod_imp.name, mod_imp.path);
  }
  // import lib: #
  else if (ctx->tok_v.match_any({TokTy::HASHTAG, TokTy::USR_LIB})) {
    mod_imp.import_source = ModuleImportation::EImportSource::UserLib;
    auto id               = ctx->p_expr->identifier();
    extract_id(*id, mod_imp.name, mod_imp.path);
  }
  // import ext:
  else if (ctx->tok_v.match(TokTy::EXT_LIB)) {
    // import from external code e.g. import extern C::stdio
    mod_imp.import_source = ModuleImportation::EImportSource::Extern;
    mod_imp.name          = ctx->parse_name();
    ctx->tok_v.expect(8, TokTy::STATIC_ACCESS, "Expected static access '::' after extern import source name!", hint);
    mod_imp.extern_lib = ctx->tok_v.next().val;
  } else {
    mod_imp.import_source = ModuleImportation::EImportSource::Unknown;
    auto id               = ctx->p_expr->identifier();
    extract_id(*id, mod_imp.name, mod_imp.path);
  }

  auto uptr_imp = std::make_unique<ModuleImportation>(mod_imp);
  auto ptr_imp  = uptr_imp.get();
  ctx->scr_info.imported_mod.push_back(std::move(uptr_imp));

  return ptr_imp;
}

std::shared_ptr<ast::declaration::Export> parser::Parser_Base::parse_export()
{
  static const std::string hint =
      R"(define export module like:
  - export module `export {...}`
  - export imported module (mirror) `export import <name>`
  - export to other language `export extern <language> {...}`
  - export/import module only declared in the root scope)";

  ctx->tok_v.match(TokTy::EXPORT);

  Token exp_tok = ctx->tok_v.peek(-1);

  ModuleExportation mod_exp;

  // is mirror e.g. export import math
  if (ctx->tok_v.match(TokTy::IMPORT)) {
    mod_exp.is_mirror = true;
    mod_exp.mirror    = parse_import();

    auto uptr_exp = std::make_unique<ModuleExportation>(mod_exp);
    ctx->scr_info.exported_mod.push_back(std::move(uptr_exp));
    return nullptr;
  }

  // is external exportation e.g. export math extern C
  if (ctx->tok_v.match(TokTy::EXT_LIB)) {
    mod_exp.extern_lib = ctx->parse_name("Expected external language name to export", hint);
  }

  auto uptr_exp = std::make_shared<ModuleExportation>(mod_exp);

  auto exp_node         = ctx->Create_Decl<ast::declaration::Export>(exp_tok);
  exp_node->mod_exp_sym = uptr_exp;

  ctx->scr_info.exported_mod.push_back(uptr_exp);

  ctx->tok_v.expect(9, TokTy::OPEN_BRACE, "Expected export begin scope '{' after import instruction.", hint);

  ctx->m_sym->in_export = true;

  if (ctx->tok_v.match(TokTy::CLOSE_BRACE)) {
    ctx->m_sym->in_export = false;
    return exp_node;
  }

  while (!ctx->tok_v.is_end()) {
    if (ctx->tok_v.check_any({TokTy::IMPORT, TokTy::EXPORT})) {
      ctx->tok_v.add_error_tok(10, ctx->tok_v.peek(), "Illegal nested module export/import instruction.", hint);
    }

    exp_node->declarations.push_back(ctx->p_decl->parse_declaration());

    ctx->tok_v.match(TokTy::SEMICOLON);
    if (ctx->match_field_separator(TokTy::S_END_OF_FILE, TokTy::CLOSE_BRACE)) break;
  }


  ctx->m_sym->in_export = false;

  return exp_node;
}

std::shared_ptr<ast::declaration::Extern> parser::Parser_Base::parse_extern()
{
  static const std::string hint = R"(define extern like: `extern "ABI" {...}`)";

  ctx->tok_v.match(TokTy::EXTERN);

  auto ext_tok = ctx->tok_v.peek(-1);

  auto ext_node  = ctx->Create_Decl<ast::declaration::Extern>(ext_tok);
  ext_node->name = ctx->tok_v.expect(153, TokTy::L_TEXTUAL, "Expected literal string to define ABI.", hint).val;


  ctx->tok_v.expect(9, TokTy::OPEN_BRACE, "Expected export begin scope '{' after import instruction.", hint);

  ctx->in_extern = true;

  if (ctx->tok_v.match(TokTy::CLOSE_BRACE)) {
    ctx->in_extern = false;
    return ext_node;
  }

  while (!ctx->tok_v.is_end()) {
    if (ctx->tok_v.check_any({TokTy::IMPORT, TokTy::EXPORT})) {
      ctx->tok_v.add_error_tok(10, ctx->tok_v.peek(), "Illegal nested module export/import instruction.", hint);
    }

    ext_node->declarations.push_back(ctx->p_decl->parse_declaration());

    ctx->tok_v.match(TokTy::SEMICOLON);
    if (ctx->match_field_separator(TokTy::S_END_OF_FILE, TokTy::CLOSE_BRACE)) break;
  }

  ctx->in_extern = false;

  return ext_node;
}

ast::CodeBlock_instruction parser::Parser_Base::parse_instruction()
{
  static const std::string hint =
      R"(define insutrction like:
  - statement `if/elif/else/match/while/do-while/loop/for/goto`
  - local variable declaration `let/var/const`
  - lambda declaration `lam ...`
  - assignation `left copy=/move=/ref=/mut= ...`
  - operation assignation `left +=/-=/*=//=/... ...`
  - function call `name()`
  - system call `entity_name::>system_name()`
  - memory deletion `del pointer_name`)";

  // if elif else for ...
  if (auto statement = ctx->p_state->parse_statement(true)) {
    return ast::CodeBlock_instruction(std::move(statement));
  }
  // local variable + lambda
  else if (auto local = ctx->p_loc->parse_local(true)) {
    return ast::CodeBlock_instruction(local);
  }
  // del
  else if (ctx->tok_v.check(TokTy::DEL)) {
    auto del = ctx->p_mem->del();
    return ast::CodeBlock_instruction(std::move(del));
  } else if (auto expr = ctx->p_expr->parse_expression()) {
    // assignation and operator assignment
    if (ctx->tok_v.check_any(kAssignationTokens)) {
      auto assign = ctx->p_op->assignment(std::move(expr));

      return ast::CodeBlock_instruction(std::move(assign));
    }
    // call and sys_call
    else if (dynamic_cast<ast::expression::Call*>(expr.get())) {
      return ast::CodeBlock_instruction(std::move(expr));
    }
  }

  ctx->tok_v.add_error_tok(11, ctx->tok_v.peek(), "Unexpected instruction", hint);
  return ast::CodeBlock_instruction{};
}
