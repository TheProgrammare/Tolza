
#include "Parser_Statement.hpp"

#include "Parser_Context.hpp"
#include "Parser_Declaration_COP.hpp"
#include "Parser_Declaration_Local.hpp"
#include "Parser_Declaration.hpp"
#include "Parser_Expression.hpp"
#include "Parser_Literal.hpp"
#include "Parser_Memory.hpp"
#include "Parser_Operator.hpp"
#include "Parser_Reference.hpp"
#include "Parser_Statement.hpp"
#include "Parser_Type.hpp"

#include "Visitor/Symbol_Manager.hpp"

#include "AST/AST_Statement.hpp"
#include "AST/AST_Type.hpp"


std::unique_ptr<AST::Node> PAR::Parser_Statement::parse_statement(bool is_silent_error) {
	ctx.tok_v.match(TokTy::SEMICOLON); // consume because the parser use only for explicit end instruction

	switch (ctx.tok_v.peek().type)
	{
	case TokTy::IF:						return if_statement();
	case TokTy::FOR:					return for_statement();
	case TokTy::WHILE:
	case TokTy::DO_WHILE:				return while_statement();
	case TokTy::LOOP:					return loop_statement();
	case TokTy::MATCH:					return match_statement();
	case TokTy::BREAK:					ctx.tok_v.match(TokTy::BREAK);		return ctx.Create_Node<AST::Statement::Break>(ctx.tok_v.peek(-1));
	case TokTy::END:					ctx.tok_v.match(TokTy::END);		return ctx.Create_Node<AST::Statement::Return>(ctx.tok_v.peek(-1));
	case TokTy::CONTINUE:				ctx.tok_v.match(TokTy::CONTINUE); 	return ctx.Create_Node<AST::Statement::Continue>(ctx.tok_v.peek(-1));
	case TokTy::RETURN:					return return_flow();
	case TokTy::GOTO:					return goto_statement();
	case TokTy::GOTO_LABEL:				return goto_label_statement();
	default: break;
	}

	if (!is_silent_error)
		ctx.tok_v.add_error("PAR1444",
			"Unexpected '" + ctx.tok_v.peek().val + "' keyword type (" + ctx.tok_v.peek().val + ") not allowed in function statement.",
			"you can define in functions: variable, entity, enum, safe cast, if, else, do, while, match, break, continue, return");
	return nullptr;
}


std::unique_ptr<AST::Statement::If> PAR::Parser_Statement::if_statement() {
	auto ifState = ctx.Create_Node<AST::Statement::If>(ctx.tok_v.peek());

	ctx.tok_v.match(TokTy::IF);
	ctx.tok_v.match(TokTy::ELIF);

	ctx.m_sym->enter_scope("if", EScopeType::If);

	ifState->evaluator = ctx.p_loc->parse_evaluator(nullptr);

	ifState->codeblock = ctx.p_loc->code_block_instruction();

	// else or else if case  
	if (ctx.tok_v.match(TokTy::ELIF)) {
		std::unique_ptr<AST::Statement::If> elifState;
		elifState = if_statement(); // recursive call
		elifState->isElseNoCondition = false;
	}
	else if (ctx.tok_v.match(TokTy::ELSE)) {
		std::unique_ptr<AST::Statement::If> elseState;
		elseState = ctx.Create_Node<AST::Statement::If>(ctx.tok_v.peek(-2)); // peek to else token
		elseState->codeblock = ctx.p_loc->code_block_instruction();
		elseState->isElseNoCondition = true;

		ifState->alternative_statement = std::move(elseState);
	}

	ctx.m_sym->exit_scope();

	return ifState;
}

