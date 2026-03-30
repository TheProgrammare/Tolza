#include "pipeline_filesystem.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>

#include <compiler_context.hpp>
#include <common.hpp>

#include "compiler/compiler.hpp"
#include "misc/script_info.hpp"

namespace fs = std::filesystem;

namespace
{

std::vector<std::string> str_files(const std::set<std::string>& fPaths)
{
  std::vector<std::string> filesStr;
  filesStr.reserve(fPaths.size());

  for (auto& path : fPaths) {
    std::ifstream if_stream(path);
    if (!if_stream) {
      std::cerr << color_RED << "[file:error] " color_MAGENTA "\"" << path << "\"" color_RESET << std::endl;
    }
    std::ostringstream content;
    content << if_stream.rdbuf(); // read all the file content
    filesStr.push_back(content.str());
  }

  return filesStr;
}

std::vector<std::vector<std::string>> lines_files(const std::vector<std::string>& strFiles)
{
  std::vector<std::vector<std::string>> lineFiles;
  lineFiles.reserve(strFiles.size());

  for (auto& str_f : strFiles) {
    std::istringstream       stream(str_f);
    std::vector<std::string> lines;
    std::string              tempL;
    while (std::getline(stream, tempL)) {
      lines.push_back(tempL);
    }

    lineFiles.push_back(lines);
  }

  return lineFiles;
}

} // namespace

std::vector<std::shared_ptr<ScriptInfo>> pipeline_start_filesystem(const std::string& target_dir)
{
  auto filesFounds = common::filesystem::find_velox_files(target_dir, true);
  return pipeline_start_filesystem_on_files(filesFounds);
}


std::vector<std::shared_ptr<ScriptInfo>> pipeline_start_filesystem_on_files(const std::set<std::string>& target_files)
{
  static bool log = compiler::COMP_CTX.logs.contains("filesystem");

  if (target_files.empty()) return {};

  std::vector<std::string> vec_target_f(target_files.begin(), target_files.end());

  // build file into string and lines (for better debug)
  std::vector<std::string>              files_str = str_files(target_files);
  std::vector<std::vector<std::string>> lineFiles = lines_files(files_str);

  std::vector<std::shared_ptr<ScriptInfo>> scr_infos;
  scr_infos.reserve(files_str.size());

  for (size_t i = 0; i < files_str.size(); ++i) {
    scr_infos.push_back(std::make_shared<ScriptInfo>(vec_target_f[i], files_str[i], lineFiles[i]));
  }

  if (log) {
    static size_t count = 1;
    for (auto& file : target_files) {
      std::cout << "[filesystem:" << count++ << "] \"" << file << "\"" << std::flush;
    }
  }

  return scr_infos;
}