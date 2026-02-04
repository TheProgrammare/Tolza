#include "Parser_Declaration_Local.hpp"

#include "AST/AST_Base.hpp"
#include "Parser_Base.hpp"
#include "Parser_Type.hpp"
#include "Parser_Reference.hpp"
#include "Parser_Literal.hpp"
#include "Parser_Statement.hpp"
#include "Parser_Context.hpp"
#include "Parser_Expression.hpp"
#include "Visitor/Symbol_Manager.hpp"
#include "Parser_Declaration.hpp"
#include "Parser_Declaration_Local.hpp"

std::shared_ptr<AST::ALocal> PAR::Parser_Declaration_Local::parse_local(bool silent_error) {
	auto ty = ctx.tok_v.peek().ty;
	switch (ty)
	{
	case TokTy::VAR:
	case TokTy::LET:
	case TokTy::CONST: {
		if (ctx.tok_v.peek(1).ty == TokTy::OPEN_PAREN)
			return variable_unpack();
		return variable();
	}
	case TokTy::LAMBDA:							return lambda();
	case TokTy::CAPA_REF:
	case TokTy::CAPA_MUT:						return capability(); 
	}
	
	if (!silent_error) {
		ctx.tok_v.add_error("PAR1877", "Illegal instruction '" + ctx.tok_v.peek().val + "' in local.", 
			"you can define in local: variable, lambda, call, operation, assignation, statement");
	}

	return nullptr;
}

AST::Declaration::Local::Pattern_Element PAR::Parser_Declaration_Local::pattern_mapping(ECapability capa) {
	if (auto lit = ctx.p_lit->try_literal(true)) {
		return AST::Declaration::Local::Pattern_Element(std::move(lit.value()));
	}
	else {
		auto bind = ctx.Create_Decl<AST::Declaration::Local::Variable_Binding>(ctx.tok_v.peek());
		bind->capability = capa;
		
		if (ctx.tok_v.match_any(kCapabilityKind)) {
			bind->capability = TokTy_to_ECapability(ctx.tok_v.peek(-1).ty);
		}

		bind->id = ctx.p_ref->identifier(true);
		bind->name = bind->id.name;

		return AST::Declaration::Local::Pattern_Element(bind);
	}
}

AST::Evaluator PAR::Parser_Declaration_Local::parse_evaluator(std::shared_ptr<AST::AReference> comparison_ref) {
	if (ctx.tok_v.match_any({ TokTy::CAPA_MUT, TokTy::CAPA_REF })) {
		return AST::Evaluator(parse_pattern(comparison_ref)); 
	}

	// avoid false positive literal component/entity `name {...}` inside a condition [if/else/elif/while] <evaluator> {<statement>}
	ctx.p_ref->lit_comp_entity_allowed = false;
	return AST::Evaluator(ctx.p_expr->parse_expression());
	ctx.p_ref->lit_comp_entity_allowed = true;
}

#include <llvm/ADT/APInt.h>

std::unique_ptr<AST::Declaration::Local::Pattern> PAR::Parser_Declaration_Local::parse_pattern(std::shared_ptr<AST::AReference> comparison_ref)
{
	ECapability capa = TokTy_to_ECapability(ctx.tok_v.peek(-1).ty);

	if (ctx.tok_v.match(TokTy::OPEN_PAREN)) {
		return tuple_pattern(capa, comparison_ref);
	}

	auto id = ctx.p_ref->identifier();

	// enum pattern : if ref Some(a) = val {...} 
	if (ctx.tok_v.match(TokTy::OPEN_PAREN)) {
		return enum_pattern(capa, id, comparison_ref);
	}
	// entity pattern / component pattern
	else if (ctx.tok_v.match(TokTy::ENTITY_START_LIT)) {
		return entity_pattern(capa, id, comparison_ref);
	}
	else if (ctx.tok_v.match(TokTy::OPEN_BRACE)) {
		return component_pattern(capa, id, comparison_ref);
	}

	ctx.tok_v.add_error("PAR1178",
		"Expected pattern.",
		"define auto inferred variable like `let myName = expression;`");
}

