#pragma once

#include <string>
#include <vector>
#include <set>
#include <memory>

#include "AST/AST_Base.hpp"
#include "Lexer/Token.hpp"
#include "AST/AST_Forward.hpp"

struct ScriptInfo;

struct Extern_Item {
	enum class Kind {
		Function,
		Type,
		Global,
		Enum,
		Union,
		Component,
		System,
		Entity,
		Generic,
		Metacode,
	};
	Kind kind;
	AST::ID id;

	Extern_Item(const AST::ID &_id, Kind _kind)
		: id(_id)
		, kind(_kind)
	{}
};

Extern_Item::Kind AST_AReference_to_Extern_Item_Kind(const AST::AReference &n);

struct Symbols_Manager;

namespace META {
	struct MetablockManager;
} // namespace META


struct ModuleImportation {
	enum class EImportSource { Undefined, User, StandardLib };

	std::string name;
	// if from another language or lib
	[[maybe_unused]]
	std::string extern_lib;

	EImportSource import_source = EImportSource::Undefined;

	// is from another language or lib
	bool is_std = false;

	std::vector<ScriptInfo*> target_modules;

	std::vector<Extern_Item> extern_references;

	bool is_external() const { return !extern_lib.empty(); }

	void add_extern_reference(const Extern_Item &ext_item) {
		extern_references.push_back(ext_item);
	}
};

struct ModuleExportation {
	std::string name;

	// export to external (for other language for conversion)
	[[maybe_unused]]
	std::string extern_lib;

	// no module namespace need for usage
	bool is_native = false;

	ScriptInfo *target_module = nullptr;

	[[maybe_unused]]
	ScriptInfo *script_mirror = nullptr;

	bool is_mirror = false;

	[[maybe_unused]]
	ModuleImportation *mirror = nullptr;

	bool is_external() const { return !extern_lib.empty(); }
};

struct ScriptInfo {
	ScriptInfo(const std::string& _name) :
		name(_name) {
	}

	std::string name;
	std::string file_path;
	std::vector<std::string> src_lines;
	std::vector<Token> tokens;
	META::MetablockManager* m_meta = nullptr;

	// names of modules exported
	std::vector<std::shared_ptr<ModuleExportation>> exported_mod;
	// names of modules imported
	std::vector<std::shared_ptr<ModuleImportation>> imported_mod;

	AST::Root* rootNode = nullptr;

	std::vector<std::string> semantic_resolveType_errors;
	std::vector<std::string> semantic_checkType_errors;

	Symbols_Manager* m_sym = nullptr;


	void add_export(const ModuleExportation &exp);
	void add_import(const ModuleImportation &imp);

	ModuleExportation *get_export_module(const std::string &name);
	ModuleImportation *get_import_module(const std::string &name);

	std::set<ModuleImportation*> get_externs();

	std::set<std::string> get_extern_languages();

	// No copy
	ScriptInfo(const ScriptInfo&) = delete;
	ScriptInfo& operator=(const ScriptInfo&) = delete;

	// can move
	ScriptInfo(ScriptInfo&&) = default;
	ScriptInfo& operator=(ScriptInfo&&) = default;
};




