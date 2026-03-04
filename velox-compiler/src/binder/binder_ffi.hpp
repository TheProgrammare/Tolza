/*
 *	The Velox programming language - Apache License, Version 2.0
 *  Copyright 2024-2026 Foz Florian
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "script_info.hpp"

namespace ffi
{

struct Bind_Package {
  std::shared_ptr<ScriptInfo> scr_info;
  std::vector<Extern_Item>    items;
  std::string                 lang;
  std::string                 lib;
  std::string                 abi;
  fs::path                    path;
};

enum class EPassMode {
  NONE,
  copy,
  ref,
  mut,
  move,
  addr,
};

enum class EType {
  INVALID,
  _i8,
  _i16,
  _i32,
  _i64,
  _i128,
  _isize,
  _u8,
  _u16,
  _u32,
  _u64,
  _u128,
  _usize,
  _b8,
  _b16,
  _b32,
  _b64,
  _b128,
  _bsize,
  _f32,
  _f64,
  _f128,
  _fsize,
  _str,
  _text,
  _ascii,
  _utf32,
  _schar,
  _short,
  _long,
  _longlong,
  _int,
  _uchar,
  _ushort,
  _ulong,
  _ulonglong,
  _uint,
  _float,
  _double,
  _longdouble,
  _ptrdiff,
  _void,
  _bool,
  _enum,
  _comp,
  _entity,
  _union,
  _flag,
  _proto,
  _alias
};

struct Func;
struct Comp;
struct Union;
struct Flag;
struct Entity;
struct Enum;
struct Prototype;

struct Type {
  Type() = default;

  EType       base_type = ffi::EType::INVALID;
  std::string complex_type_name;

  std::unique_ptr<Prototype> proto_type;
  std::unique_ptr<Comp>      comp_type;
  std::unique_ptr<Entity>    entity_type;
  std::unique_ptr<Union>     union_type;
  std::unique_ptr<Flag>      flag_type;
  std::unique_ptr<Enum>      enum_type;

  bool                is_table             = false;
  bool                is_table_of_pointers = false;
  std::vector<size_t> table_size;

  bool is_val_type_const    = false;
  bool is_val_type_volatile = false;

  bool is_pointer          = false;
  bool is_pointer_double   = false;
  bool is_pointer_on_table = false;
  bool is_pointer_const    = false;
  bool is_pointer_volatile = false;

  bool is_atomic = false;

  [[nodiscard]] bool is_string() const
  {
    return base_type == ffi::EType::_str || base_type == ffi::EType::_text
           || ((base_type == ffi::EType::_schar || base_type == ffi::EType::_uchar) && is_pointer
               && !is_pointer_double);
  }
  [[nodiscard]] bool is_complex() const
  {
    return base_type == ffi::EType::_comp || base_type == ffi::EType::_union || base_type == ffi::EType::_flag
           || base_type == ffi::EType::_proto || base_type == ffi::EType::_entity || base_type == ffi::EType::_enum;
  }
  [[nodiscard]] bool is_opaque() const
  {
    return base_type == ffi::EType::_void && is_pointer;
  }
  [[nodiscard]] bool is_flexible_table() const
  {
    return is_table && table_size.empty();
  }
};

struct Prototype {
  Type return_type;
  bool is_variadic = false;

  // passmode, type, restrict
  std::vector<std::tuple<ffi::EPassMode, Type, bool>> params;
};

enum class CallConvention { C, Stdcall, Fastcall, Vectorcall, SystemV };

struct Func {
  std::string              name;
  std::vector<std::string> param_names;
  Prototype                proto;
  CallConvention           call_convention = CallConvention::C;
};

struct Entity {
  std::string       name;
  std::vector<Comp> components;
};

struct Flag {
  std::string name;
  EType       underlying_type;

  // string: name, size_t: bitvalue
  std::vector<std::pair<std::string, size_t>> members;
};

struct Enum {
  std::string name;

  // string: name, size_t: bitvalue
  std::vector<std::pair<std::string, std::vector<Type>>> members;
};

struct FieldLayout {
  size_t offset = 0;
  size_t size   = 1;
  size_t align  = 1;
};

struct Union {
  std::string                               name;
  // key: name, val: type
  std::vector<std::pair<std::string, Type>> members;
};

struct Comp {
  std::string                               name;
  std::vector<std::pair<std::string, Type>> fields;
  std::vector<FieldLayout>                  layouts;
};

struct Global {
  std::string name;
  Type        type;
  bool        is_const = false;
};

struct TypeAlias {
  std::string name;
  Type        type;
};

struct Import {
  enum class EImportType { lib, user, stdlib, unknown };

  std::string              name;
  std::vector<std::string> path;
  EImportType              type;
};

struct AST {
  Bind_Package           bind;
  std::vector<Import>    imports;
  std::vector<Comp>      comps;
  std::vector<Union>     unions;
  std::vector<Flag>      flags;
  std::vector<Enum>      enums;
  std::vector<Entity>    entities;
  std::vector<Func>      funcs;
  std::vector<Global>    globals;
  std::vector<TypeAlias> typealias;
};

[[nodiscard]] ffi::EPassMode type_to_passMode(const Type& ty);
[[nodiscard]] std::string    EPassMode_to_str(ffi::EPassMode pm);
[[nodiscard]] std::string    EType_to_str(EType ty);
[[nodiscard]] std::string    type_to_str(const Type& ty);

[[nodiscard]] std::string import_to_str(const Import& _imp);
[[nodiscard]] std::string comp_to_str(const Comp& _comp);
[[nodiscard]] std::string entity_to_str(const Entity& _entity);
[[nodiscard]] std::string func_to_str(const Func& _func);
[[nodiscard]] std::string union_to_str(const Union& _union);
[[nodiscard]] std::string flag_to_str(const Flag& _flag);
[[nodiscard]] std::string enum_to_str(const Enum& _enum);
[[nodiscard]] std::string global_to_str(const Global& _glo);
[[nodiscard]] std::string typealias_to_str(const TypeAlias& _ty_alias);


void write_ast(const AST& ast, const fs::path& target_path);


// %0 language
// %1 library
// %2 imports
const char BINDER_FILE_HEADER[] =
    R"(
// +-------------------------------------+
// |    Velox auto generated wrappers    |
// | Lang: %0 |
// |  Lib: %1 |
// |                                     |
// |    Please do not modify the file    |
// +-------------------------------------+

%2

export {

extern "%3" {
)";

const char BINDER_IMPORT_HEADER[] =
    R"(
// +-----------------------+
// |   import definition   |
// +-----------------------+
)";

const char BINDER_ENUM_HEADER[] =
    R"(
// +-----------------------+
// |    enum definition    |
// +-----------------------+
)";

const char BINDER_COMP_HEADER[] =
    R"(
// +-----------------------+
// |    comp definition    |
// +-----------------------+
)";

const char BINDER_UNION_HEADER[] =
    R"(
// +-----------------------+
// |    union definition   |
// +-----------------------+
)";

const char BINDER_FLAG_HEADER[] =
    R"(
// +-----------------------+
// |    flag definition    |
// +-----------------------+
)";

const char BINDER_GLOBAL_HEADER[] =
    R"(
// +-----------------------+
// |   global definition   |
// +-----------------------+
)";

const char BINDER_FUNCTION_HEADER[] =
    R"(
// +-----------------------+
// |  function definition  |
// +-----------------------+
)";

const char BINDER_ENTITY_HEADER[] =
    R"(
// +-----------------------+
// |   entity definition   |
// +-----------------------+
)";

const char BINDER_TYPEALIAS_HEADER[] =
    R"(
// +-----------------------+
// |    type definition    |
// +-----------------------+
)";

// %0 extern name
const char BINDER_EXTERN_TEMPALTE[] = "extern \"%0\" {\n";

// %0 name
// %1 params
// %2 return
const char BINDER_EXTERN_FN_TEMPALTE[] = "fn %0(%1) -> %2;\n";

// %0 pass mode
// %1 name
// %2 type
const char BINDER_EXTERN_PARAM_TEMPALTE[] = "%0 %1: %2";

// %0 name
// %1 underlying_type
// %2 members
const char BINDER_EXTERN_FLAG_TEMPLATE[] =
    R"(
flag %0 : %1 {
  %2
}
)";

// %0 name
// %1 members
const char BINDER_EXTERN_ENUM_TEMPLATE[] =
    R"(
enum %0 {
  %1
}
)";

// %0 name
// %1 members
const char BINDER_EXTERN_UNION_TEMPLATE[] =
    R"(
union %0 {
  %1
}
)";

// %0 name
// %1 type
const char BINDER_EXTERN_FIELD[] =
    R"(# no default
%1: %2,)";

// %0 type
// %1 path
const char BINDER_IMPORT_TEMPLATE[] = "import %0 %1";

// %0 name
// %1 members
const char BINDER_EXTERN_COMP_TEMPLATE[] =
    R"(
comp %0 {
  %1
}
)";

// %0 name
// %1 members
const char BINDER_EXTERN_ENTITY_TEMPLATE[] =
    R"(
entity %0 {
  %1
}
)";

// %0 kind
// %1 name
// %2 type
const char BINDER_EXTERN_GLOBAL_TEMPLATE[] = "%0 %1: %2\n";

// %0 name
// %1 type
const char BINDER_EXTERN_TYPEALIAS_TEMPLATE[] = "type %0 = %1\n";

// %0 parameters
// %1 retuns
const char BINDER_PROTOTYPE_TEMPLATE[] = "fn(%0) -> (%1)";

} // namespace ffi