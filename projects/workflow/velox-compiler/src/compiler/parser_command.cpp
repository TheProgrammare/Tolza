#include "parser_command.hpp"

#include <iostream>
#include <string>
#include <filesystem>


#include <common/common.hpp>
#include <common/fileutils.hpp>
#include <common/compiler_options.hpp>

#include <CLIUtils/CLI11.hpp>
#include <Neargye/magic_enum.hpp>


#include "binder/ffi_c_reader.hpp"
#include "binder/ffi_json_reader.hpp"

// unused false positive
// mandatory
#include "CLI11_type_seralization.hpp"


#include "compiler/compiler.hpp"


namespace fs = std::filesystem;


#define OUT_LOG std::cout << "[ffi] "
#define OUT_ERR std::cerr << "[ffi:ERROR] "


void compiler::Commander::init_command_cogito() noexcept
{
  auto* cogito   = app.add_subcommand("velox-toolchain", "Revceive the toolchain cogito ask");
  auto* ergo_sum = cogito->add_subcommand("cogito", "Respond to René Descartes");
  ergo_sum->callback([&]() {
    std::cout << "[velox-compiler] ergo sum\n  " << common::env::get_exe_dir() << "\n"; /*endl*/
    exit(0);
  });
}

void compiler::Commander::init_command_build() noexcept
{
  auto* build = app.add_subcommand("build", "Compile your velox project");
  build->alias("b");


  auto add_opt_path = [&](std::string_view option_name, std::basic_string<char>& variable,
                          std::string_view option_description) {
    return build
        ->add_option_function<fs::path>(
            std::string(option_name),
            [&](const fs::path& path) { variable = common::fileutils::resolve_path(path.string()); },
            std::string(option_description))
        ->type_name("<path>");
  };


#define new_flag(flag_name, flag, desc) build->add_flag(flag_name, compiler::OPTIONS.flag, desc)->type_name("<flag>")
#define new_opt(opt_name, var, desc)    build->add_option(opt_name, compiler::OPTIONS.var, desc)->type_name("<type>")

  new_flag("--mute", mute, "Mute any output log");

  // base
  new_opt("--project-name", project_name, "Set the target project name");
  new_opt("--sub-config", sub_configs, "Set the sub config to apply after main config");

  // preset
  build
      ->add_option_function<std::string>(
          "--preset",
          [&](const std::string& s) {
            auto preset = magic_enum::enum_cast<common::compiler::EPlatformFlavor>(s, magic_enum::case_insensitive);
            if (preset) compiler::OPTIONS = common::compiler::Options::get_preset(preset.value());
          },
          "e.g. linux_glibc, linux_musl, apple_macos, apple_ios, windows_msvc, windows_mingw, bsd_generic, wasi, "
          "baremetal, custom")
      ->type_name("<platform>");

  // error stop mode
  new_opt("--error-mode", error_mode, "Mode of failure on error encounted during the compilation.");

  // target
  new_opt("--arch", target.arch,
          "e.g. x86_64, x86_32, arm64, arm32, ppc64, ppc32, mips64, mips32, wasm64, wasm32, sparc64, custom");
  new_opt("--platform", target.platform,
          "e.g. linux, macos, windows, freebsd, openbsd, netbsd, dragonflybsd, android, ios, solaris, custom");
  new_opt("--vendor", target.vendor, "e.g. apple, pc, w64");
  new_opt("--abi", target.abi,
          "e.g. sysv, win64, gnu, aapcs, aapcs64, darwin_arm64, msvc_x86, msvc_x64, riscv_ilp32, riscv_lp64, "
          "wasm64, wasm32, custom");
  new_opt("--cpu", target.cpu, "e.g. generic, core-avx2, znver4");
  new_opt("--features", target.features, "e.g. \"sse,sse2,sse3,ssse3,avx,avx2,avx512,neon,sve,rvc,rvv,custom\"");
  new_opt("--call-conv", target.calling_conv,
          "e.g. sysv, cdecl, stdcall, fastcall, thiscall, win64, aapcs, aapcs_vfp, vectorcall, custom");
  new_opt("--code-model", target.code_model, "e.g. tiny, small, kernel, medium, large");
  new_opt("--reloc-model", target.reloc_model, "e.g. pic, static, pie, ropi, rwpi, ropi_rwpi");

  // profile
  auto* opt_debug   = new_opt("--debug,-d", profile.debug, "Compile in debug mode");
  auto* opt_release = build->add_flag(
      "--release,-r", [&](std::size_t count) { compiler::OPTIONS.profile.debug = false; }, "Compile in release mode");
  opt_debug->excludes(opt_release);
  new_opt("--optimization,-O", profile.optimization, "Optimization level 0..3..s..z");


  // clang
  new_opt("--libc", clang.libc, "e.g. glibc, musl, libsystem, ucrt, msvcrt, mingw_libc, bionic, bsd_libc, custom");
  new_opt("--libc-version", clang.libc_version, "major.minor, e.g. 1.20")->type_name("<version>");


  new_opt("--logs", log.logs,
          "e.g. "
          "\"all,filesystem,lexer,preprocessor,parser,binder,exporter,resolver_symbol,resolver_type,"
          "resolver_semantic,codegen,optimization,emit,linker\"");

  new_opt("--warns", warn.warns, "e.g. \"all, extra, pedantic, unused, dead_code, as_error\"");
  new_opt("--warning,-W", warn.level, "Warn level 0..3");

  new_opt("--debugs", debug.debugs, "e.g. \"ast\"");

  build
      ->add_option_function<std::string>("-D",
                                         [&](std::string_view text) {
                                           auto pos = text.find('=');
                                           if (pos == std::string::npos) {
                                             compiler::OPTIONS.preprocessor.defines[std::string(text)] = "true";
                                           } else {
                                             compiler::OPTIONS.preprocessor.defines[std::string(text.substr(0, pos))] =
                                                 text.substr(pos + 1);
                                           }
                                         })
      ->type_name("<key>");

  build
      ->add_option_function<std::string>(
          "-U", [&](std::string_view text) { compiler::OPTIONS.preprocessor.undefines.emplace_back(text.data()); })
      ->type_name("<key>");

  new_opt("--emits", target.emits, "Code emission");

  add_opt_path("--dir-project", compiler::OPTIONS.dir.project, "Set the project directory");
  add_opt_path("--dir-build", compiler::OPTIONS.dir.build, "Set the build directory");
  add_opt_path("--dir-src", compiler::OPTIONS.dir.source, "Set the source code directory");
  add_opt_path("--dir-vendor", compiler::OPTIONS.dir.vendor, "Set the vendor source code directory");
  add_opt_path("--dir-ffi-json", compiler::OPTIONS.dir.ffi_json, "Set the vendor source code directory");
  add_opt_path("--dir-binding", compiler::OPTIONS.dir.binding, "Set the binding directory");
  add_opt_path("--dir-compiler", compiler::OPTIONS.dir.compiler, "Set the compiler directory");
  add_opt_path("--dir-stdlib", compiler::OPTIONS.dir.stdlib, "Set the stdlib directory");
  add_opt_path("--dir-packages", compiler::OPTIONS.dir.packages, "Set the packages directory");

  build->add_flag("--llvm-verify_module", compiler::OPTIONS.llvm.verify_module,
                  "Enable llvm verification before and after passes");

  build->allow_extras();

  build->callback([&]() {
    if (!compiler::OPTIONS.mute) {
      OUT_LOG "Command executed: \n";
      for (const auto& arg : app.remaining()) std::cout << arg << " ";
      std::cout << "\n";
    }

    compiler::COMPILER.run_requested = true;
  });

#undef new_flag
#undef new_opt
}

