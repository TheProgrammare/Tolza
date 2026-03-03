#include "pipeline_exporter.hpp"

#include <chrono>
#include <iostream>

#include "compiler/ast/ast_base.hpp"
#include "globals.hpp"

bool pipeline_start_exporter(const std::vector<std::shared_ptr<ScriptInfo>>& scr_infos)
{
  auto start = std::chrono::high_resolution_clock::now();

  std::map<fs::path, ScriptInfo*>                                exportations;
  std::map<fs::path, std::pair<ModuleImportation*, ScriptInfo*>> importations;

  for (auto& scr_info : scr_infos) {
    size_t pos = scr_info->file_path.string().find("src/");
    if (pos == std::string::npos) {
      size_t pos = scr_info->file_path.string().find("bind/");
      return false;
    }

    for (auto& exp : scr_info->exported_mod) {
      if (exp->is_external()) continue;

      std::string relative_path   = scr_info->file_path.string().substr(pos + 5);
      exportations[relative_path] = scr_info.get();
    }

    for (auto& imp : scr_info->imported_mod) {
      importations.insert({
          imp->get_path(), {imp.get(), scr_info.get()}
      });
    }
  }

  std::cout << "[export] [info] exports: " << exportations.size() << " | imports: " << importations.size() << std::endl;

  bool   success = true;
  size_t count   = 0;
  for (auto& [name, pair] : importations) {
    auto& [imp, imp_scr] = pair;
    auto range           = exportations.equal_range(name);

    size_t mods_count = 0;
    for (auto it = range.first; it != range.second; ++it) {
      imp->target_modules.push_back(it->second);
      mods_count++;
    }

    std::string external = imp->is_external() ? "(external) " : "";

    if (mods_count > 0) {

      std::cout << color_GREEN "[Export] [" << ++count << "/" << importations.size()
                << "] [success] " color_RESET "importation of '" color_MAGENTA << name << color_RESET "' " << external
                << "resolved ! From: " color_YELLOW << mods_count << color_RESET " reference(s) to: " color_MAGENTA
                << imp_scr->file_path.filename() << color_RESET << std::endl;
    } else {
      std::cout << color_RED "[Export] [" << ++count << "/" << importations.size()
                << "] [error] importation of '" color_MAGENTA << name << color_RESET "' " << external
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