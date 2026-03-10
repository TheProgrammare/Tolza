#include "visitor_print.hpp"

#include "ast/ast_base.hpp"
#include "ast/ast_declaration.hpp"
#include "ast/ast_declaration_cop.hpp"
#include "ast/ast_generic.hpp"
#include "ast/ast_literal.hpp"
#include "ast/ast_memory.hpp"
#include "ast/ast_operation.hpp"
#include "ast/ast_statement.hpp"
#include "ast/ast_expression.hpp"
#include "ast/ast_type.hpp"
#include "compiler.hpp"

#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <llvm-19/llvm/IR/Type.h>
#include <ostream>
#include <sstream>

fs::path Visitor_Print::get_file_path() const
{
  fs::path path = compiler::COMP_CTX.get_debug_graph_dir() / scr_info.file_path.filename();
  path.replace_extension(".html");
  return path;
}


// ============ AST ============
void Visitor_Print::visit(ast::Node& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  sstr << "</ul></li>\n";
}

void Visitor_Print::visit(ast::AType& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::ALiteral& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::ADeclaration& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::ALocal& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::AExpression& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::AIdentifier& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::Expr_ID& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::Expr_ID_Qualified& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::Expr_ID_Type& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  for (auto& elem : n.gen_args) elem->accept(*this);
  sstr << "</ul></li>\n";
}

void Visitor_Print::visit(ast::Root& n)
{
  std::fstream out_file;

  try {
    fs::create_directories(get_file_path().parent_path());
  } catch (const std::exception& e) {
    std::cerr << "Cannot create directory at " << get_file_path().parent_path() << ": " << e.what() << std::endl;
  }

  try {
    sstr.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    out_file.open(get_file_path(), std::ios::out);
  } catch (const std::ios_base::failure& e) {
    std::cerr << "Cannot create file at " << get_file_path() << ": " << e.what() << std::endl;
    return;
  }

  std::string f_template = PRINT_FILE_HTLM_TEMPLATE;

  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  for (auto& elem : n.global_nodes) elem->accept(*this);
  sstr << "</ul></li>\n";

  compiler::fmt_template(f_template, {compiler::VELOX_COMPILER_VERSION, scr_info.file_path.filename(), sstr.str()});

  out_file.clear();
  out_file << f_template;
  out_file.close();
}

// ============ DECLARATION ============
void Visitor_Print::visit(ast::declaration::Global& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  n.expression->accept(*this);
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::declaration::Function& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  if (n.codeblock) n.codeblock->accept(*this);
  sstr << "</ul></li>\n";
}

void Visitor_Print::visit(ast::declaration::Mod& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  for (auto& elem : n.declarations) elem->accept(*this);
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::declaration::Export& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  for (auto& elem : n.declarations) elem->accept(*this);
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::declaration::Extern& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  for (auto& elem : n.declarations) elem->accept(*this);
  sstr << "</ul></li>\n";
}

void Visitor_Print::visit(ast::declaration::Enum& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  for (auto& elem : n.variants) elem->accept(*this);
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::declaration::Enum_Element& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  sstr << "</ul></li>\n";
}

void Visitor_Print::visit(ast::declaration::Flag& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  for (auto& field : n.fields) {
    sstr << "<li class='node'>" << field << "<ul class='children'>\n";
    sstr << "</ul></li>\n";
  }
  sstr << "</ul></li>\n";
}

void Visitor_Print::visit(ast::declaration::Type_Alias& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::declaration::Mod_Alias& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  sstr << "</ul></li>\n";
}

void Visitor_Print::visit(ast::declaration::Generic& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  for (auto& elem : n.gen_args) elem->accept(*this);
  for (auto& elem : n.conditions) elem->accept(*this);
  sstr << "</ul></li>\n";
}

// ============ LOCAL ============
void Visitor_Print::visit(ast::declaration::local::CodeBlock& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  for (auto& elem : n.elements) {
    if (auto elem_n = elem.node()) elem_n->accept(*this);
  }
  sstr << "</ul></li>\n";
}

