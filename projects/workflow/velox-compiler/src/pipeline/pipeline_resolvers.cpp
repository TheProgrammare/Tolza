#include "pipeline_resolvers.hpp"

#include <chrono>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <tuple>
#include <vector>

#include "compiler/compiler.hpp"
#include "misc/script_info.hpp"

#include "ast/ast_base.hpp"
#include "visitor/symbol_manager.hpp"
#include "visitor/visitor_semantic.hpp"
#include "visitor/visitor_symbol.hpp"
#include "visitor/visitor_type.hpp"

bool pipeline_start_resolvers(const std::vector<std::shared_ptr<ScriptInfo>>& scr_infos)
{
  const static std::string resolver_head_str    = "[resolver:%0/3] " color_YELLOW "%1 " color_RESET "resolver begins";
  const static std::string resolver_section_str = "[" color_YELLOW "%0" color_RESET ":%1/%2] \"%3\"...";

  std::string passName[3] = {"symbol", "type", "semantic"};

  for (size_t k = 0; k < 3; k++) {
    std::string name       = passName[k];
    std::string header_txt = resolver_head_str;
    compiler::fmt_template(header_txt, {std::to_string(k + 1), name});
    std::cout << header_txt << std::endl;

    std::vector<std::tuple<std::string, std::vector<std::string>>> resErrors;

    size_t count = 0;
    for (auto scr_info : scr_infos) {
      std::string section_txt = resolver_section_str;
      compiler::fmt_template(section_txt,
                             {name, std::to_string(++count), std::to_string(scr_infos.size()), scr_info->file_path});
      std::cout << section_txt << std::flush;

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
      std::cerr << color_RED "[" << name << "] Failed" color_RESET << std::endl;
      for (auto& [path, fileError] : resErrors) {
        std::cerr << color_RED "[resolver:ERROR] [file] " color_MAGENTA "\"" << path << "\"" color_RESET "\n";
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