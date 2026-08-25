#include "codegen_tools.hpp"

#include <cstdint>
#include <exception>
#include <expected>

#include <functional>
#include <llvm-19/llvm/Analysis/LoopInfo.h>
#include <llvm-19/llvm/IR/GlobalValue.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/ADT/APFloat.h>
#include <llvm/IR/Value.h>
#include <llvm/ADT/APInt.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>

#include <string>
#include <string_view>
#include <vector>

#include <common/compiler_options.hpp>

#include "codegen/codegen.hpp"
#include "codegen/codegen_insurance.hpp"
#include "codegen/codegen_type.hpp"
#include "nexus/ast/data.hpp"
#include "nexus/ast/forward.hpp"
#include "nexus/type/data.hpp"
#include "nexus/type/definition.hpp"

#include "ast/ast_expression.hpp"

#include "compiler/compiler.hpp"

#include "nexus/forward.hpp"
#include "nexus/ids.hpp"

#include <Neargye/magic_enum.hpp>

#define NOT_DEFINED assert(false);


codegen::Tools::Tools(codegen::Codegen_AST& p_res)
  : res(p_res)
  , ctx(res.ctx)
  , builder(res.builder)
  , mod(res.mod)
{
}


llvm::Value* codegen::Tools::engage_move_semantic(ast::ID p_target) noexcept
{
  auto* src = res.codegen_node(p_target);
  auto* ty  = res.get_type(p_target.type());

  auto* dest = builder.CreateAlloca(ty, nullptr, "tmp_moved");
  return dest;
}
llvm::Value* codegen::Tools::engage_copy_semantic(ast::ID target) noexcept {NOT_DEFINED}


std::expected<llvm::Constant*, std::string> codegen::Tools::create_constant(ast::ID lit_id) noexcept
{
  if (const auto* ptr = lit_id.as<ast::Literal_Integral>())
    return get_int_constant(type::EPrimitiveTypeKind_to_bits(ptr->type), 0, ptr->val.i128_to_string(),
                            type::EPrimitiveTypeKind_is_signed(ptr->type));

  if (const auto* ptr = lit_id.as<ast::Literal_Floating_Point>())
    return get_float_constant(type::EPrimitiveTypeKind_to_bits(ptr->type), 0, ptr->val.float128_to_string());

  if (const auto* ptr = lit_id.as<ast::Literal_Fixed_Point>())
    return get_int_constant(type::EPrimitiveTypeKind_to_bits(ptr->raw_type), 0, ptr->val.i128_to_string(),
                            type::EPrimitiveTypeKind_is_signed(ptr->raw_type));

  if (const auto* ptr = lit_id.as<ast::Literal_Boolean>()) return get_int_constant(1, ptr->val);

  if (lit_id.is<ast::Literal_NullPtr>()) return llvm::ConstantPointerNull::getNullValue(res.get_type(lit_id.type()));

  if (const auto* ptr = lit_id.as<ast::Literal_Text_Pure>()) {
    switch (lit_id.type().as<type::String>()->kind) {
    case type::ETextType::_cstr: return get_cstr_constant(ptr->val);
    case type::ETextType::_cune:
    case type::ETextType::_str:  return get_str_constant(ptr->val);
    case type::ETextType::_rune:
    case type::ETextType::_text: {
      std::u32string utf32;
      try {
        utf32 = utf8_to_utf32(ptr->val);
      } catch (const std::exception& e) {
        res.add_error(224, ptr->header, e.what(), "");
      }
      return get_text_constant(utf32);
    }
    default:
      return std::unexpected(std::format("The text type is invalid ({})", magic_enum::enum_name(ptr->text_type)));
    }
  }

  return std::unexpected("Impossible to create a constant from a complex type");
}

llvm::Constant* codegen::Tools::get_cstr_constant(std::string_view val) noexcept
{
  auto* txt = llvm::ConstantDataArray::getString(ctx, val, true);

  auto* g = new llvm::GlobalVariable(*mod, txt->getType(), true, llvm::GlobalVariable::PrivateLinkage, txt, ".cstr");
  g->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);

  return llvm::ConstantExpr::getInBoundsGetElementPtr(txt->getType(), g, res.const_int(0));
}
llvm::Constant* codegen::Tools::get_str_constant(std::string_view val) noexcept
{
  auto* txt = llvm::ConstantDataArray::getString(ctx, val, true);

  auto* g = new llvm::GlobalVariable(*mod, txt->getType(), true, llvm::GlobalVariable::PrivateLinkage, txt, ".str");
  g->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);

  auto* txt_ptr = llvm::ConstantExpr::getInBoundsGetElementPtr(txt->getType(), g, res.const_int(0));

  // build the text fat pointer
  auto* lenght_const = llvm::ConstantInt::get(codegen::LLVM_TYPEID_usize, val.size());
  auto* fat_ptr      = llvm::ConstantStruct::get(codegen::LLVM_TYPEID_str, {txt_ptr, lenght_const});

  return fat_ptr;
}
llvm::Constant* codegen::Tools::get_text_constant(const std::u32string& val) noexcept
{
  std::vector<uint32_t> codepoints;
  codepoints.reserve(val.size());
  for (char32_t c : val) codepoints.emplace_back(static_cast<uint32_t>(c));

  auto* txt = llvm::ConstantDataArray::get(ctx, codepoints);

  auto* g = new llvm::GlobalVariable(*mod, txt->getType(), true, llvm::GlobalVariable::PrivateLinkage, txt, ".text");
  g->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);

  auto* txt_ptr = llvm::ConstantExpr::getInBoundsGetElementPtr(txt->getType(), g, res.const_int(0));

  // build the text fat pointer
  auto* lenght_const = llvm::ConstantInt::get(codegen::LLVM_TYPEID_usize, val.size());
  auto* fat_ptr      = llvm::ConstantStruct::get(codegen::LLVM_TYPEID_text, {txt_ptr, lenght_const});

  return fat_ptr;
}

