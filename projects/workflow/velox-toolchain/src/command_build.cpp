/*
 * This program include and use the marzer/tomlplusplus project
 * You can find this project at
 *
 *     https://marzer.github.io/tomlplusplus/
 *
 * Used for the ini format file reading.
 */

#include "command_build.hpp"

#include <cstdlib>
#include <expected>
#include <filesystem>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <unistd.h>

#include <marzer/toml++.hpp>


#include "command_check.hpp"
#include "common.hpp"
#include "toolchain/toolchain.hpp"
#include "toolchain_context.hpp"

namespace fs = std::filesystem;


void command::build::log(const std::string& msg)
{
  std::cout << "[build] " << msg << std::endl;
}

void command::build::err(const std::string& msg)
{
  std::cerr << "[build:ERROR] " << msg << std::endl;
}


std::string remove_quotes(const std::string& s)
{
  if (s.size() >= 2 && s.front() == '"' && s.back() == '"') {
    return s.substr(1, s.size() - 2);
  }
  return "";
}

common::CompCtx command::build::generate_compilation_context(const std::string& path)
{
  static const std::string welcome =
      R"([velox]
Welcome to the Velox toolchain !
  Version: %0
  Toolchain launched at: %1 
)";

  std::string wel = welcome;
  common::fmt_template(wel, {common::SOFTWARE_VERSION, path});
  std::cout << wel << std::endl;

  fs::path config_path = fs::path(path) / "velox-compiler.toml";
  if (auto result = command::check::check_workspace(path, false); !result) {
    err("The velox.toml at " + fs::path(path).string() + " is invalid.");
    return common::CompCtx::invalid();
  }

  toml::table tbl;
  try {
    tbl = toml::parse_file(config_path.string());
  } catch (const toml::parse_error& e) {
    err("Parsing failed:\n" + std::string(e.what()) + "\n");
    return common::CompCtx::invalid();
  }

  std::string target_config = tbl.at_path("target.sub_config").value_or("");
  if (target_config == "self") {
    return config_to_compilation_context(config_path);
  } else {
    // velox.toml set all defaults
    auto ctx = config_to_compilation_context(config_path);

    std::string sub_config_path = tbl.at_path("sub_configs." + target_config).value_or("");
    if (sub_config_path.empty() && target_config != "self") {
      err("The specified config at " + sub_config_path + " is not defined in [sub_configs] section.");
      return common::CompCtx::invalid();
    }

    if (!fs::exists(sub_config_path)) {
      err("The specified condig defined in [sub_configs] section located at " + sub_config_path + " dosen't exists.");
      return common::CompCtx::invalid();
    }

    if (auto result = command::check::check_velox_config(sub_config_path, false); !result)
      err("The velox.toml at " + sub_config_path + " is invalid.");
    return common::CompCtx::invalid();

    return config_to_compilation_context(sub_config_path);
  }
}


