
#include "parser_context.hpp"

#include <cassert>
#include <cstddef>
#include <stdexcept>
#include <string_view>

#include "ast/ast_declaration_global.hpp"
#include "nexus/ids.hpp"
#include "nexus/lexer/token.hpp"
#include "nexus/lexer/token_viewer.hpp"
#include "ast/ast_base.hpp"
#include "nexus/ast/ast.hpp"
#include "nexus/forward.hpp"
#include "nexus/module.hpp"
#include "nexus/metacode/metacode.hpp"
#include "nexus/scope.hpp"
#include "nexus/script.hpp"
#include "nexus/symbol.hpp"
#include "compiler/compiler.hpp"

#include "parser_base.hpp"
#include "parser_declaration_global.hpp"
#include "parser_declaration_cop.hpp"
#include "parser_declaration_local.hpp"
#include "parser_expression.hpp"
#include "parser_literal.hpp"
#include "parser_memory.hpp"
#include "parser_operation.hpp"
#include "parser_statement.hpp"
#include "parser_type.hpp"

#include "nexus/pipeline.hpp"


parser::Parser_Context::Parser_Context(script::_id p_scr_id)
  : scr_id(p_scr_id)
  , scr_info(compiler::COMPILER.pipeline.get_script(p_scr_id))
  , compilation_nodes(new ast::Arena())
  , meta(*scr_info.metacodes)
  , tok_v(new token::Viewer(compiler::COMPILER.pipeline.get_script(p_scr_id)))
  , p_cop(new Parser_Declaration_COP(*this))
  , p_decl(new Parser_Declaration(*this))
  , p_expr(new Parser_Expression(*this))
  , p_lit(new Parser_Literal(*this))
  , p_loc(new Parser_Declaration_Local(*this))
  , p_mem(new Parser_Memory(*this))
  , p_op(new Parser_Operator(*this))
  , p_state(new Parser_Statement(*this))
  , p_type(new Parser_Type(*this))
  , p_base(new Parser_Base(*this))
{
  tok_v->phase = compiler::EPhase::parser;

  auto root             = scr_info.nodes->add<ast::Root>();
  scr_info.root_node_id = root.get_gnid(scr_id);
  auto src              = script::EFileSource_to_str(scr_info.file_info.source);

  module::Module* parent_module = nullptr;
  switch (scr_info.file_info.source) {
  case script::EFileSource::src:        parent_module = &compiler::COMPILER.modules.src; break;
  case script::EFileSource::vendor_lib: parent_module = &compiler::COMPILER.modules.vendor; break;
  case script::EFileSource::stdlib:     parent_module = &compiler::COMPILER.modules.std; break;
  case script::EFileSource::pkg_lib:    parent_module = &compiler::COMPILER.modules.pkg; break;
  case script::EFileSource::binding:    parent_module = &compiler::COMPILER.modules.bind; break;
  case script::EFileSource::relative:   parent_module = &compiler::COMPILER.modules.src; break;
  }

  // init file module
  scr_info.module_id = compiler::COMPILER.modules.new_module(parent_module->id, module::Module::from_script(p_scr_id));
  auto& mod          = compiler::COMPILER.modules.get(scr_info.module_id);
  // init scope of file module
  mod.scp_id = compiler::COMPILER.scopes.new_scope(NO_ID, scope::Scope::from_module(scr_id, scr_info.module_id));

  current_module_id = scr_info.module_id;
}

parser::Parser_Context::~Parser_Context()
{
}

bool parser::Parser_Context::start_parsing()
{
  auto root_node    = scr_info.nodes->get_as<ast::Root>(scr_info.root_node_id.get_node_id());
  current_module_id = scr_info.module_id;
  auto& mod         = compiler::COMPILER.modules.get(scr_info.module_id);
  current_scope_id  = mod.get_scope().id;

  size_t errs = compiler::COMPILER.errors.size();

  try {
    while (!tok_v->is_end()) {
      auto line = p_decl->parse_declaration();
      if (line.get_node_id().value() == 0) return false;
      if (line) root_node->global_nodes.push_back(line);
      if (tok_v->match(token::ETokenKind::S_END_OF_FILE)) break;
    }
  } catch (const std::runtime_error& e) {
    // std::cerr << e.what() << std::endl; context.tokView.synchronize(); attempt_recovery();
    return false;
  }

  return errs == compiler::COMPILER.errors.size();
}

