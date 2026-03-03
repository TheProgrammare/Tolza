#include "command_audit.hpp"
#include "globals.hpp"

#include <filesystem>
#include <initializer_list>
#include <ios>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>

bool command::audit::is_blank(const std::string& line)
{
  for (char c : line)
    if (!isspace(c)) return false;
  return true;
}


void command::audit::process_file(const fs::path& file, CategoryStats& cat_stats, GlobalStats& global_stats)
{
  auto contains_word = [](const std::string& line, const std::string& word) {
    size_t pos = line.find(word);
    while (pos != std::string::npos) {
      bool left_ok  = (pos == 0 || !std::isalnum(line[pos - 1]));
      bool right_ok = (pos + word.size() >= line.size() || !std::isalnum(line[pos + word.size()]));

      if (left_ok && right_ok) return true;

      pos = line.find(word, pos + 1);
    }
    return false;
  };

  std::ifstream in(file);
  if (!in.is_open()) return;

  cat_stats.files++;

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
    if (contains_word(trimmed, "import")) global_stats.imports++;
    if (contains_word(trimmed, "export")) global_stats.exports++;
    if (contains_word(trimmed, "fn")) global_stats.functions++;
    if (contains_word(trimmed, "gen")) global_stats.generics++;
    if (contains_word(trimmed, "role")) global_stats.roles++;
    if (contains_word(trimmed, "entity")) global_stats.entities++;
    if (contains_word(trimmed, "comp")) global_stats.comps++;
    if (contains_word(trimmed, "enum")) global_stats.enums++;
    if (contains_word(trimmed, "union")) global_stats.unions++;
    if (contains_word(trimmed, "flag")) global_stats.flags++;
    if (contains_word(trimmed, "sys")) global_stats.sys++;
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
    %32    %20    %21    %22    %23    %24
 Enumerations    Functions     Generics       Unions        Flags      Exports
    %25    %26    %27    %28    %29    %30
=============================================================================== 
 [Disk Size]     %31 Ko
===============================================================================
  )";

  auto fmt_number = [&](size_t num) {
    std::ostringstream os;
    os << std::setw(9) << std::setfill(' ') << num;
    return os.str();
  };

  auto fmt_dbl = [&](double num) {
    std::ostringstream os;
    os << std::fixed << std::setprecision(2) << std::setw(9) << std::setfill(' ') << num;
    return os.str();
  };
  std::cout << "[velox] starting audit..." << std::endl;

  CategoryStats source_code;
  CategoryStats third_party;
  CategoryStats binder;
  GlobalStats   global;

  // Collect files
  struct Task {
    fs::path path;
    enum { SRC, THIRD, BIND } category;
  };

  std::vector<Task> tasks;

  for (auto& entry : fs::recursive_directory_iterator(root)) {
    if (!entry.is_regular_file()) continue;

    auto ext = entry.path().extension().string();
    if (ext != ".velox" && ext != ".vlx" && ext != ".vlxb") continue;

    std::string relative_path = entry.path().lexically_relative(root).string();

    if (relative_path.find("src/") == 0)
      tasks.push_back({entry.path(), Task::SRC});
    else if (relative_path.find("thirdparty/") == 0)
      tasks.push_back({entry.path(), Task::THIRD});
    else if (relative_path.find("bind/") == 0)
      tasks.push_back({entry.path(), Task::BIND});
  }

  const size_t        total_files = tasks.size();
  std::atomic<size_t> processed{0};
  std::mutex          stats_mutex;

  // Thread progression
  std::atomic<bool> done{false};
  std::thread       progress_thread([&]() {
    while (!done) {
      size_t p       = processed.load();
      double percent = total_files == 0 ? 100.0 : (100.0 * p) / total_files;

      std::cout << "\r\033[K[velox] auditing: " << p << "/" << total_files << " (" << std::fixed << std::setprecision(1)
                << percent << "%)" << std::flush;

      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  });

  // Basic thread pool
  const size_t thread_count = std::max<size_t>(1, std::thread::hardware_concurrency());

  std::atomic<size_t>      index{0};
  std::vector<std::thread> workers;

  for (size_t t = 0; t < thread_count; ++t) {
    workers.emplace_back([&]() {
      CategoryStats local_src{};
      CategoryStats local_third{};
      CategoryStats local_bind{};
      GlobalStats   local_global{};

      while (true) {
        size_t i = index.fetch_add(1);
        if (i >= total_files) break;

        const auto& task = tasks[i];

        switch (task.category) {
        case Task::SRC:   process_file(task.path, local_src, local_global); break;
        case Task::THIRD: process_file(task.path, local_third, local_global); break;
        case Task::BIND:  process_file(task.path, local_bind, local_global); break;
        }

        processed++;
      }

      // Protected merge
      std::lock_guard<std::mutex> lock(stats_mutex);

      source_code += local_src;
      third_party += local_third;
      binder += local_bind;
      global += local_global;
    });
  }

  for (auto& w : workers) w.join();

  done = true;
  progress_thread.join();

  std::cout << "\r\033[K[velox] audit complete." << std::endl;

  global.global_cat.lines         = source_code.lines + third_party.lines + binder.lines;
  global.global_cat.code_lines    = source_code.code_lines + third_party.code_lines + binder.code_lines;
  global.global_cat.comment_lines = source_code.comment_lines + third_party.comment_lines + binder.comment_lines;
  global.global_cat.blank_lines   = source_code.blank_lines + third_party.blank_lines + binder.blank_lines;
  global.global_cat.files         = source_code.files + third_party.files + binder.files;

  double file_size = static_cast<double>(global.byte_size) / 1024.0;

  size_t population = global.comps + global.entities + global.enums + global.exports + global.flags + global.functions
                      + global.generics + global.imports + global.roles + global.sys + global.unions;


  std::string out = out_str;

  std::initializer_list<std::string> vars = {

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
      fmt_dbl(file_size),
      fmt_number(population)

  };

  fmt_template(out, vars);

  std::cout << out << std::endl;
}