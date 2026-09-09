#include "ffi/c_reader.hpp"

#include "ast/data.hpp"
#include "ast/forward.hpp"
#include "ast/node/declaration_global.hpp"
#include "ast/node/declaration_local.hpp"
#include "ast/node/declaration_sfm.hpp"
#include "ast/pool.hpp"
#include "compiler/compilation_unit.hpp"
#include "compiler/compiler.hpp"
#include "ffi/manager.hpp"
#include "id/base.hpp"
#include "id/nodeid.hpp"
#include "id/typeid.hpp"
#include "nexus/forward.hpp"
#include "type/pool.hpp"
#include "type/type.hpp"

#include "clang-c/CXErrorCode.h"
#include "clang-c/CXSourceLocation.h"

#include <cassert>
#include <clang-c/CXString.h>
#include <clang-c/Index.h>
#include <common/common.hpp>
#include <common/compiler_options.hpp>
#include <common/environment.hpp>
#include <common/fileutils.hpp>
#include <common/utils.hpp>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <initializer_list>
#include <marzer/toml++.hpp>
#include <memory>
#include <ostream>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>


namespace fs = std::filesystem;


ffi::C_Reader::C_Reader()
  : current_ast(std::make_unique<ffi::AST>())
  , current_cu(*current_ast->temp_cu)
{
}


void ffi::C_Reader::generate_libc_wrappers() noexcept
{
  const auto& ctx = OPTIONS;

  BindManifest m = {
      .binding_language          = "C",
      .target_arch               = ctx.target.triple.arch,
      .target_os                 = ctx.target.triple.platform,
      .target_abi                = ctx.target.triple.abi,
      .clang_libc                = ctx.c_ffi.libc,
      .clang_libc_version        = ctx.c_ffi.libc_version,
      .features_gnu_source       = ctx.target.triple.abi == common::env::EABI::gnu,
      .features_posix_c_source   = common::env::supports_posix(m.target_os),
      .features_file_offset_bits = common::env::file_offset_bits(m.clang_libc, m.target_arch),
      .features_time_bits        = common::env::time_bits(m.clang_libc, m.target_arch),
  };

  auto tmp_path = fs::path(common::env::get_cache_dir());
  fs::create_directories(tmp_path);
  tmp_path /= "tmp_include.c";

  const auto& c_ffi_args = ctx.c_ffi.generate_preprocessor_args();


  auto wrap_headers = [&](const std::initializer_list<std::string_view>& headers) {
    for (auto header : headers) {
      std::ofstream ofs(tmp_path);
      ofs.clear();
      ofs << "#include <" << header << ">\n";
      ofs.close();
      auto ast       = parse_c_compilation_unit(tmp_path.string(), c_ffi_args);
      ast->bind.lang = "C";
      ast->bind.lib  = header.substr(0, header.size() - 2);

      ast->tolza_codegen(ast->bind.get_file_path());
    }
  };

  wrap_headers(HEADERS_C_ISO);

  auto& platform = OPTIONS.target.triple.platform;

  // headers resolution
  if (platform == common::env::EPlatform::_linux) {
    wrap_headers(HEADERS_LINUX);
  } else if (platform == common::env::EPlatform::_macos || platform == common::env::EPlatform::_ios) {
    wrap_headers(HEADERS_POSIX);
    wrap_headers(HEADERS_APPLE_POSIX);
    wrap_headers(HEADERS_APPLE_BSD);
    wrap_headers(HEADERS_APPLE_MACH);
  } else if (platform == common::env::EPlatform::_windows) {
    wrap_headers(HEADERS_WINDOWS_CORE);
    wrap_headers(HEADERS_WINDOWS_SOCKET);
    wrap_headers(HEADERS_WINDOWS_ADVANCED);
  } else if (platform == common::env::EPlatform::_freebsd) {
    wrap_headers(HEADERS_POSIX);
    wrap_headers(HEADERS_FREEBSD);
  } else if (platform == common::env::EPlatform::_openbsd) {
    wrap_headers(HEADERS_POSIX);
    wrap_headers(HEADERS_OPENBSD);
  } else if (platform == common::env::EPlatform::_netbsd) {
    wrap_headers(HEADERS_POSIX);
    wrap_headers(HEADERS_NETBSD);
  } else if (platform == common::env::EPlatform::_dragonflybsd) {
    wrap_headers(HEADERS_POSIX);
    wrap_headers(HEADERS_DRAGONFLYBSD);
  } else if (platform == common::env::EPlatform::_android) {
    wrap_headers(HEADERS_POSIX);
    wrap_headers(HEADERS_LINUX);
    wrap_headers(HEADERS_ANDROID);
  } else if (platform == common::env::EPlatform::_solaris) {
    wrap_headers(HEADERS_POSIX);
    wrap_headers(HEADERS_SOLARIS);
  } else if (platform == common::env::EPlatform::NONE || platform == common::env::EPlatform::_custom) {
    wrap_headers(HEADERS_C_ISO);
  }

  std::string c_bindings_path = fs::path(OPTIONS.get_dir_binding_profile()) / "C";
  common::fileutils::write_barrel(c_bindings_path, "C");

  fs::remove(tmp_path);
}

