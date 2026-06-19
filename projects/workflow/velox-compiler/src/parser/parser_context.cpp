
#include "parser_context.hpp"

#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

#include "Neargye/magic_enum.hpp"
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
#include "compiler/compilation_unit.hpp"
#include "nexus/symbol.hpp"
#include "compiler/compiler.hpp"

#include "nexus/type/type.hpp"
#include "parser/parser_declaration_extension.hpp"
#include "parser_base.hpp"
#include "parser_declaration_global.hpp"
#include "parser_declaration_sfm.hpp"
#include "parser_declaration_local.hpp"
#include "parser_expression.hpp"
#include "parser_literal.hpp"
#include "parser_memory.hpp"
#include "parser_operation.hpp"
#include "parser_statement.hpp"
#include "parser_type.hpp"


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
  , p_mem(new Parser_Memory(*this))
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

template <typename T>
T& parser::Parser_Context::add_get_node(token::ID tokid) noexcept
{
  static_assert(std::is_base_of_v<ast::Node, T>, "The node type must inherit from ast::Node");
  T& n            = CU.nodes->add_get<T>();
  n.node_token_id = tokid;
  n.scpid         = current_scpid;
  node_count++;
  return n;
}

bool parser::Parser_Context::start_parsing()
{
  auto* root_node = CU.nodes->get_file_root();
  current_modid   = CU.modules->get_file_root().modid;
  current_scpid   = CU.scopes->get_file_root().scpid;

  size_t errs = compiler::COMPILER.errors.size();

  try {
    while (!tok_v->is_end()) {
      const auto line = p_decl->parse_declaration();
      if (!line) return false;
      if (line) root_node->global_nodes.emplace_back(line);
      if (tok_v->match(token::ETokenKind::S_END_OF_FILE)) break;
    }
  } catch (const std::runtime_error& e) {
    // std::cerr << e.what() << "\n"; /*endl*/ context.tokView.synchronize(); attempt_recovery();
    return false;
  }

  return errs == compiler::COMPILER.errors.size();

  CU.nodes->freeze   = true;
  CU.types->freeze   = true;
  CU.symbols->freeze = true;
  CU.modules->freeze = true;
  CU.scopes->freeze  = true;
}

bool parser::is_gen_args(token::Viewer& tok_v) noexcept
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

const metacode::Metacode* parser::Parser_Context::get_metacode(size_t                                  file_pos,
                                                               std::initializer_list<std::string_view> pattern) const
{
  const auto* metacode = meta.audit.get_metacode_from_pos(file_pos);
  return meta.audit.get_metacode(metacode->metaid, pattern);
}


void parser::Parser_Context::enter_scope(ast::Node& node, std::string_view debug_name)
{
  const auto scp = scope::Scope{
      .scpid      = node.scpid,
      .debug_name = std::string(debug_name),
      .modid      = current_modid,
  };

  scope_depth++;
#ifdef DEBUG
  for (size_t i = 0; i < scope_depth; ++i) std::cout << "│ ";
  std::cout << "┌scope: " << debug_name << "\n";
#endif

  current_scpid = CU.scopes->add(current_scpid, scp);
}
void parser::Parser_Context::exit_scope()
{
#ifdef DEBUG
  for (size_t i = 0; i < scope_depth; ++i) std::cout << "│ ";
  std::cout << "└end\n";
#endif
  scope_depth--;
  assert(scope_depth >= 0 && "Too much exit scope");

  current_scpid = current_scpid.parent();
}


void parser::Parser_Context::enter_module(ast::Node& node, std::string_view debug_name)
{
  const auto mod = module::Module::new_node_module(node.nodeid, debug_name);
  current_modid  = CU.modules->add(current_modid, mod);

  enter_scope(node, debug_name);
}
void parser::Parser_Context::exit_module()
{
  current_modid = current_modid.parent();
  exit_scope();
}

std::string_view parser::Parser_Context::tok_to_str(token::ID id) const noexcept
{
  return CU.file_info.tokens->audit.Token_to_str(id);
}

size_t parser::Parser_Context::tok_to_pos(token::ID id) const noexcept
{
  return CU.file_info.tokens->get(id).begin;
}


void parser::Parser_Context::attempt_recovery()
{
  /*
  // recovery loop case
  static Token lastokRecovered;
  if (lastokRecovered.span == tok_v->peek().span) {
    std::cerr << "\n--------------------- ! COMPILATION STOPPED ! -------------------" << "\n"; // endl
    std::cerr << "  Compiler: Infinitive recovery loop detected!"
                   << "\n"; // endl
    std::cerr
      << "  Please check the code source."
      << "\n"; // endl
  return;
}
else
{
  lastokRecovered = tok_v->peek();
}

return;
*/
}

