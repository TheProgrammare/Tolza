#include "binder_ffi.hpp"

#include <cstddef>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>

#include <common/common.hpp>
#include <common/fileutils.hpp>
#include <common/time.hpp>
#include <common/utils.hpp>
#include <common/compiler_options.hpp>


#include "Neargye/magic_enum.hpp"
#include "ast/ast_base.hpp"
#include "ast/ast_declaration_sfm.hpp"
#include "ast/ast_declaration_global.hpp"
#include "ast/ast_declaration_local.hpp"
#include "nexus/ast/data.hpp"
#include "nexus/ast/definition.hpp"
#include "nexus/ast/ast.hpp"
#include "nexus/ast/forward.hpp"
#include "compiler/compiler.hpp"
#include "nexus/ids.hpp"
#include "compiler/compilation_unit.hpp"
#include "nexus/inference.hpp"
#include "nexus/type/data.hpp"
#include "nexus/type/definition.hpp"
#include "nexus/type/type.hpp"

namespace fs = std::filesystem;


const size_t ffi::AST::k_type_offset = type::TYPEID_text.index() + 1;


void ffi::AST::velox_codegen(std::string_view dest) const
{
  fs::create_directories(fs::path(dest).parent_path());
  fs::path f(dest);

  std::ofstream os(f, std::ios::out | std::ios::trunc);

  if (!os) common::FATAL_ERROR("Cannot open file: \"" + fs::path(dest).string() + "\"");

  std::vector<ast::Global_Reexport const*>   reexports;
  std::vector<ast::Import const*>            imports;
  std::vector<ast::Global_Enum const*>       enums;
  std::vector<ast::SFM_Facet const*>         facets;
  std::vector<ast::Global_Union const*>      unions;
  std::vector<ast::Global_Variable const*>   globals;
  std::vector<ast::Global_Function const*>   funcs;
  std::vector<ast::Global_Alias_Type const*> typealiases;
  std::vector<ast::Global_Flag const*>       flags;
  std::vector<ast::SFM_Form const*>          entities;

  // distribute nodes
  size_t count = 0;
  for (const auto& elem : nodes->nodes) {
    const auto id = ast::ID::make(cu::ID::main(), count++);
    switch (elem.kind()) {
    case ast::ENodeKind::Global_Reexport:   reexports.emplace_back(nodes->as<ast::Global_Reexport>(id)); break;
    case ast::ENodeKind::Import:            imports.emplace_back(nodes->as<ast::Import>(id)); break;
    case ast::ENodeKind::Global_Enum:       enums.emplace_back(nodes->as<ast::Global_Enum>(id)); break;
    case ast::ENodeKind::SFM_Facet:         facets.emplace_back(nodes->as<ast::SFM_Facet>(id)); break;
    case ast::ENodeKind::Global_Union:      unions.emplace_back(nodes->as<ast::Global_Union>(id)); break;
    case ast::ENodeKind::Global_Variable:   globals.emplace_back(nodes->as<ast::Global_Variable>(id)); break;
    case ast::ENodeKind::Global_Function:   funcs.emplace_back(nodes->as<ast::Global_Function>(id)); break;
    case ast::ENodeKind::Global_Alias_Type: typealiases.emplace_back(nodes->as<ast::Global_Alias_Type>(id)); break;
    case ast::ENodeKind::Global_Flag:       flags.emplace_back(nodes->as<ast::Global_Flag>(id)); break;
    case ast::ENodeKind::SFM_Form:          entities.emplace_back(nodes->as<ast::SFM_Form>(id)); break;
    default:                                continue;
    }
  }

  std::string date = common::time::now_datetime();

  std::map<std::string_view, std::string_view> header_fmt = {
      {"vc_version", common::VELOX_COMMON_VERSION},
      {"date",       date                        },
      {"language",   bind.lang                   },
      {"lib",        bind.lib                    },
      {"g_version",  "NONE"                      },
      {"author",     "NONE"                      },
      {"abi",        bind.lang                   },
  };


  std::string header(ffi::BINDER_FILE_HEADER);
  common::utils::fmt_template(header, header_fmt);
  os << header << std::flush;

  if (!reexports.empty()) {
    os << ffi::BINDER_REEXPORT_HEADER;

    for (const auto* elem : reexports) os << reexport_to_str(*elem);
  }

  if (!imports.empty()) {
    os << ffi::BINDER_IMPORT_HEADER;

    for (const auto* elem : imports) os << import_to_str(*elem);
  }

  if (!typealiases.empty()) {
    os << ffi::BINDER_TYPEALIAS_HEADER;

    for (const auto* elem : typealiases) os << typealias_to_str(*elem);
  }
  if (!enums.empty()) {
    os << ffi::BINDER_ENUM_HEADER;

    for (const auto* elem : enums) os << enum_to_str(*elem);
  }
  if (!facets.empty()) {
    os << ffi::BINDER_FACET_HEADER;

    for (const auto* elem : facets) os << facet_to_str(*elem);
  }
  if (!unions.empty()) {
    os << ffi::BINDER_UNION_HEADER;

    for (const auto* elem : unions) os << union_to_str(*elem);
  }
  if (!globals.empty()) {
    os << ffi::BINDER_GLOBAL_HEADER;

    for (const auto* elem : globals) os << global_to_str(*elem);
  }
  if (!funcs.empty()) {
    os << ffi::BINDER_FUNCTION_HEADER;

    for (const auto* elem : funcs) os << func_to_str(*elem);
  }

  if (!flags.empty()) {
    os << ffi::BINDER_FLAG_HEADER;

    for (const auto* elem : flags) os << flag_to_str(*elem);
  }
  if (!entities.empty()) {
    os << ffi::BINDER_FORM_HEADER;

    for (const auto* elem : entities) os << form_to_str(*elem);
  }

  os << "\n} // extern" << bind.abi << "\n\n} // export\n" << std::flush;

  os.close();
}


