#include "codegen.hpp"

#include "ast/ast_base.hpp"
#include "ast/ast_declaration_extension.hpp"
#include "ast/ast_declaration_global.hpp"
#include "ast/ast_declaration_local.hpp"
#include "ast/ast_declaration_sfm.hpp"
#include "ast/ast_expression.hpp"
#include "ast/ast_literal.hpp"
#include "ast/ast_operation.hpp"
#include "ast/ast_statement.hpp"
#include "codegen/codegen_insurance.hpp"
#include "codegen/codegen_type.hpp"
#include "codegen_tools.hpp"
#include "compiler/compilation_unit.hpp"
#include "compiler/compiler.hpp"
#include "nexus/ast/ast.hpp"
#include "nexus/ast/data.hpp"
#include "nexus/ast/definition.hpp"
#include "nexus/ast/forward.hpp"
#include "nexus/extension.hpp"
#include "nexus/forward.hpp"
#include "nexus/ids.hpp"
#include "nexus/inference.hpp"
#include "nexus/resolved.hpp"
#include "nexus/type/data.hpp"
#include "nexus/type/definition.hpp"
#include "nexus/type/type.hpp"
#include "pipeline/pipeline.hpp"
#include "resolver/resolver_base.hpp"
#include "static_evaluation.hpp"

#include <common/compiler_options.hpp>
#include <cstdint>
#include <cstring>
#include <functional>
#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/APInt.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/Attributes.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/Utils/ModuleUtils.h>
#include <string_view>
#include <vector>



#define NOT_DEFINED assert(false);

#define GENERATION_GUARD                                                                                               \
  if (auto it = generation.find(n.nodeid()); it != generation.end()) return it->second;


codegen::Codegen_AST::Codegen_AST(cu::CU& p_CU, llvm::LLVMContext& p_ctx)
  : resolver::Base(p_CU)
  , ctx(p_ctx)
  , types(*new codegen::Codegen_Type(p_CU, p_ctx))
  , mod(CU.llvm_module)
  , builder(*new llvm::IRBuilder<>(ctx))
  , tools(*new Tools(*this))
  , eval(*new Static_Evaluator(*this))
  , insurance(*new Insurance(*this))
{
  types.res = this;
}


compiler::EPhase codegen::Codegen_AST::current_EPhase() const
{
  return compiler::EPhase::llvmir;
}


size_t codegen::Codegen_AST::start_codegen() noexcept
{
  types.init_llvm_types();
  types.prepare_codegen_types();

  //(void)codegen_node(CU.nodes->get_file_root()->nodeid());

  (void)codegen_node(CU.ast->get_file_root()->nodeid());

  finalize_globals_ctor();

  return llvm_item_count;
}

llvm::Value* codegen::Codegen_AST::codegen_node(ast::ID nodeid) noexcept
{
  const auto kind = nodeid.kind();

#define case_node(name)                                                                                                \
  case ast::ENodeKind::name: return codegen_##name(*nodeid.as<ast::name>());

#define case_node_noret(name)                                                                                          \
  case ast::ENodeKind::name: (void)codegen_##name(*nodeid.as<ast::name>()); return nullptr;

  switch (kind) {
    case_node(Global_Variable);
    case_node(Global_Function);
    case_node(Global_Extend_Fn);
    case_node(Global_Extend_Cast);
    case_node(Global_Extend_Op_Bin);
    case_node(Global_Extend_Op_Un);
    case_node(Global_Extend_Op_Subscript);
    case_node(Global_Extend_Op_Transfert);
    case_node(Global_Extend_Op_Other);
    case_node_noret(Global_Module);
    case_node_noret(Global_Extern);
    case_node_noret(Global_Export);
    case_node_noret(CodeBlock);
    case_node(Symbol_Id);
    case_node(Symbol_Qualified);
    case_node_noret(Symbol_Type);
    case_node_noret(Root);
    case_node(Local_Lambda);
    case_node(Local_Lambda_Capture);
    case_node(Local_Parameter);
    case_node(Local_Gen_Param_Elem);
    case_node(Local_Gen_Params);
    case_node(Local_Pattern_Element);
    case_node(Local_Pattern_Enum);
    case_node(Local_Pattern_Tuple);
    case_node(Local_Pattern_Form);
    case_node(Local_Pattern_Rule_Facet);
    case_node(Local_Pattern_Facet);
    case_node(Local_Binding);
    case_node_noret(Local_Tuple_Destructuring);
    case_node(Local_Variable);
    case_node(Local_Capability);
    case_node(SFM_Facet);
    case_node(SFM_Form);
    case_node(SFM_Rule);
    case_node(SFM_Rule_Case);
    case_node(Literal_Boolean);
    case_node(Literal_NullPtr);
    case_node(Literal_Integral);
    case_node(Literal_Fixed_Point);
    case_node(Literal_Floating_Point);
    case_node(Literal_Cune);
    case_node(Literal_Rune);
    case_node(Literal_Text_Pure);
    case_node(Literal_Text_Interpolation);
    case_node(Literal_Textual_Format);
    case_node(Literal_Format_Specifier);
    case_node(Literal_Table);
    case_node(Literal_Table_Population);
    case_node(Literal_Map);
    case_node(Literal_Tuple);
    case_node(Literal_Range);
    case_node(Literal_Record);
    case_node(Expression_If_Ternary);
    case_node(Expression_Member_Access);
    case_node(Expression_Self);
    case_node(Expression_Other);
    case_node(Expression_Invocation);
    case_node(Expression_Invocation_Arg);
    case_node(Expression_Invocation_Extend);
    case_node(Expression_Invocation_Rule);
    case_node(Expression_Table_Access);
    case_node(Expression_Ptr_Val);
    case_node(Expression_Mut_Of);
    case_node(Expression_Ref_Of);
    case_node(Expression_Move_Of);
    case_node(Expression_Copy_Of);
    case_node(Expression_Addr_Of);
    case_node(Expression_Size_Of);
    case_node(Expression_GetBits);
    case_node(Expression_New_Ptr);
    case_node(Expression_Get_Type);
    case_node_noret(Statement_If);
    case_node_noret(Statement_For);
    case_node_noret(Statement_Loop);
    case_node_noret(Statement_While);
    case_node_noret(Statement_GoTo);
    case_node(Statement_GoTo_Label);
    case_node(Statement_Return);
    case_node(Statement_Break);
    case_node(Statement_Continue);
    case_node_noret(Statement_Match);
    case_node_noret(Statement_Match_Case);
    case_node(Operation_Cast_As);
    case_node(Operation_Is);
    case_node(Operation_In);
    case_node(Operation_Transfert);
    case_node(Operation_Binary);
    case_node(Operation_Unary);
    case_node(Operation_Interval);
  case ast::ENodeKind::NONE:
  case ast::ENodeKind::Path_Regex:
  case ast::ENodeKind::Import:
  case ast::ENodeKind::Global_Enum:
  case ast::ENodeKind::Global_Flag:
  case ast::ENodeKind::Global_Union:
  case ast::ENodeKind::Enum_Field:
  case ast::ENodeKind::Flag_Field:
  case ast::ENodeKind::Union_Field:
  case ast::ENodeKind::SFM_Facet_Field:
  case ast::ENodeKind::SFM_View:
  case ast::ENodeKind::Generic_Type:
  case ast::ENodeKind::Generic_Cast:
  case ast::ENodeKind::Generic_Op:
  case ast::ENodeKind::Generic_View:
  case ast::ENodeKind::Generic_Facet:
  case ast::ENodeKind::Generic_Extension:
  case ast::ENodeKind::Generic_Rule:
  case ast::ENodeKind::Memory_Del:
  case ast::ENodeKind::Memory_Align:
  case ast::ENodeKind::Memory_Drop:
  case ast::ENodeKind::Global_Reexport:
  case ast::ENodeKind::Global_Alias_Type:
  case ast::ENodeKind::Global_Alias_Module:
  case ast::ENodeKind::Global_Generic:      break;
  }

#undef case_node_noret
#undef case_node

  return nullptr;
}


