
#include "parser_base.hpp"

#include "ast/ast_base.hpp"
#include "ast/ast_declaration_global.hpp"
#include "compiler/compiler.hpp"
#include "nexus/forward.hpp"
#include "nexus/ids.hpp"
#include "nexus/lexer/token.hpp"
#include "nexus/ast/ast.hpp"
#include "ast/ast_declaration_local.hpp"
#include "ast/ast_expression.hpp"
#include "ast/ast_operation.hpp"
#include "ast/ast_memory.hpp"

#include "nexus/module.hpp"
#include "nexus/scope.hpp"
#include "parser_context.hpp"
#include "parser_declaration_global.hpp"
#include "parser_declaration_cop.hpp"
#include "parser_declaration_local.hpp"
#include "parser_context.hpp"
#include "parser_expression.hpp"
#include "parser_literal.hpp"
#include "parser_memory.hpp"
#include "parser_operation.hpp"
#include "parser_statement.hpp"
#include "parser_type.hpp"
#include "ast/ast_declaration_global.hpp"

#include "nexus/script.hpp"
#include "nexus/symbol.hpp"

#include "nexus/metacode/metacode.hpp"
#include <iostream>
#include <memory>
#include <string_view>
#include <vector>

parser::Parser_Base::Parser_Base(Parser_Context& p_ctx)
  : p(p_ctx)
{
}

parser::Parser_Base::~Parser_Base()
{
}

ast::_gnid parser::Parser_Base::parse_import()
{
  constexpr std::string_view hint =
      R"(define import module like:
  - import `import <name>[::*] as <alias>`
  - import user (default) `import src::<name>[::*] as <alias>`
  - import standard `import std::<name>[::*] as <alias>`
  - import package `import pkg::<name>[::*] as <alias>`
  - import vendor `import ven::<name>[::*] as <alias>`
  - import external `import bind::<lang>::<lib> as <alias>`
  - export/import module only declared in the root scope)";

  p.match(token::ETokenKind::IMPORT);

  auto base_tok = p.peek(-1);

  parser_add_node(imp, Import, p.peek().id);

  imp->regex = p.p_base->regex_path();
  auto regex = p.scr_info.nodes->get_as<ast::Path_Regex>(imp->regex.get_node_id());

  if (auto regex = p.compilation_nodes->get_as<ast::Path_Regex>(imp->regex)) {
    if (regex->source == script::EFileSource::binding) {
      p.expect(251, token::ETokenKind::AS, "expected alias after a importation", "");
    } else if (p.match(token::ETokenKind::AS)) {
      imp->
    }
  }

  std::string out_err;
  auto mod = compiler::COMPILER.modules.tools.build_module_from_path(p.scr_id, regex->path, regex->source, out_err);
  if (!mod) {
    auto tok = p.scr_info.file_info.tokens->get(imp->node_token_id);
    auto err =
        Error_Diagnostic(p.scr_id, 238, tok.begin, tok.begin + tok.length, compiler::EPhase::binder, out_err, "");
    compiler::COMPILER.add_error(std::move(err));
  }

  p.get_current_scope().port.prepare_import(imp->node_id);

  p.scr_info.imports[imp->node_id] = mod;

  return imp->node_id;
}

ast::_gnid parser::Parser_Base::parse_export()
{
  constexpr std::string_view hint = "define export module like: `export {...}`";

  p.match(token::ETokenKind::EXPORT);

  parser_add_node(exp_node, Global_Export, p.peek().id);

  p.expect(9, token::ETokenKind::OPEN_BRACE, "Expected export begin scope '{' after import instruction.", hint);
  p.enter_scope(*exp_node, "export");

  p.in_export = true;

  if (p.match(token::ETokenKind::CLOSE_BRACE)) {
    p.in_export = false;
    return exp_node->node_id;
  }

  while (!p.is_end()) {
    if (p.check_any({token::ETokenKind::EXPORT})) {
      p.add_error_tok(10, p.peek(), "Illegal nested export module instruction.", hint);
    }

    exp_node->codeblock = p.p_loc->parse_codeblock();

    p.match(token::ETokenKind::SEMICOLON);
    if (p.match_field_separator(token::ETokenKind::S_END_OF_FILE, token::ETokenKind::CLOSE_BRACE)) break;
  }

  p.in_export            = false;
  p.scr_info.node_export = exp_node->node_id;

  return exp_node->node_id;
}

