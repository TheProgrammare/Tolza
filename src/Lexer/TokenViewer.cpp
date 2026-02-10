#include "TokenViewer.hpp"

#include <system_error>

#include "ErrorOutput.hpp"
#include "ScriptInfo.hpp"

void TokenViewer::jump(size_t newPosition) {
    if (newPosition < tokens.size()) {
        current = newPosition;
    }
    else {
        current = tokens.size() - 1;
    }

    currentTokStr = tokens[current].val;
    currentTokTy = tokens[current].type;
    currentLine = tokens[current].span.line;
    if (currentLine - 1 >= lines.size()) currentLine = lines.size();
    currentLineStr = lines[currentLine - 1];
}

TokenViewer::TokenViewer(const ScriptInfo *scr_info) : tokens(scr_info->tokens), lines(scr_info->src_lines), file(scr_info->file_path) {}
TokenViewer::TokenViewer(ScriptInfo *scr_info) : tokens(scr_info->tokens), lines(scr_info->src_lines), file(scr_info->file_path) {}

Token TokenViewer::next()
{
    if (!is_end()) {
        auto pre_tok = tokens[current];
		current++;
        auto post_tok = tokens[current];
        currentTokStr = post_tok.val;
        currentTokTy = post_tok.type;
        currentLine = post_tok.span.line;
        if (currentLine - 1 >= lines.size()) currentLine = lines.size();
        currentLineStr = lines[currentLine - 1];
		return pre_tok;
	}
	return tokens.back();
}

Token TokenViewer::peek(int offset) const {
	size_t index = current + offset;
	if (index < tokens.size())
		return tokens[index];
	return tokens.back();
}

Token TokenViewer::prev() {
    if (current == 0) {
        return tokens[0];
    }
    current--;
	return tokens[current];
}

bool TokenViewer::is_end() const {
	return current == tokens.size() - 1 || tokens[current].type == ETokenType::S_END_OF_FILE;
}

bool TokenViewer::look_ahead(TokTy check, TokTy terminaison)
{
	for (size_t offset = 0; ; offset++) {
		TokTy type = peek(offset).type;
		if (type == check) return true;
		if (type == terminaison) return false;
		if (type == TokTy::S_END_OF_FILE) return false;
		offset++;
	}
}

// will ignore new line if not specified
bool TokenViewer::check(TokTy type) {
	if (!is_end()) {
		return peek().type == type;
	}
	return false;
}

bool TokenViewer::check_any(const std::initializer_list<ETokenType>& types) {
	for (ETokenType type : types) {
		if (check(type)) 
			return true;
	}
	return false;
}

bool TokenViewer::match(TokTy type) {
	if (check(type)) {
		next();
		return true;
	}
	return false;
}

bool TokenViewer::check_id_val(const std::string& val) {
    if (check(TokTy::IDENTIFIER)) {
        return peek().val == val;
    }
    return false;
}

bool TokenViewer::check_val(const std::string& val) {
    return peek().val == val;
}

std::string TokenViewer::match_any_val(const std::initializer_list<std::string>& val) {
    const std::string tok = peek().val;
    for (auto& elem : val) {
        if (tok == elem) {
            next();
            return elem;
        }
    }

    return "";
}

bool TokenViewer::match_val(const std::string& val) {
    if (peek().val == val) {
        next();
        return true;
    }
    return false;
}

bool TokenViewer::match_id_val(const std::string& val) {
    if (check_id_val(val)) {
        next();
        return true;
    }
    return false;
}

bool TokenViewer::match_any(const std::initializer_list<ETokenType>& types) {
	for (ETokenType type : types) {
		if (check(type)) {
			next();
			return true;
		}
	}
	return false;
}

Token TokenViewer::expect(TokTy type, const std::string& errCode, const std::string& errMsg, const std::string& hintMsg) {
	if (!check(type)) {
		add_error(errCode, errMsg, hintMsg);
	}
	return next();
}

Token TokenViewer::expect_any(const std::initializer_list<ETokenType>& types, const std::string& errCode, const std::string& errMsg, const std::string& hintMsg) {
	for (ETokenType type : types) {
		if (check(type)) {
			return next();
		}
	}
	add_error(errCode, errMsg, hintMsg);
    return Token();
}

std::string TokenViewer::expect_id(const std::string& errCode, const std::string& errMsg, const std::string& hintMsg) {
	const Token& tok = expect(TokTy::IDENTIFIER, errCode, errMsg, hintMsg);
	return tok.val;
}

size_t TokenViewer::position() const {
	return current;
}

size_t TokenViewer::line() const {
	return peek().span.line;
}

// Go back to a know position
void TokenViewer::rewind(size_t pos) {
    if (pos >= tokens.size()) pos = tokens.size() - 1;
	current = pos;
    currentTokStr = tokens[pos].val;
    currentTokTy = tokens[pos].type;
    currentLine = tokens[pos].span.line;
    if (currentLine >= lines.size()) currentLine = lines.size() - 1;
    currentLineStr = lines[currentLine - 1];
}

void TokenViewer::add_error(const std::string& errCode, const std::string& errMsg, const std::string& hintMsg) {
    errors.push_back(Error_Text(peek().span.line, peek().span.col, peek().span.size, lines[peek().span.line - 1], errCode, errMsg, hintMsg, file).print_error());
    
    throw std::runtime_error("");
}

void TokenViewer::add_error_tok(const Token& tok, const std::string& errCode, const std::string& errMsg, const std::string& hintMsg) {
    errors.push_back(Error_Text(tok.span.line, peek().span.col, tok.span.size, lines[tok.span.line - 1], errCode, errMsg, hintMsg, file).print_error());
   
    throw std::runtime_error("");
}

void TokenViewer::synchronize() {
	while (!is_end()) {
		if (peek(-1).type == TokTy::SEMICOLON) return;

		switch (peek().type)
		{
		case TokTy::LET: case TokTy::VAR: case TokTy::ENTITY: case TokTy::METACODE:
		case TokTy::ENUM: case TokTy::IF: case TokTy::ELSE: case TokTy::WHILE:
		case TokTy::INJECT: case TokTy::FOR: case TokTy::RETURN: case TokTy::BREAK:
		case TokTy::CONTINUE: case TokTy::MATCH: case TokTy::CAST: case TokTy::FUNCTION:
		case TokTy::SYSTEM: case TokTy::OP: case TokTy::COMPONENT: case TokTy::IDENTIFIER:
			return; // end the function
		default:
			break; // continue
		}

		next(); // consume token and continue
	}
}

std::string TokenViewer::get_line_str_at(size_t line) const
{
	int offset = line - 1;
	if (lines.size() < offset) return "EOF";
	return lines[offset];
}

const Token &TokenViewer::get(size_t position)
{
    if (tokens.size() < position)
		auto val = tokens.back();
	return tokens[position];
}