#include "codegen_tools.hpp"

#include <cstdint>
#include <exception>
#include <expected>

#include <llvm/ADT/APFloat.h>
#include <llvm/IR/Value.h>
#include <llvm/ADT/APInt.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Instructions.h>

#include <llvm/IR/Constants.h>
#include <llvm/IR/Type.h>
#include <memory>
#include <string>
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
#include "visitor_codegen.hpp"

llvm::Value* LLVM_Tools::engage_move_semantic(ast::AExpression& p_target)
{
  auto src = p_target.codegen(v);
  auto ty  = p_target.expression_inferred_type->codegen_ty(v);

  auto dest = v.builder.CreateAlloca(ty, nullptr, "tmp_moved");

  if (auto ptr = std::dynamic_pointer_cast<ast::declaration::cop::Entity>(p_target.expression_inferred_type)) {
  }
}
llvm::Value* LLVM_Tools::engage_copy_semantic(ast::AExpression& target)
{
}
llvm::Value* LLVM_Tools::engage_clone_semantic(ast::AExpression& target)
{
}

std::expected<std::shared_ptr<ast::ADeclaration>, std::string> LLVM_Tools::find_symbol(const ast::AExpression& p_expr)
{
  if (auto ptr = dynamic_cast<const ast::AIdentifier*>(&p_expr)) {
    return ptr->identifier_symbol;
  } else if (auto ptr = dynamic_cast<const ast::expression::Member_Access*>(&p_expr)) {
    return find_symbol(*ptr->right);
  } else if (auto ptr = dynamic_cast<const ast::expression::Call*>(&p_expr)) {
    return find_symbol(*ptr->callee);
  } else if (auto ptr = dynamic_cast<const ast::expression::Call_Pipe*>(&p_expr)) {
    return find_symbol(*ptr->callee);
  }
}

std::expected<ast::AExpression*, std::string> LLVM_Tools::get_symbol_expression(ast::ADeclaration& p_symbol)
{
  if (auto ptr = dynamic_cast<ast::declaration::Global*>(&p_symbol)) {
    if (ptr->expression) return ptr->expression.get();
  } else if (auto ptr = dynamic_cast<ast::declaration::local::Variable*>(&p_symbol)) {
    if (ptr->expression) return ptr->expression.get();
  } else if (auto ptr = dynamic_cast<ast::declaration::local::Variable_Binding*>(&p_symbol)) {
    return ptr->parent_pattern->right.get();
  }
  Error_Diagnostic error(v.scr_info, 166, p_symbol.node_scr_info.get(), p_symbol.node_token, compiler::EPhase::llvmir,
                         "The symbol don't have an expression.", "");
  return std::unexpected(error.print_error());
}


std::expected<llvm::Constant*, std::string> LLVM_Tools::create_constant(const ast::ALiteral& p_value)
{
  common::CompCtx ctx;
  if (auto ptr = dynamic_cast<const ast::literal::Integral*>(&p_value)) {
    return get_int_constant(EPrimType_to_bits(ptr->type), 0, ptr->val.i128_to_string(), EPrimType_is_signed(ptr->type));
  } else if (auto ptr = dynamic_cast<const ast::literal::Floating_Point*>(&p_value)) {
    return get_float_constant(EPrimType_to_bits(ptr->type), 0, ptr->val.float128_to_string());
  } else if (auto ptr = dynamic_cast<const ast::literal::Fixed_Point*>(&p_value)) {
    return get_int_constant(EPrimType_to_bits(ptr->raw_type), 0, ptr->val.i128_to_string(),
                            EPrimType_is_signed(ptr->raw_type));
  } else if (auto ptr = dynamic_cast<const ast::literal::Boolean*>(&p_value)) {
    return get_int_constant(1, ptr->val);
  } else if (auto ptr = dynamic_cast<const ast::literal::Textual_Format*>(&p_value)) {
    if (auto txt = ptr->get_if_pure_text()) {
      switch (txt->text_type) {
      case EPrimType::c_str: return get_cstr_constant(txt->val);
      case EPrimType::str:   return get_str_constant(txt->val);
      case EPrimType::text:  {
        std::u32string utf32;
        try {
          utf32 = utf8_to_utf32(txt->val);
        } catch (const std::exception& e) {
          v.error_add(224, *txt, e.what(), "");
        }
        return get_text_constant(utf32);
      }
      default: return std::unexpected("The text type is invalid (" + EPrimType_to_str(txt->text_type) + ")");
      }
    }
  } else if (auto ptr = dynamic_cast<const ast::literal::Enum*>(&p_value)) {
    if (auto ptr2 = std::dynamic_pointer_cast<ast::declaration::Enum>(ptr->expression_inferred_type)) {
      // return llvm::ConstantStruct::get(v.visit(*ptr2), v.visit(ptr));
    }
    // llvm::StructType* ty = llvm::StructType::create(v.ctx, )
  }

  return std::unexpected("Impossible to create a constant from a complex type");
}

