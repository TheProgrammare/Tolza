#include "ffi-json_reader.hpp"

#include <nlohmann/json.hpp>

#include <fstream>
#include <unordered_map>

// Helpers pour convertir string → enum
ffi::EType ffi::JSON::str_to_etype(const std::string& s)
{
  static const std::unordered_map<std::string, EType> table = {
      {"",           EType::INVALID    },
      {"i8",         EType::_i8        },
      {"i16",        EType::_i16       },
      {"i32",        EType::_i32       },
      {"i64",        EType::_i64       },
      {"i128",       EType::_i128      },
      {"isize",      EType::_isize     },
      {"u8",         EType::_u8        },
      {"u16",        EType::_u16       },
      {"u32",        EType::_u32       },
      {"u64",        EType::_u64       },
      {"u128",       EType::_u128      },
      {"usize",      EType::_usize     },
      {"b8",         EType::_b8        },
      {"b16",        EType::_b16       },
      {"b32",        EType::_b32       },
      {"b64",        EType::_b64       },
      {"b128",       EType::_b128      },
      {"bsize",      EType::_bsize     },
      {"f32",        EType::_f32       },
      {"f64",        EType::_f64       },
      {"f128",       EType::_f128      },
      {"fsize",      EType::_fsize     },
      {"str",        EType::_str       },
      {"text",       EType::_text      },
      {"ascii",      EType::_ascii     },
      {"utf32",      EType::_utf32     },
      {"schar",      EType::_schar     },
      {"short",      EType::_short     },
      {"long",       EType::_long      },
      {"longlong",   EType::_longlong  },
      {"int",        EType::_int       },
      {"uchar",      EType::_uchar     },
      {"ushort",     EType::_ushort    },
      {"ulong",      EType::_ulong     },
      {"ulonglong",  EType::_ulonglong },
      {"uint",       EType::_uint      },
      {"float",      EType::_float     },
      {"double",     EType::_double    },
      {"longdouble", EType::_longdouble},
      {"sc",         EType::_schar     },
      {"s",          EType::_short     },
      {"l",          EType::_long      },
      {"ll",         EType::_longlong  },
      {"i",          EType::_int       },
      {"uc",         EType::_uchar     },
      {"us",         EType::_ushort    },
      {"ul",         EType::_ulong     },
      {"ull",        EType::_ulonglong },
      {"ui",         EType::_uint      },
      {"f",          EType::_float     },
      {"d",          EType::_double    },
      {"ld",         EType::_longdouble},
      {"ptrdiff",    EType::_ptrdiff   },
      {"void",       EType::_void      },
      {"bool",       EType::_bool      },
      {"enum",       EType::_enum      },
      {"comp",       EType::_comp      },
      {"entity",     EType::_entity    },
      {"union",      EType::_union     },
      {"flag",       EType::_flag      },
      {"proto",      EType::_proto     },
      {"protoype",   EType::_proto     },
      {"alias",      EType::_alias     },
  };
  auto it = table.find(s);
  return it != table.end() ? it->second : EType::INVALID;
}

ffi::EPassMode ffi::JSON::str_to_passmode(const std::string& s)
{
  if (s == "copy") return EPassMode::copy;
  if (s == "ref") return EPassMode::ref;
  if (s == "mut") return EPassMode::mut;
  if (s == "move") return EPassMode::move;
  if (s == "addr") return EPassMode::addr;
  return EPassMode::NONE;
}

ffi::Type ffi::JSON::json_to_type(const json& j)
{
  Type t;
  t.base_type            = str_to_etype(j.value("base_type", ""));
  t.complex_type_name    = j.value("complex_type_name", "");
  t.is_pointer           = j.value("is_pointer", false);
  t.is_pointer_double    = j.value("is_pointer_double", false);
  t.is_pointer_const     = j.value("is_pointer_const", false);
  t.is_pointer_volatile  = j.value("is_pointer_volatile", false);
  t.is_table             = j.value("is_table", false);
  t.is_table_of_pointers = j.value("is_table_of_pointers", false);
  t.is_val_type_const    = j.value("is_val_type_const", false);
  t.is_val_type_volatile = j.value("is_val_type_volatile", false);
  t.is_atomic            = j.value("is_atomic", false);

  if (j.contains("table_size")) {
    for (auto& sz : j["table_size"]) t.table_size.push_back(sz.get<size_t>());
  }

  // TODO: parse func_type, comp_type, entity_type, union_type, flag_type si nécessaires
  return t;
}

