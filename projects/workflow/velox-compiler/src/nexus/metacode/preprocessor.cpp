#include "preprocessor.hpp"

#include <cassert>
#include <string_view>

#include "nexus/forward.hpp"
#include "nexus/ids.hpp"
#include "nexus/metacode/metacode.hpp"
#include "nexus/metacode/token_generator.hpp"
#include "nexus/script.hpp"
#include "compiler/compiler.hpp"
#include "nexus/lexer/token.hpp"
#include "nexus/lexer/token_viewer.hpp"
#include "misc/error_output.hpp"


metacode::Preprocessor::Preprocessor(script::ScriptInfo& scr_info)
  : scr_info(scr_info)
{
  tok_v = new token::Viewer(scr_info);
}


bool metacode::Preprocessor::start_preprocessor()
{
  auto last_err_count       = compiler::COMPILER.errors.size();
  scr_info.root_metacode_id = scr_info.metacodes->root;
  auto root                 = scr_info.metacodes->get_as<metacode::Root>(scr_info.root_metacode_id);
  assert(root);
  current_parent = root;

  // parse metacodes
  while (!tok_v->is_end()) {
    tok_v->match(token::ETokenKind::S_METACODE_END);

    if (preprocess_any()) continue;
    add_token_to_generate(tok_v->next());
  }

  return last_err_count == compiler::COMPILER.errors.size();
}
token::_id metacode::Preprocessor::preprocess_token(token::Token& tok)
{
  if (tok.kind == token::ETokenKind::S_METACODE_PLACEHOLDER) {
    assert(current_expand);

    auto p_name = tok_to_str(tok_v->peek().id);

    assert(!p_name.empty() && "The placeholder must have a name");

    size_t p_pos = 0;
    for (auto& p : current_expand->placeholders) {
      if (p.name == p_name) break;
      p_pos++;
    }

    if (p_pos == current_expand->placeholders.size()) {
      tok_v->add_error_tok(146, tok_v->peek(-2),
                           "Expected valid placeholder, identifier '" + std::string(p_name)
                               + "' dosen't exists in the expansion declaration.",
                           "");
      return NO_ID;
    }


    // push flag to prevent placeholder calculation
    const size_t placeholder_id = metacode::k_placeholder_flag_start.value() + p_pos;
    return token::_id(placeholder_id);
  }

  // no preprocess needed
  return tok.id;
}

void metacode::Preprocessor::add_token_to_generate(token::Token& tok)
{
  add_generated_token(preprocess_token(tok));
}

void metacode::Preprocessor::add_generated_token(token::_id id)
{
  current_parent->tokens_to_generate.push_back(id);
}

