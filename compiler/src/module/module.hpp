#pragma once


#include "id/modid.hpp"
#include "id/nodeid.hpp"
#include "id/scpid.hpp"
#include "nexus/forward.hpp"

#include <cstdint>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>


namespace module
{
using ReexpRef = std::pair<module::ID, ast::ID>;


enum class EModuleKind : uint8_t { Inline, File, Generated, System };

struct Port final {
  Module& mod;

  // any global declaration node
  std::unordered_map<std::string, ast::ID> exported_items;

  // node is Reexport
  // alias, <module, node>
  std::unordered_map<std::string, std::set<ReexpRef>> reexported_module;

  void               export_item(ast::ID nodeid) noexcept;
  [[nodiscard]] bool reexport_item(module::ID modid, ast::ID nodeid) noexcept;

  [[nodiscard]] const std::set<ReexpRef>& find_reexport(const std::string& alias) const noexcept;
};

struct Module final {
  // generate module
  [[nodiscard]] static Module new_node_module(ast::ID nodeid, std::string_view p_debug_name) noexcept;
  // generate rule module
  [[nodiscard]] static Module from_system(std::string_view p_name, std::string_view p_debug_name) noexcept;
  static Module               from_script();

  // to get private items (non exported nodes)
  // go to the scope port
  Port port{.mod = *this};

  EModuleKind      kind = EModuleKind::Inline;
  ast::EVisibility visibility;
  ID               modid;
  std::string      name;
  std::string      debug_name;

  scope::ID scpid;
  ast::ID   nodeid;
};

} // namespace module