llvm::Type* codegen::Codegen_AST::get_type(type::ID tyid) noexcept
{
  if (auto* ty = types.get(tyid)) return ty;

  return types.codegen_type(tyid);
}
llvm::Value* codegen::Codegen_AST::add_generation(ast::ID nodeid, llvm::Value* v) noexcept
{
  assert(v && "Invalid llvm value");

  generation.try_emplace(nodeid, v);
  llvm_item_count++;
  return v;
}
llvm::Constant* codegen::Codegen_AST::add_generation(ast::ID nodeid, llvm::Constant* v) noexcept
{
  assert(v && "Invalid llvm constant");

  generation.try_emplace(nodeid, v);
  llvm_item_count++;
  return v;
}
bool codegen::Codegen_AST::is_generated(ast::ID nodeid) const noexcept
{
  return generation.contains(nodeid);
}

std::pair<llvm::Type*, llvm::Value*> codegen::Codegen_AST::codegen_collection_data_ptr(ast::ID nodeid) noexcept
{
  auto  tyid = nodeid.type();
  auto* ptr  = codegen_node(nodeid);
  auto* ty   = get_type(tyid);

  // layout { ptr, i64 }
  if (const auto* typtr = tyid.as<type::Array>()) {
    return {get_type(typtr->inner), builder.CreateStructGEP(ty, ptr, 0, "array.data")};
  }
  // layout { ptr, i64, i64 }
  if (const auto* typtr = tyid.as<type::Buffer>()) {
    return {get_type(typtr->inner), builder.CreateStructGEP(ty, ptr, 0, "buffer.data")};
  }
  // layout { ptr, i64 }
  if (const auto* typtr = tyid.as<type::Slice>()) {
    return {get_type(typtr->inner), builder.CreateStructGEP(ty, ptr, 0, "slice.data")};
  }
  // layout ptr
  if (const auto* typtr = tyid.as<type::Ptr>()) {
    return {get_type(typtr->inner), ptr};
  }

  for (auto ext : tyid.extensions()) {
    if (const auto* ext_ptr = ext.as<ast::Global_Extend_Cast>()) {
      // layout { ptr, i64 }
      if (const auto* typtr = ext_ptr->as_type.as<type::Array>()) {
        auto* ext_ptr = codegen_node(ext);
        return {get_type(typtr->inner), builder.CreateStructGEP(ty, ptr, 0, "table.data")};
      }
      // layout { ptr, i64, i64 }
      if (const auto* typtr = ext_ptr->as_type.as<type::Buffer>()) {
        auto* ext_ptr = codegen_node(ext);
        return {get_type(typtr->inner), builder.CreateStructGEP(ty, ptr, 0, "table.data")};
      }
      // layout { ptr, i64 }
      if (const auto* typtr = ext_ptr->as_type.as<type::Slice>()) {
        auto* ext_ptr = codegen_node(ext);
        return {get_type(typtr->inner), builder.CreateStructGEP(ty, ptr, 0, "table.data")};
      }
    }
  }

  assert(false);
}

llvm::Function* codegen::Codegen_AST::codegen_globals_ctor() noexcept
{
  if (globals_ctor) return globals_ctor;

  auto* fnTy = llvm::FunctionType::get(llvm::Type::getVoidTy(ctx), false);

  auto* fn = llvm::Function::Create(fnTy, llvm::GlobalValue::InternalLinkage, "__globals_ctor", mod);

  auto* entry = llvm::BasicBlock::Create(ctx, "entry", fn);

  globals_ctor       = fn;
  globals_ctor_entry = entry;

  llvm::appendToGlobalCtors(*mod, fn, 65535);

  return fn;
}

void codegen::Codegen_AST::codegen_inject_global_init(std::function<void()> f) noexcept
{
  auto* ctor = codegen_globals_ctor();

  auto ip = builder.saveIP();

  builder.SetInsertPoint(globals_ctor_entry);

  f();

  builder.restoreIP(ip);
}

void codegen::Codegen_AST::finalize_globals_ctor() noexcept
{
  if (!globals_ctor) return;

  auto ip = builder.saveIP();

  builder.SetInsertPoint(globals_ctor_entry);

  if (!globals_ctor_entry->getTerminator()) builder.CreateRetVoid();

  builder.restoreIP(ip);
}


void codegen::Codegen_AST::build_init_func() noexcept
{
  if (init_func) return;

  auto* ty = llvm::FunctionType::get(codegen::LLVM_TYPEID_u0, false);
  auto* fn = llvm::Function::Create(ty, llvm::Function::ExternalLinkage, "init_module_" + mod->getName(), *mod);

  auto*             bb = llvm::BasicBlock::Create(ctx, "entry", fn);
  llvm::IRBuilder<> irb(bb);

  init_func = fn;
}

llvm::Constant* codegen::Codegen_AST::const_int(size_t val) noexcept
{
  return llvm::ConstantInt::get(llvm::Type::getIntNTy(ctx, compiler::OPTIONS.target.get_arch_size()), val);
}


llvm::AllocaInst* codegen::Codegen_AST::create_alloca(type::ID tyid, std::string_view name) noexcept
{
  auto* ty = get_type(tyid);
  return builder.CreateAlloca(ty, nullptr, name);
}


llvm::Function* codegen::Codegen_AST::generate_stub(llvm::FunctionType* fn_ty, std::string_view name,
                                                    llvm::Function::LinkageTypes link_ty) noexcept
{
  if (auto* func = mod->getFunction(name)) return func;

  auto* fn = llvm::Function::Create(fn_ty, link_ty, name, *mod);
  return fn;
}

void codegen::Codegen_AST::codegen_Root(const ast::Root& n) noexcept
{
  for (auto elem : n.global_nodes) (void)codegen_node(elem);
}

llvm::Value* codegen::Codegen_AST::codegen_Symbol_Id(const ast::Symbol_Id& n) noexcept
{
  GENERATION_GUARD

  // auto* v = insurance.ensure_lvalue(compiler::resolved.get_symbol(n.nodeid()).node());
  auto* v = codegen_node(compiler::resolved.get_definition(n.nodeid()).node());
  return add_generation(n.nodeid(), v);
}
llvm::Value* codegen::Codegen_AST::codegen_Symbol_Qualified(const ast::Symbol_Qualified& n) noexcept
{
  GENERATION_GUARD

  auto* v = codegen_node(compiler::resolved.get_definition(n.nodeid()).node());
  return add_generation(n.nodeid(), v);
}
llvm::Type* codegen::Codegen_AST::codegen_Symbol_Type(const ast::Symbol_Type& n) noexcept
{
  const auto tyid = compiler::inference.get_inference(n.nodeid());
  assert(tyid && "Invalid type");
  return get_type(tyid);
}


