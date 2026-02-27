#pragma once

#include <string>
#include <filesystem>

namespace fs = std::filesystem;

struct CategoryStats {
  size_t lines         = 0;
  size_t code_lines    = 0;
  size_t comment_lines = 0;
  size_t blank_lines   = 0;
  size_t size_bytes    = 0;
};

struct GlobalStats {
  size_t imports   = 0;
  size_t exports   = 0;
  size_t functions = 0;
  size_t entities  = 0;
  size_t comps     = 0;
  size_t enums     = 0;
  size_t unions    = 0;
  size_t flags     = 0;
  size_t sys       = 0;
};

namespace command
{
namespace audit
{
bool is_blank(const std::string& line);
void process_file(const fs::path& file, CategoryStats& cat_stats, GlobalStats& global_stats);
void audit_workspace(const fs::path& root);
} // namespace audit
} // namespace command
