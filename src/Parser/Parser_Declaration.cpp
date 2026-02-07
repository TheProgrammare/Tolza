
#include "Parser_Declaration.hpp"

#include <iostream>

#include "Parser_Base.hpp"

#include "Parser_Context.hpp"

#include "Parser_Base.hpp"
#include "Parser_Reference.hpp"
#include "Parser_Type.hpp"
#include "Parser_Statement.hpp"
#include "Parser_Declaration_ECS.hpp"
#include "Parser_Declaration_Local.hpp"
#include "Parser_Expression.hpp"

#include "Metacode.hpp"

#include "AST/AST_Declaration.hpp"
#include "AST/AST_Declaration_ECS.hpp"

#include "Visitor/Symbol_Manager.hpp"

std::shared_ptr<AST::ADeclaration> PAR::Parser_Declaration::parse_declaration()
{
	auto tok = ctx.tok_v.peek();
	switch (tok.ty)
	{
	case TokTy::MOD: 			return module();
	case TokTy::ENUM: 			return enumeration();
	case TokTy::VAR:
	case TokTy::LET:
	case TokTy::CONST:			return global_variable();
	case TokTy::FUNCTION:		return function();
	case TokTy::GENERIC:		return generic();
	case TokTy::TYPE:			return type_alias();
	case TokTy::COMPONENT:		return ctx.p_ecs->component();
	case TokTy::SYSTEM:			return ctx.p_ecs->system();
	case TokTy::ENTITY:			return ctx.p_ecs->entity();
	case TokTy::EXPORT:			return ctx.p_base->parse_export();
	case TokTy::IMPORT: {
		auto ignore = ctx.p_base->parse_import(); 
		return nullptr;
	}
	default:
		break;
	}

	ctx.tok_v.add_error("PAR1877", "Illegal instruction '" + tok.val + "' in global.", 
			"you can define in global: namespace, variable, function, entity, component, system");
			
	return nullptr;
}

std::shared_ptr<AST::Declaration::Mod> PAR::Parser_Declaration::module()
{
    static const std::string hint =
		"define module like `mod myName { ... }`";
	ctx.tok_v.match(TokTy::MOD);

	auto node = ctx.Create_Decl<AST::Declaration::Mod>(ctx.tok_v.peek());
	node->id = ctx.p_ref->identifier(false, true);
	ctx.m_sym->enter_scope(node->id.name, EScopeType::Mod);

	ctx.tok_v.expect(TokTy::OPEN_BRACE, "PAR1251", "Expected open code block '{' after module name.", hint);

	while (!ctx.tok_v.is_end()) {
		if (ctx.tok_v.check_any({ TokTy::IMPORT, TokTy::EXPORT })) {
			ctx.tok_v.add_error_tok(ctx.tok_v.peek(), "PAR1897", "Illegal nested module export/import instruction.", hint);
		}

		node->elements.push_back(ctx.p_decl->parse_declaration());

		if (ctx.match_field_separator(TokTy::S_END_OF_FILE, TokTy::CLOSE_BRACE)) break;
	}

	ctx.m_sym->exit_scope();

	return node;
}

std::shared_ptr<AST::Declaration::Enum> PAR::Parser_Declaration::enumeration() {
	static const std::string hint =
		"define enum like:"
		"\n  - typed enum `enum Option { Valid(T), Invalid }"
		"\n  - classic enum `enum Cardinal { North, South, East, West }";
	ctx.tok_v.match(TokTy::ENUM);

	auto enu = ctx.Create_Decl<AST::Declaration::Enum>(ctx.tok_v.peek());

	enu->id = ctx.p_ref->identifier();
	ctx.m_sym->add_decl(enu);
	ctx.m_sym->enter_scope(enu->id.name, EScopeType::Enum);

	ctx.tok_v.expect(TokTy::OPEN_BRACE, "PAR1261", "Expected start enum block '{' after enum name declaration.", hint);

	while (!ctx.tok_v.is_end()) {
		auto elem = ctx.Create_Node<AST::Declaration::Enum_Element>(ctx.tok_v.peek());
		elem->name = ctx.tok_v.expect_id("PAR1262", "Expected name (identifier) in enum block.", hint);

		if (ctx.tok_v.match(TokTy::OPEN_PAREN)) {
			while (!ctx.tok_v.is_end()) {
				elem->types.push_back(ctx.p_type->parse_type());

				if (ctx.match_field_separator(TokTy::COMMA, TokTy::CLOSE_PAREN)) break;
			}
		}

		enu->variants.push_back(std::move(elem));
		if (ctx.match_field_separator(TokTy::COMMA, TokTy::CLOSE_BRACE)) break;
	}

	ctx.m_sym->exit_scope();

	return enu;
}

