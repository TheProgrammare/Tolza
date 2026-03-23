/*
 * This program include and use the benhoyt/inih project
 * You can find this project at
 *
 *     https://github.com/benhoyt/inih
 *
 * Used for the ini format file reading.
 */

#include "command_build.hpp"

#include <cstdlib>
#include <expected>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <sys/types.h>
#include <unistd.h>

#include <benhoyt/cpp/INIReader.h>


#include "command_check.hpp"
#include "parser_command.hpp"
#include "toolchain.hpp"

namespace fs = std::filesystem;

void command::build::err(const std::string& msg)
{
  std::cerr << "[build:ERROR] " << msg << std::endl;
}


std::string remove_quotes(const std::string& str)
{
  // Vérifie si la chaîne est trop courte ou n'a pas de guillemets
  if (str.length() < 2) return str;

  size_t start = 0;
  size_t end   = str.length() - 1;

  // Retire le guillemet de début
  if (str.front() == '"') start++;

  // Retire le guillemet de fin
  if (str.back() == '"') end--;

  // Retourne la sous-chaîne sans guillemets
  return str.substr(start, end - start + 1);
}

common::CompCtx command::build::init_compilation_context(const std::string& path)
{
  static const std::string welcome =
      R"([velox-toolchain]
Welcome to the Velox toolchain !
  Version: %0
  Toolchain launched at: %1 
)";

  std::string wel = welcome;
  fmt_template(wel, {toolchain::VELOX_TOOLCHAIN_VERSION, path});
  std::cout << wel << std::endl;

  fs::path config_path = fs::path(path) / "velox.config";
  if (auto result = command::check::check_workspace(path, false); !result) {
    err("The velox.config at " + fs::path(path).string() + " is invalid.");
    return common::CompCtx::invalid();
  }

  INIReader reader(config_path);
  auto      target_config = remove_quotes(reader.GetString("target", "config", "self"));
  if (target_config == "self") {
    return config_to_compilation_context(config_path);
  } else {
    // velox.config set all defaults
    auto ctx = config_to_compilation_context(config_path);

    auto sub_config_path = remove_quotes(reader.GetString("sub_configs", target_config, ""));
    if (sub_config_path.empty() && target_config != "self") {
      err("The specified config at " + sub_config_path + " is not defined in [sub_configs] section.");
      return common::CompCtx::invalid();
    }

    if (!fs::exists(sub_config_path)) {
      err("The specified condig defined in [sub_configs] section located at " + sub_config_path + " dosen't exists.");
      return common::CompCtx::invalid();
    }

    if (auto result = command::check::check_velox_config(sub_config_path, false); !result)
      err("The velox.config at " + sub_config_path + " is invalid.");
    return common::CompCtx::invalid();

    return config_to_compilation_context(sub_config_path);
  }
}


