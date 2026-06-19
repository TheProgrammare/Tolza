#include "ffi_json_reader.hpp"

#include <fstream>
#include <filesystem>
#include <memory>
#include <string_view>
#include <functional>

#include <nlohmann/json.hpp>
#include <Neargye/magic_enum.hpp>


#include <common/common.hpp>
#include <common/utils.hpp>
#include <common/compiler_options.hpp>

#include "ast/ast_base.hpp"
#include "ast/ast_declaration_extension.hpp"
#include "ast/ast_declaration_sfm.hpp"
#include "ast/ast_declaration_global.hpp"
#include "ast/ast_declaration_local.hpp"
#include "ast/ast_literal.hpp"
#include "binder/binder_ffi.hpp"
#include "nexus/ast/ast.hpp"
#include "nexus/forward.hpp"
#include "nexus/ids.hpp"
#include "nexus/type/type.hpp"

namespace fs = std::filesystem;


ffi::JSON_Reader::JSON_Reader()
  : current_ast(std::make_unique<ffi::AST>())
{
}


type::ID ffi::JSON_Reader::to_type(const json& j) noexcept
{
  const bool is_forward_id = !j.contains("kind") && j.contains("name");

  type::Qualifier dec;
  if (j.contains("qualifier")) {
    std::stringstream        ss(j.value("qualifier", ""));
    std::string              elem;
    std::vector<std::string> elems;
    elems.reserve(3);

    while (std::getline(ss, elem, ',')) {
      elems.emplace_back(elem);
    }

    for (const auto& s : elems) {
      if (s == "const")
        dec.is_constant = true;
      else if (s == "volatile")
        dec.is_volatile = true;
      else if (s == "optional")
        dec.is_optional = true;
    }
  }

  if (is_forward_id) {
    auto name = j.value("name", "");

    if (!common::utils::is_valid_identifier(name))
      common::FATAL_ERROR("Expected valid identifier ([a-zA-Z_][a-zA-Z0-9_]*) path separation possible '::'");

    return current_ast->types->factory.make_forward_identifier(name);
  }


  type::ETypeKind kind;
  kind = magic_enum::enum_cast<type::ETypeKind>(j.value("kind", "NONE")).value_or(type::ETypeKind::NONE);

  auto data = j.value("data", json::object());

  if (kind == type::ETypeKind::NONE) switch (kind) {
    case type::ETypeKind::NONE: {
      common::FATAL_ERROR("Invalid type kind specified (" + GET_ENUM_NAME(kind) + ")");
      return NO_ID;
    }
    case type::ETypeKind::Primitive: {
      expect_field(data, "primitive");
      auto txt = data.value("primitive", "NONE");
      txt      = "_" + txt;

      type::EPrimitiveTypeKind primty =
          magic_enum::enum_cast<type::EPrimitiveTypeKind>(txt).value_or(type::EPrimitiveTypeKind::NONE);
      if (primty == type::EPrimitiveTypeKind::NONE)
        common::FATAL_ERROR("Invalid primitive type specified (" + txt.substr(1) + ")");
      return current_ast->types->factory.make_primitive(primty, dec);
    }
    case type::ETypeKind::String: {
      expect_field(data, "kind");
      auto txt = data.value("kind", "NONE");
      txt      = "_" + txt;

      type::ETextType strty = magic_enum::enum_cast<type::ETextType>(txt).value_or(type::ETextType::NONE);
      if (strty == type::ETextType::NONE) common::FATAL_ERROR("Invalid textual kind specified (" + txt.substr(1) + ")");
      return current_ast->types->factory.make_string(strty, dec);
    }
    case type::ETypeKind::Tuple: {
      std::vector<type::ID> types;
      expect_field(data, "elements");
      auto elems = data.value("elements", json::array());
      types.reserve(elems.size());
      for (const auto& elem : elems) types.emplace_back(to_type(elem));

      return current_ast->types->factory.make_tuple(types, dec);
    }
    case type::ETypeKind::StaticArray: {
      expect_field(data, "inner");
      auto inner = to_type(data.value("inner", json::object()));

      expect_field(data, "size");
      size_t size = data.value("size", 0);
      if (size <= 0) common::FATAL_ERROR("Illegal size specified (" + std::to_string(size) + ") <= 0");

      return current_ast->types->factory.make_static_array(inner, size, dec);
    }
    case type::ETypeKind::Ptr: {
      expect_field(data, "inner");
      auto inner = to_type(data.value("inner", json::object()));

      return current_ast->types->factory.make_ptr(inner, dec);
    }
    case type::ETypeKind::DynamicArray: {
      expect_field(data, "inner");
      auto inner = to_type(data.value("inner", json::object()));

      return current_ast->types->factory.make_dynamic_array(inner, dec);
    }
    case type::ETypeKind::Prototype: {
      using Param = type::Prototype::Param;

      type::ID ret_type = type::TYPEID_u0;
      if (data.contains("ret_type")) ret_type = to_type(data.value("ret_type", json::object()));

      bool is_variadic = data.value("is_variadic", false);

      std::vector<Param> params;
      if (data.contains("params")) {
        auto data_params = data.value("params", json::array());
        params.reserve(data_params.size());

        for (const auto& param : data_params) {
          expect_field(param, "pass_mode");
          expect_field(param, "type");

          Param p{
              .passmode =
                  magic_enum::enum_cast<ast::EPassMode>(param.value("pass_mode", "")).value_or(ast::EPassMode::NONE),
              .type        = to_type(param.value("type", json::object())),
              .is_restrict = param.value("is_restrict", false),
          };

          params.emplace_back(p);
        }
      }

      return current_ast->types->factory.make_prototype(params, ret_type, is_variadic);
    }
    case type::ETypeKind::Identifier: {
      expect_field(data, "name");
      auto name = data.value("name", "");
      if (!common::utils::is_valid_identifier(name, true))
        common::FATAL_ERROR("Expected valid identifier ([a-zA-Z_][a-zA-Z0-9_]*) path separation possible '::'");

      return current_ast->types->factory.make_forward_identifier(name, dec);
    }
    default: {
      common::FATAL_ERROR("Invalid type kind specified (" + GET_ENUM_NAME(kind) + ")");
    }
    }

  return NO_ID;
}

