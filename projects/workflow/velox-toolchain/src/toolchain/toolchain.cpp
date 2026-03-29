#include "toolchain/toolchain.hpp"

#include <filesystem>
#include <fstream>
#include <string>
#include <expected>
#include <iostream>

#include <common.hpp>
#include <toolchain_context.hpp>
#include <marzer/toml++.hpp>

namespace fs = std::filesystem;

extern const std::string common::SOFTWARE_NAME = "velox-toolchain";

void toolchain::init_autocompletion()
{
  // write autocompletion bash
  static const fs::path bash_dir =
      fs::path(common::get_local_data_dir()).parent_path() / "bash-completion" / "completions" / "velox.sh";
  fs::create_directories(bash_dir.parent_path());
  if (!fs::exists(bash_dir)) {
    std::ofstream bash_f(bash_dir);
    bash_f << VELOX_AUTOCOMPLETION_BASH;
    bash_f.close();
  }
}

void toolchain::link_stdlib()
{
  fs::path stdpath = common::get_stdlib_dir();
  fs::path current = fs::current_path();
  fs::path source  = fs::current_path() / "libs" / "std";

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


void toolchain::err(const std::string& msg)
{
  std::cerr << "[velox:ERROR] " << msg << std::endl;
}

void toolchain::log(const std::string& msg)
{
  std::cerr << "[velox] " << msg << std::endl;
}
