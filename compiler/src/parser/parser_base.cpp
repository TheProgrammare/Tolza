
#include "parser_base.hpp"

#include "ast/data.hpp"
#include "ast/definition.hpp"
#include "ast/definition/ast_base.hpp"
#include "ast/definition/ast_declaration_global.hpp"
#include "ast/definition/ast_declaration_local.hpp"
#include "ast/forward.hpp"
#include "compiler/compilation_unit.hpp"
#include "compiler/compiler.hpp"
#include "lexer/token_viewer.hpp"
#include "nexus/forward.hpp"
#include "nexus/ids.hpp"
#include "parser/parser_recover.hpp"
#include "parser_context.hpp"
#include "parser_declaration_global.hpp"
#include "parser_declaration_local.hpp"
#include "parser_expression.hpp"
#include "parser_memory.hpp"
#include "parser_operation.hpp"
#include "parser_statement.hpp"
#include "parser_type.hpp"
#include "pool/module.hpp"
#include "pool/scope.hpp"
#include "pool/token.hpp"
#include "pool/type.hpp"
#include "pool/unresolved.hpp"

#include <algorithm>
#include <stdexcept>
#include <string_view>
#include <vector>


parser::Parser_Base::Parser_Base(Parser_Context& p_ctx)
  : p(p_ctx)
{
}

parser::Parser_Base::~Parser_Base()
{
}

ast::ID parser::Parser_Base::parse_import()
{
  constexpr std::string_view hint =
      R"(define import module like:
  - import `import <name>[::*] as <alias>`
  - import user (default) `import src::<name>[::*] as <alias>`
  - import user from script file position`import self::<name>[::*] as <alias>`
  - import specific elements `import std::math{ sin, cos, tan } as math`
  - import standard `import std::<name>[::*] as <alias>`
  - import package `import pkg::<name>[::*] as <alias>`
  - import vendor `import ven::<name>[::*] as <alias>`
  - import external `import bind::<lang>::<lib> as <alias>`
  - export/import module only declared in the root scope)";

  (void)p.match(token::ETokenKind::IMPORT);

  auto& base_tok = p.peek(-1);

  auto& imp = p.add_get_node<ast::Import>(p.peek().tokid);

  imp.regex         = p.p_base->regex_path();
  const auto* regex = imp.regex.as<ast::Path_Regex>();
  assert(regex && "Illegal regex node type");

  if (regex->source == cu::EFileSource::binding) {
    (void)p.expect(254, token::ETokenKind::AS, "expected alias after a importation", "");
    p.tok_v->jump(p.tok_v->position() - 1);
  }

  if (p.match(token::ETokenKind::AS)) {
    imp.alias = p.parse_name("Expected alias name after 'as' keyword inside a import instruction.");
  }

  std::string out_err;
  auto        mod = module::build_module_from_path(p.cuid, regex->path, regex->source, out_err);
  if (!mod) {
    auto& tok = p.CU.file_info.tokens->get(imp.header.start_tokid);
    auto  err = Error_Diagnostic(p.cuid, 238, tok.begin, tok.begin + tok.length, compiler::EPhase::binder, out_err, "");
    COMPILER.add_error(err);
  }

  assert(mod && "invalid moduleid");

  const bool scp_r = p.get_current_scope().port.import_module(mod, imp.nodeid());
  assert(scp_r && "Importation failed");
#ifdef DEBUG
  for (size_t i = 0; i < p.scope_depth; ++i) std::print("│ ");
  std::println("├import: {}", imp.alias);
#endif

  p.CU.imports.insert_or_assign(imp.nodeid(), mod);

  return imp.nodeid();
}

ast::ID parser::Parser_Base::parse_export()
{
  constexpr std::string_view hint = "define export module like: `export {...}`";

  (void)p.match(token::ETokenKind::EXPORT);

  auto& exp_node = p.add_get_node<ast::Global_Export>(p.peek().tokid);

#ifdef DEBUG
  for (size_t i = 0; i < p.scope_depth; ++i) std::print("│ ");
  std::println("├set scope export");
#endif

  p.current_visibility = EVisibility::Cross_File_Scope;
  exp_node.codeblock   = p.p_decl->parse_codeblock_declaration(false, true);
  p.current_visibility = EVisibility::File_Scope;
  p.CU.node_export     = exp_node.nodeid();

#ifdef DEBUG
  for (size_t i = 0; i < p.scope_depth; ++i) std::print("│ ");
  std::println("├unset scope export");
#endif

  return exp_node.nodeid();
}

