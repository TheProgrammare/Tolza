#include "time.hpp"

#include <chrono>


std::string common::time::now_datetime() noexcept
{
  using namespace std::chrono;

  try {
    auto       now = system_clock::now();
    auto       sec = floor<seconds>(now);
    zoned_time zt{current_zone(), sec};

    return std::format("{0:%Y-%m-%d %H:%M:%S}", zt);
  } catch (...) {
    return "";
  }
}

std::string common::time::now_date() noexcept
{
  using namespace std::chrono;

  try {
    auto       now = system_clock::now();
    auto       sec = floor<seconds>(now);
    zoned_time zt{current_zone(), sec};

    return std::format("{:%Y-%m-%d}", zt);
  } catch (...) {
    return "";
  }
}

std::string common::time::now_time() noexcept
{
  using namespace std::chrono;

  try {
    auto       now = system_clock::now();
    auto       sec = floor<seconds>(now);
    zoned_time zt{current_zone(), sec};

    return std::format("{:%H:%M:%S}", zt);
  } catch (...) {
    return "";
  }
}
