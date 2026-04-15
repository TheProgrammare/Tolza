#include "visitor_default.hpp"

#include "compiler/compiler.hpp"
#include "misc/error_output.hpp"

#include "ast/ast_evaluator.hpp"

#include "ast/ast_base.hpp"
#include "ast/ast_declaration.hpp"
#include "ast/ast_declaration_local.hpp"
#include "ast/ast_declaration_cop.hpp"
#include "ast/ast_generic.hpp"
#include "ast/ast_literal.hpp"
#include "ast/ast_memory.hpp"
#include "ast/ast_operation.hpp"
#include "ast/ast_statement.hpp"
#include "ast/ast_expression.hpp"
#include "ast/ast_type.hpp"
#include "misc/script_info.hpp"
#include <algorithm>
#include <iostream>


void Visitor_Default::add_error(ErrorCode code, const ast::Node& n, const std::string& msg,
                                const std::string& hint) const
{
  auto error = Error_Diagnostic(scr_info, code, n.node_scr_info.get(), n.node_token, current_EPhase(), msg, hint);

  errors.push_back(error.print_error());
}

void Visitor_Default::add_error_two_nodes(ErrorCode code, const ast::Node& first, const ast::Node& second,
                                          const std::string& msg, const std::string& hint) const
{
  auto err = Error_Diagnostic_Two(scr_info, code, first, second, current_EPhase(), msg, hint);
  errors.push_back(err.print_error());
}


// ============ AST ============
void Visitor_Default::visit(ast::Node& n)
{
}

void Visitor_Default::visit(ast::AType& n)
{
}
void Visitor_Default::visit(ast::ALiteral& n)
{
}
void Visitor_Default::visit(ast::ADeclaration& n)
{
}
void Visitor_Default::visit(ast::ALocal& n)
{
}
void Visitor_Default::visit(ast::AExpression& n)
{
}
void Visitor_Default::visit(ast::AIdentifier& n)
{
}
void Visitor_Default::visit(ast::Expr_ID& n)
{
}
void Visitor_Default::visit(ast::Expr_ID_Qualified& n)
{
}
void Visitor_Default::visit(ast::Expr_ID_Type& n)
{
  n.name->accept(*this);
  for (auto& elem : n.gen_args) elem->accept(*this);
}

void Visitor_Default::visit(ast::Root& n)
{
  for (auto& elem : n.global_nodes) elem->accept(*this);
}

// ============ DECLARATION ============
void Visitor_Default::visit(ast::declaration::Global& n)
{
  if (n.type) n.type->accept(*this);
  if (n.expression) n.expression->accept(*this);
}
void Visitor_Default::visit(ast::declaration::Function& n)
{
  n.prototype->accept(*this);
  if (n.codeblock) n.codeblock->accept(*this);
}

void Visitor_Default::visit(ast::declaration::Mod& n)
{
  for (auto& elem : n.declarations) elem->accept(*this);
}
void Visitor_Default::visit(ast::declaration::Export& n)
{
  for (auto& elem : n.declarations) elem->accept(*this);
}
void Visitor_Default::visit(ast::declaration::Extern& n)
{
  for (auto& elem : n.declarations) elem->accept(*this);
}

void Visitor_Default::visit(ast::declaration::Enum& n)
{
  for (auto& elem : n.variants) elem->accept(*this);
}
void Visitor_Default::visit(ast::declaration::Enum_Element& n)
{
  for (auto& elem : n.types) elem->accept(*this);
}

void Visitor_Default::visit(ast::declaration::Flag& n)
{
}

void Visitor_Default::visit(ast::declaration::Union& n)
{
  for (auto& [name, ty] : n.fields) ty->accept(*this);
}

void Visitor_Default::visit(ast::declaration::Type_Alias& n)
{
  n.type->accept(*this);
}
void Visitor_Default::visit(ast::declaration::Mod_Alias& n)
{
  n.module->accept(*this);
}

