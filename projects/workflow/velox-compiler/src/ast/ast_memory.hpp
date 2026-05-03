#pragma once

#include "nexus/ast/ast.hpp"

namespace ast
{

// del var
AST_NODE(Memory_Del)
{
  SET_NODE(target);
};

AST_NODE(Memory_Align)
{
  SET_NODE(target);
  size_t align = 0;
};

AST_NODE(Memory_Drop)
{
  SET_NODE(target);
};

} // namespace ast
  // AST