void ffi::C_Reader::generate_c_api_wrappers(std::string_view from, std::string_view to,
                                            std::string_view unique_alias) noexcept
{
  const fs::path pfrom(from);
  const fs::path pto(to);

  const auto& c_ffi_args = OPTIONS.c_ffi.generate_preprocessor_args();

  auto generate_wrapper = [&c_ffi_args, this](const fs::path& _from, std::string_view _to) {
    auto ast       = parse_c_compilation_unit(_from.string(), c_ffi_args);
    ast->bind.lang = "C";
    ast->bind.lib  = _from.stem();
    ast->tolza_codegen(_to);
    return;
  };

  if (fs::is_regular_file(pfrom)) {
    if (common::fileutils::is_tolza_file(to)) {
      common::FATAL_ERROR("Must be a tolza file output when the origin is also a file.");
    }

    generate_wrapper(pfrom, to);
    return;
  }

  for (const auto& entry : fs::recursive_directory_iterator(pto)) {
    if (entry.is_regular_file() && entry.path().extension() == ".h") {
      const std::string out_path =
          std::string(pto / entry.path().stem()) + std::string(common::fileutils::TOLZA_FILE_EXTENSION);
      generate_wrapper(entry.path(), out_path);
    }
  }

  common::fileutils::write_barrel(to, unique_alias);
}


CXChildVisitResult ffi::c_universal_visitor(CXCursor p_cursor, CXCursor p_parent, CXClientData p_client_data) noexcept
{
  // no interop allowed on anonymous symbols
  if (clang_Cursor_isAnonymous(p_cursor)) return CXChildVisit_Recurse;

  // no interop allowed if internal
  CXLinkageKind linkage = clang_getCursorLinkage(p_cursor);
  if (linkage == CXLinkage_Internal) return CXChildVisit_Recurse;


  // if is some std code to not use
  const auto* name = clang_getCString(clang_getCursorDisplayName(p_cursor));
  if (name[0] != '\0' && name[1] != '\0') {
    if (name[0] == '_' && name[1] == '_') return CXChildVisit_Recurse;
    if (name[0] == '_' && name[1] == 'G' && name[2] == '_') return CXChildVisit_Recurse;
    if (name[3] != '\0') {
      if (name[0] == '_' && name[1] == 'I' && name[2] == 'O' && name[3] == '_') return CXChildVisit_Recurse;
    }
  }

  auto* reader = static_cast<ffi::C_Reader*>(p_client_data);

  auto [_, success] = reader->definitions_generated.emplace(name);
  if (!success) return CXChildVisit_Recurse; // continue recurse


  CXCursorKind kind = clang_getCursorKind(p_cursor);
  switch (kind) {
    /*
  case CXCursor_StructDecl: {
    if (!clang_isCursorDefinition(p_cursor)) break;
    (void)reader->c_struct_to_facet(p_cursor);
    break;
  }

  case CXCursor_UnionDecl: {
    if (!clang_isCursorDefinition(p_cursor)) break;
    (void)reader->c_union_to_union(p_cursor);
    break;
  }

  case CXCursor_EnumDecl: {
    if (!clang_isCursorDefinition(p_cursor)) break;
    (void)reader->c_enum_to_flag(p_cursor);
    break;
  }
    */

  case CXCursor_FunctionDecl: {
    (void)reader->c_function_to_func(p_cursor);
    break;
  }

  case CXCursor_VarDecl: {
    (void)reader->c_global_to_global(p_cursor);
    break;
  }

  default: break;
  }

  return CXChildVisit_Recurse; // continue recurse
}

std::unique_ptr<ffi::AST> ffi::C_Reader::parse_c_compilation_unit(std::string_view                p_file_path,
                                                                  const std::vector<std::string>& p_args) noexcept
{
  if (!fs::exists(p_file_path))
    common::FATAL_ERROR(std::format("[ffi:C::ERROR] The file located at \"{}\" dosen't exists.", p_file_path));

  CXIndex       index = clang_createIndex(0, 0);
  ffi::C_Reader r;

  std::vector<const char*> cargs;
  cargs.reserve(p_args.size());
  for (const auto& s : p_args) cargs.emplace_back(s.data());

  CXTranslationUnit tu;
  CXErrorCode       error = clang_parseTranslationUnit2(
      index, p_file_path.data(), cargs.data(), static_cast<int>(cargs.size()), nullptr, 0, CXTranslationUnit_None, &tu);

  assert(error == CXError_Success && "Failed to parse translation unit");

  CXCursor rootCursor = clang_getTranslationUnitCursor(tu);

  clang_visitChildren(rootCursor, c_universal_visitor, &r);

  clang_disposeTranslationUnit(tu);
  clang_disposeIndex(index);

  return std::move(r.current_ast);
}

