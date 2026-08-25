#include "ids.hpp"

#include "compiler/compiler.hpp"

#include "nexus/ast/ast.hpp"
#include "nexus/ast/data.hpp"
#include "nexus/extension.hpp"
#include "nexus/inference.hpp"
#include "nexus/module.hpp"
#include "nexus/pipeline.hpp"
#include "nexus/lexer/token.hpp"
#include "nexus/resolved.hpp"
#include "nexus/scope.hpp"
#include "nexus/definition.hpp"
#include "nexus/ast/definition.hpp"
#include "nexus/ast/forward.hpp"
#include "nexus/type/definition.hpp"
#include "nexus/type/type.hpp"
#include <cstdint>


cu::CU& cu::ID::get() noexcept
{
  auto& scrs = compiler::pipeline.compilation_units;

  assert(*this && "Must be valid id");
  auto _offset = offset();

  assert(_offset < std::numeric_limits<uint32_t>::max() && "ID index will overflow on encoding");

  if (is_temp()) {
    auto& temps = compiler::pipeline.temp_compilation_units;
    assert(_offset < temps.size() && _offset >= 0 && "ID index is out of bound");
    return *temps.at(_offset);
  }

  assert(_offset < scrs.size() && _offset >= 0 && "ID index is out of bound");

  return *scrs.at(_offset);
}
const cu::CU& cu::ID::get() const noexcept
{
  auto& scrs = compiler::pipeline.compilation_units;

  assert(*this && "Must be valid id");
  auto _offset = offset();
  assert(_offset < std::numeric_limits<uint32_t>::max() && "ID index will overflow on encoding");

  if (is_temp()) {
    auto& temps = compiler::pipeline.temp_compilation_units;
    assert(_offset < temps.size() && _offset >= 0 && "ID index is out of bound");
    return *temps.at(_offset);
  }

  assert(_offset < scrs.size() && _offset >= 0 && "ID index is out of bound");
  return *scrs.at(_offset);
}

bool cu::ID::is_temp() const noexcept
{
  return id & FLAG_TEMP_CU;
}


