/*
 *	The Velox programming language - Apache License, Version 2.0
 *  Copyright 2024-2026 Foz Florian
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#pragma once

#include <map>
#include <memory>
#include <set>
#include <unordered_map>
#include <vector>

#include "AST/AST_Base.hpp"
#include "Lexer/Token.hpp"

constexpr size_t k_metacode_flag = 18446744073709551614ULL;

struct ScriptInfo;

namespace META
{

// one word (one token or list of alternative tokens) of instruction line
struct MetaWord {
  std::vector<Token> tokens;

  MetaWord(const Token& tok) : tokens({tok}) {}
  MetaWord(const std::vector<Token>& tok) : tokens(tok) {}

  [[nodiscard]]
  bool is_alternative() const
  {
    return tokens.size() > 1;
  }
  [[nodiscard]]
  bool contains(TokTy type) const
  {
    for (auto& tok : tokens) {
      if (tok.type == type) return true;
    }
    return false;
  }
  [[nodiscard]]
  bool contains(const std::string& s) const
  {
    for (auto& tok : tokens) {
      if (tok.val == s) return true;
    }
    return false;
  }
  [[nodiscard]]
  bool contains_one(const std::initializer_list<TokTy>& l) const
  {
    for (const TokTy& m_elem : l) {
      if (contains(m_elem)) return true;
    }
    return false;
  }
  [[nodiscard]]
  Token at_tok(size_t x) const
  {
    if (tokens.size() > x) return tokens[x];
    return Token();
  }
  [[nodiscard]]
  std::string at_str(size_t x) const
  {
    if (tokens.size() > x) return tokens[x].val;
    return "";
  }

  std::string debug_str() const
  {
    std::string str;
    size_t      count = 0;
    for (auto tok : tokens) {
      str += tok.val;
      if (++count >= tokens.size()) break;
      str += " | ";
    }
    return str;
  }
};

enum struct EPatternKey {
  None,
  Any,         // "<*>"
  Identifier,  // "<a>"
  Numeric,     // "<0>"
  Alternative, // "<_>"
};

[[nodiscard]]
inline EPatternKey get_pattern_key(const std::string& key)
{
  if (key == "<*>") return EPatternKey::Any;
  if (key == "<a>") return EPatternKey::Identifier;
  if (key == "<0>") return EPatternKey::Numeric;
  if (key == "<_>") return EPatternKey::Alternative;
  return EPatternKey::None;
}

[[nodiscard]]
inline bool check_pattern(const MetaWord& word, const std::string& pattern)
{
  EPatternKey pattern_key = get_pattern_key(pattern);
  switch (pattern_key) {
  case EPatternKey::Any:         return true;
  case EPatternKey::Identifier:  return word.contains(TokTy::IDENTIFIER);
  case EPatternKey::Numeric:     return word.contains_one(kNumericTypeTokens);
  case EPatternKey::Alternative: return word.is_alternative();
  case EPatternKey::None:        return word.contains(pattern);
  }
}

// forward declaration
struct MetaBlock;

// one instruction line of metablock
struct MetaInstruct {
  std::vector<MetaWord> words;
  // parent code block of the instruction
  MetaBlock*            code_block = nullptr;

  MetaInstruct(MetaBlock* code_block, const std::vector<MetaWord>& words) : code_block(code_block), words(words) {}

  [[nodiscard]]
  bool match_pattern(const std::vector<std::string>& pattern) const
  {
    if (pattern.empty()) return false;

    bool explicit_end = pattern.back() == "<!>";

    if (explicit_end && pattern.size() - 1 != words.size()) return false;
    if (!explicit_end && pattern.size() != words.size()) return false;

    for (size_t i = 0; i < pattern.size(); i++) {
      if (explicit_end && i == pattern.size() - 1) return true;
      if (!check_pattern(words[i], pattern[i])) return false;
    }
    return true;
  }
  [[nodiscard]]
  // returns the first occurence index
  // -1 = not found
  int8_t contains(const std::string& s) const
  {
    int8_t count = 0;
    for (auto& word : words) {
      if (word.contains(s)) return count;
      count++;
    }
    return -1;
  }
  [[nodiscard]]
  // returns the first occurence index
  // -1 = not found
  int8_t contains(TokTy type) const
  {
    int8_t count = 0;
    for (auto& word : words) {
      if (word.contains(type)) return count;
      count++;
    }
    return -1;
  }
  [[nodiscard]]
  std::string at_str(size_t x, size_t alt = 0) const
  {
    if (words.size() > x) return words[x].at_str(alt);
    return "";
  }
  [[nodiscard]]
  Token at_tok(size_t x, size_t alt = 0) const
  {
    if (words.size() > x) return words[x].at_tok(alt);
    return Token();
  }

  std::string debug_str()
  {
    std::string str = "# ";
    for (auto& word : words) {
      str += word.debug_str();
      str += " ";
    }
    return str;
  }
};

// scope of code block
struct MetaScope {
  std::string name;
  size_t      start_scope_position = 1;
  size_t      end_scope_position   = 1;
  // subline scope don't keep track of the token generation flow
  bool        is_subline_scope     = true;

  [[nodiscard]]
  bool is_valid() const
  {
    return start_scope_position < end_scope_position;
  }
};

// code block
struct MetaBlock {
  [[maybe_unused]]
  MetaBlock*                parent = nullptr;
  // if scoped
  MetaScope                 _scope;
  // all instructions
  std::vector<MetaInstruct> _instructions;

  // anteprocess indexation reference
  std::vector<size_t> tokens_to_generate;

  std::string debug_str;

  size_t position;

  // scope tok pos, codeblock
  std::vector<std::unique_ptr<MetaBlock>> _childrens;

  virtual ~MetaBlock() = default;

  void add_children(std::unique_ptr<MetaBlock> cb)
  {
    cb->parent = this;
    _childrens.push_back(std::move(cb));
    tokens_to_generate.push_back(k_metacode_flag); // mark when a children must be generated before
                                                   // continue the rest of tokens
  }

  [[nodiscard]]
  virtual std::vector<Token> generate_tokens(ScriptInfo& scr_info) const;

  [[nodiscard]]
  bool is_scoped() const
  {
    return _scope.is_valid();
  }

  [[nodiscard]]
  bool is_pos_concerned(int pos) const
  {
    if (pos < 1) return false;

    return pos >= _scope.start_scope_position && pos <= _scope.end_scope_position;
  }

  [[nodiscard]]
  bool must_generate_code() const
  {
    return !tokens_to_generate.empty();
  }

  [[nodiscard]]
  bool contains(const std::string& s) const
  {
    for (auto& elem : _instructions) {
      if (elem.contains(s)) return true;
    }
    return false;
  }
  [[nodiscard]]
  bool contains(TokTy type) const
  {
    for (auto& elem : _instructions) {
      if (elem.contains(type)) return true;
    }
    return false;
  }

  [[nodiscard]]
  bool contains(const std::initializer_list<std::string>& pattern) const
  {
    for (auto& elem : _instructions) {
      if (elem.match_pattern(pattern)) return true;
    }
    return false;
  }

  [[nodiscard]]
  std::vector<const MetaInstruct*> match_pattern(const std::initializer_list<std::string>& pattern) const
  {
    std::vector<const MetaInstruct*> founds;
    for (auto& elem : _instructions) {
      if (elem.match_pattern(pattern)) founds.push_back(&elem);
    }
    return founds;
  }

  [[nodiscard]]
  std::string at_str(size_t x, size_t alt = 0) const
  {
    for (auto& elem : _instructions) {
      if (elem.words.size() > x) return elem.words[x].at_str(alt);
    }
    return "";
  }
  [[nodiscard]]
  Token at_tok(size_t x, size_t alt = 0) const
  {
    for (auto& elem : _instructions) {
      if (elem.words.size() > x) return elem.words[x].at_tok(alt);
    }
    return Token();
  }
};

// to create node, set some data, store in resolvers
template <typename MetaType>
concept DerivedFromMeta = std::is_base_of_v<MetaBlock, MetaType>;

// AST param imitation
struct MetaBlock_Reuse_Param {
  enum struct EPassMode { Any, Mut, Ref, Copy, Move, Comptime, Addr };

  EPassMode                                   pass_mode = EPassMode::Copy;
  std::unique_ptr<AST::AType>                 type;
  [[maybe_unused]] std::unique_ptr<AST::Node> defaultValue;
  bool                                        is_variadic = false;

  bool operator==(const MetaBlock_Reuse_Param& other) const
  {
    return pass_mode == other.pass_mode && type == other.type && is_variadic == other.is_variadic;
  }
};

bool is_equivalent_ReusableBlock_Param(const MetaBlock_Reuse_Param& a, const MetaBlock_Reuse_Param& b);

// block reuable (with name and params)
struct MetaBlock_Reuse : public MetaBlock {
  std::string                        name;
  std::vector<MetaBlock_Reuse_Param> params;

  [[nodiscard]]
  bool is_scoped() const
  {
    return _scope.is_valid();
  }
};
// Node de base
struct Cond_Base {
  virtual ~Cond_Base() = default;

  [[nodiscard]]
  virtual bool eval(const std::multimap<std::string, std::string>& ctx) const = 0;
  [[nodiscard]]
  virtual std::string print_eval() const = 0;
};

// Comparaison simple : _T == "i128"
struct Cond_Eq : Cond_Base {
  std::string placeholder;
  std::string value;
  bool        isNot = false;

  Cond_Eq(const std::string& ph, const std::string& val, bool isN) : placeholder(ph), value(val), isNot(isN) {}

  // placeholders values
  // placeholder_name == place_holder_symbol
  // _T == "i32"
  [[nodiscard]]
  bool eval(const std::multimap<std::string, std::string>& ctx) const override
  {
    auto range = ctx.equal_range(placeholder);
    if (range.first == range.second) return isNot; // no key -> if negate: is "true"

    for (auto it = range.first; it != range.second; ++it) {
      bool eq = (it->second == value);
      if (!isNot && eq) return true; // _T == value
      if (isNot && eq) return false; // _T != value -> find: so "false"
    }

    // No corresponding value found :
    return isNot; // if !=, so "not found" = "true"
  }
  [[nodiscard]]
  std::string print_eval() const override
  {
    if (isNot)
      return placeholder + " != " + value;
    else
      return placeholder + " == " + value;
  }
};

// ET logique
struct Cond_And : Cond_Base {
  std::shared_ptr<Cond_Base> lhs;
  std::shared_ptr<Cond_Base> rhs;

  Cond_And(std::shared_ptr<Cond_Base> l, std::shared_ptr<Cond_Base> r) : lhs(l), rhs(r) {}

  [[nodiscard]]
  bool eval(const std::multimap<std::string, std::string>& ctx) const override
  {
    return lhs->eval(ctx) && rhs->eval(ctx);
  }
  [[nodiscard]]
  std::string print_eval() const override
  {
    return lhs->print_eval() + " and " + rhs->print_eval();
  }
};

// OU logique
struct Cond_Or : Cond_Base {
  std::shared_ptr<Cond_Base> lhs;
  std::shared_ptr<Cond_Base> rhs;

  Cond_Or(std::shared_ptr<Cond_Base> l, std::shared_ptr<Cond_Base> r) : lhs(l), rhs(r) {}

  [[nodiscard]]
  bool eval(const std::multimap<std::string, std::string>& ctx) const override
  {
    return lhs->eval(ctx) || rhs->eval(ctx);
  }
  [[nodiscard]]
  std::string print_eval() const override
  {
    return lhs->print_eval() + " or " + rhs->print_eval();
  }
};

// Négation
struct Cond_Not : Cond_Base {
  std::shared_ptr<Cond_Base> child;

  Cond_Not(std::shared_ptr<Cond_Base> c) : child(c) {}
  [[nodiscard]]
  bool eval(const std::multimap<std::string, std::string>& ctx) const override
  {
    return !child->eval(ctx);
  }
  [[nodiscard]]
  std::string print_eval() const override
  {
    return "!(" + child->print_eval() + ")";
  }
};

// block with condition
struct MetaBlock_If : public MetaBlock {
  // if nullptr : else with no condition
  [[maybe_unused]]
  std::unique_ptr<Cond_Base> condition;
  [[maybe_unused]]
  std::unique_ptr<MetaBlock_If> alternative = nullptr;
  enum struct EFlowType { IF, ELIF, ELSE };
  EFlowType flow_type = EFlowType::IF;

  [[nodiscard]]
  std::vector<Token> generate_tokens(ScriptInfo& scr_info) const override;

  [[nodiscard]]
  MetaBlock_If* get_alternative() const
  {
    return alternative.get();
  }

  [[nodiscard]]
  std::string print_type() const
  {
    switch (flow_type) {
    case EFlowType::IF:   return "if";
    case EFlowType::ELIF: return "elif";
    case EFlowType::ELSE: return "else";
    }
  }

  [[nodiscard]]
  bool eval(const std::multimap<std::string, std::string>& ctx) const
  {
    if (condition)
      return condition->eval(ctx);
    else
      return true;
  }
  [[nodiscard]]
  bool eval_placeholders(const std::map<std::string, Token>& ctx) const;
  [[nodiscard]]
  std::string print_eval() const
  {
    return condition->print_eval();
  }
};

// expansion condition
struct Expand_If : public MetaBlock {
  // if nullptr : else with no condition
  [[maybe_unused]]
  std::unique_ptr<Cond_Base> condition;
  [[maybe_unused]]
  std::unique_ptr<Expand_If> alternative = nullptr;
  enum struct EFlowType { IF, ELIF, ELSE };
  EFlowType flow_type = EFlowType::IF;

  [[nodiscard]]
  Expand_If* get_alternative() const
  {
    return alternative.get();
  }

  [[nodiscard]]
  std::string print_type() const
  {
    switch (flow_type) {
    case EFlowType::IF:   return "if";
    case EFlowType::ELIF: return "elif";
    case EFlowType::ELSE: return "else";
    }
  }

  [[nodiscard]]
  bool eval(const std::map<std::string, Token>& ctx) const;
  [[nodiscard]]
  std::string print_eval() const
  {
    return condition->print_eval();
  }
};

// for _T as i32 | i64 | ...
struct MetaBlock_Expand : public MetaBlock {
  // convention : corresponding placeholder identifier token next after the flag
  static const size_t k_placeholder_flag;
  static const size_t k_expand_if_flag;

  // placeholder name, alts sym
  std::map<std::string, std::vector<Token>> placeholders;
  std::vector<std::unique_ptr<Expand_If>>   expand_conditions;
  // code model position, placeholder target
  std::unordered_map<size_t, std::string>   placeholders_pos;

  std::vector<Token> tokens_expanded;

  std::vector<Token> generate_tokens(ScriptInfo& scr_info) const override;
  // result of generate tokens
  std::vector<Token> generate_model_expansion(ScriptInfo& scr_info, std::vector<Token>& model) const;

  void add_expand_condition(std::unique_ptr<Expand_If> exp_cond);

  [[nodiscard]]
  bool is_valid_placeholder_name(const std::string& name) const
  {
    return placeholders.contains(name);
  }

  [[nodiscard]]
  std::set<std::string> get_placeholder_names() const
  {
    std::set<std::string> names;
    for (auto& elem : placeholders) {
      // for _T as ... | ...
      names.insert(elem.first);
    }
    return names;
  }

  [[nodiscard]]
  size_t get_expansion_count() const
  {
    size_t count = 1;
    for (auto& elem : placeholders) {
      // for _T as ... | ...
      count *= elem.second.size();
    }
    return count;
  }

  void generate_combinations(std::map<std::string, Token>&                             current,
                             std::map<std::string, std::vector<Token>>::const_iterator it,
                             std::vector<std::map<std::string, Token>>&                result) const;

  // Generic wrapper
  std::vector<std::map<std::string, Token>> generate_all_combinations() const;
};

// manage all metablocks
struct MetablockManager {
  // key: position, val: MetaBlock
  std::map<size_t, MetaBlock*> metablocks;

  // Avoid memory destruction
  MetaBlock root_metabock;

  [[nodiscard]]
  const MetaBlock* get_closest_metablock_at(size_t pos) const
  {
    // invalid position
    if (pos < 1) return nullptr;
    // no metablocks
    if (metablocks.empty()) return nullptr;

    // get the map iterator the closets superior to the position
    auto it = metablocks.lower_bound(pos);

    // we need the back metablock because
    // metablocks impacts the next position not the back position

    // metablocks keys represent start positions; a metablock applies
    // from its key up to its logical end
    if (it == metablocks.begin()) return nullptr;

    auto mb = (--it)->second; // go back to get the key < pos

    // check the metablock scope valid
    if (!mb->is_pos_concerned(pos)) return nullptr;

    return mb;
  }

  [[nodiscard]]
  std::vector<const MetaBlock*> get_metablocks_parent_cascade(const MetaBlock* mb) const
  {
    if (!mb) return {};

    std::vector<const MetaBlock*> result;
    result.reserve(8);

    auto current_mb = mb;
    while (current_mb) {
      // save metablock to the result
      result.push_back(current_mb);
      // go to the parent to check if another metablock to add
      current_mb = current_mb->parent;
    }

    return result;
  }

  void filter_metablocks_parent_cascade(std::vector<const MetaBlock*>& input) const
  {
    const std::vector<const MetaBlock*>& tmp = input;

    std::vector<const MetaBlock*> tmp_first_filter;
    tmp_first_filter.reserve(tmp.size());
    std::set<std::string> mb_scopes_excluded;

    for (auto& mb : tmp) {
      if (!mb->match_pattern({"exclude", "all", "<!>"}).empty()) {
        tmp_first_filter.push_back(mb);

        // the rest of parent metablocks must be excluded
        break;
      }
      // explicit specific scope exclusion
      else if (auto pattern = mb->match_pattern({"exclude", "<*>", "<!>"}); !pattern.empty()) {
        // parent scope name excluded
        std::string exclusion_name = pattern[0]->at_str(0, 0);

        mb_scopes_excluded.insert(exclusion_name);
      }

      tmp_first_filter.push_back(mb);
    }

    input.clear();
    input.reserve(tmp_first_filter.size());

    for (auto& mb : tmp_first_filter) {
      const std::string& mb_scope_name = mb->_scope.name;

      // mb is excluded, don't add it
      if (!mb_scope_name.empty() && mb_scopes_excluded.contains(mb_scope_name)) continue;

      input.push_back(mb);
    }
  }

  // will try to get all metablocks who have a influence to the specified position
  // handle the metablock scoped exclusion
  [[nodiscard]]
  std::vector<const MetaBlock*> get_metablocks_at(size_t pos) const
  {

    // while nested metablock exists
    auto closest_mb = get_closest_metablock_at(pos);

    // temporary list of metablocks found
    std::vector<const MetaBlock*> tmp = get_metablocks_parent_cascade(closest_mb);

    filter_metablocks_parent_cascade(tmp);

    return tmp;
  }

  [[nodiscard]]
  // get first codeblock directly
  const MetaBlock* get_metablock(size_t pos, const std::initializer_list<std::string>& pattern) const
  {
    auto line_metas = get_metablocks_at(pos);
    for (const MetaBlock* meta : line_metas) {
      if (!meta->match_pattern(pattern).empty()) return meta;
    }
    return nullptr;
  }

  [[nodiscard]]
  // get first instruction directly
  const MetaInstruct* get_instruct(size_t pos, const std::initializer_list<std::string>& pattern) const
  {
    auto line_metas = get_metablocks_at(pos);
    for (const MetaBlock* meta : line_metas) {
      if (auto instruct = meta->match_pattern(pattern); !instruct.empty()) return instruct[0];
    }
    return nullptr;
  }

  [[nodiscard]]
  bool contains(size_t pos, const std::string& s) const
  {
    auto line_metas = get_metablocks_at(pos);
    for (const MetaBlock* meta : line_metas) {
      if (meta->contains(s)) return true;
    }
    return false;
  }

  [[nodiscard]]
  bool contains(size_t pos, TokTy t) const
  {
    auto line_metas = get_metablocks_at(pos);
    for (const MetaBlock* meta : line_metas) {
      if (meta->contains(t)) return true;
    }
    return false;
  }

  [[nodiscard]]
  std::string get_export_name(size_t pos) const
  {
    // named export
    if (auto cb = get_metablock(pos, {"export", "<*>", "<!>"})) {
      return cb->at_str(1, 0);
    } else if (auto cb = get_metablock(pos, {"export", "module", "<*>", "<!>"})) {
      return cb->at_str(2, 0);
    }
    // export to the first module
    else if (get_metablock(pos, {"export", "<!>"})) {
      return "export";
    }
    return "";
  }
};
} // namespace META
