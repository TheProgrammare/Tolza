
#include "parser_context.hpp"

#include <iostream>
#include <memory>

#include "ast/ast_data.hpp"
#include "misc/module_manager.hpp"
#include "misc/script_info.hpp"

#include "parser_base.hpp"
#include "parser_declaration.hpp"
#include "parser_declaration_cop.hpp"
#include "parser_declaration_local.hpp"
#include "parser_expression.hpp"
#include "parser_literal.hpp"
#include "parser_memory.hpp"
#include "parser_operation.hpp"
#include "parser_statement.hpp"
#include "parser_type.hpp"

#include "lexer/token_viewer.hpp"

#include "misc/metacode.hpp"

parser::Parser_Context::Parser_Context(std::shared_ptr<ScriptInfo> p_scr_info)
  : scr_info_sptr(p_scr_info)
  , scr_info(*p_scr_info.get())
  , meta_m(p_scr_info->meta_m)
  , tok_v(TokenViewer(*p_scr_info.get()))
  , p_cop(std::make_unique<Parser_Declaration_COP>(*this))
  , p_decl(std::make_unique<Parser_Declaration>(*this))
  , p_expr(std::make_unique<Parser_Expression>(*this))
  , p_lit(std::make_unique<Parser_Literal>(*this))
  , p_loc(std::make_unique<Parser_Declaration_Local>(*this))
  , p_mem(std::make_unique<Parser_Memory>(*this))
  , p_op(std::make_unique<Parser_Operator>(*this))
  , p_state(std::make_unique<Parser_Statement>(*this))
  , p_type(std::make_unique<Parser_Type>(*this))
  , p_base(std::make_unique<Parser_Base>(*this))
{
  scr_info.root_node                = std::make_shared<ast::Root>();
  scr_info.root_node->node_scr_info = p_scr_info;
  scr_info.module_root              = std::make_shared<module::Module>(scr_info_sptr);
  scr_info.root_node->node_module   = scr_info.module_root;

  current_module = scr_info.module_root;
}

parser::Parser_Context::~Parser_Context()
{
}

std::vector<std::string> parser::Parser_Context::start_parsing()
{
  try {
    while (!tok_v.is_end()) {
      auto line = p_decl->parse_declaration();
      if (line) scr_info.root_node->global_nodes.push_back(line);
      if (tok_v.match(TokTy::S_END_OF_FILE)) break;
    }
  } catch (const std::runtime_error& e) {
    // std::cerr << e.what() << std::endl; context.tokView.synchronize(); attempt_recovery();
  }

  return tok_v.errors;
}

bool parser::is_gen_args(TokenViewer& p_tok_v)
{
  size_t originPos           = p_tok_v.position();
  bool   isGenArgsValid      = true;
  bool   isAfterGenArgsValid = false;
  size_t count               = 0;
  size_t nestedBrackets      = 1;
  // = 0 valid nested result; > 0 invalid nested result; begins at 1
  // because generic type args begins with < and must end by >

  // check genArgs
  while (!p_tok_v.is_end()) {
    bool tokValid = false;

    auto tok = p_tok_v.peek(count);
    for (auto tok_valid : k_args_generic_valid) {
      if (tok.type == tok_valid) {
        if (tok.type == TokTy::OPEN_BRACKETS) ++nestedBrackets;
        if (tok.type == TokTy::CLOSE_BRACKETS) --nestedBrackets;
        tokValid = true;
        break;
      }
    }

    if (!tokValid) {
      isGenArgsValid = false;
      break;
    }

    // nested args are finish
    if (nestedBrackets == 0) break;

    count++;
  }

  // if is a valid generic args with correct nested brackets check next token
  // if is not a false positive generic args
  // (case of interval comparaison like a < b > 0)
  if (isGenArgsValid && nestedBrackets == 0) {
    auto tokAfterGenArgs = p_tok_v.peek(count + 1);

    if (tokAfterGenArgs.type == TokTy::OPEN_PAREN)
      isAfterGenArgsValid = true;
    else {
      isAfterGenArgsValid =
          !(p_tok_v.check_any(k_lit) || p_tok_v.check_any(k_operator) || p_tok_v.check_any(k_op_comparison));
    }
  }

  const bool result = isGenArgsValid && isAfterGenArgsValid;
  if (result)
    p_tok_v.rewind(originPos);
  else
    p_tok_v.rewind(originPos - 1);

  // is an effective gen args ?
  return result;
}

