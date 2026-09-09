#pragma once

#include "nexus/forward.hpp"

#include <string_view>

namespace resolver
{

struct Evaluator;
struct Evaluable;


struct Base {
  Base(cu::CU& p_CU);

  cu::CU& CU;

  resolver::Evaluator& eval;
  resolver::Evaluable& evaluable;

  void add_error(ErrorCode code, const ast::NodeHeader& n, std::string_view msg, std::string_view hint) const;

  void add_error_two_nodes(ErrorCode, const ast::NodeHeader& first, const ast::NodeHeader& second, std::string_view msg,
                           std::string_view hint) const;

  [[nodiscard]] virtual compiler::EPhase current_EPhase() const;
};

} // namespace resolver