#include "Pipeline_Lexer.hpp"

#include <iostream>

#include "Pipeline.hpp"
#include "Lexer/Lexer.hpp"
#include "Globals.hpp"


namespace {

std::optional<std::vector<std::vector<Token>>> lexer_pass(const FileSystemOut &fs_out) {
	std::vector<std::vector<Token>> lexs;
	std::vector<std::tuple<std::string, std::vector<std::string>>> lexErrors;
	
	std::chrono::duration<double> final_duration;
	size_t final_toks = 0;
	
	size_t path_count = 0;
	for (const auto& str_file: fs_out.files_str) {
		Lexer lexer(str_file, fs_out.files_paths[path_count], fs_out.files_lines[path_count]);
		if (in_binding_compilation) std::cout << "[EMBinder] ";
		std::cout << "[lex]";
        std::cout << color_CYAN " [" << path_count + 1 << "/" << fs_out.files_str.size() << "] " color_RESET;
        std::cout << color_MAGENTA << fs_out.files_paths[path_count] << color_RESET << "... " << std::flush;


		auto start = std::chrono::high_resolution_clock::now();
		lexer.tokenize(std::set<char>{ EOF });
		auto end = std::chrono::high_resolution_clock::now();
		double milli = std::chrono::duration<double, std::milli>(end - start).count();

		lexs.push_back(lexer.tokens);

		if (!lexer.errors.empty()) {
			lexErrors.push_back({ fs_out.files_paths[path_count].filename().string(), lexer.errors});
			std::cout << color_RED << "ERR " << color_YELLOW << milli << " ms" << color_RESET << std::endl;
		}
		else {
			std::cout << color_GREEN << "OK " << color_YELLOW << milli << " ms" << color_RESET;
			std::cout << color_CYAN " (" << lexer.tokens.size() << " tokens)" color_RESET << std::endl;
		}

		path_count++;
		final_duration += end - start;
		final_toks += lexer.tokens.size();
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
		return std::nullopt;
	}

	double milli = std::chrono::duration<double, std::milli>(final_duration).count();

	if (in_binding_compilation) std::cout << color_YELLOW "[EMBinder] ";
	std::cout << color_YELLOW  "[lex] [summary] " << color_RESET << 
		"duration: " << color_YELLOW << milli << " ms" << color_RESET <<
		" | tokens: " << color_YELLOW << final_toks << color_RESET << "\n";
	std::cout << std::endl;

	return lexs;
}

}



std::optional<PipelineScripts*> pipeline_start_lexer(const FileSystemOut &fs_out) {
    auto tokens_out = lexer_pass(fs_out);
	if (!tokens_out.has_value()) return std::nullopt;
	std::vector<std::vector<Token>> lexs = tokens_out.value();

	// build scrInfos to store all necessary data
	std::vector<std::shared_ptr<ScriptInfo>> scrInfos;
	scrInfos.reserve(lexs.size());
	for (size_t i = 0; i < lexs.size(); i++) {
		std::filesystem::path f(fs_out.files_paths[i]);
		std::shared_ptr<ScriptInfo> info = std::make_shared<ScriptInfo>(f.filename().string());
		info->tokens = lexs[i];
		info->src_lines = fs_out.files_lines[i];
		info->file_path = fs_out.files_paths[i];

		scrInfos.push_back(info);
	}
	
    return new PipelineScripts{scrInfos};
}