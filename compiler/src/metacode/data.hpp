#pragma once

#include "id/metaid.hpp"
#include "id/tokid.hpp"
#include "nexus/forward.hpp"

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <string>
#include <string_view>
#include <vector>


namespace metacode
{

constexpr uint32_t k_section_size = UINT32_MAX / 4;

// file tokens : 1+2+3 section        |x|x|x|_|
// placeholder tokens : 3rd section   |_|_|_|x| - 1
// metacode flag : max u32            |_|_|_|_| - 1 <-
constexpr token::ID k_placeholder_flag_start = token::ID::make(cu::ID::main(), k_section_size * 3);
constexpr token::ID k_placeholder_flag_end   = token::ID::make(cu::ID::main(), (k_section_size * 4) - 1);
constexpr token::ID k_metacode_flag          = token::ID::make(cu::ID::main(), UINT32_MAX);

[[nodiscard]] constexpr size_t to_placeholder_index(token::ID id)
{
  return size_t(id.raw() - k_placeholder_flag_start.raw());
}


#define METACODE_NODE(name)        ID name;
#define METACODE_VECTOR_NODE(name) std::vector<ID> name;

enum class EMetacodeKind : uint8_t {
  Root,
  Metablock,
  Cond_expr,
  Binary_Cond,
  Unary_Not_Cond,
  If,
  Macro,
  Expand,
};

struct Word final {
  Word(cu::CU& p_CU);

  cu::CU& CU;

  std::vector<std::string> tokens;

  [[nodiscard]] bool contains(std::string_view s) const noexcept;
  [[nodiscard]] bool contains(token::ETokenKind kind) const noexcept;
  [[nodiscard]] bool contains_one(const std::initializer_list<token::ETokenKind>& l) const noexcept;

  [[nodiscard]] bool have_key(const std::string_view& key) const noexcept;
};

enum class EPatternKey : uint8_t {
  None,
  Any,         // "<*>"
  Identifier,  // "<a>"
  Numeric,     // pattern_constants::numeric
  Alternative, // "<_>"
};

[[nodiscard]] EPatternKey str_to_EPatternKey(const std::string_view& key) noexcept;

struct Instruction final {
  std::vector<Word> words;

  [[nodiscard]] bool             match_pattern(const std::initializer_list<std::string_view>& pattern) const noexcept;
  [[nodiscard]] std::string_view at_str(size_t pos, size_t alt) const noexcept;
};


struct MetacodeHeader final {
  MetacodeHeader(EMetacodeKind p_kind)
    : kind(p_kind)
  {
  }

  struct Scope final {
    std::string name;
    size_t      start_pos = 1;
    size_t      end_pos   = 1;
    bool        is_inline = true;
  };

  ID metaid;

  Scope                    scope;
  std::vector<Instruction> instructions;
  size_t                   start_toks = -1;
  size_t                   end_toks   = -1;
  std::vector<token::ID>   tokens_to_generate;


  size_t position = -1;

  EMetacodeKind kind = EMetacodeKind::Metablock;
};


#define METACODE_HEADER(name)                                                                                          \
  MetacodeHeader                 header      = MetacodeHeader(EMetacodeKind::name);                                    \
  static constexpr EMetacodeKind static_kind = EMetacodeKind::name;                                                    \
  [[nodiscard]] ID               metaid() const noexcept                                                               \
  {                                                                                                                    \
    return header.metaid;                                                                                              \
  }


struct Root final {
  METACODE_HEADER(Root);
};

struct Metablock final {
  METACODE_HEADER(Metablock)
};


struct Macro final {
  METACODE_HEADER(Macro)

  std::vector<std::string> params;
  std::vector<token::ID>   body;
};

struct Cond_expr final {
  METACODE_HEADER(Cond_expr)

  bool      is_constant    = false;
  bool      is_placeholder = false;
  token::ID val;
};

struct Binary_Cond final {
  METACODE_HEADER(Binary_Cond)
  ast::EOp_Bin type;

  METACODE_NODE(left)
  METACODE_NODE(right)
};

struct Unary_Not_Cond final {
  METACODE_HEADER(Unary_Not_Cond)

  METACODE_NODE(term)
};

struct If final {
  METACODE_HEADER(If)

  METACODE_NODE(condition)
  METACODE_NODE(alternative)

  bool is_elif = false;
  bool is_else = false;
};


struct Expand final {
  METACODE_HEADER(Expand)


  struct Placeholder final {
    std::string_view       name;
    std::vector<token::ID> variants;
  };

  std::vector<Placeholder> placeholders;
  METACODE_VECTOR_NODE(expand_conditions)

  std::string expansion;
};

template <typename T>
concept Generic = requires(T obj) {
  obj.header;
  obj.static_kind;
  obj.header.metaid;
  obj.metaid();
};

} // namespace metacode