CXType ffi::C_Reader::c_type_normalize(CXType t) noexcept
{
  while (true) {
    switch (t.kind) {
    case CXType_Elaborated: t = clang_Type_getNamedType(t); continue;
    case CXType_Attributed: t = clang_Type_getModifiedType(t); continue;
    default:                return t;
    }
  }
}

type::EPrimitiveTypeKind ffi::C_Reader::c_type_base_to_primitive(CXType t) noexcept
{
  t = c_type_normalize(t);

  auto map_signed = [&](size_t size) {
    switch (size) {
    case 1:  return type::EPrimitiveTypeKind::_s8;
    case 2:  return type::EPrimitiveTypeKind::_s16;
    case 4:  return type::EPrimitiveTypeKind::_s32;
    case 8:  return type::EPrimitiveTypeKind::_s64;
    case 16: return type::EPrimitiveTypeKind::_s128;
    default: return type::EPrimitiveTypeKind::NONE;
    }
  };

  auto map_unsigned = [&](size_t size) {
    switch (size) {
    case 1:  return type::EPrimitiveTypeKind::_u8;
    case 2:  return type::EPrimitiveTypeKind::_u16;
    case 4:  return type::EPrimitiveTypeKind::_u32;
    case 8:  return type::EPrimitiveTypeKind::_u64;
    case 16: return type::EPrimitiveTypeKind::_u128;
    default: return type::EPrimitiveTypeKind::NONE;
    }
  };

  auto map_floating = [&](size_t size) {
    switch (size) {
    case 2:  return type::EPrimitiveTypeKind::_f16;
    case 4:  return type::EPrimitiveTypeKind::_f32;
    case 8:  return type::EPrimitiveTypeKind::_f64;
    case 10: return type::EPrimitiveTypeKind::_f80;
    case 16: return type::EPrimitiveTypeKind::_f128;
    default: return type::EPrimitiveTypeKind::NONE;
    }
  };

  const auto type_size = clang_Type_getSizeOf(t);

  switch (t.kind) {
  case CXType_Char_S:
  case CXType_Char_U: return type::EPrimitiveTypeKind::_cune;

  case CXType_WChar:  {
    switch (type_size) {
    case 2:  return type::EPrimitiveTypeKind::_u16;
    case 4:  return type::EPrimitiveTypeKind::_rune;
    default: return type::EPrimitiveTypeKind::NONE;
    }
  }
  case CXType_Char16:     return type::EPrimitiveTypeKind::_u16;
  case CXType_Char32:     return type::EPrimitiveTypeKind::_rune;

  case CXType_SChar:
  case CXType_Short:
  case CXType_Int:
  case CXType_Long:
  case CXType_LongLong:   return map_signed(type_size);

  case CXType_UChar:
  case CXType_UShort:
  case CXType_UInt:
  case CXType_ULong:
  case CXType_ULongLong:  return map_unsigned(type_size);

  case CXType_Float:      return type::EPrimitiveTypeKind::_f32;
  case CXType_Double:     return type::EPrimitiveTypeKind::_f64;
  case CXType_Half:       return type::EPrimitiveTypeKind::_f16;
  case CXType_LongDouble: return map_floating(type_size);

  case CXType_Int128:     return type::EPrimitiveTypeKind::_s128;
  case CXType_UInt128:    return type::EPrimitiveTypeKind::_u128;

  case CXType_Bool:       return type::EPrimitiveTypeKind::_bool;
  case CXType_Void:       return type::EPrimitiveTypeKind::_u0;

  case CXType_NullPtr:    return type::EPrimitiveTypeKind::_opaque;

  default:                return type::EPrimitiveTypeKind::NONE;
  }
}

