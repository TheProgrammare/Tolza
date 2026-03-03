#pragma once

#include <memory>
#include <string>
#include <vector>

#include "compiler/script_info.hpp"
#include "compiler/binder/binder_ffi.hpp"

struct PipelineScripts;

bool generate_script(const ffi::Bind_Package& bind);

bool generate_binds(const std::vector<ffi::Bind_Package>& binds);

bool pipeline_start_binder(const std::vector<std::shared_ptr<ScriptInfo>>& pipe_scripts);
