#include "parser_statement.hpp"

#include "ast/ast_declaration_local.hpp"
#include "ast/ast_statement.hpp"
#include "compiler/compilation_unit.hpp"
#include "nexus/ast/data.hpp"
#include "nexus/ast/definition.hpp"
#include "nexus/ast/forward.hpp"
#include "nexus/forward.hpp"
#include "nexus/ids.hpp"
#include "nexus/lexer/token.hpp"
#include "nexus/type/type.hpp"
#include "parser_base.hpp"
#include "parser_context.hpp"
#include "parser_declaration_local.hpp"
#include "parser_expression.hpp"
#include "parser_operation.hpp"


ast::ID parser::Parser_Statement::parse_statement(bool p_is_silent_error)
{
  constexpr std::string_view hint =
      "you can define in functions: variable, form, enum, safe cast, if, "
      "else, do, while, match, break, continue, return";

  (void)p.match(token::ETokenKind::SEMICOLON); // consume because the parser use only for explicit end instruction

  switch (p.peek().kind) {
  case token::ETokenKind::IF:       return if_statement();
  case token::ETokenKind::FOR:      return for_statement();
  case token::ETokenKind::WHILE:
  case token::ETokenKind::DO_WHILE: return while_statement();
  case token::ETokenKind::LOOP:     return loop_statement();
  case token::ETokenKind::MATCH:    return match_statement();
  case token::ETokenKind::BREAK:    {
    (void)p.match(token::ETokenKind::BREAK);
    auto& node      = p.add_get_node<ast::Statement_Break>(p.peek(-1).tokid);
    node.breakeable = p.current_breakable;
    return node.nodeid();
  }
  case token::ETokenKind::END: {
    (void)p.match(token::ETokenKind::END);
    auto& node      = p.add_get_node<ast::Statement_Return>(p.peek(-1).tokid);
    node.returnable = p.current_returnable;
    return node.nodeid();
  }
  case token::ETokenKind::CONTINUE: {
    (void)p.match(token::ETokenKind::CONTINUE);
    auto& node       = p.add_get_node<ast::Statement_Continue>(p.peek(-1).tokid);
    node.continuable = p.current_breakable;
    return node.nodeid();
  }
  case token::ETokenKind::RETURN:     return return_flow();
  case token::ETokenKind::GOTO:       return goto_statement();
  case token::ETokenKind::GOTO_LABEL: return goto_label_statement();
  default:                            break;
  }

  if (!p_is_silent_error) {
    p.add_error(116,
                "Unexpected '" + std::string(p.tok_to_str(p.peek().tokid))
                    + "' keyword not allowed in function statement.",
                hint);
    THROW_BAD_NODE;
  }
  return BAD_NODE_ID;
}

ast::ID parser::Parser_Statement::if_statement()
{
  auto& node = p.add_get_node<ast::Statement_If>(p.peek().tokid);

  (void)p.match(token::ETokenKind::IF);
  (void)p.match(token::ETokenKind::ELIF);

  p.enter_scope(node.nodeid(), p.peek(-1).kind == token::ETokenKind::IF ? "if" : "elif");

  node.evaluator = p.p_loc->parse_evaluator(ast::ID::invalid());
  assert(node.evaluator && "Invalid id");

  node.codeblock = p.p_loc->parse_codeblock_instruction();

  p.exit_scope(); // exit if scope

  // else or else if case
  if (p.match(token::ETokenKind::ELIF)) {
    auto  node_elif_id = if_statement(); // recursive call
    auto* node_elif    = node_elif_id.as<ast::Statement_If>();
    node_elif->is_elif = true;

    node.alternative_statement = node_elif->nodeid();
  } else if (p.match(token::ETokenKind::ELSE)) {
    auto& node_else = p.add_get_node<ast::Statement_If>(p.peek(-1).tokid);
    p.enter_scope(node_else.nodeid(), "else");
    node_else.codeblock = p.p_loc->parse_codeblock_instruction();
    node_else.is_else   = true;

    node.alternative_statement = node_else.nodeid();
    p.exit_scope(); // exit else scope
  }

  return node.nodeid();
}

