
#include "Parser_Operator.hpp"

#include "Parser_Headers.hpp"
#include "AST/AST_Headers.hpp"


// Parsing precedence hierarchy:
//
// primary
//   -> unary (+, -, NOT, B_NOT)
//     -> power (^)
//       -> multiply/divide/modulo (*, /, %, %/)
//         -> add/subtract (+, -)
//           -> bitwise shift (<<, >>)
//             -> comparison (<, >, <=, >=)
//               -> equality (==, !=)
//                 -> bitwise AND (B_AND)
//                   -> bitwise XOR (B_XOR)
//                     -> bitwise OR (B_OR)
//                       -> logical AND (AND, NAND)
//                         -> logical XOR (XOR)
//                           -> logical OR (OR, NOR)
//                             -> logical XNOR (XNOR)

std::unique_ptr<AST::Node> PAR::Parser_Operator::power() {
	auto left = ctx.p_expr->parse_expression_term();
	if (ctx.tok_v.match(TokTy::OP_POWER)) {
		auto right = power();
		return Create_BinOp(std::move(left), EBinOpType::Pow, std::move(right));
	}
	return left;
}

std::unique_ptr<AST::Node> PAR::Parser_Operator::multiply() {
	auto node = power();
	while (ctx.tok_v.match_any({ TokTy::OP_ASTERISK, TokTy::OP_DIVIDE, TokTy::OP_MODULO, TokTy::OP_QUOTIEN, TokTy::OP_REMAIN })) {
		auto op = TokTy_to_EBinOpType(ctx.tok_v.peek(-1).type);
		auto right = power();
		node = Create_BinOp(std::move(node), op, std::move(right));
	}
	return node;
}

std::unique_ptr<AST::Node> PAR::Parser_Operator::add() {
	auto node = multiply();
	while (ctx.tok_v.match_any({ TokTy::OP_PLUS, TokTy::OP_MINUS })) {
		auto op = TokTy_to_EBinOpType(ctx.tok_v.peek(-1).type);
		auto right = multiply();
		node = Create_BinOp(std::move(node), op, std::move(right));
	}
	return node;
}

std::unique_ptr<AST::Node> PAR::Parser_Operator::shift() {
	auto node = add();
	while (ctx.tok_v.match_any(kBitwiseTokens)) {
		auto op = TokTy_to_EBinOpType(ctx.tok_v.peek(-1).type);
		auto right = add();
		node = Create_BinOp(std::move(node), op, std::move(right));
	}
	return node;
}

std::unique_ptr<AST::Node> PAR::Parser_Operator::comparison() {
	auto node = shift();
	while (ctx.tok_v.match_any({ TokTy::OPEN_BRACKETS, TokTy::CLOSE_BRACKETS, TokTy::OP_LOWER_EQUAL, TokTy::OP_GREATER_EQUAL })) {
		auto op = TokTy_to_EBinOpType(ctx.tok_v.peek(-1).type);
		auto right = shift();
		node = Create_BinOp(std::move(node), op, std::move(right));
	}
	return node;
}

std::unique_ptr<AST::Node> PAR::Parser_Operator::equality() {
	auto node = comparison();
	while (ctx.tok_v.match_any({ TokTy::OP_EQUAL, TokTy::OP_NOT_EQUAL, TokTy::IN, TokTy::IS })) {
		auto op = TokTy_to_EBinOpType(ctx.tok_v.peek(-1).type);
		auto right = comparison();
		node = Create_BinOp(std::move(node), op, std::move(right));
	}
	return node;
}

std::unique_ptr<AST::Node> PAR::Parser_Operator::bitwise_not() {
	if (ctx.tok_v.match(TokTy::B_NOT)) {
		auto op = TokTy_to_EUnaryOpType(ctx.tok_v.peek(-1).type);
		auto operand = bitwise_not();
		return Create_UnOp(op, std::move(operand));
	}
	return equality();
}

std::unique_ptr<AST::Node> PAR::Parser_Operator::bitwise_and_nand() {
	auto node = bitwise_not();
	while (ctx.tok_v.match_any({ TokTy::B_AND, TokTy::B_NAND })) {
		auto op = TokTy_to_EBinOpType(ctx.tok_v.peek(-1).type);
		auto right = bitwise_not();
		node = Create_BinOp(std::move(node), op, std::move(right));
	}
	return node;
}

