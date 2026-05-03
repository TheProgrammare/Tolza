#include "token_generator.hpp"

#include "ast/ast_base.hpp"
#include "compiler/compiler.hpp"
#include "compiler_options.hpp"
#include "misc/error_output.hpp"
#include "nexus/ast/ast.hpp"
#include "nexus/lexer/token.hpp"
#include "nexus/metacode/metacode.hpp"
#include "nexus/script.hpp"
#include "nexus/metacode/preprocessor.hpp"
#include <cassert>
#include <cstddef>
#include <string_view>


metacode::Generator::Generator(script::ScriptInfo& p_scr_info, metacode::Preprocessor& p_prepro)
  : scr_info(p_scr_info)
  , prepro(p_prepro)
{
  tokens_generated.reserve(scr_info.file_info.tokens->tokens.size() * 1.33);
}

std::vector<metacode::_id>* metacode::Generator::get_children(metacode::_id id)
{
  auto it = scr_info.metacodes->children.find(id);
  if (it != scr_info.metacodes->children.end()) return &it->second;
  return nullptr;
}

metacode::Metacode& metacode::Generator::get_child(const std::vector<metacode::_id>* children, size_t gen_count)
{
  assert(children);
  assert(gen_count < children->size());

  auto& child_id = (*children)[gen_count];
  return scr_info.metacodes->get(child_id);
}


metacode::Generator::EPostTokenKind metacode::Generator::scan_token(token::_id tok)
{
  if (tok >= 0 && tok <= k_placeholder_flag_start - 1) return EPostTokenKind::filesource;
  if (tok >= k_placeholder_flag_start && tok <= k_placeholder_flag_end) return EPostTokenKind::placeholder;
  if (tok == k_metacode_flag) return EPostTokenKind::metacode;

  return EPostTokenKind::filesource;
}

void metacode::Generator::add_token(token::Token& tok)
{
  tok.id = token::_id(tokens_generated.size());
  tokens_generated.push_back(tok);
}


bool metacode::Generator::start_generator()
{
  auto last_err_count = compiler::COMPILER.errors.size();

  auto m = scr_info.metacodes->get_as<metacode::Root>(scr_info.root_metacode_id);
  assert(m);

  gen_Root(*m);

  return last_err_count == compiler::COMPILER.errors.size();
}

void metacode::Generator::gen_tokens(const std::vector<token::_id>& toks)
{
  assert(current_metacode && "tokens must be generated with a metacode");

  auto children = get_children(current_metacode->id);

  size_t child_gen_count = 0;
  for (auto& tok_id : current_metacode->tokens_to_generate) {
    auto& tok = scr_info.file_info.tokens->get(tok_id);

    switch (scan_token(tok_id)) {
    case EPostTokenKind::filesource: {
      tokens_generated.push_back(tok);
      break;
    }
    case EPostTokenKind::metacode: {
      auto& child_m = get_child(children, child_gen_count++);

      gen_Metacode(child_m);
      break;
    }
    case EPostTokenKind::placeholder: {
      if (!current_expand) {
        auto err = Error_Diagnostic(scr_info.id, 248, tok.begin, tok.begin + tok.length, compiler::EPhase::preprosessor,
                                    "Illegal placeholder outside an expand preprocessor instruction.", "");
        compiler::COMPILER.add_error(std::move(err));
        continue;
      }

      auto placeholder_id = to_placeholder_index(tok_id);

      assert(placeholder_id < current_placeholder_env.size());

      auto& tok_var = scr_info.file_info.tokens->get_mut(current_placeholder_env[placeholder_id]);
      add_token(tok_var);
    }
    }
  }
}

