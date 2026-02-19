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
#include <vector>

inline bool in_binding_compilation = false;

inline const bool   ENGAGE_BRAKPOINT_ON_ERROR  = true;
inline bool         DEBUG_MODE                 = false;
inline bool         DEBUG_POSTPROCESSOR_OUTPUT = false;
inline bool         DOT_PRINT                  = false;
inline bool         DOT_EXPOSER_PRINT          = false;
inline const size_t POINTER_SIZE               = sizeof(void*);
inline const size_t ARCHITECTURE_SIZE          = sizeof(void*);
inline const size_t MAX_PATH_SEG_SIZE          = 12;
inline const size_t MAX_KEYWORD_SIZE           = 32;

inline void fmt_template(std::string& templateStr, const std::vector<std::string>& args)
{
  for (size_t i = 0; i < args.size(); ++i) {
    std::string placeholder = "%" + std::to_string(i + 1);
    size_t      pos         = 0;
    while ((pos = templateStr.find(placeholder, pos)) != std::string::npos) {
      templateStr.replace(pos, placeholder.length(), args[i]);
      pos += args[i].length();
    }
  }
}

enum class EPhase {
  filesystem,
  lexer,
  preprosessor,
  parser,
  embinder,
  resolver_symbol,
  resolver_type,
  resolver_semantic,
  llvmir,
  linker
};

[[nodiscard]]
inline std::string Phase_to_code(EPhase phase)
{
  switch (phase) {
  case EPhase::filesystem:        return "FSYS";
  case EPhase::lexer:             return "LEXE";
  case EPhase::preprosessor:      return "PREP";
  case EPhase::parser:            return "PARS";
  case EPhase::embinder:          return "EMBI";
  case EPhase::resolver_symbol:   return "SYMB";
  case EPhase::resolver_type:     return "TYPE";
  case EPhase::resolver_semantic: return "SEMA";
  case EPhase::llvmir:            return "LLVM";
  case EPhase::linker:            return "LINK";
  }
}

[[nodiscard]]
inline std::string Phase_to_str(EPhase phase)
{
  switch (phase) {
  case EPhase::filesystem:        return "file system";
  case EPhase::lexer:             return "lexer";
  case EPhase::preprosessor:      return "preprocessor";
  case EPhase::parser:            return "parser";
  case EPhase::embinder:          return "external module binder";
  case EPhase::resolver_symbol:   return "resolver symbol";
  case EPhase::resolver_type:     return "resolver type";
  case EPhase::resolver_semantic: return "resolver semantic";
  case EPhase::llvmir:            return "LLVM IR";
  case EPhase::linker:            return "linker";
  }
}

#define color_RESET   "\033[0m"
#define color_BLACK   "\033[30m" /* Black */
#define color_RED     "\033[31m" /* Red */
#define color_GREEN   "\033[32m" /* Green */
#define color_YELLOW  "\033[33m" /* Yellow */
#define color_BLUE    "\033[34m" /* Blue */
#define color_MAGENTA "\033[35m" /* Magenta */
#define color_CYAN    "\033[36m" /* Cyan */
#define color_WHITE   "\033[37m" /* White */

static const std::string COMP_ABORT =
    "[build] Compilation aborted\n"
    "[note] You must resolve all stage errors before to pass to the next stage!\n"
    "Please see above to locate all errors.\n\n";

// format with title
const char DIAGRAPH_VIEW_PARAM_AST_begin[] =
    "digraph AST {\n"
    "\n  graph ["
    "\n    rankdir=LR,"
    "\n    bgcolor=\"#1e1e1e\","
    "\n    nodesep=0.1,"
    "\n    ranksep=0.2,"
    "\n    label=\"";

// format with title
const char DIAGRAPH_VIEW_PARAM_LINKAGE_begin[] =
    "digraph AST {\n"
    "\n  splines=\"line\";"
    "\n  graph ["
    "\n    rankdir=LR,"
    "\n    bgcolor=\"#1e1e1e\","
    "\n    nodesep=1,"
    "\n    ranksep=1,"
    "\n    label=\"";

