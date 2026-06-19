#include "ast.hpp"

#include <cstdint>

#include <Neargye/magic_enum.hpp>

#include "ast/ast_base.hpp"
#include "ast/ast_declaration_extension.hpp"
#include "ast/ast_declaration_sfm.hpp"
#include "ast/ast_declaration_global.hpp"
#include "ast/ast_declaration_local.hpp"
#include "nexus/forward.hpp"
#include "nexus/ids.hpp"
#include "nexus/module.hpp"
#include "compiler/compilation_unit.hpp"

std::vector<ast::ID> ast::get_parameters(ast::ID nodeid) noexcept
{
  if (const auto* n = nodeid.as<ast::Global_Function>()) return n->parameters;
  if (const auto* n = nodeid.as<ast::Local_Lambda>()) return n->parameters;
  if (const auto* n = nodeid.as<ast::SFM_Rule>()) return n->parameters;

  return {};
}


std::string ast::get_decl_name(ast::ID nodeid) noexcept
{
  auto* n = nodeid.get();

#define case_n(kind)                                                                                                   \
  case ENodeKind::kind: return nodeid.as<ast::kind>()

  switch (n->kind()) {
    case_n(Identifier)->name;
    case_n(ID_Qualified)->name;
  case ENodeKind::ID_Typed:
    return get_decl_name(nodeid.as<ast::ID_Typed>()->name);
    case_n(Global_Variable)->name;
    case_n(Global_Function)->name;
    case_n(Global_Module)->name;
    case_n(Global_Extern)->abi;
    case_n(Global_Reexport)->alias;
    case_n(Global_Enum)->name;
    case_n(Global_Flag)->name;
    case_n(Global_Union)->name;
    case_n(Global_Alias_Type)->alias;
    case_n(Global_Alias_Module)->alias;
    case_n(Global_Generic)->name;
    case_n(Local_Lambda)->name;
    case_n(Local_Parameter)->name;
    case_n(Local_Gen_Param_Elem)->name;
    case_n(Local_Binding)->name;
    case_n(Local_Variable)->name;
    case_n(Local_Capability)->name;
    case_n(SFM_Facet)->name;
    case_n(SFM_Facet_Field)->name;
    case_n(SFM_View)->name;
    case_n(SFM_Form)->name;
    case_n(SFM_Rule)->name;
    case_n(Import)->alias;
  default: assert(false && "Must be used on named node or with alias");
  }

#undef case_n
}

EVisibility ast::get_decl_visibility(ast::ID nodeid) noexcept
{
  auto* n = nodeid.get();

#define case_n(kind)                                                                                                   \
  case ENodeKind::kind: return nodeid.as<ast::kind>()->visibility

  switch (n->kind()) {
    case_n(Global_Variable);
    case_n(Global_Function);
    case_n(Global_Extend_Fn);
    case_n(Global_Extend_Cast);
    case_n(Global_Extend_Op_Bin);
    case_n(Global_Extend_Op_Un);
    case_n(Global_Extend_Op_Access);
    case_n(Global_Extend_Op_Transfert);
    case_n(Global_Extend_Op_Other);
    case_n(Global_Module);
    case_n(Global_Enum);
    case_n(Global_Flag);
    case_n(Global_Union);
    case_n(Global_Alias_Type);
    case_n(Global_Alias_Module);
    case_n(Global_Generic);
    case_n(SFM_Facet);
    case_n(SFM_View);
    case_n(SFM_Form);
    case_n(SFM_Rule);
  default: assert(false && "Must be used on declaration node with a visibility");
  }

#undef case_n
}


