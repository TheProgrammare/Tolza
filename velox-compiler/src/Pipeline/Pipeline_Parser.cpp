#include "Pipeline_Parser.hpp"

#include "Globals.hpp"

#include "Parser/Parser_Base.hpp"
#include "Parser/Parser_Context.hpp"
#include "Pipeline.hpp"
#include "Visitor/Symbol_Manager.hpp"

#include <chrono>
#include <iostream>
#include <string>

bool pipeline_start_parser(const std::vector<std::shared_ptr<ScriptInfo>>& scr_infos)
{
  std::vector<std::tuple<fs::path, std::vector<std::string>>> parErrors;
  std::vector<std::tuple<fs::path, std::vector<std::string>>> declErrors;

  std::chrono::duration<double> final_duration;
  size_t                        final_node_count = 0;

  size_t count = 0;
  for (auto scr_info : scr_infos) {

    PAR::Parser_Base inParser(*scr_info);

    if (Config::in_binding_compilation) std::cout << "[EMBinder] ";
    std::cout << "[parse]";
    std::cout << color_CYAN " [" << ++count << "/" << scr_infos.size() << "] " color_RESET;
    std::cout << color_MAGENTA << scr_info->file_path << color_CYAN "... " << std::flush;

    auto                      start       = std::chrono::high_resolution_clock::now();
    std::vector<std::string>  out_par_err = inParser.start_parsing();
    std::vector<std::string>& out_sym_err = inParser.ctx->m_sym->decl_errors;
    auto                      end         = std::chrono::high_resolution_clock::now();
    double                    milli       = std::chrono::duration<double, std::milli>(end - start).count();

    if (!out_par_err.empty() || !out_sym_err.empty()) {
      parErrors.push_back({scr_info->file_path, out_par_err});
      declErrors.push_back({scr_info->file_path, out_sym_err});
      std::cout << color_RED << "ERR " color_YELLOW << milli << " ms" << color_RESET << std::endl;
    } else {
      std::cout << color_GREEN << "OK " color_YELLOW << milli << " ms" << color_RESET;
      std::cout << color_CYAN " (" << inParser.ctx->node_count << " nodes)" color_RESET << std::endl;
    }

    final_duration += end - start;
    final_node_count += inParser.ctx->node_count;
  }

  if (!parErrors.empty()) {
    if (Config::in_binding_compilation) std::cout << color_RED "[EMBinder] ";
    std::cerr << color_RED "[build] Parser failed\n" color_RESET;
    // sum of errors
    size_t err_count = 0;
    for (auto& [_, fileError] : parErrors) {
      err_count += fileError.size();
    }
    std::cerr << color_YELLOW "[summary] " << color_RED << err_count << " errors, build failed\n" color_RESET "\n";

    for (auto& [path, fileError] : parErrors) {
      if (fileError.empty()) continue;

      if (Config::in_binding_compilation) std::cout << color_RED "[EMBinder] ";
      std::cerr << color_RED "[parse] [error] [file] " color_MAGENTA << path << color_RESET "\n\n";
      for (const auto& f_err : fileError) {
        std::cerr << f_err << "\n";
      }
      std::cerr << std::endl;
    }
  }

  if (!declErrors.empty()) {
    std::cerr << color_RED "[build] Declaration failed\n";
    // sum of errors
    size_t err_count = 0;
    for (auto& p_err : declErrors) {
      err_count += std::get<1>(p_err).size();
    }
    std::cerr << color_YELLOW "[parse] [summary] " << color_RED << err_count
              << " errors, build failed\n" color_RESET "\n";

    for (auto& [name, fileError] : declErrors) {
      if (fileError.empty()) continue;

      if (Config::in_binding_compilation) std::cout << color_RED "[EMBinder] ";
      std::cerr << color_RED "[declaration] [error] [file] " color_MAGENTA << name << color_RESET "\n\n";
      for (const auto& f_err : fileError) {
        std::cerr << f_err << "\n";
      }
      std::cerr << std::endl;
    }
  }

  if (Config::in_binding_compilation) std::cout << color_YELLOW "[EMBinder] ";

  double milli = std::chrono::duration<double, std::milli>(final_duration).count();

  std::cout << color_YELLOW "[parse] [summary] " << color_RESET << "duration: " << color_YELLOW << milli << " ms"
            << color_RESET << " | nodes: " << color_YELLOW << final_node_count << color_RESET << "\n";
  std::cout << std::endl;

  if (!declErrors.empty() || !parErrors.empty()) return false;

  return true;
}
