#pragma once

#include <optional>
#include <string>
#include <tuple>

#include "Visitor_Default.hpp"

namespace AST
{
struct ATypeInfo;
}

template <typename T>
concept DerivedFromType = std::is_base_of_v<AST::ATypeInfo, T>;

struct Visitor_Type : public Visitor_Default {
  // keep
  // parent
  // constructor
  using Visitor_Default::Visitor_Default;

  std::optional<AST::ATypeInfo*> resolve_type(AST::Node& n, AST::ATypeInfo* input_type, bool silentError = false);
};