void Visitor_Print::visit(ast::declaration::local::Lambda& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  n.codeblock->accept(*this);
  n.capture->accept(*this);
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::declaration::local::Lambda_Capture& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  for (auto& elem : n.elements) elem->accept(*this);
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::declaration::local::Capture_Member& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  sstr << "</ul></li>\n";
}

void Visitor_Print::visit(ast::declaration::local::Parameter& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::declaration::local::Generic_Parameter_Element& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  for (auto& elem : n.generic_references) elem->accept(*this);
  sstr << "</ul></li>\n";
}

void Visitor_Print::visit(ast::declaration::local::Generic_Parameters& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  sstr << "</ul></li>\n";
}

void Visitor_Print::visit(ast::declaration::local::Pattern& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  if (n.additive_evaluator) n.additive_evaluator->accept(*this);
  if (n.right) n.right->accept(*this);
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::declaration::local::Pattern_Enum& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  for (auto& elem : n.mapping) {
    if (auto node = elem.node()) node->accept(*this);
  }
  Visitor_Print::visit(static_cast<ast::declaration::local::Pattern&>(n));
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::declaration::local::Pattern_Tuple& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  for (auto& elem : n.mapping) {
    if (auto node = elem.node()) node->accept(*this);
  }
  Visitor_Print::visit(static_cast<ast::declaration::local::Pattern&>(n));
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::declaration::local::Pattern_Entity& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  for (auto& elem : n.mapping) {
    for (auto& [_, elem2] : elem->mapping) elem2.node()->accept(*this);
  }
  Visitor_Print::visit(static_cast<ast::declaration::local::Pattern&>(n));
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::declaration::local::Pattern_Component& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  for (auto& [_, elem] : n.mapping) {
    if (auto node = elem.node()) node->accept(*this);
  }
  Visitor_Print::visit(static_cast<ast::declaration::local::Pattern&>(n));
  sstr << "</ul></li>\n";
}

void Visitor_Print::visit(ast::declaration::local::Variable_Binding& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::declaration::local::Variable_Unpack& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  for (auto& elem : n.elements) elem->accept(*this);
  n.right->accept(*this);
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::declaration::local::Variable& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  if (n.expression) n.expression->accept(*this);
  sstr << "</ul></li>\n";
}

void Visitor_Print::visit(ast::declaration::local::Capability& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  n.right->accept(*this);
  sstr << "</ul></li>\n";
}

// ============ COP ============
void Visitor_Print::visit(ast::declaration::cop::Component& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  for (auto& elem : n.fields) elem->accept(*this);
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::declaration::cop::Component_Field& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  sstr << "</ul></li>\n";
}

void Visitor_Print::visit(ast::declaration::cop::Role& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  for (auto& elem : n.components) elem->accept(*this);
  sstr << "</ul></li>\n";
}

void Visitor_Print::visit(ast::declaration::cop::Entity& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  for (auto& elem : n.comps) elem->accept(*this);
  for (auto& [proto, elem] : n.constructors) {
    proto->accept(*this);
    elem->accept(*this);
  }
  if (n.gen_params) n.gen_params->accept(*this);
  for (auto& elem : n.operators) elem->accept(*this);
  for (auto& elem : n.casts) elem->accept(*this);
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::declaration::cop::Entity_Cast& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  n.source->accept(*this);
  n.target->accept(*this);
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::declaration::cop::Entity_Op& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  n.codeblock->accept(*this);
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::declaration::cop::Entity_OpIndex& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  n.codeblock->accept(*this);
  n.return_type->accept(*this);
  sstr << "</ul></li>\n";
}

void Visitor_Print::visit(ast::declaration::cop::System& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  n.prototype->accept(*this);
  for (auto& elem : n.cases) elem->accept(*this);
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::declaration::cop::System_Case& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  for (auto& elem : n.bindings) elem->accept(*this);
  n.codeblock->accept(*this);
  sstr << "</ul></li>\n";
}

