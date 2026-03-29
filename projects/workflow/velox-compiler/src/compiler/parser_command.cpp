#include "compiler/parser_command.hpp"

#include <iostream>
#include <string>
#include <filesystem>

#include "binder/ffi-json_reader.hpp"
#include "common.hpp"
#include "compiler/compiler.hpp"
#include "compiler_context.hpp"

#include <CLIUtils/CLI11.hpp>


namespace fs = std::filesystem;


void Command::init_command_cogito()
{
  auto cogito   = app.add_subcommand("velox-toolchain", "Revceive the toolchain cogito ask");
  auto ergo_sum = cogito->add_subcommand("cogito", "Respond to René Descartes");
  ergo_sum->callback([&]() {
    std::cout << "[velox-compiler] ergo sum\n  " << common::get_exe_dir() << std::endl;
    exit(0);
  });
}

void Command::init_command_ffi_json()
{
  auto ffi_json = app.add_subcommand("generate-ffi-json", "Translate .json ast ffi to .vlxbind binder wrapper");
  ffi_json->alias("gen-ffi");
  ffi_json->add_option("source", dir_source, "Source file of the .json to convert")->required()->expected(1);
  ffi_json->add_option("dest", dir_dest, "Destination directory of the .vlxbind wrapper generated")
      ->required()
      ->expected(1);
  ffi_json->callback([&]() {
    dir_dest   = common::resolve_path(dir_dest);
    dir_source = common::resolve_path(dir_source);

    if (!fs::exists(dir_source)) {
      std::cerr << "[gen-ffi:ERROR] The source path dosen't exists." << std::endl;
      exit(1);
    }
    if (!fs::exists(dir_dest)) {
      std::cerr << "[gen-ffi:ERROR] The destination path dosen't exists." << std::endl;
      exit(1);
    }

    if (fs::is_regular_file(dir_source)) {
      auto     ast       = ffi::JSON::read_ffi_json_file(dir_source);
      fs::path dest_file = fs::path(dir_dest) / ast.bind.lang / std::string(ast.bind.lib + ".vlxbind");
      ffi::write_ast(ast, dest_file);
      exit(0);
    } else {
      std::cerr << "[gen-ffi:ERROR] The source path must be a file." << std::endl;
      exit(1);
    }
  });
}

