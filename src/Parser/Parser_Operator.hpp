
#pragma once

#include "AST/AST_Forward.hpp"
#include "AST/AST_Base.hpp"

namespace PAR {
	struct Parser_Context;
	struct Parser_Operator {
		Parser_Operator(Parser_Context& ctx) : ctx(ctx) {}

		bool no_literal_ecs_mode = false;

        [[nodiscard]] std::unique_ptr<AST::Node> 						try_operation();
		[[nodiscard]] std::unique_ptr<AST::Operation::Assignment>		assignment(std::unique_ptr<AST::AReference> left);
	private:
		[[nodiscard]] std::unique_ptr<AST::Node>						power();
		[[nodiscard]] std::unique_ptr<AST::Node>						multiply();
		[[nodiscard]] std::unique_ptr<AST::Node>						add();
		[[nodiscard]] std::unique_ptr<AST::Node>						shift();
		[[nodiscard]] std::unique_ptr<AST::Node>						comparison();
		[[nodiscard]] std::unique_ptr<AST::Node>						equality();
		[[nodiscard]] std::unique_ptr<AST::Node>						bitwise_not();
		[[nodiscard]] std::unique_ptr<AST::Node>						bitwise_and_nand();
		[[nodiscard]] std::unique_ptr<AST::Node>						bitwise_xor_xnor();
		[[nodiscard]] std::unique_ptr<AST::Node>						bitwise_or_nor();
		[[nodiscard]] std::unique_ptr<AST::Node>						logical_not();
		[[nodiscard]] std::unique_ptr<AST::Node>						logical_and_nand();
		[[nodiscard]] std::unique_ptr<AST::Node>						logicial_xor_xnor();
		[[nodiscard]] std::unique_ptr<AST::Node>						logicial_or_nor();
        [[nodiscard]] std::unique_ptr<AST::Node> 						memory_distance();



        [[nodiscard]] std::unique_ptr<AST::Operation::Binary>			Create_BinOp(std::unique_ptr<AST::Node> left, EBinOpType op, std::unique_ptr<AST::Node> right);
		[[nodiscard]] std::unique_ptr<AST::Operation::Unary>			Create_UnOp(EUnaryOpType op, std::unique_ptr<AST::Node> base);

        Parser_Context& ctx;
	};
}