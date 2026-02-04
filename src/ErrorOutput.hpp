#pragma once

#include <string>

struct error_text {
	size_t line;
	size_t col;
	size_t cursor_len; 
	std::string line_str; 
	std::string error_code; 
	std::string error_msg;
	std::string hint_msg; 
	std::string file;

	std::string print_error() const;
	std::ostringstream print_line() const;
	std::ostringstream print_source() const: 

private:
	std::string trim(const std::string& str) const;
	std::string escapeChar(unsigned char c) const;
};

