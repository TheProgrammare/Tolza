/*
 *	The Tolza programming language - Apache License, Version 2.0
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

#include "misc/error_output.hpp"
#include "nexus/forward.hpp"

#include <common/enum_lite.hpp>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// log color
#define color_RESET   "\033[0m"
#define color_BLACK   "\033[30m" /* Black */
#define color_RED     "\033[31m" /* Red */
#define color_GREEN   "\033[32m" /* Green */
#define color_YELLOW  "\033[33m" /* Yellow */
#define color_MAGENTA "\033[35m" /* Magenta */

#ifndef SOFTWARE_VERSION
#define SOFTWARE_VERSION "2026-09-03"
#endif
#ifndef TOLZA_VERSION
#define TOLZA_VERSION "2026-09"
#endif
#ifndef SOFTWARE_NAME
#define SOFTWARE_NAME "tolza-compiler"
#endif

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
struct Manifest;
}

constexpr std::string_view k_facet_abort =
    R"([tolza-compiler] Compilation aborted
  [note] You must resolve all stage errors before to pass to the next stage!"
  Please see above to locate all errors.
)";

namespace compiler
{


constexpr std::string_view SOFTWARE_ABOUT =
    "Tolza-Compiler\n"
    "  Version: " SOFTWARE_VERSION
    "\n"
    "  Tolza version: " TOLZA_VERSION
    "\n"
    "  License: Apache License, Version 2.0\n"
    "  Author: Florian Foz\n"
    "  Source: https://github.com/TheProgrammare/Tolza";


DEFINE_ENUM(EPhase, uint8_t,         //
            filesystem, 1,           //
            lexer, 2,                //
            preprosessor, 3,         //
            parser, 4,               //
            shipowner, 5,            //
            binder, 6,               //
            resolver_symbol, 7,      //
            resolver_type, 8,        //
            resolver_semantic, 9,    //
            resolver_evaluation, 10, //
            llvmir, 11,              //
            linker, 12,              //
)


struct Compiler {
  Compiler();

  unresolved::Arena& unresolved;
  resolved::Arena&   resolved;
  inference::Arena&  inference;
  semantic::Arena&   semantic_metadata;
  evaluated::Arena&  evaluated;

  bool run_requested          = false;
  long start_compilation_time = 0;

  // vector to keep the chronology
  std::vector<std::pair<cu::ID, std::vector<Error_Diagnostic>>> errors;

  [[nodiscard]] bool start_compilation();

  void add_error(const Error_Diagnostic& error);
  void print_errors() const;
};

[[nodiscard]] std::string Phase_to_code(EPhase phase);


} // namespace compiler


extern pipeline::Pipeline         PIPELINE;
extern compiler::Compiler         COMPILER;
extern common::compiler::Manifest OPTIONS;
inline llvm::TargetMachine*       TARGET_MACHINE = nullptr;