llvm::Constant* codegen::Tools::get_int_constant(size_t bits_size, int64_t int_val, std::string_view str_val,
                                                 bool is_signed, size_t radix) noexcept
{
  llvm::APInt ap =
      str_val.empty() ? llvm::APInt(bits_size, int_val, is_signed) : llvm::APInt(bits_size, str_val, radix);

  llvm::Type* int_ty = llvm::Type::getIntNTy(ctx, bits_size);
  return llvm::ConstantInt::get(int_ty, ap);
}

llvm::Constant* codegen::Tools::get_float_constant(size_t bits_size, double double_val,
                                                   std::string_view str_val) noexcept
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

  return llvm::ConstantFP::get(ctx, ap);
}

llvm::Constant* codegen::Tools::get_text_zeroinit(type::ETextType ty) noexcept
{
  switch (ty) {
  case type::ETextType::_cstr: return get_cstr_constant("");
  case type::ETextType::NONE:
  case type::ETextType::_cune:
  case type::ETextType::_str:  return get_str_constant("");
  case type::ETextType::_rune:
  case type::ETextType::_text: return get_text_constant(U"");
  }
}

llvm::Constant* codegen::Tools::get_primtive_zeroinit(type::EPrimitiveTypeKind ty) noexcept
{
  switch (ty) {
  case type::EPrimitiveTypeKind::_bool:    return get_int_constant(1, 0);
  case type::EPrimitiveTypeKind::_cune:    return get_int_constant(8, 0);
  case type::EPrimitiveTypeKind::_rune:    return get_int_constant(32, 0);
  case type::EPrimitiveTypeKind::_ptrdiff:
  case type::EPrimitiveTypeKind::_dsize:
  case type::EPrimitiveTypeKind::_udsize:
  case type::EPrimitiveTypeKind::_ssize:
  case type::EPrimitiveTypeKind::_usize:
  case type::EPrimitiveTypeKind::_bsize:   return get_int_constant(compiler::OPTIONS.target.get_arch_size(), 0);
  case type::EPrimitiveTypeKind::_s8:
  case type::EPrimitiveTypeKind::_u8:
  case type::EPrimitiveTypeKind::_b8:      return get_int_constant(8, 0);
  case type::EPrimitiveTypeKind::_s16:
  case type::EPrimitiveTypeKind::_u16:
  case type::EPrimitiveTypeKind::_b16:     return get_int_constant(16, 0);
  case type::EPrimitiveTypeKind::_d32:
  case type::EPrimitiveTypeKind::_ud32:
  case type::EPrimitiveTypeKind::_s32:
  case type::EPrimitiveTypeKind::_u32:
  case type::EPrimitiveTypeKind::_b32:     return get_int_constant(32, 0);
  case type::EPrimitiveTypeKind::_d64:
  case type::EPrimitiveTypeKind::_ud64:
  case type::EPrimitiveTypeKind::_s64:
  case type::EPrimitiveTypeKind::_u64:
  case type::EPrimitiveTypeKind::_b64:     return get_int_constant(64, 0);
  case type::EPrimitiveTypeKind::_d128:
  case type::EPrimitiveTypeKind::_ud128:
  case type::EPrimitiveTypeKind::_s128:
  case type::EPrimitiveTypeKind::_u128:
  case type::EPrimitiveTypeKind::_b128:    return get_int_constant(128, 0);
  case type::EPrimitiveTypeKind::_fsize:   return get_float_constant(compiler::OPTIONS.target.get_arch_size(), 0);
  case type::EPrimitiveTypeKind::_f16:     return get_float_constant(16, 0);
  case type::EPrimitiveTypeKind::_f32:     return get_float_constant(32, 0);
  case type::EPrimitiveTypeKind::_f64:     return get_float_constant(64, 0);
  case type::EPrimitiveTypeKind::_f80:     return get_float_constant(80, 0);
  case type::EPrimitiveTypeKind::_f128:    return get_float_constant(128, 0);
  default:                                 return nullptr;
  }
}


