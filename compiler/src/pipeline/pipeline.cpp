#include "pipeline.hpp"

#include <cassert>
#include <cerrno>
#include <chrono>
#include <common/common.hpp>
#include <common/compiler_options.hpp>
#include <common/fileutils.hpp>
#include <filesystem>
#include <fstream>
#include <functional>
#include <memory>
#include <print>
#include <string>
#include <unordered_set>
#include <vector>


// Project headers
#include "binder/ffi_c_reader.hpp"
#include "compiler/compilation_unit.hpp"
#include "compiler/compiler.hpp"
#include "compiler/io.hpp"
#include "nexus/forward.hpp"
#include "nexus/ids.hpp"
#include "pipeline/preparer.hpp"

namespace fs = std::filesystem;


inline double timing(const std::function<void()>& f) noexcept
{
  auto start = std::chrono::high_resolution_clock::now();

  f();

  auto end = std::chrono::high_resolution_clock::now();

  double milli = std::chrono::duration<double, std::milli>(end - start).count();

  return milli;
}


pipeline::Pipeline::Pipeline()
{
  auto root_cu         = std::make_unique<cu::CU>();
  root_cu->status.root = true;

  compilation_units.emplace_back(std::move(root_cu));
}


std::unique_ptr<cu::CU> pipeline::Pipeline::build_CU_from_path(cu::ID parent_cuid, std::string_view path) noexcept
{
  assert(common::fileutils::is_tolza_file(path));

  auto p = common::fileutils::get_tolza_file(path);

  std::ifstream f(std::string(p).c_str());

  const std::string   data((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
  std::vector<size_t> last_offset_line;
  size_t              nb_newlines = std::count(data.begin(), data.end(), '\n');
  last_offset_line.reserve(nb_newlines);
  size_t offset = 0;

  for (char c : data) {
    if (c == '\n') last_offset_line.emplace_back(offset);
    offset++;
  }

  auto cuid = cu::ID::make(compiler::pipeline.compilation_units.size());

  auto ptr = std::make_unique<cu::CU>(parent_cuid, cuid, path, data, last_offset_line);

  f.close();

  return ptr;
}

std::vector<cu::ID> pipeline::Pipeline::query_CUs_at_dir(cu::ID parent_cuid, std::string_view path) noexcept
{
  auto f_founds = common::fileutils::find_tolza_files(path, true);

  std::vector<cu::ID> compilation_units_ids;

  for (const auto& file : f_founds) {
    if (auto it = path_generated.find(file); it != path_generated.end()) {
      compilation_units_ids.emplace_back(it->second);
      continue;
    }

    auto cu   = build_CU_from_path(parent_cuid, file);
    auto cuid = cu->cuid;
    compilation_units_ids.emplace_back(cu->cuid);
    compilation_units.emplace_back(std::move(cu));

    path_generated.try_emplace(file, cuid);
    if (!prepared_compilation_units.contains(cuid)) unprepared_compilation_units.insert(cuid);
  }

  return compilation_units_ids;
}
cu::ID pipeline::Pipeline::query_CU_at_path(cu::ID parent_cuid, std::string_view path) noexcept
{
  if (auto it = path_generated.find(path); it != path_generated.end()) return it->second;

  auto cu     = build_CU_from_path(parent_cuid, path);
  auto new_id = cu->cuid;
  compilation_units.emplace_back(std::move(cu));

  path_generated.try_emplace(std::string(path), new_id);

  return new_id;
}


bool pipeline::Pipeline::generate_libc_wrappers() noexcept
{
  auto p = fs::path(compiler::OPTIONS.get_dir_binding_profile()) / "C";
  fs::remove_all(p);
  p = common::fileutils::get_tolza_file(p.string());
  fs::remove(p);

  // native C language lib handler
  auto duration = timing([&]() { ffi::C_Reader::generate_libc_wrappers(); });

  IO::println(IO_PASS::binder, "C wrappers generation completed | {:.2f} ms", duration);

  return true;
}

bool pipeline::Pipeline::engage_bindings() noexcept
{
  for (const auto& bind_path : binding_compilation_units_to_prepare) {
    auto cu = query_CU_at_path(cu::ID::main(), bind_path);
    if (!preparer::prepare_cu(cu)) return false;
  }

  return true;
}
