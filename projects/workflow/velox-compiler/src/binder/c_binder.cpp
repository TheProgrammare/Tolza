
#include "c_binder.hpp"

#include <assert.h>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <ostream>

#include <compiler_context.hpp>
#include <common.hpp>

#include "binder/binder_ffi.hpp"
#include "misc/script_info.hpp"


void ffi::c::c_lib_to_velox_lib(const ffi::Bind_Package& _bind)
{
  auto tmp_path = std::filesystem::path(common::get_cache_dir());
  std::filesystem::create_directories(tmp_path);
  tmp_path /= "tmp_include.c";

  {
    std::ofstream ofs(tmp_path);
    ofs.clear();
    ofs << "#include <" << _bind.lib << ".h>\n";
  }

  ffi::AST ast = parse_translation_unit(_bind, tmp_path, {});
  Import   imp;
  imp.type = Import::EImportType::stdlib;
  imp.path = {"ffi"};
  imp.name = "C";

  ast.imports["C"] = imp;

  ffi::write_ast(ast, _bind.path);

  std::filesystem::remove(tmp_path);
}

CXChildVisitResult ffi::c::universal_visitor(CXCursor cursor, CXCursor parent, CXClientData client_data)
{
  ffi::AST*    ast  = static_cast<ffi::AST*>(client_data);
  CXCursorKind kind = clang_getCursorKind(cursor);

  // no interop allowed if internal
  CXLinkageKind linkage = clang_getCursorLinkage(cursor);
  if (linkage == CXLinkage_Internal) return CXChildVisit_Recurse;

  auto find_item = [&](const std::vector<Extern_Item>& items, const std::string& name) -> bool {
    for (auto item : items) {
      // no mangling in C
      if (item.name == name) return true;
    }
    return false;
  };


  switch (kind) {
  case CXCursor_StructDecl: {
    std::string name = clang_getCString(clang_getCursorSpelling(cursor));
    if (!find_item(ast->bind.extern_comp, name)) break;

    if (!clang_isCursorDefinition(cursor)) break;
    ffi::Comp comp        = c_struct_to_comp(cursor);
    ast->comps[comp.name] = std::move(comp);
    break;
  }

  case CXCursor_UnionDecl: {
    std::string name = clang_getCString(clang_getCursorSpelling(cursor));
    if (!find_item(ast->bind.extern_union, name)) break;

    if (!clang_isCursorDefinition(cursor)) break;
    ffi::Union u        = c_union_to_union(cursor);
    ast->unions[u.name] = std::move(u);
    break;
  }

  case CXCursor_EnumDecl: {
    std::string name = clang_getCString(clang_getCursorSpelling(cursor));
    if (!find_item(ast->bind.extern_flag, name)) break;

    if (!clang_isCursorDefinition(cursor)) break;
    ffi::Flag e        = c_enum_to_flag(cursor);
    ast->flags[e.name] = e;
    break;
  }

  case CXCursor_FunctionDecl: {
    std::string name = clang_getCString(clang_getCursorSpelling(cursor));
    if (!find_item(ast->bind.extern_fn, name)) break;

    ffi::Func f        = c_function_to_func(cursor);
    ast->funcs[f.name] = std::move(f);
    break;
  }

  case CXCursor_VarDecl: {
    std::string name = clang_getCString(clang_getCursorSpelling(cursor));
    if (!find_item(ast->bind.extern_glo, name)) break;

    ffi::Global g        = c_global_to_global(cursor);
    ast->globals[g.name] = std::move(g);
    break;
  }

  default: break;
  }

  return CXChildVisit_Recurse; // continuer récursivement
}

ffi::AST ffi::c::parse_translation_unit(const ffi::Bind_Package& _bind, const std::string& file_path,
                                        const std::vector<std::string>& args = {})
{
  CXIndex index = clang_createIndex(0, 0);

  std::vector<const char*> cargs;
  for (const auto& s : args) cargs.push_back(s.c_str());

  CXTranslationUnit tu;
  CXErrorCode       error = clang_parseTranslationUnit2(
      index, file_path.c_str(), cargs.data(), static_cast<int>(cargs.size()), nullptr, 0, CXTranslationUnit_None, &tu);

  assert(error == CXError_Success && "Failed to parse translation unit");

  CXCursor rootCursor = clang_getTranslationUnitCursor(tu);

  ffi::AST ast;
  ast.bind = _bind;
  clang_visitChildren(rootCursor, universal_visitor, &ast);

  clang_disposeTranslationUnit(tu);
  clang_disposeIndex(index);

  return ast;
}