llvm::Constant* codegen::Tools::get_zeroinitializer(type::ID ty) noexcept
{
  if (auto* ptr = ty.as<type::Primitive>()) return get_primtive_zeroinit(ptr->primitive);
  if (auto* ptr = ty.as<type::String>()) return get_text_zeroinit(ptr->kind);
  return nullptr;
}


std::u32string codegen::Tools::utf8_to_utf32(std::string_view s) noexcept
{
  std::u32string result;
  size_t         i = 0;

  while (i < s.size()) {
    uint32_t c = static_cast<unsigned char>(s[i]);

    if (c < 0x80) {
      result += c;
      i++;
    } else if ((c >> 5) == 0x6) {
      if (i + 1 >= s.size()) continue;
      uint32_t _c = ((c & 0x1F) << 6) | (static_cast<unsigned char>(s[i + 1]) & 0x3F);
      result += _c;
      i += 2;
    } else if ((c >> 4) == 0xE) {
      if (i + 2 >= s.size()) continue;
      uint32_t _c = ((c & 0x0F) << 12) | ((static_cast<unsigned char>(s[i + 1]) & 0x3F) << 6)
                    | (static_cast<unsigned char>(s[i + 2]) & 0x3F);
      result += _c;
      i += 3;
    } else if ((c >> 3) == 0x1E) {
      if (i + 3 >= s.size()) continue;
      uint32_t _c = ((c & 0x07) << 18) | ((static_cast<unsigned char>(s[i + 1]) & 0x3F) << 12)
                    | ((static_cast<unsigned char>(s[i + 2]) & 0x3F) << 6)
                    | (static_cast<unsigned char>(s[i + 3]) & 0x3F);
      result += _c;
      i += 4;
    } else {
      continue;
    }
  }

  return result;
}


llvm::Value* codegen::Tools::primitive_coerce(llvm::Value* p_val, llvm::Type* p_src, llvm::Type* p_dst) noexcept
{
  if (p_src == p_dst) return p_val;

  // bool to int
  if (p_src->getPrimitiveSizeInBits() == 1 && p_dst->isIntegerTy()) {
    return builder.CreateZExt(p_val, p_dst);
  }

  // int <-> int
  if (p_src->isIntegerTy() && p_dst->isIntegerTy()) {
    auto src_bits = p_src->getIntegerBitWidth();
    auto dst_bits = p_dst->getIntegerBitWidth();

    if (src_bits < dst_bits) return builder.CreateSExt(p_val, p_dst);
    if (src_bits > dst_bits) return builder.CreateTrunc(p_val, p_dst);
    return p_val;
  }

  // float <-> float
  if (p_src->isFloatingPointTy() && p_dst->isFloatingPointTy()) {
    if (p_src->getPrimitiveSizeInBits() < p_dst->getPrimitiveSizeInBits()) return builder.CreateFPExt(p_val, p_dst);

    if (p_src->getPrimitiveSizeInBits() > p_dst->getPrimitiveSizeInBits()) return builder.CreateFPTrunc(p_val, p_dst);

    return p_val;
  }

  // int -> float
  if (p_src->isIntegerTy() && p_dst->isFloatingPointTy()) return builder.CreateSIToFP(p_val, p_dst);

  // float -> int
  if (p_src->isFloatingPointTy() && p_dst->isIntegerTy()) return builder.CreateFPToSI(p_val, p_dst);

  // ptr
  if (p_src->isPointerTy() && p_dst->isPointerTy()) return builder.CreateBitCast(p_val, p_dst);

  // int -> ptr
  if (p_src->isIntegerTy() && p_dst->isPointerTy()) return builder.CreateIntToPtr(p_val, p_dst);

  // ptr -> int
  if (p_src->isPointerTy() && p_dst->isIntegerTy()) return builder.CreatePtrToInt(p_val, p_dst);

  return p_val;
}


llvm::Value* codegen::Tools::codegen_explicit_cast(ast::ID from, type::ID to) const noexcept
{
  auto  from_ty = from.type();
  auto* from_v  = res.insurance.ensure_rvalue(from);

  if (from_ty == to) return from_v;

  if (from_ty.as<type::Primitive>() && to.as<type::Primitive>()) {
    auto* cast = res.tools.primitive_coerce(from_v, res.get_type(from_ty), res.get_type(to));
    return res.add_generation(from, cast);
  }

  if (from_ty.is<type::Array>() && to.is<type::String>()) {
    std::println("test");
    std::cout << std::flush;
    const auto* buffer  = from_ty.as<type::Array>();
    const auto* out_str = to.as<type::String>();


    // is string buffer
    if (const auto* inner = buffer->inner.as<type::Primitive>()) {
      switch (inner->primitive) {
      case type::EPrimitiveTypeKind::_cune: {
        assert(out_str->kind != type::ETextType::_text && "Illegal explicit cast from cune buffer to text");

        if (out_str->kind == type::ETextType::_cstr)
          return builder.CreateGEP(res.get_type(from_ty), from_v, {res.const_int(0), res.const_int(0)});

        if (out_str->kind == type::ETextType::_str) {
          auto* ty       = codegen::LLVM_TYPEID_str;
          auto* str_data = builder.CreateGEP(res.get_type(from_ty), from_v, {res.const_int(0), res.const_int(0)});
          auto* size     = builtin_c_strlen(from_v);

          auto* alloca     = builder.CreateAlloca(ty, 0, "str");
          auto* a_str_data = builder.CreateStructGEP(ty, alloca, 0);
          auto* a_size     = builder.CreateStructGEP(ty, alloca, 1);
          builder.CreateStore(str_data, a_str_data);
          builder.CreateStore(size, a_size);
          return alloca;
        }
      }
      case type::EPrimitiveTypeKind::_rune: {
      }
      default: {
      }
      }
    }
  }

  if (from_ty.is<type::String>() && to.is<type::String>()) {
    if (from_ty == type::TYPEID_str && to == type::TYPEID_cstr) {
      return builder.CreateExtractValue(from_v, 0);
    }
  }

  return nullptr;
}

