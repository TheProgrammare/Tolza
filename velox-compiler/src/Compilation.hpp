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
#include <vector>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

struct CompCtx {
  enum class EEmitMode { LLVM, OBJ, ASM, BC, BIN };

  std::string                        target_abi;
  std::string                        target_arch;
  size_t                             target_arch_bits;
  std::string                        target_os;
  std::string                        libc;
  bool                               is_debug;
  size_t                             opt_level;
  bool                               is_size_opt;
  // name, value
  std::map<std::string, std::string> defines;
  std::vector<std::string>           undefines;
  fs::path                           output;
  bool                               LLVM_info_debug;
  EEmitMode                          emit_mode;
  fs::path                           src_file;
  fs::path                           dest_file;
};

extern std::map<std::string, std::string> COMPILATION_ARGS;
extern CompCtx                            COMP_CTX;
