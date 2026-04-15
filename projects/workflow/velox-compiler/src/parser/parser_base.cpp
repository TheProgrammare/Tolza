
#include "parser_base.hpp"

#include "ast/ast_base.hpp"
#include "ast/ast_codeblock_instruction.hpp"
#include "ast/ast_data.hpp"
#include "ast/ast_declaration.hpp"
#include "ast/ast_declaration_local.hpp"
#include "ast/ast_expression.hpp"
#include "ast/ast_operation.hpp"
#include "ast/ast_memory.hpp"

#include "misc/module_manager.hpp"
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

#include "misc/script_info.hpp"
#include "misc/symbol_manager.hpp"

#include "misc/metacode.hpp"
#include <iostream>
#include <memory>
#include <vector>

parser::Parser_Base::Parser_Base(Parser_Context& p_ctx)
  : ctx(p_ctx)
{
}

parser::Parser_Base::~Parser_Base()
{
}

std::shared_ptr<ast::declaration::Import> parser::Parser_Base::parse_import()
{
  static const std::string hint =
      R"(define import module like:
  - import `import <name>[::*]`
  - import standard `import @<name>[::*]`
  - import user (default) `import $<name>[::*]`
  - import external `import extern <lang>::<lib>`
  - export/import module only declared in the root scope)";

  auto full_path = [](const ast::AIdentifier& id) -> std::vector<std::string> {
    if (auto ptr = dynamic_cast<const ast::Expr_ID*>(&id)) {
      std::vector<std::string> path;
      path.push_back(ptr->name);
      return path;
    } else if (auto ptr = dynamic_cast<const ast::Expr_ID_Qualified*>(&id)) {
      std::vector<std::string> path = ptr->path;
      path.push_back(ptr->name);
      return path;
    }
  };

  ctx.tok_v.match(TokTy::IMPORT);

  auto base_tok = ctx.tok_v.peek(-1);

  auto imp = ctx.Create_Decl<ast::declaration::Import>(base_tok);

  EFileSource f_src;

  // import std: @
  if (ctx.tok_v.match_any({TokTy::AT, TokTy::STD_LIB})) {
    f_src = EFileSource::stdlib;
  }
  // import usr: $
  else if (ctx.tok_v.match_any({TokTy::DOLLAR, TokTy::USR_LIB})) {
    f_src = EFileSource::src;
  }
  // import pkg: #
  else if (ctx.tok_v.match_any({TokTy::HASHTAG, TokTy::PKG_LIB})) {
    f_src = EFileSource::pkg_lib;
  }
  // import bind: ?
  else if (ctx.tok_v.match_any({TokTy::INTERROGATIVE, TokTy::BIND_LIB})) {
    f_src = EFileSource::binding;
  } else {
    f_src = EFileSource::relative;
  }

  // name path
  {
    auto next_path = full_path(*ctx.p_expr->identifier().get());
    imp->path.insert(imp->path.end(), next_path.begin(), next_path.end());
  }

  auto mod = module::build_module_from_path(ctx.current_module, imp->path, f_src);


  if (!mod) {
    ctx.tok_v.add_error_tok(238, base_tok, mod.error(), "");
    return imp;
  }

  bool success = ctx.current_module->import_module(mod.value());

  if (!success) {
    ctx.tok_v.add_error_tok(
        239, base_tok, "Impossible to add the module \"" + mod.value()->debug_name + "\" in the current module", "");
  }

  return imp;
}

std::shared_ptr<ast::declaration::Export> parser::Parser_Base::parse_export()
{
  static const std::string hint = "define export module like: `export {...}`";

  ctx.tok_v.match(TokTy::EXPORT);

  if (ctx.current_module->find_item("export"))
    ctx.tok_v.add_error(237, "Illegal double export declaration in same scope",
                        "Specify only one `export {...}` declaration for each scope.");

  auto exp_node = ctx.Create_Decl<ast::declaration::Export>(ctx.tok_v.peek(-1));

  ctx.tok_v.expect(9, TokTy::OPEN_BRACE, "Expected export begin scope '{' after import instruction.", hint);
  ctx.enter_module(exp_node, "export");

  ctx.in_export = true;


  if (ctx.tok_v.match(TokTy::CLOSE_BRACE)) {
    ctx.in_export = false;
    return exp_node;
  }

  while (!ctx.tok_v.is_end()) {
    if (ctx.tok_v.check_any({TokTy::EXPORT})) {
      ctx.tok_v.add_error_tok(10, ctx.tok_v.peek(), "Illegal nested export module instruction.", hint);
    }

    exp_node->declarations.push_back(ctx.p_decl->parse_declaration());

    ctx.tok_v.match(TokTy::SEMICOLON);
    if (ctx.match_field_separator(TokTy::S_END_OF_FILE, TokTy::CLOSE_BRACE)) break;
  }

  ctx.in_export = false;


  return exp_node;
}

