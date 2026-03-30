#include "compiler_context.hpp"
#include "common.hpp"

#include <algorithm>
#include <set>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;


size_t common::CompCtx::get_size_bit() const
{
  // on abi_size
  if (!target_size_abi.empty()) {
    std::string abi = target_size_abi;
    // case insensible
    for (auto& c : abi) c = std::toupper(c);

    if (abi == "ILP32") {
      return 32;
    } else if (abi == "LP64" || abi == "LLP64" || abi == "ILP64") {
      return 64;
    } else {
      // unknown, check arch
    }
  }

  // on target_arch
  if (!target_arch.empty()) {
    std::string arch = target_arch;
    for (auto& c : arch) c = std::tolower(c);

    if (arch == "x86" || arch == "arm") {
      return 32;
    } else if (arch == "x86_64" || arch == "amd64" || arch == "aarch64" || arch == "riscv64") {
      return 64;
    }
  }

  // fallback
  return 64;
}

std::string common::CompCtx::get_dir_project() const
{
  return resolve_path(dir_project, fs::path(current_config_file).parent_path());
}
std::string common::CompCtx::get_dir_build() const
{
  return resolve_path(dir_build, get_dir_project());
}
std::string common::CompCtx::get_dir_source() const
{
  return resolve_path(dir_source, get_dir_project());
}
std::string common::CompCtx::get_dir_vendor() const
{
  return resolve_path(dir_vendor, get_dir_project());
}
std::string common::CompCtx::get_dir_binding() const
{
  return resolve_path(dir_binding, get_dir_project());
}
std::string common::CompCtx::get_dir_ffi_json() const
{
  return resolve_path(dir_ffi_json, get_dir_project());
}
std::string common::CompCtx::get_dir_compiler() const
{
  return resolve_path(dir_compiler, get_dir_project());
}
std::string common::CompCtx::get_dir_stdlib() const
{
  return resolve_path(dir_stdlib, get_dir_project());
}
std::string common::CompCtx::get_dir_packages() const
{
  return resolve_path(dir_packages, get_dir_project());
}

const std::string& common::CompCtx::get_preprocess_dir() const
{
  static auto out = (fs::path(dir_build) / "preprocess").string();
  return out;
}

const std::string& common::CompCtx::get_debug_graph_dir() const
{
  static auto out = (fs::path(dir_build) / "graph").string();
  return out;
}

const std::string& common::CompCtx::get_llvmir_dir() const
{
  static auto out = (fs::path(dir_build) / "llvm-ir").string();
  return out;
}

std::string common::CompCtx::get_project_name() const
{
  if (target_project_name.empty()) return fs::path(dir_project).stem().string();
  return target_project_name;
}

const std::string& common::CompCtx::get_out_name() const
{
  static auto out = fs::path(current_config_file).stem().string();
  return out;
}

const std::string& common::CompCtx::get_config_file() const
{
  static std::string out;
  if (!out.empty()) return out;

  if (target_sub_config.empty() || target_sub_config == "self") return out = current_config_file;
  if (auto find = sub_configs.find(target_sub_config); find != sub_configs.end()) return out = find->second;

  return out = current_config_file;
}

std::string common::CompCtx::get_target_triple() const
{
  if (!llvm_triple.empty()) return llvm_triple;
  std::string triple;
  triple += target_arch.empty() ? "unknown" : target_arch;
  triple += "-";
  triple += target_vendor.empty() ? "unknown" : target_vendor;
  triple += "-";
  triple += target_os.empty() ? "unknown" : target_os;
  if (!target_abi.empty()) {
    triple += "-";
    triple += target_abi;
  }
  return triple;
}

void common::CompCtx::generate_preprocessor_args()
{
  auto to_std_arg = [](const std::string& _str) {
    std::string tmp = _str;
    std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
    return "__" + tmp + "__";
  };

  auto build_arg = [&](const std::string& _str, const std::string& key) {
    const std::string std_arg  = to_std_arg(_str);
    PREPROCESSOR_ARGS[key]     = std_arg;
    PREPROCESSOR_ARGS[std_arg] = "true";
  };

  PREPROCESSOR_ARGS.clear();

  PREPROCESSOR_ARGS["project_name"] = target_project_name;
  build_arg(target_arch, "arch");
  build_arg(target_os, "os");
  build_arg(target_vendor, "vendor");
  build_arg(target_abi, "abi");
  build_arg(target_size_abi, "abi_size");
  build_arg(target_libc, "libc");
  build_arg(target_cpu, "cpu");
  build_arg(target_features, "features");

  build_arg(ECodeModel_to_str(target_code_model), "code_model");
  build_arg(ERelocModel_to_str(target_reloc_model), "reloc_model");
  for (auto& emit : target_emits) {
    PREPROCESSOR_ARGS[to_std_arg(EEmit_to_str(emit))] = "true";
  }

  if (profile_debug) {
    PREPROCESSOR_ARGS["debug"] = "true";
  } else {
    PREPROCESSOR_ARGS["release"] = "true";
  }

  build_arg(EOptimization_to_str(profile_optimization), "optimization");

  for (auto& log : logs) build_arg(log, "log");

  for (auto& warn : warns) build_arg(warn, "warning");
  build_arg(EWarnLevel_to_str(warn_level), "warn_level");

  for (auto& debug : debugs) build_arg(debug, "debug");

  for (auto& [key, val] : defines) PREPROCESSOR_ARGS[key] = val;

  for (auto& undef : undefines) PREPROCESSOR_ARGS[undef] = "true";

  build_arg(llvm_triple, "llvm_triple");
}

