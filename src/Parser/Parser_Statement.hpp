
#pragma once

#include <memory>

#include "AST/AST_Forward.hpp"

namespace PAR {
	struct Parser_Context;
    struct Parser_Statement {
        Parser_Statement(Parser_Context& ctx) : ctx(ctx) {}
		
		// decl_possible means parse_instruction can failed without error to try to parse a declaration after
		[[nodiscard]] std::unique_ptr<AST::Node>						parse_statement(bool is_silent_error = false);
		[[nodiscard]] std::unique_ptr<AST::Statement::If>				if_statement();
		[[nodiscard]] std::unique_ptr<AST::Statement::For>				for_statement();
		[[nodiscard]] std::unique_ptr<AST::Statement::While>			while_statement();
		[[nodiscard]] std::unique_ptr<AST::Statement::Loop>				loop_statement();
		[[nodiscard]] std::unique_ptr<AST::Statement::Match>			match_statement();
		[[nodiscard]] std::unique_ptr<AST::Statement::GoTo>				goto_statement();
        [[nodiscard]] std::unique_ptr<AST::Statement::If_Ternary> 		ternary_if();
        [[nodiscard]] std::unique_ptr<AST::Statement::Return> 			return_flow();

        Parser_Context& ctx;
    };
}