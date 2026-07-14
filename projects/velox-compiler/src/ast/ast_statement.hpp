#pragma once

#include "nexus/ast/ast.hpp"


namespace ast
{

AST_NODE(Statement_If)
{
  SET_NODE(evaluator);
  SET_NODE(codeblock);
  SET_NODE(alternative_statement);
  bool is_else = false;
  bool is_elif = false;
};

// for i in range {}
AST_NODE(Statement_For)
{
  SET_NODE(expression);
  // variable binding
  SET_NODE(index);
  // variable binding
  SET_VECTOR_NODE(items);
  SET_NODE(codeblock);
  bool is_reverse = false;
};

// loop {...}
AST_NODE(Statement_Loop)
{
  SET_NODE(codeblock);
};

// while condition {...}
AST_NODE(Statement_While)
{
  SET_NODE(evaluator);
  SET_NODE(codeblock);
  bool is_do = false;
};

// normally not an expression
AST_NODE(Statement_GoTo)
{
  std::string label;
};

// label azerty {...}
AST_NODE(Statement_GoTo_Label)
{
  std::string label;
  SET_NODE(codeblock);
};

// return a, b, c
AST_NODE(Statement_Return)
{
  SET_NODE(value);

  SET_NODE(returnable);
};

AST_NODE(Statement_Break)
{
  SET_NODE(breakeable);
};

AST_NODE(Statement_Continue)
{
  SET_NODE(continuable);
};

// constant/comparison => {}
AST_NODE(Statement_Match_Case)
{
  SET_NODE(evaluator);
  SET_NODE(codeblock);
};

// match <base> { <const/comparison> => {...} _ => {...} }
AST_NODE(Statement_Match)
{
  SET_NODE(base);
  SET_VECTOR_NODE(cases);
  SET_NODE(other_case);
};

} // namespace ast
  // AST