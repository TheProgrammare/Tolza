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
#include <set>
#include <map>
#include <vector>
#include <memory>


// log color
#define color_RESET   "\033[0m"
#define color_BLACK   "\033[30m" /* Black */
#define color_RED     "\033[31m" /* Red */
#define color_GREEN   "\033[32m" /* Green */
#define color_YELLOW  "\033[33m" /* Yellow */
#define color_MAGENTA "\033[35m" /* Magenta */


#define k_max_path_seg_size 12
#define k_max_keyword_size  32

using ErrorCode = short;

struct ScriptInfo;

namespace llvm
{
class LLVMContext;
class TargetMachine;
} // namespace llvm

namespace common
{
struct CompCtx;
}

inline const char* k_comp_abort =
    R"([velox-compiler] Compilation aborted
[note] You must resolve all stage errors before to pass to the next stage!"
Please see above to locate all errors.
)";

struct Compiler {


  Compiler()
  {
  }


  std::map<std::string, std::shared_ptr<ScriptInfo>> prepared_scripts;
  std::set<std::string>                              imported_modules;

  double actual_duration = 0.0f;


  bool start_compilation();
  bool prepare_scripts(const std::vector<std::shared_ptr<ScriptInfo>>& scr_infos);
  bool analyze_scripts(const std::vector<std::shared_ptr<ScriptInfo>>& scr_infos);
};


namespace compiler
{

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

[[nodiscard]] std::string Phase_to_code(EPhase phase);
[[nodiscard]] std::string Phase_to_str(EPhase phase);

extern Compiler             COMP;
extern common::CompCtx      COMP_CTX;
extern llvm::LLVMContext    LLVM_CTX;
inline llvm::TargetMachine* TM = nullptr;

} // namespace compiler