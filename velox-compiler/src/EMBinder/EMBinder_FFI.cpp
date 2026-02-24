#include "EMBinder_FFI.hpp"

#include <fstream>
#include <filesystem>
#include <stdexcept>

#include "Globals.hpp"

inline void FFI::fmt_template(std::string& templateStr, const std::vector<std::string>& args)
{
  for (size_t i = 0; i < args.size(); ++i) {
    std::string placeholder = "%" + std::to_string(i + 1);
    size_t      pos         = 0;
    while ((pos = templateStr.find(placeholder, pos)) != std::string::npos) {
      templateStr.replace(pos, placeholder.length(), args[i]);
      pos += args[i].length();
    }
  }
}

FFI::EPassMode FFI::type_to_passMode(const Type& ty)
{
  if (!ty.is_pointer && ty.base_type != FFI::EType::_comp && ty.base_type != FFI::EType::_proto
      && ty.base_type != FFI::EType::_union && ty.base_type != FFI::EType::_entity && ty.base_type != FFI::EType::_enum)
    return FFI::EPassMode::copy;

  if (ty.is_pointer && ty.is_val_type_const) return FFI::EPassMode::ref;

  if (ty.is_pointer) return FFI::EPassMode::mut;

  if (ty.is_pointer_double) return FFI::EPassMode::addr;

  return FFI::EPassMode::NONE;
}

std::string FFI::EPassMode_to_str(EPassMode pm)
{
  switch (pm) {
  case FFI::EPassMode::NONE: return "/*INVALID PASS MODE*/";
  case FFI::EPassMode::copy: return "copy";
  case FFI::EPassMode::ref:  return "ref";
  case FFI::EPassMode::mut:  return "mut";
  case FFI::EPassMode::move: return "move";
  case FFI::EPassMode::addr: return "addr";
  }
}

std::string FFI::EType_to_str(const EType ty)
{
  switch (ty) {
  case FFI::EType::INVALID:     return "/*INVALID TYPE*/";

  case FFI::EType::_i8:         return "i8";
  case FFI::EType::_i16:        return "i16";
  case FFI::EType::_i32:        return "i32";
  case FFI::EType::_i64:        return "i64";
  case FFI::EType::_i128:       return "i128";
  case FFI::EType::_isize:      return "isize";

  case FFI::EType::_u8:         return "u8";
  case FFI::EType::_u16:        return "u16";
  case FFI::EType::_u32:        return "u32";
  case FFI::EType::_u64:        return "u64";
  case FFI::EType::_u128:       return "u128";
  case FFI::EType::_usize:      return "usize";

  case FFI::EType::_b8:         return "b8";
  case FFI::EType::_b16:        return "b16";
  case FFI::EType::_b32:        return "b32";
  case FFI::EType::_b64:        return "b64";
  case FFI::EType::_b128:       return "b128";

  case FFI::EType::_f32:        return "f32";
  case FFI::EType::_f64:        return "f64";
  case FFI::EType::_f128:       return "f128";

  case FFI::EType::_str:        return "str";
  case FFI::EType::_text:       return "text";
  case FFI::EType::_ascii:      return "ascii";
  case FFI::EType::_utf32:      return "utf32";

  case FFI::EType::_schar:      return "FFI::C::_schar";
  case FFI::EType::_short:      return "FFI::C::_short";
  case FFI::EType::_long:       return "FFI::C::_long";
  case FFI::EType::_longlong:   return "FFI::C::_longlong";
  case FFI::EType::_int:        return "FFI::C::_int";

  case FFI::EType::_uchar:      return "FFI::C::_uchar";
  case FFI::EType::_ushort:     return "FFI::C::_ushort";
  case FFI::EType::_ulong:      return "FFI::C::_ulong";
  case FFI::EType::_ulonglong:  return "FFI::C::_ulonglong";
  case FFI::EType::_uint:       return "FFI::C::_uint";

  case FFI::EType::_ptrdiff:    return "ptrdiff";

  case FFI::EType::_float:      return "FFI::C::_float";
  case FFI::EType::_double:     return "FFI::C::_double";
  case FFI::EType::_longdouble: return "FFI::C::_longdouble";

  case FFI::EType::_bool:       return "bool";
  case FFI::EType::_void:       return "void";
  default:                      return "";
  }
}

