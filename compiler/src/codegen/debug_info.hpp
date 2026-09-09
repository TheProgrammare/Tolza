#pragma once

#include "codegen/llvm_forward.hpp"
#include "id/cuid.hpp"
#include "id/nodeid.hpp"

namespace codegen
{
struct Codegen_AST;

struct DebugInfo final {
  DebugInfo(codegen::Codegen_AST* resolver, cu::ID cuid);

  llvm::DIBuilder&      builder;
  codegen::Codegen_AST* res;


  llvm::DIFile*        file = nullptr;
  llvm::DICompileUnit& compileUnit;
  llvm::DILocalScope*  current_scp = nullptr;

  llvm::DILocation* location(ast::ID nodeid, llvm::DILocalScope* new_scope = nullptr);
};

} // namespace codegen
