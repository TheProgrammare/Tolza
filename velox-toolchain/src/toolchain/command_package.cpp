#include "command_package.hpp"

#include <iostream>

bool command::package::install(const std::string& pkg_name)
{
  std::cout << "[pkg] installing " << pkg_name << " ..." << std::endl;
  return true;
}
bool command::package::remove(const std::string& pkg_name)
{
  std::cout << "[pkg] removing " << pkg_name << " ..." << std::endl;
  return true;
}
bool command::package::info(const std::string& pkg_name)
{
  std::cout << "[pkg] info of " << pkg_name << " ..." << std::endl;
  return true;
}
bool command::package::purge(const std::string& pkg_name)
{
  std::cout << "[pkg] purging " << pkg_name << " ..." << std::endl;
  return true;
}
bool command::package::check(const std::string& pkg_name)
{
  std::cout << "[pkg] checking " << pkg_name << " ..." << std::endl;
  return true;
}
bool command::package::list(bool only_installed, bool only_upgradable)
{
  std::cout << "[pkg] listing";
  if (only_installed) std::cout << " (installed)";
  if (only_upgradable) std::cout << " (upgradable)";
  std::cout << std::endl;
  return true;
}
bool command::package::update()
{
  std::cout << "[pkg] updating the package cache list ..." << std::endl;
  return true;
}
bool command::package::upgrade()
{
  std::cout << "[pkg] check for upgrades ..." << std::endl;
  return true;
}
bool command::package::clean()
{
  std::cout << "[pkg] cleaning the package cache list ..." << std::endl;
  return true;
}