std::shared_ptr<AST::Declaration::Global> PAR::Parser_Declaration::global_variable() {
	static const std::string hint = 
	"define global variable like:"
	"\n  - mutable : `var name: type = expression` `var name = expression`"
	"\n  - immutable : `let name: type = expression` `let name = expression`"
	"\n  - constant (compiletime value): `const name: type = expression` `const name = expression`"
	"\n  - external mutable :\n   `# extern\n    var name: type`"
	"\n  - external immutable :\n   `# extern\n    let name: type";
	

	auto kind_tok = ctx.tok_v.expect_any({ TokTy::LET, TokTy::VAR, TokTy::CONST }, "PAR1587", "Expected global variable declaration token", hint); 
	EVariableKind kind = TokTy_to_EVariableKind(kind_tok.ty);

	auto var = ctx.Create_Decl<AST::Declaration::Global>(ctx.tok_v.peek());
	var->kind = kind;
	var->id = ctx.p_ref->identifier(true);

	var->isExtern = ctx.metablock_contains(*var, "extern");

	ctx.m_sym->add_decl(var);

	if (var->id.name.empty()) {
		ctx.tok_v.add_error("PAR1588", "Invalid Identifier !", ""); 
	}

	// type definition 
	// no expression
	if (var->isExtern) {
		ctx.tok_v.expect(TokTy::COLON, "PAR1589", "Expected type definition for an global variable marked external.", hint);
		var->ty = ctx.p_type->parse_type();
		ctx.tok_v.match(TokTy::SEMICOLON);
		return var;
	}

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

	if (var->assignment == EAssignmentType::NONE && isAutoTy) 
		ctx.tok_v.add_error("PAR1161",
		"Expected assignation '=' in auto inferred variable type.",
		"define auto inferred variable like `let myName = expression;`");

	auto expr = ctx.p_expr->parse_expression();

	///if (isAutoTy) var->type = resolve_type(expr.get());

	var->expression = std::move(expr);

	return var;
}

std::shared_ptr<AST::Declaration::Function> PAR::Parser_Declaration::function() {
	static const std::string hint =
		"define function like:"
		"\n  - definition `fn myName() { ... }`"
		"\n  - definition with return `fn myName() -> i32 { ... }`."
		"\n  - extern declaration\n   `# extern\n    fn myName();`."
		"\n  - extern declaration with return\n   `# extern\n    fn myName() -> i32;`.";

	ctx.tok_v.match(TokTy::FUNCTION);

	auto fn = ctx.Create_Decl<AST::Declaration::Function>(ctx.tok_v.peek());

	fn->id = ctx.p_ref->identifier();
	fn->isConst = ctx.metablock_contains(*fn, "const");
	fn->isPure = ctx.metablock_contains(*fn, "pure");

	fn->isExtern = ctx.metablock_contains(*fn, "extern");
	if (auto pattern = ctx.get_instruct(*fn, { "extern", "<*>" })) {
		fn->extern_call_convention = pattern->at_str(1, 0);
	}

	ctx.m_sym->add_decl(fn);
	ctx.m_sym->enter_scope(fn->id.name, EScopeType::Function);

	fn->prototype = ctx.p_type->explicit_function_proto(false);

	// if extern : no definition
	if (fn->isExtern && ctx.tok_v.check(TokTy::OPEN_BRACE))
		ctx.tok_v.add_error("PAR1362", "Unexpected start code block '{' after a extern function declaration", hint);

	if (!fn->isExtern) fn->codeblock = ctx.p_loc->code_block_instruction();

	ctx.m_sym->exit_scope();

	return fn;
}

