#pragma once

#include <vector>
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
    {"logs",        "embinder"       },
    {"logs",        "exporter"       },
    {"logs",        "resolver"       },
    {"logs",        "llvm-ir"        },
    {"logs",        "linker"         },
    {"defines",     ""               },
    {"undefines",   ""               },
    {"codegen",     "emit_mode"      },
    {"codegen",     "output_dir"     },
    {"codegen",     "dest_file_dir"  },
    {"project",     "project_dir"    },
    {"project",     "source_dir"     },
    {"project",     "thrid_party_dir"},
    {"sub_configs", ""               },
};

std::vector<std::string> check_velox_config_sanity(const fs::path& file, bool full_config);
bool                     check_workspace_sanity(const fs::path& ws_path);

} // namespace sanity
} // namespace command