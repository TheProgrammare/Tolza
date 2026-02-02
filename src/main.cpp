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
#include "Globals.hpp"
#include "Pipeline/Pipeline.hpp"


int main(int argc, char** argv) {
	argc += 8;
	argv[1] = "--arch=amd64";
	argv[2] = "--bits=64";
	argv[3] = "--os=linux";
	argv[4] = "--abi=LP64";
	argv[5] = "--debug";
	argv[6] = "--debug-postprocessor-output";
	argv[7] = "--debug-dot";
	argv[8] = "--debug-dot-exposer";
	
	parseArgs(argc, argv);
	std::string src_dir = PROJECT_DIR + "/source";
	start_compilation(src_dir);
}