#pragma once

#include <string>

std::string trim(const std::string& str);

std::string make_error_output(
	size_t line, 
	size_t col, 
	size_t cursor_len, 
	const std::string& lineStr, 
	const std::string& errCode, 
	const std::string& errMsg, 
	const std::string& hintMsg, 
	const std::string& f);

inline std::string escapeChar(unsigned char c);