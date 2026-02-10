#include "ScriptInfo.hpp"
#include "AST/AST_Base.hpp"
#include "AST/AST_Literal.hpp"
#include "AST/AST_Reference.hpp"

void ScriptInfo::add_export(const ModuleExportation &exp)
{
    exported_mod.push_back(std::make_unique<ModuleExportation>(exp));
}

void ScriptInfo::add_import(const ModuleImportation &imp)
{
    imported_mod.push_back(std::make_unique<ModuleImportation>(imp));
}

ModuleExportation *ScriptInfo::get_export_module(const std::string &name)
{
    for (const auto &exp : exported_mod) {
        if (exp->name == name) return exp.get();
    }

    return nullptr;
}

ModuleImportation *ScriptInfo::get_import_module(const std::string &name)
{
    for (const auto &imp : imported_mod) {
        if (imp->name == name) return imp.get();
    }

    return nullptr;
}

std::set<ModuleImportation*> ScriptInfo::get_externs()
{
    std::set<ModuleImportation*> result;
    for (auto &imp : imported_mod) {
        if (imp->is_external()) {
            result.insert(imp.get());
        }
    }
    return result;
}

std::set<std::string> ScriptInfo::get_extern_languages()
{
    std::set<std::string> result;
    for (auto &imp : get_externs()) {
        if (imp->is_external()) {
            result.insert(imp->name);
        }
    }
    return result;
}


Extern_Item::Kind AST_AReference_to_Extern_Item_Kind(const AST::AReference &n) 
{
    if (dynamic_cast<const AST::Reference::Enum*>(&n))          return Extern_Item::Kind::Enum;
    if (dynamic_cast<const AST::Reference::Call*>(&n))          return Extern_Item::Kind::Function;
    if (dynamic_cast<const AST::Reference::Call_Pipe*>(&n))     return Extern_Item::Kind::Function;
    if (dynamic_cast<const AST::AType_Reference*>(&n)) {
        if (dynamic_cast<const AST::Literal::Component*>(&n))   return Extern_Item::Kind::Component;
        if (dynamic_cast<const AST::Literal::Entity*>(&n))      return Extern_Item::Kind::Entity;
        return Extern_Item::Kind::Type;
    }

    return Extern_Item::Kind::Global;
}