llvm::Value* codegen::Codegen_AST::codegen_Global_Variable(const ast::Global_Variable& n) noexcept
{
  GENERATION_GUARD

  auto* ty = get_type(n.nodeid().type());

  if (!n.extern_abi.empty()) {
    auto* ty = get_type(n.nodeid().type());
    auto* v  = new llvm::GlobalVariable(*mod, ty, true, llvm::GlobalVariable::ExternalLinkage, nullptr, n.name);
    return add_generation(n.nodeid(), v);
  }

  llvm::Function::LinkageTypes linkage = n.visibility == EVisibility::Cross_File_Scope || !n.extern_abi.empty()
                                             ? llvm::Function::ExternalLinkage
                                             : llvm::Function::InternalLinkage;

  const bool is_val_static = n.kind == ast::EVariableKind::_const;

  if (is_val_static) {
    auto result = eval.evaluate_expression(n.expression);
    if (result) return add_generation(n.nodeid(), result.value());

    compiler::COMPILER.add_error(
        Error_Diagnostic(CU.cuid, 278, n.nodeid(), compiler::EPhase::llvmir, result.error(), ""));
    return nullptr;
  }

  const bool is_const = n.kind != ast::EVariableKind::_var;

  // prohibied for static table : init enforced
  if (n.is_uninit) {
    auto* v = new llvm::GlobalVariable(*mod, ty, is_const, linkage, nullptr, n.name);

    return add_generation(n.nodeid(), v);
  }

  if (n.expression) {
    if (auto out = eval.evaluate_expression(n.expression)) {
      auto* v = new llvm::GlobalVariable(*mod, ty, is_const, linkage, out.value(), n.name);
      assert(v->getValueType() == out.value()->getType() && "Not the same type");
      return add_generation(n.nodeid(), v);
    }

    auto* v = new llvm::GlobalVariable(*mod, ty, is_const, linkage, nullptr, n.name);

    codegen_inject_global_init([&]() {
      auto* expr = codegen_node(n.expression);
      assert(v->getValueType() == expr->getType() && "Not the same type");
      builder.CreateStore(expr, v);
    });

    return add_generation(n.nodeid(), v);
  }


  auto* init = insurance.ensure_null_global_value(n.type.canonical());
  auto* v    = new llvm::GlobalVariable(*mod, ty, is_const, linkage, init, n.name);
  assert(v->getValueType() == init->getType() && "Not the same type");

  return add_generation(n.nodeid(), v);
}


// ============ FUNCTION ============
llvm::Value* codegen::Codegen_AST::codegen_Global_Function(const ast::Global_Function& n) noexcept
{
  GENERATION_GUARD

  const bool is_main = n.name == "main";

  llvm::Function* fn = nullptr;

  llvm::Function::LinkageTypes linkage = n.visibility == EVisibility::Cross_File_Scope || !n.extern_abi.empty()
                                             ? llvm::Function::ExternalLinkage
                                             : llvm::Function::InternalLinkage;

  llvm::FunctionType* fn_ty = nullptr;

  // special main function case
  if (is_main) {
    linkage = llvm::Function::ExternalLinkage;
    std::vector<llvm::Type*> param_tys;
    for (const auto& param : n.prototype.as<type::Prototype>()->params) param_tys.emplace_back(get_type(param.type));
    fn_ty = llvm::FunctionType::get(codegen::LLVM_TYPEID_s32, param_tys, false);
  } else {
    fn_ty = llvm::cast<llvm::FunctionType>(get_type(n.prototype));
  }

  fn = generate_stub(fn_ty, n.name, linkage);
  fn->setLinkage(linkage);

  // param attributes
  size_t count = 0;
  for (auto& arg : fn->args()) {
    if (count >= n.parameters.size()) break;

    const auto* param = n.parameters[count].as<ast::Local_Parameter>();
    assert(param);
    arg.setName(param->name);
    if (arg.getType()->isPointerTy()) arg.addAttr(llvm::Attribute::NoAlias);
    (void)add_generation(n.parameters[count], &arg);

    count++;
  }


  // add generation before codeblock generation to access on the function header generation for the parameters
  (void)add_generation(n.nodeid(), fn);

  if (!n.codeblock) return fn;

  auto* entry = llvm::BasicBlock::Create(ctx, "entry", fn);
  builder.SetInsertPoint(entry);

  (void)codegen_node(n.codeblock);

  // TERMINATE BLOCKS PROPERLY
  if (!builder.GetInsertBlock()->getTerminator()) {
    if (fn_ty->getReturnType()->isVoidTy())
      builder.CreateRetVoid();
    else
      builder.CreateRet(llvm::Constant::getNullValue(fn_ty->getReturnType()));
  }

  return fn;
}

llvm::Value* codegen::Codegen_AST::codegen_Global_Extend_Fn(const ast::Global_Extend_Fn& n) noexcept {
    NOT_DEFINED} llvm::Value* codegen::Codegen_AST::codegen_Global_Extend_Cast(const ast::Global_Extend_Cast&
                                                                                   n) noexcept {
    NOT_DEFINED} llvm::Value* codegen::Codegen_AST::codegen_Global_Extend_Op_Bin(const ast::Global_Extend_Op_Bin&
                                                                                     n) noexcept {
    NOT_DEFINED} llvm::Value* codegen::Codegen_AST::codegen_Global_Extend_Op_Un(const ast::Global_Extend_Op_Un&
                                                                                    n) noexcept {NOT_DEFINED} llvm::
    Value* codegen::Codegen_AST::codegen_Global_Extend_Op_Subscript(const ast::Global_Extend_Op_Subscript& n) noexcept {
        NOT_DEFINED} llvm::Value* codegen::Codegen_AST::
        codegen_Global_Extend_Op_Transfert(const ast::Global_Extend_Op_Transfert& n) noexcept {NOT_DEFINED} llvm::
            Value* codegen::Codegen_AST::codegen_Global_Extend_Op_Other(const ast::Global_Extend_Op_Other& n) noexcept
{
  NOT_DEFINED
}

void codegen::Codegen_AST::codegen_Global_Module(const ast::Global_Module& n) noexcept
{
  (void)codegen_node(n.codeblock);
}
void codegen::Codegen_AST::codegen_Global_Export(const ast::Global_Export& n) noexcept
{
  (void)codegen_node(n.codeblock);
}
void codegen::Codegen_AST::codegen_Global_Extern(const ast::Global_Extern& n) noexcept
{
  (void)codegen_node(n.codeblock);
}

llvm::Function* codegen::Codegen_AST::codegen_SFM_Facet(const ast::SFM_Facet& n) noexcept
{
  static llvm::Function* ctor = nullptr;
  if (ctor) return ctor;

  auto* ty = get_type(n.nodeid().type());

  auto* ctor_ty = llvm::FunctionType::get(codegen::LLVM_TYPEID_u0, {ty->getPointerTo()}, false);
  ctor          = llvm::Function::Create(ctor_ty, llvm::Function::ExternalLinkage, n.name + "_default_ctor", mod);

  auto* entry = llvm::BasicBlock::Create(ctx, "entry", ctor);
  builder.SetInsertPoint(entry);

  auto* this_ptr = ctor->getArg(0);

  unsigned offset = 0;
  for (auto fid : n.fields) {
    auto* f = fid.as<ast::SFM_Facet_Field>();
    assert(f && "Invalid ast facial kind");

    if (f->is_no_default) {
      offset++;
      continue;
    }

    auto* f_ptr = builder.CreateStructGEP(ty, this_ptr, offset, f->name + "_" + std::to_string(offset));
    auto* f_val = codegen_node(f->default_value);
    builder.CreateStore(f_val, f_ptr);

    offset++;
  }

  builder.CreateRetVoid();

  return ctor;
}

llvm::Function* codegen::Codegen_AST::codegen_SFM_Form(const ast::SFM_Form& n) noexcept
{
  static llvm::Function* ctor = nullptr;
  if (ctor) return ctor;

  auto* ty = get_type(n.nodeid().type());

  auto* ctor_ty = llvm::FunctionType::get(codegen::LLVM_TYPEID_u0, {ty->getPointerTo()}, false);
  ctor          = llvm::Function::Create(ctor_ty, llvm::Function::ExternalLinkage, n.name + "_default_ctor", mod);

  auto* entry = llvm::BasicBlock::Create(ctx, "entry", ctor);
  builder.SetInsertPoint(entry);

  auto* this_ptr = ctor->getArg(0);

  unsigned offset = 0;
  for (auto fid : n.facets) {
    auto* f_def          = fid.def().node().as<ast::SFM_Facet>();
    auto* this_facet_ptr = builder.CreateStructGEP(ty, this_ptr, offset, f_def->name + "_" + std::to_string(offset));
    assert(f_def && "Invalid ast facial kind");

    auto* f_ctor = codegen_SFM_Facet(*f_def);

    builder.CreateCall(f_ctor, {this_facet_ptr});

    if (auto* lit_rec = fid.as<ast::Literal_Record>()) {
      auto* rec_val = codegen_Literal_Record(*lit_rec);
      builder.CreateStore(rec_val, this_facet_ptr);
    }

    offset++;
  }

  builder.CreateRetVoid();

  return ctor;
}