std::string ffi::Bind_Package::get_file_path() const noexcept
{
  fs::path path = compiler::OPTIONS.dir.get_dir_binding();
  if (!lang.empty()) path /= lang;
  if (!lib.empty()) path /= lib;
  path.replace_extension(common::fileutils::VELOX_FILE_EXTENSION);
  return path.string();
}


std::string ffi::AST::import_to_str(const ast::Import& _imp) const noexcept
{
  std::string path;
  _imp.nodeid().dump();
  const auto* regex = nodes->as<ast::Path_Regex>(_imp.regex);

  path = cu::EFileSource_to_str(regex->source);
  path += "::";

  for (const auto& elem : regex->elements) path += elem + "::";
  path = path.substr(0, path.size() - 2);

  std::string out(BINDER_IMPORT_TEMPLATE);
  common::utils::fmt_template(out, {path, regex->path.back()});
  return out;
}

std::string ffi::AST::reexport_to_str(const ast::Global_Reexport& _imp) const noexcept
{
  std::string path;

  const auto* regex = nodes->as<ast::Path_Regex>(_imp.regex);

  path = cu::EFileSource_to_str(regex->source);
  path += "::";

  for (const auto& elem : regex->elements) path += elem + "::";
  path = path.substr(0, path.size() - 2);

  std::string out(BINDER_REEXPORT_TEMPLATE);
  common::utils::fmt_template(out, {path, regex->path.back()});
  return out;
}


std::string ffi::AST::facet_to_str(const ast::SFM_Facet& p_facet) const noexcept
{
  std::string members;

  for (auto nodeid : p_facet.fields) {
    const auto* f = nodes->as<ast::SFM_Facet_Field>(nodeid);

    std::string field(BINDER_EXTERN_FIELD);
    common::utils::fmt_template(field, {f->name, type_to_str(types->get(f->type))});

    members += field;
  }

  std::string out(BINDER_EXTERN_FACET_TEMPLATE);
  common::utils::fmt_template(out, {p_facet.name, members});
  return out;
}

std::string ffi::AST::form_to_str(const ast::SFM_Form& p_form) const noexcept
{
  std::string members;

  for (auto nodeid : p_form.facets) {
    const auto* facet = nodes->as<ast::SFM_Facet>(nodeid);
    members += std::string(facet->name) + ", \n";
  }

  std::string out(BINDER_EXTERN_FORM_TEMPLATE);
  common::utils::fmt_template(out, {p_form.name, members});
  return out;
}

