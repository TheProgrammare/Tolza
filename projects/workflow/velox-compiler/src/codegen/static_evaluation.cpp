#include "static_evaluation.hpp"

#include <expected>
#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/APInt.h>
#include <cmath>

#include "ast/ast_base.hpp"
#include "ast/ast_data.hpp"
#include "ast/ast_literal.hpp"
#include "ast/ast_numeric_128_bits.hpp"
#include "ast/ast_operation.hpp"
#include "codegen/visitor_codegen.hpp"
#include "codegen_tools.hpp"
#include "compiler/compiler.hpp"
#include "misc/error_output.hpp"

Static_Evaluator::Static_Evaluator(Visitor_Codegen& _v)
  : tools(new LLVM_Tools(_v))
  , v(_v)
{
}


std::expected<ast::ALiteral*, std::string> Static_Evaluator::evaluate_expression(ast::AExpression& value)
{
  if (auto ptr = dynamic_cast<ast::ALiteral*>(&value)) {
    return ptr;
  } else if (auto ptr = dynamic_cast<ast::Expr_ID*>(&value)) {
    auto expr = tools->get_symbol_expression(*ptr->identifier_symbol);

  } else if (auto ptr = dynamic_cast<ast::Expr_ID_Qualified*>(&value)) {
    auto expr = tools->get_symbol_expression(*ptr->identifier_symbol);
  } else if (auto ptr = dynamic_cast<ast::operation::Binary*>(&value)) {
    auto lhs = evaluate_expression(*ptr->left);
    if (!lhs) return std::unexpected(lhs.error());

    auto rhs = evaluate_expression(*ptr->right);
    if (!rhs) return std::unexpected(rhs.error());

    auto& L = *lhs.value();
    auto& R = *rhs.value();

    if (auto L_ptr = dynamic_cast<ast::literal::Integral*>(&L)) {
      auto R_ptr = dynamic_cast<ast::literal::Integral*>(&R);
      return integral(*L_ptr, *R_ptr, ptr->op_ty);
    } else if (auto L_ptr = dynamic_cast<ast::literal::Floating_Point*>(&L)) {
      auto R_ptr = dynamic_cast<ast::literal::Floating_Point*>(&R);
      return floating(*L_ptr, *R_ptr, ptr->op_ty);
    } else if (auto L_ptr = dynamic_cast<ast::literal::Fixed_Point*>(&L)) {
      auto R_ptr = dynamic_cast<ast::literal::Fixed_Point*>(&R);
      return decimal(*L_ptr, *R_ptr, ptr->op_ty);
    } else if (auto L_ptr = dynamic_cast<ast::literal::Boolean*>(&L)) {
      auto R_ptr = dynamic_cast<ast::literal::Boolean*>(&R);
      return boolean(*L_ptr, *R_ptr, ptr->op_ty);
    }
  } else if (auto ptr = dynamic_cast<ast::operation::Unary*>(&value)) {
    auto term = evaluate_expression(*ptr->base);
    if (!term) return std::unexpected(term.error());

    auto& T = *term.value();

    switch (ptr->unary_op) {
    case EUnaryOpType::NONE:         break;
    case EUnaryOpType::_not:         return boolean_not(T);
    case EUnaryOpType::_plus:        return scalar_plus(T);
    case EUnaryOpType::_minus:       return scalar_minus(T);
    case EUnaryOpType::_invert_sign: return boolean_not(T);
    }
  }

  Error_Diagnostic err(v.scr_info, 169, value._scr_info, value._token, compiler::EPhase::llvmir,
                       "The expression can't be evaluated at compilation time", "");
  return std::unexpected(err.print_error());
}


