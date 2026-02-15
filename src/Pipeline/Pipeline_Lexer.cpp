#include "Pipeline_Lexer.hpp"

#include <iostream>
#include <chrono>

#include "Lexer/Lexer.hpp"
#include "Globals.hpp"


bool pipeline_start_lexer(const std::vector<std::shared_ptr<ScriptInfo>> &scr_infos) {
	std::vector<std::tuple<std::string, std::vector<std::string>>> lexErrors;
	
	std::chrono::duration<double> final_duration;
	size_t final_toks = 0;
	
	size_t path_count = 0;
	for (auto &scr_info: scr_infos) {
		Lexer lexer(*scr_info.get());
		if (in_binding_compilation) std::cout << "[EMBinder] ";
		std::cout << "[lex]";
        std::cout << color_CYAN " [" << path_count + 1 << "/" << scr_infos.size() << "] " color_RESET;
        std::cout << color_MAGENTA << scr_info->file_path << color_RESET << "... " << std::flush;


		auto start = std::chrono::high_resolution_clock::now();
		lexer.tokenize(std::set<char>{ EOF });
		auto end = std::chrono::high_resolution_clock::now();
		double milli = std::chrono::duration<double, std::milli>(end - start).count();

		if (!lexer.errors.empty()) {
			lexErrors.push_back({ scr_info->name, lexer.errors});
			std::cout << color_RED << "ERR " << color_YELLOW << milli << " ms" << color_RESET << std::endl;
		}
		else {
			std::cout << color_GREEN << "OK " << color_YELLOW << milli << " ms" << color_RESET;
			std::cout << color_CYAN " (" << scr_info->tokens.size() << " tokens)" color_RESET << std::endl;
		}

		path_count++;
		final_duration += end - start;
		final_toks += scr_info->tokens.size();
	}


	if (!lexErrors.empty()) {
		if (in_binding_compilation) std::cout << color_RED "[EMBinder] ";
		std::cerr << color_RED "[build] Lexer failed\n" color_RESET;
		for (auto& errs : lexErrors) {
			auto [name, fileError] = errs;
			for (const auto& f_err : fileError) {
				std::cerr << f_err << "\n";
			}
		}

		std::cerr << COMP_ABORT;
		return false;
	}

	double milli = std::chrono::duration<double, std::milli>(final_duration).count();

	if (in_binding_compilation) std::cout << color_YELLOW "[EMBinder] ";
	std::cout << color_YELLOW  "[lex] [summary] " << color_RESET << 
		"duration: " << color_YELLOW << milli << " ms" << color_RESET <<
		" | tokens: " << color_YELLOW << final_toks << color_RESET << "\n";
	std::cout << std::endl;

	return true;
}