std::string ffi::AST::union_to_str(const ast::Global_Union& p_union) const noexcept
{
  std::string members;

  for (const auto& f_id : p_union.variants) {
    const auto* uf     = nodes->as<ast::Union_Field>(f_id);
    size_t      offset = uf->type.index();
    size_t      cu     = uf->type.cu().raw();
    assert(uf->type && "Invalid type");
    members += std::string(uf->name) + ": " + type_to_str(types->get(uf->type)) + ",\n";
  }

  std::string out(BINDER_EXTERN_UNION_TEMPLATE);
  common::utils::fmt_template(out, {p_union.name, members});
  return out;
}

std::string ffi::AST::flag_to_str(const ast::Global_Flag& p_flag) const noexcept
{
  std::string members;

  for (const auto& f_id : p_flag.flags) {
    const auto* ff = nodes->as<ast::Flag_Field>(f_id);
    members += std::string(ff->name) + ",\n";
  }

  std::string out(BINDER_EXTERN_FLAG_TEMPLATE);
  common::utils::fmt_template(out, {p_flag.name, type_to_str(type::TYPEID_usize.get()), members});
  return out;
}

std::string ffi::AST::enum_to_str(const ast::Global_Enum& p_enum) const noexcept
{
  std::string members;

  for (const auto& v_id : p_enum.variants) {
    const auto* v = nodes->as<ast::Enum_Field>(v_id);
    members += std::string(v->name) + "(" + type_to_str(types->get(v->type)) + "),\n";
  }

  std::string out(BINDER_EXTERN_ENUM_TEMPLATE);
  common::utils::fmt_template(out, {p_enum.name, members});
  return out;
}

std::string ffi::AST::func_to_str(const ast::Global_Function& p_func) const noexcept
{
  std::string params;
  const auto* proto = types->as<type::Prototype>(p_func.prototype);

  for (size_t i = 0; i < proto->params.size(); i++) {
    const auto& param_ty = types->get(proto->params[i].type);
    const auto* param    = nodes->as<ast::Local_Parameter>(p_func.parameters[i]);
    std::string str_pm(ast::EPassMode_to_str(param->passmode));

    params += str_pm + " " + std::string(param->name) + ": " + type_to_str(param_ty);

    if (i != proto->params.size() - 1) params += ", ";
  }

  if (proto->is_variadic) {
    if (proto->params.size() > 0) params += ", ";
    params += "...";
  }

  std::string out(BINDER_EXTERN_FN_TEMPALTE);
  common::utils::fmt_template(out, {p_func.name, params, type_to_str(types->get(proto->ret))});
  return out;
}

std::string ffi::AST::global_to_str(const ast::Global_Variable& p_glo) const noexcept
{
  std::string kind = p_glo.kind == ast::EVariableKind::_let ? "let" : "var";

  std::string out(BINDER_EXTERN_GLOBAL_TEMPLATE);
  common::utils::fmt_template(out, {kind, p_glo.name, type_to_str(types->get(p_glo.type))});
  return out;
}

std::string ffi::AST::typealias_to_str(const ast::Global_Alias_Type& p_ty_alias) const noexcept
{
  if (p_ty_alias.type) {
    std::string out(BINDER_EXTERN_TYPEALIAS_TEMPLATE);
    common::utils::fmt_template(out, {p_ty_alias.alias, type_to_str(types->get(p_ty_alias.type))});
    return out;
  }

  std::string out(BINDER_EXTERN_OPAQUE_TEMPLATE);
  common::utils::fmt_template(out, {p_ty_alias.alias});
  return out;
}

