#pragma once

#include "nexus/ast/data.hpp"
#include "nexus/ast/definition.hpp"
#include "nexus/ast/forward.hpp"

namespace ast
{

// del var
struct Memory_Del final {
  NODE_HEADER(Memory_Del);

  SET_NODE(target);
};

struct Memory_Align final {
  NODE_HEADER(Memory_Align);

  SET_NODE(target);
  size_t align = 0;
};

struct Memory_Drop final {
  NODE_HEADER(Memory_Drop);

  SET_NODE(target);
};

} // namespace ast
  // AST