ffi::EType ffi::c::c_type_base_to_type_base(CXType cType, CXType& out_base_cType)
{
  switch (cType.kind) {
  case CXType_Char_S:
  case CXType_SChar:      out_base_cType = cType; return ffi::EType::_schar;
  case CXType_Short:      out_base_cType = cType; return ffi::EType::_short;
  case CXType_Int:        out_base_cType = cType; return ffi::EType::_int;
  case CXType_LongLong:   out_base_cType = cType; return ffi::EType::_longlong;
  case CXType_Long:       out_base_cType = cType; return ffi::EType::_long; // target dependant
  case CXType_Char_U:
  case CXType_UChar:      out_base_cType = cType; return ffi::EType::_uchar;
  case CXType_UShort:     out_base_cType = cType; return ffi::EType::_ushort;
  case CXType_UInt:       out_base_cType = cType; return ffi::EType::_uint;
  case CXType_ULong:      out_base_cType = cType; return ffi::EType::_ulong; // target dependant
  case CXType_ULongLong:  out_base_cType = cType; return ffi::EType::_ulonglong;
  case CXType_Float:      out_base_cType = cType; return ffi::EType::_float;
  case CXType_Double:     out_base_cType = cType; return ffi::EType::_double;
  case CXType_LongDouble: out_base_cType = cType; return ffi::EType::_longdouble;
  case CXType_Bool:       out_base_cType = cType; return ffi::EType::_bool;
  case CXType_Void:       out_base_cType = cType; return ffi::EType::_void;
  case CXType_Pointer:    {
    CXType pointee_type = clang_getPointeeType(cType);
    return c_type_base_to_type_base(pointee_type, out_base_cType);
  }
  case CXType_Record:          out_base_cType = cType; return ffi::EType::_comp;
  case CXType_Enum:            out_base_cType = cType; return ffi::EType::_flag;
  case CXType_IncompleteArray:
  case CXType_ConstantArray:   {
    CXType pointee_type = clang_getPointeeType(cType);
    return c_type_base_to_type_base(pointee_type, out_base_cType);
  }
  case CXType_FunctionProto:
  case CXType_FunctionNoProto: out_base_cType = cType; return ffi::EType::_proto;
  default:                     out_base_cType = cType; return ffi::EType::_alias; // typedef / unknown
  }
}

ffi::Type ffi::c::c_type_to_type(CXType cType)
{
  ffi::Type vt;
  CXType    base_cType;

  vt.base_type = c_type_base_to_type_base(cType, base_cType);


  vt.is_pointer = cType.kind == CXType_Pointer;
  vt.is_table   = cType.kind == CXType_ConstantArray || cType.kind == CXType_IncompleteArray;

  vt.is_val_type_const    = clang_isConstQualifiedType(base_cType);
  vt.is_val_type_volatile = clang_isVolatileQualifiedType(base_cType);

  if (clang_isConstQualifiedType(cType)) {
    if (vt.is_pointer)
      vt.is_pointer_const = true;
    else
      vt.is_val_type_const = true;
  }
  if (clang_isVolatileQualifiedType(cType)) {
    if (vt.is_pointer)
      vt.is_pointer_volatile = true;
    else
      vt.is_val_type_volatile = true;
  }


  if (cType.kind == CXType_Atomic) {
    vt.is_atomic = true;
  }


  if (vt.is_pointer) {
    CXType pointee       = clang_getPointeeType(cType);
    vt.is_pointer_double = pointee.kind == CXType_Pointer;

    vt.is_pointer_const    = clang_isConstQualifiedType(cType);
    vt.is_pointer_volatile = clang_isVolatileQualifiedType(cType);

    vt.is_pointer_on_table = base_cType.kind == CXType_ConstantArray || base_cType.kind == CXType_IncompleteArray;
  }

  else if (vt.is_table) {
    CXType element_type     = clang_getArrayElementType(cType);
    vt.is_table_of_pointers = element_type.kind = CXType_Pointer;
  }


  if (base_cType.kind == CXType_ConstantArray) {
    vt.is_table = true;
    vt.table_size.push_back(static_cast<size_t>(clang_getArraySize(cType)));
  } else if (base_cType.kind == CXType_IncompleteArray) {
    vt.is_table = true; // flexible
  }

  if (base_cType.kind == CXType_FunctionProto || base_cType.kind == CXType_FunctionNoProto) {

    CXType returnType          = clang_getResultType(base_cType);
    vt.proto_type              = std::make_unique<ffi::Prototype>();
    vt.proto_type->return_type = c_type_to_type(returnType);


    int numArgs = clang_getNumArgTypes(base_cType);
    for (int i = 0; i < numArgs; ++i) {
      CXType         argType   = clang_getArgType(base_cType, i);
      ffi::Type      ty        = c_type_to_type(argType);
      ffi::EPassMode pass_mode = ffi::type_to_passMode(ty);
      vt.proto_type->params.emplace_back(pass_mode, std::move(ty), clang_isRestrictQualifiedType(argType));
    }
  }


  if (base_cType.kind == CXType_Record || base_cType.kind == CXType_Enum) {
    CXCursor decl = clang_getTypeDeclaration(cType);

    if (clang_getCursorKind(decl) == CXCursor_StructDecl) {
      vt.base_type = ffi::EType::_comp;
      vt.comp_type = std::make_unique<ffi::Comp>(c_struct_to_comp(decl));
    } else if (clang_getCursorKind(decl) == CXCursor_UnionDecl) {
      vt.base_type  = ffi::EType::_union;
      vt.union_type = std::make_unique<ffi::Union>(c_union_to_union(decl));
    } else if (clang_getCursorKind(decl) == CXCursor_EnumDecl) {
      vt.base_type = ffi::EType::_flag;
      vt.flag_type = std::make_unique<ffi::Flag>(c_enum_to_flag(decl));
    }

    CXString name        = clang_getCursorSpelling(decl);
    vt.complex_type_name = clang_getCString(name);
    clang_disposeString(name);
  }

  return vt;
}

