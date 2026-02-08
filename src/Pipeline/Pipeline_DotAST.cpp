#include "Pipeline_DotAST.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

#include "Globals.hpp"

#include "Pipeline.hpp"
#include "Visitor/ASTViewer.hpp"

void generate_AST_View(const PipelineScripts *pipe_scripts) {
	auto basePath = PROJECT_DIR "/dot";
	std::filesystem::create_directories(basePath);
	for (size_t i = 0; i < pipe_scripts->scripts_infos.size(); i++) {
		auto scrInfo = pipe_scripts->scripts_infos[i].get();
		std::filesystem::path path = basePath; path.append(scrInfo->name + ".dot");
		std::ofstream f(path);
		if (!f) throw std::runtime_error("Impossible to open " + path.string());
		
		AST_Viewer ast_view(*scrInfo, f);
		ast_view.visit(*scrInfo->rootNode);
        std::cout << "[debug] [AST]";
        std::cout << color_CYAN " [" << i + 1 << "/" << pipe_scripts->scripts_infos.size() << "] " color_RESET;
		std::cout << "AST View " color_MAGENTA << path << color_RESET "... " << std::flush;
	}

	for (size_t i = 0; i < pipe_scripts->scripts_infos.size(); i++) {
		auto scrInfo = pipe_scripts->scripts_infos[i].get();
		std::filesystem::path pathFile = basePath; pathFile.append(scrInfo->name + ".dot");
		std::filesystem::path pathGen = pathFile; pathGen.replace_extension(".pdf");

		std::string cmd = "dot -Tpdf \"" + pathFile.string() + "\" -o \"" + pathGen.string() + "\"";
		int ret = std::system(cmd.c_str());

		if (ret == 0) {
			std::cout << color_GREEN << "OK " color_MAGENTA << pathGen << color_RESET << std::endl;
		}
		else {
			std::cerr << color_RED << "ERR impossible to executate Graphviz (dot).\n" << color_RESET;
			std::cerr << "Check that Graphviz is installed and accessible from the PATH.\n";
		}
	}
	std::cout << "\n";
}