llvm::Value* codegen::Tools::make_struct(llvm::Type* ty, std::vector<llvm::Value*> fields) const noexcept
{
  auto* structTy = llvm::cast<llvm::StructType>(ty);
  assert(structTy && "Invalid non struct type");

  llvm::Value* agg = llvm::UndefValue::get(structTy);

  // build SSA
  for (unsigned i = 0; i < fields.size(); ++i) {
    agg = builder.CreateInsertValue(agg, fields[i], {i});
  }

  return agg;
}

llvm::Value* codegen::Tools::builtin_c_strlen(llvm::Value* in) const noexcept
{
  llvm::FunctionType* ty = llvm::FunctionType::get(codegen::LLVM_TYPEID_usize,  // size_t
                                                   {codegen::LLVM_TYPEID_cstr}, // const char*
                                                   false);

  llvm::FunctionCallee callee = mod->getOrInsertFunction("strlen", ty);

  return builder.CreateCall(callee, in);
}

// start, length, end included
std::tuple<llvm::Value*, llvm::Value*, bool> codegen::Tools::get_span(ast::ID nodeid) const noexcept
{
  auto* ty = res.get_type(nodeid.type());


  // layout {start, end}
  if (auto* ptr = nodeid.as<ast::Literal_Range>()) {
    auto* start = res.insurance.ensure_rvalue(ptr->start, "it.start");
    auto* end   = res.insurance.ensure_rvalue(ptr->end, "it.end");
    auto* len   = builder.CreateSub(end, start, "it.len");
    return {start, len, ptr->endInclude};
  }
  // lauout {data1, data2, ...}
  if (auto* ptr = nodeid.as<ast::Literal_Table>()) {
    return {
        res.const_int(0),
        // arry size at comptime
        res.const_int(ptr->values.size()),
        false,
    };
  }
  // layout {data, size}
  if (nodeid.type().as<type::Slice>()) {
    auto* v    = res.codegen_node(nodeid);
    auto* size = builder.CreateStructGEP(res.get_type(nodeid.type()), v, 1, "slice.size");
    auto* len  = builder.CreateLoad(codegen::LLVM_TYPEID_usize, size, "it.len");
    return {
        res.const_int(0),
        // slice len at runtime
        len,
        true,
    };
  }
  // layout {data, size, capa}
  if (nodeid.type().as<type::Buffer>()) {
    auto* v    = res.codegen_node(nodeid);
    auto* size = builder.CreateStructGEP(res.get_type(nodeid.type()), v, 1, "slice.size");
    auto* len  = builder.CreateLoad(codegen::LLVM_TYPEID_usize, size, "it.len");
    return {
        res.const_int(0),
        // buffer size at runtime
        len,
        true,
    };
  }
  // layout {data, size}
  if (const auto* ptr = nodeid.type().as<type::Array>()) {
    auto* v = res.codegen_node(nodeid);
    return {
        res.const_int(0),
        // arry size at comptime
        res.const_int(ptr->size),
        true,
    };
  }
  // table access selector
  if (const auto* ptr = nodeid.as<ast::Literal_Integral>()) {
    return {
        res.const_int(ptr->val.val->getLimitedValue()),
        // one selected
        res.const_int(1),
        true,
    };
  }
  if (const auto* ptr = nodeid.as<ast::Expression_Table_Access>()) {
    return get_span(ptr->selector);
  }
  // for identifiers
  if (nodeid.def()) {
    return get_span(nodeid.def().node());
  }

  assert(false && "invalid range");
}

