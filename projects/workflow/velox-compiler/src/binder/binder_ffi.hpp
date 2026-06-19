/*
 * =============================================================================
 * The Velox programming language (2026.1.1) - Apache License, Version 2.0
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

#include <cstddef>
#include <map>
#include <set>
#include <string>
#include <string_view>

#include "nexus/ast/forward.hpp"
#include "nexus/forward.hpp"
#include "nexus/ids.hpp"
#include "nexus/inference.hpp"
#include "nexus/type/type.hpp"

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

// to generate a velox file from any ast without any compilation unit creation
struct AST {
  AST();

  static const size_t k_type_offset;

  Bind_Package            bind;
  ast::Arena* const       nodes;
  type::Arena* const      types;
  inference::Arena* const inferences;

  std::set<std::string> aliases_defined;

  void velox_codegen(std::string_view dest) const;

  template <typename T>
  [[nodiscard]] T& add_get_node() noexcept;

  template <typename T>
  [[nodiscard]] T& add_get_type(const type::Qualifier& dec = {}) noexcept;


  [[nodiscard]] std::string import_to_str(const ast::Import& p_imp) const noexcept;
  [[nodiscard]] std::string reexport_to_str(const ast::Global_Reexport& p_imp) const noexcept;
  [[nodiscard]] std::string typealias_to_str(const ast::Global_Alias_Type& p_ty_alias) const noexcept;
  [[nodiscard]] std::string facet_to_str(const ast::SFM_Facet& p_facet) const noexcept;
  [[nodiscard]] std::string form_to_str(const ast::SFM_Form& p_form) const noexcept;
  [[nodiscard]] std::string union_to_str(const ast::Global_Union& p_union) const noexcept;
  [[nodiscard]] std::string flag_to_str(const ast::Global_Flag& p_flag) const noexcept;
  [[nodiscard]] std::string enum_to_str(const ast::Global_Enum& p_enum) const noexcept;
  [[nodiscard]] std::string func_to_str(const ast::Global_Function& p_func) const noexcept;
  [[nodiscard]] std::string global_to_str(const ast::Global_Variable& p_glo) const noexcept;

  // necessary because velox_codegen is based on existent compilation unit
  [[nodiscard]] std::string type_to_str(const type::Type& ty) const noexcept;
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
 *  Velox Compiler - Generated Binding File
 * =============================================================================
 *
 * Velox Compiler Version  : %vc_version
 * Generated on            : %date
 * 
 * Source Language         : %language
 * Source Library          : %lib
 * Generation Version      : %g_version
 * Author                  : %author
 *
 * -----------------------------------------------------------------------------
 * WARNING: This file is auto-generated.
 * Do not edit manually.
 * -----------------------------------------------------------------------------
 */

export {

extern "%abi" {
)";


constexpr std::string_view BINDER_IMPORT_HEADER = R"(
/*
 * ----------------------------------------------------------------------------
 *  Velox Binding - Import Block
 * ----------------------------------------------------------------------------
 */
)";

constexpr std::string_view BINDER_REEXPORT_HEADER = R"(
/*
 * ----------------------------------------------------------------------------
 *  Velox Binding - Re-export Block
 * ----------------------------------------------------------------------------
 */
)";

constexpr std::string_view BINDER_ENUM_HEADER = R"(
/*
 * ----------------------------------------------------------------------------
 *  Velox Binding - Enum Block
 * ----------------------------------------------------------------------------
 */
)";

constexpr std::string_view BINDER_FACET_HEADER = R"(
/*
 * ----------------------------------------------------------------------------
 *  Velox Binding - Facet Block
 * ----------------------------------------------------------------------------
 */
)";

constexpr std::string_view BINDER_UNION_HEADER = R"(
/*
 * ----------------------------------------------------------------------------
 *  Velox Binding - Union Block
 * ----------------------------------------------------------------------------
 */
)";