llvm::Function* codegen::Codegen_AST::codegen_SFM_Rule(const ast::SFM_Rule& n) noexcept {
    NOT_DEFINED} llvm::Value* codegen::Codegen_AST::codegen_SFM_Rule_Case(const ast::SFM_Rule_Case& n) noexcept
{
  NOT_DEFINED
}


// ============ LOCAL ============
void codegen::Codegen_AST::codegen_CodeBlock(const ast::CodeBlock& n) noexcept
{
  for (auto elem : n.elements) (void)codegen_node(elem);
}

llvm::Value* codegen::Codegen_AST::codegen_Local_Lambda(const ast::Local_Lambda& n) noexcept {
    NOT_DEFINED} llvm::Value* codegen::Codegen_AST::codegen_Local_Lambda_Capture(const ast::Local_Lambda_Capture&
                                                                                     n) noexcept {
    NOT_DEFINED} llvm::Value* codegen::Codegen_AST::codegen_Local_Parameter(const ast::Local_Parameter& n) noexcept
{
  GENERATION_GUARD

  auto* llvm_fn = llvm::cast<llvm::Function>(codegen_node(n.parent_callable));
  assert(llvm_fn);
  auto* arg = llvm_fn->getArg(n.position);
  assert(arg);

  return add_generation(n.nodeid(), arg);
}

llvm::Value* codegen::Codegen_AST::codegen_Local_Gen_Param_Elem(const ast::Local_Gen_Param_Elem& n) noexcept {
    NOT_DEFINED} llvm::Value* codegen::Codegen_AST::codegen_Local_Gen_Params(const ast::Local_Gen_Params& n) noexcept {
    NOT_DEFINED} llvm::Value* codegen::Codegen_AST::codegen_Local_Pattern_Element(const ast::Local_Pattern_Element&
                                                                                      n) noexcept {
    NOT_DEFINED} llvm::Value* codegen::Codegen_AST::codegen_Local_Pattern_Enum(const ast::Local_Pattern_Enum&
                                                                                   n) noexcept {
    NOT_DEFINED} llvm::Value* codegen::Codegen_AST::codegen_Local_Pattern_Tuple(const ast::Local_Pattern_Tuple&
                                                                                    n) noexcept {
    NOT_DEFINED} llvm::Value* codegen::Codegen_AST::codegen_Local_Pattern_Form(const ast::Local_Pattern_Form&
                                                                                   n) noexcept {NOT_DEFINED} llvm::
    Value* codegen::Codegen_AST::codegen_Local_Pattern_Rule_Facet(const ast::Local_Pattern_Rule_Facet& n) noexcept {
        NOT_DEFINED} llvm::Value* codegen::Codegen_AST::codegen_Local_Pattern_Facet(const ast::Local_Pattern_Facet&
                                                                                        n) noexcept {
        NOT_DEFINED} llvm::Value* codegen::Codegen_AST::codegen_Local_Binding(const ast::Local_Binding& n) noexcept
{
  NOT_DEFINED
}


void codegen::Codegen_AST::codegen_Local_Tuple_Destructuring(const ast::Local_Tuple_Destructuring& n) noexcept {
    NOT_DEFINED} llvm::Value* codegen::Codegen_AST::codegen_Local_Variable(const ast::Local_Variable& n) noexcept
{
  GENERATION_GUARD

  if (n.kind == ast::EVariableKind::_const) {
    if (n.is_uninit) {
      auto* ty       = get_type(n.nodeid().type());
      auto* constant = llvm::Constant::getNullValue(ty);
      return add_generation(n.nodeid(), constant);
    }
    if (auto ptr = eval.evaluate_expression(n.expression)) {
      return add_generation(n.nodeid(), ptr.value());
    }
  }

  if (n.kind == ast::EVariableKind::_let) {
    auto* v = codegen_node(n.expression);
    return add_generation(n.nodeid(), v);
  }

  auto* alloca = create_alloca(n.nodeid().type(), n.name);

  if (n.is_uninit) return add_generation(n.nodeid(), alloca);

  if (n.expression) {
    // auto* expr = insurance.ensure_rvalue(n.expression);
    // if (!expr) {
    //   add_error(227, *n.expression.get(), "Impossible to assign unevaluated an expression.", "");
    //   return nullptr;
    // }
    builder.CreateStore(codegen_node(n.expression), alloca, n.nodeid().type().get().qualifier.is_volatile);
  } else {
    auto* ty = get_type(n.nodeid().type());
    builder.CreateStore(llvm::Constant::getNullValue(ty), alloca);
  }


  return add_generation(n.nodeid(), alloca);
}

llvm::Value* codegen::Codegen_AST::codegen_Local_Capability(const ast::Local_Capability& n) noexcept
{
  GENERATION_GUARD

  llvm::AllocaInst* alloca = nullptr;
  switch (n.kind) {
  case ast::ECapability::NONE:
  case ast::ECapability::move:
  case ast::ECapability::ref:
  case ast::ECapability::mut:  {
    auto* ty = get_type(n.nodeid().type())->getPointerTo();
    alloca   = builder.CreateAlloca(ty, nullptr, n.name);
    break;
  }
  case ast::ECapability::copy: {
    auto* ty = get_type(n.nodeid().type());
    alloca   = builder.CreateAlloca(ty, nullptr, n.name);
    break;
  }
  }

  builder.CreateStore(codegen_node(n.expression), alloca);

  return add_generation(n.nodeid(), alloca);
}

#define CONSTANT_GUARD                                                                                                 \
  if (auto it = generation.find(n.nodeid()); it != generation.end()) return llvm::cast<llvm::Constant>(it->second);

llvm::Constant* codegen::Codegen_AST::codegen_Literal_Boolean(const ast::Literal_Boolean& n) noexcept
{
  CONSTANT_GUARD

  return add_generation(n.nodeid(), llvm::ConstantInt::get(codegen::LLVM_TYPEID_bool, n.val));
}
llvm::Constant* codegen::Codegen_AST::codegen_Literal_NullPtr(const ast::Literal_NullPtr& n) noexcept
{
  CONSTANT_GUARD

  return add_generation(n.nodeid(), llvm::ConstantPointerNull::getNullValue(get_type(n.nodeid().type())));
}
llvm::Constant* codegen::Codegen_AST::codegen_Literal_Integral(const ast::Literal_Integral& n) noexcept
{
  CONSTANT_GUARD

  if (auto ptr = eval.evaluate_expression(n.nodeid())) return add_generation(n.nodeid(), ptr.value());
  return nullptr;
}
llvm::Constant* codegen::Codegen_AST::codegen_Literal_Fixed_Point(const ast::Literal_Fixed_Point& n) noexcept
{
  CONSTANT_GUARD

  if (auto ptr = eval.evaluate_expression(n.nodeid())) return add_generation(n.nodeid(), ptr.value());
  return nullptr;
}
llvm::Constant* codegen::Codegen_AST::codegen_Literal_Floating_Point(const ast::Literal_Floating_Point& n) noexcept
{
  CONSTANT_GUARD

  if (auto ptr = eval.evaluate_expression(n.nodeid())) return add_generation(n.nodeid(), ptr.value());
  return nullptr;
}

