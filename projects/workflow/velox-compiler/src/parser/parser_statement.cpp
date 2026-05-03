#include "parser_statement.hpp"

#include "compiler/compiler.hpp"
#include "nexus/ast/ast.hpp"
#include "nexus/forward.hpp"
#include "nexus/ids.hpp"
#include "nexus/lexer/lexer.hpp"
#include "nexus/lexer/token.hpp"

#include "ast/ast_statement.hpp"

#include "nexus/module.hpp"
#include "nexus/script.hpp"
#include "nexus/symbol.hpp"
#include "nexus/type.hpp"
#include "parser_context.hpp"


#include "ast/ast_declaration_global.hpp"
#include "ast/ast_declaration_cop.hpp"
#include "ast/ast_declaration_local.hpp"

#include "ast/ast_statement.hpp"
#include "nexus/type.hpp"

#include "nexus/lexer/token.hpp"

#include "parser_declaration_global.hpp"
#include "parser_declaration_cop.hpp"
#include "parser_declaration_local.hpp"
#include "parser_expression.hpp"
#include "parser_literal.hpp"
#include "parser_memory.hpp"
#include "parser_operation.hpp"
#include "parser_statement.hpp"
#include "parser_type.hpp"
#include <string_view>


ast::_gnid parser::Parser_Statement::parse_statement(bool p_is_silent_error)
{
  constexpr std::string_view hint =
      "you can define in functions: variable, entity, enum, safe cast, if, "
      "else, do, while, match, break, continue, return";

  p.match(token::ETokenKind::SEMICOLON); // consume because the parser use only for explicit end instruction

  switch (p.peek().kind) {
  case token::ETokenKind::IF:       return if_statement();
  case token::ETokenKind::FOR:      return for_statement();
  case token::ETokenKind::WHILE:
  case token::ETokenKind::DO_WHILE: return while_statement();
  case token::ETokenKind::LOOP:     return loop_statement();
  case token::ETokenKind::MATCH:    return match_statement();
  case token::ETokenKind::BREAK:    {
    p.match(token::ETokenKind::BREAK);
    parser_add_node(node, Statement_Break, p.peek(-1).id);
    return node->node_id;
  }
  case token::ETokenKind::END: {
    p.match(token::ETokenKind::END);
    parser_add_node(node, Statement_Return, p.peek(-1).id);
    return node->node_id;
  }
  case token::ETokenKind::CONTINUE: {
    p.match(token::ETokenKind::CONTINUE);
    parser_add_node(node, Statement_Continue, p.peek(-1).id);
    return node->node_id;
  }
  case token::ETokenKind::RETURN:     return return_flow();
  case token::ETokenKind::GOTO:       return goto_statement();
  case token::ETokenKind::GOTO_LABEL: return goto_label_statement();
  default:                            break;
  }

  if (!p_is_silent_error)
    p.add_error(
        116, "Unexpected '" + std::string(p.tok_to_str(p.peek().id)) + "' keyword not allowed in function statement.",
        hint);
  return BAD_NODE_ID;
}

ast::_gnid parser::Parser_Statement::if_statement()
{
  parser_add_node(node, Statement_If, p.peek().id);

  p.match(token::ETokenKind::IF);
  p.match(token::ETokenKind::ELIF);

  p.enter_scope(*node, p.peek(-1).kind == token::ETokenKind::IF ? "if" : "elif");

  node->evaluator = p.p_loc->parse_evaluator(ast::_gnid());

  node->codeblock = p.p_loc->parse_codeblock();

  // else or else if case
  if (p.match(token::ETokenKind::ELIF)) {
    auto node_elif_id  = if_statement(); // recursive call
    auto node_elif     = p.scr_info.nodes->get_as<ast::Statement_If>(node_elif_id.get_node_id());
    node_elif->is_elif = true;

    node->alternative_statement = node_elif->node_id;
  } else if (p.match(token::ETokenKind::ELSE)) {
    parser_add_node(node_else, Statement_If, p.peek(-1).id);
    node_else->codeblock = p.p_loc->parse_codeblock();
    node_else->is_else   = true;

    node->alternative_statement = node_else->node_id;
  }

  p.exit_scope();

  return node->node_id;
}

ast::_gnid parser::Parser_Statement::for_statement()
{
  constexpr std::string_view hint =
      R"(define for state like:
  - for index : `for i in start..end { ... }`
  - for array : `for mut/ref/copy item in array` { ... }
  - for array with index : `for i, mut/ref/copy item in array { ... }`
  - for map : `for mut/ref/copy (item, key) in map { ... }`
  - for map with index : `for i, mut/ref/copy (item, key) in map { ... }`
  - for unpack : `for mut/ref/copy (a, b, ...) in array_tuple { ... }`
  - for unpack with index : `for i, mut/ref/copy (a, b, ...) in array_tuple { ... }`)";

  parser_add_node(node, Statement_For, p.peek().id);

  p.match(token::ETokenKind::FOR);

  p.enter_scope(*node, "for");

  // index
  if (p.check(token::ETokenKind::IDENTIFIER)) {
    parser_add_node(index, Local_Binding, p.peek().id);
    index->name       = p.parse_name("", hint);
    index->capability = ast::ECapability::Mut;

    index->inferred_type = type::TYPEID_iSize;

    p.add_symbol(index->node_id.get_node_id());
    node->index = index->node_id;
  }
  // items
  if (p.check_any({token::k_capability})) {
    ast::ECapability capa = ast::ETokenKind_to_ECapability(p.next().kind);

    if (p.match(token::ETokenKind::OPEN_PAREN)) {
      while (!p.is_end()) {
        parser_add_node(item, Local_Binding, p.peek().id);
        item->capability = capa;
        item->name       = p.parse_name("", hint);

        p.add_symbol(item->node_id.get_node_id());
        node->items.push_back(item->node_id);

        if (p.match_field_separator(token::ETokenKind::COMMA, token::ETokenKind::CLOSE_PAREN)) break;
      }
    } else {
      parser_add_node(item, Local_Binding, p.peek().id);
      item->capability = capa;
      item->name       = p.parse_name("", hint);

      p.add_symbol(item->node_id.get_node_id());
      node->items.push_back(item->node_id);
    }
  }

  p.expect(117, token::ETokenKind::IN, "Expected in keyword 'in' after for identifier.", hint);

  node->expression = p.p_expr->parse_expression();

  node->codeblock = p.p_loc->parse_codeblock();

  p.exit_scope();

  return node->node_id;
}

