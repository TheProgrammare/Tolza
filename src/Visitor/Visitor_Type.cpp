
#include "Visitor_Type.hpp"

#include "Visitor_Symbol.hpp"
#include "AST/AST_Nodes.hpp"


std::optional<AST::AType*> VisitorType::resolve_ty(AST::Node& n, AST::AType* targetNode, bool silentError) {
	const std::string error =
		"Type for node: '" + n.debug_str() + "',"
		"\n  targeting type: '" + targetNode->get_node()->debug_str() + "'"
		"\n  not found.";
	const std::string error_target =
		"Symbol for node: '" + n.debug_str() + "',"
		"\n  targeting type: '" + targetNode->get_node()->debug_str() + "'"
		"\n  target not compatible.";

	if (auto ty = get_type(&n)) {
		if (ty.has_value() && typeid(ty.value()) == typeid(*targetNode)) {
			targetNode = ty.value();
			return ty;
		}
		if (!silentError) error_add(n, "TYP1500", error_target, SYM_HINT);
		return std::nullopt;
	}
	if (!silentError) error_add(n, "TYP1500", error, SYM_HINT);
	return std::nullopt;
}

void VisitorType::visit(AST::Decl_LocalVar& n) {
	VisitorDefault::visit(n);
	// type already defined
	if (n.resolved_ty || n.ty) return;
	// type inferred
	if (n.expression) {
		// affect expression type to the variable
		if (resolve_ty(n, n.resolved_ty)) return;

		error_add(n, "TYP1000", "Expected typed expression.", SYM_HINT);
	}
	else {
		error_add(
			n, "TYP1001",
			"Expected expression after inferred type variable.",
			"define variable like:"
			"\n  - explicit type `var a: i32` or `var a: i32 = 10`"
			"\n  - inferred type `var a = 10");
	}
}

void VisitorType::visit(AST::Decl_GlobalVar& n) {
	VisitorDefault::visit(n);
	// type already defined
	if (n.ty) return;
	// type inferred
	if (n.expression) {
		if (resolve_ty(n, n.ty.get())) return;

		error_add(n, "TYP1000", "Expected typed expression.", SYM_HINT);
	}
	else {
		error_add(
			n, "TYP1011",
			"Expected expression after inferred type variable.",
			"define variable like:"
			"\n  - explicit type `var a: i32` or `var a: i32 = 10`"
			"\n  - inferred type `var a = 10");
	}
}

void VisitorType::visit(AST::UnpackVar& n) {
	VisitorDefault::visit(n);

	// resolve unpack
	if (auto call = dynamic_cast<AST::ICallable*>(n.call->get_symbol_resolution())) {
		auto& return_call_ty = call->get_signature()->returnType->types;
		size_t tySize = return_call_ty.size();

		if (tySize != n.var_names.size()) {
			error_add(n, "TYP1020",
				"Not the same size of variables (" + std::to_string(n.var_names.size()) + ") unpacked and call types returned (" + std::to_string(tySize) + ").",
				"Makes sure there is the same amount of unpacked variables and call types returned.");
			return;
		}

		// get type from return and assign types to variables
		size_t counter = 0;
		for (auto& var_ty : n.var_names) {
			if (var_ty) {
				var_ty->resolved_ty = return_call_ty[counter].get();
			}
			counter++;
		}
	}
	else {
		error_add(*n.call, "TYP1021", "Expected a callable symbol reference.", SYM_HINT);
	}
}

void VisitorType::visit(AST::Op_Index& n) {
	VisitorDefault::visit(n);
	bool isRangeOp = false;
	if (auto ptr = dynamic_cast<AST::Lit_Range*>(n.selector.get())) {
		bool isRangeOp = true;
	}
	else if (auto ptr = dynamic_cast<AST::Id_Ref*>(n.selector.get())) {
		ptr->get_symbol_resolution();
	}
}

void VisitorType::visit(AST::Op_Binary& n) {
	VisitorDefault::visit(n);
	auto leftTy = get_type(n.left.get());
	auto rightTy = get_type(n.right.get());

	if (!leftTy.has_value() || !rightTy.has_value()) {
		error_add(n, "TYP1040",
			"Type not found for binary term",
			"`++i`"
			"\n  - Did you use correct identifiers ?"
			"\n  - Did you use correct literal ?"
			"\n  - Did you use correct operation ?");
		return;
	}

	auto [ty, err, hint] = get_binOp_type(leftTy.value(), rightTy.value(), n.op);
	if (ty) n.resolved_type = ty;
	else	error_add(n, "TYP1041", err, hint);
}

