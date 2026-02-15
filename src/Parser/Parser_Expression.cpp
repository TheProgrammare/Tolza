#include "Parser_Expression.hpp"

#include "AST/AST_Base.hpp"
#include "AST/AST_Expression.hpp"
#include "Parser_Headers.hpp"
#include "AST/AST_Headers.hpp"

#include "Lexer/TokenViewer.hpp"
#include <algorithm>
#include <memory>
#include <stdexcept>

std::unique_ptr<AST::AExpression> PAR::Parser_Expression::parse_expression() {
	return ctx.p_op->try_operation();
}


std::unique_ptr<AST::AExpression> PAR::Parser_Expression::parse_expression_term() {
	if (ctx.tok_v.check(TokTy::IF))
		return if_ternary();
	if (ctx.tok_v.check(TokTy::VAL_OF))
		return ptr_val();
	if (ctx.tok_v.check(TokTy::ADDR_OF))
		return addr_of();
	if (ctx.tok_v.check(TokTy::SIZE_OF))
		return size_of();

	std::unique_ptr<AST::AExpression> term;

	EExprPassMode pass_mode = TokTy_to_EExprPassMode(ctx.tok_v.peek().type);
	if (pass_mode != EExprPassMode::NONE) ctx.tok_v.next();

	if (ctx.tok_v.check(TokTy::OPEN_PAREN)) {
		term = parse_expression();
		ctx.tok_v.expect<78>(TokTy::CLOSE_PAREN, "Expected end of nested expression ')'", "");
	}

	if (auto lit = ctx.p_lit->try_literal(true)) {
		term = std::move(lit.value());
	}
	else if (auto expr = parse_expression()) {
		// get bit case
		if (ctx.tok_v.check(TokTy::TILDE))
			term = getbits(std::move(expr)); 
		else if (ctx.tok_v.check(TokTy::PTR_AT))
			term = ptr_at(std::move(expr));
		else if (ctx.tok_v.match(TokTy::PTR_OFFSET))
			term = ptr_offset(std::move(expr));
		else
			term = std::move(expr);
	}

	if (!term) ctx.tok_v.add_error<79>("Unexpected '" + ctx.tok_v.peek().val + "' keyword.", "define a term with literal, identifier, ternary if, tuple, nested expression '()', literal array '{}' or nothing '_'.");

	if (ctx.tok_v.check_any({ TokTy::RANGE, TokTy::RANGE_INCLUSIVE })) {
		term = ctx.p_lit->literal_range(std::move(term));
	}

	// if cast
	if (ctx.tok_v.check_any(kCastType)) {
		term = cast_as(std::move(term));
	}

	return term;
}

std::unique_ptr<AST::Operation::Cast_As> PAR::Parser_Expression::cast_as(std::unique_ptr<AST::AExpression> expr) {
	auto asCast = ctx.Create_Node<AST::Operation::Cast_As>(ctx.tok_v.peek());

	switch (ctx.tok_v.peek().type) {
		case TokTy::AS: 
			asCast->cast_type = AST::Operation::Cast_As::ECastType::AS;
		case TokTy::AS_REINTERPRET: 
			asCast->cast_type = AST::Operation::Cast_As::ECastType::AS_REINTERPRET;
		case TokTy::AS_SAFE: 
			asCast->cast_type = AST::Operation::Cast_As::ECastType::AS_SAFE;
		default: 
			ctx.tok_v.add_error<1000>("Unexpected token encounted in casting", "");
	}

	ctx.tok_v.next(); // consume as
	asCast->valueCasted = std::move(expr);
	asCast->typeCasted = ctx.p_type->parse_type();

	return asCast;
}



