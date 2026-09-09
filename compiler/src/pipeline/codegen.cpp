#include "pipeline/codegen.hpp"

#include "ast/tool.hpp"
#include "codegen/codegen.hpp"
#include "compiler/compilation_unit.hpp"
#include "compiler/compiler.hpp"
#include "compiler/file_info.hpp"
#include "compiler/io.hpp"
#include "id/cuid.hpp"

#include <cassert>
#include <chrono>
#include <common/compiler_options.hpp>
#include <csignal>
#include <cstddef>
#include <filesystem>
#include <format>
#include <functional>
#include <llvm-19/llvm/ADT/StringRef.h>
#include <llvm-19/llvm/Analysis/CGSCCPassManager.h>
#include <llvm-19/llvm/Analysis/LoopAnalysisManager.h>
#include <llvm-19/llvm/IR/LLVMContext.h>
#include <llvm-19/llvm/IR/LegacyPassManager.h>
#include <llvm-19/llvm/IR/Module.h>
#include <llvm-19/llvm/IR/PassManager.h>
#include <llvm-19/llvm/IR/Verifier.h>
#include <llvm-19/llvm/Linker/Linker.h>
#include <llvm-19/llvm/MC/TargetRegistry.h>
#include <llvm-19/llvm/Passes/OptimizationLevel.h>
#include <llvm-19/llvm/Passes/PassBuilder.h>
#include <llvm-19/llvm/Support/CodeGen.h>
#include <llvm-19/llvm/Support/CommandLine.h>
#include <llvm-19/llvm/Support/FileSystem.h>
#include <llvm-19/llvm/Support/TargetSelect.h>
#include <llvm-19/llvm/Support/raw_ostream.h>
#include <llvm-19/llvm/Target/TargetMachine.h>
#include <llvm-19/llvm/Target/TargetOptions.h>
#include <llvm-19/llvm/TargetParser/Host.h>
#include <ratio>
#include <stdexcept>
#include <string>
#include <system_error>


namespace fs = std::filesystem;

inline double timing(const std::function<void()>& f) noexcept
{
  auto start = std::chrono::high_resolution_clock::now();

  f();

  auto end = std::chrono::high_resolution_clock::now();

  double milli = std::chrono::duration<double, std::milli>(end - start).count();

  return milli;
}


bool codegen::codegen_cu(cu::ID cuid) noexcept
{
  if (!generate_llvm_ir_cu(cuid)) return false;

  // llvm .ll file emission is before llvm optimization
  if (common::compiler::FEmit_has_flag(OPTIONS.target.emits, common::compiler::FEmit::llvm))
    (void)emit_llvm_ir_cu(cuid);

  if (!optimizing_cu(cuid)) return false;

  return true;
}


void codegen::generate_target_machine() noexcept
{
  // Init backends
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  std::string target_triple = OPTIONS.target.triple.dump();

  std::string         err;
  const llvm::Target* target = llvm::TargetRegistry::lookupTarget(target_triple, err);

  if (!target) {
    llvm::errs() << err;
    return;
  }

  llvm::Reloc::Model reloc;
  switch (OPTIONS.target.reloc_model) {
  case common::compiler::ERelocModel::STATIC:    reloc = llvm::Reloc::Static; break;
  case common::compiler::ERelocModel::NONE:
  case common::compiler::ERelocModel::PIC:       reloc = llvm::Reloc::PIC_; break;
  case common::compiler::ERelocModel::PIE:       reloc = llvm::Reloc::DynamicNoPIC; break;
  case common::compiler::ERelocModel::ROPI:      reloc = llvm::Reloc::ROPI; break;
  case common::compiler::ERelocModel::RWPI:      reloc = llvm::Reloc::RWPI; break;
  case common::compiler::ERelocModel::ROPI_RWPI: reloc = llvm::Reloc::ROPI_RWPI; break;
  }

  std::string features = common::compiler::FCPUFeature_to_str(OPTIONS.target.features);
  features             = features == "NONE" ? "" : features;

  // Init target machine
  llvm::TargetOptions opt;
  TARGET_MACHINE = target->createTargetMachine(target_triple, OPTIONS.target.cpu, features, opt, reloc);
}

