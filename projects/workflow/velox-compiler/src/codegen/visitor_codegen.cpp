#include "visitor_codegen.hpp"

#include <cstdint>
#include <filesystem>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>


#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/Casting.h>

#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Value.h>

#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/APInt.h>


#include <compiler_context.hpp>

#include "ast/ast_data.hpp"
#include "ast/ast_inferred_type_singleton.hpp"
#include "codegen_tools.hpp"
#include "static_evaluation.hpp"
#include "compiler/compiler.hpp"
#include "misc/script_info.hpp"
#include "compiler/compiler.hpp"
#include "misc/error_output.hpp"

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
#include "static_evaluation.hpp"
#include "misc/symbol_manager.hpp"


Visitor_Codegen::Visitor_Codegen(ScriptInfo& _scr_info)
  : scr_info(_scr_info)
  , ctx(compiler::LLVM_CTX)
  , __module(std::make_unique<llvm::Module>(_scr_info.file_info.get_module_name(), ctx))
  , mod(__module.get())
  , builder(*new llvm::IRBuilder<>(ctx))
  , tools(*new LLVM_Tools(*this))
  , eval(*new Static_Evaluator(*this))
  , u0Ty(llvm::Type::getVoidTy(ctx))
  , i1Ty(llvm::Type::getInt1Ty(ctx))
  , i8Ty(llvm::Type::getInt8Ty(ctx))
  , i16Ty(llvm::Type::getInt16Ty(ctx))
  , i32Ty(llvm::Type::getInt32Ty(ctx))
  , i64Ty(llvm::Type::getInt64Ty(ctx))
  , i128Ty(llvm::Type::getInt128Ty(ctx))
  , iSizeTy(llvm::Type::getIntNTy(ctx, compiler::COMP_CTX.get_arch_size()))
  , f16Ty(llvm::Type::getHalfTy(ctx))
  , f32Ty(llvm::Type::getFloatTy(ctx))
  , f64Ty(llvm::Type::getDoubleTy(ctx))
  , f80Ty(llvm::Type::getX86_FP80Ty(ctx))
  , f128Ty(llvm::Type::getFP128Ty(ctx))
  , fSizeTy(compiler::COMP_CTX.get_arch_size() == 32   ? f32Ty
            : compiler::COMP_CTX.get_arch_size() == 64 ? f64Ty
                                                       : f128Ty)
  , strTy(llvm::StructType::get(ctx, {i8Ty->getPointerTo(), i32Ty}))
  , textTy(llvm::StructType::get(ctx, {i32Ty->getPointerTo(), i32Ty}))
  , cstrTy(llvm::Type::getInt8Ty(ctx)->getPointerTo())
  , get_zero(llvm::ConstantInt::get(i32Ty, 0))
{
}

void Visitor_Codegen::build_init_func()
{
  if (init_func) return;

  auto ty = llvm::FunctionType::get(u0Ty, false);
  auto fn = llvm::Function::Create(ty, llvm::Function::ExternalLinkage, "init_module_" + mod->getName(), *mod);

  auto              bb = llvm::BasicBlock::Create(ctx, "entry", fn);
  llvm::IRBuilder<> irb(bb);
  irb.CreateRetVoid();

  init_func = fn;
}

void Visitor_Codegen::error_add(ErrorCode code, const ast::Node& n, const std::string& msg,
                                const std::string& hint) const
{
  auto error = Error_Diagnostic(scr_info, code, &scr_info, n.node_token, compiler::EPhase::llvmir, msg, hint);

  errors.push_back(error.print_error());
}

void Visitor_Codegen::error_two_lines(ErrorCode code, const ast::Node& first, const ast::Node& second,
                                      const std::string& msg, const std::string& hint) const
{
  auto err = Error_Diagnostic_Two(scr_info, code, first, second, compiler::EPhase::llvmir, msg, hint);
  errors.push_back(err.print_error());
}

llvm::Value* Visitor_Codegen::ensure_rvalue(ast::AExpression& expr, const std::string& name)
{
  if (expr.is_rvalue()) return expr.codegen(*this);

  auto val = expr.codegen(*this);
  auto ty  = expr.expression_inferred_type->codegen_ty(*this);
  // load the value
  return builder.CreateLoad(ty, val, name);
}


llvm::Value* Visitor_Codegen::ensure_lvalue(ast::AExpression& expr, bool is_silent_error)
{
  if (expr.is_lvalue()) return expr.codegen(*this);

  if (!is_silent_error) error_add(225, expr, "Expected a lvalue expression.", "a lvalue is frequently a variable.");
  return nullptr;
}


llvm::Function* Visitor_Codegen::generate_stub(ast::type::Function_Proto& proto, const std::string& name,
                                               llvm::Function::LinkageTypes link_ty)
{
  if (auto func = mod->getFunction(name)) return func;

  auto fn_ty = llvm::cast<llvm::FunctionType>(proto.codegen_ty(*this));
  auto fn    = llvm::Function::Create(fn_ty, link_ty, name, *mod);

  size_t idx = 0;
  for (auto& arg : fn->args()) {
    auto& node_param     = proto.parameters[idx++];
    node_param->llvm_arg = &arg;
  }

  return fn;
}


// ============ AST ============
void Visitor_Codegen::visit(ast::Node& n)
{
}

llvm::Type* Visitor_Codegen::visit(ast::AType& n)
{
  return n.codegen_ty(*this);
}
void Visitor_Codegen::visit(ast::ALiteral& n)
{
}
void Visitor_Codegen::visit(ast::ADeclaration& n)
{
}
void Visitor_Codegen::visit(ast::ALocal& n)
{
}
void Visitor_Codegen::visit(ast::AExpression& n)
{
}
void Visitor_Codegen::visit(ast::AIdentifier& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::Expr_ID& n)
{
  if (n.llvm_value) return n.llvm_value;
  return n.llvm_value = n.identifier_symbol->codegen_pass(*this);
}
llvm::Value* Visitor_Codegen::visit(ast::Expr_ID_Qualified& n)
{
  if (n.llvm_value) return n.llvm_value;
  return n.llvm_value = n.identifier_symbol->codegen_pass(*this);
}
llvm::Value* Visitor_Codegen::visit(ast::Expr_ID_Type& n)
{
  if (n.llvm_value) return n.llvm_value;
  return n.llvm_value = n.codegen(*this);
}
llvm::Type* Visitor_Codegen::visit_ty(ast::Expr_ID_Type& n)
{
  if (n.llvm_type) return n.llvm_type;
  if (n.expression_inferred_type) {
    return n.llvm_type = n.expression_inferred_type->codegen_ty(*this);
  } else if (n.name->expression_inferred_type) {
    return n.llvm_type = n.name->expression_inferred_type->codegen_ty(*this);
  } else if (auto ptr = dynamic_cast<ast::AType*>(n.identifier_symbol.get())) {
    return n.llvm_type = ptr->codegen_ty(*this);
  } else {
    std::cout << "type lost in addr " << &n << std::endl;
    error_add(194, n, "Inferred type lost", "");
    return nullptr;
  }
}

void Visitor_Codegen::visit(ast::Root& n)
{
  for (auto& elem : n.global_nodes) {
    if (auto ptr = dynamic_cast<ast::AType*>(elem.get())) {
      auto a = ptr->codegen_ty(*this);
    } else if (auto ptr = dynamic_cast<ast::ADeclaration*>(elem.get())) {
      auto a = ptr->codegen_pass(*this);
    }
  }
}

