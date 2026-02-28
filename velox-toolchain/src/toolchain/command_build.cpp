/*
 * This program include and use the benhoyt/inih project
 * You can find this project at
 *
 *     https://github.com/benhoyt/inih
 *
 * Used for the ini format file reading.
 */

#include "command_build.hpp"

#include <iostream>
#include <string>
#include <benhoyt/cpp/INIReader.h>

#include "command_sanity.hpp"

std::optional<CompCtx> command::build::init_compilation_context(const fs::path& path)
{
  // check the workspace sanity
  std::cout << "[velox] Welcome to the Velox toolchain !" << "\n  Version: " << VELOX_COMPILER_VERSION
            << "\n  Toolchain launched at: " << path << "\n"
            << "\n[velox] Checking workspace sanity..." << std::endl;
  fs::path config_path = path / "velox.config";
  if (!command::sanity::check_workspace_sanity(path)) {
    return std::nullopt;
  }

  // check config file sanity
  std::cout << "[velox] [config] checking velox.config sanity..." << std::endl;
  try {
    command::sanity::check_velox_config_sanity(config_path, true);
  } catch (const std::runtime_error& e) {
    std::cerr << e.what() << std::endl;
    return std::nullopt;
  }

  INIReader reader(config_path);
  auto      target_config = reader.GetString("target", "config", "self");
  if (target_config == "self") {
    return parse_compilation_context(config_path);
  } else {
    // velox.config set all defaults
    auto ctx = parse_compilation_context(config_path);

    auto sub_config_path = reader.GetString("sub_configs", target_config, "");
    if (sub_config_path.empty() && target_config != "self") {
      std::cerr << "[velox] [config] [error] The specified config " + target_config
                       + " is not defined in [sub_configs] section."
                << std::endl;
      return std::nullopt;
    }

    if (!fs::exists(sub_config_path)) {
      std::cerr << "[velox] [config] [error] The specified config " + target_config
                       + " defined in [sub_configs] section, the file located at \n"
                << "  " << sub_config_path << " doesn't exists." << std::endl;
      return std::nullopt;
    }

    try {
      command::sanity::check_velox_config_sanity(sub_config_path, false);
    } catch (const std::runtime_error& e) {
      std::cerr << e.what() << std::endl;
      return std::nullopt;
    }

    return parse_compilation_context(sub_config_path);
  }
}


CompCtx command::build::parse_compilation_context(const fs::path& config_path)
{
  CompCtx result;

  INIReader reader(config_path);

  result.target_abi    = reader.GetString("target", "abi", "LP64");
  result.target_arch   = reader.GetString("target", "arch", "");
  result.target_bits   = reader.GetInteger("target", "bits", 64);
  result.target_os     = reader.GetString("target", "os", "");
  result.target_libc   = reader.GetString("target", "libc", "");
  result.target_config = reader.GetString("target", "config", "self");

  result.profile_debug     = reader.GetBoolean("profile", "debug", false);
  result.profile_opt_level = reader.GetInteger("profile", "opt_level", 0);
  result.profile_size_opt  = reader.GetBoolean("profile", "size_opt", false);

  result.log_all          = reader.GetBoolean("logs", "all", false);
  result.log_filesystem   = reader.GetBoolean("logs", "filesystem", false);
  result.log_lexer        = reader.GetBoolean("logs", "lexer", false);
  result.log_preprocessor = reader.GetBoolean("logs", "preprocessor", false);
  result.log_parser       = reader.GetBoolean("logs", "parser", false);
  result.log_binder       = reader.GetBoolean("logs", "embinder", false);
  result.log_exporter     = reader.GetBoolean("logs", "exporter", false);
  result.log_resolver     = reader.GetBoolean("logs", "resolver", false);
  result.log_LLVM_IR      = reader.GetBoolean("logs", "llvm-ir", false);
  result.log_linker       = reader.GetBoolean("logs", "linker", false);

  for (auto& define : reader.Keys("defines")) {
    auto val               = reader.GetString("defines", define, "");
    result.defines[define] = val;
  }

  result.undefines = reader.Keys("undefines");

  auto emit_mode = reader.GetString("codegen", "emit_mode", "BIN");
  if (emit_mode == "LLVM") result.codegen_emit_mode = CompCtx::EEmitMode::LLVM;
  if (emit_mode == "OBJ") result.codegen_emit_mode = CompCtx::EEmitMode::OBJ;
  if (emit_mode == "ASM") result.codegen_emit_mode = CompCtx::EEmitMode::ASM;
  if (emit_mode == "BC") result.codegen_emit_mode = CompCtx::EEmitMode::BC;
  if (emit_mode == "BIN") result.codegen_emit_mode = CompCtx::EEmitMode::BIN;
  result.codegen_output_dir = reader.GetString("codegen", "output_dir", "./build");
  result.codegen_dest_file  = reader.GetString("codegen", "dest_dir", "./build/app");

  result.project_dir     = reader.GetString("project", "project_dir", "./");
  result.source_dir      = reader.GetString("project", "source_dir", "./src");
  result.thrid_party_dir = reader.GetString("project", "third_party_dir", "./thirdparty");

  for (auto& sub_config : reader.Keys("sub_configs")) {
    auto val                       = reader.GetString("sub_configs", sub_config, "");
    result.sub_configs[sub_config] = val;
  }

  return result;
}


