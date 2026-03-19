#include "visitor_codegen.hpp"

#include <cstdint>
#include <filesystem>

#include <iostream>
#include <llvm-19/llvm/IR/GlobalValue.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>


#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/Support/Casting.h>
#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Value.h>
#include <memory>
#include <vector>


#include "ast/ast_data.hpp"
#include "codegen_tools.hpp"
#include "static_evaluation.hpp"
#include "compiler.hpp"
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
#include "static_evaluation.hpp"
#include "visitor/symbol_manager.hpp"


Visitor_Codegen::Visitor_Codegen(ScriptInfo& _scr_info)
  : scr_info(_scr_info)
  , ctx(*new llvm::LLVMContext())
  , mod(*new llvm::Module(std::filesystem::path(_scr_info.file_path).filename().stem().c_str(), ctx))
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
  , iSizeTy(llvm::Type::getIntNTy(ctx, compiler::COMP_CTX.target_bits))
  , f32Ty(llvm::Type::getFloatTy(ctx))
  , f64Ty(llvm::Type::getDoubleTy(ctx))
  , f128Ty(llvm::Type::getFP128Ty(ctx))
  , fSizeTy(compiler::COMP_CTX.target_bits == 32 ? f32Ty : f64Ty)
  , strTy(llvm::StructType::get(ctx, {i32Ty->getPointerTo(), i32Ty}))
  , zero(llvm::ConstantInt::get(i32Ty, 0))
{
}

