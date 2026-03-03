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
#include <filesystem>
#include <initializer_list>

// log color
#define color_RESET         "\033[0m"
#define color_BLACK         "\033[30m" /* Black */
#define color_RED           "\033[31m" /* Red */
#define color_GREEN         "\033[32m" /* Green */
#define color_YELLOW        "\033[33m" /* Yellow */
#define color_BLUE          "\033[34m" /* Blue */
#define color_MAGENTA       "\033[35m" /* Magenta */
#define color_CYAN          "\033[36m" /* Cyan */
#define color_WHITE         "\033[37m" /* White */
//
#define k_pointer_size      sizeof(void*)
#define k_architecture_size sizeof(void*)
#define k_max_path_seg_size 12
#define k_max_keyword_size  32

namespace fs = std::filesystem;


void fmt_template(std::string& templateStr, const std::initializer_list<std::string>& args);

[[nodiscard]] fs::path get_home_dir();
[[nodiscard]] fs::path get_stdlib_dir();
[[nodiscard]] fs::path get_user_lib_path();
[[nodiscard]] fs::path get_exe_path();
[[nodiscard]] fs::path get_exe_dir();

struct Config {
  // compilation flags
  inline static bool in_binding_compilation = false; // mutable

  // Constantes immuables
  inline static constexpr bool engage_breakpoint_on_error = true;

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


public:
  [[nodiscard]] static fs::path get_stdlib_dir();
  [[nodiscard]] static fs::path get_packages_dir();

  [[nodiscard]] static std::string Phase_to_code(EPhase phase);
  [[nodiscard]] static std::string Phase_to_str(EPhase phase);


  inline static constexpr std::string_view k_comp_abort =
      "[build] Compilation aborted\n[note] You must resolve all stage errors before to pass to the next stage!\n"
      "Please see above to locate all errors.\n\n";

  // format with title
  inline static constexpr std::string_view DIAGRAPH_VIEW_PARAM_AST_begin =
      "digraph AST {\n\n  "
      "graph [\n"
      "rankdir=LR,\n"
      "bgcolor=\"#1e1e1e\",\n"
      "nodesep=0.1,\n"
      "ranksep=0.2,\n"
      "label=\"";

  // format with title
  inline static constexpr std::string_view DIAGRAPH_VIEW_PARAM_LINKAGE_begin =
      "digraph AST {\n\n  "
      "splines=\"line\";\n  "
      "graph [\n"
      "rankdir=LR,\n"
      "bgcolor=\"#1e1e1e\",\n"
      "nodesep=1,\n"
      "ranksep=1,\n"
      "label=\"";

  inline static constexpr std::string_view DIAGRAPH_VIEW_PARAM_end =
      "\",\n"
      "labelloc=t,\n"
      "splines=spline,\n"
      "fontname=\"Cascadia Mono\",\n"
      "fontcolor=white,\n"
      "fontsize=25\n  "
      "]\n\n  "
      "node [\n"
      "style=\"rounded,filled\",\n"
      "fillcolor=\"#2d2d2d\",\n"
      "shape=rect,\n"
      "fontname=\"Cascadia Mono\",\n"
      "fontcolor=white,\n"
      "fontsize=10\n  "
      "]\n\n  "
      "edge [\n"
      "color=\"#bbbbbb\",\n"
      "fontname=\"Cascadia Mono\",\n"
      "fontcolor=white,\n"
      "fontsize=10\n  "
      "]\n";
};
