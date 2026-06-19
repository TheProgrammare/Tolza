#include "common.hpp"

#include "utils.hpp"

#include <iostream>
#include <random>
#include <string_view>


common::utils::FastRNG common::utils::RAND = common::utils::FastRNG(std::random_device{}());


void common::FATAL_ERROR(std::string_view msg) noexcept
{
  std::cerr << "[velox:ERROR] Fatal error: " << msg << "\n";
  std::abort();
}