// ============ DECLARATION ============
llvm::Value* Visitor_Codegen::visit(ast::declaration::Global& n)
{
  if (n.llvm_value) return n.llvm_value;

  auto ty = n.type->codegen_ty(*this);

  if (n.declaration_is_external) {
    return n.llvm_value = new llvm::GlobalVariable(*mod, ty, true, llvm::GlobalVariable::ExternalLinkage,
                                                   tools.get_zeroinitializer(*n.type), n.declaration_name);
  }

  auto linkage =
      n.declaration_is_exported ? llvm::GlobalVariable::ExternalLinkage : llvm::GlobalVariable::InternalLinkage;
  const bool is_const = n.kind != EVariableKind::Var;

  if (!n.expression) {
    return n.llvm_value = new llvm::GlobalVariable(*mod, ty, is_const, linkage, tools.get_zeroinitializer(*n.type),
                                                   n.declaration_name);
  }

  if (auto expr = eval.evaluate_expression(*n.expression)) {
    if (auto expr_const = tools.create_constant(*expr.value())) {
      return n.llvm_value = new llvm::GlobalVariable(*mod, ty, is_const,
                                                     n.declaration_is_exported ? llvm::GlobalVariable::ExternalLinkage
                                                                               : llvm::GlobalVariable::InternalLinkage,
                                                     expr_const.value(), n.declaration_name);
    } else {
      error_add(223, *n.expression, expr_const.error(), "");
      return nullptr;
    }
  } else {
    error_add(222, *n.expression, expr.error(), "");
    return nullptr;
  }

  /*
    build_init_func();
    auto&             bb = init_func->getEntryBlock();
    llvm::IRBuilder<> irb(&bb);
    irb.SetInsertPoint(bb.getTerminator()); // before ret

    irb.CreateStore(n.expression->codegen(*this), glo);

    return n.llvm_value = glo;
  */
}

// ============ FUNCTION ============
llvm::Function* Visitor_Codegen::visit(ast::declaration::Function& n)
{
  if (n.llvm_fn) return n.llvm_fn;

  const bool is_main = n.declaration_name == "main";

  llvm::Function* fn = nullptr;

  if (n.declaration_symbol->codegen_pass(*this)) {
    fn = llvm::cast<llvm::Function>(n.declaration_symbol->codegen_pass(*this));
  } else {
    llvm::Function::LinkageTypes linkage = n.declaration_is_exported || n.declaration_is_external
                                               ? llvm::Function::ExternalLinkage
                                               : llvm::Function::InternalLinkage;

    llvm::FunctionType* fn_ty;

    // special main function case
    if (is_main) {
      linkage = llvm::Function::ExternalLinkage;
      std::vector<llvm::Type*> param_tys;
      for (auto& param : n.prototype->parameters) param_tys.emplace_back(param->type->codegen_ty(*this));
      fn_ty = llvm::FunctionType::get(i32Ty, param_tys, false);
    } else {
      fn_ty = llvm::cast<llvm::FunctionType>(n.prototype->codegen_ty(*this));
    }

    fn = generate_stub(*n.prototype, n.declaration_name, linkage);

    // param naming
    size_t count = 0;
    for (auto& arg : fn->args()) {
      auto              param = n.prototype->parameters[count++];
      const std::string name  = param->declaration_name;
      arg.setName(name);
    }
  }

  if (!n.codeblock) return fn;

  auto entry = llvm::BasicBlock::Create(ctx, "entry", fn);
  builder.SetInsertPoint(entry);

  n.codeblock->codegen_pass(*this);

  // TERMINATE BLOCKS PROPERLY
  if (!builder.GetInsertBlock()->getTerminator()) {
    if (is_main)
      builder.CreateRet(llvm::ConstantInt::get(i32Ty, 0));
    else
      builder.CreateRetVoid();
  }

  return n.llvm_fn = fn;
}

void Visitor_Codegen::visit(ast::declaration::Mod& n)
{
  for (auto& elem : n.declarations) elem->codegen_pass(*this);
}
void Visitor_Codegen::visit(ast::declaration::Export& n)
{
  for (auto& elem : n.declarations) elem->codegen_pass(*this);
}
void Visitor_Codegen::visit(ast::declaration::Extern& n)
{
  for (auto& elem : n.declarations) elem->codegen_pass(*this);
}

llvm::Type* Visitor_Codegen::visit(ast::declaration::Enum& n)
{
  if (n.llvm_type) return n.llvm_type;


  size_t payload_size  = 1;
  size_t payload_align = 0;
  for (auto& elem : n.variants) {
    auto   elem_ty    = elem->codegen_ty(*this);
    size_t elem_size  = mod->getDataLayout().getTypeAllocSize(elem_ty);
    size_t elem_align = mod->getDataLayout().getABITypeAlign(elem_ty).value();

    if (payload_size < elem_size) payload_size = elem_size;
    if (payload_align < elem_align) payload_align = elem_align;
  }

  return n.llvm_type = llvm::StructType::create(ctx,
                                                {
                                                    i32Ty,                                   // tag
                                                    llvm::ArrayType::get(i8Ty, payload_size) // payload
                                                },
                                                n.mangle_type());
}

llvm::Type* Visitor_Codegen::visit(ast::declaration::Enum_Element& n)
{
  if (n.llvm_type) return n.llvm_type;


  if (n.types.empty()) return n.llvm_type = llvm::StructType::get(ctx);
  if (n.types.size() == 1) return n.llvm_type = n.types[0]->codegen_ty(*this);


  std::vector<llvm::Type*> sub_elements;
  for (auto& elem_sub_type : n.types) sub_elements.push_back(elem_sub_type->codegen_ty(*this));

  return n.llvm_type = llvm::StructType::get(ctx, sub_elements);
}

llvm::Type* Visitor_Codegen::visit(ast::declaration::Flag& n)
{
  if (n.llvm_type) return n.llvm_type;


  return n.llvm_type =
             llvm::StructType::create(ctx, {i32Ty, llvm::Type::getIntNTy(ctx, n.fields.size())}, n.mangle_type());
}

llvm::Type* Visitor_Codegen::visit(ast::declaration::Union& n)
{
  if (n.llvm_type) return n.llvm_type;


  size_t payload_size  = 1;
  size_t payload_align = 0;
  for (auto& [_, elem] : n.fields) {
    auto   elem_ty    = elem->codegen_ty(*this);
    size_t elem_size  = mod->getDataLayout().getTypeAllocSize(elem_ty);
    size_t elem_align = mod->getDataLayout().getABITypeAlign(elem_ty).value();

    if (payload_size < elem_size) payload_size = elem_size;
    if (payload_align < elem_align) payload_align = elem_align;
  }

  return n.llvm_type =
             llvm::StructType::create(ctx, {i32Ty, llvm::Type::getIntNTy(ctx, n.fields.size())}, n.mangle_type());
}

void Visitor_Codegen::visit(ast::declaration::Mod_Alias& n)
{
}
llvm::Type* Visitor_Codegen::visit(ast::declaration::Type_Alias& n)
{
  if (n.type->llvm_type) return n.type->llvm_type;

  return n.type->llvm_type = n.type->codegen_ty(*this);
}

llvm::Type* Visitor_Codegen::visit(ast::declaration::Generic& n)
{
}

// ============ LOCAL ============
void Visitor_Codegen::visit(ast::declaration::local::CodeBlock& n)
{
  for (auto& elem : n.elements) {
    switch (elem.kind) {
    case ast::CodeBlock_instruction::EKind::None:         continue;
    case ast::CodeBlock_instruction::EKind::Shared_local: elem.data_local->codegen_pass(*this); break;
    case ast::CodeBlock_instruction::EKind::Unique_base:  {
      if (auto ptr = dynamic_cast<ast::Trait_LLVM_Typed*>(elem.data_base.get()))
        ptr->codegen_ty(*this);
      else if (auto ptr = dynamic_cast<ast::Trait_LLVM_Value*>(elem.data_base.get()))
        ptr->codegen(*this);
      else if (auto ptr = dynamic_cast<ast::Trait_LLVM_Passage*>(elem.data_base.get()))
        ptr->codegen_pass(*this);
      else if (auto ptr = dynamic_cast<ast::Trait_LLVM_Callable*>(elem.data_base.get()))
        ptr->codegen(*this);
      break;
    }
    }
  }
}

llvm::Function* Visitor_Codegen::visit(ast::declaration::local::Lambda& n)
{
  return nullptr;
}
void Visitor_Codegen::visit(ast::declaration::local::Lambda_Capture& n)
{
}
void Visitor_Codegen::visit(ast::declaration::local::Capture_Member& n)
{
}

