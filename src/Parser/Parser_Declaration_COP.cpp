#include "Parser_Declaration_COP.hpp"

#include <vector>

#include "AST/AST_Declaration_COP.hpp"
#include "AST/AST_Statement.hpp"
#include "AST/AST_Declaration_Local.hpp"
#include "AST/AST_Type.hpp"
#include "Visitor/Symbol_Manager.hpp"

#include "Parser_Context.hpp"
#include "Parser_Reference.hpp"
#include "Parser_Type.hpp"
#include "Parser_Expression.hpp"
#include "Parser_Declaration_Local.hpp"

std::shared_ptr<AST::Declaration::COP::Component> PAR::Parser_Declaration_COP::component() {
	static const std::string hint =
		"define component declaration like:"
		"\n  - multi filed `comp name { var field_name: type = value, ... }`"
		"\n  - no field `comp name {}";

	ctx.tok_v.match(TokTy::COMPONENT);

	auto comp = ctx.Create_Decl<AST::Declaration::COP::Component>(ctx.tok_v.peek());

	// not handled if (auto where = ctx.p_meta->metacode_where()) comp->gen_where;

	comp->id = ctx.p_ref->identifier(comp.get());
	ctx.tok_v.expect(TokTy::OPEN_BRACE, "PAR1401", "Expected start code block '{' after '" + comp->debug_str() + "'.", hint);

	// is no typed component
	if (ctx.tok_v.match(TokTy::CLOSE_BRACE)) return comp;

	while (!ctx.tok_v.is_end()) {
		auto field = ctx.Create_Decl<AST::Declaration::COP::Component_Field>(ctx.tok_v.peek());
		field->isNoDefault = ctx.metablock_contains(*field, "nodefault");

		field->id = ctx.p_ref->identifier(field.get(), true);
		ctx.tok_v.expect(TokTy::COLON, "PAR1404", "Expected type defintion symbol ':' after field name", hint);

		field->ty = ctx.p_type->parse_type();

		ctx.m_sym->add_decl(field);

		if (!field->isNoDefault) {
			ctx.tok_v.expect(TokTy::ASSIGN, "PAR1405", "Expected default value assignation '=' after field declaration", hint);

			field->default_value = ctx.p_expr->parse_expression();
		}

		comp->fields.push_back(field);

		if (ctx.match_field_separator(TokTy::COMMA, TokTy::CLOSE_BRACE)) break;
	}

	return comp;
}

std::shared_ptr<AST::Declaration::COP::Role> PAR::Parser_Declaration_COP::role() {
	static const std::string hint = "define role like: `role name { comp1, comp2, ... }`";

	ctx.tok_v.match(TokTy::ROLE);

	Token tok = ctx.tok_v.peek();

	auto role = ctx.Create_Decl<AST::Declaration::COP::Role>(tok);
	role->id = ctx.p_ref->identifier(role.get());
	ctx.m_sym->add_decl(role);
	ctx.m_sym->enter_scope(role->id.name, EScopeType::Role);

	ctx.tok_v.expect(TokTy::OPEN_BRACE, "PAR1271", "Expected start definition '{' after role declaration.", hint);

	while (!ctx.tok_v.is_end()) {
		auto ref = ctx.p_ref->parse_reference();
		if (dynamic_cast<AST::Type_Reference*>(ref.get())) {
			role->components.push_back(std::move(ref));
		}
		else if (dynamic_cast<AST::Identifier_Reference*>(ref.get())) {
			role->components.push_back(std::move(ref));
		}
		else {
			ctx.tok_v.add_error_tok(ref->_token, "PAR1220", "Expected a identifier ou a typed identifier reference of component.", hint);
		}

		if (ctx.match_field_separator(TokTy::COMMA, TokTy::CLOSE_BRACE)) break;
	}

	ctx.m_sym->exit_scope();

	return role;
}

