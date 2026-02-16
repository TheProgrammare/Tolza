
#pragma once

#include "Visitor_Default.hpp"

static const std::string ERR_TY_NOT_FOUND = "Type not found!";
static const std::string HINT_NOT_FOUND =
    ""
    "  - Did you use a correct identifier ?"
    "  - Did you use a correct type ?"
    "  - Did you use a correct path ?"
    "  - Did you are in correct scope ?";

struct Visitor_Semantic : Visitor_Default {
  // keep parent constructor
  using Visitor_Default::Visitor_Default;

  // all commented visit are not concerned by the semantic resolution
};