bool parser::is_gen_args(token::Viewer& p_tok_v)
{
  size_t originPos           = p_tok_v.position();
  bool   isGenArgsValid      = true;
  bool   isAfterGenArgsValid = false;
  size_t count               = 0;
  size_t nestedBrackets      = 1;
  // = 0 valid nested result; > 0 invalid nested result; begins at 1
  // because generic type args begins with < and must end by >

  // check genArgs
  while (!p_tok_v.is_end()) {
    bool tokValid = false;

    auto tok = p_tok_v.peek(count);
    for (auto tok_valid : ::token::k_args_generic_valid) {
      if (tok.kind == tok_valid) {
        if (tok.kind == ::token::ETokenKind::OPEN_BRACKETS) ++nestedBrackets;
        if (tok.kind == ::token::ETokenKind::CLOSE_BRACKETS) --nestedBrackets;
        tokValid = true;
        break;
      }
    }

    if (!tokValid) {
      isGenArgsValid = false;
      break;
    }

    // nested args are finish
    if (nestedBrackets == 0) break;

    count++;
  }

  // if is a valid generic args with correct nested brackets check next token
  // if is not a false positive generic args
  // (case of interval comparaison like a < b > 0)
  if (isGenArgsValid && nestedBrackets == 0) {
    auto tokAfterGenArgs = p_tok_v.peek(count + 1);

    if (tokAfterGenArgs.kind == token::ETokenKind::OPEN_PAREN)
      isAfterGenArgsValid = true;
    else {
      isAfterGenArgsValid = !(p_tok_v.check_any(::token::k_lit) || p_tok_v.check_any(token::k_operator)
                              || p_tok_v.check_any(token::k_op_comparison));
    }
  }

  const bool result = isGenArgsValid && isAfterGenArgsValid;
  if (result)
    p_tok_v.rewind(originPos);
  else
    p_tok_v.rewind(originPos - 1);

  // is an effective gen args ?
  return result;
}

bool parser::Parser_Context::metablock_contains(size_t file_pos, std::string_view s) const
{
  auto m_pos = meta.audit.get_metacode_from_pos(file_pos);
  return meta.audit.contains(m_pos->id, s);
}

bool parser::Parser_Context::metablock_contains(size_t file_pos, token::ETokenKind t) const
{
  auto m_pos = meta.audit.get_metacode_from_pos(file_pos);
  return meta.audit.contains(m_pos->id, t);
}


const metacode::Instruction*
parser::Parser_Context::get_instruction(size_t file_pos, std::initializer_list<std::string_view> pattern) const
{
  auto metacode = meta.audit.get_metacode_from_pos(file_pos);
  return meta.audit.get_instruction(metacode->id, pattern);
}

const metacode::Metacode* parser::Parser_Context::get_metacode(size_t                                  file_pos,
                                                               std::initializer_list<std::string_view> pattern) const
{
  auto metacode = meta.audit.get_metacode_from_pos(file_pos);
  return meta.audit.get_metacode(metacode->id, pattern);
}


void parser::Parser_Context::enter_scope(ast::Node& node, std::string_view debug_name)
{
  auto scp      = scope::Scope::from_node(node.node_id, debug_name);
  scp.module_id = current_module_id;

  current_scope_id = compiler::COMPILER.scopes.new_scope(current_scope_id, scp);
}
void parser::Parser_Context::exit_scope()
{
  auto parent = compiler::COMPILER.scopes.parent.find(current_scope_id);
  assert(parent != compiler::COMPILER.scopes.parent.end());
  current_scope_id = parent->second;
}


void parser::Parser_Context::enter_module(ast::Node& node, std::string_view debug_name)
{
  auto mod_lit      = module::Module::new_node_module(node.node_id, debug_name);
  current_module_id = compiler::COMPILER.modules.new_module(current_module_id, mod_lit);
  auto& mod         = compiler::COMPILER.modules.get(current_module_id);
  auto  scp         = scope::Scope::from_module(scr_id, current_module_id);
  current_scope_id  = compiler::COMPILER.scopes.new_scope(scope::_id(), scp);
}
void parser::Parser_Context::exit_module()
{
  auto parent = compiler::COMPILER.modules.parent.find(current_module_id);
  assert(parent != compiler::COMPILER.modules.parent.end());
  current_module_id = parent->second;
}

std::string_view parser::Parser_Context::tok_to_str(token::_id id) const
{
  return scr_info.file_info.tokens->audit.Token_to_str(id);
}

size_t parser::Parser_Context::tok_to_pos(token::_id id) const
{
  return scr_info.file_info.tokens->get(id).begin;
}


void parser::Parser_Context::attempt_recovery()
{
  /*
  // recovery loop case
  static Token lastokRecovered;
  if (lastokRecovered.span == tok_v->peek().span) {
    std::cerr << "\n--------------------- ! COMPILATION STOPPED ! -------------------" << std::endl;
    std::cerr << "  Compiler: Infinitive recovery loop detected!" << std::endl;
    std::cerr << "  Please check the code source." << std::endl;
    return;
  } else {
    lastokRecovered = tok_v->peek();
  }

  return;
  */
}

bool parser::Parser_Context::match_field_separator(token::ETokenKind separator = token::ETokenKind::COMMA,
                                                   token::ETokenKind end       = token::ETokenKind::CLOSE_BRACE)
{
  if (separator != token::ETokenKind::S_END_OF_FILE) {
    if (match(separator)) return false;
    if (match(end)) return true;
    tok_v->add_error(12, "Unexpected token '" + std::string(tok_to_str(tok_v->peek().id)) + "' in expression.",
                     "expected a separator '" + std::to_string(int(separator)) + "' or a ending '"
                         + std::to_string(int(end)) + "'");

  } else {
    if (match(end)) return true;
  }
  return false;
}

