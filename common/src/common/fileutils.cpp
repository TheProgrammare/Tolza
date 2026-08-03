#include "fileutils.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

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


std::string common::fileutils::resolve_path(std::string_view s, std::string_view relative) noexcept
{
  if (s.empty()) return std::string(s);

  std::string path_str(s);

  // Expand tilde
  if (s[0] == '~') {
    const char* home =
#if __unix__
        std::getenv("HOME");
#elif _WIN32
        std::getenv("USERPROFILE");
#else
        home = nullptr
#endif

    if (!home) FATAL_ERROR("The home environment is not defined");

    path_str = std::string(home) + std::string(s.substr(1));
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


bool common::fileutils::is_velox_file(std::string_view file_path) noexcept
{
  fs::path f(file_path);
  return f.extension() == VELOX_FILE_EXTENSION;
}

std::string common::fileutils::get_velox_file(std::string_view path) noexcept
{
  auto p = fs::weakly_canonical(path);

  if (p.has_extension() && p.extension() != VELOX_FILE_EXTENSION) return "";
  if (!p.has_extension()) {
    p.replace_extension(VELOX_FILE_EXTENSION);
  }

  return fs::exists(p) ? p : "";
}

std::set<std::string> common::fileutils::find_velox_files(std::string_view target_dir, bool is_recursive) noexcept
{
  std::set<std::string> out;
  fs::path              dir(target_dir);

  assert(fs::exists(target_dir) && "Target directory dosen't exists");

  try {
    if (is_recursive) {
      for (const auto& entry : fs::recursive_directory_iterator(dir)) {
        if (fs::is_regular_file(entry) && entry.path().extension() == VELOX_FILE_EXTENSION)
          out.insert(entry.path().string());
      }
    } else {
      for (const auto& entry : fs::directory_iterator(dir)) {
        if (fs::is_regular_file(entry) && entry.path().extension() == VELOX_FILE_EXTENSION)
          out.insert(entry.path().string());
      }
    }
  } catch (const std::runtime_error& err) {
    std::cerr << err.what() << "\n";
    return {};
  }

  return out;
}


bool common::fileutils::is_barrel_file(std::string_view file_path) noexcept
{
  if (!fs::exists(file_path)) FATAL_ERROR("File path at \"" + std::string(file_path) + "\" dosen't exists");

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

  auto files = common::fileutils::find_velox_files(target_dir, false);

  std::string out_str;
  out_str.reserve(60 * files.size());

  for (const auto& file : files) {
    std::string name  = fs::path(file).stem();
    std::string alias = common_alias.empty() ? name : std::string(common_alias);

    out_str += "reexport self::" + name + " as " + alias + "\n";
  }


  fs::path p = fs::path(target_dir).parent_path() / fs::path(target_dir).stem();
  p.replace_extension(VELOX_FILE_EXTENSION);


  // if file already exists and is not a barrel file : it's some user code !
  if (fs::exists(p) && !is_barrel_file(p.string())) {
    std::cout << "[velox] Barrel creation: user code detected at barrel path " << p << "\n";

    // get barrel usercode standard location
    fs::path barrel_usercode = p.parent_path() / p.stem() / "mod";
    barrel_usercode.replace_extension(VELOX_FILE_EXTENSION);

    // barrel usercode already exists, do not modify any existing usercode !
    // move code to ./.current.vlx
    if (fs::exists(barrel_usercode)) {
      // create temporary file of the user to let him decide
      std::string temp_name     = "." + p.stem().string() + "." + std::string(VELOX_FILE_EXTENSION);
      fs::path    temp_usercode = p.parent_path() / temp_name;

      fs::copy_file(p, temp_usercode, fs::copy_options::overwrite_existing);
      std::cout << "[velox] User code moved to " << temp_usercode << "\n";
    }
    // move code to ./current/mod.vlx
    else {
      fs::copy_file(p, barrel_usercode, fs::copy_options::overwrite_existing);
      std::string alias = common_alias.empty() ? std::string(p.stem()) : std::string(common_alias);

      out_str += "reexport self::mod as " + alias + "\n";

      std::cout << "[velox] User code moved to " << barrel_usercode << "\n";
    }
  }

  std::string date = time::now_datetime();

  std::map<std::string_view, std::string_view> header_fmt = {
      {"vc_version", common::VELOX_COMMON_VERSION},
      {"date",       date                        },
  };

  std::string header(BARREL_FILE_HEADER);
  common::utils::fmt_template(header, header_fmt);
  std::ofstream f(p);
  f << header;
  f << out_str;
  f.close();
}