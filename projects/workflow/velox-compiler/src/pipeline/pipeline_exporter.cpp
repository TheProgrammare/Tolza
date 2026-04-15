#include "pipeline_exporter.hpp"

#include <chrono>
#include <filesystem>
#include <iostream>
#include <memory>

#include "ast/ast_base.hpp"
#include "ast/ast_declaration.hpp"
#include "common.hpp"
#include "compiler/compiler.hpp"
#include "compiler_context.hpp"
#include "misc/module_manager.hpp"
#include "misc/script_info.hpp"

namespace fs = std::filesystem;


bool pipeline_start_exporter(const std::vector<std::shared_ptr<ScriptInfo>>& p_scr_infos)
{
  static bool log = compiler::COMP_CTX.logs.contains("exporter");

  // %0 current file count
  // %1 total files count
  // %2 error inscription
  // %3 importation name
  // %4 target script
  static const char* log_str = color_GREEN "[export:%0/%1%2] " color_RESET " import " color_MAGENTA
                                           "\"%3\" " color_RESET "to " color_MAGENTA "\"%4\"" color_RESET;

  auto start = std::chrono::high_resolution_clock::now();

  std::map<fs::path, std::shared_ptr<ast::declaration::Export>>       exportations;
  std::vector<std::pair<fs::path, std::shared_ptr<module::Imported>>> importations;

  for (auto& scr_info : p_scr_infos) {
    if (scr_info->mod_m->script_export) {
      exportations[scr_info->file_info.get_module_path()] = scr_info->mod_m->script_export;
    }

    for (auto& [path, imp] : scr_info->mod_m->imported_modules) {
      importations.push_back({imp->get_module_path(), imp});
    }
  }


  if (log) {
    std::cout << "[exporter] " << exportations.size() << " exports | " << importations.size() << " imports"
              << std::endl;
  }

  bool   success = true;
  size_t count   = 0;

  for (auto& [path, imp] : importations) {

    if (auto it = exportations.find(path); it != exportations.end()) {
      auto exp           = it->second;
      imp->target_script = exp->node_scr_info;

      if (log) {
        std::string log_txt = log_str;
        common::fmt_template(log_txt, {std::to_string(++count), std::to_string(importations.size()), "",
                                       imp->debug_name(), imp->target_script->file_info.get_file_name()});
        std::cout << log_txt << std::endl;
      }
    } else {
      std::string log_txt = log_str;
      common::fmt_template(log_txt, {std::to_string(++count), std::to_string(importations.size()), ":ERROR",
                                     imp->debug_name(), imp->get_name()});
      std::cerr << log_txt << std::endl;
      success = false;
    }
  }

  auto   end            = std::chrono::high_resolution_clock::now();
  auto   final_duration = end - start;
  double milli          = std::chrono::duration<double, std::milli>(final_duration).count();

  if (success) {
    if (log) {
      std::cout << color_YELLOW "[export:summary] " color_RESET << "duration: " color_YELLOW << milli
                << " ms\n" color_RESET << std::endl;
    }
  } else {
    std::cout << color_RED "[export:ERROR] Exportation failed " color_RESET << "duration: " color_YELLOW << milli
              << " ms\n" color_RESET << std::endl;
  }

  return success;
}