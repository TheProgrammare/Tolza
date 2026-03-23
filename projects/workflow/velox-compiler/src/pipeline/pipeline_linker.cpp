#include "pipeline_linker.hpp"

#include <filesystem>
#include <iostream>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Linker/Linker.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/raw_ostream.h>

#include <compiler_context.hpp>

#include "compiler/compiler.hpp"
#include "misc/script_info.hpp"
#include "compiler/compiler.hpp"

bool pipeline_start_linker(const std::vector<std::shared_ptr<ScriptInfo>>& scr_infos)
{
  std::string extension = compiler::COMP_CTX.target_os == "windows" ? ".exe" : "";

  std::filesystem::path dest_path =
      std::filesystem::path(compiler::COMP_CTX.codegen_build_dir) / compiler::COMP_CTX.get_project_name();
  dest_path.replace_extension(".o");
  std::filesystem::path out_bin =
      std::filesystem::path(compiler::COMP_CTX.codegen_build_dir) / compiler::COMP_CTX.get_project_name();
  out_bin.replace_extension(extension);

  std::string command = "clang " + dest_path.string() + " -o " + out_bin.string();

  std::cout << "[linker] Clang linking command:\n  " << command << std::endl;
  if (auto err_code = system(command.c_str()); err_code != 0) {
    std::cerr << "[linker:ERROR] Linker failed: system code error " << err_code << std::endl;
    return false;
  }

  std::cout << "[linker] Executable created at " << out_bin << std::endl;
  return true;
}