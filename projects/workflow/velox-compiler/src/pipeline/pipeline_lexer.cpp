#include "pipeline_lexer.hpp"

#include <chrono>
#include <iostream>
#include <filesystem>

#include "compiler_context.hpp"
#include "misc/script_info.hpp"
#include "compiler/compiler.hpp"
#include "lexer/lexer.hpp"

namespace fs = std::filesystem;

bool pipeline_start_lexer(const std::vector<std::shared_ptr<ScriptInfo>>& p_scr_infos)
{
  static bool log = compiler::COMP_CTX.logs.contains("lexer");

  std::vector<std::tuple<std::string, std::vector<std::string>>> lexErrors;

  size_t final_toks = 0;

  for (auto& scr_info : p_scr_infos) {
    Lexer lexer(*scr_info.get());

    auto start = std::chrono::high_resolution_clock::now();
    lexer.tokenize(std::set<char>{EOF});
    auto   end   = std::chrono::high_resolution_clock::now();
    double milli = std::chrono::duration<double, std::milli>(end - start).count();

    if (!lexer.errors.empty()) lexErrors.push_back({scr_info->file_info.path, lexer.errors});

    if (log) {
      static size_t count = 1;
      std::cout << "[lexer:" << count++ << "] \"" << scr_info->file_info.get_file_name() << "\" | "
                << lexer.scr_info.file_info.tokens.size() << " tokens | " << milli << " ms" << std::flush;
    }
  }

  if (!lexErrors.empty()) {
    std::cerr << color_RED "[lex:ERROR] Lexer failed\n" color_RESET;
    for (auto& errs : lexErrors) {
      auto [_, fileError] = errs;
      for (const auto& f_err : fileError) {
        std::cerr << "\"" << f_err << "\"\n";
      }
    }

    return false;
  }
  return true;
}