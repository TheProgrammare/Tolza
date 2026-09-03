#include "static_evaluation.hpp"

#include "ast/data.hpp"
#include "ast/definition.hpp"
#include "ast/definition/ast_declaration_global.hpp"
#include "ast/definition/ast_declaration_local.hpp"
#include "ast/definition/ast_literal.hpp"
#include "ast/definition/ast_numeric_128_bits.hpp"
#include "ast/definition/ast_operation.hpp"
#include "ast/forward.hpp"
#include "codegen/codegen.hpp"
#include "codegen/codegen_type.hpp"
#include "codegen_tools.hpp"
#include "compiler/compilation_unit.hpp"
#include "compiler/compiler.hpp"
#include "misc/error_output.hpp"
#include "type/data.hpp"

#include <cmath>
#include <expected>
#include <llvm-19/llvm/IR/Constant.h>
#include <llvm-19/llvm/IR/Constants.h>
#include <llvm-19/llvm/IR/Type.h>
#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/APInt.h>

codegen::Static_Evaluator::Static_Evaluator(codegen::Codegen_AST& p_res)
  : tools(new Tools(p_res))
  , res(p_res)
{
}


std::expected<llvm::Constant*, std::string> codegen::Static_Evaluator::evaluate_expression(ast::ID expr) noexcept
{
  expr = expr.canonical();

  if (auto* ptr = expr.as<ast::Operation_Binary>()) {
    auto lhs = evaluate_expression(ptr->left);
    if (!lhs) return std::unexpected(lhs.error());

    auto rhs = evaluate_expression(ptr->right);
    if (!rhs) return std::unexpected(rhs.error());

    auto& L = *lhs.value();
    auto& R = *rhs.value();

    if (auto* L_ptr = ptr->left.as<ast::Literal_Integral>()) {
      auto* R_ptr = ptr->right.as<ast::Literal_Integral>();
      return integral(*L_ptr, *R_ptr, ptr->op_ty);
    }
    if (auto* L_ptr = ptr->left.as<ast::Literal_Floating_Point>()) {
      auto* R_ptr = ptr->right.as<ast::Literal_Floating_Point>();
      return floating(*L_ptr, *R_ptr, ptr->op_ty);
    }
    if (auto* L_ptr = ptr->left.as<ast::Literal_Fixed_Point>()) {
      auto* R_ptr = ptr->right.as<ast::Literal_Fixed_Point>();
      return decimal(*L_ptr, *R_ptr, ptr->op_ty);
    }
    if (auto* L_ptr = ptr->left.as<ast::Literal_Boolean>()) {
      auto* R_ptr = ptr->right.as<ast::Literal_Boolean>();
      return boolean(*L_ptr, *R_ptr, ptr->op_ty);
    }
  } else if (auto* ptr = expr.as<ast::Operation_Unary>()) {
    switch (ptr->unary_op) {
    case ast::EOp_Unary::NONE:         return evaluate_expression(ptr->base);
    case ast::EOp_Unary::_not:         return boolean_not(ptr->base);
    case ast::EOp_Unary::_plus:        return scalar_plus(ptr->base);
    case ast::EOp_Unary::_minus:       return scalar_minus(ptr->base);
    case ast::EOp_Unary::_invert_sign: return boolean_not(ptr->base);
    }
  }

  if (auto ptr = tools->create_constant(expr)) return ptr.value();

  if (auto* glo = expr.as<ast::Global_Variable>()) return evaluate_expression(glo->expression);
  if (auto* loc = expr.as<ast::Local_Variable>()) return evaluate_expression(loc->expression);

  Error_Diagnostic err(res.CU.cuid, 169, expr, compiler::EPhase::llvmir,
                       "The expression can't be evaluated at compilation time", "");
  return std::unexpected(err.print_userfriendly_error());
}