std::shared_ptr<AST::Declaration::Local::Variable> PAR::Parser_Declaration_Local::variable() {
	auto var = ctx.Create_Decl<AST::Declaration::Local::Variable>(ctx.tok_v.peek());
	var->kind = TokTy_to_EVariableKind(ctx.tok_v.next().ty);

	var->id = ctx.p_ref->identifier(var.get(), true);

	var->isStatic = ctx.metablock_contains(*var, "static");

	ctx.m_sym->add_decl(var);

	bool isAutoTy = false;

	// explicit type case
	if (ctx.tok_v.match(TokTy::COLON))
		var->ty = ctx.p_type->parse_type();
	// auto deduce type case
	else
		isAutoTy = true;

	// check affectation
	Token assign_tok = ctx.tok_v.next();
	var->assignment = TokTy_to_EAssignmentType(assign_tok.ty);

	if (var->assignment == EAssignmentType::None && isAutoTy) 
		ctx.tok_v.add_error("PAR1171",
			"Expected assignation '=' in auto inferred variable type.",
			"define auto inferred variable like `let myName = expression;`");
	
	auto expr = ctx.p_expr->parse_expression();

	//if (isAutoTy) var->type = resolve_type(expr.get());

	var->expression = std::move(expr);

	return var;
}

std::shared_ptr<AST::Declaration::Local::Variable_Unpack> PAR::Parser_Declaration_Local::variable_unpack() {
	static const std::string hint =
		"define unpack like:"
		"\n  - `var (a, b, c) = myFunction()`"
		"\n  - with ignored values `var (a, _, c) = myFunction()`";

	const auto varKind = ctx.tok_v.next().ty;
	auto unpack = ctx.Create_Node<AST::Declaration::Local::Variable_Unpack>(ctx.tok_v.peek());
	unpack->kind = TokTy_to_EVariableKind(ctx.tok_v.next().ty);

	unpack->isStatic = ctx.metablock_contains(*unpack, "static");

	while (!ctx.tok_v.is_end()) {
		// ignore variable
		if (!ctx.tok_v.match(TokTy::UNDERSCORE)) {
			std::string name = ctx.tok_v.expect_id("PAR1130", "Expected name (identifier) in variable unpack declaration.", hint);
			auto loc = ctx.Create_Decl<AST::Declaration::Local::Variable_Binding>(ctx.tok_v.peek(-1));
			loc->id.name = name;
			unpack->elements.push_back(loc);
			ctx.m_sym->add_decl(loc);
		}
		else unpack->elements.push_back(nullptr);

		if (ctx.match_field_separator(TokTy::COMMA, TokTy::ASSIGN)) break;
	}

	unpack->reference = ctx.p_ref->parse_reference();

	return std::shared_ptr<AST::Declaration::Local::Variable_Unpack>(unpack.release());
}

std::unique_ptr<AST::Declaration::Local::Lambda_Capture> PAR::Parser_Declaration_Local::lambda_capture() {
	static const std::string hint =
		"define capture like:"
		"\n  - modify all variables `[mut]` or `[mut, copy a, ...]`"
		"\n  - copy all variables `[copy] or `[copy, mut a, ...]`"
		"\n  - get instance `[..., self, ...]`";

	auto capture = ctx.Create_Node<AST::Declaration::Local::Lambda_Capture>(ctx.tok_v.peek());

	// all by ref
	if (ctx.tok_v.check_val("mut") && ctx.tok_v.peek(1).ty == TokTy::CLOSE_SQUARE) {
		capture->isAllRef = true;
		ctx.tok_v.next();
		return capture;
	}
	// all by copy
	else if (ctx.tok_v.check_val("copy" ) && ctx.tok_v.peek(1).ty == TokTy::CLOSE_SQUARE) {
		capture->isAllRef = false;
		ctx.tok_v.next();
		return capture;
	}

	if (ctx.tok_v.match(TokTy::SELF)) {
		capture->isCaptureSelf = true;
	}

	while (!ctx.tok_v.is_end()) {
		auto elem = ctx.Create_Node<AST::Declaration::Local::Capture_Member>(ctx.tok_v.peek());

		auto tok_capa = ctx.tok_v.expect_any(kCapabilityKind, "PAR1782", "Expected capture capability kind.", hint);
		elem->capability = TokTy_to_ECapability(tok_capa.ty);

		elem->id = ctx.p_ref->identifier(true);
		capture->elements.push_back(std::move(elem));
	}

	return capture;
}

