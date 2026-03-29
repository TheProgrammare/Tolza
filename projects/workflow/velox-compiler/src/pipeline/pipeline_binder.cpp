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
#include "misc/script_info.hpp"
#include "pipeline/pipeline_filesystem.hpp"

namespace fs = std::filesystem;

bool generate_script(const ffi::Bind_Package& bind)
{
  std::cout << color_MAGENTA << bind.path << color_RESET " generation... " << std::flush;

  if (bind.lang == "C" || bind.lang == "c") {
    ffi::Bind_Package _bind_w_abi = bind;
    _bind_w_abi.abi               = "C";
    ffi::c::c_lib_to_velox_lib(_bind_w_abi);
  } else if (fs::exists(bind.path)) {
    auto ast = ffi::JSON::read_ffi_json_file(bind.path);
    ffi::write_ast(ast, bind.path);
  } else {
    std::cerr << color_RED "\n[binder] FFI JSON file doesn't exists at " << bind.path << std::endl;
    return false;
  }

  return true;
}

bool generate_binds(const std::vector<ffi::Bind_Package>& binds)
{
  std::cout << "[binder:generation] External Module Binder code Generation begins" << std::endl;

  auto                          start = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> final_duration;

  std::set<std::string> scripts;

  size_t count = 0;
  for (const auto& bind : binds) {
    std::cout << "[generation:" << ++count << "/" << binds.size() << "] ";

    bool success = generate_script(bind);

    scripts.insert(bind.path);

    auto end = std::chrono::high_resolution_clock::now();

    auto milli = std::chrono::duration<double, std::milli>(end - start).count();

    if (success)
      std::cout << color_GREEN "OK " color_YELLOW << milli << " ms" << color_RESET << std::endl;
    else
      std::cout << color_RED "ERR " color_YELLOW << milli << " ms" << color_RESET << std::endl;
    final_duration += end - start;
  }

  auto milli = std::chrono::duration<double, std::milli>(final_duration).count();

  std::cout << color_YELLOW "[generation:summary] " color_RESET << "duration: " color_YELLOW << milli << " ms"
            << color_RESET << " | bind files: " color_YELLOW << binds.size() << color_RESET << "\n";
  std::cout << std::endl;


  start     = std::chrono::high_resolution_clock::now();
  auto scrs = pipeline_start_filesystem_on_files(scripts);
  compiler::COMP.prepare_scripts(scrs);
  auto end = std::chrono::high_resolution_clock::now();

  milli = std::chrono::duration<double, std::milli>(end - start).count();

  std::cout << color_YELLOW "[generation:summary] " << color_RESET << "duration: " << color_YELLOW << milli << " ms"
            << color_RESET << "\n";
  std::cout << std::endl;

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
    path.replace_filename(".vlxbind");
    ffi::write_ast(ast, path);
  }
}

bool pipeline_start_binder(const std::vector<std::shared_ptr<ScriptInfo>>& scr_infos)
{
  std::vector<ffi::Bind_Package> binds;
  binds.reserve(scr_infos.size());

  auto                          start = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> final_duration;
  size_t                        final_binds = 0;

  // affect all symbols imported according to the imported module name
  size_t count = 1;
  for (auto& scr_info : scr_infos) {
    std::cout << "[binder:";
    std::cout << count++ << "/" << scr_infos.size() << "] " color_RESET;
    std::cout << color_MAGENTA << scr_info->file_path << color_RESET "... " << std::flush;

    fs::create_directories(compiler::COMP_CTX.get_dir_binding());
    size_t bind_count = 0;

    for (const auto& extern_imp : scr_info->get_externs()) {
      ffi::Bind_Package bind;
      fs::path path = fs::path(compiler::COMP_CTX.get_dir_binding()) / extern_imp->name / extern_imp->extern_lib;
      path.replace_extension(".vlxbind"); // same as .vlx but for wrapper/headers
      std::ofstream f(path);
      f.clear();
      f.close();

      bind.scr_info = scr_info;
      bind.lang     = extern_imp->name;
      bind.lib      = extern_imp->extern_lib;
      bind.items    = extern_imp->extern_references;
      bind.path     = path;

      bind_count += bind.items.size();

      binds.push_back(bind);
    }

    auto   end   = std::chrono::high_resolution_clock::now();
    double delta = std::chrono::duration<double, std::milli>(end - start).count();
    std::cout << color_GREEN "OK " color_YELLOW << delta << " ms (" << bind_count << " binds)" << std::endl;
    final_duration += end - start;
    final_binds += bind_count;
  }

  if (binds.empty()) {
    std::cout << "[binder] No binds to generate, compilation continue" << std::endl;
    return true;
  }

  double milli = std::chrono::duration<double, std::milli>(final_duration).count();

  std::cout << color_YELLOW "[binder:summary" << color_RESET << "] duration: " << color_YELLOW << milli << " ms"
            << color_RESET << " | binds: " << color_YELLOW << final_binds << color_RESET << "\n";
  std::cout << std::endl;

  return generate_binds(binds);
}
