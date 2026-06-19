/*
 * =============================================================================
 * The Velox programming language (2026.1.1) - Apache License, Version 2.0
 * Copyright 2024-2026 Foz Florian
 * =============================================================================
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * =============================================================================
 */


/*
 * -----------------------------------------------------------------------------
 * This program include and use the nlohmann/json project
 * You can find this project at
 *
 *     https://json.nlohmann.me
 *
 * Used for the JSON file reading.
 * -----------------------------------------------------------------------------
 */

#pragma once

#include <memory>
#include <string_view>

#include <nlohmann/json_fwd.hpp>

#include "binder/binder_ffi.hpp"
#include "nexus/ids.hpp"


using json = nlohmann::json;

namespace ffi
{

struct JSON_Reader {
  JSON_Reader();

  [[nodiscard]] type::ID to_type(const json& j) noexcept;
  [[nodiscard]] ast::ID  to_func(const json& j, std::string_view j_name) noexcept;
  [[nodiscard]] ast::ID  to_flag(const json& j, std::string_view j_name) noexcept;
  [[nodiscard]] ast::ID  to_union(const json& j, std::string_view j_name) noexcept;
  [[nodiscard]] ast::ID  to_enum(const json& j, std::string_view j_name) noexcept;
  [[nodiscard]] ast::ID  to_facet(const json& j, std::string_view j_name) noexcept;
  [[nodiscard]] ast::ID  to_form(const json& j, std::string_view j_name) noexcept;
  [[nodiscard]] ast::ID  to_global(const json& j, std::string_view j_name) noexcept;
  [[nodiscard]] ast::ID  to_typealias(const json& j, std::string_view j_name) noexcept;

  static void expect_field(const json& j, std::string_view field_name) noexcept;

  [[nodiscard]] static std::unique_ptr<ffi::AST> parse_json_compilation_unit(std::string_view json_path) noexcept;

private:
  std::unique_ptr<ffi::AST> current_ast = nullptr;
};

} // namespace ffi