llvm::Value* Visitor_Codegen::visit(ast::declaration::local::Parameter& n)
{
  return n.llvm_arg;
}
void Visitor_Codegen::visit(ast::declaration::local::Generic_Parameter_Element& n)
{
}
void Visitor_Codegen::visit(ast::declaration::local::Generic_Parameters& n)
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
llvm::Value* Visitor_Codegen::visit(ast::declaration::local::Pattern_System_Component& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::declaration::local::Pattern_Component& n)
{
}

llvm::Value* Visitor_Codegen::visit(ast::declaration::local::Variable_Binding& n)
{
  if (n.llvm_value) return n.llvm_value;

  auto ty     = n.type->codegen_ty(*this);
  auto alloca = builder.CreateAlloca(ty, nullptr, n.declaration_name);

  if (n.parent_pattern && n.parent_pattern->right) {
    auto init_val = n.parent_pattern->right->codegen(*this);
    builder.CreateStore(init_val, alloca);
  }

  return n.llvm_value = alloca;
}
void Visitor_Codegen::visit(ast::declaration::local::Tuple_Destructuring& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::declaration::local::Variable& n)
{
  if (n.llvm_value) return n.llvm_value;


  if (n.kind == EVariableKind::Const) {
    if (auto ptr = eval.evaluate_expression(*n.expression)) {
      if (auto val_const = tools.create_constant(*ptr.value())) {
        return n.llvm_value = val_const.value();
      }
    }
  }

  llvm::AllocaInst* alloca;
  if (auto ptr_ty_table = dynamic_cast<ast::type::Table*>(n.type.get())) {
    if (ptr_ty_table->table_size > 0) {
      auto ty = ptr_ty_table->inner->codegen_ty(*this);
      alloca  = builder.CreateAlloca(ty, builder.getInt32(ptr_ty_table->table_size), n.declaration_name);
    } else {
      auto ty = n.type->codegen_ty(*this);
      alloca  = builder.CreateAlloca(ty, nullptr, n.declaration_name);
    }
  } else {
    auto ty = n.type->codegen_ty(*this);
    alloca  = builder.CreateAlloca(ty, nullptr, n.declaration_name);
  }

  if (n.expression) {
    auto expr = ensure_rvalue(*n.expression);
    if (!expr) {
      error_add(227, *n.expression, "Impossible to assign unevaluated an expression.", "");
      return nullptr;
    }
    builder.CreateStore(expr, alloca, n.type->type_is_volatile);
  }

  return n.llvm_value = alloca;
}

llvm::Value* Visitor_Codegen::visit(ast::declaration::local::Capability& n)
{
  if (n.llvm_value) return n.llvm_value;

  auto right = n.right->codegen(*this);

  llvm::AllocaInst* ptr = nullptr;
  switch (n.kind) {
  case ECapability::NONE:
  case ECapability::Move:
  case ECapability::Ref:
  case ECapability::Mut:  {
    auto ty = n.right->expression_inferred_type->codegen_ty(*this)->getPointerTo();

    ptr = builder.CreateAlloca(ty, nullptr, n.declaration_name);
    break;
  }
  case ECapability::Copy:
  case ECapability::Clone: {
    auto ty = n.right->expression_inferred_type->codegen_ty(*this);

    ptr = builder.CreateAlloca(ty, nullptr, n.declaration_name);
    break;
  }
  }

  builder.CreateStore(right, ptr);

  return ptr;
}

// ============ COP ============
llvm::Type* Visitor_Codegen::visit(ast::declaration::cop::Component& n)
{
  if (n.llvm_type) return n.llvm_type;

  std::vector<llvm::Type*> tys;
  for (auto& field : n.fields) tys.emplace_back(field->codegen_ty(*this));

  return n.llvm_type = llvm::StructType::get(ctx, tys);
}
llvm::Type* Visitor_Codegen::visit(ast::declaration::cop::Component_Field& n)
{
  auto ty = n.type->codegen_ty(*this);

  if (n.borrow != ast::declaration::cop::Component_Field::EBorrow::None) n.type->llvm_type = ty->getPointerTo();
  return ty;
}

llvm::Type* Visitor_Codegen::visit(ast::declaration::cop::Role& n)
{
  if (n.llvm_type) return n.llvm_type;

  std::vector<llvm::Type*> tys;
  for (auto& comp : n.components) {
    comp->codegen(*this);
    tys.emplace_back(comp->expression_inferred_type->codegen_ty(*this));
  }

  return n.llvm_type = llvm::StructType::get(ctx, tys);
}

llvm::Type* Visitor_Codegen::visit(ast::declaration::cop::Entity& n)
{
  if (n.llvm_type) return n.llvm_type;

  std::vector<llvm::Type*> tys;
  for (auto& comp : n.comps) {
    comp->codegen(*this);
    tys.emplace_back(comp->expression_inferred_type->codegen_ty(*this));
  }

  return n.llvm_type = llvm::StructType::get(ctx, tys);
}
llvm::Function* Visitor_Codegen::visit(ast::declaration::cop::Entity_New& n)
{
  if (n.llvm_fn) return n.llvm_fn;

  auto fn_ty = llvm::cast<llvm::FunctionType>(n.prototype->codegen_ty(*this));

  auto fn = llvm::Function::Create(fn_ty,
                                   n.parent_entity->declaration_is_external || n.parent_entity->declaration_is_exported
                                       ? llvm::Function::ExternalLinkage
                                       : llvm::Function::InternalLinkage,
                                   n.mangle_scope() + n.declaration_name);

  if (!n.codeblock) return fn;

  auto entry = llvm::BasicBlock::Create(ctx, "entry", fn);
  builder.SetInsertPoint(entry);

  n.codeblock->codegen_pass(*this);

  if (!entry->getTerminator()) builder.CreateRetVoid();

  return n.llvm_fn = fn;
}
llvm::Function* Visitor_Codegen::visit(ast::declaration::cop::Entity_Del& n)
{
  if (n.llvm_fn) return n.llvm_fn;

  auto fn_ty = llvm::FunctionType::get(u0Ty, false);

  auto fn = llvm::Function::Create(fn_ty,
                                   n.parent_entity->declaration_is_external || n.parent_entity->declaration_is_exported
                                       ? llvm::Function::ExternalLinkage
                                       : llvm::Function::InternalLinkage,
                                   n.mangle_scope() + n.declaration_name);

  if (!n.codeblock) return fn;

  auto entry = llvm::BasicBlock::Create(ctx, "entry", fn);
  builder.SetInsertPoint(entry);

  n.codeblock->codegen_pass(*this);

  if (!entry->getTerminator()) builder.CreateRetVoid();

  return n.llvm_fn = fn;
}
llvm::Function* Visitor_Codegen::visit(ast::declaration::cop::Entity_Cast& n)
{
  return nullptr;
}
llvm::Function* Visitor_Codegen::visit(ast::declaration::cop::Entity_Op& n)
{
  return nullptr;
}
llvm::Function* Visitor_Codegen::visit(ast::declaration::cop::Entity_Access_Op& n)
{
  return nullptr;
}
llvm::Function* Visitor_Codegen::visit(ast::declaration::cop::Entity_Transfert& n)
{
  return nullptr;
}