ffi::Prototype ffi::JSON::json_to_prototype(const json& j)
{
  Prototype proto;
  if (j.contains("return_type")) {
    proto.return_type = json_to_type(j["return_type"]);
  } else {
    Type ty;
    ty.base_type      = EType::_void;
    proto.return_type = std::move(ty);
  }

  proto.is_variadic = j.value("is_variadic", false);

  if (j.contains("params")) {
    for (auto& p : j["params"]) {
      EPassMode pm            = str_to_passmode(p[0].get<std::string>());
      Type      t             = json_to_type(p[1]);
      bool      restrict_flag = p[2].get<bool>();
      proto.params.emplace_back(pm, std::move(t), restrict_flag);
    }
  }

  return proto;
}

ffi::CallConvention ffi::JSON::str_to_callconvention(const std::string& s)
{
  if (s == "C") return CallConvention::C;
  if (s == "std_call") return CallConvention::Stdcall;
  if (s == "fast_call") return CallConvention::Fastcall;
  if (s == "vector_call") return CallConvention::Vectorcall;
  if (s == "systemv") return CallConvention::SystemV;
  return CallConvention::C;
}

ffi::Func ffi::JSON::json_to_func(const json& j)
{
  if (!(j.contains("name") || j.contains("param_names")))
    std::runtime_error("Invalid 'Function' JSON, must include a 'name' field and a 'param_names' field.");

  Func f;
  f.name            = j.value("name", "");
  f.call_convention = str_to_callconvention(j.value("call_convention", "C"));
  f.proto           = json_to_prototype(j["prototype"]);

  f.param_names.reserve(j["param_names"].size());
  for (auto& n : j["param_names"]) f.param_names.push_back(n.get<std::string>());

  return f;
}

ffi::Flag ffi::JSON::json_to_flag(const json& j)
{
  if (!(j.contains("name"))) std::runtime_error("Invalid 'Flag' JSON, must include a 'name' field.");

  Flag f;
  f.name = j.value("name", "");
  if (j.contains("underlying_type")) f.underlying_type = str_to_etype(j["underlying_type"]);

  if (j.contains("members")) {
    for (auto& n : j["members"]) f.members.emplace_back(n[0].get<std::string>(), n[1].get<size_t>());
  }
  return f;
}

ffi::Union ffi::JSON::json_to_union(const json& j)
{
  if (!(j.contains("name"))) std::runtime_error("Invalid 'Union' JSON, must include a 'name' field.");

  Union u;
  u.name = j.value("name", "");

  if (j.contains("members")) {
    for (auto& n : j["members"]) u.members.emplace_back(n[0].get<std::string>(), json_to_type(n[1]));
  }
  return u;
}

ffi::Enum ffi::JSON::json_to_enum(const json& j)
{
  if (!(j.contains("name"))) std::runtime_error("Invalid 'Enum' JSON, must include a 'name' field.");

  Enum e;
  e.name = j.value("name", "");

  if (j.contains("members")) {
    e.members.reserve(j["members"].size());

    for (auto& n : j["members"]) {
      std::string name = j[0].get<std::string>();

      std::vector<Type> types;
      types.reserve(j[1].size());

      for (auto& ty : j[1]) {
        types.push_back(json_to_type(ty));
      }

      e.members.emplace_back(name, std::move(types));
    }
  }
  return e;
}

