#pragma once

#include "common/utils.hpp"
#include <common/common.hpp>
#include <common/compiler_options.hpp>

#include <Neargye/magic_enum.hpp>
#include <Neargye/magic_enum_flags.hpp>
#include <CLIUtils/CLI11.hpp>

#define SERALIZE_FLAG(_flag)                                                                                           \
  template <>                                                                                                          \
  inline bool lexical_cast(const std::string& input, _flag& output)                                                    \
  {                                                                                                                    \
    auto value = STR_TO_FLAGS(input, _flag);                                                                           \
    if (!value.has_value()) return false;                                                                              \
    output = *value;                                                                                                   \
    return true;                                                                                                       \
  }


#define SERALIZE_ENUM(_enum)                                                                                           \
  template <>                                                                                                          \
  inline bool lexical_cast(const std::string& input, _enum& output)                                                    \
  {                                                                                                                    \
    auto value = STR_TO_ENUM(input, _enum);                                                                            \
    if (!value.has_value()) return false;                                                                              \
    output = *value;                                                                                                   \
    return true;                                                                                                       \
  }


namespace CLI::detail
{

SERALIZE_FLAG(common::compiler::FCPUFeature)
SERALIZE_FLAG(common::compiler::FCSource)
template <>
[[nodiscard]] inline bool lexical_cast(const std::string& input, common::compiler::FPass& output)
{
  const auto value = STR_TO_FLAGS(input, common::compiler::FPass);
  if (!value.has_value()) return false;
  output = *value;
  return true;
}
SERALIZE_FLAG(common::compiler::FDebugPrinter)
SERALIZE_FLAG(common::compiler::FEmit)

SERALIZE_ENUM(common::compiler::ECStandard)
SERALIZE_ENUM(common::compiler::EPlatformFlavor)
SERALIZE_ENUM(common::compiler::EEnvironment)
SERALIZE_ENUM(common::compiler::ELibC)
SERALIZE_ENUM(common::compiler::ECodeModel)
SERALIZE_ENUM(common::compiler::ERelocModel)
SERALIZE_ENUM(common::compiler::ECallingConv)
SERALIZE_ENUM(common::env::EABI)
SERALIZE_ENUM(common::env::EPlatform)
SERALIZE_ENUM(common::env::EArch)
SERALIZE_ENUM(common::env::EVendor)

template <>
[[nodiscard]] inline bool lexical_cast(const std::string& input, common::compiler::EOptimization& output)
{
  if (input.size() > 1 || input.empty()) return false;

  switch (input[0]) {
  case '0': output = common::compiler::EOptimization::O0; return true;
  case '1': output = common::compiler::EOptimization::O1; return true;
  case '2': output = common::compiler::EOptimization::O2; return true;
  case '3': output = common::compiler::EOptimization::O3; return true;
  case 's':
  case 'S': output = common::compiler::EOptimization::Os; return true;
  case 'z':
  case 'Z': output = common::compiler::EOptimization::Oz; return true;
  default:  return false;
  }
}
template <>
[[nodiscard]] inline bool lexical_cast(const std::string& input, common::compiler::EWarnLevel& output)
{
  int level{};

  try {
    level = std::stoi(input);
  } catch (...) {
    return false;
  }

  switch (level) {
  case 0:  output = common::compiler::EWarnLevel::W0; return true;
  case 1:  output = common::compiler::EWarnLevel::W1; return true;
  case 2:  output = common::compiler::EWarnLevel::W2; return true;
  case 3:  output = common::compiler::EWarnLevel::W3; return true;
  default: return false;
  }
}

template <>
[[nodiscard]] inline bool lexical_cast(const std::string& input, common::compiler::Clang::LibCVersion& output)
{
  output = common::compiler::Clang::LibCVersion::parse(input);
  return output.major != 0 && output.minor != 0;
}


} // namespace CLI::detail

#undef SERALIZE_FLAG
#undef SERALIZE_ENUM