llvm::Function* Visitor_Codegen::visit(ast::declaration::cop::System& n)
{
  return nullptr;
}
void Visitor_Codegen::visit(ast::declaration::cop::System_Case& n)
{
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
llvm::Type* Visitor_Codegen::visit(ast::type::Ptr& n)
{
  if (n.llvm_type) return n.llvm_type;

  auto inner_ty = n.inner->codegen_ty(*this);

  if (n.pointer_type == EPtrType::raw_ptr) {
    return n.llvm_type = llvm::PointerType::get(inner_ty, 0);
  }

  // provisional
  return n.llvm_type;
}
llvm::Type* Visitor_Codegen::visit(ast::type::Table& n)
{
  return nullptr;
}
llvm::Type* Visitor_Codegen::visit(ast::type::Primitive& n)
{
  if (n.llvm_type) return n.llvm_type;

  if (auto ty = tools.get_primtive_type(n.type)) {
    return n.llvm_type = ty;
  } else {
    error_add(202, n, "Invalid primitive type", "");
  }

  return nullptr;
}
llvm::Type* Visitor_Codegen::visit(ast::type::Tuple& n)
{
  if (n.llvm_type) return n.llvm_type;

  if (n.types.empty()) return n.llvm_type = u0Ty;

  std::vector<llvm::Type*> tys;
  for (auto& ty : n.types) tys.emplace_back(ty->codegen_ty(*this));

  if (tys.size() == 1) return n.llvm_type = tys[0];

  return n.llvm_type = llvm::StructType::get(ctx, tys);
}
llvm::Type* Visitor_Codegen::visit(ast::type::Function_Proto& n)
{
  if (n.llvm_type) return n.llvm_type;

  auto                     ret_ty = n.return_ty ? n.return_ty->codegen_ty(*this) : u0Ty;
  std::vector<llvm::Type*> params;
  for (auto& param_ty : n.parameters) {
    auto llvm_ty = tools.generate_parameter_type(*param_ty);
    params.push_back(llvm_ty);
  }

  return n.llvm_type = llvm::FunctionType::get(ret_ty, params, n.is_variadic);
}

llvm::Type* Visitor_Codegen::visit(ast::type::Get_Expr_Type& n)
{
  if (n.llvm_type) return n.llvm_type;


  if (auto ptr = n.target->codegen(*this)) return n.llvm_type = n.target->expression_inferred_type->llvm_type;
  return nullptr;
}

// ============ LITERAL ============
llvm::Value* Visitor_Codegen::visit(ast::literal::Boolean& n)
{
  if (n.llvm_value) return n.llvm_value;


  return n.llvm_value = llvm::ConstantInt::get(i1Ty, n.val);
}
llvm::Value* Visitor_Codegen::visit(ast::literal::Integral& n)
{
  if (n.llvm_value) return n.llvm_value;

  if (auto ptr = tools.create_constant(n)) return n.llvm_value = ptr.value();
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::literal::Fixed_Point& n)
{
  if (n.llvm_value) return n.llvm_value;


  if (auto ptr = tools.create_constant(n)) return n.llvm_value = ptr.value();
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::literal::Floating_Point& n)
{
  if (n.llvm_value) return n.llvm_value;


  if (auto ptr = tools.create_constant(n)) return n.llvm_value = ptr.value();

  return nullptr;
}

llvm::Value* Visitor_Codegen::visit(ast::literal::CUNE& n)
{
  if (n.llvm_value) return n.llvm_value;

  return n.llvm_value = llvm::ConstantInt::get(i8Ty, n.val);
}
llvm::Value* Visitor_Codegen::visit(ast::literal::RUNE& n)
{
  if (n.llvm_value) return n.llvm_value;


  assert(n.code_points.size() == 4 && "UTF-32 character must be 4 bytes");
  uint32_t codePoint = 0;
  for (int i = 0; i < 4; ++i)
    codePoint |= static_cast<uint32_t>(static_cast<unsigned char>(n.code_points[i])) << (8 * i);

  return n.llvm_value = llvm::ConstantInt::get(i32Ty, codePoint);
}

llvm::Value* Visitor_Codegen::visit(ast::literal::Text_Pure& n)
{
  if (n.llvm_value) return n.llvm_value;

  auto txt = llvm::ConstantDataArray::getString(ctx, n.val, true);
  auto glo_txt =
      new llvm::GlobalVariable(*mod, txt->getType(), true, llvm::GlobalVariable::PrivateLinkage, txt, ".str");
  glo_txt->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);

  auto txt_ptr = llvm::ConstantExpr::getInBoundsGetElementPtr(txt->getType(), glo_txt, get_zero);

  if (n.text_type == EPrimType::c_str) {
    return n.llvm_value = txt_ptr;
  }

  // build the text fat pointer
  auto fat_ptr_ty   = llvm::StructType::get(ctx, i32Ty->getPointerTo(), i32Ty);
  auto lenght_const = llvm::ConstantInt::get(i32Ty, n.val.size());
  auto fat_ptr      = llvm::ConstantStruct::get(fat_ptr_ty, {txt_ptr, lenght_const});

  return n.llvm_value = fat_ptr;
}
llvm::Value* Visitor_Codegen::visit(ast::literal::Text_Interpolation& n)
{
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::literal::Textual_Format& n)
{
  if (n.values.size() == 1 && dynamic_cast<ast::literal::Text_Pure*>(n.values[0].get())) {
    return n.values[0]->codegen(*this);
  }
  for (auto& elem : n.values) {
    elem->codegen(*this);
  }
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
  if (n.llvm_value) return n.llvm_value;

  std::vector<llvm::Type*>  types;
  std::vector<llvm::Value*> vals;

  for (auto& val : n.values) {
    vals.emplace_back(val->codegen(*this));
    types.emplace_back(val->expression_inferred_type->codegen_ty(*this));
  }

  auto tuple_ty = llvm::StructType::get(ctx, types);

  auto ptr = builder.CreateAlloca(tuple_ty);

  size_t count = 0;
  for (auto val : vals) {
    auto val_ptr = builder.CreateStructGEP(tuple_ty, ptr, count++);
    builder.CreateStore(val, val_ptr);
  }

  return n.llvm_value = builder.CreateLoad(tuple_ty, ptr);
}

llvm::Value* Visitor_Codegen::visit(ast::literal::Range& n)
{
  if (n.llvm_value) return n.llvm_value;

  auto start_ty = n.start->expression_inferred_type->codegen_ty(*this);
  auto end_ty   = n.end->expression_inferred_type->codegen_ty(*this);
  auto step_ty  = n.step->expression_inferred_type->codegen_ty(*this);

  auto range_ty = llvm::StructType::get(ctx, {start_ty, end_ty, step_ty});

  auto ptr = builder.CreateAlloca(range_ty);

  auto start_ptr = builder.CreateStructGEP(range_ty, ptr, 0);
  auto end_ptr   = builder.CreateStructGEP(range_ty, ptr, 1);
  auto step_ptr  = builder.CreateStructGEP(range_ty, ptr, 2);

  auto start_val = n.start->codegen(*this);
  auto end_val   = n.end->codegen(*this);
  auto step_val  = n.step->codegen(*this);

  builder.CreateStore(start_val, start_ptr);
  builder.CreateStore(end_val, end_ptr);
  builder.CreateStore(step_val, step_ptr);

  return n.llvm_value = builder.CreateLoad(range_ty, ptr);
}

llvm::Value* Visitor_Codegen::visit(ast::literal::Iterator& n)
{
  if (n.llvm_value) return n.llvm_value;
  return nullptr;
}

llvm::Value* Visitor_Codegen::visit(ast::literal::Enum& n)
{
  if (n.llvm_value) return n.llvm_value;

  auto tag = llvm::ConstantInt::get(i32Ty, n.expression_in_type_position);

  if (!n.expression_inferred_type->llvm_type) n.expression_inferred_type->codegen_ty(*this);


  // assume type_llvm on enum = { i32, [N x i8] } ; tag, payload
  auto enum_ty = llvm::cast<llvm::StructType>(n.expression_inferred_type->llvm_type);

  // allocate enum struct
  auto ptr = builder.CreateAlloca(enum_ty);

  // store tag : on first field = 0
  auto tag_ptr = builder.CreateStructGEP(enum_ty, ptr, 0);
  builder.CreateStore(tag, tag_ptr);

  // store payload : on second field = 1
  auto payload_ptr = builder.CreateStructGEP(enum_ty, ptr, 1);

  size_t offset = 0;
  // store each enum elem values in payload
  for (auto& elem : n.member_values) {
    // get elem value
    auto val = elem->codegen(*this);

    // point on payload section with offset
    auto elem_ptr = builder.CreateGEP(i8Ty, payload_ptr, llvm::ConstantInt::get(i32Ty, offset));

    // cast value for the payload
    auto typed_ptr = builder.CreateBitCast(elem_ptr, val->getType()->getPointerTo());

    // store value in payload
    builder.CreateStore(val, typed_ptr);

    // update offset according to the last value allocation size
    offset += mod->getDataLayout().getTypeAllocSize(val->getType());
  }

  return llvm::cast<llvm::LoadInst>(n.llvm_value = builder.CreateLoad(enum_ty, ptr));
}