constexpr std::string_view BINDER_FLAG_HEADER = R"(
/*
 * ----------------------------------------------------------------------------
 *  Velox Binding - Flag Block
 * ----------------------------------------------------------------------------
 */
)";

constexpr std::string_view BINDER_GLOBAL_HEADER = R"(
/*
 * ----------------------------------------------------------------------------
 *  Velox Binding - Global Block
 * ----------------------------------------------------------------------------
 */
)";

constexpr std::string_view BINDER_FUNCTION_HEADER = R"(
/*
 * ----------------------------------------------------------------------------
 *  Velox Binding - Function Block
 * ----------------------------------------------------------------------------
 */
)";

constexpr std::string_view BINDER_FORM_HEADER = R"(
/*
 * ----------------------------------------------------------------------------
 *  Velox Binding - Form Block
 * ----------------------------------------------------------------------------
 */
)";

constexpr std::string_view BINDER_TYPEALIAS_HEADER = R"(
/*
 * ----------------------------------------------------------------------------
 *  Velox Binding - Type Block
 * ----------------------------------------------------------------------------
 */
)";

// %0 extern name
constexpr std::string_view BINDER_EXTERN_TEMPALTE = "extern \"%0\" {\n";

// %0 name
// %1 params
// %2 return
constexpr std::string_view BINDER_EXTERN_FN_TEMPALTE = "fn %0(%1) -> %2;\n";

// %0 pass mode
// %1 name
// %2 type
constexpr std::string_view BINDER_EXTERN_PARAM_TEMPALTE = "%0 %1: %2";

// %0 name
// %1 underlying_type
// %2 members
constexpr std::string_view BINDER_EXTERN_FLAG_TEMPLATE =
    R"(
flag %0 : %1 {
  %2
}
)";

// %0 name
// %1 members
constexpr std::string_view BINDER_EXTERN_ENUM_TEMPLATE =
    R"(
enum %0 {
  %1
}
)";

// %0 name
// %1 members
constexpr std::string_view BINDER_EXTERN_UNION_TEMPLATE =
    R"(
union %0 {
  %1
}
)";

// %0 name
// %1 type
constexpr std::string_view BINDER_EXTERN_FIELD =
    R"(# no default
%1: %2,)";

// %0 path
// %1 alias
constexpr std::string_view BINDER_IMPORT_TEMPLATE   = "import %0 as %1\n";
// %0 path
// %1 alias
constexpr std::string_view BINDER_REEXPORT_TEMPLATE = "reexport %0 as %1\n";

// %0 name
// %1 members
constexpr std::string_view BINDER_EXTERN_FACET_TEMPLATE =
    R"(
facet %0 {
  %1
}
)";

// %0 name
// %1 members
constexpr std::string_view BINDER_EXTERN_FORM_TEMPLATE =
    R"(
form %0 {
  %1
}
)";

// %0 kind
// %1 name
// %2 type
constexpr std::string_view BINDER_EXTERN_GLOBAL_TEMPLATE = "%0 %1: %2\n";

// %0 name
// %1 type
constexpr std::string_view BINDER_EXTERN_TYPEALIAS_TEMPLATE = "type %0 = %1\n";

// %0 name
constexpr std::string_view BINDER_EXTERN_OPAQUE_TEMPLATE = "type %0 // opaque\n";

// %0 parameters
// %1 retuns
constexpr std::string_view BINDER_PROTOTYPE_TEMPLATE = "fn(%0) -> (%1)";

// %0  binding.language
// %1  target.triple
// %2  target.arch
// %3  target.platform
// %4  target.env
// %5  libc.kind
// %6  libc.version
// %7  compiler.clang_version
// %8  features.gnu_source
// %9  features.posix_c_source
// %10 features.file_offset_bits
// %11 features.time_bits
// %12 sysroot.path
// %13 sysroot.hash
constexpr std::string_view BINDER_MANIFEST = R"(
# ==============================================================================
#  Velox Compiler - Generated Binding Manifest
# ==============================================================================
# 
#  Velox Compiler Version  : %vc_version
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