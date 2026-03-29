#pragma once

#include <string>

namespace command
{
namespace compiler
{

void err(const std::string& msg);
void log(const std::string& msg, bool sub_log = false);

std::string find_compiler_version(const std::string& dir_search, const std::string& version);
void        apply_compiler(const std::string& file);
void        cogito_compiler(const std::string& file);

} // namespace compiler
} // namespace command
