#pragma once

#include <cstddef>
#include <unordered_map>
#include <map>

#include "codegen/codegen.hpp"
#include "nexus/ids.hpp"
#include "nexus/type/forward.hpp"
#include "llvm_forward.hpp"


namespace codegen
{

struct Codegen_AST;

struct Codegen_Type {
  Codegen_Type(cu::CU& p_CU, llvm::LLVMContext& p_ctx);

  llvm::LLVMContext& ctx;
  cu::CU&            cu;
  Codegen_AST*       res;

  std::map<type::ID, llvm::Type*, type::ID::Compare> llvm_types;

  [[nodiscard]] llvm::Type* get(type::ID tyid) noexcept;
  [[nodiscard]] llvm::Type* replace_type(type::ID tyid, llvm::Type* ty) noexcept;

  void init_llvm_types() noexcept;

  size_t prepare_codegen_types() noexcept;

  [[nodiscard]] llvm::Type* codegen_type(type::ID tyid) noexcept;

  [[nodiscard]] llvm::Type*                          codegen_Primitive(const type::Primitive& ty) noexcept;
  [[nodiscard]] llvm::Type*                          codegen_String(const type::String& ty) noexcept;
  [[nodiscard]] llvm::Type*                          codegen_Tuple(const type::Tuple& ty) noexcept;
  // wrapper type, array size
  [[nodiscard]] std::pair<llvm::StructType*, size_t> codegen_Array(const type::Array& ty) noexcept;
  [[nodiscard]] llvm::Type*                          codegen_Buffer(const type::Buffer& ty) noexcept;
  [[nodiscard]] llvm::Type*                          codegen_Slice(const type::Slice& ty) noexcept;
  [[nodiscard]] llvm::Type*                          codegen_Ptr(const type::Ptr& ty) noexcept;
  [[nodiscard]] llvm::Type*                          codegen_Prototype(const type::Prototype& ty) noexcept;
  [[nodiscard]] llvm::Type*                          codegen_Parameter(const type::Prototype_Param& param) noexcept;
  [[nodiscard]] llvm::Type*                          codegen_Facet(const type::Facet& ty) noexcept;
  [[nodiscard]] llvm::Type*                          codegen_View(const type::View& ty) noexcept;
  [[nodiscard]] llvm::Type*                          codegen_Form(const type::Form& ty) noexcept;
  [[nodiscard]] llvm::Type*                          codegen_Enum(const type::Enum& ty) noexcept;
  [[nodiscard]] llvm::Type*                          codegen_Flag(const type::Flag& ty) noexcept;
  [[nodiscard]] llvm::Type*                          codegen_Union(const type::Union& ty) noexcept;
  [[nodiscard]] llvm::Type*                          codegen_Identifier(const type::Identifier& ty) noexcept;

  [[nodiscard]] std::pair<size_t, size_t> calculate_payload(const std::vector<llvm::Type*>& tys) const noexcept;
};


inline std::unordered_map<const type::ID, llvm::Type*, type::ID::Hash> llvm_primitives;

inline llvm::Type*        LLVM_TYPEID_u0      = nullptr;
inline llvm::Type*        LLVM_TYPEID_bool    = nullptr;
inline llvm::Type*        LLVM_TYPEID_cune    = nullptr;
inline llvm::Type*        LLVM_TYPEID_rune    = nullptr;
inline llvm::Type*        LLVM_TYPEID_ssize   = nullptr;
inline llvm::Type*        LLVM_TYPEID_s8      = nullptr;
inline llvm::Type*        LLVM_TYPEID_s16     = nullptr;
inline llvm::Type*        LLVM_TYPEID_s32     = nullptr;
inline llvm::Type*        LLVM_TYPEID_s64     = nullptr;
inline llvm::Type*        LLVM_TYPEID_s128    = nullptr;
inline llvm::Type*        LLVM_TYPEID_usize   = nullptr;
inline llvm::Type*        LLVM_TYPEID_u8      = nullptr;
inline llvm::Type*        LLVM_TYPEID_u16     = nullptr;
inline llvm::Type*        LLVM_TYPEID_u32     = nullptr;
inline llvm::Type*        LLVM_TYPEID_u64     = nullptr;
inline llvm::Type*        LLVM_TYPEID_u128    = nullptr;
inline llvm::Type*        LLVM_TYPEID_bsize   = nullptr;
inline llvm::Type*        LLVM_TYPEID_b8      = nullptr;
inline llvm::Type*        LLVM_TYPEID_b16     = nullptr;
inline llvm::Type*        LLVM_TYPEID_b32     = nullptr;
inline llvm::Type*        LLVM_TYPEID_b64     = nullptr;
inline llvm::Type*        LLVM_TYPEID_b128    = nullptr;
inline llvm::Type*        LLVM_TYPEID_ptrdiff = nullptr;
inline llvm::Type*        LLVM_TYPEID_fsize   = nullptr;
inline llvm::Type*        LLVM_TYPEID_f16     = nullptr;
inline llvm::Type*        LLVM_TYPEID_f32     = nullptr;
inline llvm::Type*        LLVM_TYPEID_f64     = nullptr;
inline llvm::Type*        LLVM_TYPEID_f80     = nullptr;
inline llvm::Type*        LLVM_TYPEID_f128    = nullptr;
inline llvm::Type*        LLVM_TYPEID_dsize   = nullptr;
inline llvm::Type*        LLVM_TYPEID_d32     = nullptr;
inline llvm::Type*        LLVM_TYPEID_d64     = nullptr;
inline llvm::Type*        LLVM_TYPEID_d128    = nullptr;
inline llvm::Type*        LLVM_TYPEID_udsize  = nullptr;
inline llvm::Type*        LLVM_TYPEID_ud32    = nullptr;
inline llvm::Type*        LLVM_TYPEID_ud64    = nullptr;
inline llvm::Type*        LLVM_TYPEID_ud128   = nullptr;
inline llvm::PointerType* LLVM_TYPEID_ptr     = nullptr;
inline llvm::PointerType* LLVM_TYPEID_cstr    = nullptr;
inline llvm::StructType*  LLVM_TYPEID_str     = nullptr;
inline llvm::StructType*  LLVM_TYPEID_text    = nullptr;

} // namespace codegen