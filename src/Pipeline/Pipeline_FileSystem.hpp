#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <optional>

struct FileSystemOut {
    std::vector<std::filesystem::path>      files_paths;
    std::vector<std::string>                files_str;
    std::vector<std::vector<std::string>>   files_lines;
};


std::optional<FileSystemOut> pipeline_start_filesystem(const std::string &target_file);