void VisitorType::visit(AST::Op_Unary& n) {
	VisitorDefault::visit(n);
	auto innerTy = get_type(n.base.get());

	if (!innerTy.has_value()) {
		error_add(n, "TYP1050",
			"Type not found for unary term.",
			"`a + b`"
			"\n  - Did you use correct identifiers ?"
			"\n  - Did you use correct literal ?"
			"\n  - Did you use correct operation ?");
		return;
	}

	auto [result, err, hint] = check_unaryOp(innerTy.value(), n.unitaryOp);
	
	if (!result) error_add(n, "TYP1051", err, hint);
}

void VisitorType::visit(AST::Cond_Interval& n) {
	VisitorDefault::visit(n);
	auto leftTy = get_type(n.left.get());
	auto centerTy = get_type(n.center.get());
	auto rightTy = get_type(n.right.get());

	if (!leftTy.has_value() || !centerTy.has_value() || !rightTy.has_value()) {
		error_add(n, "TYP1060", "Type not found for interval term", "`a < b < c`" + SYM_HINT);
		return;
	}

	auto [l_ty, l_err, l_hint] = get_binOp_type(leftTy.value(), centerTy.value(), n.left_comparator);
	auto [r_ty, r_err, r_hint] = get_binOp_type(centerTy.value(), rightTy.value(), n.right_comparator);

	if (!l_ty) error_add(n, "TYP1061", l_err, l_hint);
	if (!r_ty) error_add(n, "TYP1062", r_err, r_hint);
}

void VisitorType::visit(AST::Cast_As& n) {
	VisitorDefault::visit(n);
	auto valTy = get_type(n.valueCasted.get());

	if (!valTy.has_value()) {
		error_add(n, "TYP1070", "Type not found for the value casted", "`val as type`" + SYM_HINT);
		return;
	}

	auto [result, castTo, err, hint] = check_explicit_cast(valTy.value(), n.typeCasted.get());

	if (!result) error_add(n, "TYP1071", err, hint);
}

void VisitorType::visit(AST::Lit_Entity& n) {
	VisitorDefault::visit(n);
	static const std::string hint =
		"define entity litteral like: `Person{ CIdentity{ name: \"Zagreus\", age: 25 } }`";

	std::unordered_map<std::string, AST::Ref_Lit_Comp*> comp_args;
	std::unordered_map<std::string, AST::Ref_Lit_Comp*> comp_tys;

	for (auto& comp : n.resolved_sym->comps) {
		comp_tys.insert({ comp->resolved_sym->decl_mangling(), comp.get() });
	}
	
	for (auto& elem : n.comp_args) {
		std::string comp_arg_name = elem->resolved_sym->decl_mangling();

		if (auto comp_ty = comp_tys.find(comp_arg_name)->second) {
			if (comp_args.contains(comp_arg_name)) {
				error_add(
					n, "TYP1080",
					"Component '" + comp_arg_name + "' is already invoked in literal entity.",
					hint);
			}

			std::string ty_name = comp_ty->resolved_sym->decl_mangling();

			if (comp_arg_name == ty_name) {
				comp_args.insert({ elem->id.mangle_local_name(), elem.get()});
			}

			error_add(*elem,
				"TYP1081",
				"The invoked component '" + comp_arg_name + "' is not the same type of '" + ty_name + "'",
				hint);
		}
		else {
			error_add(
				n, "TYP1081",
				"Component '" + elem->id.debug_str() + "' doesn't used in entity '" + n.id.debug_str() + "'",
				""
				"\n  - Did you use correct entity ?"
				"\n  - Did you use correct components ?");
		}
	}
}


void VisitorType::visit(AST::Mem_GetBits& n) {
	VisitorDefault::visit(n);

	// lit range already defined
	if (n.resolved_range_ty) return;

	if (resolve_ty(*n.target.get(), n.resolved_range_ty)) return;

	error_add(
		n, "TYP1090",
		"Type not found for the range argument",
		"`target~[a]` or `target~[0..8]`"
		"\n  - Did you use correct identifiers ?"
		"\n  - Did you use correct literal range ?");
}

void VisitorType::visit(AST::Mem_Ref& n) {
	VisitorDefault::visit(n);
	if (auto ty = get_type(n.target.get())) {
		// &a have the type : ptr'a
		auto ptr_ty = new AST::Ty_Ptr;
		ptr_ty->inner = std::unique_ptr<AST::AType>(ty.value());
		n.resolved_type = ptr_ty;
		return;
	}

	error_add(
		n, "TYP1090",
		"Type not found for the address of variable.",
		"`&a`"
		"\n  - Did you use correct identifiers ?"
		"\n  - Did you use correct type ?");
}