ffi::Comp ffi::c::c_struct_to_comp(CXCursor cCur)
{
  ffi::Comp comp;
  comp.name = clang_getCString(clang_getCursorSpelling(cCur));
  clang_visitChildren(
      cCur,
      [](CXCursor cur, CXCursor parent, CXClientData client_data) {
        auto* comp_ptr = static_cast<ffi::Comp*>(client_data);
        if (clang_getCursorKind(cur) == CXCursor_FieldDecl) {
          ffi::Type   t    = c_type_to_type(clang_getCursorType(cur));
          std::string name = clang_getCString(clang_getCursorSpelling(cur));
          comp_ptr->fields.emplace_back(name, std::move(t));
        }
        return CXChildVisit_Continue;
      },
      &comp);
  return comp;
}

ffi::Union ffi::c::c_union_to_union(CXCursor cCur)
{
  ffi::Union u;
  u.name = clang_getCString(clang_getCursorSpelling(cCur));
  clang_visitChildren(
      cCur,
      [](CXCursor cur, CXCursor parent, CXClientData client_data) {
        auto* u_ptr = static_cast<ffi::Union*>(client_data);
        if (clang_getCursorKind(cur) == CXCursor_FieldDecl) {
          ffi::Type   t    = c_type_to_type(clang_getCursorType(cur));
          std::string name = clang_getCString(clang_getCursorSpelling(cur));
          u_ptr->members.emplace_back(name, std::move(t));
        }
        return CXChildVisit_Continue;
      },
      &u);
  return u;
}

ffi::Flag ffi::c::c_enum_to_flag(CXCursor cCur)
{
  ffi::Flag e;
  e.name = clang_getCString(clang_getCursorSpelling(cCur));
  clang_visitChildren(
      cCur,
      [](CXCursor cur, CXCursor parent, CXClientData client_data) {
        auto* e_ptr = static_cast<ffi::Flag*>(client_data);
        if (clang_getCursorKind(cur) == CXCursor_EnumConstantDecl) {
          std::string        name       = clang_getCString(clang_getCursorSpelling(cur));
          CXType             t          = clang_getEnumDeclIntegerType(clang_getCursorSemanticParent(cur));
          ffi::Type          underlying = c_type_to_type(t);
          unsigned long long val        = clang_getEnumConstantDeclValue(cur);
          e_ptr->underlying_type        = underlying.base_type;
          e_ptr->members.emplace_back(name, static_cast<size_t>(val));
        }
        return CXChildVisit_Continue;
      },
      &e);
  return e;
}

ffi::Global ffi::c::c_global_to_global(CXCursor cCur)
{
  ffi::Global g;
  g.name     = clang_getCString(clang_getCursorSpelling(cCur));
  g.type     = c_type_to_type(clang_getCursorType(cCur));
  g.is_const = clang_isConstQualifiedType(clang_getCursorType(cCur));
  return g;
}

ffi::Func ffi::c::c_function_to_func(CXCursor cCur)
{
  ffi::Func f;
  f.name              = clang_getCString(clang_getCursorSpelling(cCur));
  CXType cType        = clang_getCursorType(cCur);
  f.proto.return_type = c_type_to_type(clang_getResultType(cType));

  int nargs = clang_Cursor_getNumArguments(cCur);
  for (int i = 0; i < nargs; ++i) {
    CXCursor       paramCur    = clang_Cursor_getArgument(cCur, i);
    ffi::Type      ty          = c_type_to_type(clang_getCursorType(paramCur));
    std::string    name        = clang_getCString(clang_getCursorSpelling(paramCur));
    bool           is_restrict = clang_isRestrictQualifiedType(clang_getCursorType(paramCur));
    ffi::EPassMode pass_mode   = ffi::type_to_passMode(ty);
    f.proto.params.emplace_back(pass_mode, std::move(ty), is_restrict);
    f.param_names.push_back(name);
  }

  f.proto.is_variadic = clang_isFunctionTypeVariadic(cType);
  return f;
}