void compiler::Commander::init_commands() noexcept
{
  init_command_build();
  init_command_cogito();
}


void compiler::Commander::exec_ffi_command() noexcept
{
  to_path   = common::fileutils::resolve_path(to_path);
  from_path = common::fileutils::resolve_path(from_path);

  if (!fs::exists(from_path)) {
    OUT_ERR "The source path doesn't exists.\n";
    exit(1);
  }
  if (!fs::exists(to_path)) {
    OUT_ERR "The destination path doesn't exists.\n";
    exit(1);
  }

  if (ffi_json_flag) {
    auto     ast       = ffi::JSON_Reader::parse_json_compilation_unit(from_path);
    fs::path dest_file = fs::path(to_path) / ast->bind.lang / ast->bind.lib;
    dest_file.replace_extension(common::fileutils::VELOX_FILE_EXTENSION);

    ast->velox_codegen(dest_file.string());

    exit(0);
  } else if (ffi_c_flag) {
    auto     ast       = ffi::C_Reader::parse_c_compilation_unit(from_path);
    fs::path dest_file = fs::path(to_path) / ast->bind.lang / ast->bind.lib;
    dest_file.replace_extension(common::fileutils::VELOX_FILE_EXTENSION);

    ast->velox_codegen(dest_file.string());

    exit(0);
  } else {
    OUT_ERR "Unspecified ffi mode.\n";
    exit(1);
  }
}