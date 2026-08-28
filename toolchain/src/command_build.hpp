#pragma once

#include <string>
#include <string_view>

std::string remove_quotes(std::string_view str);

namespace command::build
{


bool generate_ffi_json(std::string_view from, std::string_view to);
bool generate_ffi_c(std::string_view from, std::string_view to);
bool start_compilation(int argc, const char* argv[]);

} // namespace command::build