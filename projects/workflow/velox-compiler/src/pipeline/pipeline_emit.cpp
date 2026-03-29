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

  // Emit object
  if (compiler::COMP_CTX.target_emits.contains(common::CompCtx::EEmit::Obj)) {
    std::error_code       err_c;
    std::filesystem::path dest_path =
        std::filesystem::path(compiler::COMP_CTX.get_dir_build()) / compiler::COMP_CTX.get_project_name();
    dest_path.replace_extension(".o");

    llvm::raw_fd_ostream dest(dest_path.string(), err_c, llvm::sys::fs::OF_None);

    if (err_c) {
      llvm::errs() << "[emitter:ERROR] File error : " << err_c.message();
      return false;
    }


    llvm::legacy::PassManager pass;

    if (compiler::TM->addPassesToEmitFile(pass, dest, nullptr, llvm::CodeGenFileType::ObjectFile)) {
      llvm::errs() << "[emitter:ERROR] TargetMachine dosen't support obj emit";
      return false;
    }

    pass.run(*mod);
    dest.flush();

    std::cout << "[emitter] Object emitted at " << dest_path << std::endl;
  }

  return true;
}