std::expected<llvm::Constant*, std::string> codegen::Static_Evaluator::integral(const ast::Literal_Integral& L,
                                                                                const ast::Literal_Integral& R,
                                                                                ast::EOp_Bin op) noexcept
{
  auto to_int = [&](const llvm::APInt& value) {
    return llvm::ConstantInt::getIntegerValue(res.get_type(L.nodeid().type()), value);
  };
  auto to_fp = [&](double value) { return llvm::ConstantFP::get(res.get_type(L.nodeid().type()), value); };

  auto to_bool = [](bool value) { return llvm::ConstantInt::get(codegen::LLVM_TYPEID_bool, value); };

  auto& L_val = *L.val.val;
  auto& R_val = *R.val.val;

  bool is_signed = type::EPrimitiveTypeKind_is_signed(L.type);

  if (type::EPrimitiveTypeKind_is_integral(L.type)) {
    switch (op) {
    case ast::EOp_Bin::_add: {
      return to_int(L_val + R_val);
    }
    case ast::EOp_Bin::_sub: {
      return to_int(L_val - R_val);
    }
    case ast::EOp_Bin::_mul: {
      return to_int(L_val * R_val);
    }
    case ast::EOp_Bin::_div: {
      double L_f;
      double R_f;

      if (is_signed) {
        L_f = L_val.signedRoundToDouble();
        R_f = R_val.signedRoundToDouble();
      } else {
        L_f = L_val.roundToDouble();
        R_f = R_val.roundToDouble();
      }

      return to_fp(L_f / R_f);
    }
    case ast::EOp_Bin::_mod: {
      if (is_signed) to_int(L_val.srem(R_val));
      return to_int(L_val.urem(R_val));
    }
    case ast::EOp_Bin::_quo: {
      if (is_signed) {
        llvm::APInt r = L_val.srem(R_val);

        if (r.isNegative()) r += R_val.abs();

        return to_int(r);
      }
      return to_int(L_val.urem(R_val));
    }
    case ast::EOp_Bin::_rem: {
      if (is_signed) {
        llvm::APInt q = L_val.sdiv(R_val);
        llvm::APInt r = L_val.srem(R_val);

        if (r.isNegative()) {
          llvm::APInt absB = R_val.abs();
          if (R_val.isNegative())
            q += 1;
          else
            q -= 1;
        }

        return to_int(q);
      }

      return to_int(L_val.udiv(R_val));
    }
    case ast::EOp_Bin::_divrem: {
      Error_Diagnostic err(res.CU.cuid, 170, L.nodeid(), compiler::EPhase::llvmir,
                           "Unexpected operation for compilation time evaluation.", "");
      return std::unexpected(err.print_userfriendly_error());
    }
    case ast::EOp_Bin::_pow: {
      llvm::APInt result = llvm::APInt(L_val.getBitWidth(), 1);
      auto&       base   = L_val;
      auto&       exp    = R_val;
      while (!exp.isZero()) {
        if ((*R.val.val)[0]) // low bit
          result *= base;
        exp = exp.lshr(1); // div exp by 2
        base *= base;      // fast exponentiation
      }
      return to_int(result);
    }
    case ast::EOp_Bin::_gre: {
      if (is_signed) return to_bool(L_val.sgt(R_val));
      return to_bool(L_val.ugt(R_val));
    }
    case ast::EOp_Bin::_low: {
      if (is_signed) return to_bool(L_val.slt(R_val));
      return to_bool(L_val.ult(R_val));
    }
    case ast::EOp_Bin::_gre_eq: {
      if (is_signed) return to_bool(L_val.sge(R_val));
      return to_bool(L_val.uge(R_val));
    }
    case ast::EOp_Bin::_low_eq: {
      if (is_signed) return to_bool(L_val.sle(R_val));
      return to_bool(L_val.ule(R_val));
    }
    case ast::EOp_Bin::_eq:
    case ast::EOp_Bin::_eqs:
    case ast::EOp_Bin::_is:
    case ast::EOp_Bin::_in:  {
      return to_bool(L_val.eq(R_val));
    }
    case ast::EOp_Bin::_neq:
    case ast::EOp_Bin::_neqs:
    case ast::EOp_Bin::_nis:
    case ast::EOp_Bin::_nin:  {
      return to_bool(L_val.ne(R_val));
    }
    default: {
      Error_Diagnostic err(res.CU.cuid, 171, L.nodeid(), compiler::EPhase::llvmir, "Unexpected operation on integral.",
                           "");
      return std::unexpected(err.print_userfriendly_error());
    }
    }
  } else if (type::EPrimitiveTypeKind_is_byte(L.type)) {
    llvm::APInt mask = llvm::APInt::getAllOnes(L_val.getBitWidth());

    switch (op) {
    case ast::EOp_Bin::_and:
    case ast::EOp_Bin::_b_and: {
      return to_int(L_val & R_val);
    }
    case ast::EOp_Bin::_nand:
    case ast::EOp_Bin::_b_nand: {
      return to_int(~(L_val & R_val) & mask);
    }
    case ast::EOp_Bin::_or:
    case ast::EOp_Bin::_b_or: {
      return to_int(L_val | R_val);
    }
    case ast::EOp_Bin::_xor:
    case ast::EOp_Bin::_b_xor: {
      return to_int(L_val ^ R_val);
    }
    case ast::EOp_Bin::_nor:
    case ast::EOp_Bin::_b_nor: {
      return to_int(~(L_val | R_val) & mask);
    }
    case ast::EOp_Bin::_xnor:
    case ast::EOp_Bin::_b_xnor: {
      return to_int(~(L_val ^ R_val) & mask);
    }
    case ast::EOp_Bin::_b_shl_0: {
      return to_int(L_val.shl(R_val.getLimitedValue()));
    }
    case ast::EOp_Bin::_b_shl_1: {
      unsigned width = L_val.getBitWidth();
      unsigned shift = R_val.getLimitedValue();
      auto     r     = L_val.shl(shift);
      auto     fill  = llvm::APInt::getAllOnes(width).lshr(width - shift);
      r |= fill;
      return to_int(r);
    }
    case ast::EOp_Bin::_b_shl_a: {
    }
    case ast::EOp_Bin::_b_shr_0: {
      return to_int(L_val.lshr(R_val.getLimitedValue()));
    }
    case ast::EOp_Bin::_b_shr_1: {
      unsigned width = L_val.getBitWidth();
      unsigned shift = R_val.getLimitedValue();

      llvm::APInt r    = L_val.lshr(shift);                                 // shift classique
      llvm::APInt fill = llvm::APInt::getAllOnes(width).shl(width - shift); // bits de gauche mis à 1
      r |= fill;
      return to_int(r);
    }
    case ast::EOp_Bin::_b_shr_a: {
      return to_int(L_val.ashr(R_val.getLimitedValue()));
    }
    case ast::EOp_Bin::_b_rol: {
      return to_int(L_val.rotl(R_val.getLimitedValue()));
    }
    case ast::EOp_Bin::_b_ror: {
      return to_int(L_val.rotr(R_val.getLimitedValue()));
    }
    default: {
      Error_Diagnostic err(res.CU.cuid, 172, L.nodeid(), compiler::EPhase::llvmir, "Unexpected operation on byte.", "");
      return std::unexpected(err.print_userfriendly_error());
    }
    }
  }

  Error_Diagnostic err(res.CU.cuid, 173, L.nodeid(), compiler::EPhase::llvmir, "Unexpected type.", "");
  return std::unexpected(err.print_userfriendly_error());
}
std::expected<llvm::Constant*, std::string> codegen::Static_Evaluator::floating(const ast::Literal_Floating_Point& L,
                                                                                const ast::Literal_Floating_Point& R,
                                                                                ast::EOp_Bin op) noexcept
{
  auto to_fp = [&](const llvm::APFloat& value) {
    return llvm::ConstantFP::get(res.get_type(L.nodeid().type()), value);
  };
  auto to_bool = [](bool value) { return llvm::ConstantInt::get(codegen::LLVM_TYPEID_bool, value); };

  auto& L_val = *L.val.val;
  auto& R_val = *R.val.val;

  bool is_signed = type::EPrimitiveTypeKind_is_signed(L.type);

  switch (op) {
  case ast::EOp_Bin::_add: {
    return to_fp(L_val + R_val);
  }
  case ast::EOp_Bin::_sub: {
    return to_fp(L_val - R_val);
  }
  case ast::EOp_Bin::_mul: {
    return to_fp(L_val * R_val);
  }
  case ast::EOp_Bin::_div: {
    return to_fp(L_val / R_val);
  }
  case ast::EOp_Bin::_mod: {
    llvm::APFloat q = L_val;
    q.divide(R_val, llvm::APFloat::rmTowardZero);
    q.roundToIntegral(llvm::APFloat::rmTowardZero);

    llvm::APFloat qb = q;
    qb.multiply(R_val, llvm::APFloat::rmNearestTiesToEven);

    llvm::APFloat r = L_val;
    r.subtract(qb, llvm::APFloat::rmNearestTiesToEven);

    if (r.isNegative()) {
      llvm::APFloat abs_b = R_val;
      abs_b.clearSign();
      r.add(abs_b, llvm::APFloat::rmNearestTiesToEven);
    }

    return to_fp(r);
  }
  case ast::EOp_Bin::_quo: {
    Error_Diagnostic err(res.CU.cuid, 174, L.nodeid(), compiler::EPhase::llvmir,
                         "Unexpected opration for floating type, use floor(fsize)", "");
  }
  case ast::EOp_Bin::_rem: {
    llvm::APFloat q = L_val;
    q.divide(R_val, llvm::APFloat::rmTowardZero);   // q = a/b
    q.roundToIntegral(llvm::APFloat::rmTowardZero); // trunc

    llvm::APFloat qb = q;
    qb.multiply(R_val, llvm::APFloat::rmNearestTiesToEven);

    llvm::APFloat r = L_val;
    r.subtract(qb, llvm::APFloat::rmNearestTiesToEven);

    return to_fp(r);
  }
  case ast::EOp_Bin::_divrem: {

    Error_Diagnostic err(res.CU.cuid, 175, L.nodeid(), compiler::EPhase::llvmir,
                         "Unexpected operation for compilation time evaluation.", "");
    return std::unexpected(err.print_userfriendly_error());
  }
  case ast::EOp_Bin::_pow: {
    double base = L_val.convertToDouble();
    double exp  = R_val.convertToDouble();

    if (base == 0.0 && exp == 0.0) return std::unexpected("0 ** 0 is undefined");

    double result = std::pow(base, exp);

    if (std::isnan(result)) return std::unexpected("invalid floating power");

    return llvm::ConstantFP::get(res.get_type(L.nodeid().type()), result);
  }
  case ast::EOp_Bin::_gre: {
    return to_bool(L_val > R_val);
  }
  case ast::EOp_Bin::_low: {
    return to_bool(L_val < R_val);
  }
  case ast::EOp_Bin::_gre_eq: {
    return to_bool(L_val >= R_val);
  }
  case ast::EOp_Bin::_low_eq: {
    return to_bool(L_val <= R_val);
  }
  case ast::EOp_Bin::_eq:
  case ast::EOp_Bin::_in: {
    return to_bool(float_almost_eq_ULP(L_val, R_val));
  }
  case ast::EOp_Bin::_neq:
  case ast::EOp_Bin::_nin: {
    return to_bool(!float_almost_eq_ULP(L_val, R_val));
    bool test = L_val == R_val;
  }
  case ast::EOp_Bin::_eqs:
  case ast::EOp_Bin::_is:  {
    return to_bool(L_val == R_val);
  }
  case ast::EOp_Bin::_neqs:
  case ast::EOp_Bin::_nis:  {
    return to_bool(L_val != R_val);
  }
  default: {
    Error_Diagnostic err(res.CU.cuid, 176, L.nodeid(), compiler::EPhase::llvmir, "Unexpected operation on byte.", "");
    return std::unexpected(err.print_userfriendly_error());
  }
  }
}