std::string ast::get_mangled_id(ast::ID nodeid) noexcept
{
  auto mod_mangle = module::mangle_canonical_module_path(nodeid.module());

#define case_n(kind)                                                                                                   \
  case ENodeKind::kind: return mod_mangle + "." + id.as<kind>()

  switch (nodeid.get()->kind()) {
  case ast::ENodeKind::Identifier:        return mod_mangle + "." + std::string(nodeid.as<Identifier>()->name);
  case ast::ENodeKind::ID_Qualified:      return mod_mangle + "." + std::string(nodeid.as<ID_Qualified>()->name);
  case ENodeKind::ID_Typed:               return get_mangled_id(ast::ID::make(nodeid.cu(), nodeid.as<ID_Typed>()->name.offset()));
  case ast::ENodeKind::Global_Variable:   return mod_mangle + "." + std::string(nodeid.as<Global_Variable>()->name);
  case ast::ENodeKind::Global_Function:   return mod_mangle + "." + std::string(nodeid.as<Global_Function>()->name);
  case ast::ENodeKind::Global_Module:     return mod_mangle + "." + std::string(nodeid.as<Global_Module>()->name);
  case ast::ENodeKind::Global_Extern:     return mod_mangle + "." + std::string(nodeid.as<Global_Extern>()->abi);
  case ast::ENodeKind::Global_Reexport:   return mod_mangle + "." + std::string(nodeid.as<Global_Reexport>()->alias);
  case ast::ENodeKind::Enum_Field:        return mod_mangle + "." + std::string(nodeid.as<Enum_Field>()->name);
  case ast::ENodeKind::Global_Enum:       return mod_mangle + "." + std::string(nodeid.as<Global_Enum>()->name);
  case ast::ENodeKind::Flag_Field:        return mod_mangle + "." + std::string(nodeid.as<Flag_Field>()->name);
  case ast::ENodeKind::Global_Flag:       return mod_mangle + "." + std::string(nodeid.as<Global_Flag>()->name);
  case ast::ENodeKind::Union_Field:       return mod_mangle + "." + std::string(nodeid.as<Union_Field>()->name);
  case ast::ENodeKind::Global_Union:      return mod_mangle + "." + std::string(nodeid.as<Global_Union>()->name);
  case ast::ENodeKind::Global_Alias_Type: return mod_mangle + "." + std::string(nodeid.as<Global_Alias_Type>()->alias);
  case ast::ENodeKind::Global_Alias_Module:
    return mod_mangle + "." + std::string(nodeid.as<Global_Alias_Module>()->alias);
  case ast::ENodeKind::Global_Generic:  return mod_mangle + "." + std::string(nodeid.as<Global_Generic>()->name);
  case ast::ENodeKind::Local_Lambda:    return mod_mangle + "." + std::string(nodeid.as<Local_Lambda>()->name);
  case ast::ENodeKind::Local_Parameter: return mod_mangle + "." + std::string(nodeid.as<Local_Parameter>()->name);
  case ast::ENodeKind::Local_Gen_Param_Elem:
    return mod_mangle + "." + std::string(nodeid.as<Local_Gen_Param_Elem>()->name);
  case ast::ENodeKind::Local_Binding:    return mod_mangle + "." + std::string(nodeid.as<Local_Binding>()->name);
  case ast::ENodeKind::Local_Variable:   return mod_mangle + "." + std::string(nodeid.as<Local_Variable>()->name);
  case ast::ENodeKind::Local_Capability: return mod_mangle + "." + std::string(nodeid.as<Local_Capability>()->name);
  case ast::ENodeKind::SFM_Facet:        return mod_mangle + "." + std::string(nodeid.as<SFM_Facet>()->name);
  case ast::ENodeKind::SFM_Facet_Field:  return mod_mangle + "." + std::string(nodeid.as<SFM_Facet_Field>()->name);
  case ast::ENodeKind::SFM_View:         return mod_mangle + "." + std::string(nodeid.as<SFM_View>()->name);
  case ast::ENodeKind::SFM_Form:         return mod_mangle + "." + std::string(nodeid.as<SFM_Form>()->name);
  case ast::ENodeKind::SFM_Rule:         return mod_mangle + "." + std::string(nodeid.as<SFM_Rule>()->name);
  default:                               assert(false && "Must be used on named node or with alias");
  }

#undef case_n
}

ast::ID ast::Arena::get_next_id() const noexcept
{
  return ast::ID::make(cuid, static_cast<uint32_t>(nodes.size()));
}


ast::Node& ast::get(ID id) noexcept
{
  auto& cu = id.cu().get();
  assert(cu.cuid && "Invalid compilation unit id");

  assert(id.offset() < cu.nodes->nodes.size() && "Offset out of bounds");
  return *cu.nodes->nodes[id.offset()];
}

[[nodiscard]] std::string ast::get_debug_str(ID id) noexcept
{
  assert(id && "Invalid id");

  const auto* n = id.get();

  assert(n && "Node not found");

  const std::string str_kind(magic_enum::enum_name<ast::ENodeKind>(n->kind()));
  const std::string str_decl_name(get_decl_name(id));

  return str_kind + ": " + str_decl_name;
}