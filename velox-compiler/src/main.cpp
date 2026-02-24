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

#include <iostream>

#include "Globals.hpp"
#include "Compilation.hpp"
#include "Pipeline/Pipeline.hpp"


int velox_main(int argc, const char* argv[])
{
  parseArgs(argc, argv);

  auto comp_result = start_compilation(COMP_CTX.src_file);

  if (!comp_result) {
    std::cerr << color_RED << Config::k_comp_abort << color_RESET;
    return 1;
  }

  return 0;
}

int main(int argc, const char* argv[])
{
#ifdef VELOX_DEV_ARGS
  std::vector<std::string> args = {argv[0],
                                   "--arch=amd64",
                                   "--bits=64",
                                   "--os=linux",
                                   "--abi=LP64",
                                   "--debug",
                                   "--debug-pp",
                                   "--debug-dot",
                                   "--debug-exposer",
                                   std::string("--src=") + k_project_dir + "/source",
                                   std::string("--dest=") + k_project_dir + "/dest"};

  std::vector<const char*> cargs;
  for (auto& s : args) cargs.push_back(s.c_str());

  return velox_main((int)cargs.size(), cargs.data());
#else
  return velox_main(argc, argv);
#endif
}
