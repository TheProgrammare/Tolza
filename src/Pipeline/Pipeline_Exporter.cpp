#include "Pipeline_Exporter.hpp"

#include <iostream>
#include <chrono>

#include "Globals.hpp"

bool pipeline_start_exporter(const std::vector<std::shared_ptr<ScriptInfo>> &scr_infos) {
	auto start = std::chrono::high_resolution_clock::now();

    std::multimap<std::string, ScriptInfo*> exportations;
    std::multimap<std::string, std::pair<ModuleImportation*, ScriptInfo*>> importations;

    for (auto &scr_info : scr_infos) {
        for (auto &exp : scr_info->exported_mod) {
            exportations.insert({ exp->name, scr_info.get() });
        }
        for (auto &imp : scr_info->imported_mod) {
            importations.insert({ imp->name, { imp.get(), scr_info.get() }});
        }
    }

    std::cout << "[export] [info] exports: " << exportations.size() 
        << " | imports: " << importations.size() << std::endl;

    bool success = true;
    size_t count = 0;
    for (auto &[name, pair] : importations) {
        auto &[imp, imp_scr] = pair;
        auto range = exportations.equal_range(name);

        size_t mods_count = 0;
        for (auto it = range.first; it != range.second; ++it) {
            imp->target_modules.push_back(it->second);
            mods_count++;
        }

        std::string external = imp->is_external() ? "(external) " : "";

        if (mods_count > 0) {
                
            std::cout << color_GREEN "[Export] [" 
                << ++count << "/" << importations.size() 
                << "] [success] " color_RESET "importation of '" 
                color_MAGENTA << name << color_RESET "' " << external << "resolved ! From: " 
                color_YELLOW << mods_count << color_RESET " reference(s) to: " 
                color_MAGENTA << imp_scr->name << color_RESET << std::endl;
        }
        else {
            std::cout << color_RED "[Export] ["
                << ++count << "/" << importations.size() 
                << "] [error] importation of '" 
                color_MAGENTA << name << color_RESET "' " << external << "symbol not found ! From: " 
                color_YELLOW "0" color_RESET " reference(s) to: " 
                color_MAGENTA << imp_scr->name << color_RESET << std::endl;
            success = false;
        }
    }

	auto end = std::chrono::high_resolution_clock::now();
    auto final_duration = end - start;
    double milli = std::chrono::duration<double, std::milli>(final_duration).count();

    if (success)
        std::cout << color_YELLOW "[export] [summary] " color_RESET << "duration: " color_YELLOW << milli << " ms\n" color_RESET << std::endl;
    else
        std::cout << color_RED "[export] [failed] " color_RESET << "duration: " color_YELLOW << milli << " ms\n" color_RESET << std::endl;

    return success;
}