ast::ID parser::Parser_Base::parse_reexport()
{
  constexpr std::string_view hint = "define re-export module like: `reexport <path>`";

  (void)p.match(token::ETokenKind::REEXPORT);

  auto& base_tok = p.peek(0);


  auto& reexp = p.add_get_node<ast::Global_Reexport>(p.peek().tokid);

  reexp.regex       = p.p_base->regex_path();
  const auto* regex = reexp.regex.as<ast::Path_Regex>();

  std::string out_err;
  auto        mod = module::build_module_from_path(p.cuid, regex->path, regex->source, out_err);

  (void)p.expect(260, token::ETokenKind::AS, "Expected 'as' to specify an alias for the reexport", hint);
  reexp.alias = p.parse_name();

  if (!mod) {
    p.add_error_tok(240, base_tok, out_err, "");
    return reexp.nodeid();
  }

  const bool result = p.get_current_module().port.reexport_item(mod, reexp.nodeid());
#ifdef DEBUG
  for (size_t i = 0; i < p.scope_depth; ++i) std::print("│ ");
  std::println("├reexport: {}", reexp.alias);
#endif
  assert(result && "reexport failed");

  p.CU.imports.try_emplace(reexp.nodeid(), mod);

  return reexp.nodeid();
}


ast::ID parser::Parser_Base::parse_extern()
{
  constexpr std::string_view hint = R"(define extern like: `extern "ABI" {...}`)";

  (void)p.match(token::ETokenKind::EXTERN);

  auto& ext_tok = p.peek(-1);

  auto& ext_node = p.add_get_node<ast::Global_Extern>(p.peek().tokid);
  ext_node.abi =
      p.tok_to_str(p.expect(153, token::ETokenKind::L_TEXTUAL, "Expected literal string to define ABI.", hint).tokid);

#ifdef DEBUG
  for (size_t i = 0; i < p.scope_depth; ++i) std::print("│ ");
  std::println("├set scope extern");
#endif

  p.extern_abi       = ext_node.abi;
  ext_node.codeblock = p.p_decl->parse_codeblock_declaration(true, true);
  p.extern_abi.clear();

#ifdef DEBUG
  for (size_t i = 0; i < p.scope_depth; ++i) std::print("│ ");
  std::println("├unset scope extern");
#endif

  return ext_node.nodeid();
}

ast::ID parser::Parser_Base::parse_instruction()
{
  constexpr std::string_view hint =
      R"(define insutrction like:
  - statement `if/elif/else/match/while/do-while/loop/for/goto`
  - local variable declaration `let/var/const`
  - lambda declaration `lam ...`
  - assignation `left copy=/move= ...`
  - operation assignation `left +=/-=/*=//=/... ...`
  - function call `name()`
  - rule call `form_name::>rule_name()`
  - memory deletion `del pointer_name`)";

  // if elif else for ...
  if (auto statement = p.p_state->parse_statement(true)) return statement;

  if (p.check_any({token::ETokenKind::VAR, token::ETokenKind::LET, token::ETokenKind::CONST})
      && p.check_at(1, token::ETokenKind::L_PAREN)) {
    auto tuple = p.p_loc->tuple_destructuring();
    return tuple;
  }

  // local variable + lambda
  if (auto local = p.p_loc->parse_local(true)) {
    return local;
  }

  // del
  if (p.check(token::ETokenKind::DEL)) {
    auto del = p.p_mem->del();
    return del;
  }

  if (auto expr = p.p_expr->parse_expression()) {
    // assignation and operator assignment
    if (p.check_any(token::k_op_assign)) {
      auto assign = p.p_op->assignment(expr);

      return assign;
    }

    // call and rule_call
    return expr;
  }

  throw Parser_Exception(p, 11, p.peek(), "Unexpected instruction", hint);
}

