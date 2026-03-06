#include "pipeline_llvm-ir.hpp"

#include <chrono>
#include <filesystem>
#include <iostream>
#include <llvm-19/llvm/IR/Instructions.h>
#include <string>
#include <tuple>
#include <vector>

#include "compiler.hpp"
#include "compiler.hpp"

#include "script_info.hpp"
#include "visitor/visitor_codegen.hpp"

bool pipeline_start_LLVM_IR(const std::vector<std::shared_ptr<ScriptInfo>>& scr_infos)
{
  std::vector<std::tuple<fs::path, std::vector<std::string>>> llvmIRErrors;

  std::filesystem::create_directories(compiler::COMP_CTX.get_llvmir_dir());

  size_t count = 0;
  for (auto& scr_info : scr_infos) {
    std::cout << "[LLVM IR]";
    std::cout << color_CYAN " [" << ++count << "/" << scr_infos.size() << "] " color_RESET;
    std::cout << color_MAGENTA << scr_info->file_path << color_RESET "... " << std::flush;

    auto            start = std::chrono::high_resolution_clock::now();
    Visitor_Codegen codegen_visi(*scr_info);
    codegen_visi.visit(*scr_info->rootNode);
    auto   end   = std::chrono::high_resolution_clock::now();
    double milli = std::chrono::duration<double, std::milli>(end - start).count();

    if (codegen_visi.errors.empty()) {
      std::cout << color_GREEN << "OK " color_YELLOW << milli << " ms" << color_RESET << std::endl;

    } else {
      llvmIRErrors.push_back({scr_info->file_path, codegen_visi.errors});
      std::cout << color_RED << "ERR " color_YELLOW << milli << " ms" << color_RESET << std::endl;
    }
  }

  std::cout << std::endl;

  if (!llvmIRErrors.empty()) {
    std::cerr << color_RED "[velox-compiler] LLVM IR Generation failed !" color_RESET "\n";
    for (auto& [path, fileError] : llvmIRErrors) {

      std::cerr << color_RED "[LLVM IR] [error] [file] " << path << color_RESET "\n";
      for (auto& error : fileError) {
        std::cerr << error << "\n";
      }
      std::cerr << std::endl;
    }

    return false;
  }

  return true;
}