std::shared_ptr<ast::declaration::ReExport> parser::Parser_Base::parse_reexport()
{
  static const std::string hint = "define re-export module like: `reexport <path>`";

  auto full_path = [](const ast::AIdentifier& id) -> std::vector<std::string> {
    if (auto ptr = dynamic_cast<const ast::Expr_ID*>(&id)) {
      std::vector<std::string> path;
      path.push_back(ptr->name);
      return path;
    } else if (auto ptr = dynamic_cast<const ast::Expr_ID_Qualified*>(&id)) {
      std::vector<std::string> path = ptr->path;
      path.push_back(ptr->name);
      return path;
    }
  };


  ctx.tok_v.match(TokTy::REEXPORT);

  auto base_tok = ctx.tok_v.peek(-1);

  auto reexp_node = ctx.Create_Decl<ast::declaration::ReExport>(base_tok);


  EFileSource f_src;

  // import std: @
  if (ctx.tok_v.match_any({TokTy::AT, TokTy::STD_LIB})) {
    f_src = EFileSource::stdlib;
  }
  // import usr: $
  else if (ctx.tok_v.match_any({TokTy::DOLLAR, TokTy::USR_LIB})) {
    f_src = EFileSource::src;
  }
  // import pkg: #
  else if (ctx.tok_v.match_any({TokTy::HASHTAG, TokTy::PKG_LIB})) {
    f_src = EFileSource::pkg_lib;
  }
  // import bind: ?
  else if (ctx.tok_v.match_any({TokTy::INTERROGATIVE, TokTy::BIND_LIB})) {
    f_src = EFileSource::binding;
  } else {
    f_src = EFileSource::relative;
  }

  // name path
  {
    auto next_path = full_path(*ctx.p_expr->identifier().get());
    reexp_node->path.insert(reexp_node->path.end(), next_path.begin(), next_path.end());
  }

  auto mod = module::build_module_from_path(ctx.current_module, reexp_node->path, f_src);


  if (!mod) {
    ctx.tok_v.add_error_tok(240, base_tok, mod.error(), "");
    return reexp_node;
  }

  bool success = ctx.current_module->reexport_module(mod.value());

  if (!success) {
    ctx.tok_v.add_error_tok(
        238, base_tok, "Impossible to add the module \"" + mod.value()->debug_name + "\" in the current module", "");
  }

  return reexp_node;
}


std::shared_ptr<ast::declaration::Extern> parser::Parser_Base::parse_extern()
{
  static const std::string hint = R"(define extern like: `extern "ABI" {...}`)";

  ctx.tok_v.match(TokTy::EXTERN);

  auto ext_tok = ctx.tok_v.peek(-1);

  auto ext_node = ctx.Create_Decl<ast::declaration::Extern>(ext_tok);
  ext_node->declaration_name =
      ctx.tok_v.expect(153, TokTy::L_TEXTUAL, "Expected literal string to define ABI.", hint).val;


  ctx.tok_v.expect(9, TokTy::OPEN_BRACE, "Expected export begin scope '{' after import instruction.", hint);

  ctx.in_extern = true;
  ctx.enter_module(ext_node, "extern");

  if (ctx.tok_v.match(TokTy::CLOSE_BRACE)) {
    ctx.in_extern = false;
    return ext_node;
  }

  while (!ctx.tok_v.is_end()) {
    if (ctx.tok_v.check_any({TokTy::IMPORT, TokTy::EXPORT})) {
      ctx.tok_v.add_error_tok(10, ctx.tok_v.peek(), "Illegal nested module export/import instruction.", hint);
    }

    ext_node->declarations.push_back(ctx.p_decl->parse_declaration());

    ctx.tok_v.match(TokTy::SEMICOLON);
    if (ctx.match_field_separator(TokTy::S_END_OF_FILE, TokTy::CLOSE_BRACE)) break;
  }

  ctx.in_extern = false;
  ctx.exit_module();

  return ext_node;
}

ast::CodeBlock_instruction parser::Parser_Base::parse_instruction()
{
  static const std::string hint =
      R"(define insutrction like:
  - statement `if/elif/else/match/while/do-while/loop/for/goto`
  - local variable declaration `let/var/const`
  - lambda declaration `lam ...`
  - assignation `left copy=/move= ...`
  - operation assignation `left +=/-=/*=//=/... ...`
  - function call `name()`
  - system call `entity_name::>system_name()`
  - memory deletion `del pointer_name`)";

  // if elif else for ...
  if (auto statement = ctx.p_state->parse_statement(true)) {
    return ast::CodeBlock_instruction(std::move(statement));
  } else if (ctx.tok_v.check_any({TokTy::VAR, TokTy::LET, TokTy::CONST})
             && ctx.tok_v.peek(1).type == TokTy::OPEN_PAREN) {
    auto tuple = ctx.p_loc->tuple_destructuring();
    return ast::CodeBlock_instruction(std::move(tuple));
  }
  // local variable + lambda
  else if (auto local = ctx.p_loc->parse_local(true)) {
    return ast::CodeBlock_instruction(local);
  }
  // del
  else if (ctx.tok_v.check(TokTy::DEL)) {
    auto del = ctx.p_mem->del();
    return ast::CodeBlock_instruction(std::move(del));
  } else if (auto expr = ctx.p_expr->parse_expression()) {
    // assignation and operator assignment
    if (ctx.tok_v.check_any(kAssignationTokens)) {
      auto assign = ctx.p_op->assignment(std::move(expr));

      return ast::CodeBlock_instruction(std::move(assign));
    }
    // call and sys_call
    else if (dynamic_cast<ast::expression::Call*>(expr.get())) {
      return ast::CodeBlock_instruction(std::move(expr));
    }
  }

  ctx.tok_v.add_error_tok(11, ctx.tok_v.peek(), "Unexpected instruction", hint);
  return ast::CodeBlock_instruction{};
}