ast::EPassMode ffi::C_Reader::c_type_to_pass_mode(CXType input) noexcept
{
  //  Direct Value
  if (input.kind != CXType_Pointer) {
    return ast::EPassMode::copy;
  }

  // Level 1
  CXType lvl1 = clang_getPointeeType(input);

  bool lvl1_const = clang_isConstQualifiedType(lvl1);

  bool lvl1_is_ptr = lvl1.kind == CXType_Pointer;


  // Basic pointer
  if (!lvl1_is_ptr) {

    // -------------------------
    // const char* -> cstr
    // -------------------------

    bool is_char = lvl1.kind == CXType_Char_S || lvl1.kind == CXType_SChar || lvl1.kind == CXType_UChar;

    if (lvl1_const && is_char) {
      return ast::EPassMode::ref;
    }

    // -------------------------
    // const T* -> ref T
    // T*       -> mut T
    // -------------------------

    return lvl1_const ? ast::EPassMode::ref : ast::EPassMode::mut;
  }


  // Double pointer
  return ast::EPassMode::addr;
}

type::ID ffi::C_Reader::c_type_resolve_ptr(CXType input) noexcept
{
  size_t pointer_depth = 0;
  CXType t             = input;

  while (t.kind == CXType_Pointer) {
    ++pointer_depth;
    t = clang_getPointeeType(t);
  }

  if (pointer_depth == 0) return NO_ID;

  const bool is_pointer_const    = clang_isConstQualifiedType(input);
  const bool is_pointer_volatile = clang_isVolatileQualifiedType(input);

  type::Qualifier qualifier{
      .is_volatile = is_pointer_volatile,
      .is_constant = is_pointer_const,
  };

  type::ID inner_ty = c_type_to_type(t);
  type::ID temp_ty  = inner_ty;

  for (size_t i = 0; i < pointer_depth; ++i) {
    temp_ty = current_cu.types->factory.make_ptr(temp_ty, qualifier);
  }

  // char* / const char* -> cstr
  if (pointer_depth == 1 && inner_ty == type::TYPEID_cune) {
    const auto& ptr = temp_ty.as<type::Ptr>();

    if (ptr->header.qualifier.is_constant) return type::TYPEID_cstr;
  }

  if (pointer_depth == 1 && inner_ty == type::TYPEID_u0) {
    return current_cu.types->factory.make_ptr(type::TYPEID_opaque, qualifier);
  }

  return temp_ty;
}

type::ID ffi::C_Reader::c_type_resolve_atomic(CXType input) noexcept
{
  // const bool is_atomic = t.kind == CXType_Atomic;
  return type::ID::invalid();
}

type::ID ffi::C_Reader::c_type_resolve_array(CXType input, type::Qualifier& dec) noexcept
{
  CXType elem    = clang_getArrayElementType(input);
  auto   elem_ty = c_type_to_type(elem);

  return current_cu.types->factory.make_slice(elem_ty, dec, true);
}

type::ID ffi::C_Reader::c_type_resolve_proto(CXType input, type::Qualifier& dec) noexcept
{
  auto ret = c_type_to_type(clang_getResultType(input));

  const int  n           = clang_getNumArgTypes(input);
  const bool is_variadic = clang_isFunctionTypeVariadic(input);

  std::vector<type::Prototype_Param> params;
  params.reserve(n);

  for (int i = 0; i < n; i++) {
    CXType                arg = clang_getArgType(input, i);
    type::Prototype_Param param;

    param.passmode = c_type_to_pass_mode(arg);
    switch (param.passmode) {
    case ast::EPassMode::mut:
    case ast::EPassMode::ref:
    case ast::EPassMode::addr: arg = clang_getPointeeType(arg); break;
    default:                   break;
    }

    param.type = c_type_to_type(arg);

    // C string type detected (constnat pointer on char)
    if (param.passmode == ast::EPassMode::ref && param.type == type::TYPEID_cune) {
      param.type = type::TYPEID_cstr;
    }

    // no void parameter type -> set to opaque semantic
    if (param.type == type::TYPEID_u0) param.type = type::TYPEID_opaque;

    params.emplace_back(param);
  }

  return current_cu.types->factory.make_prototype(params, ret, is_variadic, dec);
}

type::ID ffi::C_Reader::c_type_resolve_typedef(CXCursor decl, type::Qualifier& dec) noexcept
{
  CXType underlying = clang_getTypedefDeclUnderlyingType(decl);
  underlying        = c_type_normalize(underlying);

  auto& alias_node = current_cu.ast->add_get<ast::Global_Alias_Type>();
  alias_node.alias = clang_getCString(clang_getCursorSpelling(decl));
  alias_node.type  = c_type_to_type(underlying);

  return current_cu.types->factory.make_identifier(alias_node.alias, alias_node.nodeid(), alias_node.nodeid().def(),
                                                   dec);
}