llvm::Constant* codegen::Codegen_AST::codegen_Literal_Cune(const ast::Literal_Cune& n) noexcept
{
  CONSTANT_GUARD

  return add_generation(n.nodeid(), llvm::ConstantInt::get(codegen::LLVM_TYPEID_cune, n.val));
}
llvm::Constant* codegen::Codegen_AST::codegen_Literal_Rune(const ast::Literal_Rune& n) noexcept
{
  CONSTANT_GUARD

  assert(n.code_points.size() == 4 && "UTF-32 character must be 4 bytes");
  uint32_t codePoint = 0;
  for (int i = 0; i < 4; ++i)
    codePoint |= static_cast<uint32_t>(static_cast<unsigned char>(n.code_points[i])) << (8 * i);

  return add_generation(n.nodeid(), llvm::ConstantInt::get(codegen::LLVM_TYPEID_rune, codePoint));
}

llvm::Constant* codegen::Codegen_AST::codegen_Literal_Text_Pure(const ast::Literal_Text_Pure& n) noexcept
{
  CONSTANT_GUARD

  switch (n.text_type) {
  case type::ETextType::_cstr: {
    if (n.val.empty()) return add_generation(n.nodeid(), llvm::Constant::getNullValue(LLVM_TYPEID_cstr));
    return add_generation(n.nodeid(), tools.get_cstr_constant(n.val));
  }
  case type::ETextType::_cune: {
    if (n.val.empty()) return add_generation(n.nodeid(), llvm::Constant::getNullValue(LLVM_TYPEID_cune));
    auto  character = llvm::APInt(8, n.val[0]);
    auto* constant  = llvm::Constant::getIntegerValue(LLVM_TYPEID_cune, character);
    return add_generation(n.nodeid(), constant);
  }
  case type::ETextType::NONE:
  case type::ETextType::_str: {
    if (n.val.empty()) return add_generation(n.nodeid(), llvm::Constant::getNullValue(LLVM_TYPEID_str));
    return add_generation(n.nodeid(), tools.get_str_constant(n.val));
  }
  case type::ETextType::_rune: {
    if (n.val.empty()) return add_generation(n.nodeid(), llvm::Constant::getNullValue(LLVM_TYPEID_rune));
    std::u32string utf32;
    try {
      utf32 = tools.utf8_to_utf32(n.val);
    } catch (const std::exception& e) {
      add_error(224, n.header, e.what(), "");
    }
    auto  character = llvm::APInt(32, utf32[0]);
    auto* constant  = llvm::Constant::getIntegerValue(LLVM_TYPEID_rune, character);
    return add_generation(n.nodeid(), constant);
  }
  case type::ETextType::_text: {
    if (n.val.empty()) return add_generation(n.nodeid(), llvm::Constant::getNullValue(LLVM_TYPEID_text));
    std::u32string utf32;
    try {
      utf32 = tools.utf8_to_utf32(n.val);
    } catch (const std::exception& e) {
      add_error(224, n.header, e.what(), "");
    }
    return add_generation(n.nodeid(), tools.get_text_constant(utf32));
  }
  }

  return nullptr;
}
llvm::Value* codegen::Codegen_AST::codegen_Literal_Text_Interpolation(const ast::Literal_Text_Interpolation& n) noexcept
{
  return nullptr;
}
llvm::Value* codegen::Codegen_AST::codegen_Literal_Textual_Format(const ast::Literal_Textual_Format& n) noexcept
{
  GENERATION_GUARD

  if (n.values.size() == 1 && n.values[0].as<ast::Literal_Text_Pure>())
    return add_generation(n.nodeid(), codegen_node(n.values[0]));

  for (const auto& elem : n.values) {
    (void)codegen_node(elem);
  }
  return nullptr;
}
llvm::Value* codegen::Codegen_AST::codegen_Literal_Format_Specifier(const ast::Literal_Format_Specifier& n) noexcept
{
  return nullptr;
}

llvm::ConstantArray* codegen::Codegen_AST::codegen_Literal_Table(const ast::Literal_Table& n) noexcept
{
  return nullptr;
}
llvm::Value* codegen::Codegen_AST::codegen_Literal_Table_Population(const ast::Literal_Table_Population& n) noexcept
{
  return nullptr;
}

llvm::Value* codegen::Codegen_AST::codegen_Literal_Map(const ast::Literal_Map& n) noexcept
{
  return nullptr;
}
llvm::Value* codegen::Codegen_AST::codegen_Literal_Tuple(const ast::Literal_Tuple& n) noexcept
{
  GENERATION_GUARD

  std::vector<llvm::Type*>  types;
  std::vector<llvm::Value*> vals;

  for (const auto& f : n.fields) {
    vals.emplace_back(codegen_node(f.value));
    types.emplace_back(get_type(f.value.type()));
  }

  auto* tuple_ty = llvm::StructType::get(ctx, types);

  auto* ptr = builder.CreateAlloca(tuple_ty, nullptr, "literal.tuple");

  size_t count = 0;
  for (auto* val : vals) {
    auto* val_ptr = builder.CreateStructGEP(tuple_ty, ptr, count++);
    builder.CreateStore(val, val_ptr);
  }

  return add_generation(n.nodeid(), ptr);
}

llvm::Value* codegen::Codegen_AST::codegen_Literal_Range(const ast::Literal_Range& n) noexcept
{
  GENERATION_GUARD

  auto* start_ty = get_type(n.start.type());
  auto* end_ty   = get_type(n.end.type());

  auto* range_ty = llvm::StructType::get(ctx, {start_ty, end_ty});

  auto* ptr = builder.CreateAlloca(range_ty, nullptr, "literal.range");

  auto* start_ptr = builder.CreateStructGEP(range_ty, ptr, 0);
  auto* end_ptr   = builder.CreateStructGEP(range_ty, ptr, 1);

  assert(start_ptr);
  assert(end_ptr);

  auto* start_val = codegen_node(n.start);
  auto* end_val   = codegen_node(n.end);

  assert(start_val);
  assert(end_val);

  builder.CreateStore(start_val, start_ptr);
  builder.CreateStore(end_val, end_ptr);

  return add_generation(n.nodeid(), ptr);
}

llvm::Value* codegen::Codegen_AST::codegen_Literal_Record(const ast::Literal_Record& n) noexcept
{
  GENERATION_GUARD

  const auto* def = n.nodeid().def().node().as<ast::SFM_Facet>();

  // assume type_llvm on structured data = { field1, field2 }
  auto* struct_ty = llvm::cast<llvm::StructType>(get_type(n.nodeid().type()));

  std::vector<llvm::Value*> vals;
  std::vector<size_t>       offsets;
  vals.reserve(n.fields_args.size());

  size_t arg_index = 0;
  for (auto field : n.fields_args) {
    // form field : facets fields
    if (const auto* ptr = field.as<ast::Literal_Record>()) {
      vals.emplace_back(codegen_Literal_Record(*ptr));
      continue;
    }

    // facet field : values fields
    auto* val = codegen_node(field);

    const auto& name = n.fields_names[arg_index++];

    size_t     offset = 0;
    const auto it     = std::ranges::find_if(def->fields, [&](ast::ID f) -> bool {
      const auto* f_def = f.as<ast::SFM_Facet_Field>();
      if (f_def->name == name) return true;
      offset++;
      return false;
    });

    offsets.emplace_back(offset);

    if (it == def->fields.end())
      add_error_two_nodes(171, field.get(), def->header,
                          std::format("The field \"{}\" dosen't exists in type \"", name) + def->name + "\".", "");

    auto n_found = *it;
    vals.emplace_back(codegen_node(n_found));
  }

  llvm::Value* ptr = builder.CreateAlloca(struct_ty, nullptr, def->name);

  for (size_t i = 0; i < vals.size(); i++) {
    auto& val    = vals[i];
    auto  offset = offsets[i];

    auto* f_ptr = builder.CreateStructGEP(struct_ty, ptr, offset, "field_" + std::to_string(offset));
    builder.CreateStore(val, f_ptr);
  }

  return add_generation(n.nodeid(), ptr);
}

