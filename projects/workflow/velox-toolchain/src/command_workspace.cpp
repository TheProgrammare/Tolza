#include "command_workspace.hpp"

#include <filesystem>
#include <iostream>
#include <fstream>
#include <ostream>

#include "cli_wrapper.hpp"
#include "common.hpp"
#include "compiler_context.hpp"
#include "toolchain/toolchain.hpp"
#include "toolchain_context.hpp"

namespace fs = std::filesystem;

void command::workspace::err(const std::string& msg)
{
  std::cerr << "[workspace:ERROR] " << msg << std::endl;
}

void command::workspace::log(const std::string& msg)
{
  std::cout << "[workspace] " << msg << std::endl;
}

std::string command::workspace::generate_velox_workspace(const std::string& project_name, const std::string& path,
                                                         bool force)
{
  fs::path project_path = fs::path(path) / project_name;

  if (!force
      && !cli::yes_no_question("Do you want to create a new velox projet named \"" + project_name + "\" at\n  \""
                               + project_path.string() + "\"?\n ")) {
    log("Velox workspace generation aborted...");
    return "";
  }

  if (fs::exists(fs::path(project_path / project_name))) {
    err("The file already exists.");
    log("Velox workspace generation aborted...");
    return "";
  }

  log("\"" + project_path.string() + "\"");

  bool success    = true;
  auto dir_create = [&](const std::string& _path) {
    try {
      fs::create_directory(_path);
    } catch (const fs::filesystem_error e) {
      err(e.what());
      return success = false;
    }
    log("\"" + _path + "\"");
    return true;
  };

  if (!dir_create(project_path)) success = false;
  if (!dir_create(project_path / "src")) success = false;
  if (!dir_create(project_path / "vendor")) success = false;
  if (!dir_create(project_path / "binding")) success = false;
  if (!dir_create(project_path / "binding" / "ffi_json")) success = false;
  if (!dir_create(project_path / "build")) success = false;
  if (!dir_create(project_path / "build" / "debug")) success = false;
  if (!dir_create(project_path / "build" / "release")) success = false;
  if (!dir_create(project_path / "config")) success = false;

  if (write_config_file(project_path, "velox.toml", false).empty()) success = false;
  if (write_config_file(project_path / "config", "velox_debug.toml", true).empty()) success = false;

  if (write_file(project_path / "src" / "main.vlx", toolchain::VELOX_MAIN_TEMPLATE).empty()) success = false;

  if (!success)
    err("An error has occured, workspace generation aborted...");
  else
    log("Workspace successfully generated!");

  return project_path;
}

std::string command::workspace::write_file(const std::string& path, const std::string& text, bool verbose)
{
  std::ofstream f;
  try {
    f = std::ofstream(path);
  } catch (const fs::filesystem_error e) {
    err(e.what());
    return "";
  }

  f << text;
  if (verbose) log("\"" + path + "\"");
  return path;
}

std::string command::workspace::write_config_file(const std::string& path, const std::string& name,
                                                  bool file_debug_mode)
{
  common::CompCtx ctx;
  ctx.target_project_name = name;
  ctx.target_arch         = common::DETECTED_ARCH;
  ctx.target_os           = common::DETECTED_OS;
  ctx.target_vendor       = common::DETECTED_VENDOR;
  ctx.target_abi          = common::DETECTED_ABI;
  auto config_txt         = compiler_context_to_config(ctx);
  return write_file(fs::path(path) / name, config_txt);
}


void command::workspace::ask_new_workspace(const std::string& ws_path, const std::string& name)
{
  std::string filename = name;
  if (cli::yes_no_question("Do you want to generate a Velox project in a new folder?")) {
  retry_project_name:
    if (filename.empty()) filename = cli::get_input("Write down your project name (file name only valid)");

    if (!cli::is_valid_filename(filename)) {
      log("Invalid project name \"" + filename + "\".");
      auto sanitize = cli::sanitize_filename(filename);

      if (!cli::yes_no_question("Do you want to use \"" + sanitize + "\" instead?")) {
        if (cli::yes_no_question("Do you want to retry?")) {
          filename.clear();
          goto retry_project_name;
        }

        log("Velox workspace generation aborted...");
        return;
      } else {
        generate_velox_workspace(sanitize, ws_path);
        return;
      }
    }

    generate_velox_workspace(filename, ws_path);
  } else {
    log("Velox workspace generation aborted...");
  }
}

