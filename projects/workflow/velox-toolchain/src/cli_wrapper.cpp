#include "cli_wrapper.hpp"

#include <regex>
#include <iostream>


bool cli::is_valid_filename(const std::string& name)
{
  // file name only
  static const std::regex pattern("^[A-Za-z0-9_-]+$");
  return std::regex_match(name, pattern);
}

std::string cli::sanitize_filename(std::string name)
{
  // rempalce unautorized characters as '_'
  std::replace_if(name.begin(), name.end(), [](char c) { return !std::isalnum(c) && c != '_' && c != '-'; }, '_');
  return name;
}

bool cli::yes_no_question(const std::string& msg)
{
  std::cout << "[velox:ask] " << msg << " [Y/n]: ";
  std::string reponse;
  std::getline(std::cin, reponse);

  return reponse[0] == 'y' || reponse[0] == 'Y';
}

std::string cli::get_input(const std::string& msg)
{
  std::cout << "[velox:ask] " << msg << " : ";
  std::string reponse;
  std::getline(std::cin, reponse);

  return reponse;
}

std::string cli::ask_filename()
{
retry:
  std::cout << "[velox:ask] Write down the file name: ";
  std::string name;
  std::getline(std::cin, name);

  if (!is_valid_filename(name)) {
    sanitize_filename(name);
    if (yes_no_question("File name invalid, do you want to use this version: \"" + name + "\" ?")) return name;

    if (yes_no_question("Do you want to continue ?")) goto retry;

    std::cout << "[velox] Operation aborted..." << std::endl;
  }
  return name;
}