std::shared_ptr<AST::Declaration::Local::Lambda> PAR::Parser_Declaration_Local::lambda() {
	auto lam = ctx.Create_Decl<AST::Declaration::Local::Lambda>(ctx.tok_v.peek());

	if (ctx.tok_v.check(TokTy::IDENTIFIER)) {
		lam->id = ctx.p_ref->identifier(lam.get(), true);
		ctx.m_sym->add_decl(lam);
		ctx.m_sym->enter_scope(lam->id.name, EScopeType::Lambda);
	}
	else {
		ctx.m_sym->enter_scope("lam", EScopeType::Lambda);
	}

	lam->isConst 			= ctx.metablock_contains(*lam, "const");
	lam->isMutable 			= ctx.metablock_contains(*lam, "mutable");
	lam->isPure 			= ctx.metablock_contains(*lam, "pure");
	lam->isNoexcept 		= ctx.metablock_contains(*lam, "noexcept");
	lam->isConstexpr 		= ctx.metablock_contains(*lam, "constexpr");
	lam->isLambdaConstexpr 	= ctx.metablock_contains(*lam, "lam_constexpr");

	// check capture
	if (ctx.tok_v.match(TokTy::OPEN_SQUARE))
		lam->capture = lambda_capture();

	lam->prototype = ctx.p_type->function_proto(true);
	lam->codeblock = ctx.p_loc->code_block_instruction();

	ctx.m_sym->exit_scope();

	return lam;
}


std::unique_ptr<AST::Declaration::Local::CodeBlock> PAR::Parser_Declaration_Local::code_block(
	bool is_silent_error
	, std::function<AST::CodeBlock_instruction()> in_function)
{
	ctx.tok_v.expect_any({ TokTy::OPEN_BRACE, TokTy::INJECT }, "PAR1199", "Expected start code block '{' or linecode '=>'.", "");

	bool inline_code = ctx.tok_v.peek(-1).ty == TokTy::INJECT;
	auto cb = ctx.Create_Node<AST::Declaration::Local::CodeBlock>(ctx.tok_v.peek(-1));
	
	while (!ctx.tok_v.is_end()) {
		cb->elements.push_back(in_function());

		// one instruction
		if (inline_code) break;
		if (ctx.match_field_separator(TokTy::S_END_OF_FILE, TokTy::CLOSE_BRACE)) break;
	}

	return cb;
}

std::unique_ptr<AST::Declaration::Local::CodeBlock> PAR::Parser_Declaration_Local::code_block_instruction()
{
	ctx.tok_v.expect_any({ TokTy::OPEN_BRACE, TokTy::INJECT }, "PAR1199", "Expected start code block '{' or linecode '=>'.", "");

	bool inline_code = ctx.tok_v.peek(-1).ty == TokTy::INJECT;
	auto cb = ctx.Create_Node<AST::Declaration::Local::CodeBlock>(ctx.tok_v.peek(-1));
	
	while (!ctx.tok_v.is_end()) {
		cb->elements.push_back(ctx.p_base->parse_instruction());

		// one instruction
		if (inline_code) break;
		if (ctx.match_field_separator(TokTy::S_END_OF_FILE, TokTy::CLOSE_BRACE)) break;
	}

	return cb;
}

std::shared_ptr<AST::Declaration::Local::Capability> PAR::Parser_Declaration_Local::capability() {
	static const std::string hint =
		"define capability like:"
		"\n  - reference (read only) `ref a = lvalue`"
		"\n  - mutable (read/write) `mut a = lvalue`";
	
	Token capa_tok_kind = ctx.tok_v.expect_any(kCapabilityKind, "PAR1207", "Expected capability kind.", hint);
	auto capa = ctx.Create_Decl<AST::Declaration::Local::Capability>(ctx.tok_v.peek(-1));
	capa->kind = TokTy_to_ECapability(capa_tok_kind.ty);	

	capa->id = ctx.p_ref->identifier(true);

	ctx.tok_v.expect(TokTy::ASSIGN, "PAR1209", "Expected classic assignation '=' after capability declaration.", hint);

	capa->reference = std::shared_ptr<AST::AReference>(ctx.p_ref->parse_reference());

	return capa;
}

