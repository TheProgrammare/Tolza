#include "id/nodeid.hpp"

#include "ast/data.hpp"
#include "ast/node/base.hpp"
#include "ast/pool.hpp"
#include "ast/tool.hpp"
#include "compiler/compilation_unit.hpp"
#include "compiler/compiler.hpp"
#include "id/base.hpp"
#include "id/defid.hpp"
#include "id/modid.hpp"
#include "id/scpid.hpp"
#include "id/tokid.hpp"
#include "id/typeid.hpp"
#include "pool/node_resolved.hpp"
#include "pool/node_to_inf.hpp"
#include "pool/node_to_metadata.hpp"

#include <cassert>
#include <string>

ast::ID ast::ID::canonical() const noexcept
{
  assert(*this && "Must be valid id");

  const auto k = kind();

  if (ast::ENodeKind_is_symbol(k)) {
    if (auto defid = COMPILER.resolved.get_definition(*this)) {
      if (auto nodeid = defid.node()) return nodeid;
    }
  }

  return *const_cast<ast::ID*>(this);
}
ast::ENodeKind ast::ID::kind() const noexcept
{
  assert(*this && "Must be valid id");
  return get().kind;
}
token::ID ast::ID::token() const noexcept
{
  assert(*this && "Must be valid id");
  return get().start_tokid;
}
type::ID ast::ID::type() const noexcept
{
  assert(*this && "Must be valid id");
  if (auto inf = COMPILER.inference.get_inference(*this)) return inf;

  return ast::get_decl_type(*this);
}
bool ast::ID::is_inferred() const noexcept
{
  assert(*this && "Must be valid id");
  return COMPILER.inference.is_inferred(*this);
}
definition::ID ast::ID::def() const noexcept
{
  assert(*this && "Must be valid id");

  if (auto defid = COMPILER.resolved.get_definition(*this)) return defid;

  return NO_ID;
}
bool ast::ID::is_resolved() const noexcept
{
  assert(*this && "Must be valid id");

  return COMPILER.resolved.is_resolved(*this);
}
scope::ID ast::ID::scope() const noexcept
{
  assert(*this && "Must be valid id");
  return get().scpid;
}
module::ID ast::ID::module() const noexcept
{
  assert(*this && "Must be valid id");
  return scope().module();
}
ast::ID ast::ID::value() const noexcept
{
  assert(*this && "Must be valid id");
  return get_value(*this);
}


bool ast::ID::is_rvalue() const noexcept
{
  assert(*this && "Must be valid id");

  const auto k = kind();

  assert(k != ENodeKind::NONE && "invalid node facial kind");
  if (k == ENodeKind::Expression_Table_Access) return false;
  if (k >= ENodeKind::Literal_Boolean && k <= ENodeKind::Literal_Record) return true;
  if (k >= ENodeKind::Expression_If_Ternary && k <= ENodeKind::Expression_New_Ptr) return true;
  if (k >= ENodeKind::Operation_Cast_As && k <= ENodeKind::Operation_Interval) return true;

  return false;
}
bool ast::ID::is_lvalue() const noexcept
{
  assert(*this && "Must be valid id");
  return !is_rvalue();
}
template <ast::Generic T>
T* ast::ID::as() noexcept
{
  assert(*this && "Must be valid id");
  return cu().get().ast->as<T>(*this);
}
template <ast::Generic T>
const T* ast::ID::as() const noexcept
{
  assert(*this && "Must be valid id");
  return cu().get().ast->as<T>(*this);
}
template <ast::Generic T>
bool ast::ID::is() const noexcept
{
  assert(*this && "Must be valid id");
  return cu().get().ast->get(*this).kind == T::static_kind;
}
bool ast::ID::is_builtin() const noexcept
{
  assert(*this && "Must be valid id");
  return raw() < ast::NODEID_USER_START;
}

