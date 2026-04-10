
#include "misc/preprocessor.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

#include "compiler/compiler.hpp"

#include "ast/ast_data.hpp"
#include <compiler_context.hpp>
#include "lexer/token.hpp"
#include "lexer/token_viewer.hpp"
#include "misc/script_info.hpp"
#include "misc/metacode.hpp"


Preprocessor::Preprocessor(ScriptInfo& _scr_info)
  : scr_info(_scr_info)
  , tok_v(new TokenViewer(_scr_info))
{
  m_meta       = new meta::MetablockManager;
  tok_v->phase = compiler::EPhase::preprosessor;
}


template <typename MetaNode>
std::unique_ptr<MetaNode> Preprocessor::Create_Meta(const Token& tok, size_t start_scope_pos)
{
  static_assert(std::derived_from<MetaNode, meta::Metablock>, "MetaNode must derive from meta::Metablock");

  std::unique_ptr<MetaNode> node = std::make_unique<MetaNode>();

  if (auto ptr = dynamic_cast<meta::Metablock*>(node.get())) {
    ptr->debug_str                   = tok_v->get_line_str_at(tok.span.line);
    ptr->position                    = tok.span.anteprocess_pos;
    ptr->_scope.start_scope_position = start_scope_pos;
  }

  m_meta->metablocks.insert({tok.span.anteprocess_pos, node.get()});
  return node;
};

std::vector<Token> Preprocessor::preprocess()
{
  // parse metacodes
  while (!tok_v->is_end()) {
    tok_v->match(TokTy::S_METACODE_END);

    if (process_any_meta(m_meta->root_metabock)) {
      continue;
    }

    m_meta->root_metabock.tokens_to_generate.push_back(tok_v->next().span.anteprocess_pos);
  }
  // keep the EOF token
  m_meta->root_metabock.tokens_to_generate.push_back(scr_info.tokens.back().span.anteprocess_pos);

  std::vector<Token> final_tokens = m_meta->root_metabock.generate_tokens(scr_info);

  size_t final_pos_count = 0;
  for (auto& tok : final_tokens) {
    tok.span.pos = final_pos_count++;
  }

  if (compiler::COMP_CTX.logs.contains("preprocessor")) {
    debug_write_postprocess_code_files(final_tokens);
  }

  return final_tokens;
}

void Preprocessor::debug_write_postprocess_code_files(const std::vector<Token>& toks)
{
  std::filesystem::create_directories(compiler::COMP_CTX.get_preprocess_dir());

  auto path = std::filesystem::path(compiler::COMP_CTX.get_preprocess_dir())
              / std::filesystem::path(scr_info.file_path).filename();
  path.replace_extension(".txt");

  std::ofstream o_gen(path);
  if (!o_gen) {
    std::cerr << "Error on file creation at " << path;
  }

  o_gen.clear();

  for (auto& tok : toks) {
    o_gen << tok.display();
    char end = tok.debug_end_of_line ? '\n' : ' ';
    o_gen << end;
  }

  o_gen.close();
}

bool Preprocessor::process_any_meta(meta::Metablock& parent)
{
  // check if special metacode case
  if (process_if(parent)) {
    return true;
  } else if (process_expand(parent)) {
    return true;
  } else if (match_metacode({"end", "<!>"})) {
    tok_v->add_error_tok(135, tok_v->peek(-2), "Unexpected metacode scope end instruction.",
                         "It's might be a excessive end instruction.");
    return false;
  } else {
    return process_metablock(parent);
  }
}

bool Preprocessor::process_metablock(meta::Metablock& parent)
{
  if (!tok_v->check(TokTy::METACODE)) return false;

  auto metablock = Create_Meta<meta::Metablock>(tok_v->peek(), 0);

  while (!tok_v->is_end()) {
    // no more metacode to process in one block
    if (tok_v->match(TokTy::S_METACODE_END) && !tok_v->match(TokTy::METACODE)) break;

    // if start of scope -> go to the scope function creator
    if (check_metacode({"scope", "<!>"}) || check_metacode({"scope", "<a>", "<!>"})) {
      break;
    }

    std::vector<meta::MetaWord> instruct_words;
    while (!tok_v->is_end()) {
      if (tok_v->check(TokTy::S_METACODE_END)) break;

      meta::MetaWord word({tok_v->next()});

      if (tok_v->match(TokTy::PIPE)) {

        while (!tok_v->is_end()) {
          if (!tok_v->match(TokTy::PIPE)) break;
          word.tokens.push_back(tok_v->next());
        }
      }

      instruct_words.push_back(word);
    }

    meta::MetaInstruct instruct(metablock.get(), instruct_words);

    metablock->_instructions.push_back(instruct);
  }

  if (metablock->_instructions.empty()) return false;

  process_scope(*metablock);
  parent.add_children(std::move(metablock));

  return true;
}

