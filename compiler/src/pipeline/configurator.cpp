#include "pipeline/configurator.hpp"

#include "compiler/compiler.hpp"
#include "compiler/io.hpp"

#include <common/compiler_options.hpp>
#include <common/fileutils.hpp>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace fs = std::filesystem;

void apply_foreach_profiles(std::string_view manifest_path, const std::vector<std::string>& source)
{
  for (const auto& profile : source) {
    auto f = common::fileutils::resolve_path(profile + ".toml", OPTIONS.dir.get_dir_profile());
    if (!fs::exists(f)) {
      IO::println(stderr, IO_PASS::NONE, R"(The profile "{}" at "{}" dosen't exist. Profile ignored.)", profile, f);
    } else {
      OPTIONS = common::compiler::Profile::read_profile(manifest_path, OPTIONS.project_path).merge_context(OPTIONS);
    }
  }

  for (const auto& profile : source) {
    auto f = common::fileutils::resolve_path(profile + ".toml", OPTIONS.dir.get_dir_profile());
    if (!fs::exists(f)) IO::println("  apply: \"{}\"", f);
  }
}

void configurator::init_compiler_OPTIONS(std::string_view                 manifest_path,
                                         const common::compiler::Profile& command_args) noexcept
{
  OPTIONS = common::compiler::Manifest::read_manifest(manifest_path);

  // merge args to main
  apply_foreach_profiles(manifest_path, OPTIONS.profiles);
  apply_foreach_profiles(manifest_path, command_args.profiles);
  OPTIONS = command_args.merge_context(OPTIONS);

  OPTIONS.is_check_mode = command_args.is_check_mode;
}