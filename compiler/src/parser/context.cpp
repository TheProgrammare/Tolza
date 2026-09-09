
#include "parser/context.hpp"

#include "ast/forward.hpp"
#include "ast/node/base.hpp"
#include "ast/node/declaration_global.hpp"
#include "ast/pool.hpp"
#include "compiler/compilation_unit.hpp"
#include "compiler/compiler.hpp"
#include "compiler/file_info.hpp"
#include "compiler/io.hpp"
#include "id/nodeid.hpp"
#include "id/tokid.hpp"
#include "lexer/data.hpp"
#include "lexer/pool.hpp"
#include "lexer/token_viewer.hpp"
#include "metacode/pool.hpp"
#include "module/module.hpp"
#include "module/pool.hpp"
#include "nexus/forward.hpp"
#include "parser/base.hpp"
#include "parser/declaration_extension.hpp"
#include "parser/declaration_global.hpp"
#include "parser/declaration_local.hpp"
#include "parser/declaration_sfm.hpp"
#include "parser/expression.hpp"
#include "parser/literal.hpp"
#include "parser/operation.hpp"
#include "parser/recovery.hpp"
#include "parser/statement.hpp"
#include "parser/type.hpp"
#include "pool/node_to_def.hpp"
#include "scope/pool.hpp"
#include "scope/scope.hpp"
#include "type/pool.hpp"

#include <cassert>
#include <cstddef>
#include <format>
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <string_view>


parser::Parser_Context::Parser_Context(cu::ID _cuid)
  : cuid(_cuid)
  , CU(_cuid.get())
  , meta(*CU.metacodes)
  , tok_v(new token::Viewer(_cuid.get()))
  , p_extend(new Parser_Declaration_Extension(*this))
  , p_sfm(new Parser_Declaration_SFM(*this))
  , p_decl(new Parser_Declaration(*this))
  , p_expr(new Parser_Expression(*this))
  , p_lit(new Parser_Literal(*this))
  , p_loc(new Parser_Declaration_Local(*this))
  , p_op(new Parser_Operator(*this))
  , p_state(new Parser_Statement(*this))
  , p_type(new Parser_Type(*this))
  , p_base(new Parser_Base(*this))
{
  tok_v->phase = compiler::EPhase::parser;

  auto& mod = CU.modules->get_file_root();
  auto& scp = CU.scopes->get_file_root();

  current_modid = mod.modid;
  current_scpid = scp.scpid;
}

parser::Parser_Context::~Parser_Context()
{
}

template <ast::Generic T>
T& parser::Parser_Context::add_get_node(token::ID tokid)
{
  assert(tokid && "Invalid token id");
  assert(tokid.pos() < CU.file_info.data.size() && "Invalid token position");
  T& n                 = CU.ast->add_get<T>();
  n.header.start_tokid = tokid;
  n.header.scpid       = current_scpid;
  node_count++;
  return n;
}

bool parser::Parser_Context::start_parsing()
{
  auto* root_node = CU.ast->get_file_root();
  current_modid   = CU.modules->get_file_root().modid;
  current_scpid   = CU.scopes->get_file_root().scpid;

  size_t errs = COMPILER.errors.size();

  try {
    while (!tok_v->is_end()) {
      const auto line = p_decl->parse_declaration();
      if (!line) return false;
      if (line) root_node->global_nodes.emplace_back(line);
      if (tok_v->match(token::ETokenKind::S_END_OF_FILE)) break;
    }
  } catch (const Parser_Exception& e) {
    return error::recover(*this);
  } catch (const std::runtime_error& e) {
    IO::println(stderr, IO_PASS::parser, "{}", e.what());
    assert(false && "Invalid throw error");
  } catch (...) {
    assert(false && "Invalid throw error");
  }

  return errs == COMPILER.errors.size();

  CU.ast->freeze         = true;
  CU.types->freeze       = true;
  CU.definitions->freeze = true;
  CU.modules->freeze     = true;
  CU.scopes->freeze      = true;
}