semantic::Metadata* ast::ID::sem() noexcept
{
  assert(*this && "Must be valid id");
  return COMPILER.semantic_metadata.get_metadata(*this);
}
const semantic::Metadata* ast::ID::sem() const noexcept
{
  assert(*this && "Must be valid id");
  return COMPILER.semantic_metadata.get_metadata(*this);
}

/*
 * =============================================================================
 *  Template Instanciation Section
 * =============================================================================
 * To avoid any massive inclusion in headers and keep easy identifier usage
 * -----------------------------------------------------------------------------
 */


#include "ast/node/base.hpp"
#include "ast/node/declaration_extension.hpp"
#include "ast/node/declaration_global.hpp"
#include "ast/node/declaration_local.hpp"
#include "ast/node/declaration_sfm.hpp"
#include "ast/node/expression.hpp"
#include "ast/node/generic.hpp"
#include "ast/node/literal.hpp"
#include "ast/node/numeric_128_bits.hpp"
#include "ast/node/operation.hpp"
#include "ast/node/statement.hpp"
#include "ast/pool.hpp"

#define AST_GET_INSTANCE(T)                                                                                            \
  template T*       ast::ID::as<T>() noexcept;                                                                         \
  template const T* ast::ID::as<T>() const noexcept;                                                                   \
  template bool     ast::ID::is<T>() const noexcept;


AST_GET_INSTANCE(ast::Unknown)

AST_GET_INSTANCE(ast::Symbol_Id)
AST_GET_INSTANCE(ast::Symbol_Qualified)
AST_GET_INSTANCE(ast::Symbol_Type)

AST_GET_INSTANCE(ast::Path_Regex)
AST_GET_INSTANCE(ast::Root)

AST_GET_INSTANCE(ast::Import)
AST_GET_INSTANCE(ast::Global_Variable)
AST_GET_INSTANCE(ast::Global_Function)
AST_GET_INSTANCE(ast::Call_Contract)
AST_GET_INSTANCE(ast::Global_Extend_Fn)
AST_GET_INSTANCE(ast::Global_Extend_Cast)
AST_GET_INSTANCE(ast::Global_Extend_Op_Bin)
AST_GET_INSTANCE(ast::Global_Extend_Op_Un)
AST_GET_INSTANCE(ast::Global_Extend_Op_Subscript)
AST_GET_INSTANCE(ast::Global_Extend_Op_Transfert)
AST_GET_INSTANCE(ast::Global_Extend_Op_Other)
AST_GET_INSTANCE(ast::Global_Module)
AST_GET_INSTANCE(ast::Global_Extern)
AST_GET_INSTANCE(ast::Global_Export)
AST_GET_INSTANCE(ast::Global_Reexport)
AST_GET_INSTANCE(ast::Global_Enum)
AST_GET_INSTANCE(ast::Global_Flag)
AST_GET_INSTANCE(ast::Global_Union)
AST_GET_INSTANCE(ast::Global_Alias_Type)
AST_GET_INSTANCE(ast::Global_Alias_Module)
AST_GET_INSTANCE(ast::Global_Generic)
AST_GET_INSTANCE(ast::Enum_Field)
AST_GET_INSTANCE(ast::Flag_Field)
AST_GET_INSTANCE(ast::Union_Field)

AST_GET_INSTANCE(ast::CodeBlock)
AST_GET_INSTANCE(ast::Local_Lambda)
AST_GET_INSTANCE(ast::Local_Lambda_Capture)
AST_GET_INSTANCE(ast::Local_Parameter)
AST_GET_INSTANCE(ast::Local_Gen_Param_Elem)
AST_GET_INSTANCE(ast::Local_Gen_Params)
AST_GET_INSTANCE(ast::Local_Pattern_Element)
AST_GET_INSTANCE(ast::Local_Pattern_Enum)
AST_GET_INSTANCE(ast::Local_Pattern_Tuple)
AST_GET_INSTANCE(ast::Local_Pattern_Form)
AST_GET_INSTANCE(ast::Local_Pattern_Rule_Facet)
AST_GET_INSTANCE(ast::Local_Pattern_Facet)
AST_GET_INSTANCE(ast::Local_Binding)
AST_GET_INSTANCE(ast::Local_Tuple_Destructuring)
AST_GET_INSTANCE(ast::Local_Variable)
AST_GET_INSTANCE(ast::Local_Capability)

