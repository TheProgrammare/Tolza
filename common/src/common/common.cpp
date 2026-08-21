#include "common.hpp"

#include "utils.hpp"

#include <print>
#include <random>
#include <string_view>


common::utils::FastRNG common::utils::RAND = common::utils::FastRNG(std::random_device{}());


void common::FATAL_ERROR(std::string_view msg) noexcept
{
  std::println(stderr, "[tolza:ERROR] Fatal error: {}", msg);
  std::abort();
}
