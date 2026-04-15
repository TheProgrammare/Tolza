#include "pipeline_binder.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <vector>

#include <compiler_context.hpp>

#include "binder/binder_ffi.hpp"
#include "compiler/compiler.hpp"

#include "binder/c_binder.hpp"
#include "binder/ffi-json_reader.hpp"
#include "misc/module_manager.hpp"
#include "misc/script_info.hpp"
#include "misc/symbol_manager.hpp"
#include "pipeline/pipeline_filesystem.hpp"

namespace fs = std::filesystem;

bool generate_script(const ffi::Bind_Package& p_bind)
{
  static bool log = compiler::COMP_CTX.logs.contains("binder");


  if (p_bind.lang == "C" || p_bind.lang == "c") {
    ffi::Bind_Package _bind_w_abi = p_bind;
    _bind_w_abi.abi               = "C";
    ffi::c::c_lib_to_velox_lib(_bind_w_abi);
  } else if (fs::exists(p_bind.get_file_path())) {
    auto ast = ffi::JSON::read_ffi_json_file(p_bind.get_file_path());
    ffi::write_ast(ast, p_bind.get_file_path());
  } else {
    std::cerr << color_RED "\n[binder] FFI JSON file doesn't exists at " << p_bind.get_file_path() << std::endl;
    return false;
  }

  return true;
}

bool generate_binds(std::vector<ffi::Bind_Package>& p_binds)
{
  static bool log = compiler::COMP_CTX.logs.contains("binder");

  std::set<std::string> scripts;

  for (const auto& bind : p_binds) {
    auto start = std::chrono::high_resolution_clock::now();

    bool success = generate_script(bind);

    scripts.insert(bind.get_file_path());

    auto end = std::chrono::high_resolution_clock::now();

    auto milli = std::chrono::duration<double, std::milli>(end - start).count();

    size_t count = 1;
    if (log)
      std::cout << "[binder:generation:" << count++ << "] \"" << bind.scr_info->file_info.path << "\"" << std::endl;

    if (!success)
      std::cout << color_RED "ERR " color_RESET "\"" << bind.scr_info->file_info.path << "\"" color_YELLOW << milli
                << " ms" << color_RESET << std::endl;
  }

  auto scrs = pipeline_start_filesystem_on_files(scripts);

  for (size_t i = 0; i < p_binds.size(); i++) {
    p_binds[i].scr_info = scrs[i];
  }

  compiler::COMP.prepare_scripts(scrs);

  return true;
}

void binder_generate_FFI_JSON()
{
  fs::create_directories(compiler::COMP_CTX.get_dir_ffi_json());
  std::vector<fs::path> json_files;

  try {
    for (const auto& entry : fs::directory_iterator(compiler::COMP_CTX.get_dir_ffi_json())) {
      if (entry.is_regular_file() && entry.path().extension() == ".json") {
        json_files.push_back(entry.path());
      }
    }

    std::cout << "[binder] ";
    std::cout << "Found JSON files:\n";
    for (const auto& path : json_files) {
      std::cout << "  - \"" color_MAGENTA << path << color_RESET "\"\n";
    }

  } catch (const fs::filesystem_error& e) {
    std::runtime_error("Filesystem error: " + std::string(e.what()) + "\n");
  }

  for (auto& json_f : json_files) {
    auto     ast  = ffi::JSON::read_ffi_json_file(json_f);
    fs::path path = fs::path(compiler::COMP_CTX.get_dir_ffi_json()) / ast.bind.lang / ast.bind.lib;
    path.replace_extension(".vlxbind");
    ffi::write_ast(ast, path);
  }
}

bool pipeline_start_binder(const std::vector<std::shared_ptr<ScriptInfo>>& p_scr_infos)
{
  static bool log = compiler::COMP_CTX.logs.contains("binder");

  std::vector<ffi::Bind_Package> binds;
  binds.reserve(p_scr_infos.size());

  auto                          start = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> final_duration;
  size_t                        final_binds = 0;

  // affect all symbols imported according to the imported module name
  size_t count = 1;
  for (auto& scr_info : p_scr_infos) {
    fs::create_directories(compiler::COMP_CTX.get_dir_binding());
    size_t bind_count = 0;

    ffi::Bind_Package bind;
    auto              path = fs::path(compiler::COMP_CTX.get_dir_binding());

    for (auto [ext_mod, sym_map] : scr_info->sym_m->unresolved_extern_symbols) {
      ffi::Bind_Package bind;
      bind.lang         = ext_mod->path[0];
      bind.lib          = ext_mod->path[1];
      bind.extern_items = sym_map;

      auto bind_path = ext_mod->get_script_path();

      // clean binding
      std::ofstream bind_file(bind_path);
      bind_file.clear();
      bind_file.close();


      binds.push_back(bind);
    }

    auto   end   = std::chrono::high_resolution_clock::now();
    double milli = std::chrono::duration<double, std::milli>(end - start).count();

    if (log) {
      static size_t count = 1;
      std::cout << "[binder:" << count++ << "] \"" << scr_info->file_info.get_file_name() << "\" | " << bind_count
                << " binds | " << milli << " ms" << std::flush;
    }
  }

  if (binds.empty()) {
    if (log) std::cout << "[binder] No binds to generate, compilation continue" << std::endl;
    return true;
  }

  return generate_binds(binds);
}
