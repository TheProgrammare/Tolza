#include "Parser_Memory.hpp"

#include "Parser_Context.hpp"

#include "Parser_Operator.hpp"
#include "Parser_Reference.hpp"
#include "Parser_Expression.hpp"
#include "Parser_Type.hpp"

#include "AST/AST_Base.hpp"
#include "AST/AST_Memory.hpp"
#include "AST/AST_Type.hpp"
#include "AST/AST_Operation.hpp"

std::optional<std::unique_ptr<AST::Node>> PAR::Parser_Memory::try_memory(bool is_silent_error) 
{
	switch (ctx.tok_v.peek().type) {
		case TokTy::TILDE:						return getbits();
		case TokTy::NEW:						return _new();
		case TokTy::DEL:						return del();
		case TokTy::CAPA_MOVE:					return move();
		case TokTy::VAL_OF:						return val_of_ptr();
		case TokTy::ADDR_OF:					return addr_of_ref();
		case TokTy::DROP:						return drop();
		default:	break;
	}

	if (!is_silent_error) {
		ctx.tok_v.add_error("PAR1545", "Expected literal value", "");
	}

	return std::nullopt;
}

std::unique_ptr<AST::Memory::GetBits> PAR::Parser_Memory::getbits() 
{
	static const std::string hint = "define get bit like: `target~[0..8]` get first octect on target.";

	ctx.tok_v.match(TokTy::TILDE);
	ctx.tok_v.expect(TokTy::OPEN_SQUARE, "PAR1055", "Expected start slice block '[' after a get bit operator '~'", hint);
	auto get_bit = ctx.Create_Node<AST::Memory::GetBits>(ctx.tok_v.peek(-2));
	get_bit->range = ctx.p_expr->parse_expression();
	ctx.tok_v.expect(TokTy::CLOSE_SQUARE, "PAR1056", "Expected end slice block ']' after range expression", hint);
	if (auto range_ptr = dynamic_cast<AST::AType*>(get_bit->range.get())) {
		get_bit->resolved_range_type = std::shared_ptr<AST::AType>(dynamic_cast<AST::AType*>(get_bit->range.release()));
	}
	return get_bit;
}

std::unique_ptr<AST::Memory::New> PAR::Parser_Memory::_new() 
{
	static const std::string hint =
		"define a dynamic memory allocation:"
		"\n  - primitive `var myPtr = new ptr'i32(10)`"
		"\n  - array `var myPtr = new ptr'[i32 -> 3]({ 1, 2, 3 })`"
		"\n  - array on all `var myPtr = new ptr'[i32 -> 3](0)`"
		"\n  - entity `var myPtr = new ptr'Person(Person{ CIdentity{ name: \"Zagreus\", age: 25 } })`";

	ctx.tok_v.match(TokTy::NEW);

	auto node = ctx.Create_Node<AST::Memory::New>(ctx.tok_v.peek());
	ctx.tok_v.expect_any(kPointerTokens, "PAR1065", "Expected pointer specification after 'new' token.", hint);
	node->pointer = TokTy_to_EPtrType(ctx.tok_v.peek(-1).type);

	ctx.tok_v.expect(TokTy::TICK, "PAR1066", "Expected tick ' between pointer and type", hint);

	node->type = ctx.p_type->parse_type();
	
	ctx.tok_v.expect(TokTy::OPEN_PAREN, "PAR1067", "Expected start value '(' after type", hint);
	node->expression = ctx.p_expr->parse_expression();
	ctx.tok_v.expect(TokTy::CLOSE_PAREN, "PAR1068", "Expected end value ')'", hint);

	return node;
}

std::unique_ptr<AST::Memory::Size> PAR::Parser_Memory::size() 
{
	static const std::string hint = "define value memory size like: `mem::size(a)`";
	auto node = ctx.Create_Node<AST::Memory::Size>(ctx.tok_v.peek(-1));

	ctx.tok_v.expect(TokTy::OPEN_PAREN, "PAR1790", "Expected start arg '('.", hint);
	node->target = ctx.p_ref->parse_reference();
	ctx.tok_v.expect(TokTy::CLOSE_PAREN, "PAR1791", "Expected end arg ')'.", hint);

	return node;
}

std::unique_ptr<AST::Memory::Align> PAR::Parser_Memory::align() 
{
	static const std::string hint = "define value memory alignment like: `mem::size(a)`";
	auto node = ctx.Create_Node<AST::Memory::Align>(ctx.tok_v.peek(-1));

	ctx.tok_v.expect(TokTy::OPEN_PAREN, "PAR1790", "Expected start arg '('.", hint);
	node->target = ctx.p_ref->parse_reference();
	ctx.tok_v.expect(TokTy::CLOSE_PAREN, "PAR1790", "Expected end arg ')'.", hint);

	return node;
}

std::unique_ptr<AST::Memory::Move> PAR::Parser_Memory::move() 
{
	ctx.tok_v.match(TokTy::CAPA_MOVE_OF);
	auto node = ctx.Create_Node<AST::Memory::Move>(ctx.tok_v.peek());
	node->target = ctx.p_ref->parse_reference();
	return node;
}

std::unique_ptr<AST::Memory::Del> PAR::Parser_Memory::del() 
{
	auto node = ctx.Create_Node<AST::Memory::Del>(ctx.tok_v.peek());
	ctx.tok_v.match(TokTy::DEL);
	node->target = ctx.p_ref->parse_reference();

	return node;
}

std::unique_ptr<AST::Memory::Val_Of_Ptr> PAR::Parser_Memory::val_of_ptr() 
{
	auto node = ctx.Create_Node<AST::Memory::Val_Of_Ptr>(ctx.tok_v.peek());
	ctx.tok_v.match(TokTy::VAL_OF);
	node->target = ctx.p_ref->parse_reference();

	return node;
}

std::unique_ptr<AST::Memory::Addr_Of_Ref> PAR::Parser_Memory::addr_of_ref() 
{
	auto node = ctx.Create_Node<AST::Memory::Addr_Of_Ref>(ctx.tok_v.peek());
	ctx.tok_v.match(TokTy::ADDR_OF);
	node->target = ctx.p_ref->parse_reference();

	return node;
}

std::unique_ptr<AST::Memory::Drop> PAR::Parser_Memory::drop()
{
	auto node = ctx.Create_Node<AST::Memory::Drop>(ctx.tok_v.peek());
	ctx.tok_v.match(TokTy::DROP);
	node->target = ctx.p_ref->parse_reference();

	return node;
}




