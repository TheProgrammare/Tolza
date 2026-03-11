#include "codegen_tools.hpp"

#include "ast/ast_base.hpp"
#include "ast/ast_type.hpp"

llvm::Value* llvm_tools::create_const(ast::AType& ty, ast::AExpression& value)
{

  if (auto ptr_ty = dynamic_cast<ast::type::Primitive*>(&ty)) {
    switch (ptr_ty->type) {

    case EPrimType::NONE:
    case EPrimType::boolean:
    case EPrimType::ASCII:
    case EPrimType::UTF32:
    case EPrimType::str:
    case EPrimType::text:
    case EPrimType::iSize:
    case EPrimType::i8:
    case EPrimType::i16:
    case EPrimType::i32:
    case EPrimType::i64:
    case EPrimType::i128:
    case EPrimType::uSize:
    case EPrimType::u8:
    case EPrimType::u16:
    case EPrimType::u32:
    case EPrimType::u64:
    case EPrimType::u128:
    case EPrimType::bSize:
    case EPrimType::b8:
    case EPrimType::b16:
    case EPrimType::b32:
    case EPrimType::b64:
    case EPrimType::b128:
    case EPrimType::ptrdiff:
    case EPrimType::fSize:
    case EPrimType::f32:
    case EPrimType::f64:
    case EPrimType::f128:
    case EPrimType::Void:
    case EPrimType::deci:
    case EPrimType::udeci:
    case EPrimType::Enum:
    case EPrimType::Flag:
    case EPrimType::Component:
    case EPrimType::Role:
    case EPrimType::Entity:
    case EPrimType::Generic:
    case EPrimType::Function:
    case EPrimType::Fn_Proto:
    case EPrimType::tuple:
    case EPrimType::Array:
    case EPrimType::map:
    case EPrimType::Range:
    case EPrimType::Iterator:
    case EPrimType::Slice:     break;
    }
  }
}