llvm::Value* codegen::Tools::codegen_unary_op(ast::ID term, ast::EOp_Unary unary_op) const noexcept
{
  auto* base_ptr = res.insurance.ensure_rvalue(term, "un.term");

  auto  tyid       = term.type();
  auto* llvm_op_ty = res.get_type(term.type());

  llvm::Value* op = nullptr;

  auto& builder = res.builder;

  switch (unary_op) {
  case ast::EOp_Unary::NONE: break;
  case ast::EOp_Unary::_not: {
    if (auto* ptr = tyid.as<type::Primitive>()) {
      if (ptr->primitive == type::EPrimitiveTypeKind::_bool || type::EPrimitiveTypeKind_is_byte(ptr->primitive)) {
        op = builder.CreateNot(base_ptr, "un.not");
      }
    }

    break;
  }
  case ast::EOp_Unary::_plus: {
    if (auto* ptr = tyid.as<type::Primitive>()) {
      if (type::EPrimitiveTypeKind_is_integral(ptr->primitive)) {
        unsigned bits = llvm_op_ty->getIntegerBitWidth();
        auto     mask = llvm::APInt(bits, 1);
        mask <<= (bits - 1);

        auto inv = ~mask;

        op = builder.CreateAnd(base_ptr, llvm::ConstantInt::get(llvm_op_ty, inv), "un.plus");
      } else if (type::EPrimitiveTypeKind_is_floating(ptr->primitive)) {
        llvm::Type* f_ty_size = codegen::llvm_primitives.at(type::ID::make_primitive(ptr->primitive));

        auto* bits    = builder.CreateBitCast(base_ptr, f_ty_size);
        auto* mask    = llvm::ConstantInt::get(bits->getType(), 1U << (f_ty_size->getPrimitiveSizeInBits() - 1));
        auto* flipped = builder.CreateAnd(bits, mask);
        op            = builder.CreateBitCast(flipped, llvm_op_ty, "un.plus");
      }
    }

    break;
  }
  case ast::EOp_Unary::_minus: {
    if (auto* ptr = tyid.as<type::Primitive>()) {
      if (type::EPrimitiveTypeKind_is_integral(ptr->primitive)) {
        unsigned bits = llvm_op_ty->getIntegerBitWidth();
        auto     mask = llvm::APInt(bits, 1);
        mask <<= (bits - 1);

        auto inv = ~mask;

        op = builder.CreateOr(base_ptr, llvm::ConstantInt::get(llvm_op_ty, inv), "un.minus");
      } else if (type::EPrimitiveTypeKind_is_floating(ptr->primitive)) {
        llvm::Type* f_ty_size = codegen::llvm_primitives.at(type::ID::make_primitive(ptr->primitive));

        auto* bits    = builder.CreateBitCast(base_ptr, f_ty_size);
        auto* mask    = llvm::ConstantInt::get(bits->getType(), 1U << (f_ty_size->getPrimitiveSizeInBits() - 1));
        auto* flipped = builder.CreateOr(bits, mask);
        op            = builder.CreateBitCast(flipped, llvm_op_ty, "un.minus");
      }
    }

    break;
  }
  case ast::EOp_Unary::_invert_sign: {
    if (auto* ptr = tyid.as<type::Primitive>()) {
      if (type::EPrimitiveTypeKind_is_integral(ptr->primitive)) {
        unsigned bits = llvm_op_ty->getIntegerBitWidth();
        auto     mask = llvm::APInt(bits, 1);
        mask <<= (bits - 1);

        auto inv = ~mask;

        op = builder.CreateXor(base_ptr, llvm::ConstantInt::get(llvm_op_ty, inv), "un.inv");
      } else if (type::EPrimitiveTypeKind_is_floating(ptr->primitive)) {
        llvm::Type* f_ty_size = codegen::llvm_primitives.at(type::ID::make_primitive(ptr->primitive));

        auto* bits    = builder.CreateBitCast(base_ptr, f_ty_size);
        auto* mask    = llvm::ConstantInt::get(bits->getType(), 1U << (f_ty_size->getPrimitiveSizeInBits() - 1));
        auto* flipped = builder.CreateXor(bits, mask);
        op            = builder.CreateBitCast(flipped, llvm_op_ty, "un.inv");
      }
    }

    break;
  }
  }

  if (!op)
    res.add_error(
        207, term.get(),
        std::format("Illegal unary operation ({}) on type \"{}\'", ast::EOp_Unary_to_str(unary_op), term.type().dump()),
        "");

  return op;
}


