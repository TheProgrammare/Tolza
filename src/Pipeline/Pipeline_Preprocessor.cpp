#include "Pipeline_Preprocessor.hpp"

#include <chrono>
#include <iostream>
#include <memory>
#include <vector>

#include "Globals.hpp"
#include "Lexer/TokenViewer.hpp"
#include "Preprocessor.hpp"
#include "ScriptInfo.hpp"

bool pipeline_start_preprocessor(const std::vector<std::shared_ptr<ScriptInfo>>& scr_infos)
{
  std::vector<std::tuple<std::string, std::vector<std::string>>> errs;

  std::chrono::duration<double> final_duration;

  const size_t files_amount = scr_infos.size();

  size_t count = 0;
  for (auto scr_info : scr_infos) {
    Preprocessor pre(*scr_info);

    if (in_binding_compilation) std::cout << "[EMBinder] ";
    std::cout << "[preprocess]";
    std::cout << color_CYAN " [" << ++count << "/" << files_amount << "] " color_RESET;
    std::cout << color_MAGENTA << scr_info->file_path << color_RESET "... " << std::flush;

    auto                      start      = std::chrono::high_resolution_clock::now();
    std::vector<Token>        final_toks = pre.preprocess();
    META::MetablockManager*&  meta       = pre.m_meta;
    std::vector<std::string>& err        = pre.tok_v->errors;

    auto   end   = std::chrono::high_resolution_clock::now();
    double milli = std::chrono::duration<double, std::milli>(end - start).count();

    if (!err.empty()) {
      errs.push_back({scr_info->name, err});
      std::cout << color_RED << "ERR " color_YELLOW << milli << " ms" << color_RESET << std::endl;
    } else {
      scr_info->tokens = final_toks;
      scr_info->m_meta = meta;
      std::cout << color_GREEN << "OK " color_YELLOW << milli << " ms" << color_RESET;
      std::cout << color_CYAN " (" << final_toks.size() << " tokens)" color_RESET << std::endl;
    }

    final_duration += end - start;
  }

  if (!errs.empty()) {
    if (in_binding_compilation) std::cout << color_RED "[EMBinder] ";
    std::cerr << color_RED "[build] Preprocessor failed\n" color_RESET;
    // sum of errors
    size_t err_count = 0;
    for (auto& [name, fileError] : errs) {
      err_count += fileError.size();
    }
    std::cerr << color_YELLOW "[summary] " << color_RED << err_count << " errors, build failed\n" color_RESET "\n";

    for (auto& [name, fileError] : errs) {
      if (fileError.empty()) continue;

      if (in_binding_compilation) std::cout << color_RED "[EMBinder] ";
      std::cerr << color_RED "[preprocess] [error] [file] " color_MAGENTA << name << color_RESET "\n\n";
      for (const auto& f_err : fileError) {
        std::cerr << f_err << "\n";
      }
      std::cerr << std::endl;
    }

    std::cerr << COMP_ABORT;
  }

  if (in_binding_compilation) std::cout << color_YELLOW "[EMBinder] ";
  std::cout << color_YELLOW "[preprocess] [summary] " << color_CYAN << "duration: " << color_YELLOW
            << std::chrono::duration<double, std::milli>(final_duration).count() << " ms" << color_RESET "\n"
            << std::endl;

  if (!errs.empty()) return false;

  return true;
}