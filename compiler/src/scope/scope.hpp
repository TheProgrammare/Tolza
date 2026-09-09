#pragma once

#include "id/defid.hpp"
#include "id/modid.hpp"
#include "id/nodeid.hpp"
#include "id/scpid.hpp"
#include "nexus/forward.hpp"

#include <set>
#include <string>
#include <unordered_map>
#include <utility>


namespace scope
{

using Imp_Ref = std::pair<module::ID, ast::ID>;


struct Items final {
  // declaration name, reference
  std::unordered_map<std::string, definition::ID> definitions;

  [[nodiscard]] bool           add_definition(definition::ID defid) noexcept;
  [[nodiscard]] definition::ID find_definition(const std::string& name) const noexcept;
};

struct Port final {
  // alias, <module, node>
  std::unordered_map<std::string, std::set<Imp_Ref>> imported;

  [[nodiscard]] bool                     import_module(module::ID target_module, ast::ID import_nodeid) noexcept;
  [[nodiscard]] const std::set<Imp_Ref>& find_imports(const std::string& alias) const noexcept;
};

struct Scope final {
  [[nodiscard]] static Scope make_from_module(module::ID modid, ast::ID nodeid) noexcept;
  [[nodiscard]] static Scope make_from_node(ast::ID nodeid) noexcept;
  [[nodiscard]] static Scope make_from_script_root(module::ID modid, ast::ID nodeid) noexcept;


  ID          scpid;
  std::string debug_name;


  Items items;
  Port  port;

  ast::EVisibility visibility;

  ast::ID    nodeid; // node origin
  module::ID modid;
};


} // namespace scope