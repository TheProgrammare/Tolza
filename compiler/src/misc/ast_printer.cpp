#include "ast_printer.hpp"

#include "ast/ast_base.hpp"
#include "ast/ast_declaration_global.hpp"
#include "ast/ast_declaration_local.hpp"
#include "ast/ast_declaration_sfm.hpp"
#include "ast/ast_expression.hpp"
#include "ast/ast_generic.hpp"
#include "ast/ast_literal.hpp"
#include "ast/ast_memory.hpp"
#include "ast/ast_operation.hpp"
#include "ast/ast_statement.hpp"
#include "compiler/compilation_unit.hpp"
#include "compiler/compiler.hpp"
#include "nexus/ast/data.hpp"
#include "nexus/ast/definition.hpp"
#include "nexus/ast/forward.hpp"
#include "nexus/type/type.hpp"

#include <common/common.hpp>
#include <common/compiler_options.hpp>
#include <filesystem>
#include <fstream>
#include <print>

#define OUT_ELEM(elem) out_print += std::format("<li class='node'>{}<ul class='children'>\n", elem);
#define OUT_START      out_print += OUT_ELEM(n.debug_str());
#define OUT_END        out_print += "</ul></li>\n";

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
  OUT_START
  OUT_END
}

void AST_Printer::visit(ast::ID& n)
{
  OUT_START
  OUT_END
}
void AST_Printer::visit(ast::Symbol_Qualified& n)
{
  OUT_START
  OUT_END
}
void AST_Printer::visit(ast::Symbol_Type& n)
{
  OUT_START
  for (const auto& elem : n.gen_args) elem->accept(*this);
  OUT_END
}

void AST_Printer::visit(ast::Root& n)
{
  std::fstream out_file;

  try {
    std::filesystem::create_directories(std::filesystem::path(get_file_path()).parent_path());
  } catch (const std::exception& e) {
    std::println(stderr, "Cannot create directory at \"{}\" : {}", std::filesystem::path(get_file_path()).parent_path(),
  e.what());
  }

  try {
    out_file.open(get_file_path(), std::ios::out);
  } catch (const std::ios_base::failure& e) {
    std::println(stderr, "Cannot create file at \"{}\" : {}", std::filesystem::path(get_file_path()), e.what());
  }

  std::string f_template = PRINT_FILE_HTLM_TEMPLATE;

  OUT_START
  for (const auto& elem : n.global_nodes) elem->accept(*this);
  OUT_END

  common::utils::fmt_template(f_template, {common::SOFTWARE_VERSION, CU.file_info.get_file_name(), out_print});

  out_file.clear();
  out_file << f_template;
  out_file.close();
}

// ============ DECLARATION ============
void AST_Printer::visit(ast::Global_Variable& n)
{
  OUT_START
  n.expression->accept(*this);
  OUT_END
}
void AST_Printer::visit(ast::Global_Function& n)
{
  OUT_START
  if (n.codeblock) n.codeblock->accept(*this);
  OUT_END
}

void AST_Printer::visit(ast::Global_Mod& n)
{
  OUT_START
  for (const auto& elem : n.declarations) elem->accept(*this);
  OUT_END
}
void AST_Printer::visit(ast::Global_Export& n)
{
  OUT_START
  for (const auto& elem : n.declarations) elem->accept(*this);
  OUT_END
}
void AST_Printer::visit(ast::Global_Extern& n)
{
  OUT_START
  for (const auto& elem : n.declarations) elem->accept(*this);
  OUT_END
}

void AST_Printer::visit(ast::Global_Enum& n)
{
  OUT_START
  for (const auto& elem : n.variants) elem->accept(*this);
  OUT_END
}
void AST_Printer::visit(ast::Global_Enum_Element& n)
{
  OUT_START
  OUT_END
}

void AST_Printer::visit(ast::Global_Flag& n)
{
  OUT_START
  for (const auto& field : n.fields) {
    OUT_ELEM( field )
    OUT_END
  }
  OUT_END
}

void AST_Printer::visit(ast::Global_Union& n)
{
  OUT_START
  for (const auto& [name, field] : n.fields) {
    OUT_ELEM( name )
    OUT_ELEM( field->debug_str() )
    OUT_END
    OUT_END
  }
  OUT_END
}

void AST_Printer::visit(ast::Global_Type_Alias& n)
{
  OUT_START
  OUT_END
}
void AST_Printer::visit(ast::Global_Module_Alias& n)
{
  OUT_START
  OUT_END
}

