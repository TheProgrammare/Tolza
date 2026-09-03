#include "ast.hpp"

#include "ast/data.hpp"
#include "ast/definition/ast_base.hpp"
#include "ast/definition/ast_declaration_extension.hpp"
#include "ast/definition/ast_declaration_global.hpp"
#include "ast/definition/ast_declaration_local.hpp"
#include "ast/definition/ast_declaration_sfm.hpp"
#include "binder/binder_ffi.hpp"
#include "compiler/compilation_unit.hpp"
#include "nexus/forward.hpp"
#include "nexus/ids.hpp"
#include "pool/module.hpp"
#include "pool/type.hpp"
#include "type/definition.hpp"

#include <Neargye/magic_enum.hpp>
#include <common/utils.hpp>
#include <cstdint>
#include <print>
#include <string>


std::vector<ast::ID> ast::get_parameters(ast::ID nodeid) noexcept
{
#define if_get_params(kind)                                                                                            \
  if (const auto* n = nodeid.as<ast::kind>()) return n->parameters;

  if_get_params(Global_Function);
  if_get_params(Local_Lambda);
  if_get_params(SFM_Rule);

  return {};

#undef if_get_params
}


std::string ast::get_decl_name(ast::ID nodeid) noexcept
{
#define case_n(kind)                                                                                                   \
  case ENodeKind::kind: return nodeid.as<ast::kind>()

  switch (nodeid.kind()) {
    case_n(Symbol_Id)->name;
    case_n(Symbol_Qualified)->name;
  case ENodeKind::Symbol_Type:
    return get_decl_name(nodeid.as<ast::Symbol_Type>()->name);
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
#define case_n(kind)                                                                                                   \
  case ENodeKind::kind: return nodeid.as<ast::kind>()->visibility

  switch (nodeid.kind()) {
    case_n(Global_Variable);
    case_n(Global_Function);
    case_n(Global_Extend_Fn);
    case_n(Global_Extend_Cast);
    case_n(Global_Extend_Op_Bin);
    case_n(Global_Extend_Op_Un);
    case_n(Global_Extend_Op_Subscript);
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

type::ID ast::get_decl_type(ast::ID nodeid) noexcept
{
#define case_n(kind)                                                                                                   \
  case ENodeKind::kind: return nodeid.as<ast::kind>()

  switch (nodeid.kind()) {
    case_n(Global_Variable)->type;
    case_n(Global_Function)->prototype;
    case_n(Global_Alias_Type)->type;
    case_n(Local_Lambda)->prototype;
    case_n(Local_Parameter)->type;
    case_n(Local_Binding)->type;
    case_n(Local_Variable)->type;
    case_n(Local_Capability)->type;
  default: assert(false && "Must be used on named node or with alias");
  }

#undef case_n
}


std::string ast::get_mangled_id(ast::ID nodeid) noexcept
{
  auto mod_mangle = module::mangle_canonical_module_path(nodeid.module());

#define case_n(kind)                                                                                                   \
  case ENodeKind::kind: return std::format("{}.{}", mod_mangle, nodeid.as<kind>()

  switch (nodeid.kind()) {
    case_n(Symbol_Id)->name);
    case_n(Symbol_Qualified)->name);
  case ENodeKind::Symbol_Type:
    return get_mangled_id(ast::ID::make(nodeid.cu(), nodeid.as<Symbol_Type>()->name.index()));
  case_n(Global_Variable)->name);
  case_n(Global_Function)->name);
  case_n(Global_Module)->name);
  case_n(Global_Extern)->abi);
  case_n(Global_Reexport)->alias);
  case_n(Enum_Field)->name);
  case_n(Global_Enum)->name);
  case_n(Flag_Field)->name);
  case_n(Global_Flag)->name);
  case_n(Union_Field)->name);
  case_n(Global_Union)->name);
  case_n(Global_Alias_Type)->alias);
  case_n(Global_Alias_Module)->alias);
  case_n(Global_Generic)->name);
  case_n(Local_Lambda)->name);
  case_n(Local_Parameter)->name);
  case_n(Local_Gen_Param_Elem)->name);
  case_n(Local_Binding)->name);
  case_n(Local_Variable)->name);
  case_n(Local_Capability)->name);
  case_n(SFM_Facet)->name);
  case_n(SFM_Facet_Field)->name);
  case_n(SFM_View)->name);
  case_n(SFM_Form)->name);
  case_n(SFM_Rule)->name);
  default: assert(false && "Must be used on named node or with alias");
  }

#undef case_n
}

ast::ID ast::Arena::get_next_id() const noexcept
{
  return ast::ID::make(cuid, static_cast<uint32_t>(nodes.size()));
}

std::string ast::debug_node_on_line(ID nodeid) noexcept
{
  assert(nodeid && "Invalid id");

  // e.g. path_to_file:10:30
  //      | var count = 0
  return std::format("{}:{}:{}\n| token: {}\n| {}:{}: {}", nodeid.cu().get().file_info.path, nodeid.token().line() + 1,
                     nodeid.token().col() + 1, nodeid.token().str(), nodeid.token().line() + 1,
                     nodeid.token().col() + 1, nodeid.token().line_str());
}

std::string ast::dump_debug(ID nodeid) noexcept
{
  assert(nodeid && "Invalid id");

  const auto n = nodeid.get();

  std::string str_kind(magic_enum::enum_name<ast::ENodeKind>(n.kind));

  if (ast::ENodeKind_is_declaration(n.kind)) {
    const std::string str_decl_name(get_decl_name(nodeid));
    return std::format("{}: {}", str_kind, str_decl_name);
  }

  return str_kind;
}

