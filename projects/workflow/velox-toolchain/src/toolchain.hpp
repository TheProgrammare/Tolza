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

namespace toolchain
{

struct ToolchainCtx {
  std::string compiler_used;

  void apply_context();
};

inline ToolchainCtx TOOL_CTX;

int init_toolchain();

void log(const std::string& msg);
void err(const std::string& msg);

inline constexpr char VELOX_TOOLCHAIN_VERSION[] = "2026.2.0b";

} // namespace toolchain
