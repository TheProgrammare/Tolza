#include "PipelineResolvers.hpp"

#include <string>
#include <vector>
#include <tuple>
#include <iostream>
#include <chrono>

#include "Globals.hpp"
#include "Pipeline.hpp"
#include "ScriptInfo.hpp"

#include "Visitor/VisitorSymbol.hpp"
#include "Visitor/VisitorType.hpp"
#include "Visitor/VisitorSemantic.hpp"
#include "AST/ASTNodes.hpp"
#include "AST/ASTSymbol.hpp"


bool pipeline_start_resolvers(const PipelineScripts *pipe_scripts) {
	std::string passName[3] = { "Symbol", "Type", "Semantic" };

	for (size_t k = 0; k < 3; k++) {
		std::string name = passName[k];
		
		std::vector<std::tuple<std::string, std::vector<std::string>>> resErrors;

		std::cout << color_BLUE "[build] [resolver]";
        std::cout << color_CYAN " [" << k + 1 << "/3] " color_RESET;
        std::cout << name << " Resolver begins" << std::endl;

		size_t count = 0;
		for (auto scrInfo : pipe_scripts->scripts_infos) {
            std::cout << "[resolve]";   
            std::cout << color_CYAN " [" << ++count << "/3] " color_RESET;
            std::cout << color_CYAN " [" << count << "/" << pipe_scripts->scripts_infos.size() << "] " color_RESET;
			std::cout << name << " for " color_MAGENTA << scrInfo->file_path << color_RESET "... " << std::flush;

			auto start = std::chrono::high_resolution_clock::now();
			std::vector<std::string> errs;
			// symbols
			if (k == 0) {
				VisitorSym sym(scrInfo.get());
				scrInfo->rootNode->accept(sym);
				errs = scrInfo->m_sym->decl_errors;
				errs.insert(errs.begin(), sym.errors.begin(), sym.errors.end());
			}
			// types
			if (k == 1) {
				VisitorType ty(scrInfo.get());
				scrInfo->rootNode->accept(ty);
				errs = ty.errors;
			}
			// semantics
			if (k == 2) {
				VisitorSem sem(scrInfo.get());
				scrInfo->rootNode->accept(sem);
				errs = sem.errors;
			}

			// begin resolution
			auto end = std::chrono::high_resolution_clock::now();
			double milli = std::chrono::duration<double, std::milli>(end - start).count();

			if (errs.empty()) {
				std::cout << color_GREEN << "OK " color_YELLOW << milli << " ms" << color_RESET << std::endl;
			}
			else {
				resErrors.push_back({ scrInfo->name, errs });
				std::cerr << color_RED << "ERR " color_YELLOW << milli << " ms" << color_RESET << std::endl;
			}

		}

		std::cout << std::endl;

		if (!resErrors.empty()) {
			std::cerr << color_RED "[build] " << name << " Resolver failed" color_RESET << std::endl;
			for (auto &[file_name, fileError] : resErrors) {
				std::cerr << color_RED "[resolver] [error] [file] " color_MAGENTA << file_name <<  color_MAGENTA "\n";
				for (auto &error : fileError) {
					std::cerr << error << "\n";
				}
				std::cerr << std::endl;
			}

			std::cerr << COMP_ABORT << std::endl;

			return false;
		}
		std::cout << "\n";
	}

	return true;
}