ast::ID ast::ID::canonical() const noexcept
{
  assert(*this && "Must be valid id");

  const auto k = kind();

  if (ast::ENodeKind_is_symbol(k)) {
    if (auto defid = compiler::resolved.get_definition(*this)) {
      if (auto nodeid = defid.node()) return nodeid;
    }
  }

  return *this;
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
  return compiler::inference.get_inference(*this);
}
bool ast::ID::is_inferred() const noexcept
{
  assert(*this && "Must be valid id");
  return compiler::inference.is_inferred(*this);
}
definition::ID ast::ID::def() const noexcept
{
  assert(*this && "Must be valid id");

  if (auto defid = compiler::resolved.get_definition(*this)) return defid;

  return NO_ID;
}
bool ast::ID::is_resolved() const noexcept
{
  assert(*this && "Must be valid id");

  return compiler::resolved.is_resolved(*this);
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
std::string ast::ID::dump() const noexcept
{
  assert(*this && "Must be valid id");
  return ast::dump(*this);
}


bool ast::ID::is_rvalue() const noexcept
{
  assert(*this && "Must be valid id");

  const auto k = kind();

  assert(k != ENodeKind::Unknown && "invalid node facial kind");
  if (k == ENodeKind::Expression_Table_Access) return false;
  if (k >= ENodeKind::Literal_Boolean && k <= ENodeKind::Literal_Record) return true;
  if (k >= ENodeKind::Expression_If_Ternary && k <= ENodeKind::Expression_Get_Type) return true;
  if (k >= ENodeKind::Operation_Cast_As && k <= ENodeKind::Operation_Interval) return true;

  return false;
}
bool ast::ID::is_lvalue() const noexcept
{
  assert(*this && "Must be valid id");
  return !is_rvalue();
}
ast::NodeHeader& ast::ID::get() noexcept
{
  assert(*this && "Must be valid id");
  return cu().get().ast->get(*this);
}
const ast::NodeHeader& ast::ID::get() const noexcept
{
  assert(*this && "Must be valid id");
  return cu().get().ast->get(*this);
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
size_t token::ID::col() const noexcept
{
  assert(*this && "Must be valid id");
  return cu().get().file_info.get_column_from_pos(pos());
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

type::ID type::ID::canonical() const noexcept
{
  assert(*this && "Must be valid id");
  return get().tyid; // get is always canonical
}
type::ETypeKind type::ID::kind() const noexcept
{
  assert(*this && "Must be valid id");
  return get().kind;
}
definition::ID type::ID::def() const noexcept
{
  assert(*this && "Must be valid id");
  return compiler::inference.get_declaration(*this).def();
}
const std::unordered_set<ast::ID, ast::ID::Hash>& type::ID::extensions() const noexcept
{
  assert(*this && "Must be valid id");
  return cu().get().extensions->get_extensions(*this);
}
std::string type::ID::dump() const noexcept
{
  assert(*this && "Must be valid id");
  return type::dump(*this);
}
type::TypeHeader& type::ID::get() noexcept
{
  assert(*this && "Must be valid id");
  return cu().get().types->get(*this);
}
const type::TypeHeader& type::ID::get() const noexcept
{
  assert(*this && "Must be valid id");
  return cu().get().types->get(*this);
}
template <type::Generic T>
T* type::ID::as() noexcept
{
  assert(*this && "Must be valid id");
  return cu().get().types->as<T>(*this);
}
template <type::Generic T>
const T* type::ID::as() const noexcept
{
  assert(*this && "Must be valid id");
  return cu().get().types->as<T>(*this);
}

template <type::Generic T>
bool type::ID::is() const noexcept
{
  assert(*this && "Must be valid id");
  return cu().get().types->get(*this).kind == T::static_kind;
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


  assert(index() > 0 && index() < module::get_vendor().modid.index()
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

type::ID definition::ID::type() const noexcept
{
  assert(*this && "Must be valid id");
  return node().type();
}
ast::ID definition::ID::node() const noexcept
{
  assert(*this && "Must be valid id");
  return get().nodeid;
}
scope::ID definition::ID::scope() const noexcept
{
  assert(*this && "Must be valid id");
  return node().scope();
}
module::ID definition::ID::module() const noexcept
{
  assert(*this && "Must be valid id");
  return node().module();
}
definition::Definition& definition::ID::get() noexcept
{
  assert(*this && "Must be valid id");
  if (cu()) return cu().get().definitions->get(*this);

  return definition::get(*this);
}
const definition::Definition& definition::ID::get() const noexcept
{
  assert(*this && "Must be valid id");
  if (cu()) return cu().get().definitions->get(*this);

  return definition::get(*this);
}

/*
 * =============================================================================
 *  Template Instanciation Section
 * =============================================================================
 * To avoid any massive inclusion in headers and keep easy identifier usage
 * -----------------------------------------------------------------------------
 */


// #include "ast/ast_base.hpp"
// #include "ast/ast_declaration_extension.hpp"
// #include "ast/ast_declaration_sfm.hpp"
// #include "ast/ast_declaration_global.hpp"
// #include "ast/ast_declaration_local.hpp"
// #include "ast/ast_expression.hpp"
// #include "ast/ast_generic.hpp"
// #include "ast/ast_literal.hpp"
// #include "ast/ast_memory.hpp"
// #include "ast/ast_operation.hpp"
// #include "ast/ast_statement.hpp"
#include "ast/ast.hpp"


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
AST_GET_INSTANCE(ast::Literal_Table_Population)
AST_GET_INSTANCE(ast::Literal_Map)
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
AST_GET_INSTANCE(ast::Operation_Transfert)
AST_GET_INSTANCE(ast::Operation_Binary)
AST_GET_INSTANCE(ast::Operation_Unary)
AST_GET_INSTANCE(ast::Operation_Interval)

AST_GET_INSTANCE(ast::Memory_Del)
AST_GET_INSTANCE(ast::Memory_Align)
AST_GET_INSTANCE(ast::Memory_Drop)

#undef AST_GET_INSTANCE


#define TYPE_GET_INSTANCE(T)                                                                                           \
  template T*       type::ID::as<T>() noexcept;                                                                        \
  template const T* type::ID::as<T>() const noexcept;                                                                  \
  template bool     type::ID::is<T>() const noexcept;

TYPE_GET_INSTANCE(type::Primitive)
TYPE_GET_INSTANCE(type::String)
TYPE_GET_INSTANCE(type::Tuple)
TYPE_GET_INSTANCE(type::Array)
TYPE_GET_INSTANCE(type::Buffer)
TYPE_GET_INSTANCE(type::Slice)
TYPE_GET_INSTANCE(type::Ptr)
TYPE_GET_INSTANCE(type::Prototype)
TYPE_GET_INSTANCE(type::Facet)
TYPE_GET_INSTANCE(type::View)
TYPE_GET_INSTANCE(type::Form)
TYPE_GET_INSTANCE(type::Enum)
TYPE_GET_INSTANCE(type::Flag)
TYPE_GET_INSTANCE(type::Union)
TYPE_GET_INSTANCE(type::Identifier)

#undef TYPE_GET_INSTANCE
