#include "ast_viewer.hpp"

#include "compiler.hpp"

#include "script_info.hpp"

#include "ast/ast_base.hpp"
#include "ast/ast_codeblock_instruction.hpp"
#include "ast/ast_data.hpp"
#include "ast/ast_declaration.hpp"
#include "ast/ast_declaration_cop.hpp"
#include "ast/ast_declaration_local.hpp"
#include "ast/ast_evaluator.hpp"
#include "ast/ast_generic.hpp"
#include "ast/ast_literal.hpp"
#include "ast/ast_memory.hpp"
#include "ast/ast_operation.hpp"
#include "ast/ast_statement.hpp"
#include "ast/ast_type.hpp"

void AST_Viewer::parent_dot(const ast::Node& n, const std::string& context)
{
  std::string l = n.debug_str();
  if (n.debug_str().empty()) l = "Node";
  os_ << "  n" << get_id(n) << " [label=\"" << escapeDot(l) << "\"]\n";
}

void AST_Viewer::child_dot(const ast::Node& parent, const ast::Node& child, const std::string& context)
{
  parent_dot(child, context);
  os_ << "  n" << get_id(parent) << " -> n" << get_id(child) << " [label=\"" << escapeDot(context) << "\"]\n";
}
