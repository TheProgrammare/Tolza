#include "ffi/manager.hpp"

#include "ast/data.hpp"
#include "ast/dumper.hpp"
#include "ast/forward.hpp"
#include "ast/node/base.hpp"
#include "ast/node/declaration_global.hpp"
#include "ast/node/declaration_sfm.hpp"
#include "ast/pool.hpp"
#include "compiler/compilation_unit.hpp"
#include "compiler/compiler.hpp"
#include "compiler/io.hpp"
#include "id/cuid.hpp"
#include "pipeline/pipeline.hpp"
#include "pool/node_to_inf.hpp"
#include "type/pool.hpp"

#include <common/common.hpp>
#include <common/compiler_options.hpp>
#include <common/fileutils.hpp>
#include <common/time.hpp>
#include <cstddef>
#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

namespace fs = std::filesystem;


void ffi::AST::tolza_codegen(std::string_view dest)
{
  fs::path f = std::string(dest);
  fs::create_directories(f.parent_path());

  std::ofstream os(f, std::ios::out | std::ios::trunc);

  if (!os) common::FATAL_ERROR(std::format("Cannot open file: \"{}\"", dest));

  std::vector<ast::Global_Reexport const*>   reexports;
  std::vector<ast::Import const*>            imports;
  std::vector<ast::Global_Enum const*>       enums;
  std::vector<ast::SFM_Facet const*>         facets;
  std::vector<ast::Global_Union const*>      unions;
  std::vector<ast::Global_Variable const*>   globals;
  std::vector<ast::Global_Function const*>   funcs;
  std::vector<ast::Global_Alias_Type const*> typealiases;
  std::vector<ast::Global_Flag const*>       flags;
  std::vector<ast::SFM_Form const*>          entities;

  // distribute nodes
  size_t count = 0;
  for (const auto& elem : temp_cu->ast->entries) {
    const auto id = ast::ID::make(cu::ID::main(), count++);
    switch (elem.kind) {
    case ast::ENodeKind::Global_Reexport: reexports.emplace_back(temp_cu->ast->as<ast::Global_Reexport>(id)); break;
    case ast::ENodeKind::Import:          imports.emplace_back(temp_cu->ast->as<ast::Import>(id)); break;
    case ast::ENodeKind::Global_Enum:     enums.emplace_back(temp_cu->ast->as<ast::Global_Enum>(id)); break;
    case ast::ENodeKind::SFM_Facet:       facets.emplace_back(temp_cu->ast->as<ast::SFM_Facet>(id)); break;
    case ast::ENodeKind::Global_Union:    unions.emplace_back(temp_cu->ast->as<ast::Global_Union>(id)); break;
    case ast::ENodeKind::Global_Variable: globals.emplace_back(temp_cu->ast->as<ast::Global_Variable>(id)); break;
    case ast::ENodeKind::Global_Function: funcs.emplace_back(temp_cu->ast->as<ast::Global_Function>(id)); break;
    case ast::ENodeKind::Global_Alias_Type:
      typealiases.emplace_back(temp_cu->ast->as<ast::Global_Alias_Type>(id));
      break;
    case ast::ENodeKind::Global_Flag: flags.emplace_back(temp_cu->ast->as<ast::Global_Flag>(id)); break;
    case ast::ENodeKind::SFM_Form:    entities.emplace_back(temp_cu->ast->as<ast::SFM_Form>(id)); break;
    default:                          continue;
    }
  }

  std::string date = common::time::now_datetime();

  std::string header =
      std::format(ffi::BINDER_FILE_HEADER, TOLZA_VERSION, date, bind.lang, bind.lib, "NONE", "NONE", bind.lang);

  os << header << std::flush;

  if (!reexports.empty()) {
    os << ffi::BINDER_REEXPORT_HEADER;

    for (const auto* elem : reexports) os << dump(elem->nodeid());
  }

  if (!imports.empty()) {
    os << ffi::BINDER_IMPORT_HEADER;

    for (const auto* elem : imports) os << dump(elem->nodeid());
  }

  if (!typealiases.empty()) {
    os << ffi::BINDER_TYPEALIAS_HEADER;

    for (const auto* elem : typealiases) os << dump(elem->nodeid());
  }
  if (!enums.empty()) {
    os << ffi::BINDER_ENUM_HEADER;

    for (const auto* elem : enums) os << dump(elem->nodeid());
  }
  if (!facets.empty()) {
    os << ffi::BINDER_FACET_HEADER;

    for (const auto* elem : facets) os << dump(elem->nodeid());
  }
  if (!unions.empty()) {
    os << ffi::BINDER_UNION_HEADER;

    for (const auto* elem : unions) os << dump(elem->nodeid());
  }
  if (!globals.empty()) {
    os << ffi::BINDER_GLOBAL_HEADER;

    for (const auto* elem : globals) os << dump(elem->nodeid());
  }
  if (!funcs.empty()) {
    os << ffi::BINDER_FUNCTION_HEADER;

    for (const auto* elem : funcs) os << dump(elem->nodeid());
  }

  if (!flags.empty()) {
    os << ffi::BINDER_FLAG_HEADER;

    for (const auto* elem : flags) os << dump(elem->nodeid());
  }
  if (!entities.empty()) {
    os << ffi::BINDER_FORM_HEADER;

    for (const auto* elem : entities) os << dump(elem->nodeid());
  }

  os << std::format("\n}} // extern\n{}\n\n}} // export\n\n", bind.abi);

  os.close();
}


std::string ffi::Bind_Package::get_file_path() const noexcept
{
  fs::path path(OPTIONS.get_dir_binding_profile());
  if (!lang.empty()) path /= lang;
  if (!lib.empty()) path /= lib;
  path.replace_extension(common::fileutils::TOLZA_FILE_EXTENSION);
  return path.string();
}


bool ffi::check_ast_generation(const AST& ast) noexcept
{
  std::vector<std::string> errs;

  auto add_err = [&](const module::Extern_Item& item) {

  };

  // need change
  /*
  for (const auto& item : ast.bind.extern_items) {
    bool find = false;
    for (const auto& [name, fn] : ast.funcs) {
      if (name == item->declaration_name) find = true;
    }
    if (!find) {
      Error_Diagnostic err(*ast.bind.CU, 203, ast.bind.CU.get(), item->node_token,
                           compiler::EPhase::binder, "External reference never generated.",
                           "Check your workspace ressources, your packages, or the reference name.");
      errs.emplace_back(err.print_error());
    }
    break;
  }
    */

  for (const auto& err : errs) {
    IO::println(stderr, IO_PASS::NONE, "{}", err);
  }

  return errs.empty();
}

ffi::AST::AST()
  : offset(PIPELINE.temp_compilation_units.size())
  , inferences(new inference::Arena())
{
  auto id = cu::ID::make(offset);
  id.set_temp();
  auto* _tmp_cu = new cu::TEMP_CU(id, nullptr, new ast::Arena(id), new type::Arena(id));
  PIPELINE.temp_compilation_units.emplace_back(_tmp_cu);
}

ffi::AST::~AST()
{
  PIPELINE.temp_compilation_units.erase(PIPELINE.temp_compilation_units.begin() + (long)offset);
}
