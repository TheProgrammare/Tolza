#include "Pipeline_EMBinder.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdio.h>
#include <string>
#include <vector>

#include "Compilation.hpp"
#include "Globals.hpp"

#include "EMBinder/C_EMBinder.hpp"
#include "Pipeline.hpp"
#include "ScriptInfo.hpp"

bool generate_script(const Bind_Package& bind)
{
  std::cout << color_MAGENTA << "\"" << BINDING_DIR << "/" << bind.bind_name << color_RESET " generation... "
            << std::flush;
  std::ofstream f(BINDING_DIR + "/" + bind.bind_name);

  if (!f) throw std::runtime_error("Impossible to open \"" + bind.scr_info->file_path + "\"");
  f.clear();

  std::string _lang  = bind.lang + std::string(labs(static_cast<long>(29 - bind.lang.size())), ' ');
  std::string _lib   = bind.lib + std::string(labs(static_cast<long>(29 - bind.lib.size())), ' ');
  std::string header = EMBINDER_FILE_HEADER;
  fmt_template(header, {_lang, _lib, bind.lang});
  f << header << std::flush;

  EMBinder_LibC em_binder(bind, f);
  auto          ignore = em_binder.c_lib_to_velox_lib();

  // end of export lang
  f << "\n}" << std::endl;
  f.close();
  return true;
}

bool generate_binds(const std::vector<Bind_Package>& binds)
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

  in_binding_compilation = true;
  start                  = std::chrono::high_resolution_clock::now();
  if (!start_compilation(BINDING_DIR)) {
    in_binding_compilation = false;
    return false;
  }
  auto end               = std::chrono::high_resolution_clock::now();
  in_binding_compilation = false;

  milli = std::chrono::duration<double, std::milli>(end - start).count();

  std::cout << color_YELLOW "[EMBinder] [summary] " << color_RESET << "duration: " << color_YELLOW << milli << " ms"
            << color_RESET << "\n";
  std::cout << std::endl;

  return true;
}

bool pipeline_start_EMBinder(const std::vector<std::shared_ptr<ScriptInfo>>& scr_infos)
{
  std::vector<Bind_Package> binds;
  binds.reserve(scr_infos.size());

  auto                          start = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> final_duration;
  size_t                        final_binds = 0;

  // affect all symbols imported
  // according to the imported module name
  size_t count = 1;
  for (auto& scr_info : scr_infos) {
    std::cout << "[EMBinder]";
    std::cout << color_CYAN " [" << count++ << "/" << scr_infos.size() << "] " color_RESET;
    std::cout << color_MAGENTA << scr_info->file_path << color_RESET "... " << std::flush;

    std::filesystem::create_directories(BINDING_DIR);
    size_t bind_count = 0;

    for (const auto& extern_imp : scr_info->get_externs()) {
      Bind_Package  bind;
      std::string   f_name = "EMB_" + extern_imp->name + "_" + extern_imp->extern_lib + ".vlxb";
      std::string   path   = BINDING_DIR + "/" + f_name; // same as .velox but for wrapper/headers
      std::ofstream f(path);
      f.clear();
      f.close();

      bind.bind_name = f_name;
      bind.scr_info  = scr_info;
      bind.lang      = extern_imp->name;
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
