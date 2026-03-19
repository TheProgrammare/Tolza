#include "pipeline_llvm-ir.hpp"

#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>

#include "codegen/visitor_codegen.hpp"
#include "compiler_data.hpp"
#include "compiler.hpp"

#include "script_info.hpp"

void emit_llvm_to_file(const Visitor_Codegen& v)
{
  std::filesystem::path out_llvm_file(compiler::COMP_CTX.get_llvmir_dir());
  std::filesystem::create_directories(out_llvm_file);
  out_llvm_file /= v.mod.getName().str();
  out_llvm_file.replace_extension("ll");

  std::error_code EC;

  llvm::raw_fd_ostream out_f(out_llvm_file.string(), EC, llvm::sys::fs::OF_None);

  if (EC) {
    llvm::errs() << "Error cannot open the file: " << EC.message() << "\n";
    return;
  }

  v.mod.print(out_f, nullptr);
  std::cout << "emit llvm-ir to " color_MAGENTA << out_llvm_file << " " << std::endl;
}

bool pipeline_start_LLVM_IR(const std::vector<std::shared_ptr<ScriptInfo>>& scr_infos)
{
  std::vector<std::tuple<std::string, std::vector<std::string>>> llvmIRErrors;

  std::filesystem::create_directories(compiler::COMP_CTX.get_llvmir_dir());

  size_t count = 0;
  for (auto& scr_info : scr_infos) {
    std::cout << "[llvm-ir:" << ++count << "/" << scr_infos.size() << "] " color_RESET;
    std::cout << color_MAGENTA << scr_info->file_path << color_RESET "... " << std::endl;

    auto            start = std::chrono::high_resolution_clock::now();
    Visitor_Codegen codegen_visit(*scr_info);
    codegen_visit.visit(*scr_info->rootNode);

    if (compiler::COMP_CTX.emit_llvm) emit_llvm_to_file(codegen_visit);

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

  return true;
}