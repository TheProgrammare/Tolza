#pragma once

#include <optional>
#include <string>
#include <tuple>

#include "visitor_default.hpp"

namespace ast
{
struct ATypeInfo;
}

template <typename T>
concept DerivedFromType = std::is_base_of_v<ast::ATypeInfo, T>;

struct Visitor_Type : public Visitor_Default {
  // keep
  // parent
  // constructor
  using Visitor_Default::Visitor_Default;

  std::optional<ast::ATypeInfo*> resolve_type(ast::Node& n, ast::ATypeInfo* input_type, bool silentError = false);
};