bool parser::Parser_Context::match_field_any_separator(token::ETokenKind p_separator = token::ETokenKind::COMMA,
                                                       std::initializer_list<token::ETokenKind> p_end = {
                                                           token::ETokenKind::CLOSE_BRACE})
{
  if (match(p_separator)) return false;
  if (tok_v->match_any(p_end)) return true;
  std::string sym_end;
  size_t      sym_count = 0;
  for (auto& elem : p_end) {
    sym_end += "'" + std::to_string(int(elem)) + "', ";
    if (++sym_count > 10) {
      sym_end += "\n";
      sym_count = 0;
    }
  }
  tok_v->add_error(13, "Unexpected token '" + std::string(tok_to_str(tok_v->peek().id)) + "' in expression.",
                   "expected a separator '" + std::to_string(int(p_separator)) + "' or a ending {" + sym_end + "}");
  return false;
}

std::string_view parser::Parser_Context::parse_name(std::string_view p_msg, std::string_view p_hint)
{
  constexpr std::string_view msg = "Expected identifier (classic name).";
  constexpr std::string_view hint =
      R"(define identifier (classic name) like:
  - rule `[a-zA-Z_][a-zA-Z0-9_]*`
  - first character is alphabetical or `_`
  - other character is alphanumeric or `_`)";

  const std::string_view final_msg  = p_msg.empty() ? msg : p_msg;
  const std::string_view final_hint = p_hint.empty() ? hint : p_hint;
  auto&                  tok        = tok_v->expect(777, token::ETokenKind::IDENTIFIER, final_msg, final_hint);

  return tok_to_str(tok.id);
}


token::Token& parser::Parser_Context::expect(ErrorCode code, token::ETokenKind tok, std::string_view msg,
                                             std::string_view hint)
{
  return tok_v->expect(code, tok, msg, hint);
}
token::Token& parser::Parser_Context::expect_any(ErrorCode code, const std::initializer_list<token::ETokenKind>& types,
                                                 std::string_view msg, std::string_view hint)
{
  return tok_v->expect_any(code, types, msg, hint);
}
token::Token& parser::Parser_Context::next()
{
  return tok_v->next();
}
bool parser::Parser_Context::is_end()
{
  return tok_v->is_end();
}
bool parser::Parser_Context::match(token::ETokenKind tok)
{
  return tok_v->match(tok);
}
bool parser::Parser_Context::match_any(std::initializer_list<token::ETokenKind> toks)
{
  return tok_v->match_any(toks);
}
bool parser::Parser_Context::check(token::ETokenKind tok) const
{
  return tok_v->check(tok);
}
bool parser::Parser_Context::check_any(std::initializer_list<token::ETokenKind> toks) const
{
  return tok_v->check_any(toks);
}
bool parser::Parser_Context::check_at(size_t offset, token::ETokenKind tok) const
{
  return tok_v->peek(offset).kind == tok;
}
bool parser::Parser_Context::check_val(std::string_view val) const
{
  return tok_v->check_val(val);
}
bool parser::Parser_Context::match_val(std::string_view val) const
{
  return tok_v->match_val(val);
}
token::Token& parser::Parser_Context::peek(size_t offset) const
{
  return tok_v->peek(offset);
}
void parser::Parser_Context::rewind(size_t pos)
{
  tok_v->rewind(pos);
}
symbol::_id parser::Parser_Context::add_symbol(ast::_id n_id)
{
  symbol::Symbol sym;

  sym.gnid      = ast::GNID_Factory::make_gnid(scr_id, n_id);
  sym.module_id = current_module_id;

  auto& mod      = compiler::COMPILER.modules.get(current_module_id);
  sym.visibility = mod.visibility;

  return compiler::COMPILER.symbols.add(sym);
}

void parser::Parser_Context::add_error(ErrorCode code, std::string_view msg, std::string_view hint) const
{
  static size_t count = 0;
  tok_v->add_error(code, msg, hint);
  if (count++ > MAX_ERRORS) throw std::runtime_error("Too many errors emitted. Parser aborted.");
}
void parser::Parser_Context::add_error_tok(ErrorCode code, const token::Token& tok, std::string_view msg,
                                           std::string_view hint) const
{
  static size_t count = 0;
  tok_v->add_error_tok(code, tok, msg, hint);
  if (count++ > MAX_ERRORS) throw std::runtime_error("Too many errors emitted. Parser aborted.");
}


module::Module& parser::Parser_Context::get_current_module() const
{
  return compiler::COMPILER.modules.get(current_module_id);
}
scope::Scope& parser::Parser_Context::get_current_scope() const
{
  return compiler::COMPILER.scopes.get(current_scope_id);
}