const char DIAGRAPH_VIEW_PARAM_end[] =
    "\","
    "\n    labelloc=t,"
    "\n    splines=spline,"
    "\n    fontname=\"Cascadia Mono\","
    "\n    fontcolor=white,"
    "\n    fontsize=25"
    "\n  ]\n"
    "\n  node ["
    "\n    style=\"rounded,filled\","
    "\n    fillcolor=\"#2d2d2d\","
    "\n    shape=rect,"
    "\n    fontname=\"Cascadia Mono\","
    "\n    fontcolor=white,"
    "\n    fontsize=10"
    "\n  ]\n"
    "\n  edge ["
    "\n    color=\"#bbbbbb\","
    "\n    fontname=\"Cascadia Mono\","
    "\n    fontcolor=white,"
    "\n    fontsize=10"
    "\n  ]\n";

const char EMBINDER_FILE_HEADER[] =
    "\n"
    "// +-------------------------------------+\n"
    "// |    Velox auto generated wrappers    |\n"
    "// | Lang: %1 |\n"
    "// |  Lib: %2 |\n"
    "// |                                     |\n"
    "// |    Please do not modify the file    |\n"
    "// +-------------------------------------+\n"
    "\n"
    "\n"
    "export %3 {\n"
    "";

const char EMBINDER_ENUM_HEADER[] =
    "\n"
    "// +-----------------------+\n"
    "// |    enum definition    |\n"
    "// +-----------------------+\n"
    "\n";

const char EMBINDER_COMP_HEADER[] =
    "\n"
    "// +-----------------------+\n"
    "// |    comp definition    |\n"
    "// +-----------------------+\n"
    "\n";

const char EMBINDER_UNION_HEADER[] =
    "\n"
    "// +-----------------------+\n"
    "// |    union definition   |\n"
    "// +-----------------------+\n"
    "\n";

const char EMBINDER_GLOBAL_HEADER[] =
    "\n"
    "// +-----------------------+\n"
    "// |   global definition   |\n"
    "// +-----------------------+\n"
    "\n";

const char EMBINDER_FUNCTION_HEADER[] =
    "\n"
    "// +-----------------------+\n"
    "// |  function definition  |\n"
    "// +-----------------------+\n"
    "\n";

// %1 name
// %2 params
// %3 return
const char EMBINDER_EXTERN_FN_TEMPALTE[] =
    "# extern\n"
    "fn %1(%2) -> %3;\n";

// %1 pass mode
// %2 name
// %3 type
const char EMBINDER_EXTERN_PARAM_TEMPALTE[] = "%1 %2: %3";

const char EMBINDER_EXTERN_PARAM_VARIADIC[] = "args: addr ...";

// %1 name
// %2 underlying_type
// %3 members
const char EMBINDER_EXTERN_FLAG_TEMPLATE[] =
    "# extern\n"
    "flag %1 : %2 {\n"
    "%3"
    "}\n";

// %1 name
// %2 members
const char EMBINDER_EXTERN_UNION_TEMPLATE[] =
    "# extern\n"
    "union %1 {\n"
    "%2"
    "}\n";

// %1 kind
// %2 name
// %3 type
const char EMBINDER_EXTERN_FIELD[] =
    "# no default\n"
    "%2: %3,\n";

// %1 name
// %2 members
const char EMBINDER_EXTERN_COMP_TEMPLATE[] =
    "# extern\n"
    "comp %1 {\n"
    "%2"
    "}\n";

// %1 kind
// %2 name
// %3 type
const char EMBINDER_EXTERN_GLOBAL_TEMPLATE[] =
    "# extern\n"
    "%1 %2: %3\n";

// %1 parameters
// %2 retuns
const char EMBINDER_FN_TYPE_TEMPLATE[] = "fn(%1) -> (%2)";
