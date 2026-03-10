#pragma once

#include <expected>
#include <string>
#include <filesystem>
#include <map>

namespace fs = std::filesystem;

namespace command
{
namespace check
{
void err(const std::string& msg);
void log(const std::string& ms, bool sub_log = false);

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
    {"warnings",    "all"            },
    {"warnings",    "extra"          },
    {"warnings",    "pedantic"       },
    {"warnings",    "level"          },
    {"warnings",    "unused"         },
    {"warnings",    "dead_code"      },
    {"warnings",    "as_error"       },
    {"printer",     "ast"            },
    {"defines",     ""               },
    {"undefines",   ""               },
    {"codegen",     "emit_mode"      },
    {"codegen",     "build_dir"      },
    {"project",     "project_dir"    },
    {"project",     "source_dir"     },
    {"project",     "thrid_party_dir"},
    {"project",     "ffi-json_dir"   },
    {"project",     "compiler_file"  },
    {"sub_configs", ""               },
};

bool check_velox_config(const fs::path& file, bool full_config, bool verbose = true);
bool check_workspace(const fs::path& ws_path, bool verbose = true);

} // namespace check
} // namespace command