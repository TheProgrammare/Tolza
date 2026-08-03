#include "toolchain/toolchain.hpp"

#include <filesystem>
#include <fstream>
#include <string>
#include <expected>
#include <iostream>

#include <common/common.hpp>
#include <common/environment.hpp>
#include <common/utils.hpp>
#include <common/fileutils.hpp>
#include <common/toolchain_options.hpp>

#include <marzer/toml++.hpp>

namespace fs = std::filesystem;

extern const std::string common::SOFTWARE_NAME = "velox-toolchain";

void toolchain::link_stdlib()
{
  fs::path stdpath = common::env::get_stdlib_dir();
  fs::path source  = common::fileutils::resolve_path(
      (fs::path(common::env::get_exe_dir()) / ".." / ".." / ".." / "libs" / "std").string());

  fs::remove(stdpath);
  fs::create_directories(stdpath.parent_path());

  // Supprime le lien ou dossier existant si présent
  std::error_code ec;
  if (fs::exists(stdpath, ec)) {
    fs::remove(stdpath, ec);
    if (ec) {
      std::cerr << "Cannot delete old symlink : " << ec.message() << "\n";
      return;
    }
  }

#if defined(_WIN32)
  if (!fs::create_directory_symlink(fs::absolute(source), stdpath, ec)) {
    std::cerr << "Cannot create the symlink: " << ec.message() << "\n";
  }
#elif __unix__
  fs::create_symlink(fs::absolute(source), stdpath, ec);
  if (ec) {
    std::cerr << "cannot create the symlink : " << ec.message() << "\n";
  }
#endif
}

int toolchain::exec_compiler_cmd(std::string_view cmd) noexcept
{
  const fs::path compiler_path = common::toolchain::OPTIONS.compiler_used;

  if (!fs::exists(compiler_path))
    common::FATAL_ERROR("The compiler located at \"" + compiler_path.string()
                        + "\" dosen't exists. Please, change the compiler used.");

  const std::string final_cmd = std::string(compiler_path) + " " + std::string(cmd);
  return std::system(final_cmd.data());
}