bool codegen::Static_Evaluator::float_almost_eq_ULP(const llvm::APFloat& L, const llvm::APFloat& R,
                                                    unsigned maxULP) noexcept
{
  if (L.isNaN() || R.isNaN()) return false;

  if (L.compare(R) == llvm::APFloat::cmpEqual) return true;

  llvm::APInt li = L.bitcastToAPInt();
  llvm::APInt ri = R.bitcastToAPInt();

  unsigned bw        = li.getBitWidth();
  auto     sign_mask = llvm::APInt::getSignMask(bw);

  auto ordered = [&](llvm::APInt x) {
    if (x.isNegative()) return ~x;
    return x | sign_mask;
  };

  li = ordered(li);
  ri = ordered(ri);

  auto diff = li.uge(ri) ? li - ri : ri - li;

  return diff.ule(maxULP);
}

std::expected<llvm::Constant*, std::string> codegen::Static_Evaluator::decimal(const ast::Literal_Fixed_Point& L,
                                                                               const ast::Literal_Fixed_Point& R,
                                                                               ast::EOp_Bin op) noexcept
{
  auto to_dec = [&](const llvm::APInt& v) { return llvm::ConstantInt::get(res.get_type(L.nodeid().type()), v); };

  llvm::APInt L_val = *L.val.val;
  llvm::APInt R_val = *R.val.val;

  const size_t max_scale = std::max(L.scale, R.scale);

  auto align_scales = [&](llvm::APInt& v, size_t s) {
    size_t diff = max_scale - s;
    if (diff > 0) v <<= diff; // FIX: power-of-two scaling
  };

  switch (op) {

  case ast::EOp_Bin::_add:
  case ast::EOp_Bin::_sub: {
    align_scales(L_val, L.scale);
    align_scales(R_val, R.scale);

    llvm::APInt result = (op == ast::EOp_Bin::_add) ? (L_val + R_val) : (L_val - R_val);

    return to_dec(result);
  }

  case ast::EOp_Bin::_mul: {
    // widen to avoid overflow
    llvm::APInt wideL = L_val.sext(L_val.getBitWidth() * 2);
    llvm::APInt wideR = R_val.sext(R_val.getBitWidth() * 2);

    llvm::APInt shift(wideL.getBitWidth(), max_scale);

    llvm::APInt result = (wideL * wideR);

    if (type::EPrimitiveTypeKind_is_signed(L.raw_type))
      result = result.ashr(shift);
    else
      result = result.lshr(shift);

    return to_dec(result.trunc(L_val.getBitWidth()));
  }

  case ast::EOp_Bin::_div: {
    llvm::APInt scale    = llvm::APInt(L_val.getBitWidth(), 1).shl(max_scale);
    llvm::APInt dividend = L_val * scale;

    llvm::APInt result = type::EPrimitiveTypeKind_is_signed(L.raw_type) ? dividend.sdiv(R_val) : dividend.udiv(R_val);

    return to_dec(result);
  }

  case ast::EOp_Bin::_quo: {
    llvm::APInt result = type::EPrimitiveTypeKind_is_signed(L.raw_type) ? L_val.sdiv(R_val) : L_val.udiv(R_val);

    return to_dec(result);
  }

  case ast::EOp_Bin::_rem: {
    llvm::APInt result = type::EPrimitiveTypeKind_is_signed(L.raw_type) ? L_val.srem(R_val) : L_val.urem(R_val);

    return to_dec(result);
  }

  case ast::EOp_Bin::_eq:
  case ast::EOp_Bin::_neq: {
    align_scales(L_val, L.scale);
    align_scales(R_val, R.scale);

    bool res_bool = (op == ast::EOp_Bin::_eq) ? (L_val == R_val) : (L_val != R_val);

    return llvm::ConstantInt::get(codegen::LLVM_TYPEID_bool, res_bool);
  }

  case ast::EOp_Bin::_divrem: return std::unexpected("Invalid compile-time operation");

  default:                    return std::unexpected("Unexpected operation on Decimal");
  }
}