std::unique_ptr<AST::Declaration::Local::Pattern_Component> PAR::Parser_Declaration_Local::component_pattern(
	ECapability capa, 
	AST::ID &comp_id, 
	std::shared_ptr<AST::AReference> comparison_ref)
{
	static const std::string hint = 
		"define component mapping like:"
		"\n  - `[ref/mut] CComponent{field1: [ref/mut/copy/clone] a, field2: 10} = val`";

	auto comp_pat = ctx.Create_Node<AST::Declaration::Local::Pattern_Component>(ctx.tok_v.peek());

	comp_pat->component_id = comp_id;
	comp_pat->capability = capa;

	while (!ctx.tok_v.is_end()) {
		std::string field_name = ctx.p_ref->identifier(true).name;

		ctx.tok_v.expect(TokTy::COLON, "PAR1228", "Expected field mapping association ':'.", hint);

		comp_pat->mapping.push_back({ field_name, pattern_mapping(capa) });

		if (ctx.match_field_separator(TokTy::COMMA, TokTy::CLOSE_BRACE)) break;
	}

	if (!comparison_ref) {
		ctx.tok_v.expect(TokTy::ASSIGN, "PAR1229", "Expected assignation on pattern.", hint);
		comp_pat->component_reference = std::shared_ptr<AST::AReference>(ctx.p_ref->parse_reference());
	}
	else {
		comp_pat->component_reference = comparison_ref;
	}

	if (ctx.tok_v.match(TokTy::IF))
		comp_pat->additive_evaluator = ctx.p_expr->parse_expression();

  	return comp_pat;
}

std::unique_ptr<AST::Declaration::Local::Pattern_Entity> PAR::Parser_Declaration_Local::entity_pattern(
	ECapability capa, 
	AST::ID &entity_id, 
	std::shared_ptr<AST::AReference> comparison_ref)
{
	static const std::string hint = 
		"define entity mapping like:"
		"\n  - component general mapping `[ref/mut] MyEntity::{CComponent{field1: [ref/mut/copy/clone] a, field2: 10}}`";
		"\n  - component field mapping `[ref/mut] MyEntity::{CComponent.field1: [ref/mut/copy/clone] a, CComponent.field2: 10}`";

	auto entity_pat = ctx.Create_Node<AST::Declaration::Local::Pattern_Entity>(ctx.tok_v.peek());

	entity_pat->entity_id = entity_id;
	entity_pat->capability = capa;

	while (!ctx.tok_v.is_end()) {
		AST::ID comp_id = ctx.p_ref->identifier();

		if (ctx.tok_v.match(TokTy::OPEN_BRACE)) {
			while (!ctx.tok_v.is_end()) {
				std::string name = ctx.p_ref->identifier(true).name;

				ctx.tok_v.expect(TokTy::COLON, "PAR1229", "Expected field mapping association ':'.", hint);

				entity_pat->mapping.push_back({ comp_id, name, pattern_mapping(capa) });

				if (ctx.match_field_separator(TokTy::COMMA, TokTy::CLOSE_BRACE)) break;
			}
		}
		else  {
			ctx.tok_v.expect(TokTy::DOT, "PAR1336", "Expected component field mapping '.' or start component general mapping '{'.", hint);

			std::string name = ctx.p_ref->identifier(true).name;

			ctx.tok_v.expect(TokTy::COLON, "PAR1229", "Expected field mapping association ':'.", hint);

			entity_pat->mapping.push_back({ comp_id, name, pattern_mapping(capa) });
		}

		if (ctx.match_field_separator(TokTy::COMMA, TokTy::CLOSE_BRACE)) break;
	}

	if (!comparison_ref) {
		ctx.tok_v.expect(TokTy::ASSIGN, "PAR1229", "Expected assignation on pattern.", hint);
		entity_pat->entity_reference = std::shared_ptr<AST::AReference>(ctx.p_ref->parse_reference());
	}
	else {
		entity_pat->entity_reference = comparison_ref;
	}

	if (ctx.tok_v.match(TokTy::IF))
		entity_pat->additive_evaluator = ctx.p_expr->parse_expression();
	
	return entity_pat;
}

