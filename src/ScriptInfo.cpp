#include "ScriptInfo.hpp"

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