std::expected<ast::ALiteral*, std::string> Static_Evaluator::integral(const ast::literal::Integral& L,
                                                                      const ast::literal::Integral& R, EBinOpType op)
{
  auto to_lit  = [](const Int128& value) { return new ast::literal::Integral(value); };
  auto to_bool = [](bool value) { return new ast::literal::Boolean(value); };

  auto& L_val = *L.val.val;
  auto& R_val = *R.val.val;

  bool is_signed = EPrimType_is_signed(L.type);

  if (EPrimType_is_integral(L.type)) {
    switch (op) {
    case EBinOpType::Add: {
      return to_lit(Int128(L_val + R_val));
    }
    case EBinOpType::Sub: {
      return to_lit(Int128(L_val - R_val));
    }
    case EBinOpType::Mul: {
      return to_lit(Int128(L_val * R_val));
    }
    case EBinOpType::Div: {
      if (is_signed) {
        auto L_f = L_val.signedRoundToDouble();
        auto R_f = R_val.signedRoundToDouble();
        return new ast::literal::Floating_Point(Float128(L_f / R_f));
      } else {
        auto L_f = L_val.roundToDouble();
        auto R_f = R_val.roundToDouble();
        return new ast::literal::Floating_Point(Float128(L_f / R_f));
      }
    }
    case EBinOpType::Mod: {
      if (is_signed)
        return to_lit(Int128(L_val.srem(R_val)));
      else
        return to_lit(Int128(L_val.urem(R_val)));
    }
    case EBinOpType::Quo: {
      if (is_signed) {
        llvm::APInt r = L_val.srem(R_val);

        if (r.isNegative()) r += R_val.abs();

        return to_lit(Int128(r));
      } else {
        return to_lit(Int128(L_val.urem(R_val)));
      }
    }
    case EBinOpType::Rem: {
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

        return to_lit(Int128(q));
      } else {
        return to_lit(Int128(L_val.udiv(R_val)));
      }
    }
    case EBinOpType::Divrem: {
      Error_Diagnostic err(v.scr_info, 170, L._scr_info, L._token, compiler::EPhase::llvmir,
                           "Unexpected operation for compilation time evaluation.", "");
      return std::unexpected(err.print_error());
    }
    case EBinOpType::Pow: {
      Int128 result(1);
      auto&  base = L_val;
      auto&  exp  = R_val;
      while (!exp.isZero()) {
        if ((*R.val.val)[0]) // bit de poids faible
          *result.val *= base;
        exp = exp.lshr(1); // diviser exp par 2
        base *= base;      // exponentiation rapide
      }
      return new ast::literal::Integral(result);
    }
    case EBinOpType::Gre: {
      if (is_signed)
        return to_bool(L_val.sgt(R_val));
      else
        return to_bool(L_val.ugt(R_val));
    }
    case EBinOpType::Low: {
      if (is_signed)
        return to_bool(L_val.slt(R_val));
      else
        return to_bool(L_val.ult(R_val));
    }
    case EBinOpType::Gre_eq: {
      if (is_signed)
        return to_bool(L_val.sge(R_val));
      else
        return to_bool(L_val.uge(R_val));
    }
    case EBinOpType::Low_eq: {
      if (is_signed)
        return to_bool(L_val.sle(R_val));
      else
        return to_bool(L_val.ule(R_val));
    }
    case EBinOpType::_eq:
    case EBinOpType::_eqs:
    case EBinOpType::_is:
    case EBinOpType::_in:  {
      return to_bool(L_val.eq(R_val));
    }
    case EBinOpType::_neq:
    case EBinOpType::_neqs:
    case EBinOpType::_nis:
    case EBinOpType::_nin:  {
      return to_bool(L_val.ne(R_val));
    }
    default: {
      Error_Diagnostic err(v.scr_info, 171, L._scr_info, L._token, compiler::EPhase::llvmir,
                           "Unexpected operation on integral.", "");
      return std::unexpected(err.print_error());
    }
    }
  } else if (EPrimType_is_byte(L.type)) {
    llvm::APInt mask = llvm::APInt::getAllOnes(L_val.getBitWidth());

    switch (op) {
    case EBinOpType::_and:
    case EBinOpType::_b_and: {
      return to_lit(Int128(L_val & R_val));
    }
    case EBinOpType::_nand:
    case EBinOpType::_b_nand: {
      return to_lit(Int128(~(L_val & R_val) & mask));
    }
    case EBinOpType::_or:
    case EBinOpType::_b_or: {
      return to_lit(Int128(L_val | R_val));
    }
    case EBinOpType::_xor:
    case EBinOpType::_b_xor: {
      return to_lit(Int128(L_val ^ R_val));
    }
    case EBinOpType::_nor:
    case EBinOpType::_b_nor: {
      return to_lit(Int128(~(L_val | R_val) & mask));
    }
    case EBinOpType::_xnor:
    case EBinOpType::_b_xnor: {
      return to_lit(Int128(~(L_val ^ R_val) & mask));
    }
    case EBinOpType::ls0: {
      return to_lit(Int128(L_val.shl(R_val.getLimitedValue())));
    }
    case EBinOpType::ls1: {
      unsigned width = L_val.getBitWidth();
      unsigned shift = R_val.getLimitedValue();
      auto     r     = L_val.shl(shift);
      auto     fill  = llvm::APInt::getAllOnes(width).lshr(width - shift);
      r |= fill;
      return to_lit(Int128(r));
    }
    case EBinOpType::lsa: {
    }
    case EBinOpType::rs0: {
      return to_lit(Int128(L_val.lshr(R_val.getLimitedValue())));
    }
    case EBinOpType::rs1: {
      unsigned width = L_val.getBitWidth();
      unsigned shift = R_val.getLimitedValue();

      llvm::APInt r    = L_val.lshr(shift);                                 // shift classique
      llvm::APInt fill = llvm::APInt::getAllOnes(width).shl(width - shift); // bits de gauche mis à 1
      r |= fill;
      return to_lit(Int128(r));
    }
    case EBinOpType::rsa: {
      return to_lit(Int128(L_val.ashr(R_val.getLimitedValue())));
    }
    case EBinOpType::lr: {
      return to_lit(Int128(L_val.rotl(R_val.getLimitedValue())));
    }
    case EBinOpType::rr: {
      return to_lit(Int128(L_val.rotr(R_val.getLimitedValue())));
    }
    default: {
      Error_Diagnostic err(v.scr_info, 172, L._scr_info, L._token, compiler::EPhase::llvmir,
                           "Unexpected operation on byte.", "");
      return std::unexpected(err.print_error());
    }
    }
  } else {
    Error_Diagnostic err(v.scr_info, 173, L._scr_info, L._token, compiler::EPhase::llvmir, "Unexpected type.", "");
    return std::unexpected(err.print_error());
  }
}
std::expected<ast::ALiteral*, std::string>
Static_Evaluator::floating(const ast::literal::Floating_Point& L, const ast::literal::Floating_Point& R, EBinOpType op)
{
  auto to_lit  = [](const Float128& value) { return new ast::literal::Floating_Point(value); };
  auto to_bool = [](bool value) { return new ast::literal::Boolean(value); };

  auto& L_val = *L.val.val;
  auto& R_val = *R.val.val;

  bool is_signed = EPrimType_is_signed(L.type);

  switch (op) {
  case EBinOpType::Add: {
    return to_lit(Float128(L_val + R_val));
  }
  case EBinOpType::Sub: {
    return to_lit(Float128(L_val - R_val));
  }
  case EBinOpType::Mul: {
    return to_lit(Float128(L_val * R_val));
  }
  case EBinOpType::Div: {
    return to_lit(Float128(L_val / R_val));
  }
  case EBinOpType::Mod: {
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

    return to_lit(Float128(r));
  }
  case EBinOpType::Quo: {
    Error_Diagnostic err(v.scr_info, 174, L._scr_info, L._token, compiler::EPhase::llvmir,
                         "Unexpected opration for floating type, use floor(fsize)", "");
  }
  case EBinOpType::Rem: {
    llvm::APFloat q = L_val;
    q.divide(R_val, llvm::APFloat::rmTowardZero);   // q = a/b
    q.roundToIntegral(llvm::APFloat::rmTowardZero); // trunc

    llvm::APFloat qb = q;
    qb.multiply(R_val, llvm::APFloat::rmNearestTiesToEven);

    llvm::APFloat r = L_val;
    r.subtract(qb, llvm::APFloat::rmNearestTiesToEven);

    return to_lit(Float128(r));
  }
  case EBinOpType::Divrem: {

    Error_Diagnostic err(v.scr_info, 175, L._scr_info, L._token, compiler::EPhase::llvmir,
                         "Unexpected operation for compilation time evaluation.", "");
    return std::unexpected(err.print_error());
  }
  case EBinOpType::Pow: {
    double base = L_val.convertToDouble();
    double exp  = R_val.convertToDouble();

    if (base == 0.0 && exp == 0.0) return std::unexpected("0 ** 0 is undefined");

    double result = std::pow(base, exp);

    if (std::isnan(result)) return std::unexpected("invalid floating power");

    return to_lit(Float128(result));
  }
  case EBinOpType::Gre: {
    return to_bool(L_val > R_val);
  }
  case EBinOpType::Low: {
    return to_bool(L_val < R_val);
  }
  case EBinOpType::Gre_eq: {
    return to_bool(L_val >= R_val);
  }
  case EBinOpType::Low_eq: {
    return to_bool(L_val <= R_val);
  }
  case EBinOpType::_eq:
  case EBinOpType::_in: {
    return to_bool(float_almost_eq_ULP(L_val, R_val));
  }
  case EBinOpType::_neq:
  case EBinOpType::_nin: {
    return to_bool(!float_almost_eq_ULP(L_val, R_val));
    bool test = L_val == R_val;
  }
  case EBinOpType::_eqs:
  case EBinOpType::_is:  {
    return to_bool(L_val == R_val);
  }
  case EBinOpType::_neqs:
  case EBinOpType::_nis:  {
    return to_bool(L_val != R_val);
  }
  default: {
    Error_Diagnostic err(v.scr_info, 176, L._scr_info, L._token, compiler::EPhase::llvmir,
                         "Unexpected operation on byte.", "");
    return std::unexpected(err.print_error());
  }
  }
}


