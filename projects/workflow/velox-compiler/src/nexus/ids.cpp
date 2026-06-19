#include "ids.hpp"

#include "compiler/compiler.hpp"

#include "nexus/inference.hpp"
#include "nexus/module.hpp"
#include "nexus/pipeline.hpp"
#include "nexus/lexer/token.hpp"
#include "nexus/resolved.hpp"
#include "nexus/scope.hpp"
#include "nexus/symbol.hpp"
#include "nexus/ast/ast.hpp"
#include "nexus/type/type.hpp"
#include <cstdint>


cu::CU& cu::ID::get() noexcept
{
  auto& scrs = compiler::pipeline.compilation_units;

  assert(*this && "Must be valid id");
  auto rawid = raw();
  assert(rawid < std::numeric_limits<uint32_t>::max() && "ID index will overflow on encoding");
  assert(rawid < scrs.size() && rawid >= 0 && "ID index is out of bound");

  return *scrs.at(rawid);
}
const cu::CU& cu::ID::get() const noexcept
{
  auto& scrs = compiler::pipeline.compilation_units;

  assert(*this && "Must be valid id");
  auto rawid = raw();
  assert(rawid < std::numeric_limits<uint32_t>::max() && "ID index will overflow on encoding");
  assert(rawid < scrs.size() && rawid >= 0 && "ID index is out of bound");

  return *scrs.at(rawid);
}


token::ID ast::ID::token() const noexcept
{
  assert(*this && "Must be valid id");
  return get()->node_token_id;
}
type::ID ast::ID::type() const noexcept
{
  assert(*this && "Must be valid id");
  return compiler::inference.get_inference(*this);
}
bool ast::ID::is_inferred() const noexcept
{
  assert(*this && "Must be valid id");
  return compiler::inference.is_inferred(*this);
}
symbol::ID ast::ID::symbol() const noexcept
{
  assert(*this && "Must be valid id");

  return compiler::resolved.get_symbol(*this);
}
scope::ID ast::ID::scope() const noexcept
{
  assert(*this && "Must be valid id");
  return get()->scpid;
}
module::ID ast::ID::module() const noexcept
{
  assert(*this && "Must be valid id");
  return scope().module();
}
ast::Node* ast::ID::get() noexcept
{
  assert(*this && "Must be valid id");
  if (cu()) return &cu().get().nodes->get(*this);
  return &ast::get(*this);
}
const ast::Node* ast::ID::get() const noexcept
{
  assert(*this && "Must be valid id");
  if (cu()) return &cu().get().nodes->get(*this);
  return &ast::get(*this);
}
template <typename T>
[[nodiscard]] T* ast::ID::as() noexcept
{
  assert(*this && "Must be valid id");
  if (cu()) return cu().get().nodes->as<T>(*this);
  return ast::as<T>(*this);
}
template <typename T>
[[nodiscard]] const T* ast::ID::as() const noexcept
{
  assert(*this && "Must be valid id");
  if (cu()) return cu().get().nodes->as<T>(*this);
  return ast::as<T>(*this);
}


std::string_view token::ID::str() const noexcept
{
  assert(*this && "Must be valid id");
  const auto* arena = cu().get().file_info.tokens;
  const auto& data  = cu().get().file_info.data;

  const auto& tok = arena->get(*this);
  assert(data.data() && "File must have a textual representation");
  if (tok.begin == data.size()) {
    return {data.data() + tok.begin - 1, 1};
  }
  assert(tok.begin + tok.length <= data.size());
  return {data.data() + tok.begin, tok.length};
}
size_t token::ID::pos() const noexcept
{
  assert(*this && "Must be valid id");
  return get().begin;
}
size_t token::ID::line() const noexcept
{
  assert(*this && "Must be valid id");
  return cu().get().file_info.get_line_from_pos(pos());
}
std::string_view token::ID::line_str() const noexcept
{
  assert(*this && "Must be valid id");
  return cu().get().file_info.get_line(line());
}
token::Token& token::ID::get() noexcept
{
  assert(*this && "Must be valid id");
  return cu().get().file_info.tokens->get(*this);
}
const token::Token& token::ID::get() const noexcept
{
  assert(*this && "Must be valid id");
  return cu().get().file_info.tokens->get(*this);
}


