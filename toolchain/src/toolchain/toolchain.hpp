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

#include <string_view>

namespace toolchain
{


void              link_stdlib();
[[nodiscard]] int exec_compiler_cmd(std::string_view cmd) noexcept;

constexpr std::string_view SOFTWARE_ABOUT =
    "Tolza-Toolchain\n"
    "  Version: " SOFTWARE_VERSION
    "\n"
    "  Tolza version: " TOLZA_VERSION
    "\n"
    "  License: Apache License, Version 2.0\n"
    "  Author: Florian Foz\n"
    "  Source: https://github.com/TheProgrammare/Tolza";


constexpr std::string_view TOLZA_MAIN_TEMPLATE = R"(
import bind::C::stdio as C

fn main() {
  C::printf("hello world!")
}

)";

constexpr std::string_view TOLZA_LAUNCH_TEMPLATE = R"(
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug",
            "type": "lldb-dap",
            "request": "launch",
            "program": "${workspaceFolder}/build/debug",
            "cwd": "${workspaceFolder}",
            "preLaunchTask": "build-debug",
        }
    ]
}
)";

constexpr std::string_view TOLZA_TASKS_TEMPLATE = R"(
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "clear",
            "type": "shell",
            "command": "clear"
        },
        {
            "label": "build-debug",
            "dependsOn": "clear",
            "type": "shell",
            "command": "tolza-compiler",
            "args": [
                "build",
                "--profiles",
                "debug",
                "${workspaceFolder}/tolza.toml"
            ],
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "problemMatcher": []
        },
        {
            "label": "build-release",
            "dependsOn": "clear",
            "type": "shell",
            "command": "tolza-compiler",
            "args": [
                "build",
                "--profiles",
                "release",
                "${workspaceFolder}/tolza.toml"
            ],
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "problemMatcher": []
        }
    ]
}
)";

} // namespace toolchain