// ============ GENERIC ============
void Visitor_Print::visit(ast::generic::Is_Type& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  for (auto& elem : n.inType) elem->accept(*this);
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::generic::Can_Cast& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  n.target->accept(*this);
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::generic::Have_Op& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  if (n.explicit_return_type) n.explicit_return_type->accept(*this);
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::generic::Have_Role& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  n.role->accept(*this);
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::generic::Use_Component& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  n.component->accept(*this);
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::generic::Compatible_System& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  n.system->accept(*this);
  sstr << "</ul></li>\n";
}

// ============ TYPE ============
void Visitor_Print::visit(ast::type::Ptr& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  n.inner->accept(*this);
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::type::Table& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  if (n.sizeSymbol) n.sizeSymbol->accept(*this);
  n.inner->accept(*this);
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::type::Primitive& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::type::Tuple& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  for (auto& elem : n.types) elem->accept(*this);
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::type::Function_Proto& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  for (auto& elem : n.parameters) elem->accept(*this);
  for (auto& elem : n.gen_parameters) elem->accept(*this);
  if (n.returnType) n.returnType->accept(*this);
  sstr << "</ul></li>\n";
}

void Visitor_Print::visit(ast::type::Get_Expr_Type& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  n.target->accept(*this);
  sstr << "</ul></li>\n";
}

// ============ LITERAL ============
void Visitor_Print::visit(ast::literal::Boolean& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::literal::Integral& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::literal::Decimal& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::literal::Floating& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  sstr << "</ul></li>\n";
}

void Visitor_Print::visit(ast::literal::ASCII& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::literal::UTF32& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  sstr << "</ul></li>\n";
}

void Visitor_Print::visit(ast::literal::Text& n)
{
  sstr << "<li class='node'> text " << n.debug_str() << "<ul class='children'>\n";
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::literal::Text_Interpolation& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  if (n.expression) n.expression->accept(*this);
  if (n.spec) n.spec->accept(*this);
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::literal::Textual_Element& n)
{
  sstr << "<li class='node'> elem " << (n.kind == ast::literal::Textual_Element::Kind::Text ? "text" : "interpolation")
       << "<ul class='children'>\n";
  n.val->accept(*this);
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::literal::Textual_Format& n)
{
  for (auto& elem : n.values) elem.val->accept(*this);
}
void Visitor_Print::visit(ast::literal::Format_Specifier& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  if (n.width) n.width->accept(*this);
  if (n.precision) n.precision->accept(*this);
  sstr << "</ul></li>\n";
}

void Visitor_Print::visit(ast::literal::Table& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  for (auto& elem : n.values) elem->accept(*this);
  if (n.population) n.population->accept(*this);
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::literal::Table_Population& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  for (auto& elem : n.ranges) elem->accept(*this);
  n.expression->accept(*this);
  if (n.map_expression_value) n.map_expression_value->accept(*this);
  sstr << "</ul></li>\n";
}

void Visitor_Print::visit(ast::literal::Map& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  for (auto& elem : n.keys) elem->accept(*this);
  for (auto& elem : n.values) elem->accept(*this);
  if (n.population) n.population->accept(*this);
  sstr << "</ul></li>\n";
}

void Visitor_Print::visit(ast::literal::Tuple& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  for (auto& elem : n.values) elem->accept(*this);
  for (auto& elem : n.tys) elem->accept(*this);
  sstr << "</ul></li>\n";
}

void Visitor_Print::visit(ast::literal::Range& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  if (n.start) n.start->accept(*this);
  if (n.end) n.end->accept(*this);
  if (n.step) n.step->accept(*this);
  sstr << "</ul></li>\n";
}

void Visitor_Print::visit(ast::literal::Iterator& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  n.collection->accept(*this);
  sstr << "</ul></li>\n";
}


void Visitor_Print::visit(ast::literal::Enum& n)
{
  sstr << "<li class ='node'>" << n.debug_str() << "<ul class='children'>\n";
  for (auto& elem : n.member_values) elem->accept(*this);
  sstr << "</ul></li>\n";
}

