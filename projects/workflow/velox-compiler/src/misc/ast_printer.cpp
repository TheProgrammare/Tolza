#include "ast_printer.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>

#include <common/compiler_options.hpp>

#include "ast/ast_base.hpp"
#include "nexus/ast/ast.hpp"
#include "ast/ast_declaration_global.hpp"
#include "ast/ast_declaration_local.hpp"
#include "ast/ast_declaration_sfm.hpp"
#include "ast/ast_generic.hpp"
#include "ast/ast_literal.hpp"
#include "ast/ast_memory.hpp"
#include "ast/ast_operation.hpp"
#include "ast/ast_statement.hpp"
#include "ast/ast_expression.hpp"
#include "nexus/type/type.hpp"

#include "compiler/compiler.hpp"
#include "compiler/compilation_unit.hpp"

#include <common/common.hpp>
/*

std::string AST_Printer::get_file_path() const
{
  std::filesystem::path path =
      std::filesystem::path(compiler::OPTIONS.get_debug_graph_dir()) / CU.file_info.get_file_name();
  path.replace_extension(".html");
  return path.string();
}

// ============ AST ============
void AST_Printer::visit(ast::Node& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::ID& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::ID_Qualified& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::ID_Typed& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (const auto& elem : n.gen_args) elem->accept(*this);
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::Root& n)
{
  std::fstream out_file;

  try {
    std::filesystem::create_directories(std::filesystem::path(get_file_path()).parent_path());
  } catch (const std::exception& e) {
    std::cerr << "Cannot create directory at " << std::filesystem::path(get_file_path()).parent_path() << ": "
              << e.what() << "\n"; // endl
}

try {
  out_file.open(get_file_path(), std::ios::out);
} catch (const std::ios_base::failure& e) {
  std::cerr << "Cannot create file at " << std::filesystem::path(get_file_path()) << ": " << e.what()
            << "\n"; // endl
            // return;
}

std::string f_template = PRINT_FILE_HTLM_TEMPLATE;

out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
for (const auto& elem : n.global_nodes) elem->accept(*this);
out_print += "</ul></li>\n";

common::utils::fmt_template(f_template, {common::SOFTWARE_VERSION, CU.file_info.get_file_name(), out_print});

out_file.clear();
out_file << f_template;
out_file.close();
}

// ============ DECLARATION ============
void AST_Printer::visit(ast::Global_Variable& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.expression->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Global_Function& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  if (n.codeblock) n.codeblock->accept(*this);
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::Global_Mod& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (const auto& elem : n.declarations) elem->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Global_Export& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (const auto& elem : n.declarations) elem->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Global_Extern& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (const auto& elem : n.declarations) elem->accept(*this);
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::Global_Enum& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (const auto& elem : n.variants) elem->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Global_Enum_Element& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::Global_Flag& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (const auto& field : n.fields) {
    out_print += "<li class='node'>" + field + "<ul class='children'>\n";
    out_print += "</ul></li>\n";
  }
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::Global_Union& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (const auto& [name, field] : n.fields) {
    out_print += "<li class='node'>" + name + "<ul class='children'>\n";
    out_print += "<li class='node'>" + field->debug_str() + "<ul class='children'>\n";
    out_print += "</ul></li>\n";
    out_print += "</ul></li>\n";
  }
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::Global_Type_Alias& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Global_Module_Alias& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::Global_Generic& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (const auto& elem : n.gen_args) elem->accept(*this);
  for (const auto& elem : n.conditions) elem->accept(*this);
  out_print += "</ul></li>\n";
}

// ============ LOCAL ============
void AST_Printer::visit(ast::CodeBlock& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (const auto& elem : n.elements) {
    if (auto elem_n = elem.node()) elem_n->accept(*this);
  }
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::Local_Lambda& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.codeblock->accept(*this);
  n.capture->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Local_Lambda_Capture& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (const auto& elem : n.elements) elem->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Local_Capture_Member& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::Local_Parameter& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Local_Generic_Parameter_Element& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (const auto& elem : n.generic_references) elem->accept(*this);
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::Local_Generic_Parameters& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::Local_Pattern& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  if (n.additive_evaluator) n.additive_evaluator->accept(*this);
  if (n.right) n.right->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Local_Pattern_Enum& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  AST_Printer::visit(static_cast<Local_Pattern&>(n));
  for (const auto& elem : n.mapping) {
    if (auto node = elem->node()) node->accept(*this);
  }
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Local_Pattern_Tuple& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  AST_Printer::visit(static_cast<Local_Pattern&>(n));
  for (const auto& elem : n.mapping) {
    if (auto node = elem->node()) node->accept(*this);
  }
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Local_Pattern_Form& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  AST_Printer::visit(static_cast<Local_Pattern&>(n));
  for (const auto& elem : n.mapping) {
    for (const auto& [_, elem2] : elem->mapping) elem2->node()->accept(*this);
  }
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Local_Pattern_Rule_Facet& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  AST_Printer::visit(static_cast<Local_Pattern&>(n));
  n.bind->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Local_Pattern_Facet& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  AST_Printer::visit(static_cast<Local_Pattern&>(n));
  for (const auto& [_, elem] : n.mapping) {
    if (auto node = elem->node()) node->accept(*this);
  }
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::Local_Variable_Binding& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Local_Tuple_Destructuring& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (const auto& elem : n.elements) elem->accept(*this);
  n.right->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Local_Variable& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  if (n.expression) n.expression->accept(*this);
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::Local_Capability& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.right->accept(*this);
  out_print += "</ul></li>\n";
}

// ============ SFM ============
void AST_Printer::visit(ast::SFM_Facet& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (const auto& elem : n.fields) elem->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::SFM_Facet_Field& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::SFM_View& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (const auto& elem : n.facets) elem->accept(*this);
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::SFM_Form& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (const auto& elem : n.facets) elem->accept(*this);
  if (n.gen_params) n.gen_params->accept(*this);
  for (const auto& elem : n.operators) elem->accept(*this);
  for (const auto& elem : n.casts) elem->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::SFM_Form_New& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.codeblock->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::SFM_Form_Del& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.codeblock->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::SFM_Form_Cast& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.source->accept(*this);
  n.target->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::SFM_Form_Op& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.codeblock->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::SFM_Form_Access_Op& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.codeblock->accept(*this);
  n.return_type->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::SFM_Form_Transfert& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.codeblock->accept(*this);
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::SFM_Rule& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.prototype->accept(*this);
  for (const auto& elem : n.cases) elem->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::SFM_Rule_Case& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (const auto& elem : n.bindings) elem->accept(*this);
  n.codeblock->accept(*this);
  out_print += "</ul></li>\n";
}

// ============ GENERIC ============
void AST_Printer::visit(ast::Generic_Type& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (const auto& elem : n.in_type) elem->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Generic_Can_Cast& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.target->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Generic_Have_Op& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  if (n.explicit_return_type) n.explicit_return_type->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Generic_View& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.view->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Generic_Facet& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.facet->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Generic_Rule& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.rule->accept(*this);
  out_print += "</ul></li>\n";
}

// ============ TYPE ============
void AST_Printer::visit(type::Ptr& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.inner->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(type::Table& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  if (n.size_sym) n.size_sym->accept(*this);
  n.inner->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(type::Primitive& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(type::Tuple& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (const auto& elem : n.types) elem->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(type::Function_Proto& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (const auto& elem : n.parameters) elem->accept(*this);
  for (const auto& elem : n.gen_parameters) elem->accept(*this);
  if (n.return_ty) n.return_ty->accept(*this);
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(type::Get_Expr_Type& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.target->accept(*this);
  out_print += "</ul></li>\n";
}

// ============ LITERAL ============
void AST_Printer::visit(ast::Literal_Boolean& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Literal_Integral& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Literal_Fixed_Point& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Literal_Floating_Point& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::Literal_CUNE& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Literal_RUNE& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::Literal_Text_Pure& n)
{
  out_print += "<li class='node'> text " + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Literal_Text_Interpolation& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  if (n.expression) n.expression->accept(*this);
  if (n.spec) n.spec->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Literal_Textual_Format& n)
{
  for (const auto& elem : n.values) elem->accept(*this);
}
void AST_Printer::visit(ast::Literal_Format_Specifier& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  if (n.width) n.width->accept(*this);
  if (n.precision) n.precision->accept(*this);
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::Literal_Table& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (const auto& elem : n.values) elem->accept(*this);
  if (n.population) n.population->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Literal_Table_Population& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (const auto& elem : n.ranges) elem->accept(*this);
  n.expression->accept(*this);
  if (n.map_expression_value) n.map_expression_value->accept(*this);
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::Literal_Map& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (const auto& elem : n.keys) elem->accept(*this);
  for (const auto& elem : n.values) elem->accept(*this);
  if (n.population) n.population->accept(*this);
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::Literal_Tuple& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (const auto& elem : n.values) elem->accept(*this);
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::Literal_Range& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  if (n.start) n.start->accept(*this);
  if (n.end) n.end->accept(*this);
  if (n.step) n.step->accept(*this);
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::Literal_Iterator& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.collection->accept(*this);
  out_print += "</ul></li>\n";
}


void AST_Printer::visit(ast::Literal_Enum& n)
{
  out_print += "<li class ='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (const auto& elem : n.member_values) elem->accept(*this);
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::Literal_Structured_Data& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.name->accept(*this);
  for (const auto& elem : n.field_args) elem->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Literal_Form& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.name->accept(*this);
  for (const auto& elem : n.facet_args) elem->accept(*this);
  out_print += "</ul></li>\n";
}

// ============ Expression ============
void AST_Printer::visit(ast::Expression_If_Ternary& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  if (auto node = n.evaluator.get_node()) node->accept(*this);
  n.true_line->accept(*this);
  n.false_line->accept(*this);
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::Expression_Member_Access& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.left->accept(*this);
  n.right->accept(*this);
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::Expression_Self& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Expression_Other& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::Expression_Call& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Expression_Call_Argument& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Expression_Call_Rule& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Expression_Call_Pipe& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  for (const auto& elem : n.gen_args) {
    for (const auto& elem1 : elem) elem1->accept(*this);
  }
  for (const auto& elem : n.arguments) {
    for (const auto& elem1 : elem) elem1->accept(*this);
  }
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::Expression_Table_Access& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.selector->accept(*this);
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::Expression_Ptr_At& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.target->accept(*this);
  n.index->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Expression_Ptr_Offset& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.target->accept(*this);
  n.offset->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Expression_Ptr_Val& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.target->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Expression_Mut_Of& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.target->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Expression_Ref_Of& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.target->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Expression_Addr_Of& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.target->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Expression_Size_Of& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.target->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Expression_GetBits& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.target->accept(*this);
  n.range->accept(*this);
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::Expression_Move& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.target->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Expression_New_Ptr& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.type->accept(*this);
  n.expression->accept(*this);
  out_print += "</ul></li>\n";
}

// ============ STATEMENT ============
void AST_Printer::visit(ast::Statement_If& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  if (auto node = n.evaluator.get_node()) node->accept(*this);
  n.codeblock->accept(*this);
  out_print += "</ul></li>\n";
  if (n.alternative_statement) {
    n.alternative_statement->accept(*this);
  }
}

void AST_Printer::visit(ast::Statement_For& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.expression->accept(*this);
  if (n.index) n.index->accept(*this);
  for (const auto& elem : n.items) elem->accept(*this);
  n.codeblock->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Statement_Loop& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.codeblock->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Statement_While& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  if (auto node = n.evaluator.get_node()) node->accept(*this);
  n.codeblock->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Statement_GoTo& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Statement_GoTo_Label& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::Statement_Return& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  if (n.value) n.value->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Statement_Break& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Statement_Continue& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  out_print += "</ul></li>\n";
}

void AST_Printer::visit(ast::Statement_Match& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.base->accept(*this);
  for (const auto& elem : n.cases) elem->accept(*this);
  if (n.other_case) n.other_case->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Statement_Match_Case& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  if (auto node = n.evaluator.get_node()) node->accept(*this);
  n.codeblock->accept(*this);
  out_print += "</ul></li>\n";
}

// ============ OPERATION ============
void AST_Printer::visit(ast::Operation_Cast_As& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.expression->accept(*this);
  n.type->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Operation_Is& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.left->accept(*this);
  n.right->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Operation_In& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.left->accept(*this);
  n.right->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Operation_Assignment& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.left->accept(*this);
  n.right->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Operation_Binary& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.left->accept(*this);
  n.right->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Operation_Unary& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.base->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Operation_Interval& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.left->accept(*this);
  n.center->accept(*this);
  n.right->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Operation_Ptr_Dist& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.left->accept(*this);
  n.right->accept(*this);
  out_print += "</ul></li>\n";
}

// ============ MEMORY ============
void AST_Printer::visit(ast::Memory_Del& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.target->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Memory_Align& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.target->accept(*this);
  out_print += "</ul></li>\n";
}
void AST_Printer::visit(ast::Memory_Drop& n)
{
  out_print += "<li class='node'>" + n.debug_str() + "<ul class='children'>\n";
  n.target->accept(*this);
  out_print += "</ul></li>\n";
}
*/