metacode::_id metacode::Preprocessor::preprocess_file()
{
}
metacode::_id metacode::Preprocessor::preprocess_any()
{
  if (!tok_v->check(token::ETokenKind::METACODE)) return NO_ID;

  if (auto id = preprocess_if()) {
    return id;
  } else if (auto id = preprocess_expand()) {
    return id;
  } else if (match_metacode({"end", pattern_constants::end})) {
    auto err =
        Error_Diagnostic(scr_info.id, 135, tok_v->peek(-2).begin, tok_v->peek(-2).begin, compiler::EPhase::preprosessor,
                         "Unexpected metacode scope end instruction.", "It's might be a excessive end instruction.");
    compiler::COMPILER.add_error(std::move(err));
    return NO_ID;
  } else {
    return preprocess_metablock();
  }
}
metacode::_id metacode::Preprocessor::preprocess_metablock()
{
  if (!tok_v->check(token::ETokenKind::METACODE)) return NO_ID;

  METACODE_CREATE(block, Metablock)

  auto old_parent = current_parent;
  current_parent  = block;

  while (!tok_v->is_end()) {
    // no more metacode to process in one block
    if (tok_v->match(token::ETokenKind::S_METACODE_END) && !tok_v->match(token::ETokenKind::METACODE)) break;

    // if start of scope -> go to the scope function creator
    if (check_metacode({"scope", pattern_constants::end})
        || check_metacode({"scope", pattern_constants::identifier, pattern_constants::end})) {
      break;
    }

    std::vector<metacode::Word> instruct_words;
    while (!tok_v->is_end()) {
      if (tok_v->check(token::ETokenKind::S_METACODE_END)) break;

      metacode::Word word(scr_info);
      word.tokens = {tok_to_str(tok_v->next().id)};

      if (tok_v->match(token::ETokenKind::PIPE)) {

        while (!tok_v->is_end()) {
          if (!tok_v->match(token::ETokenKind::PIPE)) break;
          word.tokens.push_back(tok_to_str(tok_v->next().id));
        }
      }

      instruct_words.push_back(word);
    }


    metacode::Instruction instruction{.words = std::move(instruct_words)};
    block->instructions.push_back(std::move(instruction));
  }

  preprocess_scope(*block);

  current_parent = old_parent;

  return block->id;
}
void metacode::Preprocessor::preprocess_scope(metacode::Metacode& meta)
{
  constexpr std::string_view hint =
      R"(define scope like:
  - `# scope ...code lines... # end scope`
  - named `# scope <name> ...code lines... # end scope`)";

  if (match_metacode({"scope", pattern_constants::identifier, pattern_constants::end})) {
    meta.scope.name = tok_to_str(tok_v->peek(-2).id);
  } else if (!match_metacode({"scope", pattern_constants::end})) {
    meta.scope.is_inline = true;
    meta.scope.start_pos = tok_v->position();
    size_t line          = scr_info.file_info.get_line_from_pos(tok_v->peek().begin);
    meta.scope.end_pos   = scr_info.file_info.get_line_end(line);
    return;
  }

  meta.scope.start_pos = tok_v->position();

  while (!tok_v->is_end()) {
    if (match_metacode({"end", "scope", pattern_constants::end})) {
      meta.scope.end_pos = tok_v->position() - 4;
      return;
    }

    if (preprocess_any()) continue;
    add_token_to_generate(tok_v->next());
  }

  tok_v->add_error(136, "Expected end of metacode scope (`# end` is never encounted).", hint);
}

constexpr std::string_view metacode_if_hint =
    R"(define metacode condition like:
  - if `# if <condition> ...lines... # end if`
  - if-else `# if <condition> ...lines... # else ...lines... # end if`
  - if-elif `# if <condition> ...lines... # elif <condition> ...lines... # end if`
  - if-elif-...-else `# if <condition> ...lines... # elif <condition> ...lines... # else ...lines... # end if`)";

metacode::_id metacode::Preprocessor::preprocess_if()
{
  // # if ...
  if (check_metacode({"elif"})) {
    tok_v->add_error(137, "Unexpected '# elif' metacode without a start metacode condition '# if ...' before.",
                     metacode_if_hint);
    return NO_ID;
  }
  if (check_metacode({"end", "if", pattern_constants::end})) {
    tok_v->add_error(138,
                     "Unexpected '# end if' metacode without a start metacode condition '# if "
                     "... # elif ... # else ...' before.",
                     metacode_if_hint);
    return NO_ID;
  }
  if (!match_metacode({"if"})) return NO_ID;

  METACODE_CREATE(if_meta, If)

  if_meta->condition       = preprocess_condition();
  if_meta->scope.start_pos = tok_v->peek().begin;

  auto old_parent = current_parent;
  current_parent  = if_meta;

  while (!tok_v->is_end()) {
    // end of flow
    if (__if_end(*if_meta) || __else(*if_meta) || __elif(*if_meta)) return NO_ID;

    if (preprocess_any()) continue;
    add_token_to_generate(tok_v->next());
  }

  current_parent = old_parent;

  tok_v->add_error(
      139, "Expected end of metacode condition `# if ...` (`# end` or `# elif ...` or `# else` are never encounted)",
      metacode_if_hint);

  return NO_ID;
}
bool metacode::Preprocessor::__if_end(metacode::If& end_wait)
{
  if (match_metacode({"end", "if", pattern_constants::end})) {
    end_wait.scope.end_pos = tok_v->peek(-4).begin;
    return true;
  }
  return false;
}
bool metacode::Preprocessor::__else(metacode::If& before_else)
{
  if (match_metacode({"else", pattern_constants::end})) {
    METACODE_CREATE(else_meta, If)

    else_meta->is_else = true;

    auto old_parent = current_parent;
    current_parent  = else_meta;

    bool correct_else_exit = false;
    while (!tok_v->is_end()) {
      // end of flow
      if (__if_end(*else_meta)) {
        before_else.alternative = else_meta->id;
        return true;
      }

      if (preprocess_any()) continue;
      add_token_to_generate(tok_v->next());
    }

    current_parent = old_parent;

    tok_v->add_error(140, "Expected end of metacode condition `# else ...` (`# end if` is never encounted)",
                     metacode_if_hint);
  }
  return false;
}

