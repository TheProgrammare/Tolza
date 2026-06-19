#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <limits>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "nexus/ast/ast.hpp"
#include "nexus/forward.hpp"
#include "nexus/ids.hpp"

namespace metacode
{

using _Meta_Key     = std::string_view;
using _Meta_Pattern = std::initializer_list<std::string_view>;

constexpr uint32_t k_section_size = std::numeric_limits<uint32_t>::max() / 4;

// file tokens : 1+2+3 section        |x|x|x|_|
// placeholder tokens : 3rd section   |_|_|_|x| - 1
// metacode flag : max u32            |_|_|_|_| - 1 <-
constexpr token::ID k_placeholder_flag_start = token::ID::make(cu::ID::main(), k_section_size * 3);
constexpr token::ID k_placeholder_flag_end   = token::ID::make(cu::ID::main(), (k_section_size * 4) - 1);
constexpr token::ID k_metacode_flag          = token::ID::make(cu::ID::main(), std::numeric_limits<uint32_t>::max());

[[nodiscard]] constexpr size_t to_placeholder_index(token::ID id)
{
  return size_t(id.raw() - k_placeholder_flag_start.raw());
}

#define METACODE_SET_KIND(kind)                                                                                        \
  static constexpr EMetacodeKind static_kind = EMetacodeKind::kind;                                                    \
  kind()                                                                                                               \
    : Metacode(EMetacodeKind::kind)                                                                                    \
  {                                                                                                                    \
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
  Word(cu::CU& p_CU)
    : CU(p_CU)
  {
  }

  cu::CU& CU;

  std::vector<std::string> tokens;

  [[nodiscard]] bool contains(std::string_view s) const noexcept;
  [[nodiscard]] bool contains(token::ETokenKind kind) const noexcept;
  [[nodiscard]] bool contains_one(const std::initializer_list<token::ETokenKind>& l) const noexcept;

  [[nodiscard]] bool have_key(const _Meta_Key& key) const noexcept;
};

enum class EPatternKey : uint8_t {
  None,
  Any,         // "<*>"
  Identifier,  // "<a>"
  Numeric,     // pattern_constants::numeric
  Alternative, // "<_>"
};

[[nodiscard]] EPatternKey str_to_EPatternKey(const _Meta_Key& key) noexcept;

struct Instruction final {
  std::vector<Word> words;

  [[nodiscard]] bool             match_pattern(const _Meta_Pattern& pattern) const noexcept;
  [[nodiscard]] std::string_view at_str(size_t pos, size_t alt) const noexcept;
};


struct Metacode {
  Metacode() = default;
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

private:
  EMetacodeKind metacode_kind = EMetacodeKind::Metablock;

protected:
  Metacode(EMetacodeKind kind)
    : metacode_kind(kind)
  {
  }

public:
  [[nodiscard]] EMetacodeKind kind() const
  {
    return metacode_kind;
  }

  virtual ~Metacode() = default;
};


struct Root final : Metacode {
  METACODE_SET_KIND(Root)
};

struct Metablock final : Metacode {
  METACODE_SET_KIND(Metablock)
};


struct Macro final : Metacode {
  METACODE_SET_KIND(Macro)

  std::vector<std::string> params;
  std::vector<token::ID>   body;
};

struct Cond_expr final : Metacode {
  METACODE_SET_KIND(Cond_expr)

  bool      is_constant    = false;
  bool      is_placeholder = false;
  token::ID val;
};

struct Binary_Cond final : Metacode {
  METACODE_SET_KIND(Binary_Cond)
  ast::EBinOpType type;

  METACODE_NODE(left)
  METACODE_NODE(right)
};

struct Unary_Not_Cond final : Metacode {
  METACODE_SET_KIND(Unary_Not_Cond)

  METACODE_NODE(term)
};

struct If final : Metacode {
  METACODE_SET_KIND(If)

  METACODE_NODE(condition)
  METACODE_NODE(alternative)

  bool is_elif = false;
  bool is_else = false;
};


struct Expand final : Metacode {
  METACODE_SET_KIND(Expand)


  struct Placeholder final {
    std::string_view       name;
    std::vector<token::ID> variants;
  };

  std::vector<Placeholder> placeholders;
  METACODE_VECTOR_NODE(expand_conditions)

  std::string expansion;
};


template <typename T>
concept DerivedMetacode = std::is_base_of_v<Metacode, T> && (!std::is_same_v<Metacode, T>);


struct Graph final {
  struct Audit final {
    Graph& graph;

    [[nodiscard]] bool               contains(ID id, _Meta_Key s) const noexcept;
    [[nodiscard]] bool               contains(ID id, token::ETokenKind tok) const noexcept;
    [[nodiscard]] const Instruction* get_instruction(ID start_id, _Meta_Pattern pattern) const noexcept;
    [[nodiscard]] const Metacode*    get_metacode(ID start_id, _Meta_Pattern pattern) const noexcept;
    // will returns the closets metacode to file_pos
    [[nodiscard]] const Metacode*    get_metacode_from_pos(size_t file_pos) const noexcept;
  };

  Graph()
  {
    (void)add<metacode::Root>(NO_ID);
  }

  bool freeze = false;

  Audit audit{*this};

  std::vector<Metacode*> metacodes;

  std::unordered_map<ID, ID, ID::Hash>              parent;
  std::unordered_map<ID, std::vector<ID>, ID::Hash> children;

  ~Graph()
  {
    for (const auto* m : metacodes) delete m;
  }

  template <DerivedMetacode T>
  [[nodiscard]] ID add(ID p_parent)
  {
    assert(!freeze && "Pool is immutable after preprocessor pass");

    const auto new_id = ID::make(metacodes.size());

    T* obj      = new T();
    obj->metaid = new_id;
    metacodes.emplace_back(obj);

    if (p_parent) {
      parent[new_id] = p_parent;
      children[p_parent].emplace_back(new_id);
    }

    return new_id;
  }

  [[nodiscard]] Metacode& get(ID id) noexcept
  {
    assert(id.raw() < metacodes.size());
    return *metacodes[id.raw()];
  }

  template <DerivedMetacode T>
  [[nodiscard]] T* as(ID id) noexcept
  {
    Metacode* m = &get(id);
    if (!m) return nullptr;

    if (m->kind() != T::static_kind) return nullptr;

    return static_cast<T*>(m);
  }

  [[nodiscard]] Root& get_file_root() noexcept
  {
    return *as<Root>(ID::make(0));
  }
};


} // namespace metacode