void Visitor_Codegen::build_init_func()
{
  if (init_func) return;

  auto ty = llvm::FunctionType::get(u0Ty, false);
  auto fn = llvm::Function::Create(ty, llvm::Function::ExternalLinkage, "init_module_" + mod.getName(), mod);

  auto bb = llvm::BasicBlock::Create(ctx, "entry", fn);
  builder.CreateRetVoid();

  init_func = fn;
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

llvm::Function* Visitor_Codegen::generate_stub(ast::type::Function_Proto& proto, const std::string& name,
                                               llvm::Function::LinkageTypes link_ty)
{
  if (auto func = mod.getFunction(name)) return func;

  auto fn_ty = llvm::cast<llvm::FunctionType>(proto.codegen_ty(*this));
  auto fn    = llvm::Function::Create(fn_ty, link_ty, name, mod);

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
  return n.llvm_value = n.symbol->symbol->codegen_pass(*this);
}
llvm::Value* Visitor_Codegen::visit(ast::Expr_ID_Qualified& n)
{
  if (n.llvm_value) return n.llvm_value;
  return n.llvm_value = n.symbol->symbol->codegen_pass(*this);
}
llvm::Value* Visitor_Codegen::visit(ast::Expr_ID_Type& n)
{
  if (n.llvm_value) return n.llvm_value;
  return n.llvm_value = n.codegen(*this);
}
llvm::Type* Visitor_Codegen::visit_ty(ast::Expr_ID_Type& n)
{
  if (n.llvm_type) return n.llvm_type;
  if (n.inferred_type) {
    return n.llvm_type = n.inferred_type->codegen_ty(*this);
  } else if (n.name->inferred_type) {
    return n.llvm_type = n.name->inferred_type->codegen_ty(*this);
  } else if (auto ptr = dynamic_cast<ast::AType*>(n.symbol->symbol.get())) {
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

  mod.print(llvm::outs(), nullptr);
}

// ============ DECLARATION ============
llvm::Value* Visitor_Codegen::visit(ast::declaration::Global& n)
{
  if (n.llvm_value) return n.llvm_value;


  auto ty = n.type->codegen_ty(*this);
  if (n.kind == EVariableKind::Const) {
    if (n.is_external) {
      return n.llvm_value =
                 new llvm::GlobalVariable(mod, ty, true, llvm::GlobalVariable::ExternalLinkage, nullptr, n.name);
    }
    if (auto expr = eval.evaluate_expression(*n.expression)) {
      if (auto expr_const = tools.create_constant(*n.expression->inferred_type, *expr.value())) {
        return n.llvm_value = new llvm::GlobalVariable(mod, ty, true,
                                                       n.is_exported ? llvm::GlobalVariable::ExternalLinkage
                                                                     : llvm::GlobalVariable::InternalLinkage,
                                                       expr_const.value(), n.name);
      }
    }
  } else {
    auto glo = new llvm::GlobalVariable(
        mod, ty, false, n.is_exported ? llvm::GlobalVariable::ExternalLinkage : llvm::GlobalVariable::InternalLinkage,
        nullptr, n.name);

    build_init_func();
    auto&             bb = init_func->getEntryBlock();
    llvm::IRBuilder<> irb(&bb);
    irb.SetInsertPoint(bb.getTerminator()); // before ret
    irb.CreateStore(n.expression->codegen(*this), glo);

    return n.llvm_value = glo;
  }
}
llvm::Function* Visitor_Codegen::visit(ast::declaration::Function& n)
{
  if (n.llvm_fn) return n.llvm_fn;

  const bool is_main = n.name == "main";

  llvm::Function::LinkageTypes linkage =
      n.is_exported || n.is_external ? llvm::Function::ExternalLinkage : llvm::Function::InternalLinkage;

  llvm::FunctionType* fn_ty;

  if (is_main) {
    linkage = llvm::Function::ExternalLinkage;

    std::vector<llvm::Type*> param_tys;
    for (auto& param : n.prototype->parameters) param_tys.emplace_back(param->type->codegen_ty(*this));
    fn_ty = llvm::FunctionType::get(i32Ty, param_tys, false);
  } else {
    fn_ty = llvm::cast<llvm::FunctionType>(n.prototype->codegen_ty(*this));
  }

  auto fn = llvm::Function::Create(fn_ty, linkage, n.name, mod);

  if (!n.codeblock) return fn;

  auto entry = llvm::BasicBlock::Create(ctx, "entry", fn);
  builder.SetInsertPoint(entry);

  n.codeblock->codegen_pass(*this);

  if (is_main)
    builder.CreateRet(llvm::ConstantInt::get(i32Ty, 0));
  else if (!entry->getTerminator())
    builder.CreateRetVoid();
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
    size_t elem_size  = mod.getDataLayout().getTypeAllocSize(elem_ty);
    size_t elem_align = mod.getDataLayout().getABITypeAlign(elem_ty).value();

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
    size_t elem_size  = mod.getDataLayout().getTypeAllocSize(elem_ty);
    size_t elem_align = mod.getDataLayout().getABITypeAlign(elem_ty).value();

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
  if (n.llvm_type) return n.llvm_type;

  return n.llvm_type = n.type->codegen_ty(*this);
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
      if (auto ptr = dynamic_cast<ast::AType*>(elem.data_base.get()))
        ptr->codegen_ty(*this);
      else if (auto ptr = dynamic_cast<ast::AExpression*>(elem.data_base.get()))
        ptr->codegen(*this);
      else if (auto ptr = dynamic_cast<ast::ADeclaration*>(elem.data_base.get()))
        ptr->codegen_pass(*this);
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

void Visitor_Codegen::visit(ast::declaration::local::Parameter& n)
{
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
  return nullptr;
}
void Visitor_Codegen::visit(ast::declaration::local::Tuple_Destructuring& n)
{
}
llvm::Value* Visitor_Codegen::visit(ast::declaration::local::Variable& n)
{
  if (n.llvm_value) return n.llvm_value;


  if (n.kind == EVariableKind::Const) {
    if (auto ptr = eval.evaluate_expression(*n.expression)) {
      if (auto val_const = tools.create_constant(*n.type, *ptr.value())) {
        return n.llvm_value = val_const.value();
      }
    }
  }

  llvm::AllocaInst* alloca;
  if (auto ptr_ty_table = dynamic_cast<ast::type::Table*>(n.type.get())) {
    if (ptr_ty_table->table_size.has_value()) {
      auto ty = ptr_ty_table->inner->codegen_ty(*this);
      alloca  = builder.CreateAlloca(ty, builder.getInt32(ptr_ty_table->table_size.value()), n.name);
    } else {
      auto ty = n.type->codegen_ty(*this);
      alloca  = builder.CreateAlloca(ty, nullptr, n.name);
    }
  } else {
    auto ty = n.type->codegen_ty(*this);
    alloca  = builder.CreateAlloca(ty, nullptr, n.name);
  }

  if (n.expression) {
    builder.CreateStore(n.expression->codegen(*this), alloca, n.type->type_isVolatile);
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
    auto ty = n.right->inferred_type->codegen_ty(*this)->getPointerTo();

    ptr = builder.CreateAlloca(ty, nullptr, n.name);
    break;
  }
  case ECapability::Copy:
  case ECapability::Clone: {
    auto ty = n.right->inferred_type->codegen_ty(*this);

    ptr = builder.CreateAlloca(ty, nullptr, n.name);
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
    tys.emplace_back(comp->inferred_type->codegen_ty(*this));
  }

  return n.llvm_type = llvm::StructType::get(ctx, tys);
}

llvm::Type* Visitor_Codegen::visit(ast::declaration::cop::Entity& n)
{
  if (n.llvm_type) return n.llvm_type;

  std::vector<llvm::Type*> tys;
  for (auto& comp : n.comps) {
    comp->codegen(*this);
    tys.emplace_back(comp->inferred_type->codegen_ty(*this));
  }

  return n.llvm_type = llvm::StructType::get(ctx, tys);
}
llvm::Function* Visitor_Codegen::visit(ast::declaration::cop::Entity_New& n)
{
  if (n.llvm_fn) return n.llvm_fn;

  auto fn_ty = llvm::cast<llvm::FunctionType>(n.prototype->codegen_ty(*this));

  auto fn = llvm::Function::Create(fn_ty,
                                   n.parent_entity->is_external || n.parent_entity->is_exported
                                       ? llvm::Function::ExternalLinkage
                                       : llvm::Function::InternalLinkage,
                                   n.mangle_scope() + n.name);

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
                                   n.parent_entity->is_external || n.parent_entity->is_exported
                                       ? llvm::Function::ExternalLinkage
                                       : llvm::Function::InternalLinkage,
                                   n.mangle_scope() + n.name);

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
llvm::Function* Visitor_Codegen::visit(ast::declaration::cop::Entity_OpIndex& n)
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
}
llvm::Type* Visitor_Codegen::visit(ast::type::Table& n)
{
  return nullptr;
}
llvm::Type* Visitor_Codegen::visit(ast::type::Primitive& n)
{
  if (n.llvm_type) return n.llvm_type;

  return n.llvm_type = tools.get_primtive_type(n.type);
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


  auto                     ret_ty = n.returnType->codegen_ty(*this);
  std::vector<llvm::Type*> params;
  for (auto& param_ty : n.parameters) params.push_back(tools.generate_parameter_type(*param_ty));

  return n.llvm_type = llvm::FunctionType::get(ret_ty, params, n.isVariadic);
}

llvm::Type* Visitor_Codegen::visit(ast::type::Get_Expr_Type& n)
{
  if (n.llvm_type) return n.llvm_type;


  if (auto ptr = n.target->codegen(*this)) return n.llvm_type = n.target->inferred_type->llvm_type;
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

  if (auto ptr = tools.create_constant(*n.inferred_type, n)) return n.llvm_value = ptr.value();
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::literal::Decimal& n)
{
  if (n.llvm_value) return n.llvm_value;


  if (auto ptr = tools.create_constant(*n.inferred_type, n)) return n.llvm_value = ptr.value();
  return nullptr;
}
llvm::Value* Visitor_Codegen::visit(ast::literal::Floating& n)
{
  if (n.llvm_value) return n.llvm_value;


  if (auto ptr = tools.create_constant(*n.inferred_type, n)) return n.llvm_value = ptr.value();

  return nullptr;
}

llvm::Value* Visitor_Codegen::visit(ast::literal::ASCII& n)
{
  if (n.llvm_value) return n.llvm_value;


  return n.llvm_value = llvm::ConstantInt::get(i8Ty, n.val);
}
llvm::Value* Visitor_Codegen::visit(ast::literal::UTF32& n)
{
  if (n.llvm_value) return n.llvm_value;


  assert(n.codePoints.size() == 4 && "UTF-32 character must be 4 bytes");
  uint32_t codePoint = 0;
  for (int i = 0; i < 4; ++i)
    codePoint |= static_cast<uint32_t>(static_cast<unsigned char>(n.codePoints[i])) << (8 * i);

  return n.llvm_value = llvm::ConstantInt::get(i32Ty, codePoint);
}

llvm::Value* Visitor_Codegen::visit(ast::literal::Text& n)
{
  if (n.llvm_value) return n.llvm_value;

  if (n.is_c_string) {
    auto txt = llvm::ConstantDataArray::getString(ctx, n.val, true);
    auto glo_txt =
        new llvm::GlobalVariable(mod, txt->getType(), true, llvm::GlobalVariable::PrivateLinkage, txt, ".cstr");
    glo_txt->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);

    return n.llvm_value = llvm::ConstantExpr::getInBoundsGetElementPtr(txt->getType(), glo_txt, zero);
  }

  std::vector<llvm::Constant*> elements;

  // build [N x i32] type const table
  for (char32_t c : n.val) {
    elements.push_back(llvm::ConstantInt::get(i32Ty, static_cast<uint32_t>(c)));
  }
  auto arr_ty    = llvm::ArrayType::get(i32Ty, elements.size());
  auto arr_const = llvm::ConstantArray::get(arr_ty, elements);

  // build global constant variable to store the text
  auto glo_arr = new llvm::GlobalVariable(mod, arr_ty, true, llvm::GlobalVariable::PrivateLinkage, arr_const, ".txt");
  glo_arr->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);

  // point to the table first character
  auto arr_ptr = llvm::ConstantExpr::getGetElementPtr(arr_ty, glo_arr, zero);

  // build the text fat pointer
  auto fat_ptr_ty   = llvm::StructType::get(ctx, i32Ty->getPointerTo(), i32Ty);
  auto lenght_const = llvm::ConstantInt::get(i32Ty, n.val.size());
  auto fat_ptr      = llvm::ConstantStruct::get(fat_ptr_ty, {arr_ptr, lenght_const});

  return n.llvm_value = fat_ptr;
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
  if (n.is_pure_literal_text) return n.values[0].val->codegen(*this);

  for (auto& elem : n.values) {
    elem.val->codegen(*this);
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
    types.emplace_back(val->inferred_type->codegen_ty(*this));
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

  auto start_ty = n.start->inferred_type->codegen_ty(*this);
  auto end_ty   = n.end->inferred_type->codegen_ty(*this);
  auto step_ty  = n.step->inferred_type->codegen_ty(*this);

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

  auto tag = llvm::ConstantInt::get(i32Ty, n.in_type_position);

  if (!n.inferred_type->llvm_type) n.inferred_type->codegen_ty(*this);


  // assume type_llvm on enum = { i32, [N x i8] } ; tag, payload
  auto enum_ty = llvm::cast<llvm::StructType>(n.inferred_type->llvm_type);

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
    offset += mod.getDataLayout().getTypeAllocSize(val->getType());
  }

  return llvm::cast<llvm::LoadInst>(n.llvm_value = builder.CreateLoad(enum_ty, ptr));
}

