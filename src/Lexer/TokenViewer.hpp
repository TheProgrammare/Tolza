#pragma once

#include <vector>
#include <string>

#include "Token.hpp"

struct ScriptInfo;

class TokenViewer {
public:
    TokenViewer(std::vector<Token>& tokens, const std::vector<std::string>& lines, const std::string& f) :
        tokens(tokens), lines(lines), file(f) {}

    TokenViewer(const ScriptInfo *scr_info);
    TokenViewer(ScriptInfo *scr_info);

    Token next();
    void jump(size_t newPosition);
    Token peek(int offset = 0) const;
    Token prev();

    bool is_end() const;

    bool look_ahead(TokTy check, TokTy terminaison);

    bool check(TokTy expected);
    bool check_id_val(const std::string& val);

    bool check_val(const std::string& val);
    std::string match_any_val(const std::initializer_list<std::string>& val);
    bool match_val(const std::string& val);

    bool match(TokTy expected);
    bool match_id_val(const std::string& val);
    bool check_any(const std::initializer_list<TokTy>& types);
    bool match_any(const std::initializer_list<TokTy>& types);
    Token expect(TokTy expected, const std::string& errCode, const std::string& errMsg, const std::string& hintMsg);
    Token expect_any(const std::initializer_list<TokTy>& types, const std::string& errCode, const std::string& errMsg, const std::string& hintMsg);
    std::string expect_id(const std::string& errCode, const std::string& errMsg, const std::string& hintMsg);
    size_t position() const;
    size_t line() const;
    void rewind(size_t pos);

    void add_error(const std::string& errCode, const std::string& errMsg, const std::string& hintMsg);
    void add_error_tok(const Token& tok, const std::string& errCode, const std::string& errMsg, const std::string& hintMsg);
    void synchronize();

    // automatic line offset
    std::string get_line_str_at(size_t line) const;

    const Token& get(size_t position);

    std::vector<std::string> errors;
    std::vector<Token> tokens;
    const std::vector<std::string>& lines;
    const std::string &file;
private:
    size_t current = 0;
    std::string currentTokStr;
    TokTy currentTokTy = TokTy::S_END_OF_FILE;
    size_t currentLine = 1;
    std::string currentLineStr;
    // tabe sorted with logest keywords first
};