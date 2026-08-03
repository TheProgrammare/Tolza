#include "codegen_type.hpp"

#include "codegen/static_evaluation.hpp"
#include "common/compiler_options.hpp"
#include "compiler/compilation_unit.hpp"
#include "compiler/compiler.hpp"
#include "misc/error_output.hpp"
#include "nexus/ast/data.hpp"
#include "nexus/ids.hpp"
#include "nexus/inference.hpp"
#include "nexus/type/data.hpp"
#include "nexus/type/definition.hpp"
#include "nexus/type/type.hpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <llvm-19/llvm/IR/Constants.h>
#include <llvm-19/llvm/IR/DerivedTypes.h>
#include <llvm-19/llvm/IR/Function.h>
#include <llvm-19/llvm/IR/Instructions.h>
#include <llvm-19/llvm/IR/LLVMContext.h>
#include <llvm-19/llvm/IR/Module.h>
#include <llvm-19/llvm/IR/Type.h>
#include <llvm-19/llvm/Support/Casting.h>
#include <llvm-19/llvm/Support/raw_ostream.h>
#include <vector>


codegen::Codegen_Type::Codegen_Type(cu::CU& p_CU, llvm::LLVMContext& p_ctx)
  : cu(p_CU)
  , ctx(p_ctx)
{
}

llvm::Type* codegen::Codegen_Type::get(const type::ID tyid) noexcept
{
  size_t offset = tyid.index();
  if (offset >= 0 && offset < type::TYPEID_USER_START)
    return llvm_primitives.at(type::ID::make(cu::ID::main(), offset));

  auto it = llvm_types.find(tyid);
  if (it == llvm_types.end()) return nullptr;
  return it->second;
}
llvm::Type* codegen::Codegen_Type::replace_type(const type::ID tyid, llvm::Type* ty) noexcept
{
  size_t offset = tyid.index();
  assert(offset >= type::TYPEID_USER_START && "Illegal type replace on primitives");

  llvm_types.try_emplace(tyid, ty);

  return ty;
}


