
#include "VisitorSemantic.hpp"

#include "VisitorType.hpp"
#include "AST/ASTNodes.hpp"


void VisitorSem::visit(AST::Op_Index& n) {
	
	auto base_ty = get_type(n.target.get());
	if (!base_ty.has_value()) {
		error_add(*n.target, "SEM1000", ERR_TY_NOT_FOUND, HINT_NOT_FOUND);
	}
}



