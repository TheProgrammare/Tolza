#include "fileutils.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <print>
#include <format>

#include "common.hpp"
#include "time.hpp"
#include "utils.hpp"

namespace fs = std::filesystem;


bool common::fileutils::is_sub_path(std::string_view base, std::string_view path) noexcept
{
  auto f_base = fs::weakly_canonical(base);
  auto f_path = fs::weakly_canonical(path);
  try {
    auto rel = fs::relative(f_path, f_base);
    return !rel.empty() && rel.native().find("..") == std::string::npos;
  } catch (...) {
    return false;
  }
}


std::string common::fileutils::resolve_path(std::string_view current, std::string_view relative) noexcept
{
  if (current.empty()) return std::string(relative);

  std::string path_str(current);

  // Expand tilde
  if (current[0] == '~') {
    const char* home =
#if __unix__
        std::getenv("HOME");
#elif _WIN32
        std::getenv("USERPROFILE");
#else
        home = nullptr
#endif

    if (!home) FATAL_ERROR("The home environment is not defined");

    path_str = std::string(home) + std::string(current.substr(1));
  }

  fs::path p(path_str);

  // Résoudre par rapport à relative si nécessaire
  fs::path abs_path = p.is_absolute() ? p : fs::path(relative) / p;

  // Canonicalize si possible, sinon normalisation lexicale
  try {
    return fs::canonical(abs_path).string();
  } catch (...) {
    return abs_path.lexically_normal().string();
  }
}


bool common::fileutils::is_tolza_file(std::string_view file_path) noexcept
{
  fs::path f(file_path);
  return f.extension() == TOLZA_FILE_EXTENSION;
}

std::string common::fileutils::get_tolza_file(std::string_view path) noexcept
{
  auto p = fs::weakly_canonical(path);

  if (p.has_extension() && p.extension() != TOLZA_FILE_EXTENSION) return "";
  if (!p.has_extension()) {
    p.replace_extension(TOLZA_FILE_EXTENSION);
  }

  return fs::exists(p) ? p : "";
}

std::set<std::string> common::fileutils::find_tolza_files(std::string_view target_dir, bool is_recursive) noexcept
{
  std::set<std::string> out;
  fs::path              dir(target_dir);

  assert(fs::exists(target_dir) && "Target directory dosen't exist");

  try {
    if (is_recursive) {
      for (const auto& entry : fs::recursive_directory_iterator(dir)) {
        if (fs::is_regular_file(entry) && entry.path().extension() == TOLZA_FILE_EXTENSION)
          out.insert(entry.path().string());
      }
    } else {
      for (const auto& entry : fs::directory_iterator(dir)) {
        if (fs::is_regular_file(entry) && entry.path().extension() == TOLZA_FILE_EXTENSION)
          out.insert(entry.path().string());
      }
    }
  } catch (const std::runtime_error& err) {
    std::println("{}", err.what());
    return {};
  }

  return out;
}

std::string common::fileutils::find_tolza_toml(std::string_view file_path) noexcept
{
  std::error_code ec;
  fs::path        p(resolve_path(file_path));

  if (p.empty()) return {};

  if (!fs::is_directory(p, ec)) {
    if (ec) return {};
    p = p.parent_path();
  }

  if (p.empty()) return {};

  for (const auto& entry : fs::directory_iterator(p, ec)) {
    if (ec) return {};

    if (!entry.is_regular_file(ec) || ec) continue;

    const auto& path = entry.path();

    if (path.filename() == "tolza.toml") return path.string();
  }

  return {};
}


bool common::fileutils::is_barrel_file(std::string_view file_path) noexcept
{
  if (!fs::exists(file_path)) FATAL_ERROR(std::format("File path at \"{}\" dosen't exists", file_path));

  std::ifstream f(file_path.data());
  std::string   line;

  while (std::getline(f, line)) {
    if (line.find_first_not_of(" \t\r\n") == std::string::npos) continue;

    return line.starts_with("# barrel");
  }

  return false;
}

bool common::fileutils::is_barrel_usercode(std::string_view file_path) noexcept
{
  fs::path p(file_path.data());

  return p.stem() == "mod";
}


void common::fileutils::write_barrel(std::string_view target_dir, std::string_view common_alias) noexcept
{
  // generate sub barrels before the parent barrel
  for (const auto& entry : fs::directory_iterator(target_dir)) {
    if (entry.is_directory()) write_barrel(entry.path().string(), common_alias);
  }

  auto files = common::fileutils::find_tolza_files(target_dir, false);

  std::string out_str;
  out_str.reserve(60 * files.size());

  for (const auto& file : files) {
    std::string name  = fs::path(file).stem();
    std::string alias = common_alias.empty() ? name : std::string(common_alias);

    out_str += std::format("reexport self::{} as {}\n", name, alias);
  }


  fs::path p = fs::path(target_dir).parent_path() / fs::path(target_dir).stem();
  p.replace_extension(TOLZA_FILE_EXTENSION);


  // if file already exists and is not a barrel file : it's some user code !
  if (fs::exists(p) && !is_barrel_file(p.string())) {
    std::println("[tolza] Barrel creation: user code detected at barrel path {}", p.string());

    // get barrel usercode standard location
    fs::path barrel_usercode = p.parent_path() / p.stem() / "mod";
    barrel_usercode.replace_extension(TOLZA_FILE_EXTENSION);

    // barrel usercode already exists, do not modify any existing usercode !
    // move code to ./.current.tlz
    if (fs::exists(barrel_usercode)) {
      // create temporary file of the user to let him decide
      std::string temp_name     = std::format(".{}.{}", p.stem().string(), TOLZA_FILE_EXTENSION);
      fs::path    temp_usercode = p.parent_path() / temp_name;

      fs::copy_file(p, temp_usercode, fs::copy_options::overwrite_existing);
      std::println("[tolza] User code moved to {}", temp_usercode.string());
    }
    // move code to ./current/mod.tlz
    else {
      fs::copy_file(p, barrel_usercode, fs::copy_options::overwrite_existing);
      std::string alias = common_alias.empty() ? std::string(p.stem()) : std::string(common_alias);

      out_str += std::format("reexport self::mod as {}", alias);

      std::println("[tolza] User code moved to {}", barrel_usercode.string());
    }
  }

  std::string date = time::now_datetime();

  std::string   header = std::format(BARREL_FILE_HEADER, common::TOLZA_VERSION, date);
  std::ofstream f(p);
  f << header;
  f << out_str;
  f.close();
}