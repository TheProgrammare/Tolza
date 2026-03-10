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
#include <benhoyt/cpp/INIReader.h>
#include <sys/types.h>
#include <unistd.h>

#include "command_check.hpp"
#include "parser_command.hpp"
#include "toolchain.hpp"

void command::build::err(const std::string& msg)
{
  std::cerr << "[build] [ERROR] " << msg << std::endl;
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

std::optional<toolchain::CompCtx> command::build::init_compilation_context(const fs::path& path)
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

  fs::path config_path = path / "velox.config";
  if (auto result = command::check::check_workspace(path, false); !result) {
    err("The velox.config at " + path.string() + " is invalid.");
    return std::nullopt;
  }

  INIReader reader(config_path);
  auto      target_config = remove_quotes(reader.GetString("target", "config", "self"));
  if (target_config == "self") {
    return parse_compilation_context(config_path);
  } else {
    // velox.config set all defaults
    auto ctx = parse_compilation_context(config_path);

    auto sub_config_path = remove_quotes(reader.GetString("sub_configs", target_config, ""));
    if (sub_config_path.empty() && target_config != "self") {
      err("The specified config at " + sub_config_path + " is not defined in [sub_configs] section.");
      return std::nullopt;
    }

    if (!fs::exists(sub_config_path)) {
      err("The specified condig defined in [sub_configs] section located at " + sub_config_path + " dosen't exists.");
      return std::nullopt;
    }

    if (auto result = command::check::check_velox_config(sub_config_path, false); !result)
      err("The velox.config at " + sub_config_path + " is invalid.");
    return std::nullopt;

    return parse_compilation_context(sub_config_path);
  }
}


toolchain::CompCtx command::build::parse_compilation_context(const fs::path& config_path)
{
  auto resolve_path = [&config_path](const fs::path& _path) -> fs::path {
    if (_path.is_relative()) {
      return fs::weakly_canonical(config_path.parent_path() / _path);
    } else {
      return fs::absolute(_path);
    }
  };

  toolchain::CompCtx result;

  result.config_path = config_path;

  INIReader reader(config_path);

  result.target_abi    = remove_quotes(reader.GetString("target", "abi", "LP64"));
  result.target_arch   = remove_quotes(reader.GetString("target", "arch", ""));
  result.target_bits   = reader.GetInteger("target", "bits", 64);
  result.target_os     = remove_quotes(reader.GetString("target", "os", ""));
  result.target_libc   = remove_quotes(reader.GetString("target", "libc", ""));
  result.target_config = remove_quotes(reader.GetString("target", "config", "self"));

  result.profile_debug     = reader.GetBoolean("profile", "debug", false);
  result.profile_opt_level = reader.GetInteger("profile", "opt_level", 0);
  result.profile_size_opt  = reader.GetBoolean("profile", "size_opt", false);

  result.log_all          = reader.GetBoolean("logs", "all", false);
  result.log_filesystem   = reader.GetBoolean("logs", "filesystem", false);
  result.log_lexer        = reader.GetBoolean("logs", "lexer", false);
  result.log_preprocessor = reader.GetBoolean("logs", "preprocessor", false);
  result.log_parser       = reader.GetBoolean("logs", "parser", false);
  result.log_binder       = reader.GetBoolean("logs", "binder", false);
  result.log_exporter     = reader.GetBoolean("logs", "exporter", false);
  result.log_resolver     = reader.GetBoolean("logs", "resolver", false);
  result.log_LLVM_IR      = reader.GetBoolean("logs", "llvm-ir", false);
  result.log_linker       = reader.GetBoolean("logs", "linker", false);

  result.warn_all       = reader.GetBoolean("warnings", "all", true);
  result.warn_extra     = reader.GetBoolean("warnings", "extra", true);
  result.warn_pedantic  = reader.GetBoolean("warnings", "pedantic", true);
  result.warn_level     = reader.GetInteger("warnings", "level", 3);
  result.warn_unused    = reader.GetBoolean("warnings", "unused", true);
  result.warn_dead_code = reader.GetBoolean("warnings", "dead_code", true);
  result.warn_as_error  = reader.GetBoolean("warnings", "as_error", true);

  result.print_ast = reader.GetBoolean("printer", "ast", false);

  for (auto& define : reader.Keys("defines")) {
    auto val               = remove_quotes(reader.GetString("defines", define, ""));
    result.defines[define] = val;
  }

  result.undefines = reader.Keys("undefines");

  auto emit_mode = remove_quotes(reader.GetString("codegen", "emit_mode", "BIN"));
  if (emit_mode == "LLVM") result.codegen_emit_mode = toolchain::CompCtx::EEmitMode::LLVM;
  if (emit_mode == "OBJ") result.codegen_emit_mode = toolchain::CompCtx::EEmitMode::OBJ;
  if (emit_mode == "ASM") result.codegen_emit_mode = toolchain::CompCtx::EEmitMode::ASM;
  if (emit_mode == "BC") result.codegen_emit_mode = toolchain::CompCtx::EEmitMode::BC;
  if (emit_mode == "BIN") result.codegen_emit_mode = toolchain::CompCtx::EEmitMode::BIN;

  result.codegen_build_dir = resolve_path(remove_quotes(reader.GetString("codegen", "build_dir", "./build")));

  result.project_dir  = resolve_path(remove_quotes(reader.GetString("project", "project_dir", "./")));
  result.source_dir   = resolve_path(remove_quotes(reader.GetString("project", "source_dir", "./src")));
  result.vendor_dir   = resolve_path(remove_quotes(reader.GetString("project", "vendor_dir", "./vendor")));
  result.ffi_json_dir = resolve_path(remove_quotes(reader.GetString("project", "ffi_json_dir", "./ffi-json")));
  result.binding_dir  = resolve_path(remove_quotes(reader.GetString("project", "binding_dir", "./binding")));

  fs::path default_compiler_file;

  result.compiler_file = resolve_path(remove_quotes(reader.GetString("project", "compiler_file", "")));

  if (result.compiler_file.empty()) {
    if (auto comp = toolchain::find_lastest_compiler()) result.compiler_file = comp.value();

    result.compiler_file = "";
  }

  for (auto& sub_config : reader.Keys("sub_configs")) {
    result.sub_configs[sub_config] = resolve_path(remove_quotes(reader.GetString("sub_configs", sub_config, "")));
  }

  return result;
}