std::expected<llvm::Constant*, std::string> codegen::Static_Evaluator::boolean(const ast::Literal_Boolean& L,
                                                                               const ast::Literal_Boolean& R,
                                                                               ast::EOp_Bin                op) noexcept
{
  auto to_bool = [](bool value) { return llvm::ConstantInt::get(codegen::LLVM_TYPEID_bool, value); };

  bool L_val = L.val;
  bool R_val = R.val;

  switch (op) {
  // EQUAL
  case ast::EOp_Bin::_eq:
  case ast::EOp_Bin::_is:     return to_bool(L_val == R_val);
  case ast::EOp_Bin::_neq:
  case ast::EOp_Bin::_nis:    return to_bool(L_val != R_val);
  // AND
  case ast::EOp_Bin::_b_and:
  case ast::EOp_Bin::_and:    return to_bool(L_val && R_val);
  case ast::EOp_Bin::_b_nand:
  case ast::EOp_Bin::_nand:   return to_bool(!(L_val && R_val));
  // OR
  case ast::EOp_Bin::_b_or:
  case ast::EOp_Bin::_or:     return to_bool(L_val || R_val);
  case ast::EOp_Bin::_b_nor:
  case ast::EOp_Bin::_nor:    return to_bool(!(L_val || R_val));
  default:                    {
    Error_Diagnostic err(res.CU.cuid, 178, L.nodeid(), compiler::EPhase::llvmir, "Unexpected operation on boolean.",
                         "");
    return std::unexpected(err.print_userfriendly_error());
  }
  }
}

