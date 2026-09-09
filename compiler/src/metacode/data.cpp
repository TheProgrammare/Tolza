#include "id/cuid.hpp"
#include "lexer/data.hpp"
#include "metacode/data.hpp"
#include "nexus/forward.hpp"

#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <string_view>

metacode::Word::Word(cu::CU& p_CU)
  : CU(p_CU)
{
}

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
    auto it = token::k_keywords.find(tok);
    if (it != token::k_keywords.end() && token::k_keywords.find(tok)->second == kind) return true;
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
bool metacode::Word::have_key(const std::string_view& key) const noexcept
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

metacode::EPatternKey metacode::str_to_EPatternKey(const std::string_view& key) noexcept
{
  if (key == pattern_constants::wildcard) return EPatternKey::Any;
  if (key == pattern_constants::identifier) return EPatternKey::Identifier;
  if (key == pattern_constants::numeric) return EPatternKey::Numeric;
  if (key == pattern_constants::alternative) return EPatternKey::Alternative;
  return EPatternKey::None;
}

bool metacode::Instruction::match_pattern(const std::initializer_list<std::string_view>& pattern) const noexcept
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