void command::build::parse_args_for_compilation_context(toolchain::CompCtx& ctx, int start_arg, int argc,
                                                        const char* argv[])
{
  for (int i = start_arg; i < argc; ++i) {
    const std::string& arg = argv[i];

    // e.g. --debug
    auto bool_arg = [&](bool& input, const std::string& arg_name, const std::string& alt_arg_name = "") {
      if (!arg_name.empty() && alt_arg_name.empty()) {
        input = false;
        return false;
      }

      if (!arg_name.empty() && arg.rfind("--" + arg_name, 0) == 0) {
        ctx.COMPILATION_ARGS.emplace(arg_name, "true");
        input = true;
      } else if (!arg_name.empty() && arg.rfind("--!" + arg_name, 0) == 0) {
        ctx.COMPILATION_ARGS.emplace(arg_name, "false");
        input = false;
      }
      if (!alt_arg_name.empty() && arg.rfind("-" + alt_arg_name, 0) == 0) {
        ctx.COMPILATION_ARGS.emplace(alt_arg_name, "true");
        input = true;
      } else if (!alt_arg_name.empty() && arg.rfind("-!" + alt_arg_name, 0) == 0) {
        ctx.COMPILATION_ARGS.emplace(alt_arg_name, "false");
        input = false;
      }
      return true;
    };

    // e.g. --os="linux"
    auto str_arg = [&](std::string& input, const std::string& arg_name, const std::string& alt_arg_name = "") {
      if (arg_name.empty() && alt_arg_name.empty()) {
        return false;
      }

      if (!arg_name.empty() && arg.rfind("--" + arg_name + "=", 0) == 0) {
        ctx.COMPILATION_ARGS.emplace(arg_name, arg);
      }
      if (!alt_arg_name.empty() && arg.rfind("-" + alt_arg_name, 0) == 0) {
        ctx.COMPILATION_ARGS.emplace(alt_arg_name, arg);
      }

      input = arg;
      return true;
    };

    // e.g. --dest="/mnt/data/my_project"
    auto path_arg = [&](fs::path& input, const std::string& arg_name, const std::string& alt_arg_name = "") {
      if (arg_name.empty() && alt_arg_name.empty()) {
        return false;
      }

      if (!arg_name.empty() && arg.rfind("--" + arg_name + "=", 0) == 0) {
        ctx.COMPILATION_ARGS.emplace(arg_name, arg);
      }
      if (!alt_arg_name.empty() && arg.rfind("-" + alt_arg_name, 0) == 0) {
        ctx.COMPILATION_ARGS.emplace(alt_arg_name, arg);
      }

      input = fs::weakly_canonical(arg);
      return true;
    };

    // e.g. --opt-level=0
    auto size_arg = [&](size_t& input, const std::string& arg_name, const std::string& alt_arg_name = "") {
      if (arg_name.empty() && alt_arg_name.empty()) {
        return false;
      }

      if (!arg_name.empty() && arg.rfind("--" + arg_name + "=", 0) == 0) {
        ctx.COMPILATION_ARGS.emplace(arg_name, arg);
      }
      if (!alt_arg_name.empty() && arg.rfind("-" + alt_arg_name, 0) == 0) {
        ctx.COMPILATION_ARGS.emplace(alt_arg_name, arg);
      }

      input = std::stoul(arg);
      return true;
    };

    // target
    if (str_arg(ctx.target_abi, "abi")) continue;
    if (str_arg(ctx.target_arch, "arch")) continue;
    if (size_arg(ctx.target_bits, "bits")) continue;
    if (str_arg(ctx.target_os, "os")) continue;
    if (str_arg(ctx.target_libc, "libc")) continue;
    if (str_arg(ctx.target_config, "config")) continue;

    // profile
    if (bool_arg(ctx.profile_debug, "debug", "d")) continue;
    if (bool_arg(ctx.profile_debug, "release", "r")) {
      ctx.profile_debug = false;
      continue;
    }
    if (size_arg(ctx.profile_opt_level, "opt-level")) continue;
    if (bool_arg(ctx.profile_size_opt, "size-opt")) continue;

    // logs
    if (bool_arg(ctx.log_all, "log-all", "lall")) continue;
    if (bool_arg(ctx.log_filesystem, "log-filesystem", "lfs")) continue;
    if (bool_arg(ctx.log_lexer, "log-lexer", "llex")) continue;
    if (bool_arg(ctx.log_preprocessor, "log-pre", "lpre")) continue;
    if (bool_arg(ctx.log_parser, "log-parser", "lpar")) continue;
    if (bool_arg(ctx.log_binder, "log-binder", "lemb")) continue;
    if (bool_arg(ctx.log_exporter, "log-exporter", "lexp")) continue;
    if (bool_arg(ctx.log_resolver, "log-resolver", "lres")) continue;
    if (bool_arg(ctx.log_LLVM_IR, "log-llvm", "lllvm")) continue;
    if (bool_arg(ctx.log_linker, "log-linker", "llink")) continue;

    // warnings
    if (bool_arg(ctx.warn_all, "warn-all", "wall")) continue;
    if (bool_arg(ctx.warn_extra, "warn-extra", "wextra")) continue;
    if (bool_arg(ctx.warn_pedantic, "warn-pedantic", "wpedan")) continue;
    if (size_arg(ctx.warn_level, "warn-level")) continue;
    if (bool_arg(ctx.warn_unused, "warn-unused", "wun")) continue;
    if (bool_arg(ctx.warn_dead_code, "warn-dead-code", "wdc")) continue;
    if (bool_arg(ctx.warn_as_error, "warn-as-error", "wae")) continue;

    // printer
    if (bool_arg(ctx.print_ast, "print-ast")) continue;

    // define macro
    if (arg.rfind("-D", 0) == 0) {
      auto def    = arg.substr(2);
      auto eq_pos = def.find('=');

      std::string name, value;

      if (eq_pos != std::string::npos) {
        name  = def.substr(0, eq_pos);
        value = def.substr(eq_pos + 1);
      } else {
        name  = def;
        value = "1"; // implicit value
      }
      ctx.COMPILATION_ARGS.emplace(name, value);
      ctx.defines.emplace(name, value);

      continue;
    }

    // undefine macro
    if (arg.rfind("-U", 0) == 0) {
      auto name = arg.substr(2);

      ctx.COMPILATION_ARGS.emplace(name, ""); // no value for -UName
      ctx.undefines.push_back(name);
      continue;
    }

    // codegen
    std::string emit_mode;
    if (str_arg(emit_mode, "emit")) {
      if (emit_mode == "obj")
        ctx.codegen_emit_mode = toolchain::CompCtx::EEmitMode::OBJ;
      else if (emit_mode == "asm")
        ctx.codegen_emit_mode = toolchain::CompCtx::EEmitMode::ASM;
      else if (emit_mode == "bc")
        ctx.codegen_emit_mode = toolchain::CompCtx::EEmitMode::BC;
      else if (emit_mode == "bin")
        ctx.codegen_emit_mode = toolchain::CompCtx::EEmitMode::BIN;
      else
        std::cerr << "Invalid emit mode value --emit-mode=" << emit_mode << std::endl;
      continue;
    }
    if (path_arg(ctx.codegen_build_dir, "build")) continue;

    // project
    if (path_arg(ctx.project_dir, "project")) continue;
    if (path_arg(ctx.source_dir, "src")) continue;
    if (path_arg(ctx.binding_dir, "binding")) continue;
    if (path_arg(ctx.vendor_dir, "vendor")) continue;
    if (path_arg(ctx.ffi_json_dir, "ffi-json")) continue;
    if (path_arg(ctx.compiler_file, "compiler")) continue;

    std::cerr << "Warning: unknown argument '" << arg << "'" << std::endl;
  }
}

bool command::build::generate_ffi_json(const fs::path& compiler_file, const fs::path& target_dir,
                                       const fs::path& dest_dir)
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

bool command::build::start_compilation(const toolchain::CompCtx& ctx)
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

  fmt_template(cmd, {ctx.get_compiler_file(), args + "-fvt"});

  log("Compiler command launched: \n" + cmd);

  int res = std::system(cmd.c_str());
  // 0 == no error
  // 1 == error
  return res == 0;
}