bool codegen::generate_llvm_ir_cu(cu::ID cuid) noexcept
{
  static llvm::LLVMContext ctx = llvm::LLVMContext();

  static bool once = true;
  if (once) {
    once           = false;
    auto& cu       = cu::ID::main().get();
    cu.llvm_module = new llvm::Module(OPTIONS.get_project_name(), ctx);
  }

  auto& cu       = cuid.get();
  cu.llvm_module = new llvm::Module(cu.file_info.get_module_path(), ctx);


  if (OPTIONS.llvm.args.size() > 0) {
    llvm::cl::ParseCommandLineOptions(int(OPTIONS.llvm.args.size()), OPTIONS.llvm.args.data());
  }

  auto duration = timing([&]() {
    codegen::Codegen_AST cg(cu, ctx);
    (void)cg.start_codegen();
  });

  static size_t count = 1;
  IO::println("Codegen \"{}\" >> {:.2f} ms", cu.file_info.path, duration);

  count++;

  return true;
}
bool codegen::emit_llvm_ir_cu(cu::ID cuid) noexcept
{
  auto& cu = cuid.get();

  try {
    fs::create_directories(OPTIONS.get_dir_llvmir());
  } catch (const std::runtime_error& e) {
    IO::println(stderr, IO_PASS::emit, "Directory creation failed: {}", e.what());
    return false;
  }


  fs::path out_llvm_file(OPTIONS.get_dir_llvmir());
  fs::create_directories(out_llvm_file);
  out_llvm_file /= cu.file_info.get_file_name();
  out_llvm_file.replace_extension("ll");

  std::error_code EC;

  llvm::raw_fd_ostream out_f(out_llvm_file.string(), EC, llvm::sys::fs::OF_None);

  static size_t count = 1;
  if (EC) {
    llvm::errs() << std::format("[emit:llvm::ERROR] Cannot open the file: {}\n", EC.message());
    count++;
    return false;
  }

  cu.llvm_module->print(out_f, nullptr);

  // IO::println(IO_PASS::codegen, "IR emission at \"{}\"", out_llvm_file.string());
  count++;

  return true;
}

bool codegen::optimizing_cu(cu::ID cuid) noexcept
{
  llvm::Module* mod = cuid.get().llvm_module;
  assert(mod);

  bool success = false;

  auto duration = timing([&]() {
    // Init target
    std::string target_triple = OPTIONS.target.triple.dump();
    mod->setTargetTriple(target_triple);

    mod->setDataLayout(TARGET_MACHINE->createDataLayout());

    if (!mod) {
      IO::println(stderr, IO_PASS::optimization, "Module is nullptr!");
      success = false;
      return;
    }

    std::string verif_errs;
    {
      llvm::raw_string_ostream os(verif_errs);
      llvm::verifyModule(*mod, &os);
    } // auto flush

    if (!verif_errs.empty()) {
      llvm::errs() << std::format("[tolza:opti:ERROR] Module verification failed\n  {}\n", verif_errs);
      success = false;
      return;
    }

    llvm::LoopAnalysisManager     LAM;
    llvm::FunctionAnalysisManager FAM;
    llvm::CGSCCAnalysisManager    CGAM;
    llvm::ModuleAnalysisManager   MAM;

    // Enregistre les analyses
    llvm::PassBuilder PB(TARGET_MACHINE);
    PB.registerModuleAnalyses(MAM);
    PB.registerFunctionAnalyses(FAM);
    PB.registerLoopAnalyses(LAM);
    PB.registerCGSCCAnalyses(CGAM);
    PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

    llvm::OptimizationLevel opt_level = llvm::OptimizationLevel::O0;

    switch (OPTIONS.profile.optimization) {
    case common::compiler::EOptimization::O0:   opt_level = llvm::OptimizationLevel::O0; break;
    case common::compiler::EOptimization::O1:   opt_level = llvm::OptimizationLevel::O1; break;
    case common::compiler::EOptimization::O2:   opt_level = llvm::OptimizationLevel::O2; break;
    case common::compiler::EOptimization::O3:   opt_level = llvm::OptimizationLevel::O3; break;
    case common::compiler::EOptimization::Os:   opt_level = llvm::OptimizationLevel::Os; break;
    case common::compiler::EOptimization::Oz:   opt_level = llvm::OptimizationLevel::Oz; break;
    case common::compiler::EOptimization::NONE: opt_level = llvm::OptimizationLevel::O0; break;
    }

    if (mod->empty()) {
      IO::println(IO_PASS::optimization, "Module is empty, stop generation");
      success = true;
      return;
    }

    llvm::ModulePassManager MPM = PB.buildPerModuleDefaultPipeline(opt_level);
    MPM.addPass(llvm::VerifierPass());
    MPM.run(*mod, MAM);

    success = true;
    return;
  });

  // IO::println(IO_PASS::optimization, "Summary {:.2f} ms", duration);

  return success;
}


void codegen::emit_other(cu::ID cuid) noexcept
{
}