std::vector<std::unique_ptr<AST::Expression::Call_Argument>> PAR::Parser_Expression::call_arguments() {
	static const std::string hint =
		"define call argument like:\n"
		"  - ordinal `call(10)`"
		"  - named `call(param_name: 10)`"
		"  - variadic `call(... 10, 20, 30)`";

	if (ctx.tok_v.match(TokTy::CLOSE_PAREN)) return {};

	std::vector<std::unique_ptr<AST::Expression::Call_Argument>> params;

	while (!ctx.tok_v.is_end()) {
		auto param = ctx.Create_Node<AST::Expression::Call_Argument>(ctx.tok_v.peek());

		// if parameter invocation
		if (ctx.tok_v.check(TokTy::IDENTIFIER) && ctx.tok_v.peek(1).type == TokTy::ASSIGN) {
			param->name = ctx.tok_v.next().val;

			ctx.tok_v.expect<111>(TokTy::COLON,
				"Expected parameter assignation ':' after a parameter argument name invocation.",
				hint);

			param->expression = ctx.p_expr->parse_expression();
			params.push_back(std::move(param));
		}
		// ordered parameter affectation
		else {
			param->expression = ctx.p_expr->parse_expression();

			params.push_back(std::move(param));
		}

		// stop when an unexpected token is encounted 
		// permit to avoid ; for pipe calls
		// args are always separated by comma or ...
		// so other token indicates a terminaison
		if (ctx.tok_v.match_any({ TokTy::COMMA, TokTy::VARIADIC })) continue;
		ctx.tok_v.match(TokTy::CLOSE_PAREN);
		break;
	}

	return params;
}


std::optional<std::unique_ptr<AST::Expression::Call>> PAR::Parser_Expression::try_function_call(const AST::ID& id) {
	ctx.tok_v.match(TokTy::OPEN_PAREN);

	auto call = ctx.Create_Node<AST::Expression::Call>(ctx.tok_v.peek());
	call->name = id;

	call->param_args = call_arguments();

	return call;
}

std::optional<std::unique_ptr<AST::Expression::Call_Pipe>> PAR::Parser_Expression::try_function_call_pipe(const AST::ID& id) {
	if (!ctx.tok_v.match_any({ TokTy::PIPE, TokTy::PIPE_MUT })) return std::nullopt;

	auto pipe_call = ctx.Create_Node<AST::Expression::Call_Pipe>(ctx.tok_v.peek());
	pipe_call->isMutable = ctx.tok_v.peek(-1).type == TokTy::PIPE_MUT;
	pipe_call->id = id;
/*
	static const std::string hint = "define pipe-call like:"
		"\n  - pure pipe-call `var result = myFunction | param1 | param2 | param3;`"
		"\n  - pure pipe-call generic `var result = myFunction<gen_args> | <gen_args> param1 | <gen_args> param2 | <gen_args> param3;`"
		"\n  - mutable pipe-call `var result = myFunction <-| param1 |+ param2 |- param3;`"
		"\n  - mutable pipe-call generic `var result = myFunction<gen_args> <-| <gen_args> param1 |+ <gen_args> param2 |- <gen_args> param3;`";


	while (!ctx.tok_v.is_end()) {
		pipe_call->arguments.push_back(Create_Node<AST::Expression::Call_Argument>(ctx.tok_v.peek()));


		if (pipe_call->isMutable) {
			auto op = ctx.tok_v.expect_any(kOperatorTokens, "PAR1975", "Expected an operator keyword after pipe operation in mutable pipe call.", hint);

			arg-> = TokTy_to_EOpType(op.type);
		}
		else {
			std::get<>(elem) = EOpType::COUNT;
		}

		if (ctx.tok_v.match(TokTy::OPEN_BRACKETS)) {
			auto gen_arg = make_genArgs();
			std::get<1>(elem) = std::move(gen_arg);
		}

		auto params = _call_args();
		std::get<2>(elem) = std::move(params);

		result.push_back(std::move(elem));
		if (ctx.tok_v.match(TokTy::PIPE)) continue;
		break;
	}

	return result;

	for (auto& [pipe_op, pipe_gen, pipe_args] : result) {
		if (pipe_op != EOpType::COUNT) pipe_call->mutableOperators.push_back(pipe_op);
		if (pipe_gen) pipe_call->gen_args.push_back(std::move(pipe_gen));
		pipe_call->arguments.push_back(std::move(pipe_args));
	}
		*/
	return pipe_call;
}


ModuleImportation *PAR::Parser_Expression::get_external_source(const std::string &name, const std::vector<std::string> &path)
{
	if (id.path.empty()) return nullptr;
	const std::string &base = id.path[0];
	return ctx.scr_info.get_import_module(base);
}

