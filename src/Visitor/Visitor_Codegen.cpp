#include "Visitor_Codegen.hpp"

#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>



/*
Value* AST::Lit_BoolAST::codegen() const {
	return ConstantInt::get(Type::getInt1Ty(infoScript.context), value);
}

Value* AST::Lit_EnumAST::codegen() const {
	return nullptr;
}

Value* AST::Lit_IntAST::codegen() const {
	// type size deduction

	switch (integralTy)
	{
	case TokTy::T_I8:		return ConstantInt::get(Type::getInt8Ty(infoScript.context), APInt(8, value, true));
	case TokTy::T_I16:		return ConstantInt::get(Type::getInt16Ty(infoScript.context), APInt(16, value, true));
	case TokTy::T_I32:		return ConstantInt::get(Type::getInt32Ty(infoScript.context), APInt(32, value, true));
	case TokTy::T_I64:		return ConstantInt::get(Type::getInt64Ty(infoScript.context), APInt(64, value, true));
	case TokTy::T_I128: {
		// to pass int128 type with 2 int64_t
		APInt value128(128, 0, true);
		value128 = value128.zext(128);
		value128 = value128 | APInt(128, value).shl(64);
		value128 |= APInt(128, low_128);
		return ConstantInt::get(Type::getInt128Ty(infoScript.context), value128);
	}
	case TokTy::T_U8:		return ConstantInt::get(Type::getInt8Ty(infoScript.context), APInt(8, value, false));
	case TokTy::T_U16:		return ConstantInt::get(Type::getInt16Ty(infoScript.context), APInt(16, value, false));
	case TokTy::T_U32:		return ConstantInt::get(Type::getInt32Ty(infoScript.context), APInt(32, value, false));
	case TokTy::T_U64:		return ConstantInt::get(Type::getInt64Ty(infoScript.context), APInt(64, value, false));
	case TokTy::T_U128: {
		// to pass int128 type with 2 int64_t
		APInt value128(128, 0, false);
		value128 = value128.zext(128);
		value128 = value128 | APInt(128, value).shl(64);
		value128 |= APInt(128, low_128);
		return ConstantInt::get(Type::getInt128Ty(infoScript.context), value128);
	}
	default:
		break;
	}


}

Value* AST::Lit_FloatAST::codegen() const {
	switch (_token.type)
	{
	case TokTy::T_F32:	return ConstantFP::get(Type::getFloatTy(infoScript.context), APFloat(value));
	case TokTy::T_F64:  return ConstantFP::get(Type::getDoubleTy(infoScript.context), APFloat(value));
	default:
		break;
	}
}

Value* AST::Lit_StrAST::codegen() const {
	return ConstantDataArray::getString(infoScript.context, value, true);
}

Value* AST::Inst_TupleAST::codegen() const {
	std::vector<Value*> elems;
	std::vector<Type*> types;

	auto tuple_id = ConstantInt::get(Type::getInt8Ty(infoScript.context), APInt(8, static_cast<int>(TokTy::T_TUPLE)));

	types.push_back(tuple_id->getType());
	elems.push_back(tuple_id);

	for (auto& val : values) {
		Value* elemVal = val->codegen();
		Type* elemTy = elemVal->getType();
		if (!elemVal) {
			return nullptr;
		}
		elems.push_back(elemVal);
		types.push_back(elemTy);
	}

	StructType* tupleType = StructType::get(infoScript.context, types);

	Value* tupleAlloc = UndefValue::get(tupleType);

	for (size_t i = 0; i < elems.size(); ++i) {
		tupleAlloc = infoScript.builder.CreateInsertValue(tupleAlloc, elems[i], i, "elem_" + std::to_string(i));
	}

	return tupleAlloc;
}

std::string AST::Inst_TupleAST::mangling() const
{
	return std::string();
}

Value* AST::Inst_BinOpAST::codegen() const {
	// Ensure that the left and right nodes are generated first
	Value* leftVal = left->codegen();
	Value* rightVal = right->codegen();

	if (!leftVal || !rightVal) {
		return nullptr;
	}

	switch (op) {
	case TokTy::OP_PLUS:
		return infoScript.builder.CreateAdd(leftVal, rightVal, "add_tmp");
	case TokTy::OP_MINUS:
		return infoScript.builder.CreateSub(leftVal, rightVal, "sub_tmp");
	case TokTy::OP_ASTERISK:
		return infoScript.builder.CreateMul(leftVal, rightVal, "mul_tmp");
	case TokTy::OP_DIVIDE:
		return infoScript.builder.CreateSDiv(leftVal, rightVal, "div_tmp");
	case TokTy::OP_MODULO:
		return infoScript.builder.CreateSRem(leftVal, rightVal, "modr_tmp");
	case TokTy::OP_POWER: {
		Function* powFunc = Intrinsic::getOrInsertDeclaration(
			infoScript.builder.GetInsertBlock()->getParent()->getParent(),
			Intrinsic::pow, { leftVal->getType(), rightVal->getType() }
		);
		return infoScript.builder.CreateCall(powFunc, { leftVal, rightVal }, "pow_tmp");
	}
	case TokTy::OP_QUOTIEN: {
		// left to int
		if (!leftVal->getType()->isIntegerTy()) {
			leftVal = infoScript.builder.CreateIntCast(leftVal, Type::getInt32Ty(infoScript.context), true, "lhs_cast");
		}
		// right to int
		if (!rightVal->getType()->isIntegerTy()) {
			rightVal = infoScript.builder.CreateIntCast(rightVal, Type::getInt32Ty(infoScript.context), true, "rhs_cast");
		}
		return infoScript.builder.CreateSDiv(leftVal, rightVal, "modq_tmp");
	}
							  // AND operation (bitwise AND)
	case TokTy::AND:
	case TokTy::B_AND: {
		return infoScript.builder.CreateAnd(leftVal, rightVal, "and_tmp");
	}
					 // NAND operation (NOT AND)
	case TokTy::NAND:
	case TokTy::B_NAND: {
		Value* andResult = infoScript.builder.CreateAnd(leftVal, rightVal, "and_tmp");
		return infoScript.builder.CreateNot(andResult, "nand_tmp");
	}
					  // OR operation (bitwise OR)
	case TokTy::OR:
	case TokTy::B_OR: {
		return infoScript.builder.CreateOr(leftVal, rightVal, "or_tmp");
	}
					// XOR operation (exclusive OR)
	case TokTy::XOR:
	case TokTy::B_XOR: {
		return infoScript.builder.CreateXor(leftVal, rightVal, "xor_tmp");
	}
					 // NOR operation (NOT OR)
	case TokTy::NOR:
	case TokTy::B_NOR: {
		Value* orResult = infoScript.builder.CreateOr(leftVal, rightVal, "or_tmp");
		return infoScript.builder.CreateNot(orResult, "nor_tmp");
	}
					 // XNOR operation (NOT XOR)
	case TokTy::XNOR:
	case TokTy::B_XNOR: {
		Value* xorResult = infoScript.builder.CreateXor(leftVal, rightVal, "xor_tmp");
		return infoScript.builder.CreateNot(xorResult, "xnor_tmp");
	}
					  // NOT operation (bitwise NOT)
	case TokTy::NOT:
	case TokTy::B_NOT: {
		return infoScript.builder.CreateNot(leftVal, "not_tmp");
	}
					 // Default case if the operator is not recognized
	default:
		return nullptr;
	}
}

void AST::Inst_BinOpAST::resolve_type(const std::weak_ptr<ScriptInfo> scr_info) {
	switch (op) {
		// comparison / logical
	case ETokenType::IS:
	case ETokenType::USE:
	case ETokenType::IN:
	case ETokenType::OPEN_BRACKETS:
	case ETokenType::CLOSE_BRACKETS:
	case ETokenType::AND:
	case ETokenType::NAND:
	case ETokenType::OR:
	case ETokenType::XOR:
	case ETokenType::NOR:
	case ETokenType::XNOR:
	case ETokenType::NOT:
	case ETokenType::B_AND:
	case ETokenType::B_NAND:
	case ETokenType::B_OR:
	case ETokenType::B_XOR:
	case ETokenType::B_NOR:
	case ETokenType::B_XNOR:
	case ETokenType::B_NOT:
	case ETokenType::OP_EQUAL:
	case ETokenType::OP_NOT_EQUAL:
	case ETokenType::OP_GREATER_EQUAL:
	case ETokenType::OP_LOWER_EQUAL: {
		resolved_type = new AST::Ty_PrimitiveAST(ETokenType::T_BOOL);
		return;
	}
									 // Op�rateurs arithm�tiques - simplifi� (on suppose que left et right ont d�j� leur type r�solu)
	case ETokenType::OP_PLUS:
	case ETokenType::OP_MINUS:
	case ETokenType::OP_ASTERISK:
	case ETokenType::OP_DIVIDE:
	case ETokenType::OP_MODULO:
	case ETokenType::OP_QUOTIEN: {
		if (!left || !right) {
			// erreur, pas d'op�randes
			resolved_type = nullptr;
			return;
		}

		if (auto leftIType = dynamic_cast<ITypeResolution*>(left.get())) {
			leftIType->resolve_type();
			// ...
		}


		// Sinon, faire une promotion de type simplifi�e, ex:
		// bool < int < float < double, etc.
		// Pour simplifier, on prend ici float si l'un est float, int sinon
		if (left_native->type == ETokenType::T_FLOAT || right_native->type == ETokenType::T_FLOAT) {
			resolved_type = new AST::Ty_PrimitiveAST(ETokenType::T_FLOAT);
			return;
		}

		// Par d�faut, on choisit int
		resolved_type = new AST::Ty_PrimitiveAST(ETokenType::T_INT);
		return;
	}

										// Op�rateurs d'assignation - le type r�sultant est celui de la gauche
	case ETokenType::ASSIGN_PLUS:
	case ETokenType::ASSIGN_MINUS:
	case ETokenType::ASSIGN_MULTIPLY:
	case ETokenType::ASSIGN_DIVIDE:
	case ETokenType::ASSIGN_MODULO:
	case ETokenType::ASSIGN_QUOTIEN: {
		if (!left) {
			resolved_type = nullptr;
			return;
		}
		resolved_type = left->get_type();
		return;
	}

											   // Op�rateurs unaires comme OP_INCREMENT, OP_DECREMENT, NOT, etc. (� adapter)
	case ETokenType::OP_INCREMENT:
	case ETokenType::OP_DECREMENT: {
		if (!left) {
			resolved_type = nullptr;
			return;
		}
		resolved_type = left->get_type();
		return;
	}

								   // Cas par d�faut
	default:
		resolved_type = nullptr;
		return;
	}
}

void AST::Inst_BinOpAST::check_type(const std::weak_ptr<ScriptInfo> scr_info) {

}

Value* AST::ComparaisonAST::codegen() const {
	// Generate the code for the left and right operands
	Value* leftVal = left->codegen();
	Value* rightVal = right->codegen();

	// If either operand is invalid, return nullptr
	if (!leftVal || !rightVal) {
		return nullptr;
	}

	switch (op) {
	case TokTy::OP_EQUAL: {
		// Equality comparison (==)
		return infoScript.builder.CreateICmpEQ(leftVal, rightVal, "eq_tmp");
	}
	case TokTy::OP_NOT_EQUAL: {
		// Inequality comparison (!=)
		return infoScript.builder.CreateICmpNE(leftVal, rightVal, "neq_tmp");
	}
	case TokTy::OPEN_BRACKETS: {
		// Less than comparison (<)
		return infoScript.builder.CreateICmpSLT(leftVal, rightVal, "lt_tmp");
	}
	case TokTy::OP_LOWER_EQUAL: {
		// Less than or equal to comparison (<=)
		return infoScript.builder.CreateICmpSLE(leftVal, rightVal, "leq_tmp");
	}
	case TokTy::CLOSE_BRACKETS: {
		// Greater than comparison (>)
		return infoScript.builder.CreateICmpSGT(leftVal, rightVal, "gt_tmp");
	}
	case TokTy::OP_GREATER_EQUAL: {
		// Greater than or equal to comparison (>=)
		return infoScript.builder.CreateICmpSGE(leftVal, rightVal, "geq_tmp");
	}
	default:
		return nullptr;  // If the comparator is not recognized
	}
}

Value* AST::Inst_InterCondAST::codegen() const {
	// Generate the code for the left, center, and right operands
	Value* leftVal = left->codegen();
	Value* centerVal = center->codegen();
	Value* rightVal = right->codegen();

	// If any operand is invalid, return nullptr
	if (!leftVal || !centerVal || !rightVal) {
		return nullptr;
	}

	// First comparison (left comparator)
	Value* leftComparison = nullptr;
	switch (left_comparator) {
	case TokTy::OP_EQUAL: {
		// Equality comparison (==)
		leftComparison = infoScript.builder.CreateICmpEQ(leftVal, centerVal, "lhs_eq_tmp");
	}
	case TokTy::OP_NOT_EQUAL: {
		// Inequality comparison (!=)
		leftComparison = infoScript.builder.CreateICmpNE(leftVal, centerVal, "lhs_neq_tmp");
	}
	case TokTy::OPEN_BRACKETS: {
		// Less than comparison (<)
		leftComparison = infoScript.builder.CreateICmpSLT(leftVal, centerVal, "lhs_tlt_tmp");
	}
	case TokTy::OP_LOWER_EQUAL: {
		// Less than or equal to comparison (<=)
		leftComparison = infoScript.builder.CreateICmpSLE(leftVal, centerVal, "lhs_leq_tmp");
	}
	case TokTy::CLOSE_BRACKETS: {
		// Greater than comparison (>)
		leftComparison = infoScript.builder.CreateICmpSGT(leftVal, centerVal, "lhs_gt_tmp");
	}
	case TokTy::OP_GREATER_EQUAL: {
		// Greater than or equal to comparison (>=)
		leftComparison = infoScript.builder.CreateICmpSGE(leftVal, centerVal, "lhs_geq_tmp");
	}
	default:
		return nullptr;
	}

	// Second comparison (right comparator)
	Value* rightComparison = nullptr;
	switch (right_comparator) {
	case TokTy::OP_EQUAL: {
		// Equality comparison (==)
		rightComparison = infoScript.builder.CreateICmpEQ(centerVal, rightVal, "rhs_eq_tmp");
	}
	case TokTy::OP_NOT_EQUAL: {
		// Inequality comparison (!=)
		rightComparison = infoScript.builder.CreateICmpNE(centerVal, rightVal, "rhs_neq_tmp");
	}
	case TokTy::OPEN_BRACKETS: {
		// Less than comparison (<)
		rightComparison = infoScript.builder.CreateICmpSLT(centerVal, rightVal, "rhs_lt_tmp");
	}
	case TokTy::OP_LOWER_EQUAL: {
		// Less than or equal to comparison (<=)
		rightComparison = infoScript.builder.CreateICmpSLE(centerVal, rightVal, "rhs_leq_tmp");
	}
	case TokTy::CLOSE_BRACKETS: {
		// Greater than comparison (>)
		rightComparison = infoScript.builder.CreateICmpSGT(centerVal, rightVal, "rhs_gt_tmp");
	}
	case TokTy::OP_GREATER_EQUAL: {
		// Greater than or equal to comparison (>=)
		rightComparison = infoScript.builder.CreateICmpSGE(centerVal, rightVal, "rhs_geq_tmp");
	}
	default:
		return nullptr;
	}

	// Combine both comparisons with a logical AND
	return infoScript.builder.CreateAnd(leftComparison, rightComparison, "interval_comp");
}

Value* AST::State_IfAST::codegen() const {
	// Generate the condition code
	Value* condVal = condition->codegen();
	if (!condVal) {
		return nullptr; // If the condition is invalid, return nullptr
	}

	// Convert condition to boolean if necessary
	condVal = infoScript.builder.CreateTrunc(condVal, infoScript.builder.getInt1Ty(), "ifcond");

	// Create basic blocks for the if and else statements
	Function* func = infoScript.builder.GetInsertBlock()->getParent();
	BasicBlock* thenBB = BasicBlock::Create(infoScript.context, "then", func);
	BasicBlock* elseBB = BasicBlock::Create(infoScript.context, "else", func);
	BasicBlock* mergeBB = BasicBlock::Create(infoScript.context, "ifcont", func);

	// Conditional branch: if condition is true, jump to thenBB, otherwise jump to elseBB
	infoScript.builder.CreateCondBr(condVal, thenBB, elseBB);

	// Generate the "then" block
	infoScript.builder.SetInsertPoint(thenBB);
	for (auto& line : lines) {
		line->codegen();
	}
	infoScript.builder.CreateBr(mergeBB); // Jump to the merge block

	// Generate the "else" block
	infoScript.builder.SetInsertPoint(elseBB);
	if (isElseNoCondition && elseStatement) {
		elseStatement->codegen();  // If the else has no condition, just execute its block
	}
	else if (!isElseNoCondition && elseStatement) {
		// If there's an else statement with its own condition, evaluate it
		elseStatement->codegen();
	}
	infoScript.builder.CreateBr(mergeBB); // Jump to the merge block

	// Generate the merge block (this is the continuation of the function after the if-else statement)
	infoScript.builder.SetInsertPoint(mergeBB);

	return nullptr; // Return value isn't needed for control flow statements like "if"
}

Value* AST::State_ForAST::codegen() const {
	if (range->_token.type == TokTy::RANGE) {
		Value* rangeVal = range->codegen();

		// Extract the individual values from the range structure
		// Extract '_ty_ref_id' from index 0
		Value* _ty_ref_id = infoScript.builder.CreateExtractValue(rangeVal, { 0 });
		// Extract 'start' from index 1
		Value* startVal = infoScript.builder.CreateExtractValue(rangeVal, { 1 });
		// Extract 'end' from index 2
		Value* endVal = infoScript.builder.CreateExtractValue(rangeVal, { 2 });
		// Extract 'step' from index 3
		Value* stepVal = infoScript.builder.CreateExtractValue(rangeVal, { 3 });
		// Extract 'endInclude' (a boolean) from index 4
		Value* endIncludeVal = infoScript.builder.CreateExtractValue(rangeVal, { 4 });

		// range
		auto rangeValues = dynamic_cast<Inst_RangeAST*>(range.get());
		bool endInclude = rangeValues->endInclude;

		if (endInclude) infoScript.builder.CreateAdd(endVal, ConstantInt::get(Type::getInt32Ty(infoScript.context), 1));

		Function* F = infoScript.builder.GetInsertBlock()->getParent();

		// Block Creation
		BasicBlock* loopStart = BasicBlock::Create(infoScript.context, "loopStart", F);
		BasicBlock* loopBody = BasicBlock::Create(infoScript.context, "loopBody", F);
		BasicBlock* loopEnd = BasicBlock::Create(infoScript.context, "loopEnd", F);

		// Init branch
		infoScript.builder.CreateBr(loopStart);
		infoScript.builder.SetInsertPoint(loopStart);

		PHINode* iter = infoScript.builder.CreatePHI(Type::getInt32Ty(infoScript.context), 2, "iter");
		iter->addIncoming(startVal, infoScript.builder.GetInsertBlock());

		// detect direction (ascending or descending)
		Value* isReverse = infoScript.builder.CreateICmpSLT(startVal, endVal, "is_reserved");

		// Stop condition
		Value* condNormal = infoScript.builder.CreateICmpSLE(iter, endVal);
		Value* condReserve = infoScript.builder.CreateICmpSGE(iter, endVal);
		Value* cond = infoScript.builder.CreateSelect(isReverse, condReserve, condNormal);

		// Condition to exit loop
		infoScript.builder.CreateCondBr(cond, loopBody, loopEnd);
		for (auto& line : lines) {
			line->codegen();
		}

		// Maj iterator (optimized)
		Value* nextIterNormal = infoScript.builder.CreateAdd(iter, stepVal);
		Value* nextIterReserve = infoScript.builder.CreateSub(iter, stepVal);
		Value* nextIter = infoScript.builder.CreateSelect(isReverse, nextIterReserve, nextIterNormal);

		// Maj var PHI
		iter->addIncoming(nextIter, loopBody);

		// Return to loop start
		infoScript.builder.CreateBr(loopStart);

		// Loop end
		infoScript.builder.SetInsertPoint(loopEnd);

		return nullptr;
	}
	// range is a llvm list
	else {
		Function* F = infoScript.builder.GetInsertBlock()->getParent();
		BasicBlock* preheaderBB = infoScript.builder.GetInsertBlock();
		BasicBlock* loopBB = BasicBlock::Create(infoScript.context, "loopBody", F);
		BasicBlock* afterBB = BasicBlock::Create(infoScript.context, "loopEnd", F);

		// G�n�rer le code pour l'expression range
		Value* container = range->codegen();
		if (!container)
			return nullptr;

		// getPtr ont first and end
		Value* beginPtr = getBeginPointer(infoScript.builder, container);
		Value* endPtr = getEndPointer(infoScript.builder, container);

		// Allocate iterator
		Value* iterator = infoScript.builder.CreateAlloca(container->getType()->getStructElementType(1), nullptr, itemName);

		// Init iterator with begin()
		infoScript.builder.CreateStore(beginPtr, iterator);

		// Jump to loop
		infoScript.builder.CreateBr(loopBB);

		// Insert loop bloc
		infoScript.builder.SetInsertPoint(loopBB);
		Value* current = infoScript.builder.CreateLoad(container->getType()->getStructElementType(1), iterator);

		// Loop condition : iterator != end
		Value* cond = infoScript.builder.CreateICmpNE(current, endPtr, "loopcond");
		infoScript.builder.CreateCondBr(cond, loopBB, afterBB);

		// generate loop body
		for (auto& line : lines) {
			line->codegen();
		}

		// Increment iterator
		Value* next = incrementIterator(infoScript.builder, current);
		infoScript.builder.CreateStore(next, iterator);

		// return to the loop start
		infoScript.builder.CreateBr(loopBB);

		// Exit the loop
		infoScript.builder.SetInsertPoint(afterBB);
		return nullptr;
	}
}

Value* AST::Inst_RangeAST::codegen() const {
	// Generate the start and end values
	Value* _ty_ref_id = ConstantInt::get(Type::getInt32Ty(infoScript.context), static_cast<int8_t>(TokTy::RANGE));
	Value* startVal = start->codegen();
	Value* endVal = end->codegen();
	Value* stepVal = step->codegen();
	Value* endIncludeVal = infoScript.builder.getInt1(endInclude);

	// V�rification que les valeurs sont valides
	if (!startVal || !endVal) {
		return nullptr; // Gestion d'erreur
	}

	if (!stepVal) {
		stepVal = ConstantInt::get(Type::getInt32Ty(infoScript.context), 1);
	}

	// Cr�ation du type de structure {i32, i32, i32, i32, bool}
	StructType* StructType = StructType::get(
		infoScript.context, {
			Type::getInt32Ty(infoScript.context),	// typeid
			Type::getInt32Ty(infoScript.context),	// start
			Type::getInt32Ty(infoScript.context),	// end
			Type::getInt32Ty(infoScript.context),	// step
			Type::getInt1Ty(infoScript.context),	// endInclude
		}
		);

	Value* rangeVal = UndefValue::get(StructType);

	rangeVal = infoScript.builder.CreateInsertValue(rangeVal, _ty_ref_id, { 0 });
	rangeVal = infoScript.builder.CreateInsertValue(rangeVal, startVal, { 1 });
	rangeVal = infoScript.builder.CreateInsertValue(rangeVal, endVal, { 2 });
	rangeVal = infoScript.builder.CreateInsertValue(rangeVal, stepVal, { 3 });
	rangeVal = infoScript.builder.CreateInsertValue(rangeVal, endIncludeVal, { 4 });

	return rangeVal;
}

std::tuple<Value*, Value*, Value*, Value*> AST::Inst_RangeAST::range_codegen()
{
	return std::tuple<Value*, Value*, Value*, Value*>();
}

Value* AST::Def_ParamAST::codegen() const {
	// LLVM type
	Type* paramType = type->codegen()->getType();

	// allocation
	AllocaInst* _allocaInst = infoScript.builder.CreateAlloca(paramType, nullptr, name);

	if (defaultValue) {
		Value* defVal = defaultValue->codegen();
		infoScript.builder.CreateStore(defaultValue->codegen(), _allocaInst);
	}

	return _allocaInst;
}

std::string AST::Def_ParamAST::mangling() const {
	std::string outStr;
	switch (passMode) {
	case AST::Def_ParamAST::EPassMode::None:	outStr += "";		break;
	case AST::Def_ParamAST::EPassMode::Mut:		outStr += "mut:";	break;
	case AST::Def_ParamAST::EPassMode::Copy:	outStr += "copy:";	break;
	case AST::Def_ParamAST::EPassMode::Move:	outStr += "move:";	break;
	case AST::Def_ParamAST::EPassMode::Compiletime:	outStr += "cons:";	break;
	case AST::Def_ParamAST::EPassMode::Addr:	outStr += "addr:";	break;
	}

	outStr += type->mangling();
	return outStr;
}

Value* AST::Inst_ReturnAST::codegen() const {
	Value* returnValue = nullptr;

	if (values) {
		if (values->values.empty()) return infoScript.builder.CreateRetVoid();
		if (values->values.size() == 1) return infoScript.builder.CreateRet(values->values[0]->codegen());

		std::vector<Type*> tupleTy;
		std::vector<Value*> tupleVal;
		for (auto& elem : values->values) {
			auto val = elem->codegen();
			tupleVal.push_back(val);
			tupleTy.push_back(val->getType());
		}

		auto ty = StructType::get(infoScript.context, tupleTy);
		Value* val = UndefValue::get(ty);
		uint8_t i = 0;
		for (auto& elem : tupleVal) {
			val = infoScript.builder.CreateInsertValue(val, elem, { i++ });
		}

		return infoScript.builder.CreateRet(val);
	}

	return infoScript.builder.CreateRetVoid();  // If no value to return
}

std::string AST::Inst_ReturnAST::mangling() const
{
	return std::string();
}

Value* AST::Def_EnumAST::codegen() const {
	// Cr�e un type entier pour l'enum (int32 par exemple)
	auto enumType = llvm::Type::getInt32Ty(infoScript.context);
	/*
	if (isGlobal) {
		// Si l'enum est global, on cr�e des variables globales
		for (size_t i = 0; i < elements.size(); ++i) {
			str entryName = name + "::" + elements[i];

			// Cr�e une constante LLVM pour cete valeur
			llvm::Constant* enumVal = llvm::ConstantInt::get(enumType, i);

			// Cr�e une variable globale LLVM pour l��l�ment
			new llvm::GlobalVariable(
				infoScript.mod,
				enumType,
				true,
				llvm::GlobalValue::ExternalLinkage,
				enumVal,
				entryName
			);
		}
	}
	else {
		// Si l'enum est local, nous devons cr�er des variables locales dans le contexte actuel de la fonction
		Function* func = infoScript.builder.GetInsertBlock()->getParent();
		if (!func) {
			std::cerr << "No current function context for local enum!" << std::endl;
			return nullptr;
		}

		// Cr�e des variables locales dans la fonction
		BasicBlock* entry = &func->getEntryBlock();
		for (size_t i = 0; i < elements.size(); ++i) {
			str entryName = name + "::" + elements[i];

			// Cr�e une alloca pour une variable locale dans la fonction
			AllocaInst* alloca = infoScript.builder.CreateAlloca(enumType, nullptr, entryName);

			// Initialise la variable locale avec la valeur de l'�num�ration
			llvm::Constant* enumVal = llvm::ConstantInt::get(enumType, i);
			infoScript.builder.CreateStore(enumVal, alloca);
		}
	}

	// Retourne nullptr, car aucune valeur n'est n�cessaire ici pour l'�num�ration
	
	return nullptr;
}

std::string AST::Def_EnumAST::mangling() const
{
	return std::string();
}

Type* AST::Def_ClassAST::typegen() const {
	assert(!infoScript.mod.getNamedGlobal(name) && "Class name '" + name + "' is already declared!");

	std::vector<Type*> class_Ty;
	class_Ty.reserve(atributes.size() + 1);
	class_Ty.push_back(Type::getInt32Ty(infoScript.context)); // sturct id

	for (auto& elem : atributes) {
		Value* atribute_val = elem->type->codegen();
		Type* atributeTy = atribute_val->getType();

		assert(atributeTy && "Atribute type invalid!");

		class_Ty.push_back(atributeTy);
	}


	StructType* classType = StructType::create(infoScript.context, class_Ty, mangling());

	for (auto& op : operators) {
		op->codegen();
	}

	for (auto& met : methods) {
		met->codegen();
	}

	return classType;
}

Value* AST::Def_ClassAST::codegen() const {
	auto ty = typegen();

	for (auto& at : atributes) {
		at->codegen();
	}

	for (auto& meth : methods) {
		meth->codegen();
	}

	for (auto& impl : implementations) {
		impl->codegen();
	}

	return nullptr;
}

std::string AST::Def_ClassAST::mangling() const {
	return "cls:" + name;
}

Value* AST::Def_GlobalVarAST::codegen() const {
	assert(type && "Type dosen't exists or founds!");

	Type* baseType = type->codegen()->getType();
	assert(baseType && "Type invalid!");

	Type* T = baseType;
	Constant* Init = nullptr;

	if (!type->size.has_value()) std::runtime_error("error, table size is not defined");

	// check array size
	int tableSize = type->size.value();

	// if variable is a table
	if (tableSize > 1) {
		T = ArrayType::get(baseType, tableSize);

		std::vector<Constant*> init_elements;
		init_elements.reserve(tableSize);

		// init table
		if (expression && expression->_token.type == TokTy::L_ARRAY) {
			auto* initArray = dynamic_cast<AST::Lit_ArrayAST*>(expression.get());
			assert(initArray && initArray->identifiers.argsSize() == tableSize && "Array list expression is invalid!");

			for (auto& elem : initArray->elements) {
				if (auto c = dyn_cast<Constant>(elem.get()->codegen())) {
					init_elements.push_back(c);
				}
			}
		}
		else {
			init_elements = std::vector<Constant*>(tableSize, Constant::getNullValue(baseType));
		}

		Init = ConstantArray::get(static_cast<ArrayType*>(T), init_elements);
	}
	// Scalar value
	else {
		if (expression) {
			if (auto c = dyn_cast<Constant>(expression->codegen())) {
				Init = c;
			}
			else assert("Global var need to be initialize by a constant!");
		}
		else {
			Init = Constant::getNullValue(baseType);
		}
	}

	// D�terminer la linkage
	GlobalValue::LinkageTypes L =
		isStatic ? GlobalValue::InternalLinkage :
		isExport ? GlobalValue::ExternalLinkage :
		GlobalValue::PrivateLinkage;

	// Cr�ation de la variable globale
	auto* globalVar = new GlobalVariable(
		infoScript.mod,
		T,
		isConst,
		L,
		Init,
		name
	);

	return globalVar;
}

std::string AST::Def_GlobalVarAST::mangling() const
{
	return std::string();
}

Value* AST::Def_LocalVarAST::codegen() const {
	assert(type && "Type dosen't exists or founds!");

	// Obtenir le type LLVM � partir de l'AST
	Type* baseType = type->codegen()->getType();
	assert(baseType && "Type invalid!");

	Type* T = baseType;
	Value* Init = nullptr;


	if (!type->size.has_value()) std::runtime_error("error, table size is not defined");

	// check array size
	int tableSize = type->size.value();

	// if variable is a table
	if (tableSize > 1) {
		T = ArrayType::get(baseType, tableSize);

		std::vector<Constant*> init_elements;
		init_elements.reserve(tableSize);

		// init table
		if (expression && _token.type == TokTy::L_ARRAY) {
			auto* initArray = dynamic_cast<AST::Lit_ArrayAST*>(expression.get());
			assert(initArray && initArray->identifiers.argsSize() == tableSize && "Array list expression is invalid!");

			for (auto& elem : initArray->elements) {
				if (auto c = dyn_cast<Constant>(elem->codegen())) {
					init_elements.push_back(c);
				}
			}
		}
		else {
			init_elements = std::vector<Constant*>(tableSize, Constant::getNullValue(baseType));
		}

		Init = ConstantArray::get(static_cast<ArrayType*>(T), init_elements);
	}
	// Scalar value
	else {
		if (expression) {
			auto init_val = expression->codegen();
			if (auto c = dyn_cast<Constant>(init_val)) {
				Init = c;
			}
			else {
				Init = init_val;
			}
		}
		else {
			Init = Constant::getNullValue(baseType);
		}
	}

	Function* F = infoScript.builder.GetInsertBlock()->getParent();

	// Local : Allocate on stack
	AllocaInst* localVarAlloc = infoScript.builder.CreateAlloca(T, nullptr, name);

	// affect value
	if (!Init) {
		infoScript.builder.CreateStore(Init, localVarAlloc);
	}

	return localVarAlloc;
}

std::string AST::Def_LocalVarAST::mangling() const
{
	return std::string();
}

Function* AST::Def_FnAST::codegen_declaration_only() const {
	// returns
	Type* RT = body->returnType ? body->returnType->typegen() : Type::getVoidTy(infoScript.context);
	// params
	std::vector<Type*> PT(body->parameters.size());
	for (auto& param : body->parameters) {
		PT.push_back(param->typegen());
	}

	// link
	Function::LinkageTypes L = exportName != "" ? Function::LinkageTypes::ExternalLinkage : Function::LinkageTypes::InternalLinkage;

	// fn infoScript.context
	FunctionType* FT = FunctionType::get(
		RT, // return
		PT, // params
		body->isParamVariadic // idVariadic
	);

	// fn creation with types
	Function* F = Function::Create(
		FT,		// fn type
		L,	// linkage
		name,		// name
		infoScript.mod);	// module
	return F;
}

Value* AST::Def_FnAST::codegen() const {
	// if is an generic function: wait to write new versions from instances
	if (body->genWhere) {
		return nullptr;
	}

	auto F = codegen_declaration_only();

	// fn statement (entry)
	BasicBlock* entry = BasicBlock::Create(infoScript.context, "entry", F);
	infoScript.builder.SetInsertPoint(entry);
	std::unique_ptr<IRBuilder<>> tempBuilder = std::make_unique<IRBuilder<>>(entry);

	for (auto& elem : body->lines) {
		elem->codegen();
	}


	// fn echec statement (echec_entry)
	if (body->failure_lines.empty()) {
		BasicBlock* echec_BB = BasicBlock::Create(infoScript.context, "echec_entry", F);
		infoScript.builder.SetInsertPoint(echec_BB);
		tempBuilder = std::make_unique<IRBuilder<>>(echec_BB);

		for (auto& elem : body->failure_lines) {
			elem->codegen();
		}
	}

	return F;
}

std::string AST::Def_FnAST::mangling() const {
	return "fn:" + name + body->mangling();
}

AST::Def_FnBodyAST* AST::Def_FnAST::get_body() const { return body.get(); }

std::string AST::Def_FnAST::callee_mangling() const { return mangling(); }

Value* AST::Inst_AssignmentAST::codegen() const
{
	return nullptr;
}

void AST::Inst_AssignmentAST::check_type(const std::weak_ptr<ScriptInfo> scr_info)
{
}

Value* AST::Inst_NewAST::codegen() const {
	Value* targetVal = targetValue->codegen();

	// G�n�rer le code pour le type cible
	Type* targetTypeLLVM = targetType->codegen()->getType(); // Devrait renvoyer un type LLVM valide
	if (!targetTypeLLVM) {
		return nullptr; // Si le type n'est pas valide, retourner un nullptr
	}

	// Calculer la taille du type cible (en octets)
	Type* sizeType = Type::getInt64Ty(infoScript.context); // On suppose que la taille est en 64 bits
	Value* sizeVal = ConstantInt::get(sizeType, targetTypeLLVM->getPrimitiveSizeInBits() / 8);

	// Appeler malloc pour allouer de la m�moire sur le tas
	Function* mallocFunc = infoScript.mod.getFunction("malloc");
	if (!mallocFunc) {
		// Si malloc n'est pas d�j� d�fini, nous devons le d�clarer comme une fonction externe
		mallocFunc = Function::Create(FunctionType::get(PointerType::getUnqual(Type::getInt8Ty(infoScript.context)), { sizeType }, false),
			GlobalValue::ExternalLinkage, "malloc", infoScript.mod);
	}

	// Appeler malloc et obtenir un pointeur vers la m�moire allou�e
	Value* allocatedMemory = infoScript.builder.CreateCall(mallocFunc, { sizeVal });
	if (!allocatedMemory) {
		return nullptr; // Si l'appel � malloc �choue, retourner un nullptr
	}

	// Convertir le pointeur allou� au type cible (comme un pointeur vers targetType)
	Value* targetPtr = infoScript.builder.CreateBitCast(allocatedMemory, PointerType::getUnqual(targetTypeLLVM));

	// Retourner le pointeur allou�
	return targetPtr;
}

Value* AST::Def_NamespaceAST::codegen() const {
	for (auto& elem : elements) {
		elem->codegen();
	}

	return nullptr;
}

std::string AST::Def_NamespaceAST::mangling() const {
	return name;
}

Value* AST::Inst_ArrayElemAST::codegen() const {
	// generate identifier, index_range
	Value* array = base->codegen();
	Value* index = index_range->codegen();

	if (index_range->_token.type == TokTy::RANGE) {
		auto ptr = dynamic_cast<AST::Inst_RangeAST*>(index_range.get());
		// If endInclude is false, decrement end by 1
		Value* isEndIncludeFalse = infoScript.builder.CreateICmpEQ(index, infoScript.builder.getInt1(false));
		auto end = infoScript.builder.CreateSelect(isEndIncludeFalse, infoScript.builder.CreateSub(ptr->end->codegen(), infoScript.builder.getInt32(1)), ptr->end->codegen());
		// If step is positive, calculate the size
		Value* sizeExpr = infoScript.builder.CreateSub(end, ptr->start->codegen(), "size_expr");
		Value* listSize = infoScript.builder.CreateSDiv(sizeExpr, ptr->step->codegen(), "list_size"); // Calculate (end - start) / step

		// Return the sublist
		// return subList;
	}
	else if (index_range->_token.type == TokTy::L_I) {
		Value* elemPtr = infoScript.builder.CreateGEP(array->getType()->getStructElementType(0), { index }, array, "array_elem");
		return infoScript.builder.CreateLoad(elemPtr->getType()->getStructElementType(0), elemPtr, "array_access");
	}

	// else generate regular access at table element;
	Value* elemPtr = infoScript.builder.CreateGEP(array->getType(), array, { index }, "array_elem");
	return infoScript.builder.CreateLoad(array->getType(), elemPtr, "array_access");
}


// for container
Value* getBeginPointer(IRBuilder<>& builder, Value* container) {
	// assume second struct elem is pointer to elements
	return builder.CreateStructGEP(container->getType(), container, 1, "beginPtr");
}

// for container
Value* getEndPointer(IRBuilder<>& builder, Value* container) {
	// assume third struct elem is size
	Value* size = builder.CreateStructGEP(container->getType(), container, 2, "sizePtr");
	Value* beginPtr = getBeginPointer(builder, container);
	return builder.CreateGEP(container->getType()->getStructElementType(1), beginPtr, size, "endPtr");
}

Value* incrementIterator(IRBuilder<>& builder, Value* current) {
	// Incr�menter l'it�rateur
	Value* one = ConstantInt::get(current->getType(), 1);
	return builder.CreateAdd(current, one, "increment");
}


void generate_IR(std::vector<std::unique_ptr<AST::NodeAST>>& nodes) {
	for (auto& elem : nodes) {
		elem->codegen();
	}
}

Value* AST::Def_ClassMethodAST::codegen() const {
	// 1 - find llvm class
	StructType* class_ty = StructType::getTypeByName(infoScript.context, class_sym->name);

	// 2 - build method args
	std::vector<Type*> arg_ty;
	// add class origin at first method argument
	if (isSelf) arg_ty.push_back(PointerType::get(class_ty, 0));

	// add other method args
	if (!body->parameters.empty()) {
		for (auto& param : body->parameters) {
			Type* param_ty = param->type->typegen();
			arg_ty.push_back(param_ty);
		}
	}

	// 3 - build method return
	Value* ret_ty_val = body->returnType ? body->returnType->codegen() : nullptr;
	Type* ret_ty = body->returnType ? ret_ty_val->getType() : Type::getVoidTy(infoScript.context);

	// 4 - build method function type
	FunctionType* fn_ty = FunctionType::get(ret_ty, arg_ty, body->isParamVariadic);

	// 5 - build method function
	std::string fullName = mangling();
	Function* fn = Function::Create(fn_ty, Function::ExternalLinkage, fullName, infoScript.mod);

	// 6 - name arguments
	unsigned idx = 0;
	for (auto& arg : fn->args()) {
		if (idx == 0 && isSelf) {
			arg.setName("self");
		}
		else {
			if (body->parameters.empty() && idx - 1 < body->parameters.size()) {
				arg.setName(body->parameters[idx - 1]->name);
			}
			else {
				arg.setName("arg" + std::to_string(idx - 1));
			}
		}
		idx++;
	}

	// 7 - Build BasicBlock for the method body
	BasicBlock* entry = BasicBlock::Create(infoScript.context, "entry", fn);
	infoScript.builder.SetInsertPoint(entry);

	// 8 - generate main function body
	if (!body->lines.empty()) {
		for (auto& line : body->lines) {
			line->codegen();
		}
	}
	else {
		infoScript.builder.CreateRetVoid();
	}


	if (!body->failure_lines.empty()) {
		// 7.2 - Build BasicBlock for the method body
		BasicBlock* failure = BasicBlock::Create(infoScript.context, "failure", fn);
		infoScript.builder.SetInsertPoint(failure);

		// 8.2 - generate main function body
		for (auto& line : body->failure_lines) {
			line->codegen();
		}
	}

	return fn;
}

std::string AST::Def_ClassMethodAST::mangling() const {
	return class_sym->mangling() + "fn:" + name + body->mangling();
}

AST::Def_FnBodyAST* AST::Def_ClassMethodAST::get_body() const { return body.get(); }

std::string AST::Def_ClassMethodAST::callee_mangling() const { return mangling(); }

Value* AST::Def_ClassAttributeAST::codegen() const {
	// Generate the type for the atribute
	llvm::Type* atrType = type->codegen()->getType();

	// Check if the atribute is static or not
	llvm::GlobalVariable* globalVar = nullptr;
	if (isStatic) {
		// For static atributes, we generate global variables.
		globalVar = new llvm::GlobalVariable(infoScript.mod, atrType, false, llvm::GlobalValue::InternalLinkage, nullptr, name);

		// If an expression (initial value) is provided, set the initial value for the global variable
		if (expression) {
			llvm::Value* initValue = expression->codegen();
			if (!initValue) {
				return nullptr; // If initialization fails, return null
			}
			globalVar->setInitializer(llvm::cast<llvm::Constant>(initValue)); // std::set the initializer for static atribute
		}
	}
	else {
		// For non-static atributes, we typically store them within an object or structure
		llvm::AllocaInst* _alloca = infoScript.builder.CreateAlloca(atrType, nullptr, name);
		if (expression) {
			llvm::Value* initValue = expression->codegen();
			if (!initValue) {
				return nullptr; // If initialization fails, return null
			}
			infoScript.builder.CreateStore(initValue, _alloca); // Store the initialized value
		}
		return _alloca;
	}

	// Handle visibility (can be useful for static atributes or global visibility)
	if (visibility == TokTy::PUBLIC) {
		globalVar->setVisibility(GlobalValue::VisibilityTypes::DefaultVisibility);
	}
	else if (visibility == TokTy::PRIVATE) {
		globalVar->setVisibility(GlobalValue::VisibilityTypes::HiddenVisibility);
	}
	else if (visibility == TokTy::PROTECTED) {
		globalVar->setVisibility(GlobalValue::VisibilityTypes::ProtectedVisibility);
	}

	return globalVar;
}

std::string AST::Def_TraitAST::mangling() const {
	return "trt:" + name;
}

Type* AST::Ty_PrimitiveAST::typegen() const {
	switch (type)
	{
	case TokTy::T_BOOL:				return Type::getInt1Ty(infoScript.context);
	case TokTy::T_CHAR:
	case TokTy::T_ASCII:			return Type::getInt8Ty(infoScript.context);
	case TokTy::T_USIZE: {
		unsigned ptrSize = infoScript.mod.getDataLayout().getPointerSizeInBits();
		return IntegerType::get(infoScript.context, ptrSize);
	}
	case TokTy::T_ENUM:				return Type::getInt32Ty(infoScript.context);
	case TokTy::T_I8:				return Type::getInt8Ty(infoScript.context);
	case TokTy::T_I16:				return Type::getInt16Ty(infoScript.context);
	case TokTy::T_I32:				return Type::getInt32Ty(infoScript.context);
	case TokTy::T_I64:				return Type::getInt64Ty(infoScript.context);
	case TokTy::T_I128:				return IntegerType::get(infoScript.context, 128);
	case TokTy::T_U8:				return Type::getInt8Ty(infoScript.context);
	case TokTy::T_U16:				return Type::getInt16Ty(infoScript.context);
	case TokTy::T_U32:				return Type::getInt32Ty(infoScript.context);
	case TokTy::T_U64:				return Type::getInt64Ty(infoScript.context);
	case TokTy::T_U128:				return IntegerType::get(infoScript.context, 128);
	case TokTy::T_F32:				return Type::getFloatTy(infoScript.context);
	case TokTy::T_F64:				return Type::getDoubleTy(infoScript.context);
	case TokTy::T_DECIMAL:
	case TokTy::T_UDECIMAL:			return Type::getInt64Ty(infoScript.context); // fallback
	case TokTy::T_STRING:
	case TokTy::T_TEXT:
	case TokTy::T_UTF8:
	case TokTy::T_UTF16:
	case TokTy::T_UTF32:			return PointerType::getUnqual(Type::getInt8Ty(infoScript.context));
	case TokTy::L_ARRAY:	return PointerType::getUnqual(Type::getInt8Ty(infoScript.context));
	case TokTy::T_TUPLE:
	case TokTy::HASHTAG:
	default:
		break;
	}
}

std::string AST::Ty_PrimitiveAST::mangling() const {
	std::string outStr;

	if (pointer) {
		switch (pointer->pointer_type)
		{
		case TokTy::PTR:
			outStr += "ptr'";
			break;
		case TokTy::UPTR:
			outStr += "std::unique_ptr'";
			break;
		case TokTy::SPTR:
			outStr += "std::shared_ptr'";
			break;
		case TokTy::PTR_SAFE:
			outStr += "ptrs'";
			break;
		case TokTy::UPTR_SAFE:
			outStr += "uptrs'";
			break;
		case TokTy::SPTR_SAFE:
			outStr += "sptrs'";
			break;
		default:
			break;
		}
	}

	switch (type)
	{
	case TokTy::T_BOOL:
		return outStr + "b";
	case TokTy::T_CHAR:
		return outStr + "char";
	case TokTy::T_USIZE:
		return outStr + "usize";
	case TokTy::T_I8:
		return outStr + "i8";
	case TokTy::T_I16:
		return outStr + "i16";
	case TokTy::T_I32:
		return outStr + "i32";
	case TokTy::T_I64:
		return outStr + "i64";
	case TokTy::T_I128:
		return outStr + "i128";
	case TokTy::T_U8:
		return outStr + "u8";
	case TokTy::T_U16:
		return outStr + "u16";
	case TokTy::T_U32:
		return outStr + "u32";
	case TokTy::T_U64:
		return outStr + "u64";
	case TokTy::T_U128:
		return outStr + "u128";
	case TokTy::T_F32:
		return outStr + "f32";
	case TokTy::T_F64:
		return outStr + "f64";
	case TokTy::T_DECIMAL:
		return outStr + "deci";
	case TokTy::T_UDECIMAL:
		return outStr + "udeci";
	case TokTy::T_STRING:
		return outStr + "str";
	case TokTy::T_ASCII:
		return outStr + "str";
	case TokTy::T_TEXT:
		return outStr + "utf8";
	case TokTy::T_UTF8:
		return outStr + "utf8";
	case TokTy::T_UTF16:
		return outStr + "utf16";
	case TokTy::T_UTF32:
		return outStr + "utf32";
	default:
		break;
	}

	return outStr;
}


Value* AST::Def_PtrAST::codegen() const
{
	return nullptr;
}

std::string AST::Def_PtrAST::mangling() const {
	switch (pointer_type)
	{
	case TokTy::PTR:			return "ptr'";
	case TokTy::UPTR:			return "std::unique_ptr'";
	case TokTy::SPTR:			return "std::shared_ptr'";
	case TokTy::PTR_SAFE:		return "ptrs'";
	case TokTy::UPTR_SAFE:	return "uptrs'";
	case TokTy::SPTR_SAFE:	return "sptrs'";
	default:
		break;
	}

	return std::string();
}

Value* AST::Ty_IdAST::codegen() const
{
	return nullptr;
}

std::string AST::Ty_IdAST::mangling() const {
	std::string outStr;
	outStr += identifier;

	size_t i = 0;
	if (!gen_args.empty()) outStr += "<";
	for (auto& elem : gen_args) {
		outStr += elem->mangling();
		if (i < gen_args.size() - 1) outStr += ",";
		i++;
	}
	if (!gen_args.empty()) outStr += ">";

	return outStr;
}

std::string AST::Def_TypeBaseAST::mangling() const {
	std::string outStr;

	if (pointer) outStr += pointer->mangling();

	switch (_token.type)
	{
	case TokTy::T_BOOL:
		return outStr + "b";
	case TokTy::T_CHAR:
		return outStr + "char";
	case TokTy::T_USIZE:
		return outStr + "usize";
	case TokTy::T_I8:
		return outStr + "i8";
	case TokTy::T_I16:
		return outStr + "i16";
	case TokTy::T_I32:
		return outStr + "i32";
	case TokTy::T_I64:
		return outStr + "i64";
	case TokTy::T_I128:
		return outStr + "i128";
	case TokTy::T_U8:
		return outStr + "u8";
	case TokTy::T_U16:
		return outStr + "u16";
	case TokTy::T_U32:
		return outStr + "u32";
	case TokTy::T_U64:
		return outStr + "u64";
	case TokTy::T_U128:
		return outStr + "u128";
	case TokTy::T_F32:
		return outStr + "f32";
	case TokTy::T_F64:
		return outStr + "f64";
	case TokTy::T_DECIMAL:
		return outStr + "deci";
	case TokTy::T_UDECIMAL:
		return outStr + "udeci";
	case TokTy::T_STRING:
		return outStr + "str";
	case TokTy::T_ASCII:
		return outStr + "str";
	case TokTy::T_TEXT:
		return outStr + "utf8";
	case TokTy::T_UTF8:
		return outStr + "utf8";
	case TokTy::T_UTF16:
		return outStr + "utf16";
	case TokTy::T_UTF32:
		return outStr + "utf32";
	default:
		break;
	}

	return outStr;
}

Value* AST::Inst_CallAST::codegen() const {
	std::vector<Value*> args;

	if (!resolved_sym) throw std::runtime_error("targetCall definition not found!");

	// if function base is generic : need to build an new definiton
	
	if (definition->codegen()->genWhere) {
		// if function version already exist
		if (auto callee = infoScript.mod.getFunction(mangling())) {
			return infoScript.builder.CreateCall(callee, args, "call_" + mangling());
		}
		// if function doesn't exist
		else {
			//definition
		}
	}
	

	// if function is not generic
	if (auto callee = infoScript.mod.getFunction(mangling())) {
		return infoScript.builder.CreateCall(callee, args, "call_" + mangling());
	}

	throw std::runtime_error("Function: '" + mangling() + "' not found!");
}

std::string AST::Inst_CallAST::mangling() const {
	/*if (!definition) std::runtime_error("definition not resolved!");
	str outStr;
	outStr += defintion.->mangling();
	outStr += callee;

	size_t i = 0;
	if (!generic_arguments.empty()) outStr += "<";
	for (auto& elem : generic_arguments) {
		outStr += elem->mangling();
		if (i < generic_arguments.size() - 1) outStr += ",";
		i++;
	}
	if (!generic_arguments.empty()) outStr += ">";

	i = 0;
	if (!arguments.empty()) outStr += "(";
	for (auto& elem : arguments) {
		outStr += elem->mangling();
		if (i < generic_arguments.size() - 1) outStr += ",";
		i++;
	}
	if (!arguments.empty()) outStr += ")";

	if (returnType) outStr += returnType->mangling();

	return outStr;*/