bool parser::Parser_Context::match_field_separator(token::ETokenKind separator = token::ETokenKind::COMMA,
                                                   token::ETokenKind end = token::ETokenKind::R_CURLY) const noexcept
{
  if (separator != token::ETokenKind::S_END_OF_FILE) {
    if (match(separator)) return false;
    if (match(end)) return true;
    tok_v->add_error(12, "Unexpected token '" + std::string(tok_to_str(tok_v->peek().tokid)) + "' in expression.",
                     "expected a separator '" + std::to_string(int(separator)) + "' or a ending '"
                         + std::to_string(int(end)) + "'");

  } else {
    if (match(end)) return true;
  }
  return false;
}

bool parser::Parser_Context::match_field_any_separator(token::ETokenKind p_separator = token::ETokenKind::COMMA,
                                                       std::initializer_list<token::ETokenKind> p_end = {
                                                           token::ETokenKind::R_CURLY}) const noexcept
{
  if (match(p_separator)) return false;
  if (tok_v->match_any(p_end)) return true;
  std::string sym_end;
  size_t      sym_count = 0;
  for (const auto& elem : p_end) {
    sym_end += "'" + std::to_string(int(elem)) + "', ";
    if (++sym_count > 10) {
      sym_end += "\n";
      sym_count = 0;
    }
  }
  tok_v->add_error(13, "Unexpected token '" + std::string(tok_to_str(tok_v->peek().tokid)) + "' in expression.",
                   "expected a separator '" + std::to_string(int(p_separator)) + "' or a ending {" + sym_end + "}");
  return false;
}

std::string parser::Parser_Context::parse_name(std::string_view p_msg, std::string_view p_hint) const noexcept
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
                                             std::string_view hint) const noexcept
{
  return tok_v->expect(code, tok, msg, hint);
}
token::Token& parser::Parser_Context::expect_any(ErrorCode code, const std::initializer_list<token::ETokenKind>& types,
                                                 std::string_view msg, std::string_view hint) const noexcept
{
  return tok_v->expect_any(code, types, msg, hint);
}
token::Token& parser::Parser_Context::next() const noexcept
{
  return tok_v->next();
}
bool parser::Parser_Context::is_end() const noexcept
{
  return tok_v->is_end();
}
bool parser::Parser_Context::match(token::ETokenKind tok) const noexcept
{
  return tok_v->match(tok);
}
bool parser::Parser_Context::match_any(std::initializer_list<token::ETokenKind> toks) const noexcept
{
  return tok_v->match_any(toks);
}
bool parser::Parser_Context::check(token::ETokenKind tok) const noexcept
{
  return tok_v->check(tok);
}
bool parser::Parser_Context::check_any(std::initializer_list<token::ETokenKind> toks) const noexcept
{
  return tok_v->check_any(toks);
}
bool parser::Parser_Context::check_at(size_t offset, token::ETokenKind tok) const noexcept
{
  return tok_v->peek(offset).kind == tok;
}
bool parser::Parser_Context::check_val(std::string_view val) const noexcept
{
  return tok_v->check_val(val);
}
bool parser::Parser_Context::match_val(std::string_view val) const noexcept
{
  return tok_v->match_val(val);
}
bool parser::Parser_Context::check_chain(std::initializer_list<token::ETokenKind> l) const noexcept
{
  return tok_v->check_chain(l);
}
bool parser::Parser_Context::match_chain(std::initializer_list<token::ETokenKind> l) const noexcept
{
  return tok_v->match_chain(l);
}
token::Token& parser::Parser_Context::peek(size_t offset) const noexcept
{
  return tok_v->peek(offset);
}
void parser::Parser_Context::rewind(size_t pos) const noexcept
{
  tok_v->rewind(pos);
}
symbol::ID parser::Parser_Context::add_symbol(ast::ID nodeid)
{
  symbol::Symbol sym{
      .nodeid     = nodeid,
      .visibility = current_modid.get().visibility,
  };

  const auto symid = CU.symbols->add(sym);

#ifdef DEBUG
  for (size_t i = 0; i < scope_depth; ++i) std::cout << "│ ";
  std::cout << "├sym: " << magic_enum::enum_name(nodeid.get()->kind()) << " : " << sym.get_name() << "\n";
#endif
  assert(current_scpid && "Invalid scope");

  const bool result = get_current_scope().items.add_symbol(symid);
  assert(result && "Symbol integration failed");

  return symid;
}