void Preprocessor::process_scope(meta::Metablock& meta)
{
  static const std::string hint =
      "define scope like:"
      "\n  - `# scope ...code lines... # end scope`\n  - named `# scope <name> ...code lines... # end scope`";

  // named scope
  if (match_metacode({"scope", "<a>", "<!>"})) {
    meta._scope.name = tok_v->peek(-2).val;
  }
  // unamed scope
  else if (!match_metacode({"scope", "<!>"})) {
    // no any scope -> subline scope mode
    meta._scope.is_subline_scope     = true;
    meta._scope.start_scope_position = tok_v->position();
    meta._scope.end_scope_position   = get_line_last_tok_pos(tok_v->line());
    return;
  };

  meta._scope.start_scope_position = tok_v->position();

  while (!tok_v->is_end()) {
    if (match_metacode({"end", "scope", "<!>"})) {
      meta._scope.end_scope_position = tok_v->peek(-4).span.anteprocess_pos;
      return;
    }

    // sub metacode
    if (process_any_meta(meta)) {
      continue;
    }

    // keep token to generate
    meta.tokens_to_generate.push_back(tok_v->next().span.anteprocess_pos);
  }

  tok_v->add_error(136, "Expected end of metacode scope (`# end` is never encounted).", hint);
}

static const std::string metacode_if_hint =
    "define metacode condition like:"
    "\n  - if `# if <condition> ...code lines... # end if`"
    "\n  - if-else `# if <condition> ...code lines... # else ...code lines... # end if`"
    "\n  - if-elif `# if <condition> ...code lines... # elif <condition> ...code lines... # end if`"
    "\n  - if-elif-...-else `# if <condition> ...code lines... # elif <condition> ...code lines... # else ...code "
    "lines... # end if`";

bool Preprocessor::process_if(meta::Metablock& parent)
{
  // # if
  // ...
  if (check_metacode({"elif"})) {
    tok_v->add_error(137, "Unexpected '# elif' metacode without a start metacode condition '# if ...' before.",
                     metacode_if_hint);
    return false;
  }
  if (check_metacode({"end", "if", "<!>"})) {
    tok_v->add_error(138,
                     "Unexpected '# end if' metacode without a start metacode condition '# if "
                     "... # elif ... # else ...' before.",
                     metacode_if_hint);
    return false;
  }
  if (!match_metacode({"if"})) return false;

  auto if_meta                         = Create_Meta<meta::Metablock_If>(tok_v->peek(-1), 0);
  if_meta->condition                   = process_condition();
  if_meta->_scope.start_scope_position = tok_v->position();

  while (!tok_v->is_end()) {
    // end of flow
    if (_if_end_metacode(*if_meta) || _else_metacode(*if_meta) || _elif_metacode(*if_meta)) {
      parent.add_children(std::move(if_meta));
      return true;
    }

    // if sub-meta found (other directive)
    if (!process_any_meta(*if_meta)) {
      // keep token to generate
      if_meta->tokens_to_generate.push_back(tok_v->next().span.anteprocess_pos);
    }
  }

  tok_v->add_error(
      139, "Expected end of metacode condition `# if ...` (`# end` or `# elif ...` or `# else` are never encounted)",
      metacode_if_hint);

  return false;
}

bool Preprocessor::_if_end_metacode(meta::Metablock_If& end_wait)
{
  if (match_metacode({"end", "if", "<!>"})) {
    end_wait._scope.end_scope_position = tok_v->peek(-4).span.anteprocess_pos;
    return true;
  }
  return false;
}

