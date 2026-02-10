#include "Parser_Expression.hpp"

#include "Parser_Headers.hpp"
#include "AST/AST_Headers.hpp"

#include "Lexer/TokenViewer.hpp"

std::unique_ptr<AST::Node> PAR::Parser_Expression::parse_expression() {
	return ctx.p_op->try_operation();
}


std::unique_ptr<AST::Node> PAR::Parser_Expression::parse_expression_term() {
	if (ctx.tok_v.check(TokTy::IF))
		return ctx.p_state->ternary_if();

	std::unique_ptr<AST::Node> term;

	EExprPassMode pass_mode = TokTy_to_EExprPassMode(ctx.tok_v.peek().type);
	if (pass_mode != EExprPassMode::NONE) ctx.tok_v.next();

	if (ctx.tok_v.check(TokTy::OPEN_PAREN)) {
		term = parse_expression();
		ctx.tok_v.expect(TokTy::CLOSE_PAREN, "PAR1812", "Expected end of nested expression ')'", "");
	}

	if (auto lit = ctx.p_lit->try_literal(true)) {
		term = std::move(lit.value());
	}
	else if (auto mem = ctx.p_mem->try_memory(true)) {
		term = std::move(mem.value());
	}
	else if (auto ref = ctx.p_ref->parse_reference()){
		// get bit case
		if (ctx.tok_v.check(TokTy::TILDE)) {
			auto get_bits = ctx.p_mem->getbits(); 
			get_bits->target = std::move(ref);
			term = std::move(get_bits);
		}
		else {
			term = std::move(ref);
		}
	}

	if (!term) ctx.tok_v.add_error("PAR1811", "Unexpected '" + ctx.tok_v.peek().val + "' keyword.", "define a term with literal, identifier, ternary if, tuple, nested expression '()', literal array '{}' or nothing '_'.");

	// if cast
	if (auto as = try_cast_as()) {
		as.value()->valueCasted = std::move(term);
		term = std::move(as.value());
	}

	if (ctx.tok_v.check_any({ TokTy::RANGE, TokTy::RANGE_INCLUSIVE })) {
		auto lit_range = ctx.p_lit->literal_range(std::move(term));
		term = std::move(lit_range);
	}

	return term;
}

std::optional<std::unique_ptr<AST::Operation::Cast_As>> PAR::Parser_Expression::try_cast_as() {
	if (!ctx.tok_v.check_any(kCastType)) return std::nullopt; 

	auto asCast = ctx.Create_Node<AST::Operation::Cast_As>(ctx.tok_v.peek());

	switch (ctx.tok_v.peek().type) {
		case TokTy::AS: 
			asCast->cast_type = AST::Operation::Cast_As::ECastType::AS;
		case TokTy::AS_REINTERPRET: 
			asCast->cast_type = AST::Operation::Cast_As::ECastType::AS_REINTERPRET;
		case TokTy::AS_SAFE: 
			asCast->cast_type = AST::Operation::Cast_As::ECastType::AS_SAFE;
		default: break;
	}

	ctx.tok_v.next(); // consume as
	asCast->typeCasted = ctx.p_type->parse_type();

	return asCast;
}


