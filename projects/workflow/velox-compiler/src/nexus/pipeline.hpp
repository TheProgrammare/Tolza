#pragma once

#include <memory>
#include <string_view>
#include <vector>
#include <unordered_set>
#include <functional>


#include "nexus/forward.hpp"
#include "nexus/ids.hpp"
#include "nexus/script.hpp"


namespace pipeline
{

// start compilation:
// └> engage preparer
// |└> pass lexer
// |└> pass preprocessor
// |└> pass parser
// └o wait all scripts prepared
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
// |└> pass script emitter (for other format)
// └> engage module linker (llvm specific)
// └> engage linker (executable generation)
struct Pipeline {
  std::unordered_map<std::string, script::_id>      path_generated;
  std::vector<std::unique_ptr<script::ScriptInfo>>  compilation_scripts;
  std::unordered_set<script::_id, script::_id_hash> unprepared_scripts;
  std::unordered_set<script::_id, script::_id_hash> prepared_scripts;
  std::unordered_set<script::_id, script::_id_hash> analyzed_scripts;

  // push to prepared_scripts
  std::vector<script::_id>            query_scripts_at_dir(std::string_view path);
  // push to prepared_scripts
  script::_id                         query_script_at_path(std::string_view path);
  std::unique_ptr<script::ScriptInfo> build_script_from_path(std::string_view path);


  double timing(std::function<void()> f);

  // for each script
  bool   engage_preparer(script::_id scr_id);
  // for all prepared_scripts
  size_t engage_shipowner();
  // for each script
  bool   engage_analyzer(script::_id scr_id);
  // for all analyzed_scripts
  bool   engage_generator(script::_id scr_id);
  // link and make object
  bool   engage_module_linker(script::_id scr_id);
  bool   engage_linker();


  script::ScriptInfo& get_script(script::_id scr_id);

private:
  // preparer
  bool pass_lexer(script::_id scr_id);
  bool pass_preprocessor(script::_id scr_id);
  bool pass_parser(script::_id scr_id);

  bool pass_binding_generation(std::vector<std::string_view> path, std::string_view alias);

  // analyzer
  size_t pass_resolution_symbol(script::_id scr_id);
  size_t pass_resolution_inference(script::_id scr_id);
  size_t pass_resolution_semantic(script::_id scr_id);

  // generator
  bool pass_code_generation(script::_id scr_id);
  bool pass_llvm_optimization(script::_id scr_id);
  bool pass_llvm_emitter(script::_id scr_id);
  bool pass_script_emitter(script::_id scr_id);
};


} // namespace pipeline