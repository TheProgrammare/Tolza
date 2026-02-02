
#pragma once

#include <memory>
#include <string>
#include <optional>
#include <unordered_map>
#include <stack>
#include <vector>
#include <set>
#include <variant>

struct ScriptInfo;

enum class EScopeType;

namespace AST {
	struct ADeclaration;
}

struct ScopeData {
	std::string name;
	EScopeType type;
	size_t depth = 0;
};

struct SymbolData {
	ESymbolType type = ESymbolType::NONE;

	std::string mangling;
	std::string mangling_convention_name = "Velox";

	std::shared_ptr<AST::ADeclaration> symbol;

	bool is_exported = false;
	bool is_external = false;
};

struct Symbols_Manager {
	Symbols_Manager(ScriptInfo *scrInfo) : scrInfo(scrInfo) {}

	ScriptInfo *scrInfo = nullptr;
	
	std::vector<SymbolData>	symbols;

	std::vector<ScopeData> current_scope_path;

	std::vector<std::string> decl_errors;

	std::string get_current_export_name() const;

	void add_decl(std::shared_ptr<AST::ADeclaration> declaration);
	void enter_scope(const std::string& name, EScopeType ty, size_t depth = 0);
	void exit_scope();
	[[nodiscard]] std::vector<std::string> get_current_path() const;

	[[nodiscrad]] std::optional<std::shared_ptr<AST::ADeclaration>> find_symbol(const std::string &full_name);

};
