#include "symbol.hpp"

#include <string_view>

#include "compiler/compiler.hpp"

#include "nexus/pipeline.hpp"
#include "nexus/script.hpp"
#include "nexus/module.hpp"
#include "nexus/ast/ast.hpp"

std::string symbol::Symbol::mangle_name() const
{
  auto& mod = compiler::COMPILER.modules.get(module_id);

  return compiler::modules.tools.mangle_name(mod.id) + "." + std::string(get_name());
}

std::string_view symbol::Symbol::get_name() const
{
  auto& scr = compiler::COMPILER.pipeline.get_script(gnid.get_script_id());
  return scr.nodes->tools.get_node_declaration_name(gnid.get_node_id());
}

std::string_view symbol::Arena::get_sym_name(_id sym_id) const
{
  auto& sym = get(sym_id);

  auto& scr = compiler::COMPILER.pipeline.get_script(sym.gnid.get_script_id());
  return scr.nodes->tools.get_node_declaration_name(sym.gnid.get_node_id());
}