std::unique_ptr<AST::Statement::For> PAR::Parser_Statement::for_statement() {
	static const std::string hint =
		"define for state like:"
		"\n  - for index : `for i in start..end { ... }`"
		"\n  - for array : `for mut/ref/copy item in array` { ... }"
		"\n  - for array with index : `for i, mut/ref/copy item in array { ... }`"
		"\n  - for map : `for mut/ref/copy (item, key) in map { ... }`"
		"\n  - for map with index : `for i, mut/ref/copy (item, key) in map { ... }`"
		"\n  - for unpack : `for mut/ref/copy (a, b, ...) in array_tuple { ... }`"
		"\n  - for unpack with index : `for i, mut/ref/copy (a, b, ...) in array_tuple { ... }`";

	auto forState = ctx.Create_Node<AST::Statement::For>(ctx.tok_v.peek());

	ctx.tok_v.match(TokTy::FOR);

	ctx.m_sym->enter_scope("for", EScopeType::For);

	// index
	if (ctx.tok_v.check(TokTy::IDENTIFIER)) {
		auto index = ctx.Create_Decl<AST::Declaration::Local::Parameter>(ctx.tok_v.peek());
		index->id = ctx.p_ref->identifier(true);
		index->passMode = EPassMode::Mut;
		auto type = ctx.Create_Node<AST::Type::Primitive>(index.get()->_token);
		type->_type = EPrimType::iSize;
		index->type = std::shared_ptr<AST::Type::Primitive>(type.release());
		
		ctx.m_sym->add_decl(index);
		forState->index = std::move(index);
	}
	// items 
	if (ctx.tok_v.check_any(kParameterPassMode)) {
		EPassMode pass_mode = TokTy_to_EPassMode(ctx.tok_v.next().type);

		if (ctx.tok_v.match(TokTy::OPEN_PAREN)) {
			while (!ctx.tok_v.is_end()) {
				auto item = ctx.Create_Decl<AST::Declaration::Local::Parameter>(ctx.tok_v.peek());
				item->passMode = pass_mode;
				item->id = ctx.p_ref->identifier(true);

				ctx.m_sym->add_decl(item);
				forState->items.push_back(std::move(item));

				if (ctx.match_field_separator(TokTy::COMMA, TokTy::CLOSE_PAREN)) break;
			}
		}
		else {
			auto item = ctx.Create_Decl<AST::Declaration::Local::Parameter>(ctx.tok_v.peek());
			item->passMode = pass_mode;
			item->id = ctx.p_ref->identifier(true);

			ctx.m_sym->add_decl(item);
			forState->items.push_back(std::move(item));
		}
	}

	ctx.tok_v.expect(TokTy::IN, "PAR1292", "Expected in keyword 'in' after for identifier.", hint);

	forState->src = ctx.p_expr->parse_expression();

	forState->codeblock = ctx.p_loc->code_block_instruction();

	ctx.m_sym->exit_scope();

	return forState;
}

std::unique_ptr<AST::Statement::Loop> PAR::Parser_Statement::loop_statement() {
	static const std::string hint = "define loop statement like: `loop { ... }`";

	auto flow = ctx.Create_Node<AST::Statement::Loop>(ctx.tok_v.peek());

	ctx.tok_v.match(TokTy::LOOP);
	ctx.m_sym->enter_scope("loop", EScopeType::Loop);

	flow->codeblock = ctx.p_loc->code_block_instruction();

	ctx.m_sym->exit_scope();

	return flow;
}

std::unique_ptr<AST::Statement::While> PAR::Parser_Statement::while_statement() {
	static const std::string while_hint =
		"define while statement like:"
		"\n  - `while <condition> { ... }`"
		"\n  - `while <confition> => ...`";
	static const std::string do_while_hint =
		"define do-while statement like:"
		"\n  - `do { ... } while condition;`"
		"\n  - `do => ... while confition;`";

	auto flow = ctx.Create_Node<AST::Statement::While>(ctx.tok_v.peek());
	ctx.m_sym->enter_scope("while", EScopeType::While);

	if (ctx.tok_v.match(TokTy::WHILE)) {
		flow->isDo = false;

		flow->evaluator = ctx.p_loc->parse_evaluator(nullptr);

		flow->codeblock = ctx.p_loc->code_block_instruction();
	}
	else if (ctx.tok_v.match(TokTy::DO_WHILE)) {
		flow->isDo = true;

		flow->codeblock = ctx.p_loc->code_block_instruction();

		ctx.tok_v.expect(TokTy::WHILE, "PAR1312", "Expected while keyword after do statement.", do_while_hint);

		flow->evaluator = ctx.p_loc->parse_evaluator(nullptr);

		ctx.tok_v.match(TokTy::SEMICOLON);
	}
	else ctx.tok_v.add_error(
		"PAR1313",
		"Expected do or while keyword!",
		do_while_hint);

	ctx.m_sym->exit_scope();

	return flow;
}