void Visitor_Print::visit(ast::literal::Component& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  n.name->accept(*this);
  for (auto& elem : n.field_args) elem->accept(*this);
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::literal::Entity& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  n.name->accept(*this);
  for (auto& elem : n.comp_args) elem->accept(*this);
  sstr << "</ul></li>\n";
}

// ============ Expression ============
void Visitor_Print::visit(ast::expression::If_Ternary& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  n.evaluator.node()->accept(*this);
  n.true_line->accept(*this);
  n.false_line->accept(*this);
  sstr << "</ul></li>\n";
}

void Visitor_Print::visit(ast::expression::Member_Access& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  n.left->accept(*this);
  n.right->accept(*this);
  sstr << "</ul></li>\n";
}

void Visitor_Print::visit(ast::expression::Self& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::expression::Other& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  sstr << "</ul></li>\n";
}

void Visitor_Print::visit(ast::expression::Call& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::expression::Call_Argument& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::expression::Call_System& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::expression::Call_Pipe& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  for (auto& elem : n.gen_args) {
    for (auto& elem1 : elem) elem1->accept(*this);
  }
  for (auto& elem : n.arguments) {
    for (auto& elem1 : elem) elem1->accept(*this);
  }
  sstr << "</ul></li>\n";
}

void Visitor_Print::visit(ast::expression::Table_Access& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  n.selector->accept(*this);
  sstr << "</ul></li>\n";
}

void Visitor_Print::visit(ast::expression::Ptr_At& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  n.target->accept(*this);
  n.index->accept(*this);
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::expression::Ptr_Offset& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  n.target->accept(*this);
  n.offset->accept(*this);
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::expression::Ptr_Val& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  n.target->accept(*this);
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::expression::Addr_Of& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  n.target->accept(*this);
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::expression::Size_Of& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  n.target->accept(*this);
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::expression::GetBits& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  n.target->accept(*this);
  n.range->accept(*this);
  sstr << "</ul></li>\n";
}

void Visitor_Print::visit(ast::expression::Move& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  n.target->accept(*this);
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::expression::New_Ptr& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  n.type->accept(*this);
  n.expression->accept(*this);
  sstr << "</ul></li>\n";
}

// ============ STATEMENT ============
void Visitor_Print::visit(ast::statement::If& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  if (auto node = n.evaluator.node()) node->accept(*this);
  n.codeblock->accept(*this);
  if (n.alternative_statement) n.alternative_statement->accept(*this);
  sstr << "</ul></li>\n";
}

void Visitor_Print::visit(ast::statement::For& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  n.src->accept(*this);
  if (n.index) n.index->accept(*this);
  for (auto& elem : n.items) elem->accept(*this);
  n.codeblock->accept(*this);
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::statement::Loop& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  n.codeblock->accept(*this);
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::statement::While& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  if (auto node = n.evaluator.node()) node->accept(*this);
  n.codeblock->accept(*this);
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::statement::GoTo& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::statement::GoTo_Label& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  sstr << "</ul></li>\n";
}

void Visitor_Print::visit(ast::statement::Return& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  if (n.value) n.value->accept(*this);
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::statement::Break& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::statement::Continue& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  sstr << "</ul></li>\n";
}

void Visitor_Print::visit(ast::statement::Match& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  n.base->accept(*this);
  for (auto& elem : n.cases) elem->accept(*this);
  if (n.other_case) n.other_case->accept(*this);
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::statement::Match_Case& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  if (auto node = n.evaluator.node()) node->accept(*this);
  n.codeblock->accept(*this);
  sstr << "</ul></li>\n";
}

// ============ OPERATION ============
void Visitor_Print::visit(ast::operation::Cast_As& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  n.valueCasted->accept(*this);
  n.typeCasted->accept(*this);
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::operation::Is& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  n.left->accept(*this);
  n.right->accept(*this);
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::operation::In& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  n.left->accept(*this);
  n.right->accept(*this);
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::operation::Assignment& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  n.left->accept(*this);
  n.right->accept(*this);
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::operation::Binary& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  n.left->accept(*this);
  n.right->accept(*this);
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::operation::Unary& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  n.base->accept(*this);
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::operation::Interval& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  n.left->accept(*this);
  n.center->accept(*this);
  n.right->accept(*this);
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::operation::Ptr_Dist& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  n.left->accept(*this);
  n.right->accept(*this);
  sstr << "</ul></li>\n";
}

