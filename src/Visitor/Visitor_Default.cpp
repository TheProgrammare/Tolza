#include "Visitor_Default.hpp"

#include "ErrorOutput.hpp"
#include "ScriptInfo.hpp"

#include "AST/AST_Base.hpp"
#include "AST/AST_Declaration.hpp"
#include "AST/AST_Declaration_ECS.hpp"
#include "AST/AST_Generic.hpp"
#include "AST/AST_Literal.hpp"
#include "AST/AST_Memory.hpp"
#include "AST/AST_Operation.hpp"
#include "AST/AST_Reference.hpp"
#include "AST/AST_Statement.hpp"
#include "AST/AST_Type.hpp"


void Visitor_Default::error_add(AST::Node& n, const std::string& errCode, const std::string& err, const std::string& hint) {
	auto pos = n._token.span;
	std::string errStr = make_error_output(pos.line, pos.col, pos.size, scrInfo.src_lines[pos.line - 1], errCode, err, hint, scrInfo.file_path);
	errors.push_back(errStr);
}



// ============ AST ============
void Visitor_Default::visit(AST::Node &n)
{

}

void Visitor_Default::visit(AST::AType &n)
{

}
void Visitor_Default::visit(AST::ALiteral &n)
{

}				
void Visitor_Default::visit(AST::ADeclaration &n)
{

}			
void Visitor_Default::visit(AST::ALocal&n)
{

}			
void Visitor_Default::visit(AST::AReference &n)
{

}				
void Visitor_Default::visit(AST::Identifier_Reference &n)
{

}			
void Visitor_Default::visit(AST::Type_Reference &n)
{

}			
void Visitor_Default::visit(AST::Type_Arguments &n)
{
	for (auto &arg : n.arguments) arg->accept(*this);
}			

void Visitor_Default::visit(AST::Root &n)
{

}

void Visitor_Default::visit(AST::ID &n)
{

}	

// ============ DECLARATION ============
void Visitor_Default::visit(AST::Declaration::Global &n)
{
	n.ty->accept(*this);
	n.expression->accept(*this);
}			
void Visitor_Default::visit(AST::Declaration::Function &n)
{
	n.prototype->accept(*this);
	n.codeblock->accept(*this);
}		

void Visitor_Default::visit(AST::Declaration::Mod &n)
{
	for (auto &elem : n.elements) elem->accept(*this);
}		
void Visitor_Default::visit(AST::Declaration::Export &n)
{
	for (auto &elem : n.elements) elem->accept(*this);
}		

void Visitor_Default::visit(AST::Declaration::Enum &n)
{
	for (auto &elem : n.variants) elem->accept(*this);
}
void Visitor_Default::visit(AST::Declaration::Enum_Element &n)
{
	for (auto &elem : n.types) elem->accept(*this);
}			

void Visitor_Default::visit(AST::Declaration::Flag &n)
{

}

void Visitor_Default::visit(AST::Declaration::Type_Alias &n)
{
	n.ty->accept(*this);
}				

void Visitor_Default::visit(AST::Declaration::Generic &n)
{
	n.gen_args->accept(*this);
	for (auto &elem : n.conditions) elem->accept(*this);
}

// ============ LOCAL ============
void Visitor_Default::visit(AST::Declaration::Local::CodeBlock &n)
{
	for (auto &elem : n.elements) {
		if (auto elem_n = elem.node())
			elem_n->accept(*this);
	}
}	

void Visitor_Default::visit(AST::Declaration::Local::Lambda &n)
{
	n.prototype->accept(*this);
	n.codeblock->accept(*this);
	n.capture->accept(*this);
}
void Visitor_Default::visit(AST::Declaration::Local::Lambda_Capture &n)
{
	for (auto &elem : n.elements) elem->accept(*this);
}			
void Visitor_Default::visit(AST::Declaration::Local::Capture_Member &n)
{

}		

void Visitor_Default::visit(AST::Declaration::Local::Parameter &n)
{
	n.ty->accept(*this);
	n.defaultValue->accept(*this);
}				
void Visitor_Default::visit(AST::Declaration::Local::Generic_Parameter &n)
{
	for (auto &elem : n.generic_references) elem->accept(*this);
}