type::ID ffi::C_Reader::c_type_resolve_opaque(CXCursor decl, type::Qualifier& dec) noexcept
{
  const char* name = clang_getCString(clang_getCursorSpelling(decl));


  if (!current_ast->aliases_defined.contains(name)) {
    current_ast->aliases_defined.insert(name);

    auto& node = current_cu.ast->add_get<ast::Global_Alias_Type>();
    node.alias = name;

    // safe expansion
    {
      auto underlying = clang_getTypedefDeclUnderlyingType(decl);
      underlying      = c_type_normalize(underlying);

      auto prim = c_type_base_to_primitive(underlying);
      if (prim != type::EPrimitiveTypeKind::NONE) node.type = type::ID::make_primitive(prim);
    }
  }

  return current_cu.types->factory.make_forward_identifier(name, dec);
}

type::ID ffi::C_Reader::c_type_resolve_struct(CXCursor decl, type::Qualifier& dec) noexcept
{
  auto facet = c_struct_to_facet(decl); // <-- placeholder clean
  return facet.type();
}

type::ID ffi::C_Reader::c_type_resolve_union(CXCursor decl, type::Qualifier& dec) noexcept
{
  auto _union = c_union_to_union(decl);
  return _union.type();
}


type::ID ffi::C_Reader::c_type_resolve_enum(CXCursor decl, type::Qualifier& dec) noexcept
{
  auto _flag = c_enum_to_flag(decl);

  return _flag.type();
}

type::ID ffi::C_Reader::c_type_resolve_primitive(CXType input) noexcept
{
  const type::EPrimitiveTypeKind prim_ty = c_type_base_to_primitive(input);

  assert(prim_ty != type::EPrimitiveTypeKind::NONE && "Illegal type detected");

  // could be between 0 and ffi::AST::k_type_offset
  return type::ID::make(cu::ID::main(), static_cast<uint32_t>(prim_ty));
}

enum class FinalTypeKind : uint8_t { Primitive, Pointer, Array, Function, TypedefAlias, Struct, Union, Enum, Opaque };

bool is_known_libc_handle(CXCursor decl) noexcept
{
  CXString    spelling = clang_getCursorSpelling(decl);
  const char* name     = clang_getCString(spelling);

  if (!name || name[0] == '\0') {
    clang_disposeString(spelling);
    return false;
  }

  std::string_view n(name);

  // 1. LIST
  static const std::unordered_set<std::string_view> opaque_handles = {
      "FILE",
      "DIR",
      "regex_t",
      "locale_t",
      "sem_t",
      "pthread_t",
      "pthread_mutex_t",
      "pthread_cond_t",
      "pthread_rwlock_t",
      "pthread_key_t",
      "mqd_t",
      "timer_t",
      "clockid_t",
      "sigevent",
  };

  if (opaque_handles.contains(n)) {
    clang_disposeString(spelling);
    return true;
  }

  // 2. PREFIX PATTERNS (glibc / musl internal ABI types)
  auto has_prefix = [&](std::string_view p) { return n.starts_with(p); };

  if (has_prefix("_IO_") ||                               // glibc FILE internals
      has_prefix("__pthread") ||                          // pthread internals
      has_prefix("_pthread") || has_prefix("__darwin") || // macOS ABI
      has_prefix("_IO_FILE")) {
    clang_disposeString(spelling);
    return true;
  }

  // 3. HEURISTIC: incomplete struct used only via pointers
  CXType t = clang_getCursorType(decl);

  bool is_struct     = (clang_getCursorKind(decl) == CXCursor_StructDecl);
  bool is_incomplete = !clang_isCursorDefinition(decl);

  if (is_struct && is_incomplete) {
    // opaque if never used by value
    clang_disposeString(spelling);
    return true;
  }

  clang_disposeString(spelling);
  return false;
}

FinalTypeKind decide_final_kind(CXType t, CXCursor decl) noexcept
{
  const CXCursorKind k = clang_getCursorKind(decl);

  if (t.kind != CXType_Pointer && k == CXCursor_NoDeclFound) return FinalTypeKind::Primitive;

  if (k == CXCursor_TypedefDecl) return FinalTypeKind::TypedefAlias;

  if (clang_Location_isInSystemHeader(clang_getCursorLocation(decl))) return FinalTypeKind::Opaque;

  if (k == CXCursor_StructDecl || k == CXCursor_UnionDecl) {
    if (!clang_isCursorDefinition(decl)) return FinalTypeKind::Opaque;

    if (is_known_libc_handle(decl)) return FinalTypeKind::Opaque;

    return FinalTypeKind::Struct;
  }

  if (k == CXCursor_EnumDecl) return FinalTypeKind::Enum;

  return FinalTypeKind::Opaque;
}

