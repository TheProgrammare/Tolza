#include "token_generator.hpp"

#include "ast/ast_base.hpp"
#include "compiler/compiler.hpp"
#include <common/compiler_options.hpp>
#include "misc/error_output.hpp"
#include "nexus/ast/ast.hpp"
#include "nexus/ids.hpp"
#include "nexus/lexer/token.hpp"
#include "nexus/metacode/metacode.hpp"
#include "compiler/compilation_unit.hpp"
#include "nexus/metacode/preprocessor.hpp"
#include <cassert>
#include <cstddef>
#include <string_view>


metacode::Generator::Generator(cu::CU& p_CU, metacode::Preprocessor& p_prepro) noexcept
  : CU(p_CU)
  , prepro(p_prepro)
{
  tokens_generated.reserve(CU.file_info.tokens->tokens.size() * 1.33);
}

std::vector<metacode::ID>* metacode::Generator::get_children(metacode::ID id) const noexcept
{
  auto it = CU.metacodes->children.find(id);
  if (it != CU.metacodes->children.end()) return &it->second;
  return nullptr;
}

metacode::Metacode& metacode::Generator::get_child(const std::vector<metacode::ID>* children,
                                                   size_t                           gen_count) const noexcept
{
  assert(children);
  assert(gen_count < children->size());

  auto child_id = (*children)[gen_count];
  return CU.metacodes->get(child_id);
}


metacode::Generator::EPostTokenKind metacode::Generator::scan_token(token::ID tok) noexcept
{
  if (tok >= 0 && tok <= k_placeholder_flag_start - 1) return EPostTokenKind::filesource;
  if (tok >= k_placeholder_flag_start && tok <= k_placeholder_flag_end) return EPostTokenKind::placeholder;
  if (tok == k_metacode_flag) return EPostTokenKind::metacode;

  return EPostTokenKind::filesource;
}

void metacode::Generator::add_token(token::Token& tok) noexcept
{
  tok.tokid = token::ID::make(CU.cuid, tokens_generated.size());
  tokens_generated.emplace_back(tok);
}


bool metacode::Generator::start_generator() noexcept
{
  auto last_err_count = compiler::COMPILER.errors.size();

  auto* m = &CU.metacodes->get_file_root();
  gen_Root(*m);

  // some tokens (metacodes + comments) are erased after the postprocessing
  // so all tokid offset are incorrects
  normalize_tokens_generated_ids();

  return last_err_count == compiler::COMPILER.errors.size();
}

void metacode::Generator::normalize_tokens_generated_ids() noexcept
{
  // change only the offset inside the tokid
  size_t count = 0;
  for (auto& tok : tokens_generated) {
    tok.tokid.set_offset(count++);
  }
}

void metacode::Generator::gen_tokens(const std::vector<token::ID>& toks) noexcept
{
  assert(current_metacode && "tokens must be generated with a metacode");

  const auto* children = get_children(current_metacode->metaid);

  size_t child_gen_count = 0;
  for (const auto& tok_id : current_metacode->tokens_to_generate) {
    auto& tok = CU.file_info.tokens->get(tok_id);

    switch (scan_token(tok_id)) {
    case EPostTokenKind::filesource: {
      tokens_generated.emplace_back(tok);
      break;
    }
    case EPostTokenKind::metacode: {
      auto& child_m = get_child(children, child_gen_count++);

      gen_Metacode(child_m);
      break;
    }
    case EPostTokenKind::placeholder: {
      if (!current_expand) {
        auto err = Error_Diagnostic(CU.cuid, 248, tok.begin, tok.begin + tok.length, compiler::EPhase::preprosessor,
                                    "Illegal placeholder outside an expand preprocessor instruction.", "");
        compiler::COMPILER.add_error(err);
        continue;
      }

      auto placeholder_id = to_placeholder_index(tok_id);

      assert(placeholder_id < current_placeholder_env.size());

      auto& tok_var = CU.file_info.tokens->get(current_placeholder_env[placeholder_id]);
      add_token(tok_var);
    }
    }
  }
}