void Visitor_Default::visit(AST::Declaration::Local::Pattern &n)
{
	if (n.additive_evaluator) n.additive_evaluator->accept(*this);
}	
void Visitor_Default::visit(AST::Declaration::Local::Pattern_Enum &n)
{
	for (auto &elem : n.mapping) {
		if (auto node = elem.node()) node->accept(*this);
	}
	n.enum_reference->accept(*this);
	visit(static_cast<AST::Declaration::Local::Pattern&>(n));
}	
void Visitor_Default::visit(AST::Declaration::Local::Pattern_Tuple &n)
{
	for (auto &elem : n.mapping) {
		if (auto node = elem.node()) node->accept(*this);
	}
	n.tuple_reference->accept(*this);
	visit(static_cast<AST::Declaration::Local::Pattern&>(n));
}	
void Visitor_Default::visit(AST::Declaration::Local::Pattern_Entity &n)
{
	for (auto &[_, _, elem] : n.mapping) {
		if (auto node = elem.node()) node->accept(*this);
	}
	n.entity_reference->accept(*this);
	visit(static_cast<AST::Declaration::Local::Pattern&>(n));
}	
void Visitor_Default::visit(AST::Declaration::Local::Pattern_Component &n)
{
	for (auto &[_, elem] : n.mapping) {
		if (auto node = elem.node()) node->accept(*this);
	}
	n.component_reference->accept(*this);
	visit(static_cast<AST::Declaration::Local::Pattern&>(n));
}	

void Visitor_Default::visit(AST::Declaration::Local::Variable_Binding &n)
{

}	
void Visitor_Default::visit(AST::Declaration::Local::Variable_Unpack &n)
{
	for (auto &elem : n.elements) elem->accept(*this);
	n.reference->accept(*this);
}	
void Visitor_Default::visit(AST::Declaration::Local::Variable &n)
{
	if (n.ty) n.ty->accept(*this);
	if (n.expression) n.expression->accept(*this); 
}	

void Visitor_Default::visit(AST::Declaration::Local::Capability &n)
{
	n.reference->accept(*this);
}	

// ============ ECS ============
void Visitor_Default::visit(AST::Declaration::ECS::Component &n)
{
	if (n.gen_where) n.gen_where->accept(*this);
	for (auto &elem : n.fields) elem->accept(*this);

}			
void Visitor_Default::visit(AST::Declaration::ECS::Component_Field &n)
{
	n.ty->accept(*this);
	if (n.default_value) n.default_value->accept(*this);
}		

void Visitor_Default::visit(AST::Declaration::ECS::Role &n)
{
	for (auto &elem : n.components) elem->accept(*this);
}				

void Visitor_Default::visit(AST::Declaration::ECS::Entity &n)
{
	for (auto &elem : n.comps) elem->accept(*this);
	for (auto &[proto, elem] : n.constructors) {
		proto->accept(*this);
		elem->accept(*this);
	}
	if (n.gen_params) n.gen_params->accept(*this);
	for (auto &elem : n.operators) elem->accept(*this);
	for (auto &elem : n.casts) elem->accept(*this);
}				
void Visitor_Default::visit(AST::Declaration::ECS::Entity_Cast &n)
{
	n.source->accept(*this);
	n.target->accept(*this);
}			
void Visitor_Default::visit(AST::Declaration::ECS::Entity_Op &n)
{
	n.codeblock->accept(*this);
}			
void Visitor_Default::visit(AST::Declaration::ECS::Entity_OpIndex &n)
{
	n.codeblock->accept(*this);
	n.return_type->accept(*this);
}		

void Visitor_Default::visit(AST::Declaration::ECS::System &n)
{
	n.prototype->accept(*this);
	for (auto &elem : n.cases) elem->accept(*this); 
}				
void Visitor_Default::visit(AST::Declaration::ECS::System_Case &n)
{
	for (auto &elem : n.bindings) elem->accept(*this);
	n.codeblock->accept(*this);
}	

// ============ GENERIC ============
void Visitor_Default::visit(AST::Generic::Is_Type &n)
{
	for (auto &elem : n.inType) elem->accept(*this);
}
void Visitor_Default::visit(AST::Generic::Can_Cast &n)
{
	n.target->accept(*this);
}				
void Visitor_Default::visit(AST::Generic::Have_Op &n)
{
	if (n.explicit_return_ty) n.explicit_return_ty->accept(*this);
}
void Visitor_Default::visit(AST::Generic::Have_Role &n)
{
	n.role->accept(*this);
}				
void Visitor_Default::visit(AST::Generic::Use_Component &n)
{
	n.component->accept(*this);
}			
void Visitor_Default::visit(AST::Generic::Compatible_System &n)
{
	n.system->accept(*this);
}		

