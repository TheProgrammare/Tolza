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

#include <filesystem>
#include <string>
#include <vector>

#include <clang-c/Index.h>

#include "binder/binder_ffi.hpp"

namespace ffi
{
namespace c
{

Comp   c_struct_to_comp(CXCursor cCur);
Func   c_function_to_func(CXCursor cCur);
Union  c_union_to_union(CXCursor cCur);
Flag   c_enum_to_flag(CXCursor cCur);
Global c_global_to_global(CXCursor cCur);
EType  c_type_base_to_type_base(CXType cType, CXType& out_base_cType);
Type   c_type_to_type(CXType cType);


void c_lib_to_velox_lib(const Bind_Package& _bind);

CXChildVisitResult universal_visitor(CXCursor cursor, CXCursor parent, CXClientData client_data);
AST parse_translation_unit(const Bind_Package& _bind, const fs::path& file, const std::vector<std::string>& args);

} // namespace c
} // namespace ffi