void VisitorType::visit(AST::Mem_Val& n) {
	VisitorDefault::visit(n);
	if (resolve_ty(n, n.resolved_type)) return;

	error_add(n, "TYP1100", "Type not found for the value of ", SYM_HINT);
}

/*
void TypeVisitor::visit(AST::Mem_Align& n) {
	VisitorDefault::visit(n);
	if (auto ty = get_type(n.target.get())) {
		n.align = ty.value()->typeAlignment();
	}
}

void TypeVisitor::visit(AST::Mem_Size& n) {
	VisitorDefault::visit(n);
	if (auto ty = get_type(n.target.get())) {
		n.size = ty.value()->typeSizeByte();
	}
}
*/

void VisitorType::visit(AST::Get_Expr_Type& n) {
	VisitorDefault::visit(n);
	resolve_ty(n, n.resolved_sym);
}

/*
void TypeVisitor::visit(AST::Lit_Tuple& n) {
	VisitorDefault::visit(n);
	n.tys_size.reserve(n.values.size());
	n.tys_padding.reserve(n.values.size());

	size_t offset = 0;
	n.ty_alignment = 1;

	for (const auto& val_ty : n.values) {
		if (auto ty = get_type(val_ty.get())) {
			size_t size = ty.value()->typeSizeByte();
			size_t align = 1;

			// Calculate alignment = greater power of 2 <= size
			while (align * 2 <= size) align *= 2;

			// Update global alignment
			if (align > n.ty_alignment) n.ty_alignment = align;

			// Padding required to align this element
			size_t padding = (align - (offset % align)) % align;

			// Save
			n.tys_padding.push_back(padding);
			n.tys_size.push_back(size);

			offset += padding + size;
		}
	}

	// Final padding for the structure is multiple of his alignment
	size_t final_padding = (n.ty_alignment - (offset % n.ty_alignment)) % n.ty_alignment;
	n.ty_sizeByte = offset + final_padding;
}
*/

void VisitorType::visit(AST::Lit_Table& n) {
	VisitorDefault::visit(n);
	std::string base_ty_mangling;

	bool first = true;
	for (auto& elem : n.values) {
		if (auto ptr = get_type(elem.get())) {
			const std::string elem_mangling = ptr.value()->ty_mangling();
			
			if (base_ty_mangling.empty()) {
				base_ty_mangling = elem_mangling;
			}
			else if (base_ty_mangling != elem_mangling) {
				error_add(*elem, "TYP1110",
					"Literal table contain different type.",
					"Different type in table is prohibied"
					"\n  - Did you use correct type ?"
					"\n  - Did you need a tuple instead ?");
			}

			if (first) {
				n.resolved_elem_ty = ptr.value();
				first = false;
			}

			continue;
		}
		error_add(*elem, "TYP1111",
			"Type not found for the table element",
			"\n  - Did you use correct identifiers ?"
			"\n  - Did you use correct type ?");
	}

	if (n.population) {
		for (auto& range : n.population->ranges) {
			if (auto range_ptr = dynamic_cast<AST::Lit_Range*>(range.get())) {
				
			}
			else if (auto range_ty = dynamic_cast<AST::Id_Ref*>(range.get())) {

			}
		}

		resolve_ty(*n.population->expression, n.population->resolved_elem_ty);
	}
}

void VisitorType::visit(AST::Table_Population &n) {

}

void VisitorType::visit(AST::Lit_Map& n) {
	VisitorDefault::visit(n);
	if (!resolve_ty(*n.keys[0].get(), n.resolved_key_ty)) {
		error_add(n, "TYP1120", "type not found for the key '" + n.keys[0]->debug_str() + "'.", SYM_HINT);
	}
	if (!resolve_ty(*n.values[0].get(), n.resolved_value_ty)) {
		error_add(n, "TYP1120", "type not found for the value '" + n.values[0]->debug_str() + "'.", SYM_HINT);
	}

	//n.ty_sizeByte = n.resolved_key_ty->typeSizeByte() + n.resolved_value_ty->typeSizeByte();
}