void command::build::parse_args_for_compilation_context(CompCtx& ctx, int start_arg, int argc, const char* argv[])
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
        COMPILATION_ARGS.emplace(arg_name, "true");
      }
      if (!alt_arg_name.empty() && arg.rfind("-" + alt_arg_name, 0) == 0) {
        COMPILATION_ARGS.emplace(alt_arg_name, "true");
      }

      input = true;
      return true;
    };

    // e.g. --os="linux"
    auto str_arg = [&](std::string& input, const std::string& arg_name, const std::string& alt_arg_name = "") {
      if (arg_name.empty() && alt_arg_name.empty()) {
        return false;
      }

      if (!arg_name.empty() && arg.rfind("--" + arg_name + "=", 0) == 0) {
        COMPILATION_ARGS.emplace(arg_name, arg);
      }
      if (!alt_arg_name.empty() && arg.rfind("-" + alt_arg_name + "=", 0) == 0) {
        COMPILATION_ARGS.emplace(alt_arg_name, arg);
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
        COMPILATION_ARGS.emplace(arg_name, arg);
      }
      if (!alt_arg_name.empty() && arg.rfind("-" + alt_arg_name + "=", 0) == 0) {
        COMPILATION_ARGS.emplace(alt_arg_name, arg);
      }

      input = arg;
      return true;
    };

    // e.g. --opt-level=0
    auto size_arg = [&](size_t& input, const std::string& arg_name, const std::string& alt_arg_name = "") {
      if (arg_name.empty() && alt_arg_name.empty()) {
        return false;
      }

      if (!arg_name.empty() && arg.rfind("--" + arg_name + "=", 0) == 0) {
        COMPILATION_ARGS.emplace(arg_name, arg);
      }
      if (!alt_arg_name.empty() && arg.rfind("-" + alt_arg_name + "=", 0) == 0) {
        COMPILATION_ARGS.emplace(alt_arg_name, arg);
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
    if (bool_arg(ctx.log_binder, "log-embinder", "lemb")) continue;
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

    // dot
    if (bool_arg(ctx.dot_ast, "dot-ast")) continue;
    if (bool_arg(ctx.dot_link, "dot-link")) continue;

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
      COMPILATION_ARGS.emplace(name, value);
      ctx.defines.emplace(name, value);

      continue;
    }

    // undefine macro
    if (arg.rfind("-U", 0) == 0) {
      auto name = arg.substr(2);

      COMPILATION_ARGS.emplace(name, ""); // no value for -UName
      ctx.undefines.push_back(name);
      continue;
    }

    // codegen
    std::string emit_mode;
    if (str_arg(emit_mode, "emit")) {
      if (emit_mode == "obj")
        ctx.codegen_emit_mode = CompCtx::EEmitMode::OBJ;
      else if (emit_mode == "asm")
        ctx.codegen_emit_mode = CompCtx::EEmitMode::ASM;
      else if (emit_mode == "bc")
        ctx.codegen_emit_mode = CompCtx::EEmitMode::BC;
      else if (emit_mode == "bin")
        ctx.codegen_emit_mode = CompCtx::EEmitMode::BIN;
      else
        std::cerr << "Invalid emit mode value --emit-mode=" << emit_mode << std::endl;
      continue;
    }
    if (path_arg(ctx.codegen_output_dir, "output")) continue;
    if (path_arg(ctx.codegen_dest_file, "dest")) continue;

    // project
    if (path_arg(ctx.project_dir, "project")) continue;
    if (path_arg(ctx.source_dir, "src")) continue;
    if (path_arg(ctx.thrid_party_dir, "third-party")) continue;

    std::cerr << "Warning: unknown argument '" << arg << "'" << std::endl;
  }
}
