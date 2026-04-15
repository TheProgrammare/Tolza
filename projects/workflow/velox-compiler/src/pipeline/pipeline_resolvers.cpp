#include "pipeline_resolvers.hpp"

#include <chrono>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <tuple>
#include <vector>

#include "common.hpp"
#include "compiler/compiler.hpp"
#include "compiler_context.hpp"
#include "misc/script_info.hpp"

#include "ast/ast_base.hpp"
#include "misc/symbol_manager.hpp"
#include "visitor/visitor_semantic.hpp"
#include "visitor/visitor_symbol.hpp"
#include "visitor/visitor_type.hpp"

bool pipeline_start_resolvers(const std::vector<std::shared_ptr<ScriptInfo>>& p_scr_infos)
{
  static bool log_sym = compiler::COMP_CTX.logs.contains("resolver_symbol");
  static bool log_typ = compiler::COMP_CTX.logs.contains("resolver_type");
  static bool log_sem = compiler::COMP_CTX.logs.contains("resolver_semantic");

  for (size_t k = 0; k < 3; k++) {
    std::vector<std::tuple<std::string, std::vector<std::string>>> resErrors;

    size_t count = 0;
    for (auto scr_info : p_scr_infos) {
      auto                     start = std::chrono::high_resolution_clock::now();
      std::vector<std::string> errs;
      // symbols
      if (k == 0) {
        Visitor_Symbol sym(*scr_info);
        scr_info->root_node->accept(sym);
        errs = scr_info->sym_m->decl_errors;
        errs.insert(errs.begin(), sym.errors.begin(), sym.errors.end());

        static size_t count = 1;
        if (log_sym)
          std::cout << "[resolver:symbol:" << count++ << "] \"" << scr_info->file_info.path << "\"" << std::endl;
      }
      // types
      if (k == 1) {
        Visitor_Type type(*scr_info);
        scr_info->root_node->accept(type);
        errs = type.errors;

        static size_t count = 1;
        if (log_sym)
          std::cout << "[resolver:type:" << count++ << "] \"" << scr_info->file_info.path << "\"" << std::endl;
      }
      // semantics
      if (k == 2) {
        Visitor_Semantic sem(*scr_info);
        scr_info->root_node->accept(sem);
        errs = sem.errors;

        static size_t count = 1;
        if (log_sym)
          std::cout << "[resolver:semantic:" << count++ << "] \"" << scr_info->file_info.path << "\"" << std::endl;
      }

      // begin resolution
      auto   end   = std::chrono::high_resolution_clock::now();
      double milli = std::chrono::duration<double, std::milli>(end - start).count();

      if (!errs.empty()) {
        resErrors.push_back({scr_info->file_info.path, errs});
        std::cerr << color_RED "ERR " color_RESET "\"" << scr_info->file_info.path << "\"" color_YELLOW << milli
                  << " ms" << color_RESET << std::endl;
      }
    }

    if (!resErrors.empty()) {
      const std::string visitor_type = k == 0 ? "symbol" : k == 1 ? "type" : "semantic";
      std::cerr << color_RED "[resolver:" << visitor_type << "] Failed" color_RESET << std::endl;
      for (auto& [path, fileError] : resErrors) {
        std::cerr << color_RED "[resolver:ERROR] [file] " color_MAGENTA "\"" << path << "\"" color_RESET "\n";
        for (auto& error : fileError) {
          std::cerr << error << "\n";
        }
        std::cerr << std::endl;
      }

      return false;
    }
  }

  return true;
}