#include "ast_viewer.hpp"

#include "globals.hpp"

#include "compiler/script_info.hpp"

#include "compiler/ast/ast_base.hpp"
#include "compiler/ast/ast_codeblock_instruction.hpp"
#include "compiler/ast/ast_data.hpp"
#include "compiler/ast/ast_declaration.hpp"
#include "compiler/ast/ast_declaration_cop.hpp"
#include "compiler/ast/ast_declaration_local.hpp"
#include "compiler/ast/ast_evaluator.hpp"
#include "compiler/ast/ast_generic.hpp"
#include "compiler/ast/ast_literal.hpp"
#include "compiler/ast/ast_memory.hpp"
#include "compiler/ast/ast_operation.hpp"
#include "compiler/ast/ast_statement.hpp"
#include "compiler/ast/ast_type.hpp"

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
