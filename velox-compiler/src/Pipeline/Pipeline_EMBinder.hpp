#pragma once

#include <memory>
#include <string>
#include <vector>

#include "ScriptInfo.hpp"
#include "EMBinder/EMBinder_FFI.hpp"

struct PipelineScripts;


bool generate_script(const FFI::Bind_Package& bind);

bool generate_binds(const std::vector<FFI::Bind_Package>& binds);

bool pipeline_start_EMBinder(const std::vector<std::shared_ptr<ScriptInfo>>& pipe_scripts);