void AST_Printer::visit(ast::Global_Generic& n)
{
  OUT_START
  for (const auto& elem : n.gen_args) elem->accept(*this);
  for (const auto& elem : n.conditions) elem->accept(*this);
  OUT_END
}

// ============ LOCAL ============
void AST_Printer::visit(ast::CodeBlock& n)
{
  OUT_START
  for (const auto& elem : n.elements) {
    if (auto elem_n = elem.node()) elem_n->accept(*this);
  }
  OUT_END
}

void AST_Printer::visit(ast::Local_Lambda& n)
{
  OUT_START
  n.codeblock->accept(*this);
  n.capture->accept(*this);
  OUT_END
}
void AST_Printer::visit(ast::Local_Lambda_Capture& n)
{
  OUT_START
  for (const auto& elem : n.elements) elem->accept(*this);
  OUT_END
}
void AST_Printer::visit(ast::Local_Capture_Member& n)
{
  OUT_START
  OUT_END
}

void AST_Printer::visit(ast::Local_Parameter& n)
{
  OUT_START
  OUT_END
}
void AST_Printer::visit(ast::Local_Generic_Parameter_Element& n)
{
  OUT_START
  for (const auto& elem : n.generic_references) elem->accept(*this);
  OUT_END
}

void AST_Printer::visit(ast::Local_Generic_Parameters& n)
{
  OUT_START
  OUT_END
}

void AST_Printer::visit(ast::Local_Pattern& n)
{
  OUT_START
  if (n.additive_evaluator) n.additive_evaluator->accept(*this);
  if (n.right) n.right->accept(*this);
  OUT_END
}
void AST_Printer::visit(ast::Local_Pattern_Enum& n)
{
  OUT_START
  AST_Printer::visit(static_cast<Local_Pattern&>(n));
  for (const auto& elem : n.mapping) {
    if (auto node = elem->node()) node->accept(*this);
  }
  OUT_END
}
void AST_Printer::visit(ast::Local_Pattern_Tuple& n)
{
  OUT_START
  AST_Printer::visit(static_cast<Local_Pattern&>(n));
  for (const auto& elem : n.mapping) {
    if (auto node = elem->node()) node->accept(*this);
  }
  OUT_END
}
void AST_Printer::visit(ast::Local_Pattern_Form& n)
{
  OUT_START
  AST_Printer::visit(static_cast<Local_Pattern&>(n));
  for (const auto& elem : n.mapping) {
    for (const auto& [_, elem2] : elem->mapping) elem2->node()->accept(*this);
  }
  OUT_END
}
void AST_Printer::visit(ast::Local_Pattern_Rule_Facet& n)
{
  OUT_START
  AST_Printer::visit(static_cast<Local_Pattern&>(n));
  n.bind->accept(*this);
  OUT_END
}
void AST_Printer::visit(ast::Local_Pattern_Facet& n)
{
  OUT_START
  AST_Printer::visit(static_cast<Local_Pattern&>(n));
  for (const auto& [_, elem] : n.mapping) {
    if (auto node = elem->node()) node->accept(*this);
  }
  OUT_END
}

void AST_Printer::visit(ast::Local_Variable_Binding& n)
{
  OUT_START
  OUT_END
}
void AST_Printer::visit(ast::Local_Tuple_Destructuring& n)
{
  OUT_START
  for (const auto& elem : n.elements) elem->accept(*this);
  n.right->accept(*this);
  OUT_END
}
void AST_Printer::visit(ast::Local_Variable& n)
{
  OUT_START
  if (n.expression) n.expression->accept(*this);
  OUT_END
}

void AST_Printer::visit(ast::Local_Capability& n)
{
  OUT_START
  n.right->accept(*this);
  OUT_END
}

// ============ SFM ============
void AST_Printer::visit(ast::SFM_Facet& n)
{
  OUT_START
  for (const auto& elem : n.fields) elem->accept(*this);
  OUT_END
}
void AST_Printer::visit(ast::SFM_Facet_Field& n)
{
  OUT_START
  OUT_END
}

void AST_Printer::visit(ast::SFM_View& n)
{
  OUT_START
  for (const auto& elem : n.facets) elem->accept(*this);
  OUT_END
}