llvm::Type* LLVM_Tools::generate_parameter_type(ast::declaration::local::Parameter& p_param)
{
  p_param.codegen_pass(v);
  auto ty = p_param.type->codegen_ty(v);

  switch (p_param.passmode) {
  case EPassMode::NONE:
  case EPassMode::Move:
  case EPassMode::Ref:  {
    if (ty->isSingleValueType())
      return ty;

    else
      return p_param.type->llvm_type = ty->getPointerTo();
  }
  case EPassMode::Mut: {
    return ty->getPointerTo();
  }
  case EPassMode::Copy:
  case EPassMode::Clone: {
    if (!ty->isSingleValueType())
      return p_param.type->llvm_type = ty->getPointerTo();
    else
      return ty;
  }
  case EPassMode::Addr: {
    return p_param.type->llvm_type = ty->getPointerTo()->getPointerTo();
  }
  }
}


llvm::Type* LLVM_Tools::get_primtive_type(EPrimType ty)
{
  switch (ty) {
  case EPrimType::boolean: return v.i1Ty;
  case EPrimType::cune:    return v.i8Ty;
  case EPrimType::rune:    return v.i32Ty;
  case EPrimType::ptrdiff:
  case EPrimType::dSize:
  case EPrimType::udSize:
  case EPrimType::iSize:
  case EPrimType::uSize:
  case EPrimType::bSize:   return v.iSizeTy;
  case EPrimType::i8:
  case EPrimType::u8:
  case EPrimType::b8:      return v.i8Ty;
  case EPrimType::i16:
  case EPrimType::u16:
  case EPrimType::b16:     return v.i16Ty;
  case EPrimType::d32:
  case EPrimType::ud32:
  case EPrimType::i32:
  case EPrimType::u32:
  case EPrimType::b32:     return v.i32Ty;
  case EPrimType::d64:
  case EPrimType::ud64:
  case EPrimType::i64:
  case EPrimType::u64:
  case EPrimType::b64:     return v.i64Ty;
  case EPrimType::d128:
  case EPrimType::ud128:
  case EPrimType::i128:
  case EPrimType::u128:
  case EPrimType::b128:    return v.i128Ty;
  case EPrimType::fSize:   return v.fSizeTy;
  case EPrimType::f16:     return v.f16Ty;
  case EPrimType::f32:     return v.f32Ty;
  case EPrimType::f64:     return v.f64Ty;
  case EPrimType::f80:     return v.f80Ty;
  case EPrimType::f128:    return v.f128Ty;
  case EPrimType::u0:      return v.u0Ty;
  case EPrimType::Flag:    return v.iSizeTy;
  case EPrimType::text:    return v.textTy;
  case EPrimType::str:     return v.strTy;
  case EPrimType::c_str:   return v.cstrTy;
  default:                 {
    return nullptr;
  }
  }
}