// size, align
std::pair<size_t, size_t> ffi::JSON::EType_size_and_align(EType ty)
{
  switch (ty) {
  case EType::_i8:
  case EType::_u8:
  case EType::_b8:
  case EType::_schar:
  case EType::_uchar:      return {1, 1};
  case EType::_i16:
  case EType::_u16:
  case EType::_b16:
  case EType::_short:
  case EType::_ushort:     return {2, 2};
  case EType::_i32:
  case EType::_u32:
  case EType::_f32:
  case EType::_b32:
  case EType::_float:
  case EType::_long:
  case EType::_ulong:      return {4, 4};
  case EType::_i64:
  case EType::_u64:
  case EType::_f64:
  case EType::_b64:
  case EType::_double:
  case EType::_longlong:
  case EType::_ulonglong:  return {8, 8};
  case EType::_i128:
  case EType::_u128:
  case EType::_f128:
  case EType::_longdouble:
  case EType::_b128:       return {16, 16};
  case EType::_isize:
  case EType::_usize:
  case EType::_bsize:
  case EType::_fsize:
  case EType::_proto:
  case EType::_ptrdiff:    return {8, 8};
  case EType::_bool:       return {1, 1};
  case EType::_void:       return {1, 1};

  default:                 return {0, 0};
  }
}

// size, align
std::pair<size_t, size_t> ffi::JSON::type_size_and_align(const Type& ty)
{
  if (ty.is_pointer || ty.is_pointer_double) {
    return {8, 8}; // 64-bit ptr
  }

  size_t table_product = 1;
  if (ty.is_table) {
    if (!ty.table_size.empty()) {
      // product of all dimensions
      size_t count = 1;
      for (auto s : ty.table_size) count *= s;
    } else {
      return {8, 8}; // static table -> pointer-sized
    }
  }

  auto [size, align] = EType_size_and_align(ty.base_type);

  switch (ty.base_type) {
  case EType::_enum: {
    if (ty.enum_type) {
      for (auto& [_, m_tys] : ty.enum_type->members) {
        for (auto& m_ty : m_tys) {
          auto [m_size, m_align] = type_size_and_align(m_ty);
          size                   = m_size > size ? m_size : size;
          align                  = m_align > align ? m_align : align;
        }
      }
      break;
    }
    break;
  }
  case EType::_comp: {
    if (ty.comp_type) {
      for (auto& [_, m_ty] : ty.comp_type->fields) {
        auto [m_size, m_align] = type_size_and_align(m_ty);
        size += m_size;
        align += m_align;
      }
    }
    break;
  }
  case EType::_entity: {
    if (ty.entity_type) {
      for (auto& f_ty : ty.entity_type->components) {
        for (auto& [_, l_size, l_align] : f_ty.layouts) {
          size += l_size;
          align = l_align > align ? l_align : align;
        }
      }
    }
    break;
  }
  case EType::_union: {
    if (ty.union_type) {
      for (auto& [_, m_ty] : ty.union_type->members) {
        auto [m_size, m_align] = type_size_and_align(m_ty);
        size                   = m_size > size ? m_size : size;
        align                  = m_align > align ? m_align : align;
      }
      break;
    }
    break;
  }
  case EType::_flag: {
    if (ty.flag_type) {
      if (ty.flag_type->underlying_type != EType::INVALID) {
        auto [m_size, m_align] = EType_size_and_align(ty.flag_type->underlying_type);
        size                   = m_size;
        align                  = m_align;
      }
    }
    break;
  }
  case EType::_alias: {
  }
  default: break;
  }

  return {size * table_product, align};
}

std::vector<ffi::FieldLayout> ffi::JSON::generate_layout(const std::vector<Type>& types)
{
  std::vector<FieldLayout> layout;
  size_t                   offset       = 0;
  size_t                   struct_align = 1;

  for (const auto& ty : types) {
    auto [size, align] = type_size_and_align(ty);
    struct_align       = std::max(struct_align, align);

    // aligner l'offset
    if (offset % align != 0) {
      offset += align - (offset % align);
    }

    layout.push_back({offset, size, align});
    offset += size;
  }

  // padding final pour que la structure entière soit alignée
  if (offset % struct_align != 0) {
    offset += struct_align - (offset % struct_align);
  }

  return layout;
}

