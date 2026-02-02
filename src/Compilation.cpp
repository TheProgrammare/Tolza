#include "Compilation.hpp"

#include <filesystem>

const std::string PROJECT_DIR = std::filesystem::current_path().parent_path().string();
const std::string POSTPROCESS_OUT_DIR = PROJECT_DIR + "/post_process";
const std::string BINDING_DIR = PROJECT_DIR + "/EMBinds";
const std::string LLVM_IR_DIR = PROJECT_DIR + "/llvm_ir";
