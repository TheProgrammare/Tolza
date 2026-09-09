#pragma once

#include "id/cuid.hpp"

#include <cstddef>

namespace resolver
{
bool   resolve_cu(cu::ID cuid) noexcept;
size_t symbol_resolution_cu(cu::ID cuid) noexcept;
size_t inference_resolution_cu(cu::ID cuid) noexcept;
size_t semantic_resolution_cu(cu::ID cuid) noexcept;
size_t eval_resolution_cu(cu::ID cuid) noexcept;
} // namespace resolver