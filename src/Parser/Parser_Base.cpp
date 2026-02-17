
#include "Parser_Base.hpp"

#include "AST/AST_Expression.hpp"
#include "AST/AST_Headers.hpp"
#include "Parser_Headers.hpp"

#include "ScriptInfo.hpp"
#include "Visitor/Symbol_Manager.hpp"

#include "Metacode.hpp"

PAR::Parser_Base::Parser_Base(ScriptInfo &scr_info)
{
  ctx = new Parser_Context(scr_info);

  Parser_Declaration_COP   *p_cop   = new Parser_Declaration_COP(*ctx);
  Parser_Declaration       *p_decl  = new Parser_Declaration(*ctx);
  Parser_Expression        *p_expr  = new Parser_Expression(*ctx);
  Parser_Literal           *p_lit   = new Parser_Literal(*ctx);
  Parser_Declaration_Local *p_loc   = new Parser_Declaration_Local(*ctx);
  Parser_Memory            *p_mem   = new Parser_Memory(*ctx);
  Parser_Operator          *p_op    = new Parser_Operator(*ctx);
  Parser_Statement         *p_state = new Parser_Statement(*ctx);
  Parser_Type              *p_type  = new Parser_Type(*ctx);

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

PAR::Parser_Base::~Parser_Base() {}

std::vector<std::string> PAR::Parser_Base::start_parsing()
{
  // generate and enter in global scope
  if (!ctx->m_sym) ctx->m_sym = new Symbols_Manager(ctx->scr_info);
  ctx->scr_info.rootNode = new AST::Root;

  try {
    while (!ctx->tok_v.is_end()) {
      auto line = ctx->p_decl->parse_declaration();
      if (line) ctx->scr_info.rootNode->global_nodes.push_back(line);
      if (ctx->tok_v.match(TokTy::S_END_OF_FILE)) break;
    }
  } catch (const std::runtime_error &e) {
    // std::cerr << e.what() << std::endl;
    // context.tokView.synchronize();
    // attempt_recovery();
  }

  return ctx->tok_v.errors;
}

ModuleImportation *PAR::Parser_Base::parse_import()
{
  static const std::string hint =
      "define import module like:"
      "\n  - import `import <name>`"
      "\n  - import standard `import @<name>`"
      "\n  - import user (default) `import $<name>`"
      "\n  - import external `import extern <lang>::<lib>`"
      "\n  - export/import module only declared in the root scope";

  ctx->tok_v.match(TokTy::IMPORT);

  ModuleImportation mod_imp;

  // standard lib importation
  // e.g. import @io
  if (ctx->tok_v.match(TokTy::AT)) {
    mod_imp.import_source = ModuleImportation::EImportSource::StandardLib;
  }
  // user importation
  // e.g. import $io
  else if (ctx->tok_v.match(TokTy::DOLLAR)) {
    mod_imp.import_source = ModuleImportation::EImportSource::User;
  } else {
    // import from external code
    // e.g. import extern C::stdio
    if (ctx->tok_v.match(TokTy::EXTERN)) {
      mod_imp.name = ctx->tok_v.next().val;
      ctx->tok_v.expect<8>(TokTy::STATIC_ACCESS, "Expected static access '::' after extern import source name!", hint);
      mod_imp.extern_lib = ctx->tok_v.next().val;
    } else {
      mod_imp.name = ctx->tok_v.next().val;
    }
  }

  auto uptr_imp = std::make_unique<ModuleImportation>(mod_imp);
  auto ptr_imp  = uptr_imp.get();
  ctx->scr_info.imported_mod.push_back(std::move(uptr_imp));

  return ptr_imp;
}

std::shared_ptr<AST::Declaration::Export> PAR::Parser_Base::parse_export()
{
  static const std::string hint =
      "define export module like:"
      "\n  - export module `export <name> {...}`"
      "\n  - export imported module (mirror) `export import <name>`"
      "\n  - export to other language `export <name> extern <language> {...}`"
      "\n  - export/import module only declared in the root scope";

  ctx->tok_v.match(TokTy::EXPORT);

  Token exp_tok = ctx->tok_v.peek(-1);

  ModuleExportation mod_exp;

  mod_exp.is_native = ctx->m_meta->contains(ctx->tok_v.position(), "native");

  // is mirror
  // e.g. export import math
  if (ctx->tok_v.match(TokTy::IMPORT)) {
    mod_exp.is_mirror = true;
    mod_exp.mirror    = parse_import();

    auto uptr_exp = std::make_unique<ModuleExportation>(mod_exp);
    ctx->scr_info.exported_mod.push_back(std::move(uptr_exp));
    return nullptr;
  }

  mod_exp.name = ctx->tok_v.next().val;

  // is external exportation
  // e.g. export math extern C
  if (ctx->tok_v.match(TokTy::EXTERN)) {
    mod_exp.extern_lib = ctx->tok_v.next().val;
  }

  auto uptr_exp = std::make_shared<ModuleExportation>(mod_exp);

  auto exp_node         = ctx->Create_Decl<AST::Declaration::Export>(exp_tok);
  exp_node->name        = mod_exp.name;
  exp_node->mod_exp_sym = uptr_exp;

  ctx->scr_info.exported_mod.push_back(uptr_exp);

  ctx->tok_v.expect<9>(TokTy::OPEN_BRACE, "Expected export begin scope '{' after import instruction.", hint);

  ctx->m_sym->enter_scope(mod_exp.name, EScopeType::Export, 0);

  while (!ctx->tok_v.is_end()) {
    if (ctx->tok_v.check_any({TokTy::IMPORT, TokTy::EXPORT})) {
      ctx->tok_v.add_error_tok<10>(ctx->tok_v.peek(), "Illegal nested module export/import instruction.", hint);
    }

    exp_node->elements.push_back(ctx->p_decl->parse_declaration());

    ctx->tok_v.match(TokTy::SEMICOLON);
    if (ctx->match_field_separator(TokTy::S_END_OF_FILE, TokTy::CLOSE_BRACE)) break;
  }

  return exp_node;
}

std::optional<AST::CodeBlock_instruction> PAR::Parser_Base::parse_instruction()
{
  static const std::string hint =
      "define insutrction like:"
      "\n  - statement `if/elif/else/match/while/do-while/loop/for/goto`"
      "\n  - local variable declaration `let/var/const`"
      "\n  - lambda declaration `lam ...`"
      "\n  - assignation `left copy=/move=/ref=/mut= ...`"
      "\n  - operation assignation `left +=/-=/*=//=/... ...`"
      "\n  - function call `name()`"
      "\n  - system call `entity_name::>system_name()"
      "\n  - memory deletion `del pointer_name`";

  // if elif else for ...
  if (auto statement = ctx->p_state->parse_statement(true)) {
    AST::CodeBlock_instruction cb;
    cb.data = std::move(statement);
    return cb;
  }
  // local variable + lambda
  else if (auto local = ctx->p_loc->parse_local(true)) {
    AST::CodeBlock_instruction cb;
    cb.data = local;
    return cb;
  }
  // del
  else if (ctx->tok_v.check(TokTy::DEL)) {
    auto                       del = ctx->p_mem->del();
    AST::CodeBlock_instruction cb;
    cb.data = std::move(del);
    return cb;
  } else if (auto expr = ctx->p_expr->parse_expression()) {
    // assignation and operator assignment
    if (ctx->tok_v.check_any(kAssignationTokens)) {
      auto assign = ctx->p_op->assignment(std::move(expr));

      AST::CodeBlock_instruction cb;
      cb.data = std::move(assign);
      return cb;
    }
    // call and sys_call
    else if (dynamic_cast<AST::Expression::Call *>(expr.get())) {
      AST::CodeBlock_instruction cb;
      cb.data = std::move(expr);
      return cb;
    }
  }

  ctx->tok_v.add_error_tok<11>(ctx->tok_v.peek(), "Unexpected instruction", hint);
  return std::nullopt;
}