// ============ TYPE ============
void Visitor_Default::visit(AST::Type::Ptr &n)
{
	n.inner->accept(*this);
}	
void Visitor_Default::visit(AST::Type::Table &n)
{
	if (n.sizeSymbol) n.sizeSymbol->accept(*this);
	n.inner->accept(*this);
}
void Visitor_Default::visit(AST::Type::Primitive &n)
{

}				
void Visitor_Default::visit(AST::Type::Tuple &n)
{
	for (auto &elem : n.types) elem->accept(*this);
}
void Visitor_Default::visit(AST::Type::Function_Proto &n)
{
	for (auto &elem : n.parameters) elem->accept(*this);
	for (auto &elem : n.gen_parameters) elem->accept(*this);
	if (n.returnType) n.returnType->accept(*this);
	if (n.variadic_ty) n.variadic_ty->accept(*this);
}			

void Visitor_Default::visit(AST::Type::Get_Expr_Type &n)
{
	n.target->accept(*this);
}			

// ============ LITERAL ============
void Visitor_Default::visit(AST::Literal::Boolean &n)
{

}
void Visitor_Default::visit(AST::Literal::Integral &n)
{

}				
void Visitor_Default::visit(AST::Literal::Decimal &n)
{

}
void Visitor_Default::visit(AST::Literal::Floating &n)
{

}				

void Visitor_Default::visit(AST::Literal::ASCII &n)
{

}
void Visitor_Default::visit(AST::Literal::UFT32 &n)
{

}

void Visitor_Default::visit(AST::Literal::Text &n)
{

}			
void Visitor_Default::visit(AST::Literal::Text_Lerp &n)
{
	if (n.expression) n.expression->accept(*this);
	if (n.spec) n.spec->accept(*this);
}			
void Visitor_Default::visit(AST::Literal::Textual_Element &n)
{
	n.val->accept(*this);
}			
void Visitor_Default::visit(AST::Literal::Textual_Format &n)
{
	for (auto &elem : n.values) {
		elem.val->accept(*this);
	}
}			
void Visitor_Default::visit(AST::Literal::Format_Specifier &n)
{
	if (n.width) n.width->accept(*this);
	if (n.precision) n.precision->accept(*this);
}		

void Visitor_Default::visit(AST::Literal::Table &n)
{
	for (auto &elem: n.values) elem->accept(*this);
	if (n.population) n.population->accept(*this);
}
void Visitor_Default::visit(AST::Literal::Table_Population &n)
{
	for (auto &elem : n.ranges) elem->accept(*this);
	n.expression->accept(*this);
	if (n.map_expression_value) n.map_expression_value->accept(*this);
}		

void Visitor_Default::visit(AST::Literal::Map &n)
{
	for (auto &elem : n.keys) elem->accept(*this);
	for (auto &elem : n.values) elem->accept(*this);
	if (n.population) n.population->accept(*this);
}	

void Visitor_Default::visit(AST::Literal::Tuple &n)
{
	for (auto &elem : n.values) elem->accept(*this);
	for (auto &elem : n.tys) elem->accept(*this);
}

void Visitor_Default::visit(AST::Literal::Range &n)
{
	if (n.start) n.start->accept(*this);
	if (n.end) n.end->accept(*this);
	if (n.step) n.step->accept(*this);
}

void Visitor_Default::visit(AST::Literal::Component &n)
{
	if (n.gen_args) n.gen_args->accept(*this);
	for (auto &elem : n.field_args) elem->accept(*this);
}
void Visitor_Default::visit(AST::Literal::Entity &n)
{
	if (n.gen_args) n.gen_args->accept(*this);
	for (auto &elem : n.comp_args) elem->accept(*this);
}

// ============ REFERENCE ============
void Visitor_Default::visit(AST::Reference::Enum &n)
{
	for (auto &elem : n.member_values) elem->accept(*this);
}

void Visitor_Default::visit(AST::Reference::Member_Access &n)
{
	n.left->accept(*this);
	n.right->accept(*this);
}		

void Visitor_Default::visit(AST::Reference::Self &n)
{

}
void Visitor_Default::visit(AST::Reference::Other &n)
{

}

void Visitor_Default::visit(AST::Reference::Call &n)
{
	if (n.gen_args) n.gen_args->accept(*this);
	for (auto &elem : n.param_args) elem->accept(*this);
}
void Visitor_Default::visit(AST::Reference::Call_Argument &n)
{
	n.val->accept(*this);
}			
void Visitor_Default::visit(AST::Reference::Call_System &n)
{
	n.target_entity->accept(*this);
}				
void Visitor_Default::visit(AST::Reference::Call_Pipe &n)
{
	if (n.base_gen_args) n.base_gen_args->accept(*this);
	for (auto &elem : n.gen_args) elem->accept(*this);
	for (auto &args : n.arguments) {
		for (auto &arg : args) arg->accept(*this);
	}
}				

