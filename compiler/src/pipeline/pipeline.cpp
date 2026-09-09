#include "pipeline.hpp"

#include <algorithm>
#include <cassert>
#include <cerrno>
#include <chrono>
#include <common/compiler_options.hpp>
#include <common/fileutils.hpp>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iterator>
#include <ratio>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>


// Project headers
#include "compiler/compilation_unit.hpp"
#include "compiler/compiler.hpp"
#include "compiler/io.hpp"
#include "ffi/c_reader.hpp"
#include "id/cuid.hpp"
#include "nexus/forward.hpp"
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
  auto* root_cu        = new cu::CU();
  root_cu->status.root = true;

  compilation_units.emplace_back(root_cu);
}


cu::CU* pipeline::Pipeline::build_CU_from_path(cu::ID parent_cuid, std::string_view path) noexcept
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
  f.close();

  auto cuid = cu::ID::make(PIPELINE.compilation_units.size());

  if (OPTIONS.is_check_mode) {
    std::string file_path =
        common::fileutils::is_sub_path(common::fileutils::get_temp_dir(), path)
            ? common::fileutils::overlay_to_real_path(path, OPTIONS.dir.overlay, OPTIONS.project_path)
            : std::string(path);
    return new cu::CU(parent_cuid, cuid, file_path, data, last_offset_line);
  }

  return new cu::CU(parent_cuid, cuid, path, data, last_offset_line);
}

std::vector<cu::ID> pipeline::Pipeline::query_CUs_at_dir(cu::ID parent_cuid, std::string_view path) noexcept
{
  auto set      = common::fileutils::find_tolza_files(path, true);
  auto f_founds = std::vector<std::string>(set.begin(), set.end());

  if (OPTIONS.is_check_mode) {
    const auto relative_path = fs::relative(path, OPTIONS.project_path);

    const auto overlay_path = OPTIONS.dir.overlay / relative_path;

    std::vector<cu::ID> compilation_units_ids;

    for (auto& file : f_founds) {
      const auto relative_file = fs::relative(file, path);
      const auto overlay_file  = overlay_path / relative_file;

      if (fs::exists(overlay_file)) {
        file = overlay_file.string();
      }

      if (auto it = path_generated.find(file); it != path_generated.end()) {
        compilation_units_ids.emplace_back(it->second);
        continue;
      }

      auto* cu   = build_CU_from_path(parent_cuid, file);
      auto  cuid = cu->cuid;

      compilation_units_ids.emplace_back(cuid);
      compilation_units.emplace_back(cu);

      path_generated.try_emplace(file, cuid);

      if (!prepared_compilation_units.contains(cuid)) {
        unprepared_compilation_units.insert(cuid);
      }
    }

    return compilation_units_ids;
  }

  std::vector<cu::ID> compilation_units_ids;

  for (auto& file : f_founds) {
    if (auto it = path_generated.find(file); it != path_generated.end()) {
      compilation_units_ids.emplace_back(it->second);
      continue;
    }

    auto* cu   = build_CU_from_path(parent_cuid, file);
    auto  cuid = cu->cuid;

    compilation_units_ids.emplace_back(cuid);
    compilation_units.emplace_back(cu);

    path_generated.try_emplace(file, cuid);

    if (!prepared_compilation_units.contains(cuid)) {
      unprepared_compilation_units.insert(cuid);
    }
  }

  return compilation_units_ids;
}


cu::ID pipeline::Pipeline::query_CU_at_path(cu::ID parent_cuid, std::string_view path) noexcept
{
  fs::path file_path{path};

  if (OPTIONS.is_check_mode) {
    const auto relative_path = fs::relative(file_path, OPTIONS.project_path);

    const auto overlay_path = OPTIONS.dir.overlay / relative_path;

    if (fs::exists(overlay_path)) {
      file_path = overlay_path;
    }

    if (auto it = path_generated.find(file_path.string()); it != path_generated.end()) {
      return it->second;
    }

    auto* cu     = build_CU_from_path(parent_cuid, file_path.string());
    auto  new_id = cu->cuid;

    compilation_units.emplace_back(cu);
    path_generated.try_emplace(file_path.string(), new_id);

    return new_id;
  }

  if (auto it = path_generated.find(file_path.string()); it != path_generated.end()) {
    return it->second;
  }

  auto* cu     = build_CU_from_path(parent_cuid, file_path.string());
  auto  new_id = cu->cuid;

  compilation_units.emplace_back(cu);
  path_generated.try_emplace(file_path.string(), new_id);

  return new_id;
}


bool pipeline::Pipeline::generate_libc_wrappers() noexcept
{
  auto p = fs::path(OPTIONS.get_dir_binding_profile()) / "C";
  if (fs::exists(p)) fs::remove_all(p);
  p = common::fileutils::get_tolza_file(p.string());
  if (fs::exists(p)) fs::remove(p);

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
