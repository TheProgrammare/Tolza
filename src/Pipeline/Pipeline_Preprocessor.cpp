#include "PipelinePreprocessor.hpp"

#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>

#include "Compilation.hpp"
#include "Globals.hpp"
#include "Pipeline.hpp"
#include "ScriptInfo.hpp"
#include "Preprocessor.hpp"
#include "Lexer/TokenViewer.hpp"

bool pipeline_start_preprocessor(const PipelineScripts* pipe_scripts) {
	std::vector<std::tuple<std::string, std::vector<std::string>>> errs;

	std::chrono::duration<double> final_duration;

    const size_t files_amount = pipe_scripts->scripts_infos.size();

	size_t count = 0;
	for (auto info : pipe_scripts->scripts_infos) {
		Preprocessor pre(info.get());

		if (in_binding_compilation) std::cout << "[EMBinder] ";
		std::cout << "[preprocess]";
        std::cout << color_CYAN " [" << ++count << "/" << files_amount << "] " color_RESET;
        std::cout << color_MAGENTA << info->file_path << color_RESET "... " << std::flush;

		auto start = std::chrono::high_resolution_clock::now();
		std::vector<Token> final_toks = pre.preprocess();
		META::MetablockManager*& meta = pre.m_meta;
		std::vector<std::string>& err = pre.tok_v->errors;
		
		auto end = std::chrono::high_resolution_clock::now();
		double milli = std::chrono::duration<double, std::milli>(end - start).count();

		if (!err.empty()) {
			errs.push_back({ info->name, err });
			std::cout << color_RED << "ERR " color_YELLOW << milli << " ms" << color_RESET << std::endl;
		}
		else {
            info->tokens = final_toks;
            info->m_meta = meta;
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
		for	(auto& [name, fileError] : errs) { err_count += fileError.size(); }
		std::cerr << color_YELLOW "[summary] " << color_RED << err_count << " errors, build failed\n" color_RESET;
		
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
	std::cout << color_YELLOW "[preprocess] [summary] " << color_RESET << 
		"duration: " << color_CYAN << std::chrono::duration<double, std::milli>(final_duration).count() << " ms" << color_RESET << "\n";
	std::cout << std::endl;

	if (!errs.empty()) return false;


    return true;
}