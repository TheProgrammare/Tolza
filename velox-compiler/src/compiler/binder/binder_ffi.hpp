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

#include "compiler/script_info.hpp"

namespace ffi
{

struct Bind_Package {
  std::string                 bind_name;
  std::shared_ptr<ScriptInfo> scr_info;
  std::vector<Extern_Item>    items;
  std::string                 lang;
  std::string                 lib;
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

struct AST {
  Bind_Package           bind;
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

[[nodiscard]] std::string comp_to_str(const Comp& _comp);
[[nodiscard]] std::string entity_to_str(const Entity& _entity);
[[nodiscard]] std::string func_to_str(const Func& _func);
[[nodiscard]] std::string union_to_str(const Union& _union);
[[nodiscard]] std::string flag_to_str(const Flag& _flag);
[[nodiscard]] std::string enum_to_str(const Enum& _enum);
[[nodiscard]] std::string global_to_str(const Global& _glo);
[[nodiscard]] std::string typealias_to_str(const TypeAlias& _ty_alias);

void write_ast(const AST& ast, const std::string& target_path);


const char EMBINDER_FILE_HEADER[] =
    "\n"
    "// +-------------------------------------+\n"
    "// |    Velox auto generated wrappers    |\n"
    "// | Lang: %0 |\n"
    "// |  Lib: %1 |\n"
    "// |                                     |\n"
    "// |    Please do not modify the file    |\n"
    "// +-------------------------------------+\n"
    "\n"
    "\n"
    "export %2 {\n";

const char EMBINDER_ENUM_HEADER[] =
    "\n"
    "// +-----------------------+\n"
    "// |    enum definition    |\n"
    "// +-----------------------+\n"
    "\n";

const char EMBINDER_COMP_HEADER[] =
    "\n"
    "// +-----------------------+\n"
    "// |    comp definition    |\n"
    "// +-----------------------+\n"
    "\n";

const char EMBINDER_UNION_HEADER[] =
    "\n"
    "// +-----------------------+\n"
    "// |    union definition   |\n"
    "// +-----------------------+\n"
    "\n";

const char EMBINDER_FLAG_HEADER[] =
    "\n"
    "// +-----------------------+\n"
    "// |    flag definition    |\n"
    "// +-----------------------+\n"
    "\n";

const char EMBINDER_GLOBAL_HEADER[] =
    "\n"
    "// +-----------------------+\n"
    "// |   global definition   |\n"
    "// +-----------------------+\n"
    "\n";

const char EMBINDER_FUNCTION_HEADER[] =
    "\n"
    "// +-----------------------+\n"
    "// |  function definition  |\n"
    "// +-----------------------+\n"
    "\n";

const char EMBINDER_ENTITY_HEADER[] =
    "\n"
    "// +-----------------------+\n"
    "// |   entity definition   |\n"
    "// +-----------------------+\n"
    "\n";

const char EMBINDER_TYPEALIAS_HEADER[] =
    "\n"
    "// +-----------------------+\n"
    "// |    type definition    |\n"
    "// +-----------------------+\n"
    "\n";

// %0 name
// %1 params
// %2 return
const char EMBINDER_EXTERN_FN_TEMPALTE[] =
    "# extern\n"
    "fn %0(%1) -> %2;\n";

// %0 pass mode
// %1 name
// %2 type
const char EMBINDER_EXTERN_PARAM_TEMPALTE[] = "%0 %1: %2";

const char EMBINDER_EXTERN_PARAM_VARIADIC[] = "args: addr ...";

// %0 name
// %1 underlying_type
// %2 members
const char EMBINDER_EXTERN_FLAG_TEMPLATE[] =
    "# extern\n"
    "flag %0 : %1 {\n"
    "%2"
    "}\n";

// %0 name
// %1 members
const char EMBINDER_EXTERN_ENUM_TEMPLATE[] =
    "# extern\n"
    "enum %0 {\n"
    "%1"
    "}\n";

// %0 name
// %1 members
const char EMBINDER_EXTERN_UNION_TEMPLATE[] =
    "# extern\n"
    "union %0 {\n"
    "%1"
    "}\n";

// %0 name
// %1 type
const char EMBINDER_EXTERN_FIELD[] =
    "# no default\n"
    "%1: %2,\n";

// %0 name
// %1 members
const char EMBINDER_EXTERN_COMP_TEMPLATE[] =
    "# extern\n"
    "comp %0 {\n"
    "%1"
    "}\n";

// %0 name
// %1 members
const char EMBINDER_EXTERN_ENTITY_TEMPLATE[] =
    "# extern\n"
    "entity %0 {\n"
    "%1"
    "}\n";

// %0 kind
// %1 name
// %2 type
const char EMBINDER_EXTERN_GLOBAL_TEMPLATE[] =
    "# extern\n"
    "%0 %1: %2\n";

// %0 name
// %1 type
const char EMBINDER_EXTERN_TYPEALIAS_TEMPLATE[] =
    "# extern\n"
    "type %0: %1\n";

// %0 parameters
// %1 retuns
const char EMBINDER_PROTOTYPE_TEMPLATE[] = "fn(%0) -> (%1)";

} // namespace ffi