std::unique_ptr<AST::AExpression> PAR::Parser_Expression::parse_Expression() 
{
	auto id = identifier();
	std::unique_ptr<AST::AExpression> target_ref;
	std::vector<std::unique_ptr<AST::AType>> gen_args;

	if (ctx.tok_v.check(TokTy::TURBO_FISH)) {
		auto id_typed = identifier_typed(id);

		// only identifier typed
		// no value accessible
		if (!ctx.tok_v.check_any({TokTy::OPEN_PAREN, TokTy::OPEN_BRACE})) {
			check_Expression_external(id, Extern_Item::Kind::Global);

			return std::unique_ptr<AST::AExpression>(id_typed.value().release());
		}
		gen_args = std::move(id_typed.value()->gen_args);

		check_Expression_external(id, Extern_Item::Kind::Type);
	}

	// call e.g. add(a, b)
	if (ctx.tok_v.check(TokTy::OPEN_PAREN)) {
		auto call = function_call(id)
		call.value()->gen_args = std::move(gen_args);
		target_ref = std::move(call.value());

		check_Expression_external(id, Extern_Item::Kind::Function);
	}
	// literal entity / component
	else if (ctx.tok_v.match(TokTy::OPEN_BRACKETS) && lit_comp_entity_allowed) {
		Token base_tok = ctx.tok_v.peek();
		auto id = identifier();

		// it's a literal component (field access dot)
		if (ctx.tok_v.match(TokTy::DOT)) {
			check_Expression_external(id, Extern_Item::Kind::Component);

			return ctx.p_lit->literal_component(id, gen_args);
		}
		else {
			// reset the moving to correctly parse the literal entity
			ctx.tok_v.jump(base_tok.span.pos);

			check_Expression_external(id, Extern_Item::Kind::Entity);

			return ctx.p_lit->literal_entity(id, gen_args);
		}
	}
	else {
		auto id_ref = ctx.Create_Node<AST::AExpression>(ctx.tok_v.peek());
		id_ref->id = id;
		target_ref = std::move(id_ref);
	}

	if (!gen_args.empty()) {
		
	}

	return target_ref;
}

std::optional<std::unique_ptr<AST::AExpression>> PAR::Parser_Expression::try_Expression_suffix_operation() 
{
	if (auto table_access = try_table_access()) {
		table_access.value()->id = id;
		target_ref = std::move(table_access.value());
	}
	if (auto member_access = try_member_access(id)) {
		member_access.value()->left = std::move(target_ref);
		target_ref = std::move(member_access.value());
	}
}

void PAR::Parser_Expression::check_Expression_external(const AST::ID &_id, Extern_Item::Kind _kind) 
{
	if (!_id.is_qualified_id()) return;

	if (auto imp_mod = ctx.scr_info.get_import_module(_id.path[0])) {
		Extern_Item ext(_id, _kind);
		imp_mod->add_extern_Expression(ext);
	}
}

std::unique_ptr<AST::Expression::If_Ternary> PAR::Parser_Expression::if_ternary() {
	auto ternary = ctx.Create_Node<AST::Statement::If_Ternary>(ctx.tok_v.peek());

	ternary->evaluator = ctx.p_loc->parse_evaluator(nullptr);
	ternary->true_line = ctx.p_loc->code_block_instruction();

	if (ctx.tok_v.match(TokTy::ELSE)) {
		ternary->false_line = ctx.p_loc->code_block_instruction();
	}

	return ternary;
}
// NOTE MOI : terminer avec nouvelle logique
std::unique_ptr<AST::AExpression> PAR::Parser_Expression::identifier(bool no_qualified_id, bool keyword_allowed)
{
    static const std::string hint =
		"define identifier like:"
		"\n  - classic `name` -> name"
		"\n  - with scope path `mod A { name }` -> A_name"
		"\n  - with qualified id `A::B::C` -> A_B_C";
		//"\n  - if exported : `# export module A\n # scope\nname\n# end\n -> A_name`;

	bool qualification_at_root_scope = false;
	bool qualification_at_parent_scope = false;
	bool qualification_at_current_scope = false;

	if (ctx.tok_v.match(TokTy::STATIC_ACCESS)) {
		id.qualification_at_root_scope = true;
	}
	else if (ctx.tok_v.match(TokTy::SUPER_MOD)) {
		id.qualification_at_parent_scope = true;
	}
	else if (ctx.tok_v.match(TokTy::SELF_MOD)) {
		id.qualification_at_current_scope = true;
	}
	// it's a simple id with no path
	else if (ctx.tok_v.peek(1).type != TokTy::STATIC_ACCESS) {
		if (!keyword_allowed)
			id.name =  ctx.tok_v.expect_id<112>("Expected identifier name.", hint);
		else 
			id.name = ctx.tok_v.next().val;

		return id;
	}

	// it's qualified id
	if (no_qualified_id) 
		ctx.tok_v.add_error_tok<113>(ctx.tok_v.peek(-1), "Unexpected qualified id.", hint);

	size_t count = 0;
	while (!ctx.tok_v.is_end()) {
		id.path.push_back(ctx.tok_v.next().val);

		ctx.tok_v.match(TokTy::STATIC_ACCESS);

		// no more path : the last element is the name
		if (ctx.tok_v.peek(1).type != TokTy::STATIC_ACCESS) {
			id.name = ctx.tok_v.next().val;
			break;
		}

		count++;
		if (count > 12) {
			ctx.tok_v.add_error<114>("Explicit path for identifier is too long (> " + std::to_string(12) + ")", hint);
			break;
		}
	}
	
	return id;
}

