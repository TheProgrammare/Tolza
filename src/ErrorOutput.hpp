#pragma once

#include <string>

namespace AST {
	struct Node;
}

struct Error_Text {
	size_t line;
	size_t column;
	size_t cursor_len; 
	std::string line_str; 
	std::string code; 
	std::string msg;
	std::string hint; 
	std::string file;

	Error_Text(size_t _line, size_t _column, size_t _cursor_len, 
    const std::string &_line_str, 
    const std::string &_code, 
    const std::string &_msg, 
    const std::string &_hint, 
    const std::string &_file) :
		line(_line), column(_column), cursor_len(_cursor_len),
		line_str(_line_str),
		code(_code),
		msg(_msg),
		hint(_hint),
		file(_file) {};


	[[nodiscard]] std::string print_error() const;
	[[nodiscard]] std::string print_line() const;
	[[nodiscard]] std::string print_source() const;
	[[nodiscard]] std::string print_link_error() const;
	[[nodiscard]] std::string print_messages() const;

private:
	std::string trim(const std::string& str) const;
	std::string escapeChar(unsigned char c) const;
};

