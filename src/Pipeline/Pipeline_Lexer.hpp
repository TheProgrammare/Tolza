#pragma once

#include <optional>
#include <vector>
#include <filesystem>

#include "Lexer/Token.hpp"
#include "ScriptInfo.hpp"
#include "Pipeline_FileSystem.hpp"

struct PipelineScripts;


std::optional<PipelineScripts*> pipeline_start_lexer(const FileSystemOut &fs_out);

