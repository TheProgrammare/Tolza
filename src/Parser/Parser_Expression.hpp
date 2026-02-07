#pragma once

#include <memory>
#include <optional>

#include "AST/AST_Forward.hpp"

namespace PAR {
	struct Parser_Context;
	struct Parser_Expression {
		Parser_Expression(Parser_Context& ctx) : ctx(ctx) {}

        [[nodiscard]] std::unique_ptr<AST::Node> 							    parse_expression();
		[[nodiscard]] std::unique_ptr<AST::Node>							    parse_expression_term();

        [[nodiscard]] std::optional<std::unique_ptr<AST::Operation::Cast_As>> 	try_cast_as();

        Parser_Context& ctx;
	};
}