std::shared_ptr<AST::Declaration::COP::Entity> PAR::Parser_Declaration_COP::entity() {
	static const std::string hint =
		"define entity like:"
		"\n  - `entity MyName { ... }`"
		"\n  - with parent `entity MyName : MyParent { ... }`";

	ctx.tok_v.match(TokTy::ENTITY);

	auto def_entity = ctx.Create_Decl<AST::Declaration::COP::Entity>(ctx.tok_v.peek());

	// metacode
	def_entity->isCastable = !ctx.metablock_contains(*def_entity, "nocast");
	def_entity->isExtCastable = !ctx.metablock_contains(*def_entity, "no_extern_cast");

	def_entity->isMoveable = !ctx.metablock_contains(*def_entity, "no_move");
	def_entity->isDestructible = !ctx.metablock_contains(*def_entity, "no_destruct");

	def_entity->id = ctx.p_ref->identifier(def_entity.get());
	ctx.m_sym->add_decl(def_entity);
	ctx.m_sym->enter_scope(def_entity->id.name, EScopeType::Entity);

	ctx.tok_v.expect(TokTy::OPEN_BRACE, "PAR1391",
		"Expected start code block '{' after entity declaration.", hint);

	while (!ctx.tok_v.is_end()) {
		parse_entity_declaration(def_entity);

		if (ctx.match_field_separator(TokTy::S_END_OF_FILE, TokTy::CLOSE_BRACE)) break;
	}

	ctx.m_sym->exit_scope();

	return def_entity;
}

void PAR::Parser_Declaration_COP::parse_entity_declaration(std::shared_ptr<AST::Declaration::COP::Entity> inEntity) {
	static const std::string hint =
		"define comp usage like:\n"
		"\n  - `use name { field1: val1, field2: val2 }`\n"
		"\n  - `use name`";
	static const std::string new_hint =
		"define new entity def like:\n"
		"\n  - `new(params) { ... }`";

	if (ctx.tok_v.match(TokTy::USE)) {
		auto comp_ref = ctx.p_ref->parse_reference();

		if (auto ptr = dynamic_cast<AST::Literal::Component*>(comp_ref.get())) {
			inEntity->comps.push_back(std::unique_ptr<AST::Literal::Component>(ptr));
			comp_ref.release();
		}
		else if (auto ptr = dynamic_cast<AST::Identifier_Reference*>(comp_ref.get())) {
			auto lit_comp = ctx.Create_Node<AST::Literal::Component>(comp_ref.get()->_token);
			lit_comp->id = ptr->id;
			lit_comp->_token = ptr->_token;
			lit_comp->_scope = ptr->_scope;
			comp_ref.release();
			inEntity->comps.push_back(std::move(lit_comp));
		}
		else {
			ctx.tok_v.add_error("PAR1454", "Expected Literal component after 'use' instruction", hint);
		}

		return;
	}
	else if (ctx.tok_v.match(TokTy::NEW)) {
		ctx.m_sym->enter_scope("new", EScopeType::Entity_New);
		auto new_fn_ty = ctx.p_type->explicit_function_proto();
		ctx.tok_v.expect(TokTy::OPEN_BRACE, "PAR1452", "Expected start code '{'.", new_hint);
		
		auto cb = ctx.p_loc->code_block_instruction();

		ctx.m_sym->exit_scope();
		inEntity->constructors.push_back({ std::move(new_fn_ty), std::move(cb) });
		return;
	}
	else if (ctx.tok_v.match(TokTy::OP)) {
		auto op = _entity_op(inEntity);
		inEntity->operators.push_back(std::move(op));
		return;
	}
	else if (ctx.tok_v.match(TokTy::CAST)) {
		auto cast = _entity_cast(inEntity);
		inEntity->casts.push_back(std::move(cast));
		return;
	}

	ctx.tok_v.add_error("PAR1453", "Unexpected '" + ctx.tok_v.peek().val + "' keyword not allowed in entity code block.", "you can define in functions: atribute, method, typealias, operator overloading, trait implementation.");
}

