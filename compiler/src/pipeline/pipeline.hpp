#pragma once

#include "compiler/compilation_unit.hpp"
#include "nexus/forward.hpp"
#include "nexus/ids.hpp"

#include <functional>
#include <memory>
#include <set>
#include <string_view>
#include <unordered_set>
#include <vector>


namespace pipeline
{

// start compilation:
// └> engage preparer
// |└> pass lexer
// |└> pass preprocessor
// |└> pass parser
// └o wait all compilation_units prepared
// └> engage shipowner
// |└> (if import from binding)
// ||└> pass binding generation
// |└> engage preparer
// └o wait all imports prepared
// └> engage analyzer
// |└> pass resolution symbol
// |└> pass resolution inference
// |└> pass resolution semantic
// └> engage generator
// |└> pass code generation
// |└> (if llvm emit enabled)
// ||└> pass llvm emitter (for .ll format)
// |└> pass llvm optimization
// |└> pass compilation unit emitter (for other format)
// └> engage module linker (llvm specific)
// └> engage linker (executable generation)
struct Pipeline {
  Pipeline();

  StringMap<cu::ID>                         path_generated;
  // paths
  std::vector<std::unique_ptr<cu::CU>>      compilation_units;
  std::vector<std::unique_ptr<cu::TEMP_CU>> temp_compilation_units;
  std::set<std::string>                     binding_compilation_units_to_prepare;
  std::unordered_set<cu::ID, cu::ID::Hash>  unprepared_compilation_units;
  std::unordered_set<cu::ID, cu::ID::Hash>  prepared_compilation_units;
  std::unordered_set<cu::ID, cu::ID::Hash>  analyzed_compilation_units;

  // push to prepared_compilation_units
  [[nodiscard]] std::vector<cu::ID> query_CUs_at_dir(cu::ID parent_cuid, std::string_view path) noexcept;
  // push to prepared_compilation_units
  [[nodiscard]] cu::ID              query_CU_at_path(cu::ID parent_cuid, std::string_view path) noexcept;

  [[nodiscard]] static std::unique_ptr<cu::CU> build_CU_from_path(cu::ID parent_cuid, std::string_view path) noexcept;

  [[nodiscard]] bool generate_libc_wrappers() noexcept;

  [[nodiscard]] bool engage_bindings() noexcept;
};


} // namespace pipeline