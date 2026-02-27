#include "command_audit.hpp"

#include <iostream>
#include <fstream>

bool command::audit::is_blank(const std::string& line)
{
  for (char c : line)
    if (!isspace(c)) return false;
  return true;
}

void command::audit::process_file(const fs::path& file, CategoryStats& cat_stats, GlobalStats& global_stats)
{
  std::ifstream in(file);
  if (!in.is_open()) return;

  cat_stats.size_bytes += fs::file_size(file);

  std::string line;
  bool        in_block_comment = false;

  while (std::getline(in, line)) {
    cat_stats.lines++;

    std::string trimmed = line;
    trimmed.erase(0, trimmed.find_first_not_of(" \t\r\n"));

    if (trimmed.empty()) {
      cat_stats.blank_lines++;
      continue;
    }

    if (in_block_comment) {
      cat_stats.comment_lines++;
      if (trimmed.find("*/") != std::string::npos) in_block_comment = false;
      continue;
    }

    if (trimmed.find("//") == 0) {
      cat_stats.comment_lines++;
      continue;
    }

    if (trimmed.find("/*") == 0) {
      cat_stats.comment_lines++;
      if (trimmed.find("*/") == std::string::npos) in_block_comment = true;
      continue;
    }

    // ligne de code
    cat_stats.code_lines++;

    // compter les instructions globalement
    if (trimmed.find("import") != std::string::npos) global_stats.imports++;
    if (trimmed.find("export") != std::string::npos) global_stats.exports++;
    if (trimmed.find("fn") != std::string::npos) global_stats.functions++;
    if (trimmed.find("entity") != std::string::npos) global_stats.entities++;
    if (trimmed.find("comp") != std::string::npos) global_stats.comps++;
    if (trimmed.find("enum") != std::string::npos) global_stats.enums++;
    if (trimmed.find("union") != std::string::npos) global_stats.unions++;
    if (trimmed.find("flag") != std::string::npos) global_stats.flags++;
    if (trimmed.find("sys") != std::string::npos) global_stats.sys++;
  }
}

void command::audit::audit_workspace(const fs::path& root)
{
  CategoryStats source_code;
  CategoryStats third_party;
  CategoryStats binder;
  GlobalStats   global;

  for (auto& entry : fs::recursive_directory_iterator(root)) {
    if (!entry.is_regular_file()) continue;

    auto ext = entry.path().extension().string();
    if (ext != ".velox" && ext != ".vlx" && ext != ".vlxb") continue;

    std::string relative_path = entry.path().lexically_relative(root).string();

    if (relative_path.find("src/") == 0) {
      process_file(entry.path(), source_code, global);
    } else if (relative_path.find("thirdparty/") == 0) {
      process_file(entry.path(), third_party, global);
    }

    if (ext == ".vlxb") {
      process_file(entry.path(), binder, global);
    }
  }

  auto print_category = [](const std::string& name, const CategoryStats& stats) {
    std::cout << "=== " << name << " ===\n";
    std::cout << "Lines: " << stats.lines << "\n";
    std::cout << "Code: " << stats.code_lines << "\n";
    std::cout << "Comment: " << stats.comment_lines << "\n";
    std::cout << "Blank: " << stats.blank_lines << "\n";
    std::cout << "Size on disk: " << stats.size_bytes << " bytes\n\n";
  };

  print_category("Source Code", source_code);
  print_category("Third Party", third_party);
  print_category("Binder", binder);

  std::cout << "=== Global Stats ===\n";
  std::cout << "Imports: " << global.imports << "\n";
  std::cout << "Exports: " << global.exports << "\n";
  std::cout << "Functions: " << global.functions << "\n";
  std::cout << "Entities: " << global.entities << "\n";
  std::cout << "Comps: " << global.comps << "\n";
  std::cout << "Enums: " << global.enums << "\n";
  std::cout << "Unions: " << global.unions << "\n";
  std::cout << "Flags: " << global.flags << "\n";
  std::cout << "Sys: " << global.sys << "\n";
}