void codegen::Codegen_Type::init_llvm_types() noexcept
{
  static bool once = false;

  if (once) return;
  once = true;

  const size_t arch_size = compiler::OPTIONS.target.get_arch_size();

  codegen::LLVM_TYPEID_u0      = llvm::Type::getVoidTy(ctx);
  codegen::LLVM_TYPEID_bool    = llvm::Type::getInt1Ty(ctx);
  codegen::LLVM_TYPEID_cune    = llvm::Type::getInt8Ty(ctx);
  codegen::LLVM_TYPEID_rune    = llvm::Type::getInt32Ty(ctx);
  codegen::LLVM_TYPEID_ssize   = llvm::Type::getIntNTy(ctx, arch_size);
  codegen::LLVM_TYPEID_s8      = llvm::Type::getInt8Ty(ctx);
  codegen::LLVM_TYPEID_s16     = llvm::Type::getInt16Ty(ctx);
  codegen::LLVM_TYPEID_s32     = llvm::Type::getInt32Ty(ctx);
  codegen::LLVM_TYPEID_s64     = llvm::Type::getInt64Ty(ctx);
  codegen::LLVM_TYPEID_s128    = llvm::Type::getInt128Ty(ctx);
  codegen::LLVM_TYPEID_usize   = llvm::Type::getIntNTy(ctx, arch_size);
  codegen::LLVM_TYPEID_u8      = llvm::Type::getInt8Ty(ctx);
  codegen::LLVM_TYPEID_u16     = llvm::Type::getInt16Ty(ctx);
  codegen::LLVM_TYPEID_u32     = llvm::Type::getInt32Ty(ctx);
  codegen::LLVM_TYPEID_u64     = llvm::Type::getInt64Ty(ctx);
  codegen::LLVM_TYPEID_u128    = llvm::Type::getInt128Ty(ctx);
  codegen::LLVM_TYPEID_bsize   = llvm::Type::getIntNTy(ctx, arch_size);
  codegen::LLVM_TYPEID_b8      = llvm::Type::getInt8Ty(ctx);
  codegen::LLVM_TYPEID_b16     = llvm::Type::getInt16Ty(ctx);
  codegen::LLVM_TYPEID_b32     = llvm::Type::getInt32Ty(ctx);
  codegen::LLVM_TYPEID_b64     = llvm::Type::getInt64Ty(ctx);
  codegen::LLVM_TYPEID_b128    = llvm::Type::getInt128Ty(ctx);
  codegen::LLVM_TYPEID_ptrdiff = llvm::Type::getIntNTy(ctx, arch_size);
  codegen::LLVM_TYPEID_fsize   = arch_size == 16    ? llvm::Type::getHalfTy(ctx)
                                 : arch_size == 32  ? llvm::Type::getFloatTy(ctx)
                                 : arch_size == 128 ? llvm::Type::getFP128Ty(ctx)
                                                    : llvm::Type::getDoubleTy(ctx);

  codegen::LLVM_TYPEID_f16    = llvm::Type::getHalfTy(ctx);
  codegen::LLVM_TYPEID_f32    = llvm::Type::getFloatTy(ctx);
  codegen::LLVM_TYPEID_f64    = llvm::Type::getDoubleTy(ctx);
  codegen::LLVM_TYPEID_f80    = llvm::Type::getX86_FP80Ty(ctx);
  codegen::LLVM_TYPEID_f128   = llvm::Type::getFP128Ty(ctx);
  codegen::LLVM_TYPEID_dsize  = llvm::Type::getIntNTy(ctx, arch_size);
  codegen::LLVM_TYPEID_d32    = llvm::Type::getInt32Ty(ctx);
  codegen::LLVM_TYPEID_d64    = llvm::Type::getInt64Ty(ctx);
  codegen::LLVM_TYPEID_d128   = llvm::Type::getInt128Ty(ctx);
  codegen::LLVM_TYPEID_udsize = llvm::Type::getIntNTy(ctx, arch_size);
  codegen::LLVM_TYPEID_ud32   = llvm::Type::getInt32Ty(ctx);
  codegen::LLVM_TYPEID_ud64   = llvm::Type::getInt64Ty(ctx);
  codegen::LLVM_TYPEID_ud128  = llvm::Type::getInt128Ty(ctx);
  codegen::LLVM_TYPEID_ptr    = llvm::PointerType::getUnqual(ctx);
  codegen::LLVM_TYPEID_cstr   = llvm::PointerType::getUnqual(ctx);
  codegen::LLVM_TYPEID_str =
      llvm::StructType::get(ctx, {llvm::PointerType::getUnqual(LLVM_TYPEID_cune), LLVM_TYPEID_usize});
  codegen::LLVM_TYPEID_text =
      llvm::StructType::get(ctx, {llvm::PointerType::getUnqual(LLVM_TYPEID_rune), LLVM_TYPEID_usize});

  llvm_primitives.reserve(type::TYPEID_USER_START);
  llvm_primitives.try_emplace(type::TYPEID_u0, LLVM_TYPEID_u0);
  llvm_primitives.try_emplace(type::TYPEID_bool, LLVM_TYPEID_bool);
  llvm_primitives.try_emplace(type::TYPEID_cune, LLVM_TYPEID_cune);
  llvm_primitives.try_emplace(type::TYPEID_rune, LLVM_TYPEID_rune);
  llvm_primitives.try_emplace(type::TYPEID_ssize, LLVM_TYPEID_ssize);
  llvm_primitives.try_emplace(type::TYPEID_s8, LLVM_TYPEID_s8);
  llvm_primitives.try_emplace(type::TYPEID_s16, LLVM_TYPEID_s16);
  llvm_primitives.try_emplace(type::TYPEID_s32, LLVM_TYPEID_s32);
  llvm_primitives.try_emplace(type::TYPEID_s64, LLVM_TYPEID_s64);
  llvm_primitives.try_emplace(type::TYPEID_s128, LLVM_TYPEID_s128);
  llvm_primitives.try_emplace(type::TYPEID_usize, LLVM_TYPEID_usize);
  llvm_primitives.try_emplace(type::TYPEID_u8, LLVM_TYPEID_u8);
  llvm_primitives.try_emplace(type::TYPEID_u16, LLVM_TYPEID_u16);
  llvm_primitives.try_emplace(type::TYPEID_u32, LLVM_TYPEID_u32);
  llvm_primitives.try_emplace(type::TYPEID_u64, LLVM_TYPEID_u64);
  llvm_primitives.try_emplace(type::TYPEID_u128, LLVM_TYPEID_u128);
  llvm_primitives.try_emplace(type::TYPEID_bsize, LLVM_TYPEID_bsize);
  llvm_primitives.try_emplace(type::TYPEID_b8, LLVM_TYPEID_b8);
  llvm_primitives.try_emplace(type::TYPEID_b16, LLVM_TYPEID_b16);
  llvm_primitives.try_emplace(type::TYPEID_b32, LLVM_TYPEID_b32);
  llvm_primitives.try_emplace(type::TYPEID_b64, LLVM_TYPEID_b64);
  llvm_primitives.try_emplace(type::TYPEID_b128, LLVM_TYPEID_b128);
  llvm_primitives.try_emplace(type::TYPEID_ptrdiff, LLVM_TYPEID_ptrdiff);
  llvm_primitives.try_emplace(type::TYPEID_fsize, LLVM_TYPEID_fsize);
  llvm_primitives.try_emplace(type::TYPEID_f16, LLVM_TYPEID_f16);
  llvm_primitives.try_emplace(type::TYPEID_f32, LLVM_TYPEID_f32);
  llvm_primitives.try_emplace(type::TYPEID_f64, LLVM_TYPEID_f64);
  llvm_primitives.try_emplace(type::TYPEID_f80, LLVM_TYPEID_f80);
  llvm_primitives.try_emplace(type::TYPEID_f128, LLVM_TYPEID_f128);
  llvm_primitives.try_emplace(type::TYPEID_dsize, LLVM_TYPEID_dsize);
  llvm_primitives.try_emplace(type::TYPEID_d32, LLVM_TYPEID_d32);
  llvm_primitives.try_emplace(type::TYPEID_d64, LLVM_TYPEID_d64);
  llvm_primitives.try_emplace(type::TYPEID_d128, LLVM_TYPEID_d128);
  llvm_primitives.try_emplace(type::TYPEID_udsize, LLVM_TYPEID_udsize);
  llvm_primitives.try_emplace(type::TYPEID_ud32, LLVM_TYPEID_ud32);
  llvm_primitives.try_emplace(type::TYPEID_ud64, LLVM_TYPEID_ud64);
  llvm_primitives.try_emplace(type::TYPEID_ud128, LLVM_TYPEID_ud128);
  llvm_primitives.try_emplace(type::TYPEID_opaque, LLVM_TYPEID_ptr);
  llvm_primitives.try_emplace(type::TYPEID_cstr, LLVM_TYPEID_cstr);
  llvm_primitives.try_emplace(type::TYPEID_str, LLVM_TYPEID_str);
  llvm_primitives.try_emplace(type::TYPEID_text, LLVM_TYPEID_text);

  for (auto [nodeid, tyid] : compiler::inference.inference) {
    (void)codegen_type(tyid);
  }
}