llvm::Value* Visitor_Codegen::visit(ast::literal::Structured_Data& n)
{
  if (n.llvm_value) return n.llvm_value;

  if (!n.expression_inferred_type->llvm_type) n.expression_inferred_type->codegen_ty(*this);


  // assume type_llvm on structured data = { field1, field2 }
  auto struct_ty = llvm::cast<llvm::StructType>(n.expression_inferred_type->llvm_type);

  // allocate structured data
  llvm::Value* v = llvm::UndefValue::get(struct_ty);

  for (auto& field : n.field_args) {
    // get field value
    auto val = field->codegen(*this);

    // get field position in struct
    size_t offset = field->expression_in_type_position;

    v = builder.CreateInsertValue(v, val, {(unsigned)offset});
  }

  return n.llvm_value = v;
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
  if (n.llvm_value) return n.llvm_value;

  llvm::Function* fn_callee;

  // if already generated
  if (n.function_symbol->codegen_pass(*this)) {
    fn_callee = llvm::cast<llvm::Function>(n.function_symbol->codegen_pass(*this));
  }
  // else, generate a stub
  else {
    llvm::Function::LinkageTypes linkage =
        n.function_symbol->declaration_is_external || n.function_symbol->declaration_is_exported
            ? llvm::Function::ExternalLinkage
            : llvm::Function::InternalLinkage;

    auto ptr  = dynamic_cast<ast::AIdentifier*>(n.callee.get());
    fn_callee = generate_stub(*n.function_proto, ptr->mangle_id_node(), linkage);

    auto fn_sym     = dynamic_cast<ast::ACallable*>(n.function_symbol.get());
    fn_sym->llvm_fn = fn_callee;
  }

  size_t                    count = 0;
  std::vector<llvm::Value*> args;
  for (auto& arg : n.param_args) {
    args.emplace_back(arg->codegen(*this));
    count++;
  }

  llvm::CallInst* result = nullptr;

  if (!n.function_proto->return_ty || n.function_proto->return_ty->is_same(*ast::type::get_void_type())) {
    result = builder.CreateCall(fn_callee, args);
  } else {
    std::string call_name = n.callee->debug_str();
    result                = builder.CreateCall(fn_callee, args, call_name);
  }

  return n.llvm_value = result;
}
llvm::Value* Visitor_Codegen::visit(ast::expression::Call_Argument& n)
{
  if (n.llvm_value) return n.llvm_value;

  // variadic arguement
  // assume C conventions
  if (n.variadic_arg) {
    if (n.expression->is_forced_lvalue()) {
      return ensure_lvalue(*n.expression);
    }
    return n.llvm_value = ensure_rvalue(*n.expression);
  }

  switch (n.fn_param_type->passmode) {
  case EPassMode::Mut:  return n.llvm_value = ensure_lvalue(*n.expression);
  case EPassMode::Move:
  case EPassMode::Ref:  {
    if (auto l_val = ensure_lvalue(*n.expression, true)) return n.llvm_value = l_val;

    return n.llvm_value = ensure_rvalue(*n.expression);
  }
  case EPassMode::Copy:
  case EPassMode::Clone: return n.llvm_value = ensure_rvalue(*n.expression, "arg." + n.fn_param_type->declaration_name);
  case EPassMode::Addr:  {
    llvm::Value* ptr     = ensure_lvalue(*n.expression);
    llvm::Type*  ptrTy   = ptr->getType();
    llvm::Value* storage = builder.CreateAlloca(ptrTy, nullptr, "addr.storage");
    builder.CreateStore(ptr, storage);
    return n.llvm_value = storage;
  }
  case EPassMode::NONE: {
    return n.llvm_value = n.expression->codegen(*this);
  }
  }
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
llvm::Value* Visitor_Codegen::visit(ast::expression::Mut_Of& n)
{
  if (n.llvm_value) return n.llvm_value;

  auto targetPtr = n.target->codegen(*this);

  if (!targetPtr->getType()->isPointerTy()) {
    throw std::runtime_error("& requires an lvalue expression");
  }

  // return the pointer directly
  n.llvm_value = targetPtr;
  return targetPtr;
}
llvm::Value* Visitor_Codegen::visit(ast::expression::Ref_Of& n)
{
  if (n.llvm_value) return n.llvm_value;

  auto targetPtr = n.target->codegen(*this);

  if (!targetPtr->getType()->isPointerTy()) {
    throw std::runtime_error("& requires an lvalue expression");
  }

  // return the pointer directly
  n.llvm_value = targetPtr;
  return targetPtr;
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

// ============ IF STATEMENT ============
void Visitor_Codegen::visit(ast::statement::If& n)
{
  auto function = builder.GetInsertBlock()->getParent();

  auto bb_then  = llvm::BasicBlock::Create(ctx, "if.then", function);
  auto bb_merge = llvm::BasicBlock::Create(ctx, "if.merge", function);

  llvm::Value* cond = n.evaluator.node ? n.evaluator.node->codegen(*this) : nullptr;

  if (n.alternative_statement) {
    auto bb_else = llvm::BasicBlock::Create(ctx, "if.alt." + n.alternative_statement->debug_str(), function);
    builder.CreateCondBr(cond, bb_then, bb_else);

    builder.SetInsertPoint(bb_then);
    n.codeblock->codegen_pass(*this);
    if (!builder.GetInsertBlock()->getTerminator()) builder.CreateBr(bb_merge);

    builder.SetInsertPoint(bb_else);
    visit(*n.alternative_statement.get());
    if (!builder.GetInsertBlock()->getTerminator()) builder.CreateBr(bb_merge);
  } else {
    if (cond)
      builder.CreateCondBr(cond, bb_then, bb_merge);
    else
      builder.CreateBr(bb_then);

    builder.SetInsertPoint(bb_then);
    n.codeblock->codegen_pass(*this);
    if (!builder.GetInsertBlock()->getTerminator()) builder.CreateBr(bb_merge);
  }

  builder.SetInsertPoint(bb_merge);
}

// ============ FOR LOOP ============
void Visitor_Codegen::visit(ast::statement::For& n)
{
  auto function = builder.GetInsertBlock()->getParent();

  llvm::AllocaInst* index_alloca = n.index ? llvm::cast<llvm::AllocaInst>(n.index->codegen(*this)) : nullptr;

  if (index_alloca) index_alloca->setName("for.index");

  auto* range = dynamic_cast<ast::literal::Range*>(n.expression.get());
  if (!range) return;

  auto start = ensure_rvalue(*range->start);
  auto end   = ensure_rvalue(*range->end);

  if (index_alloca) builder.CreateStore(start, index_alloca);

  auto bb_header = llvm::BasicBlock::Create(ctx, "for.header", function);
  auto bb_body   = llvm::BasicBlock::Create(ctx, "for.body", function);
  auto bb_after  = llvm::BasicBlock::Create(ctx, "for.after", function);

  builder.CreateBr(bb_header);

  // HEADER
  builder.SetInsertPoint(bb_header);

  llvm::Value* index = index_alloca ? builder.CreateLoad(index_alloca->getAllocatedType(), index_alloca) : nullptr;

  // IMPORTANT: canonical loop condition = index < end
  llvm::Value* cond = builder.CreateICmpULT(index, end);

  builder.CreateCondBr(cond, bb_body, bb_after);

  // BODY
  current_bb_break    = bb_after;
  current_bb_continue = bb_header;

  builder.SetInsertPoint(bb_body);

  n.codeblock->codegen_pass(*this);

  if (!builder.GetInsertBlock()->getTerminator()) {
    if (index_alloca) {
      llvm::Value* old = builder.CreateLoad(index_alloca->getAllocatedType(), index_alloca);
      llvm::Value* one = llvm::ConstantInt::get(index_alloca->getAllocatedType(), 1);
      llvm::Value* inc = builder.CreateAdd(old, one);
      builder.CreateStore(inc, index_alloca);
    }

    builder.CreateBr(bb_header);
  }

  current_bb_break    = nullptr;
  current_bb_continue = nullptr;

  // EXIT
  builder.SetInsertPoint(bb_after);
}
// ============ LOOP ============
void Visitor_Codegen::visit(ast::statement::Loop& n)
{
  auto function = builder.GetInsertBlock()->getParent();

  auto bb_header = llvm::BasicBlock::Create(ctx, "loop.header", function);
  auto bb_body   = llvm::BasicBlock::Create(ctx, "loop.body", function);
  auto bb_after  = llvm::BasicBlock::Create(ctx, "loop.after", function);

  builder.CreateBr(bb_header);

  current_bb_break    = bb_after;
  current_bb_continue = bb_header;

  builder.SetInsertPoint(bb_header);
  builder.CreateBr(bb_body);

  builder.SetInsertPoint(bb_body);
  n.codeblock->codegen_pass(*this);

  if (!builder.GetInsertBlock()->getTerminator()) builder.CreateBr(bb_header);

  current_bb_break    = nullptr;
  current_bb_continue = nullptr;

  builder.SetInsertPoint(bb_after);
}
// ============ WHILE LOOP ============
void Visitor_Codegen::visit(ast::statement::While& n)
{
  auto function = builder.GetInsertBlock()->getParent();

  auto bb_header = llvm::BasicBlock::Create(ctx, "while.header", function);
  auto bb_body   = llvm::BasicBlock::Create(ctx, "while.body", function);
  auto bb_after  = llvm::BasicBlock::Create(ctx, "while.after", function);

  builder.CreateBr(bb_header);

  builder.SetInsertPoint(bb_header);

  llvm::Value* cond = nullptr;
  if (auto ptr = n.evaluator.get_condition())
    cond = ptr->codegen(*this);
  else if (auto ptr = n.evaluator.get_pattern())
    cond = ptr->codegen(*this);

  if (n.is_do) {
    builder.CreateBr(bb_body);
  } else {
    builder.CreateCondBr(cond, bb_body, bb_after);
  }

  current_bb_break    = bb_after;
  current_bb_continue = bb_header;

  builder.SetInsertPoint(bb_body);
  n.codeblock->codegen_pass(*this);

  current_bb_break    = nullptr;
  current_bb_continue = nullptr;

  if (n.is_do) {
    llvm::Value* condDo = n.evaluator.get_condition() ? n.evaluator.get_condition()->codegen(*this)
                                                      : n.evaluator.get_pattern()->codegen(*this);
    builder.CreateCondBr(condDo, bb_body, bb_after);
  } else {
    if (!builder.GetInsertBlock()->getTerminator()) builder.CreateBr(bb_header);
  }

  builder.SetInsertPoint(bb_after);
}

// ============ GOTO / LABEL ============
void Visitor_Codegen::visit(ast::statement::GoTo& n)
{
  builder.CreateBr(n.label_sym->llvm_bb);
}

llvm::Value* Visitor_Codegen::visit(ast::statement::GoTo_Label& n)
{
  if (!n.llvm_bb) {
    auto function = builder.GetInsertBlock()->getParent();
    n.llvm_bb     = llvm::BasicBlock::Create(ctx, n.declaration_name, function);
  }

  builder.SetInsertPoint(n.llvm_bb);
  n.codeblock->codegen_pass(*this);

  if (!builder.GetInsertBlock()->getTerminator()) {
    auto dead = llvm::BasicBlock::Create(ctx, "label.dead", builder.GetInsertBlock()->getParent());
    builder.CreateBr(dead);
    builder.SetInsertPoint(dead);
  }

  return n.llvm_bb;
}

// ============ RETURN / BREAK / CONTINUE ============
llvm::ReturnInst* Visitor_Codegen::visit(ast::statement::Return& n)
{
  auto ret = n.value ? builder.CreateRet(n.value->codegen(*this)) : builder.CreateRetVoid();

  auto dead = llvm::BasicBlock::Create(ctx, "fn.dead", builder.GetInsertBlock()->getParent());
  builder.SetInsertPoint(dead);

  return ret;
}

llvm::BranchInst* Visitor_Codegen::visit(ast::statement::Break& n)
{
  return current_bb_break ? builder.CreateBr(current_bb_break) : nullptr;
}

llvm::BranchInst* Visitor_Codegen::visit(ast::statement::Continue& n)
{
  return current_bb_continue ? builder.CreateBr(current_bb_continue) : nullptr;
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
  if (n.llvm_value) return n.llvm_value;

  auto expr      = ensure_rvalue(*n.expression);
  auto target_ty = n.type->codegen_ty(*this);

  if (!expr) return nullptr;

  switch (n.cast_type) {
  case ast::operation::Cast_As::ECastType::AS: {
    if (dynamic_cast<ast::type::Primitive*>(n.expression->expression_inferred_type.get())
        && dynamic_cast<ast::type::Primitive*>(n.expression->expression_inferred_type.get())) {
      return n.llvm_value = tools.primitive_coerce(expr, expr->getType(), target_ty);
    }
  }
  case ast::operation::Cast_As::ECastType::AS_REINTERPRET: {
    return n.llvm_value = builder.CreateBitCast(expr, target_ty);
  }
  case ast::operation::Cast_As::ECastType::AS_SAFE: return n.llvm_value = builder.CreateBitCast(expr, target_ty);
  }
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
  if (n.llvm_value) return n.llvm_value;

  auto val = ensure_rvalue(*n.right);
  auto ptr = ensure_lvalue(*n.left);

  if (!val) return nullptr;
  if (!ptr) return nullptr;

  return n.llvm_value = builder.CreateStore(val, ptr);
}
llvm::Value* Visitor_Codegen::visit(ast::operation::Binary& n)
{
  auto l_val = ensure_rvalue(*n.left, "bin.l");
  auto r_val = ensure_rvalue(*n.right, "bin.r");

  if (!l_val) error_add(226, *n.left, "Can't be evaluated as value.", "");
  if (!r_val) error_add(226, *n.right, "Can't be evaluated as value.", "");
  if (!l_val || !r_val) return nullptr;

  auto op_ty      = n.expression_inferred_type;
  auto llvm_op_ty = op_ty->llvm_type;

  using Func = std::function<void()>;

  auto arith = [&](Func fn_sint, Func fn_uint, Func fn_float) {
    if (auto ptr = std::dynamic_pointer_cast<ast::type::Primitive>(n.left->expression_inferred_type)) {
      if (EPrimType_is_floating(ptr->type)) {
        if (fn_float) fn_float();
      } else if (EPrimType_is_integral(ptr->type)) {
        if (EPrimType_is_signed(ptr->type)) {
          if (fn_sint) fn_sint();
        } else {
          if (fn_uint)
            fn_uint();
          else if (fn_sint)
            fn_sint();
        }
      }
    }
  };

  auto byte = [&](Func bin) {
    if (auto ptr = std::dynamic_pointer_cast<ast::type::Primitive>(n.left->expression_inferred_type)) {
      if (ptr->type == EPrimType::boolean || EPrimType_is_byte(ptr->type)) {
        bin();
      }
    }
  };


  llvm::Value* op = nullptr;

  switch (n.op_ty) {
  case EBinOpType::NONE:
  case EBinOpType::Add:  {
    arith([&]() { op = builder.CreateAdd(l_val, r_val, "bin.add"); }, nullptr,
          [&]() { op = builder.CreateFAdd(l_val, r_val, "bin.add"); });

    break;
  }
  case EBinOpType::Sub: {
    arith([&]() { op = builder.CreateSub(l_val, r_val, "bin.sub"); }, nullptr,
          [&]() { op = builder.CreateFSub(l_val, r_val, "bin.sub"); });

    break;
  }
  case EBinOpType::Mul: {
    arith([&]() { op = builder.CreateMul(l_val, r_val, "bin.mul"); }, nullptr,
          [&]() { op = builder.CreateFMul(l_val, r_val, "bin.mul"); });

    break;
  }
  case EBinOpType::Div: {
    arith(
        [&]() {
          auto fl = builder.CreateSIToFP(l_val, fSizeTy);
          auto fr = builder.CreateSIToFP(r_val, fSizeTy);
          op      = builder.CreateFDiv(fl, fr, "bin.div");
        },
        [&]() {
          auto fl = builder.CreateUIToFP(l_val, fSizeTy);
          auto fr = builder.CreateUIToFP(r_val, fSizeTy);
          op      = builder.CreateFDiv(fl, fr, "bin.div");
        },
        [&]() { op = builder.CreateFDiv(l_val, r_val, "bin.div"); });

    break;
  }
  case EBinOpType::Mod: {
    arith(
        [&]() {
          auto rem    = builder.CreateSRem(l_val, r_val, "bin.rem");
          auto is_neg = builder.CreateICmpSLT(rem, get_zero, "is_neg");
          op          = builder.CreateSelect(is_neg, builder.CreateAdd(rem, r_val, "bin.mod_adjusted"), rem, "bin.mod");
        },
        [&]() {
          auto rem    = builder.CreateURem(l_val, r_val, "bin.rem");
          auto is_neg = builder.CreateICmpSLT(rem, get_zero, "is_neg");
          op          = builder.CreateSelect(is_neg, builder.CreateAdd(rem, r_val, "bin.mod_adjusted"), rem, "bin.mod");
        },
        nullptr);

    break;
  }
  case EBinOpType::Quo: {
    arith(
        [&]() {
          auto rem    = builder.CreateSRem(l_val, r_val, "bin.rem");
          auto is_neg = builder.CreateICmpSLT(rem, get_zero, "is_neg");
          auto mod    = builder.CreateSelect(is_neg, builder.CreateAdd(rem, r_val, "bin.mod_adjusted"), rem, "bin.mod");
          op          = builder.CreateSDiv(builder.CreateSub(l_val, mod), r_val, "bin.quo");
        },
        [&]() {
          auto rem    = builder.CreateURem(l_val, r_val, "bin.rem");
          auto is_neg = builder.CreateICmpSLT(rem, get_zero, "is_neg");
          auto mod    = builder.CreateSelect(is_neg, builder.CreateAdd(rem, r_val, "bin.mod_adjusted"), rem, "bin.mod");
          op          = builder.CreateSDiv(builder.CreateSub(l_val, mod), r_val, "bin.quo");
        },
        nullptr);

    break;
  }
  case EBinOpType::Rem: {
    arith([&]() { op = builder.CreateSRem(l_val, r_val, "bin.rem"); },
          [&]() { op = builder.CreateURem(l_val, r_val, "bin.rem"); },
          [&]() { op = builder.CreateFRem(l_val, r_val, "bin.rem"); });

    break;
  }
  case EBinOpType::Divrem: break;
  case EBinOpType::Pow:    {
    arith(
        [&]() {
          auto pow_fn = llvm::Intrinsic::getDeclaration(mod, llvm::Intrinsic::powi, {llvm_op_ty, llvm_op_ty});

          op = builder.CreateCall(pow_fn, {l_val, r_val}, "bin.pow");
        },
        nullptr,
        [&]() {
          auto pow_fn = llvm::Intrinsic::getDeclaration(mod, llvm::Intrinsic::pow, {llvm_op_ty, llvm_op_ty});

          op = builder.CreateCall(pow_fn, {l_val, r_val}, "bin.pow");
        });

    break;
  }
  case EBinOpType::Gre: {
    arith([&]() { op = builder.CreateICmpSGT(l_val, r_val, "bin.gre"); },
          [&]() { op = builder.CreateICmpUGT(l_val, r_val, "bin.gre"); },
          [&]() { op = builder.CreateFCmpOGT(l_val, r_val, "bin.gre"); });

    byte([&]() { op = builder.CreateICmpUGT(l_val, r_val, "bin.gre"); });

    break;
  }
  case EBinOpType::Low: {
    arith([&]() { op = builder.CreateICmpSLT(l_val, r_val, "bin.low"); },
          [&]() { op = builder.CreateICmpULT(l_val, r_val, "bin.low"); },
          [&]() { op = builder.CreateFCmpOLT(l_val, r_val, "bin.low"); });

    byte([&]() { op = builder.CreateICmpULT(l_val, r_val, "bin.low"); });

    break;
  }
  case EBinOpType::Gre_eq: {
    arith([&]() { op = builder.CreateICmpSGE(l_val, r_val, "bin.gre_eq"); },
          [&]() { op = builder.CreateICmpUGE(l_val, r_val, "bin.gre_eq"); },
          [&]() { op = builder.CreateFCmpOGE(l_val, r_val, "bin.gre_eq"); });

    byte([&]() { op = builder.CreateICmpUGE(l_val, r_val, "bin.gre_eq"); });

    break;
  }
  case EBinOpType::Low_eq: {
    arith([&]() { op = builder.CreateICmpSLE(l_val, r_val, "bin.low_eq"); },
          [&]() { op = builder.CreateICmpULE(l_val, r_val, "bin.low_eq"); },
          [&]() { op = builder.CreateFCmpOLE(l_val, r_val, "bin.low_eq"); });

    byte([&]() { op = builder.CreateICmpULE(l_val, r_val, "bin.low_eq"); });

    break;
  }
  case EBinOpType::_in:
  case EBinOpType::_is:
  case EBinOpType::_eq: {
    arith([&]() { op = builder.CreateICmpEQ(l_val, r_val, "bin.eq"); }, nullptr,
          [&]() { op = builder.CreateFCmpOEQ(l_val, r_val, "bin.eq"); });

    byte([&]() { op = builder.CreateICmpEQ(l_val, r_val, "bin.eq"); });

    break;
  }
  case EBinOpType::_nin:
  case EBinOpType::_nis:
  case EBinOpType::_neq: {
    arith([&]() { op = builder.CreateICmpNE(l_val, r_val, "bin.neq"); }, nullptr,
          [&]() { op = builder.CreateFCmpONE(l_val, r_val, "bin.neq"); });

    byte([&]() { op = builder.CreateICmpNE(l_val, r_val, "bin.eq"); });

    break;
  }
  case EBinOpType::_eqs: {
    arith([&]() { op = builder.CreateICmpEQ(l_val, r_val, "bin.eqs"); }, nullptr,
          [&]() { op = builder.CreateFCmpOEQ(l_val, r_val, "bin.eqs"); });

    byte([&]() { op = builder.CreateICmpEQ(l_val, r_val, "bin.eq"); });

    break;
  }
  case EBinOpType::_neqs: {
    arith([&]() { op = builder.CreateICmpNE(l_val, r_val, "bin.neq"); }, nullptr,
          [&]() { op = builder.CreateFCmpONE(l_val, r_val, "bin.neq"); });

    byte([&]() { op = builder.CreateICmpNE(l_val, r_val, "bin.eq"); });

    break;
  }
  case EBinOpType::_b_and:
  case EBinOpType::_and:   {
    byte([&]() { op = builder.CreateAnd(l_val, r_val, "bin.and"); });

    break;
  }
  case EBinOpType::_b_nand:
  case EBinOpType::_nand:   {
    byte([&]() { op = builder.CreateNot(builder.CreateAnd(l_val, r_val, "bin.nand")); });

    break;
  }
  case EBinOpType::_b_or:
  case EBinOpType::_or:   {
    byte([&]() { op = builder.CreateAnd(l_val, r_val, "bin.or"); });

    break;
  }
  case EBinOpType::_b_xor:
  case EBinOpType::_xor:   {
    byte([&]() { op = builder.CreateXor(l_val, r_val, "bin.xor"); });

    break;
  }
  case EBinOpType::_b_nor:
  case EBinOpType::_nor:   {
    byte([&]() { op = builder.CreateNot(builder.CreateOr(l_val, r_val, "bin.nor")); });

    break;
  }
  case EBinOpType::_b_xnor:
  case EBinOpType::_xnor:   {
    byte([&]() { op = builder.CreateNot(builder.CreateXor(l_val, r_val, "bin.xnor")); });

    break;
  }
  case EBinOpType::ls0: {
    byte([&]() { op = builder.CreateShl(l_val, r_val, "bin.ls0"); });

    break;
  }
  case EBinOpType::ls1: {
    auto all_ones = builder.getInt32(~0);
    auto shifted  = builder.CreateShl(l_val, r_val, "bin.ls0");
    auto mask     = builder.CreateLShr(all_ones, builder.CreateSub(builder.getInt32(32), r_val), "bin.mask");
    op            = builder.CreateOr(shifted, mask, "ls1");

    break;
  }
  // impossible
  case EBinOpType::lsa: break;
  case EBinOpType::rs0: {
    byte([&]() { op = builder.CreateLShr(l_val, r_val, "bin.rs0"); });

    break;
  }
  case EBinOpType::rs1: {
    auto lshr     = builder.CreateLShr(l_val, r_val, "lshr");
    auto all_ones = builder.getInt32(~0);
    auto shifted  = builder.CreateSub(builder.getInt32(32), r_val, "bin.shifted");
    auto mask     = builder.CreateShl(all_ones, shifted, "bin.mask");
    op            = builder.CreateOr(lshr, mask, "bin.rs1");

    break;
  }
  case EBinOpType::rsa: {
    byte([&]() { op = builder.CreateAShr(l_val, r_val, "bin.rsa"); });

    break;
  }
  case EBinOpType::lr: {
    byte([&]() {
      auto fshr_fn = llvm::Intrinsic::getDeclaration(mod, llvm::Intrinsic::fshl, {l_val->getType()});
      op           = builder.CreateCall(fshr_fn, {l_val, l_val, r_val}, "bin.lr");
    });

    break;
  }
  case EBinOpType::rr: {
    byte([&]() {
      auto fshr_fn = llvm::Intrinsic::getDeclaration(mod, llvm::Intrinsic::fshr, {l_val->getType()});
      op           = builder.CreateCall(fshr_fn, {l_val, l_val, r_val}, "bin.rr");
    });

    break;
  }
  }

  if (!op)
    error_add(207, n,
              "Illegal operation  (" + EBinOpType_to_str(n.op_ty) + ") between types: \n\""
                  + n.left->expression_inferred_type->debug_str() + "\" " + EBinOpType_to_str(n.op_ty) + " \""
                  + n.right->expression_inferred_type->debug_str() + "\"",
              "");

  return op;
}
llvm::Value* Visitor_Codegen::visit(ast::operation::Unary& n)
{
  auto base_ptr = ensure_rvalue(*n.base, "un.base");

  auto op_ty      = n.base->expression_inferred_type;
  auto llvm_op_ty = op_ty->llvm_type;

  llvm::Value* op;

  switch (n.unary_op) {
  case EUnaryOpType::NONE: break;
  case EUnaryOpType::_not: {
    if (auto ptr = std::dynamic_pointer_cast<ast::type::Primitive>(op_ty)) {
      if (ptr->type == EPrimType::boolean || EPrimType_is_byte(ptr->type)) {
        op = builder.CreateNot(base_ptr, "un.not");
      }
    }

    break;
  }
  case EUnaryOpType::_plus: {
    if (auto ptr = std::dynamic_pointer_cast<ast::type::Primitive>(op_ty)) {
      if (EPrimType_is_integral(ptr->type)) {
        unsigned bits = llvm_op_ty->getIntegerBitWidth();
        auto     mask = llvm::APInt(bits, 1);
        mask <<= (bits - 1);

        auto inv = ~mask;

        op = builder.CreateAnd(base_ptr, llvm::ConstantInt::get(llvm_op_ty, inv), "un.plus");
      } else if (EPrimType_is_floating(ptr->type)) {
        llvm::Type* f_ty_size = nullptr;

        if (ptr->type == EPrimType::f16)
          f_ty_size = f16Ty;
        else if (ptr->type == EPrimType::f32)
          f_ty_size = f32Ty;
        else if (ptr->type == EPrimType::f64)
          f_ty_size = f64Ty;
        else if (ptr->type == EPrimType::f80)
          f_ty_size = f80Ty;
        else if (ptr->type == EPrimType::f128)
          f_ty_size = f128Ty;
        else if (ptr->type == EPrimType::fSize)
          f_ty_size = fSizeTy;

        auto bits    = builder.CreateBitCast(base_ptr, f_ty_size);
        auto mask    = llvm::ConstantInt::get(bits->getType(), 1u << (f_ty_size->getPrimitiveSizeInBits() - 1));
        auto flipped = builder.CreateAnd(bits, mask);
        op           = builder.CreateBitCast(flipped, llvm_op_ty, "un.plus");
      }
    }

    break;
  }
  case EUnaryOpType::_minus: {
    if (auto ptr = std::dynamic_pointer_cast<ast::type::Primitive>(op_ty)) {
      if (EPrimType_is_integral(ptr->type)) {
        unsigned bits = llvm_op_ty->getIntegerBitWidth();
        auto     mask = llvm::APInt(bits, 1);
        mask <<= (bits - 1);

        auto inv = ~mask;

        op = builder.CreateOr(base_ptr, llvm::ConstantInt::get(llvm_op_ty, inv), "un.minus");
      } else if (EPrimType_is_floating(ptr->type)) {
        llvm::Type* f_ty_size = nullptr;

        if (ptr->type == EPrimType::f16)
          f_ty_size = f16Ty;
        else if (ptr->type == EPrimType::f32)
          f_ty_size = f32Ty;
        else if (ptr->type == EPrimType::f64)
          f_ty_size = f64Ty;
        else if (ptr->type == EPrimType::f80)
          f_ty_size = f80Ty;
        else if (ptr->type == EPrimType::f128)
          f_ty_size = f128Ty;
        else if (ptr->type == EPrimType::fSize)
          f_ty_size = fSizeTy;

        auto bits    = builder.CreateBitCast(base_ptr, f_ty_size);
        auto mask    = llvm::ConstantInt::get(bits->getType(), 1u << (f_ty_size->getPrimitiveSizeInBits() - 1));
        auto flipped = builder.CreateOr(bits, mask);
        op           = builder.CreateBitCast(flipped, llvm_op_ty, "un.minus");
      }
    }

    break;
  }
  case EUnaryOpType::_invert_sign: {
    if (auto ptr = std::dynamic_pointer_cast<ast::type::Primitive>(op_ty)) {
      if (EPrimType_is_integral(ptr->type)) {
        unsigned bits = llvm_op_ty->getIntegerBitWidth();
        auto     mask = llvm::APInt(bits, 1);
        mask <<= (bits - 1);

        auto inv = ~mask;

        op = builder.CreateXor(base_ptr, llvm::ConstantInt::get(llvm_op_ty, inv), "un.inv");
      } else if (EPrimType_is_floating(ptr->type)) {
        llvm::Type* f_ty_size = nullptr;

        if (ptr->type == EPrimType::f16)
          f_ty_size = f16Ty;
        else if (ptr->type == EPrimType::f32)
          f_ty_size = f32Ty;
        else if (ptr->type == EPrimType::f64)
          f_ty_size = f64Ty;
        else if (ptr->type == EPrimType::f80)
          f_ty_size = f80Ty;
        else if (ptr->type == EPrimType::f128)
          f_ty_size = f128Ty;
        else if (ptr->type == EPrimType::fSize)
          f_ty_size = fSizeTy;

        auto bits    = builder.CreateBitCast(base_ptr, f_ty_size);
        auto mask    = llvm::ConstantInt::get(bits->getType(), 1u << (f_ty_size->getPrimitiveSizeInBits() - 1));
        auto flipped = builder.CreateXor(bits, mask);
        op           = builder.CreateBitCast(flipped, llvm_op_ty, "un.inv");
      }
    }

    break;
  }
  }

  if (!op)
    error_add(207, n,
              "Illegal unary operation (" + EUnaryOpType_to_str(n.unary_op) + ") on type \""
                  + n.expression_inferred_type->debug_str() + "\"",
              "");


  return op;
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
void Visitor_Codegen::visit(ast::memory::Del& n)
{
}
void Visitor_Codegen::visit(ast::memory::Align& n)
{
}
void Visitor_Codegen::visit(ast::memory::Drop& n)
{
}