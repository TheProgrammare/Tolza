#pragma once

#include <memory>

#include "AST/AST_Forward.hpp"

namespace PAR {
	struct Parser_Context;
	struct Parser_Memory {
		Parser_Memory(Parser_Context& ctx) : ctx(ctx) {}

		// special memory expression
		[[nodiscard]] std::unique_ptr<AST::Memory::GetBits>				getbits();
		[[nodiscard]] std::unique_ptr<AST::Memory::New>					_new();
		[[nodiscard]] std::unique_ptr<AST::Memory::Del>					del();
        [[nodiscard]] std::unique_ptr<AST::Memory::Val_Of_Ptr> 			val_of_ptr();
        [[nodiscard]] std::unique_ptr<AST::Memory::Addr_Of_Ref> 		addr_of_ref();
		[[nodiscard]] std::unique_ptr<AST::Memory::Size>				size();
		[[nodiscard]] std::unique_ptr<AST::Memory::Align>				align();
		[[nodiscard]] std::unique_ptr<AST::Memory::Move>            	move();

		Parser_Context& ctx;
	};
}