#pragma once

#include <common/utils.hpp>
#include <print>
#include <Neargye/magic_enum.hpp>
#include <Neargye/magic_enum_flags.hpp>
#include <common/common.hpp>
#include <common/compiler_options.hpp>


namespace CLI::type_ser
{

template <typename Flag>
inline Flag parse_flags(std::string_view input)
{
  auto value = magic_enum::enum_flags_cast<Flag>(common::utils::str_to_snake(input), '|', magic_enum::case_insensitive);
  if (!value.has_value()) {
    return Flag::NONE;
  }
  return *value;
}

template <typename Enum>
inline Enum parse_enum(std::string_view input)
{
  auto value = magic_enum::enum_cast<Enum>(common::utils::str_to_snake(input), magic_enum::case_insensitive);
  if (!value.has_value()) {
    return Enum::NONE;
  }
  return *value;
}

inline common::compiler::EOptimization parse_opti(std::string_view input)
{
  if (input.size() > 1 || input.empty()) return common::compiler::EOptimization::NONE;

  switch (input[0]) {
  case '0': return common::compiler::EOptimization::O0;
  case '1': return common::compiler::EOptimization::O1;
  case '2': return common::compiler::EOptimization::O2;
  case '3': return common::compiler::EOptimization::O3;
  case 's':
  case 'S': return common::compiler::EOptimization::Os;
  case 'z':
  case 'Z': return common::compiler::EOptimization::Oz;
  default:  return common::compiler::EOptimization::NONE;
  }
}
inline common::compiler::EWarnLevel parse_warnlevel(std::string_view input)
{
  int level{};

  try {
    level = std::stoi(std::string(input));
  } catch (...) {
    return common::compiler::EWarnLevel::NONE;
  }

  switch (level) {
  case 0:  return common::compiler::EWarnLevel::W0;
  case 1:  return common::compiler::EWarnLevel::W1;
  case 2:  return common::compiler::EWarnLevel::W2;
  case 3:  return common::compiler::EWarnLevel::W3;
  default: return common::compiler::EWarnLevel::NONE;
  }
}

} // namespace CLI::type_ser

#undef SERALIZE_FLAG
#undef SERALIZE_ENUM
