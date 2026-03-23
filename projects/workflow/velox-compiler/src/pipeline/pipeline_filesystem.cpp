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
  return ext == ".vel" || ext == ".vlx" || ext == ".velox" || ext == ".velb" || ext == ".vlxb" || ext == ".veloxb";
}

ScriptInfo::Origin scriptOrigin_from_file(const fs::path& file)
{
  if (file.filename() == fs::path(compiler::COMP_CTX.source_dir).filename()) return ScriptInfo::Origin::user;
  if (file.filename() == fs::path(compiler::COMP_CTX.binding_dir).filename()) return ScriptInfo::Origin::binding;
  if (file.filename() == fs::path(compiler::COMP_CTX.vendor_dir).filename()) return ScriptInfo::Origin::vendor_lib;
  if (file == common::get_stdlib_dir()) return ScriptInfo::Origin::stdlib;
  if (file == common::get_packages_dir()) return ScriptInfo::Origin::lib;
  return ScriptInfo::Origin::user;
}

std::vector<fs::path> find_files(const fs::path& target_path)
{
  std::vector<fs::path> filesFounds;

  if (compiler::in_binding_compilation) std::cout << "[binder] ";
  std::cout << color_CYAN "[file] search scripts at source: " color_MAGENTA << target_path << color_RESET << std::endl;

  bool error = false;

  try {
    for (auto& entry : fs::recursive_directory_iterator(target_path)) {
      if (fs::is_regular_file(entry) && hasTargetExtension(entry.path())) {
        filesFounds.push_back(entry.path());
      }
    }

    size_t count = 0;
    for (const auto& lines : filesFounds) {
      if (compiler::in_binding_compilation) std::cout << "[binder] ";
      std::cout << "[file:";
      std::cout << color_CYAN << ++count << "/" << filesFounds.size() << "] " color_RESET;
      std::cout << color_MAGENTA << lines << color_RESET << "\n" << std::flush;
    }

  } catch (const fs::filesystem_error& e) {
    if (compiler::in_binding_compilation) std::cout << "[binder] ";
    std::cout << "[file] ";
    std::cerr << color_RED << "ERR reading " << e.what() << color_RESET << "\n";
    error = true;
  }
  if (error) return {};
  return filesFounds;
}

std::vector<std::string> str_files(const std::vector<fs::path>& fPaths)
{
  std::vector<std::string> filesStr;

  size_t i = 0;
  for (auto& path : fPaths) {
    std::ifstream if_stream(path);
    if (!if_stream) {
      std::cerr << color_RED << "[file:error] " color_MAGENTA << fPaths[i] << color_RESET << std::endl;
    }
    std::ostringstream content;
    content << if_stream.rdbuf(); // read all the file content
    filesStr.push_back(content.str());

    i++;
  }

  return filesStr;
}

std::vector<std::vector<std::string>> lines_files(const std::vector<std::string>& strFiles)
{
  std::vector<std::vector<std::string>> lineFiles;
  lineFiles.reserve(strFiles.size());

  for (size_t i = 0; i < strFiles.size(); i++) {
    // size_t lineSize = std::count(strFiles[i].begin(), strFiles[i].end(), '\n') + (strFiles[i].empty() ? 0 : 1);

    std::istringstream       stream(strFiles[i]);
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
  if (filesFounds.empty()) return {};

  std::cout << color_YELLOW "[file:summary] " color_CYAN "files found: " color_YELLOW
                   + std::to_string(filesFounds.size()) + color_RESET "\n"
            << std::endl;

  // build file into string and lines (for better debug)
  std::vector<std::string>              files_str = str_files(filesFounds);
  std::vector<std::vector<std::string>> lineFiles = lines_files(files_str);

  std::vector<std::shared_ptr<ScriptInfo>> scr_infos;
  scr_infos.reserve(files_str.size());

  for (size_t i = 0; i < files_str.size(); ++i) {
    scr_infos.push_back(std::make_shared<ScriptInfo>(filesFounds[i], files_str[i], lineFiles[i]));
  }

  return scr_infos;
}