std::unique_ptr<AST::Statement::Match> PAR::Parser_Statement::match_statement() {
	static const std::string hint =
		"define match like:"
		"\n  `match value {"
		"\n     case > 100 => ..."
		"\n     case in 0..=10 => { ... }"
		"\n     case Validation::Valid(a) => { ... }"
		"\n     _ => { ... }"
		"\n   }`";


	auto match = ctx.Create_Node<AST::Statement::Match>(ctx.tok_v.peek());

	ctx.tok_v.match(TokTy::MATCH);
	ctx.m_sym->enter_scope("match", EScopeType::Match);

	match->base = std::shared_ptr<AST::AReference>(ctx.p_ref->parse_reference());

	ctx.tok_v.expect(TokTy::OPEN_BRACE, "PAR1320", "Expected start code block '{' after match defintion.", hint);
	bool otherDefine = false;

	// check all cases
	while (!ctx.tok_v.is_end()) {
		if (otherDefine) ctx.tok_v.add_error("PAR1321", "Expected end match '}' after the other '_ =>' case definition.", hint);

		if (ctx.tok_v.match(TokTy::CLOSE_BRACE)) {
			if (match->cases.empty()) { 
				ctx.tok_v.add_error("PAR1322", "Match case without any case defined", hint);
			}
			break;
		}

		if (ctx.tok_v.match(TokTy::UNDERSCORE)) {
			auto ncase = ctx.Create_Node<AST::Statement::Match_Case>(ctx.tok_v.peek(-1));
			ncase->codeblock = ctx.p_loc->code_block_instruction();
			otherDefine = true;
			break;
		}

		auto ncase = ctx.Create_Node<AST::Statement::Match_Case>(ctx.tok_v.peek());

		if (ctx.tok_v.match_any({ TokTy::CAPA_MUT, TokTy::CAPA_REF })) {
			ncase->evaluator = ctx.p_loc->parse_pattern(match->base); 
		}
		else {
			ctx.p_ref->lit_comp_entity_allowed = false;
			ncase->evaluator = ctx.p_expr->parse_expression();
			ctx.p_ref->lit_comp_entity_allowed = true;
		}

		ncase->codeblock = ctx.p_loc->code_block_instruction();

		if (ctx.match_field_separator(TokTy::S_END_OF_FILE, TokTy::CLOSE_BRACE)) break;
	}

	ctx.m_sym->exit_scope();

	return match;
}

std::unique_ptr<AST::Statement::GoTo> PAR::Parser_Statement::goto_statement()
{
	ctx.tok_v.match(TokTy::GOTO);
	auto goto_statement = ctx.Create_Node<AST::Statement::GoTo>(ctx.tok_v.peek(-1));
	goto_statement->id = ctx.p_ref->identifier(true);
	return goto_statement;
}

std::unique_ptr<AST::Statement::If_Ternary> PAR::Parser_Statement::ternary_if() {
	auto ternaryIf = ctx.Create_Node<AST::Statement::If_Ternary>(ctx.tok_v.peek());

	ternaryIf->evaluator = ctx.p_loc->parse_evaluator(nullptr);
	ternaryIf->true_line = ctx.p_loc->code_block_instruction();

	if (ctx.tok_v.match(TokTy::ELSE)) {
		ternaryIf->false_line = ctx.p_loc->code_block_instruction();
	}

	return ternaryIf;
}


std::unique_ptr<AST::Statement::Return> PAR::Parser_Statement::return_flow() {
	auto node = ctx.Create_Node<AST::Statement::Return>(ctx.tok_v.peek());

	ctx.tok_v.match(TokTy::RETURN);

	if (ctx.tok_v.match(TokTy::SEMICOLON)) return node;
	if (ctx.tok_v.check(TokTy::CLOSE_BRACE)) return node;

	// return with value 
	node->value = ctx.p_expr->parse_expression();
	return node;
}

std::unique_ptr<AST::Statement::GoTo_Label> PAR::Parser_Statement::goto_label_statement()
{
	ctx.tok_v.match(TokTy::GOTO_LABEL);
 	auto goto_label =  ctx.Create_Decl<AST::Statement::GoTo_Label>(ctx.tok_v.peek(-1));
	goto_label->id = ctx.p_ref->identifier(true);
	ctx.tok_v.expect(TokTy::COLON, "PAR1978", "Expected colon ':' after label name.", 
		"define goto label like: `label my_label:`");
	ctx.m_sym->add_decl(goto_label);
	return std::unique_ptr<AST::Statement::GoTo_Label>(goto_label.get());
}