int internal_score(CXCursor decl) noexcept
{
  int score = 0;

  const char*      name = clang_getCString(clang_getCursorSpelling(decl));
  CXSourceLocation loc  = clang_getCursorLocation(decl);

  // 1. rule header (STRONG)
  if (clang_Location_isInSystemHeader(loc)) score += 3;

  // 2. not a definition
  if (!clang_isCursorDefinition(decl)) score += 2;

  // 3. naming heuristic
  if (name) {
    std::string_view n(name);

    if (n.starts_with("__") || n.starts_with("_IO_") || n.starts_with("_pthread") || n.starts_with("__internal"))
      score += 1;
  }

  return score;
}

bool is_internal(CXCursor decl) noexcept
{
  return internal_score(decl) >= 3;
}


type::ID ffi::C_Reader::c_type_to_type(CXType input) noexcept
{
  auto t = c_type_normalize(input);

  if (auto id = c_type_resolve_ptr(t)) return id;

  // build qualifier
  const bool is_val_type_const    = clang_isConstQualifiedType(t);
  const bool is_val_type_volatile = clang_isVolatileQualifiedType(t);

  type::Qualifier dec{.is_volatile = is_val_type_volatile, .is_constant = is_val_type_const};

  // c_type_resolve_atomic(t);

  if (t.kind == CXType_ConstantArray || t.kind == CXType_IncompleteArray) return c_type_resolve_array(t, dec);

  if (input.kind == CXType_FunctionProto || input.kind == CXType_FunctionNoProto) return c_type_resolve_proto(t, dec);

  CXCursor      decl = clang_getTypeDeclaration(t);
  FinalTypeKind kind = decide_final_kind(t, decl);

  if (is_internal(decl)) return c_type_resolve_opaque(decl, dec);

  switch (kind) {
  case FinalTypeKind::Primitive:    return c_type_resolve_primitive(t);
  case FinalTypeKind::Pointer:      return c_type_resolve_ptr(t);
  case FinalTypeKind::Array:        return c_type_resolve_array(t, dec);
  case FinalTypeKind::Function:     return c_type_resolve_proto(t, dec);
  case FinalTypeKind::TypedefAlias: return c_type_resolve_typedef(decl, dec);
  case FinalTypeKind::Struct:       // return c_type_resolve_struct(decl, dec);
  case FinalTypeKind::Union:        // return c_type_resolve_union(decl, dec);
  case FinalTypeKind::Enum:         // return c_type_resolve_enum(decl, dec);
  case FinalTypeKind::Opaque:
  default:                          return c_type_resolve_opaque(decl, dec);
  }
}

ast::ID ffi::C_Reader::c_nodecl_to_opaque_facet(CXCursor cur) noexcept
{
  const auto name = clang_getCursorSpelling(cur);

  auto& node = current_cu.ast->add_get<ast::SFM_Facet>();

  node.name = clang_getCString(name);
  clang_disposeString(name);

  return node.nodeid();
}

struct Visit_Injector {
  ast::ID        nodeid;
  ffi::C_Reader& reader;
};

ast::ID ffi::C_Reader::c_struct_to_facet(CXCursor cur) noexcept
{
  const auto name = clang_getCursorSpelling(cur);

  auto& node = current_cu.ast->add_get<ast::SFM_Facet>();

  node.name = clang_getCString(name);
  clang_disposeString(name);

  auto data = Visit_Injector{.nodeid = node.nodeid(), .reader = *this};

  clang_visitChildren(
      cur,
      [](CXCursor cur, CXCursor parent, CXClientData client_data) {
        auto* inject = static_cast<Visit_Injector*>(client_data);
        auto* n_ptr  = inject->nodeid.as<ast::SFM_Facet>();

        if (clang_getCursorKind(cur) == CXCursor_FieldDecl) {
          const auto name = clang_getCursorSpelling(cur);
          const auto tyid = inject->reader.c_type_to_type(clang_getCursorType(cur));
          auto&      node = inject->reader.current_cu.ast->add_get<ast::SFM_Facet_Field>();
          node.name       = clang_getCString(name);
          clang_disposeString(name);

          n_ptr->fields.emplace_back(node.nodeid());
        }
        return CXChildVisit_Continue;
      },
      &data);

  std::vector<type::ID> tys;
  tys.reserve(node.fields.size());
  for (const auto& field : node.fields) {
    tys.emplace_back(field.type());
  }

  auto ty = current_cu.types->factory.make_facet(tys, node.nodeid().def());
  current_ast->inferences->add(node.nodeid(), ty);

  return node.nodeid();
}