void metacode::Generator::gen_Metacode(metacode::Metacode& m)
{
  switch (m.kind()) {
  case EMetacodeKind::Root:      gen_Root(*scr_info.metacodes->get_as<metacode::Root>(m.id)); break;
  case EMetacodeKind::Metablock: gen_Metablock(*scr_info.metacodes->get_as<metacode::Metablock>(m.id)); break;
  case EMetacodeKind::If:        gen_If(*scr_info.metacodes->get_as<metacode::If>(m.id)); break;
  case EMetacodeKind::Expand:    gen_Expand(*scr_info.metacodes->get_as<metacode::Expand>(m.id)); break;
  default:                       assert(false);
  }
}
void metacode::Generator::gen_Root(metacode::Root& m)
{
  size_t child_gen_count = 0;

  current_metacode = &m;
  gen_tokens(m.tokens_to_generate);
  current_metacode = nullptr;
}
void metacode::Generator::gen_Metablock(metacode::Metablock& m)
{
}
void metacode::Generator::gen_If(metacode::If& m)
{
  auto children = get_children(m.id);

  auto old_metacode = current_metacode;
  current_metacode  = &m;

  if (m.is_else)
    gen_tokens(m.tokens_to_generate);
  else if (eval_cond(m.condition))
    gen_tokens(m.tokens_to_generate);
  else if (m.alternative)
    gen_Metacode(scr_info.metacodes->get(m.alternative));

  current_metacode = old_metacode;
}
void metacode::Generator::gen_Expand(metacode::Expand& m)
{
  size_t child_gen_count = 0;

  auto children             = get_children(m.id);
  auto old_current_metacode = current_metacode;
  current_metacode          = &m;
  auto old_current_expand   = current_expand;
  current_expand            = &m;


  std::vector<size_t> bases;
  for (auto& p : m.placeholders) bases.push_back(p.variants.size());

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

      env.push_back(ph.variants[idx[j]]);
    }

    current_placeholder_env = env;

    gen_tokens(m.tokens_to_generate);
  }


  current_metacode = old_current_metacode;
  current_expand   = old_current_expand;
}

bool metacode::Generator::eval_cond(metacode::_id id)
{
  auto& c = scr_info.metacodes->get(id);
  switch (c.kind()) {
  case EMetacodeKind::Binary_Cond: return eval_binary(*scr_info.metacodes->get_as<metacode::Binary_Cond>(c.id));
  case EMetacodeKind::Unary_Not_Cond:
    return eval_not_unary(*scr_info.metacodes->get_as<metacode::Unary_Not_Cond>(c.id));
  case metacode::EMetacodeKind::Cond_expr: {
  }
  default: assert(false);
  }
}
bool metacode::Generator::eval_binary(metacode::Binary_Cond& c)
{
  switch (c.type) {
  case ast::EBinOpType::_eq:   return eval_expr(c.left) == eval_expr(c.right);
  case ast::EBinOpType::_neq:  return eval_expr(c.left) != eval_expr(c.right);
  case ast::EBinOpType::_and:  return eval_cond(c.left) && eval_cond(c.right);
  case ast::EBinOpType::_nand: return !(eval_cond(c.left) && eval_cond(c.right));
  case ast::EBinOpType::_or:   return eval_cond(c.left) || eval_cond(c.right);
  case ast::EBinOpType::_xor:  return !eval_cond(c.left) != !eval_cond(c.right);
  case ast::EBinOpType::_xnor: return !(!eval_cond(c.left) || !eval_cond(c.right));
  default:                     assert(false && "Illegal bin op type in preprocessor instruction");
  }
}
bool metacode::Generator::eval_not_unary(metacode::Unary_Not_Cond& c)
{
  return !eval_cond(c.term);
}
std::string metacode::Generator::eval_expr(metacode::_id id)
{
  auto expr = scr_info.metacodes->get_as<metacode::Cond_expr>(id);

  auto& m            = scr_info.metacodes->get(id);
  auto  line         = scr_info.file_info.get_line_from_pos(m.position);
  auto  line_end_pos = scr_info.file_info.get_line_end(line);

  if (!expr) {
    auto err = Error_Diagnostic(scr_info.id, 247, m.position, line_end_pos, compiler::EPhase::preprosessor,
                                "Expected a preprocessor expression", "");
    compiler::COMPILER.add_error(std::move(err));

    return "0";
  }

  if (expr->is_constant) {
    auto str = std::string(scr_info.file_info.tokens->audit.Token_to_str(expr->val));
    return str;
  } else if (expr->is_placeholder) {
    size_t p_id = to_placeholder_index(expr->val);
    if (!current_expand) {
      auto err = Error_Diagnostic(scr_info.id, 248, m.position, line_end_pos, compiler::EPhase::preprosessor,
                                  "Illegal placeholder outside an expand preprocessor instruction.", "");
      compiler::COMPILER.add_error(std::move(err));
      return "";
    }

    assert(p_id < current_placeholder_env.size() && "Placeholder index out of bound");

    auto p   = current_placeholder_env[p_id];
    auto str = std::string(scr_info.file_info.tokens->audit.Token_to_str(p));
    return str;
  } else {
    auto& prepro_args = compiler::COMPILER_OPTIONS.PREPROCESSOR_ARGS;
    auto  str         = std::string(scr_info.file_info.tokens->audit.Token_to_str(expr->val));

    if (auto it = prepro_args.find(str); it != prepro_args.end()) {
      return it->second;
    } else {
      // no preprocessor found : return 0
      return "0";
    }
  }
}