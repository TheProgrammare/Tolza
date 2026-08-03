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

#define VELOX_COMPILER

#include <string>
#include <string_view>

#include "nexus/forward.hpp"
#include "misc/error_output.hpp"
#include "nexus/ids.hpp"

// log color
#define color_RESET   "\033[0m"
#define color_BLACK   "\033[30m" /* Black */
#define color_RED     "\033[31m" /* Red */
#define color_GREEN   "\033[32m" /* Green */
#define color_YELLOW  "\033[33m" /* Yellow */
#define color_MAGENTA "\033[35m" /* Magenta */

constexpr size_t MAX_ERRORS = 100;


constexpr size_t k_max_path_seg_size = 12;
constexpr size_t k_max_keyword_size  = 32;

using ErrorCode = short;

struct Error_Diagnostic;

namespace llvm
{
class TargetMachine;
} // namespace llvm

namespace common::compiler
{
struct Options;
}

constexpr std::string_view k_facet_abort =
    R"([velox-compiler] Compilation aborted
  [note] You must resolve all stage errors before to pass to the next stage!"
  Please see above to locate all errors.
)";

namespace compiler
{


enum class EPhase : uint8_t {
  filesystem,
  lexer,
  preprosessor,
  parser,
  shipowner,
  binder,
  resolver_symbol,
  resolver_type,
  resolver_semantic,
  llvmir,
  linker
};


struct Compiler {
  Compiler();

  bool run_requested = false;

  // vector to keep the chronology
  std::vector<std::pair<cu::ID, std::vector<Error_Diagnostic>>> errors;

  [[nodiscard]] bool start_compilation();

  void add_error(const Error_Diagnostic& error);
  void print_errors() const;
};

[[nodiscard]] std::string Phase_to_code(EPhase phase);
[[nodiscard]] std::string Phase_to_str(EPhase phase);

extern pipeline::Pipeline pipeline;

extern module::Dispatcher modules;
extern unresolved::Arena  unresolved;
extern resolved::Arena    resolved;
extern inference::Arena   inference;

extern Compiler                  COMPILER;
extern common::compiler::Options OPTIONS;
inline llvm::TargetMachine*      TM = nullptr;

} // namespace compiler