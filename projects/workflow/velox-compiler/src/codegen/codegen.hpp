
#pragma once

#include <functional>
#include <llvm/IR/IRBuilder.h>

#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

#include "nexus/ast/forward.hpp"
#include "llvm_forward.hpp"
#include "nexus/ids.hpp"
#include "resolver/resolver_base.hpp"


using ErrorCode = short;

namespace codegen
{
struct Tools;
struct Insurance;
struct Static_Evaluator;
struct Codegen_Type;

struct Codegen_AST : resolver::Base {
  Codegen_AST(cu::CU& p_CU, llvm::LLVMContext& p_ctx);


  compiler::EPhase current_EPhase() const override;


  std::unordered_map<ast::ID, llvm::Value*, ast::ID::Hash>      generation;
  std::unordered_map<ast::ID, llvm::BasicBlock*, ast::ID::Hash> gotos;
  std::unordered_map<ast::ID, llvm::ReturnInst*, ast::ID::Hash> returns;
  std::unordered_map<ast::ID, llvm::BranchInst*, ast::ID::Hash> branchs;

  llvm::LLVMContext& ctx;
  llvm::IRBuilder<>& builder;
  llvm::Module*      mod;

  Codegen_Type& types;


  std::unordered_map<std::string, llvm::Value*> locals;

  Static_Evaluator& eval;
  Insurance&        insurance;

  Tools& tools;

  llvm::Function* init_func = nullptr;

  mutable std::vector<std::string> errors;

  llvm::Function*   globals_ctor       = nullptr;
  llvm::BasicBlock* globals_ctor_entry = nullptr;

  size_t llvm_item_count = 0;

  llvm::BasicBlock* current_bb_break    = nullptr;
  llvm::BasicBlock* current_bb_continue = nullptr;

  [[nodiscard]] size_t start_codegen() noexcept;

  void build_init_func() noexcept;

  [[nodiscard]] llvm::Constant* const_int(size_t val) noexcept;

  [[nodiscard]] llvm::Type*     get_type(type::ID tyid) noexcept;
  [[nodiscard]] llvm::Value*    add_generation(ast::ID nodeid, llvm::Value* v) noexcept;
  [[nodiscard]] llvm::Constant* add_generation(ast::ID nodeid, llvm::Constant* v) noexcept;
  [[nodiscard]] bool            is_generated(ast::ID nodeid) const noexcept;

  // inner type, data ptr
  [[nodiscard]] std::pair<llvm::Type*, llvm::Value*> codegen_collection_data_ptr(ast::ID nodeid) noexcept;

  [[nodiscard]] llvm::Function* codegen_globals_ctor() noexcept;
  void                          codegen_inject_global_init(std::function<void()> f) noexcept;
  void                          finalize_globals_ctor() noexcept;


  [[nodiscard]] llvm::Function* generate_stub(llvm::FunctionType* fn_ty, std::string_view name,
                                              llvm::Function::LinkageTypes link_ty) noexcept;

  [[nodiscard]] llvm::AllocaInst* create_alloca(type::ID tyid, std::string_view name) noexcept;

  // Nodes
  [[nodiscard]] llvm::Value* codegen_node(ast::ID nodeid) noexcept;

  void codegen_Root(const ast::Root& n) noexcept;

  [[nodiscard]] llvm::Value* codegen_Symbol_Id(const ast::Symbol_Id& n) noexcept;
  [[nodiscard]] llvm::Value* codegen_Symbol_Qualified(const ast::Symbol_Qualified& n) noexcept;
  [[nodiscard]] llvm::Type*  codegen_Symbol_Type(const ast::Symbol_Type& n) noexcept;


  [[nodiscard]] llvm::Value* codegen_Global_Variable(const ast::Global_Variable& n) noexcept;
  [[nodiscard]] llvm::Value* codegen_Global_Function(const ast::Global_Function& n) noexcept;
  [[nodiscard]] llvm::Value* codegen_Global_Extend_Fn(const ast::Global_Extend_Fn& n) noexcept;
  [[nodiscard]] llvm::Value* codegen_Global_Extend_Cast(const ast::Global_Extend_Cast& n) noexcept;
  [[nodiscard]] llvm::Value* codegen_Global_Extend_Op_Bin(const ast::Global_Extend_Op_Bin& n) noexcept;
  [[nodiscard]] llvm::Value* codegen_Global_Extend_Op_Un(const ast::Global_Extend_Op_Un& n) noexcept;
  [[nodiscard]] llvm::Value* codegen_Global_Extend_Op_Subscript(const ast::Global_Extend_Op_Subscript& n) noexcept;
  [[nodiscard]] llvm::Value* codegen_Global_Extend_Op_Transfert(const ast::Global_Extend_Op_Transfert& n) noexcept;
  [[nodiscard]] llvm::Value* codegen_Global_Extend_Op_Other(const ast::Global_Extend_Op_Other& n) noexcept;
  void                       codegen_Global_Module(const ast::Global_Module& n) noexcept;
  void                       codegen_Global_Extern(const ast::Global_Extern& n) noexcept;
  void                       codegen_Global_Export(const ast::Global_Export& n) noexcept;

