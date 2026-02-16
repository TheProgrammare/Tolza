#include "Pipeline_DotAST.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

#include "Globals.hpp"

#include "Visitor/ASTViewer.hpp"

void generate_AST_View(const std::vector<std::shared_ptr<ScriptInfo>> &scr_infos)
{
  auto basePath = PROJECT_DIR "/dot";
  std::filesystem::create_directories(basePath);
  for (size_t i = 0; i < scr_infos.size(); i++) {
    auto                  scr_info = scr_infos[i].get();
    std::filesystem::path path     = basePath;
    path.append(scr_info->name + ".dot");
    std::ofstream f(path);
    if (!f) throw std::runtime_error("Impossible to open " + path.string());

    AST_Viewer ast_view(*scr_info, f);
    ast_view.visit(*scr_info->rootNode);
    std::cout << "[debug] [AST]";
    std::cout << color_CYAN " [" << i + 1 << "/" << scr_infos.size() << "] " color_RESET;
    std::cout << "AST View " color_MAGENTA << path << color_RESET "... " << std::flush;
  }

  for (size_t i = 0; i < scr_infos.size(); i++) {
    auto                  scr_info = scr_infos[i].get();
    std::filesystem::path pathFile = basePath;
    pathFile.append(scr_info->name + ".dot");
    std::filesystem::path pathGen = pathFile;
    pathGen.replace_extension(".pdf");

    std::string cmd = "dot -Tpdf \"" + pathFile.string() + "\" -o \"" + pathGen.string() + "\"";
    int         ret = std::system(cmd.c_str());

    if (ret == 0) {
      std::cout << color_GREEN << "OK " color_MAGENTA << pathGen << color_RESET << std::endl;
    } else {
      std::cerr << color_RED << "ERR impossible to executate Graphviz (dot).\n" << color_RESET;
      std::cerr << "Check that Graphviz is installed and accessible from the PATH.\n";
    }
  }
  std::cout << "\n";
}