std::unique_ptr<AST::Declaration::Local::Pattern_Tuple> PAR::Parser_Declaration_Local::tuple_pattern(
	ECapability capa,
	std::shared_ptr<AST::AReference> comparison_ref)
{
	static const std::string hint = 
		"define enum pattern on condition like:" 
		"\n  - binding pattern `[if/elif/while] [ref/mut] ([copy/clone/mut/ref] a, _, 10) = tuple {...}`"
		"\n  Binding rules: - ref -> bind primitives by `copy`, bind complex types by `ref`"
		"\n                 - mut -> bind primitives and complex types by `mut`"
		"\n                 - override binding rule by explicit binding mode `([copy/clone/mut/ref] a)`";

	auto pat = ctx.Create_Node<AST::Declaration::Local::Pattern_Tuple>(ctx.tok_v.peek());
	pat->capability = capa;

	if (ctx.tok_v.check(TokTy::CLOSE_PAREN))
		ctx.tok_v.add_error("PAR1634", "Unexpected void tuple.", hint);
	
	while (!ctx.tok_v.is_end()) {
		pat->mapping.push_back(pattern_mapping(capa));
		if (ctx.match_field_separator(TokTy::COMMA, TokTy::CLOSE_PAREN)) break;
	}

	if (!comparison_ref) {
		ctx.tok_v.expect(TokTy::ASSIGN, "PAR1229", "Expected assignation on pattern.", hint);
		pat->tuple_reference = std::shared_ptr<AST::AReference>(ctx.p_ref->parse_reference());
	}
	else {
		pat->tuple_reference = comparison_ref;
	}

	if (ctx.tok_v.match(TokTy::IF))
		pat->additive_evaluator = ctx.p_expr->parse_expression();

	return pat;
}

std::unique_ptr<AST::Declaration::Local::Pattern_Enum> PAR::Parser_Declaration_Local::enum_pattern(
	ECapability capa, 
	AST::ID &enum_id,
	std::shared_ptr<AST::AReference> comparison_ref) {
	static const std::string hint = 
		"define enum pattern on condition like:\n" 
		"  - binding pattern `[if/elif/while] [ref/mut] Some(a) = value {...}`"
		"  - binding pattern with more condition `if [ref/mut] Some(a) = value if a > 10 {...}`"
		"  - binding pattern conditionnal `if [ref/mut] Some(10) = value {...}`"
		"  - check only indexation `[if/elif/while] value == Some`";

	auto pat = ctx.Create_Node<AST::Declaration::Local::Pattern_Enum>(ctx.tok_v.peek());
	pat->enum_id = enum_id;

	ctx.tok_v.expect(TokTy::OPEN_PAREN, "PAR1205", "Expected start binding on enum types '('.", hint);

	if (ctx.tok_v.check(TokTy::CLOSE_PAREN))
		ctx.tok_v.add_error("PAR1206", "Unexpected end of binding on enum types ')'. A enum pattern on condition must have at least one binding. Else, use a check indexation.", hint);

	EExprPassMode pass_mode = TokTy_to_EExprPassMode(ctx.tok_v.peek().ty);
	if (pass_mode != EExprPassMode::None) ctx.tok_v.next();

	while (!ctx.tok_v.is_end()) {
		pat->mapping.push_back(pattern_mapping(capa));
		if (ctx.match_field_separator(TokTy::COMMA, TokTy::CLOSE_PAREN)) break;
	}

	if (!comparison_ref) {
		ctx.tok_v.expect(TokTy::ASSIGN, "PAR1229", "Expected assignation on pattern.", hint);
		pat->enum_reference = std::shared_ptr<AST::AReference>(ctx.p_ref->parse_reference());
	}
	else {
		pat->enum_reference = comparison_ref;
	}

	if (ctx.tok_v.match(TokTy::IF))
		pat->additive_evaluator = ctx.p_expr->parse_expression();

	return pat;
}



