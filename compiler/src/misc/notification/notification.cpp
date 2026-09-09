#include "notification.hpp"

#include "compiler/compiler.hpp"

#include <common/compiler_options.hpp>
#include <cstdlib>
#include <format>
#include <string>
#include <string_view>

#ifdef _WIN32

#include "misc/notification/win.hpp"

#endif

void notification::notify(std::string_view title, std::string_view msg, bool success) noexcept
{

  if (OPTIONS.log.level == common::compiler::ELogLevel::quiet) return;
  if (!OPTIONS.log.notify) return;

  try {
#ifdef _WIN32

    win_notify(title, msg, success);

#elif defined(__APPLE__)

    const std::string command = std::format("osascript -e 'display notification \"{}\" with title \"{}\"", msg, title);

    std::system(command.c_str());

#elif defined(__linux__)

    const std::string command = std::format(R"(notify-send "{}" "{}" -a "Tolza-Compiler" -i {})", title, msg,
                                            success ? "emblem-checked" : "emblem-error");
    std::system(command.c_str());

#endif

  } catch (...) {
    // no error permitted
  }
}