ast::ID ffi::C_Reader::c_union_to_union(CXCursor cur) noexcept
{
  const auto name = clang_getCursorSpelling(cur);
  auto&      node = current_cu.ast->add_get<ast::Global_Union>();
  node.name       = clang_getCString(name);
  clang_disposeString(name);

  auto data = Visit_Injector{.nodeid = node.nodeid(), .reader = *this};

  clang_visitChildren(
      cur,
      [](CXCursor cur, CXCursor parent, CXClientData client_data) {
        auto* inject = static_cast<Visit_Injector*>(client_data);
        auto* n_ptr  = inject->nodeid.as<ast::Global_Union>();

        if (clang_getCursorKind(cur) == CXCursor_FieldDecl) {
          const auto name = clang_getCursorSpelling(cur);
          const auto tyid = inject->reader.c_type_to_type(clang_getCursorType(cur));
          auto&      node = inject->reader.current_cu.ast->add_get<ast::Union_Field>();
          node.name       = clang_getCString(name);
          node.type       = tyid;
          clang_disposeString(name);

          n_ptr->variants.emplace_back(node.nodeid());
        }
        return CXChildVisit_Continue;
      },
      &data);

  std::vector<type::ID> tys;
  tys.reserve(node.variants.size());
  for (const auto& field : node.variants) {
    tys.emplace_back(field.type());
  }

  auto ty = current_cu.types->factory.make_union(tys, node.nodeid().def());
  current_ast->inferences->add(node.nodeid(), ty);

  return node.nodeid();
}

ast::ID ffi::C_Reader::c_enum_to_flag(CXCursor cur) noexcept
{
  const auto cur_name = clang_getCursorSpelling(cur);
  auto&      node     = current_cu.ast->add_get<ast::Global_Flag>();
  node.name           = clang_getCString(cur_name);
  clang_disposeString(cur_name);

  auto data = Visit_Injector{.nodeid = node.nodeid(), .reader = *this};

  clang_visitChildren(
      cur,
      [](CXCursor cur, CXCursor parent, CXClientData client_data) {
        auto* inject = static_cast<Visit_Injector*>(client_data);
        auto* n_ptr  = inject->nodeid.as<ast::Global_Flag>();

        if (clang_getCursorKind(cur) == CXCursor_EnumConstantDecl) {
          const auto name = clang_getCursorSpelling(cur);
          const auto tyid = inject->reader.c_type_to_type(clang_getCursorType(cur));
          auto&      node = inject->reader.current_cu.ast->add_get<ast::Flag_Field>();
          node.name       = clang_getCString(name);
          clang_disposeString(name);

          n_ptr->flags.emplace_back(node.nodeid());
        }
        return CXChildVisit_Continue;
      },
      &data);

  auto ty = current_cu.types->factory.make_flag(node.flags.size(), node.nodeid().def());
  current_ast->inferences->add(node.nodeid(), ty);

  return node.nodeid();
}

ast::ID ffi::C_Reader::c_global_to_global(CXCursor cur) noexcept
{
  const auto cur_name = clang_getCursorSpelling(cur);
  const auto cur_ty   = clang_getCursorType(cur);
  auto&      node     = current_cu.ast->add_get<ast::Global_Variable>();
  node.name           = clang_getCString(cur_name);
  clang_disposeString(cur_name);

  if (clang_isConstQualifiedType(cur_ty))
    node.kind = ast::EVariableKind::_let;
  else
    node.kind = ast::EVariableKind::_var;

  node.type = c_type_to_type(cur_ty);

  return node.nodeid();
}

ast::ID ffi::C_Reader::c_function_to_func(CXCursor cur) noexcept
{
  const auto cur_name = clang_getCursorSpelling(cur);
  const auto cur_ty   = clang_getCursorType(cur);
  auto&      node     = current_cu.ast->add_get<ast::Global_Function>();
  node.name           = clang_getCString(cur_name);
  clang_disposeString(cur_name);

  node.prototype = c_type_to_type(cur_ty);
  auto* proto_ty = node.prototype.as<type::Prototype>();
  assert(proto_ty && "Must be a proto type");

  const int            n = clang_getNumArgTypes(cur_ty);
  std::vector<ast::ID> params;
  params.reserve(n);

  for (int i = 0; i < n; i++) {
    auto& proto_arg_id = proto_ty->params[i].type;

    auto arg      = clang_Cursor_getArgument(cur, i);
    auto arg_ty   = clang_getCursorType(arg);
    auto arg_name = clang_getCursorSpelling(arg);

    auto& n_param    = current_cu.ast->add_get<ast::Local_Parameter>();
    n_param.type     = proto_arg_id;
    n_param.passmode = c_type_to_pass_mode(arg_ty);
    n_param.name     = clang_getCString(arg_name);

    if (n_param.name.empty()) {
      n_param.name = std::format("__param_{}", char('a' + i));
    }

    params.emplace_back(n_param.nodeid());
    clang_disposeString(arg_name);
  }

  node.parameters = params;

  return node.nodeid();
}


