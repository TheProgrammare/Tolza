#include "pipeline/linker.hpp"

#include "compiler/compilation_unit.hpp"
#include "compiler/compiler.hpp"
#include "compiler/file_info.hpp"
#include "compiler/io.hpp"
#include "id/cuid.hpp"
#include "pipeline/pipeline.hpp"

#include <cassert>
#include <common/compiler_options.hpp>
#include <common/environment.hpp>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <llvm-19/llvm/ADT/StringRef.h>
#include <llvm-19/llvm/IR/LLVMContext.h>
#include <llvm-19/llvm/IR/LegacyPassManager.h>
#include <llvm-19/llvm/IR/Module.h>
#include <llvm-19/llvm/IR/PassManager.h>
#include <llvm-19/llvm/IR/Verifier.h>
#include <llvm-19/llvm/Linker/Linker.h>
#include <llvm-19/llvm/MC/TargetRegistry.h>
#include <llvm-19/llvm/Passes/PassBuilder.h>
#include <llvm-19/llvm/Support/CodeGen.h>
#include <llvm-19/llvm/Support/FileSystem.h>
#include <llvm-19/llvm/Support/raw_ostream.h>
#include <llvm-19/llvm/Target/TargetMachine.h>
#include <llvm-19/llvm/TargetParser/Host.h>
#include <memory>
#include <print>
#include <string>
#include <system_error>
#include <utility>

namespace fs = std::filesystem;


bool linker::link_modules() noexcept
{
  const auto& CUs = PIPELINE.compilation_units;
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
  std::string extension = OPTIONS.target.triple.platform == common::env::EPlatform::_windows ? ".exe" : "";

  fs::create_directories(OPTIONS.get_dir_build_profile());

  fs::path target_o = fs::path(OPTIONS.get_dir_build_profile()) / OPTIONS.get_project_name();
  target_o.replace_extension(".o");
  fs::path out_bin = fs::path(OPTIONS.get_dir_build_profile()) / OPTIONS.get_project_name();
  out_bin.replace_extension(extension);

  auto* main_mod = cu::ID::main().get().llvm_module;

  std::string cmd = std::format(R"(clang "{}" -o "{}")", target_o.string(), out_bin.string());
  IO::println(IO_PASS::linker, "Clang linking command:\n  {}", cmd);

  // if (auto err_code = llvm::sys::ExecuteAndWait("ld.lld", {target_o.string(), "-lc", "-o", out_bin.string()});

  const std::string mode = (OPTIONS.profile.debug) ? "debug" : "release";

  if (auto err_code = std::system(cmd.c_str()); err_code != 0) {
    if (OPTIONS.diagnostic.out_format == common::compiler::EDiagnosticFormat::json) {
      std::println(stderr, R"(@@TOLZA_EXORDIUM_RESULTATI@@
{{"success":false,"executable":"","mode":"{}"}}
@@TOLZA_CLAUSULA_RESULTATI@@)",
                   mode);
    } else {
      IO::println(stderr, IO_PASS::linker, "Linker failed: system code error {}", err_code);
    }
    return false;
  }

  if (OPTIONS.diagnostic.out_format == common::compiler::EDiagnosticFormat::json) {
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
  fs::path        dest_path = fs::path(OPTIONS.get_dir_build_profile()) / OPTIONS.get_project_name();
  dest_path.replace_extension(".o");
  fs::create_directories(dest_path.parent_path());

  llvm::raw_fd_ostream dest(dest_path.string(), err_c, llvm::sys::fs::OF_None);

  if (err_c) {
    llvm::errs() << std::format("[emitter:ERROR] File error: {}\n", err_c.message());
    return false;
  }


  llvm::legacy::PassManager pass;

  if (TARGET_MACHINE->addPassesToEmitFile(pass, dest, nullptr, llvm::CodeGenFileType::ObjectFile)) {
    llvm::errs() << "[emitter:ERROR] TargetMachine doesn't support obj emit";
    return false;
  }

  pass.run(*mod);
  dest.flush();

  IO::println("Emit obj \"{}\"", dest_path.string());
  return true;
}