AST_GET_INSTANCE(ast::SFM_Facet)
AST_GET_INSTANCE(ast::SFM_Facet_Field)
AST_GET_INSTANCE(ast::SFM_View)
AST_GET_INSTANCE(ast::SFM_Form)
AST_GET_INSTANCE(ast::SFM_Rule)
AST_GET_INSTANCE(ast::SFM_Rule_Case)

AST_GET_INSTANCE(ast::Generic_Type)
AST_GET_INSTANCE(ast::Generic_Cast)
AST_GET_INSTANCE(ast::Generic_Op)
AST_GET_INSTANCE(ast::Generic_View)
AST_GET_INSTANCE(ast::Generic_Facet)
AST_GET_INSTANCE(ast::Generic_Extension)
AST_GET_INSTANCE(ast::Generic_Rule)

AST_GET_INSTANCE(ast::Literal_Boolean)
AST_GET_INSTANCE(ast::Literal_NullPtr)
AST_GET_INSTANCE(ast::Literal_Integral)
AST_GET_INSTANCE(ast::Literal_Fixed_Point)
AST_GET_INSTANCE(ast::Literal_Floating_Point)
AST_GET_INSTANCE(ast::Literal_Cune)
AST_GET_INSTANCE(ast::Literal_Rune)
AST_GET_INSTANCE(ast::Literal_Text_Pure)
AST_GET_INSTANCE(ast::Literal_Text_Interpolation)
AST_GET_INSTANCE(ast::Literal_Textual_Format)
AST_GET_INSTANCE(ast::Literal_Format_Specifier)
AST_GET_INSTANCE(ast::Literal_Table)
AST_GET_INSTANCE(ast::Literal_Tuple)
AST_GET_INSTANCE(ast::Literal_Range)
AST_GET_INSTANCE(ast::Literal_Record)

AST_GET_INSTANCE(ast::Expression_If_Ternary)
AST_GET_INSTANCE(ast::Expression_Member_Access)
AST_GET_INSTANCE(ast::Expression_Self)
AST_GET_INSTANCE(ast::Expression_Other)
AST_GET_INSTANCE(ast::Expression_Invocation)
AST_GET_INSTANCE(ast::Expression_Invocation_Arg)
AST_GET_INSTANCE(ast::Expression_Invocation_Rule)
AST_GET_INSTANCE(ast::Expression_Invocation_Extend)
AST_GET_INSTANCE(ast::Expression_Table_Access)
AST_GET_INSTANCE(ast::Expression_New_Ptr)

AST_GET_INSTANCE(ast::Statement_If)
AST_GET_INSTANCE(ast::Statement_For)
AST_GET_INSTANCE(ast::Statement_Loop)
AST_GET_INSTANCE(ast::Statement_While)
AST_GET_INSTANCE(ast::Statement_GoTo)
AST_GET_INSTANCE(ast::Statement_GoTo_Label)
AST_GET_INSTANCE(ast::Statement_Return)
AST_GET_INSTANCE(ast::Statement_Break)
AST_GET_INSTANCE(ast::Statement_Continue)
AST_GET_INSTANCE(ast::Statement_Match)
AST_GET_INSTANCE(ast::Statement_Match_Case)

AST_GET_INSTANCE(ast::Operation_Cast_As)
AST_GET_INSTANCE(ast::Operation_Is)
AST_GET_INSTANCE(ast::Operation_In)
AST_GET_INSTANCE(ast::Operation_Transfert)
AST_GET_INSTANCE(ast::Operation_Binary)
AST_GET_INSTANCE(ast::Operation_Unary)
AST_GET_INSTANCE(ast::Operation_Interval)
AST_GET_INSTANCE(ast::Operation_Mem)

#undef AST_GET_INSTANCE