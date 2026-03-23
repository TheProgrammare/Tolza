#include "pipeline_llvm_opti.hpp"

#include <iostream>

#include <llvm/IR/Verifier.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/IR/Module.h>

#include <compiler_context.hpp>

#include "compiler/compiler.hpp"
#include "misc/script_info.hpp"

#include "compiler/compiler.hpp"

bool pipeline_start_llvm_opti(const std::vector<std::shared_ptr<ScriptInfo>>& scr_infos)
{
  if (scr_infos.empty()) return true;

  auto start = std::chrono::high_resolution_clock::now();

  llvm::Module* mod = scr_infos[0]->llvm_module;

  if (!mod) {
    std::cerr << "[llvm-opti:ERROR] module is nullptr!" << std::endl;
    return false;
  }

  if (llvm::verifyModule(*mod, &llvm::errs())) {
    llvm::errs() << "[llvm-opti:ERROR] Module verification failed";
    return false;
  }


  llvm::LoopAnalysisManager     LAM;
  llvm::FunctionAnalysisManager FAM;
  llvm::CGSCCAnalysisManager    CGAM;
  llvm::ModuleAnalysisManager   MAM;

  // Enregistre les analyses
  llvm::PassBuilder PB;
  PB.registerModuleAnalyses(MAM);
  PB.registerFunctionAnalyses(FAM);
  PB.registerLoopAnalyses(LAM);
  PB.registerCGSCCAnalyses(CGAM);

  llvm::OptimizationLevel opt = llvm::OptimizationLevel::O0;

  if (!compiler::COMP_CTX.profile_debug) {
    if (compiler::COMP_CTX.profile_extrem_size_opt)
      opt = llvm::OptimizationLevel::Oz;
    else if (compiler::COMP_CTX.profile_size_opt)
      opt = llvm::OptimizationLevel::Os;
    else if (compiler::COMP_CTX.profile_opt_level == 1)
      opt = llvm::OptimizationLevel::O1;
    else if (compiler::COMP_CTX.profile_opt_level == 2)
      opt = llvm::OptimizationLevel::O2;
    else if (compiler::COMP_CTX.profile_opt_level == 3)
      opt = llvm::OptimizationLevel::O2;
  }

  if (mod->empty()) {
    std::cout << "[llvm-opti] Module is empty, stop generation";
    return true;
  }

  llvm::ModulePassManager MPM = PB.buildPerModuleDefaultPipeline(opt);
  // MPM.run(*mod, MAM);

  if (llvm::verifyModule(*mod, &llvm::errs())) {
    llvm::errs() << "[llvm-opti:ERROR] Module verification failed!\n";
    return false;
  }

  auto   end            = std::chrono::high_resolution_clock::now();
  auto   final_duration = end - start;
  double milli          = std::chrono::duration<double, std::milli>(final_duration).count();

  std::cout << color_YELLOW "[llvm-opti:summary] " color_RESET "duration: " color_YELLOW << milli << " ms\n"
            << std::endl;

  return true;
}