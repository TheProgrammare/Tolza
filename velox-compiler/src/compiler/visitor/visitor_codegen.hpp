
#pragma once

#include "visitor_default.hpp"

struct Visitor_Codegen : public Visitor_Default {
  // keep parent constructor
  using Visitor_Default::Visitor_Default;
};
