#include "ast_printer.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>

#include <compiler_options.hpp>

#include "nexus/ast/ast.hpp"
#include "ast/ast_declaration_global.hpp"
#include "ast/ast_declaration_local.hpp"
#include "ast/ast_declaration_cop.hpp"
#include "ast/ast_generic.hpp"
#include "ast/ast_literal.hpp"
#include "ast/ast_memory.hpp"
#include "ast/ast_operation.hpp"
#include "ast/ast_statement.hpp"
#include "ast/ast_expression.hpp"
#include "nexus/type.hpp"

#include "compiler/compiler.hpp"
#include "nexus/script.hpp"

#include "common.hpp"


/*

std::string AST_Printer::get_file_path() const
{
  std::filesystem::path path =
      std::filesystem::path(compiler::COMPILER_OPTIONS.get_debug_graph_dir()) / scr_info.file_info.get_file_name();
  path.replace_extension(".html");
  return path.string();
}

// ============ AST ============
void AST_Printer::visit(ast::Node& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::AType& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::ALiteral& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::ADeclaration& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::ALocal& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::AExpression& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::AIdentifier& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Expr_ID& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Expr_ID_Qualified& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Expr_ID_Type& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (auto& elem : n.gen_args) elem->accept(*this);
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::Root& n)
{
  std::fstream out_file;

  try {
    std::filesystem::create_directories(std::filesystem::path(get_file_path()).parent_path());
  } catch (const std::exception& e) {
    std::cerr << "Cannot create directory at " << std::filesystem::path(get_file_path()).parent_path() << ": "
              << e.what() << std::endl;
  }

  try {
    out_file.open(get_file_path(), std::ios::out);
  } catch (const std::ios_base::failure& e) {
    std::cerr << "Cannot create file at " << std::filesystem::path(get_file_path()) << ": " << e.what() << std::endl;
    return;
  }

  std::string f_template = PRINT_FILE_HTLM_TEMPLATE;

  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (auto& elem : n.global_nodes) elem->accept(*this);
  out_print += "</ul></li>\n";

  common::fmt_template(f_template, {common::SOFTWARE_VERSION, scr_info.file_info.get_file_name(), out_print});

  out_file.clear();
  out_file << f_template;
  out_file.close();
}

// ============ DECLARATION ============
void AST_Printer::visit(ast::declaration::Global_Variable& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.expression->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::declaration::Function& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  if (n.codeblock) n.codeblock->accept(*this);
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::declaration::Mod& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (auto& elem : n.declarations) elem->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::declaration::Export& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (auto& elem : n.declarations) elem->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::declaration::Extern& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (auto& elem : n.declarations) elem->accept(*this);
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::declaration::Enum& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (auto& elem : n.variants) elem->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::declaration::Enum_Element& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::declaration::Flag& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (auto& field : n.fields) {
    out_print += "<li class='node'>" + field + "<ul class='children'>\n";
    out_print += "</ul></li>\n";
  }
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::declaration::Union& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (auto& [name, field] : n.fields) {
    out_print += "<li class='node'>" + name + "<ul class='children'>\n";
    out_print += "<li class='node'>" + field->debug_str() + "<ul class='children'>\n";
    out_print += "</ul></li>\n";
    out_print += "</ul></li>\n";
  }
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::declaration::Type_Alias& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::declaration::Module_Alias& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::declaration::Global_Generic& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (auto& elem : n.gen_args) elem->accept(*this);
  for (auto& elem : n.conditions) elem->accept(*this);
  out_print += "</ul></li>\n";
}

// ============ LOCAL ============
void AST_Printer::visit(Local_CodeBlock& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (auto& elem : n.elements) {
    if (auto elem_n = elem.node()) elem_n->accept(*this);
  }
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(Local_Lambda& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.codeblock->accept(*this);
  n.capture->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(Local_Lambda_Capture& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (auto& elem : n.elements) elem->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(Local_Capture_Member& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(Local_Parameter& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(Local_Generic_Parameter_Element& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (auto& elem : n.generic_references) elem->accept(*this);
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(Local_Generic_Parameters& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(Local_Pattern& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  if (n.additive_evaluator) n.additive_evaluator->accept(*this);
  if (n.right) n.right->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(Local_Pattern_Enum& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  AST_Printer::visit(static_cast<Local_Pattern&>(n));
  for (auto& elem : n.mapping) {
    if (auto node = elem->node()) node->accept(*this);
  }
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(Local_Pattern_Tuple& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  AST_Printer::visit(static_cast<Local_Pattern&>(n));
  for (auto& elem : n.mapping) {
    if (auto node = elem->node()) node->accept(*this);
  }
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(Local_Pattern_Entity& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  AST_Printer::visit(static_cast<Local_Pattern&>(n));
  for (auto& elem : n.mapping) {
    for (auto& [_, elem2] : elem->mapping) elem2->node()->accept(*this);
  }
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(Local_Pattern_System_Component& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  AST_Printer::visit(static_cast<Local_Pattern&>(n));
  n.bind->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(Local_Pattern_Component& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  AST_Printer::visit(static_cast<Local_Pattern&>(n));
  for (auto& [_, elem] : n.mapping) {
    if (auto node = elem->node()) node->accept(*this);
  }
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(Local_Variable_Binding& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(Local_Tuple_Destructuring& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (auto& elem : n.elements) elem->accept(*this);
  n.right->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(Local_Variable& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  if (n.expression) n.expression->accept(*this);
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(Local_Capability& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.right->accept(*this);
  out_print += "</ul></li>\n";
}

// ============ COP ============
void AST_Printer::visit(ast::declaration::cop::Component& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (auto& elem : n.fields) elem->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::declaration::cop::Component_Field& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::declaration::cop::Role& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (auto& elem : n.components) elem->accept(*this);
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::declaration::cop::Entity& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (auto& elem : n.comps) elem->accept(*this);
  if (n.gen_params) n.gen_params->accept(*this);
  for (auto& elem : n.operators) elem->accept(*this);
  for (auto& elem : n.casts) elem->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::declaration::cop::Entity_New& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.codeblock->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::declaration::cop::Entity_Del& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.codeblock->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::declaration::cop::Entity_Cast& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.source->accept(*this);
  n.target->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::declaration::cop::Entity_Op& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.codeblock->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::declaration::cop::Entity_Access_Op& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.codeblock->accept(*this);
  n.return_type->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::declaration::cop::Entity_Transfert& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.codeblock->accept(*this);
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::declaration::cop::System& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.prototype->accept(*this);
  for (auto& elem : n.cases) elem->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::declaration::cop::System_Case& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (auto& elem : n.bindings) elem->accept(*this);
  n.codeblock->accept(*this);
  out_print += "</ul></li>\n";
}

// ============ GENERIC ============
void AST_Printer::visit(ast::generic::Is_Type& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (auto& elem : n.in_type) elem->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::generic::Can_Cast& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.target->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::generic::Have_Op& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  if (n.explicit_return_type) n.explicit_return_type->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::generic::Have_Role& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.role->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::generic::Use_Component& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.component->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::generic::Compatible_System& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.system->accept(*this);
  out_print += "</ul></li>\n";
}

// ============ TYPE ============
void AST_Printer::visit(ast::type::Ptr& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.inner->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::type::Table& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  if (n.size_sym) n.size_sym->accept(*this);
  n.inner->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::type::Primitive& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::type::Tuple& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (auto& elem : n.types) elem->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::type::Function_Proto& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (auto& elem : n.parameters) elem->accept(*this);
  for (auto& elem : n.gen_parameters) elem->accept(*this);
  if (n.return_ty) n.return_ty->accept(*this);
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::type::Get_Expr_Type& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.target->accept(*this);
  out_print += "</ul></li>\n";
}

// ============ LITERAL ============
void AST_Printer::visit(ast::literal::Boolean& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::literal::Integral& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::literal::Fixed_Point& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::literal::Floating_Point& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::literal::CUNE& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::literal::RUNE& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::literal::Text_Pure& n)
{
  out_print += "<li class='node'> text " + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::literal::Text_Interpolation& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  if (n.expression) n.expression->accept(*this);
  if (n.spec) n.spec->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::literal::Textual_Format& n)
{
  for (auto& elem : n.values) elem->accept(*this);
}
void AST_Printer::visit(ast::literal::Format_Specifier& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  if (n.width) n.width->accept(*this);
  if (n.precision) n.precision->accept(*this);
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::literal::Table& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (auto& elem : n.values) elem->accept(*this);
  if (n.population) n.population->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::literal::Table_Population& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (auto& elem : n.ranges) elem->accept(*this);
  n.expression->accept(*this);
  if (n.map_expression_value) n.map_expression_value->accept(*this);
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::literal::Map& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (auto& elem : n.keys) elem->accept(*this);
  for (auto& elem : n.values) elem->accept(*this);
  if (n.population) n.population->accept(*this);
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::literal::Tuple& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (auto& elem : n.values) elem->accept(*this);
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::literal::Range& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  if (n.start) n.start->accept(*this);
  if (n.end) n.end->accept(*this);
  if (n.step) n.step->accept(*this);
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::literal::Iterator& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.collection->accept(*this);
  out_print += "</ul></li>\n";
}


void AST_Printer::visit(ast::literal::Enum& n)
{
  out_print += "<li class ='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (auto& elem : n.member_values) elem->accept(*this);
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::literal::Structured_Data& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.name->accept(*this);
  for (auto& elem : n.field_args) elem->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::literal::Entity& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.name->accept(*this);
  for (auto& elem : n.comp_args) elem->accept(*this);
  out_print += "</ul></li>\n";
}

// ============ Expression ============
void AST_Printer::visit(ast::expression::If_Ternary& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  if (auto node = n.evaluator.get_node()) node->accept(*this);
  n.true_line->accept(*this);
  n.false_line->accept(*this);
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::expression::Member_Access& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.left->accept(*this);
  n.right->accept(*this);
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::expression::Self& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::expression::Other& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::expression::Call& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::expression::Call_Argument& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::expression::Call_System& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::expression::Call_Pipe& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (auto& elem : n.gen_args) {
    for (auto& elem1 : elem) elem1->accept(*this);
  }
  for (auto& elem : n.arguments) {
    for (auto& elem1 : elem) elem1->accept(*this);
  }
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::expression::Table_Access& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.selector->accept(*this);
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::expression::Ptr_At& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.target->accept(*this);
  n.index->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::expression::Ptr_Offset& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.target->accept(*this);
  n.offset->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::expression::Ptr_Val& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.target->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::expression::Mut_Of& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.target->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::expression::Ref_Of& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.target->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::expression::Addr_Of& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.target->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::expression::Size_Of& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.target->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::expression::GetBits& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.target->accept(*this);
  n.range->accept(*this);
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::expression::Move& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.target->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::expression::New_Ptr& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.type->accept(*this);
  n.expression->accept(*this);
  out_print += "</ul></li>\n";
}

// ============ STATEMENT ============
void AST_Printer::visit(ast::statement::If& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  if (auto node = n.evaluator.get_node()) node->accept(*this);
  n.codeblock->accept(*this);
  out_print += "</ul></li>\n";
  if (n.alternative_statement) {
    n.alternative_statement->accept(*this);
  }
}

void AST_Printer::visit(ast::statement::For& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.expression->accept(*this);
  if (n.index) n.index->accept(*this);
  for (auto& elem : n.items) elem->accept(*this);
  n.codeblock->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::statement::Loop& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.codeblock->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::statement::While& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  if (auto node = n.evaluator.get_node()) node->accept(*this);
  n.codeblock->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::statement::GoTo& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::statement::GoTo_Label& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::statement::Return& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  if (n.value) n.value->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::statement::Break& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::statement::Continue& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::statement::Match& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.base->accept(*this);
  for (auto& elem : n.cases) elem->accept(*this);
  if (n.other_case) n.other_case->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::statement::Match_Case& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  if (auto node = n.evaluator.get_node()) node->accept(*this);
  n.codeblock->accept(*this);
  out_print += "</ul></li>\n";
}

// ============ OPERATION ============
void AST_Printer::visit(ast::operation::Cast_As& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.expression->accept(*this);
  n.type->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::operation::Is& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.left->accept(*this);
  n.right->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::operation::In& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.left->accept(*this);
  n.right->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::operation::Assignment& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.left->accept(*this);
  n.right->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::operation::Binary& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.left->accept(*this);
  n.right->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::operation::Unary& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.base->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::operation::Interval& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.left->accept(*this);
  n.center->accept(*this);
  n.right->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::operation::Ptr_Dist& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.left->accept(*this);
  n.right->accept(*this);
  out_print += "</ul></li>\n";
}

// ============ MEMORY ============
void AST_Printer::visit(ast::memory::Del& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.target->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::memory::Align& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.target->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::memory::Drop& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.target->accept(*this);
  out_print += "</ul></li>\n";
}
  */