void Visitor_Default::visit(AST::Reference::Table_Access &n)
{
	n.selector->accept(*this);
}				

// ============ STATEMENT ============
void Visitor_Default::visit(AST::Statement::If &n)
{
	if (auto node = n.evaluator.node()) node->accept(*this);
	n.codeblock->accept(*this);
	if (n.alternative_statement) n.alternative_statement->accept(*this);
}	
void Visitor_Default::visit(AST::Statement::If_Ternary &n)
{
	if (auto node = n.evaluator.node()) node->accept(*this);
	n.true_line->accept(*this);
	if (n.false_line) n.false_line->accept(*this);
}				

void Visitor_Default::visit(AST::Statement::For &n)
{
	n.src->accept(*this);
	if (n.index) n.index->accept(*this);
	for (auto &elem : n.items) elem->accept(*this);
	n.codeblock->accept(*this);
}	
void Visitor_Default::visit(AST::Statement::Loop &n)
{
	n.codeblock->accept(*this);
}
void Visitor_Default::visit(AST::Statement::While &n)
{
	if (auto node = n.evaluator.node()) node->accept(*this);
	n.codeblock->accept(*this);
}
void Visitor_Default::visit(AST::Statement::GoTo &n)
{

}
void Visitor_Default::visit(AST::Statement::GoTo_Label &n)
{

}				

void Visitor_Default::visit(AST::Statement::Return &n)
{
	if (n.value) n.value->accept(*this);
}
void Visitor_Default::visit(AST::Statement::Break &n)
{

}
void Visitor_Default::visit(AST::Statement::Continue &n)
{

}				

void Visitor_Default::visit(AST::Statement::Match &n)
{
	n.base->accept(*this);
	for (auto &elem : n.cases) elem->accept(*this);
	if (n.other_case) n.other_case->accept(*this);
}
void Visitor_Default::visit(AST::Statement::Match_Case &n)
{
	if (auto node = n.evaluator.node()) node->accept(*this);
	n.codeblock->accept(*this);
}

// ============ OPERATION ============
void Visitor_Default::visit(AST::Operation::Cast_As &n)
{
	n.valueCasted->accept(*this);
	n.typeCasted->accept(*this);
}                 
void Visitor_Default::visit(AST::Operation::Is &n)
{
	n.left->accept(*this);
	n.right->accept(*this);
}                      
void Visitor_Default::visit(AST::Operation::In &n)
{
	n.left->accept(*this);
	n.right->accept(*this);
}                      
void Visitor_Default::visit(AST::Operation::Assignment &n)
{
	n.left->accept(*this);
	n.right->accept(*this);
}              
void Visitor_Default::visit(AST::Operation::Binary &n)
{
	n.left->accept(*this);
	n.right->accept(*this);
}
void Visitor_Default::visit(AST::Operation::Unary &n)
{
	n.base->accept(*this);
}
void Visitor_Default::visit(AST::Operation::Interval &n)
{
	n.left->accept(*this);
	n.center->accept(*this);
	n.right->accept(*this);
}


// ============ MEMORY ============
void Visitor_Default::visit(AST::Memory::Move &n)
{
	n.target->accept(*this);
}
void Visitor_Default::visit(AST::Memory::New &n)
{
	n.type->accept(*this);
	n.expression->accept(*this);
}
void Visitor_Default::visit(AST::Memory::Del &n)
{
	n.element->accept(*this);
}
void Visitor_Default::visit(AST::Memory::Val_Of_Ptr &n)
{
	n.target->accept(*this);
}
void Visitor_Default::visit(AST::Memory::Addr_Of_Ref &n)
{
	n.target->accept(*this);
}
void Visitor_Default::visit(AST::Memory::Dist &n)
{
	n.left->accept(*this);
	n.right->accept(*this);
}
void Visitor_Default::visit(AST::Memory::Size &n)
{
	n.target->accept(*this);
}
void Visitor_Default::visit(AST::Memory::Align &n)
{
	n.target->accept(*this);
}
void Visitor_Default::visit(AST::Memory::GetBits &n)
{
	n.target->accept(*this);
	n.range->accept(*this);
}
void Visitor_Default::visit(AST::Memory::Drop &n)
{
	n.target->accept(*this);
}