  // generate default ctor
  [[nodiscard]] llvm::Function* codegen_SFM_Facet(const ast::SFM_Facet& n) noexcept;
  // generate default ctor
  [[nodiscard]] llvm::Function* codegen_SFM_Form(const ast::SFM_Form& n) noexcept;
  [[nodiscard]] llvm::Function* codegen_SFM_Rule(const ast::SFM_Rule& n) noexcept;
  [[nodiscard]] llvm::Value*    codegen_SFM_Rule_Case(const ast::SFM_Rule_Case& n) noexcept;

  void codegen_CodeBlock(const ast::CodeBlock& n) noexcept;

  [[nodiscard]] llvm::Value* codegen_Local_Lambda(const ast::Local_Lambda& n) noexcept;
  [[nodiscard]] llvm::Value* codegen_Local_Lambda_Capture(const ast::Local_Lambda_Capture& n) noexcept;
  [[nodiscard]] llvm::Value* codegen_Local_Parameter(const ast::Local_Parameter& n) noexcept;
  [[nodiscard]] llvm::Value* codegen_Local_Gen_Param_Elem(const ast::Local_Gen_Param_Elem& n) noexcept;
  [[nodiscard]] llvm::Value* codegen_Local_Gen_Params(const ast::Local_Gen_Params& n) noexcept;
  [[nodiscard]] llvm::Value* codegen_Local_Pattern_Element(const ast::Local_Pattern_Element& n) noexcept;
  [[nodiscard]] llvm::Value* codegen_Local_Pattern_Enum(const ast::Local_Pattern_Enum& n) noexcept;
  [[nodiscard]] llvm::Value* codegen_Local_Pattern_Tuple(const ast::Local_Pattern_Tuple& n) noexcept;
  [[nodiscard]] llvm::Value* codegen_Local_Pattern_Form(const ast::Local_Pattern_Form& n) noexcept;
  [[nodiscard]] llvm::Value* codegen_Local_Pattern_Rule_Facet(const ast::Local_Pattern_Rule_Facet& n) noexcept;
  [[nodiscard]] llvm::Value* codegen_Local_Pattern_Facet(const ast::Local_Pattern_Facet& n) noexcept;
  [[nodiscard]] llvm::Value* codegen_Local_Binding(const ast::Local_Binding& n) noexcept;
  void                       codegen_Local_Tuple_Destructuring(const ast::Local_Tuple_Destructuring& n) noexcept;
  [[nodiscard]] llvm::Value* codegen_Local_Variable(const ast::Local_Variable& n) noexcept;
  [[nodiscard]] llvm::Value* codegen_Local_Capability(const ast::Local_Capability& n) noexcept;

  [[nodiscard]] llvm::Constant* codegen_Literal_Boolean(const ast::Literal_Boolean& n) noexcept;
  [[nodiscard]] llvm::Constant* codegen_Literal_NullPtr(const ast::Literal_NullPtr& n) noexcept;
  [[nodiscard]] llvm::Constant* codegen_Literal_Integral(const ast::Literal_Integral& n) noexcept;
  [[nodiscard]] llvm::Constant* codegen_Literal_Fixed_Point(const ast::Literal_Fixed_Point& n) noexcept;
  [[nodiscard]] llvm::Constant* codegen_Literal_Floating_Point(const ast::Literal_Floating_Point& n) noexcept;
  [[nodiscard]] llvm::Constant* codegen_Literal_Cune(const ast::Literal_Cune& n) noexcept;
  [[nodiscard]] llvm::Constant* codegen_Literal_Rune(const ast::Literal_Rune& n) noexcept;
  [[nodiscard]] llvm::Constant* codegen_Literal_Text_Pure(const ast::Literal_Text_Pure& n) noexcept;
  [[nodiscard]] llvm::Value*    codegen_Literal_Text_Interpolation(const ast::Literal_Text_Interpolation& n) noexcept;
  [[nodiscard]] llvm::Value*    codegen_Literal_Textual_Format(const ast::Literal_Textual_Format& n) noexcept;
  [[nodiscard]] llvm::Value*    codegen_Literal_Format_Specifier(const ast::Literal_Format_Specifier& n) noexcept;
  [[nodiscard]] llvm::ConstantArray* codegen_Literal_Table(const ast::Literal_Table& n) noexcept;
  [[nodiscard]] llvm::Value*         codegen_Literal_Table_Population(const ast::Literal_Table_Population& n) noexcept;
  [[nodiscard]] llvm::Value*         codegen_Literal_Map(const ast::Literal_Map& n) noexcept;
  [[nodiscard]] llvm::Value*         codegen_Literal_Tuple(const ast::Literal_Tuple& n) noexcept;
  [[nodiscard]] llvm::Value*         codegen_Literal_Range(const ast::Literal_Range& n) noexcept;
  [[nodiscard]] llvm::Value*         codegen_Literal_Record(const ast::Literal_Record& n) noexcept;

