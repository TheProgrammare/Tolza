#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "AST/AST_Forward.hpp"
#include "ScriptInfo.hpp"

struct ModuleImportation;

namespace PAR {
	struct Parser_Context;
	struct Parser_Reference {
		Parser_Reference(Parser_Context& ctx) : ctx(ctx) {}

		[[nodiscard]] std::unique_ptr<AST::AReference>									parse_reference();

		void 																			check_reference_external(const AST::ID &_id, Extern_Item::Kind kind);


		[[nodiscard]] AST::ID															identifier(bool no_qualified_id = false, bool keyword_allowed = false);
		[[nodiscard]] std::optional<std::unique_ptr<AST::AType_Reference>>				try_identifier_typed(AST::ID &id);
        [[nodiscard]] std::optional<std::unique_ptr<AST::Reference::Member_Access>>		try_member_access(AST::ID &id);
        [[nodiscard]] std::optional<std::unique_ptr<AST::Reference::Table_Access>> 		try_table_access();
        [[nodiscard]] std::optional<std::unique_ptr<AST::Reference::Call>> 				try_function_call(const AST::ID &id);
        [[nodiscard]] std::optional<std::unique_ptr<AST::Reference::Call_Pipe>>			try_function_call_pipe(const AST::ID& id);
		[[nodiscard]] std::vector<std::unique_ptr<AST::Reference::Call_Argument>>		call_arguments();

		[[nodiscard]] ModuleImportation*												get_external_source(AST::ID &id);

		Parser_Context& ctx;
		bool lit_comp_entity_allowed = true;
	};
}