ast::ID parser::Parser_Base::regex_path()
{
  constexpr std::string_view hint =
      R"(define regex path like: 
  - anchor: `std::` `src::` `pkg::` `bind::` `vend::` ``
  - context relative: `self::`
  - path: `a::b::c`
  - multiple elements: `my_mod::{a, b, c}`
  - all elements: `my_mod::*`)";

  auto& regex  = p.add_get_node<ast::Path_Regex>(p.peek().tokid);
  regex.source = cu::EFileSource::relative;

  // import std:: @
  if (p.match(token::ETokenKind::AT) || p.match_val("std")) {
    (void)p.expect(250, token::ETokenKind::STATIC_ACCESS, "Expected static access (a path) after a source specifier.",
                   hint);
    regex.source = cu::EFileSource::stdlib;
  }
  // import src:: $
  else if (p.match(token::ETokenKind::DOLLAR) || p.match_val("src")) {
    (void)p.expect(251, token::ETokenKind::STATIC_ACCESS, "Expected static access (a path) after a source specifier.",
                   hint);
    regex.source = cu::EFileSource::src;
  }
  // import pkg:: #
  else if (p.match(token::ETokenKind::HASHTAG) || p.match_val("pkg")) {
    (void)p.expect(252, token::ETokenKind::STATIC_ACCESS, "Expected static access (a path) after a source specifier.",
                   hint);
    regex.source = cu::EFileSource::pkg_lib;
  }
  // import bind:: ?
  else if (p.match(token::ETokenKind::INTERROGATIVE) || p.match_val("bind")) {
    (void)p.expect(253, token::ETokenKind::STATIC_ACCESS, "Expected static access (a path) after a source specifier.",
                   hint);
    regex.source = cu::EFileSource::binding;
  }
  // import self:: ~
  else if (p.match(token::ETokenKind::TILDE) || p.match_val("self")) {
    (void)p.expect(255, token::ETokenKind::STATIC_ACCESS, "Expected static access (a path) after a source specifier.",
                   hint);
    regex.source = cu::EFileSource::relative;
  } else {
    p.add_error(254, "Expected anchor point in regex path", hint);
  }

  while (!p.is_end()) {
    if (p.check(token::ETokenKind::IDENTIFIER)) {
      regex.path.emplace_back(p.parse_name("Expected identifier in regex path.", hint));
    } else {
      const auto& tok_str = p.peek().tokid.str();
      auto        it      = std::ranges::find_if(token::k_keywords, [&](const auto& pair) -> bool {
        const auto& name = pair.first;
        const auto& kind = pair.second;
        return tok_str == name;
      });

      if (it != token::k_keywords.end()) {
        regex.path.emplace_back(tok_str);
        (void)p.next();
      }
    }

    if (p.match(token::ETokenKind::L_CURLY)) {
      while (!p.is_end()) {
        regex.elements.emplace_back(p.parse_name("Expected identifier inside a regex path element selection.", hint));

        if (p.match_field_separator(token::ETokenKind::COMMA, token::ETokenKind::R_CURLY)) break;
      }

      break;
    }
    if (!p.match(token::ETokenKind::STATIC_ACCESS)) break;
  }

  return regex.nodeid();
}


