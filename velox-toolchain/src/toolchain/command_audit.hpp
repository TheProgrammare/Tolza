#pragma once

#include <cstddef>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

struct CategoryStats {
  size_t files         = 0;
  size_t lines         = 0;
  size_t code_lines    = 0;
  size_t comment_lines = 0;
  size_t blank_lines   = 0;
};

struct GlobalStats {
  CategoryStats global_cat;
  size_t        byte_size = 0;
  size_t        imports   = 0;
  size_t        exports   = 0;
  size_t        functions = 0;
  size_t        generics  = 0;
  size_t        roles     = 0;
  size_t        entities  = 0;
  size_t        comps     = 0;
  size_t        enums     = 0;
  size_t        unions    = 0;
  size_t        flags     = 0;
  size_t        sys       = 0;
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
