#include "Pipeline_EMBinder.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <vector>

#include "Compilation.hpp"
#include "Globals.hpp"

#include "EMBinder/C_EMBinder.hpp"
#include "Pipeline.hpp"
#include "EMBinder/FFI_JSON_Reader.hpp"
#include "ScriptInfo.hpp"

bool generate_script(const FFI::Bind_Package& bind)
{
  std::cout << color_MAGENTA << "\"" << Config::get_binding_dir() << "/" << bind.bind_name
            << color_RESET " generation... " << std::flush;

  FFI::C::c_lib_to_velox_lib(bind);

  return true;
}

bool generate_binds(const std::vector<FFI::Bind_Package>& binds)
{
  auto                          start = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> final_duration;

  size_t count = 0;
  for (const auto& bind : binds) {
    std::cout << "[EMBinder]" color_CYAN " [" << ++count << "/" << binds.size() << "] " color_RESET;

    bool success = generate_script(bind);

    auto end = std::chrono::high_resolution_clock::now();

    auto milli = std::chrono::duration<double, std::milli>(end - start).count();

    if (success)
      std::cout << color_GREEN "OK " color_YELLOW << milli << " ms" << color_RESET << std::endl;
    else
      std::cout << color_RED "ERR " color_YELLOW << milli << " ms" << color_RESET << std::endl;
    final_duration += end - start;
  }

  auto milli = std::chrono::duration<double, std::milli>(final_duration).count();

  std::cout << color_YELLOW "[EMBinder] [summary] " color_RESET << "duration: " color_YELLOW << milli << " ms"
            << color_RESET << " | bind files: " color_YELLOW << binds.size() << color_RESET << "\n";
  std::cout << std::endl;

  Config::in_binding_compilation = true;
  start                          = std::chrono::high_resolution_clock::now();
  if (!start_compilation(Config::get_binding_dir())) {
    Config::in_binding_compilation = false;
    return false;
  }
  auto end                       = std::chrono::high_resolution_clock::now();
  Config::in_binding_compilation = false;

  milli = std::chrono::duration<double, std::milli>(end - start).count();

  std::cout << color_YELLOW "[EMBinder] [summary] " << color_RESET << "duration: " << color_YELLOW << milli << " ms"
            << color_RESET << "\n";
  std::cout << std::endl;

  return true;
}

void EMBinder_generate_FFI_JSON()
{
  std::filesystem::create_directories(Config::get_FFI_JSON_dir());
  std::vector<std::filesystem::path> json_files;

  try {
    for (const auto& entry : std::filesystem::directory_iterator(Config::get_FFI_JSON_dir())) {
      if (entry.is_regular_file() && entry.path().extension() == ".json") {
        json_files.push_back(entry.path());
      }
    }

    std::cout << "[EMBinder] ";
    std::cout << color_CYAN "Found JSON files:\n";
    for (const auto& path : json_files) {
      std::cout << "  - \"" color_MAGENTA << path << color_RESET "\"\n";
    }

  } catch (const std::filesystem::filesystem_error& e) {
    std::runtime_error("Filesystem error: " + std::string(e.what()) + "\n");
  }

  for (auto& json_f : json_files) {
    auto ast = FFI::JSON::json_to_ast(json_f);
  }
}

bool pipeline_start_EMBinder(const std::vector<std::shared_ptr<ScriptInfo>>& scr_infos)
{
  std::vector<FFI::Bind_Package> binds;
  binds.reserve(scr_infos.size());

  auto                          start = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> final_duration;
  size_t                        final_binds = 0;

  // affect all symbols imported according to the imported module name
  size_t count = 1;
  for (auto& scr_info : scr_infos) {
    std::cout << "[EMBinder]";
    std::cout << color_CYAN " [" << count++ << "/" << scr_infos.size() << "] " color_RESET;
    std::cout << color_MAGENTA << scr_info->file_path << color_RESET "... " << std::flush;

    std::filesystem::create_directories(Config::get_binding_dir());
    size_t bind_count = 0;

    for (const auto& extern_imp : scr_info->get_externs()) {
      FFI::Bind_Package bind;
      std::string       f_name = extern_imp->name->get_base_name() + "/" + extern_imp->extern_lib + ".vlxb";
      fs::path          path   = Config::get_binding_dir() / f_name; // same as .velox but for wrapper/headers
      std::ofstream     f(path);
      f.clear();
      f.close();

      bind.bind_name = f_name;
      bind.scr_info  = scr_info;
      bind.lang      = extern_imp->name->get_base_name();
      bind.lib       = extern_imp->extern_lib;
      bind.items     = extern_imp->extern_references;

      bind_count += bind.items.size();

      binds.push_back(bind);
    }

    auto   end   = std::chrono::high_resolution_clock::now();
    double delta = std::chrono::duration<double, std::milli>(end - start).count();
    std::cout << color_GREEN "OK " color_YELLOW << delta << " ms" color_CYAN " (" << bind_count << " binds)" color_RESET
              << std::endl;
    final_duration += end - start;
    final_binds += bind_count;
  }

  double milli = std::chrono::duration<double, std::milli>(final_duration).count();

  std::cout << color_YELLOW "[EMBinder] [summary" << color_RESET << "] duration: " << color_YELLOW << milli << " ms"
            << color_RESET << " | binds: " << color_YELLOW << final_binds << color_RESET << "\n";
  std::cout << std::endl;

  return generate_binds(binds);
}
