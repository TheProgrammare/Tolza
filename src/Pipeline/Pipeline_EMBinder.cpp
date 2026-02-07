#include "Pipeline_EMBinder.hpp"

#include <vector>
#include <unordered_map>
#include <set>
#include <string>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <stdio.h>

#include "Globals.hpp"
#include "Compilation.hpp"

#include "ScriptInfo.hpp"
#include "Pipeline.hpp"
#include "AST/AST_Forward.hpp"
#include "AST/AST_Base.hpp"
#include "EMBinder/C_EMBinder.hpp"

bool generate_script(
	ScriptInfo* &bind_info,
	std::unordered_map<std::string, EExternItem> &items_to_generate,
	const std::string &lang,
	const std::string &lib)
{

	std::cout << color_MAGENTA << bind_info->name << color_RESET " generation... " << std::flush;
	std::ofstream f(bind_info->file_path.c_str());

	if (!f) throw std::runtime_error("Impossible to open \"" + bind_info->file_path + "\"");
	f.clear();

	std::string _lang = lang + std::string(labs(static_cast<long>(29 - lang.size())), ' '); 
	std::string _lib = lib + std::string(labs(static_cast<long>(29 - lib.size())), ' '); 
	std::string header = EMBINDER_FILE_HEADER;
	fmt_template(header, { _lang, _lib, lang });
	f << header << std::flush;

	EMBinder_LibC bind(lang, lib, f, items_to_generate);
	auto ignore = bind.c_lib_to_velox_lib();

	// end of export lang
	f << "\n}" << std::endl;
	f.close();
	return true;
}

bool generate_binds(
	std::unordered_map<std::string, ScriptInfo*> &bind_scrInfo,
	std::unordered_map<std::string, std::unordered_map<std::string, EExternItem>> &items_to_generate,
	std::unordered_map<std::string, std::tuple<std::string, std::string>> &bind_context_generated) 
{
	auto start = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> final_duration;
	
	size_t count = 0;
	for (auto& [bind_name, bindInfo] : bind_scrInfo) {
		auto& [lang, lib] = bind_context_generated[bind_name];
		std::cout << "[EMBinder]" color_CYAN " [" << ++count << "/" << bind_scrInfo.size() << "] " color_RESET;
        
		bool success = generate_script(bindInfo, items_to_generate[bind_name], lang, lib);

		auto end = std::chrono::high_resolution_clock::now();

		auto milli = std::chrono::duration<double, std::milli>(end - start).count();

		if (success) std::cout << color_GREEN "OK " color_YELLOW << milli << " ms" << color_RESET << std::endl;
		else std::cout << color_RED "ERR " color_YELLOW << milli << " ms" << color_RESET << std::endl;
		final_duration += end - start;
	}

	auto milli = std::chrono::duration<double, std::milli>(final_duration).count();

	std::cout << color_YELLOW "[EMBinder] [summary] " color_RESET << 
		"duration: " color_YELLOW << milli << " ms" << color_RESET <<
		" | bind files: " color_YELLOW << bind_scrInfo.size() << color_RESET << "\n";
	std::cout << std::endl;

	in_binding_compilation = true;
	start = std::chrono::high_resolution_clock::now();
	if (!start_compilation(BINDING_DIR)) {
		in_binding_compilation = false;
		return false;
	}
	auto end = std::chrono::high_resolution_clock::now();
	in_binding_compilation = false;

	milli = std::chrono::duration<double, std::milli>(end - start).count();

	std::cout << color_YELLOW "[EMBinder] [summary] " << color_RESET << 
		"duration: " << color_YELLOW << milli << " ms" << color_RESET << "\n";
	std::cout << std::endl;

	return true;
}


bool pipeline_start_EMBinder(const PipelineScripts *pipe_scripts) {
	// file name, info
	std::unordered_map<std::string, ScriptInfo*> bind_scrInfo;
	// file name, fn to generate
	std::unordered_map<std::string, std::unordered_map<std::string, EExternItem>> items_to_generate;
	// file name, (lang, lib)
	std::unordered_map<std::string, std::tuple<std::string, std::string>> bind_context_generated;

	auto start = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> final_duration;
	size_t final_binds = 0;

	// affect all symbols imported
	// according to the imported module name
	size_t count = 1;
	for (auto& sInfo : pipe_scripts->scripts_infos) {
		std::cout << "[EMBinder] [";
        std::cout << color_CYAN " [" << count++ << "/" << pipe_scripts->scripts_infos.size() << "] " color_RESET;
        std::cout << color_MAGENTA << sInfo->file_path << color_RESET "... " << std::flush;

		std::filesystem::create_directories(BINDING_DIR);
		size_t bind_count = 0;

		for (auto& extern_imp : sInfo->get_externs()) {
			std::string f_name = "EMB_" + extern_imp->name + "_" + extern_imp->extern_lib + ".vlxb";
			std::string path = BINDING_DIR + "/" + f_name; // same as .velox but for wrapper/headers
			std::ofstream f(path); f.clear(); f.close();
			
			if (!bind_scrInfo.count(f_name))
				bind_scrInfo[f_name] = new ScriptInfo(f_name);
			ScriptInfo* bind = bind_scrInfo[f_name];
			bind->name = f_name;
			bind->file_path = path;
			bind_context_generated[f_name] = { extern_imp->name, extern_imp->extern_lib };

			// load all lib symbols used
			for (auto& [id_ref, type] : extern_imp->id_references) {
				items_to_generate[f_name].insert({ id_ref->id.name, type });
				bind_count++;
			}
		}

		auto end = std::chrono::high_resolution_clock::now();
		double delta = std::chrono::duration<double, std::milli>(end - start).count();
		std::cout << color_GREEN "OK " color_YELLOW << delta << " ms" color_CYAN " (" << bind_count << " binds)" color_RESET << std::endl;
		final_duration += end - start;
		final_binds += bind_count;
	}

	double milli = std::chrono::duration<double, std::milli>(final_duration).count();

	std::cout << color_YELLOW "[EMBinder] [summary" << color_RESET << 
		"] duration: " << color_YELLOW << milli << " ms" << color_RESET <<
		" | binds: " << color_YELLOW << final_binds << color_RESET << "\n";
	std::cout << std::endl;

	return generate_binds(bind_scrInfo, items_to_generate, bind_context_generated);
}