#undef CONSTANT_GUARD

llvm::Value* codegen::Codegen_AST::codegen_Expression_If_Ternary(const ast::Expression_If_Ternary& n) noexcept
{
  return nullptr;
}
llvm::Value* codegen::Codegen_AST::codegen_Expression_Member_Access(const ast::Expression_Member_Access& n) noexcept
{
  return nullptr;
}

llvm::Value* codegen::Codegen_AST::codegen_Expression_Self(const ast::Expression_Self& n) noexcept
{
  return nullptr;
}
llvm::Value* codegen::Codegen_AST::codegen_Expression_Other(const ast::Expression_Other& n) noexcept
{
  return nullptr;
}

llvm::Value* codegen::Codegen_AST::codegen_Expression_Invocation(const ast::Expression_Invocation& n) noexcept
{
  GENERATION_GUARD

  if (auto* fn_ptr = n.nodeid().def().node().as<ast::Global_Function>()) {
    llvm::Function::LinkageTypes liknage =
        fn_ptr->visibility == EVisibility::Cross_File_Scope || !fn_ptr->extern_abi.empty()
            ? llvm::Function::ExternalLinkage
            : llvm::Function::InternalLinkage;

    auto* fn_ty = llvm::cast<llvm::FunctionType>(get_type(fn_ptr->prototype));
    assert(fn_ty);
    auto* callee = generate_stub(fn_ty, fn_ptr->name, liknage);

    size_t                    count = 0;
    std::vector<llvm::Value*> args;
    args.reserve(n.arguments.size());
    for (auto argid : n.arguments) args.emplace_back(codegen_node(argid));

    auto* call = builder.CreateCall(callee, args);
    return add_generation(n.nodeid(), call);
  }

  NOT_DEFINED
}
llvm::Value* codegen::Codegen_AST::codegen_Expression_Invocation_Arg(const ast::Expression_Invocation_Arg& n) noexcept
{
  GENERATION_GUARD

  auto sym = n.nodeid().def();

  // variadic argument : no parameter symbol reference
  // codegen on expression alone
  if (!sym) return add_generation(n.nodeid(), insurance.ensure_variadic_arg(n.expression));


  const auto* def_n = n.nodeid().def().node().as<ast::Local_Parameter>();

  switch (def_n->passmode) {
  case ast::EPassMode::mut:
  case ast::EPassMode::move:
  case ast::EPassMode::ref:  {
    auto* ptrty = llvm::PointerType::getUnqual(get_type(n.expression.type()));
    // auto* load  = builder.CreateLoad(ptrty, codegen_node(n.expression), "arg." + def_n->name);

    return add_generation(n.nodeid(), codegen_node(n.expression));
  }
  case ast::EPassMode::copy:
    return add_generation(n.nodeid(), insurance.ensure_rvalue(n.expression, "arg." + def_n->name));
  case ast::EPassMode::addr: {
    llvm::Value* ptr     = insurance.ensure_lvalue(n.expression);
    llvm::Type*  ptrTy   = get_type(n.expression.type());
    llvm::Value* storage = builder.CreateAlloca(ptrTy, nullptr, "addr.storage");
    builder.CreateStore(storage, ptr);
    return add_generation(n.nodeid(), storage);
  }
  case ast::EPassMode::NONE: {
    return add_generation(n.nodeid(), codegen_node(n.expression));
  }
  }
}
llvm::Value*
codegen::Codegen_AST::codegen_Expression_Invocation_Extend(const ast::Expression_Invocation_Extend& n) noexcept {
    NOT_DEFINED} llvm::Value* codegen::Codegen_AST::
    codegen_Expression_Invocation_Rule(const ast::Expression_Invocation_Rule& n) noexcept
{
  return nullptr;
}
llvm::Value* codegen::Codegen_AST::codegen_Expression_Table_Access(const ast::Expression_Table_Access& n) noexcept
{
  auto [ty, ptr] = codegen_collection_data_ptr(n.target);
  auto* idx      = insurance.ensure_rvalue(n.selector);

  auto* gep = builder.CreateGEP(ty, ptr, {idx}, "ptr_idx");
  return add_generation(n.nodeid(), gep);
}

llvm::Value* codegen::Codegen_AST::codegen_Expression_Ptr_Val(const ast::Expression_Ptr_Val& n) noexcept
{
  return add_generation(n.nodeid(), insurance.ensure_lvalue(n.target));
}
llvm::Value* codegen::Codegen_AST::codegen_Expression_Mut_Of(const ast::Expression_Mut_Of& n) noexcept
{
  GENERATION_GUARD

  auto* targetPtr = codegen_node(n.target);

  if (!targetPtr->getType()->isPointerTy()) {
    assert(false && "requires an lvalue expression");
  }

  // return the pointer directly
  return add_generation(n.nodeid(), targetPtr);
}
llvm::Value* codegen::Codegen_AST::codegen_Expression_Ref_Of(const ast::Expression_Ref_Of& n) noexcept
{
  GENERATION_GUARD

  auto* targetPtr = codegen_node(n.target);

  if (!targetPtr->getType()->isPointerTy()) {
    assert(false && "& requires an lvalue expression");
  }

  // return the pointer directly
  return add_generation(n.nodeid(), targetPtr);
}
llvm::Value* codegen::Codegen_AST::codegen_Expression_Addr_Of(const ast::Expression_Addr_Of& n) noexcept
{
  GENERATION_GUARD

  return add_generation(n.nodeid(), insurance.ensure_lvalue(n.target));
}
llvm::Value* codegen::Codegen_AST::codegen_Expression_Size_Of(const ast::Expression_Size_Of& n) noexcept
{
  return nullptr;
}
llvm::Value* codegen::Codegen_AST::codegen_Expression_GetBits(const ast::Expression_GetBits& n) noexcept
{
  return nullptr;
}

llvm::Value* codegen::Codegen_AST::codegen_Expression_Move_Of(const ast::Expression_Move_Of& n) noexcept
{
  return nullptr;
}

llvm::Value* codegen::Codegen_AST::codegen_Expression_Copy_Of(const ast::Expression_Copy_Of& n) noexcept {NOT_DEFINED}

llvm::Value* codegen::Codegen_AST::codegen_Expression_New_Ptr(const ast::Expression_New_Ptr& n) noexcept
{
  return nullptr;
}

llvm::Value* codegen::Codegen_AST::codegen_Expression_Get_Type(const ast::Expression_Get_Type& n) noexcept
{
  NOT_DEFINED
}

void codegen::Codegen_AST::codegen_Statement_If(const ast::Statement_If& n, llvm::BasicBlock* bb_parent_merge) noexcept
{
  if (n.is_else) {
    codegen_Statement_Else(n, bb_parent_merge);
    return;
  }
  if (n.is_elif) {
    codegen_Statement_Elif(n, bb_parent_merge);
    return;
  }

  auto* function = builder.GetInsertBlock()->getParent();

  auto*             bb_then = llvm::BasicBlock::Create(ctx, "if.then", function);
  llvm::BasicBlock* bb_alt  = (n.alternative_statement) ? llvm::BasicBlock::Create(ctx, "if.alt", function) : nullptr;

  auto* bb_merge = bb_parent_merge ? bb_parent_merge : llvm::BasicBlock::Create(ctx, "if.merge", function);

  auto* cond = insurance.ensure_rvalue(n.evaluator);

  if (bb_alt)
    builder.CreateCondBr(cond, bb_then, bb_alt);
  else
    builder.CreateCondBr(cond, bb_then, bb_merge);

  // then
  builder.SetInsertPoint(bb_then);
  (void)codegen_node(n.codeblock);
  if (!builder.GetInsertBlock()->getTerminator()) builder.CreateBr(bb_merge);

  // alternative statement
  if (bb_alt) {
    builder.SetInsertPoint(bb_alt);
    const auto* alt = n.alternative_statement.as<ast::Statement_If>();
    codegen_Statement_If(*alt, bb_merge);
  }

  builder.SetInsertPoint(bb_merge);
}