bool parser::Parser_Context::metablock_contains(const ast::Node& n, const std::string& s) const
{
  return meta_m->contains(n.get_tok_antepos(), s);
}

bool parser::Parser_Context::metablock_contains(const ast::Node& n, TokTy t) const
{
  return meta_m->contains(n.get_tok_antepos(), t);
}

std::string parser::Parser_Context::get_export_name(const ast::Node& n) const
{
  return meta_m->get_export_name(n.get_tok_antepos());
}

const meta::MetaInstruct* parser::Parser_Context::get_instruct(const ast::Node&                          n,
                                                               const std::initializer_list<std::string>& pattern) const
{
  return meta_m->get_instruct(n.get_tok_antepos(), pattern);
}

const meta::Metablock* parser::Parser_Context::get_metablock(const ast::Node&                          n,
                                                             const std::initializer_list<std::string>& pattern)
{
  return meta_m->get_metablock(n.get_tok_antepos(), pattern);
}


void parser::Parser_Context::enter_module(std::shared_ptr<ast::ADeclaration> p_decl, std::string debug_name)
{
  auto mod = std::make_shared<module::Module>(p_decl, debug_name);

  if (!current_module->add_sub_module(mod)) {
    tok_v.add_error_tok(239, p_decl->node_token, "Impossible to enter in the module " + debug_name, "");
    return;
  }

  current_module = mod;
}
void parser::Parser_Context::exit_module()
{
  if (!current_module->parent_module) {
    tok_v.add_error_tok(239, current_module->owner.lock()->node_token,
                        "Impossible to exit the current scope, the module \"" + current_module->debug_name
                            + "\" dosen't have parent module.",
                        "");
  }
}

void parser::Parser_Context::attempt_recovery()
{

  // recovery loop case
  static Token lastokRecovered;
  if (lastokRecovered.span == tok_v.peek().span) {
    std::cerr << "\n--------------------- ! COMPILATION STOPPED ! -------------------" << std::endl;
    std::cerr << "  Compiler: Infinitive recovery loop detected!" << std::endl;
    std::cerr << "  Please check the code source." << std::endl;
    return;
  } else {
    lastokRecovered = tok_v.peek();
  }

  return;
}

bool parser::Parser_Context::match_field_separator(TokTy separator, TokTy end)
{
  if (separator != TokTy::S_END_OF_FILE) {
    if (tok_v.match(separator)) return false;
    if (tok_v.match(end)) return true;
    tok_v.add_error(12, "Unexpected token '" + tok_v.peek().val + "' in expression.",
                    "expected a separator '" + std::to_string(int(separator)) + "' or a ending '"
                        + std::to_string(int(end)) + "'");

  } else {
    if (tok_v.match(end)) return true;
  }
  return false;
}

bool parser::Parser_Context::match_field_any_separator(TokTy p_separator, std::initializer_list<TokTy> p_end)
{
  if (tok_v.match(p_separator)) return false;
  if (tok_v.match_any(p_end)) return true;
  std::string sym_end;
  size_t      sym_count = 0;
  for (auto& elem : p_end) {
    sym_end += "'" + std::to_string(int(elem)) + "', ";
    if (++sym_count > 10) {
      sym_end += "\n";
      sym_count = 0;
    }
  }
  tok_v.add_error(13, "Unexpected token '" + tok_v.peek().val + "' in expression.",
                  "expected a separator '" + std::to_string(int(p_separator)) + "' or a ending {" + sym_end + "}");
  return false;
}

std::string parser::Parser_Context::parse_name(const std::string& p_msg, const std::string& p_hint)
{
  static const std::string msg = "Expected identifier (classic name).";
  static const std::string hint =
      "define identifier (classic name) like:"
      "\n  - rule `[a-zA-Z_][a-zA-Z0-9_]*`"
      "\n  - first character is alphabetical or `_`"
      "\n  - other character is alphanumeric or `_`";

  const std::string final_msg  = p_msg.empty() ? msg : p_msg;
  const std::string final_hint = p_hint.empty() ? hint : p_hint;

  return tok_v.expect(777, TokTy::IDENTIFIER, final_msg, final_hint).val;
}