/*
	return nullptr;
}

Value* AST::Def_TupleAST::codegen() const
{
	return nullptr;
}

std::string AST::Def_TupleAST::mangling() const {
	std::string outStr;

	size_t i = 0;
	outStr += "(";
	for (auto& elem : types) {
		outStr += elem->mangling();
		if (i < types.size() - 1) outStr += ",";
		i++;
	}
	outStr += ")";

	return outStr;
}

Value* AST::State_SafeCastAST::codegen() const
{
	return nullptr;
}

std::string AST::State_SafeCastAST::mangling() const {
	return "cast:" + binding + "." + type->mangling();
}

std::string AST::Def_ClassAttributeAST::mangling() const {
	std::string outStr;
	outStr += isConst ? "let" : "var";

	if (!class_sym) throw std::runtime_error("class not found!");

	return class_sym->mangling() + ":" + outStr + ":" + name + type->mangling();
}

std::string AST::Def_FnBodyAST::mangling() const {
	std::string outStr;

	size_t i = 0;
	if (!parameters.empty()) outStr += "(";
	i = 0;
	for (auto& param : parameters) {
		outStr += param->mangling();
		if (i < parameters.size() - 1) outStr += ",";
		i++;
	}
	if (!parameters.empty()) outStr += ")";

	if (returnType) {
		outStr += returnType->mangling();
	}

	return outStr;
}

bool AST::Def_FnBodyAST::isHeader() const
{
	return lines.empty();
}

Value* AST::Def_LamAST::codegen() const
{
	return nullptr;
}

std::string AST::Def_LamAST::mangling() const {
	return "lam:" + name + body->mangling();
}

AST::Def_FnBodyAST* AST::Def_LamAST::get_body() const { return body.get(); }

std::string AST::Def_LamAST::callee_mangling() const { return mangling(); }

std::string AST::Def_ImplAST::mangling() const {
	return class_sym->mangling() + "impl:" + name;
}

AST::Def_FnBodyAST* AST::Def_TraitMethodAST::get_body() const { return body.get(); }

std::string AST::Def_TraitMethodAST::callee_mangling() const { return name; }

AST::Def_FnBodyAST* AST::Def_ImplMethodAST::get_body() const { return body.get(); }

std::string AST::Def_ImplMethodAST::callee_mangling() const { return mangling(); }

Value* AST::Def_ClassOpAST::codegen() const
{
	return nullptr;
}

std::string AST::Def_ClassOpAST::mangling() const {
	std::string str;
	switch (operatorType) {
	case TokTy::OPEN_BRACKETS:			str = "open_bra"; break;
	case TokTy::CLOSE_BRACKETS:			str = "close_bra"; break;
	case TokTy::AND:						str = "and"; break;
	case TokTy::NAND:						str = "nand"; break;
	case TokTy::OR:						str = "or"; break;
	case TokTy::XOR:						str = "xor"; break;
	case TokTy::NOR:						str = "nor"; break;
	case TokTy::XNOR:						str = "xnor"; break;
	case TokTy::NOT:						str = "not"; break;
	case TokTy::B_AND:					str = "andb"; break;
	case TokTy::B_NAND:					str = "nandb"; break;
	case TokTy::B_OR:						str = "orb"; break;
	case TokTy::B_XOR:					str = "xorb"; break;
	case TokTy::B_NOR:					str = "norb"; break;
	case TokTy::B_XNOR:					str = "xnorb"; break;
	case TokTy::B_NOT:					str = "notb"; break;
	case TokTy::OP_EQUAL:					str = "eq"; break;
	case TokTy::OP_NOT_EQUAL:				str = "neq"; break;
	case TokTy::OP_GREATER_EQUAL:			str = "geq"; break;
	case TokTy::OP_LOWER_EQUAL:				str = "leq"; break;
	case TokTy::OP_PLUS:						str = "plus"; break;
	case TokTy::OP_MINUS:					str = "minus"; break;
	case TokTy::OP_ASTERISK:					str = "mult"; break;
	case TokTy::OP_POWER:					str = "pow"; break;
	case TokTy::OP_DIVIDE:					str = "div"; break;
	case TokTy::OP_MODULO:					str = "mod"; break;
	case TokTy::OP_QUOTIEN:			str = "modq"; break;
	case TokTy::ASSIGN_PLUS:				str = "plus_eq"; break;
	case TokTy::ASSIGN_MINUS:				str = "minus_eq"; break;
	case TokTy::ASSIGN_MULTIPLY:			str = "mult_eq"; break;
	case TokTy::ASSIGN_POWER:				str = "pow_eq"; break;
	case TokTy::ASSIGN_DIVIDE:			str = "div_eq"; break;
	case TokTy::ASSIGN_MODULO:			str = "mod_eq"; break;
	case TokTy::ASSIGN_QUOTIEN:	str = "modq_eq"; break;
	case TokTy::OP_INCREMENT:				str = "incr"; break;
	case TokTy::OP_DECREMENT:				str = "decr"; break;
	case TokTy::LEFT_SHIFT:				str = "ls"; break;
	case TokTy::RIGHT_SHIFT:				str = "rs"; break;
	case TokTy::SIGNATOR:					str = "sign"; break;
	case TokTy::IN:						str = "in"; break;
	default:										throw std::runtime_error("operator unexpected!");
	}

	auto other = otherType->mangling();

	std::string op = isClassOnRight ? other + "-" + str + "-class" : "class-" + str + "-" + other;
	return class_sym->mangling() + "op:" + str;
}

AST::Def_ClassOpAST::EOpReturnType AST::Def_ClassOpAST::deduce_op_return() const {
	for (auto tok : kComparisonOpTokens) {
		if (tok == operatorType) return EOpReturnType::T_BOOL;
	}

	for (auto tok : kModificatorOpTokens) {
		if (tok == operatorType) return EOpReturnType::REF;
	}

	for (auto tok : kArithmeticOpTokens) {
		if (tok == operatorType) return EOpReturnType::COPY;
	}

	throw std::runtime_error("operator unknown!");
}

std::string AST::Def_ClassOpAST::callee_mangling() const { return mangling(); }

bool AST::GenUse_OpAST::isTypeValid(std::map<std::string, Def_TypeBaseAST*>& types) const {
	std::unordered_set<ETokenType> opHandled;

	if (!types.count(targetGenSym)) return false;
	auto targetType = types.find(targetGenSym)->second;

	switch (targetType->_token.type)
	{
	case TokTy::T_BOOL:
	case TokTy::TRUE:
	case TokTy::FALSE:
		opHandled = integerOpHandled;
		break;
	case TokTy::T_CHAR:
		opHandled = charOpHandled;
		break;
	case TokTy::T_USIZE:
	case TokTy::T_ENUM:
	case TokTy::T_I8:
	case TokTy::T_I16:
	case TokTy::T_I32:
	case TokTy::T_I64:
	case TokTy::T_I128:
	case TokTy::T_U8:
	case TokTy::T_U16:
	case TokTy::T_U32:
	case TokTy::T_U64:
	case TokTy::T_U128:
	case TokTy::L_BIN:
	case TokTy::L_OCT:
	case TokTy::L_HEX:
	case TokTy::L_I:
	case TokTy::L_U:
	case TokTy::T_DECIMAL:
	case TokTy::T_UDECIMAL:
		opHandled = integerOpHandled;
		break;
	case TokTy::T_F32:
	case TokTy::T_F64:
	case TokTy::L_F:
		opHandled = floatingOpHandled;
		break;
	case TokTy::T_STRING:
	case TokTy::T_ASCII:
		opHandled = strOpHandled;
	case TokTy::T_TEXT:
	case TokTy::T_UTF8:
	case TokTy::T_UTF16:
	case TokTy::T_UTF32:
		opHandled = strOpHandled;
		break;
	case TokTy::L_ARRAY:
		opHandled = arrayOpHandled;
		break;
	case TokTy::T_TUPLE:
		return false;
	default:
		break;
	}

	// if type found by node type
	if (opHandled.empty()) return opHandled.count(operatorType);

	if (typeid(targetType) == typeid(Inst_TupleAST))				return false;
	else if (typeid(targetType) == typeid(Lit_BoolAST))			opHandled = boolOpHandled;
	else if (typeid(targetType) == typeid(Lit_CharAST))			opHandled = charOpHandled;
	else if (typeid(targetType) == typeid(Lit_StrAST))			opHandled = strOpHandled;
	else if (typeid(targetType) == typeid(Lit_FStrAST))	opHandled = strOpHandled;
	else if (typeid(targetType) == typeid(Lit_IntAST))			opHandled = integerOpHandled;
	else if (typeid(targetType) == typeid(Lit_FloatAST))		opHandled = floatingOpHandled;
	else if (typeid(targetType) == typeid(Lit_DecimalAST))		opHandled = integerOpHandled;
	else if (typeid(targetType) == typeid(Lit_ArrayAST))			opHandled = arrayOpHandled;
	else if (auto defClass = dynamic_cast<Def_ClassAST*>(targetType)) {
		// check class op
		bool isSymetrisized = false;
		bool isOperatorFound = false;
		for (auto& classOp : defClass->operators) {
			if (classOp->operatorType != operatorType) continue;

			// check if symetric op is specified
			if (isSymetric) {
				if (!types.count(targetGenSymSymetric)) continue;
				auto symetricType = types.find(targetGenSymSymetric)->second;

				// check if A operator B exist from A
				if (typeid(classOp->otherType) == typeid(symetricType)) isOperatorFound = true;

				if (classOp->isClassOnRight) {
					isSymetrisized = true;
					isOperatorFound = true;
					continue;
				}

				// check if B operator A exist from B
				if (auto defClass2 = dynamic_cast<Def_ClassAST*>(symetricType)) {
					for (auto& classOp2 : defClass2->operators) {
						if (classOp2->operatorType != operatorType) continue;

						if (typeid(classOp2->otherType) == typeid(classOp)) {
							isSymetrisized = true;
							break;
						}
					}
				}

				isOperatorFound = true;
			}

			// check if other op needed
			if (otherOperand) {
				if (!classOp->otherType) isOperatorFound = false;
				if (typeid(otherOperand) != typeid(classOp->otherType)) isOperatorFound = false;;
			}

			if (isOperatorFound) break;
		}

		if (isSymetric) return isSymetrisized && isOperatorFound;
		else return isOperatorFound;
	}
	else throw std::runtime_error("Type not identified!");

	return opHandled.count(operatorType);
}

bool AST::GenUse_TraitAST::isTypeValid(std::map<std::string, Def_TypeBaseAST*>& types) const {
	if (!types.count(targetGenSym)) return false;
	auto targetType = types.find(targetGenSym)->second;

	if (auto class_ptr = dynamic_cast<Def_ClassAST*>(targetType)) {
		for (auto& impl : class_ptr->implementations) {
			if (impl->name == name) return true;
		}
	}

	return false;
}

bool AST::GenUse_TypeAliasAST::isTypeValid(std::map<std::string, Def_TypeBaseAST*>& types) const {
	if (!types.count(targetGenSym)) return false;
	auto targetType = types.find(targetGenSym)->second;

	if (auto class_ptr = dynamic_cast<Def_ClassAST*>(targetType)) {
		for (auto& type : class_ptr->aliases) {
			if (type->name == alias) return true;
		}
	}

	return false;
}

bool AST::GenUse_MethodAST::isTypeValid(std::map<std::string, Def_TypeBaseAST*>& types) const {
	if (!types.count(targetGenSym)) return false;
	auto targetType = types.find(targetGenSym)->second;

	if (auto class_ptr = dynamic_cast<Def_ClassAST*>(targetType)) {
		for (auto& method : class_ptr->methods) {
			if (method->name == callee) {

				// parameters check
				const size_t paramSize = method->body->parameters.size();
				if (paramSize != parameters.size()) continue;

				for (size_t i = 0; i < paramSize; i++) {
					auto methodParam = method->body->parameters[i].get();
					auto genParam = parameters[i].get();
					if (methodParam->mangling() != genParam->mangling()) {
						// different params
						return false;
					}
				}

				// return check
				const size_t retSize = method->body->returnType ? method->body->returnType->types.size() : 0;
				if (retSize == 0 && !returnType) return true;
				if (retSize != returnType->types.size()) return false;

				for (size_t i = 0; i < retSize; i++) {
					auto methodRet = method->body->returnType->types[i].get();
					auto genRet = returnType->types[i].get();
					if (methodRet->mangling() != genRet->mangling()) {
						// different returns
						return false;
					}
				}

				return true;
			}
		}
	}

	return false;
}

bool AST::Def_MetacodeAST::isScoped() const {
	return scope_name != "" ? true : scope_endPos != scope_startPos;
}

bool AST::Def_MetacodeAST::isConcerned(size_t position) const {
	if (isScoped()) {
		return
			scope_startPos <= position &&
			scope_endPos >= position;
	}
	else {
		return next_valid_tok == position;
	}
}

Value* AST::Def_MetacodeAST::codegen()
{
	return nullptr;
}

Value* AST::Inst_AddrOfAST::codegen() const {
	Value* val = target->codegen();
	if (!val) {
		return nullptr;
	}

	if (val->getType()->isPointerTy()) {
		return val;
	}

	throw std::runtime_error("Impossible to take the address of an rvalue!");
}

Value* AST::Inst_DerefAST::codegen() const {
	Value* ptr = target->codegen();
	if (!ptr) {
		return nullptr;
	}

	if (!ptr->getType()->isPointerTy()) {
		throw std::runtime_error("Impossible to deref a non pointer!");
	}

	/*
	Type* refTy;

	swi

	return infoScript.builder.CreateLoad(ptr->getType()->, ptr, "deref");*/
