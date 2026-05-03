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

#include <string>
#include <string_view>
#include <vector>

#include <clang-c/Index.h>

#include "binder/binder_ffi.hpp"

namespace ffi
{
namespace c
{

Comp   c_struct_to_comp(CXCursor p_cCur);
Func   c_function_to_func(CXCursor p_cCur);
Union  c_union_to_union(CXCursor p_cCur);
Flag   c_enum_to_flag(CXCursor p_cCur);
Global c_global_to_global(CXCursor p_cCur);
EType  c_type_base_to_type_base(CXType p_cType, CXType& p_out_base_cType);
Type   c_type_to_type(CXType p_cType);


void c_lib_to_velox_lib(const Bind_Package& p_bind);

CXChildVisitResult universal_visitor(CXCursor p_cursor, CXCursor p_parent, CXClientData p_client_data);
AST                parse_translation_unit(const Bind_Package& p_bind, std::string_view p_file_path,
                                          const std::vector<std::string_view>& p_args = {});

} // namespace c
} // namespace ffi