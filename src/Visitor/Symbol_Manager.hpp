
#pragma once

#include <memory>
#include <string>
#include <optional>
#include <vector>

#include "AST/AST_Base.hpp"

struct ScriptInfo;

enum class EScopeType;
enum class ESymbolType;

namespace AST {
	struct ADeclaration;
	struct AReference;
}

struct ScopeData {
	std::string name;
	EScopeType type;
	size_t depth = 0;
};

struct Declaration_Data {
	ESymbolType type;

	std::string mangling;
	std::string mangling_convention_name = "Velox";

	std::shared_ptr<AST::ADeclaration> symbol;

	bool is_exported = false;
	bool is_external = false;
};

struct Reference_Data {
	ESymbolType type;

	std::string manging;
	std::string mangling_convention_name = "Velox";

	AST::SYM_REF<AST::AReference> *symbol;

	bool is_external = false;
};

struct Symbols_Manager {
	Symbols_Manager(ScriptInfo *scrInfo) : scrInfo(scrInfo) {}

	ScriptInfo *scrInfo = nullptr;
	
	std::vector<Declaration_Data> declarations;
	std::vector<Reference_Data> references;

	std::vector<ScopeData> current_scope_path;

	std::vector<std::string> decl_errors;

	std::string get_current_export_name() const;

	void add_decl(std::shared_ptr<AST::ADeclaration> declaration);
	void add_ref(AST::SYM_REF<AST::AReference> &reference);
	void enter_scope(const std::string& name, EScopeType ty, size_t depth = 0);
	void exit_scope();
	[[nodiscard]] std::vector<std::string> get_current_path() const;

	[[nodiscard]] std::optional<std::shared_ptr<AST::ADeclaration>> find_symbol(const std::string &full_name);

};