size_t codegen::Codegen_Type::prepare_codegen_types() noexcept
{
  // llvm_types.reserve(cu.types->types.size());

  size_t offset = type::TYPEID_USER_START;

  for (size_t i = offset; i < offset + cu.types->types.size(); ++i) (void)codegen_type(type::ID::make(cu.cuid, i));

  return cu.types->types.size();
}

llvm::Type* codegen::Codegen_Type::codegen_type(const type::ID tyid) noexcept
{
  assert(tyid);

  if (auto it = llvm_types.find(tyid); it != llvm_types.end()) return it->second;

  llvm::Type* out   = nullptr;
  const auto  canon = tyid.canonical();
  const auto  kind  = canon.get().kind();

#define case_ty(name)                                                                                                  \
  case type::ETypeKind::name: out = codegen_##name(*canon.as<type::name>()); break;

  switch (kind) {
    case_ty(Primitive);
    case_ty(String);
    case_ty(Tuple);
    case_ty(Buffer);
    case_ty(Slice);
    case_ty(Ptr);
    case_ty(Prototype);
    case_ty(Facet);
    case_ty(View);
    case_ty(Form);
    case_ty(Enum);
    case_ty(Flag);
    case_ty(Union);
    case_ty(Identifier);
  case type::ETypeKind::Array: {
    auto* ty = codegen_Array(*canon.as<type::Array>()).first;
    llvm_types.try_emplace(tyid, ty);
    return ty;
  }
  default: assert(false && "Invalid type defined as None");
  }

#undef case_ty

  llvm_types.try_emplace(tyid, out);
  return out;
}

llvm::Type* codegen::Codegen_Type::codegen_Primitive(const type::Primitive& ty) noexcept
{
  return llvm_primitives.at(type::ID::make_primitive(ty.primitive));
}

