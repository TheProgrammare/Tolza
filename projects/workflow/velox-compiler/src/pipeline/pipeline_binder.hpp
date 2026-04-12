#pragma once

#include <memory>
#include <vector>

namespace ffi
{
struct Bind_Package;
}

struct Compiler;

struct ScriptInfo;

bool generate_script(const ffi::Bind_Package& p_bind);

bool generate_binds(const std::vector<ffi::Bind_Package>& p_binds);

bool pipeline_start_binder(const std::vector<std::shared_ptr<ScriptInfo>>& p_scr_infos);