llvm::Value* Visitor_Codegen::visit(ast::literal::Structured_Data& n)
{
  if (n.llvm_value) return n.llvm_value;

  if (!n.inferred_type->llvm_type) n.inferred_type->codegen_ty(*this);


  // assume type_llvm on structured data = { field1, field2 }
  auto struct_ty = llvm::cast<llvm::StructType>(n.inferred_type->llvm_type);

  // allocate structured data
  llvm::Value* v = llvm::UndefValue::get(struct_ty);

  for (auto& field : n.field_args) {
    // get field value
    auto val = field->codegen(*this);

    // get field position in struct
    size_t offset = field->in_type_position;

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
  if (!n.function_symbol->llvm_symbol) {
    if (auto ptr = dynamic_cast<ast::AIdentifier*>(n.callee.get())) {
      fn_callee = generate_stub(*n.function_proto, ptr->get_base_name(),
                                n.function_symbol->symbol->_scr_info == n._scr_info ? llvm::Function::InternalLinkage
                                                                                    : llvm::Function::ExternalLinkage);
    }
  } else {
    fn_callee = llvm::cast<llvm::Function>(n.function_symbol->llvm_symbol);
  }

  std::vector<llvm::Value*> args;
  for (auto& arg : n.param_args) args.emplace_back(arg->codegen(*this));
  auto result = builder.CreateCall(fn_callee, args, "tmp_call");

  return n.llvm_value = result;
}
llvm::Value* Visitor_Codegen::visit(ast::expression::Call_Argument& n)
{
  if (n.llvm_value) return n.llvm_value;

  return n.llvm_value = n.expression->codegen(*this);
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
void Visitor_Codegen::visit(ast::statement::If& n)
{
  auto bb_then = llvm::BasicBlock::Create(ctx, "then");
  auto bb_else = llvm::BasicBlock::Create(ctx, "else");

  auto bb_merge = llvm::BasicBlock::Create(ctx, "merge");

  auto cond = n.evaluator.node->codegen(*this);

  builder.CreateCondBr(cond, bb_then, bb_else);

  if (n.codeblock) {
    builder.SetInsertPoint(bb_then);
    n.codeblock->codegen_pass(*this);
  }

  if (n.alternative_statement) {
    builder.SetInsertPoint(bb_else);
    visit(*n.alternative_statement.get());
  }
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
llvm::Value* Visitor_Codegen::visit(ast::statement::GoTo& n)
{
}
void Visitor_Codegen::visit(ast::statement::GoTo_Label& n)
{
}

llvm::ReturnInst* Visitor_Codegen::visit(ast::statement::Return& n)
{
  return nullptr;
} // optionnel : retourne la instruction
llvm::BranchInst* Visitor_Codegen::visit(ast::statement::Break& n)
{
  return nullptr;
}
llvm::BranchInst* Visitor_Codegen::visit(ast::statement::Continue& n)
{
  return nullptr;
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
void Visitor_Codegen::visit(ast::memory::Del& n)
{
}
void Visitor_Codegen::visit(ast::memory::Align& n)
{
}
void Visitor_Codegen::visit(ast::memory::Drop& n)
{
}