bool Preprocessor::_else_metacode(meta::Metablock_If& before_else)
{
  if (match_metacode({"else", "<!>"})) {
    auto else_meta       = Create_Meta<meta::Metablock_If>(tok_v->peek(-2), tok_v->position());
    else_meta->flow_type = meta::Metablock_If::EFlowType::ELSE;

    bool correct_else_exit = false;
    while (!tok_v->is_end()) {
      // end of flow
      if (_if_end_metacode(*else_meta)) {
        before_else.alternative = std::move(else_meta);
        return true;
      }

      // if sub-meta found (other directive)
      if (!process_any_meta(*else_meta)) {
        // keep token to generate
        else_meta->tokens_to_generate.push_back(tok_v->next().span.anteprocess_pos);
      }
    }

    tok_v->add_error(140, "Expected end of metacode condition `# else ...` (`# end if` is never encounted)",
                     metacode_if_hint);
  }
  return false;
}

bool Preprocessor::_elif_metacode(meta::Metablock_If& before_elif)
{
  if (match_metacode({"elif"})) {
    auto elif_meta       = Create_Meta<meta::Metablock_If>(tok_v->peek(-1), tok_v->position());
    elif_meta->flow_type = meta::Metablock_If::EFlowType::ELIF;
    elif_meta->condition = process_condition();

    while (!tok_v->is_end()) {
      if (_if_end_metacode(*elif_meta) || _else_metacode(*elif_meta) || _elif_metacode(*elif_meta)) {
        before_elif.alternative = std::move(elif_meta);
        return true;
      }

      // elif sub-meta found (other directive)
      if (!process_any_meta(*elif_meta)) {
        // keep token to generate
        elif_meta->tokens_to_generate.push_back(tok_v->next().span.anteprocess_pos);
      }
    }

    tok_v->add_error(141,
                     "Expected end of metacode condition `# elif ...` (`# end if` or `# elif "
                     "...` is never encounted)",
                     metacode_if_hint);
  }

  return false;
}

static const std::string expand_hint =
    "define expand like:"
    "\n  `# expand"
    "\n   # for <placeholder-name> as <symbol1> | <symbol2> | ..."
    "\n   # <other-for-placeholder-as-symbol>"
    "\n   # each\n     ...code lines..."
    "\n     # expand if [[placeholder_name]] ..."
    "\n       ...code lines..."
    "\n     # end expand if"
    "\n   # end each`";

std::unique_ptr<meta::Metablock_Expand> Preprocessor::_expand_header()
{
  // # expand
  auto  expansion_meta = Create_Meta<meta::Metablock_Expand>(tok_v->peek(-2), 0);
  auto& phs            = expansion_meta->placeholders;
  auto& phs_pos        = expansion_meta->placeholders_pos;

  bool clean_end = false;
  // parse for(s)
  while (!tok_v->is_end()) {
    if (tok_v->check(TokTy::METACODE)) {
      if (match_metacode({"for", "<a>", "as"})) {
        std::string         placeholder_id   = tok_v->peek(-2).val;
        std::vector<Token>& placeholder_syms = phs[placeholder_id];

        // list of alternatives
        while (!tok_v->is_end()) {
          if (tok_v->match(TokTy::S_METACODE_END)) break;
          placeholder_syms.push_back(tok_v->next());
          tok_v->match(TokTy::PIPE);
        }
      } else if (match_metacode({"each", "<!>"})) {
        expansion_meta->_scope.start_scope_position = tok_v->position();
        clean_end                                   = true;
        break;
      } else {
        tok_v->add_error(142, "Expected metacode expand instruction.", expand_hint);
        return nullptr;
      }
    } else {
      tok_v->add_error(143, "Unexpected symbol in metacode expand instruction.", expand_hint);
      return nullptr;
    }
  }

  if (!clean_end) {
    tok_v->add_error(144,
                     "Expected start of metacode `# each [<name>]` instruction at the end of "
                     "metacode expand instruction.",
                     expand_hint);
    return nullptr;
  }

  return expansion_meta;
}

void Preprocessor::_expand_body(meta::Metablock_Expand& expansion_meta)
{
  bool clean_end = false;

  // parse  expansion  code  body
  while (!tok_v->is_end()) {

    // expand if case
    if (_expand_if(expansion_meta)) {
      continue;
    }
    // end of expand
    else if (match_metacode({"end", "each", "<!>"})) {
      expansion_meta._scope.end_scope_position = tok_v->position() - 4;
      clean_end                                = true;
      break;
    }
    // sub metacode -> no direct generation need the metacode influence is keep in the original script before code
    // expansion the tokens save their original position and their expanded position
    else if (process_any_meta(expansion_meta)) {

    } else if (_expand_placeholder(expansion_meta)) {

    } else {
      // keep token to generate
      expansion_meta.tokens_to_generate.push_back(tok_v->next().span.anteprocess_pos);
    }
  }

  if (!clean_end) {
    tok_v->add_error(145, "Expected end of expand metacode scope `# end expand if`.", expand_hint);
  }
}