void ffi::JSON_Reader::expect_field(const json& j, std::string_view field_name) noexcept
{
  if (!j.contains(field_name))
    common::FATAL_ERROR("Expected valid identifier ([a-zA-Z_][a-zA-Z0-9_]*) path separation possible '::'");
}


ast::ID ffi::JSON_Reader::to_func(const json& j, std::string_view j_name) noexcept
{
  auto& n = current_ast->add_get_node<ast::Global_Function>();
  n.name  = j_name;

  expect_field(j, "param_names");
  expect_field(j, "prototype");
  expect_field(j, "call_convention");

  if (common::utils::is_valid_identifier(n.name))
    common::FATAL_ERROR("Expected valid identifier (" + std::string(n.name) + ")");
  n.call_convention = magic_enum::enum_cast<common::compiler::ECallingConv>(j.value("call_convention", ""))
                          .value_or(common::compiler::ECallingConv::unknown);
  n.prototype = to_type(j.value("prototype", json::object()));

  auto        j_param_names = j.value("param_names", json::array());
  const auto* proto_ty      = n.prototype.as<type::Prototype>();

  if (proto_ty->params.size() != j_param_names.size())
    common::FATAL_ERROR("Unexpected prototype parameter count (" + std::to_string(proto_ty->params.size())
                        + ") not equals to function parameter names count (" + std::to_string(j_param_names.size())
                        + ")");
  for (size_t i = 0; i < j_param_names.size(); i++) {
    const auto& proto_param = proto_ty->params[i];
    const auto& param_name  = j_param_names[i];

    auto& param_n       = current_ast->add_get_node<ast::Local_Parameter>();
    param_n.name        = param_name;
    param_n.passmode    = proto_param.passmode;
    param_n.type        = proto_param.type;
    param_n.is_restrict = proto_param.is_restrict;

    n.parameters.emplace_back(param_n.nodeid);
  }

  return n.nodeid;
}

ast::ID ffi::JSON_Reader::to_flag(const json& j, std::string_view j_name) noexcept
{
  expect_field(j, "fields");

  auto& n = current_ast->add_get_node<ast::Global_Flag>();
  n.name  = j_name;

  n.underlying_type = type::ID::make(
      cu::ID::make(-1),
      static_cast<size_t>(magic_enum::enum_cast<type::EPrimitiveTypeKind>(j.value("underlying_type", "usize"))
                              .value_or(type::EPrimitiveTypeKind::_usize)));

  auto fields = j.value("fields", json::array());
  n.flags.reserve(fields.size());

  for (const auto& field : decltype(fields)::array()) {
    auto& f = current_ast->add_get_node<ast::Flag_Field>();
    f.name  = field.get<std::string>();
    if (f.name.empty()) common::FATAL_ERROR("Expected field name");

    n.flags.emplace_back(f.nodeid);
  }

  return n.nodeid;
}

ast::ID ffi::JSON_Reader::to_union(const json& j, std::string_view j_name) noexcept
{
  expect_field(j, "fields");

  auto& n = current_ast->add_get_node<ast::Global_Union>();
  n.name  = j_name;

  auto fields = j.value("fields", json::object());
  n.variants.reserve(fields.size());

  for (auto it = fields.begin(); it != fields.end(); ++it) {
    const auto& name = it.key();
    const auto& data = it.value();

    expect_field(data, "type");

    auto& f = current_ast->add_get_node<ast::Union_Field>();
    f.name  = name;
    if (f.name.empty()) common::FATAL_ERROR("Expected field name");

    f.type = to_type(data.value("type", json::object()));

    n.variants.emplace_back(f.nodeid);
  }

  return n.nodeid;
}

