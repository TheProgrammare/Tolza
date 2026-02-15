#include "Parser_Memory.hpp"

#include "Parser_Context.hpp"

#include "Parser_Operator.hpp"
#include "Parser_Expression.hpp"
#include "Parser_Type.hpp"

#include "AST/AST_Memory.hpp"


std::unique_ptr<AST::Memory::Align> PAR::Parser_Memory::align() 
{
	static const std::string hint = "define value memory alignment like: `align(a)`";
	auto node = ctx.Create_Node<AST::Memory::Align>(ctx.tok_v.peek(-1));

	ctx.tok_v.expect<106>(TokTy::OPEN_PAREN, "Expected start arg '('.", hint);
	node->target = ctx.p_expr->parse_expression();
	ctx.tok_v.expect<107>(TokTy::CLOSE_PAREN, "Expected end arg ')'.", hint);

	return node;
}


std::unique_ptr<AST::Memory::Del> PAR::Parser_Memory::del() 
{
	auto node = ctx.Create_Node<AST::Memory::Del>(ctx.tok_v.peek());
	ctx.tok_v.match(TokTy::DEL);
	node->target = ctx.p_expr->parse_expression();

	return node;
}


std::unique_ptr<AST::Memory::Drop> PAR::Parser_Memory::drop()
{
	auto node = ctx.Create_Node<AST::Memory::Drop>(ctx.tok_v.peek());
	ctx.tok_v.match(TokTy::DROP);
	node->target = ctx.p_expr->parse_expression();

	return node;
}