bool Preprocessor::_expand_placeholder(meta::Metablock_Expand& expansion_meta)
{
  if (!tok_v->check(TokTy::S_METACODE_PLACEHOLDER)) return false;

  std::string placeholder_id = tok_v->peek().val;

  if (placeholder_id.empty()) return false;

  if (!expansion_meta.is_valid_placeholder_name(placeholder_id)) {
    tok_v->add_error_tok(146, tok_v->peek(-2),
                         "Expected valid placeholder identifier '" + placeholder_id
                             + "' dosen't exists in the expansion declaration.",
                         expand_hint);
    return false;
  }

  // push flag to prevent placeholder calculation
  expansion_meta.tokens_to_generate.push_back(meta::Metablock_Expand::k_placeholder_flag);
  // push the placeholder identifier
  expansion_meta.tokens_to_generate.push_back(tok_v->next().span.anteprocess_pos);

  return true;
}

bool Preprocessor::process_expand(meta::Metablock& parent)
{
  if (!match_metacode({"expand", "<!>"})) return false;

  auto expansion_meta = _expand_header();

  _expand_body(*expansion_meta);

  parent.add_children(std::move(expansion_meta));

  return true;
}

std::unique_ptr<meta::Cond_Base> Preprocessor::_cond_atom()
{
  static const std::string hint =
      "define metacode condition like:"
      "\n  - define `# if _T`"
      "\n  - define with value `# if _T == 'hello'`"
      "\n  - complex condition `# if _T != i32 or _U == f128";

  if (tok_v->match(TokTy::OPEN_PAREN)) {
    auto node = process_condition();
    if (!tok_v->match(TokTy::CLOSE_PAREN)) {
      tok_v->add_error(147, "Expected ')'", "");
    }
    return node;
  } else if (tok_v->match(TokTy::NOT)) {
    auto child = _cond_atom();
    return std::make_unique<meta::Cond_Not>(std::move(child));
  }

  std::string placeholder = tok_v->next().val;
  bool        isNot       = false;
  std::string right;

  if (tok_v->check_any({TokTy::OP_EQ, TokTy::OP_NEQ})) {
    isNot = tok_v->next().type == TokTy::OP_NEQ;
  } else if (tok_v->check(TokTy::S_METACODE_END)) {
    return std::make_unique<meta::Cond_Eq>(placeholder, "1", isNot);
  } else {
    tok_v->add_error(148, "Expect only equal '==' or not equal '!=' in metacode comparison.", hint);
  }
  // technically any token possible
  right = tok_v->next().val;

  return std::make_unique<meta::Cond_Eq>(placeholder, right, isNot);
}

// Expression avec AND / OR
std::unique_ptr<meta::Cond_Base> Preprocessor::process_condition()
{
  auto lhs = _cond_atom();

  while (true) {
    if (tok_v->check_any({TokTy::AND, TokTy::OR})) {
      TokTy op  = tok_v->next().type;
      auto  rhs = _cond_atom();

      if (op == TokTy::AND)
        lhs = std::make_unique<meta::Cond_And>(std::move(lhs), std::move(rhs));
      else
        lhs = std::make_unique<meta::Cond_Or>(std::move(lhs), std::move(rhs));
    } else {
      break; // no op -> end
    }
  }

  tok_v->match(TokTy::S_METACODE_END);

  return lhs;
}

// token viewver will stay at his original position check first if have the start metacode token : '#' don't start by
// '#' pattern !
bool Preprocessor::check_metacode(std::initializer_list<std::string> pattern)
{
  if (pattern.size() == 0) return false;

  if (*pattern.begin() == "#") throw std::runtime_error("Illegal pattern start symbole");

  if (tok_v->peek().type != TokTy::METACODE) return false;

  size_t i = 1; // first token already checked
  for (const auto& elem : pattern) {
    if (!is_tok_in_pattern(tok_v->peek(i), elem)) return false;
    ++i;
  }
  return true;
}

