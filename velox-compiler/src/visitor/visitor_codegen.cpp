#include "visitor_codegen.hpp"

#include <filesystem>

#include <llvm-19/llvm/IR/Constant.h>
#include <llvm-19/llvm/IR/Instructions.h>
#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Value.h>

#include "ast/ast_data.hpp"
#include "script_info.hpp"
#include "compiler_data.hpp"
#include "error_output.hpp"

#include "ast/ast_base.hpp"
#include "ast/ast_declaration.hpp"
#include "ast/ast_declaration_local.hpp"
#include "ast/ast_declaration_cop.hpp"
#include "ast/ast_generic.hpp"
#include "ast/ast_literal.hpp"
#include "ast/ast_memory.hpp"
#include "ast/ast_operation.hpp"
#include "ast/ast_statement.hpp"
#include "ast/ast_expression.hpp"
#include "ast/ast_type.hpp"


Visitor_Codegen::Visitor_Codegen(ScriptInfo& _scr_info)
  : scr_info(_scr_info)
  , ctx()
  , mod(std::filesystem::path(_scr_info.file_path).filename().stem().c_str(), ctx)
  , builder(ctx)
{
}


void Visitor_Codegen::error_add(ErrorCode code, const ast::Node& n, const std::string& msg,
                                const std::string& hint) const
{
  auto error =
      Error_Diagnostic(code, scr_info, n._token, {}, compiler::EPhase::llvmir, EErrorSeverity::error, {}, msg, hint);

  errors.push_back(error.print_error());
}

void Visitor_Codegen::error_two_lines(ErrorCode code, const ast::Node& first, const ast::Node& second,
                                      const std::string& msg, const std::string& hint) const
{
  auto first_error = Error_Diagnostic(code, *first._scr_info, first._token, {}, compiler::EPhase::llvmir,
                                      EErrorSeverity::error, {}, msg, hint);

  auto second_error = Error_Diagnostic(code, *second._scr_info, second._token, {}, compiler::EPhase::llvmir,
                                       EErrorSeverity::error, {}, msg, hint);

  std::string out = "[from file] " color_MAGENTA + first_error.print_source() + color_RESET "\n";
  out += first_error.print_line() + color_RESET "\n";
  out += "[to file]   " color_MAGENTA + second_error.print_source() + color_RESET "\n";
  out += second_error.print_line() + color_RESET "\n";

  out += first_error.print_messages();
  errors.push_back(out);
}

/*
llvm::Value* Visitor_Codegen::visit(ast::Root& n)
{
  llvm::Value* last_valid = nullptr;

  for (auto& elem : n.global_nodes) {
    last_valid = elem->codegen(*this);
  }
  // test main fn
  auto fn_ty   = llvm::FunctionType::get(builder.getInt32Ty(), false);
  auto main_fn = llvm::Function::Create(fn_ty, llvm::Function::ExternalLinkage, "main", mod);

  auto entry = llvm::BasicBlock::Create(ctx, "entry", main_fn);
  builder.SetInsertPoint(entry);
  builder.CreateRet(builder.getInt32(0));
  mod.print(llvm::outs(), nullptr);
  return nullptr;
}*/


// ============ AST ============
llvm::Value* Visitor_Codegen::visit(ast::Node& n)
{
}

llvm::Type* Visitor_Codegen::visit(ast::AType& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::ALiteral& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::ADeclaration& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::ALocal& n)
{
  if (auto ptr = dynamic_cast<ast::declaration::local::Variable*>(&n)) visit(*ptr);
}
llvm::Value* Visitor_Codegen::visit(ast::AExpression& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::AIdentifier& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::Expr_ID& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::Expr_ID_Qualified& n)
{
}
llvm::Type* Visitor_Codegen::visit(ast::Expr_ID_Type& n)
{
}

void Visitor_Codegen::visit(ast::Root& n)
{
  for (auto& elem : n.global_nodes) {
    if (auto ptr = dynamic_cast<ast::declaration::Function*>(elem.get())) visit(*ptr);
    if (auto ptr = dynamic_cast<ast::declaration::Global*>(elem.get())) visit(*ptr);
  }
}

