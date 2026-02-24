#include "Compilation.hpp"

CompCtx COMP_CTX = CompCtx{
    "LP64",
    "native",
    64,
    "",
    "",
    true,
    0,
    false,
    {},
    {},
    k_project_dir "/a.out",
    true,
    CompCtx::EEmitMode::OBJ,
    k_project_dir,
    k_project_dir,
};

std::map<std::string, std::string> COMPILATION_ARGS;