bool Static_Evaluator::float_almost_eq_ULP(const llvm::APFloat& L, const llvm::APFloat& R, unsigned maxULP)
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

std::expected<ast::ALiteral*, std::string> Static_Evaluator::decimal(const ast::literal::Fixed_Point& L,
                                                                     const ast::literal::Fixed_Point& R, EBinOpType op)
{
  auto to_lit = [](const llvm::APInt& value, size_t scale, EPrimType raw_type) -> ast::ALiteral* {
    return new ast::literal::Fixed_Point(Int128(value), scale, raw_type);
  };
  auto to_bool = [](bool value) { return new ast::literal::Boolean(value); };

  llvm::APInt L_val = *L.val.val; // copy
  llvm::APInt R_val = *R.val.val; // copy

  size_t max_scale = std::max(L.scale, R.scale);

  auto align_scales = [&](llvm::APInt& val, size_t val_decimal) {
    for (size_t i = val_decimal; i < max_scale; ++i) val = val * 10;
  };

  switch (op) {
  case EBinOpType::Add: {
    align_scales(L_val, L.scale);
    align_scales(R_val, R.scale);
    llvm::APInt result = L_val + R_val;
    return to_lit(result, max_scale, L.raw_type);
  }
  case EBinOpType::Sub: {
    align_scales(L_val, L.scale);
    align_scales(R_val, R.scale);
    llvm::APInt result = L_val - R_val;
    return to_lit(result, max_scale, L.raw_type);
  }
  case EBinOpType::Mul: {
    llvm::APInt result       = L_val * R_val;
    size_t      result_scale = L.scale + R.scale;
    return to_lit(result, result_scale, L.raw_type);
  }
  case EBinOpType::Div: {
    // scale dividende to keep precision
    llvm::APInt scale(L_val.getBitWidth(), 1);
    for (size_t i = 0; i < max_scale; ++i) scale = scale * 10;
    llvm::APInt dividend = L_val * scale;
    llvm::APInt result;
    if (EPrimType_is_signed(L.raw_type))
      result = dividend.udiv(R_val);
    else
      result = dividend.sdiv(R_val);

    return to_lit(result, max_scale, L.raw_type);
  }
  case EBinOpType::Quo: {
    // integral quotient
    llvm::APInt result;
    if (EPrimType_is_signed(L.raw_type))
      result = L_val.udiv(R_val);
    else
      result = L_val.sdiv(R_val);

    return to_lit(result, 0, L.raw_type);
  }
  case EBinOpType::Rem: {
    llvm::APInt result;
    if (EPrimType_is_signed(L.raw_type))
      result = L_val.urem(R_val);
    else
      result = L_val.srem(R_val);

    return to_lit(result, 0, L.raw_type);
  }
  case EBinOpType::Divrem: {

    Error_Diagnostic err(v.scr_info, 177, L._scr_info, L._token, compiler::EPhase::llvmir,
                         "Unexpected operation for compilation time evaluation.", "");
    return std::unexpected(err.print_error());
  }
  case EBinOpType::_eq: {
    align_scales(L_val, L.scale);
    align_scales(R_val, R.scale);
    return to_bool(L_val == R_val);
  }
  case EBinOpType::_neq: {
    align_scales(L_val, L.scale);
    align_scales(R_val, R.scale);
    return to_bool(L_val != R_val);
  }
  default: return std::unexpected("Unexpected operation on Decimal");
  }
}