llvm::Type* codegen::Codegen_Type::codegen_String(const type::String& ty) noexcept
{
  switch (ty.kind) {
  case type::ETextType::_str:  return llvm_primitives.at(type::TYPEID_str);
  case type::ETextType::_cstr: return llvm_primitives.at(type::TYPEID_cstr);
  case type::ETextType::_text: return llvm_primitives.at(type::TYPEID_text);
  case type::ETextType::_cune: return llvm_primitives.at(type::TYPEID_cune);
  case type::ETextType::_rune: return llvm_primitives.at(type::TYPEID_rune);
  case type::ETextType::NONE:  assert(false && "Invalid type facial kind");
  }
}

llvm::Type* codegen::Codegen_Type::codegen_Tuple(const type::Tuple& ty) noexcept
{
  std::vector<llvm::Type*> tys;
  tys.reserve(ty.elems.size());
  for (const auto& elemid : ty.elems) tys.push_back(codegen_type(elemid));

  return llvm::StructType::create(ctx, tys);
}

std::pair<llvm::StructType*, size_t> codegen::Codegen_Type::codegen_Array(const type::Array& ty) noexcept
{
  if (auto it = llvm_types.find(ty.tyid); it != llvm_types.end()) {
    auto* llvm_ty = llvm::cast<llvm::StructType>(it->second);
    return {llvm_ty, ty.size};
  }

  llvm::ArrayType* array_data = nullptr;
  size_t           size       = 0;

  if (ty.size <= 0) {
    auto expr = res->eval.evaluate_expression(ty.size_expression);

    if (expr) {
      auto* v    = llvm::cast<llvm::ConstantInt>(expr.value());
      size       = v->getValue().getLimitedValue();
      array_data = llvm::ArrayType::get(codegen_type(ty.inner.canonical()), size);
    }

    if (ty.size_expression.canonical()) {
      std::cout << ty.size_expression.dump() << "\n";
      std::cout << ty.size_expression.token().line_str() << "\n";
      compiler::COMPILER.add_error(
          Error_Diagnostic(cu.cuid, 279, ty.size_expression, ty.size_expression.canonical(), compiler::EPhase::llvmir,
                           "Impossible to evaluate the expression for a table size at compilation time.", ""));
    } else {
      compiler::COMPILER.add_error(
          Error_Diagnostic(cu.cuid, 279, ty.size_expression, compiler::EPhase::llvmir,
                           "Impossible to evaluate the expression for a table size at compilation time.", ""));
    }
  } else {
    array_data = llvm::ArrayType::get(codegen_type(ty.inner.canonical()), ty.size);
    size       = ty.size;
  }

  // array {data: ptr'T, size: usize}
  std::array<llvm::Type*, 2> tys;
  tys[0] = llvm::PointerType::getUnqual(codegen_type(ty.inner)); // data
  tys[1] = llvm_primitives.at(type::TYPEID_usize);               // size

  auto* out_ty = llvm::StructType::create(ctx, tys, "array");
  llvm_types.try_emplace(ty.tyid, out_ty);
  return {out_ty, size};
}

llvm::Type* codegen::Codegen_Type::codegen_Buffer(const type::Buffer& ty) noexcept
{
  // buffer {data: ptr'T, size: usize, capacity: usize}
  std::array<llvm::Type*, 3> tys;
  tys[0] = llvm::PointerType::getUnqual(codegen_type(ty.inner)); // data
  tys[1] = llvm_primitives.at(type::TYPEID_usize);               // size
  tys[2] = llvm_primitives.at(type::TYPEID_usize);               // capacity

  return llvm::StructType::create(ctx, tys, "buffer");
}

llvm::Type* codegen::Codegen_Type::codegen_Slice(const type::Slice& ty) noexcept
{
  // slice {data: ptr'T, size: usize }
  std::array<llvm::Type*, 2> tys;
  tys[0] = llvm::PointerType::getUnqual(codegen_type(ty.inner)); // data
  tys[1] = llvm_primitives.at(type::TYPEID_usize);               // size

  return llvm::StructType::create(ctx, tys, "slice");
}

llvm::Type* codegen::Codegen_Type::codegen_Ptr(const type::Ptr& ty) noexcept
{
  return llvm::PointerType::getUnqual(codegen_type(ty.inner));
}


