#include "Visitor_Symbol.hpp"

#include <string>

#include "ScriptInfo.hpp"
#include "Symbol_Manager.hpp"

Visitor_Symbol::~Visitor_Symbol() = default;


std::shared_ptr<AST::ADeclaration> Visitor_Symbol::resolve_def(AST::AReference& ref, std::shared_ptr<AST::ADeclaration> target_resolution, bool silentError) {
	if (!ref.id.parent) ref.id.parent = &ref;

	const std::string name = ref.id.mangle_local_name();
	const std::string absolue_name = ref.id.mangle_absolute_name();

	// get local symbol
	if (auto sym = scrInfo.m_sym->find_symbol(name)) {
		target_resolution = sym.value();
		return sym.value();
	}
	else if (auto sym = scrInfo.m_sym->find_symbol(absolue_name)) {
		target_resolution = sym.value();
		return sym.value();
	}
	// get imported symbol
	else if (!ref.id.path.empty()) {
		const std::string supposed_import_name = ref.id.path[0];
	
		if (auto imp = scrInfo.get_import_module(supposed_import_name)) {
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
