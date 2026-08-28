#include "toolchain/toolchain.hpp"

#include <common/common.hpp>
#include <common/environment.hpp>
#include <common/fileutils.hpp>
#include <common/toolchain_options.hpp>
#include <common/utils.hpp>
#include <expected>
#include <filesystem>
#include <marzer/toml++.hpp>
#include <print>
#include <string>

namespace fs = std::filesystem;

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
      std::println(stderr, "Cannot delete old symlink : {}", ec.message());
      return;
    }
  }

#if defined(_WIN32)
  if (!fs::create_directory_symlink(fs::absolute(source), stdpath, ec)) {
    std::println(stderr, "Cannot create the symlink : {}", ec.message());
  }
#elif __unix__
  fs::create_symlink(fs::absolute(source), stdpath, ec);
  if (ec) {
    std::println(stderr, "Cannot create the symlink : {}", ec.message());
  }
#endif
}

int toolchain::exec_compiler_cmd(std::string_view cmd) noexcept
{
  const fs::path compiler_path = common::toolchain::OPTIONS.compiler_used;

  if (!fs::exists(compiler_path))
    common::FATAL_ERROR(std::format("The compiler located at \"{}\" dosen't exist. Please, change the compiler used.",
                                    compiler_path.string()));

  const std::string final_cmd = std::format("{} {}", compiler_path.string(), cmd);
  return std::system(final_cmd.data());
}
