#include "pipeline_preprocessor.hpp"

#include <chrono>
#include <iostream>
#include <memory>
#include <vector>
#include <filesystem>

#include "compiler/compiler.hpp"
#include "compiler_context.hpp"
#include "lexer/token_viewer.hpp"
#include "misc/preprocessor.hpp"
#include "misc/script_info.hpp"

namespace fs = std::filesystem;

bool pipeline_start_preprocessor(const std::vector<std::shared_ptr<ScriptInfo>>& p_scr_infos)
{
  static bool log = compiler::COMP_CTX.logs.contains("preprocessor");

  std::vector<std::tuple<std::string, std::vector<std::string>>> errs;

  size_t count = 0;
  for (auto scr_info : p_scr_infos) {
    if (scr_info->tokens.empty()) continue;
    Preprocessor pre(*scr_info);

    auto                      start      = std::chrono::high_resolution_clock::now();
    std::vector<Token>        final_toks = pre.preprocess();
    meta::MetablockManager*&  meta       = pre.m_meta;
    std::vector<std::string>& err        = pre.tok_v->errors;

    auto   end   = std::chrono::high_resolution_clock::now();
    double milli = std::chrono::duration<double, std::milli>(end - start).count();

    if (log) {
      static size_t count = 1;
      std::cout << "[preprocessor:" << count++ << "] \"" << fs::path(scr_info->file_path).filename() << "\" | "
                << final_toks.size() << " tokens | " << milli << " ms" << std::flush;
    }

    if (!err.empty()) {
      errs.push_back({scr_info->file_path, err});
      std::cout << color_RED "ERR " color_RESET "\"" << scr_info->file_path << "\" " color_YELLOW << milli << " ms"
                << color_RESET << std::endl;
    } else {
      scr_info->tokens = final_toks;
      scr_info->m_meta = meta;
    }
  }

  if (!errs.empty()) {
    std::cerr << color_RED "[preprocess] Failed\n" color_RESET;
    // sum of errors
    size_t err_count = 0;
    for (auto& [_, fileError] : errs) {
      err_count += fileError.size();
    }
    std::cerr << color_YELLOW "[preprocess:summary] " << color_RED << err_count
              << " errors, build failed\n" color_RESET "\n";

    for (auto& [path, fileError] : errs) {
      if (fileError.empty()) continue;

      std::cerr << color_RED "[preprocess:ERROR] [file] " color_MAGENTA << path << color_RESET "\n\n";
      for (const auto& f_err : fileError) {
        std::cerr << "\"" << f_err << "\"\n";
      }
      std::cerr << std::endl;
    }

    return false;
  }

  return true;
}