void parser::Parser_Context::add_error(ErrorCode code, std::string_view msg, std::string_view hint) const
{
  static size_t count = 0;
  tok_v->add_error(code, msg, hint);
  if (count++ > MAX_ERRORS) throw std::runtime_error("Too many errors emitted. Parser aborted.");

  common::compiler::DEBUG_VELOX_ICE(msg);
}
void parser::Parser_Context::add_error_tok(ErrorCode code, const token::Token& tok, std::string_view msg,
                                           std::string_view hint) const
{
  static size_t count = 0;
  tok_v->add_error_tok(code, tok, msg, hint);
  if (count++ > MAX_ERRORS) throw std::runtime_error("Too many errors emitted. Parser aborted.");

  common::compiler::DEBUG_VELOX_ICE(msg);
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


#include "ast/ast_base.hpp"
#include "ast/ast_declaration_extension.hpp"
#include "ast/ast_declaration_sfm.hpp"
#include "ast/ast_declaration_global.hpp"
#include "ast/ast_declaration_local.hpp"
#include "ast/ast_expression.hpp"
#include "ast/ast_generic.hpp"
#include "ast/ast_literal.hpp"
#include "ast/ast_memory.hpp"
#include "ast/ast_operation.hpp"
#include "ast/ast_statement.hpp"


#define AST_ADD_NODE_INSTANCE(T) template T& parser::Parser_Context::add_get_node<T>(token::ID tokid) noexcept;


AST_ADD_NODE_INSTANCE(ast::Unknown)
AST_ADD_NODE_INSTANCE(ast::Identifier)
AST_ADD_NODE_INSTANCE(ast::ID_Qualified)
AST_ADD_NODE_INSTANCE(ast::ID_Typed)
AST_ADD_NODE_INSTANCE(ast::Path_Regex)
AST_ADD_NODE_INSTANCE(ast::Root)
AST_ADD_NODE_INSTANCE(ast::Import)
AST_ADD_NODE_INSTANCE(ast::Global_Variable)
AST_ADD_NODE_INSTANCE(ast::Global_Function)
AST_ADD_NODE_INSTANCE(ast::Global_Extend_Fn)
AST_ADD_NODE_INSTANCE(ast::Global_Extend_Cast)
AST_ADD_NODE_INSTANCE(ast::Global_Extend_Op_Bin)
AST_ADD_NODE_INSTANCE(ast::Global_Extend_Op_Un)
AST_ADD_NODE_INSTANCE(ast::Global_Extend_Op_Access)
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
AST_ADD_NODE_INSTANCE(ast::Literal_Table_Population)
AST_ADD_NODE_INSTANCE(ast::Literal_Map)
AST_ADD_NODE_INSTANCE(ast::Literal_Tuple)
AST_ADD_NODE_INSTANCE(ast::Literal_Range)
AST_ADD_NODE_INSTANCE(ast::Literal_Iterator)
AST_ADD_NODE_INSTANCE(ast::Literal_Enum)
AST_ADD_NODE_INSTANCE(ast::Literal_Structured_Data)
AST_ADD_NODE_INSTANCE(ast::Literal_Form)
AST_ADD_NODE_INSTANCE(ast::Expression_If_Ternary)
AST_ADD_NODE_INSTANCE(ast::Expression_Member_Access)
AST_ADD_NODE_INSTANCE(ast::Expression_Self)
AST_ADD_NODE_INSTANCE(ast::Expression_Other)
AST_ADD_NODE_INSTANCE(ast::Expression_Call)
AST_ADD_NODE_INSTANCE(ast::Expression_Call_Argument)
AST_ADD_NODE_INSTANCE(ast::Expression_Call_Rule)
AST_ADD_NODE_INSTANCE(ast::Expression_Call_Pipe)
AST_ADD_NODE_INSTANCE(ast::Expression_Table_Access)
AST_ADD_NODE_INSTANCE(ast::Expression_Ptr_Val)
AST_ADD_NODE_INSTANCE(ast::Expression_Mut_Of)
AST_ADD_NODE_INSTANCE(ast::Expression_Ref_Of)
AST_ADD_NODE_INSTANCE(ast::Expression_Move_Of)
AST_ADD_NODE_INSTANCE(ast::Expression_Copy_Of)
AST_ADD_NODE_INSTANCE(ast::Expression_Addr_Of)
AST_ADD_NODE_INSTANCE(ast::Expression_Size_Of)
AST_ADD_NODE_INSTANCE(ast::Expression_GetBits)
AST_ADD_NODE_INSTANCE(ast::Expression_New_Ptr)
AST_ADD_NODE_INSTANCE(ast::Expression_Get_Type)
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
AST_ADD_NODE_INSTANCE(ast::Operation_Assignment)
AST_ADD_NODE_INSTANCE(ast::Operation_Binary)
AST_ADD_NODE_INSTANCE(ast::Operation_Unary)
AST_ADD_NODE_INSTANCE(ast::Operation_Interval)
AST_ADD_NODE_INSTANCE(ast::Memory_Del)
AST_ADD_NODE_INSTANCE(ast::Memory_Align)
AST_ADD_NODE_INSTANCE(ast::Memory_Drop)

#undef AST_ADD_NODE_INSTANCE
