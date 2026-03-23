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
#include <initializer_list>

// log color
#define color_RESET   "\033[0m"
#define color_BLACK   "\033[30m" /* Black */
#define color_RED     "\033[31m" /* Red */
#define color_GREEN   "\033[32m" /* Green */
#define color_YELLOW  "\033[33m" /* Yellow */
#define color_MAGENTA "\033[35m" /* Magenta */
#define color_CYAN    "\033[36m" /* Cyan */
#define color_WHITE   "\033[37m" /* White */


#define k_max_path_seg_size 12
#define k_max_keyword_size  32

using ErrorCode = short;

namespace llvm
{
class LLVMContext;
}

namespace common
{
struct CompCtx;
}

namespace compiler
{

constexpr const char* VELOX_COMPILER_VERSION = "2026.2.0b";
extern bool           in_binding_compilation;


constexpr const char* k_comp_abort =
    R"([velox-compiler] Compilation aborted
[note] You must resolve all stage errors before to pass to the next stage!"
Please see above to locate all errors.
)";

enum class EPhase {
  filesystem,
  lexer,
  preprosessor,
  parser,
  binder,
  resolver_symbol,
  resolver_type,
  resolver_semantic,
  llvmir,
  linker
};

extern common::CompCtx   COMP_CTX;
extern llvm::LLVMContext LLVM_CTX;

inline bool in_binding_compilation = false;

void fmt_template(std::string& templateStr, const std::initializer_list<std::string>& args);

[[nodiscard]] std::string Phase_to_code(EPhase phase);
[[nodiscard]] std::string Phase_to_str(EPhase phase);


} // namespace compiler