void Visitor_Default::visit(ast::declaration::Generic& n)
{
  for (auto& elem : n.gen_args) elem->accept(*this);
  for (auto& elem : n.conditions) elem->accept(*this);
}

// ============ LOCAL ============
void Visitor_Default::visit(ast::declaration::local::CodeBlock& n)
{
  for (auto& elem : n.elements) {
    if (auto elem_n = elem.node()) elem_n->accept(*this);
  }
}

void Visitor_Default::visit(ast::declaration::local::Lambda& n)
{
  n.prototype->accept(*this);
  n.codeblock->accept(*this);
  n.capture->accept(*this);
}
void Visitor_Default::visit(ast::declaration::local::Lambda_Capture& n)
{
  for (auto& elem : n.elements) elem->accept(*this);
}
void Visitor_Default::visit(ast::declaration::local::Capture_Member& n)
{
}

void Visitor_Default::visit(ast::declaration::local::Parameter& n)
{
  n.type->accept(*this);
  if (n.defaultValue) n.defaultValue->accept(*this);
}
void Visitor_Default::visit(ast::declaration::local::Generic_Parameter_Element& n)
{
  for (auto& elem : n.generic_references) elem->accept(*this);
}
void Visitor_Default::visit(ast::declaration::local::Generic_Parameters& n)
{
  for (auto& elem : n.parameters) elem->accept(*this);
}

void Visitor_Default::visit(ast::declaration::local::Pattern& n)
{
  if (n.additive_evaluator) n.additive_evaluator->accept(*this);
  if (n.right) n.right->accept(*this);
}
void Visitor_Default::visit(ast::declaration::local::Pattern_Enum& n)
{
  Visitor_Default::visit(static_cast<ast::declaration::local::Pattern&>(n));
  for (auto& elem : n.mapping) {
    if (auto node = elem->node()) node->accept(*this);
  }
}
void Visitor_Default::visit(ast::declaration::local::Pattern_Tuple& n)
{
  Visitor_Default::visit(static_cast<ast::declaration::local::Pattern&>(n));
  for (auto& elem : n.mapping) {
    if (auto node = elem->node()) node->accept(*this);
  }
}
void Visitor_Default::visit(ast::declaration::local::Pattern_Entity& n)
{
  Visitor_Default::visit(static_cast<ast::declaration::local::Pattern&>(n));
  for (auto& elem : n.mapping) {
    for (auto& [_, elem2] : elem->mapping) elem2->node()->accept(*this);
  }
}
void Visitor_Default::visit(ast::declaration::local::Pattern_System_Component& n)
{
  Visitor_Default::visit(static_cast<ast::declaration::local::Pattern&>(n));
  n.name->accept(*this);
  n.bind->accept(*this);
}
void Visitor_Default::visit(ast::declaration::local::Pattern_Component& n)
{
  Visitor_Default::visit(static_cast<ast::declaration::local::Pattern&>(n));
  for (auto& [_, elem] : n.mapping) {
    if (auto node = elem->node()) node->accept(*this);
  }
}

void Visitor_Default::visit(ast::declaration::local::Variable_Binding& n)
{
}
void Visitor_Default::visit(ast::declaration::local::Tuple_Destructuring& n)
{
  for (auto& elem : n.elements) elem->accept(*this);
  n.right->accept(*this);
}
void Visitor_Default::visit(ast::declaration::local::Variable& n)
{
  if (n.type) n.type->accept(*this);
  if (n.expression) n.expression->accept(*this);
}

void Visitor_Default::visit(ast::declaration::local::Capability& n)
{
  n.right->accept(*this);
}

// ============ COP ============
void Visitor_Default::visit(ast::declaration::cop::Component& n)
{
  if (n.gen_where) n.gen_where->accept(*this);
  for (auto& elem : n.fields) elem->accept(*this);
}
void Visitor_Default::visit(ast::declaration::cop::Component_Field& n)
{
  n.type->accept(*this);
  if (n.default_value) n.default_value->accept(*this);
}

void Visitor_Default::visit(ast::declaration::cop::Role& n)
{
  for (auto& elem : n.components) elem->accept(*this);
}

