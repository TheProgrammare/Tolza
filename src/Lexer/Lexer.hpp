/*
 *	The Velox programming language - Apache License, Version 2.0 
 *  Copyright 2024-2026 Foz Florian
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#pragma once

#include <string>
#include <map>
#include <set>
#include <unordered_set>
#include <sstream>
#include <vector>

#include "StreamTracker.hpp"
#include "Token.hpp"

enum class ETokenType;
using TokTy = ETokenType;

const std::unordered_set<std::string> kScriptMeta = {
	"author", "title", "version", "description", "created", "updated",
	"language_version", "encoding", "license", "contributor", "contact", "copyright",
	"wiki", "doc", "os"
};


class Lexer {
public:
	Lexer(const std::string& s, const std::string& f, const std::vector<std::string>& lines) : 
		stream(s), file_path(f), lines(lines) {}

	enum class EPrefixFound { None, Prefix, All };

	EPrefixFound get_prefix_keyword(TokTy _type, const std::string& _key, const std::string& _search);
	bool is_valid_prefix(char prefix, const std::string& _current);

	void tokenize(const std::set<char>& exit_char);

	void process_escape();

	void tokenize_textual();
	bool tokenize_spec();
	void tokenize_comment();
	void tokenize_metacode();
	void tokenize_numeric();
	// not idependent
	void tokenize_identifier();
	void tokenize_keyword();
	std::pair<TokTy, std::string> getToken();
	void addToken(TokTy type);
	bool eat();

	void add_error(const std::string &code, const std::string &err, const std::string &hint);

	TokTy classifyNumerals(std::string& outValue);
	TokTy classifyKeyword(std::string& outWord);
	TokTy classifyFormatSpec(std::string& outFormat);



	StreamTracker stream;
	const std::string& file_path;
	const std::vector<std::string>& lines;
	std::vector<std::string> errors;
	std::vector<Token> tokens;
	std::string buffer;
	char ch = '\0';
};