ast::ID ffi::JSON_Reader::to_enum(const json& j, std::string_view j_name) noexcept
{
  expect_field(j, "fields");

  auto& n = current_ast->add_get_node<ast::Global_Enum>();
  n.name  = j_name;

  auto fields = j.value("fields", json::object());
  n.variants.reserve(fields.size());

  for (auto it = fields.begin(); it != fields.end(); ++it) {
    const auto& name = it.key();
    const auto& data = it.value();

    expect_field(data, "type");

    auto& f = current_ast->add_get_node<ast::Enum_Field>();
    f.name  = name;
    if (f.name.empty()) common::FATAL_ERROR("Expected field name");

    f.type = to_type(data.value("type", json::object()));

    n.variants.emplace_back(f.nodeid);
  }

  return n.nodeid;
}

ast::ID ffi::JSON_Reader::to_facet(const json& j, std::string_view j_name) noexcept
{
  expect_field(j, "fields");

  auto& n = current_ast->add_get_node<ast::SFM_Facet>();
  n.name  = j_name;

  auto fields = j.value("fields", json::object());
  for (auto it = fields.begin(); it != fields.end(); ++it) {
    const auto& name = it.key();
    const auto& data = it.value();

    expect_field(data, "kind");
    expect_field(data, "type");

    auto& f = current_ast->add_get_node<ast::SFM_Facet_Field>();
    f.name  = name;

    f.type       = to_type(data.value("type", json::object()));
    f.capability = magic_enum::enum_cast<ast::ECapability>(data.value("kind", "NONE")).value_or(ast::ECapability::NONE);
    if (f.capability == ast::ECapability::NONE)
      common::FATAL_ERROR("Illegal capability kind specified (" + data.value("kind", "NONE")
                          + "), expected `ref`, `mut` or `var`");

    n.fields.emplace_back(f.nodeid);
  }

  return n.nodeid;
}

ast::ID ffi::JSON_Reader::to_form(const json& j, std::string_view j_name) noexcept
{
  expect_field(j, "facets");

  auto& n = current_ast->add_get_node<ast::SFM_Form>();
  n.name  = j_name;

  auto facets = j.value("facets", json::object());
  for (const auto& facet : decltype(facets)::array()) {
    const auto& f    = current_ast->add_get_node<ast::Literal_Structured_Data>();
    auto&       name = current_ast->add_get_node<ast::Identifier>();
    name.name        = facet.get<std::string>();

    n.facets.emplace_back(f.nodeid);
  }

  return n.nodeid;
}

ast::ID ffi::JSON_Reader::to_global(const json& j, std::string_view j_name) noexcept
{
  expect_field(j, "type");
  expect_field(j, "is_const");

  auto& n = current_ast->add_get_node<ast::Global_Variable>();
  n.name  = j_name;

  n.type = to_type(j.value("type", json::object()));
  n.kind = j.value("is_const", false) ? ast::EVariableKind::Let : ast::EVariableKind::Var;

  return n.nodeid;
}

ast::ID ffi::JSON_Reader::to_typealias(const json& j, std::string_view j_name) noexcept
{
  expect_field(j, "type");

  auto& n = current_ast->add_get_node<ast::Global_Alias_Type>();
  n.alias = j_name;
  n.type  = to_type(j.value("type", json::object()));

  return n.nodeid;
}

std::unique_ptr<ffi::AST> ffi::JSON_Reader::parse_json_compilation_unit(std::string_view json_path) noexcept
{
  if (!fs::exists(json_path))
    common::FATAL_ERROR("[ffi:JSON::ERROR] The file located at \"" + std::string(json_path) + "\" doesn't exists.");

  std::ifstream f(json_path.data());
  std::string   buf;
  f >> buf;

  json j(buf);

  expect_field(j, "bind");
  expect_field(j["bind"], "lang");
  expect_field(j["bind"], "abi");
  expect_field(j["bind"], "lib");
  expect_field(j["bind"], "is_barrel");

  ffi::JSON_Reader r;
  r.current_ast->bind.lang = j["bind"].value("lang", "");
  r.current_ast->bind.abi  = j["bind"].value("abi", "");
  r.current_ast->bind.lib  = j["bind"].value("lib", "");


  using TConvert = std::function<ast::ID(const json&, std::string_view s)>;

  auto generate = [&](const TConvert& converter, std::string_view category) {
    if (j.contains(category)) {
      auto elems = j.value(category, json::object());
      for (auto it = elems.begin(); it != elems.end(); ++it) {
        const auto& name = it.key();
        const auto& data = it.value();
        converter(data, name);
      }
    }
  };

  generate([&](const json& j, std::string_view j_name) { return r.to_func(j, j_name); }, "functions");
  generate([&](const json& j, std::string_view j_name) { return r.to_facet(j, j_name); }, "facets");
  generate([&](const json& j, std::string_view j_name) { return r.to_global(j, j_name); }, "globals");
  generate([&](const json& j, std::string_view j_name) { return r.to_enum(j, j_name); }, "enums");
  generate([&](const json& j, std::string_view j_name) { return r.to_union(j, j_name); }, "unions");
  generate([&](const json& j, std::string_view j_name) { return r.to_union(j, j_name); }, "flags");
  generate([&](const json& j, std::string_view j_name) { return r.to_union(j, j_name); }, "entities");
  generate([&](const json& j, std::string_view j_name) { return r.to_union(j, j_name); }, "typealiases");

  return std::move(r.current_ast);
}
