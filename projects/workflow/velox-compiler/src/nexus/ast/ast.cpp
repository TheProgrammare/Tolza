#include "ast.hpp"

#include "ast/ast_base.hpp"
#include "ast/ast_declaration_cop.hpp"
#include "ast/ast_declaration_global.hpp"
#include "ast/ast_declaration_local.hpp"
#include "compiler/compiler.hpp"
#include "nexus/forward.hpp"
#include "nexus/module.hpp"
#include "nexus/lexer/token.hpp"
#include "nexus/scope.hpp"
#include "nexus/script.hpp"
#include "nexus/pipeline.hpp"


std::string_view ast::ScriptArena::Tools::get_node_declaration_name(_id id) const
{
  auto& n = arena.get(id);

#define case_n(kind)                                                                                                   \
  case ENodeKind::kind: return arena.get_as<ast::kind>(id)

  switch (n.kind()) {
    case_n(ID)->name;
    case_n(ID_Qualified)->name;
  case ENodeKind::ID_Typed:
    return get_node_declaration_name(arena.get_as<ast::ID_Typed>(id)->name.get_node_id());
    case_n(Global_Variable)->name;
    case_n(Global_Function)->name;
    case_n(Global_Module)->name;
    case_n(Global_Extern)->api;
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
    case_n(COP_Component)->name;
    case_n(COP_Component_Field)->name;
    case_n(COP_Role)->name;
    case_n(COP_Entity)->name;
    case_n(COP_System)->name;
  default: return "";
  }

#undef case_n
}


std::string ast::Arena::Tools::get_declaration_mangle_name(ast::_gnid gnid) const
{
  auto& n          = arena.get(gnid);
  auto& scp        = compiler::COMPILER.scopes.get(n.scope_id);
  auto  mod_mangle = compiler::COMPILER.modules.tools.mangle_name(scp.module_id);

#define case_n(kind)                                                                                                   \
  case ENodeKind::kind: return mod_mangle + "." + arena.get_as<ast::kind>(gnid)

  switch (n.kind()) {
  case ast::ENodeKind::ID:           return mod_mangle + "." + std::string(arena.get_as<ast::ID>(gnid)->name);
  case ast::ENodeKind::ID_Qualified: return mod_mangle + "." + std::string(arena.get_as<ast::ID_Qualified>(gnid)->name);
  case ENodeKind::ID_Typed:
    return get_declaration_mangle_name(
        ast::GNID_Factory::make_gnid(scp.scr_id, arena.get_as<ast::ID_Typed>(gnid)->name.get_node_id()));
  case ast::ENodeKind::Global_Variable:
    return mod_mangle + "." + std::string(arena.get_as<ast::Global_Variable>(gnid)->name);
  case ast::ENodeKind::Global_Function:
    return mod_mangle + "." + std::string(arena.get_as<ast::Global_Function>(gnid)->name);
  case ast::ENodeKind::Global_Module:
    return mod_mangle + "." + std::string(arena.get_as<ast::Global_Module>(gnid)->name);
  case ast::ENodeKind::Global_Extern:
    return mod_mangle + "." + std::string(arena.get_as<ast::Global_Extern>(gnid)->api);
  case ast::ENodeKind::Global_Reexport:
    return mod_mangle + "." + std::string(arena.get_as<ast::Global_Reexport>(gnid)->alias);
  case ast::ENodeKind::Global_Enum:  return mod_mangle + "." + std::string(arena.get_as<ast::Global_Enum>(gnid)->name);
  case ast::ENodeKind::Global_Flag:  return mod_mangle + "." + std::string(arena.get_as<ast::Global_Flag>(gnid)->name);
  case ast::ENodeKind::Global_Union: return mod_mangle + "." + std::string(arena.get_as<ast::Global_Union>(gnid)->name);
  case ast::ENodeKind::Global_Alias_Type:
    return mod_mangle + "." + std::string(arena.get_as<ast::Global_Alias_Type>(gnid)->alias);
  case ast::ENodeKind::Global_Alias_Module:
    return mod_mangle + "." + std::string(arena.get_as<ast::Global_Alias_Module>(gnid)->alias);
  case ast::ENodeKind::Global_Generic:
    return mod_mangle + "." + std::string(arena.get_as<ast::Global_Generic>(gnid)->name);
  case ast::ENodeKind::Local_Lambda: return mod_mangle + "." + std::string(arena.get_as<ast::Local_Lambda>(gnid)->name);
  case ast::ENodeKind::Local_Parameter:
    return mod_mangle + "." + std::string(arena.get_as<ast::Local_Parameter>(gnid)->name);
  case ast::ENodeKind::Local_Gen_Param_Elem:
    return mod_mangle + "." + std::string(arena.get_as<ast::Local_Gen_Param_Elem>(gnid)->name);
  case ast::ENodeKind::Local_Binding:
    return mod_mangle + "." + std::string(arena.get_as<ast::Local_Binding>(gnid)->name);
  case ast::ENodeKind::Local_Variable:
    return mod_mangle + "." + std::string(arena.get_as<ast::Local_Variable>(gnid)->name);
  case ast::ENodeKind::Local_Capability:
    return mod_mangle + "." + std::string(arena.get_as<ast::Local_Capability>(gnid)->name);
  case ast::ENodeKind::COP_Component:
    return mod_mangle + "." + std::string(arena.get_as<ast::COP_Component>(gnid)->name);
  case ast::ENodeKind::COP_Component_Field:
    return mod_mangle + "." + std::string(arena.get_as<ast::COP_Component_Field>(gnid)->name);
  case ast::ENodeKind::COP_Role:   return mod_mangle + "." + std::string(arena.get_as<ast::COP_Role>(gnid)->name);
  case ast::ENodeKind::COP_Entity: return mod_mangle + "." + std::string(arena.get_as<ast::COP_Entity>(gnid)->name);
  case ast::ENodeKind::COP_System: return mod_mangle + "." + std::string(arena.get_as<ast::COP_System>(gnid)->name);
  default:                         return "";
  }

#undef case_n
}

ast::Node& ast::Arena::get(_gnid gnid)
{
  auto& scr = compiler::COMPILER.pipeline.get_script(gnid.get_script_id());
  return scr.nodes->get(gnid.get_node_id());
}

ast::_gnid ast::ScriptArena::get_next_gnid() const
{
  return GNID_Factory::make_gnid(scr.id, ast::_id(nodes.size()));
}
