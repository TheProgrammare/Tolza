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

extern const std::string PROJECT_DIR;
extern const std::string POSTPROCESS_OUT_DIR;
extern const std::string BINDING_DIR;
extern const std::string LLVM_IR_DIR;

inline const std::string EASTER_EGGS;
inline std::multimap<std::string, std::string> COMPILATION_ARGS;

struct CompCtx {
    enum class EEmitMode { LLVM, OBJ, ASM, BC, BIN };

    std::string target_abi = "LP64";
    std::string target_arch = "native";
    size_t target_arch_bits = 64;
    std::string target_os = "";
    std::string libc = "";
    bool is_debug = true;
    size_t opt_level = 0;
    bool is_size_opt = false;
    std::map<std::string, std::string> defines = {};
    std::vector<std::string> undefines = {};
    std::string output = PROJECT_DIR + "/a.out";
    bool LLVM_info_debug = true;
    EEmitMode emit_mode = EEmitMode::OBJ;
    std::string src_file = PROJECT_DIR;
    std::string dest_file = PROJECT_DIR;
};

inline CompCtx COMP_CTX;


// metacode easter eggs
inline bool bFR_KEY = false; 
const std::initializer_list<std::string> FR_KEY = { "FR" };
const std::string FR_KEY_STR = "Vive la France!";
inline bool bCATH = false; 
const std::initializer_list<std::string> CATH = { "LEX", "REX", "LUX" }; // always print 
const std::string CATH_STR = "LEX sine REX est anarchia. REX sine LUX est tyrannis. LUX sine LEX est chaos.";
inline bool bCATH_10 = false; 
const std::initializer_list<std::string> CATH_10 = { "X", "Decalogus" }; // always print the decalogus
const std::string CATH_10_STR = 
    "   I. You shall not commit without testing.\n" 
    "  II. Honor your compiler and your debugger.\n"
    " III. Thou shalt write comments for thy future self.\n"
    "  IV. Respect variable names; they reveal thy intent.\n"
    "   V. Do not repeat code; abstraction is holy.\n"
    "  VI. Handle errors with care and grace.\n"
    " VII. Keep your functions small and readable.\n"
    "VIII. Thou shalt not introduce magic numbers.\n"
    "  IX. Version control shall be thy friend.\n"
    "   X. Clean code is the path to salvation.\n";
inline bool bFR_GAULLE = false; 
const std::initializer_list<std::string> FR_GAULLE = { "Charles", "de", "Gaulle" }; // print on parser error
const std::string FR_GAULLE_STR = "La France ne peut être la France sans grandeur. -- Charles de Gaulle";
inline bool bFR_NAP = false; 
const std::initializer_list<std::string> FR_NAP = { "Napoleon", "Bonaparte" }; // print on success Impossible n'est pas Français
const std::string FR_NAP_STR = "Impossible n'est pas Français. -- Napoléon Ier";
inline bool bSUM_GIL_ENK = false; 
const std::initializer_list<std::string> SUM_GIL_ENK = { "Gilgamesh", "Enkidu" }; // print on error
const std::string SUM_GIL_ENK_STR = "Like Enkidu, the process falls, and Gilgamesh weeps.";
inline bool bCH_CONF = false; 
const std::initializer_list<std::string> CH_CONF = { "Confucius" }; // print on error
const std::string CH_CONF_STR = "The man who has two paths in his code gets lost in the bug. -- Confucius";