llvm::Value* codegen::Tools::codegen_binary_op(ast::ID left, ast::ID right, ast::EOp_Bin op_ty) const noexcept
{
  auto* l_val = res.insurance.ensure_rvalue(left, "bin.l");
  auto* r_val = res.insurance.ensure_rvalue(right, "bir");

  if (!l_val) res.add_error(226, left.get(), "Can't be evaluated as value.", "");
  if (!r_val) res.add_error(226, right.get(), "Can't be evaluated as value.", "");
  if (!l_val || !r_val) return nullptr;

  auto* llvm_op_ty = res.get_type(left.type());

  using Func = std::function<void()>;

  auto arith = [&](Func fn_sint, Func fn_uint, Func fn_float) {
    if (auto* ptr = left.type().as<type::Primitive>()) {
      if (type::EPrimitiveTypeKind_is_floating(ptr->primitive)) {
        if (fn_float) fn_float();
      } else if (type::EPrimitiveTypeKind_is_integral(ptr->primitive)) {
        if (type::EPrimitiveTypeKind_is_signed(ptr->primitive)) {
          if (fn_sint) fn_sint();
        } else {
          if (fn_uint)
            fn_uint();
          else if (fn_sint)
            fn_sint();
        }
      } else if (type::EPrimitiveTypeKind_is_textual(ptr->primitive)) {
        if (fn_uint)
          fn_uint();
        else if (fn_sint)
          fn_sint();
      }
    }
  };

  auto byte = [&](Func bin) {
    if (auto* ptr = left.type().as<type::Primitive>()) {
      if (ptr->primitive == type::EPrimitiveTypeKind::_bool || type::EPrimitiveTypeKind_is_byte(ptr->primitive)) {
        bin();
      }
    }
  };


  llvm::Value* op = nullptr;

  switch (op_ty) {
  case ast::EOp_Bin::NONE:
  case ast::EOp_Bin::_add: {
    arith([&]() { op = builder.CreateAdd(l_val, r_val, "bin.add"); }, nullptr,
          [&]() { op = builder.CreateFAdd(l_val, r_val, "bin.add"); });

    break;
  }
  case ast::EOp_Bin::_sub: {
    arith([&]() { op = builder.CreateSub(l_val, r_val, "bin.sub"); }, nullptr,
          [&]() { op = builder.CreateFSub(l_val, r_val, "bin.sub"); });

    break;
  }
  case ast::EOp_Bin::_mul: {
    arith([&]() { op = builder.CreateMul(l_val, r_val, "bin.mul"); }, nullptr,
          [&]() { op = builder.CreateFMul(l_val, r_val, "bin.mul"); });

    break;
  }
  case ast::EOp_Bin::_div: {
    arith(
        [&]() {
          auto* fl = builder.CreateSIToFP(l_val, codegen::LLVM_TYPEID_fsize);
          auto* fr = builder.CreateSIToFP(r_val, codegen::LLVM_TYPEID_fsize);
          op       = builder.CreateFDiv(fl, fr, "bin.div");
        },
        [&]() {
          auto* fl = builder.CreateUIToFP(l_val, codegen::LLVM_TYPEID_fsize);
          auto* fr = builder.CreateUIToFP(r_val, codegen::LLVM_TYPEID_fsize);
          op       = builder.CreateFDiv(fl, fr, "bin.div");
        },
        [&]() { op = builder.CreateFDiv(l_val, r_val, "bin.div"); });

    break;
  }
  case ast::EOp_Bin::_mod: {
    arith(
        [&]() {
          auto* rem    = builder.CreateSRem(l_val, r_val, "bin.rem");
          auto* is_neg = builder.CreateICmpSLT(rem, llvm::ConstantInt::get(rem->getType(), 0), "is_neg");
          op = builder.CreateSelect(is_neg, builder.CreateAdd(rem, r_val, "bin.mod_adjusted"), rem, "bin.mod");
        },
        [&]() {
          auto* rem    = builder.CreateURem(l_val, r_val, "bin.rem");
          auto* is_neg = builder.CreateICmpSLT(rem, llvm::ConstantInt::get(rem->getType(), 0), "is_neg");
          op = builder.CreateSelect(is_neg, builder.CreateAdd(rem, r_val, "bin.mod_adjusted"), rem, "bin.mod");
        },
        nullptr);

    break;
  }
  case ast::EOp_Bin::_quo: {
    arith(
        [&]() {
          auto* rem    = builder.CreateSRem(l_val, r_val, "bin.rem");
          auto* is_neg = builder.CreateICmpSLT(rem, llvm::ConstantInt::get(rem->getType(), 0), "is_neg");
          auto* mod = builder.CreateSelect(is_neg, builder.CreateAdd(rem, r_val, "bin.mod_adjusted"), rem, "bin.mod");
          op        = builder.CreateSDiv(builder.CreateSub(l_val, mod), r_val, "bin.quo");
        },
        [&]() {
          auto* rem    = builder.CreateURem(l_val, r_val, "bin.rem");
          auto* is_neg = builder.CreateICmpSLT(rem, llvm::ConstantInt::get(rem->getType(), 0), "is_neg");
          auto* mod = builder.CreateSelect(is_neg, builder.CreateAdd(rem, r_val, "bin.mod_adjusted"), rem, "bin.mod");
          op        = builder.CreateSDiv(builder.CreateSub(l_val, mod), r_val, "bin.quo");
        },
        nullptr);

    break;
  }
  case ast::EOp_Bin::_rem: {
    arith([&]() { op = builder.CreateSRem(l_val, r_val, "bin.rem"); },
          [&]() { op = builder.CreateURem(l_val, r_val, "bin.rem"); },
          [&]() { op = builder.CreateFRem(l_val, r_val, "bin.rem"); });

    break;
  }
  case ast::EOp_Bin::_divrem: break;
  case ast::EOp_Bin::_pow:    {
    arith(
        [&]() {
          auto* pow_fn = llvm::Intrinsic::getDeclaration(mod, llvm::Intrinsic::powi, {llvm_op_ty, llvm_op_ty});

          op = builder.CreateCall(pow_fn, {l_val, r_val}, "bin.pow");
        },
        nullptr,
        [&]() {
          auto* pow_fn = llvm::Intrinsic::getDeclaration(mod, llvm::Intrinsic::pow, {llvm_op_ty, llvm_op_ty});

          op = builder.CreateCall(pow_fn, {l_val, r_val}, "bin.pow");
        });

    break;
  }
  case ast::EOp_Bin::_gre: {
    arith([&]() { op = builder.CreateICmpSGT(l_val, r_val, "bin.gre"); },
          [&]() { op = builder.CreateICmpUGT(l_val, r_val, "bin.gre"); },
          [&]() { op = builder.CreateFCmpOGT(l_val, r_val, "bin.gre"); });

    byte([&]() { op = builder.CreateICmpUGT(l_val, r_val, "bin.gre"); });

    break;
  }
  case ast::EOp_Bin::_low: {
    arith([&]() { op = builder.CreateICmpSLT(l_val, r_val, "bin.low"); },
          [&]() { op = builder.CreateICmpULT(l_val, r_val, "bin.low"); },
          [&]() { op = builder.CreateFCmpOLT(l_val, r_val, "bin.low"); });

    byte([&]() { op = builder.CreateICmpULT(l_val, r_val, "bin.low"); });

    break;
  }
  case ast::EOp_Bin::_gre_eq: {
    arith([&]() { op = builder.CreateICmpSGE(l_val, r_val, "bin.gre_eq"); },
          [&]() { op = builder.CreateICmpUGE(l_val, r_val, "bin.gre_eq"); },
          [&]() { op = builder.CreateFCmpOGE(l_val, r_val, "bin.gre_eq"); });

    byte([&]() { op = builder.CreateICmpUGE(l_val, r_val, "bin.gre_eq"); });

    break;
  }
  case ast::EOp_Bin::_low_eq: {
    arith([&]() { op = builder.CreateICmpSLE(l_val, r_val, "bin.low_eq"); },
          [&]() { op = builder.CreateICmpULE(l_val, r_val, "bin.low_eq"); },
          [&]() { op = builder.CreateFCmpOLE(l_val, r_val, "bin.low_eq"); });

    byte([&]() { op = builder.CreateICmpULE(l_val, r_val, "bin.low_eq"); });

    break;
  }
  case ast::EOp_Bin::_in:
  case ast::EOp_Bin::_is:
  case ast::EOp_Bin::_eq: {
    arith([&]() { op = builder.CreateICmpEQ(l_val, r_val, "bin.eq"); }, nullptr,
          [&]() { op = builder.CreateFCmpOEQ(l_val, r_val, "bin.eq"); });

    byte([&]() { op = builder.CreateICmpEQ(l_val, r_val, "bin.eq"); });

    break;
  }
  case ast::EOp_Bin::_nin:
  case ast::EOp_Bin::_nis:
  case ast::EOp_Bin::_neq: {
    arith([&]() { op = builder.CreateICmpNE(l_val, r_val, "bin.neq"); }, nullptr,
          [&]() { op = builder.CreateFCmpONE(l_val, r_val, "bin.neq"); });

    byte([&]() { op = builder.CreateICmpNE(l_val, r_val, "bin.eq"); });

    break;
  }
  case ast::EOp_Bin::_eqs: {
    arith([&]() { op = builder.CreateICmpEQ(l_val, r_val, "bin.eqs"); }, nullptr,
          [&]() { op = builder.CreateFCmpOEQ(l_val, r_val, "bin.eqs"); });

    byte([&]() { op = builder.CreateICmpEQ(l_val, r_val, "bin.eq"); });

    break;
  }
  case ast::EOp_Bin::_neqs: {
    arith([&]() { op = builder.CreateICmpNE(l_val, r_val, "bin.neq"); }, nullptr,
          [&]() { op = builder.CreateFCmpONE(l_val, r_val, "bin.neq"); });

    byte([&]() { op = builder.CreateICmpNE(l_val, r_val, "bin.eq"); });

    break;
  }
  case ast::EOp_Bin::_b_and:
  case ast::EOp_Bin::_and:   {
    byte([&]() { op = builder.CreateAnd(l_val, r_val, "bin.and"); });

    break;
  }
  case ast::EOp_Bin::_b_nand:
  case ast::EOp_Bin::_nand:   {
    byte([&]() { op = builder.CreateNot(builder.CreateAnd(l_val, r_val, "bin.nand")); });

    break;
  }
  case ast::EOp_Bin::_b_or:
  case ast::EOp_Bin::_or:   {
    byte([&]() { op = builder.CreateOr(l_val, r_val, "bin.or"); });

    break;
  }
  case ast::EOp_Bin::_b_xor:
  case ast::EOp_Bin::_xor:   {
    byte([&]() { op = builder.CreateXor(l_val, r_val, "bin.xor"); });

    break;
  }
  case ast::EOp_Bin::_b_nor:
  case ast::EOp_Bin::_nor:   {
    byte([&]() { op = builder.CreateNot(builder.CreateOr(l_val, r_val, "bin.nor")); });

    break;
  }
  case ast::EOp_Bin::_b_xnor:
  case ast::EOp_Bin::_xnor:   {
    byte([&]() { op = builder.CreateNot(builder.CreateXor(l_val, r_val, "bin.xnor")); });

    break;
  }
  case ast::EOp_Bin::_b_shl_0: {
    byte([&]() { op = builder.CreateShl(l_val, r_val, "bin.ls0"); });

    break;
  }
  case ast::EOp_Bin::_b_shl_1: {
    auto* all_ones = builder.getInt32(~0);
    auto* shifted  = builder.CreateShl(l_val, r_val, "bin.ls0");
    auto* mask     = builder.CreateLShr(all_ones, builder.CreateSub(builder.getInt32(32), r_val), "bin.mask");
    op             = builder.CreateOr(shifted, mask, "ls1");

    break;
  }
  // impossible
  case ast::EOp_Bin::_b_shl_a: break;
  case ast::EOp_Bin::_b_shr_0: {
    byte([&]() { op = builder.CreateLShr(l_val, r_val, "bin.rs0"); });

    break;
  }
  case ast::EOp_Bin::_b_shr_1: {
    auto* lshr     = builder.CreateLShr(l_val, r_val, "lshr");
    auto* all_ones = builder.getInt32(~0);
    auto* shifted  = builder.CreateSub(builder.getInt32(32), r_val, "bin.shifted");
    auto* mask     = builder.CreateShl(all_ones, shifted, "bin.mask");
    op             = builder.CreateOr(lshr, mask, "bin.rs1");

    break;
  }
  case ast::EOp_Bin::_b_shr_a: {
    byte([&]() { op = builder.CreateAShr(l_val, r_val, "bin.rsa"); });

    break;
  }
  case ast::EOp_Bin::_b_rol: {
    byte([&]() {
      auto* fshr_fn = llvm::Intrinsic::getDeclaration(mod, llvm::Intrinsic::fshl, {l_val->getType()});
      op            = builder.CreateCall(fshr_fn, {l_val, l_val, r_val}, "bin.lr");
    });

    break;
  }
  case ast::EOp_Bin::_b_ror: {
    byte([&]() {
      auto* fshr_fn = llvm::Intrinsic::getDeclaration(mod, llvm::Intrinsic::fshr, {l_val->getType()});
      op            = builder.CreateCall(fshr_fn, {l_val, l_val, r_val}, "bin.rr");
    });

    break;
  }
  case ast::EOp_Bin::_ordering:
  case ast::EOp_Bin::_mem_add:
  case ast::EOp_Bin::_mem_sub:
  case ast::EOp_Bin::_mem_dist: break;
  }

  if (!op)
    res.add_error(207, left.get(),
                  std::format("Illegal operation ({}) between types:\n  `{}` {} `{}`",
                              std::string(ast::EOp_Bin_to_str(op_ty)), left.type().dump(), ast::EOp_Bin_to_str(op_ty),
                              right.type().dump()),
                  "");

  return op;
}

