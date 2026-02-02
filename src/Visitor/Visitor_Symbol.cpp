#include "Visitor_Symbol.hpp"

#include <string>
#include <unordered_set>
#include <iostream>

#include "ScriptInfo.hpp"
#include "Symbol_Manager.hpp"

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


std::shared_ptr<AST::ADeclaration> Visitor_Symbol::resolve_def(AST::AReference& ref, std::shared_ptr<AST::ADeclaration> target_resolution, bool silentError) {
	if (!ref.id.parent) ref.id.parent = &ref;

	const std::string name = ref.id.mangle_local_name();
	const std::string absolue_name = ref.id.mangle_absolute_name();

	// get local symbol
	if (auto sym = scrInfo->m_sym->find_symbol(name)) {
		target_resolution = sym.value();
		return sym.value();
	}
	else if (auto sym = scrInfo->m_sym->find_symbol(absolue_name)) {
		target_resolution = sym.value();
		return sym.value();
	}
	// get imported symbol
	else if (!ref.id.path.empty()) {
		const std::string supposed_import_name = ref.id.path[0];
	
		if (auto imp = scrInfo->get_import_module(supposed_import_name)) {
			for (auto &mod : imp->target_modules) {
				if (auto sym = mod->m_sym->find_symbol(absolue_name)) {
					target_resolution = sym.value();
					return sym.value();
				}
			}
		}
	}

	if (!silentError)
		error_add(ref, "SYM1000", "Symbol '" + ref.id.debug_str() + "' definition not found!", "");
	
	return nullptr;
}

// Handle any AST::AReference node children
void Visitor_Symbol::visit(AST::AReference &n)
{
	Visitor_Default::visit(n);
	resolve_def(n, n.get_symbol_resolution());
}


void Visitor_Symbol::visit(AST::Declaration::ECS::Entity &n) 
{
	current_entity = &n;
    Visitor_Default::visit(n);
	current_entity = nullptr;
}


// all AST::Reference handled by AST::AReference 

void Visitor_Symbol::visit(AST::Reference::Self &n)
{
	Visitor_Default::visit(n);
	if (!current_entity)
		error_add(n, "SYM1001", "No entity found the his context.", SYM_HINT);
	
	n.source_sym = std::shared_ptr<AST::Declaration::ECS::Entity>(current_entity);
}