std::shared_ptr<AST::Declaration::COP::Entity_Op> PAR::Parser_Declaration_COP::_entity_op(std::shared_ptr<AST::Declaration::COP::Entity> inEntity) {
	static const std::string hint = "define entity operator overloading like `op + { ... }`.";
	static const std::string hint_index = "define entity index overloading like:\n  - index `op [a] -> T {...}`\n  - range `op [r..] -> Slice<T> {...}`.";
	auto tok = ctx.tok_v.peek();

	ctx.m_sym->enter_scope("op", EScopeType::Entity_Op);

	std::shared_ptr<AST::Declaration::COP::Entity_Op> entity_op;

	// if index operator case op [] -> T { ... }
	if (ctx.tok_v.match(TokTy::OPEN_SQUARE)) {
		auto _op_index = ctx.Create_Decl<AST::Declaration::COP::Entity_OpIndex>(tok);
		_op_index->operatorType = EBinOpType::Index;
		_op_index->parameter_name = ctx.tok_v.expect_id("PAR1331", "Expected index name binding", hint_index);
		

		if (ctx.tok_v.peek().ty == TokTy::DOT && ctx.tok_v.peek(1).ty == TokTy::DOT) {
			_op_index->operatorType = EBinOpType::Slice;
		}

		ctx.tok_v.expect(TokTy::CLOSE_SQUARE, "PAR1332", "Expected closed index operator ']'", hint_index);
		ctx.tok_v.expect(TokTy::ARROW, "PAR1333", "Expected explicit return type '-> T'", hint_index);

		// return type expected for index operator 
		ctx.tok_v.expect(TokTy::ARROW, "PAR1332", "Expected return definition '-> T' after index operator '[]' overload.", hint_index);

		_op_index->return_type = ctx.p_type->parse_type();

		if (_op_index->operatorType == EBinOpType::Slice && _op_index->return_type->get_type() != EPrimType::Slice) {
			ctx.tok_v.expect_id("PAR1334", "Expected type 'Slice<T>' after a range-based index operator.", hint_index);
		}

		entity_op = _op_index;
	}
	// other operator case op + - / * ...
	else {
		auto _op = ctx.Create_Decl<AST::Declaration::COP::Entity_Op>(tok);
		auto op_tok = ctx.tok_v.expect_any(kOperatorTokens, "PAR1333", "Expected operator in entity operator overloading.", hint);
		_op->operatorType = TokTy_to_EBinOpType(op_tok.ty);

		if (ctx.tok_v.check(TokTy::ARROW)) 
			ctx.tok_v.add_error("PAR1332", "Unexpected retrun type definition '-> T' after a entity operator '" + EBinOpType_to_str(_op->operatorType) + "'.", hint);

		entity_op = _op;
	}

	ctx.tok_v.expect(TokTy::OPEN_BRACE, "PAR1334", "Expected start code block '{' after entity operator overloading.", hint);

	entity_op->entity_sym = inEntity;

	entity_op->codeblock = ctx.p_loc->code_block_instruction();

	ctx.m_sym->exit_scope();

	return entity_op;
}

std::shared_ptr<AST::Declaration::COP::Entity_Cast> PAR::Parser_Declaration_COP::_entity_cast(std::shared_ptr<AST::Declaration::COP::Entity> inEntity) {
	static const std::string hint =
		"define entity cast overloading like:"
		"\n  - `cast self as T { ... }`."
		"\n  - `cast T as self { ... }`.";

	auto cast = ctx.Create_Decl<AST::Declaration::COP::Entity_Cast>(ctx.tok_v.peek());

	ctx.m_sym->enter_scope("cast", EScopeType::Entity_Cast);

	auto self_case = [&]() {
		auto self = ctx.Create_Node<AST::Reference::Self>(ctx.tok_v.peek());
		self->source_sym = inEntity;

		return self;
		};

	auto ty_case = [&]() {
		auto ty = ctx.p_type->parse_type();
		auto other_id_ty = ctx.Create_Node<AST::Reference::Other>(ty->_token);
		other_id_ty->resolved_type = std::shared_ptr<AST::AType>(ty.release());
		return ty;
		};

	if (ctx.tok_v.match(TokTy::SELF)) {
		auto self = self_case();
		cast->source = std::move(self);
		cast->isSourceSelf = true;
	}
	else {
		auto id = ty_case();
		cast->target = std::move(id);
	}

	ctx.tok_v.expect(TokTy::AS, "PAR1340", "Expected cast linker 'as' after casting source.", hint);

	if (ctx.tok_v.match(TokTy::SELF)) {
		if (cast->isSourceSelf) {
			ctx.tok_v.add_error("PAR1341", "Expected other type than 'self' in target cast after 'self' in source cast, cast can't be with himself.", hint);
			return nullptr;
		}

		auto self = self_case();
		cast->source = std::move(self);
	}
	else {
		if (!cast->isSourceSelf) {
			ctx.tok_v.add_error("PAR1342", "Expected 'self' in target cast after '" + cast->source->debug_str() + "' in source cast.\n  Cast can't be extern to the concerned entity.", hint);
			return nullptr;
		}

		auto id = ty_case();
		cast->target = std::move(id);
	}

	ctx.tok_v.expect(TokTy::OPEN_BRACE, "PAR1343", "Expected start code block '{' after casting definition.", hint);

	cast->codeblock = ctx.p_loc->code_block_instruction();

	ctx.m_sym->exit_scope();
	return cast;
}

