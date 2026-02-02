#pragma once

#include <map>
#include <unordered_map>
#include <string>
#include <tuple>

enum class EExternItem;
struct ScriptInfo;
struct PipelineScripts;

bool generate_script(
    ScriptInfo *&bind_info, 
    std::unordered_map<std::string, EExternItem> &items_to_generate, 
    const std::string &lang, 
    const std::string &lib);

bool generate_binds(
    std::unordered_map<std::string, ScriptInfo *> &bind_scrInfo, 
    std::unordered_map<std::string, std::unordered_map<std::string, EExternItem>> &items_to_generate, 
    std::unordered_map<std::string, std::tuple<std::string, std::string>> &bind_context_generated);

bool pipeline_start_EMBinder(const PipelineScripts *pipe_scripts);
