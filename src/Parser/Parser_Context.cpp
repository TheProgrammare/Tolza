
#include "Parser_Context.hpp"

#include <iostream>

#include "AST/AST_Data.hpp"
#include "ScriptInfo.hpp"

#include "Parser_Base.hpp"
#include "Parser_Declaration.hpp"
#include "Parser_Declaration_COP.hpp"
#include "Parser_Declaration_Local.hpp"
#include "Parser_Expression.hpp"
#include "Parser_Literal.hpp"
#include "Parser_Memory.hpp"
#include "Parser_Operator.hpp"
#include "Parser_Statement.hpp"
#include "Parser_Type.hpp"
#include "Visitor/Symbol_Manager.hpp"

#include "Lexer/TokenViewer.hpp"

#include "Metacode.hpp"

PAR::Parser_Context::Parser_Context(ScriptInfo &_scr_info)
    : scr_info(_scr_info), m_sym(new Symbols_Manager(_scr_info)), m_meta(_scr_info.m_meta),
      tok_v(TokenViewer(_scr_info))
{
  scr_info.m_sym = m_sym;
}

PAR::Parser_Context::~Parser_Context()
{

  delete p_cop;
  p_cop = nullptr;
  delete p_decl;
  p_decl = nullptr;
  delete p_expr;
  p_expr = nullptr;
  delete p_lit;
  p_lit = nullptr;
  delete p_loc;
  p_loc = nullptr;
  delete p_mem;
  p_mem = nullptr;
  delete p_op;
  p_op = nullptr;
  delete p_state;
  p_state = nullptr;
  delete p_type;
  p_type = nullptr;

  delete p_base;
  p_base = nullptr;
}