ffi::BindManifest ffi::BindManifest::read_manifest(std::string_view path) noexcept
{
#define get_enum(_path, _enum_kind) _enum_kind##_from_str(tbl.at_path(_path).value_or(""))

#define get_str(_path) tbl.at_path("target.cpu").value_or("")

  toml::table tbl;
  try {
    tbl = toml::parse_file(path);
  } catch (...) {
    return BindManifest{};
  }

  BindManifest m;

  m.binding_language = get_str("binding.language");

  m.target_arch = common ::env ::EArch_from_str(tbl.at_path("target.arch").value_or(""));
  m.target_os   = get_enum("target.platform", common::env::EPlatform);
  m.target_abi  = get_enum("target.abi", common::env::EABI);

  m.clang_libc         = get_enum("libc.kind", common::env::ELibC);
  m.clang_libc_version = common::compiler::Cffi::LibCVersion::parse(get_str("libc.version"));

  m.compiler_clang_version = get_str("compiler.clang_version");

  auto f_gnu_src = tbl.at_path("features.gnu_source");
  if (!f_gnu_src)
    m.features_gnu_source = common::env::supports_gnu(m.target_os);
  else
    m.features_gnu_source = true;

  auto f_posix_c_src = tbl.at_path("features.posix_c_source");
  if (!f_posix_c_src)
    m.features_posix_c_source = common::env::supports_posix(m.target_os);
  else
    m.features_posix_c_source = true;

  auto f_f_offset_bits = tbl.at_path("features.file_offset_bits");
  if (!f_f_offset_bits)
    m.features_file_offset_bits = common::env::file_offset_bits(m.clang_libc, m.target_arch);
  else
    m.features_file_offset_bits = f_f_offset_bits.value_or(64);

  auto f_time_bits = tbl.at_path("features.time_bits");
  if (!f_time_bits)
    m.features_time_bits = common::env::time_bits(m.clang_libc, m.target_arch);
  else
    m.features_time_bits = f_time_bits.value_or(64);

  m.sysroot_path = get_str("sysroot.path");
  m.sysroot_hash = get_str("sysroot.hash");
  m.is_valid     = true;

  return m;
}
bool ffi::BindManifest::write_manifest(std::string_view path) const noexcept
{
  std::string txt(BINDER_MANIFEST);

  std::initializer_list<std::pair<std::string, std::string>> map{
      {"binding.language",         binding_language                                     },
      {"target.arch",              std::string(common::env::EArch_to_str(target_arch))  },
      {"target.platform",          std::string(common::env::EPlatform_to_str(target_os))},
      {"target.abi",               std::string(common::env::EABI_to_str(target_abi))    },
      {"libc.kind",                std::string(common::env::ELibC_to_str(clang_libc))   },
      {"libc.version",             clang_libc_version.dump()                            },
      {"compiler.clang_version",   compiler_clang_version                               },
      {"feature.gnu_source",       std::to_string(features_gnu_source)                  },
      {"feature.posix_c_source",   std::to_string(features_posix_c_source)              },
      {"feature.file_offset_bits", std::to_string(features_file_offset_bits)            },
      {"feature.time_bits",        std::to_string(features_time_bits)                   },
      {"sysroot.path",             sysroot_path                                         },
      {"sysroot.hash",             sysroot_hash                                         },
  };

  common::utils::fmt_template(txt, map);

  try {
    std::ofstream f(path.data());
    f.clear();
    f << txt << std::flush;
    f.close();
  } catch (...) {
    return false;
  }
  return true;
}

std::vector<std::string> ffi::BindManifest::to_clang_args() const noexcept
{
  std::vector<std::string> out;

  // GNU source
  if (features_gnu_source) {
    out.emplace_back("-D_GNU_SOURCE=1");
  }

  // POSIX level
  if (features_posix_c_source) {
    out.emplace_back(std::format("-D_POSIX_C_SOURCE={}", features_posix_c_source));
  }

  // File offset bits
  if (features_file_offset_bits != 0) {
    out.emplace_back(std::format("-D_FILE_OFFSET_BITS={}", features_file_offset_bits));
  }

  // Time bits
  if (features_time_bits != 0) {
    out.emplace_back(std::format("-D_TIME_BITS={}", features_time_bits));
  }

  // -------------------------
  // Architecture-specific defines (optional but useful)
  // -------------------------
  if (target_arch == common::env::EArch::x86_64) {
    out.emplace_back("-D__x86_64__=1");
  } else if (target_arch == common::env::EArch::aarch64) {
    out.emplace_back("-D__aarch64__=1");
  }

  // -------------------------
  // Sysroot include path
  // -------------------------
  if (!sysroot_path.empty()) {
    out.emplace_back("-isysroot " + sysroot_path);
  }

  return out;
}