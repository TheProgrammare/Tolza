#include "Symbol_Manager.hpp"

#include "AST/ASTBase.hpp"
#include "AST/ASTDeclaration.hpp"

std::string Symbols_Manager::get_current_export_name() const
{
	if (current_scope_path.empty()) return "";

	if (current_scope_path[0].type == EScopeType::Export) 
		return current_scope_path[0].name;

	return "";
}

void Symbols_Manager::add_decl(std::shared_ptr<AST::ADeclaration> declaration)
{
    declaration->_scope = get_current_path();

	SymbolData sym;
	sym.symbol = declaration;
	sym.mangling = declaration->id.mangle_local_name();
	sym.is_exported = !get_current_export_name().empty();

	sym.type = declaration->get_symbol_type();
	
	if (auto ptr = std::dynamic_pointer_cast<AST::Declaration::Global>(declaration))
		sym.is_external = ptr->isExtern;
	else if (auto ptr = std::dynamic_pointer_cast<AST::Declaration::Function>(declaration))
		sym.is_external = ptr->isExtern;

	symbols.push_back(sym);
}

void Symbols_Manager::enter_scope(const std::string& name, EScopeType ty, size_t depth) {
	current_scope_path.push_back(ScopeData{ name, ty, depth });
}

void Symbols_Manager::exit_scope() {
	current_scope_path.pop_back();
}

std::vector<std::string> Symbols_Manager::get_current_path() const
{
	std::vector<std::string> result;
	result.reserve(current_scope_path.size());
	for (auto &elem : current_scope_path)
		result.push_back(elem.name);

	std::reverse(result.begin(), result.end());
	return result;
}

std::optional<std::shared_ptr<AST::ADeclaration>> Symbols_Manager::find_symbol(const std::string &full_name)
{
	if (auto it = symbols.find(full_name); it != symbols.end())
		return it->second;
	else
		return std::nullopt;
}