common::CompCtx command::build::config_to_compilation_context(const std::string& config_file)
{
  auto resolve_path = [&config_file](const std::string& _path) -> fs::path {
    if (fs::path(_path).is_relative()) {
      return fs::weakly_canonical(fs::path(config_file).parent_path() / _path);
    } else {
      return fs::absolute(_path);
    }
  };

  auto is_same_key = [](const std::string& a, const std::string& b) {
    if (a.size() != b.size()) return false;
    return std::equal(a.begin(), a.end(), b.begin(),
                      [](char c1, char c2) { return std::tolower(c1) == std::tolower(c2); });
  };

  auto out = common::CompCtx::invalid();

  out.current_config_file = config_file;

  toml::table tbl;
  tbl = toml::parse_file(config_file);

  // target
  out.target_project_name = tbl.at_path("target.project_name").value_or("");
  out.target_arch         = tbl.at_path("target.arch").value_or("");
  out.target_os           = tbl.at_path("target.os").value_or("");
  out.target_vendor       = tbl.at_path("target.vendor").value_or("");
  out.target_abi          = tbl.at_path("target.abi").value_or("");
  out.target_libc         = tbl.at_path("target.libc").value_or("");
  out.target_cpu          = tbl.at_path("target.cpu").value_or("");
  out.target_features     = tbl.at_path("target.features").value_or("");
  if (std::string arg = tbl.at_path("target.code_model").value_or(""); !arg.empty()) {
    if (is_same_key(arg, "tiny"))
      out.target_code_model = common::CompCtx::ECodeModel::Tiny;
    else if (is_same_key(arg, "small"))
      out.target_code_model = common::CompCtx::ECodeModel::Small;
    else if (is_same_key(arg, "kernel"))
      out.target_code_model = common::CompCtx::ECodeModel::Kernel;
    else if (is_same_key(arg, "medium"))
      out.target_code_model = common::CompCtx::ECodeModel::Medium;
    else if (is_same_key(arg, "large"))
      out.target_code_model = common::CompCtx::ECodeModel::Large;
  }
  if (std::string arg = tbl.at_path("target.reloc_model").value_or(""); !arg.empty()) {
    if (is_same_key(arg, "static"))
      out.target_reloc_model = common::CompCtx::ERelocModel::Static;
    else if (is_same_key(arg, "pic"))
      out.target_reloc_model = common::CompCtx::ERelocModel::PIC;
    else if (is_same_key(arg, "pie"))
      out.target_reloc_model = common::CompCtx::ERelocModel::PIE;
    else if (is_same_key(arg, "ropi"))
      out.target_reloc_model = common::CompCtx::ERelocModel::ROPI;
    else if (is_same_key(arg, "rwpi"))
      out.target_reloc_model = common::CompCtx::ERelocModel::RWPI;
    else if (is_same_key(arg, "ropi_rwpi"))
      out.target_reloc_model = common::CompCtx::ERelocModel::ROPI_RWPI;
  }
  out.target_sub_config = tbl.at_path("target.sub_config").value_or("");

  // profile
  out.profile_debug = tbl.at_path("profile.debug").value_or(false);
  if (std::string arg = tbl.at_path("profile.optimization").value_or(""); !arg.empty()) {
    if (arg == "O0")
      out.profile_optimization = common::CompCtx::EOptimization::O0;
    else if (arg == "O1")
      out.profile_optimization = common::CompCtx::EOptimization::O1;
    else if (arg == "O2")
      out.profile_optimization = common::CompCtx::EOptimization::O2;
    else if (arg == "O3")
      out.profile_optimization = common::CompCtx::EOptimization::O3;
    else if (arg == "Os")
      out.profile_optimization = common::CompCtx::EOptimization::Os;
    else if (arg == "Oz")
      out.profile_optimization = common::CompCtx::EOptimization::Oz;
    else {
      err("Invalid optimization key '" + arg + "'.");
      throw std::runtime_error("");
    }
  }

  if (const toml::array* emits = tbl.at_path("target.emits").as_array()) {
    for (auto& elem : *emits) {
      auto emit = elem.as_string()->value_or("");

      if (common::Key_Arg_Asso::val_is_valid("target.emits", emit)) {
        out.target_emits.insert(common::CompCtx::str_to_EEmit(emit));
      } else {
        err("Invalid emits key '" + std::string(emit) + "'.");
        throw std::runtime_error("");
      }
    }
  }

  if (const toml::array* logs = tbl.at_path("log.logs").as_array()) {
    for (auto& elem : *logs) {
      auto log = elem.as_string()->value_or("");

      if (common::Key_Arg_Asso::val_is_valid("log.logs", log)) {
        out.logs.insert(log);
      } else {
        err("Invalid logs key '" + std::string(log) + "'.");
        throw std::runtime_error("");
      }
    }
  }


  if (const toml::array* warns = tbl.at_path("warning.warnings").as_array()) {
    for (auto& elem : *warns) {
      auto warn = elem.as_string()->value_or("");

      if (common::Key_Arg_Asso::val_is_valid("warning.warnings", warn)) {
        out.warns.insert(warn);
      } else {
        err("Invalid warnings key '" + std::string(warn) + "'.");
        throw std::runtime_error("");
      }
    }
  }


  if (std::string arg = tbl.at_path("warning.level").value_or(""); !arg.empty()) {
    if (is_same_key(arg, "W0"))
      out.warn_level = common::CompCtx::EWarnLevel::W0;
    else if (is_same_key(arg, "W1"))
      out.warn_level = common::CompCtx::EWarnLevel::W1;
    else if (is_same_key(arg, "W2"))
      out.warn_level = common::CompCtx::EWarnLevel::W2;
    else if (is_same_key(arg, "W3"))
      out.warn_level = common::CompCtx::EWarnLevel::W3;
    else {
      err("Invalid warning level key '" + std::string(arg) + "'.");
      throw std::runtime_error("");
    }
  }

  if (const toml::array* debugs = tbl.at_path("debug.debugs").as_array()) {
    for (auto& elem : *debugs) {
      auto debug = elem.as_string()->value_or("");

      if (common::Key_Arg_Asso::val_is_valid("debug.debugs", debug)) {
        out.debugs.insert(debug);
      } else {
        err("Invalid debugs key '" + std::string(debug) + "'.");
        throw std::runtime_error("");
      }
    }
  }

  if (const toml::table* defines = tbl.at_path("defines").as_table()) {
    for (auto& [key, val] : *defines) out.defines[std::string(key.str())] = val.value_or("");
  }

  if (const toml::array* undefines = tbl.at_path("undefines").as_array()) {
    for (auto& key : *undefines) out.undefines.push_back(key.value_or(""));
  }

  // directories
  out.dir_project  = resolve_path(tbl.at_path("directory.project").value_or(""));
  out.dir_build    = resolve_path(tbl.at_path("directory.build").value_or(""));
  out.dir_source   = resolve_path(tbl.at_path("directory.source").value_or(""));
  out.dir_vendor   = resolve_path(tbl.at_path("directory.vendor").value_or(""));
  out.dir_binding  = resolve_path(tbl.at_path("directory.binding").value_or(""));
  out.dir_ffi_json = resolve_path(tbl.at_path("directory.ffi_json").value_or(""));
  out.dir_compiler = resolve_path(tbl.at_path("directory.compiler").value_or(""));
  out.dir_stdlib   = resolve_path(tbl.at_path("directory.stdlib").value_or(""));
  out.dir_packages = resolve_path(tbl.at_path("directory.packages").value_or(""));

  if (const toml::table* sub_configs = tbl.at_path("config").as_table()) {
    for (auto& [key, val] : *sub_configs) out.sub_configs[std::string(key)] = val.value_or("./");
  }

  // llvm
  out.llvm_triple        = tbl.at_path("llvm.triple").value_or("");
  out.llvm_verify_module = tbl.at_path("llvm.verify_module").value_or(false);
  if (const toml::array* llvm_args = tbl.at_path("llvm.args").as_array()) {
    for (auto& key : *llvm_args) out.llvm_args.push_back(key.value_or(""));
  }

  // make CompCtx valid : assign argc, argv
  auto args = out.to_args();
  out.argc  = args.size();

  std::vector<const char*> c_strs;
  c_strs.reserve(args.size());
  for (auto& arg : args) c_strs.emplace_back(arg.c_str());
  out.argv = c_strs.data();

  return out;
}


bool command::build::generate_ffi_json(const std::string& compiler_file, const std::string& source_dir,
                                       const std::string& dest_dir)
{
  // %0 target executable
  // %1 target dir
  // %2 destination dir
  static const std::string base_cmd = R"("%0" gen-ffi "%1" "%2")";

  std::string cmd;
  common::fmt_template(cmd, {compiler_file, source_dir, dest_dir});

  int res = std::system(cmd.c_str());
  // 0 == no error
  // 1 == error
  return res == 0;
}

bool command::build::start_compilation(int argc, const char* argv[])
{
  // %0 target executable
  // %1 args
  static const std::string base_cmd = R"("%0" build %1)";

  std::string cmd = base_cmd;
  std::string args;
  for (size_t i = 0; i < argc; i++) {
    args += argv[i];
    args += " ";
  }

  common::fmt_template(cmd, {common::TOOL_CTX.compiler_used, args});

  log("Compiler command launched: \n" + cmd);

  int res = std::system(cmd.c_str());
  // 0 == no error
  // 1 == error
  return res == 0;
}