/*
	return nullptr;
}

std::string AST::Inst_PathAST::print_path() const {
	auto printAccess = [&](AST::Inst_PathSegmentAST::AccessKind access) {
		switch (access)
		{
		case AST::Inst_PathSegmentAST::AccessKind::Dot: return ".";
		case AST::Inst_PathSegmentAST::AccessKind::Static: return "::";
		default: return "";
		}
		};

	std::string outStr;
	for (size_t i = 0; i < segments.size(); i++) {
		outStr += i > 0 ? printAccess(segments[i - 1]->accessKind) : "";

		switch (segments[i]->segmentype)
		{
		case AST::Inst_PathSegmentAST::Segmentype::Identifier: {
			auto idNode = dynamic_cast<AST::IdAST*>(segments[i]->linkedNode.get());
			outStr += idNode->identifier;
			outStr += idNode->generic_args.empty() ? "" : "<...>";
			break;
		}
		case AST::Inst_PathSegmentAST::Segmentype::Call: {
			auto callNode = dynamic_cast<AST::Inst_CallAST*>(segments[i]->linkedNode.get());
			outStr += callNode->callee_name;
			outStr += callNode->gen_args.empty() ? "" : "<...>";
			outStr += "(...)";
			break;
		}
		case AST::Inst_PathSegmentAST::Segmentype::PipeCall: {
			auto pipeCallNode = dynamic_cast<AST::Inst_PipeCallAST*>(segments[i]->linkedNode.get());
			outStr += pipeCallNode->callee_name;
			outStr += pipeCallNode->gen_args.empty() ? "" : "<...>";
			outStr += " |...|";
			break;
		}
		case AST::Inst_PathSegmentAST::Segmentype::ClassInst: {
			auto classInstNode = dynamic_cast<AST::Inst_ClassAST*>(segments[i]->linkedNode.get());
			outStr += classInstNode->name;
			outStr += classInstNode->gen_args.empty() ? "" : "<...>";
			outStr += " {...}";
			break;
		}
		}
	}

	return outStr;
}

struct Scope;

std::shared_ptr<Scope> AST::Inst_PathAST::getScope() {
	if (segments.empty()) return nullptr;
	auto last = segments.back().get();
	return last->_scopePosition;
}

// use empty string argument "" to specify any pattern element and get his value
// e.g. "import", "" -> if an import exist, the return pattern will be "import", "module_name"
std::optional<std::vector<Token>> AST::Inst_MetacodeAST::contains_pattern(const std::initializer_list<std::string>& inPattern) const {
	std::vector<std::string> pattern{ inPattern };
	std::vector<Token> result(pattern.size());
	if (pattern.empty()) return std::nullopt;

	for (size_t i = 0; i + pattern.size() <= elems.size(); i++) {
		bool match = true;
		for (size_t j = 0; j < pattern.size(); j++) {
			// "" est un joker qui match tout
			if (pattern[j] != "" && elems[i + j].value != pattern[j]) {
				match = false;
				break;
			}
			else {
				result[j] = elems[i + j];
			}
		}
		if (match) {
			return result;
		}
	}

	return std::nullopt;
}

std::optional<std::vector<Token>> AST::Def_MetacodeAST::contains_pattern(const std::initializer_list<std::string>& inPattern) const {
	for (const auto& elem : lines) {
		if (auto pattern = elem->contains_pattern(inPattern); pattern.has_value()) {
			return pattern;
		}
	}

	return std::nullopt;
}
*/