ast::_gnid parser::Parser_Statement::loop_statement()
{
  constexpr std::string_view hint = "define loop statement like: `loop { ... }`";

  parser_add_node(node, Statement_Loop, p.peek().id);

  p.match(token::ETokenKind::LOOP);
  p.enter_scope(*node, "loop");

  node->codeblock = p.p_loc->parse_codeblock();

  p.exit_scope();

  return node->node_id;
}

ast::_gnid parser::Parser_Statement::while_statement()
{
  constexpr std::string_view while_hint =
      R"(define while statement like:
  - `while <condition> { ... }`
  - `while <confition> => ...`)";
  constexpr std::string_view do_while_hint =
      R"(define do-while statement like:
  - `do { ... } while condition;`
  - `do => ... while confition;`)";

  parser_add_node(node, Statement_While, p.peek().id);
  p.enter_scope(*node, "while");

  if (p.match(token::ETokenKind::WHILE)) {
    node->is_do = false;

    node->evaluator = p.p_loc->parse_evaluator(ast::_gnid());

    node->codeblock = p.p_loc->parse_codeblock();
  } else if (p.match(token::ETokenKind::DO_WHILE)) {
    node->is_do = true;

    node->codeblock = p.p_loc->parse_codeblock();

    p.expect(118, token::ETokenKind::WHILE, "Expected while keyword after do statement.", do_while_hint);

    node->evaluator = p.p_loc->parse_evaluator(ast::_gnid());

    p.match(token::ETokenKind::SEMICOLON);
  } else
    p.add_error(119, "Expected do or while keyword!", do_while_hint);

  p.exit_scope();

  return node->node_id;
}

ast::_gnid parser::Parser_Statement::match_statement()
{
  constexpr std::string_view hint =
      R"(define match like:
  `match value {
     case > 100 => ...
     case in 0..=10 => { ... }
     case Validation::Valid(a) => { ... }
     _ => { ... }
   }`)";

  parser_add_node(node, Statement_Match, p.peek().id);

  p.match(token::ETokenKind::MATCH);
  p.enter_scope(*node, "match");

  node->base = p.p_expr->parse_expression();

  p.expect(120, token::ETokenKind::OPEN_BRACE, "Expected start code block '{' after match defintion.", hint);
  bool otherDefine = false;

  // check all cases
  while (!p.is_end()) {
    if (otherDefine) p.add_error(121, "Expected end match '}' after the other '_ =>' case definition.", hint);

    if (p.match(token::ETokenKind::CLOSE_BRACE)) {
      if (node->cases.empty()) {
        p.add_error(122, "Match case without any case defined", hint);
      }
      break;
    }

    if (p.match(token::ETokenKind::UNDERSCORE)) {
      parser_add_node(n_case, Statement_Match_Case, p.peek().id);
      n_case->codeblock = p.p_loc->parse_codeblock();
      otherDefine       = true;
      break;
    }

    parser_add_node(n_case, Statement_Match_Case, p.peek().id);

    if (p.match_any({token::ETokenKind::CAPA_MUT, token::ETokenKind::CAPA_REF})) {
      n_case->evaluator = p.p_loc->parse_pattern(node->base);
    } else {
      n_case->evaluator = p.p_expr->parse_expression();
    }

    n_case->codeblock = p.p_loc->parse_codeblock();

    if (p.match_field_separator(token::ETokenKind::S_END_OF_FILE, token::ETokenKind::CLOSE_BRACE)) break;
  }

  p.exit_scope();

  return node->node_id;
}

ast::_gnid parser::Parser_Statement::goto_statement()
{
  p.match(token::ETokenKind::GOTO);
  parser_add_node(node, Statement_GoTo, p.peek().id);
  node->label = p.parse_name("", "define goto statement like: `goto name`.");
  return node->node_id;
}

ast::_gnid parser::Parser_Statement::return_flow()
{
  parser_add_node(node, Statement_Return, p.peek().id);
  p.match(token::ETokenKind::RETURN);

  if (p.match(token::ETokenKind::SEMICOLON)) return node->node_id;
  if (p.check(token::ETokenKind::CLOSE_BRACE)) return node->node_id;

  // return with value
  node->value = p.p_expr->parse_expression();
  return node->node_id;
}

ast::_gnid parser::Parser_Statement::goto_label_statement()
{
  constexpr std::string_view hint = "define goto label like: `label my_label:`";

  p.match(token::ETokenKind::GOTO_LABEL);

  parser_add_node(node, Statement_GoTo_Label, p.peek().id);
  node->label = p.parse_name("", hint);

  p.add_symbol(node->node_id.get_node_id());

  node->codeblock = p.p_loc->parse_codeblock();

  return node->node_id;
}
