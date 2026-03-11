#pragma once

#include <llvm/IR/Value.h>

#include "ast/ast_forward.hpp"

namespace llvm_tools
{
llvm::Value* create_const(ast::AType& ty, ast::AExpression& value);
}