ast::_gnid parser::Parser_Base::parse_reexport()
{
  constexpr std::string_view hint = "define re-export module like: `reexport <path>`";

  p.match(token::ETokenKind::REEXPORT);

  auto base_tok = p.peek(-1);


  parser_add_node(reexp_node, Global_Reexport, p.peek().id);

  reexp_node->regex = p.p_base->regex_path();
  auto regex        = p.scr_info.nodes->get_as<ast::Path_Regex>(reexp_node->regex.get_node_id());

  std::string out_err;
  auto mod = compiler::COMPILER.modules.tools.build_module_from_path(p.scr_id, regex->path, regex->source, out_err);


  if (!mod) {
    p.add_error_tok(240, base_tok, out_err, "");
    return reexp_node->node_id;
  }

  p.get_current_module().port.reexport_item(reexp_node->node_id);

  return reexp_node->node_id;
}


ast::_gnid parser::Parser_Base::parse_extern()
{
  constexpr std::string_view hint = R"(define extern like: `extern "ABI" {...}`)";

  p.match(token::ETokenKind::EXTERN);

  auto ext_tok = p.peek(-1);

  parser_add_node(ext_node, Global_Extern, p.peek().id);
  ext_node->api =
      p.tok_to_str(p.expect(153, token::ETokenKind::L_TEXTUAL, "Expected literal string to define ABI.", hint).id);


  p.expect(9, token::ETokenKind::OPEN_BRACE, "Expected export begin scope '{' after import instruction.", hint);

  p.in_extern = true;
  p.enter_scope(*ext_node, "extern");

  if (p.match(token::ETokenKind::CLOSE_BRACE)) {
    p.in_extern = false;
    return ext_node->node_id;
  }

  while (!p.is_end()) {
    if (p.check_any({token::ETokenKind::IMPORT, token::ETokenKind::EXPORT})) {
      p.add_error_tok(10, p.peek(), "Illegal nested module export/import instruction.", hint);
    }

    ext_node->codeblock = p.p_loc->parse_codeblock();

    p.match(token::ETokenKind::SEMICOLON);
    if (p.match_field_separator(token::ETokenKind::S_END_OF_FILE, token::ETokenKind::CLOSE_BRACE)) break;
  }

  p.in_extern = false;
  p.exit_scope();

  return ext_node->node_id;
}

ast::_gnid parser::Parser_Base::parse_instruction()
{
  constexpr std::string_view hint =
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
  if (auto statement = p.p_state->parse_statement(true)) {
    return statement;
  } else if (p.check_any({token::ETokenKind::VAR, token::ETokenKind::LET, token::ETokenKind::CONST})
             && p.check_at(1, token::ETokenKind::OPEN_PAREN)) {
    auto tuple = p.p_loc->tuple_destructuring();
    return tuple;
  }
  // local variable + lambda
  else if (auto local = p.p_loc->parse_local(true)) {
    return local;
  }
  // del
  else if (p.check(token::ETokenKind::DEL)) {
    auto del = p.p_mem->del();
    return del;
  } else if (auto expr = p.p_expr->parse_expression()) {
    // assignation and operator assignment
    if (p.check_any(token::kAssignationTokens)) {
      auto assign = p.p_op->assignment(std::move(expr));

      return assign;
    }
    // call and sys_call
    else if (expr) {
      return expr;
    }
  }

  p.add_error_tok(11, p.peek(), "Unexpected instruction", hint);
  return BAD_NODE_ID;
}

