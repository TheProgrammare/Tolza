/*
 * =============================================================================
 * The Tolza programming language (2026.1.1) - Apache License, Version 2.0
 * Copyright 2024-2026 Foz Florian
 * =============================================================================
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * =============================================================================
 */

#pragma once

#include "nexus/ast/forward.hpp"
#include "nexus/forward.hpp"
#include "nexus/ids.hpp"
#include "nexus/inference.hpp"

#include <cstddef>
#include <map>
#include <set>
#include <string>
#include <string_view>

namespace module
{
struct Extern_Item;
}

namespace ffi
{

struct Bind_Package {
  Bind_Package() = default;

  cu::ID CU;

  // mangle name, data
  std::map<std::string, ast::ID> extern_items;

  std::string lang;
  std::string lib;
  std::string abi;

  size_t bind_count = 0;

  [[nodiscard]] std::string get_file_path() const noexcept;
};

struct AST {
  AST();
  ~AST();

  Bind_Package bind;
  const size_t offset;
  cu::CU*      temp_cu = nullptr;

  inference::Arena* const inferences;
  std::set<std::string>   aliases_defined;

  void tolza_codegen(std::string_view dest);
};


struct FieldLayout {
  size_t offset = 0;
  size_t size   = 0;
  size_t align  = 0;
};


[[nodiscard]] bool check_ast_generation(const AST& p_ast) noexcept;


// %vc_version
// %date
// %language
// %lib
// %g_version
// %author
// %abi
constexpr std::string_view BINDER_FILE_HEADER =
    R"(/*
 * =============================================================================
 *  Tolza Compiler - Generated Binding File
 * =============================================================================
 *
 * Tolza Compiler Version  : {0}
 * Generated on            : {1}
 * 
 * Source Language         : {2}
 * Source Library          : {3}
 * Generation Version      : {4}
 * Author                  : {5}
 *
 * -----------------------------------------------------------------------------
 * WARNING: This file is auto-generated.
 * Do not edit manually.
 * -----------------------------------------------------------------------------
 */

export {{

extern "{6}" {{
)";


#define BINDER_HEADER(name)                                                                                            \
  "\n/*\n"                                                                                                             \
  " * ----------------------------------------------------------------------------\n"                                  \
                                                                                                                       \
  " *  Tolza Binding - " name                                                                                          \
  "\n"                                                                                                                 \
                                                                                                                       \
  " * ----------------------------------------------------------------------------\n"                                  \
  " */\n"


constexpr std::string_view BINDER_IMPORT_HEADER    = BINDER_HEADER("Import Block");
constexpr std::string_view BINDER_REEXPORT_HEADER  = BINDER_HEADER("Re-export Block");
constexpr std::string_view BINDER_ENUM_HEADER      = BINDER_HEADER("Enum Block");
constexpr std::string_view BINDER_FACET_HEADER     = BINDER_HEADER("Facet Block");
constexpr std::string_view BINDER_UNION_HEADER     = BINDER_HEADER("Union Block");
constexpr std::string_view BINDER_FLAG_HEADER      = BINDER_HEADER("Flag Block");
constexpr std::string_view BINDER_GLOBAL_HEADER    = BINDER_HEADER("Global Block");
constexpr std::string_view BINDER_FUNCTION_HEADER  = BINDER_HEADER("Function Block");
constexpr std::string_view BINDER_FORM_HEADER      = BINDER_HEADER("Form Block");
constexpr std::string_view BINDER_TYPEALIAS_HEADER = BINDER_HEADER("Type Block");

// {0} extern name
constexpr std::string_view BINDER_EXTERN_TEMPALTE = "extern \"{0}\" {{\n";

// {0} name
// {1} params
// {2} return
constexpr std::string_view BINDER_EXTERN_FN_TEMPALTE = "fn {0}({1}) -> {2};\n";

// {0} pass mode
// {1} name
// {2} type
constexpr std::string_view BINDER_EXTERN_PARAM_TEMPALTE = "{0} {1}: {2}";

// {0} name
// {1} underlying_type
// {2} members
constexpr std::string_view BINDER_EXTERN_FLAG_TEMPLATE =
    R"(
flag {0} : {1} {{
  {2}
}}
)";

// {0} name
// {1} members
constexpr std::string_view BINDER_EXTERN_ENUM_TEMPLATE =
    R"(
enum {0} {{
  {1}
}}
)";

// {0} name
// {1} members
constexpr std::string_view BINDER_EXTERN_UNION_TEMPLATE =
    R"(
union {0} {{
  {1}
}}
)";

// {0} name
// {1} type
constexpr std::string_view BINDER_EXTERN_FIELD =
    R"(# no default
{0}: {1},)";

// {0} path
// {1} alias
constexpr std::string_view BINDER_IMPORT_TEMPLATE   = "import {0} as {1}\n";
// {0} path
// {1} alias
constexpr std::string_view BINDER_REEXPORT_TEMPLATE = "reexport {0} as {1}\n";

// {0} name
// {1} members
constexpr std::string_view BINDER_EXTERN_FACET_TEMPLATE =
    R"(
facet {0} {{
  {1}
}}
)";

// {0} name
// {1} members
constexpr std::string_view BINDER_EXTERN_FORM_TEMPLATE =
    R"(
form {0} {{
  {1}
}}
)";

// {0} kind
// {1} name
// {2} type
constexpr std::string_view BINDER_EXTERN_GLOBAL_TEMPLATE = "{0} {1}: {2}\n";

// {0} name
// {1} type
constexpr std::string_view BINDER_EXTERN_TYPEALIAS_TEMPLATE = "type {0} = {1}\n";

// {0} name
constexpr std::string_view BINDER_EXTERN_OPAQUE_TEMPLATE = "type {0} = opaque\n";

// {0} parameters
// {1} retuns
constexpr std::string_view BINDER_PROTOTYPE_TEMPLATE = "fn({0}) -> ({1})";

// {0}  %binding.language
// {1}  %target.triple
// {2}  %target.arch
// {3}  %target.platform
// {4}  %target.env
// {5}  %libc.kind
// {6}  %libc.version
// {7}  %compiler.clang_version
// {8}  %features.gnu_source
// {9}  %features.posix_c_source
// {10} %features.file_offset_bits
// {11} %features.time_bits
// {12} %sysroot.path
// {13} %sysroot.hash
constexpr std::string_view BINDER_MANIFEST = R"(
# ==============================================================================
#  Tolza Compiler - Generated Binding Manifest
# ==============================================================================
# 
#  Tolza Compiler Version  : %vc_version
#  Generated on            : %date
#  
#  Source Language         : %language
#  Source Library          : %lib
# ------------------------------------------------------------------------------

[binding]
language = "%binding.language"

[target]
arch = "%target.arch"
platform = "%target.platform"
abi = "%target.abi"

[libc]
kind = "%libc.kind"
version = "%libc.version"

[compiler]
clang_version = "%compiler.clang_version"

[feature]
# true, false
gnu_source = %feature.gnu_source
# true, false
posix_c_source = %feature.posix_c_source
# 0..32..64
file_offset_bits = %feature.file_offset_bits
# 0..32..64
time_bits = %feature.time_bits

[sysroot]
path = "%sysroot.path"
hash = "%sysroot.hash"
)";

} // namespace ffi