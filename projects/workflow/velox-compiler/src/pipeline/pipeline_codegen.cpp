#include "pipeline_codegen.hpp"

#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Linker/Linker.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/InitLLVM.h>

#include <compiler_context.hpp>

#include "codegen/visitor_codegen.hpp"
#include "compiler/compiler.hpp"
#include "misc/script_info.hpp"

void emit_llvm_to_file(const Visitor_Codegen& v)
{
  std::filesystem::path out_llvm_file(compiler::COMP_CTX.get_llvmir_dir());
  std::filesystem::create_directories(out_llvm_file);
  out_llvm_file /= v.mod->getName().str();
  out_llvm_file.replace_extension("ll");

  std::error_code EC;

  llvm::raw_fd_ostream out_f(out_llvm_file.string(), EC, llvm::sys::fs::OF_None);

  if (EC) {
    llvm::errs() << "Error cannot open the file: " << EC.message() << "\n";
    return;
  }

  v.mod->print(out_f, nullptr);
  std::cout << "emit llvm-ir to " color_MAGENTA << out_llvm_file << " " << std::endl;
}

bool llvm_link(const std::vector<std::shared_ptr<ScriptInfo>>& scr_infos)
{
  if (scr_infos.empty()) return true;

  auto& main_mod = scr_infos[0]->llvm_module;

  llvm::Linker linker(*main_mod);

  llvm::Linker::Flags flag = llvm::Linker::Flags::None;

  bool first_linked = true;
  bool failed       = false;
  for (auto& scr_info : scr_infos) {
    if (first_linked) {
      first_linked = false;
      continue;
    }

    if (linker.linkInModule(std::unique_ptr<llvm::Module>(scr_info->llvm_module)), flag) {
      std::cerr << "[llvm-ir:ERROR] Link failed on script " << scr_info->file_path << std::endl;
      failed = true;
    }
  }

  if (failed) return false;

  if (llvm::verifyModule(*main_mod, &llvm::errs())) {
    llvm::errs() << "[llvm-ir:ERROR] Module verification failed!\n";
    return false;
  }

  return true;
}

bool pipeline_start_codegen(const std::vector<std::shared_ptr<ScriptInfo>>& scr_infos)
{
  if (compiler::COMP_CTX.llvm_args.size() > 0) {
    llvm::cl::ParseCommandLineOptions(compiler::COMP_CTX.llvm_args.size(), compiler::COMP_CTX.llvm_args.data());
  }

  std::vector<std::tuple<std::string, std::vector<std::string>>> llvmIRErrors;

  auto start = std::chrono::high_resolution_clock::now();

  try {
    std::filesystem::create_directories(compiler::COMP_CTX.get_llvmir_dir());
  } catch (const std::runtime_error& e) {
    std::cerr << "[llvm-ir:ERROR] Directory creation failed: " << e.what() << std::endl;
    return false;
  }

  size_t count = 0;
  for (auto& scr_info : scr_infos) {
    std::cout << "[llvm-ir:" << ++count << "/" << scr_infos.size() << "] " color_RESET;
    std::cout << color_MAGENTA << scr_info->file_path << color_RESET "... " << std::endl;

    auto            start = std::chrono::high_resolution_clock::now();
    Visitor_Codegen codegen_visit(*scr_info);
    codegen_visit.visit(*scr_info->rootNode);
    scr_info->llvm_module = codegen_visit.mod;

    if (compiler::COMP_CTX.target_emits.contains(common::CompCtx::EEmit::LLVM)) emit_llvm_to_file(codegen_visit);

    auto   end   = std::chrono::high_resolution_clock::now();
    double milli = std::chrono::duration<double, std::milli>(end - start).count();

    if (codegen_visit.errors.empty()) {
      std::cout << color_GREEN << "OK " color_YELLOW << milli << " ms" << color_RESET << std::endl;

    } else {
      llvmIRErrors.push_back({scr_info->file_path, codegen_visit.errors});
      std::cout << color_RED << "ERR " color_YELLOW << milli << " ms" << color_RESET << std::endl;
    }
  }

  std::cout << std::endl;

  if (!llvmIRErrors.empty()) {
    std::cerr << color_RED "[llvm-ir] Generation failed !" color_RESET "\n";
    for (auto& [path, fileError] : llvmIRErrors) {

      std::cerr << color_RED "[llvm-ir:ERROR] [file] " << path << color_RESET "\n";
      for (auto& error : fileError) {
        std::cerr << error << "\n";
      }
      std::cerr << std::endl;
    }

    return false;
  }

  if (!llvm_link(scr_infos)) return false;

  auto   end            = std::chrono::high_resolution_clock::now();
  auto   final_duration = end - start;
  double milli          = std::chrono::duration<double, std::milli>(final_duration).count();

  std::cout << color_YELLOW "[llvm-ir:summary] " color_RESET "duration: " color_YELLOW << milli << " ms\n" << std::endl;
  return true;
}