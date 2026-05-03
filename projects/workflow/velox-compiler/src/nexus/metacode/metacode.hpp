#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <limits>
#include <map>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "common.hpp"
#include "nexus/ast/ast.hpp"
#include "nexus/forward.hpp"
#include "nexus/ids.hpp"

namespace metacode
{

using _Key     = std::string_view;
using _Pattern = std::initializer_list<std::string_view>;

constexpr uint32_t k_section_size = std::numeric_limits<uint32_t>::max() / 4;

// file tokens : 1+2+3 section        |x|x|x|_|
// placeholder tokens : 3rd section   |_|_|_|x| - 1
// metacode flag : max u32            |_|_|_|_| - 1 <-
constexpr token::_id k_placeholder_flag_start = token::_id(k_section_size * 3);
constexpr token::_id k_placeholder_flag_end   = token::_id(k_section_size * 4 - 1);
constexpr token::_id k_metacode_flag          = token::_id(std::numeric_limits<uint32_t>::max());

constexpr inline size_t to_placeholder_index(token::_id id)
{
  return size_t(id.value() - k_placeholder_flag_start.value());
}

#define METACODE_SET_KIND(kind)                                                                                        \
  static constexpr EMetacodeKind static_kind = EMetacodeKind::kind;                                                    \
  kind()                                                                                                               \
    : Metacode(EMetacodeKind::kind)                                                                                    \
  {                                                                                                                    \
  }

#define METACODE_NODE(name)        _id name;
#define METACODE_VECTOR_NODE(name) std::vector<_id> name;

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
  Word(script::ScriptInfo& p_scr_info)
    : scr_info(p_scr_info)
  {
  }

  script::ScriptInfo& scr_info;

  std::vector<std::string_view> tokens;

  [[nodiscard]] bool contains(std::string_view s) const;
  [[nodiscard]] bool contains(token::ETokenKind type) const;
  [[nodiscard]] bool contains_one(const std::initializer_list<token::ETokenKind>& l) const;

  [[nodiscard]] bool have_key(const _Key& key) const;
};

enum class EPatternKey {
  None,
  Any,         // "<*>"
  Identifier,  // "<a>"
  Numeric,     // pattern_constants::numeric
  Alternative, // "<_>"
};

[[nodiscard]] EPatternKey str_to_EPatternKey(const _Key& pattern);

struct Instruction final {
  std::vector<Word> words;

  [[nodiscard]] bool             match_pattern(const _Pattern& pattern) const;
  [[nodiscard]] std::string_view at_str(size_t pos, size_t alt) const;
};


struct Metacode {
  Metacode() = default;
  struct Scope final {
    std::string name;
    size_t      start_pos = 1;
    size_t      end_pos   = 1;
    bool        is_inline = true;
  };

  _id id;

  Scope                    scope;
  std::vector<Instruction> instructions;
  size_t                   start_toks = -1;
  size_t                   end_toks   = -1;
  std::vector<token::_id>  tokens_to_generate;


  size_t position = -1;

private:
  EMetacodeKind metacode_kind = EMetacodeKind::Metablock;

protected:
  Metacode(EMetacodeKind kind)
    : metacode_kind(kind)
  {
  }

public:
  EMetacodeKind kind() const
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
  std::vector<token::_id>  body;
};

struct Cond_expr final : Metacode {
  METACODE_SET_KIND(Cond_expr)

  bool       is_constant    = false;
  bool       is_placeholder = false;
  token::_id val;
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
    std::string_view        name;
    std::vector<token::_id> variants;
  };

  std::vector<Placeholder> placeholders;
  METACODE_VECTOR_NODE(expand_conditions)

  std::string expansion;
};


template <typename T>
concept DerivedMetacode = std::is_base_of_v<Metacode, T> && (!std::is_same_v<Metacode, T>);


struct ScriptGraph final {
  struct Audit final {
    ScriptGraph& graph;

    bool               contains(_id id, _Key s) const;
    bool               contains(_id id, token::ETokenKind tok) const;
    const Instruction* get_instruction(_id start_id, _Pattern pattern) const;
    const Metacode*    get_metacode(_id start_id, _Pattern pattern) const;
    // will returns the closets metacode to file_pos
    const Metacode*    get_metacode_from_pos(size_t file_pos) const;
  };

  ScriptGraph()
    : root(new_metacode<metacode::Root>(NO_ID))
  {
  }

  Audit audit{*this};


  std::vector<Metacode*> metacodes;

  std::unordered_map<_id, _id, _id_hash>              parent;
  std::unordered_map<_id, std::vector<_id>, _id_hash> children;


  _id root;


  ~ScriptGraph()
  {
    for (auto m : metacodes) delete m;
  }

  template <DerivedMetacode T>
  _id new_metacode(_id p_parent)
  {
    const _id new_id(metacodes.size());

    T* obj  = new T();
    obj->id = new_id;
    metacodes.emplace_back(obj);

    if (p_parent) {
      parent[new_id] = p_parent;
      children[p_parent].push_back(new_id);
    }

    return new_id;
  }

  Metacode& get(_id id)
  {
    assert(id.value() < metacodes.size());
    return *metacodes[id.value()];
  }

  template <DerivedMetacode T>
  T* get_as(_id id)
  {
    Metacode* m = &get(id);
    if (!m) return nullptr;

    if (m->kind() != T::static_kind) return nullptr;

    return static_cast<T*>(m);
  }
};


} // namespace metacode