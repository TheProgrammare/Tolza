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

#include <map>
#include <string>

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

#define MAX_ERRORS 100


#define k_max_path_seg_size 12
#define k_max_keyword_size  32

using ErrorCode = short;

struct Error_Diagnostic;

namespace llvm
{
class LLVMContext;
class TargetMachine;
} // namespace llvm

namespace common
{
struct Compiler_Options;
}

inline const char* k_comp_abort =
    R"([velox-compiler] Compilation aborted
  [note] You must resolve all stage errors before to pass to the next stage!"
  Please see above to locate all errors.
)";

namespace compiler
{


enum class EPhase {
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
  Compiler(module::Graph& p_modules, ast::Arena& p_nodes, type::Arena& p_types, symbol::Arena& p_symbols,
           pipeline::Pipeline& p_pipeline, scope::Graph& p_scopes, unresolved::Arena& p_unresolved,
           resolved::Arena& p_resolved, inference::Arena& p_inference)
    : modules(p_modules)
    , nodes(p_nodes)
    , types(p_types)
    , symbols(p_symbols)
    , pipeline(p_pipeline)
    , scopes(p_scopes)
    , unresolved(p_unresolved)
    , resolved(p_resolved)
    , inference(p_inference)
  {
  }

  module::Graph&      modules;
  type::Arena&        types;
  symbol::Arena&      symbols;
  scope::Graph&       scopes;
  pipeline::Pipeline& pipeline;
  unresolved::Arena&  unresolved;
  resolved::Arena&    resolved;
  inference::Arena&   inference;
  ast::Arena&         nodes;


  std::map<script::_id, std::vector<Error_Diagnostic>> errors;

  double actual_duration = 0.0f;

  bool start_compilation();

  void add_error(Error_Diagnostic&& error);
};

[[nodiscard]] std::string Phase_to_code(EPhase phase);
[[nodiscard]] std::string Phase_to_str(EPhase phase);

extern pipeline::Pipeline pipeline;
extern module::Graph      modules;
extern ast::Arena         nodes;
extern type::Arena        types;
extern symbol::Arena      symbols;
extern scope::Graph       scopes;
extern unresolved::Arena  unresolved;
extern resolved::Arena    resolved;
extern inference::Arena   inference;

extern Compiler                 COMPILER;
extern common::Compiler_Options COMPILER_OPTIONS;
extern llvm::LLVMContext        LLVM_CTX;
inline llvm::TargetMachine*     TM = nullptr;

} // namespace compiler