// ============ DECLARATION ============
llvm::GlobalVariable* Visitor_Codegen::visit(ast::declaration::Global& n)
{
}
llvm::Function* Visitor_Codegen::visit(ast::declaration::Function& n)
{
  /*
llvm::Value* Visitor_Codegen::visit(ast::Root& n)
{
  llvm::Value* last_valid = nullptr;

  for (auto& elem : n.global_nodes) {
    last_valid = elem->codegen(*this);
  }
  // test main fn
  auto fn_ty   = llvm::FunctionType::get(builder.getInt32Ty(), false);
  auto main_fn = llvm::Function::Create(fn_ty, llvm::Function::ExternalLinkage, "main", mod);

  auto entry = llvm::BasicBlock::Create(ctx, "entry", main_fn);
  builder.SetInsertPoint(entry);
  builder.CreateRet(builder.getInt32(0));
  mod.print(llvm::outs(), nullptr);
  return nullptr;
}*/

  auto fn_ty = visit(*n.prototype.get());
  auto fn    = llvm::Function::Create(
      fn_ty, n.is_exported || n.is_extern ? llvm::Function::ExternalLinkage : llvm::Function::InternalLinkage, n.name,
      mod);

  if (!n.codeblock) return fn;

  auto entry = llvm::BasicBlock::Create(ctx, "entry", fn);
  builder.SetInsertPoint(entry);

  visit(*n.codeblock.get());

  if (!entry->getTerminator()) builder.CreateRetVoid();
}

void Visitor_Codegen::visit(ast::declaration::Mod& n)
{
}
void Visitor_Codegen::visit(ast::declaration::Export& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::declaration::Extern& n)
{
}

void Visitor_Codegen::visit(ast::declaration::Enum& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::declaration::Enum_Element& n)
{
}

void Visitor_Codegen::visit(ast::declaration::Flag& n)
{
}

void Visitor_Codegen::visit(ast::declaration::Mod_Alias& n)
{
}
llvm::Type* Visitor_Codegen::visit(ast::declaration::Type_Alias& n)
{
}

void Visitor_Codegen::visit(ast::declaration::Generic& n)
{
}

// ============ LOCAL ============
void Visitor_Codegen::visit(ast::declaration::local::CodeBlock& n)
{
  for (auto& elem : n.elements) {
    switch (elem.kind) {
    case ast::CodeBlock_instruction::EKind::None:         continue;
    case ast::CodeBlock_instruction::EKind::Shared_local: visit(*elem.data_local.get()); break;
    case ast::CodeBlock_instruction::EKind::Unique_base:  visit(*elem.data_base); break;
    }
  }
}

llvm::Function* Visitor_Codegen::visit(ast::declaration::local::Lambda& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::declaration::local::Lambda_Capture& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::declaration::local::Capture_Member& n)
{
}

llvm::Value* Visitor_Codegen::visit(ast::declaration::local::Parameter& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::declaration::local::Generic_Parameter_Element& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::declaration::local::Generic_Parameters& n)
{
}

llvm::Value* Visitor_Codegen::visit(ast::declaration::local::Pattern& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::declaration::local::Pattern_Enum& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::declaration::local::Pattern_Tuple& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::declaration::local::Pattern_Entity& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::declaration::local::Pattern_Component& n)
{
}

llvm::Value* Visitor_Codegen::visit(ast::declaration::local::Variable_Binding& n)
{
}
void Visitor_Codegen::visit(ast::declaration::local::Variable_Unpack& n)
{
}
llvm::AllocaInst* Visitor_Codegen::visit(ast::declaration::local::Variable& n)
{
  auto var_ty = visit(*n.type.get());

  if (n.kind == EVariableKind::Const) {
  }

  llvm::AllocaInst* var;
  if (auto ptr_ty_table = dynamic_cast<ast::type::Table*>(n.type.get())) {
    if (ptr_ty_table->table_size.has_value()) {
      var =
          builder.CreateAlloca(visit(*ptr_ty_table->inner), builder.getInt32(ptr_ty_table->table_size.value()), n.name);
    }
    var = builder.CreateAlloca(visit(*n.type.get()), nullptr, n.name);
  } else {
    var = builder.CreateAlloca(visit(*n.type.get()), nullptr, n.name);
  }

  if (n.expression) builder.CreateStore(visit(*n.expression.get()), var, n.type->type_isVolatile);
}

llvm::Value* Visitor_Codegen::visit(ast::declaration::local::Capability& n)
{
}

// ============ COP ============
llvm::Value* Visitor_Codegen::visit(ast::declaration::cop::Component& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::declaration::cop::Component_Field& n)
{
}

llvm::Value* Visitor_Codegen::visit(ast::declaration::cop::Role& n)
{
}

llvm::Value* Visitor_Codegen::visit(ast::declaration::cop::Entity& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::declaration::cop::Entity_Cast& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::declaration::cop::Entity_Op& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::declaration::cop::Entity_OpIndex& n)
{
}

