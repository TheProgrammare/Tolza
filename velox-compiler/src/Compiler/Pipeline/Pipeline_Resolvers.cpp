#include "Pipeline_Resolvers.hpp"

#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "Globals.hpp"
#include "Compiler/ScriptInfo.hpp"

#include "Compiler/AST/AST_Base.hpp"
#include "Compiler/Visitor/Symbol_Manager.hpp"
#include "Compiler/Visitor/Visitor_Semantic.hpp"
#include "Compiler/Visitor/Visitor_Symbol.hpp"
#include "Compiler/Visitor/Visitor_Type.hpp"

bool pipeline_start_resolvers(const std::vector<std::shared_ptr<ScriptInfo>>& scr_infos)
{
  std::string passName[3] = {"Symbol", "Type", "Semantic"};

  for (size_t k = 0; k < 3; k++) {
    std::string name = passName[k];

    std::vector<std::tuple<fs::path, std::vector<std::string>>> resErrors;

    std::cout << color_BLUE "[build] [resolver] stage ";
    std::cout << color_CYAN "[" << k + 1 << "/3] " color_RESET;
    std::cout << name << " Resolver begins" << std::endl;

    size_t count = 0;
    for (auto scr_info : scr_infos) {
      std::cout << "[resolver] ";
      std::cout << color_CYAN "[" << ++count << "/" << scr_infos.size() << "] " color_RESET;
      std::cout << name << " for " color_MAGENTA << scr_info->file_path << color_RESET "... " << std::flush;

      auto                     start = std::chrono::high_resolution_clock::now();
      std::vector<std::string> errs;
      // symbols
      if (k == 0) {
        Visitor_Symbol sym(*scr_info);
        scr_info->rootNode->accept(sym);
        errs = scr_info->m_sym->decl_errors;
        errs.insert(errs.begin(), sym.errors.begin(), sym.errors.end());
      }
      // types
      if (k == 1) {
        Visitor_Type type(*scr_info);
        scr_info->rootNode->accept(type);
        errs = type.errors;
      }
      // semantics
      if (k == 2) {
        Visitor_Semantic sem(*scr_info);
        scr_info->rootNode->accept(sem);
        errs = sem.errors;
      }

      // begin resolution
      auto   end   = std::chrono::high_resolution_clock::now();
      double milli = std::chrono::duration<double, std::milli>(end - start).count();

      if (errs.empty()) {
        std::cout << color_GREEN << "OK " color_YELLOW << milli << " ms" << color_RESET << std::endl;
      } else {
        resErrors.push_back({scr_info->file_path, errs});
        std::cerr << color_RED << "ERR " color_YELLOW << milli << " ms" << color_RESET << std::endl;
      }
    }

    std::cout << std::endl;

    if (!resErrors.empty()) {
      std::cerr << color_RED "[build] " << name << " Resolver failed" color_RESET << std::endl;
      for (auto& [path, fileError] : resErrors) {
        std::cerr << color_RED "[resolver] [error] [file] " color_MAGENTA << path << color_MAGENTA "\n";
        for (auto& error : fileError) {
          std::cerr << error << "\n";
        }
        std::cerr << std::endl;
      }

      return false;
    }
    std::cout << "\n";
  }

  return true;
}