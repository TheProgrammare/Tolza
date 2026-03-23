#include "compiler/compiler.hpp"

#include <llvm/IR/LLVMContext.h>

#include <compiler_context.hpp>

common::CompCtx   compiler::COMP_CTX;
llvm::LLVMContext compiler::LLVM_CTX;


void compiler::fmt_template(std::string& templateStr, const std::initializer_list<std::string>& args)
{
  size_t count = 0;
  for (auto& arg : args) {
    std::string placeholder = "%" + std::to_string(count++);
    size_t      pos         = 0;
    while ((pos = templateStr.find(placeholder, pos)) != std::string::npos) {
      templateStr.replace(pos, placeholder.length(), arg);
      pos += arg.length();
    }
  }
}


std::string compiler::Phase_to_code(EPhase phase)
{
  switch (phase) {
  case compiler::EPhase::filesystem:        return "FSYS";
  case compiler::EPhase::lexer:             return "LEXE";
  case compiler::EPhase::preprosessor:      return "PREP";
  case compiler::EPhase::parser:            return "PARS";
  case compiler::EPhase::binder:            return "EMBI";
  case compiler::EPhase::resolver_symbol:   return "SYMB";
  case compiler::EPhase::resolver_type:     return "TYPE";
  case compiler::EPhase::resolver_semantic: return "SEMA";
  case compiler::EPhase::llvmir:            return "LLVM";
  case compiler::EPhase::linker:            return "LINK";
  }
}

std::string compiler::Phase_to_str(EPhase phase)
{
  switch (phase) {
  case compiler::EPhase::filesystem:        return "file system";
  case compiler::EPhase::lexer:             return "lexer";
  case compiler::EPhase::preprosessor:      return "preprocessor";
  case compiler::EPhase::parser:            return "parser";
  case compiler::EPhase::binder:            return "external module binder";
  case compiler::EPhase::resolver_symbol:   return "resolver symbol";
  case compiler::EPhase::resolver_type:     return "resolver type";
  case compiler::EPhase::resolver_semantic: return "resolver semantic";
  case compiler::EPhase::llvmir:            return "LLVM IR";
  case compiler::EPhase::linker:            return "linker";
  }
}