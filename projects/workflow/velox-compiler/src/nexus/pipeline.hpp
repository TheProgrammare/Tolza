#pragma once

#include <memory>
#include <set>
#include <string_view>
#include <vector>
#include <unordered_set>
#include <functional>


#include "nexus/forward.hpp"
#include "nexus/ids.hpp"
#include "compiler/compilation_unit.hpp"


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

  StringMap<cu::ID>                        path_generated;
  // paths
  std::vector<std::unique_ptr<cu::CU>>     compilation_units;
  std::set<std::string>                    binding_compilation_units_to_prepare;
  std::unordered_set<cu::ID, cu::ID::Hash> unprepared_compilation_units;
  std::unordered_set<cu::ID, cu::ID::Hash> prepared_compilation_units;
  std::unordered_set<cu::ID, cu::ID::Hash> analyzed_compilation_units;

  // push to prepared_compilation_units
  [[nodiscard]] std::vector<cu::ID> query_CUs_at_dir(cu::ID parent_cuid, std::string_view path);
  // push to prepared_compilation_units
  [[nodiscard]] cu::ID              query_CU_at_path(cu::ID parent_cuid, std::string_view path);

  [[nodiscard]] static std::unique_ptr<cu::CU> build_CU_from_path(cu::ID parent_cuid, std::string_view path);


  [[nodiscard]] static double timing(const std::function<void()>& f);

  // for each compilation unit
  [[nodiscard]] bool        engage_preparer(cu::ID cuid);
  // for all prepared_compilation_units
  [[nodiscard]] size_t      engage_shipowner();
  [[nodiscard]] bool        engage_bindings();
  // for each compilation unit
  [[nodiscard]] bool        engage_analyzer(cu::ID cuid);
  // for all analyzed_compilation_units
  [[nodiscard]] bool        engage_generator(cu::ID cuid);
  // link and make object
  [[nodiscard]] bool        engage_module_linker(cu::ID cuid) const;
  [[nodiscard]] static bool engage_linker();


private:
  // preparer
  [[nodiscard]] static bool pass_lexer(cu::ID cuid);
  [[nodiscard]] static bool pass_preprocessor(cu::ID cuid);
  [[nodiscard]] static bool pass_parser(cu::ID cuid);


  [[nodiscard]] bool pass_binding_generation(const std::vector<std::string>& path, std::string_view alias);

  // analyzer
  [[nodiscard]] static size_t pass_resolution_symbol(cu::ID cuid);
  [[nodiscard]] static size_t pass_resolution_inference(cu::ID cuid);
  [[nodiscard]] static size_t pass_resolution_semantic(cu::ID cuid);

  // generator
  [[nodiscard]] static bool pass_code_generation(cu::ID cuid);
  [[nodiscard]] bool        pass_llvm_optimization(cu::ID cuid) const;
  [[nodiscard]] static bool pass_llvm_emitter(cu::ID cuid);
  [[nodiscard]] bool        pass_script_emitter(cu::ID cuid) const;
};


} // namespace pipeline