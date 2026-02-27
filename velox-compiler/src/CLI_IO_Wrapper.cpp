#include "CLI_IO_Wrapper.hpp"

#include <regex>
#include <iostream>


bool CLI::is_valid_filename(const std::string& name)
{
  // file name only
  static const std::regex pattern("^[A-Za-z0-9_-]+$");
  return std::regex_match(name, pattern);
}

std::string CLI::sanitize_filename(std::string name)
{
  // rempalce unautorized characters as '_'
  std::replace_if(name.begin(), name.end(), [](char c) { return !std::isalnum(c) && c != '_' && c != '-'; }, '_');
  return name;
}

bool CLI::yes_no_question(const std::string& msg)
{
  std::cout << msg;
  std::string reponse;
  std::getline(std::cin, reponse);

  return reponse[0] == 'y' || reponse[0] == 'Y';
}

std::string CLI::get_input(const std::string& msg)
{
  std::cout << msg;
  std::string reponse;
  std::getline(std::cin, reponse);

  return reponse;
}