  [[nodiscard]] llvm::Value* codegen_Expression_If_Ternary(const ast::Expression_If_Ternary& n) noexcept;
  [[nodiscard]] llvm::Value* codegen_Expression_Member_Access(const ast::Expression_Member_Access& n) noexcept;
  [[nodiscard]] llvm::Value* codegen_Expression_Self(const ast::Expression_Self& n) noexcept;
  [[nodiscard]] llvm::Value* codegen_Expression_Other(const ast::Expression_Other& n) noexcept;
  [[nodiscard]] llvm::Value* codegen_Expression_Invocation(const ast::Expression_Invocation& n) noexcept;
  [[nodiscard]] llvm::Value* codegen_Expression_Invocation_Arg(const ast::Expression_Invocation_Arg& n) noexcept;
  [[nodiscard]] llvm::Value* codegen_Expression_Invocation_Extend(const ast::Expression_Invocation_Extend& n) noexcept;
  [[nodiscard]] llvm::Value* codegen_Expression_Invocation_Rule(const ast::Expression_Invocation_Rule& n) noexcept;
  [[nodiscard]] llvm::Value* codegen_Expression_Table_Access(const ast::Expression_Table_Access& n) noexcept;
  [[nodiscard]] llvm::Value* codegen_Expression_Ptr_Val(const ast::Expression_Ptr_Val& n) noexcept;
  [[nodiscard]] llvm::Value* codegen_Expression_Mut_Of(const ast::Expression_Mut_Of& n) noexcept;
  [[nodiscard]] llvm::Value* codegen_Expression_Ref_Of(const ast::Expression_Ref_Of& n) noexcept;
  [[nodiscard]] llvm::Value* codegen_Expression_Move_Of(const ast::Expression_Move_Of& n) noexcept;
  [[nodiscard]] llvm::Value* codegen_Expression_Copy_Of(const ast::Expression_Copy_Of& n) noexcept;
  [[nodiscard]] llvm::Value* codegen_Expression_Addr_Of(const ast::Expression_Addr_Of& n) noexcept;
  [[nodiscard]] llvm::Value* codegen_Expression_Size_Of(const ast::Expression_Size_Of& n) noexcept;
  [[nodiscard]] llvm::Value* codegen_Expression_GetBits(const ast::Expression_GetBits& n) noexcept;
  [[nodiscard]] llvm::Value* codegen_Expression_New_Ptr(const ast::Expression_New_Ptr& n) noexcept;
  [[nodiscard]] llvm::Value* codegen_Expression_Get_Type(const ast::Expression_Get_Type& n) noexcept;

  void codegen_Statement_If(const ast::Statement_If& n, llvm::BasicBlock* bb_parent_merge = nullptr) noexcept;
  void codegen_Statement_Elif(const ast::Statement_If& n, llvm::BasicBlock* bb_parent_merge = nullptr) noexcept;
  void codegen_Statement_Else(const ast::Statement_If& n, llvm::BasicBlock* bb_parent_merge = nullptr) noexcept;
  void codegen_Statement_For(const ast::Statement_For& n) noexcept;
  void codegen_Statement_For_Index(const ast::Statement_For& n) noexcept;
  void codegen_Statement_For_Items(const ast::Statement_For& n) noexcept;
  void codegen_Statement_Loop(const ast::Statement_Loop& n) noexcept;
  void codegen_Statement_While(const ast::Statement_While& n) noexcept;
  void codegen_Statement_While_Classic(const ast::Statement_While& n) noexcept;
  void codegen_Statement_While_Do(const ast::Statement_While& n) noexcept;

  void                            codegen_Statement_GoTo(const ast::Statement_GoTo& n) noexcept;
  [[nodiscard]] llvm::BasicBlock* codegen_Statement_GoTo_Label(const ast::Statement_GoTo_Label& n) noexcept;
  [[nodiscard]] llvm::ReturnInst* codegen_Statement_Return(const ast::Statement_Return& n) noexcept;
  [[nodiscard]] llvm::BranchInst* codegen_Statement_Break(const ast::Statement_Break& n) noexcept;
  [[nodiscard]] llvm::BranchInst* codegen_Statement_Continue(const ast::Statement_Continue& n) noexcept;
  void                            codegen_Statement_Match(const ast::Statement_Match& n) noexcept;
  void                            codegen_Statement_Match_Case(const ast::Statement_Match_Case& n) noexcept;

  [[nodiscard]] llvm::Value* codegen_Operation_Cast_As(const ast::Operation_Cast_As& n) noexcept;
  [[nodiscard]] llvm::Value* codegen_Operation_Is(const ast::Operation_Is& n) noexcept;
  [[nodiscard]] llvm::Value* codegen_Operation_In(const ast::Operation_In& n) noexcept;
  [[nodiscard]] llvm::Value* codegen_Operation_Transfert(const ast::Operation_Transfert& n) noexcept;
  [[nodiscard]] llvm::Value* codegen_Operation_Binary(const ast::Operation_Binary& n) noexcept;
  [[nodiscard]] llvm::Value* codegen_Operation_Unary(const ast::Operation_Unary& n) noexcept;
  [[nodiscard]] llvm::Value* codegen_Operation_Interval(const ast::Operation_Interval& n) noexcept;
};


} // namespace codegen