void AST_Printer::visit(ast::SFM_Form& n)
{
  OUT_START
  for (const auto& elem : n.facets) elem->accept(*this);
  if (n.gen_params) n.gen_params->accept(*this);
  for (const auto& elem : n.operators) elem->accept(*this);
  for (const auto& elem : n.casts) elem->accept(*this);
  OUT_END
}
void AST_Printer::visit(ast::SFM_Form_New& n)
{
  OUT_START
  n.codeblock->accept(*this);
  OUT_END
}
void AST_Printer::visit(ast::SFM_Form_Del& n)
{
  OUT_START
  n.codeblock->accept(*this);
  OUT_END
}
void AST_Printer::visit(ast::SFM_Form_Cast& n)
{
  OUT_START
  n.source->accept(*this);
  n.target->accept(*this);
  OUT_END
}
void AST_Printer::visit(ast::SFM_Form_Op& n)
{
  OUT_START
  n.codeblock->accept(*this);
  OUT_END
}
void AST_Printer::visit(ast::SFM_Form_Access_Op& n)
{
  OUT_START
  n.codeblock->accept(*this);
  n.return_type->accept(*this);
  OUT_END
}
void AST_Printer::visit(ast::SFM_Form_Transfert& n)
{
  OUT_START
  n.codeblock->accept(*this);
  OUT_END
}

void AST_Printer::visit(ast::SFM_Rule& n)
{
  OUT_START
  n.prototype->accept(*this);
  for (const auto& elem : n.cases) elem->accept(*this);
  OUT_END
}
void AST_Printer::visit(ast::SFM_Rule_Case& n)
{
  OUT_START
  for (const auto& elem : n.bindings) elem->accept(*this);
  n.codeblock->accept(*this);
  OUT_END
}

// ============ GENERIC ============
void AST_Printer::visit(ast::Generic_Type& n)
{
  OUT_START
  for (const auto& elem : n.in_type) elem->accept(*this);
  OUT_END
}
void AST_Printer::visit(ast::Generic_Can_Cast& n)
{
  OUT_START
  n.target->accept(*this);
  OUT_END
}
void AST_Printer::visit(ast::Generic_Have_Op& n)
{
  OUT_START
  if (n.explicit_return_type) n.explicit_return_type->accept(*this);
  OUT_END
}
void AST_Printer::visit(ast::Generic_View& n)
{
  OUT_START
  n.view->accept(*this);
  OUT_END
}
void AST_Printer::visit(ast::Generic_Facet& n)
{
  OUT_START
  n.facet->accept(*this);
  OUT_END
}
void AST_Printer::visit(ast::Generic_Rule& n)
{
  OUT_START
  n.rule->accept(*this);
  OUT_END
}

// ============ TYPE ============
void AST_Printer::visit(type::Ptr& n)
{
  OUT_START
  n.inner->accept(*this);
  OUT_END
}
void AST_Printer::visit(type::Table& n)
{
  OUT_START
  if (n.size_sym) n.size_sym->accept(*this);
  n.inner->accept(*this);
  OUT_END
}
void AST_Printer::visit(type::Primitive& n)
{
  OUT_START
  OUT_END
}
void AST_Printer::visit(type::Tuple& n)
{
  OUT_START
  for (const auto& elem : n.types) elem->accept(*this);
  OUT_END
}
void AST_Printer::visit(type::Function_Proto& n)
{
  OUT_START
  for (const auto& elem : n.parameters) elem->accept(*this);
  for (const auto& elem : n.gen_parameters) elem->accept(*this);
  if (n.return_ty) n.return_ty->accept(*this);
  OUT_END
}

void AST_Printer::visit(type::Get_Expr_Type& n)
{
  OUT_START
  n.target->accept(*this);
  OUT_END
}

// ============ LITERAL ============
void AST_Printer::visit(ast::Literal_Boolean& n)
{
  OUT_START
  OUT_END
}
void AST_Printer::visit(ast::Literal_Integral& n)
{
  OUT_START
  OUT_END
}
void AST_Printer::visit(ast::Literal_Fixed_Point& n)
{
  OUT_START
  OUT_END
}
void AST_Printer::visit(ast::Literal_Floating_Point& n)
{
  OUT_START
  OUT_END
}

void AST_Printer::visit(ast::Literal_CUNE& n)
{
  OUT_START
  OUT_END
}
void AST_Printer::visit(ast::Literal_RUNE& n)
{
  OUT_START
  OUT_END
}

void AST_Printer::visit(ast::Literal_Text_Pure& n)
{
  OUT_ELEM(n.debug_str());
  OUT_END
}
void AST_Printer::visit(ast::Literal_Text_Interpolation& n)
{
  OUT_START
  if (n.expression) n.expression->accept(*this);
  if (n.spec) n.spec->accept(*this);
  OUT_END
}
void AST_Printer::visit(ast::Literal_Textual_Format& n)
{
  for (const auto& elem : n.values) elem->accept(*this);
}
void AST_Printer::visit(ast::Literal_Format_Specifier& n)
{
  OUT_START
  if (n.width) n.width->accept(*this);
  if (n.precision) n.precision->accept(*this);
  OUT_END
}

