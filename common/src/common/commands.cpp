#include "commands.hpp"

#include <iostream>

#include <CLIUtils/CLI11.hpp>

#include "common/common.hpp"
#include "common/environment.hpp"
#include "common/toolchain_options.hpp"
#include "fileutils.hpp"


void common::Commander::init_command_compiler() noexcept
{
  auto* compiler = app.add_subcommand("compiler", "Compiler manager commands");
  compiler->alias("c");

  {
    auto* find        = compiler->add_subcommand("find", "Search of velox-compiler");
    auto* opt_all     = find->add_flag("--all", compiler_all, "Find all velox-compiler installed");
    auto* opt_version = find->add_option("--version", compiler_version, "Find specific velox-compiler version");
    auto* opt_latest  = find->add_flag("--latest", compiler_latest, "Find latest velox-compiler version");
    auto* opt_path    = find->add_option("path", from_path,
                                         "If no path provided, will use standard path or custom path in toolchain.toml");

    opt_all->excludes(opt_version, opt_latest);
    opt_version->excludes(opt_all, opt_latest);
    opt_latest->excludes(opt_all, opt_version);

    find->callback([&]() {
      from_path = common::fileutils::resolve_path(from_path);

      auto found = [](std::string_view c) { std::cout << "[compiler] Found at \"" << c << "\"\n"; };

      if (!from_path.empty()) {
        if (compiler_all) {
          if (auto result = common::env::find_all_compilers(from_path); !result.empty()) {
            for (auto& elem : result) found(elem);
            return;
          }
        } else if (compiler_latest) {
          if (auto result = common::env::find_latest_compiler(from_path); !result.empty()) {
            found(result);
            return;
          }
        } else if (!compiler_version.empty()) {
          if (auto result = common::env::find_compiler_version(from_path, compiler_version); !result.empty()) {
            found(result);
            return;
          }
        }
        return;
      }

      for (auto& dir : common::env::get_compiler_dirs()) {
        if (compiler_all) {
          if (auto result = common::env::find_all_compilers(dir); !result.empty()) {
            for (auto& elem : result) found(elem);
            return;
          }
        } else if (compiler_latest) {
          if (auto result = common::env::find_latest_compiler(from_path); !result.empty()) {
            found(result);
            return;
          }
        } else if (!compiler_version.empty()) {
          if (auto result = common::env::find_compiler_version(from_path, compiler_version); !result.empty()) {
            found(result);
            return;
          }
        }
      }
      std::cout << "[compiler] No compiler found.\n";
    });
  }
  {
    auto* set         = compiler->add_subcommand("set", "Set the compiler used in velox.toml configuration");
    auto* opt_path    = set->add_option("path", from_path, "Set the path of the velox-compiler used");
    auto* opt_latest  = set->add_option("--latest", compiler_latest, "Set the latest velox-compiler used");
    auto* opt_version = set->add_option("--version,-v", compiler_version, "Set specific velox-compiler version used");
    auto* opt_custom_path = set->add_option("--custom-path", to_path, "Set the custom compiler directory to search");
    opt_path->excludes(opt_version, opt_latest);
    opt_version->excludes(opt_path, opt_latest, opt_custom_path);
    opt_latest->excludes(opt_path, opt_version, opt_custom_path);

    set->callback([&]() {
      from_path = common::fileutils::resolve_path(from_path);
      to_path   = common::fileutils::resolve_path(to_path);


      if (!from_path.empty()) {
        if (!to_path.empty()) {
          common::toolchain::OPTIONS.custom_compiler_dir = to_path;
          return;
        }
        common::toolchain::Options::apply_compiler(from_path);
        return;
      }

      for (auto& dir : common::env::get_compiler_dirs()) {
        if (compiler_latest) {
          auto result = common::env::find_latest_compiler(dir);
          if (!result.empty()) {
            common::toolchain::OPTIONS.compiler_used = result;
            return;
          }
        } else if (!compiler_version.empty()) {
          auto result = common::env::find_compiler_version(dir, compiler_version);
          if (!result.empty()) {
            common::toolchain::OPTIONS.compiler_used = result;
            return;
          }
        }
      }
    });
  }
#ifdef VELOX_TOOLCHAIN
  {
    auto* cogito = compiler->add_subcommand("cogito", "Check if the file is indeed the velox-compiler");
    cogito->add_option("path", from_path, "If no path provided, will check the compiler used in the velox.toml");
    cogito->callback([&]() {
      from_path = common::fileutils::resolve_path(from_path);
      toolchain::Options::cogito_compiler(
          from_path.empty() ? common::fileutils::resolve_path(common::toolchain::OPTIONS.compiler_used) : from_path);
    });
  }
#endif
}


void common::Commander::init_command_ffi() noexcept
{
  auto* ffi = app.add_subcommand("ffi", "Translate ffi representation to .vlxbind wrappers");
  ffi->add_option("--from,-f", from_path, "Source of the ffi representation")
      ->required()
      ->expected(1)
      ->type_name("<source>");
  ffi->add_option("--to,-t", to_path, "Directory destination of the .vlxbind generated")
      ->required()
      ->expected(1)
      ->type_name("<dest>");
  auto* flag_json = ffi->add_flag("--json,-j", ffi_json_flag, "JSON generation mode according to the source format");
  auto* flag_c    = ffi->add_flag("--c,-c", ffi_c_flag, "C language generation mode according to the source format");
  flag_json->excludes(flag_c);

  ffi->callback([&]() { exec_ffi_command(); });
}

void common::Commander::init_common_commands() noexcept
{
  app.set_version_flag("--version,-v", "Version: " + common::SOFTWARE_VERSION);

  app.add_flag_function(
      "--about,-a",
      [&](int count) {
        std::cout << common::SOFTWARE_ABOUT << "\n";
        exit(0);
      },
      "Show detailed software info");

  init_command_compiler();
  init_command_ffi();
}
