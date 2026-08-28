#pragma once


#include "Neargye/magic_enum_flags.hpp"
#include "common/compiler_options.hpp"
#include "compiler/compiler.hpp"

#include <print>

using IO_PASS = common::compiler::FPass;

namespace IO
{

template <typename... _Args>
inline void println(FILE* __stream, IO_PASS __phase, std::format_string<_Args...> __fmt, _Args&&... __args)
{

  const bool can_log = magic_enum::enum_flags_test(compiler::OPTIONS.log.logs, __phase)
                       || magic_enum::enum_flags_test(compiler::OPTIONS.log.logs, IO_PASS::all);

  if (!can_log) return;
  switch (compiler::OPTIONS.log.level) {
  case common::compiler::ELogLevel::quiet:
    if (__stream != stderr) return;
  case common::compiler::ELogLevel::NONE:
  case common::compiler::ELogLevel::normal:
  case common::compiler::ELogLevel::verbose:
    std::print(__stream, "[tolza:{}{}] {}\n", __phase == IO_PASS::NONE ? "" : magic_enum::enum_flags_name(__phase),
               __stream == stderr ? ":ERROR" : "", std::format(__fmt, std::forward<_Args>(__args)...));
  }
}

template <typename... _Args>
inline void println(IO_PASS __phase, std::format_string<_Args...> __fmt, _Args&&... __args)
{
  const bool can_log = magic_enum::enum_flags_test(compiler::OPTIONS.log.logs, __phase)
                       || magic_enum::enum_flags_test(compiler::OPTIONS.log.logs, IO_PASS::all);

  if (!can_log) return;
  switch (compiler::OPTIONS.log.level) {
  case common::compiler::ELogLevel::quiet: return;
  case common::compiler::ELogLevel::NONE:
  case common::compiler::ELogLevel::normal:
  case common::compiler::ELogLevel::verbose:
    std::print(stdout, "[tolza:{}] {}\n", __phase == IO_PASS::NONE ? "" : magic_enum::enum_flags_name(__phase),
               std::format(__fmt, std::forward<_Args>(__args)...));
  }
}

template <typename... _Args>
inline void println(std::format_string<_Args...> __fmt, _Args&&... __args)
{
  switch (compiler::OPTIONS.log.level) {
  case common::compiler::ELogLevel::quiet: return;
  case common::compiler::ELogLevel::NONE:
  case common::compiler::ELogLevel::normal:
  case common::compiler::ELogLevel::verbose:
    std::print(stdout, "[tolza] {}\n", std::format(__fmt, std::forward<_Args>(__args)...));
  }
}


template <typename... _Args>
inline void print(FILE* __stream, IO_PASS __phase, std::format_string<_Args...> __fmt, _Args&&... __args)
{
  const bool can_log = magic_enum::enum_flags_test(compiler::OPTIONS.log.logs, __phase)
                       || magic_enum::enum_flags_test(compiler::OPTIONS.log.logs, IO_PASS::all);

  if (!can_log) return;
  switch (compiler::OPTIONS.log.level) {
  case common::compiler::ELogLevel::quiet:
    if (__stream != stderr) return;
  case common::compiler::ELogLevel::NONE:
  case common::compiler::ELogLevel::normal:
  case common::compiler::ELogLevel::verbose:
    std::print(__stream, "[tolza:{}{}] {}", __phase == IO_PASS::NONE ? "" : magic_enum::enum_flags_name(__phase),
               __stream == stderr ? ":ERROR" : "", std::format(__fmt, std::forward<_Args>(__args)...));
  }
}

template <typename... _Args>
inline void print(IO_PASS __phase, std::format_string<_Args...> __fmt, _Args&&... __args)
{
  const bool can_log = magic_enum::enum_flags_test(compiler::OPTIONS.log.logs, __phase)
                       || magic_enum::enum_flags_test(compiler::OPTIONS.log.logs, IO_PASS::all);

  if (!can_log) return;
  switch (compiler::OPTIONS.log.level) {
  case common::compiler::ELogLevel::quiet: return;
  case common::compiler::ELogLevel::NONE:
  case common::compiler::ELogLevel::normal:
  case common::compiler::ELogLevel::verbose:
    std::print(stdout, "[tolza:{}] {}", __phase == IO_PASS::NONE ? "" : magic_enum::enum_flags_name(__phase),
               std::format(__fmt, std::forward<_Args>(__args)...));
  }
}

template <typename... _Args>
inline void print(std::format_string<_Args...> __fmt, _Args&&... __args)
{
  switch (compiler::OPTIONS.log.level) {
  case common::compiler::ELogLevel::quiet: return;
  case common::compiler::ELogLevel::NONE:
  case common::compiler::ELogLevel::normal:
  case common::compiler::ELogLevel::verbose:
    std::print(stdout, "[tolza] {}", std::format(__fmt, std::forward<_Args>(__args)...));
  }
}


template <typename... _Args>
inline void print_raw(std::format_string<_Args...> __fmt, _Args&&... __args)
{
  switch (compiler::OPTIONS.log.level) {
  case common::compiler::ELogLevel::quiet: return;
  case common::compiler::ELogLevel::NONE:
  case common::compiler::ELogLevel::normal:
  case common::compiler::ELogLevel::verbose:
    std::print(stdout, "{}", std::format(__fmt, std::forward<_Args>(__args)...));
  }
}

template <typename... _Args>
inline void print_raw(FILE* __stream, std::format_string<_Args...> __fmt, _Args&&... __args)
{
  switch (compiler::OPTIONS.log.level) {
  case common::compiler::ELogLevel::quiet:
    if (__stream != stderr) return;
  case common::compiler::ELogLevel::NONE:
  case common::compiler::ELogLevel::normal:
  case common::compiler::ELogLevel::verbose:
    std::print(__stream, "{}{}", __stream == stderr ? "ERROR: " : "",
               std::format(__fmt, std::forward<_Args>(__args)...));
  }
}

} // namespace IO