#pragma once

#include <cstddef>
#include <string_view>


struct CategoryStats {
  size_t files         = 0;
  size_t lines         = 0;
  size_t code_lines    = 0;
  size_t comment_lines = 0;
  size_t blank_lines   = 0;

  CategoryStats& operator+=(const CategoryStats& other)
  {
    files += other.files;
    lines += other.lines;
    code_lines += other.code_lines;
    comment_lines += other.comment_lines;
    blank_lines += other.blank_lines;
    return *this;
  }
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

  GlobalStats& operator+=(const GlobalStats& other)
  {
    byte_size += other.byte_size;
    imports += other.imports;
    exports += other.exports;
    functions += other.functions;
    generics += other.generics;
    roles += other.roles;
    entities += other.entities;
    comps += other.comps;
    enums += other.enums;
    unions += other.unions;
    flags += other.flags;
    sys += other.sys;
    return *this;
  }
};

namespace command::audit
{

[[nodiscard]] bool is_blank(std::string_view line) noexcept;
void               process_file(std::string_view file, CategoryStats& cat_stats, GlobalStats& global_stats) noexcept;
void               audit_workspace(std::string_view root) noexcept;

} // namespace command::audit
