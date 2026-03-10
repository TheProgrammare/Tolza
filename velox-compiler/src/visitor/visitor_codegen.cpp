#include "visitor_codegen.hpp"

#include <filesystem>

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

// ============ AST ============
llvm::Value* Visitor_Codegen::visit(ast::Node& n)
{
  return nullptr;
}

llvm::Value* Visitor_Codegen::visit(ast::AType& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::ALiteral& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::ADeclaration& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::ALocal& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::AExpression& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::AIdentifier& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::Expr_ID& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::Expr_ID_Qualified& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::Expr_ID_Type& n)
{
  return nullptr;
}

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
}

// ============ DECLARATION ============
llvm::Value* Visitor_Codegen::visit(ast::declaration::Global& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::declaration::Function& n)
{
  return nullptr;
}

llvm::Value* Visitor_Codegen::visit(ast::declaration::Mod& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::declaration::Export& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::declaration::Extern& n)
{
  return nullptr;
}

llvm::Value* Visitor_Codegen::visit(ast::declaration::Enum& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::declaration::Enum_Element& n)
{
  return nullptr;
}

llvm::Value* Visitor_Codegen::visit(ast::declaration::Flag& n)
{
  return nullptr;
}

llvm::Value* Visitor_Codegen::visit(ast::declaration::Mod_Alias& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::declaration::Type_Alias& n)
{
  return nullptr;
}

llvm::Value* Visitor_Codegen::visit(ast::declaration::Generic& n)
{
  return nullptr;
}

// ============ LOCAL ============
llvm::Value* Visitor_Codegen::visit(ast::declaration::local::CodeBlock& n)
{
  return nullptr;
}

llvm::Value* Visitor_Codegen::visit(ast::declaration::local::Lambda& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::declaration::local::Lambda_Capture& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::declaration::local::Capture_Member& n)
{
  return nullptr;
}

llvm::Value* Visitor_Codegen::visit(ast::declaration::local::Parameter& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::declaration::local::Generic_Parameter_Element& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::declaration::local::Generic_Parameters& n)
{
  return nullptr;
}

llvm::Value* Visitor_Codegen::visit(ast::declaration::local::Pattern& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::declaration::local::Pattern_Enum& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::declaration::local::Pattern_Tuple& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::declaration::local::Pattern_Entity& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::declaration::local::Pattern_Component& n)
{
  return nullptr;
}

llvm::Value* Visitor_Codegen::visit(ast::declaration::local::Variable_Binding& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::declaration::local::Variable_Unpack& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::declaration::local::Variable& n)
{
  return nullptr;
}

llvm::Value* Visitor_Codegen::visit(ast::declaration::local::Capability& n)
{
  return nullptr;
}

// ============ COP ============
llvm::Value* Visitor_Codegen::visit(ast::declaration::cop::Component& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::declaration::cop::Component_Field& n)
{
  return nullptr;
}

llvm::Value* Visitor_Codegen::visit(ast::declaration::cop::Role& n)
{
  return nullptr;
}

llvm::Value* Visitor_Codegen::visit(ast::declaration::cop::Entity& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::declaration::cop::Entity_Cast& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::declaration::cop::Entity_Op& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::declaration::cop::Entity_OpIndex& n)
{
  return nullptr;
}

llvm::Value* Visitor_Codegen::visit(ast::declaration::cop::System& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::declaration::cop::System_Case& n)
{
  return nullptr;
}

// ============ GENERIC ============
llvm::Value* Visitor_Codegen::visit(ast::generic::Is_Type& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::generic::Can_Cast& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::generic::Have_Op& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::generic::Have_Role& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::generic::Use_Component& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::generic::Compatible_System& n)
{
  return nullptr;
}

// ============ TYPE ============
llvm::Value* Visitor_Codegen::visit(ast::type::Ptr& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::type::Table& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::type::Primitive& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::type::Tuple& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::type::Function_Proto& n)
{
  return nullptr;
}

llvm::Value* Visitor_Codegen::visit(ast::type::Get_Expr_Type& n)
{
  return nullptr;
}

// ============ LITERAL ============
llvm::Value* Visitor_Codegen::visit(ast::literal::Boolean& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::literal::Integral& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::literal::Decimal& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::literal::Floating& n)
{
  return nullptr;
}

llvm::Value* Visitor_Codegen::visit(ast::literal::ASCII& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::literal::UTF32& n)
{
  return nullptr;
}

llvm::Value* Visitor_Codegen::visit(ast::literal::Text& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::literal::Text_Interpolation& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::literal::Textual_Element& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::literal::Textual_Format& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::literal::Format_Specifier& n)
{
  return nullptr;
}

llvm::Value* Visitor_Codegen::visit(ast::literal::Table& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::literal::Table_Population& n)
{
  return nullptr;
}

llvm::Value* Visitor_Codegen::visit(ast::literal::Map& n)
{
  return nullptr;
}

llvm::Value* Visitor_Codegen::visit(ast::literal::Tuple& n)
{
  return nullptr;
}

llvm::Value* Visitor_Codegen::visit(ast::literal::Range& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::literal::Iterator& n)
{
  return nullptr;
}

llvm::Value* Visitor_Codegen::visit(ast::literal::Enum& n)
{
  return nullptr;
}

llvm::Value* Visitor_Codegen::visit(ast::literal::Component& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::literal::Entity& n)
{
  return nullptr;
}

// ============ Expression ============
llvm::Value* Visitor_Codegen::visit(ast::expression::If_Ternary& n)
{
  return nullptr;
}

llvm::Value* Visitor_Codegen::visit(ast::expression::Member_Access& n)
{
  return nullptr;
}

llvm::Value* Visitor_Codegen::visit(ast::expression::Self& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::expression::Other& n)
{
  return nullptr;
}

llvm::Value* Visitor_Codegen::visit(ast::expression::Call& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::expression::Call_Argument& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::expression::Call_System& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::expression::Call_Pipe& n)
{
  return nullptr;
}

llvm::Value* Visitor_Codegen::visit(ast::expression::Table_Access& n)
{
  return nullptr;
}

llvm::Value* Visitor_Codegen::visit(ast::expression::Ptr_At& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::expression::Ptr_Offset& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::expression::Ptr_Val& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::expression::Addr_Of& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::expression::Size_Of& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::expression::GetBits& n)
{
  return nullptr;
}

llvm::Value* Visitor_Codegen::visit(ast::expression::Move& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::expression::New_Ptr& n)
{
  return nullptr;
}

// ============ STATEMENT ============
llvm::Value* Visitor_Codegen::visit(ast::statement::If& n)
{
  return nullptr;
}

llvm::Value* Visitor_Codegen::visit(ast::statement::For& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::statement::Loop& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::statement::While& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::statement::GoTo& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::statement::GoTo_Label& n)
{
  return nullptr;
}

llvm::Value* Visitor_Codegen::visit(ast::statement::Return& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::statement::Break& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::statement::Continue& n)
{
  return nullptr;
}

llvm::Value* Visitor_Codegen::visit(ast::statement::Match& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::statement::Match_Case& n)
{
  return nullptr;
}

// ============ OPERATION ============
llvm::Value* Visitor_Codegen::visit(ast::operation::Cast_As& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::operation::Is& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::operation::In& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::operation::Assignment& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::operation::Binary& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::operation::Unary& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::operation::Interval& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::operation::Ptr_Dist& n)
{
  return nullptr;
}

// ============ MEMORY ============
llvm::Value* Visitor_Codegen::visit(ast::memory::Del& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::memory::Align& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::memory::Drop& n)
{
  return nullptr;
}