std::expected<llvm::Constant*, std::string> codegen::Static_Evaluator::boolean_not(ast::ID term) noexcept
{
  term = term.canonical();

  if (auto* ptr = term.as<ast::Literal_Boolean>()) {
    bool val = !ptr->val;
    return llvm::ConstantInt::get(codegen::LLVM_TYPEID_bool, val);
  }

  Error_Diagnostic err(res.CU.cuid, 179, term, compiler::EPhase::llvmir, "Unexpected operation 'not' on term.", "");
  return std::unexpected(err.print_userfriendly_error());
}
std::expected<llvm::Constant*, std::string> codegen::Static_Evaluator::scalar_minus(ast::ID term) noexcept
{
  term = term.canonical();

  if (auto* ptr = term.as<ast::Literal_Integral>()) {
    llvm::APInt& val = *ptr->val.val;
    val.negate();
    return llvm::ConstantInt::get(res.get_type(term.type()), val);
  }
  if (auto* ptr = term.as<ast::Literal_Fixed_Point>()) {
    llvm::APInt& val = *ptr->val.val;
    val.negate();
    return llvm::ConstantInt::get(res.get_type(term.type()), val);
  }
  if (auto* ptr = term.as<ast::Literal_Floating_Point>()) {
    llvm::APFloat& val = *ptr->val.val;
    val.changeSign();
    return llvm::ConstantFP::get(res.get_type(term.type()), val);
  }

  Error_Diagnostic err(res.CU.cuid, 180, term, compiler::EPhase::llvmir, "Unexpected operation 'minus' on term.", "");
  return std::unexpected(err.print_userfriendly_error());
}
std::expected<llvm::Constant*, std::string> codegen::Static_Evaluator::scalar_plus(ast::ID term) noexcept
{
  term = term.canonical();

  if (auto* ptr = term.as<ast::Literal_Integral>()) {
    llvm::APInt& val = *ptr->val.val;
    return llvm::ConstantInt::get(res.get_type(term.type()), val);
  }
  if (auto* ptr = term.as<ast::Literal_Fixed_Point>()) {
    llvm::APInt& val = *ptr->val.val;
    return llvm::ConstantInt::get(res.get_type(term.type()), val);
  }
  if (auto* ptr = term.as<ast::Literal_Floating_Point>()) {
    llvm::APFloat& val = *ptr->val.val;
    return llvm::ConstantFP::get(res.get_type(term.type()), val);
  }

  Error_Diagnostic err(res.CU.cuid, 180, term, compiler::EPhase::llvmir, "Unexpected operation 'minus' on term.", "");
  return std::unexpected(err.print_userfriendly_error());
}