llvm::Value* Visitor_Codegen::visit(ast::declaration::cop::System& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::declaration::cop::System_Case& n)
{
}

// ============ GENERIC ============
llvm::Value* Visitor_Codegen::visit(ast::generic::Is_Type& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::generic::Can_Cast& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::generic::Have_Op& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::generic::Have_Role& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::generic::Use_Component& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::generic::Compatible_System& n)
{
}

// ============ TYPE ============
llvm::Type* Visitor_Codegen::visit(ast::type::Ptr& n)
{
}
llvm::Type* Visitor_Codegen::visit(ast::type::Table& n)
{
}
llvm::Type* Visitor_Codegen::visit(ast::type::Primitive& n)
{
}
llvm::StructType* Visitor_Codegen::visit(ast::type::Tuple& n)
{
}
llvm::FunctionType* Visitor_Codegen::visit(ast::type::Function_Proto& n)
{
}

llvm::Type* Visitor_Codegen::visit(ast::type::Get_Expr_Type& n)
{
}

// ============ LITERAL ============
llvm::Value* Visitor_Codegen::visit(ast::literal::Boolean& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::literal::Integral& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::literal::Decimal& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::literal::Floating& n)
{
}

llvm::Value* Visitor_Codegen::visit(ast::literal::ASCII& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::literal::UTF32& n)
{
}

llvm::Value* Visitor_Codegen::visit(ast::literal::Text& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::literal::Text_Interpolation& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::literal::Textual_Element& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::literal::Textual_Format& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::literal::Format_Specifier& n)
{
}

llvm::Value* Visitor_Codegen::visit(ast::literal::Table& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::literal::Table_Population& n)
{
}

llvm::Value* Visitor_Codegen::visit(ast::literal::Map& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::literal::Tuple& n)
{
}

llvm::Value* Visitor_Codegen::visit(ast::literal::Range& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::literal::Iterator& n)
{
}

llvm::Value* Visitor_Codegen::visit(ast::literal::Enum& n)
{
}

llvm::Value* Visitor_Codegen::visit(ast::literal::Component& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::literal::Entity& n)
{
}

// ============ Expression ============
llvm::Value* Visitor_Codegen::visit(ast::expression::If_Ternary& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::expression::Member_Access& n)
{
}

llvm::Value* Visitor_Codegen::visit(ast::expression::Self& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::expression::Other& n)
{
}

llvm::Value* Visitor_Codegen::visit(ast::expression::Call& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::expression::Call_Argument& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::expression::Call_System& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::expression::Call_Pipe& n)
{
}

llvm::Value* Visitor_Codegen::visit(ast::expression::Table_Access& n)
{
}

llvm::Value* Visitor_Codegen::visit(ast::expression::Ptr_At& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::expression::Ptr_Offset& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::expression::Ptr_Val& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::expression::Addr_Of& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::expression::Size_Of& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::expression::GetBits& n)
{
}

llvm::Value* Visitor_Codegen::visit(ast::expression::Move& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::expression::New_Ptr& n)
{
}

// ============ STATEMENT ============
void Visitor_Codegen::visit(ast::statement::If& n)
{
}
void Visitor_Codegen::visit(ast::statement::For& n)
{
}
void Visitor_Codegen::visit(ast::statement::Loop& n)
{
}
void Visitor_Codegen::visit(ast::statement::While& n)
{
}
void Visitor_Codegen::visit(ast::statement::GoTo& n)
{
}
void Visitor_Codegen::visit(ast::statement::GoTo_Label& n)
{
}

llvm::ReturnInst* Visitor_Codegen::visit(ast::statement::Return& n)
{
} // optionnel : retourne la instruction
llvm::BranchInst* Visitor_Codegen::visit(ast::statement::Break& n)
{
}
llvm::BranchInst* Visitor_Codegen::visit(ast::statement::Continue& n)
{
}

void Visitor_Codegen::visit(ast::statement::Match& n)
{
}
void Visitor_Codegen::visit(ast::statement::Match_Case& n)
{
}

// ============ OPERATION ============
llvm::Value* Visitor_Codegen::visit(ast::operation::Cast_As& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::operation::Is& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::operation::In& n)
{
}
llvm::Instruction* Visitor_Codegen::visit(ast::operation::Assignment& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::operation::Binary& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::operation::Unary& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::operation::Interval& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::operation::Ptr_Dist& n)
{
}

// ============ MEMORY ============
llvm::CallInst* Visitor_Codegen::visit(ast::memory::Del& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::memory::Align& n)
{
}
void Visitor_Codegen::visit(ast::memory::Drop& n)
{
}