ffi::Comp ffi::JSON::json_to_comp(const json& j)
{
  if (!(j.contains("name"))) std::runtime_error("Invalid 'Component' JSON, must include a 'name' field.");

  Comp c;
  c.name = j.value("name", "");

  if (j.contains("fields")) {
    c.fields.reserve(j["fields"].size());

    std::vector<Type> tys;
    tys.reserve(j["fields"].size());

    std::vector<std::string> f_names;
    f_names.reserve(j["fields"].size());

    for (auto& n : j["fields"]) {
      f_names.push_back(j[0].get<std::string>());
      tys.push_back(json_to_type(j[1]));
    }

    c.layouts = generate_layout(tys);
    for (size_t i = 0; i < tys.size(); ++i) {
      auto& ty   = tys[i];
      auto& name = f_names[i];

      c.fields.emplace_back(name, std::move(ty));
    }
  }

  if (j.contains("layouts")) {
    if (j["layouts"].size() != j["fields"].size())
      throw std::runtime_error("explicit layout not the same size of fields");

    c.layouts.reserve(j["layouts"].size());

    for (auto& l : j["layouts"]) {
      FieldLayout layout;
      layout.offset = l[0].get<size_t>();
      layout.size   = l[1].get<size_t>();
      layout.align  = l[2].get<size_t>();
      c.layouts.push_back(layout);
    }
  }

  return c;
}

ffi::Entity ffi::JSON::json_to_entity(const json& j)
{
  if (!(j.contains("name"))) std::runtime_error("Invalid 'Entity' JSON, must include a name field.");

  Entity e;
  e.name = j.value("name", "");

  if (j.contains("components")) {
    e.components.reserve(j["components"].size());

    for (auto& n : j["components"]) {
      e.components.push_back(json_to_comp(n));
    }
  }

  return e;
}

ffi::Global ffi::JSON::json_to_global(const json& j)
{
  if (!(j.contains("name") || j.contains("type")))
    std::runtime_error("Invalid 'Global' JSON, must include a 'name' field and a 'type' field.");

  Global g;
  g.name     = j.value("name", "");
  g.type     = json_to_type(j["type"]);
  g.is_const = j.value("is_const", false);

  return g;
}

ffi::TypeAlias ffi::JSON::json_to_typealias(const json& j)
{
  TypeAlias t;
  t.name = j.value("name", "");
  t.type = json_to_type(j["type"]);

  return t;
}

ffi::AST ffi::JSON::read_ffi_json_file(const std::string& path)
{
  std::ifstream f(path);
  if (!f.is_open()) throw std::runtime_error("Cannot open JSON AST file");

  json j;
  f >> j;

  if (!j.contains("bind")) throw std::runtime_error("Invalid JSON file, expected 'bind' base field.");

  AST ast;
  ast.bind.lang = j["bind"].value("lang", "");
  ast.bind.abi  = j["bind"].value("abi", "");
  ast.bind.lib  = j["bind"].value("lib", "");

  if (j.contains("functions")) {
    for (auto& n : j["functions"]) ast.funcs.push_back(json_to_func(n));
  }

  if (j.contains("comps")) {
    for (auto& n : j["components"]) ast.comps.push_back(json_to_comp(n));
  }

  if (j.contains("globals")) {
    for (auto& n : j["globals"]) ast.globals.push_back(json_to_global(n));
  }

  if (j.contains("enums")) {
    for (auto& n : j["enums"]) ast.enums.push_back(json_to_enum(n));
  }

  if (j.contains("unions")) {
    for (auto& n : j["unions"]) ast.unions.push_back(json_to_union(n));
  }

  if (j.contains("flags")) {
    for (auto& n : j["flags"]) ast.flags.push_back(json_to_flag(n));
  }

  if (j.contains("entities")) {
    for (auto& n : j["entities"]) ast.entities.push_back(json_to_entity(n));
  }

  if (j.contains("typealias")) {
    for (auto& n : j["typealias"]) ast.typealias.push_back(json_to_typealias(n));
  }

  return ast;
}
