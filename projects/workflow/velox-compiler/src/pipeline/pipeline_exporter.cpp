#include "pipeline_exporter.hpp"

#include <chrono>
#include <filesystem>
#include <iostream>
#include <memory>

#include "ast/ast_base.hpp"
#include "common.hpp"
#include "compiler/compiler.hpp"
#include "compiler_context.hpp"
#include "misc/script_info.hpp"

namespace fs = std::filesystem;


bool pipeline_start_exporter(const std::vector<std::shared_ptr<ScriptInfo>>& scr_infos)
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

  std::map<fs::path, std::shared_ptr<ScriptInfo>> exportations;
  std::vector<std::pair<fs::path, std::pair<std::shared_ptr<ModuleImportation>, std::shared_ptr<ScriptInfo>>>>
      importations;

  for (auto& scr_info : scr_infos) {
    for (auto& exp : scr_info->exported_mod) {
      exportations[scr_info->get_normalized_path()] = scr_info;
    }

    for (auto& imp : scr_info->imported_mod) {
      importations.push_back({
          imp->get_normalized_path(), {imp, scr_info}
      });
    }
  }


  if (log) {
    std::cout << "[exporter] " << exportations.size() << " exports | " << importations.size() << " imports"
              << std::endl;
  }

  bool   success = true;
  size_t count   = 0;

  for (auto& [path, pair] : importations) {
    auto& [imp, imp_scr] = pair;

    if (auto it = exportations.find(path); it != exportations.end()) {
      imp->target_modules.push_back(it->second);

      if (log) {
        std::string log_txt = log_str;
        common::fmt_template(log_txt, {std::to_string(++count), std::to_string(importations.size()), "",
                                       imp->debug_name(), fs::path(imp_scr->file_path).filename()});
        std::cout << log_txt << std::endl;
      }
    } else {
      std::string log_txt = log_str;
      common::fmt_template(log_txt, {std::to_string(++count), std::to_string(importations.size()), ":ERROR",
                                     imp->debug_name(), fs::path(imp_scr->file_path).filename()});
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