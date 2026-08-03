#include "cli_wrapper.hpp"

#include <algorithm>
#include <regex>
#include <iostream>


bool cli::is_valid_filename(std::string_view name) noexcept
{
  // file name only
  static const std::regex pattern("^[A-Za-z0-9_-]+$");
  try {
    return std::regex_match(std::string(name), pattern);
  } catch (...) {
    return false;
  }
}

void cli::sanitize_filename(std::string& name) noexcept
{
  // rempalce unautorized characters as '_'
  std::ranges::replace_if(name, [](char c) { return !std::isalnum(c) && c != '_' && c != '-'; }, '_');
}

bool cli::yes_no_question(std::string_view msg) noexcept
{
  std::cout << "[velox:ask] " << msg << " [Y/n]: ";
  std::string reponse;
  std::getline(std::cin, reponse);

  return reponse[0] == 'y' || reponse[0] == 'Y';
}

std::string cli::get_input(std::string_view msg) noexcept
{
  std::cout << "[velox:ask] " << msg << " : ";
  std::string reponse;
  std::getline(std::cin, reponse);

  return reponse;
}

std::string cli::ask_text(std::string_view msg, bool is_filename) noexcept
{
retry:
  std::cout << "[velox:ask] " << msg << " : ";
  std::string name;
  std::getline(std::cin, name);

  if (is_filename && !is_valid_filename(name)) {
    sanitize_filename(name);
    if (yes_no_question("File name invalid, do you want to use this version: \"" + name + "\" ?")) return name;

    if (yes_no_question("Do you want to continue ?")) goto retry;

    std::cout << "[velox] Operation aborted...\n";
  }
  return name;
}