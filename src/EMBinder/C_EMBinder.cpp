
#include "C_EMBinder.hpp"

#include <cstring>
#include <fstream>
#include <iostream>
#include <assert.h>
#include <set>
#include <filesystem>

#include "Globals.hpp"
#include "ScriptInfo.hpp"
#include "Compilation.hpp"


EMBinder_LibC::EMBinder_LibC(
    const std::string &lang
    , const std::string &lib
    , std::ofstream &os
    , std::unordered_map<std::string, EExternItem> &items_to_generate)
    : target_language(lang)
    , target_lib(lib)
    , os_(os) 
{
    for (auto& [name, type] : items_to_generate) {
        switch (type)
        {
        case EExternItem::Function:
            functions_to_generate.insert(name);
            break;
        case EExternItem::Global:
            global_to_generate.insert(name);
            break;
        case EExternItem::Type:
            types_to_generate.insert(name);
            break;
        default:
            // ignore, no C compatible
            break;
        }
    }
}

int EMBinder_LibC::c_lib_to_velox_lib()
{
    current_bind = this;

    std::string tmp_file = "tmp_include.c";
    {
        std::ofstream ofs(tmp_file);
        ofs << "#include <" << target_lib << ".h>\n";
    }

    CVeloxAST ast = parse_translation_unit(tmp_file, {});

    if (!ast.enums.empty()) {
        os_ << EMBINDER_ENUM_HEADER;

        for (auto& elem : ast.enums) {
            os_ << flag_to_str(elem);
        }
    }
    if (!ast.comps.empty()) {
        os_ << EMBINDER_COMP_HEADER;

        for (auto& elem : ast.comps) {
            os_ << comp_to_str(elem);
        }
    }
    if (!ast.unions.empty()) {
        os_ << EMBINDER_UNION_HEADER;

        for (auto& elem : ast.unions) {
            os_ << union_to_str(elem);
        }
    }
    if (!ast.globals.empty()) {
        os_ << EMBINDER_GLOBAL_HEADER;

        for (auto& elem : ast.globals) {
            os_ << global_to_str(elem);
        }
    }
    if (!ast.funcs.empty()) {
        os_ << EMBINDER_FUNCTION_HEADER;

        for (auto& elem : ast.funcs) {
            os_ << func_to_str(elem);
        }
    }

    std::filesystem::remove(tmp_file);

    return 0;
}

CXChildVisitResult universal_visitor(CXCursor cursor, CXCursor parent, CXClientData client_data) {
    CVeloxAST* ast = static_cast<CVeloxAST*>(client_data);
    CXCursorKind kind = clang_getCursorKind(cursor);
    
    // no interop allowed if internal
    CXLinkageKind linkage = clang_getCursorLinkage(cursor);
    if (linkage == CXLinkage_Internal)
        return CXChildVisit_Recurse;

    std::set<std::string> &ty_names = current_bind->types_to_generate;
    std::set<std::string> &fn_names = current_bind->functions_to_generate;
    std::set<std::string> &gl_names = current_bind->global_to_generate;

    switch (kind) {
    case CXCursor_StructDecl: {
        std::string name = clang_getCString(clang_getCursorSpelling(cursor));
        if (!ty_names.contains(name)) break;

        if (!clang_isCursorDefinition(cursor)) break; // ignorer forward declaration
        CVeloxComp comp = c_struct_to_velox_comp(cursor);
        ast->comps.push_back(std::move(comp));
        break;
    }

    case CXCursor_UnionDecl: {
        std::string name = clang_getCString(clang_getCursorSpelling(cursor));
        if (!ty_names.contains(name)) break;

        if (!clang_isCursorDefinition(cursor)) break;
        CVeloxUnion u = c_union_to_velox_union(cursor);
        ast->unions.push_back(std::move(u));
        break;
    }

    case CXCursor_EnumDecl: {
        std::string name = clang_getCString(clang_getCursorSpelling(cursor));
        if (!ty_names.contains(name)) break;

        if (!clang_isCursorDefinition(cursor)) break;
        CVeloxFlag e = c_enum_to_velox_flag(cursor);
        ast->enums.push_back(std::move(e));
        break;
    }

    case CXCursor_FunctionDecl: {
        std::string name = clang_getCString(clang_getCursorSpelling(cursor));
        if (!fn_names.contains(name)) break;

        CVeloxFunc f = c_function_to_velox_function(cursor);
        ast->funcs.push_back(std::move(f));
        break;
    }

    case CXCursor_VarDecl: {
        std::string name = clang_getCString(clang_getCursorSpelling(cursor));
        if (!fn_names.contains(name)) break;

        CVeloxGlobal g = c_global_to_velox_global(cursor);
        ast->globals.push_back(std::move(g));
        break;
    }

    default:
        break;
    }

    return CXChildVisit_Recurse; // continuer récursivement
}

