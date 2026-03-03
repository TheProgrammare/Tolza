#pragma once

#include <string>

namespace command
{
namespace package
{

bool install(const std::string& pkg_name);
bool remove(const std::string& pkg_name);
bool info(const std::string& pkg_name);
bool purge(const std::string& pkg_name);
bool check(const std::string& pkg_name);
bool list(bool only_installed, bool only_upgradable);
bool update();
bool upgrade();
bool clean();

} // namespace package
} // namespace command