void Visitor_Default::visit(ast::declaration::cop::Entity& n)
{
  for (auto& elem : n.comps) elem->accept(*this);
  if (n.gen_params) n.gen_params->accept(*this);
  for (auto& elem : n.news) elem->accept(*this);
  n.del->accept(*this);
  for (auto& elem : n.operators) elem->accept(*this);
  for (auto& elem : n.casts) elem->accept(*this);
}
void Visitor_Default::visit(ast::declaration::cop::Entity_New& n)
{
  n.prototype->accept(*this);
  n.codeblock->accept(*this);
}
void Visitor_Default::visit(ast::declaration::cop::Entity_Del& n)
{
  n.codeblock->accept(*this);
}
void Visitor_Default::visit(ast::declaration::cop::Entity_Cast& n)
{
  n.source->accept(*this);
  n.target->accept(*this);
}
void Visitor_Default::visit(ast::declaration::cop::Entity_Op& n)
{
  n.codeblock->accept(*this);
}
void Visitor_Default::visit(ast::declaration::cop::Entity_Access_Op& n)
{
  n.codeblock->accept(*this);
  n.return_type->accept(*this);
}
void Visitor_Default::visit(ast::declaration::cop::Entity_Transfert& n)
{
  n.codeblock->accept(*this);
}

void Visitor_Default::visit(ast::declaration::cop::System& n)
{
  n.prototype->accept(*this);
  for (auto& elem : n.cases) elem->accept(*this);
}
void Visitor_Default::visit(ast::declaration::cop::System_Case& n)
{
  for (auto& elem : n.bindings) elem->accept(*this);
  n.codeblock->accept(*this);
}

// ============ GENERIC ============
void Visitor_Default::visit(ast::generic::Is_Type& n)
{
  for (auto& elem : n.in_type) elem->accept(*this);
}
void Visitor_Default::visit(ast::generic::Can_Cast& n)
{
  n.target->accept(*this);
}
void Visitor_Default::visit(ast::generic::Have_Op& n)
{
  if (n.explicit_return_type) n.explicit_return_type->accept(*this);
}
void Visitor_Default::visit(ast::generic::Have_Role& n)
{
  n.role->accept(*this);
}
void Visitor_Default::visit(ast::generic::Use_Component& n)
{
  n.component->accept(*this);
}
void Visitor_Default::visit(ast::generic::Compatible_System& n)
{
  n.system->accept(*this);
}

// ============ TYPE ============
void Visitor_Default::visit(ast::type::Ptr& n)
{
  n.inner->accept(*this);
}
void Visitor_Default::visit(ast::type::Table& n)
{
  if (n.size_sym) n.size_sym->accept(*this);
  n.inner->accept(*this);
}
void Visitor_Default::visit(ast::type::Primitive& n)
{
}
void Visitor_Default::visit(ast::type::Tuple& n)
{
  for (auto& elem : n.types) elem->accept(*this);
}
void Visitor_Default::visit(ast::type::Function_Proto& n)
{
  for (auto& elem : n.parameters) elem->accept(*this);
  for (auto& elem : n.gen_parameters) elem->accept(*this);
  if (n.return_ty) n.return_ty->accept(*this);
}

void Visitor_Default::visit(ast::type::Get_Expr_Type& n)
{
  n.target->accept(*this);
}

// ============ LITERAL ============
void Visitor_Default::visit(ast::literal::Boolean& n)
{
}
void Visitor_Default::visit(ast::literal::Integral& n)
{
}
void Visitor_Default::visit(ast::literal::Fixed_Point& n)
{
}
void Visitor_Default::visit(ast::literal::Floating_Point& n)
{
}

void Visitor_Default::visit(ast::literal::CUNE& n)
{
}
void Visitor_Default::visit(ast::literal::RUNE& n)
{
}

