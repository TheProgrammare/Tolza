#include "compiler_context.hpp"

#include <set>
#include <string>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;


const std::string& common::CompCtx::get_preprocess_dir() const
{
  static auto out = (fs::path(codegen_build_dir) / "preprocess").string();
  return out;
}

const std::string& common::CompCtx::get_debug_graph_dir() const
{
  static auto out = (fs::path(codegen_build_dir) / "graph").string();
  return out;
}

const std::string& common::CompCtx::get_llvmir_dir() const
{
  static auto out = (fs::path(codegen_build_dir) / "llvm-ir").string();
  return out;
}

const std::string& common::CompCtx::get_project_name() const
{
  static auto out = fs::path(project_dir).stem().string();
  return out;
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

  if (target_config.empty() || target_config == "self") return out = current_config_file;
  if (auto find = sub_configs.find(target_config); find != sub_configs.end()) return out = find->second;

  return out = current_config_file;
}

void common::CompCtx::apply_args(int _argc, const char* _argv[])
{
  argc = _argc;
  argv = _argv;

  for (int i = 2; i < argc; ++i) {
    const std::string& arg = argv[i];

    // e.g. --debug
    auto bool_arg = [&](bool& input, const std::string& arg_name, const std::string& alt_arg_name = "") {
      if (arg_name.empty() && alt_arg_name.empty()) {
        input = false;
        return false;
      }

      bool valid_arg = false;

      if (!arg_name.empty() && arg.rfind("--" + arg_name, 0) == 0) {
        valid_arg                  = true;
        COMPILATION_ARGS[arg_name] = "true";
        input                      = true;
      } else if (!arg_name.empty() && arg.rfind("--!" + arg_name, 0) == 0) {
        valid_arg                  = true;
        COMPILATION_ARGS[arg_name] = "false";
        input                      = false;
      }
      if (!alt_arg_name.empty() && arg.rfind("-" + alt_arg_name, 0) == 0) {
        valid_arg                      = true;
        COMPILATION_ARGS[alt_arg_name] = "true";
        input                          = true;
      } else if (!alt_arg_name.empty() && arg.rfind("-!" + alt_arg_name, 0) == 0) {
        valid_arg                      = true;
        COMPILATION_ARGS[alt_arg_name] = "false";
        input                          = false;
      }

      return valid_arg;
    };

    // e.g. --os="linux"
    auto str_arg = [&](std::string& input, const std::string& arg_name, const std::string& alt_arg_name = "") {
      if (arg_name.empty() && alt_arg_name.empty()) {
        return false;
      }

      bool valid_arg = false;

      if (!arg_name.empty() && arg.rfind("--" + arg_name + "=", 0) == 0) {
        std::size_t pos = arg.find('=');
        if (pos != std::string::npos) {
          valid_arg                  = true;
          std::string value          = arg.substr(pos + 1);
          COMPILATION_ARGS[arg_name] = value;
          input                      = value;
        }
      }
      if (!alt_arg_name.empty() && arg.rfind("-" + alt_arg_name, 0) == 0) {
        valid_arg                      = true;
        std::string value              = arg.substr(1 + alt_arg_name.size()); // tout après "-o"
        COMPILATION_ARGS[alt_arg_name] = value;
        input                          = value;
      }

      return valid_arg;
    };

    // e.g. --dest="/mnt/data/my_project"
    auto path_arg = [&](std::string& input, const std::string& arg_name, const std::string& alt_arg_name = "") {
      if (arg_name.empty() && alt_arg_name.empty()) {
        return false;
      }

      bool valid_arg = false;

      if (!arg_name.empty() && arg.rfind("--" + arg_name + "=", 0) == 0) {
        std::size_t pos = arg.find('=');
        if (pos != std::string::npos) {
          valid_arg                  = true;
          std::string value          = arg.substr(pos + 1);
          COMPILATION_ARGS[arg_name] = value;
          input                      = fs::weakly_canonical(value);
        }
      }
      if (!alt_arg_name.empty() && arg.rfind("-" + alt_arg_name, 0) == 0) {
        valid_arg                      = true;
        std::string value              = arg.substr(1 + alt_arg_name.size());
        COMPILATION_ARGS[alt_arg_name] = value;
        input                          = fs::weakly_canonical(value);
      }

      return valid_arg;
    };

    // e.g. --opt-level=0
    auto size_arg = [&](size_t& input, const std::string& arg_name, const std::string& alt_arg_name = "") {
      if (arg_name.empty() && alt_arg_name.empty()) {
        return false;
      }

      bool valid_arg = false;

      if (!arg_name.empty() && arg.rfind("--" + arg_name + "=", 0) == 0) {
        std::size_t pos = arg.find('=');
        if (pos != std::string::npos) {
          std::string value          = arg.substr(pos + 1);
          COMPILATION_ARGS[arg_name] = value;
          valid_arg                  = true;
          input                      = std::stoul(value);
        }
      }
      if (!alt_arg_name.empty() && arg.rfind("-" + alt_arg_name, 0) == 0) {
        std::string value              = arg.substr(1 + alt_arg_name.size());
        COMPILATION_ARGS[alt_arg_name] = value;
        valid_arg                      = true;
        if (value == "s" || value == "S") {
          profile_size_opt = true;
          return valid_arg;
        } else if (value == "z" || value == "Z") {
          profile_extrem_size_opt = true;
          return valid_arg;
        } else {
          try {
            input = std::stoul(value);
          } catch (const std::runtime_error& e) {
            input = 0;
          }
        }
      }

      return valid_arg;
    };

    if (path_arg(current_config_file, "ccf")) continue;

    // target
    if (str_arg(target_abi, "abi")) continue;
    if (str_arg(target_arch, "arch")) continue;
    if (size_arg(target_bits, "bits")) continue;
    if (str_arg(target_os, "os")) continue;
    if (str_arg(target_libc, "libc")) continue;
    if (str_arg(target_config, "config")) continue;

    // profile
    if (bool_arg(profile_debug, "debug", "d")) continue;
    if (bool_arg(profile_debug, "release", "r")) {
      profile_debug = false;
      continue;
    }
    if (size_arg(profile_opt_level, "opt-level")) continue;
    if (bool_arg(profile_size_opt, "size-opt", "Os")) continue;
    if (bool_arg(profile_extrem_size_opt, "extrem-size-opt", "Oz")) continue;
    if (size_arg(profile_opt_level, "", "O")) continue;

    // logs
    if (bool_arg(log_all, "log-all", "lall")) continue;
    if (bool_arg(log_filesystem, "log-filesystem", "lfs")) continue;
    if (bool_arg(log_lexer, "log-lexer", "llex")) continue;
    if (bool_arg(log_preprocessor, "log-pre", "lpre")) continue;
    if (bool_arg(log_parser, "log-parser", "lpar")) continue;
    if (bool_arg(log_binder, "log-binder", "lemb")) continue;
    if (bool_arg(log_exporter, "log-exporter", "lexp")) continue;
    if (bool_arg(log_resolver, "log-resolver", "lres")) continue;
    if (bool_arg(log_LLVM_IR, "log-llvm", "lllvm")) continue;
    if (bool_arg(log_linker, "log-linker", "llink")) continue;

    // warnings
    if (bool_arg(warn_all, "warn-all", "wall")) continue;
    if (bool_arg(warn_extra, "warn-extra", "wextra")) continue;
    if (bool_arg(warn_pedantic, "warn-pedantic", "wpedan")) continue;
    if (size_arg(warn_level, "warn-level")) continue;
    if (bool_arg(warn_unused, "warn-unused", "wun")) continue;
    if (bool_arg(warn_dead_code, "warn-dead-code", "wdc")) continue;
    if (bool_arg(warn_as_error, "warn-as-error", "wae")) continue;

    // printer
    if (bool_arg(print_ast, "print-ast")) continue;

    // define macro
    if (arg.rfind("-D", 0) == 0) {
      auto def    = arg.substr(2);
      auto eq_pos = def.find('=');

      std::string name, value;

      if (eq_pos != std::string::npos) {
        name  = def.substr(0, eq_pos);
        value = def.substr(eq_pos + 1);
      } else {
        name  = def;
        value = "1"; // implicit value
      }
      COMPILATION_ARGS[name] = value;
      defines[name]          = value;

      continue;
    }

    if (arg.rfind("-!D", 0) == 0) {
      auto name = arg.substr(3);
      COMPILATION_ARGS.erase(name);
      defines.erase(name);
    }

    // undefine macro
    if (arg.rfind("-U", 0) == 0) {
      auto name = arg.substr(2);

      COMPILATION_ARGS[name] = ""; // no value for -UName
      undefines.push_back(name);
      continue;
    }

    if (arg.rfind("-!U", 0) == 0) {
      auto name = arg.substr(3);
      COMPILATION_ARGS.erase(name);

      for (auto it = undefines.begin(); it != undefines.end(); it++) {
        if (*it == name) {
          undefines.erase(it);
          break;
        }
      }
    }

    // codegen
    if (bool_arg(emit_bin, "emit-bin")) continue;
    if (bool_arg(emit_llvm, "emit-llvm")) continue;
    if (bool_arg(emit_obj, "emit-obj")) continue;
    if (bool_arg(emit_asm, "emit-asm")) continue;
    if (bool_arg(emit_bc, "emit-bc")) continue;
    if (bool_arg(emit_static_lib, "emit-static-lib")) continue;
    if (bool_arg(emit_dynamic_lib, "emit-dynamic-lib")) continue;

    if (path_arg(codegen_build_dir, "build")) continue;

    // project
    if (path_arg(project_dir, "project")) continue;
    if (path_arg(source_dir, "src")) continue;
    if (path_arg(vendor_dir, "vendor")) continue;
    if (path_arg(ffi_json_dir, "ffi-json")) continue;
    if (path_arg(binding_dir, "binding")) continue;

    if (arg == "--") {
      llvm_argc = argc - i - 1;
      std::vector<const char*> c_str_vec;
      c_str_vec.reserve(llvm_argc);
      for (int j = i + 1; j < argc; j++) {
        c_str_vec.push_back(argv[j]);
      }
      llvm_argv = c_str_vec.data();
      break;
    }

    std::cerr << "[velox-compiler:warning] Unknown argument '" << arg << "'" << std::endl;
  }
}

