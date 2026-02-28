#include "command_audit.hpp"
#include "globals.hpp"

#include <filesystem>
#include <regex>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

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

  cat_stats.files++;

  std::string line;
  bool        in_block_comment = false;

  // Préparer les regex pour les mots-clés
  std::regex r_import("\\bimport\\b");
  std::regex r_export("\\bexport\\b");
  std::regex r_fn("\\bfn\\b");
  std::regex r_gen("\\bgen\\b");
  std::regex r_role("\\brole\\b");
  std::regex r_entity("\\bentity\\b");
  std::regex r_comp("\\bcomp\\b");
  std::regex r_enum("\\benum\\b");
  std::regex r_union("\\bunion\\b");
  std::regex r_flag("\\bflag\\b");
  std::regex r_sys("\\bsys\\b");

  while (std::getline(in, line)) {
    cat_stats.lines++;

    std::string trimmed = line;
    trimmed.erase(0, trimmed.find_first_not_of(" \t\r\n"));

    if (trimmed.empty()) {
      cat_stats.blank_lines++;
      continue;
    }

    // gestion bloc de commentaire
    if (in_block_comment) {
      cat_stats.comment_lines++;
      if (trimmed.find("*/") != std::string::npos) in_block_comment = false;
      continue;
    }

    // commentaire sur une ligne
    if (trimmed.find("//") == 0) {
      cat_stats.comment_lines++;
      continue;
    }

    // début d'un bloc de commentaire
    if (trimmed.find("/*") == 0) {
      cat_stats.comment_lines++;
      if (trimmed.find("*/") == std::string::npos) in_block_comment = true;
      continue;
    }

    // ligne de code
    cat_stats.code_lines++;

    // compter les mots-clés avec regex
    if (std::regex_search(trimmed, r_import)) global_stats.imports++;
    if (std::regex_search(trimmed, r_export)) global_stats.exports++;
    if (std::regex_search(trimmed, r_fn)) global_stats.functions++;
    if (std::regex_search(trimmed, r_gen)) global_stats.generics++;
    if (std::regex_search(trimmed, r_role)) global_stats.roles++;
    if (std::regex_search(trimmed, r_entity)) global_stats.entities++;
    if (std::regex_search(trimmed, r_comp)) global_stats.comps++;
    if (std::regex_search(trimmed, r_enum)) global_stats.enums++;
    if (std::regex_search(trimmed, r_union)) global_stats.unions++;
    if (std::regex_search(trimmed, r_flag)) global_stats.flags++;
    if (std::regex_search(trimmed, r_sys)) global_stats.sys++;
  }

  global_stats.byte_size += fs::file_size(file);
}

void command::audit::audit_workspace(const fs::path& root)
{
  static const char* out_str = R"(
=============================================================================== 
 [Audit]             Files        Lines         Code     Comments       Blanks
 Source Code     %0    %1    %2    %3    %4
 Third Party     %5    %6    %7    %8    %9
 Binder          %10    %11    %12    %13    %14
=============================================================================== 
 [Total]         %15    %16    %17    %18    %19
=============================================================================== 
 [Population]        Roles     Entities   Components      Systems      Imports
                 %20    %21    %22    %23    %24
 Enumerations    Functions     Generics       Unions        Flags      Exports
    %25    %26    %27    %28    %29    %30
=============================================================================== 
  [Disk Size]            %31 Ko
===============================================================================
  )";

  auto fmt_number = [&](size_t num) {
    std::ostringstream os;
    os << std::setw(9) << std::setfill(' ') << num;
    std::string str = os.str();
    return str;
  };

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
    } else if (relative_path.find("bind/") == 0) {
      process_file(entry.path(), binder, global);
    }
  }

  global.global_cat.lines         = source_code.lines + third_party.lines + binder.lines;
  global.global_cat.code_lines    = source_code.code_lines + third_party.code_lines + binder.code_lines;
  global.global_cat.comment_lines = source_code.comment_lines + third_party.comment_lines + binder.comment_lines;
  global.global_cat.blank_lines   = source_code.blank_lines + third_party.blank_lines + binder.blank_lines;
  global.global_cat.files         = source_code.files + third_party.files + binder.files;

  double file_size = static_cast<double>(global.byte_size) / 1024.0;

  std::string out = out_str;

  std::vector<std::string> vars = {

      fmt_number(source_code.files),
      fmt_number(source_code.lines),
      fmt_number(source_code.code_lines),
      fmt_number(source_code.comment_lines),
      fmt_number(source_code.blank_lines),

      fmt_number(third_party.files),
      fmt_number(third_party.lines),
      fmt_number(third_party.code_lines),
      fmt_number(third_party.comment_lines),
      fmt_number(third_party.blank_lines),

      fmt_number(binder.files),
      fmt_number(binder.lines),
      fmt_number(binder.code_lines),
      fmt_number(binder.comment_lines),
      fmt_number(binder.blank_lines),

      fmt_number(global.global_cat.files),
      fmt_number(global.global_cat.lines),
      fmt_number(global.global_cat.code_lines),
      fmt_number(global.global_cat.comment_lines),
      fmt_number(global.global_cat.blank_lines),

      fmt_number(global.roles),
      fmt_number(global.entities),
      fmt_number(global.comps),
      fmt_number(global.sys),
      fmt_number(global.imports),
      fmt_number(global.enums),
      fmt_number(global.functions),
      fmt_number(global.generics),
      fmt_number(global.unions),
      fmt_number(global.flags),
      fmt_number(global.exports),
      std::to_string(file_size)

  };

  fmt_template(out, vars);

  std::cout << out << std::endl;
}