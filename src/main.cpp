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

#include <string>

#include "Compilation.hpp"
#include "Pipeline/Pipeline.hpp"


int main(int argc, char* argv[]) {
	argc += 10;
	argv[1] = const_cast<char*>("--arch=amd64");
	argv[2] = const_cast<char*>("--bits=64");
	argv[3] = const_cast<char*>("--os=linux");
	argv[4] = const_cast<char*>("--abi=LP64");
	argv[5] = const_cast<char*>("--debug");
	argv[6] = const_cast<char*>("--debug-pp");
	argv[7] = const_cast<char*>("--debug-dot");
	argv[8] = const_cast<char*>("--debug-exposer");
	std::string src_file = "--src=" PROJECT_DIR "/source"; 
	argv[9] = const_cast<char*>(src_file.c_str());
	std::string dest_file = "--dest=" PROJECT_DIR "/dest";
	argv[10] = const_cast<char*>(dest_file.c_str());

	for(int i = 0; i < argc; i++) {
        printf("Argument %d : %s\n", i, argv[i]);
    }
	
	parseArgs(argc, argv);
	start_compilation(COMP_CTX.src_file);
}