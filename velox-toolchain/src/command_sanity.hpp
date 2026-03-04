#pragma once

#include <string>
#include <filesystem>
#include <map>

namespace fs = std::filesystem;


namespace command
{
namespace sanity
{

// key: section | key: field
static std::map<std::string, std::string> k_config_map = {
    {"target",      "abi"            },
    {"target",      "arch"           },
    {"target",      "bits"           },
    {"target",      "os"             },
    {"target",      "libc"           },
    {"target",      "config"         },
    {"profile",     "debug"          },
    {"profile",     "opt_level"      },
    {"profile",     "size_opt"       },
    {"logs",        "all"            },
    {"logs",        "filesystem"     },
    {"logs",        "lexer"          },
    {"logs",        "preprocessor"   },
    {"logs",        "parser"         },
    {"logs",        "binder"         },
    {"logs",        "exporter"       },
    {"logs",        "resolver"       },
    {"logs",        "llvm-ir"        },
    {"logs",        "linker"         },
    {"defines",     ""               },
    {"undefines",   ""               },
    {"codegen",     "emit_mode"      },
    {"codegen",     "build_dir"      },
    {"project",     "project_dir"    },
    {"project",     "source_dir"     },
    {"project",     "thrid_party_dir"},
    {"project",     "ffi-json_dir"   },
    {"sub_configs", ""               },
};

bool check_velox_config_sanity(const fs::path& file, bool full_config, bool verbose = true);
bool check_workspace_sanity(const fs::path& ws_path, bool verbose = true);

} // namespace sanity
} // namespace command