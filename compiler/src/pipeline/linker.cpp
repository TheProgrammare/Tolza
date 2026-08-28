#include "pipeline/linker.hpp"

#include "compiler/compilation_unit.hpp"
#include "compiler/io.hpp"
#include "pipeline/pipeline.hpp"

#include <common/compiler_options.hpp>
#include <filesystem>
#include <print>


// LLVM core
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Verifier.h>

// Passes / pipeline
#include <llvm/Passes/OptimizationLevel.h>
#include <llvm/Passes/PassBuilder.h>

// Target / codegen
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Host.h>

// Backend init
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Program.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>

// Analysis managers
#include <llvm/Analysis/LoopAnalysisManager.h>

// Linking / IR tools
#include <llvm/Linker/Linker.h>

namespace fs = std::filesystem;


bool linker::link_modules() noexcept
{
  const auto& CUs = compiler::pipeline.compilation_units;
  if (CUs.empty()) return true;

  auto* main_mod = cu::ID::main().get().llvm_module;
  assert(main_mod);

  if (!main_mod) {
    IO::println(stderr, IO_PASS::linker, "Expected script file named 'main' to start the linking.");
    return false;
  }

  llvm::Linker linker(*main_mod);

  llvm::Linker::Flags flag = llvm::Linker::Flags::None;

  bool first_linked = true;
  bool failed       = false;
  for (size_t i = 1; i < CUs.size(); ++i) {
    auto& cu = cu::ID::make(i).get();
    if (!cu.llvm_module) continue; // safe

    IO::println("Link \"{}\"", cu.file_info.get_module_name());

    auto module_to_link = std::unique_ptr<llvm::Module>(cu.llvm_module);
    if (llvm::Linker::linkModules(*main_mod, std::move(module_to_link))) {
      IO::println(stderr, IO_PASS::linker, "Link failed on script \"{}\"", cu.file_info.path);
      failed = true;
    }
  }

  return !failed;
}

bool linker::link_executable() noexcept
{
  std::string extension = compiler::OPTIONS.target.triple.platform == common::env::EPlatform::windows ? ".exe" : "";

  fs::create_directories(compiler::OPTIONS.get_dir_build_profile());

  fs::path target_o = fs::path(compiler::OPTIONS.get_dir_build_profile()) / compiler::OPTIONS.get_project_name();
  target_o.replace_extension(".o");
  fs::path out_bin = fs::path(compiler::OPTIONS.get_dir_build_profile()) / compiler::OPTIONS.get_project_name();
  out_bin.replace_extension(extension);

  auto* main_mod = cu::ID::main().get().llvm_module;

  std::string cmd = std::format(R"(clang "{}" -o "{}")", target_o.string(), out_bin.string());
  IO::println(IO_PASS::linker, "Clang linking command:\n  {}", cmd);

  // if (auto err_code = llvm::sys::ExecuteAndWait("ld.lld", {target_o.string(), "-lc", "-o", out_bin.string()});

  const std::string mode = (compiler::OPTIONS.profile.debug) ? "debug" : "release";

  if (auto err_code = std::system(cmd.c_str()); err_code != 0) {
    if (compiler::OPTIONS.diagnostic.out_format == common::compiler::EDiagnosticFormat::json) {
      std::println(stderr, R"(@@TOLZA_EXORDIUM_RESULTATI@@
{{"success":false,"executable":"","mode":"{}"}}
@@TOLZA_CLAUSULA_RESULTATI@@)",
                   mode);
    } else {
      IO::println(stderr, IO_PASS::linker, "Linker failed: system code error {}", err_code);
    }
    return false;
  }

  if (compiler::OPTIONS.diagnostic.out_format == common::compiler::EDiagnosticFormat::json) {
    std::println(R"(@@TOLZA_EXORDIUM_RESULTATI@@
{{"success":true,"executable":"{}","mode":"{}"}}
@@TOLZA_CLAUSULA_RESULTATI@@)",
                 out_bin.string(), mode);
  } else {
    IO::println("Executable created at \"{}\"", out_bin.string());
  }
  return true;
}

bool linker::emit() noexcept
{
  auto* mod = cu::ID::main().get().llvm_module;
  assert(mod);


  std::error_code err_c;
  fs::path dest_path = fs::path(compiler::OPTIONS.get_dir_build_profile()) / compiler::OPTIONS.get_project_name();
  dest_path.replace_extension(".o");
  fs::create_directories(dest_path.parent_path());

  llvm::raw_fd_ostream dest(dest_path.string(), err_c, llvm::sys::fs::OF_None);

  if (err_c) {
    llvm::errs() << std::format("[emitter:ERROR] File error: {}\n", err_c.message());
    return false;
  }


  llvm::legacy::PassManager pass;

  if (compiler::TM->addPassesToEmitFile(pass, dest, nullptr, llvm::CodeGenFileType::ObjectFile)) {
    llvm::errs() << "[emitter:ERROR] TargetMachine doesn't support obj emit";
    return false;
  }

  pass.run(*mod);
  dest.flush();

  IO::println("Emit obj \"{}\"", dest_path.string());
  return true;
}