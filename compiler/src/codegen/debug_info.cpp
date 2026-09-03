#include "codegen/debug_info.hpp"

#include "ast/data.hpp"
#include "codegen/codegen.hpp"
#include "compiler/compilation_unit.hpp"
#include "pool/token.hpp"

#include <filesystem>
#include <llvm-19/llvm/IR/DIBuilder.h>
#include <llvm-19/llvm/IR/DebugInfoMetadata.h>
#include <llvm-19/llvm/IR/LLVMContext.h>
#include <llvm-19/llvm/IR/Module.h>

namespace fs = std::filesystem;

codegen::DebugInfo::DebugInfo(codegen::Codegen_AST* resolver, cu::ID cuid)
  : res(resolver)
  , builder(*new llvm::DIBuilder(*cuid.get().llvm_module))
  , file(builder.createFile(fs::path(cuid.get().file_info.path).stem().string(),
                            fs::path(cuid.get().file_info.path).parent_path().string()))
  , compileUnit(*builder.createCompileUnit(llvm::dwarf::DW_LANG_C, file, "tolza-compiler", false, "", 0))
{
}

llvm::DILocation* codegen::DebugInfo::location(ast::ID nodeid, llvm::DILocalScope* new_scope)
{
  const auto& tokid = nodeid.token();

  switch (nodeid.kind()) {
    // -------------------------------------------------------------------------
    // Function scopes
    // -------------------------------------------------------------------------

  case ast::ENodeKind::Global_Function:
  case ast::ENodeKind::Global_Extend_Fn:
  case ast::ENodeKind::Global_Extend_Cast:
  case ast::ENodeKind::Global_Extend_Op_Bin:
  case ast::ENodeKind::Global_Extend_Op_Un:
  case ast::ENodeKind::Global_Extend_Op_Subscript:
  case ast::ENodeKind::Global_Extend_Op_Transfert:
  case ast::ENodeKind::Global_Extend_Op_Other:
  case ast::ENodeKind::Local_Lambda:               current_scp = new_scope; break;

  case ast::ENodeKind::CodeBlock:                  current_scp = new_scope;

  default:                                         assert(!new_scope && "No new scope for unscoped node");
  }

  return llvm::DILocation::get(res->ctx, tokid.line(), tokid.col(), current_scp);
}