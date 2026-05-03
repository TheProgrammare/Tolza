#include "binder_ffi.hpp"

#include <fstream>
#include <filesystem>
#include <iostream>
#include <stdexcept>

#include "nexus/ast/ast.hpp"
#include "common.hpp"
#include "compiler/compiler.hpp"
#include "compiler_options.hpp"
#include "misc/error_output.hpp"

namespace fs = std::filesystem;

std::string ffi::Bind_Package::get_file_path() const
{
  fs::path path = compiler::COMPILER_OPTIONS.get_dir_binding();
  if (!lang.empty()) path /= lang;
  if (!lib.empty()) path /= lib;
  path.replace_extension(".vlxbind");
  return path.string();
}


ffi::EPassMode ffi::type_to_passMode(const Type& ty)
{
  if (!ty.is_pointer && ty.base_type != ffi::EType::_comp && ty.base_type != ffi::EType::_proto
      && ty.base_type != ffi::EType::_union && ty.base_type != ffi::EType::_entity && ty.base_type != ffi::EType::_enum)
    return ffi::EPassMode::copy;

  if (ty.is_pointer && ty.is_val_type_const) return ffi::EPassMode::ref;

  if (ty.is_pointer) return ffi::EPassMode::mut;

  if (ty.is_pointer_double) return ffi::EPassMode::addr;

  return ffi::EPassMode::NONE;
}

std::string ffi::EPassMode_to_str(ffi::EPassMode pm)
{
  switch (pm) {
  case ffi::EPassMode::NONE: return "/*INVALID PASS MODE*/";
  case ffi::EPassMode::copy: return "copy";
  case ffi::EPassMode::ref:  return "ref";
  case ffi::EPassMode::mut:  return "mut";
  case ffi::EPassMode::move: return "move";
  case ffi::EPassMode::addr: return "addr";
  }
}

std::string ffi::EType_to_str(const EType ty)
{
  switch (ty) {
  case ffi::EType::INVALID:     return "/*INVALID TYPE*/";

  case ffi::EType::_i8:         return "i8";
  case ffi::EType::_i16:        return "i16";
  case ffi::EType::_i32:        return "i32";
  case ffi::EType::_i64:        return "i64";
  case ffi::EType::_i128:       return "i128";
  case ffi::EType::_isize:      return "isize";

  case ffi::EType::_u8:         return "u8";
  case ffi::EType::_u16:        return "u16";
  case ffi::EType::_u32:        return "u32";
  case ffi::EType::_u64:        return "u64";
  case ffi::EType::_u128:       return "u128";
  case ffi::EType::_usize:      return "usize";

  case ffi::EType::_b8:         return "b8";
  case ffi::EType::_b16:        return "b16";
  case ffi::EType::_b32:        return "b32";
  case ffi::EType::_b64:        return "b64";
  case ffi::EType::_b128:       return "b128";

  case ffi::EType::_f32:        return "f32";
  case ffi::EType::_f64:        return "f64";
  case ffi::EType::_f128:       return "f128";

  case ffi::EType::_cstr:       return "c_str";
  case ffi::EType::_str:        return "str";
  case ffi::EType::_text:       return "text";
  case ffi::EType::_cune:       return "cune";
  case ffi::EType::_rune:       return "rune";

  case ffi::EType::_schar:      return "ffi::C::_schar";
  case ffi::EType::_short:      return "ffi::C::_short";
  case ffi::EType::_long:       return "ffi::C::_long";
  case ffi::EType::_longlong:   return "ffi::C::_longlong";
  case ffi::EType::_int:        return "ffi::C::_int";

  case ffi::EType::_uchar:      return "ffi::C::_uchar";
  case ffi::EType::_ushort:     return "ffi::C::_ushort";
  case ffi::EType::_ulong:      return "ffi::C::_ulong";
  case ffi::EType::_ulonglong:  return "ffi::C::_ulonglong";
  case ffi::EType::_uint:       return "ffi::C::_uint";

  case ffi::EType::_ptrdiff:    return "ptrdiff";

  case ffi::EType::_float:      return "ffi::C::_float";
  case ffi::EType::_double:     return "ffi::C::_double";
  case ffi::EType::_longdouble: return "ffi::C::_longdouble";

  case ffi::EType::_bool:       return "bool";
  case ffi::EType::_void:       return "void";
  default:                      return "";
  }
}