// --- Fonction principale pour parser un fichier C ---
CVeloxAST parse_translation_unit(const std::string& filename, const std::vector<std::string>& args = {}) {
    CXIndex index = clang_createIndex(0, 0);

    // Convertir args en format char*[]
    std::vector<const char*> cargs;
    for (const auto& s : args) cargs.push_back(s.c_str());

    CXTranslationUnit tu;
    CXErrorCode error = clang_parseTranslationUnit2(
        index,
        filename.c_str(),
        cargs.data(),
        static_cast<int>(cargs.size()),
        nullptr,
        0,
        CXTranslationUnit_None,
        &tu
    );

    assert(error == CXError_Success && "Failed to parse translation unit");

    CXCursor rootCursor = clang_getTranslationUnitCursor(tu);

    CVeloxAST ast;
    clang_visitChildren(rootCursor, universal_visitor, &ast);

    clang_disposeTranslationUnit(tu);
    clang_disposeIndex(index);

    return ast;
}


EVeloxTypeFromC c_type_base_to_velox_type_base(CXType cType, CXType &out_base_cType) {
    switch (cType.kind) {
    case CXType_SChar:              out_base_cType = cType; return EVeloxTypeFromC::_schar;
    case CXType_Short:              out_base_cType = cType; return EVeloxTypeFromC::_short;
    case CXType_Int:                out_base_cType = cType; return EVeloxTypeFromC::_int;
    case CXType_LongLong:           out_base_cType = cType; return EVeloxTypeFromC::_longlong;
    case CXType_Long:               out_base_cType = cType; return EVeloxTypeFromC::_long; // target dependant
    case CXType_UChar:              out_base_cType = cType; return EVeloxTypeFromC::_uchar;
    case CXType_UShort:             out_base_cType = cType; return EVeloxTypeFromC::_ushort;
    case CXType_UInt:               out_base_cType = cType; return EVeloxTypeFromC::_uint;
    case CXType_ULong:              out_base_cType = cType; return EVeloxTypeFromC::_ulong; // target dependant
    case CXType_ULongLong:          out_base_cType = cType; return EVeloxTypeFromC::_ulonglong;
    case CXType_Float:              out_base_cType = cType; return EVeloxTypeFromC::_float;
    case CXType_Double:             out_base_cType = cType; return EVeloxTypeFromC::_double;
    case CXType_LongDouble:         out_base_cType = cType; return EVeloxTypeFromC::_longdouble;
    case CXType_Bool:               out_base_cType = cType; return EVeloxTypeFromC::_bool;
    case CXType_Void:               out_base_cType = cType; return EVeloxTypeFromC::_void;
    case CXType_Pointer: {
        CXType pointee_type = clang_getPointeeType(cType);
        return c_type_base_to_velox_type_base(pointee_type, out_base_cType);
    }
    case CXType_Record:             out_base_cType = cType; return EVeloxTypeFromC::struct_comp;
    case CXType_Enum:               out_base_cType = cType; return EVeloxTypeFromC::enum_flag;
    case CXType_IncompleteArray:
    case CXType_ConstantArray: {
        CXType pointee_type = clang_getPointeeType(cType);
        return c_type_base_to_velox_type_base(pointee_type, out_base_cType);
    }
    case CXType_FunctionProto:
    case CXType_FunctionNoProto:    out_base_cType = cType; return EVeloxTypeFromC::func;
    default:                        out_base_cType = cType; return EVeloxTypeFromC::alias; // typedef / inconnu
    }
}


