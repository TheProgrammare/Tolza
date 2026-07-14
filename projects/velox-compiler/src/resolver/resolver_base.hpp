#pragma once

#include <string_view>

#include "nexus/forward.hpp"


namespace resolver
{


struct Base {
  Base(cu::CU& p_CU)
    : CU(p_CU)
  {
  }

  cu::CU& CU;

  void add_error(ErrorCode code, const ast::Node& n, std::string_view msg, std::string_view hint) const;

  void add_error_two_nodes(ErrorCode, const ast::Node& first, const ast::Node& second, std::string_view msg,
                           std::string_view hint) const;

  [[nodiscard]] virtual compiler::EPhase current_EPhase() const;
};

} // namespace resolver