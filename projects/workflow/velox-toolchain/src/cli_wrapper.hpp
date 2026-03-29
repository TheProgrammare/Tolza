#pragma once

#include <string>


namespace cli
{

bool        is_valid_filename(const std::string& name);
std::string sanitize_filename(std::string name);
bool        yes_no_question(const std::string& msg);
std::string get_input(const std::string& msg);
std::string ask_filename();

} // namespace cli