void AST_Printer::visit(ast::Literal_Table& n)
{
  OUT_START
  for (const auto& elem : n.values) elem->accept(*this);
  if (n.population) n.population->accept(*this);
  OUT_END
}
void AST_Printer::visit(ast::Literal_Table_Population& n)
{
  OUT_START
  for (const auto& elem : n.ranges) elem->accept(*this);
  n.expression->accept(*this);
  if (n.map_expression_value) n.map_expression_value->accept(*this);
  OUT_END
}

void AST_Printer::visit(ast::Literal_Map& n)
{
  OUT_START
  for (const auto& elem : n.keys) elem->accept(*this);
  for (const auto& elem : n.values) elem->accept(*this);
  if (n.population) n.population->accept(*this);
  OUT_END
}

void AST_Printer::visit(ast::Literal_Tuple& n)
{
  OUT_START
  for (const auto& elem : n.values) elem->accept(*this);
  OUT_END
}

void AST_Printer::visit(ast::Literal_Range& n)
{
  OUT_START
  if (n.start) n.start->accept(*this);
  if (n.end) n.end->accept(*this);
  if (n.step) n.step->accept(*this);
  OUT_END
}

void AST_Printer::visit(ast::Literal_Iterator& n)
{
  OUT_START
  n.collection->accept(*this);
  OUT_END
}


void AST_Printer::visit(ast::Literal_Enum& n)
{
  OUT_ELEM(n.debug_str());
  for (const auto& elem : n.member_values) elem->accept(*this);
  OUT_END
}

void AST_Printer::visit(ast::Literal_Record& n)
{
  OUT_START
  n.name->accept(*this);
  for (const auto& elem : n.field_args) elem->accept(*this);
  OUT_END
}

// ============ Expression ============
void AST_Printer::visit(ast::Expression_If_Ternary& n)
{
  OUT_START
  if (auto node = n.evaluator.get_node()) node->accept(*this);
  n.true_line->accept(*this);
  n.false_line->accept(*this);
  OUT_END
}

void AST_Printer::visit(ast::Expression_Member_Access& n)
{
  OUT_START
  n.left->accept(*this);
  n.right->accept(*this);
  OUT_END
}

void AST_Printer::visit(ast::Expression_Self& n)
{
  OUT_START
  OUT_END
}
void AST_Printer::visit(ast::Expression_Other& n)
{
  OUT_START
  OUT_END
}

void AST_Printer::visit(ast::Expression_Invocation& n)
{
  OUT_START
  OUT_END
}
void AST_Printer::visit(ast::Expression_Invocation_Argument& n)
{
  OUT_START
  OUT_END
}
void AST_Printer::visit(ast::Expression_Invocation_Rule& n)
{
  OUT_START
  OUT_END
}
void AST_Printer::visit(ast::Expression_Invocation_Pipe& n)
{
  OUT_START
  for (const auto& elem : n.gen_args) {
    for (const auto& elem1 : elem) elem1->accept(*this);
  }
  for (const auto& elem : n.arguments) {
    for (const auto& elem1 : elem) elem1->accept(*this);
  }
  OUT_END
}

void AST_Printer::visit(ast::Expression_Table_Access& n)
{
  OUT_START
  n.selector->accept(*this);
  OUT_END
}

void AST_Printer::visit(ast::Expression_Ptr_At& n)
{
  OUT_START
  n.target->accept(*this);
  n.index->accept(*this);
  OUT_END
}
void AST_Printer::visit(ast::Expression_Ptr_Offset& n)
{
  OUT_START
  n.target->accept(*this);
  n.offset->accept(*this);
  OUT_END
}
void AST_Printer::visit(ast::Expression_Ptr_Val& n)
{
  OUT_START
  n.target->accept(*this);
  OUT_END
}
void AST_Printer::visit(ast::Expression_Mut_Of& n)
{
  OUT_START
  n.target->accept(*this);
  OUT_END
}
void AST_Printer::visit(ast::Expression_Ref_Of& n)
{
  OUT_START
  n.target->accept(*this);
  OUT_END
}
void AST_Printer::visit(ast::Expression_Addr_Of& n)
{
  OUT_START
  n.target->accept(*this);
  OUT_END
}
void AST_Printer::visit(ast::Expression_Size_Of& n)
{
  OUT_START
  n.target->accept(*this);
  OUT_END
}
void AST_Printer::visit(ast::Expression_GetBits& n)
{
  OUT_START
  n.target->accept(*this);
  n.range->accept(*this);
  OUT_END
}