CVeloxType c_type_to_velox_type(CXType cType) {
    CVeloxType vt;
    CXType base_cType;
    // --- Type de base ---
    vt.val_type = c_type_base_to_velox_type_base(cType, base_cType);

    // --- Const / Volatile ---
    vt.is_pointer = cType.kind == CXType_Pointer;
    vt.is_table = 
        cType.kind == CXType_ConstantArray 
        || cType.kind == CXType_IncompleteArray; 

    vt.is_val_type_const = clang_isConstQualifiedType(base_cType);
    vt.is_val_type_volatile = clang_isVolatileQualifiedType(base_cType);

    if (clang_isConstQualifiedType(cType)) {
        if (vt.is_pointer) vt.is_pointer_const = true;
        else vt.is_val_type_const = true;
    }
    if (clang_isVolatileQualifiedType(cType)) {
        if (vt.is_pointer) vt.is_pointer_volatile = true;
        else vt.is_val_type_volatile = true;
    }

    // --- Atomic ---
    if (cType.kind == CXType_Atomic) {
        vt.is_atomic = true;
    }

    // --- Pointeurs (level one) ---
    if (vt.is_pointer) {
        CXType pointee = clang_getPointeeType(cType);
        vt.is_pointer_double = pointee.kind == CXType_Pointer;

        vt.is_pointer_const = clang_isConstQualifiedType(cType);
        vt.is_pointer_volatile = clang_isVolatileQualifiedType(cType);

        vt.is_pointer_on_table = 
            base_cType.kind == CXType_ConstantArray
            || base_cType.kind == CXType_IncompleteArray;
    }
    // --- Table (level one) de pointeurs ---
    else if (vt.is_table) {
        CXType element_type = clang_getArrayElementType(cType);
        vt.is_table_of_pointers = element_type.kind = CXType_Pointer;
    }

    // --- Tableaux (base) ---
    if (base_cType.kind == CXType_ConstantArray) {
        vt.is_table = true;
        vt.table_size.push_back(static_cast<size_t>(clang_getArraySize(cType)));
    } else if (base_cType.kind == CXType_IncompleteArray) {
        vt.is_table = true; // flexible
    }

    // --- Fonction ---
    if (base_cType.kind == CXType_FunctionProto || base_cType.kind == CXType_FunctionNoProto) {
        // Récupère le type de retour
        CXType returnType = clang_getResultType(base_cType);
        vt.func_type = std::make_unique<CVeloxFuncType>();
        vt.func_type->return_type = c_type_to_velox_type(returnType);

        // Récupère les paramètres
        int numArgs = clang_getNumArgTypes(base_cType);
        for (int i = 0; i < numArgs; ++i) {
            CXType argType = clang_getArgType(base_cType, i);
            vt.func_type->params.push_back({ c_type_to_velox_type(argType), clang_isRestrictQualifiedType(argType) });
        }
    }

    // --- CXType_Record(Structures or Unions) / Enums ---
    if (base_cType.kind == CXType_Record || base_cType.kind == CXType_Enum) {
        CXCursor decl = clang_getTypeDeclaration(cType);
        
        if (clang_getCursorKind(decl) == CXCursor_StructDecl) {
            vt.val_type = EVeloxTypeFromC::struct_comp;
            vt.comp_type = std::make_unique<CVeloxComp>(c_struct_to_velox_comp(decl));
        }
        else if (clang_getCursorKind(decl) == CXCursor_UnionDecl) { 
            vt.val_type = EVeloxTypeFromC::_union;
            vt.union_type = std::make_unique<CVeloxUnion>(c_union_to_velox_union(decl));
        }
        else if (clang_getCursorKind(decl) == CXCursor_EnumDecl) {
            vt.val_type = EVeloxTypeFromC::enum_flag;
            vt.flag_type = std::make_unique<CVeloxFlag>(c_enum_to_velox_flag(decl));
        }
        
        CXString name = clang_getCursorSpelling(decl);
        vt.complex_type_name = clang_getCString(name);
        clang_disposeString(name);
    }

    return vt;
}