std::string ffi::type_to_str(const Type& ty)
{
  std::string ptr;
  std::string type;
  std::string table_dim;

  if (ty.is_opaque()) return "ptr'void";
  if (ty.is_string()) return "c_str";

  // qualifiers
  if (ty.is_pointer) {
    if (ty.is_pointer_const) ptr += "$";
    if (ty.is_pointer_volatile) ptr += "!";
    ptr += "ptr'";
  } else if (ty.is_pointer_double) {
    ptr = "ptr'ptr'";
  }

  if (ty.is_table) {
    for (size_t i = 0; i < ty.table_size.size(); i++) {
      const size_t& size = ty.table_size[i];
      table_dim += std::to_string(size);
      if (i != ty.table_size.size() - 1) table_dim += ", ";
    }
    table_dim += "]";
  }

  if (ty.is_table_of_pointers) {
    ptr += "[";
  }

  // base types
  type = ffi::EType_to_str(ty.base_type);

  switch (ty.base_type) {
  case ffi::EType::_comp:
  case ffi::EType::_entity:
  case ffi::EType::_union:
  case ffi::EType::_flag:
  case ffi::EType::_enum:
  case ffi::EType::_alias:  type = ty.complex_type_name; break;
  case ffi::EType::_proto:  {
    if (ty.proto_type.get()) {
      std::string str_params;

      for (size_t i = 0; i < ty.proto_type->params.size(); i++) {
        auto& [pass_mode, type, _] = ty.proto_type->params[i];
        str_params += ffi::type_to_str(type);
        if (i != ty.proto_type->params.size() - 1) str_params += ", ";
      }

      std::string str_return = ffi::type_to_str(ty.proto_type->return_type);

      std::string fn(BINDER_PROTOTYPE_TEMPLATE);
      common::fmt_template(fn, {str_params, str_return});
      type = fn;
    } else {
      std::runtime_error("Undefined function type");
    }
    break;
  }
  default: break;
  }

  if (ty.is_val_type_const) type = "$" + type;
  if (ty.is_val_type_volatile) type = "!" + type;

  return ptr + type + table_dim;
}

std::string ffi::import_to_str(const Import& _imp)
{
  std::string type;
  std::string path;

  switch (_imp.type) {
  case Import::EImportType::pkg:     type = "pkg:"; break;
  case Import::EImportType::user:    type = "usr:"; break;
  case Import::EImportType::stdlib:  type = "std:"; break;
  case Import::EImportType::binding: type = "ext:"; break;
  case Import::EImportType::unknown: break;
  }

  for (auto& elem : _imp.path) path += elem + "::";
  path += _imp.name;

  std::string out(BINDER_IMPORT_TEMPLATE);
  common::fmt_template(out, {type, path});
  return out;
}


std::string ffi::comp_to_str(const Comp& p_comp)
{
  std::string members;

  for (size_t i = 0; i < p_comp.fields.size(); i++) {
    auto& [name, type] = p_comp.fields[i];
    std::string field(BINDER_EXTERN_FIELD);
    common::fmt_template(field, {name, type_to_str(type)});

    members += field;
  }

  std::string out(BINDER_EXTERN_COMP_TEMPLATE);
  common::fmt_template(out, {p_comp.name, members});
  return out;
}

std::string ffi::entity_to_str(const Entity& p_entity)
{
  std::string members;

  for (size_t i = 0; i < p_entity.components.size(); i++) {
    auto& comp = p_entity.components[i];
    members += "use " + comp.name + ", \n";
  }

  std::string out(BINDER_EXTERN_ENTITY_TEMPLATE);
  common::fmt_template(out, {p_entity.name, members});
  return out;
}

std::string ffi::union_to_str(const Union& p_union)
{
  std::string members;

  for (auto& [name, type] : p_union.members) {
    members += name + ": " + type_to_str(type) + ",\n";
  }

  bool test = members.empty() ? true : false;

  std::string out(BINDER_EXTERN_UNION_TEMPLATE);
  common::fmt_template(out, {p_union.name, members});
  return out;
}

std::string ffi::flag_to_str(const Flag& p_flag)
{
  std::string members;

  for (auto& [name, bits] : p_flag.members) {
    members += name + ": " + std::to_string(bits) + ",\n";
  }

  std::string out(BINDER_EXTERN_FLAG_TEMPLATE);
  common::fmt_template(out, {p_flag.name, ffi::EType_to_str(p_flag.underlying_type), members});
  return out;
}

std::string ffi::enum_to_str(const Enum& p_enum)
{
  std::string members;

  for (auto& [name, types] : p_enum.members) {
    members += name + "(";
    for (size_t i = 0; i < types.size(); ++i) {
      const Type& ty = types[i];
      members += type_to_str(ty);

      if (i != types.size() - 1) members += ", ";
    }

    members += "),\n";
  }

  std::string out(BINDER_EXTERN_ENUM_TEMPLATE);
  common::fmt_template(out, {p_enum.name, members});
  return out;
}

std::string ffi::func_to_str(const Func& p_func)
{
  std::string params;

  for (size_t i = 0; i < p_func.proto.params.size(); i++) {
    auto& [pass_mode, type, is_restrict] = p_func.proto.params[i];
    auto&       name                     = p_func.param_names[i];
    std::string str_pass_mode            = ffi::EPassMode_to_str(pass_mode);

    params += str_pass_mode + " " + name + ": " + type_to_str(type);

    if (i != p_func.proto.params.size() - 1) params += ", ";
  }

  if (p_func.proto.is_variadic) {
    if (p_func.proto.params.size() > 0) params += ", ";
    params += "...";
  }

  std::string out(BINDER_EXTERN_FN_TEMPALTE);
  common::fmt_template(out, {p_func.name, params, type_to_str(p_func.proto.return_type)});
  return out;
}