bool PAR::is_gen_args(TokenViewer &tok_v)
{
  size_t originPos           = tok_v.position();
  bool   isGenArgsValid      = true;
  bool   isAfterGenArgsValid = false;
  size_t count               = 0;
  size_t nestedBrackets      = 1; // = 0 valid nested result; > 0 invalid nested result; begins at 1
                                  // because generic type args begins with < and must end by >

  // check genArgs
  while (!tok_v.is_end()) {
    bool tokValid = false;

    auto tok = tok_v.peek(count);
    for (auto tok_valid : kGenArgsValidTokens) {
      if (tok.type == tok_valid) {
        if (tok.type == TokTy::OPEN_BRACKETS) ++nestedBrackets;
        if (tok.type == TokTy::CLOSE_BRACKETS) --nestedBrackets;
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

  // if is a valid generic args with correct nested brackets
  // check next token if is not a false positive generic args (case of interval comparaison like a <
  // b > 0)
  if (isGenArgsValid && nestedBrackets == 0) {
    auto tokAfterGenArgs = tok_v.peek(count + 1);

    if (tokAfterGenArgs.type == TokTy::OPEN_PAREN)
      isAfterGenArgsValid = true;
    else {
      isAfterGenArgsValid =
          !(tok_v.check_any(kLiteralTokens) || tok_v.check_any(kOperatorTokens) || tok_v.check_any(kComparatorTokens));
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

bool PAR::Parser_Context::metablock_contains(const AST::Node &n, const std::string &s) const
{
  return m_meta->contains(n.get_tok_antepos(), s);
}

bool PAR::Parser_Context::metablock_contains(const AST::Node &n, TokTy t) const
{
  return m_meta->contains(n.get_tok_antepos(), t);
}

std::string PAR::Parser_Context::get_export_name(const AST::Node &n) const
{
  return m_meta->get_export_name(n.get_tok_antepos());
}

const META::MetaInstruct *PAR::Parser_Context::get_instruct(const AST::Node                          &n,
                                                            const std::initializer_list<std::string> &pattern) const
{
  return m_meta->get_instruct(n.get_tok_antepos(), pattern);
}

const META::MetaBlock *PAR::Parser_Context::get_metablock(const AST::Node                          &n,
                                                          const std::initializer_list<std::string> &pattern)
{
  return m_meta->get_metablock(n.get_tok_antepos(), pattern);
}

void PAR::Parser_Context::attempt_recovery()
{

  // recovery loop case
  static Token lastokRecovered;
  if (lastokRecovered.span == tok_v.peek().span) {
    std::cerr << "\n--------------------- ! COMPILATION STOPPED ! -------------------" << std::endl;
    std::cerr << "  Compiler: Infinitive recovery loop detected!" << std::endl;
    std::cerr << "  Please check the code source." << std::endl;
    return;
  } else {
    lastokRecovered = tok_v.peek();
  }

  /*
    static size_t tryCatchCount = 0;
    try {
      switch (m_scope->current_scope.top()->type)
      {
      case EScopeType::MOD:
      case EScopeType::GLOBAL: {
        while (!tok_v.is_end()) {
          auto line = p_base->parse_root();
          if (tok_v.match(TokTy::S_END_OF_FILE)) break;
        }

        m_scope->exit_scope();
        break;
      }
      case EScopeType::FUNCTION:
      case EScopeType::LAMBDA:
      case EScopeType::IF:
      case EScopeType::ELSE:
      case EScopeType::IF_ELSE:
      case EScopeType::FOR:
      case EScopeType::WHILE:
      case EScopeType::DO_WHILE:
      case EScopeType::ENUM:
      case EScopeType::MATCH:
      case EScopeType::MATCH_CASE: {
        while (!tok_v.is_end()) {
          p_inst->parse_instruction();
          if (match_field_separator(TokTy::S_END_OF_FILE, TokTy::CLOSE_BRACE)) break;
        }

        m_scope->exit_scope();
        break;
      }
      default: {
        return;
      }
      }
    }
    catch (const std::runtime_error& e) {
      ++tryCatchCount;
      if (tryCatchCount > 1000) {
        std::cerr << "--------------------- ! COMPILATION STOPPED ! -------------------" <<
    std::endl; std::cerr << "  Compiler: Too many errors encounted (> 1000)!" << std::endl;
        std::cerr << "  Please check the code source." << std::endl;
        return;
      }
      tok_v.synchronize();
      attempt_recovery();
    }
  */

  return;
}

bool PAR::Parser_Context::match_field_separator(TokTy separator, TokTy end)
{
  if (separator != TokTy::S_END_OF_FILE) {
    if (tok_v.match(separator)) return false;
    if (tok_v.match(end)) return true;
    tok_v.add_error<12>("Unexpected token '" + tok_v.peek().val + "' in expression.",
                        "expected a separator '" + std::to_string(int(separator)) + "' or a ending '"
                            + std::to_string(int(end)) + "'");

  } else {
    if (tok_v.match(end)) return true;
  }
  return false;
}

bool PAR::Parser_Context::match_field_any_separator(TokTy separator, std::initializer_list<TokTy> end)
{
  if (tok_v.match(separator)) return false;
  if (tok_v.match_any(end)) return true;
  std::string endSymbols;
  size_t      countSym = 0;
  for (auto &elem : end) {
    endSymbols += "'" + std::to_string(int(elem)) + "', ";
    if (++countSym > 10) {
      endSymbols += "\n";
      countSym = 0;
    }
  }
  tok_v.add_error<13>("Unexpected token '" + tok_v.peek().val + "' in expression.",
                      "expected a separator '" + std::to_string(int(separator)) + "' or a ending {" + endSymbols + "}");
  return false;
}

std::string PAR::Parser_Context::parse_name(const std::string &custom_msg, const std::string &custom_hint)
{
  static const std::string _msg = "Expected identifier (classic name).";
  static const std::string _hint =
      "define identifier (classic name) like:"
      "\n  - rule `[a-zA-Z_][a-zA-Z0-9_]*`"
      "\n  - first character is alphabetical or `_`"
      "\n  - other character is alphanumeric or `_`";

  const std::string final_msg  = custom_msg.empty() ? _msg : custom_msg;
  const std::string final_hint = custom_hint.empty() ? _hint : custom_hint;

  return tok_v.expect<777>(TokTy::IDENTIFIER, final_msg, final_hint).val;
}
