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

void fmt_template(std::string& templateStr, const std::initializer_list<std::string>& args);

namespace command
{

void err(const std::string& msg);
void log(const std::string& msg, bool sub_log = false);

bool parse_commands(int argc, const char* argv[]);

bool parse_compiler(int argc, const char* argv[]);
bool parse_package(int argc, const char* argv[]);
bool parse_create(bool short_command, int argc, const char* argv[]);
bool parse_gui(int argc, const char* argv[]);
bool parse_build(int argc, const char* argv[]);
bool parse_generate_ffi_json(int argc, const char* argv[]);
bool parse_check(bool short_command, int argc, const char* argv[]);
bool help_command();
bool version_command();
bool parse_audit(int argc, const char* argv[]);
void invalid_command();


} // namespace command