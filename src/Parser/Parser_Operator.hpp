
#pragma once

#include <memory>

#include "AST/AST_Forward.hpp"

enum class EBinOpType;
enum class EUnaryOpType;

namespace PAR {
	struct Parser_Context;
	struct Parser_Operator {
		Parser_Operator(Parser_Context& ctx) : ctx(ctx) {}

		bool no_literal_cop_mode = false;

        [[nodiscard]] std::unique_ptr<AST::AExpression> 						try_operation();
		[[nodiscard]] std::unique_ptr<AST::Operation::Assignment>		assignment(std::unique_ptr<AST::AExpression> left);
	private:
		[[nodiscard]] std::unique_ptr<AST::AExpression>						power();
		[[nodiscard]] std::unique_ptr<AST::AExpression>						multiply();
		[[nodiscard]] std::unique_ptr<AST::AExpression>						add();
		[[nodiscard]] std::unique_ptr<AST::AExpression>						shift();
		[[nodiscard]] std::unique_ptr<AST::AExpression>						comparison();
		[[nodiscard]] std::unique_ptr<AST::AExpression>						equality();
		[[nodiscard]] std::unique_ptr<AST::AExpression>						bitwise_not();
		[[nodiscard]] std::unique_ptr<AST::AExpression>						bitwise_and_nand();
		[[nodiscard]] std::unique_ptr<AST::AExpression>						bitwise_xor_xnor();
		[[nodiscard]] std::unique_ptr<AST::AExpression>						bitwise_or_nor();
		[[nodiscard]] std::unique_ptr<AST::AExpression>						logical_not();
		[[nodiscard]] std::unique_ptr<AST::AExpression>						logical_and_nand();
		[[nodiscard]] std::unique_ptr<AST::AExpression>						logicial_xor_xnor();
		[[nodiscard]] std::unique_ptr<AST::AExpression>						logicial_or_nor();
        [[nodiscard]] std::unique_ptr<AST::AExpression> 						memory_distance();



        [[nodiscard]] std::unique_ptr<AST::Operation::Binary>			Create_BinOp(std::unique_ptr<AST::AExpression> left, EBinOpType op, std::unique_ptr<AST::AExpression> right);
		[[nodiscard]] std::unique_ptr<AST::Operation::Unary>			Create_UnOp(EUnaryOpType op, std::unique_ptr<AST::AExpression> base);

        Parser_Context& ctx;
	};
}