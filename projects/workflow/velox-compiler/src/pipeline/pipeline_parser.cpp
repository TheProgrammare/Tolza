#include "pipeline_parser.hpp"

#include <chrono>
#include <iostream>
#include <string>
#include <filesystem>

#include "compiler/compiler.hpp"

#include "compiler_context.hpp"
#include "parser/parser_base.hpp"
#include "parser/parser_context.hpp"
#include "misc/symbol_manager.hpp"
#include "misc/script_info.hpp"

namespace fs = std::filesystem;

bool pipeline_start_parser(const std::vector<std::shared_ptr<ScriptInfo>>& p_scr_infos)
{
  static bool log = compiler::COMP_CTX.logs.contains("parser");

  std::vector<std::tuple<std::string, std::vector<std::string>>> parErrors;
  std::vector<std::tuple<std::string, std::vector<std::string>>> declErrors;

  size_t count = 0;
  for (auto scr_info : p_scr_infos) {
    if (scr_info->file_info.tokens.empty()) continue;

    parser::Parser_Context parser(scr_info);

    auto                      start       = std::chrono::high_resolution_clock::now();
    std::vector<std::string>  out_par_err = parser.start_parsing();
    std::vector<std::string>& out_sym_err = parser.sym_m->decl_errors;
    auto                      end         = std::chrono::high_resolution_clock::now();
    double                    milli       = std::chrono::duration<double, std::milli>(end - start).count();

    if (log) {
      static size_t count = 1;
      std::cout << "[parser:" << count++ << "] \"" << scr_info->file_info.get_file_name() << "\" | "
                << parser.node_count << " nodes | " << milli << " ms" << std::flush;
    }

    if (!out_par_err.empty() || !out_sym_err.empty()) {
      parErrors.push_back({scr_info->file_info.path, out_par_err});
      declErrors.push_back({scr_info->file_info.path, out_sym_err});
      std::cout << color_RED "ERR " color_RESET "\"" << scr_info->file_info.path << "\" " color_YELLOW << milli << " ms"
                << color_RESET << std::endl;
    }
  }

  if (!parErrors.empty()) {
    std::cerr << color_RED "[parse] Failed\n" color_RESET;
    // sum of errors
    size_t err_count = 0;
    for (auto& [_, fileError] : parErrors) {
      err_count += fileError.size();
    }
    std::cerr << color_YELLOW "[parse:summary] " << color_RED << err_count
              << " errors, build failed\n" color_RESET "\n";

    for (auto& [path, fileError] : parErrors) {
      if (fileError.empty()) continue;

      std::cerr << color_RED "[parse:ERROR] [file] " color_MAGENTA << path << color_RESET "\n\n";
      for (const auto& f_err : fileError) {
        std::cerr << "\"" << f_err << "\"\n";
      }
      std::cerr << std::endl;
    }
  }

  if (!declErrors.empty()) {
    std::cerr << color_RED "[parse] Declaration failed\n";
    // sum of errors
    size_t err_count = 0;
    for (auto& p_err : declErrors) {
      err_count += std::get<1>(p_err).size();
    }
    std::cerr << color_YELLOW "[parse:summary] " << color_RED << err_count
              << " errors, build failed\n" color_RESET "\n";

    for (auto& [name, fileError] : declErrors) {
      if (fileError.empty()) continue;

      std::cerr << color_RED "[declaration:ERROR] [file] " color_MAGENTA << name << color_RESET "\n\n";
      for (const auto& f_err : fileError) {
        std::cerr << "\"" << f_err << "\"\n";
      }
      std::cerr << std::endl;
    }
  }

  if (!declErrors.empty() || !parErrors.empty()) return false;

  return true;
}