bool parser::is_gen_args(token::Viewer& tok_v)
{
  size_t originPos           = tok_v.position();
  bool   isGenArgsValid      = true;
  bool   isAfterGenArgsValid = false;
  size_t count               = 0;
  size_t nestedBrackets      = 1;
  // = 0 valid nested result; > 0 invalid nested result; begins at 1
  // because generic type args begins with < and must end by >

  // check genArgs
  while (!tok_v.is_end()) {
    bool tokValid = false;

    const auto& tok = tok_v.peek(count);
    for (const auto tok_valid : ::token::k_args_generic_valid) {
      if (tok.kind == tok_valid) {
        if (tok.kind == ::token::ETokenKind::L_ANGLE) ++nestedBrackets;
        if (tok.kind == ::token::ETokenKind::R_ANGLE) --nestedBrackets;
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
    const auto& tokAfterGenArgs = tok_v.peek(count + 1);

    if (tokAfterGenArgs.kind == token::ETokenKind::L_PAREN)
      isAfterGenArgsValid = true;
    else {
      isAfterGenArgsValid = !(tok_v.check_any(::token::k_lit) || tok_v.check_any(token::k_operator)
                              || tok_v.check_any(token::k_op_comparison));
    }
  }

  const bool result = isGenArgsValid && isAfterGenArgsValid;
  if (result)
    tok_v.rewind(originPos);
  else
    tok_v.rewind(originPos - 1);

  // is an effective gen args ?
  return result;
}

bool parser::Parser_Context::metablock_contains(size_t file_pos, std::string_view s) const
{
  const auto* m_pos = meta.audit.get_metacode_from_pos(file_pos);
  return meta.audit.contains(m_pos->metaid, s);
}

bool parser::Parser_Context::metablock_contains(size_t file_pos, token::ETokenKind t) const
{
  const auto* m_pos = meta.audit.get_metacode_from_pos(file_pos);
  return meta.audit.contains(m_pos->metaid, t);
}


const metacode::Instruction*
parser::Parser_Context::get_instruction(size_t file_pos, std::initializer_list<std::string_view> pattern) const
{
  const auto* metacode = meta.audit.get_metacode_from_pos(file_pos);
  return meta.audit.get_instruction(metacode->metaid, pattern);
}

const metacode::MetacodeHeader*
parser::Parser_Context::get_metacode(size_t file_pos, std::initializer_list<std::string_view> pattern) const
{
  const auto* metacode = meta.audit.get_metacode_from_pos(file_pos);
  return meta.audit.get_metacode(metacode->metaid, pattern);
}


void parser::Parser_Context::enter_scope(ast::ID nodeid, std::string_view debug_name)
{
  auto scp = scope::Scope{
      .scpid      = nodeid.scope(),
      .debug_name = std::string(debug_name),
      .modid      = current_modid,
  };

  scope_depth++;

  current_scpid = CU.scopes->add(current_scpid, scp);
}
void parser::Parser_Context::exit_scope()
{
  scope_depth--;
  assert(scope_depth >= 0 && "Too much exit scope");

  current_scpid = current_scpid.parent();
}


void parser::Parser_Context::enter_module(ast::ID nodeid, std::string_view debug_name)
{
  auto mod      = module::Module::new_node_module(nodeid, debug_name);
  current_modid = CU.modules->add(current_modid, mod);

  enter_scope(nodeid, debug_name);
}
void parser::Parser_Context::exit_module()
{
  current_modid = current_modid.parent();
  exit_scope();
}

std::string_view parser::Parser_Context::tok_to_str(token::ID id) const
{
  return CU.file_info.tokens->audit.Token_to_str(id);
}

size_t parser::Parser_Context::tok_to_pos(token::ID id) const
{
  return CU.file_info.tokens->get(id).begin;
}


bool parser::Parser_Context::match_field_separator(token::ETokenKind separator = token::ETokenKind::COMMA,
                                                   token::ETokenKind end       = token::ETokenKind::R_CURLY) const
{
  if (separator != token::ETokenKind::S_END_OF_FILE) {
    if (match(separator)) return false;
    if (match(end)) return true;
    tok_v->add_error(12, std::format("Unexpected token '{}' in expression.", tok_to_str(tok_v->peek().tokid)),
                     std::format("expected a separator '{}' or a ending '{}'", token::ETokenKind_to_str(separator),
                                 token::ETokenKind_to_str(end)));

  } else {
    if (match(end)) return true;
  }
  return false;
}

bool parser::Parser_Context::match_field_any_separator(token::ETokenKind p_separator = token::ETokenKind::COMMA,
                                                       std::initializer_list<token::ETokenKind> p_end = {
                                                           token::ETokenKind::R_CURLY}) const
{
  if (match(p_separator)) return false;
  if (tok_v->match_any(p_end)) return true;
  std::string sym_end;
  size_t      sym_count = 0;
  for (const auto& elem : p_end) {
    sym_end += std::format("'{}', ", token::ETokenKind_to_str(elem));
    if (++sym_count > 10) {
      sym_end += "\n";
      sym_count = 0;
    }
  }
  tok_v->add_error(
      13, std::format("Unexpected token '{}' in expression.", tok_to_str(tok_v->peek().tokid)),
      std::format("expected a separator '{}' or a ending {}", token::ETokenKind_to_str(p_separator), sym_end));
  return false;
}

std::string parser::Parser_Context::parse_name(std::string_view p_msg, std::string_view p_hint) const
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

  return std::string(tok_to_str(tok.tokid));
}


token::Token& parser::Parser_Context::expect(ErrorCode code, token::ETokenKind tok, std::string_view msg,
                                             std::string_view hint) const
{
  return tok_v->expect(code, tok, msg, hint);
}
token::Token& parser::Parser_Context::expect_any(ErrorCode code, const std::initializer_list<token::ETokenKind>& types,
                                                 std::string_view msg, std::string_view hint) const
{
  return tok_v->expect_any(code, types, msg, hint);
}
token::Token& parser::Parser_Context::next() const
{
  return tok_v->next();
}
bool parser::Parser_Context::is_end() const
{
  return tok_v->is_end();
}
bool parser::Parser_Context::match(token::ETokenKind tok) const
{
  return tok_v->match(tok);
}
bool parser::Parser_Context::match_any(std::initializer_list<token::ETokenKind> toks) const
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
bool parser::Parser_Context::check_chain(std::initializer_list<token::ETokenKind> l) const
{
  return tok_v->check_chain(l);
}
bool parser::Parser_Context::match_chain(std::initializer_list<token::ETokenKind> l) const
{
  return tok_v->match_chain(l);
}
token::Token& parser::Parser_Context::peek(size_t offset) const
{
  return tok_v->peek(offset);
}
void parser::Parser_Context::rewind(size_t pos) const
{
  tok_v->rewind(pos);
}
definition::ID parser::Parser_Context::add_definition(ast::ID nodeid)
{
  const auto defid = CU.definitions->add(nodeid, current_modid.get().visibility);

  assert(current_scpid && "Invalid scope");

  const bool result = get_current_scope().items.add_definition(defid);
  assert(result && "Symbol integration failed");

  return defid;
}

void parser::Parser_Context::add_error(ErrorCode code, std::string_view msg, std::string_view hint) const
{
  static size_t count = 0;
  tok_v->add_error(code, msg, hint);
}
void parser::Parser_Context::add_error_tok(ErrorCode code, const token::Token& tok, std::string_view msg,
                                           std::string_view hint) const
{
  static size_t count = 0;
  tok_v->add_error_tok(code, tok, msg, hint);
}


module::Module& parser::Parser_Context::get_current_module()
{
  return current_modid.get();
}
scope::Scope& parser::Parser_Context::get_current_scope()
{
  return current_scpid.get();
}


/*
 * =============================================================================
 *  Template Instanciation Section
 * =============================================================================
 * To avoid any massive inclusion in headers and keep easy identifier usage
 * -----------------------------------------------------------------------------
 */


#include "ast/node/base.hpp"
#include "ast/node/declaration_extension.hpp"
#include "ast/node/declaration_global.hpp"
#include "ast/node/declaration_local.hpp"
#include "ast/node/declaration_sfm.hpp"
#include "ast/node/expression.hpp"
#include "ast/node/generic.hpp"
#include "ast/node/literal.hpp"
#include "ast/node/operation.hpp"
#include "ast/node/statement.hpp"


#define AST_ADD_NODE_INSTANCE(T) template T& parser::Parser_Context::add_get_node<T>(token::ID tokid);


AST_ADD_NODE_INSTANCE(ast::Unknown)
AST_ADD_NODE_INSTANCE(ast::Symbol_Id)
AST_ADD_NODE_INSTANCE(ast::Symbol_Qualified)
AST_ADD_NODE_INSTANCE(ast::Symbol_Type)
AST_ADD_NODE_INSTANCE(ast::Path_Regex)
AST_ADD_NODE_INSTANCE(ast::Root)
AST_ADD_NODE_INSTANCE(ast::Import)
AST_ADD_NODE_INSTANCE(ast::Global_Variable)
AST_ADD_NODE_INSTANCE(ast::Global_Function)
AST_ADD_NODE_INSTANCE(ast::Call_Contract)
AST_ADD_NODE_INSTANCE(ast::Global_Extend_Fn)
AST_ADD_NODE_INSTANCE(ast::Global_Extend_Cast)
AST_ADD_NODE_INSTANCE(ast::Global_Extend_Op_Bin)
AST_ADD_NODE_INSTANCE(ast::Global_Extend_Op_Un)
AST_ADD_NODE_INSTANCE(ast::Global_Extend_Op_Subscript)
AST_ADD_NODE_INSTANCE(ast::Global_Extend_Op_Transfert)
AST_ADD_NODE_INSTANCE(ast::Global_Extend_Op_Other)
AST_ADD_NODE_INSTANCE(ast::Global_Module)
AST_ADD_NODE_INSTANCE(ast::Global_Extern)
AST_ADD_NODE_INSTANCE(ast::Global_Export)
AST_ADD_NODE_INSTANCE(ast::Global_Reexport)
AST_ADD_NODE_INSTANCE(ast::Global_Enum)
AST_ADD_NODE_INSTANCE(ast::Global_Flag)
AST_ADD_NODE_INSTANCE(ast::Global_Union)
AST_ADD_NODE_INSTANCE(ast::Global_Alias_Type)
AST_ADD_NODE_INSTANCE(ast::Global_Alias_Module)
AST_ADD_NODE_INSTANCE(ast::Global_Generic)
AST_ADD_NODE_INSTANCE(ast::Enum_Field)
AST_ADD_NODE_INSTANCE(ast::Flag_Field)
AST_ADD_NODE_INSTANCE(ast::Union_Field)
AST_ADD_NODE_INSTANCE(ast::CodeBlock)
AST_ADD_NODE_INSTANCE(ast::Local_Lambda)
AST_ADD_NODE_INSTANCE(ast::Local_Lambda_Capture)
AST_ADD_NODE_INSTANCE(ast::Local_Parameter)
AST_ADD_NODE_INSTANCE(ast::Local_Gen_Param_Elem)
AST_ADD_NODE_INSTANCE(ast::Local_Gen_Params)
AST_ADD_NODE_INSTANCE(ast::Local_Pattern_Element)
AST_ADD_NODE_INSTANCE(ast::Local_Pattern_Enum)
AST_ADD_NODE_INSTANCE(ast::Local_Pattern_Tuple)
AST_ADD_NODE_INSTANCE(ast::Local_Pattern_Form)
AST_ADD_NODE_INSTANCE(ast::Local_Pattern_Rule_Facet)
AST_ADD_NODE_INSTANCE(ast::Local_Pattern_Facet)
AST_ADD_NODE_INSTANCE(ast::Local_Binding)
AST_ADD_NODE_INSTANCE(ast::Local_Tuple_Destructuring)
AST_ADD_NODE_INSTANCE(ast::Local_Variable)
AST_ADD_NODE_INSTANCE(ast::Local_Capability)
AST_ADD_NODE_INSTANCE(ast::SFM_Facet)
AST_ADD_NODE_INSTANCE(ast::SFM_View)
AST_ADD_NODE_INSTANCE(ast::SFM_Form)
AST_ADD_NODE_INSTANCE(ast::SFM_Rule)
AST_ADD_NODE_INSTANCE(ast::SFM_Facet_Field)
AST_ADD_NODE_INSTANCE(ast::SFM_Rule_Case)
AST_ADD_NODE_INSTANCE(ast::Generic_Type)
AST_ADD_NODE_INSTANCE(ast::Generic_Cast)
AST_ADD_NODE_INSTANCE(ast::Generic_Op)
AST_ADD_NODE_INSTANCE(ast::Generic_View)
AST_ADD_NODE_INSTANCE(ast::Generic_Facet)
AST_ADD_NODE_INSTANCE(ast::Generic_Extension)
AST_ADD_NODE_INSTANCE(ast::Generic_Rule)
AST_ADD_NODE_INSTANCE(ast::Literal_Boolean)
AST_ADD_NODE_INSTANCE(ast::Literal_NullPtr)
AST_ADD_NODE_INSTANCE(ast::Literal_Integral)
AST_ADD_NODE_INSTANCE(ast::Literal_Fixed_Point)
AST_ADD_NODE_INSTANCE(ast::Literal_Floating_Point)
AST_ADD_NODE_INSTANCE(ast::Literal_Cune)
AST_ADD_NODE_INSTANCE(ast::Literal_Rune)
AST_ADD_NODE_INSTANCE(ast::Literal_Text_Pure)
AST_ADD_NODE_INSTANCE(ast::Literal_Text_Interpolation)
AST_ADD_NODE_INSTANCE(ast::Literal_Textual_Format)
AST_ADD_NODE_INSTANCE(ast::Literal_Format_Specifier)
AST_ADD_NODE_INSTANCE(ast::Literal_Table)
AST_ADD_NODE_INSTANCE(ast::Literal_Tuple)
AST_ADD_NODE_INSTANCE(ast::Literal_Range)
AST_ADD_NODE_INSTANCE(ast::Literal_Record)
AST_ADD_NODE_INSTANCE(ast::Expression_If_Ternary)
AST_ADD_NODE_INSTANCE(ast::Expression_Member_Access)
AST_ADD_NODE_INSTANCE(ast::Expression_Self)
AST_ADD_NODE_INSTANCE(ast::Expression_Other)
AST_ADD_NODE_INSTANCE(ast::Expression_Invocation)
AST_ADD_NODE_INSTANCE(ast::Expression_Invocation_Arg)
AST_ADD_NODE_INSTANCE(ast::Expression_Invocation_Rule)
AST_ADD_NODE_INSTANCE(ast::Expression_Invocation_Extend)
AST_ADD_NODE_INSTANCE(ast::Expression_Table_Access)
AST_ADD_NODE_INSTANCE(ast::Expression_New_Ptr)
AST_ADD_NODE_INSTANCE(ast::Statement_If)
AST_ADD_NODE_INSTANCE(ast::Statement_For)
AST_ADD_NODE_INSTANCE(ast::Statement_Loop)
AST_ADD_NODE_INSTANCE(ast::Statement_While)
AST_ADD_NODE_INSTANCE(ast::Statement_GoTo)
AST_ADD_NODE_INSTANCE(ast::Statement_GoTo_Label)
AST_ADD_NODE_INSTANCE(ast::Statement_Return)
AST_ADD_NODE_INSTANCE(ast::Statement_Break)
AST_ADD_NODE_INSTANCE(ast::Statement_Continue)
AST_ADD_NODE_INSTANCE(ast::Statement_Match)
AST_ADD_NODE_INSTANCE(ast::Statement_Match_Case)
AST_ADD_NODE_INSTANCE(ast::Operation_Cast_As)
AST_ADD_NODE_INSTANCE(ast::Operation_Is)
AST_ADD_NODE_INSTANCE(ast::Operation_In)
AST_ADD_NODE_INSTANCE(ast::Operation_Transfert)
AST_ADD_NODE_INSTANCE(ast::Operation_Binary)
AST_ADD_NODE_INSTANCE(ast::Operation_Unary)
AST_ADD_NODE_INSTANCE(ast::Operation_Interval)
AST_ADD_NODE_INSTANCE(ast::Operation_Mem)

#undef AST_ADD_NODE_INSTANCE
