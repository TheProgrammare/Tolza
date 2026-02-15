#pragma once

#include <memory>
#include <string>
#include <vector>

#include "ScriptInfo.hpp"

struct PipelineScripts;

struct Bind_Package {
    std::string bind_name;
    std::shared_ptr<ScriptInfo> scr_info;
    std::vector<Extern_Item> items;
    std::string lang;
    std::string lib;
};

bool generate_script(const Bind_Package &bind);

bool generate_binds(const std::vector<Bind_Package> &binds);

bool pipeline_start_EMBinder(const std::vector<std::shared_ptr<ScriptInfo>> &pipe_scripts);