symbol::ID type::ID::symbol() const noexcept
{
  assert(*this && "Must be valid id");
  return get().get_sym_id();
}
ast::ID type::ID::declaration() const noexcept
{
  assert(*this && "Must be valid id");
  return get().get_sym_id().node();
}
type::Type& type::ID::get() noexcept
{
  assert(*this && "Must be valid id");
  if (cu()) return cu().get().types->get(*this);
  return type::get(*this);
}
const type::Type& type::ID::get() const noexcept
{
  assert(*this && "Must be valid id");
  if (cu()) return cu().get().types->get(*this);
  return type::get(*this);
}
template <typename T>
T* type::ID::as() noexcept
{
  assert(*this && "Must be valid id");
  if (cu()) return cu().get().types->as<T>(*this);
  return type::as<T>(*this);
}
template <typename T>
const T* type::ID::as() const noexcept
{
  assert(*this && "Must be valid id");
  if (cu()) return cu().get().types->as<T>(*this);
  return type::as<T>(*this);
}


module::ID module::ID::parent() const noexcept
{
  assert(*this && "Must be valid id");

  assert(cu() && "Must be atteched to a valid script");

  const auto* graph = cu().get().modules;
  auto        it    = graph->parent.find(*this);
  if (it == graph->parent.end()) return module::ID::invalid();
  return it->second;
}
std::vector<module::ID>& module::ID::children() const noexcept
{
  static std::vector<module::ID> empty;

  assert(*this && "Must be valid id");

  if (cu()) {
    auto* graph = cu().get().modules;
    auto  it    = graph->children.find(*this);
    if (it == graph->children.end()) return empty;
    return it->second;
  }


  assert(offset() > 0 && offset() < module::get_vendor().modid.offset()
         && "Must be to a standard module without script attachment");

  auto& mod = module::get(*this);


  return empty;
}
ast::ID module::ID::node() const noexcept
{
  assert(*this && "Must be valid id");
  return get().nodeid;
}
scope::ID module::ID::scope() const noexcept
{
  assert(*this && "Must be valid id");
  return get().scpid;
}
module::Module& module::ID::get() noexcept
{
  assert(*this && "Must be valid id");
  if (cu()) return cu().get().modules->get(*this);
  return module::get(*this);
}
const module::Module& module::ID::get() const noexcept
{
  assert(*this && "Must be valid id");
  if (cu()) return cu().get().modules->get(*this);
  return module::get(*this);
}


scope::ID scope::ID::parent() const noexcept
{
  assert(*this && "Must be valid id");
  scope::Graph* graph;

  assert(cu() && "Must be attached to a valid script");

  if (cu()) {
    const auto* graph = cu().get().scopes;

    auto it = graph->parent.find(*this);
    if (it == graph->parent.end()) return scope::ID::invalid();
    return it->second;
  }

  return scope::ID::invalid();
}
std::vector<scope::ID>& scope::ID::children() const noexcept
{
  static std::vector<scope::ID> empty;

  assert(*this && "Must be valid id");

  assert(cu() && "Must be attached to a valid script");

  if (cu()) {
    auto* graph = cu().get().scopes;

    auto it = graph->children.find(*this);
    if (it == graph->children.end()) return empty;
    return it->second;
  }

  return empty;
}
ast::ID scope::ID::node() const noexcept
{
  assert(*this && "Must be valid id");
  return get().nodeid;
}
module::ID scope::ID::module() const noexcept
{
  assert(*this && "Must be valid id");
  return get().modid;
}
scope::Scope& scope::ID::get() noexcept
{
  assert(*this && "Must be valid id");
  if (cu()) return cu().get().scopes->get(*this);

  return scope::get(*this);
}
const scope::Scope& scope::ID::get() const noexcept
{
  assert(*this && "Must be valid id");
  if (cu()) return cu().get().scopes->get(*this);

  return scope::get(*this);
}

type::ID symbol::ID::type() const noexcept
{
  assert(*this && "Must be valid id");
  return node().type();
}
ast::ID symbol::ID::node() const noexcept
{
  assert(*this && "Must be valid id");
  return get().nodeid;
}
scope::ID symbol::ID::scope() const noexcept
{
  assert(*this && "Must be valid id");
  return node().scope();
}
module::ID symbol::ID::module() const noexcept
{
  assert(*this && "Must be valid id");
  return node().module();
}
symbol::Symbol& symbol::ID::get() noexcept
{
  assert(*this && "Must be valid id");
  if (cu()) return cu().get().symbols->get(*this);

  return symbol::get(*this);
}
const symbol::Symbol& symbol::ID::get() const noexcept
{
  assert(*this && "Must be valid id");
  if (cu()) return cu().get().symbols->get(*this);

  return symbol::get(*this);
}

/*
 * =============================================================================
 *  Template Instanciation Section
 * =============================================================================
 * To avoid any massive inclusion in headers and keep easy identifier usage
 * -----------------------------------------------------------------------------
 */


