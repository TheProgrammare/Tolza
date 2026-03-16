#include "toolchain.hpp"

#include <filesystem>
#include <optional>
#include <string>
#include <expected>
#include <iostream>

void toolchain::err(const std::string& msg)
{
  std::cerr << "[velox-toolchain] [ERROR] " << msg << std::endl;
}

void toolchain::log(const std::string& msg)
{
  std::cerr << "[velox-toolchain] " << msg << std::endl;
}

std::vector<std::string> toolchain::CompCtx::to_args() const
{
  std::vector<std::string> out;

  out.push_back("--abi=" + target_abi);
  out.push_back("--arch=" + target_arch);
  out.push_back("--bits=" + std::to_string(target_bits));
  out.push_back("--os=" + target_os);
  out.push_back("--libc=\"" + target_libc + "\"");
  out.push_back("--config=\"" + target_config + "\"");
  if (profile_debug)
    out.push_back("--debug");
  else
    out.push_back("--release");
  out.push_back("--opt-level=" + std::to_string(profile_opt_level));
  if (profile_debug) out.push_back("--size-opt");

  if (log_all) out.push_back("--log-all");
  if (log_filesystem) out.push_back("--log-filesystem");
  if (log_lexer) out.push_back("--log-lexer");
  if (log_preprocessor) out.push_back("--log-pre");
  if (log_parser) out.push_back("--log-parser");
  if (log_binder) out.push_back("--log-binder");
  if (log_exporter) out.push_back("--log-exporter");
  if (log_resolver) out.push_back("--log-resolver");
  if (log_LLVM_IR) out.push_back("--log-llvm");
  if (log_linker) out.push_back("--log-linker");

  if (warn_all) out.push_back("--warn-all");
  if (warn_extra) out.push_back("--warn-extra");
  if (warn_pedantic) out.push_back("--warn-pedantic");
  out.push_back("--warn-level=" + std::to_string(warn_level));
  if (warn_unused) out.push_back("--warn-unused");
  if (warn_dead_code) out.push_back("--warn-dead-code");
  if (warn_as_error) out.push_back("--warn-as-error");

  if (print_ast) out.push_back("--print-ast");

  for (auto [name, val] : defines) {
    out.push_back("-D" + name + "=" + val);
  }

  for (auto udef : undefines) {
    out.push_back("-U" + udef);
  }

  out.push_back("--emit=" + EEmitMode_to_str());
  out.push_back("--build=\"" + codegen_build_dir.string() + "\"");

  out.push_back("--project=\"" + project_dir.string() + "\"");
  out.push_back("--src=\"" + source_dir.string() + "\"");
  out.push_back("--vendor=\"" + vendor_dir.string() + "\"");
  out.push_back("--ffi-json=\"" + ffi_json_dir.string() + "\"");
  out.push_back("--binding=\"" + binding_dir.string() + "\"");

  return out;
}

std::vector<std::pair<toolchain::Version, fs::path>> toolchain::find_all_compilers()
{
  std::string exeName;
#ifdef _WIN32
  exeName = "velox-compiler.exe";
#else
  exeName = "velox-compiler";
#endif

  std::vector<fs::path> baseDirs;
#ifdef _WIN32
  baseDirs = {"C:/Program Files/Velox", "C:/Program Files (x86)/Velox"};
#elif __APPLE__
  baseDirs = {"/Applications/Velox"};
#else
  baseDirs = {"/usr/local/velox", "/opt/velox"};
#endif

  std::vector<std::pair<Version, fs::path>> compilers;

  for (const auto& base : baseDirs) {
    if (!fs::exists(base)) continue;

    for (const auto& entry : fs::directory_iterator(base)) {
      if (entry.is_directory()) {
        fs::path exePath = entry.path() / exeName;
        if (fs::exists(exePath)) {
          try {
            Version v(entry.path().filename().string(), "-");
            compilers.emplace_back(v, exePath);
          } catch (...) {
            // ignore folders with no velox version format name
          }
        }
      }
    }
  }

  if (compilers.empty()) err("No velox-compiler found.");
  log("Please, install velox-compiler to use proprely this toolchain.");
  return compilers;
}

std::optional<fs::path> toolchain::find_compiler_version(const std::string& version)
{
  try {
    Version v(version, ".");
    auto    compilers = find_all_compilers();
    if (!compilers.empty()) {
      return std::nullopt;
    }

    for (auto [ver, file] : compilers) {
      if (ver == v) return file;
    }

    toolchain::err("No velox-compiler found for the version " + version + ".");
    return std::nullopt;
  } catch (...) {
    toolchain::err("[velox-toolchain] [error] The version format \"" + version  + "\" is invalid.\n"
    "  Write version format like \"9999.99.99\" or \"9999.99.99b\" or \"9999.99.99a\"");
    return std::nullopt;
  }
}

std::optional<fs::path> toolchain::find_lastest_compiler()
{
  if (auto compilers = find_all_compilers(); !compilers.empty()) {
    auto latest = *std::max_element(compilers.begin(), compilers.end(),
                                    [](const auto& a, const auto& b) { return a.first < b.first; });
    return latest.second;
  } else {
    return std::nullopt;
  }
}

const std::string& toolchain::CompCtx::get_preprocess_dir() const
{
  static auto out = (fs::path(get_build_dir()) / "preprocess").string();
  return out;
}

const std::string& toolchain::CompCtx::get_debug_graph_dir() const
{
  static auto out = (fs::path(get_build_dir()) / "graph").string();
  return out;
}

const std::string& get_llvmir_dir() const
{
  static auto out = (fs::path(get_build_dir()) / "llvm-ir").string();
  return out;
}

toolchain::Version::Version(const std::string& str, const std::string& separator = "-")
{
  suffix        = '\0';
  size_t first  = str.find(separator);
  size_t second = str.find(separator, first + 1);
  if (first == std::string::npos || second == std::string::npos)
    throw std::invalid_argument("Invalid format version: " + str);

  year  = std::stoi(str.substr(0, first));
  month = std::stoi(str.substr(first + 1, second - first - 1));

  std::string dayPart = str.substr(second + 1);
  if (!dayPart.empty() && !isdigit(dayPart.back())) {
    suffix = dayPart.back();
    dayPart.pop_back();
  }
  day = std::stoi(dayPart);
}