std::shared_ptr<AST::Declaration::Generic> PAR::Parser_Declaration::generic() {
	static const std::string kHint_gen = "define geneneric like `gen name<T, ...> { ... }`.";
	static const std::string kHint_filter =
		"define geneneric filter like:"
		"\n  - alone operator `T use op +;`"
		"\n  - role `T use role Printable;`"
		"\n  - system `T use sys Move;`"
		"\n  - component `T use comp Position;`"
		"\n  - typealias `T use type len;`"
		"\n  - nested filter `T is gen::base_of<Animal>;`";

	auto gen = ctx.Create_Decl<AST::Declaration::Generic>(ctx.tok_v.peek());

	ctx.tok_v.match(TokTy::GENERIC);

	gen->id = ctx.p_ref->identifier(gen.get());
	ctx.m_sym->add_decl(gen);
	ctx.m_sym->enter_scope(gen->id.name, EScopeType::Generic);
	ctx.tok_v.expect(TokTy::OPEN_BRACKETS, "PAR1371", "Expected start type '<' after generic name.", kHint_gen);

	while (!ctx.tok_v.is_end()) {
		gen->targetGenericSymbols.insert(ctx.tok_v.expect_id("PAR1372", "Expected name (identifier) in generic typenames.", kHint_gen));

		if (ctx.match_field_separator(TokTy::S_END_OF_FILE, TokTy::CLOSE_BRACKETS)) break;
	}

	ctx.tok_v.expect(TokTy::OPEN_BRACE, "PAR1374", "Expected start code block '{' after generic declaration.", kHint_gen);

	while (!ctx.tok_v.is_end()) {
		if (ctx.tok_v.match(TokTy::CLOSE_BRACE)) break;
		std::string firstok = ctx.tok_v.expect_id("PAR1375", "Expected typename (identifier) in start of generic filter.", kHint_filter);

		// case: T op ...
		if (ctx.tok_v.match(TokTy::OP)) {
			auto gen_op = ctx.Create_Node<AST::Generic::Have_Op>(ctx.tok_v.peek());
			gen_op->targetGenSym = firstok;
			auto tok_op = ctx.tok_v.expect_any(kOperatorTokens, "PAR1375", "Expected operator after 'op' keyword in generic filter argument.", kHint_filter);
			gen_op->operatorType = TokTy_to_EBinOpType(tok_op.ty);
			
			gen->conditions.push_back(std::move(gen_op));
			ctx.tok_v.match(TokTy::SEMICOLON);
		}
		// case: T comp ...
		else if (ctx.tok_v.match(TokTy::COMPONENT)) {
			auto comp = ctx.Create_Node<AST::Generic::Use_Component>(ctx.tok_v.peek());
			comp->targetGenSym = firstok;
			comp->component = ctx.p_ref->parse_reference();
			gen->conditions.push_back(std::move(comp));
		}
		// case: T role ...
		else if (ctx.tok_v.match(TokTy::ROLE)) {
			auto role = ctx.Create_Node<AST::Generic::Have_Role>(ctx.tok_v.peek());
			role->targetGenSym = firstok;
			role->role = ctx.p_ref->parse_reference();
			gen->conditions.push_back(std::move(role));
		}
		// case: T sys ...
		else if (ctx.tok_v.match(TokTy::SYSTEM)) {
			auto sys = ctx.Create_Node<AST::Generic::Compatible_System>(ctx.tok_v.peek());
			sys->targetGenSym = firstok;
			sys->system = ctx.p_ref->parse_reference();
			gen->conditions.push_back(std::move(sys));
		}
		// case: T is i32 | type::floating | ...
		else if (ctx.tok_v.match(TokTy::IS)) {
			auto nested = ctx.Create_Node<AST::Generic::Is_Type>(ctx.tok_v.peek());
			nested->srcTypename = firstok;

			while (!ctx.tok_v.is_end()) {
				nested->inType.push_back(ctx.p_ref->parse_reference());

				if (ctx.tok_v.match(TokTy::PIPE)) continue;
				break;
			}

			gen->conditions.push_back(std::move(nested));
		}
		// case: T cast to/from ...
		else if (ctx.tok_v.match(TokTy::CAST)) {
			auto castNode = ctx.Create_Node<AST::Generic::Can_Cast>(ctx.tok_v.peek());
			castNode->srcTypename = firstok;

			if (ctx.tok_v.peek(0).val != "to" && ctx.tok_v.peek(0).val != "from") {
				ctx.tok_v.add_error("PAR1376", "Expected cast way 'to' or 'from' in generic filter argument.", kHint_filter);
			}

			castNode->target = ctx.p_type->parse_type();

			gen->conditions.push_back(std::move(castNode));
		}
		else ctx.tok_v.add_error("PAR1377", "Expected generic condition 'use' or 'is' in generic filter argument.", kHint_filter);

		ctx.tok_v.match(TokTy::SEMICOLON);
	}

	ctx.m_sym->exit_scope();
	return gen;
}

std::shared_ptr<AST::Declaration::Type_Alias> PAR::Parser_Declaration::type_alias() {
	auto tyAlias = ctx.Create_Decl<AST::Declaration::Type_Alias>(ctx.tok_v.peek());

	ctx.tok_v.match(TokTy::TYPE);

	tyAlias->id = ctx.p_ref->identifier(tyAlias.get());

	ctx.tok_v.expect(TokTy::ASSIGN, "PAR1041",
		"Expected assignation '=' after typealias name.",
		"define typealias like `type myAlias = i32;`.");

	tyAlias->ty = ctx.p_type->parse_type();

	ctx.m_sym->add_decl(tyAlias);
	return tyAlias;
}