EVeloxParamPassMode type_to_passMode(CVeloxType &cVel)
{
    if (!cVel.is_pointer 
        && cVel.val_type != EVeloxTypeFromC::struct_comp
        && cVel.val_type != EVeloxTypeFromC::func
        && cVel.val_type != EVeloxTypeFromC::_union)
        return EVeloxParamPassMode::copy;
    
    if (cVel.is_pointer
        && cVel.is_val_type_const)
        return EVeloxParamPassMode::ref;
    
    if (cVel.is_pointer)
        return EVeloxParamPassMode::mut;
    
    if (cVel.is_pointer_double)
        return EVeloxParamPassMode::addr;

    return EVeloxParamPassMode::NONE;
}

std::string EVeloxParamPassMode_to_str(EVeloxParamPassMode pm)
{
    switch (pm) {
    case EVeloxParamPassMode::copy: return "copy";
    case EVeloxParamPassMode::ref: return "ref";
    case EVeloxParamPassMode::mut: return "mut";
    case EVeloxParamPassMode::move: return "move";
    case EVeloxParamPassMode::addr: return "addr";
    case EVeloxParamPassMode::NONE: return "NO PARAM PASS MODE";
    }
}

// --- Conversion d'une struct ---
CVeloxComp c_struct_to_velox_comp(CXCursor cCur) {
    CVeloxComp comp;
    comp.name = clang_getCString(clang_getCursorSpelling(cCur));
    clang_visitChildren(
        cCur,
        [](CXCursor cur, CXCursor parent, CXClientData client_data) {
            auto *comp_ptr = static_cast<CVeloxComp*>(client_data);
            if (clang_getCursorKind(cur) == CXCursor_FieldDecl) {
                CVeloxType t = c_type_to_velox_type(clang_getCursorType(cur));
                std::string name = clang_getCString(clang_getCursorSpelling(cur));
                comp_ptr->fields.emplace_back(name, std::move(t));
            }
            return CXChildVisit_Continue;
        },
        &comp
    );
    return comp;
}

// --- Conversion d'une union ---
CVeloxUnion c_union_to_velox_union(CXCursor cCur) {
    CVeloxUnion u;
    u.name = clang_getCString(clang_getCursorSpelling(cCur));
    clang_visitChildren(
        cCur,
        [](CXCursor cur, CXCursor parent, CXClientData client_data) {
            auto *u_ptr = static_cast<CVeloxUnion*>(client_data);
            if (clang_getCursorKind(cur) == CXCursor_FieldDecl) {
                CVeloxType t = c_type_to_velox_type(clang_getCursorType(cur));
                std::string name = clang_getCString(clang_getCursorSpelling(cur));
                u_ptr->members.emplace_back(name, std::move(t));
            }
            return CXChildVisit_Continue;
        },
        &u
    );
    return u;
}

// --- Conversion d'un enum ---
CVeloxFlag c_enum_to_velox_flag(CXCursor cCur) {
    CVeloxFlag e;
    e.name = clang_getCString(clang_getCursorSpelling(cCur));
    clang_visitChildren(
        cCur,
        [](CXCursor cur, CXCursor parent, CXClientData client_data) {
            auto *e_ptr = static_cast<CVeloxFlag*>(client_data);
            if (clang_getCursorKind(cur) == CXCursor_EnumConstantDecl) {
                std::string name = clang_getCString(clang_getCursorSpelling(cur));
                CXType t = clang_getEnumDeclIntegerType(clang_getCursorSemanticParent(cur));
                CVeloxType underlying = c_type_to_velox_type(t);
                unsigned long long val = clang_getEnumConstantDeclValue(cur);
                e_ptr->underlying_type = underlying.val_type;
                e_ptr->members.emplace_back(name, static_cast<size_t>(val));
            }
            return CXChildVisit_Continue;
        },
        &e
    );
    return e;
}

