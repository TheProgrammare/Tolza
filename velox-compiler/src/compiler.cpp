#include "compiler.hpp"

#include <string>
#include <filesystem>

namespace fs = std::filesystem;


const std::string& compiler::CompCtx::get_preprocess_dir() const
{
  static auto out = (fs::path(get_project_dir()) / "preprocess").string();
  return out;
}

const std::string& compiler::CompCtx::get_debug_graph_dir() const
{
  static auto out = (fs::path(get_build_dir()) / "graph").string();
  return out;
}

const std::string& compiler::CompCtx::get_llvmir_dir() const
{
  static auto out = (fs::path(get_build_dir()) / "llvm-ir").string();
  return out;
}