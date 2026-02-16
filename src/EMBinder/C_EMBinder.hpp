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

#include <clang-c/Index.h>

#include "Pipeline/Pipeline_EMBinder.hpp"
#include "ScriptInfo.hpp"

enum class EVeloxParamPassMode {
  NONE,
  copy,
  ref,
  mut,
  move,
  addr,
};

std::string EVeloxParamPassMode_to_str(EVeloxParamPassMode pm);

enum class EVeloxTypeFromC {
  INVALID,
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
  _size_t,
  _ptr_diff,
  _void,
  _bool,
  struct_comp,
  _union,
  enum_flag,
  func,
  alias
};

struct CVeloxFuncType;
struct CVeloxComp;
struct CVeloxUnion;
struct CVeloxFlag;

struct CVeloxType {
  CVeloxType() = default;

  EVeloxTypeFromC val_type = EVeloxTypeFromC::INVALID;

  // struct, union, enum, alias
  std::string complex_type_name;

  // val_type == EVeloxTypeFromC::func
  std::unique_ptr<CVeloxFuncType> func_type;
  // val_type == EVeloxTypeFromC::struct_comp
  std::unique_ptr<CVeloxComp> comp_type;
  // val_type == EVeloxTypeFromC::_union
  std::unique_ptr<CVeloxUnion> union_type;
  // val_type == EVeloxTypeFromC::enum_flag
  std::unique_ptr<CVeloxFlag> flag_type;

  // T[N]
  // [T; N]
  bool is_table = false;
  // *T[N]
  // [ptr'T; N]
  bool is_table_of_pointers = false;
  // [N]
  std::vector<size_t> table_size;

  // const T
  // $T
  bool is_val_type_const = false;
  // volatile T
  // !T
  bool is_val_type_volatile = false;

  // *T
  // ptr'T
  bool is_pointer = false;
  // **T
  // ptr'ptr'T
  bool is_pointer_double = false;
  // (*T)[N]
  // ptr'[T; N]
  bool is_pointer_on_table = false;
  // T const *
  // $ptr'T
  bool is_pointer_const = false;
  // T volatile *
  // !ptr'T
  bool is_pointer_volatile = false;

  // __Atomic T
  bool is_atomic = false;

  [[nodiscard]] bool is_string() const
  {
    return (val_type == EVeloxTypeFromC::_schar || val_type == EVeloxTypeFromC::_uchar) && is_pointer
           && !is_pointer_double;
  }
  [[nodiscard]] bool is_complex() const
  {
    return val_type == EVeloxTypeFromC::struct_comp || val_type == EVeloxTypeFromC::_union
           || val_type == EVeloxTypeFromC::enum_flag || val_type == EVeloxTypeFromC::func;
  }
  [[nodiscard]] bool is_opaque() const { return val_type == EVeloxTypeFromC::_void && is_pointer; }
  [[nodiscard]] bool is_flexible_table() const { return is_table && table_size.empty(); }
};

struct CVeloxFuncType {
  CVeloxType return_type;
  // type, restrict
  std::vector<std::pair<CVeloxType, bool>> params;
  bool                                     is_variadic = false;
};

struct CVeloxFunc {
  std::string              name;
  CVeloxFuncType           type;
  std::vector<std::string> param_names;
};

struct CVeloxFlag {
  std::string     name;
  EVeloxTypeFromC underlying_type;
  // string: name, size_t: bitvalue
  std::vector<std::pair<std::string, size_t>> members;
};

struct CVeloxUnion {
  std::string name;
  // key: name, val: type
  std::vector<std::pair<std::string, CVeloxType>> members;
};

struct CVeloxComp {
  std::string                                     name;
  std::vector<std::pair<std::string, CVeloxType>> fields;
};

struct CVeloxGlobal {
  std::string name;
  CVeloxType  type;
  bool        is_const = false;
};

struct CVeloxAST {
  Bind_Package              bind;
  std::vector<CVeloxComp>   comps;
  std::vector<CVeloxUnion>  unions;
  std::vector<CVeloxFlag>   enums;
  std::vector<CVeloxFunc>   funcs;
  std::vector<CVeloxGlobal> globals;
};

CVeloxComp          c_struct_to_velox_comp(CXCursor cCur);
CVeloxFunc          c_function_to_velox_function(CXCursor cCur);
CVeloxUnion         c_union_to_velox_union(CXCursor cCur);
CVeloxFlag          c_enum_to_velox_flag(CXCursor cCur);
CVeloxGlobal        c_global_to_velox_global(CXCursor cCur);
EVeloxTypeFromC     c_type_base_to_velox_type_base(CXType cType, CXType &out_base_cType);
CVeloxType          c_type_to_velox_type(CXType cType);
EVeloxParamPassMode type_to_passMode(CVeloxType &cVel);

std::string comp_to_str(CVeloxComp &cVel);
std::string func_to_str(CVeloxFunc &cVel);
std::string union_to_str(CVeloxUnion &cVel);
std::string flag_to_str(CVeloxFlag &cVel);
std::string global_to_str(CVeloxGlobal &cVel);
std::string type_to_str(CVeloxType &cVel);

class EMBinder_LibC
{
public:
  EMBinder_LibC(const Bind_Package &_bind, std::ofstream &_os) : bind(_bind), os(_os) {}

  Bind_Package bind;

  std::ofstream &os;

  [[nodiscard]] int c_lib_to_velox_lib();
};

CXChildVisitResult universal_visitor(CXCursor cursor, CXCursor parent, CXClientData client_data);
CVeloxAST          parse_translation_unit(const Bind_Package &_bind, const std::string &filename,
                                          const std::vector<std::string> &args);
