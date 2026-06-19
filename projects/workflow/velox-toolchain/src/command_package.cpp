#include "command_package.hpp"

#include <iostream>

#define OUT_LOG std::cout << "[pkg] "
#define OUT_ERR std::cerr << "[pkg:ERROR] "

bool command::package::install(std::string_view pkg_name) noexcept
{
  OUT_LOG "Installing " << pkg_name << " ...";
  return true;
}
bool command::package::remove(std::string_view pkg_name) noexcept
{
  OUT_LOG "Removing " << pkg_name << " ...";
  return true;
}
bool command::package::info(std::string_view pkg_name) noexcept
{
  OUT_LOG "Info of " << pkg_name << " ...";
  return true;
}
bool command::package::purge(std::string_view pkg_name) noexcept
{
  OUT_LOG "Purging " << pkg_name << " ...";
  return true;
}
bool command::package::check(std::string_view pkg_name) noexcept
{
  OUT_LOG "Checking " << pkg_name << " ...";
  return true;
}
bool command::package::list(std::string_view regex, bool only_installed, bool only_upgradable) noexcept
{
  std::cout << "Listing";
  if (only_installed) std::cout << " (installed)";
  if (only_upgradable) std::cout << " (upgradable)";
  return true;
}
bool command::package::update() noexcept
{
  OUT_LOG "Updating the package cache list ...";
  return true;
}
bool command::package::upgrade() noexcept
{
  OUT_LOG "Check for upgrades ...";
  return true;
}
bool command::package::clean() noexcept
{
  OUT_LOG "Cleaning the package cache list ...";
  return true;
}