#pragma once

#include "nexus/ids.hpp"

namespace preparer
{

bool prepare_cu(cu::ID cuid) noexcept;
bool lexing_cu(cu::ID cuid) noexcept;
bool preprocessing_cu(cu::ID cuid) noexcept;
bool parsing_cu(cu::ID cuid) noexcept;

} // namespace preparer