llvm::Constant* codegen::Tools::init_global_array(const type::Array& ty, llvm::ConstantArray* default_val,
                                                  bool is_uninit, bool is_const) const noexcept
{
  assert(!(is_uninit && is_const) && "Illegal uninit on constant");

  auto [wrapper_type, size] = res.types.codegen_Array(ty);
  auto*           array_ty  = llvm::ArrayType::get(res.get_type(ty.inner), size);
  llvm::Constant* init      = is_uninit ? nullptr : default_val ? default_val : llvm::Constant::getNullValue(array_ty);

  auto* storage =
      new llvm::GlobalVariable(*mod, array_ty, is_const, llvm::GlobalValue::InternalLinkage, init, "array.storage");

  llvm::SmallVector<llvm::Constant*, 2> indices = {res.const_int(0), res.const_int(0)};

  auto* data_ptr = llvm::ConstantExpr::getInBoundsGetElementPtr(array_ty, storage, indices);

  return llvm::ConstantStruct::get(wrapper_type, {data_ptr, res.const_int(size)});
}

llvm::Value* codegen::Tools::init_array(const type::Array& ty, llvm::ConstantArray* default_val,
                                        bool is_uninit) const noexcept
{
  auto [wrapper_type, size] = res.types.codegen_Array(ty);
  auto*           array_ty  = llvm::ArrayType::get(res.get_type(ty.inner), size);
  llvm::Constant* init      = is_uninit ? nullptr : default_val ? default_val : llvm::Constant::getNullValue(array_ty);

  auto* storage = builder.CreateAlloca(array_ty, nullptr, "array.storage");
  if (default_val) builder.CreateStore(init, storage);

  auto* data_ptr = builder.CreateInBoundsGEP(array_ty, storage, {res.const_int(0), res.const_int(0)}, "array.data");

  // 4. wrapper SSA
  llvm::Value* wrapper = llvm::UndefValue::get(wrapper_type);
  wrapper              = builder.CreateInsertValue(wrapper, data_ptr, 0);
  wrapper              = builder.CreateInsertValue(wrapper, res.const_int(size), 1);

  return wrapper;
}
