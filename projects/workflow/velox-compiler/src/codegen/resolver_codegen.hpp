
#pragma once

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/IRBuilder.h>

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include "nexus/ast/ast.hpp"
#include "nexus/ast/forward.hpp"
#include "llvm_forward.hpp"


struct LLVM_Tools;
struct Static_Evaluator;

using ErrorCode = short;

namespace resolver
{


struct Codegen {
  Codegen(cu::CU& _CU);


  llvm::LLVMContext&            ctx;
  std::unique_ptr<llvm::Module> __module;
  llvm::IRBuilder<>&            builder;

  std::unordered_map<std::string, llvm::Value*> locals;

  cu::CU&           CU;
  Static_Evaluator& eval;

  LLVM_Tools& tools;

  llvm::Function* init_func = nullptr;

  mutable std::vector<std::string> errors;

  size_t start_resolver();

  void visit(ast::Root& n);

  llvm::Module* mod;

  // llvm types
  llvm::Type* const u0Ty;
  llvm::Type* const i1Ty;
  llvm::Type* const i8Ty;
  llvm::Type* const i16Ty;
  llvm::Type* const i32Ty;
  llvm::Type* const i64Ty;
  llvm::Type* const i128Ty;
  llvm::Type* const iSizeTy;

  llvm::Type* const f16Ty;
  llvm::Type* const f32Ty;
  llvm::Type* const f64Ty;
  llvm::Type* const f80Ty;
  llvm::Type* const f128Ty;
  llvm::Type* const fSizeTy;

  llvm::StructType* const textTy;
  llvm::StructType* const strTy;
  llvm::Type* const       cstrTy;

  llvm::Constant* const get_zero;

  llvm::BasicBlock* current_bb_break;
  llvm::BasicBlock* current_bb_continue;

