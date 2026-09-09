#include "codegen/insurance.hpp"

#include "ast/node/literal.hpp"
#include "codegen/codegen.hpp"
#include "codegen/tool.hpp"
#include "codegen/type.hpp"
#include "id/nodeid.hpp"
#include "id/typeid.hpp"
#include "type/type.hpp"

#include <cassert>
#include <llvm-19/llvm/IR/Constant.h>
#include <llvm-19/llvm/IR/IRBuilder.h>
#include <llvm-19/llvm/IR/Value.h>
#include <string_view>

namespace codegen
{
class Builder : public llvm::IRBuilder<>
{
public:
  using llvm::IRBuilder<>::IRBuilder;
};

} // namespace codegen


codegen::Insurance::Insurance(codegen::Codegen_AST& p_res)
  : res(p_res)
  , ctx(res.ctx)
  , builder(res.builder)
  , mod(res.mod)
{
}

llvm::Value* codegen::Insurance::ensure_rvalue(ast::ID expr, std::string_view name) noexcept
{
  // if (expr.is_rvalue()) return codegen_node(expr);

  auto* v = res.codegen_node(expr);
  assert(v);
  v->setName(name);

  auto* ty = v->getType();

  // already value not pointer
  if (!ty->isPointerTy()) return v;

  // pointer -> load
  return builder.CreateLoad(res.get_type(expr.type()), v, name);
}


llvm::Value* codegen::Insurance::ensure_lvalue(ast::ID expr, bool is_silent_error) noexcept
{
  // if (expr.is_lvalue()) return codegen_node(expr);

  auto* v = res.codegen_node(expr);
  assert(v);

  // mut be llvm ptr
  if (v->getType()->isPointerTy()) return v;

  if (!is_silent_error)
    res.add_error(225, expr.get(), "Expected a lvalue expression.", "a lvalue is frequently a variable.");
  return nullptr;
}

llvm::Value* codegen::Insurance::ensure_variadic_arg(ast::ID expr) noexcept
{
  auto* ty = res.get_type(expr.type());

  // C array-to-pointer conversion:
  //
  // T[N] -> T*
  //
  // The expression itself is not loaded: we want the address
  // of the first element.
  if (ty->isArrayTy()) {
    auto* ptr = res.codegen_node(expr);

    return builder.CreateGEP(ty, ptr, {res.const_int(0), res.const_int(0)}, "array.data");
  }

  // Variadic arguments receive the value of the expression.
  auto* value    = ensure_rvalue(expr);
  auto* value_ty = value->getType();

  // C default argument promotions:
  //
  // bool / char / short / signed variants / unsigned variants
  // narrower than int are promoted to int or unsigned int.
  // primitive_coerce should preserve the appropriate promotion.
  if (value_ty->isIntegerTy() && value_ty->getIntegerBitWidth() < 32) {

    return res.tools.primitive_coerce(value, value_ty, LLVM_TYPEID_s32);
  }

  // float -> double
  if (value_ty->isFloatingPointTy() && value_ty->getPrimitiveSizeInBits() < 64) {

    return res.tools.primitive_coerce(value, value_ty, LLVM_TYPEID_f64);
  }

  // Pointers, doubles, 32/64-bit integers, structs, etc.
  // are passed as-is.
  return value;
}

llvm::Constant* codegen::Insurance::ensure_constant(ast::ID expr) noexcept
{
  return nullptr;
}


llvm::Value* codegen::Insurance::ensure_init_global_value(ast::ID expr, bool is_const) noexcept
{
  if (auto* ptr = expr.as<ast::Literal_Table>()) {
    auto* v  = res.codegen_Literal_Table(*ptr);
    auto* ty = expr.type().as<type::Array>();
    assert(ty);
    return res.tools.init_global_array(*ty, v, false, is_const);
  }

  return res.codegen_node(expr);
}

llvm::Constant* codegen::Insurance::ensure_null_global_value(type::ID tyid) noexcept
{
  if (auto* ptr = tyid.as<type::Array>()) {
    // never uninit and const when null val
    return res.tools.init_global_array(*ptr, nullptr, false, false);
  }

  auto* ty = res.get_type(tyid);
  assert(ty);
  return llvm::Constant::getNullValue(ty);
}


llvm::Value* codegen::Insurance::ensure_init_value(ast::ID expr) noexcept
{
  if (auto* ptr = expr.as<ast::Literal_Table>()) {
    auto* v  = res.codegen_Literal_Table(*ptr);
    auto* ty = expr.type().as<type::Array>();
    assert(ty);
    return res.tools.init_array(*ty, v, false);
  }

  return res.codegen_node(expr);
}

llvm::Value* codegen::Insurance::ensure_null_value(type::ID tyid) noexcept
{
  if (auto* ptr = tyid.as<type::Array>()) {
    return res.tools.init_array(*ptr, nullptr, false);
  }

  auto* ty = res.get_type(tyid);
  assert(ty);
  return llvm::Constant::getNullValue(ty);
}