std::string FFI::type_to_str(const Type& ty)
{
  std::string ptr;
  std::string type;
  std::string table_dim;

  if (ty.is_opaque()) return "ptr'void";
  if (ty.is_string()) return "FFI::C::_str";

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
  type = FFI::EType_to_str(ty.base_type);

  switch (ty.base_type) {
  case FFI::EType::_comp:
  case FFI::EType::_entity:
  case FFI::EType::_union:
  case FFI::EType::_flag:
  case FFI::EType::_enum:
  case FFI::EType::_alias:  type = ty.complex_type_name; break;
  case FFI::EType::_proto:  {
    if (ty.proto_type.get()) {
      std::string str_params;

      for (size_t i = 0; i < ty.proto_type->params.size(); i++) {
        auto& [pass_mode, type, _] = ty.proto_type->params[i];
        str_params += FFI::type_to_str(type);
        if (i != ty.proto_type->params.size() - 1) str_params += ", ";
      }

      std::string str_return = FFI::type_to_str(ty.proto_type->return_type);

      std::string fn = EMBINDER_PROTOTYPE_TEMPLATE;
      fmt_template(fn, {str_params, str_return});
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

std::string FFI::comp_to_str(const Comp& comp)
{
  std::string members;

  for (size_t i = 0; i < comp.fields.size(); i++) {
    auto& [name, type] = comp.fields[i];
    std::string field  = EMBINDER_EXTERN_FIELD;
    fmt_template(field, {name, type_to_str(type)});

    members += field;
  }

  std::string out = EMBINDER_EXTERN_COMP_TEMPLATE;
  fmt_template(out, {comp.name, members});
  return out;
}

std::string FFI::entity_to_str(const Entity& entity)
{
  std::string members;

  for (size_t i = 0; i < entity.components.size(); i++) {
    auto& comp = entity.components[i];
    members += "use " + comp.name + ", \n";
  }

  std::string out = EMBINDER_EXTERN_ENTITY_TEMPLATE;
  fmt_template(out, {entity.name, members});
  return out;
}

std::string FFI::union_to_str(const Union& _union)
{
  std::string members;

  for (auto& [name, type] : _union.members) {
    members += name + ": " + type_to_str(type) + ",\n";
  }

  bool test = members.empty() ? true : false;

  std::string out = EMBINDER_EXTERN_UNION_TEMPLATE;
  fmt_template(out, {_union.name, members});
  return out;
}

std::string FFI::flag_to_str(const Flag& flag)
{
  std::string members;

  for (auto& [name, bits] : flag.members) {
    members += name + ": " + std::to_string(bits) + ",\n";
  }

  std::string out = EMBINDER_EXTERN_FLAG_TEMPLATE;
  fmt_template(out, {flag.name, FFI::EType_to_str(flag.underlying_type), members});
  return out;
}

std::string FFI::enum_to_str(const Enum& _enum)
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

  std::string out = EMBINDER_EXTERN_ENUM_TEMPLATE;
  fmt_template(out, {_enum.name, members});
  return out;
}

std::string FFI::func_to_str(const Func& func)
{
  std::string params;

  for (size_t i = 0; i < func.proto.params.size(); i++) {
    auto& [pass_mode, type, is_restrict] = func.proto.params[i];
    auto&       name                     = func.param_names[i];
    std::string str_pass_mode            = FFI::EPassMode_to_str(pass_mode);

    params += str_pass_mode + " " + name + ": " + type_to_str(type);

    if (i != func.proto.params.size() - 1) params += ", ";
  }

  if (func.proto.is_variadic) {
    if (func.proto.params.size() > 0) params += ", ";
    params += "__args: ptr'void...";
  }

  std::string out = EMBINDER_EXTERN_FN_TEMPALTE;
  fmt_template(out, {func.name, params, type_to_str(func.proto.return_type)});
  return out;
}

std::string FFI::global_to_str(const Global& glo)
{
  std::string kind = glo.is_const ? "let" : "var";

  std::string out = EMBINDER_EXTERN_GLOBAL_TEMPLATE;
  fmt_template(out, {kind, glo.name, type_to_str(glo.type)});
  return out;
}

std::string FFI::typealias_to_str(const TypeAlias& _ty_alias)
{
  std::string out = EMBINDER_EXTERN_TYPEALIAS_TEMPLATE;
  fmt_template(out, {_ty_alias.name, type_to_str(_ty_alias.type)});
  return out;
}


void FFI::write_ast(const FFI::AST& ast, const std::string& target_path)
{
  std::ofstream os(target_path);

  if (!os.is_open()) throw std::runtime_error("Cannot open file: \"" + target_path + "\"");

  os.clear();

  std::string _lang  = ast.bind.lang + std::string(labs(static_cast<long>(29 - ast.bind.lang.size())), ' ');
  std::string _lib   = ast.bind.lib + std::string(labs(static_cast<long>(29 - ast.bind.lib.size())), ' ');
  std::string header = FFI::EMBINDER_FILE_HEADER;
  fmt_template(header, {_lang, _lib, ast.bind.lang});
  os << header << std::flush;

  if (!ast.enums.empty()) {
    os << FFI::EMBINDER_ENUM_HEADER;

    for (auto& elem : ast.flags) {
      os << FFI::flag_to_str(elem);
    }
  }
  if (!ast.comps.empty()) {
    os << FFI::EMBINDER_COMP_HEADER;

    for (auto& elem : ast.comps) {
      os << FFI::comp_to_str(elem);
    }
  }
  if (!ast.unions.empty()) {
    os << FFI::EMBINDER_UNION_HEADER;

    for (auto& elem : ast.unions) {
      os << union_to_str(elem);
    }
  }
  if (!ast.globals.empty()) {
    os << FFI::EMBINDER_GLOBAL_HEADER;

    for (auto& elem : ast.globals) {
      os << global_to_str(elem);
    }
  }
  if (!ast.funcs.empty()) {
    os << FFI::EMBINDER_FUNCTION_HEADER;

    for (auto& elem : ast.funcs) {
      os << func_to_str(elem);
    }
  }
  if (!ast.typealias.empty()) {
    os << FFI::EMBINDER_TYPEALIAS_HEADER;

    for (auto& elem : ast.typealias) {
      os << typealias_to_str(elem);
    }
  }
  if (!ast.flags.empty()) {
    os << FFI::EMBINDER_FLAG_HEADER;

    for (auto& elem : ast.flags) {
      os << flag_to_str(elem);
    }
  }
  if (!ast.entities.empty()) {
    os << FFI::EMBINDER_ENTITY_HEADER;

    for (auto& elem : ast.entities) {
      os << entity_to_str(elem);
    }
  }

  os << "\n}" << std::flush;

  os.close();

  std::filesystem::remove(target_path);
}