void VisitorType::visit(AST::Lit_Range& n) {
	VisitorDefault::visit(n);
	bool haveError = false;
	if (!resolve_ty(*n.start.get(), n.resolved_type_start)) {
		error_add(n, "TYP1120", "type not found for the start range '" + n.start->debug_str() + "'.", SYM_HINT);
		haveError = true;
	}
	if (!resolve_ty(*n.end.get(), n.resolved_type_end)) {
		error_add(n, "TYP1120", "type not found for the end range '" + n.end->debug_str() + "'.", SYM_HINT);
		haveError = true;
	}

	if (!haveError && typeid(n.resolved_type_start) != typeid(n.resolved_type_end)) {
		error_add(n, "TYP1131",
			"Range must reference the same type between start ('" + n.resolved_type_start->get_node()->debug_str() + "') and end ('" + n.resolved_type_end->get_node()->debug_str() + "').",
			"\n  - Did you use the same literal between start and end ?"
			"\n  - Did you use the same variable source between start and end ?");
		return;
	}

	n.resolved_type = n.resolved_type_start;
}

void VisitorType::visit(AST::State_For& n) {
	if (auto src_ty = get_type(n.src.get())) {
		// for loop on map
		if (n.keyName) {
			if (auto map_ptr = dynamic_cast<AST::Lit_Map*>(src_ty.value())) {
				n.resolved_item_ty = map_ptr->resolved_value_ty;
				n.resolved_key_ty = map_ptr->resolved_key_ty;
			}
			else {
				error_add(*n.src, "TYP1140", "Expected map-like type after a for loop with key defined", SYM_HINT);
			}
		}
		else if (auto range = dynamic_cast<AST::Lit_Range*>(src_ty.value())) {
			n.resolved_item_ty = range->resolved_type;
		}
		else if (auto entity = dynamic_cast<AST::Decl_Entity*>(src_ty.value())) {
			bool iteratorFound = false;
			for (auto& op : entity->operators) {
				if (op->operatorType == EOpType::Iter) {
					iteratorFound = true;
					break;
				}
			}
		}
		else if (auto array = dynamic_cast<AST::Lit_Table*>(src_ty.value())) {

		}
		else {
			error_add(*n.src, "TYP1141", "Expected iterable type for the for loop source.", SYM_HINT);
		}
		// for loop on range / array / entity
		n.resolved_item_ty = src_ty.value();
	}
	else {
		error_add(*n.src, "TYP1142", "Type not found for the for loop source", SYM_HINT);
	}
}

void VisitorType::visit(AST::Entity_Op& n) {
	switch (n.operatorType)
	{
	case EOpType::Add: case EOpType::Sub: case EOpType::Mul: case EOpType::Div: 
	case EOpType::Mod: case EOpType::Quo: case EOpType::Pow: {
		n.resolved_result_type = n.entity_sym;
		return;
	}
	case EOpType::Incr: case EOpType::Decr: case EOpType::Sign: {
		n.resolved_result_type_isRef = true;
		n.resolved_result_type = n.entity_sym;
		return;
	}
	case EOpType::Index: {
		if (auto ptr = dynamic_cast<AST::Entity_OpIndex*>(&n)) {
			n.resolved_result_type = ptr->resolved_result_type;
			return;
		}
	}
	case EOpType::Slice: case EOpType::Iter:
		return;
	case EOpType::Gre: case EOpType::Low: case EOpType::Gre_eq: case EOpType::Low_eq:
	case EOpType::_eq: case EOpType::_neq: case EOpType::_eqs: case EOpType::_not: case EOpType::_and:
	case EOpType::_nand: case EOpType::_or: case EOpType::_xor: case EOpType::_nor: case EOpType::_xnor: {
		auto boolNodeTy = new AST::Ty_Primitive;
		boolNodeTy->_type = EPrimType::boolean;
		n.resolved_result_type = boolNodeTy;
		return;
	}
	case EOpType::_b_and: case EOpType::_b_nand: case EOpType::_b_or: 
	case EOpType::_b_xor: case EOpType::_b_nor: case EOpType::_b_xnor: {
		n.resolved_result_type = n.entity_sym;
		return;
	}
	case EOpType::COUNT:
		return;
	default:
		return;
	}
}

void VisitorType::visit(AST::Ref_Other& n) {
	resolve_ty(n, n.resolved_type);
}