void codegen::Codegen_AST::codegen_Statement_Elif(const ast::Statement_If& n,
                                                  llvm::BasicBlock*        bb_parent_merge) noexcept
{
  auto* function = builder.GetInsertBlock()->getParent();

  auto*             bb_then = llvm::BasicBlock::Create(ctx, "if.then", function);
  llvm::BasicBlock* bb_alt  = (n.alternative_statement) ? llvm::BasicBlock::Create(ctx, "if.alt", function) : nullptr;

  auto* bb_merge = bb_parent_merge ? bb_parent_merge : llvm::BasicBlock::Create(ctx, "if.merge", function);

  auto* cond = insurance.ensure_rvalue(n.evaluator);

  if (bb_alt)
    builder.CreateCondBr(cond, bb_then, bb_alt);
  else
    builder.CreateCondBr(cond, bb_then, bb_merge);

  // then
  builder.SetInsertPoint(bb_then);
  (void)codegen_node(n.codeblock);
  if (!builder.GetInsertBlock()->getTerminator()) builder.CreateBr(bb_merge);

  // alternative statement
  if (bb_alt) {
    builder.SetInsertPoint(bb_alt);
    const auto* alt = n.alternative_statement.as<ast::Statement_If>();
    codegen_Statement_If(*alt, bb_merge);
  }
}
void codegen::Codegen_AST::codegen_Statement_Else(const ast::Statement_If& n,
                                                  llvm::BasicBlock*        bb_parent_merge) noexcept
{
  auto* function = builder.GetInsertBlock()->getParent();

  auto* bb_merge = bb_parent_merge ? bb_parent_merge : llvm::BasicBlock::Create(ctx, "if.merge", function);

  // then
  (void)codegen_node(n.codeblock);
  if (!builder.GetInsertBlock()->getTerminator()) builder.CreateBr(bb_merge);
}


void codegen::Codegen_AST::codegen_Statement_For(const ast::Statement_For& n) noexcept
{
  if (n.items.empty()) {
    codegen_Statement_For_Index(n);
  } else {
    codegen_Statement_For_Items(n);
  }
}

void codegen::Codegen_AST::codegen_Statement_For_Items(const ast::Statement_For& n) noexcept
{
  auto* function = builder.GetInsertBlock()->getParent();

  auto* bb_entry  = builder.GetInsertBlock();
  auto* bb_header = llvm::BasicBlock::Create(ctx, "for.header", function);
  auto* bb_body   = llvm::BasicBlock::Create(ctx, "for.body", function);
  auto* bb_latch  = llvm::BasicBlock::Create(ctx, "for.latch", function);
  auto* bb_exit   = llvm::BasicBlock::Create(ctx, "for.exit", function);

  // =========================
  // RANGE
  // =========================
  auto [start, len, end_included] = tools.get_span(n.expression);
  auto* end                       = builder.CreateAdd(start, len);


  // jump to header
  builder.CreateBr(bb_header);

  // =========================
  // HEADER (condition)
  // =========================
  builder.SetInsertPoint(bb_header);

  auto* phi = builder.CreatePHI(codegen::LLVM_TYPEID_usize, 2, "for.i");
  if (n.index) (void)add_generation(n.index, phi); // override codegen
  phi->addIncoming(start, bb_entry);

  auto [inner_ty, data] = codegen_collection_data_ptr(n.expression);
  auto* it              = builder.CreateGEP(inner_ty, data, phi, "it");
  (void)add_generation(n.items[0], it);

  auto* cond = builder.CreateICmpULT(phi, end);
  builder.CreateCondBr(cond, bb_body, bb_exit);

  // =========================
  // BODY
  // =========================
  current_bb_break    = bb_exit;
  current_bb_continue = bb_latch;

  builder.SetInsertPoint(bb_body);

  (void)codegen_node(n.codeblock);

  if (!builder.GetInsertBlock()->getTerminator()) builder.CreateBr(bb_latch);

  current_bb_break    = nullptr;
  current_bb_continue = nullptr;

  // =========================
  // LATCH (increment)
  // =========================
  builder.SetInsertPoint(bb_latch);

  auto* next = builder.CreateAdd(phi, const_int(1), "for.i.next");

  builder.CreateBr(bb_header);

  phi->addIncoming(next, bb_latch);

  // =========================
  // EXIT
  // =========================
  builder.SetInsertPoint(bb_exit);
}
void codegen::Codegen_AST::codegen_Statement_For_Index(const ast::Statement_For& n) noexcept
{
  auto* function = builder.GetInsertBlock()->getParent();

  auto* bb_entry  = builder.GetInsertBlock();
  auto* bb_header = llvm::BasicBlock::Create(ctx, "for.header", function);
  auto* bb_body   = llvm::BasicBlock::Create(ctx, "for.body", function);
  auto* bb_latch  = llvm::BasicBlock::Create(ctx, "for.latch", function);
  auto* bb_exit   = llvm::BasicBlock::Create(ctx, "for.exit", function);

  // =========================
  // RANGE
  // =========================
  auto [start, len, end_included] = tools.get_span(n.expression);
  auto* end                       = builder.CreateAdd(start, len, "it.end");

  // jump to header

  builder.CreateBr(bb_header);

  // =========================
  // HEADER (condition)
  // =========================
  builder.SetInsertPoint(bb_header);

  auto* phi = builder.CreatePHI(codegen::LLVM_TYPEID_usize, 2, "for.i");
  (void)add_generation(n.index, phi); // override codegen
  phi->addIncoming(start, bb_entry);

  auto* cond = builder.CreateICmpULT(phi, end);
  builder.CreateCondBr(cond, bb_body, bb_exit);

  // =========================
  // BODY
  // =========================
  current_bb_break    = bb_exit;
  current_bb_continue = bb_latch;

  builder.SetInsertPoint(bb_body);

  (void)codegen_node(n.codeblock);

  if (!builder.GetInsertBlock()->getTerminator()) builder.CreateBr(bb_latch);

  current_bb_break    = nullptr;
  current_bb_continue = nullptr;

  // =========================
  // LATCH (increment)
  // =========================
  builder.SetInsertPoint(bb_latch);

  auto* next = builder.CreateAdd(phi, const_int(1), "for.i.next");

  builder.CreateBr(bb_header);

  phi->addIncoming(next, bb_latch);

  // =========================
  // EXIT
  // =========================
  builder.SetInsertPoint(bb_exit);
}


void codegen::Codegen_AST::codegen_Statement_Loop(const ast::Statement_Loop& n) noexcept
{
  auto* function = builder.GetInsertBlock()->getParent();

  auto* bb_body  = llvm::BasicBlock::Create(ctx, "loop.body", function);
  auto* bb_after = llvm::BasicBlock::Create(ctx, "loop.after", function);

  current_bb_break    = bb_after;
  current_bb_continue = bb_body;

  builder.CreateBr(bb_body);

  builder.SetInsertPoint(bb_body);
  (void)codegen_node(n.codeblock);

  if (!builder.GetInsertBlock()->getTerminator()) builder.CreateBr(bb_body);

  current_bb_break    = nullptr;
  current_bb_continue = nullptr;

  builder.SetInsertPoint(bb_after);
}

// ============ WHILE LOOP ============
void codegen::Codegen_AST::codegen_Statement_While(const ast::Statement_While& n) noexcept
{
  if (n.is_do)
    codegen_Statement_While_Do(n);
  else
    codegen_Statement_While_Classic(n);
}