CVeloxGlobal c_global_to_velox_global(CXCursor cCur)
{
    CVeloxGlobal g;
    g.name = clang_getCString(clang_getCursorSpelling(cCur));
    g.type = c_type_to_velox_type(clang_getCursorType(cCur));
    g.is_const = clang_isConstQualifiedType(clang_getCursorType(cCur));
    return g;
}

// --- Conversion d'une fonction ---
CVeloxFunc c_function_to_velox_function(CXCursor cCur) {
    CVeloxFunc f;
    f.name = clang_getCString(clang_getCursorSpelling(cCur));
    CXType cType = clang_getCursorType(cCur);
    f.type.return_type = c_type_to_velox_type(clang_getResultType(cType));

    int nargs = clang_Cursor_getNumArguments(cCur);
    for (int i = 0; i < nargs; ++i) {
        CXCursor paramCur = clang_Cursor_getArgument(cCur, i);
        CVeloxType t = c_type_to_velox_type(clang_getCursorType(paramCur));
        std::string name = clang_getCString(clang_getCursorSpelling(paramCur));
        bool is_restrict = clang_isRestrictQualifiedType(clang_getCursorType(paramCur));
        f.type.params.emplace_back(std::move(t), is_restrict);
    }

    f.type.is_variadic = clang_isFunctionTypeVariadic(cType);
    return f;
}

std::string type_to_str(CVeloxType &cVel) {
    std::string ptr;
    std::string type;
    std::string table_dim;

    if (cVel.is_opaque()) return "ptr'void";
    if (cVel.is_string()) return "c_str";

    // qualifiers
    if (cVel.is_pointer) {
        if (cVel.is_pointer_const) ptr += "$";
        if (cVel.is_pointer_volatile) ptr += "!";
        ptr += "ptr'";
    }
    else if (cVel.is_pointer_double) {
        ptr = "ptr'ptr'";
    }

    if (cVel.is_table) {
        for (size_t i = 0; i < cVel.table_size.size(); i++) {
            size_t &size = cVel.table_size[i];
            table_dim += std::to_string(size);
            if (i != cVel.table_size.size() - 1)
                table_dim += ", ";
        }
        table_dim += "]";
    }

    if (cVel.is_table_of_pointers) {
        ptr += "[";
    }
    
    // base types
    switch (cVel.val_type) {
    case EVeloxTypeFromC::INVALID:          type = "INVALID VELOX TYPE FROM C"; break;
    case EVeloxTypeFromC::_schar:           type = "C::_schar"; break;
    case EVeloxTypeFromC::_short:           type = "C::_short"; break;
    case EVeloxTypeFromC::_long:            type = "C::_long"; break;
    case EVeloxTypeFromC::_longlong:        type = "C::_longlong"; break;
    case EVeloxTypeFromC::_int:             type = "C::_int"; break;

    case EVeloxTypeFromC::_uchar:           type = "C::_uchar"; break;
    case EVeloxTypeFromC::_ushort:          type = "C::_ushort"; break;
    case EVeloxTypeFromC::_ulong:           type = "C::_ulong"; break;
    case EVeloxTypeFromC::_ulonglong:       type = "C::_ulonglong"; break;
    case EVeloxTypeFromC::_uint:            type = "C::_uint"; break;

    case EVeloxTypeFromC::_size_t:          type = "C::_size"; break;
    case EVeloxTypeFromC::_ptr_diff:        type = "C::_ptr_diff"; break;


    case EVeloxTypeFromC::_float:           type = "C::_float"; break;
    case EVeloxTypeFromC::_double:          type = "C::_double"; break;
    case EVeloxTypeFromC::_longdouble:      type = "C::_longdouble"; break;

    case EVeloxTypeFromC::_bool:            type = "bool"; break;
    case EVeloxTypeFromC::_void:            type = "void"; break;

    case EVeloxTypeFromC::struct_comp:
    case EVeloxTypeFromC::_union:
    case EVeloxTypeFromC::enum_flag:            
    case EVeloxTypeFromC::alias:
        type = cVel.complex_type_name;
        break;

    case EVeloxTypeFromC::func: {
        if (cVel.func_type.get()) {
            std::string str_params;
            
            for (size_t i = 0; i < cVel.func_type->params.size(); i++) {
                auto &[type, _] = cVel.func_type->params[i]; 
                str_params += type_to_str(type);
                if (i != cVel.func_type->params.size() - 1)
                    str_params += ", ";
            }

            std::string str_return = type_to_str(cVel.func_type->return_type);

            std::string fn = EMBINDER_FN_TYPE_TEMPLATE;
            fmt_template(fn, { str_params, str_return });
            type = fn;
        }
        else {
            std::runtime_error("Undefined function type");
        }
        break;
    }            
    }

    if (cVel.is_val_type_const)     type = "$" + type;
    if (cVel.is_val_type_volatile)  type = "!" + type;

    return ptr + type + table_dim;
}

