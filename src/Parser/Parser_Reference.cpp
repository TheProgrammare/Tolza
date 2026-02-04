#include "Parser_Reference.hpp"

#include "Globals.hpp"
#include "ScriptInfo.hpp"
#include "Parser_Context.hpp"

#include "Parser_Expression.hpp"
#include "Parser_Operator.hpp"
#include "Parser_Memory.hpp"
#include "Parser_Type.hpp"
#include "Parser_Literal.hpp"

std::vector<std::unique_ptr<AST::Reference::Call_Argument>> PAR::Parser_Reference::call_arguments() {
	static const std::string hint =
		"define call argument like:\n"
		"  - ordinal `call(10)`"
		"  - named `call(param_name: 10)`"
		"  - variadic `call(... 10, 20, 30)`";

	if (ctx.tok_v.match(TokTy::CLOSE_PAREN)) return {};

	std::vector<std::unique_ptr<AST::Reference::Call_Argument>> params;

	while (!ctx.tok_v.is_end()) {
		auto param = ctx.Create_Node<AST::Reference::Call_Argument>(ctx.tok_v.peek());

		// if parameter invocation
		if (ctx.tok_v.check(TokTy::IDENTIFIER) && ctx.tok_v.peek(1).ty == TokTy::ASSIGN) {
			param->name = ctx.tok_v.next().val;

			ctx.tok_v.expect(TokTy::COLON, "PAR1120",
				"Expected parameter assignation ':' after a parameter argument name invocation.",
				hint);

			param->val = ctx.p_expr->parse_expression();
			params.push_back(std::move(param));
		}
		// ordered parameter affectation
		else {
			param->val = ctx.p_expr->parse_expression();

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


std::optional<std::unique_ptr<AST::Reference::Call>> PAR::Parser_Reference::try_function_call(const AST::ID& id) {
	if (!ctx.tok_v.match(TokTy::OPEN_PAREN)) return std::nullopt;

	auto call = ctx.Create_Node<AST::Reference::Call>(ctx.tok_v.peek());
	call->id = id;

	call->param_args = call_arguments();

	return call;
}

std::optional<std::unique_ptr<AST::Reference::Call_Pipe>> PAR::Parser_Reference::try_function_call_pipe(const AST::ID& id) {
	if (!ctx.tok_v.match_any({ TokTy::PIPE, TokTy::PIPE_MUT })) return std::nullopt;

	auto pipe_call = ctx.Create_Node<AST::Reference::Call_Pipe>(ctx.tok_v.peek());
	pipe_call->isMutable = ctx.tok_v.peek(-1).ty == TokTy::PIPE_MUT;
	pipe_call->id = id;
/*
	static const std::string hint = "define pipe-call like:"
		"\n  - pure pipe-call `var result = myFunction | param1 | param2 | param3;`"
		"\n  - pure pipe-call generic `var result = myFunction<gen_args> | <gen_args> param1 | <gen_args> param2 | <gen_args> param3;`"
		"\n  - mutable pipe-call `var result = myFunction <-| param1 |+ param2 |- param3;`"
		"\n  - mutable pipe-call generic `var result = myFunction<gen_args> <-| <gen_args> param1 |+ <gen_args> param2 |- <gen_args> param3;`";


	while (!ctx.tok_v.is_end()) {
		pipe_call->arguments.push_back(Create_Node<AST::Reference::Call_Argument>(ctx.tok_v.peek()));


		if (pipe_call->isMutable) {
			auto op = ctx.tok_v.expect_any(kOperatorTokens, "PAR1975", "Expected an operator keyword after pipe operation in mutable pipe call.", hint);

			arg-> = TokTy_to_EOpType(op.ty);
		}
		else {
			std::get<0>(elem) = EOpType::COUNT;
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


ModuleImportation *PAR::Parser_Reference::get_external_source(AST::ID &id)
{
	if (id.path.empty()) return nullptr;
	const std::string &base = id.path[0];
	return ctx.scr_info->get_import_module(base);
}

std::unique_ptr<AST::AReference> PAR::Parser_Reference::parse_reference()
{
	AST::ID id = identifier();
	std::unique_ptr<AST::AReference> target_ref;
	std::unique_ptr<AST::Type_Arguments> gen_args;

	if (auto id_typed = try_identifier_typed(id)) {
		// only identifier typed
		// no value accessible
		if (!ctx.tok_v.check_any({TokTy::OPEN_PAREN, TokTy::OPEN_BRACE})) {
			return std::unique_ptr<AST::AReference>(id_typed.value().release());
		}
		gen_args = std::move(id_typed.value()->gen_args);
	}

	// call e.g. add(a, b)
	if (auto call = try_function_call(id)) {
		call.value()->gen_args = std::move(gen_args);
		target_ref = std::move(call.value());
	}
	// literal entity / component
	else if (ctx.tok_v.match(TokTy::OPEN_BRACKETS) && lit_comp_entity_allowed) {
		Token base_tok = ctx.tok_v.peek();
		auto id = identifier();

		// it's a literal entity e.g. Player{ CId.name: "Enoch", CId.age: 365 }
		//									          ^ found at
		// OR
		// 							  Player{ CId{ name: "Enoch", age: 365 } }
		//										 ^ found at
		if ((id.is_qualified_id() && ctx.tok_v.check(TokTy::COLON))
			|| ctx.tok_v.check(TokTy::OPEN_BRACE)) {
			// reset the moving to correctly parse the literal entity
			ctx.tok_v.jump(base_tok.span.pos);
			return ctx.p_lit->literal_entity(id, std::move(gen_args));
		}
		// it's a literal component e.g. CId{ name: "Enoch", age: 365 }
		else {
			return ctx.p_lit->literal_component(id, std::move(gen_args));
		}
	}
	else {
		auto id_ref = ctx.Create_Node<AST::Identifier_Reference>(ctx.tok_v.peek());
		id_ref->id = id;
		target_ref = std::move(id_ref);
	}

	if (!gen_args) {
		if (auto table_access = try_table_access()) {
			table_access.value()->id = id;
			target_ref = std::move(table_access.value());
		}
		if (auto member_access = try_member_access(id)) {
			member_access.value()->left = std::move(target_ref);
			target_ref = std::move(member_access.value());
		}
	}

	return target_ref;
}

AST::ID PAR::Parser_Reference::identifier(bool no_qualified_id, bool keyword_allowed)
{
    static const std::string hint =
		"define identifier like:"
		"\n  - classic `name` -> name"
		"\n  - with scope path `namespace A { name }` -> A_name"
		"\n  - with explicit path `A::B::C` -> A_B_C";
		//"\n  - if exported : `# export module A\n # scope\nname\n# end\n -> A_name`;

	AST::ID id("");

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
	else if (ctx.tok_v.peek(1).ty != TokTy::STATIC_ACCESS) {
		if (!keyword_allowed)
			id.name =  ctx.tok_v.expect_id("PAR1500", "Expected identifier name.", hint);
		else 
			id.name = ctx.tok_v.next().val;

		return id;
	}

	// it's qualified id
	if (no_qualified_id) 
		ctx.tok_v.add_error_tok(ctx.tok_v.peek(-1), "PAR1502", "Unexpected qualified id.", hint);

	size_t count = 0;
	while (!ctx.tok_v.is_end()) {
		id.path.push_back(ctx.tok_v.next().val);

		// no more path
		if (!ctx.tok_v.match(TokTy::STATIC_ACCESS)) break;

		count++;
		if (count > MAX_PATH_SEG_SIZE) {
			ctx.tok_v.add_error("PAR1501", "Explicit path for identifier is too long (> " + std::to_string(MAX_PATH_SEG_SIZE) + ")", hint);
			break;
		}
	}
	
	return id;
}

std::optional<std::unique_ptr<AST::Type_Reference>> PAR::Parser_Reference::try_identifier_typed(AST::ID &id)
{
	if (!ctx.tok_v.match(TokTy::TURBO_FISH)) return std::nullopt;

	auto id_type = ctx.Create_Node<AST::Type_Reference>(ctx.tok_v.peek(-2)); 
	id_type->id = id;
	id_type->gen_args = ctx.Create_Node<AST::Type_Arguments>(ctx.tok_v.peek(-1));

	if (ctx.tok_v.match(TokTy::CLOSE_BRACKETS)) return id_type;

	while (!ctx.tok_v.is_end()) {
		id_type->gen_args->arguments.push_back(ctx.p_type->parse_type());

		if (ctx.match_field_separator(TokTy::COMMA, TokTy::CLOSE_BRACKETS)) break;
	}

	return id_type;
}

std::optional<std::unique_ptr<AST::Reference::Member_Access>> PAR::Parser_Reference::try_member_access(AST::ID &id) {
	if (!ctx.tok_v.match(TokTy::DOT)) return std::nullopt;

	auto access = ctx.Create_Node<AST::Reference::Member_Access>(ctx.tok_v.peek(-2));

	access->right = parse_reference();

	return access;
}

std::optional<std::unique_ptr<AST::Reference::Table_Access>> PAR::Parser_Reference::try_table_access() {
	if (!ctx.tok_v.match(TokTy::OPEN_SQUARE)) return std::nullopt;

	auto table_access = ctx.Create_Node<AST::Reference::Table_Access>(ctx.tok_v.peek(-1));
	table_access->selector = ctx.p_expr->parse_expression();

	ctx.tok_v.expect(TokTy::CLOSE_SQUARE, "PAR1758", "Expected end of table access '[' after expression.", "");

	return table_access;
}