  /*
  void build_init_func();


  void error_add(ErrorCode code, const ast::Node& n, std::string_view msg, std::string_view hint) const;

  void error_two_lines(ErrorCode code, const ast::Node& first, const ast::Node& second, std::string_view msg,
                       std::string_view hint) const;

  llvm::Function* generate_stub(ast::type::Function_Proto& proto, std::string_view name,
                                llvm::Function::LinkageTypes link_ty);

  llvm::Value* ensure_rvalue(ast::AExpression& expr, std::string_view name = "");
  llvm::Value* ensure_lvalue(ast::AExpression& expr, bool is_silent_error = false);

  // ============ AST ============
  void visit(ast::Node& n);

  llvm::Type*  visit(ast::AType& n);
  void         visit(ast::ALiteral& n);
  void         visit(ast::ADeclaration& n);
  void         visit(ast::ALocal& n);
  void         visit(ast::AExpression& n);
  void         visit(ast::AIdentifier& n);
  llvm::Value* visit(ast::Expr_ID& n);
  llvm::Value* visit(ast::Expr_ID_Qualified& n);
  llvm::Value* visit(ast::Expr_ID_Type& n);
  llvm::Type*  visit_ty(ast::Expr_ID_Type& n);


  // ============ DECLARATION ============
  llvm::Value*    visit(ast::declaration::Global_Variable& n);
  llvm::Function* visit(ast::declaration::Function& n);

  void visit(ast::declaration::Mod& n);
  void visit(ast::declaration::Export& n);
  void visit(ast::declaration::Extern& n);

  llvm::Type* visit(ast::declaration::Enum& n);
  llvm::Type* visit(ast::declaration::Enum_Element& n);

  llvm::Type* visit(ast::declaration::Flag& n);
  llvm::Type* visit(ast::declaration::Union& n);

  void        visit(ast::declaration::Module_Alias& n);
  llvm::Type* visit(ast::declaration::Type_Alias& n);

  llvm::Type* visit(ast::declaration::Global_Generic& n);

  // ============ LOCAL ============
  void visit(CodeBlock& n);

  llvm::Function* visit(Local_Lambda& n);
  void            visit(Local_Lambda_Capture& n);
  void            visit(Local_Capture_Member& n);

  llvm::Value* visit(Local_Parameter& n);
  void         visit(Local_Generic_Parameter_Element& n);
  void         visit(Local_Generic_Parameters& n);

  llvm::Value* visit(Local_Pattern& n);
  llvm::Value* visit(Local_Pattern_Enum& n);
  llvm::Value* visit(Local_Pattern_Tuple& n);
  llvm::Value* visit(Local_Pattern_Form& n);
  llvm::Value* visit(Local_Pattern_Rule_Facet& n);
  llvm::Value* visit(Local_Pattern_Facet& n);

  llvm::Value* visit(Local_Variable_Binding& n);
  void         visit(Local_Tuple_Destructuring& n);
  llvm::Value* visit(Local_Variable& n);

  llvm::Value* visit(Local_Capability& n);

  // ============ SFM ============
  llvm::Type* visit(ast::declaration::sfm::Facet& n);
  llvm::Type* visit(ast::declaration::sfm::Facet_Field& n);

  llvm::Type* visit(ast::declaration::sfm::View& n);

  llvm::Type*     visit(ast::declaration::sfm::Form& n);
  llvm::Function* visit(ast::declaration::sfm::Form_New& n);
  llvm::Function* visit(ast::declaration::sfm::Form_Del& n);
  llvm::Function* visit(ast::declaration::sfm::Form_Cast& n);
  llvm::Function* visit(ast::declaration::sfm::Form_Op& n);
  llvm::Function* visit(ast::declaration::sfm::Form_Access_Op& n);
  llvm::Function* visit(ast::declaration::sfm::Form_Transfert& n);

  llvm::Function* visit(ast::declaration::sfm::Rule& n);
  void            visit(ast::declaration::sfm::Rule_Case& n);

  // ============ GENERIC ============
  llvm::Value* visit(ast::generic::Is_Type& n);
  llvm::Value* visit(ast::generic::Can_Cast& n);
  llvm::Value* visit(ast::generic::Have_Op& n);
  llvm::Value* visit(ast::generic::Have_View& n);
  llvm::Value* visit(ast::generic::Use_Facet& n);
  llvm::Value* visit(ast::generic::Compatible_Rule& n);

  // ============ TYPE ============
  llvm::Type* visit(ast::type::Ptr& n);
  llvm::Type* visit(ast::type::Table& n);
  llvm::Type* visit(ast::type::Primitive& n);
  llvm::Type* visit(ast::type::Tuple& n);
  llvm::Type* visit(ast::type::Function_Proto& n);

  llvm::Type* visit(ast::type::Get_Expr_Type& n);

  // ============ LITERAL ============
  llvm::Value* visit(ast::literal::Boolean& n);
  llvm::Value* visit(ast::literal::Integral& n);
  llvm::Value* visit(ast::literal::Fixed_Point& n);
  llvm::Value* visit(ast::literal::Floating_Point& n);

  llvm::Value* visit(ast::literal::CUNE& n);
  llvm::Value* visit(ast::literal::RUNE& n);

  llvm::Value* visit(ast::literal::Text_Pure& n);
  llvm::Value* visit(ast::literal::Text_Interpolation& n);
  llvm::Value* visit(ast::literal::Textual_Format& n);
  llvm::Value* visit(ast::literal::Format_Specifier& n);

  llvm::Value* visit(ast::literal::Table& n);
  llvm::Value* visit(ast::literal::Table_Population& n);

  llvm::Value* visit(ast::literal::Map& n);

  llvm::Value* visit(ast::literal::Tuple& n);

  llvm::Value* visit(ast::literal::Range& n);
  llvm::Value* visit(ast::literal::Iterator& n);

  llvm::Value* visit(ast::literal::Enum& n);

  llvm::Value* visit(ast::literal::Structured_Data& n);
  llvm::Value* visit(ast::literal::Form& n);

  // ============ Expression ============
  llvm::Value* visit(ast::expression::If_Ternary& n);

  llvm::Value* visit(ast::expression::Member_Access& n);

  llvm::Value* visit(ast::expression::Self& n);
  llvm::Value* visit(ast::expression::Other& n);

  llvm::Value* visit(ast::expression::Call& n);
  llvm::Value* visit(ast::expression::Call_Argument& n);
  llvm::Value* visit(ast::expression::Call_Rule& n);
  llvm::Value* visit(ast::expression::Call_Pipe& n);

  llvm::Value* visit(ast::expression::Table_Access& n);

  llvm::Value* visit(ast::expression::Ptr_At& n);
  llvm::Value* visit(ast::expression::Ptr_Offset& n);
  llvm::Value* visit(ast::expression::Ptr_Val& n);
  llvm::Value* visit(ast::expression::Mut_Of& n);
  llvm::Value* visit(ast::expression::Ref_Of& n);
  llvm::Value* visit(ast::expression::Addr_Of& n);
  llvm::Value* visit(ast::expression::Size_Of& n);
  llvm::Value* visit(ast::expression::GetBits& n);

  llvm::Value* visit(ast::expression::Move& n);
  llvm::Value* visit(ast::expression::New_Ptr& n);

  // ============ STATEMENT ============
  void visit(ast::statement::If& n);

  void         visit(ast::statement::For& n);
  void         visit(ast::statement::Loop& n);
  void         visit(ast::statement::While& n);
  void         visit(ast::statement::GoTo& n);
  llvm::Value* visit(ast::statement::GoTo_Label& n);

  llvm::ReturnInst* visit(ast::statement::Return& n);
  llvm::BranchInst* visit(ast::statement::Break& n);
  llvm::BranchInst* visit(ast::statement::Continue& n);

  void visit(ast::statement::Match& n);
  void visit(ast::statement::Match_Case& n);

  // ============ OPERATION ============
  llvm::Value* visit(ast::operation::Cast_As& n);
  llvm::Value* visit(ast::operation::Is& n);
  llvm::Value* visit(ast::operation::In& n);
  llvm::Value* visit(ast::operation::Assignment& n);
  llvm::Value* visit(ast::operation::Binary& n);
  llvm::Value* visit(ast::operation::Unary& n);
  llvm::Value* visit(ast::operation::Interval& n);
  llvm::Value* visit(ast::operation::Ptr_Dist& n);

  // ============ MEMORY ============
  void visit(ast::memory::Del& n);
  void visit(ast::memory::Align& n);
  void visit(ast::memory::Drop& n);
  */
};

} // namespace resolver