auto Import_to_str(const ast::Import& n) -> std::string
{
  std::string path;
  const auto* regex = n.regex.as<ast::Path_Regex>();

  path = std::format("{}::", cu::EFileSource_to_str(regex->source));

  for (const auto& elem : regex->elements) std::format_to(std::back_inserter(path), "{}::", elem);
  path = path.substr(0, path.size() - 2);

  return std::format(ffi::BINDER_IMPORT_TEMPLATE, path, regex->path.back());
};

auto Global_Reexport_to_str(const ast::Global_Reexport& n) -> std::string
{
  std::string path;

  const auto* regex = n.regex.as<ast::Path_Regex>();

  path = std::format("{}::", cu::EFileSource_to_str(regex->source));

  for (const auto& elem : regex->elements) std::format_to(std::back_inserter(path), "{}::", elem);
  path = path.substr(0, path.size() - 2);

  return std::format(ffi::BINDER_REEXPORT_TEMPLATE, path, regex->path.back());
};


auto SFM_Facet_to_str(const ast::SFM_Facet& n) -> std::string
{
  std::string members;

  for (auto nodeid : n.fields) {
    const auto* f = nodeid.as<ast::SFM_Facet_Field>();

    std::format_to(std::back_inserter(members), ffi::BINDER_EXTERN_FIELD, f->name, f->type.dump());
  }

  return std::format(ffi::BINDER_EXTERN_FACET_TEMPLATE, n.name, members);
};

auto SFM_Form_to_str(const ast::SFM_Form& n) -> std::string
{
  std::string members;

  for (auto nodeid : n.facets) {
    const auto* facet = nodeid.as<ast::SFM_Facet>();

    std::format_to(std::back_inserter(members), "{},\n", facet->name);
  }

  return std::format(ffi::BINDER_EXTERN_FORM_TEMPLATE, n.name, members);
};

auto Global_Union_to_str(const ast::Global_Union& n) -> std::string
{
  std::string members;

  for (const auto& f_id : n.variants) {
    const auto* uf = f_id.as<ast::Union_Field>();

    std::format_to(std::back_inserter(members), "{}: {},\n", uf->type.index(), uf->type.cu().offset());
  }

  return std::format(ffi::BINDER_EXTERN_UNION_TEMPLATE, n.name, members);
};

auto Global_Flag_to_str(const ast::Global_Flag& n) -> std::string
{
  std::string members;

  for (const auto& f_id : n.flags) {
    const auto* ff = f_id.as<ast::Flag_Field>();

    std::format_to(std::back_inserter(members), "{},\n", ff->name);
  }

  return std::format(ffi::BINDER_EXTERN_FLAG_TEMPLATE, n.name, type::TYPEID_usize.dump(), members);
};

auto Global_Enum_to_str(const ast::Global_Enum& n) -> std::string
{
  std::string members;

  for (const auto& v_id : n.variants) {
    const auto* v = v_id.as<ast::Enum_Field>();

    std::format_to(std::back_inserter(members), "{}({}),\n", v->name, v->type.dump());
  }

  return std::format(ffi::BINDER_EXTERN_ENUM_TEMPLATE, n.name, members);
};

auto Global_Function_to_str(const ast::Global_Function& n) -> std::string
{
  std::string params;
  const auto* proto = n.prototype.as<type::Prototype>();

  if (!proto->params.empty()) {
    for (size_t i = 0; i < proto->params.size(); i++) {
      const auto        param_ty = proto->params[i].type;
      const auto* const param    = n.parameters[i].as<ast::Local_Parameter>();
      assert(param);
      std::format_to(std::back_inserter(params), "{} {}: {}, ", ast::EPassMode_to_str(param->passmode), param->name,
                     param_ty.dump());
    }

    params = params.substr(0, params.size() - 2);
  }

  if (proto->is_variadic) {
    if (proto->params.size() > 0) params += ", ";
    params += "...";
  }

  return std::format(ffi::BINDER_EXTERN_FN_TEMPALTE, n.name, params, proto->ret.dump());
};

auto Global_Variable_to_str(const ast::Global_Variable& n) -> std::string
{
  return std::format(ffi::BINDER_EXTERN_GLOBAL_TEMPLATE, n.kind == ast::EVariableKind::_let ? "let" : "var", n.name,
                     n.type.dump());
};

auto Global_Alias_Type_to_str(const ast::Global_Alias_Type& n) -> std::string
{
  return std::format(ffi::BINDER_EXTERN_TYPEALIAS_TEMPLATE, n.alias, n.type ? n.type.dump() : "opaque");
};

std::string ast::dump(ID nodeid) noexcept
{
  assert(nodeid && "Invalid id");


#define case_n(kind)                                                                                                   \
  case ast::ENodeKind::kind: return kind##_to_str(*nodeid.as<ast::kind>());

  switch (nodeid.kind()) {
    case_n(Import);
    case_n(Global_Reexport);
    case_n(SFM_Facet);
    case_n(SFM_Form);
    case_n(Global_Union);
    case_n(Global_Flag);
    case_n(Global_Enum);
    case_n(Global_Function);
    case_n(Global_Variable);
    case_n(Global_Alias_Type);
  default: assert(false);
  }

#undef case_n
}

const type::Prototype* ast::get_prototype(ID id) noexcept
{
#define if_get_proto(kind)                                                                                             \
  if (const auto* ptr = id.as<ast::kind>()) return ptr->prototype.as<type::Prototype>();

  assert(id && "Invalid id");
  if_get_proto(Global_Function);
  if_get_proto(SFM_Rule);
  if_get_proto(Global_Extend_Fn);
  if_get_proto(Local_Lambda);

  assert(false && "No prototyped node");

  return NO_ID;

#undef if_get_proto
}