std::optional<AST::AType*> get_type(AST::Node* n)
{
	static auto build_bool = [&]() {
		auto boolNodeTy = new AST::Ty_Primitive;
		boolNodeTy->_type == EPrimType::boolean;
		return boolNodeTy;
		};

	if (auto direct_ty = dynamic_cast<AST::Type*>(n))			return direct_ty;

	if (auto ptr = dynamic_cast<AST::Op_Index*>(n))				return get_type(ptr->target.get());
	if (auto ptr = dynamic_cast<AST::Op_Binary*>(n))			return ptr->resolved_type;
	if (auto ptr = dynamic_cast<AST::Op_Unary*>(n))				return get_type(ptr->base.get());

	if (auto ptr = dynamic_cast<AST::Inst_In*>(n))				return build_bool();
	if (auto ptr = dynamic_cast<AST::Inst_Is*>(n))				return build_bool();
	if (auto ptr = dynamic_cast<AST::Ref_Call*>(n))				return get_type(ptr->resolved_sym);
	if (auto ptr = dynamic_cast<AST::Ref_PipeCall*>(n))			return get_type(ptr->resolved_sym);
	if (auto ptr = dynamic_cast<AST::Inst_Return*>(n))			return ptr->resolved_sym->returnType.get();

	if (auto ptr = dynamic_cast<AST::Lit_Entity*>(n))		return ptr->resolved_sym;
	if (auto ptr = dynamic_cast<AST::Entity_Cast*>(n))			return ptr->target.get();
	if (auto ptr = dynamic_cast<AST::Entity_Op*>(n))			return ptr->resolved_result_type;

	if (auto ptr = dynamic_cast<AST::Cond_Interval*>(n))		return build_bool();

	if (auto ptr = dynamic_cast<AST::Ternary_If*>(n))			return get_type(ptr->true_line.get());

	if (auto ptr = dynamic_cast<AST::Cast_As*>(n))				return ptr->typeCasted.get();

	if (auto ptr = dynamic_cast<AST::Decl_System*>(n))			return ptr->ty.get();
	if (auto ptr = dynamic_cast<AST::Decl_LocalVar*>(n))		return ptr->ty.get();
	if (auto ptr = dynamic_cast<AST::Decl_GlobalVar*>(n))		return ptr->ty.get();

	if (auto ptr = dynamic_cast<AST::Type_Reference*>(n))			return get_type(ptr->resolved_sym);

	if (auto ptr = dynamic_cast<AST::Ref_Self*>(n))			return ptr->source_sym;
	if (auto ptr = dynamic_cast<AST::Ref_Other*>(n))			return ptr->resolved_type;

	return std::nullopt;
}

std::tuple<bool, std::string, std::string> check_unaryOp(AST::Type* inner, EOpType opTy)
{
	// if entity
	if (auto ptr = dynamic_cast<AST::Decl_Entity*>(inner)) {
		for (auto& op : ptr->operators) {
			if (op->operatorType == opTy) return { true, "", "" };
		}

		return {
			false,
			"Entity '" + ptr->id.debug_str() + "' not overload the operator '" + opToStr(opTy) + "'.",
			""
			"\n  - Did you use the correct entity ?"
			"\n  - Did you overload correctly the operator ?"
		};
	}

	// if type handle unary op
	if (auto ptr = dynamic_cast<AST::Lit*>(inner->get_node())) {
		if (op_primitive[static_cast<uint8_t>(ptr->inferred_ty)][static_cast<uint8_t>(opTy)]) {
			return { true, "", "" };
		}
	}


	return {
		false,
		inner->get_node()->debug_str() + " not overload the operator '" + opToStr(opTy) + "'.",
		""
			"\n  - Did you use the correct type ?"
			"\n  - Did you use the correct operator ?"
	};
}