ast::ID parser::Parser_Statement::for_statement()
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

  auto&      node      = p.add_get_node<ast::Statement_For>(p.peek().tokid);
  const auto old_break = p.current_breakable;
  p.current_breakable  = node.nodeid();
  (void)p.match(token::ETokenKind::FOR);

  p.enter_scope(node.nodeid(), "for");

  // index
  if (p.check(token::ETokenKind::IDENTIFIER)) {
    const auto tokid = p.peek().tokid;
    auto&      index =
        p.p_base->inject_variable(p.parse_name("", hint), ast::EVariableKind::_let, type::TYPEID_ssize, NO_ID);
    index.header.start_tokid = tokid;
    node.index               = index.nodeid();
  }
  // items
  if (p.check_any(token::k_capability)) {
    ast::ECapability capa = ast::ETokenKind_to_ECapability(p.next().kind);

    if (p.match(token::ETokenKind::L_PAREN)) {
      while (!p.is_end()) {
        const auto tokid        = p.peek().tokid;
        auto&      item         = p.p_base->inject_capability(p.parse_name("", hint), capa, NO_ID, NO_ID);
        item.header.start_tokid = tokid;

        node.items.emplace_back(item.nodeid());

        if (p.match_field_separator(token::ETokenKind::COMMA, token::ETokenKind::R_PAREN)) break;
      }
    } else {
      const auto tokid        = p.peek().tokid;
      auto&      item         = p.p_base->inject_capability(p.parse_name("", hint), capa, NO_ID, NO_ID);
      item.header.start_tokid = tokid;

      node.items.emplace_back(item.nodeid());
    }
  }

  (void)p.expect(117, token::ETokenKind::IN, "Expected in keyword 'in' after for identifier.", hint);

  node.expression = p.p_expr->parse_expression();

  if (node.index) node.index.as<ast::Local_Variable>()->expression = node.expression;
  for (auto& elem : node.items) {
    auto* n       = elem.as<ast::Local_Capability>();
    n->expression = node.expression;
  }

  node.codeblock = p.p_loc->parse_codeblock_instruction();

  p.exit_scope();

  p.current_breakable = old_break;

  return node.nodeid();
}

ast::ID parser::Parser_Statement::loop_statement()
{
  constexpr std::string_view hint = "define loop statement like: `loop { ... }`";

  auto&      node      = p.add_get_node<ast::Statement_Loop>(p.peek().tokid);
  const auto old_break = p.current_breakable;
  p.current_breakable  = node.nodeid();
  (void)p.match(token::ETokenKind::LOOP);
  p.enter_scope(node.nodeid(), "loop");

  node.codeblock = p.p_loc->parse_codeblock_instruction();

  p.exit_scope();

  p.current_breakable = old_break;

  return node.nodeid();
}

ast::ID parser::Parser_Statement::while_statement()
{
  constexpr std::string_view while_hint =
      R"(define while statement like:
  - `while <condition> { ... }`
  - `while <confition> => ...`)";
  constexpr std::string_view do_while_hint =
      R"(define do-while statement like:
  - `do { ... } while condition;`
  - `do => ... while confition;`)";

  auto&      node      = p.add_get_node<ast::Statement_While>(p.peek().tokid);
  const auto old_break = p.current_breakable;
  p.current_breakable  = node.nodeid();
  p.enter_scope(node.nodeid(), "while");

  if (p.match(token::ETokenKind::WHILE)) {
    node.is_do = false;

    node.evaluator = p.p_loc->parse_evaluator(ast::ID::invalid());

    node.codeblock = p.p_loc->parse_codeblock_instruction();
  } else if (p.match(token::ETokenKind::DO_WHILE)) {
    node.is_do = true;

    node.codeblock = p.p_loc->parse_codeblock_instruction();

    (void)p.expect(118, token::ETokenKind::WHILE, "Expected while keyword after do statement.", do_while_hint);

    node.evaluator = p.p_loc->parse_evaluator(ast::ID::invalid());

    (void)p.match(token::ETokenKind::SEMICOLON);
  } else {
    p.add_error(119, "Expected do or while keyword!", do_while_hint);
  }

  p.exit_scope();

  p.current_breakable = old_break;

  return node.nodeid();
}