// token viewver will jump after the metacode pattern (the invisible end metacode token) check first if have the start
// metacode token : '#' don't start by '#' pattern !
bool Preprocessor::match_metacode(std::initializer_list<std::string> pattern)
{
  if (pattern.size() == 0) return false;

  if (check_metacode(pattern)) {
    // jump : pattern.size + 1
    tok_v->jump(tok_v->position() + pattern.size() + 1); // jump is safe
    return true;
  }
  return false;
}

bool Preprocessor::is_tok_in_pattern(const Token& tok, const std::string& pattern)
{
  if (pattern.empty()) return false; // no pattern exists, impossible to evaluate

  if (pattern == "<*>")
    return true; // any token valid
  else if (pattern == "<a>" && tok.type == TokTy::IDENTIFIER)
    return true; // identifier token valid
  else if (pattern == "<0>"
           && std::find(kNumericTypeTokens.begin(), kNumericTypeTokens.end(), tok.type) != kNumericTypeTokens.end())
    return true; // numeric token valid
  else if (pattern == "<!>" && tok.type == TokTy::S_METACODE_END)
    return true; // end metacode instruction valid
  else if (tok.val == pattern)
    return true; // token string value valid

  return false; // no pattern corresponding
}

std::vector<Token> Preprocessor::get_scope_tokens(size_t start_pos, size_t end_pos)
{
  if (start_pos >= scr_info.tokens.size() || start_pos < scr_info.tokens.size() || end_pos >= scr_info.tokens.size()
      || end_pos < scr_info.tokens.size()) {
    std::runtime_error(
        "Impossible to get tokens for metacode, start or end slicer are out of "
        "bound of the script tokens list.");
  }

  std::vector<Token> tokens;
  tokens.insert(tokens.begin(), scr_info.tokens.begin() + start_pos, scr_info.tokens.begin() + end_pos);
  return tokens;
}

size_t Preprocessor::get_line_last_tok_pos(size_t line)
{
  auto   tok      = tok_v->peek();
  size_t last_pos = tok.span.anteprocess_pos;
  size_t offset   = 0;
  while (tok.type != TokTy::S_END_OF_FILE) {
    tok = tok_v->peek(++offset);

    if (tok.span.line != line) break;

    last_pos = tok.span.anteprocess_pos;
  }

  return last_pos;
}

bool Preprocessor::_expand_if(meta::Metablock_Expand& expansion_meta)
{
  // check if metacode expansion condition '# expand if [[placeholder]]... == / !='

  if (!match_metacode({"expand", "if"})) return false;

  std::unique_ptr<meta::Expand_If> base_cond = Create_Meta<meta::Expand_If>(tok_v->peek(-2), 0);

  tok_v->expect(149, TokTy::S_METACODE_PLACEHOLDER,
                "The conditional expansion metacode must be declared with a placeholder comparison."
                "\nOtherwise, use simply the conditional compilation metacode `# if <condition> ... # end`",
                "Define expansion metacode like:"
                "\n  - `# expand if [[placeholder_identifier]] == / != <condition symbol> ... # end`");

  tok_v->prev();
  base_cond->condition = process_condition();
  tok_v->match(TokTy::S_METACODE_END);

  meta::Expand_If* current_cond = base_cond.get();
  while (!tok_v->is_end()) {
    // expand if alternative
    if (match_metacode({"expand", "elif"}) || match_metacode({"expand", "else", "<!>"})) {
      std::unique_ptr<meta::Expand_If> sub_cond = Create_Meta<meta::Expand_If>(tok_v->peek(-1), 0);
      if (tok_v->peek(-1).val == "elif") sub_cond->condition = process_condition();
      current_cond->_scope.start_scope_position = tok_v->position();
      current_cond->alternative                 = std::move(sub_cond);
      current_cond                              = current_cond->alternative.get();
    } else if (match_metacode({"expand", "<!>"})) {
      tok_v->add_error(150, "Unexepected a nested '# expand'.", "did you want to write '# expand if' ?");
      break;
    }
    // if block ended
    else if (match_metacode({"end", "expand", "if", "<!>"})) {
      current_cond->_scope.end_scope_position = tok_v->position() - 4;
      break;
    }
    // if body code
    else if (process_any_meta(expansion_meta)) {
      continue;
    } else {
      // keep token to generate
      base_cond->tokens_to_generate.push_back(tok_v->next().span.anteprocess_pos);
    }
  }

  expansion_meta.add_expand_condition(std::move(base_cond));
  return true;
}