std::string ffi::AST::type_to_str(const type::Type& ty) const noexcept
{
  switch (ty.kind()) {
  case type::ETypeKind::NONE: return "";
  case type::ETypeKind::Primitive:
    return std::string(
               magic_enum::enum_name<type::EPrimitiveTypeKind>(static_cast<const type::Primitive*>(&ty)->primitive))
        .substr(1);
  case type::ETypeKind::String:
    return std::string(magic_enum::enum_name<type::ETextType>(static_cast<const type::String*>(&ty)->kind)).substr(1);
  case type::ETypeKind::Tuple: {
    std::string out;
    const auto* tu = static_cast<const type::Tuple*>(&ty);
    for (const auto& tyid : tu->elems) {
      out += type_to_str(types->get(tyid));
      out += ", ";
    }

    return "(" + out.substr(0, out.size() - 2) + ")";
  }
  case type::ETypeKind::Array: {
    const auto* ptr = static_cast<const type::Array*>(&ty);
    return "[" + type_to_str(types->get(ptr->inner)) + "; " + std::to_string(ptr->size) + "]";
  }
  case type::ETypeKind::Buffer: {
    const auto* ptr = static_cast<const type::Buffer*>(&ty);
    return "[" + type_to_str(types->get(ptr->inner)) + "; _]";
  }
  case type::ETypeKind::Slice: {
    const auto*       ptr   = static_cast<const type::Slice*>(&ty);
    const std::string right = ptr->is_c_table ? "c" : "..";
    return "[" + type_to_str(types->get(ptr->inner)) + "; " + right + "]";
  }
  case type::ETypeKind::Ptr: {
    const auto* ptr = static_cast<const type::Ptr*>(&ty);
    return "ptr'" + type_to_str(types->get(ptr->inner));
  }
  case type::ETypeKind::Prototype: {
    std::string params;
    const auto* proto = static_cast<const type::Prototype*>(&ty);
    for (const auto& param : proto->params) {
      params += magic_enum::enum_name(param.passmode);
      params += " ";
      params += type_to_str(types->get(param.type));
      params += ", ";
    }
    params = params.substr(0, params.size() - 2);

    return "fn(" + params + ") -> " + type_to_str(types->get(proto->ret));
  }
  case type::ETypeKind::Flag: {
    auto nodeid = inferences->get_declaration(ty.tyid);
    assert(nodeid && "Type must refer to an declaration");
    const auto* node = nodes->as<ast::Global_Flag>(nodeid);
    assert(node && "type node must be a flag");

    std::string fields;
    for (auto fid : node->flags) {
      const auto* fnode = nodes->as<ast::Flag_Field>(fid);
      assert(fnode && "type node must be a flag field");

      fields += "  " + fnode->name + ",\n";
    }

    return "flag " + node->name + "{\n" + fields + "}\n";
  }
  case type::ETypeKind::Enum: {
    auto nodeid = inferences->get_declaration(ty.tyid);
    assert(nodeid && "Type must refer to an declaration");
    const auto* node = nodes->as<ast::Global_Enum>(nodeid);
    assert(node && "type node must be a enum");

    std::string fields;
    for (auto vid : node->variants) {
      const auto* vnode = nodes->as<ast::Enum_Field>(vid);
      assert(vnode && "type node must be a enum field");

      if (!vnode->type) {
        fields += "  " + vnode->name + "(),\n";
        continue;
      }

      fields += "  " + vnode->name + "(" + type_to_str(types->get(vnode->type)) + "),\n";
    }

    return "enum " + node->name + "{\n" + fields + "}\n";
  }
  case type::ETypeKind::Union: {
    auto nodeid = inferences->get_declaration(ty.tyid);
    assert(nodeid && "Type must refer to an declaration");
    const auto* node = nodes->as<ast::Global_Union>(nodeid);
    assert(node && "type node must be a union");

    std::string fields;
    for (auto vid : node->variants) {
      const auto* vnode = nodes->as<ast::Union_Field>(vid);
      assert(vnode && "type node must be a union field");

      fields += "  " + vnode->name + ": " + type_to_str(types->get(vnode->type)) + ",\n";
    }

    return "union " + node->name + "{\n" + fields + "}\n";
  }
  case type::ETypeKind::Facet: {
    auto nodeid = inferences->get_declaration(ty.tyid);
    assert(nodeid && "Type must refer to an declaration");
    const auto* node = nodes->as<ast::SFM_Facet>(nodeid);
    assert(node && "type node must be a facet");

    std::string fields;
    for (const auto& fid : node->fields) {
      const auto* f_node = nodes->as<ast::SFM_Facet_Field>(fid);
      assert(f_node && "a facet must have field nodes");

      fields += std::string(f_node->name) + ": " + type_to_str(types->get(f_node->type)) + "\n";
    }

    return "facet" + std::string(node->name) + " {\n" + fields + "}\n";
  }
  case type::ETypeKind::View: {
    auto nodeid = inferences->get_declaration(ty.tyid);
    assert(nodeid && "Type must refer to an declaration");
    const auto* node = nodes->as<ast::SFM_View>(nodeid);
    assert(node && "type node must be a view");
  }
  case type::ETypeKind::Identifier: {
    const auto* id = static_cast<const type::Identifier*>(&ty);
    assert(!id->forward_name.empty() && "A name is mandatory");
    return id->forward_name;
  }
  case type::ETypeKind::Form: break;
  }

  return "";
}


