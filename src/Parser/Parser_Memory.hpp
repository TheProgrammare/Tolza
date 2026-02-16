#pragma once

#include <memory>

#include "AST/AST_Forward.hpp"

namespace PAR
{
struct Parser_Context;
struct Parser_Memory {
  Parser_Memory(Parser_Context &ctx) : ctx(ctx) {}

  // special memory expression
  [[nodiscard]] std::unique_ptr<AST::Memory::Del>   del();
  [[nodiscard]] std::unique_ptr<AST::Memory::Align> align();
  [[nodiscard]] std::unique_ptr<AST::Memory::Drop>  drop();

  Parser_Context &ctx;
};
} // namespace PAR