ast::ID parser::Parser_Statement::match_statement()
{
  constexpr std::string_view hint =
      R"(define match like:
  `match value {
     case > 100 => ...
     case in 0..=10 => { ... }
     case Validation::Valid(a) => { ... }
     _ => { ... }
   }`)";

  auto&      node      = p.add_get_node<ast::Statement_Match>(p.peek().tokid);
  const auto old_break = p.current_breakable;
  p.current_breakable  = node.nodeid();

  (void)p.match(token::ETokenKind::MATCH);
  p.enter_scope(node.nodeid(), "match");

  node.base = p.p_expr->parse_expression();

  (void)p.expect(120, token::ETokenKind::L_CURLY, "Expected start code block '{' after match definition.", hint);
  bool otherDefine = false;

  // check all cases
  while (!p.is_end()) {
    if (otherDefine) p.add_error(121, "Expected end match '}' after the other '_ =>' case definition.", hint);

    if (p.match(token::ETokenKind::R_CURLY)) {
      if (node.cases.empty()) {
        p.add_error(122, "Match case without any case defined", hint);
      }
      break;
    }

    if (p.match(token::ETokenKind::UNDERSCORE)) {
      auto& n_case     = p.add_get_node<ast::Statement_Match_Case>(p.peek().tokid);
      n_case.codeblock = p.p_loc->parse_codeblock_instruction();
      otherDefine      = true;
      break;
    }

    auto& n_case = p.add_get_node<ast::Statement_Match_Case>(p.peek().tokid);

    if (p.match_any({token::ETokenKind::CAPA_MUT, token::ETokenKind::CAPA_REF})) {
      n_case.evaluator = p.p_loc->parse_pattern(node.base);
    } else {
      n_case.evaluator = p.p_expr->parse_expression();
    }

    n_case.codeblock = p.p_loc->parse_codeblock_instruction();

    if (p.match_field_separator(token::ETokenKind::S_END_OF_FILE, token::ETokenKind::R_CURLY)) break;
  }

  p.exit_scope();

  p.current_breakable = old_break;

  return node.nodeid();
}

ast::ID parser::Parser_Statement::goto_statement()
{
  (void)p.match(token::ETokenKind::GOTO);
  auto& node = p.add_get_node<ast::Statement_GoTo>(p.peek().tokid);
  node.label = p.parse_name("", "define goto statement like: `goto name`.");
  return node.nodeid();
}

ast::ID parser::Parser_Statement::return_flow()
{
  auto& node      = p.add_get_node<ast::Statement_Return>(p.peek().tokid);
  node.returnable = p.current_returnable;
  (void)p.match(token::ETokenKind::RETURN);

  if (p.match(token::ETokenKind::SEMICOLON)) return node.nodeid();
  if (p.check(token::ETokenKind::R_CURLY)) return node.nodeid();

  // return with value
  node.value = p.p_expr->parse_expression();
  return node.nodeid();
}

ast::ID parser::Parser_Statement::goto_label_statement()
{
  constexpr std::string_view hint = "define goto label like: `label my_label:`";

  (void)p.match(token::ETokenKind::GOTO_LABEL);

  auto& node = p.add_get_node<ast::Statement_GoTo_Label>(p.peek().tokid);
  node.label = p.parse_name("", hint);

  (void)p.add_definition(node.nodeid());

  node.codeblock = p.p_loc->parse_codeblock_instruction();

  return node.nodeid();
}