llvm::Constant* LLVM_Tools::get_cstr_constant(const std::string& val)
{
  auto txt = llvm::ConstantDataArray::getString(v.ctx, val, true);

  auto glo_txt =
      new llvm::GlobalVariable(*v.mod, txt->getType(), true, llvm::GlobalVariable::PrivateLinkage, txt, ".cstr");
  glo_txt->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);

  return llvm::ConstantExpr::getInBoundsGetElementPtr(txt->getType(), glo_txt, v.get_zero);
}
llvm::Constant* LLVM_Tools::get_str_constant(const std::string& val)
{
  auto txt = llvm::ConstantDataArray::getString(v.ctx, val, true);

  auto glo_txt =
      new llvm::GlobalVariable(*v.mod, txt->getType(), true, llvm::GlobalVariable::PrivateLinkage, txt, ".str");
  glo_txt->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);

  auto txt_ptr = llvm::ConstantExpr::getInBoundsGetElementPtr(txt->getType(), glo_txt, v.get_zero);

  // build the text fat pointer
  auto lenght_const = llvm::ConstantInt::get(v.i32Ty, val.size());
  auto fat_ptr      = llvm::ConstantStruct::get(v.strTy, {txt_ptr, lenght_const});

  return fat_ptr;
}
llvm::Constant* LLVM_Tools::get_text_constant(const std::u32string& val)
{
  std::vector<uint32_t> codepoints;
  codepoints.reserve(val.size());
  for (char32_t c : val) codepoints.push_back(static_cast<uint32_t>(c));

  auto txt = llvm::ConstantDataArray::get(v.ctx, codepoints);

  auto glo_txt =
      new llvm::GlobalVariable(*v.mod, txt->getType(), true, llvm::GlobalVariable::PrivateLinkage, txt, ".text");
  glo_txt->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);

  auto txt_ptr = llvm::ConstantExpr::getInBoundsGetElementPtr(txt->getType(), glo_txt, v.get_zero);

  // build the text fat pointer
  auto lenght_const = llvm::ConstantInt::get(v.i32Ty, val.size());
  auto fat_ptr      = llvm::ConstantStruct::get(v.textTy, {txt_ptr, lenght_const});

  return fat_ptr;
}

llvm::Constant* LLVM_Tools::get_int_constant(size_t bits_size, int64_t int_val, const std::string& str_val,
                                             bool is_signed, size_t radix)
{
  llvm::APInt ap =
      str_val.empty() ? llvm::APInt(bits_size, int_val, is_signed) : llvm::APInt(bits_size, str_val, radix);

  llvm::Type* int_ty = llvm::Type::getIntNTy(v.ctx, bits_size);
  return llvm::ConstantInt::get(int_ty, ap);
}

llvm::Constant* LLVM_Tools::get_float_constant(size_t bits_size, double double_val, const std::string& str_val)
{
  // Sélection des semantics selon bits_size
  const llvm::fltSemantics* sem = nullptr;
  switch (bits_size) {
  case 16:  sem = &llvm::APFloat::IEEEhalf(); break;   // FP16
  case 32:  sem = &llvm::APFloat::IEEEsingle(); break; // float
  case 64:  sem = &llvm::APFloat::IEEEdouble(); break; // double
  case 80:  sem = &llvm::APFloat::x87DoubleExtended(); break;
  case 128: sem = &llvm::APFloat::IEEEquad(); break;
  default:  llvm_unreachable("Unsupported floating-point width");
  }

  llvm::APFloat ap = str_val.empty() ? llvm::APFloat(*sem, std::to_string(double_val)) : llvm::APFloat(*sem, str_val);

  return llvm::ConstantFP::get(v.ctx, ap);
}


llvm::Constant* LLVM_Tools::get_primtive_zeroinitializer(EPrimType ty)
{
  switch (ty) {
  case EPrimType::NONE:
  case EPrimType::boolean: get_int_constant(1, 0);
  case EPrimType::cune:    get_int_constant(8, 0);
  case EPrimType::rune:    get_int_constant(32, 0);
  case EPrimType::c_str:   get_cstr_constant("");
  case EPrimType::str:     get_str_constant("");
  case EPrimType::text:    get_text_constant(U"");
  case EPrimType::ptrdiff:
  case EPrimType::dSize:
  case EPrimType::udSize:
  case EPrimType::iSize:
  case EPrimType::uSize:
  case EPrimType::bSize:   return get_int_constant(compiler::COMP_CTX.get_arch_size(), 0);
  case EPrimType::i8:
  case EPrimType::u8:
  case EPrimType::b8:      return get_int_constant(8, 0);
  case EPrimType::i16:
  case EPrimType::u16:
  case EPrimType::b16:     return get_int_constant(16, 0);
  case EPrimType::d32:
  case EPrimType::ud32:
  case EPrimType::i32:
  case EPrimType::u32:
  case EPrimType::b32:     return get_int_constant(32, 0);
  case EPrimType::d64:
  case EPrimType::ud64:
  case EPrimType::i64:
  case EPrimType::u64:
  case EPrimType::b64:     return get_int_constant(64, 0);
  case EPrimType::d128:
  case EPrimType::ud128:
  case EPrimType::i128:
  case EPrimType::u128:
  case EPrimType::b128:    return get_int_constant(128, 0);
  case EPrimType::fSize:   get_float_constant(compiler::COMP_CTX.get_arch_size(), 0);
  case EPrimType::f16:     get_float_constant(16, 0);
  case EPrimType::f32:     get_float_constant(32, 0);
  case EPrimType::f64:     get_float_constant(64, 0);
  case EPrimType::f80:     get_float_constant(80, 0);
  case EPrimType::f128:    get_float_constant(128, 0);

  default:                 return nullptr;
  }
}


