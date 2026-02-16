#pragma once

#include <memory>
#include <string>
#include <vector>

#include "ErrorOutput.hpp"
#include "Globals.hpp"
#include "Token.hpp"

struct ScriptInfo;

class TokenViewer
{
public:
  TokenViewer(ScriptInfo &_scr_info) : scr_info(_scr_info) {}

  Token next();
  void  jump(size_t newPosition);
  Token peek(int offset = 0) const;
  Token prev();

  bool is_end() const;

  bool look_ahead(TokTy check, TokTy terminaison);

  bool check(TokTy expected);
  bool check_id_val(const std::string &val);

  bool        check_val(const std::string &val);
  std::string match_any_val(const std::initializer_list<std::string> &val);
  bool        match_val(const std::string &val);

  bool                         match(TokTy expected);
  bool                         match_id_val(const std::string &val);
  bool                         check_any(const std::initializer_list<TokTy> &types);
  bool                         match_any(const std::initializer_list<TokTy> &types);
  template <size_t Code> Token expect(TokTy type, const std::string &msg, const std::string &hint)
  {
    if (!check(type)) {
      add_error<Code>(msg, hint);
    }
    return next();
  }

  template <size_t Code>
  Token expect_any(const std::initializer_list<ETokenType> &types, const std::string &msg, const std::string &hint)
  {
    for (ETokenType type : types) {
      if (check(type)) {
        return next();
      }
    }
    add_error<Code>(msg, hint);
    return Token();
  }

  template <size_t Code> std::string expect_id(const std::string &msg, const std::string &hint)
  {
    const Token &tok = expect<Code>(TokTy::IDENTIFIER, msg, hint);
    return tok.val;
  }

  size_t position() const;
  size_t line() const;
  void   rewind(size_t pos);

  template <size_t Code> void add_error(const std::string &msg, const std::string &hint)
  {
    auto error_diag =
        Error_Diagnostic<Code>(scr_info, peek(), {}, EPhase::parser, EErrorSeverity::error, {}, msg, hint);

    errors.push_back(error_diag.print_error());

    throw std::runtime_error("");
  }

  template <size_t Code> void add_error_tok(const Token &tok, const std::string &msg, const std::string &hint)
  {
    auto error_diag = Error_Diagnostic<Code>(scr_info, tok, {}, EPhase::parser, EErrorSeverity::error, {}, msg, hint);
    errors.push_back(error_diag.print_error());

    throw std::runtime_error("");
  }

  void synchronize();

  // automatic line offset
  std::string get_line_str_at(size_t line) const;

  const Token &get(size_t position);

  ScriptInfo              &scr_info;
  std::vector<std::string> errors;
  EPhase                   phase = EPhase::lexer;

private:
  size_t      current = 0;
  std::string currentTokStr;
  TokTy       currentTokTy = TokTy::S_END_OF_FILE;
  size_t      currentLine  = 1;
  std::string currentLineStr;
  // tabe sorted with logest keywords first
};