bool metacode::Preprocessor::__elif(metacode::If& before_elif)
{
  if (match_metacode({"elif"})) {
    METACODE_CREATE(elif_meta, If)

    elif_meta->is_elif = true;

    auto old_parent      = current_parent;
    current_parent       = elif_meta;
    elif_meta->condition = preprocess_condition();

    while (!tok_v->is_end()) {
      if (__if_end(*elif_meta) || __else(*elif_meta) || __elif(*elif_meta)) {
        before_elif.alternative = elif_meta->id;
        return true;
      }

      if (preprocess_any()) continue;
      add_token_to_generate(tok_v->next());
    }

    current_parent = old_parent;

    tok_v->add_error(141,
                     "Expected end of metacode condition `# elif ...` (`# end if` or `# elif "
                     "...` is never encounted)",
                     metacode_if_hint);
  }

  return false;
}

constexpr std::string_view expand_hint =
    R"(define expand like:
  `# expand
   # for <placeholder-name> as <symbol1> | <symbol2> | ...
   # <other-for-placeholder-as-symbol>
   # each\n     ...code lines...
     # expand if [[placeholder_name]] ...
       ...code lines...
     # end expand if
   # end each`)";

metacode::_id metacode::Preprocessor::preprocess_expand()
{
  if (!match_metacode({"expand", pattern_constants::end})) return NO_ID;

  METACODE_CREATE(expand, Expand)

  auto old_parent = current_parent;
  current_parent  = expand;
  current_expand  = expand;

  __expand_header();

  __expand_body();

  current_parent = old_parent;
  current_expand = nullptr;

  return expand->id;
}
void metacode::Preprocessor::__expand_header()
{
  assert(current_expand);

  bool clean_end = false;
  // parse for(s)
  while (!tok_v->is_end()) {
    if (tok_v->check(token::ETokenKind::METACODE)) {
      if (match_metacode({"for", pattern_constants::identifier, "as"})) {
        auto p_name = tok_to_str(tok_v->peek(-2).id);

        metacode::Expand::Placeholder placeholder{.name = p_name};

        // list of alternatives
        while (!tok_v->is_end()) {
          if (tok_v->match(token::ETokenKind::S_METACODE_END)) break;
          placeholder.variants.push_back(tok_v->next().id);
          tok_v->match(token::ETokenKind::PIPE);
        }

        current_expand->placeholders.push_back(placeholder);
      } else if (match_metacode({"each", pattern_constants::end})) {
        current_expand->scope.start_pos = tok_v->peek().begin;
        clean_end                       = true;
        break;
      } else {
        tok_v->add_error(142, "Expected metacode expand instruction.", expand_hint);
        return;
      }
    } else {
      tok_v->add_error(143, "Unexpected symbol in metacode expand instruction.", expand_hint);
      return;
    }
  }

  if (!clean_end) {
    tok_v->add_error(144,
                     "Expected start of metacode `# each [<name>]` instruction at the end of "
                     "metacode expand instruction.",
                     expand_hint);
  }
}
void metacode::Preprocessor::__expand_body()
{
  assert(current_expand);
  bool clean_end = false;

  // parse expansion code body
  while (!tok_v->is_end()) {
    // end of expand
    if (match_metacode({"end", "each", pattern_constants::end})) {
      current_expand->scope.end_pos = tok_v->peek().begin - 4;
      clean_end                     = true;
      break;
    }

    if (preprocess_any()) continue;
    add_token_to_generate(tok_v->next());
  }

  if (!clean_end) {
    tok_v->add_error(145, "Expected end of expand metacode scope `# end expand if`.", expand_hint);
  }
}

