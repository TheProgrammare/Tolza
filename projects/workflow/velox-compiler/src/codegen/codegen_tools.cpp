#include "codegen_tools.hpp"

#include <expected>

#include <llvm/ADT/APInt.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Instructions.h>

#include <llvm/IR/Constants.h>
#include <llvm/IR/Type.h>
#include <vector>

#include <compiler_context.hpp>

#include "ast/ast_base.hpp"
#include "ast/ast_declaration_cop.hpp"
#include "ast/ast_expression.hpp"
#include "ast/ast_data.hpp"
#include "ast/ast_declaration.hpp"
#include "ast/ast_declaration_local.hpp"
#include "ast/ast_literal.hpp"
#include "ast/ast_type.hpp"
#include "ast/ast_operation.hpp"

#include "compiler/compiler.hpp"
#include "misc/error_output.hpp"
#include "misc/metacode.hpp"
#include "visitor/symbol_manager.hpp"
#include "visitor_codegen.hpp"

llvm::Value* LLVM_Tools::engage_move_semantic(ast::AExpression& target)
{
  auto src = target.codegen(v);
  auto ty  = target.inferred_type->codegen_ty(v);

  auto dest = v.builder.CreateAlloca(ty, nullptr, "tmp_moved");

  if (auto ptr = dynamic_cast<ast::declaration::cop::Entity*>(target.inferred_type)) {
  }
}
llvm::Value* LLVM_Tools::engage_copy_semantic(ast::AExpression& target)
{
}
llvm::Value* LLVM_Tools::engage_clone_semantic(ast::AExpression& target)
{
}

std::expected<Symbol_Data*, std::string> LLVM_Tools::find_symbol(const ast::AExpression& expr)
{
  if (auto ptr = dynamic_cast<const ast::AIdentifier*>(&expr)) {
    return ptr->symbol;
  } else if (auto ptr = dynamic_cast<const ast::expression::Member_Access*>(&expr)) {
    return find_symbol(*ptr->right);
  } else if (auto ptr = dynamic_cast<const ast::expression::Call*>(&expr)) {
    return find_symbol(*ptr->callee);
  } else if (auto ptr = dynamic_cast<const ast::expression::Call_Pipe*>(&expr)) {
    return find_symbol(*ptr->callee);
  }
}

std::expected<ast::AExpression*, std::string> LLVM_Tools::get_symbol_expression(const Symbol_Data& symbol)
{
  auto decl = symbol.symbol.get();
  if (auto ptr = dynamic_cast<ast::declaration::Global*>(decl)) {
    if (ptr->expression) return ptr->expression.get();
  } else if (auto ptr = dynamic_cast<ast::declaration::local::Variable*>(decl)) {
    if (ptr->expression) return ptr->expression.get();
  } else if (auto ptr = dynamic_cast<ast::declaration::local::Variable_Binding*>(decl)) {
    return ptr->parent_pattern->right.get();
  }
  Error_Diagnostic error(166, *decl->_scr_info, decl->_token, {}, compiler::EPhase::llvmir, EErrorSeverity::error, {},
                         "The symbol don't have an expression.", "");
  return std::unexpected(error.print_error());
}