ast::_gnid parser::Parser_Base::regex_path()
{
  constexpr std::string_view hint =
      R"(define regex path like: 
  - source: `std::` `src::` `pkg::` `bind::` `vend::` ``
  - path: `a::b::c`
  - multiple elements: `my_mod::{a, b, c}`
  - all elements: `my_mod::*`)";

  parser_add_node(regex, Path_Regex, p.peek().id);
  regex->source = script::EFileSource::relative;

  // import std:: @
  if (p.match(token::ETokenKind::AT) || p.match_val("std")) {
    p.expect(250, token::ETokenKind::STATIC_ACCESS, "Expected static access (a path) after a source specifier.", hint);
    regex->source = script::EFileSource::stdlib;
  }
  // import src:: $
  else if (p.match(token::ETokenKind::DOLLAR) || p.match_val("src")) {
    p.expect(250, token::ETokenKind::STATIC_ACCESS, "Expected static access (a path) after a source specifier.", hint);
    regex->source = script::EFileSource::src;
  }
  // import pkg:: #
  else if (p.match(token::ETokenKind::HASHTAG) || p.match_val("pkg")) {
    p.expect(250, token::ETokenKind::STATIC_ACCESS, "Expected static access (a path) after a source specifier.", hint);
    regex->source = script::EFileSource::pkg_lib;
  }
  // import bind:: ?
  else if (p.match(token::ETokenKind::INTERROGATIVE) || p.match_val("bind")) {
    p.expect(250, token::ETokenKind::STATIC_ACCESS, "Expected static access (a path) after a source specifier.", hint);
    regex->source = script::EFileSource::binding;
  } else {
    regex->source = script::EFileSource::relative;
  }

  while (!p.is_end()) {
    regex->path.push_back(p.parse_name("Expected identifier in regex path.", hint));

    if (p.match(token::ETokenKind::STATIC_ACCESS)) {
      if (p.match(token::ETokenKind::OPEN_BRACKETS)) {
        while (!p.is_end()) {
          regex->elements.push_back(p.parse_name("Expected identifier inside a regex path element selection.", hint));

          if (p.match_field_separator(token::ETokenKind::COMMA, token::ETokenKind::CLOSE_BRACKETS)) break;
        }

        break;
      }
    } else
      break;
  }

  return regex->node_id;
}


ast::_gnid parser::Parser_Base::identifier(bool p_no_qualified_id, bool p_keyword_allowed)
{
  constexpr std::string_view hint =
      R"(define identifier like:"
  - classic `name` -> name
  - with scope path `mod A { name }` -> A.name
  - with qualified id `A::B::C` -> A.B.C)";

  bool root_scope    = false;
  bool parent_scope  = false;
  bool current_scope = false;

  if (p.match(token::ETokenKind::STATIC_ACCESS)) {
    root_scope = true;
  } else if (p.match(token::ETokenKind::SUPER_MOD)) {
    parent_scope = true;
  } else if (p.match(token::ETokenKind::SELF)) {
    current_scope = true;
  }
  // it's a simple id with no path
  else if (p.peek(1).kind != token::ETokenKind::STATIC_ACCESS) {
    parser_add_node(id, ID, p.peek().id);
    if (!p_keyword_allowed)
      id->name = p.parse_name("", hint);
    else
      id->name = std::string(p.tok_to_str(p.next().id));

    return id->node_id;
  }

  // it's qualified id
  if (p_no_qualified_id) p.add_error_tok(113, p.peek(-1), "Unexpected qualified id.", hint);

  parser_add_node(id, ID_Qualified, p.peek().id);
  size_t count = 0;
  while (!p.is_end()) {
    id->path.push_back(std::string(p.tok_to_str(p.next().id)));

    p.match(token::ETokenKind::STATIC_ACCESS);

    // no more path : the last element is the name
    if (p.peek(1).kind != token::ETokenKind::STATIC_ACCESS) {
      id->name = std::string(p.tok_to_str(p.next().id));
      break;
    }

    count++;
    if (count > 12) {
      p.add_error(114, "Explicit path for identifier is too long (> " + std::to_string(12) + ")", hint);
      break;
    }
  }

  return id->node_id;
}

std::tuple<ast::_gnid, type::_id> parser::Parser_Base::identifier_typed()
{
  p.match_any({token::ETokenKind::TURBO_FISH, token::ETokenKind::OPEN_BRACE});

  parser_add_node(id_type, ID_Typed, p.peek(-2).id);

  auto ty_id = parser_type_factory.make_identifier(id_type->node_id, symbol::_id{});

  if (p.match(token::ETokenKind::CLOSE_BRACKETS)) return {id_type->node_id, ty_id};

  while (!p.is_end()) {
    id_type->generic_args.push_back(p.p_type->parse_type());

    if (p.match_field_separator(token::ETokenKind::COMMA, token::ETokenKind::CLOSE_BRACKETS)) break;
  }

  // p.sym_m->check_if_unresolved_extern_sym(*id_type.get());

  return {id_type->node_id, ty_id};
}