void codegen::Codegen_AST::codegen_Statement_While_Classic(const ast::Statement_While& n) noexcept
{
  auto* function = builder.GetInsertBlock()->getParent();

  auto* bb_header = llvm::BasicBlock::Create(ctx, "while.header", function);
  auto* bb_body   = llvm::BasicBlock::Create(ctx, "while.body", function);

  current_bb_continue = bb_header;

  auto* bb_merge   = llvm::BasicBlock::Create(ctx, "while.merge", function);
  current_bb_break = bb_merge;

  // =========================
  // ENTRY
  // =========================
  builder.CreateBr(bb_header);

  // =========================
  // HEADER
  // =========================
  builder.SetInsertPoint(bb_header);

  auto* cond = codegen_node(n.evaluator);
  builder.CreateCondBr(cond, bb_body, bb_merge);

  // =========================
  // BODY
  // =========================
  builder.SetInsertPoint(bb_body);
  (void)codegen_node(n.codeblock);
  builder.CreateBr(bb_header);

  // =========================
  // EXIT
  // =========================
  current_bb_break    = nullptr;
  current_bb_continue = nullptr;

  builder.SetInsertPoint(bb_merge);
}

void codegen::Codegen_AST::codegen_Statement_While_Do(const ast::Statement_While& n) noexcept
{
  auto* function = builder.GetInsertBlock()->getParent();

  auto* bb_root = llvm::BasicBlock::Create(ctx, "while.root", function);
  auto* bb_body = llvm::BasicBlock::Create(ctx, "while.body", function);

  current_bb_continue = bb_root;

  auto* bb_merge = llvm::BasicBlock::Create(ctx, "while.merge", function);

  // =========================
  // ENTRY
  // =========================
  builder.CreateBr(bb_body);

  // =========================
  // BODY
  // =========================
  builder.SetInsertPoint(bb_body);
  (void)codegen_node(n.codeblock);
  builder.CreateBr(bb_root);

  // =========================
  // HEADER
  // =========================
  builder.SetInsertPoint(bb_root);

  auto* cond = codegen_node(n.evaluator);
  builder.CreateCondBr(cond, bb_body, bb_merge);

  // =========================
  // EXIT
  // =========================
  current_bb_break    = nullptr;
  current_bb_continue = nullptr;

  builder.SetInsertPoint(bb_merge);
}


// ============ GOTO / LABEL ============
void codegen::Codegen_AST::codegen_Statement_GoTo(const ast::Statement_GoTo& n) noexcept
{
  builder.CreateBr(codegen_Statement_GoTo_Label(*n.nodeid().def().node().as<ast::Statement_GoTo_Label>()));
}

llvm::BasicBlock* codegen::Codegen_AST::codegen_Statement_GoTo_Label(const ast::Statement_GoTo_Label& n) noexcept
{
  if (auto it = gotos.find(n.nodeid()); it != gotos.end()) return it->second;

  auto* fn = builder.GetInsertBlock()->getParent();
  auto* bb = llvm::BasicBlock::Create(ctx, n.label, fn);

  builder.SetInsertPoint(bb);
  (void)codegen_node(n.codeblock);

  if (!builder.GetInsertBlock()->getTerminator()) {
    auto* dead = llvm::BasicBlock::Create(ctx, "label.dead", builder.GetInsertBlock()->getParent());
    builder.CreateBr(dead);
    builder.SetInsertPoint(dead);
  }

  gotos.try_emplace(n.nodeid(), bb);
  return bb;
}

// ============ RETURN / BREAK / CONTINUE ============
llvm::ReturnInst* codegen::Codegen_AST::codegen_Statement_Return(const ast::Statement_Return& n) noexcept
{
  if (auto it = returns.find(n.nodeid()); it != returns.end()) return it->second;

  const auto protoid = type::get_prototype(n.returnable);
  assert(protoid);
  const auto* proto = protoid.as<type::Prototype>();

  llvm::ReturnInst* ret = nullptr;

  if (n.value) {
    auto* v = insurance.ensure_rvalue(n.value);
    ret     = builder.CreateRet(v);
  } else {
    ret = builder.CreateRetVoid();
  }

  returns.try_emplace(n.nodeid(), ret);
  return ret;
}

llvm::BranchInst* codegen::Codegen_AST::codegen_Statement_Break(const ast::Statement_Break& n) noexcept
{
  return current_bb_break ? builder.CreateBr(current_bb_break) : nullptr;
}

llvm::BranchInst* codegen::Codegen_AST::codegen_Statement_Continue(const ast::Statement_Continue& n) noexcept
{
  return current_bb_continue ? builder.CreateBr(current_bb_continue) : nullptr;
}

void codegen::Codegen_AST::codegen_Statement_Match(const ast::Statement_Match& n) noexcept
{
  NOT_DEFINED
}
void codegen::Codegen_AST::codegen_Statement_Match_Case(const ast::Statement_Match_Case& n) noexcept {NOT_DEFINED}

// ============ OPERATION ============
llvm::Value* codegen::Codegen_AST::codegen_Operation_Cast_As(const ast::Operation_Cast_As& n) noexcept
{
  GENERATION_GUARD

  auto* ty        = get_type(n.expression.type());
  auto* target_ty = get_type(n.nodeid().type());


  switch (n.cast_type) {
  case ast::Operation_Cast_As::ECastType::AS: {
    if (auto* cast = tools.codegen_explicit_cast(n.expression, n.type)) return cast;

    common::compiler::DEBUG_TOLZA_ICE("Unimplemented legal explicit cast");
  }
  case ast::Operation_Cast_As::ECastType::AS_REINTERPRET: {
    auto* expr = codegen_node(n.expression);
    // ptr
    if (ty->isPointerTy() && target_ty->isPointerTy())
      return add_generation(n.nodeid(), builder.CreateBitCast(expr, target_ty));
    if (ty->isPointerTy() && target_ty->isIntegerTy())
      return add_generation(n.nodeid(), builder.CreatePtrToInt(expr, target_ty));
    if (ty->isIntegerTy() && target_ty->isPointerTy())
      return add_generation(n.nodeid(), builder.CreateIntToPtr(expr, target_ty));
    auto* cast = builder.CreateBitCast(expr, target_ty);
    return add_generation(n.nodeid(), cast);
  }
  case ast::Operation_Cast_As::ECastType::AS_SAFE: break;
  }
  return nullptr;
}
llvm::Value* codegen::Codegen_AST::codegen_Operation_Is(const ast::Operation_Is& n) noexcept
{
  return nullptr;
}
llvm::Value* codegen::Codegen_AST::codegen_Operation_In(const ast::Operation_In& n) noexcept
{
  return nullptr;
}
llvm::Value* codegen::Codegen_AST::codegen_Operation_Transfert(const ast::Operation_Transfert& n) noexcept
{
  GENERATION_GUARD

  auto* ptr = insurance.ensure_lvalue(n.left);
  assert(ptr);

  llvm::Value* val = nullptr;

  if (n.assignment_op != ast::EOp_Bin::NONE) {
    val = tools.codegen_binary_op(n.left, n.right, n.assignment_op);
  } else {
    val = insurance.ensure_rvalue(n.right);
  }
  assert(val);

  return add_generation(n.nodeid(), builder.CreateStore(val, ptr));
}
llvm::Value* codegen::Codegen_AST::codegen_Operation_Binary(const ast::Operation_Binary& n) noexcept
{
  GENERATION_GUARD

  return add_generation(n.nodeid(), tools.codegen_binary_op(n.left, n.right, n.op_ty));
}
llvm::Value* codegen::Codegen_AST::codegen_Operation_Unary(const ast::Operation_Unary& n) noexcept
{
  GENERATION_GUARD

  return add_generation(n.nodeid(), tools.codegen_unary_op(n.base, n.unary_op));
}
llvm::Value* codegen::Codegen_AST::codegen_Operation_Interval(const ast::Operation_Interval& n) noexcept
{
  return nullptr;
}