std::expected<ast::ALiteral*, std::string> Static_Evaluator::boolean(const ast::literal::Boolean& L,
                                                                     const ast::literal::Boolean& R, EBinOpType op)
{
  auto to_bool = [](bool value) { return new ast::literal::Boolean(value); };

  bool L_val = L.val;
  bool R_val = R.val;

  switch (op) {
    // EQUAL
  case EBinOpType::_eq:
  case EBinOpType::_is:     return to_bool(L_val == R_val);
  case EBinOpType::_neq:
  case EBinOpType::_nis:    return to_bool(L_val != R_val);
  // AND
  case EBinOpType::_b_and:
  case EBinOpType::_and:    return to_bool(L_val && R_val);
  case EBinOpType::_b_nand:
  case EBinOpType::_nand:   return to_bool(!(L_val && R_val));
  // OR
  case EBinOpType::_b_or:
  case EBinOpType::_or:     return to_bool(L_val || R_val);
  case EBinOpType::_b_nor:
  case EBinOpType::_nor:    return to_bool(!(L_val || R_val));
  default:                  {
    Error_Diagnostic err(v.scr_info, 178, L._scr_info, L._token, compiler::EPhase::llvmir,
                         "Unexpected operation on boolean.", "");
    return std::unexpected(err.print_error());
  }
  }
}

