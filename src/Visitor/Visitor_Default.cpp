#include "Visitor_Default.hpp"

#include "ErrorOutput.hpp"
#include "ScriptInfo.hpp"
#include "Globals.hpp"

#include "AST/AST_Base.hpp"
#include "AST/AST_Declaration.hpp"
#include "AST/AST_Declaration_COP.hpp"
#include "AST/AST_Generic.hpp"
#include "AST/AST_Literal.hpp"
#include "AST/AST_Memory.hpp"
#include "AST/AST_Operation.hpp"
#include "AST/AST_Reference.hpp"
#include "AST/AST_Statement.hpp"
#include "AST/AST_Type.hpp"

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
void Visitor_Default::visit(AST::AType_Reference &n)
{
	for (auto &elem : n.gen_args) elem->accept(*this);
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
	n.type->accept(*this);
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
	n.type->accept(*this);
}				

void Visitor_Default::visit(AST::Declaration::Generic &n)
{
	for (auto &elem : n.gen_args) elem->accept(*this);
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
	n.type->accept(*this);
	n.defaultValue->accept(*this);
}				
void Visitor_Default::visit(AST::Declaration::Local::Generic_Parameter &n)
{
	for (auto &elem : n.generic_references) elem->accept(*this);
}

void Visitor_Default::visit(AST::Declaration::Local::Pattern &n)
{
	if (n.additive_evaluator) n.additive_evaluator->accept(*this);
	if (n.reference) n.reference->accept(*this);
}	
void Visitor_Default::visit(AST::Declaration::Local::Pattern_Enum &n)
{
	for (auto &elem : n.mapping) {
		if (auto node = elem.node()) node->accept(*this);
	}
	visit(static_cast<AST::Declaration::Local::Pattern&>(n));
}	
void Visitor_Default::visit(AST::Declaration::Local::Pattern_Tuple &n)
{
	for (auto &elem : n.mapping) {
		if (auto node = elem.node()) node->accept(*this);
	}
	visit(static_cast<AST::Declaration::Local::Pattern&>(n));
}	
void Visitor_Default::visit(AST::Declaration::Local::Pattern_Entity &n)
{
	for (auto &[id, _, elem] : n.mapping) {
		if (auto node = elem.node()) node->accept(*this);
	}
	visit(static_cast<AST::Declaration::Local::Pattern&>(n));
}	
void Visitor_Default::visit(AST::Declaration::Local::Pattern_Component &n)
{
	for (auto &[_, elem] : n.mapping) {
		if (auto node = elem.node()) node->accept(*this);
	}
	visit(static_cast<AST::Declaration::Local::Pattern&>(n));
}	

void Visitor_Default::visit(AST::Declaration::Local::Variable_Binding &n)
{
	n.reference->accept(*this);
}	
void Visitor_Default::visit(AST::Declaration::Local::Variable_Unpack &n)
{
	for (auto &elem : n.elements) elem->accept(*this);
	n.reference->accept(*this);
}	
void Visitor_Default::visit(AST::Declaration::Local::Variable &n)
{
	if (n.type) n.type->accept(*this);
	if (n.expression) n.expression.value()->accept(*this); 
}	

void Visitor_Default::visit(AST::Declaration::Local::Capability &n)
{
	n.reference->accept(*this);
}	

// ============ COP ============
void Visitor_Default::visit(AST::Declaration::COP::Component &n)
{
	if (n.gen_where) n.gen_where->accept(*this);
	for (auto &elem : n.fields) elem->accept(*this);

}			
void Visitor_Default::visit(AST::Declaration::COP::Component_Field &n)
{
	n.type->accept(*this);
	if (n.default_value) n.default_value->accept(*this);
}		

void Visitor_Default::visit(AST::Declaration::COP::Role &n)
{
	for (auto &elem : n.components) elem->accept(*this);
}				

void Visitor_Default::visit(AST::Declaration::COP::Entity &n)
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
void Visitor_Default::visit(AST::Declaration::COP::Entity_Cast &n)
{
	n.source->accept(*this);
	n.target->accept(*this);
}			
void Visitor_Default::visit(AST::Declaration::COP::Entity_Op &n)
{
	n.codeblock->accept(*this);
}			
void Visitor_Default::visit(AST::Declaration::COP::Entity_OpIndex &n)
{
	n.codeblock->accept(*this);
	n.return_type->accept(*this);
}		

void Visitor_Default::visit(AST::Declaration::COP::System &n)
{
	n.prototype->accept(*this);
	for (auto &elem : n.cases) elem->accept(*this); 
}				
void Visitor_Default::visit(AST::Declaration::COP::System_Case &n)
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
	if (n.explicit_return_type) n.explicit_return_type->accept(*this);
}
void Visitor_Default::visit(AST::Generic::Have_Role &n)
{
	n.role_reference->accept(*this);
}				
void Visitor_Default::visit(AST::Generic::Use_Component &n)
{
	n.component_reference->accept(*this);
}			
void Visitor_Default::visit(AST::Generic::Compatible_System &n)
{
	n.system_reference->accept(*this);
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
	if (n.variadic_type) n.variadic_type->accept(*this);
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
	for (auto &elem : n.gen_args) elem->accept(*this);
	for (auto &elem : n.field_args) elem->accept(*this);
}
void Visitor_Default::visit(AST::Literal::Entity &n)
{
	for (auto &elem : n.gen_args) elem->accept(*this);
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
	for (auto &elem : n.gen_args) elem->accept(*this);
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
	for (auto &elem : n.base_gen_args) elem->accept(*this);
	for (auto &elem : n.gen_args) {
		for (auto &elem2 : elem) 
			elem2->accept(*this);
	}
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
	n.target->accept(*this);
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
void Visitor_Default::visit(AST::Memory::Ptr_At &n)
{
	n.target->accept(*this);
	n.index->accept(*this);
}
void Visitor_Default::visit(AST::Memory::Ptr_Offset &n)
{
	n.target->accept(*this);
	n.offset->accept(*this);
}