void AST_Printer::visit(ast::Expression_Move& n)
{
  OUT_START
  n.target->accept(*this);
  OUT_END
}
void AST_Printer::visit(ast::Expression_New_Ptr& n)
{
  OUT_START
  n.type->accept(*this);
  n.expression->accept(*this);
  OUT_END
}

// ============ STATEMENT ============
void AST_Printer::visit(ast::Statement_If& n)
{
  OUT_START
  if (auto node = n.evaluator.get_node()) node->accept(*this);
  n.codeblock->accept(*this);
  OUT_END
  if (n.alternative_statement) {
    n.alternative_statement->accept(*this);
  }
}

void AST_Printer::visit(ast::Statement_For& n)
{
  OUT_START
  n.expression->accept(*this);
  if (n.index) n.index->accept(*this);
  for (const auto& elem : n.items) elem->accept(*this);
  n.codeblock->accept(*this);
  OUT_END
}
void AST_Printer::visit(ast::Statement_Loop& n)
{
  OUT_START
  n.codeblock->accept(*this);
  OUT_END
}
void AST_Printer::visit(ast::Statement_While& n)
{
  OUT_START
  if (auto node = n.evaluator.get_node()) node->accept(*this);
  n.codeblock->accept(*this);
  OUT_END
}
void AST_Printer::visit(ast::Statement_GoTo& n)
{
  OUT_START
  OUT_END
}
void AST_Printer::visit(ast::Statement_GoTo_Label& n)
{
  OUT_START
  OUT_END
}

void AST_Printer::visit(ast::Statement_Return& n)
{
  OUT_START
  if (n.value) n.value->accept(*this);
  OUT_END
}
void AST_Printer::visit(ast::Statement_Break& n)
{
  OUT_START
  OUT_END
}
void AST_Printer::visit(ast::Statement_Continue& n)
{
  OUT_START
  OUT_END
}

void AST_Printer::visit(ast::Statement_Match& n)
{
  OUT_START
  n.base->accept(*this);
  for (const auto& elem : n.cases) elem->accept(*this);
  if (n.other_case) n.other_case->accept(*this);
  OUT_END
}
void AST_Printer::visit(ast::Statement_Match_Case& n)
{
  OUT_START
  if (auto node = n.evaluator.get_node()) node->accept(*this);
  n.codeblock->accept(*this);
  OUT_END
}

// ============ OPERATION ============
void AST_Printer::visit(ast::Operation_Cast_As& n)
{
  OUT_START
  n.expression->accept(*this);
  n.type->accept(*this);
  OUT_END
}
void AST_Printer::visit(ast::Operation_Is& n)
{
  OUT_START
  n.left->accept(*this);
  n.right->accept(*this);
  OUT_END
}
void AST_Printer::visit(ast::Operation_In& n)
{
  OUT_START
  n.left->accept(*this);
  n.right->accept(*this);
  OUT_END
}
void AST_Printer::visit(ast::Operation_Transfert& n)
{
  OUT_START
  n.left->accept(*this);
  n.right->accept(*this);
  OUT_END
}
void AST_Printer::visit(ast::Operation_Binary& n)
{
  OUT_START
  n.left->accept(*this);
  n.right->accept(*this);
  OUT_END
}
void AST_Printer::visit(ast::Operation_Unary& n)
{
  OUT_START
  n.base->accept(*this);
  OUT_END
}
void AST_Printer::visit(ast::Operation_Interval& n)
{
  OUT_START
  n.left->accept(*this);
  n.center->accept(*this);
  n.right->accept(*this);
  OUT_END
}
void AST_Printer::visit(ast::Operation_Ptr_Dist& n)
{
  OUT_START
  n.left->accept(*this);
  n.right->accept(*this);
  OUT_END
}

// ============ MEMORY ============
void AST_Printer::visit(ast::Memory_Del& n)
{
  OUT_START
  n.target->accept(*this);
  OUT_END
}
void AST_Printer::visit(ast::Memory_Align& n)
{
  OUT_START
  n.target->accept(*this);
  OUT_END
}
void AST_Printer::visit(ast::Memory_Drop& n)
{
  OUT_START
  n.target->accept(*this);
  OUT_END
}
*/

#undef OUT_START