std::expected<llvm::Constant*, std::string> LLVM_Tools::create_constant(const ast::AType&    ty,
                                                                        const ast::ALiteral& value)
{
  common::CompCtx ctx;
  if (auto ptr = dynamic_cast<const ast::literal::Integral*>(&value)) {
    llvm::Type* ty = nullptr;
    switch (ptr->type) {
    case EPrimType::ptrdiff:
    case EPrimType::uSize:
    case EPrimType::bSize:
    case EPrimType::iSize:   ty = llvm::Type::getIntNTy(v.ctx, compiler::COMP_CTX.get_size_bit()); break;
    case EPrimType::i8:
    case EPrimType::u8:
    case EPrimType::b8:      ty = v.i8Ty; break;
    case EPrimType::i16:
    case EPrimType::u16:
    case EPrimType::b16:     ty = v.i16Ty; break;
    case EPrimType::i32:
    case EPrimType::u32:
    case EPrimType::b32:     ty = v.i32Ty; break;
    case EPrimType::i64:
    case EPrimType::u64:
    case EPrimType::b64:     ty = v.i64Ty; break;
    case EPrimType::i128:
    case EPrimType::u128:
    case EPrimType::b128:    ty = v.i128Ty; break;
    default:                 {
      Error_Diagnostic error(167, *value._scr_info, value._token, {}, compiler::EPhase::llvmir, EErrorSeverity::error,
                             {}, "Invalid type.", "");
      return std::unexpected(error.print_error());
    }
    }

    return llvm::ConstantInt::get(ty, *ptr->val.val);
  } else if (auto ptr = dynamic_cast<const ast::literal::Floating*>(&value)) {
    llvm::Type* ty = nullptr;
    switch (ptr->type) {
    case EPrimType::fSize: ty = v.f64Ty; break;
    case EPrimType::f32:   ty = v.f32Ty; break;
    case EPrimType::f64:   ty = v.f64Ty; break;
    case EPrimType::f128:  ty = v.f128Ty; break;
    default:               {
      Error_Diagnostic error(168, *value._scr_info, value._token, {}, compiler::EPhase::llvmir, EErrorSeverity::error,
                             {}, "Invalid type.", "");
      return std::unexpected(error.print_error());
    }
    }

    return llvm::ConstantFP::get(ty, *ptr->val.val);
  } else if (auto ptr = dynamic_cast<const ast::literal::Decimal*>(&value)) {

  } else if (auto ptr = dynamic_cast<const ast::literal::Boolean*>(&value)) {
    return llvm::ConstantInt::get(v.i1Ty, ptr->val);
  } else if (auto ptr = dynamic_cast<const ast::literal::Enum*>(&value)) {
    if (auto ptr2 = dynamic_cast<ast::declaration::Enum*>(ptr->inferred_type)) {
      // return llvm::ConstantStruct::get(v.visit(*ptr2), v.visit(ptr));
    }
    // llvm::StructType* ty = llvm::StructType::create(v.ctx, )
  }
}

llvm::Type* LLVM_Tools::generate_parameter_type(ast::declaration::local::Parameter& param)
{
  param.codegen_pass(v);
  auto ty = param.type->codegen_ty(v);

  switch (param.passMode) {
  case EPassMode::NONE:
  case EPassMode::Move:
  case EPassMode::Ref:  {
    if (ty->isSingleValueType())
      return ty;

    else
      return param.type->llvm_type = ty->getPointerTo();
  }
  case EPassMode::Mut: {
    return ty->getPointerTo();
  }
  case EPassMode::Copy:
  case EPassMode::Clone: {
    if (!ty->isSingleValueType())
      return param.type->llvm_type = ty->getPointerTo();
    else
      return ty;
  }
  case EPassMode::Addr: {
    return param.type->llvm_type = ty->getPointerTo()->getPointerTo();
  }
  }
}


llvm::Type* LLVM_Tools::get_primtive_type(EPrimType ty)
{
  switch (ty) {
  case EPrimType::boolean: return v.i1Ty;
  case EPrimType::ASCII:   return v.i8Ty;
  case EPrimType::UTF32:   return v.i32Ty;
  case EPrimType::iSize:
  case EPrimType::uSize:
  case EPrimType::bSize:   return v.iSizeTy;
  case EPrimType::i8:
  case EPrimType::u8:
  case EPrimType::b8:      return v.i8Ty;
  case EPrimType::i16:
  case EPrimType::u16:
  case EPrimType::b16:     return v.i16Ty;
  case EPrimType::i32:
  case EPrimType::u32:
  case EPrimType::b32:     return v.i32Ty;
  case EPrimType::i64:
  case EPrimType::u64:
  case EPrimType::b64:     return v.i64Ty;
  case EPrimType::i128:
  case EPrimType::u128:
  case EPrimType::b128:    return v.i128Ty;
  case EPrimType::ptrdiff: return v.iSizeTy;
  case EPrimType::fSize:   return v.fSizeTy;
  case EPrimType::f32:     return v.f32Ty;
  case EPrimType::f64:     return v.f64Ty;
  case EPrimType::f128:    return v.f128Ty;
  case EPrimType::u0:      return v.u0Ty;
  case EPrimType::deci:
  case EPrimType::udeci:   return v.i128Ty;
  case EPrimType::Flag:    return v.iSizeTy;
  case EPrimType::str:     return v.strTy;
  case EPrimType::c_str:   return v.cstrTy;
  default:                 {
    return nullptr;
  }
  }
}
