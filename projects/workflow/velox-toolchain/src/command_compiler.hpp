#pragma once

#include <string>
#include <vector>

namespace command
{
namespace compiler
{

void err(const std::string& msg);
void log(const std::string& msg, bool sub_log);

std::vector<std::string> find_all_compilers();
std::string              find_compiler_version(const std::string& version);
std::string              find_lastest_compiler();
void                     apply_compiler(const std::string& file);

} // namespace compiler
} // namespace command