std::tuple<AST::Type*, std::string, std::string> get_binOp_type(AST::Type* left, AST::Type* right, EOpType opTy)
{
	using PT = EPrimType;

	// if operation on entities
	if (left->ty_mangling() == right->ty_mangling()) {
		if (auto ptr = dynamic_cast<AST::Decl_Entity*>(left)) {
			for (auto& op : ptr->operators) {
				if (op->operatorType == opTy) return { left, "", "" };
			}

			return {
				nullptr,
				"Entity '" + ptr->id.debug_str() + "' not overload the operator '" + opToStr(opTy) + "'.",
				""
				"\n  - Did you use the correct entity ?"
				"\n  - Did you overload correctly the operator ?"
			};
		}
		else {
			return { left, "", "" };
		}
	}

	auto isFloat = [](PT ty) {		return ty == PT::f32 || ty == PT::f64 || ty == PT::f128; };
	auto isInteger = [](PT ty) {	return ty == PT::i8 || ty == PT::i16 || ty == PT::i32 || ty == PT::i64 || ty == PT::i128 || ty == PT::iSize; };
	auto isUnsigned = [](PT ty) {	return ty == PT::u8 || ty == PT::u16 || ty == PT::u32 || ty == PT::u64 || ty == PT::u128 || ty == PT::uSize; };
	auto isBinary = [](PT ty) {	return ty == PT::b8 || ty == PT::b16 || ty == PT::b32 || ty == PT::b64 || ty == PT::b128 || ty == PT::bSize; };
	auto isText = [](PT ty) {		return ty == PT::text || ty == PT::str; };

	bool validBinOp = false;
	if (auto l_ptr = dynamic_cast<AST::Lit*>(left->get_node()); auto r_ptr = dynamic_cast<AST::Lit*>(right->get_node())) {
		if (
			isFloat(l_ptr->inferred_ty) == isFloat(r_ptr->inferred_ty) ||
			isInteger(l_ptr->inferred_ty) == isInteger(r_ptr->inferred_ty) ||
			isUnsigned(l_ptr->inferred_ty) == isUnsigned(r_ptr->inferred_ty) ||
			isBinary(l_ptr->inferred_ty) == isBinary(r_ptr->inferred_ty) ||
			isText(l_ptr->inferred_ty) == isText(r_ptr->inferred_ty)
		) {
			// check if operator is handled
			if (auto ptrl = dynamic_cast<AST::Lit*>(left); auto ptrr = dynamic_cast<AST::Lit*>(right)) {
				if (op_primitive[static_cast<uint8_t>(opTy)][static_cast<uint8_t>(ptrl->inferred_ty)]) {
					// return grater type
					return { ptrl->inferred_ty > ptrr->inferred_ty ? left : right, "", "" };
				}
			}
			
			return {
				nullptr,
				"Operation between '" + left->get_node()->debug_str() + "' and '" + right->get_node()->debug_str() + "' not overlad the operator '" + opToStr(opTy) + "'.",
				""
				"\n  - Did you use the correct type ?"
				"\n  - Did you use the correct operation ?"
			};
		}
	}

	return {
		nullptr,
		"Operation between '" + left->get_node()->debug_str() + "' and '" + right->get_node()->debug_str() + "' is prohibied.",
		"Cast explicitly operation terms to have the desired behaviour like:"
		"\n  - `let a: i32 = 10 + 2.6 as i32::round`"
		"\n  - `let a: i32 = 10 + 2.6 as i32` (truncate)"
	};
}

std::tuple<bool, bool, std::string, std::string> check_explicit_cast(AST::Type* valTy, AST::Type* targetTy)
{

	// if cast on entity
	if (auto val_ptr = dynamic_cast<AST::Decl_Entity*>(valTy)) {
		for (auto& cast : val_ptr->casts) {
			if (cast->target->ty_mangling() == targetTy->ty_mangling()) return { true, true, "", "" };
		}

		// if entity casted don't have a cast
		// check if the target type is an entity and have a cast
		if (auto target_ptr = dynamic_cast<AST::Decl_Entity*>(targetTy)) {
			for (auto& cast : target_ptr->casts) {
				if (cast->target->ty_mangling() == valTy->ty_mangling()) return { true, false, "", "" };
			}

			return {
				false,
				false,
				"Entity '" + val_ptr->id.debug_str() + "' not overload the cast from and to '" + targetTy->get_node()->debug_str() + "'.",
				""
				"\n  - Did you use the correct entity ?"
				"\n  - Did you overload correctly the cast ?"
			};
		}

		return {
			false,
			false,
			"Entity '" + val_ptr->id.debug_str() + "' not overload the cast to '" + targetTy->get_node()->debug_str() + "'.",
			""
			"\n  - Did you use the correct entity ?"
			"\n  - Did you overload correctly the cast ?"
		};
	}

	// if cast on native type
	if (auto ptr_val = dynamic_cast<AST::Lit*>(valTy); auto ptr_target = dynamic_cast<AST::Lit*>(targetTy)) {
		if (auto result = cast_explicit[static_cast<int>(ptr_val->inferred_ty)][static_cast<int>(ptr_target->inferred_ty)]) {
			return { true, true, "", "" };
		}
	}
	
	return {
		false,
		false,
		"Type '" + valTy->get_node()->debug_str() + "' can't be explicitly casted to type '" + targetTy->get_node()->debug_str() + "'.",
		""
		"\n  - Did you use the correct variable to cast ?"
		"\n  - Did you use the correct variable type ?"
		"\n  - Did you cast to the correct type ?"
	};
}
