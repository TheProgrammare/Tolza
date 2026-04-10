#include "pipeline_codegen.hpp"

#include <chrono>
#include <exception>
#include <filesystem>
#include <iostream>
#include <stdexcept>
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
  static bool log = compiler::COMP_CTX.logs.contains("emit");

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
  if (log) std::cout << "emit llvm-ir to " << out_llvm_file << std::endl;
}

bool llvm_link(const std::vector<std::shared_ptr<ScriptInfo>>& scr_infos)
{
  if (scr_infos.empty()) return true;

  llvm::Module* main_mod = scr_infos[0]->llvm_module.get();

  if (!main_mod) {
    std::cerr << "[linker:ERROR] Expected script file named 'main' to start the linking." << std::endl;
    return false;
  }


  if (llvm::verifyModule(*main_mod, &llvm::errs())) {
    llvm::errs() << "[linker:ERROR] Module verification failed!\n";
    return false;
  }


  llvm::Linker linker(*main_mod);

  llvm::Linker::Flags flag = llvm::Linker::Flags::None;

  bool first_linked = true;
  bool failed       = false;
  for (size_t i = 1; i < scr_infos.size(); ++i) {
    auto& scr_info = scr_infos[i];
    if (!scr_info->llvm_module) continue; // safe


    llvm::outs() << "[linker] verify module " << scr_info->llvm_module->getName() << "\n"
                 << "  file: \"" << scr_info->file_path << "\"\n";
    if (llvm::verifyModule(*scr_info->llvm_module, &llvm::errs())) {
      llvm::errs() << "[linker:ERROR] Module verification \"" << scr_info->file_path << "\" failed !\n ";
      return false;
    }


    std::cout << "[linker] Linking module: " << scr_info->llvm_module->getModuleIdentifier() << std::endl;

    auto module_to_link = std::move(scr_info->llvm_module);
    if (linker.linkModules(*main_mod, std::move(module_to_link))) {
      std::cerr << "[linker:ERROR] Link failed on script " << scr_info->file_path << std::endl;
      failed = true;
    }
  }

  if (failed) return false;

  if (llvm::verifyModule(*main_mod, &llvm::errs())) {
    llvm::errs() << "[linker:ERROR] Module verification failed!\n";
    return false;
  }

  return true;
}

bool pipeline_start_codegen(const std::vector<std::shared_ptr<ScriptInfo>>& scr_infos)
{
  static bool log = compiler::COMP_CTX.logs.contains("codegen");

  if (compiler::COMP_CTX.llvm_args.size() > 0) {
    llvm::cl::ParseCommandLineOptions(compiler::COMP_CTX.llvm_args.size(), compiler::COMP_CTX.llvm_args.data());
  }

  std::vector<std::tuple<std::string, std::vector<std::string>>> llvmIRErrors;

  auto start = std::chrono::high_resolution_clock::now();

  try {
    std::filesystem::create_directories(compiler::COMP_CTX.get_llvmir_dir());
  } catch (const std::runtime_error& e) {
    std::cerr << "[codegen:ERROR] Directory creation failed: " << e.what() << std::endl;
    return false;
  }

  size_t count = 0;
  for (auto& scr_info : scr_infos) {
    if (log)
      std::cout << "[codegen:" << ++count << "/" << scr_infos.size() << "] \"" << scr_info->file_path << "\""
                << std::endl;

    auto            start = std::chrono::high_resolution_clock::now();
    Visitor_Codegen codegen_visit(*scr_info);
    codegen_visit.visit(*scr_info->rootNode);
    scr_info->llvm_module = std::move(codegen_visit.__module);

    if (compiler::COMP_CTX.target_emits.contains(common::CompCtx::EEmit::LLVM)) emit_llvm_to_file(codegen_visit);

    auto   end   = std::chrono::high_resolution_clock::now();
    double milli = std::chrono::duration<double, std::milli>(end - start).count();

    if (!codegen_visit.errors.empty()) {
      llvmIRErrors.push_back({scr_info->file_path, codegen_visit.errors});
      std::cout << color_RED "ERR " color_RESET "\"" << scr_info->file_path << "\" " color_YELLOW << milli << " ms"
                << color_RESET << std::endl;
    }
  }

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

  return true;
}