std::optional<std::unique_ptr<AST::AType_Expression>> PAR::Parser_Expression::try_identifier_typed(const std::string &name, const std::vector<std::string> &path)
{
	if (!ctx.tok_v.match(TokTy::TURBO_FISH)) return std::nullopt;

	auto id_type = ctx.Create_Node<AST::AType_Expression>(ctx.tok_v.peek(-2)); 
	id_type->id = id;

	if (ctx.tok_v.match(TokTy::CLOSE_BRACKETS)) return id_type;

	while (!ctx.tok_v.is_end()) {
		id_type->gen_args.push_back(ctx.p_type->parse_type());

		if (ctx.match_field_separator(TokTy::COMMA, TokTy::CLOSE_BRACKETS)) break;
	}

	return id_type;
}

std::optional<std::unique_ptr<AST::Expression::Member_Access>> PAR::Parser_Expression::try_member_access(const std::string &name, const std::vector<std::string> &path) {
	if (!ctx.tok_v.match(TokTy::DOT)) return std::nullopt;

	auto access = ctx.Create_Node<AST::Expression::Member_Access>(ctx.tok_v.peek(-2));

	access->right = parse_Expression();

	return access;
}

std::optional<std::unique_ptr<AST::Expression::Table_Access>> PAR::Parser_Expression::try_table_access() {
	if (!ctx.tok_v.match(TokTy::OPEN_SQUARE)) return std::nullopt;

	auto table_access = ctx.Create_Node<AST::Expression::Table_Access>(ctx.tok_v.peek(-1));
	table_access->selector = ctx.p_expr->parse_expression();

	ctx.tok_v.expect<115>(TokTy::CLOSE_SQUARE, "Expected end of table access '[' after expression.", "");

	return table_access;
}


std::unique_ptr<AST::Expression::Ptr_At> PAR::Parser_Expression::ptr_at(std::unique_ptr<AST::AExpression> expr) 
{
	ctx.tok_v.match(TokTy::PTR_AT);

	auto ptr_at = ctx.Create_Node<AST::Expression::Ptr_At>(ctx.tok_v.peek());
	ptr_at->target = std::move(expr);
	ptr_at->index = ctx.p_expr->parse_expression();
	ctx.tok_v.expect<108>(TokTy::CLOSE_PAREN, "Expected end of pointer at ')'.", "define pointer at like: `my_ptr'at(i)`.");

	return ptr_at;
}


std::unique_ptr<AST::Expression::Ptr_Offset> PAR::Parser_Expression::ptr_offset(std::unique_ptr<AST::AExpression> expr)
{
	ctx.tok_v.match(TokTy::PTR_OFFSET);

	auto ptr_offset = ctx.Create_Node<AST::Expression::Ptr_Offset>(ctx.tok_v.peek());
	ptr_offset->target = std::move(expr);
	ptr_offset->offset = ctx.p_expr->parse_expression();
	ctx.tok_v.expect<109>(TokTy::CLOSE_PAREN, "Expected end of pointer offset ')'.", "define pointer offset like: `my_ptr'offset(i)`.");

	return ptr_offset;
}



