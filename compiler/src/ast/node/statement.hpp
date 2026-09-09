#pragma once

#include "ast/node/base.hpp"

#include <string>


namespace ast
{

struct Statement_If final {
  NODE_HEADER(Statement_If);

  SET_NODE(evaluator);
  SET_NODE(codeblock);
  SET_NODE(alternative_statement);
  bool is_else = false;
  bool is_elif = false;
};

// for i in range {}
struct Statement_For final {
  NODE_HEADER(Statement_For);

  SET_NODE(expression);
  // variable binding
  SET_NODE(index);
  // variable binding
  SET_VECTOR_NODE(items);
  SET_NODE(codeblock);
  bool is_reverse = false;
};

// loop {...}
struct Statement_Loop final {
  NODE_HEADER(Statement_Loop);

  SET_NODE(codeblock);
};

// while condition {...}
struct Statement_While final {
  NODE_HEADER(Statement_While);

  SET_NODE(evaluator);
  SET_NODE(codeblock);
  bool is_do = false;
};

// normally not an expression
struct Statement_GoTo final {
  NODE_HEADER(Statement_GoTo);

  std::string label;
};

// label azerty {...}
struct Statement_GoTo_Label final {
  NODE_HEADER(Statement_GoTo_Label);

  std::string label;
  SET_NODE(codeblock);
};

// return a, b, c
struct Statement_Return final {
  NODE_HEADER(Statement_Return);

  SET_NODE(value);

  SET_NODE(returnable);
};

struct Statement_Break final {
  NODE_HEADER(Statement_Break);

  SET_NODE(breakeable);
};

struct Statement_Continue final {
  NODE_HEADER(Statement_Continue);

  SET_NODE(continuable);
};

// constant/comparison => {}
struct Statement_Match_Case final {
  NODE_HEADER(Statement_Match_Case);

  SET_NODE(evaluator);
  SET_NODE(codeblock);
};

// match <base> { <const/comparison> => {...} _ => {...} }
struct Statement_Match final {
  NODE_HEADER(Statement_Match);

  SET_NODE(base);
  SET_VECTOR_NODE(cases);
  SET_NODE(other_case);
};

} // namespace ast
  // AST