#include "pipeline_llvm_opti.hpp"

#include <iostream>

#include <llvm/Target/TargetMachine.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/IR/Module.h>

#include <compiler_context.hpp>

#include "compiler/compiler.hpp"
#include "misc/script_info.hpp"

#include "compiler/compiler.hpp"

bool pipeline_start_llvm_opti(const std::vector<std::shared_ptr<ScriptInfo>>& scr_infos)
{
  static bool log = compiler::COMP_CTX.logs.contains("optimization");

  if (scr_infos.empty()) return true;

  auto start = std::chrono::high_resolution_clock::now();

  llvm::Module* mod = scr_infos[0]->llvm_module;


  // Init backends
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  // Init target
  std::string target_triple = compiler::COMP_CTX.get_target_triple();
  mod->setTargetTriple(target_triple);

  std::string         err;
  const llvm::Target* target = llvm::TargetRegistry::lookupTarget(target_triple, err);

  if (!target) {
    llvm::errs() << err;
    return false;
  }

  llvm::Reloc::Model reloc;
  switch (compiler::COMP_CTX.target_reloc_model) {
  case common::CompCtx::ERelocModel::Static:    reloc = llvm::Reloc::Static; break;
  case common::CompCtx::ERelocModel::PIC:       reloc = llvm::Reloc::PIC_; break;
  case common::CompCtx::ERelocModel::PIE:       reloc = llvm::Reloc::DynamicNoPIC; break;
  case common::CompCtx::ERelocModel::ROPI:      reloc = llvm::Reloc::ROPI; break;
  case common::CompCtx::ERelocModel::RWPI:      reloc = llvm::Reloc::RWPI; break;
  case common::CompCtx::ERelocModel::ROPI_RWPI: reloc = llvm::Reloc::ROPI_RWPI; break;
  }

  // Init target machine
  llvm::TargetOptions opt;
  compiler::TM = target->createTargetMachine(target_triple, compiler::COMP_CTX.target_cpu,
                                             compiler::COMP_CTX.target_features, opt, reloc);

  mod->setDataLayout(compiler::TM->createDataLayout());

  if (!mod) {
    std::cerr << "[llvm-opti:ERROR] module is nullptr!" << std::endl;
    return false;
  }

  if (compiler::COMP_CTX.llvm_verify_module && llvm::verifyModule(*mod, &llvm::errs())) {
    llvm::errs() << "[llvm-opti:ERROR] Module verification failed";
    return false;
  }


  llvm::LoopAnalysisManager     LAM;
  llvm::FunctionAnalysisManager FAM;
  llvm::CGSCCAnalysisManager    CGAM;
  llvm::ModuleAnalysisManager   MAM;

  // Enregistre les analyses
  llvm::PassBuilder PB(compiler::TM);
  PB.registerModuleAnalyses(MAM);
  PB.registerFunctionAnalyses(FAM);
  PB.registerLoopAnalyses(LAM);
  PB.registerCGSCCAnalyses(CGAM);
  PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

  llvm::OptimizationLevel opt_level = llvm::OptimizationLevel::O0;

  switch (compiler::COMP_CTX.profile_optimization) {
  case common::CompCtx::EOptimization::O0: opt_level = llvm::OptimizationLevel::O0; break;
  case common::CompCtx::EOptimization::O1: opt_level = llvm::OptimizationLevel::O1; break;
  case common::CompCtx::EOptimization::O2: opt_level = llvm::OptimizationLevel::O2; break;
  case common::CompCtx::EOptimization::O3: opt_level = llvm::OptimizationLevel::O3; break;
  case common::CompCtx::EOptimization::Os: opt_level = llvm::OptimizationLevel::Os; break;
  case common::CompCtx::EOptimization::Oz: opt_level = llvm::OptimizationLevel::Oz; break;
  }

  if (mod->empty()) {
    std::cout << "[llvm-opti] Module is empty, stop generation";
    return true;
  }

  llvm::ModulePassManager MPM = PB.buildPerModuleDefaultPipeline(opt_level);
  MPM.addPass(llvm::VerifierPass());
  MPM.run(*mod, MAM);

  if (llvm::verifyModule(*mod, &llvm::errs())) {
    llvm::errs() << "[llvm-opti:ERROR] Module verification failed!\n";
    return false;
  }

  auto   end            = std::chrono::high_resolution_clock::now();
  auto   final_duration = end - start;
  double milli          = std::chrono::duration<double, std::milli>(final_duration).count();

  if (log)
    std::cout << color_YELLOW "[llvm-opti:summary] " color_RESET "duration: " color_YELLOW << milli << " ms\n"
              << std::endl;

  return true;
}