void Command::init_command_build(int argc, const char* argv[])
{
  auto build = app.add_subcommand("build", "Compile your velox project");
  build->alias("b");
  common::CompCtx& ctx = compiler::COMP_CTX;
  ctx.argc             = argc;
  ctx.argv             = argv;

  auto insert_set = [&](std::set<std::string>& set, const std::string& flag_option, std::string set_elem_key,
                        const std::string& description) {
    return build
        ->add_flag_function(
            flag_option, [&, set_elem_key](std::size_t count) { set.insert(set_elem_key); }, description)
        ->expected(1);
  };

  auto add_opt_path = [&](const std::string& option_name, std::basic_string<char>& variable,
                          const std::string& option_description) {
    return build
        ->add_option_function<fs::path>(
            option_name, [&](const fs::path& path) { variable = common::resolve_path(path); }, option_description)
        ->expected(1)
        ->type_name("<path>");
  };

  auto opt_path = build
                      ->add_option_function<fs::path>(
                          "path", [&](const fs::path path) { ctx.current_config_file = common::resolve_path(path); },
                          "Path to the velox.toml config file")
                      ->expected(1)
                      ->required()
                      ->type_name("<path>");

  // target
  build->add_option("--project-name", ctx.target_project_name, "Set the target project name")
      ->expected(1)
      ->type_name("<info>");
  build->add_option("--arch", ctx.target_arch, "e.g. x86_64, aarch64, wasm32")->expected(1)->type_name("<info>");
  build->add_option("--os", ctx.target_os, "e.g. linux, windows, macos")->expected(1)->type_name("<info>");
  build->add_option("--vendor", ctx.target_vendor, "e.g. pc, apple, ...")->expected(1)->type_name("<info>");
  build->add_option("--abi", ctx.target_abi, "e.g. gnu, msvc")->expected(1)->type_name("<info>");
  build->add_option("--abi-size", ctx.target_size_abi, "e.g. LP64, ILP32, LLP64")->expected(1)->type_name("<info>");
  build->add_option("--libc", ctx.target_libc, "e.g. glibc, musl, bionic, msvc")->expected(1)->type_name("<info>");
  build->add_option("--cpu", ctx.target_cpu, "e.g. generic, core-avx2, znver4")->expected(1)->type_name("<info>");
  build->add_option("--features", ctx.target_features, "e.g. \"+avx2,+bmi2,-sse2\"")->expected(1)->type_name("<info>");
  build->add_option("--sub-config", ctx.sub_configs, "Set the sub config to apply after main config")
      ->expected(1)
      ->type_name("<info>");

  // debug
  auto opt_debug = build->add_flag("--debug,-d", ctx.profile_debug, "Compile in debug mode")->expected(1);
  auto opt_release =
      build
          ->add_flag_function(
              "--release,-r", [&](std::size_t count) { ctx.profile_debug = false; }, "Compile in release mode")
          ->expected(1);
  opt_debug->excludes(opt_release);


  build
      ->add_option_function<char>(
          "-O",
          [&](char opt) {
            switch (opt) {
            case '0': ctx.profile_optimization = common::CompCtx::EOptimization::O0; return;
            case '1': ctx.profile_optimization = common::CompCtx::EOptimization::O1; return;
            case '2': ctx.profile_optimization = common::CompCtx::EOptimization::O2; return;
            case '3': ctx.profile_optimization = common::CompCtx::EOptimization::O3; return;
            case 's': ctx.profile_optimization = common::CompCtx::EOptimization::Os; return;
            case 'z': ctx.profile_optimization = common::CompCtx::EOptimization::Oz; return;
            }
          },
          "Set optimization level or mode")
      ->expected(1)
      ->type_name("0..3sz");

  auto opt_l_all               = insert_set(ctx.logs, "--log-all", "all", "Log all passes");
  auto opt_l_filesystem        = insert_set(ctx.logs, "--log-filesystem", "filesystem", "");
  auto opt_l_lexer             = insert_set(ctx.logs, "--log-lexer", "lexer", "");
  auto opt_l_preprocessor      = insert_set(ctx.logs, "--log-preprocessor", "preprocessor", "");
  auto opt_l_parser            = insert_set(ctx.logs, "--log-parser", "parser", "");
  auto opt_l_binder            = insert_set(ctx.logs, "--log-binder", "binder", "");
  auto opt_l_exporter          = insert_set(ctx.logs, "--log-exporter", "exporter", "");
  auto opt_l_resolver_symbol   = insert_set(ctx.logs, "--log-resolver_symbol", "resolver_symbol", "");
  auto opt_l_resolver_type     = insert_set(ctx.logs, "--log-resolver_type", "resolver_type", "");
  auto opt_l_resolver_semantic = insert_set(ctx.logs, "--log-resolver_semantic", "resolver_semantic", "");
  auto opt_l_codegen           = insert_set(ctx.logs, "--log-codegen", "codegen", "");
  auto opt_l_optimization      = insert_set(ctx.logs, "--log-optimization", "optimization", "");
  auto opt_l_emit              = insert_set(ctx.logs, "--log-emit", "emit", "");
  auto opt_l_linker            = insert_set(ctx.logs, "--log-linker", "linker", "");
  opt_l_all->excludes(opt_l_filesystem, opt_l_lexer, opt_l_preprocessor, opt_l_parser, opt_l_binder, opt_l_exporter,
                      opt_l_resolver_symbol, opt_l_resolver_type, opt_l_resolver_semantic, opt_l_codegen,
                      opt_l_optimization, opt_l_emit, opt_l_linker);

  auto opt_w_all      = insert_set(ctx.warns, "--warn-all", "all", "Warn all cases (override all)");
  auto opt_w_extra    = insert_set(ctx.warns, "--warn-extra", "extra", "Warn more specific cases");
  auto opt_w_pedantic = insert_set(ctx.warns, "--warn-pedantic", "pedantic", "Warn standard derivation");
  auto opt_w_unused   = insert_set(ctx.warns, "--warn-unused", "unused", "Warn unused var, fn...");
  auto opt_w_dead     = insert_set(ctx.warns, "--warn-dead-code", "dead_code", "Warn dead code, never used...");
  auto opt_w_as       = insert_set(ctx.warns, "--warn-as-error", "as_error", "All warnings treated as errors");
  opt_w_all->excludes(opt_w_extra, opt_w_pedantic, opt_w_unused, opt_w_dead, opt_w_as);

  build
      ->add_option_function<int>(
          "-W",
          [&](int level) {
            if (level < 0 && level > -1) ctx.warn_level = static_cast<common::CompCtx::EWarnLevel>(level);
          },
          "Warn level")
      ->type_name("0..3")
      ->expected(1);

  insert_set(ctx.debugs, "--debug-ast", "ast", "Generate a file view of scripts ast");

  build
      ->add_option_function<std::string>("-D",
                                         [&ctx](const std::string& text) {
                                           auto pos = text.find('=');
                                           if (pos == std::string::npos) {
                                             ctx.defines[text] = "true";
                                           } else {
                                             ctx.defines[text.substr(0, pos)] = text.substr(pos + 1);
                                           }
                                         })
      ->type_name("<key>");

  build->add_option_function<std::string>("-U", [&ctx](const std::string& text) { ctx.undefines.push_back(text); })
      ->type_name("<key>");

  build->add_flag_function("--emit-obj", [&ctx](int count) { ctx.target_emits.insert(common::CompCtx::EEmit::Obj); })
      ->expected(1);
  build->add_flag_function("--emit-asm", [&ctx](int count) { ctx.target_emits.insert(common::CompCtx::EEmit::ASM); })
      ->expected(1);
  build->add_flag_function("--emit-bc", [&ctx](int count) { ctx.target_emits.insert(common::CompCtx::EEmit::BC); })
      ->expected(1);
  build->add_flag_function("--emit-bin", [&ctx](int count) { ctx.target_emits.insert(common::CompCtx::EEmit::Bin); })
      ->expected(1);
  build->add_flag_function("--emit-llvm", [&ctx](int count) { ctx.target_emits.insert(common::CompCtx::EEmit::LLVM); })
      ->expected(1);
  build
      ->add_flag_function("--emit-s-lib", [&ctx](int count) { ctx.target_emits.insert(common::CompCtx::EEmit::s_lib); })
      ->expected(1);
  build
      ->add_flag_function("--emit-d-lib", [&ctx](int count) { ctx.target_emits.insert(common::CompCtx::EEmit::d_lib); })
      ->expected(1);

  add_opt_path("--dir-project", ctx.dir_project, "Set the project directory");
  add_opt_path("--dir-build", ctx.dir_build, "Set the build directory");
  add_opt_path("--dir-src", ctx.dir_source, "Set the source code directory");
  add_opt_path("--dir-vendor", ctx.dir_vendor, "Set the vendor source code directory");
  add_opt_path("--dir-ffi-json", ctx.dir_ffi_json, "Set the vendor source code directory");
  add_opt_path("--dir-binding", ctx.dir_binding, "Set the binding directory");

  build->add_option("--triple", ctx.llvm_triple, "Override profile info for the llvm triple")
      ->expected(1)
      ->type_name("<arch-vendor-sys-abi>");
  build->add_flag("--verify", ctx.llvm_verify_module, "Enable llvm verification before and after passes")->expected(1);

  std::vector<std::string> llvm_flags;
  build->add_option("LLVM_FLAGS", llvm_flags, "Pass-through flags to LLVM")
      ->type_name("LLVM FLAGS")
      ->allow_extra_args()
      ->take_all();

  build->callback([&]() {
    if (compiler::COMP.start_compilation())
      exit(0);
    else
      exit(1);
  });
}

void Command::init_commands(int argc, const char* argv[])
{
  app.set_version_flag("--version,-v", "Version: " + common::SOFTWARE_VERSION);
  app.add_flag_function(
      "--about,-a",
      [&](std::size_t count) {
        std::cout << common::SOFTWARE_ABOUT << std::endl;
        exit(0);
      },
      "Show detailed software info");

  init_command_cogito();
  init_command_build(argc, argv);
  init_command_ffi_json();
}
