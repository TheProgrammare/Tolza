#pragma once

#include <string>

namespace command
{
namespace package
{

void err(const std::string& msg);
void log(const std::string& msg);

bool install(const std::string& pkg_name);
bool remove(const std::string& pkg_name);
bool info(const std::string& pkg_name);
bool purge(const std::string& pkg_name);
bool check(const std::string& pkg_name);
bool list(const std::string& regex, bool only_installed, bool only_upgradable);
bool update();
bool upgrade();
bool clean();

} // namespace package
} // namespace command