metacode::_id metacode::Preprocessor::preprocess_condition()
{
  auto lhs = __cond_atom();

  while (true) {
    if (tok_v->check_any({token::ETokenKind::OP_EQ, token::ETokenKind::OP_NEQ, token::ETokenKind::OP_AND,
                          token::ETokenKind::OP_NAND, token::ETokenKind::OP_OR, token::ETokenKind::OP_NOR,
                          token::ETokenKind::OP_XOR, token::ETokenKind::OP_XNOR})) {
      token::ETokenKind op  = tok_v->next().kind;
      auto              rhs = __cond_atom();

      METACODE_CREATE(bin, Binary_Cond)

      bin->left  = lhs;
      bin->right = rhs;
      bin->type  = ast::ETokenKind_to_EBinOpType(op);
      lhs        = bin->id;
    } else {
      break; // no op -> end
    }
  }

  tok_v->match(token::ETokenKind::S_METACODE_END);

  return lhs;
}
metacode::_id metacode::Preprocessor::__cond_atom()
{
  constexpr std::string_view hint =
      R"(define metacode condition like:
  - define `# if _T`
  - define `# if !_T`
  - define with value `# if _T == 'hello'`
  - complex condition `# if _T != i32 or _U == f128
  - nested condition `# if _T != i32 and (_U == f128 or _U == f64
  - placeholder condition `# if [[_T]] == a)";

  if (tok_v->match(token::ETokenKind::OP_NOT)) {
    auto term = __cond_atom();

    METACODE_CREATE(_not, Unary_Not_Cond)

    _not->term = term;
    return _not->id;
  }

  METACODE_CREATE(expr, Cond_expr);

  expr->is_constant    = tok_v->match(token::ETokenKind::DOLLAR);
  expr->is_placeholder = tok_v->check(token::ETokenKind::S_METACODE_PLACEHOLDER);

  expr->val = preprocess_token(tok_v->next());

  return expr->id;
}

bool metacode::Preprocessor::match_metacode(std::initializer_list<std::string_view> pattern)
{
  if (pattern.size() == 0) return false;

  if (check_metacode(pattern)) {
    // jump : pattern.size + 1
    tok_v->jump(tok_v->position() + pattern.size() + 1); // jump is safe
    return true;
  }
  return false;
}

bool metacode::Preprocessor::check_metacode(std::initializer_list<std::string_view> pattern)
{
  if (pattern.size() == 0) return false;

  // "Illegal pattern start symbole");
  assert(*pattern.begin() == "#");

  if (tok_v->peek().kind != token::ETokenKind::METACODE) return false;

  size_t i = 1; // first token already checked
  for (const auto& elem : pattern) {
    if (!is_tok_in_pattern(tok_v->peek(i), elem)) return false;
    ++i;
  }
  return true;
}


bool metacode::Preprocessor::is_tok_in_pattern(const token::Token& tok, std::string_view pattern)
{
  if (pattern.empty()) return false; // no pattern exists, impossible to evaluate

  if (pattern == pattern_constants::wildcard)
    return true; // any token valid
  else if (pattern == pattern_constants::identifier && tok.kind == token::ETokenKind::IDENTIFIER)
    return true; // identifier token valid
  else if (pattern == pattern_constants::numeric
           && std::ranges::find(token::k_type_numeric.begin(), token::k_type_numeric.end(), tok.kind)
                  != token::k_type_numeric.end())
    return true; // numeric token valid
  else if (pattern == pattern_constants::end && tok.kind == token::ETokenKind::S_METACODE_END)
    return true; // end metacode instruction valid
  else if (tok_to_str(tok.id) == pattern)
    return true; // token string value valid

  return false; // no pattern corresponding
}

std::string_view metacode::Preprocessor::tok_to_str(token::_id id) const
{
  return scr_info.file_info.tokens->audit.Token_to_str(id);
}