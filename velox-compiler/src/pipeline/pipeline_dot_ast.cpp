#include "pipeline_dot_ast.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

#include "compiler.hpp"

#include "visitor/ast_viewer.hpp"
#include "compiler.hpp"

void generate_AST_View(const std::vector<std::shared_ptr<ScriptInfo>>& scr_infos)
{
  auto basePath = compiler::COMP_CTX.get_build_dir() / "dot";
  fs::create_directories(basePath);

  for (size_t i = 0; i < scr_infos.size(); i++) {
    auto     scr_info = scr_infos[i].get();
    fs::path path     = basePath / scr_info->file_path.filename();
    path.replace_extension(".dot");
    std::ofstream f(path);

    if (!f) throw std::runtime_error("Impossible to open " + path.string());

    AST_Viewer ast_view(*scr_info, f);
    ast_view.visit(*scr_info->rootNode);
    std::cout << "[debug] [AST]";
    std::cout << color_CYAN " [" << i + 1 << "/" << scr_infos.size() << "] " color_RESET;
    std::cout << "AST View " color_MAGENTA << path << color_RESET "... " << std::endl;
  }

  for (size_t i = 0; i < scr_infos.size(); i++) {
    auto     scr_info = scr_infos[i].get();
    fs::path p_source = basePath / scr_info->file_path.filename();
    p_source.replace_extension(".dot");
    fs::path p_gen = p_source;
    p_gen.replace_extension(".pdf");

    std::string cmd = "dot -Tpdf \"" + p_source.string() + "\" -o \"" + p_gen.string() + "\"";
    int         ret = std::system(cmd.c_str());

    if (ret == 0) {
      std::cout << color_GREEN << "OK " color_MAGENTA << p_gen << color_RESET << std::endl;
    } else {
      std::cerr << color_RED << "ERR impossible to executate Graphviz (dot).\n" << color_RESET;
      std::cerr << "Check that Graphviz is installed and accessible from the PATH.\n";
    }
  }
  std::cout << "\n";
}