std::shared_ptr<AST::Declaration::COP::System> PAR::Parser_Declaration_COP::system() {
	static const std::string hint =
		"define system like:"
		"\n  - single behaviour "
		"\n   `sys Move() {"
		"\n      Position(pos) + Velocity(vel) => update_physic(pos, vel)"
		"\n  - multiple behaviour "
		"\n   `sys Speak(ref msg: str) {"
		"\n      other => print(\"say \")"
		"\n      ID(id) => { "
		"\n        print(\"(\" + id.name + \"): \" + msg)"
		"\n        return"
		"\n      }"
		"\n      other => print(msg)"
		"\n    }`";

	ctx.tok_v.match(TokTy::SYSTEM);

	auto system = ctx.Create_Decl<AST::Declaration::COP::System>(ctx.tok_v.peek());

	// not handled if (auto where = ctx.p_meta->metacode_where()) system->generic = where.value();

	system->id = ctx.p_ref->identifier(system.get());
	system->prototype = ctx.p_type->explicit_function_proto();
	bool isNoCompUsed = true;

	ctx.tok_v.expect(TokTy::OPEN_BRACE, "PAR1141", "Expected start code block '{' after system declaration.", hint);

	while (!ctx.tok_v.is_end()) {
		ctx.tok_v.expect(TokTy::WITH, "PAR1142", "Expected behaviour block 'with' in system code block.", hint);

		auto sys_case = _system_case();
		sys_case->sys_symbol = system;
		if (!sys_case->bindings.empty()) isNoCompUsed = false;
		system->cases.push_back(sys_case);

		if (ctx.tok_v.check(TokTy::WITH)) continue;
		if (ctx.tok_v.match(TokTy::CLOSE_BRACE)) break;
		ctx.tok_v.add_error("PAR1143",
			"Expected behaviour block 'with' in system code block.",
			hint);
	}

	// pre semantic checking
	// system form is useful or a function is prefered ? (case of no components specifed)
	if (isNoCompUsed) {
		ctx.tok_v.add_error_tok(system->_token, "PAR1144",
			"Expected function instead of system given the behaviour of the code block: no component specified in any where statement.",
			hint);
	}

	return system;
}

std::shared_ptr<AST::Declaration::COP::System_Case> PAR::Parser_Declaration_COP::_system_case() {
	static const std::string hint =
		"define system case like:"
		"\n  - no component (for universal/last/default behaviour)"
		"\n   `other => { Println(\"NPC don't have any item!\") }`"
		"\n  - single component (for single operation most of the time)"
		"\n   `CCounter(c) => add_count(c)`"
		"\n  - multiple components (for interaction between components most of the time)"
		"\n   `CPosition(pos) + CVelocity(vel) => update_move(pos, vel)";

	auto sys_case = ctx.Create_Decl<AST::Declaration::COP::System_Case>(ctx.tok_v.peek());
	ctx.tok_v.match(TokTy::WITH);

	while (!ctx.tok_v.is_end()) {
		auto bind = ctx.Create_Decl<AST::Declaration::Local::Variable_Binding>(ctx.tok_v.peek());
		bind->resolved_ty = std::shared_ptr<AST::AType>(ctx.p_type->parse_type().release());
		ctx.tok_v.expect(TokTy::OPEN_PAREN, "PAR1150", "Expected start binding '(' after component name pattern.", hint);
		bind->id = ctx.p_ref->identifier();
		ctx.tok_v.expect(TokTy::OPEN_PAREN, "PAR1150", "Expected end binding ')' after component name pattern.", hint);

		ctx.m_sym->add_decl(bind);
		sys_case->bindings.push_back(bind);

		if (ctx.match_field_separator(TokTy::OP_PLUS, TokTy::OPEN_BRACE)) break;
	}

	sys_case->codeblock = ctx.p_loc->code_block_instruction();

	for (auto& instruction : sys_case->codeblock->elements) {
		if (dynamic_cast<AST::Statement::Return*>(instruction.node())) {
			sys_case->isReturn = true;
			break;
		}
	}

	return sys_case;
}