std::string command::workspace::new_velox_workspace()
{
  log("Generation of Velox workspace... at \"" + fs::current_path().string() + "\"");

retry_project_name:
  auto filename = cli::get_input("write down your project name (file name only valid)");

  if (!cli::is_valid_filename(filename)) {
    log("Invalid project name \"" + filename + "\".");
    auto sanitize = cli::sanitize_filename(filename);

    if (!cli::yes_no_question("Do you want to use \"" + sanitize + "\" instead? [Y/n]")) {
      if (cli::yes_no_question("Do you want to retry? [Y/n]")) goto retry_project_name;

      log("Velox workspace generation aborted...");
      return "";
    } else {
      return generate_velox_workspace(sanitize, fs::current_path());
    }
  }

  return generate_velox_workspace(filename, fs::current_path());
}

std::string command::workspace::compiler_context_to_config(const common::CompCtx& ctx)
{
  auto btos = [](bool _in) -> std::string { return _in ? "true" : "false"; };

  std::string config_txt = toolchain::VELOX_CONFIG_TEMPLATE;

  std::string reloc_model = common::CompCtx::ERelocModel_to_str(ctx.target_reloc_model);
  std::string code_model  = common::CompCtx::ECodeModel_to_str(ctx.target_code_model);
  std::string opt         = common::CompCtx::EOptimization_to_str(ctx.profile_optimization);
  std::string wlevel      = std::to_string(static_cast<int>(ctx.warn_level));

  std::string defines;
  for (auto& [key, val] : ctx.defines) defines += key + " = \"" + val + "\"\n";
  std::string undefines;
  for (auto& val : ctx.undefines) undefines += "\"" + val + "\n";
  std::string sub_configs;
  for (auto& [key, val] : ctx.sub_configs) defines += key + " = \"" + val + "\"\n";
  std::string llvm_args;
  for (auto& val : ctx.llvm_args) llvm_args += "\"" + std::string(val) + "\",\n";
  std::string logs;
  logs.reserve(ctx.logs.size());
  for (auto& log : ctx.logs) logs += "\"" + std::string(log) + "\",\n";
  std::string warns;
  warns.reserve(ctx.warns.size());
  for (auto& warn : ctx.warns) warns += "\"" + std::string(warn) + "\",\n";
  std::string debugs;
  debugs.reserve(ctx.debugs.size());
  for (auto& debug : ctx.debugs) debugs += "\"" + std::string(debug) + "\",\n";
  std::string emits;
  emits.reserve(ctx.target_emits.empty() ? 1 : ctx.target_emits.size());
  for (auto& emit : ctx.target_emits) {
    switch (emit) {
    case common::CompCtx::EEmit::Bin:   emits += "\"bin\", ";
    case common::CompCtx::EEmit::LLVM:  emits += "\"llvm\", ";
    case common::CompCtx::EEmit::Obj:   emits += "\"obj\", ";
    case common::CompCtx::EEmit::ASM:   emits += "\"asm\", ";
    case common::CompCtx::EEmit::BC:    emits += "\"bc\", ";
    case common::CompCtx::EEmit::s_lib: emits += "\"s_lib\", ";
    case common::CompCtx::EEmit::d_lib: emits += "\"d_lib\", ";
    }
  }

  std::map<std::string, std::string> config_params = {
      // target
      {"target_project_name",  ctx.target_project_name       },
      {"target_arch",          ctx.target_arch               },
      {"target_os",            ctx.target_os                 },
      {"target_vendor",        ctx.target_vendor             },
      {"target_abi",           ctx.target_abi                },
      {"target_libc",          ctx.target_libc               },
      {"target_cpu",           ctx.target_cpu                },
      {"target_features",      ctx.target_features           },
      {"target_code_model",    code_model                    },
      {"target_reloc_model",   reloc_model                   },
      {"target_sub_config",    ctx.target_sub_config         },
      {"target_emit",          emits                         },
      // profile
      {"profile_debug",        btos(ctx.profile_debug)       },
      {"profile_optimization", opt                           },
      // logs
      {"logs",                 logs                          },
      // warnings
      {"warnings",             warns                         },
      {"warn_level",           wlevel                        },
      // debug printer
      {"debugs",               debugs                        },
      // defines
      {"defines",              defines                       },
      // undefines
      {"undefines",            undefines                     },
      // directories
      {"dir_project",          ctx.dir_project               },
      {"dir_build",            ctx.dir_build                 },
      {"dir_source",           ctx.dir_source                },
      {"dir_vendor",           ctx.dir_vendor                },
      {"dir_ffi_json",         ctx.dir_ffi_json              },
      {"dir_binding",          ctx.dir_binding               },
      {"dir_compiler",         common::TOOL_CTX.compiler_used},
      {"dir_stdlib",           common::get_stdlib_dir()      },
      {"dir_packages",         common::get_packages_dir()    },
      // sub_configs
      {"sub_configs",          sub_configs                   },
      // llvm
      {"llvm_triple",          ctx.llvm_triple               },
      {"llvm_verify_module",   btos(ctx.llvm_verify_module)  },
      {"llvm_args",            llvm_args                     },
  };

  common::fmt_template(config_txt, config_params);

  return config_txt;
}