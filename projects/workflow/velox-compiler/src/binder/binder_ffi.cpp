#include "binder_ffi.hpp"

#include <fstream>
#include <filesystem>
#include <iostream>
#include <stdexcept>

#include "ast/ast_base.hpp"
#include "common.hpp"
#include "compiler/compiler.hpp"
#include "misc/error_output.hpp"

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

      std::string fn = BINDER_PROTOTYPE_TEMPLATE;
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
  case Import::EImportType::ext:     type = "ext:"; break;
  case Import::EImportType::unknown: break;
  }

  for (auto& elem : _imp.path) path += elem + "::";
  path += _imp.name;

  std::string out = BINDER_IMPORT_TEMPLATE;
  common::fmt_template(out, {type, path});
  return out;
}


std::string ffi::comp_to_str(const Comp& comp)
{
  std::string members;

  for (size_t i = 0; i < comp.fields.size(); i++) {
    auto& [name, type] = comp.fields[i];
    std::string field  = BINDER_EXTERN_FIELD;
    common::fmt_template(field, {name, type_to_str(type)});

    members += field;
  }

  std::string out = BINDER_EXTERN_COMP_TEMPLATE;
  common::fmt_template(out, {comp.name, members});
  return out;
}

std::string ffi::entity_to_str(const Entity& entity)
{
  std::string members;

  for (size_t i = 0; i < entity.components.size(); i++) {
    auto& comp = entity.components[i];
    members += "use " + comp.name + ", \n";
  }

  std::string out = BINDER_EXTERN_ENTITY_TEMPLATE;
  common::fmt_template(out, {entity.name, members});
  return out;
}

std::string ffi::union_to_str(const Union& _union)
{
  std::string members;

  for (auto& [name, type] : _union.members) {
    members += name + ": " + type_to_str(type) + ",\n";
  }

  bool test = members.empty() ? true : false;

  std::string out = BINDER_EXTERN_UNION_TEMPLATE;
  common::fmt_template(out, {_union.name, members});
  return out;
}

std::string ffi::flag_to_str(const Flag& flag)
{
  std::string members;

  for (auto& [name, bits] : flag.members) {
    members += name + ": " + std::to_string(bits) + ",\n";
  }

  std::string out = BINDER_EXTERN_FLAG_TEMPLATE;
  common::fmt_template(out, {flag.name, ffi::EType_to_str(flag.underlying_type), members});
  return out;
}

std::string ffi::enum_to_str(const Enum& _enum)
{
  std::string members;

  for (auto& [name, types] : _enum.members) {
    members += name + "(";
    for (size_t i = 0; i < types.size(); ++i) {
      const Type& ty = types[i];
      members += type_to_str(ty);

      if (i != types.size() - 1) members += ", ";
    }

    members += "),\n";
  }

  std::string out = BINDER_EXTERN_ENUM_TEMPLATE;
  common::fmt_template(out, {_enum.name, members});
  return out;
}

std::string ffi::func_to_str(const Func& func)
{
  std::string params;

  for (size_t i = 0; i < func.proto.params.size(); i++) {
    auto& [pass_mode, type, is_restrict] = func.proto.params[i];
    auto&       name                     = func.param_names[i];
    std::string str_pass_mode            = ffi::EPassMode_to_str(pass_mode);

    params += str_pass_mode + " " + name + ": " + type_to_str(type);

    if (i != func.proto.params.size() - 1) params += ", ";
  }

  if (func.proto.is_variadic) {
    if (func.proto.params.size() > 0) params += ", ";
    params += "...";
  }

  std::string out = BINDER_EXTERN_FN_TEMPALTE;
  common::fmt_template(out, {func.name, params, type_to_str(func.proto.return_type)});
  return out;
}

std::string ffi::global_to_str(const Global& glo)
{
  std::string kind = glo.is_const ? "let" : "var";

  std::string out = BINDER_EXTERN_GLOBAL_TEMPLATE;
  common::fmt_template(out, {kind, glo.name, type_to_str(glo.type)});
  return out;
}