ast::ID parser::Parser_Base::identifier(bool p_no_qualified_id, bool p_keyword_allowed)
{
  constexpr std::string_view hint =
      R"(define identifier like:"
  - classic `name` -> name
  - with scope path `mod A { name }` -> A.name
  - with qualified id `A::B::C` -> A.B.C)";

  EPathAnchor anchor = EPathAnchor::relative_self;

  if (p.match(token::ETokenKind::STATIC_ACCESS)) {
    anchor = EPathAnchor::relative_root;
  } else if (p.match(token::ETokenKind::SUPER_MOD)) {
    anchor = EPathAnchor::relative_super;
  } else if (p.match(token::ETokenKind::SELF)) {
    anchor = EPathAnchor::relative_self;
  }
  // it's a simple id with no path
  else if (p.peek(1).kind != token::ETokenKind::STATIC_ACCESS) {
    auto& id = p.add_get_node<ast::Symbol_Id>(p.peek().tokid);
    if (!p_keyword_allowed)
      id.name = p.parse_name("", hint);
    else
      id.name = std::string(p.tok_to_str(p.next().tokid));

    COMPILER.unresolved.add(id.nodeid());
    return id.nodeid();
  }

  // it's qualified id
  if (p_no_qualified_id) p.add_error_tok(113, p.peek(-1), "Unexpected qualified id.", hint);

  auto& id  = p.add_get_node<ast::Symbol_Qualified>(p.peek().tokid);
  id.anchor = anchor;

  size_t count = 0;
  while (!p.is_end()) {
    id.path.emplace_back(p.tok_to_str(p.next().tokid));

    (void)p.match(token::ETokenKind::STATIC_ACCESS);

    // no more path : the last element is the name
    if (p.peek(1).kind != token::ETokenKind::STATIC_ACCESS) {
      id.name = p.tok_to_str(p.next().tokid);
      break;
    }

    count++;
    if (count > 12) {
      p.add_error(114, std::format("Explicit path for identifier is too long (> {})", std::to_string(12)), hint);
      break;
    }
  }

  COMPILER.unresolved.add(id.nodeid());
  return id.nodeid();
}

std::tuple<ast::ID, type::ID> parser::Parser_Base::identifier_typed()
{
  (void)p.match_any({token::ETokenKind::TURBO_FISH, token::ETokenKind::L_CURLY});

  auto& id_type = p.add_get_node<ast::Symbol_Type>(p.peek(-2).tokid);

  auto tyid = parser_type_factory.make_identifier("__NONE__", id_type.nodeid(), definition::ID{});

  if (p.match(token::ETokenKind::R_ANGLE)) return {id_type.nodeid(), tyid};

  while (!p.is_end()) {
    id_type.generic_args.emplace_back(p.p_type->parse_type());

    if (p.match_field_separator(token::ETokenKind::COMMA, token::ETokenKind::R_ANGLE)) break;
  }

  // p.sym_m->check_if_unresolved_extern_sym(*id_type.get());

  return {id_type.nodeid(), tyid};
}


ast::Local_Parameter& parser::Parser_Base::inject_parameter(ast::ID parent_callable, size_t pos, std::string_view name,
                                                            ast::EPassMode passmode, type::ID tyid)
{
  auto& n           = p.add_get_node<ast::Local_Parameter>(p.peek().tokid);
  n.name            = name;
  n.passmode        = passmode;
  n.type            = tyid;
  n.parent_callable = parent_callable;
  n.position        = pos;

  (void)p.add_definition(n.nodeid());

  if (name == "self") {
    if (!p.p_extend->in_extend) p.add_error(266, "Unexpected 'self' special variable outside any extend.", "");

    p.p_extend->current_self = n.nodeid();
  } else if (name == "other") {
    if (!p.p_extend->in_extend) p.add_error(266, "Unexpected 'self' special variable outside any extend.", "");

    p.p_extend->current_other = n.nodeid();
  }

  return n;
}
ast::Local_Variable& parser::Parser_Base::inject_variable(std::string_view name, ast::EVariableKind kind, type::ID tyid,
                                                          ast::ID expr)
{
  auto& n      = p.add_get_node<ast::Local_Variable>(p.peek().tokid);
  n.name       = name;
  n.kind       = kind;
  n.type       = tyid;
  n.expression = expr;

  (void)p.add_definition(n.nodeid());

  return n;
}

ast::Local_Capability& parser::Parser_Base::inject_capability(std::string_view name, ast::ECapability capa,
                                                              type::ID tyid, ast::ID expr)
{
  auto& n      = p.add_get_node<ast::Local_Capability>(p.peek().tokid);
  n.name       = name;
  n.kind       = capa;
  n.type       = tyid;
  n.expression = expr;

  (void)p.add_definition(n.nodeid());

  return n;
}