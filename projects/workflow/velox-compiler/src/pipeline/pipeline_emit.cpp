#include "pipeline_emit.hpp"

#include <filesystem>
#include <iostream>
#include <llvm/IR/Module.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/TargetParser/Host.h>

#include <compiler_context.hpp>

#include "compiler/compiler.hpp"

#include "misc/script_info.hpp"

bool pipeline_start_emit(const std::vector<std::shared_ptr<ScriptInfo>>& scr_infos)
{
  if (scr_infos.empty()) return true;

  auto& mod = scr_infos[0]->llvm_module;

  // Init backends
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  // Init target
  std::string target_triple = llvm::sys::getDefaultTargetTriple();
  mod->setTargetTriple(target_triple);

  std::string         err;
  const llvm::Target* target = llvm::TargetRegistry::lookupTarget(target_triple, err);

  if (!target) {
    llvm::errs() << err;
    return false;
  }


  // Init target machine
  llvm::TargetOptions  opt;
  auto                 RM = std::optional<llvm::Reloc::Model>(llvm::Reloc::PIC_);
  llvm::TargetMachine* TM = target->createTargetMachine(target_triple, "generic", "", opt, RM);

  mod->setDataLayout(TM->createDataLayout());

  // Emit object
  if (compiler::COMP_CTX.emit_obj) {
    std::error_code       err_c;
    std::filesystem::path dest_path =
        std::filesystem::path(compiler::COMP_CTX.codegen_build_dir) / compiler::COMP_CTX.get_project_name();
    dest_path.replace_extension(".o");

    llvm::raw_fd_ostream dest(dest_path.string(), err_c, llvm::sys::fs::OF_None);

    if (err_c) {
      llvm::errs() << "[emitter:ERROR] File error : " << err_c.message();
      return false;
    }

    llvm::legacy::PassManager pass;

    if (TM->addPassesToEmitFile(pass, dest, nullptr, llvm::CodeGenFileType::ObjectFile)) {
      llvm::errs() << "[emitter:ERROR] TargetMachine dosen't support obj emit";
      return false;
    }

    pass.run(*mod);
    dest.flush();

    std::cout << "[emitter] Object emitted at " << dest_path << std::endl;
  }

  return true;
}
