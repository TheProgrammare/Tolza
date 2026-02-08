#include "ASTViewer.hpp"

#include "Globals.hpp"

#include "ScriptInfo.hpp"

#include "AST/AST_Base.hpp"
#include "AST/AST_CodeBlock_Instruction.hpp"
#include "AST/AST_Data.hpp"
#include "AST/AST_Declaration_COP.hpp"
#include "AST/AST_Declaration_Local.hpp"
#include "AST/AST_Declaration.hpp"
#include "AST/AST_Evaluator.hpp"
#include "AST/AST_Generic.hpp"
#include "AST/AST_Literal.hpp"
#include "AST/AST_Memory.hpp"
#include "AST/AST_Operation.hpp"
#include "AST/AST_Reference.hpp"
#include "AST/AST_Statement.hpp"
#include "AST/AST_Type.hpp"

void AST_Viewer::parent_dot(const AST::Node &n, const std::string &context)
{
	std::string l = n.debug_str();
	if (n.debug_str().empty()) l = "Node";
	os_ << "  n" << get_id(n) << " [label=\"" << escapeDot(l) << "\"]\n";
}

void AST_Viewer::child_dot(const AST::Node &parent, const AST::Node &child, const std::string &context)
{
	parent_dot(child, context);
	os_ << "  n" << get_id(parent) << " -> n" << get_id(child) << " [label=\"" << escapeDot(context) << "\"]\n";
}
