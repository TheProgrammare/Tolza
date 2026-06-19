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

#include <common/compiler_options.hpp>

#include "nexus/ast/ast.hpp"
#include "ast/ast_declaration_sfm.hpp"
#include "ast/ast_expression.hpp"
#include "nexus/ast/ast.hpp"
#include "ast/ast_declaration_global.hpp"
#include "ast/ast_declaration_local.hpp"
#include "ast/ast_literal.hpp"
#include "nexus/type/type.hpp"
#include "ast/ast_operation.hpp"

#include "compiler/compiler.hpp"
#include "misc/error_output.hpp"
#include "nexus/metacode/metacode.hpp"
#include "resolver_codegen.hpp"

/*
llvm::Value* LLVM_Tools::engage_move_semantic(ast::AExpression& p_target)
{
  auto src = p_target.codegen(v);
  auto ty  = p_target.expression_inferred_type->codegen_ty(v);

  auto dest = v.builder.CreateAlloca(ty, nullptr, "tmp_moved");

  if (auto ptr = std::dynamic_pointer_cast<ast::declaration::sfm::Form>(p_target.expression_inferred_type)) {
  }
}
llvm::Value* LLVM_Tools::engage_copy_semantic(ast::AExpression& target)
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
  } else if (auto ptr = dynamic_cast<Local_Variable*>(&p_symbol)) {
    if (ptr->expression) return ptr->expression.get();
  } else if (auto ptr = dynamic_cast<Local_Variable_Binding*>(&p_symbol)) {
    return ptr->parent_pattern->right.get();
  }
  Error_Diagnostic error(v.CU, 166, p_symbol.node_CU.get(), p_symbol.node_token, compiler::EPhase::llvmir,
                         "The symbol don't have an expression.", "");
  return std::unexpected(error.print_error());
}


std::expected<llvm::Constant*, std::string> LLVM_Tools::create_constant(const ast::ALiteral& p_value)
{
  common::compiler::Options ctx;
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
      case EPrimitiveTypeKind::c_str: return get_cstr_constant(txt->val);
      case EPrimitiveTypeKind::str:   return get_str_constant(txt->val);
      case EPrimitiveTypeKind::text:  {
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

llvm::Type* LLVM_Tools::generate_parameter_type(Local_Parameter& p_param)
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
  case EPassMode::Copy: {
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


llvm::Type* LLVM_Tools::get_primtive_type(EPrimitiveTypeKind ty)
{
  switch (ty) {
  case EPrimitiveTypeKind::boolean: return v.i1Ty;
  case EPrimitiveTypeKind::cune:    return v.i8Ty;
  case EPrimitiveTypeKind::rune:    return v.i32Ty;
  case EPrimitiveTypeKind::ptrdiff:
  case EPrimitiveTypeKind::dSize:
  case EPrimitiveTypeKind::udSize:
  case EPrimitiveTypeKind::iSize:
  case EPrimitiveTypeKind::uSize:
  case EPrimitiveTypeKind::bSize:   return v.iSizeTy;
  case EPrimitiveTypeKind::i8:
  case EPrimitiveTypeKind::u8:
  case EPrimitiveTypeKind::b8:      return v.i8Ty;
  case EPrimitiveTypeKind::i16:
  case EPrimitiveTypeKind::u16:
  case EPrimitiveTypeKind::b16:     return v.i16Ty;
  case EPrimitiveTypeKind::d32:
  case EPrimitiveTypeKind::ud32:
  case EPrimitiveTypeKind::i32:
  case EPrimitiveTypeKind::u32:
  case EPrimitiveTypeKind::b32:     return v.i32Ty;
  case EPrimitiveTypeKind::d64:
  case EPrimitiveTypeKind::ud64:
  case EPrimitiveTypeKind::i64:
  case EPrimitiveTypeKind::u64:
  case EPrimitiveTypeKind::b64:     return v.i64Ty;
  case EPrimitiveTypeKind::d128:
  case EPrimitiveTypeKind::ud128:
  case EPrimitiveTypeKind::i128:
  case EPrimitiveTypeKind::u128:
  case EPrimitiveTypeKind::b128:    return v.i128Ty;
  case EPrimitiveTypeKind::fSize:   return v.fSizeTy;
  case EPrimitiveTypeKind::f16:     return v.f16Ty;
  case EPrimitiveTypeKind::f32:     return v.f32Ty;
  case EPrimitiveTypeKind::f64:     return v.f64Ty;
  case EPrimitiveTypeKind::f80:     return v.f80Ty;
  case EPrimitiveTypeKind::f128:    return v.f128Ty;
  case EPrimitiveTypeKind::u0:      return v.u0Ty;
  case EPrimitiveTypeKind::Flag:    return v.iSizeTy;
  case EPrimitiveTypeKind::text:    return v.textTy;
  case EPrimitiveTypeKind::str:     return v.strTy;
  case EPrimitiveTypeKind::c_str:   return v.cstrTy;
  default:                          {
    return nullptr;
  }
  }
}

llvm::Constant* LLVM_Tools::get_cstr_constant(std::string_view val)
{
  auto txt = llvm::ConstantDataArray::getString(v.ctx, val, true);

  auto glo_txt =
      new llvm::GlobalVariable(*v.mod, txt->getType(), true, llvm::GlobalVariable::PrivateLinkage, txt, ".cstr");
  glo_txt->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);

  return llvm::ConstantExpr::getInBoundsGetElementPtr(txt->getType(), glo_txt, v.get_zero);
}
llvm::Constant* LLVM_Tools::get_str_constant(std::string_view val)
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
  for (char32_t c : val) codepoints.emplace_back(static_cast<uint32_t>(c));

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

llvm::Constant* LLVM_Tools::get_int_constant(size_t bits_size, int64_t int_val, std::string_view str_val,
                                             bool is_signed, size_t radix)
{
  llvm::APInt ap =
      str_val.empty() ? llvm::APInt(bits_size, int_val, is_signed) : llvm::APInt(bits_size, str_val, radix);

  llvm::Type* int_ty = llvm::Type::getIntNTy(v.ctx, bits_size);
  return llvm::ConstantInt::get(int_ty, ap);
}

llvm::Constant* LLVM_Tools::get_float_constant(size_t bits_size, double double_val, std::string_view str_val)
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


llvm::Constant* LLVM_Tools::get_primtive_zeroinitializer(EPrimitiveTypeKind ty)
{
  switch (ty) {
  case EPrimitiveTypeKind::NONE:
  case EPrimitiveTypeKind::boolean: get_int_constant(1, 0);
  case EPrimitiveTypeKind::cune:    get_int_constant(8, 0);
  case EPrimitiveTypeKind::rune:    get_int_constant(32, 0);
  case EPrimitiveTypeKind::c_str:   get_cstr_constant("");
  case EPrimitiveTypeKind::str:     get_str_constant("");
  case EPrimitiveTypeKind::text:    get_text_constant(U"");
  case EPrimitiveTypeKind::ptrdiff:
  case EPrimitiveTypeKind::dSize:
  case EPrimitiveTypeKind::udSize:
  case EPrimitiveTypeKind::iSize:
  case EPrimitiveTypeKind::uSize:
  case EPrimitiveTypeKind::bSize:   return get_int_constant(compiler::OPTIONS.get_arch_size(), 0);
  case EPrimitiveTypeKind::i8:
  case EPrimitiveTypeKind::u8:
  case EPrimitiveTypeKind::b8:      return get_int_constant(8, 0);
  case EPrimitiveTypeKind::i16:
  case EPrimitiveTypeKind::u16:
  case EPrimitiveTypeKind::b16:     return get_int_constant(16, 0);
  case EPrimitiveTypeKind::d32:
  case EPrimitiveTypeKind::ud32:
  case EPrimitiveTypeKind::i32:
  case EPrimitiveTypeKind::u32:
  case EPrimitiveTypeKind::b32:     return get_int_constant(32, 0);
  case EPrimitiveTypeKind::d64:
  case EPrimitiveTypeKind::ud64:
  case EPrimitiveTypeKind::i64:
  case EPrimitiveTypeKind::u64:
  case EPrimitiveTypeKind::b64:     return get_int_constant(64, 0);
  case EPrimitiveTypeKind::d128:
  case EPrimitiveTypeKind::ud128:
  case EPrimitiveTypeKind::i128:
  case EPrimitiveTypeKind::u128:
  case EPrimitiveTypeKind::b128:    return get_int_constant(128, 0);
  case EPrimitiveTypeKind::fSize:   get_float_constant(compiler::OPTIONS.get_arch_size(), 0);
  case EPrimitiveTypeKind::f16:     get_float_constant(16, 0);
  case EPrimitiveTypeKind::f32:     get_float_constant(32, 0);
  case EPrimitiveTypeKind::f64:     get_float_constant(64, 0);
  case EPrimitiveTypeKind::f80:     get_float_constant(80, 0);
  case EPrimitiveTypeKind::f128:    get_float_constant(128, 0);

  default:                          return nullptr;
  }
}


llvm::Constant* LLVM_Tools::get_zeroinitializer(const ast::AType& ty)
{
  if (auto ptr = dynamic_cast<const ast::type::Primitive*>(&ty)) {
    return get_primtive_zeroinitializer(ptr->type);
  }
}


std::u32string LLVM_Tools::utf8_to_utf32(std::string_view s)
{
  std::u32string result;
  size_t         i = 0;

  while (i < s.size()) {
    uint32_t c = static_cast<unsigned char>(s[i]);

    if (c < 0x80) {
      result.emplace_back(c);
      i++;
    } else if ((c >> 5) == 0x6) {
      if (i + 1 >= s.size()) throw std::runtime_error("UTF8 truncated");
      uint32_t cp = ((c & 0x1F) << 6) | (static_cast<unsigned char>(s[i + 1]) & 0x3F);
      result.emplace_back(cp);
      i += 2;
    } else if ((c >> 4) == 0xE) {
      if (i + 2 >= s.size()) throw std::runtime_error("UTF8 truncated");
      uint32_t cp = ((c & 0x0F) << 12) | ((static_cast<unsigned char>(s[i + 1]) & 0x3F) << 6)
                    | (static_cast<unsigned char>(s[i + 2]) & 0x3F);
      result.emplace_back(cp);
      i += 3;
    } else if ((c >> 3) == 0x1E) {
      if (i + 3 >= s.size()) throw std::runtime_error("UTF8 truncated");
      uint32_t cp = ((c & 0x07) << 18) | ((static_cast<unsigned char>(s[i + 1]) & 0x3F) << 12)
                    | ((static_cast<unsigned char>(s[i + 2]) & 0x3F) << 6)
                    | (static_cast<unsigned char>(s[i + 3]) & 0x3F);
      result.emplace_back(cp);
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
*/