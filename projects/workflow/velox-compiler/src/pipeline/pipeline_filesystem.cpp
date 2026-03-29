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

bool hasTargetExtension(const std::string& filePath)
{
  auto ext = fs::path(filePath).extension().string();
  return ext == ".vlx" || ext == ".vlxbind" || ext == ".vlxlib";
}

ScriptInfo::Origin scriptOrigin_from_file(const fs::path& file)
{
  if (file.filename() == fs::path(compiler::COMP_CTX.get_dir_source()).filename()) return ScriptInfo::Origin::src;
  if (file.filename() == fs::path(compiler::COMP_CTX.get_dir_binding()).filename()) return ScriptInfo::Origin::binding;
  if (file.filename() == fs::path(compiler::COMP_CTX.get_dir_vendor()).filename())
    return ScriptInfo::Origin::vendor_lib;
  if (file == common::get_stdlib_dir()) return ScriptInfo::Origin::stdlib;
  if (file == common::get_packages_dir()) return ScriptInfo::Origin::pkg_lib;
  return ScriptInfo::Origin::src;
}

std::set<std::string> find_files(const std::string& target_path)
{
  std::set<std::string> filesFounds;

  std::cout << "[file] search scripts at source: " color_MAGENTA "\"" << target_path << "\"" color_RESET << std::endl;

  bool error = false;

  try {
    for (auto& entry : fs::recursive_directory_iterator(target_path)) {
      if (fs::is_regular_file(entry) && hasTargetExtension(entry.path())) {
        filesFounds.insert(entry.path().string());
      }
    }

    size_t count = 0;
    for (const auto& lines : filesFounds) {
      std::cout << "[file:";
      std::cout << ++count << "/" << filesFounds.size() << "] " color_RESET;
      std::cout << color_MAGENTA << lines << color_RESET << "\n" << std::flush;
    }

  } catch (const fs::filesystem_error& e) {
    std::cout << "[file] ";
    std::cerr << color_RED << "ERR reading " << e.what() << color_RESET << "\n";
    error = true;
  }
  if (error) return {};
  return filesFounds;
}

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

std::vector<std::shared_ptr<ScriptInfo>> pipeline_start_filesystem(const std::string& target_file)
{
  auto filesFounds = find_files(target_file);
  return pipeline_start_filesystem_on_files(filesFounds);
}


std::vector<std::shared_ptr<ScriptInfo>> pipeline_start_filesystem_on_files(const std::set<std::string>& target_files)
{
  if (target_files.empty()) return {};

  std::vector<std::string> vec_target_f(target_files.begin(), target_files.end());

  std::cout << color_YELLOW "[file:summary] files found: " color_YELLOW + std::to_string(target_files.size())
                   + color_RESET "\n"
            << std::endl;

  // build file into string and lines (for better debug)
  std::vector<std::string>              files_str = str_files(target_files);
  std::vector<std::vector<std::string>> lineFiles = lines_files(files_str);

  std::vector<std::shared_ptr<ScriptInfo>> scr_infos;
  scr_infos.reserve(files_str.size());

  for (size_t i = 0; i < files_str.size(); ++i) {
    scr_infos.push_back(std::make_shared<ScriptInfo>(vec_target_f[i], files_str[i], lineFiles[i]));
  }

  return scr_infos;
}