std::string ffi::global_to_str(const Global& p_glo)
{
  std::string kind = p_glo.is_const ? "let" : "var";

  std::string out(BINDER_EXTERN_GLOBAL_TEMPLATE);
  common::fmt_template(out, {kind, p_glo.name, type_to_str(p_glo.type)});
  return out;
}

std::string ffi::typealias_to_str(const TypeAlias& p_ty_alias)
{
  std::string out(BINDER_EXTERN_TYPEALIAS_TEMPLATE);
  common::fmt_template(out, {p_ty_alias.name, type_to_str(p_ty_alias.type)});
  return out;
}


void ffi::write_ast(const ffi::AST& p_ast, std::string_view p_dest_file)
{
  // if (!check_ast_generation(ast)) return;

  fs::create_directories(fs::path(p_dest_file).parent_path());
  std::ofstream os(p_dest_file.data());

  if (!os) throw std::runtime_error("Cannot open file: \"" + fs::path(p_dest_file).string() + "\"");

  os.clear();

  {
    std::string _lang = p_ast.bind.lang + std::string(labs(static_cast<long>(29 - p_ast.bind.lang.size())), ' ');
    std::string _lib  = p_ast.bind.lib + std::string(labs(static_cast<long>(29 - p_ast.bind.lib.size())), ' ');
    std::string _imp;

    if (!p_ast.imports.empty()) {
      _imp = ffi::BINDER_IMPORT_HEADER;

      for (auto& [name, import] : p_ast.imports) _imp += ffi::import_to_str(import);
    }

    std::string header(ffi::BINDER_FILE_HEADER);
    common::fmt_template(header, {_lang, _lib, _imp, p_ast.bind.abi});
    os << header << std::flush;
  }

  if (!p_ast.enums.empty()) {
    os << ffi::BINDER_ENUM_HEADER;

    for (auto& [name, elem] : p_ast.enums) {
      if (p_ast.bind.extern_items.contains(name)) os << ffi::enum_to_str(elem);
    }
  }
  if (!p_ast.comps.empty()) {
    os << ffi::BINDER_COMP_HEADER;

    for (auto& [name, elem] : p_ast.comps) {
      if (p_ast.bind.extern_items.contains(name)) os << ffi::comp_to_str(elem);
    }
  }
  if (!p_ast.unions.empty()) {
    os << ffi::BINDER_UNION_HEADER;

    for (auto& [name, elem] : p_ast.unions) {
      if (p_ast.bind.extern_items.contains(name)) os << union_to_str(elem);
    }
  }
  if (!p_ast.globals.empty()) {
    os << ffi::BINDER_GLOBAL_HEADER;

    for (auto& [name, elem] : p_ast.globals) {
      if (p_ast.bind.extern_items.contains(name)) os << global_to_str(elem);
    }
  }
  if (!p_ast.funcs.empty()) {
    os << ffi::BINDER_FUNCTION_HEADER;

    for (auto& [name, elem] : p_ast.funcs) {
      if (p_ast.bind.extern_items.contains(name)) os << func_to_str(elem);
    }
  }
  if (!p_ast.typealias.empty()) {
    os << ffi::BINDER_TYPEALIAS_HEADER;

    for (auto& [name, elem] : p_ast.typealias) {
      if (p_ast.bind.extern_items.contains(name)) os << typealias_to_str(elem);
    }
  }
  if (!p_ast.flags.empty()) {
    os << ffi::BINDER_FLAG_HEADER;

    for (auto& [name, elem] : p_ast.flags) {
      if (p_ast.bind.extern_items.contains(name)) os << flag_to_str(elem);
    }
  }
  if (!p_ast.entities.empty()) {
    os << ffi::BINDER_ENTITY_HEADER;

    for (auto& [name, elem] : p_ast.entities) {
      if (p_ast.bind.extern_items.contains(name)) os << entity_to_str(elem);
    }
  }

  os << "\n} // " << p_ast.bind.abi << "\n\n} // export" << std::flush;

  os.close();
}

bool ffi::check_ast_generation(const AST& p_ast)
{
  std::vector<std::string> errs;

  auto add_err = [&](const module::Extern_Item& item) {

  };

  // need change
  /*
  for (auto& item : p_ast.bind.extern_items) {
    bool find = false;
    for (auto& [name, fn] : p_ast.funcs) {
      if (name == item->declaration_name) find = true;
    }
    if (!find) {
      Error_Diagnostic err(*p_ast.bind.scr_info, 203, p_ast.bind.scr_info.get(), item->node_token,
                           compiler::EPhase::binder, "External reference never generated.",
                           "Check your workspace ressources, your packages, or the reference name.");
      errs.push_back(err.print_error());
    }
    break;
  }
    */

  for (auto& err : errs) {
    std::cerr << err;
  }

  return errs.empty();
}