// ============ MEMORY ============
void Visitor_Print::visit(ast::memory::Del& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  n.target->accept(*this);
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::memory::Align& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  n.target->accept(*this);
  sstr << "</ul></li>\n";
}
void Visitor_Print::visit(ast::memory::Drop& n)
{
  sstr << "<li class='node'>" << n.debug_str() << "<ul class='children'>\n";
  n.target->accept(*this);
  sstr << "</ul></li>\n";
}

const char* PRINT_FILE_HTLM_TEMPLATE =
    R"(
<!DOCTYPE html>

<html lang="en">
<head>
<meta charset="UTF-8">
<title>Velox AST View</title>


<style>
  body {
    font-family: Consolas, "Courier New", monospace;
    font-size: 12px;
    background: #1E1E1E;
    color: #D4D4D4;
    margin: 20px;
  }

  h1 {
    font-size: 18px;
    margin-bottom: 10px;
  }

  h2 {
    font-size: 14px;
    margin-bottom: 5px;
  }

  #controls {
    margin-bottom: 10px;
    display: flex;
    flex-direction: row;
    gap: 10px;
  }

  button {
    padding: 6px 10px;
    font-size: 12px;
    cursor: pointer;
    border: 1px solid #5c5c5c;
    border-radius: 15px;
    background: #333333;
    width: 150px;
    color: #D4D4D4;
  }

  ul {
    list-style-type: none;
    padding-left: 1em;
  }

  li {
    margin: 2px 0;
    line-height: 2em;
  }

  .node {
    cursor: pointer;
    font-weight: 600;
    position: relative;
    padding-left: 1em;
    transition: background 0.2s;
    border-radius: 15px;
  }

  .node:hover {
    background: #1e1e1e27;
  }

  .leaf {
    cursor: default;
    font-weight: normal;
    padding-left: 1em;
    background: #22123b2d;
    border-radius: 15px;
  }

  .children {
    display: none;
    margin-left: 1em;
  }

  .expanded > .children {
    display: block;
  }

  .node::before {
    content: "▶";
    display: inline-block;
    width: 1em;
    transition: transform 0.2s;
  }

  .expanded::before {
    content: "▼";
  }

  .leaf::before {
    content: "● ";
  }

  #ast-container {
    background: #333333;
    padding: 10px;
    border-radius: 15px;
    box-shadow: 0 2px 6px rgba(0,0,0,0.1);
    max-height: 80vh;
    overflow: auto;
  }
</style>
</head>
<body>

<h1>Velox AST View - %1</h1>
<h2>Auto generated .html file by velox-compiler version %0</h2>

<div id="controls">
  <button id="expand-all">Expand all</button>
  <button id="collapse-all">Collapse all</button>
</div>

<div id="ast-container">
<ul id="ast">
  %2
</ul>
</div>

<script>
  const astNodes = document.querySelectorAll('#ast li');

  astNodes.forEach(node => {
    const childrenUl = node.querySelector(':scope > .children');
    const hasChildLi = childrenUl && childrenUl.querySelector('li');
    if (hasChildLi) {
      node.addEventListener('click', e => {
        e.stopPropagation();
        node.classList.toggle('expanded');
      });
    } else {
      node.classList.remove('node');
      node.classList.add('leaf');
    }
  });

  document.getElementById('expand-all').addEventListener('click', () => {
    astNodes.forEach(node => {
      if (node.classList.contains('node')) node.classList.add('expanded');
    });
  });

  document.getElementById('collapse-all').addEventListener('click', () => {
    astNodes.forEach(node => {
      if (node.classList.contains('node')) node.classList.remove('expanded');
    });
  });
</script>

</body>
</html>
)";