std::vector<std::string> common::CompCtx::to_args() const
{
  std::vector<std::string> out;
  std::vector<std::string> llvm_out;

  // target
  out.push_back("--project-name=\"" + target_project_name + "\"");
  out.push_back("--arch=\"" + target_arch + "\"");
  out.push_back("--os=\"" + target_os + "\"");
  out.push_back("--vendor=\"" + target_vendor + "\"");
  out.push_back("--abi=\"" + target_abi + "\"");
  out.push_back("--abi-size=\"" + target_size_abi + "\"");
  out.push_back("--libc=\"" + target_libc + "\"");
  out.push_back("--cpu=\"" + target_cpu + "\"");
  out.push_back("--features=\"" + target_features + "\"");
  if (target_sub_config != "self") out.push_back("--sub-config=\"" + target_sub_config + "\"");
  switch (target_code_model) {
  case ECodeModel::Tiny:   out.push_back("--code-model=tiny"); break;
  case ECodeModel::Small:  out.push_back("--code-model=small"); break;
  case ECodeModel::Kernel: out.push_back("--code-model=kernel"); break;
  case ECodeModel::Medium: out.push_back("--code-model=medium"); break;
  case ECodeModel::Large:  out.push_back("--code-model=large"); break;
  }
  switch (target_reloc_model) {
  case ERelocModel::Static:       out.push_back("--reloc-model=static"); break;
  case ERelocModel::Pic:          out.push_back("--reloc-model=pic"); break;
  case ERelocModel::DynamicNoPIC: out.push_back("--reloc-model=pie"); break;
  case ERelocModel::ROPI:         out.push_back("--reloc-model=ropi"); break;
  case ERelocModel::RWPI:         out.push_back("--reloc-model=rwpi"); break;
  case ERelocModel::ROPI_RWPI:    out.push_back("--reloc-model=ropi_rwpi"); break;
  }
  for (auto& emit : target_emits) {
    switch (emit) {
    case EEmit::Bin:   out.push_back("--emit-bin"); break;
    case EEmit::LLVM:  out.push_back("--emit-llvm"); break;
    case EEmit::Obj:   out.push_back("--emit-obj"); break;
    case EEmit::ASM:   out.push_back("--emit-asm"); break;
    case EEmit::BC:    out.push_back("--emit-bc"); break;
    case EEmit::s_lib: out.push_back("--emit-s-lib"); break;
    case EEmit::d_lib: out.push_back("--emit-d-lib"); break;
    }
  }

  // debug
  if (profile_debug)
    out.push_back("--debug");
  else
    out.push_back("--release");

  switch (profile_optimization) {
  case EOptimization::O0: out.push_back("-O0"); break;
  case EOptimization::O1: out.push_back("-O1"); break;
  case EOptimization::O2: out.push_back("-O2"); break;
  case EOptimization::O3: out.push_back("-O3"); break;
  case EOptimization::Os: out.push_back("-Os"); break;
  case EOptimization::Oz: out.push_back("-Oz"); break;
  }


  for (auto& log : logs) out.push_back(common::Key_Arg_Asso::val_to_arg("log.logs", log));

  for (auto& warn : warns) out.push_back(common::Key_Arg_Asso::val_to_arg("warning.warnings", warn));


  switch (warn_level) {
  case EWarnLevel::W0: out.push_back("-W0"); break;
  case EWarnLevel::W1: out.push_back("-W1"); break;
  case EWarnLevel::W2: out.push_back("-W2"); break;
  case EWarnLevel::W3: out.push_back("-W3"); break;
  }

  for (auto& debug : debugs) out.push_back(common::Key_Arg_Asso::val_to_arg("debug.debugs", debug));


  for (auto [name, val] : defines) out.push_back("-D" + name + "=" + val);

  for (auto udef : undefines) out.push_back("-U" + udef);

  out.push_back("--dir-project=\"" + dir_project + "\"");
  out.push_back("--dir-build=\"" + dir_build + "\"");
  out.push_back("--dir-src=\"" + dir_source + "\"");
  out.push_back("--dir-vendor=\"" + dir_vendor + "\"");
  out.push_back("--dir-ffi-json=\"" + dir_ffi_json + "\"");
  out.push_back("--dir-binding=\"" + dir_binding + "\"");
  out.push_back("--dir-compiler=\"" + dir_compiler + "\"");
  out.push_back("--dir-stdlib=\"" + dir_stdlib + "\"");
  out.push_back("--dir-packages=\"" + dir_packages + "\"");

  // out.push_back("--dir-ccf=\"" + current_config_file + "\"");


  if (llvm_verify_module) out.push_back("--verify-module");

  if (!llvm_args.empty()) out.push_back("--");
  for (auto& llvm_arg : llvm_args) out.push_back(llvm_arg);

  return out;
}


