#include "notification.hpp"

#include <format>
#include <string>

#ifdef _WIN32

#include "misc/notification/win.cpp"

#endif

void notification::notify(std::string_view title, std::string_view msg) noexcept
{
  try {
#ifdef _WIN32

    win_notify(title, msg);

#elif defined(__APPLE__)

    const std::string command = std::format("osascript -e 'display notification \"{}\" with title \"{}\"", msg, title);

    std::system(command.c_str());

#elif defined(__linux__)

    const std::string command = std::format(R"(notify-send "{}" "{}" -a "Tolza-Compiler")", title, msg);

    std::system(command.c_str());

#endif

  } catch (...) {
    // no error permitted
  }
}
