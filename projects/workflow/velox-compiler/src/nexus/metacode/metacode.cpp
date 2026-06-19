#include "metacode.hpp"

#include "nexus/lexer/token.hpp"
#include <cassert>

bool metacode::Word::contains(std::string_view s) const noexcept
{
  for (const auto& tok : tokens) {
    if (tok == s) return true;
  }
  return false;
}
bool metacode::Word::contains(token::ETokenKind kind) const noexcept
{
  for (const auto& tok : tokens) {
    if (token::str_to_ETokenKind(tok) == kind) return true;
  }
  return false;
}
bool metacode::Word::contains_one(const std::initializer_list<token::ETokenKind>& l) const noexcept
{
  for (const token::ETokenKind& m_elem : l) {
    if (contains(m_elem)) return true;
  }
  return false;
}
bool metacode::Word::have_key(const _Meta_Key& key) const noexcept
{
  EPatternKey pattern_key = str_to_EPatternKey(key);
  switch (pattern_key) {
  case EPatternKey::Any:         return true;
  case EPatternKey::Identifier:  return contains(::token::ETokenKind::IDENTIFIER);
  case EPatternKey::Numeric:     return contains_one(::token::k_type_numeric);
  case EPatternKey::Alternative: return tokens.size() > 0;
  case EPatternKey::None:        return contains(key);
  }
}

metacode::EPatternKey metacode::str_to_EPatternKey(const _Meta_Key& key) noexcept
{
  if (key == pattern_constants::wildcard) return EPatternKey::Any;
  if (key == pattern_constants::identifier) return EPatternKey::Identifier;
  if (key == pattern_constants::numeric) return EPatternKey::Numeric;
  if (key == pattern_constants::alternative) return EPatternKey::Alternative;
  return EPatternKey::None;
}

bool metacode::Instruction::match_pattern(const _Meta_Pattern& pattern) const noexcept
{
  if (pattern.size() == 0) return false;

  bool explicit_end = *(pattern.end() - 1) == pattern_constants::end;

  if (explicit_end && pattern.size() - 1 != words.size()) return false;
  if (!explicit_end && pattern.size() != words.size()) return false;

  size_t count = 0;
  for (const auto& pat : pattern) {
    if (explicit_end && count == pattern.size() - 1) return true;
    if (!words[count].have_key(pat)) return false;

    count++;
  }

  return false;
}

std::string_view metacode::Instruction::at_str(size_t pos, size_t alt) const noexcept
{
  assert(words.size() > pos);

  auto word = words[pos];

  assert(word.tokens.size() > alt);

  return word.tokens[alt];
}


bool metacode::Graph::Audit::contains(ID id, _Meta_Key s) const noexcept
{
  auto meta = graph.get(id);

  for (const auto& ins : meta.instructions) {
    for (const auto& word : ins.words) {
      if (word.contains(s)) return true;
    }
  }

  return false;
}
bool metacode::Graph::Audit::contains(ID id, token::ETokenKind tok) const noexcept
{
  auto meta = graph.get(id);

  for (const auto& ins : meta.instructions) {
    for (const auto& word : ins.words) {
      if (word.contains(tok)) return true;
    }
  }

  return false;
}
const metacode::Instruction* metacode::Graph::Audit::get_instruction(ID start_id, _Meta_Pattern pattern) const noexcept
{
  const auto* cur_meta = &graph.get(start_id);

  while (cur_meta) {
    for (const auto& ins : cur_meta->instructions) {
      if (ins.match_pattern(pattern)) return &ins;
    }

    auto it = graph.parent.find(start_id);
    if (it != graph.parent.end())
      cur_meta = &graph.get(it->second);
    else
      return nullptr;
  }
  return nullptr;
}
const metacode::Metacode* metacode::Graph::Audit::get_metacode(ID start_id, _Meta_Pattern pattern) const noexcept
{
  const auto* cur_meta = &graph.get(start_id);

  while (cur_meta) {
    for (const auto& ins : cur_meta->instructions) {
      if (ins.match_pattern(pattern)) return cur_meta;
    }

    auto it = graph.parent.find(start_id);
    if (it != graph.parent.end())
      cur_meta = &graph.get(it->second);
    else
      return nullptr;
  }
  return nullptr;
}


const metacode::Metacode* metacode::Graph::Audit::get_metacode_from_pos(size_t file_pos) const noexcept
{
  // root
  auto* last = &graph.get(ID::make(0));

  for (auto* meta : graph.metacodes) {
    if (meta->start_toks > file_pos) continue;
    if (meta->end_toks < file_pos) continue;

    // meta more close to file_pos than last
    if (last->start_toks < meta->start_toks) last = meta;
  }

  return last;
}