std::string ffi::typealias_to_str(const TypeAlias& _ty_alias)
{
  std::string out = BINDER_EXTERN_TYPEALIAS_TEMPLATE;
  common::fmt_template(out, {_ty_alias.name, type_to_str(_ty_alias.type)});
  return out;
}


void ffi::write_ast(const ffi::AST& ast, const std::string& dest_file)
{
  // if (!check_ast_generation(ast)) return;

  std::filesystem::create_directories(std::filesystem::path(dest_file).parent_path());
  std::ofstream os(dest_file);

  if (!os) throw std::runtime_error("Cannot open file: \"" + std::filesystem::path(dest_file).string() + "\"");

  os.clear();

  {
    std::string _lang = ast.bind.lang + std::string(labs(static_cast<long>(29 - ast.bind.lang.size())), ' ');
    std::string _lib  = ast.bind.lib + std::string(labs(static_cast<long>(29 - ast.bind.lib.size())), ' ');
    std::string _imp;

    if (!ast.imports.empty()) {
      _imp = ffi::BINDER_IMPORT_HEADER;

      for (auto& [name, import] : ast.imports) _imp += ffi::import_to_str(import);
    }

    std::string header = ffi::BINDER_FILE_HEADER;
    common::fmt_template(header, {_lang, _lib, _imp, ast.bind.abi});
    os << header << std::flush;
  }

  if (!ast.enums.empty()) {
    os << ffi::BINDER_ENUM_HEADER;

    for (auto& [_, elem] : ast.enums) os << ffi::enum_to_str(elem);
  }
  if (!ast.comps.empty()) {
    os << ffi::BINDER_COMP_HEADER;

    for (auto& [_, elem] : ast.comps) os << ffi::comp_to_str(elem);
  }
  if (!ast.unions.empty()) {
    os << ffi::BINDER_UNION_HEADER;

    for (auto& [_, elem] : ast.unions) os << union_to_str(elem);
  }
  if (!ast.globals.empty()) {
    os << ffi::BINDER_GLOBAL_HEADER;

    for (auto& [_, elem] : ast.globals) os << global_to_str(elem);
  }
  if (!ast.funcs.empty()) {
    os << ffi::BINDER_FUNCTION_HEADER;

    for (auto& [_, elem] : ast.funcs) os << func_to_str(elem);
  }
  if (!ast.typealias.empty()) {
    os << ffi::BINDER_TYPEALIAS_HEADER;

    for (auto& [_, elem] : ast.typealias) os << typealias_to_str(elem);
  }
  if (!ast.flags.empty()) {
    os << ffi::BINDER_FLAG_HEADER;

    for (auto& [_, elem] : ast.flags) os << flag_to_str(elem);
  }
  if (!ast.entities.empty()) {
    os << ffi::BINDER_ENTITY_HEADER;

    for (auto& [_, elem] : ast.entities) os << entity_to_str(elem);
  }

  os << "\n} // " << ast.bind.abi << "\n\n} // export" << std::flush;

  os.close();
}

bool ffi::check_ast_generation(const AST& ast)
{
  std::vector<std::string> errs;

  auto add_err = [&](const Extern_Item& item) {
    Error_Diagnostic err(203, *ast.bind.scr_info, item.id_node->_token, {}, compiler::EPhase::binder,
                         EErrorSeverity::error, {}, "External reference never generated.",
                         "Check your workspace ressources, your packages, or the reference name.");
    errs.push_back(err.print_error());
  };

  for (auto& item : ast.bind.extern_fn) {
    bool find = false;
    for (auto& [name, fn] : ast.funcs) {
      if (name == item.name) find = true;
    }
    if (!find) add_err(item);
    break;
  }

  for (auto& err : errs) {
    std::cerr << err;
  }

  return errs.empty();
}