common::CompCtx command::build::config_to_compilation_context(const std::string& config_path)
{
  auto resolve_path = [&config_path](const std::string& _path) -> fs::path {
    if (fs::path(_path).is_relative()) {
      return fs::weakly_canonical(fs::path(config_path).parent_path() / _path);
    } else {
      return fs::absolute(_path);
    }
  };

  auto out = common::CompCtx::invalid();

  out.current_config_file = config_path;

  INIReader reader(config_path);

  out.target_abi    = remove_quotes(reader.GetString("target", "abi", "LP64"));
  out.target_arch   = remove_quotes(reader.GetString("target", "arch", ""));
  out.target_bits   = reader.GetInteger("target", "bits", 64);
  out.target_os     = remove_quotes(reader.GetString("target", "os", ""));
  out.target_libc   = remove_quotes(reader.GetString("target", "libc", ""));
  out.target_config = remove_quotes(reader.GetString("target", "config", "self"));

  out.profile_debug     = reader.GetBoolean("profile", "debug", false);
  out.profile_opt_level = reader.GetInteger("profile", "opt_level", 0);
  out.profile_size_opt  = reader.GetBoolean("profile", "size_opt", false);

  out.log_all          = reader.GetBoolean("logs", "all", false);
  out.log_filesystem   = reader.GetBoolean("logs", "filesystem", false);
  out.log_lexer        = reader.GetBoolean("logs", "lexer", false);
  out.log_preprocessor = reader.GetBoolean("logs", "preprocessor", false);
  out.log_parser       = reader.GetBoolean("logs", "parser", false);
  out.log_binder       = reader.GetBoolean("logs", "binder", false);
  out.log_exporter     = reader.GetBoolean("logs", "exporter", false);
  out.log_resolver     = reader.GetBoolean("logs", "resolver", false);
  out.log_LLVM_IR      = reader.GetBoolean("logs", "llvm-ir", false);
  out.log_linker       = reader.GetBoolean("logs", "linker", false);

  out.warn_all       = reader.GetBoolean("warnings", "all", true);
  out.warn_extra     = reader.GetBoolean("warnings", "extra", true);
  out.warn_pedantic  = reader.GetBoolean("warnings", "pedantic", true);
  out.warn_level     = reader.GetInteger("warnings", "level", 3);
  out.warn_unused    = reader.GetBoolean("warnings", "unused", true);
  out.warn_dead_code = reader.GetBoolean("warnings", "dead_code", true);
  out.warn_as_error  = reader.GetBoolean("warnings", "as_error", true);

  out.print_ast = reader.GetBoolean("printer", "ast", false);

  for (auto& define : reader.Keys("defines")) {
    auto val            = remove_quotes(reader.GetString("defines", define, ""));
    out.defines[define] = val;
  }

  out.undefines = reader.Keys("undefines");

  out.emit_bin         = reader.GetBoolean("codegen", "emit_bin", true);
  out.emit_llvm        = reader.GetBoolean("codegen", "emit_llvm", false);
  out.emit_obj         = reader.GetBoolean("codegen", "emit_obj", false);
  out.emit_asm         = reader.GetBoolean("codegen", "emit_asm", false);
  out.emit_bc          = reader.GetBoolean("codegen", "emit_bc", false);
  out.emit_static_lib  = reader.GetBoolean("codegen", "emit_static_lib", false);
  out.emit_dynamic_lib = reader.GetBoolean("codegen", "emit_dynamic_lib", false);

  out.codegen_build_dir = resolve_path(remove_quotes(reader.GetString("codegen", "build_dir", "./build")));

  out.project_dir  = resolve_path(remove_quotes(reader.GetString("project", "project_dir", "./")));
  out.source_dir   = resolve_path(remove_quotes(reader.GetString("project", "source_dir", "./src")));
  out.vendor_dir   = resolve_path(remove_quotes(reader.GetString("project", "vendor_dir", "./vendor")));
  out.ffi_json_dir = resolve_path(remove_quotes(reader.GetString("project", "ffi_json_dir", "./ffi-json")));
  out.binding_dir  = resolve_path(remove_quotes(reader.GetString("project", "binding_dir", "./binding")));

  for (auto& sub_config : reader.Keys("sub_configs")) {
    out.sub_configs[sub_config] = resolve_path(remove_quotes(reader.GetString("sub_configs", sub_config, "")));
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


bool command::build::generate_ffi_json(const std::string& compiler_file, const std::string& target_dir,
                                       const std::string& dest_dir)
{
  // %0 target executable
  // %1 target dir
  // %2 destination dir
  static const std::string base_cmd = R"("%0" velox-compiler gen-ffi "%1" "%2")";

  std::string cmd;
  fmt_template(cmd, {compiler_file, target_dir, dest_dir});

  int res = std::system(cmd.c_str());
  // 0 == no error
  // 1 == error
  return res == 0;
}

bool command::build::start_compilation(const common::CompCtx& ctx)
{
  // %0 target executable
  // %1 args
  static const std::string base_cmd = R"("%0" build %1)";

  std::string cmd = base_cmd;
  std::string args;
  for (const auto& arg : ctx.to_args()) {
    args += arg;
    args += " ";
  }

  fmt_template(cmd, {toolchain::TOOL_CTX.compiler_used, args});

  log("Compiler command launched: \n" + cmd);

  int res = std::system(cmd.c_str());
  // 0 == no error
  // 1 == error
  return res == 0;
}