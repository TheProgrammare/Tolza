#include "toolchain.hpp"
#include <filesystem>
#include <string>

std::vector<const char*> toolchain::CompCtx::to_args() const
{
  std::vector<const char*> out;
  out.push_back(fs::current_path().c_str());

  out.push_back(std::string("--abi=" + target_abi).c_str());
  out.push_back(std::string("--arch=" + target_arch).c_str());
  out.push_back(std::string("--bits=" + std::to_string(target_bits)).c_str());
  out.push_back(std::string("--os=" + target_os).c_str());
  out.push_back(std::string("--libc=" + target_libc).c_str());
  out.push_back(std::string("--config=" + target_config).c_str());
  if (profile_debug)
    out.push_back("--debug");
  else
    out.push_back("--release");
  out.push_back(std::string("--opt-level=" + std::to_string(profile_opt_level)).c_str());
  if (profile_debug) out.push_back("--size-opt");

  if (profile_debug) out.push_back("--log-all");
  if (profile_debug) out.push_back("--log-filesystem");
  if (profile_debug) out.push_back("--log-lexer");
  if (profile_debug) out.push_back("--log-pre");
  if (profile_debug) out.push_back("--log-parser");
  if (profile_debug) out.push_back("--log-binder");
  if (profile_debug) out.push_back("--log-exporter");
  if (profile_debug) out.push_back("--log-resolver");
  if (profile_debug) out.push_back("--log-llvm");
  if (profile_debug) out.push_back("--log-linker");

  if (warn_all) out.push_back("--warn-all");
  if (warn_extra) out.push_back("--warn-extra");
  if (warn_pedantic) out.push_back("--warn-pedantic");
  out.push_back(std::string("--warn-level=" + std::to_string(warn_level)).c_str());
  if (warn_unused) out.push_back("--warn-unused");
  if (warn_dead_code) out.push_back("--warn-dead-code");
  if (warn_as_error) out.push_back("--warn-as-error");

  if (dot_ast) out.push_back("--dot-ast");
  if (dot_link) out.push_back("--dot-link");

  for (auto [name, val] : defines) {
    out.push_back(std::string("-D" + name + "=" + val).c_str());
  }

  for (auto udef : undefines) {
    out.push_back(std::string("-U" + udef).c_str());
  }

  out.push_back(std::string("--emit=" + EEmitMode_to_str()).c_str());
  out.push_back(std::string("--build=" + codegen_build_dir.string()).c_str());

  out.push_back(std::string("--project=" + project_dir.string()).c_str());
  out.push_back(std::string("--src=" + source_dir.string()).c_str());
  out.push_back(std::string("--vendor=" + vendor_dir.string()).c_str());
  out.push_back(std::string("--ffi-json=" + ffi_json_dir.string()).c_str());

  return out;
}