std::unique_ptr<AST::Expression::Ptr_Val> PAR::Parser_Expression::ptr_val() 
{
	auto node = ctx.Create_Node<AST::Expression::Val_Of_Ptr>(ctx.tok_v.peek());
	ctx.tok_v.match(TokTy::VAL_OF);
	node->target = ctx.p_ref->parse_reference();

	return node;
}

std::unique_ptr<AST::Expression::Addr_Of> PAR::Parser_Expression::addr_of() 
{
	auto node = ctx.Create_Node<AST::Expression::Addr_Of>(ctx.tok_v.peek());
	ctx.tok_v.match(TokTy::ADDR_OF);
	node->target = ctx.p_ref->parse_reference();

	return node;
}


std::unique_ptr<AST::Expression::GetBits> PAR::Parser_Expression::getbits(std::unique_ptr<AST::AExpression> expr) 
{
	static const std::string hint = "define get bit like: `target~[0..8]` get first octect on target.";

	ctx.tok_v.match(TokTy::TILDE);
	ctx.tok_v.expect<98>(TokTy::OPEN_SQUARE, "Expected start slice block '[' after a get bit operator '~'", hint);
	auto get_bit = ctx.Create_Node<AST::Expression::GetBits>(ctx.tok_v.peek(-2));
	get_bit->target = std::move(expr);
	get_bit->range = ctx.p_expr->parse_expression();
	ctx.tok_v.expect<99>(TokTy::CLOSE_SQUARE, "Expected end slice block ']' after range expression", hint);
	if (auto range_ptr = dynamic_cast<AST::AType*>(get_bit->range.get())) {
		get_bit->resolved_range_type = std::shared_ptr<AST::AType>(dynamic_cast<AST::AType*>(get_bit->range.release()));
	}
	return get_bit;
}



std::unique_ptr<AST::Expression::Size_Of> PAR::Parser_Expression::size_of() 
{
	static const std::string hint = "define value Expression size like: `size'a`";
	auto node = ctx.Create_Node<AST::Expression::Size_Of>(ctx.tok_v.peek(-1));

	ctx.tok_v.expect<104>(TokTy::OPEN_PAREN, "Expected start arg '('.", hint);
	node->target = ctx.p_ref->parse_reference();
	ctx.tok_v.expect<105>(TokTy::CLOSE_PAREN, "Expected end arg ')'.", hint);

	return node;
}


std::unique_ptr<AST::Expression::Move> PAR::Parser_Expression::move() 
{
	ctx.tok_v.match(TokTy::CAPA_MOVE_OF);
	auto node = ctx.Create_Node<AST::Expression::Move>(ctx.tok_v.peek());
	node->target = ctx.p_ref->parse_reference();
	return node;
}

std::unique_ptr<AST::Expression::New_Ptr> PAR::Parser_Expression::new_ptr() 
{
	static const std::string hint =
		"define a dynamic Expression allocation:"
		"\n  - primitive `var myPtr = new ptr'i32(10)`"
		"\n  - array `var myPtr = new ptr'[i32 -> 3]({ 1, 2, 3 })`"
		"\n  - array on all `var myPtr = new ptr'[i32 -> 3](0)`"
		"\n  - entity `var myPtr = new ptr'Person(Person{ CIdentity{ name: \"Zagreus\", age: 25 } })`";

	ctx.tok_v.match(TokTy::NEW);

	auto node = ctx.Create_Node<AST::Expression::New>(ctx.tok_v.peek());
	ctx.tok_v.expect_any<100>(kPointerTokens, "Expected pointer specification after 'new' token.", hint);
	node->pointer = TokTy_to_EPtrType(ctx.tok_v.peek(-1).type);

	ctx.tok_v.expect<101>(TokTy::TICK, "Expected tick ' between pointer and type", hint);

	node->type = ctx.p_type->parse_type();
	
	ctx.tok_v.expect<102>(TokTy::OPEN_PAREN, "Expected start value '(' after type", hint);
	node->expression = ctx.p_expr->parse_expression();
	ctx.tok_v.expect<103>(TokTy::CLOSE_PAREN, "Expected end value ')'", hint);

	return node;
}

