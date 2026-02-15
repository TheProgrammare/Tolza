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
	std::string name;
	std::vector<std::string> path;

	Extern_Item(const std::string &_name, const std::vector<std::string> &_path, Kind _kind)
		: name(_name)
		, path(_path)
		, kind(_kind)
	{}
};

Extern_Item::Kind AST_AExpression_to_Extern_Item_Kind(const AST::AExpression &n);

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
	ScriptInfo(const std::string& _name, 
		const std::string &_file_path, 
		const std::string &_file_str,
		const std::vector<std::string> &_file_lines) 
		: name(_name)
		, file_path(_file_path)
		, file_str(_file_str)
		, file_lines(_file_lines) 
	{}

	std::string name;
	std::string file_path;
	std::string file_str;
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
	
	// start at 1
	std::string get_line(size_t line) const {
		if (line - 1 > file_lines.size() || line - 1 < 0) return "LINE OVER MAX SIZE";
		return file_lines[line - 1];
	}

	size_t get_line_size() const { return file_lines.size(); }
	
	// No copy
	ScriptInfo(const ScriptInfo&) = delete;
	ScriptInfo& operator=(const ScriptInfo&) = delete;
	
	// can move
	ScriptInfo(ScriptInfo&&) = default;
	ScriptInfo& operator=(ScriptInfo&&) = default;
	
private:
	std::vector<std::string> file_lines;
};