llvm::Type* codegen::Codegen_Type::codegen_Prototype(const type::Prototype& ty) noexcept
{
  auto* ret = codegen_type(ty.ret);

  std::vector<llvm::Type*> params;
  params.reserve(ty.params.size());
  for (const auto& param : ty.params) {
    params.push_back(codegen_Parameter(param));
  }

  return llvm::FunctionType::get(ret, params, ty.is_variadic);
}

llvm::Type* codegen::Codegen_Type::codegen_Parameter(const type::Prototype_Param& param) noexcept
{
  auto* pty = codegen_type(param.type);

  switch (param.passmode) {
  case ast::EPassMode::NONE:
  case ast::EPassMode::move:
  case ast::EPassMode::ref:  {
    if (pty->isSingleValueType()) return pty;
    return pty->getPointerTo();
  }
  case ast::EPassMode::mut: {
    return pty->getPointerTo();
  }
  case ast::EPassMode::copy: {
    if (pty->isSingleValueType()) return pty;
    return pty->getPointerTo();
  }
  case ast::EPassMode::addr: {
    return pty->getPointerTo()->getPointerTo();
  }
  }
}

llvm::Type* codegen::Codegen_Type::codegen_Facet(const type::Facet& ty) noexcept
{
  std::vector<llvm::Type*> tys;
  tys.reserve(ty.fields.size());
  for (const auto& elemid : ty.fields) tys.push_back(codegen_type(elemid));

  return llvm::StructType::create(ctx, tys);
}

llvm::Type* codegen::Codegen_Type::codegen_View(const type::View& ty) noexcept
{
  std::vector<llvm::Type*> tys;
  tys.reserve(ty.facets.size());
  for (const auto& elemid : ty.facets) tys.push_back(codegen_type(elemid));

  return llvm::StructType::create(ctx, tys);
}

llvm::Type* codegen::Codegen_Type::codegen_Form(const type::Form& ty) noexcept
{
  std::vector<llvm::Type*> tys;
  tys.reserve(ty.facets.size());
  for (const auto& elemid : ty.facets) tys.push_back(codegen_type(elemid));

  return llvm::StructType::create(ctx, tys);
}

llvm::Type* codegen::Codegen_Type::codegen_Enum(const type::Enum& ty) noexcept
{
  std::vector<llvm::Type*> tys;
  tys.reserve(ty.variants.size());
  for (const auto& elemid : ty.variants) tys.push_back(codegen_type(elemid));

  const auto& [max_size, max_align] = calculate_payload(tys);
  uint64_t payload_size             = llvm::alignTo(max_size, max_size);

  // {index: usize, payload: [u8, N]}
  std::array<llvm::Type*, 2> struct_data;
  struct_data[0] = llvm_primitives.at(type::TYPEID_usize);                         // index
  struct_data[1] = llvm::ArrayType::get(llvm::Type::getInt8Ty(ctx), payload_size); // payload

  return llvm::StructType::create(ctx, struct_data);
}

llvm::Type* codegen::Codegen_Type::codegen_Flag(const type::Flag& ty) noexcept
{
  return llvm::Type::getIntNTy(ctx, ty.size);
}

llvm::Type* codegen::Codegen_Type::codegen_Union(const type::Union& ty) noexcept
{
  std::vector<llvm::Type*> tys;
  tys.reserve(ty.variants.size());
  for (const auto& elemid : ty.variants) tys.push_back(codegen_type(elemid));

  const auto& [max_size, max_align] = calculate_payload(tys);
  uint64_t payload_size             = llvm::alignTo(max_size, max_size);

  // return the payload directly
  return llvm::ArrayType::get(llvm::Type::getInt8Ty(ctx), payload_size); // payload
}

llvm::Type* codegen::Codegen_Type::codegen_Identifier(const type::Identifier& ty) noexcept
{
  return codegen_type(ty.tyid.canonical());
}

std::pair<size_t, size_t> codegen::Codegen_Type::calculate_payload(const std::vector<llvm::Type*>& tys) const noexcept
{
  const llvm::DataLayout& DL         = cu.llvm_module->getDataLayout();
  llvm::Type*             largest_ty = nullptr;
  uint64_t                max_size   = 0;
  uint64_t                max_align  = 1;

  for (llvm::Type* t : tys) {
    auto size  = DL.getTypeAllocSize(t);
    auto align = DL.getABITypeAlign(t).value();

    if (size > max_size) {
      max_size   = size;
      largest_ty = t;
    }

    max_align = std::max(max_align, align);
  }

  return {max_size, max_align};
}