llvm::Constant* LLVM_Tools::get_zeroinitializer(const ast::AType& ty)
{
  if (auto ptr = dynamic_cast<const ast::type::Primitive*>(&ty)) {
    return get_primtive_zeroinitializer(ptr->type);
  }
}


std::u32string LLVM_Tools::utf8_to_utf32(const std::string& s)
{
  std::u32string result;
  size_t         i = 0;

  while (i < s.size()) {
    uint32_t c = static_cast<unsigned char>(s[i]);

    if (c < 0x80) {
      result.push_back(c);
      i++;
    } else if ((c >> 5) == 0x6) {
      if (i + 1 >= s.size()) throw std::runtime_error("UTF8 truncated");
      uint32_t cp = ((c & 0x1F) << 6) | (static_cast<unsigned char>(s[i + 1]) & 0x3F);
      result.push_back(cp);
      i += 2;
    } else if ((c >> 4) == 0xE) {
      if (i + 2 >= s.size()) throw std::runtime_error("UTF8 truncated");
      uint32_t cp = ((c & 0x0F) << 12) | ((static_cast<unsigned char>(s[i + 1]) & 0x3F) << 6)
                    | (static_cast<unsigned char>(s[i + 2]) & 0x3F);
      result.push_back(cp);
      i += 3;
    } else if ((c >> 3) == 0x1E) {
      if (i + 3 >= s.size()) throw std::runtime_error("UTF8 truncated");
      uint32_t cp = ((c & 0x07) << 18) | ((static_cast<unsigned char>(s[i + 1]) & 0x3F) << 12)
                    | ((static_cast<unsigned char>(s[i + 2]) & 0x3F) << 6)
                    | (static_cast<unsigned char>(s[i + 3]) & 0x3F);
      result.push_back(cp);
      i += 4;
    } else {
      throw std::runtime_error("Invalid UTF8");
    }
  }

  return result;
}


llvm::Value* LLVM_Tools::primitive_coerce(llvm::Value* p_val, llvm::Type* p_src, llvm::Type* p_dst)
{
  if (p_src == p_dst) return p_val;

  // int <-> int
  if (p_src->isIntegerTy() && p_dst->isIntegerTy()) {
    auto src_bits = p_src->getIntegerBitWidth();
    auto dst_bits = p_dst->getIntegerBitWidth();

    if (src_bits < dst_bits) return v.builder.CreateSExt(p_val, p_dst);
    if (src_bits > dst_bits) return v.builder.CreateTrunc(p_val, p_dst);
    return p_val;
  }

  // float <-> float
  if (p_src->isFloatingPointTy() && p_dst->isFloatingPointTy()) {
    if (p_src->getPrimitiveSizeInBits() < p_dst->getPrimitiveSizeInBits()) return v.builder.CreateFPExt(p_val, p_dst);

    if (p_src->getPrimitiveSizeInBits() > p_dst->getPrimitiveSizeInBits()) return v.builder.CreateFPTrunc(p_val, p_dst);

    return p_val;
  }

  // int -> float
  if (p_src->isIntegerTy() && p_dst->isFloatingPointTy()) return v.builder.CreateSIToFP(p_val, p_dst);

  // float -> int
  if (p_src->isFloatingPointTy() && p_dst->isIntegerTy()) return v.builder.CreateFPToSI(p_val, p_dst);

  // ptr
  if (p_src->isPointerTy() && p_dst->isPointerTy()) return v.builder.CreateBitCast(p_val, p_dst);

  // int -> ptr
  if (p_src->isIntegerTy() && p_dst->isPointerTy()) return v.builder.CreateIntToPtr(p_val, p_dst);

  // ptr -> int
  if (p_src->isPointerTy() && p_dst->isIntegerTy()) return v.builder.CreatePtrToInt(p_val, p_dst);

  return p_val;
}
