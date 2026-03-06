#include "command_package.hpp"
#include "toolchain.hpp"

#include <iostream>

void command::package::err(const std::string& msg)
{
  std::cerr << "[pkg] [ERROR] " << msg << std::endl;
}

void command::package::log(const std::string& msg)
{
  std::cout << "[pkg] " << msg << std::endl;
}

bool command::package::install(const std::string& pkg_name)
{
  log("Installing " + pkg_name + " ...");
  return true;
}
bool command::package::remove(const std::string& pkg_name)
{
  log("Removing " + pkg_name + " ...");
  return true;
}
bool command::package::info(const std::string& pkg_name)
{
  log("Info of " + pkg_name + " ...");
  return true;
}
bool command::package::purge(const std::string& pkg_name)
{
  log("Purging " + pkg_name + " ...");
  return true;
}
bool command::package::check(const std::string& pkg_name)
{
  log("Checking " + pkg_name + " ...");
  return true;
}
bool command::package::list(bool only_installed, bool only_upgradable)
{
  std::cout << "Listing";
  if (only_installed) std::cout << " (installed)";
  if (only_upgradable) std::cout << " (upgradable)";
  std::cout << std::endl;
  return true;
}
bool command::package::update()
{
  log("Updating the package cache list ...");
  return true;
}
bool command::package::upgrade()
{
  log("Check for upgrades ...");
  return true;
}
bool command::package::clean()
{
  log("Cleaning the package cache list ...");
  return true;
}