void Visitor_Default::visit(ast::literal::Text_Pure& n)
{
}
void Visitor_Default::visit(ast::literal::Text_Interpolation& n)
{
  if (n.expression) n.expression->accept(*this);
  if (n.spec) n.spec->accept(*this);
}
void Visitor_Default::visit(ast::literal::Textual_Format& n)
{
  for (auto& elem : n.values) elem->accept(*this);
}
void Visitor_Default::visit(ast::literal::Format_Specifier& n)
{
  if (n.width) n.width->accept(*this);
  if (n.precision) n.precision->accept(*this);
}

void Visitor_Default::visit(ast::literal::Table& n)
{
  for (auto& elem : n.values) elem->accept(*this);
  if (n.population) n.population->accept(*this);
}
void Visitor_Default::visit(ast::literal::Table_Population& n)
{
  for (auto& elem : n.ranges) elem->accept(*this);
  n.expression->accept(*this);
  if (n.map_expression_value) n.map_expression_value->accept(*this);
}

void Visitor_Default::visit(ast::literal::Map& n)
{
  for (auto& elem : n.keys) elem->accept(*this);
  for (auto& elem : n.values) elem->accept(*this);
  if (n.population) n.population->accept(*this);
}

void Visitor_Default::visit(ast::literal::Tuple& n)
{
  for (auto& elem : n.values) elem->accept(*this);
}

void Visitor_Default::visit(ast::literal::Range& n)
{
  if (n.start) n.start->accept(*this);
  if (n.end) n.end->accept(*this);
  if (n.step) n.step->accept(*this);
}

void Visitor_Default::visit(ast::literal::Iterator& n)
{
  if (n.collection) n.collection->accept(*this);
}

void Visitor_Default::visit(ast::literal::Enum& n)
{
  n.name->accept(*this);
  for (auto& elem : n.member_values) elem->accept(*this);
}


void Visitor_Default::visit(ast::literal::Structured_Data& n)
{
  n.name->accept(*this);
  for (auto& elem : n.field_args) elem->accept(*this);
}
void Visitor_Default::visit(ast::literal::Entity& n)
{
  n.name->accept(*this);
  for (auto& elem : n.comp_args) elem->accept(*this);
}

// ============ Expression ============
void Visitor_Default::visit(ast::expression::If_Ternary& n)
{
  if (auto node = n.evaluator.get_node()) node->accept(*this);
  n.true_line->accept(*this);
  n.false_line->accept(*this);
}

void Visitor_Default::visit(ast::expression::Member_Access& n)
{
  n.left->accept(*this);
  n.right->accept(*this);
}

void Visitor_Default::visit(ast::expression::Self& n)
{
}
void Visitor_Default::visit(ast::expression::Other& n)
{
}

void Visitor_Default::visit(ast::expression::Call& n)
{
  n.callee->accept(*this);
  for (auto& elem : n.param_args) elem->accept(*this);
  for (auto& elem : n.gen_args) elem->accept(*this);
}
void Visitor_Default::visit(ast::expression::Call_Argument& n)
{
  n.expression->accept(*this);
}
void Visitor_Default::visit(ast::expression::Call_System& n)
{
  n.callee->accept(*this);
  n.target_entity->accept(*this);
  for (auto& elem : n.param_args) elem->accept(*this);
  for (auto& elem : n.gen_args) elem->accept(*this);
}
void Visitor_Default::visit(ast::expression::Call_Pipe& n)
{
  n.callee->accept(*this);
  for (auto& elem : n.base_gen_args) elem->accept(*this);
  for (auto& elem : n.gen_args) {
    for (auto& elem1 : elem) elem1->accept(*this);
  }
  for (auto& elem : n.arguments) {
    for (auto& elem1 : elem) elem1->accept(*this);
  }
}

void Visitor_Default::visit(ast::expression::Table_Access& n)
{
  n.selector->accept(*this);
}