void metacode::Generator::gen_Metacode(metacode::Metacode& m) noexcept
{
  switch (m.kind()) {
  case EMetacodeKind::Root:      gen_Root(*CU.metacodes->as<metacode::Root>(m.metaid)); break;
  case EMetacodeKind::Metablock: gen_Metablock(*CU.metacodes->as<metacode::Metablock>(m.metaid)); break;
  case EMetacodeKind::If:        gen_If(*CU.metacodes->as<metacode::If>(m.metaid)); break;
  case EMetacodeKind::Expand:    gen_Expand(*CU.metacodes->as<metacode::Expand>(m.metaid)); break;
  default:                       assert(false);
  }
}
void metacode::Generator::gen_Root(metacode::Root& m) noexcept
{
  size_t child_gen_count = 0;

  current_metacode = &m;
  gen_tokens(m.tokens_to_generate);
  current_metacode = nullptr;
}
void metacode::Generator::gen_Metablock(metacode::Metablock& m) noexcept
{
}
void metacode::Generator::gen_If(metacode::If& m) noexcept
{
  auto* old_metacode = current_metacode;
  current_metacode   = &m;

  if (m.is_else || eval_cond(m.condition))
    gen_tokens(m.tokens_to_generate);
  else if (m.alternative)
    gen_Metacode(CU.metacodes->get(m.alternative));

  current_metacode = old_metacode;
}
void metacode::Generator::gen_Expand(metacode::Expand& m) noexcept
{
  size_t child_gen_count = 0;

  auto* old_current_metacode = current_metacode;
  current_metacode           = &m;
  auto* old_current_expand   = current_expand;
  current_expand             = &m;


  std::vector<size_t> bases;
  bases.reserve(m.placeholders.size());
  for (const auto& p : m.placeholders) bases.emplace_back(p.variants.size());

  size_t total = 1;
  for (auto b : bases) total *= b;

  std::vector<size_t> idx(bases.size());

  for (size_t i = 0; i < total; i++) {
    size_t r = i;

    for (size_t j = 0; j < bases.size(); j++) {
      idx[j] = r % bases[j];
      r /= bases[j];
    }

    metacode::Env env;
    env.reserve(m.placeholders.size());
    for (size_t j = 0; j < m.placeholders.size(); j++) {
      const auto& ph = m.placeholders[j];

      env.emplace_back(ph.variants[idx[j]]);
    }

    current_placeholder_env = env;

    gen_tokens(m.tokens_to_generate);
  }


  current_metacode = old_current_metacode;
  current_expand   = old_current_expand;
}

bool metacode::Generator::eval_cond(metacode::ID id) noexcept
{
  auto& c = CU.metacodes->get(id);
  switch (c.kind()) {
  case EMetacodeKind::Binary_Cond:         return eval_binary(*CU.metacodes->as<metacode::Binary_Cond>(c.metaid));
  case EMetacodeKind::Unary_Not_Cond:      return eval_not_unary(*CU.metacodes->as<metacode::Unary_Not_Cond>(c.metaid));
  case metacode::EMetacodeKind::Cond_expr: {
  }
  default: assert(false);
  }
}
bool metacode::Generator::eval_binary(metacode::Binary_Cond& c) noexcept
{
  switch (c.type) {
  case ast::EOp_Bin::_eq:   return eval_expr(c.left) == eval_expr(c.right);
  case ast::EOp_Bin::_neq:  return eval_expr(c.left) != eval_expr(c.right);
  case ast::EOp_Bin::_and:  return eval_cond(c.left) && eval_cond(c.right);
  case ast::EOp_Bin::_nand: return !(eval_cond(c.left) && eval_cond(c.right));
  case ast::EOp_Bin::_or:   return eval_cond(c.left) || eval_cond(c.right);
  case ast::EOp_Bin::_xor:  return !eval_cond(c.left) != !eval_cond(c.right);
  case ast::EOp_Bin::_xnor: return eval_cond(c.left) && eval_cond(c.right);
  default:                  assert(false && "Illegal bin op type in preprocessor instruction");
  }
}
bool metacode::Generator::eval_not_unary(metacode::Unary_Not_Cond& c) noexcept
{
  return !eval_cond(c.term);
}
std::string metacode::Generator::eval_expr(metacode::ID id) noexcept
{
  const auto* expr = CU.metacodes->as<metacode::Cond_expr>(id);

  const auto& m            = CU.metacodes->get(id);
  auto        line         = CU.file_info.get_line_from_pos(m.position);
  auto        line_end_pos = CU.file_info.get_line_end(line);

  if (!expr) {
    auto err = Error_Diagnostic(CU.cuid, 247, m.position, line_end_pos, compiler::EPhase::preprosessor,
                                "Expected a preprocessor expression", "");
    compiler::COMPILER.add_error(err);

    return "0";
  }

  if (expr->is_constant) {
    auto str = std::string(CU.file_info.tokens->audit.Token_to_str(expr->val));
    return str;
  }

  if (expr->is_placeholder) {
    size_t p_id = to_placeholder_index(expr->val);
    if (!current_expand) {
      auto err = Error_Diagnostic(CU.cuid, 248, m.position, line_end_pos, compiler::EPhase::preprosessor,
                                  "Illegal placeholder outside an expand preprocessor instruction.", "");
      compiler::COMPILER.add_error(err);
      return "";
    }

    assert(p_id < current_placeholder_env.size() && "Placeholder index out of bound");

    auto p   = current_placeholder_env[p_id];
    auto str = std::string(CU.file_info.tokens->audit.Token_to_str(p));
    return str;
  }

  auto& prepro_args = compiler::OPTIONS.PREPROCESSOR_ARGS;
  auto  str         = std::string(CU.file_info.tokens->audit.Token_to_str(expr->val));

  if (auto it = prepro_args.find(str); it != prepro_args.end()) {
    return it->second;
  }

  // no preprocessor found : return 0
  return "0";
}