std::expected<ast::ALiteral*, std::string> Static_Evaluator::boolean_not(const ast::ALiteral& term)
{
  auto to_bool = [](bool value) { return new ast::literal::Boolean(value); };

  if (auto ptr = dynamic_cast<const ast::literal::Boolean*>(&term)) {
    return to_bool(!ptr->val);
  } else {
    Error_Diagnostic err(v.scr_info, 179, term._scr_info, term._token, compiler::EPhase::llvmir,
                         "Unexpected operation 'not' on term.", "");
    return std::unexpected(err.print_error());
  }
}
std::expected<ast::ALiteral*, std::string> Static_Evaluator::scalar_minus(const ast::ALiteral& term)
{
  if (auto ptr = dynamic_cast<const ast::literal::Integral*>(&term)) {
    llvm::APInt& val = *ptr->val.val;
    if (!val.isNegative()) val.negate();
    return new ast::literal::Integral(Int128(val));
  } else if (auto ptr = dynamic_cast<const ast::literal::Floating_Point*>(&term)) {
    llvm::APFloat& val = *ptr->val.val;
    if (!val.isNegative()) val.changeSign();
    return new ast::literal::Floating_Point(Float128(val));
  } else if (auto ptr = dynamic_cast<const ast::literal::Fixed_Point*>(&term)) {
    llvm::APInt& val = *ptr->val.val;
    if (!val.isNegative()) val.negate();
    return new ast::literal::Integral(Int128(val));
  } else {
    Error_Diagnostic err(v.scr_info, 180, term._scr_info, term._token, compiler::EPhase::llvmir,
                         "Unexpected operation 'minus' on term.", "");
    return std::unexpected(err.print_error());
  }
}
std::expected<ast::ALiteral*, std::string> Static_Evaluator::scalar_plus(const ast::ALiteral& term)
{
  if (auto ptr = dynamic_cast<const ast::literal::Integral*>(&term)) {
    llvm::APInt& val = *ptr->val.val;
    if (val.isNegative()) val.negate();
    return new ast::literal::Integral(Int128(val));
  } else if (auto ptr = dynamic_cast<const ast::literal::Floating_Point*>(&term)) {
    llvm::APFloat& val = *ptr->val.val;
    if (val.isNegative()) val.changeSign();
    return new ast::literal::Floating_Point(Float128(val));
  } else if (auto ptr = dynamic_cast<const ast::literal::Fixed_Point*>(&term)) {
    llvm::APInt& val = *ptr->val.val;
    if (val.isNegative()) val.negate();
    return new ast::literal::Integral(Int128(val));
  } else {
    Error_Diagnostic err(v.scr_info, 181, term._scr_info, term._token, compiler::EPhase::llvmir,
                         "Unexpected operation 'minus' on term.", "");
    return std::unexpected(err.print_error());
  }
}