void Visitor_Default::visit(ast::expression::Ptr_At& n)
{
  n.target->accept(*this);
  n.index->accept(*this);
}
void Visitor_Default::visit(ast::expression::Ptr_Offset& n)
{
  n.target->accept(*this);
  n.offset->accept(*this);
}
void Visitor_Default::visit(ast::expression::Ptr_Val& n)
{
  n.target->accept(*this);
}
void Visitor_Default::visit(ast::expression::Mut_Of& n)
{
  n.target->accept(*this);
}
void Visitor_Default::visit(ast::expression::Ref_Of& n)
{
  n.target->accept(*this);
}
void Visitor_Default::visit(ast::expression::Addr_Of& n)
{
  n.target->accept(*this);
}
void Visitor_Default::visit(ast::expression::Size_Of& n)
{
  n.target->accept(*this);
}
void Visitor_Default::visit(ast::expression::GetBits& n)
{
  n.target->accept(*this);
  n.range->accept(*this);
}

void Visitor_Default::visit(ast::expression::Move& n)
{
  n.target->accept(*this);
}
void Visitor_Default::visit(ast::expression::New_Ptr& n)
{
  n.type->accept(*this);
  n.expression->accept(*this);
}

// ============ STATEMENT ============
void Visitor_Default::visit(ast::statement::If& n)
{
  if (auto node = n.evaluator.get_node()) node->accept(*this);
  n.codeblock->accept(*this);
  if (n.alternative_statement) n.alternative_statement->accept(*this);
}

void Visitor_Default::visit(ast::statement::For& n)
{
  n.expression->accept(*this);
  if (n.index) n.index->accept(*this);
  for (auto& elem : n.items) elem->accept(*this);
  n.codeblock->accept(*this);
}
void Visitor_Default::visit(ast::statement::Loop& n)
{
  n.codeblock->accept(*this);
}
void Visitor_Default::visit(ast::statement::While& n)
{
  if (auto node = n.evaluator.get_node()) node->accept(*this);
  n.codeblock->accept(*this);
}
void Visitor_Default::visit(ast::statement::GoTo& n)
{
}
void Visitor_Default::visit(ast::statement::GoTo_Label& n)
{
}

void Visitor_Default::visit(ast::statement::Return& n)
{
  if (n.value) n.value->accept(*this);
}
void Visitor_Default::visit(ast::statement::Break& n)
{
}
void Visitor_Default::visit(ast::statement::Continue& n)
{
}

void Visitor_Default::visit(ast::statement::Match& n)
{
  n.base->accept(*this);
  for (auto& elem : n.cases) elem->accept(*this);
  if (n.other_case) n.other_case->accept(*this);
}
void Visitor_Default::visit(ast::statement::Match_Case& n)
{
  if (auto node = n.evaluator.get_node()) node->accept(*this);
  n.codeblock->accept(*this);
}

// ============ OPERATION ============
void Visitor_Default::visit(ast::operation::Cast_As& n)
{
  n.expression->accept(*this);
  n.type->accept(*this);
}
void Visitor_Default::visit(ast::operation::Is& n)
{
  n.left->accept(*this);
  n.right->accept(*this);
}
void Visitor_Default::visit(ast::operation::In& n)
{
  n.left->accept(*this);
  n.right->accept(*this);
}
void Visitor_Default::visit(ast::operation::Assignment& n)
{
  n.left->accept(*this);
  n.right->accept(*this);
}
void Visitor_Default::visit(ast::operation::Binary& n)
{
  n.left->accept(*this);
  n.right->accept(*this);
}
void Visitor_Default::visit(ast::operation::Unary& n)
{
  n.base->accept(*this);
}
void Visitor_Default::visit(ast::operation::Interval& n)
{
  n.left->accept(*this);
  n.center->accept(*this);
  n.right->accept(*this);
}
void Visitor_Default::visit(ast::operation::Ptr_Dist& n)
{
  n.left->accept(*this);
  n.right->accept(*this);
}

// ============ MEMORY ============
void Visitor_Default::visit(ast::memory::Del& n)
{
  n.target->accept(*this);
}
void Visitor_Default::visit(ast::memory::Align& n)
{
  n.target->accept(*this);
}
void Visitor_Default::visit(ast::memory::Drop& n)
{
  n.target->accept(*this);
}
