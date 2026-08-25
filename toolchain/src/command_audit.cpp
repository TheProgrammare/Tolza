#include "command_audit.hpp"

#include <common/utils.hpp>
#include <common/common.hpp>

#include <filesystem>
#include <print>
#include <fstream>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>
#include <thread>
#include <atomic>

namespace fs = std::filesystem;


bool command::audit::is_blank(std::string_view line) noexcept
{
  for (char c : line)
    if (!isspace(c)) return false;
  return true;
}


void command::audit::process_file(std::string_view file, CategoryStats& cat_stats, GlobalStats& global_stats) noexcept
{
  const fs::path f(file);

  auto contains_word = [](std::string_view line, std::string_view word) {
    size_t pos = line.find(word);
    while (pos != std::string::npos) {
      bool left_ok  = (pos == 0 || !std::isalnum(line[pos - 1]));
      bool right_ok = (pos + word.size() >= line.size() || !std::isalnum(line[pos + word.size()]));

      if (left_ok && right_ok) return true;

      pos = line.find(word, pos + 1);
    }
    return false;
  };

  std::ifstream in(f);
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
    if (trimmed.starts_with("//")) {
      cat_stats.comment_lines++;
      continue;
    }

    // début d'un bloc de commentaire
    if (trimmed.starts_with("/*")) {
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

  global_stats.byte_size += fs::file_size(f);
}

void command::audit::audit_workspace(std::string_view root) noexcept
{
  constexpr std::string_view str_template = R"(
=============================================================================== 
 [Audit]             Files        Lines         Code     Comments       Blanks
 Source Code     {0:9}    {1:9}    {2:9}    {3:9}    {4:9}
 Third Party     {5:9}    {6:9}    {7:9}    {8:9}    {9:9}
 Binder          {10:9}    {11:9}    {12:9}    {13:9}    {14:9}
=============================================================================== 
 [Total]         {15:9}    {16:9}    {17:9}    {18:9}    {19:9}
=============================================================================== 
 [Population]        Views        Forms       Facets        Rules      Imports
    {32:9}    {20:9}    {21:9}    {22:9}    {23:9}    {24:9}
 Enumerations    Functions     Generics       Unions        Flags      Exports
    {25:9}    {26:9}    {27:9}    {28:9}    {29:9}    {30:9}
=============================================================================== 
 [Disk Size]     {31:9.2f} Ko
===============================================================================
  )";

  std::println("[tolza] Starting audit...");

  auto source_code = CategoryStats();
  auto vendor      = CategoryStats();
  auto binder      = CategoryStats();
  auto global      = GlobalStats();

  // Collect files
  struct Task {
    std::string path;
    enum class ECat : uint8_t { SRC, VENDOR, BIND };
    ECat category;
  };

  std::vector<Task> tasks;

  for (const auto& entry : fs::recursive_directory_iterator(root)) {
    if (!entry.is_regular_file()) continue;

    auto ext = entry.path().extension().string();
    if (ext != ".tlz" && ext != ".tlzbind" && ext != ".tlzlib") continue;

    std::string relative_path = entry.path().lexically_relative(root).string();

    if (relative_path.starts_with("src/"))
      tasks.push_back({entry.path(), Task::ECat::SRC});
    else if (relative_path.starts_with("vendor/"))
      tasks.push_back({entry.path(), Task::ECat::VENDOR});
    else if (relative_path.starts_with("bind/"))
      tasks.push_back({entry.path(), Task::ECat::BIND});
  }

  const size_t        total_files = tasks.size();
  std::atomic<size_t> processed{0};
  std::mutex          stats_mutex;

  // Thread progression
  std::atomic<bool> done{false};
  std::thread       progress_thread([&done, &processed, &total_files]() {
    while (!done) {
      size_t p       = processed.load();
      double percent = total_files == 0 ? 1.0 : static_cast<double>(p) / static_cast<double>(total_files);

      // strange compilation error on thread when the formatter {:.2%} is used
      // std::print("\r\033[K[tolza] auditing files: {} ({:.2%})", total_files, percent);
      std::print("\r\033[K[tolza] auditing files: {} ({:.2f}%)", total_files, percent * 100);

      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  });

  // Basic thread pool
  const size_t thread_count = std::max<size_t>(1, std::thread::hardware_concurrency());

  std::atomic<size_t>      index{0};
  std::vector<std::thread> workers;

  workers.reserve(thread_count);
  for (size_t t = 0; t < thread_count; ++t) {
    workers.emplace_back([&]() {
      auto local_src    = CategoryStats();
      auto local_vendor = CategoryStats();
      auto local_bind   = CategoryStats();
      auto local_global = GlobalStats();

      while (true) {
        size_t i = index.fetch_add(1);
        if (i >= total_files) break;

        const auto& task = tasks[i];

        switch (task.category) {
        case Task::ECat::SRC:    process_file(task.path, local_src, local_global); break;
        case Task::ECat::VENDOR: process_file(task.path, local_vendor, local_global); break;
        case Task::ECat::BIND:   process_file(task.path, local_bind, local_global); break;
        }

        processed++;
      }

      // Protected merge
      std::lock_guard<std::mutex> lock(stats_mutex);

      source_code += local_src;
      vendor += local_vendor;
      binder += local_bind;
      global += local_global;
    });
  }

  for (auto& w : workers) w.join();

  done = true;
  progress_thread.join();

  std::println("\r\033[K\n[tolza] audit complete.");

  global.global_cat.lines         = source_code.lines + vendor.lines + binder.lines;
  global.global_cat.code_lines    = source_code.code_lines + vendor.code_lines + binder.code_lines;
  global.global_cat.comment_lines = source_code.comment_lines + vendor.comment_lines + binder.comment_lines;
  global.global_cat.blank_lines   = source_code.blank_lines + vendor.blank_lines + binder.blank_lines;
  global.global_cat.files         = source_code.files + vendor.files + binder.files;

  double file_size = static_cast<double>(global.byte_size) / 1024.0;

  size_t population = global.comps + global.entities + global.enums + global.exports + global.flags + global.functions
                      + global.generics + global.imports + global.roles + global.sys + global.unions;


  std::print(str_template, source_code.files, source_code.lines, source_code.code_lines, source_code.comment_lines,
             source_code.blank_lines,

             vendor.files, vendor.lines, vendor.code_lines, vendor.comment_lines, vendor.blank_lines,

             binder.files, binder.lines, binder.code_lines, binder.comment_lines, binder.blank_lines,

             global.global_cat.files, global.global_cat.lines, global.global_cat.code_lines,
             global.global_cat.comment_lines, global.global_cat.blank_lines,

             global.roles, global.entities, global.comps, global.sys, global.imports, global.enums, global.functions,
             global.generics, global.unions, global.flags, global.exports, file_size, population);
}