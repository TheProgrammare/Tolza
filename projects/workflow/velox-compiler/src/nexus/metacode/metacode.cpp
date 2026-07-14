#include "metacode.hpp"

#include "nexus/lexer/token.hpp"
#include <cassert>


bool metacode::Graph::Audit::contains(ID id, MetaKey s) const noexcept
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
const metacode::Instruction* metacode::Graph::Audit::get_instruction(ID start_id, MetaPattern pattern) const noexcept
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
const metacode::Metacode* metacode::Graph::Audit::get_metacode(ID start_id, MetaPattern pattern) const noexcept
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