// --------------------
// CVeloxComp → comp syntax
std::string comp_to_str(CVeloxComp &cVel) {
    std::string members;

    for (size_t i = 0; i < cVel.fields.size(); i++) {
        auto &[name, type] = cVel.fields[i];
        std::string field = EMBINDER_EXTERN_FIELD;
        fmt_template(field, { name, type_to_str(type) });

        members += field;
    }

    std::string out = EMBINDER_EXTERN_COMP_TEMPLATE;
    fmt_template(out, { cVel.name, members });
    return out;
}

// --------------------
// CVeloxUnion → union syntax
std::string union_to_str(CVeloxUnion &cVel) {
    std::string members;

    for (size_t i = 0; i < cVel.members.size(); i++) {
        auto &[name, type] = cVel.members[i];
        members += name + ": " + type_to_str(type) + ",\n";
    }

    std::string out = EMBINDER_EXTERN_UNION_TEMPLATE;
    fmt_template(out, { cVel.name, members });
    return out;
}

// --------------------
// CVeloxFlag → flag syntax
std::string flag_to_str(CVeloxFlag &cVel) {
    std::string members;

    for (size_t i = 0; i < cVel.members.size(); i++) {
        auto &[name, bits] = cVel.members[i];
        members += name + ": " + std::to_string(bits) + ",\n";
    }

    std::string out = EMBINDER_EXTERN_FLAG_TEMPLATE;
    fmt_template(out, { cVel.name, members });
    return out;
}

// --------------------
// CVeloxFunc → fn syntax
std::string func_to_str(CVeloxFunc &cVel) {
    std::string params;

    for (size_t i = 0; i < cVel.param_names.size(); i++) {
        std::string name = cVel.param_names[i];
        auto &[type, is_restrict] = cVel.type.params[i];
        std::string pass_mode = EVeloxParamPassMode_to_str(type_to_passMode(type));
        params +=  pass_mode + " " + name + ": " + type_to_str(type);

        if (i != cVel.param_names.size() - 1) params += ", ";
    }

    if (cVel.type.is_variadic)
        params += "p_args: ptr'void...";

    std::string out = EMBINDER_EXTERN_FN_TEMPALTE;
    fmt_template(out, { cVel.name, params, type_to_str(cVel.type.return_type) });
    return out;
}

// --------------------
// CVeloxGlobal → var syntax
std::string global_to_str(CVeloxGlobal &cVel) {
    std::string kind = cVel.is_const ? "let" : "var";

    std::string out = EMBINDER_EXTERN_GLOBAL_TEMPLATE;
    fmt_template(out, { kind, cVel.name, type_to_str(cVel.type) });
    return out;
}