common::CompCtx common::Sub_CompCtx::merge_context(const CompCtx& base_ctx) const
{
  auto apply_str = [&](std::string& _dest, const std::string& _str) {
    if (_str.empty()) return;
    _dest = _str;
  };

  auto apply_bool = [&](bool& _dest, bool _val, bool _can_apply) {
    if (!_can_apply) return;
    _dest = _val;
  };

  auto apply_int = [&](size_t& _dest, size_t _val, bool _can_apply) {
    if (!_can_apply) return;
    _dest = _val;
  };

  auto merge_list = [&](std::vector<std::string>& _dest, const std::vector<std::string>& _val, EMergeMode mode) {
    switch (mode) {
    case EMergeMode::_union: {
      _dest.insert(_dest.end(), _val.begin(), _val.end());
      break;
    }
    case EMergeMode::_intersection: {
      std::vector<std::string> tmp;
      tmp.reserve(_dest.size());
      auto tmp_set = std::set<std::string>(_val.begin(), _val.end());
      for (const auto& elem : _dest) {
        if (tmp_set.find(elem) != tmp_set.end()) tmp.emplace_back(elem);
      }
      _dest = tmp;
      break;
    }
    case EMergeMode::_anti_intersection: {
      std::vector<std::string> tmp;
      tmp.reserve(_dest.size());
      auto tmp_set = std::set<std::string>(_val.begin(), _val.end());
      for (const auto& elem : _dest) {
        if (tmp_set.find(elem) == tmp_set.end()) tmp.emplace_back(elem);
      }
      _dest = tmp;
      break;
    }
    }
  };

  auto merge_set = [&](std::set<std::string>& _dest, const std::set<std::string>& _val, EMergeMode mode) {
    switch (mode) {
    case EMergeMode::_union: {
      _dest.insert(_val.begin(), _val.end());
      break;
    }
    case EMergeMode::_intersection: {
      std::set<std::string> tmp;
      for (const auto& elem : _dest) {
        if (_val.find(elem) != _val.end()) tmp.insert(elem);
      }
      _dest = tmp;
      break;
    }
    case EMergeMode::_anti_intersection: {
      std::set<std::string> tmp;
      for (const auto& elem : _dest) {
        if (_val.find(elem) == _val.end()) tmp.insert(elem);
      }
      _dest = tmp;
      break;
    }
    }
  };

  auto merge_map = [&](std::map<std::string, std::string>& _dest, const std::map<std::string, std::string>& _val,
                       EMergeMode mode) {
    switch (mode) {
    case EMergeMode::_union: {
      for (auto& [val_key, val_val] : _val) _dest[val_key] = val_val;

      break;
    }
    case EMergeMode::_intersection: {
      std::map<std::string, std::string> tmp;
      for (const auto& [key, val] : _dest) {
        if (_val.find(key) != _val.end()) tmp[key] = val;
      }
      _dest = tmp;
      break;
    }
    case EMergeMode::_anti_intersection: {
      std::map<std::string, std::string> tmp;
      for (const auto& [key, val] : _dest) {
        if (_val.find(key) == _val.end()) tmp[key] = val;
      }
      _dest = tmp;
      break;
    }
    }
  };

  auto merge_emits = [&](std::set<EEmit>& _dest, const std::set<EEmit>& _val, EMergeMode mode) {
    switch (mode) {
    case EMergeMode::_union: {
      _dest.insert(_val.begin(), _val.end());
      break;
    }
    case EMergeMode::_intersection: {
      std::set<EEmit> tmp;
      for (auto& emit : _dest) {
        if (_val.count(emit) > 0) tmp.insert(emit);
      }
      _dest = tmp;
      break;
    }
    case EMergeMode::_anti_intersection: {
      std::set<EEmit> tmp;
      for (const auto& emit : _dest) {
        if (_val.count(emit) == 0) tmp.insert(emit);
      }
      _dest = tmp;
      break;
    }
    }
  };

  CompCtx out = base_ctx;

  apply_str(out.current_config_file, current_config_file);

  merge_map(out.PREPROCESSOR_ARGS, PREPROCESSOR_ARGS, COMPILATION_ARGS_merge_mode);

  // target
  apply_str(out.target_project_name, target_project_name);
  apply_str(out.target_arch, target_arch);
  apply_str(out.target_os, target_os);
  apply_str(out.target_abi, target_abi);
  apply_str(out.target_size_abi, target_size_abi);
  apply_str(out.target_libc, target_libc);
  apply_str(out.target_cpu, target_cpu);
  apply_str(out.target_features, target_features);

  merge_emits(out.target_emits, target_emits, emits_merge_mode);

  // profile
  apply_bool(out.profile_debug, profile_debug, has_profile_debug);
  if (has_profile_optimization) out.profile_optimization = profile_optimization;

  // logs
  merge_set(out.logs, logs, logs_merge_mode);

  // warnings
  merge_set(out.logs, logs, logs_merge_mode);
  out.warns.insert(warns.begin(), warns.end());
  if (has_warn_level) out.warn_level = warn_level;

  // debugs
  merge_set(out.debugs, debugs, debug_merge_mode);

  // defines
  merge_map(out.defines, defines, defines_merge_mode);

  // undefines
  merge_list(out.undefines, undefines, undefines_merge_mode);

  // llvm
  apply_bool(out.llvm_verify_module, llvm_verify_module, has_llvm_verify_module);
  apply_str(out.llvm_triple, llvm_triple);

  out.generate_preprocessor_args();
  return out;
}

