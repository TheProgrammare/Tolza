#pragma once

#include <string>
#include <vector>

#include "compiler/compiler.hpp"
#include "token.hpp"

struct ScriptInfo;

class TokenViewer
{
public:
  TokenViewer(ScriptInfo& _scr_info)
    : scr_info(_scr_info)
  {
  }

  Token next();
  void  jump(size_t newPosition);
  Token peek(int offset = 0) const;
  Token prev();

  bool is_end() const;

  bool look_ahead(TokTy check, TokTy terminaison);

  bool check(TokTy expected);
  bool check_id_val(const std::string& val);

  bool        check_val(const std::string& val);
  std::string match_any_val(const std::initializer_list<std::string>& val);
  bool        match_val(const std::string& val);

  bool match(TokTy expected);
  bool match_id_val(const std::string& val);
  bool check_any(const std::initializer_list<TokTy>& types);
  bool match_any(const std::initializer_list<TokTy>& types);

  Token expect(ErrorCode code, TokTy type, const std::string& msg, const std::string& hint);

  Token expect_any(ErrorCode code, const std::initializer_list<ETokenType>& types, const std::string& msg,
                   const std::string& hint);

  size_t position() const;
  size_t line() const;
  void   rewind(size_t pos);

  void add_error(ErrorCode code, const std::string& msg, const std::string& hint);

  void add_error_tok(ErrorCode code, const Token& tok, const std::string& msg, const std::string& hint);

  void synchronize();

  // automatic line offset
  std::string get_line_str_at(size_t line) const;

  const Token& get(size_t position);

  ScriptInfo&              scr_info;
  std::vector<std::string> errors;
  compiler::EPhase         phase = compiler::EPhase::lexer;

private:
  size_t      current = 0;
  std::string currentTokStr;
  TokTy       currentTokTy = TokTy::S_END_OF_FILE;
  size_t      currentLine  = 1;
  std::string currentLineStr;
  // tabe sorted with logest keywords first
};