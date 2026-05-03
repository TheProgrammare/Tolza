#pragma once

#include <string_view>

#include "nexus/forward.hpp"


namespace resolver
{


struct Base {
  Base(script::ScriptInfo& p_scr_info)
    : scr_info(p_scr_info)
  {
  }

  script::ScriptInfo& scr_info;

  void add_error(ErrorCode code, const ast::Node& n, std::string_view msg, std::string_view hint) const;

  void add_error_two_nodes(ErrorCode, const ast::Node& first, const ast::Node& second, std::string_view msg,
                           std::string_view hint) const;

  compiler::EPhase current_EPhase() const;
};

} // namespace resolver