bool ffi::check_ast_generation(const AST& ast) noexcept
{
  std::vector<std::string> errs;

  auto add_err = [&](const module::Extern_Item& item) {

  };

  // need change
  /*
  for (const auto& item : ast.bind.extern_items) {
    bool find = false;
    for (const auto& [name, fn] : ast.funcs) {
      if (name == item->declaration_name) find = true;
    }
    if (!find) {
      Error_Diagnostic err(*ast.bind.CU, 203, ast.bind.CU.get(), item->node_token,
                           compiler::EPhase::binder, "External reference never generated.",
                           "Check your workspace ressources, your packages, or the reference name.");
      errs.emplace_back(err.print_error());
    }
    break;
  }
    */

  for (const auto& err : errs) {
    std::cerr << err;
  }

  return errs.empty();
}

ffi::AST::AST()
  : nodes(new ast::Arena(cu::ID::make(-1)))
  , types(new type::Arena(cu::ID::make(-1)))
  , inferences(new inference::Arena())
{
}


template <typename T>
T& ffi::AST::add_get_node() noexcept
{
  return nodes->add_get<T>();
}


template <typename T>
T& ffi::AST::add_get_type(const type::Qualifier& dec) noexcept
{
  static_assert(type::IsDataType<T>, "Must be a type");

  auto ty       = std::make_unique<T>();
  ty->qualifier = dec;
  // assume compilation unit doesn't exists already, focus on main compilation unit
  ty->tyid      = type::ID::make(cu::ID::main(), types->types.size() + ffi::AST::k_type_offset);

  T* raw = static_cast<T*>(ty.get());

  types->types.emplace_back(std::move(ty));

  return *raw;
}

#define AST_ADD_NODE(T) template T& ffi::AST::add_get_node<T>() noexcept;


AST_ADD_NODE(ast::Global_Reexport)
AST_ADD_NODE(ast::Import)
AST_ADD_NODE(ast::Global_Enum)
AST_ADD_NODE(ast::SFM_Facet)
AST_ADD_NODE(ast::Global_Union)
AST_ADD_NODE(ast::Global_Variable)
AST_ADD_NODE(ast::Global_Function)
AST_ADD_NODE(ast::Global_Alias_Type)
AST_ADD_NODE(ast::Global_Flag)
AST_ADD_NODE(ast::SFM_Form)
AST_ADD_NODE(ast::Symbol_Id)
AST_ADD_NODE(ast::Local_Parameter)
AST_ADD_NODE(ast::SFM_Facet_Field)
AST_ADD_NODE(ast::Union_Field)
AST_ADD_NODE(ast::Flag_Field)
AST_ADD_NODE(ast::Enum_Field)
AST_ADD_NODE(ast::Literal_Record)

#undef AST_ADD_NODE


#define ADD_GET_TYPE(T) template T& ffi::AST::add_get_type<T>(const type::Qualifier& dec) noexcept;

ADD_GET_TYPE(type::Primitive)
ADD_GET_TYPE(type::String)
ADD_GET_TYPE(type::Tuple)
ADD_GET_TYPE(type::Array)
ADD_GET_TYPE(type::Buffer)
ADD_GET_TYPE(type::Slice)
ADD_GET_TYPE(type::Ptr)
ADD_GET_TYPE(type::Prototype)
ADD_GET_TYPE(type::Facet)
ADD_GET_TYPE(type::View)
ADD_GET_TYPE(type::Form)
ADD_GET_TYPE(type::Enum)
ADD_GET_TYPE(type::Flag)
ADD_GET_TYPE(type::Union)
ADD_GET_TYPE(type::Identifier)

#undef ADD_GET_TYPE
