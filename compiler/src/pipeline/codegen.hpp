#pragma once

#include "nexus/ids.hpp"

namespace codegen
{
bool codegen_cu(cu::ID cuid) noexcept;
bool generate_llvm_ir_cu(cu::ID cuid) noexcept;
bool emit_llvm_ir_cu(cu::ID cuid) noexcept;
bool optimizing_cu(cu::ID cuid) noexcept;
void generate_target_machine() noexcept;
void emit_other(cu::ID cuid) noexcept;
} // namespace codegen