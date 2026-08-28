#pragma once

#include "nexus/ids.hpp"

#include <unordered_set>

namespace module_resolver
{
bool resolve_modules(std::unordered_set<cu::ID, cu::ID::Hash>& CUs) noexcept;
bool generate_bind(const std::vector<std::string>& path, std::string_view alias) noexcept;
} // namespace module_resolver