std::vector<std::string> common::CompCtx::to_args() const
{
  std::vector<std::string> out;

  out.push_back("--abi=" + target_abi);
  out.push_back("--arch=" + target_arch);
  out.push_back("--bits=" + std::to_string(target_bits));
  out.push_back("--os=" + target_os);
  out.push_back("--libc=\"" + target_libc + "\"");
  out.push_back("--config=\"" + target_config + "\"");

  if (profile_debug)
    out.push_back("--debug");
  else
    out.push_back("--release");

  out.push_back("--opt-level=" + std::to_string(profile_opt_level));

  if (profile_debug) out.push_back("--size-opt");

  if (log_all) out.push_back("--log-all");
  if (log_filesystem) out.push_back("--log-filesystem");
  if (log_lexer) out.push_back("--log-lexer");
  if (log_preprocessor) out.push_back("--log-pre");
  if (log_parser) out.push_back("--log-parser");
  if (log_binder) out.push_back("--log-binder");
  if (log_exporter) out.push_back("--log-exporter");
  if (log_resolver) out.push_back("--log-resolver");
  if (log_LLVM_IR) out.push_back("--log-llvm");
  if (log_linker) out.push_back("--log-linker");

  if (warn_all) out.push_back("--warn-all");
  if (warn_extra) out.push_back("--warn-extra");
  if (warn_pedantic) out.push_back("--warn-pedantic");
  out.push_back("--warn-level=" + std::to_string(warn_level));
  if (warn_unused) out.push_back("--warn-unused");
  if (warn_dead_code) out.push_back("--warn-dead-code");
  if (warn_as_error) out.push_back("--warn-as-error");

  if (print_ast) out.push_back("--print-ast");

  for (auto [name, val] : defines) out.push_back("-D" + name + "=" + val);

  for (auto udef : undefines) out.push_back("-U" + udef);


  if (emit_bin) out.push_back("--emit-bin");
  if (emit_llvm) out.push_back("--emit-llvm");
  if (emit_obj) out.push_back("--emit-obj");
  if (emit_asm) out.push_back("--emit-asm");
  if (emit_bc) out.push_back("--emit-bc");
  if (emit_static_lib) out.push_back("--emit-static-lib");
  if (emit_dynamic_lib) out.push_back("--emit-dynamic-lib");
  out.push_back("--build=\"" + codegen_build_dir + "\"");

  out.push_back("--project=\"" + project_dir + "\"");
  out.push_back("--src=\"" + source_dir + "\"");
  out.push_back("--vendor=\"" + vendor_dir + "\"");
  out.push_back("--ffi-json=\"" + ffi_json_dir + "\"");
  out.push_back("--binding=\"" + binding_dir + "\"");
  out.push_back("--ccf=\"" + current_config_file + "\"");

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

  CompCtx out = base_ctx;

  apply_str(out.current_config_file, current_config_file);

  merge_map(out.COMPILATION_ARGS, COMPILATION_ARGS, COMPILATION_ARGS_merge_mode);

  // profile
  apply_bool(out.profile_debug, profile_debug, has_profile_debug);
  apply_int(out.profile_opt_level, profile_opt_level, has_profile_opt_level);
  apply_bool(out.profile_size_opt, profile_size_opt, has_profile_size_opt);
  apply_bool(out.profile_extrem_size_opt, profile_extrem_size_opt, has_profile_extrem_size_opt);

  // logs
  apply_bool(out.log_all, log_all, has_log_all);
  apply_bool(out.log_filesystem, log_filesystem, has_log_filesystem);
  apply_bool(out.log_lexer, log_lexer, has_log_lexer);
  apply_bool(out.log_preprocessor, log_preprocessor, has_log_preprocessor);
  apply_bool(out.log_parser, log_parser, has_log_parser);
  apply_bool(out.log_binder, log_binder, has_log_binder);
  apply_bool(out.log_exporter, log_exporter, has_log_exporter);
  apply_bool(out.log_resolver, log_resolver, has_log_resolver);
  apply_bool(out.log_LLVM_IR, log_LLVM_IR, has_log_LLVM_IR);
  apply_bool(out.log_linker, log_linker, has_log_linker);

  // warnings
  apply_bool(out.warn_all, warn_all, has_warn_all);
  apply_bool(out.warn_extra, warn_extra, has_warn_extra);
  apply_bool(out.warn_pedantic, warn_pedantic, has_warn_pedantic);
  apply_int(out.warn_level, warn_level, has_warn_level);
  apply_bool(out.warn_unused, warn_unused, has_warn_unused);
  apply_bool(out.warn_dead_code, warn_dead_code, has_warn_dead_code);
  apply_bool(out.warn_as_error, warn_as_error, has_warn_as_error);

  // dot
  apply_bool(out.print_ast, print_ast, has_print_ast);

  // defines
  merge_map(out.defines, defines, defines_merge_mode);

  // undefines
  merge_list(out.undefines, undefines, undefines_merge_mode);

  // codegen
  apply_bool(out.emit_bin, emit_bin, has_emit_bin);
  apply_bool(out.emit_llvm, emit_llvm, has_emit_llvm);
  apply_bool(out.emit_obj, emit_obj, has_emit_obj);
  apply_bool(out.emit_asm, emit_asm, has_emit_asm);
  apply_bool(out.emit_bc, emit_bc, has_emit_bc);
  apply_bool(out.emit_static_lib, emit_static_lib, has_emit_static_lib);
  apply_bool(out.emit_dynamic_lib, emit_dynamic_lib, has_emit_dynamic_lib);
  apply_bool(out.pic_mode, pic_mode, has_pic_mode);
  apply_str(out.codegen_build_dir, codegen_build_dir);

  // project
  apply_str(out.project_dir, project_dir);
  apply_str(out.source_dir, source_dir);
  apply_str(out.vendor_dir, vendor_dir);
  apply_str(out.ffi_json_dir, ffi_json_dir);
  apply_str(out.binding_dir, binding_dir);

  return out;
}
