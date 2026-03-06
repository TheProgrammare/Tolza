#include "pipeline_exporter.hpp"

#include <chrono>
#include <iostream>

#include "ast/ast_base.hpp"
#include "compiler.hpp"
#include "script_info.hpp"

bool pipeline_start_exporter(const std::vector<std::shared_ptr<ScriptInfo>>& scr_infos)
{
  auto start = std::chrono::high_resolution_clock::now();

  std::map<fs::path, ScriptInfo*>                                exportations;
  std::map<fs::path, std::pair<ModuleImportation*, ScriptInfo*>> importations;

  for (auto& scr_info : scr_infos) {
    for (auto& exp : scr_info->exported_mod) {
      if (exp->is_external()) continue;
      exportations[scr_info->get_normalized_path()] = scr_info.get();
    }

    for (auto& imp : scr_info->imported_mod) {
      importations.insert({
          imp->get_normalized_path(), {imp.get(), scr_info.get()}
      });
    }
  }

  std::cout << "[export] Exports: " << exportations.size() << " | Imports: " << importations.size() << std::endl;

  bool   success = true;
  size_t count   = 0;

  for (auto& [name, pair] : importations) {
    auto& [imp, imp_scr] = pair;

    if (auto it = exportations.find(name); it != exportations.end()) {
      imp->target_modules.push_back(it->second);

      std::cout << color_GREEN "[Export:" << ++count << "/" << importations.size()
                << "] " color_RESET "importation of '" color_MAGENTA << name << color_RESET "' "
                << "resolved ! From: " color_YELLOW << imp->target_modules.size()
                << color_RESET " reference(s) to: " color_MAGENTA << imp_scr->file_path.filename() << color_RESET
                << std::endl;
    } else {
      std::cout << color_RED "[Export:" << ++count << "/" << importations.size()
                << "] [error] importation of '" color_MAGENTA << name << color_RESET "' "
                << "symbol not found ! From: " color_YELLOW "0" color_RESET " reference(s) to: " color_MAGENTA
                << imp_scr->file_path.filename() << color_RESET << std::endl;
      success = false;
    }
  }

  auto   end            = std::chrono::high_resolution_clock::now();
  auto   final_duration = end - start;
  double milli          = std::chrono::duration<double, std::milli>(final_duration).count();

  if (success)
    std::cout << color_YELLOW "[export] [summary] " color_RESET << "duration: " color_YELLOW << milli
              << " ms\n" color_RESET << std::endl;
  else
    std::cout << color_RED "[export] [failed] " color_RESET << "duration: " color_YELLOW << milli << " ms\n" color_RESET
              << std::endl;

  return success;
}