std::string common::CompCtx::ERelocModel_to_str(ERelocModel reloc)
{
  switch (reloc) {
  case ERelocModel::Static:       return "static";
  case ERelocModel::Pic:          return "pic";
  case ERelocModel::DynamicNoPIC: return "pie";
  case ERelocModel::ROPI:         return "ropi";
  case ERelocModel::RWPI:         return "rwpi";
  case ERelocModel::ROPI_RWPI:    return "ropi_rwpi";
  }
}
std::string common::CompCtx::ECodeModel_to_str(ECodeModel code)
{
  switch (code) {
  case ECodeModel::Tiny:   return "tiny";
  case ECodeModel::Small:  return "small";
  case ECodeModel::Kernel: return "kernel";
  case ECodeModel::Medium: return "medium";
  case ECodeModel::Large:  return "large";
  }
}
std::string common::CompCtx::EOptimization_to_str(EOptimization opt)
{
  switch (opt) {
  case EOptimization::O0: return "O0";
  case EOptimization::O1: return "O1";
  case EOptimization::O2: return "O2";
  case EOptimization::O3: return "O3";
  case EOptimization::Os: return "Os";
  case EOptimization::Oz: return "Oz";
  }
}
std::string common::CompCtx::EWarnLevel_to_str(EWarnLevel wlevel)
{
  switch (wlevel) {
  case EWarnLevel::W0: return "W0";
  case EWarnLevel::W1: return "W1";
  case EWarnLevel::W2: return "W2";
  case EWarnLevel::W3: return "W3";
  }
}

std::string common::CompCtx::EEmit_to_str(EEmit emit)
{
  switch (emit) {
  case EEmit::Bin:   return "bin";
  case EEmit::LLVM:  return "llvm";
  case EEmit::Obj:   return "obj";
  case EEmit::ASM:   return "asm";
  case EEmit::BC:    return "bc";
  case EEmit::s_lib: return "s_lib";
  case EEmit::d_lib: return "d_lib";
  }
}

common::CompCtx::EEmit common::CompCtx::str_to_EEmit(const std::string& s)
{
  if (s == "bin") return EEmit::Bin;
  if (s == "llvm") return EEmit::LLVM;
  if (s == "obj") return EEmit::Obj;
  if (s == "asm") return EEmit::ASM;
  if (s == "bc") return EEmit::BC;
  if (s == "s_lib") return EEmit::s_lib;
  if (s == "d_lib") return EEmit::d_lib;

  return EEmit::Bin;
}


bool common::Key_Arg_Asso::val_is_valid(const std::string& key, const std::string& val)
{
  for (auto& elem : kas) {
    if (elem.config_key == key && elem.config_val == val) return true;
  }
  return false;
}
const std::string& common::Key_Arg_Asso::val_to_arg(const std::string& key, const std::string& val)
{
  static const std::string empty = "";
  for (auto& elem : kas) {
    if (elem.config_key == key) return elem.arg;
  }
  return empty;
}
const std::string& common::Key_Arg_Asso::arg_to_val(const std::string& arg)
{
  static const std::string empty = "";
  for (auto& elem : kas) {
    if (elem.arg == arg && elem.arg_alt == arg) return elem.config_val;
  }
  return empty;
}