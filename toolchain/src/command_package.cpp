#include "command_package.hpp"

#include <print>

#define HLOG "[pkg] "
#define HERR "[pkg:ERROR] "

bool command::package::install(std::string_view pkg_name) noexcept
{
  std::println(HLOG "Installing {} ...", pkg_name);
  return true;
}
bool command::package::remove(std::string_view pkg_name) noexcept
{
  std::println(HLOG "Removing {} ...", pkg_name);
  return true;
}
bool command::package::info(std::string_view pkg_name) noexcept
{
  std::println(HLOG "Info of {} ...", pkg_name);
  return true;
}
bool command::package::purge(std::string_view pkg_name) noexcept
{
  std::println(HLOG "Purging {} ...", pkg_name);
  return true;
}
bool command::package::check(std::string_view pkg_name) noexcept
{
  std::println(HLOG "Checking {} ...", pkg_name);
  return true;
}
bool command::package::list(std::string_view regex, bool only_installed, bool only_upgradable) noexcept
{
  std::print("Listing");
  if (only_installed) std::print(" (installed)");
  if (only_upgradable) std::print(" (upgradable)");
  return true;
}
bool command::package::update() noexcept
{
  std::print(HLOG "Updating the package cache list ...");
  return true;
}
bool command::package::upgrade() noexcept
{
  std::print(HLOG "Check for upgrades ...");
  return true;
}
bool command::package::clean() noexcept
{
  std::print(HLOG "Cleaning the package cache list ...");
  return true;
}