#include "ast/ast_base.hpp"
#include "ast/ast_declaration_extension.hpp"
#include "ast/ast_declaration_sfm.hpp"
#include "ast/ast_declaration_global.hpp"
#include "ast/ast_declaration_local.hpp"
#include "ast/ast_expression.hpp"
#include "ast/ast_generic.hpp"
#include "ast/ast_literal.hpp"
#include "ast/ast_memory.hpp"
#include "ast/ast_operation.hpp"
#include "ast/ast_statement.hpp"
#include "ast/ast.hpp"


#define AST_GET_INSTANCE(T)                                                                                            \
  template T*       ast::ID::as<T>() noexcept;                                                                         \
  template const T* ast::ID::as<T>() const noexcept;


AST_GET_INSTANCE(ast::Unknown)

AST_GET_INSTANCE(ast::Identifier)
AST_GET_INSTANCE(ast::ID_Qualified)
AST_GET_INSTANCE(ast::ID_Typed)

AST_GET_INSTANCE(ast::Path_Regex)
AST_GET_INSTANCE(ast::Root)

AST_GET_INSTANCE(ast::Import)
AST_GET_INSTANCE(ast::Global_Variable)
AST_GET_INSTANCE(ast::Global_Function)
AST_GET_INSTANCE(ast::Global_Extend_Fn)
AST_GET_INSTANCE(ast::Global_Extend_Cast)
AST_GET_INSTANCE(ast::Global_Extend_Op_Bin)
AST_GET_INSTANCE(ast::Global_Extend_Op_Un)
AST_GET_INSTANCE(ast::Global_Extend_Op_Access)
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
AST_GET_INSTANCE(ast::Literal_Table_Population)
AST_GET_INSTANCE(ast::Literal_Map)
AST_GET_INSTANCE(ast::Literal_Tuple)
AST_GET_INSTANCE(ast::Literal_Range)
AST_GET_INSTANCE(ast::Literal_Iterator)
AST_GET_INSTANCE(ast::Literal_Enum)
AST_GET_INSTANCE(ast::Literal_Structured_Data)
AST_GET_INSTANCE(ast::Literal_Form)

AST_GET_INSTANCE(ast::Expression_If_Ternary)
AST_GET_INSTANCE(ast::Expression_Member_Access)
AST_GET_INSTANCE(ast::Expression_Self)
AST_GET_INSTANCE(ast::Expression_Other)
AST_GET_INSTANCE(ast::Expression_Call)
AST_GET_INSTANCE(ast::Expression_Call_Argument)
AST_GET_INSTANCE(ast::Expression_Call_Rule)
AST_GET_INSTANCE(ast::Expression_Call_Pipe)
AST_GET_INSTANCE(ast::Expression_Table_Access)
AST_GET_INSTANCE(ast::Expression_Ptr_Val)
AST_GET_INSTANCE(ast::Expression_Mut_Of)
AST_GET_INSTANCE(ast::Expression_Ref_Of)
AST_GET_INSTANCE(ast::Expression_Move_Of)
AST_GET_INSTANCE(ast::Expression_Copy_Of)
AST_GET_INSTANCE(ast::Expression_Addr_Of)
AST_GET_INSTANCE(ast::Expression_Size_Of)
AST_GET_INSTANCE(ast::Expression_GetBits)
AST_GET_INSTANCE(ast::Expression_New_Ptr)
AST_GET_INSTANCE(ast::Expression_Get_Type)

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
AST_GET_INSTANCE(ast::Operation_Assignment)
AST_GET_INSTANCE(ast::Operation_Binary)
AST_GET_INSTANCE(ast::Operation_Unary)
AST_GET_INSTANCE(ast::Operation_Interval)

AST_GET_INSTANCE(ast::Memory_Del)
AST_GET_INSTANCE(ast::Memory_Align)
AST_GET_INSTANCE(ast::Memory_Drop)

#undef AST_GET_INSTANCE


#define TYPE_GET_INSTANCE(T)                                                                                           \
  template T*       type::ID::as<T>() noexcept;                                                                        \
  template const T* type::ID::as<T>() const noexcept;

TYPE_GET_INSTANCE(type::Primitive)
TYPE_GET_INSTANCE(type::String)
TYPE_GET_INSTANCE(type::Tuple)
TYPE_GET_INSTANCE(type::StaticArray)
TYPE_GET_INSTANCE(type::Ptr)
TYPE_GET_INSTANCE(type::DynamicArray)
TYPE_GET_INSTANCE(type::Prototype)
TYPE_GET_INSTANCE(type::Facet)
TYPE_GET_INSTANCE(type::View)
TYPE_GET_INSTANCE(type::Form)
TYPE_GET_INSTANCE(type::Enum)
TYPE_GET_INSTANCE(type::Flag)
TYPE_GET_INSTANCE(type::Union)
TYPE_GET_INSTANCE(type::Identifier)

#undef TYPE_GET_INSTANCE
