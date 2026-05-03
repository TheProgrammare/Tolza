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


/*
 * This program include and use the nlohmann/json project
 * You can find this project at
 *
 *     https://json.nlohmann.me
 *
 * Used for the JSON file reading.
 */

#pragma once

#include "binder_ffi.hpp"

#include <nlohmann/json_fwd.hpp>

using json = nlohmann::json;

namespace ffi
{
namespace JSON
{

AST read_ffi_json_file(std::string_view path);

Type      json_to_type(const json& j);
Prototype json_to_prototype(const json& j);
Func      json_to_func(const json& j);
Flag      json_to_flag(const json& j);
Union     json_to_union(const json& j);
Enum      json_to_enum(const json& j);
Comp      json_to_comp(const json& j);
Entity    json_to_entity(const json& j);
Global    json_to_global(const json& j);
TypeAlias json_to_typealias(const json& j);

// Helpers pour convertir string → enum
EType                     str_to_EType(std::string_view s);
EPassMode                 str_to_EPassMode(std::string_view s);
ECallConvention           str_to_ECallConvention(std::string_view s);
// size, align
std::pair<size_t, size_t> EType_size_and_align(EType ty);
// size, align
std::pair<size_t, size_t> type_size_and_align(const Type& ty);
std::vector<FieldLayout>  generate_layout(const std::vector<Type>& p_types);

} // namespace JSON
} // namespace ffi