std::unique_ptr<AST::Node> PAR::Parser_Operator::bitwise_xor_xnor() {
	auto node = bitwise_and_nand();
	while (ctx.tok_v.match_any({ TokTy::B_XOR, TokTy::B_XNOR })) {
		auto op = TokTy_to_EBinOpType(ctx.tok_v.peek(-1).type);
		auto right = bitwise_and_nand();
		node = Create_BinOp(std::move(node), op, std::move(right));
	}
	return node;
}

std::unique_ptr<AST::Node> PAR::Parser_Operator::bitwise_or_nor() {
	auto node = bitwise_xor_xnor();
	while (ctx.tok_v.match_any({ TokTy::B_OR, TokTy::B_NOR })) {
		auto op = TokTy_to_EBinOpType(ctx.tok_v.peek(-1).type);
		auto right = bitwise_xor_xnor();
		node = Create_BinOp(std::move(node), op, std::move(right));
	}
	return node;
}

std::unique_ptr<AST::Node> PAR::Parser_Operator::logical_not() {
	if (ctx.tok_v.match(TokTy::NOT)) {
		auto op = TokTy_to_EUnaryOpType(ctx.tok_v.peek(-1).type);
		auto operand = logical_not();
		return Create_UnOp(op, std::move(operand));
	}
	return bitwise_or_nor();
}

std::unique_ptr<AST::Node> PAR::Parser_Operator::logical_and_nand() {
	auto node = logical_not();
	while (ctx.tok_v.match_any({ TokTy::AND, TokTy::NAND })) {
		auto op = TokTy_to_EBinOpType(ctx.tok_v.peek(-1).type);
		auto right = logical_not();
		node = Create_BinOp(std::move(node), op, std::move(right));
	}
	return node;
}

std::unique_ptr<AST::Node> PAR::Parser_Operator::logicial_xor_xnor() {
	auto node = logical_and_nand();
	while (ctx.tok_v.match_any({ TokTy::XOR, TokTy::XNOR })) {
		auto op = TokTy_to_EBinOpType(ctx.tok_v.peek(-1).type);
		auto right = logical_and_nand();
		node = Create_BinOp(std::move(node), op, std::move(right));
	}
	return node;
}

std::unique_ptr<AST::Node> PAR::Parser_Operator::logicial_or_nor() {
	auto node = logicial_xor_xnor();
	while (ctx.tok_v.check_any({ TokTy::OR, TokTy::NOR })) {
		auto op = TokTy_to_EBinOpType(ctx.tok_v.next().type);
		auto right = logicial_xor_xnor();
		node = Create_BinOp(std::move(node), op, std::move(right));
	}
	return node;
}

std::unique_ptr<AST::Node> PAR::Parser_Operator::memory_distance() {
	auto node = logicial_xor_xnor();
	while (ctx.tok_v.check(TokTy::MEM_DIST)) {
		auto dist = ctx.Create_Node<AST::Memory::Dist>(ctx.tok_v.peek());
		auto right = logicial_xor_xnor();
		dist->left = std::move(node);
		dist->right = std::move(right);
		node = std::move(dist);
	}
	return node;
}

std::unique_ptr<AST::Operation::Assignment> PAR::Parser_Operator::assignment(std::unique_ptr<AST::AReference> left)
{
	auto assign_tok = ctx.tok_v.expect_any(kAssignationTokens, "PAR1249", "Expected assignation token.", "");

	auto assign = ctx.Create_Node<AST::Operation::Assignment>(assign_tok);
	assign->left = std::move(left);
	assign->assignmentType = TokTy_to_EAssignmentType(assign_tok.type);
	assign->right = ctx.p_expr->parse_expression();
	return assign;
}

std::unique_ptr<AST::Node> PAR::Parser_Operator::try_operation() {
	return memory_distance();
}

std::unique_ptr<AST::Operation::Binary> PAR::Parser_Operator::Create_BinOp(std::unique_ptr<AST::Node> left, EBinOpType op, std::unique_ptr<AST::Node> right) {
	auto node = ctx.Create_Node<AST::Operation::Binary>(ctx.tok_v.peek());
	node->left = std::move(left);
	node->op = op;
	node->right = std::move(right);
	return node;
}

std::unique_ptr<AST::Operation::Unary> PAR::Parser_Operator::Create_UnOp(EUnaryOpType op, std::unique_ptr<AST::Node> base) {
	auto node = ctx.Create_Node<AST::Operation::Unary>(ctx.tok_v.peek());
	node->unitaryOp = op;
	node->base = std::move(base);
	return node;
}




