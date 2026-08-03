#include "resolver_dump.hpp"
#include "Neargye/magic_enum.hpp"
#include "compiler/compilation_unit.hpp"
#include "nexus/ast/data.hpp"
#include "nexus/ast/ast.hpp"
#include "nexus/type/definition.hpp"
#include <algorithm>
#include <string>


#define NOT_DEFINED return "";
#define PROHIBIED   assert(false && "dump on this node is prohibied");

std::string utils::Dump::dump_node(ast::ID nodeid) noexcept
{
#define case_n(kind)                                                                                                   \
  case ast::ENodeKind::kind: return dump_##kind(*nodeid.as<ast::kind>());

  switch (nodeid.kind()) {
    case_n(Symbol_Id);
    case_n(Symbol_Qualified);
    case_n(Symbol_Type);
    case_n(Path_Regex);
    case_n(Root);
    case_n(Import);
    case_n(Global_Variable);
    case_n(Global_Function);
    case_n(Global_Extend_Fn);
    case_n(Global_Extend_Cast);
    case_n(Global_Extend_Op_Bin);
    case_n(Global_Extend_Op_Un);
    case_n(Global_Extend_Op_Subscript);
    case_n(Global_Extend_Op_Transfert);
    case_n(Global_Extend_Op_Other);
    case_n(Global_Module);
    case_n(Global_Extern);
    case_n(Global_Export);
    case_n(Global_Reexport);
    case_n(Global_Enum);
    case_n(Global_Flag);
    case_n(Global_Union);
    case_n(Global_Alias_Type);
    case_n(Global_Alias_Module);
    case_n(Global_Generic);
    case_n(Enum_Field);
    case_n(Flag_Field);
    case_n(Union_Field);
    case_n(CodeBlock);
    case_n(Local_Lambda);
    case_n(Local_Lambda_Capture);
    case_n(Local_Parameter);
    case_n(Local_Gen_Param_Elem);
    case_n(Local_Gen_Params);
    case_n(Local_Pattern_Element);
    case_n(Local_Pattern_Enum);
    case_n(Local_Pattern_Tuple);
    case_n(Local_Pattern_Form);
    case_n(Local_Pattern_Rule_Facet);
    case_n(Local_Pattern_Facet);
    case_n(Local_Binding);
    case_n(Local_Tuple_Destructuring);
    case_n(Local_Variable);
    case_n(Local_Capability);
    case_n(SFM_Facet);
    case_n(SFM_Facet_Field);
    case_n(SFM_View);
    case_n(SFM_Form);
    case_n(SFM_Rule);
    case_n(SFM_Rule_Case);
    case_n(Generic_Type);
    case_n(Generic_Cast);
    case_n(Generic_Op);
    case_n(Generic_View);
    case_n(Generic_Facet);
    case_n(Generic_Extension);
    case_n(Generic_Rule);
    case_n(Literal_Boolean);
    case_n(Literal_NullPtr);
    case_n(Literal_Integral);
    case_n(Literal_Fixed_Point);
    case_n(Literal_Floating_Point);
    case_n(Literal_Cune);
    case_n(Literal_Rune);
    case_n(Literal_Text_Pure);
    case_n(Literal_Text_Interpolation);
    case_n(Literal_Textual_Format);
    case_n(Literal_Format_Specifier);
    case_n(Literal_Table);
    case_n(Literal_Table_Population);
    case_n(Literal_Map);
    case_n(Literal_Tuple);
    case_n(Literal_Range);
    case_n(Literal_Record);
    case_n(Expression_If_Ternary);
    case_n(Expression_Member_Access);
    case_n(Expression_Self);
    case_n(Expression_Other);
    case_n(Expression_Invocation);
    case_n(Expression_Invocation_Arg);
    case_n(Expression_Invocation_Extend);
    case_n(Expression_Invocation_Rule);
    case_n(Expression_Table_Access);
    case_n(Expression_Ptr_Val);
    case_n(Expression_Mut_Of);
    case_n(Expression_Ref_Of);
    case_n(Expression_Move_Of);
    case_n(Expression_Copy_Of);
    case_n(Expression_Addr_Of);
    case_n(Expression_Size_Of);
    case_n(Expression_GetBits);
    case_n(Expression_New_Ptr);
    case_n(Expression_Get_Type);
    case_n(Statement_If);
    case_n(Statement_For);
    case_n(Statement_Loop);
    case_n(Statement_While);
    case_n(Statement_GoTo);
    case_n(Statement_GoTo_Label);
    case_n(Statement_Return);
    case_n(Statement_Break);
    case_n(Statement_Continue);
    case_n(Statement_Match);
    case_n(Statement_Match_Case);
    case_n(Operation_Cast_As);
    case_n(Operation_Is);
    case_n(Operation_In);
    case_n(Operation_Transfert);
    case_n(Operation_Binary);
    case_n(Operation_Unary);
    case_n(Operation_Interval);
    case_n(Memory_Del);
    case_n(Memory_Align);
    case_n(Memory_Drop);
  default: assert(false);
  }

#undef case_n
}
std::string utils::Dump::dump_Symbol_Id(const ast::Symbol_Id& n) noexcept
{
  return n.name;
}
std::string utils::Dump::dump_Symbol_Qualified(const ast::Symbol_Qualified& n) noexcept
{
  std::string s;
  for (const auto& elem : n.path) {
    s += elem;
    s += "::";
  }

  return s + n.name;
}
std::string utils::Dump::dump_Symbol_Type(const ast::Symbol_Type& n) noexcept
{
  return n.name.dump();
}
std::string utils::Dump::dump_Path_Regex(const ast::Path_Regex& n) noexcept
{
  std::string s;
  s += cu::EFileSource_to_str(n.source);
  s += "::";

  for (const auto& elem : n.path) {
    s += elem;
    s += "::";
  }
  s = s.substr(0, s.size() - 2);

  if (!n.elements.empty()) {
    s += "{";
    for (const auto& elem : n.elements) {
      s += elem;
      s += ", ";
    }
    s += "}";
  }

  return s;
}
std::string utils::Dump::dump_Root(const ast::Root& n) noexcept
{
  PROHIBIED;
}
std::string utils::Dump::dump_Import(const ast::Import& n) noexcept
{
  return n.regex.dump() + " as " + n.alias;
}
std::string utils::Dump::dump_Global_Variable(const ast::Global_Variable& n) noexcept
{
  std::string s;
  s += ast::EVariableKind_to_str(n.kind);
  s += " ";
  s += n.name;
  if (n.type) s += ": " + n.type.dump();
  if (n.expression)
    s += std::string(ast::ETransfertType_to_str(n.assignment)) + " " + n.expression.dump();
  else if (n.is_uninit)
    s += " = uninit";
  return s;
}
std::string utils::Dump::dump_Global_Function(const ast::Global_Function& n) noexcept
{
  std::string s;
  s += "fn ";
  s += n.name;
  s += "(";
  for (auto param : n.parameters) {
    s += param.dump();
  }
  if (n.is_explicit_ret) {
    const auto* proto = n.prototype.as<type::Prototype>();
    assert(proto);
    s += ") -> " + proto->ret.dump();
  } else {
    s += ")";
  }

  if (n.codeblock) {
    s += "\n" + n.codeblock.dump();
  } else {
    s += ";";
  }

  return s;
}
std::string utils::Dump::dump_Global_Extend_Fn(const ast::Global_Extend_Fn& n) noexcept
{
  std::string s;
  s += "extend ";
  s += n.extended_type.dump();
  s += " fn ";
  s += n.name;
  s += "(";
  if (n.is_self_const)
    s += "ref self, ";
  else if (!n.is_static)
    s += "mut self, ";
  for (auto param : n.parameters) {
    s += param.dump();
  }
  if (n.is_explicit_ret) {
    const auto* proto = n.prototype.as<type::Prototype>();
    assert(proto);
    s += ") -> " + proto->ret.dump();
  } else {
    s += ")";
  }

  if (n.codeblock) {
    s += "\n" + n.codeblock.dump();
  } else {
    s += ";";
  }

  return s;
}
std::string utils::Dump::dump_Global_Extend_Cast(const ast::Global_Extend_Cast& n) noexcept
{
  std::string s;
  s += "extend ";
  s += n.extended_type.dump();
  s += " as ";
  s += n.as_type.dump();
  s += "\n" + n.codeblock.dump();
  return s;
}
std::string utils::Dump::dump_Global_Extend_Op_Bin(const ast::Global_Extend_Op_Bin& n) noexcept
{
  std::string s;
  s += "extend ";
  s += n.extended_type.dump();
  s += " op ";
  s += ast::EOp_Bin_to_str(n.bin_op);
  s += "\n" + n.codeblock.dump();
  return s;
}
std::string utils::Dump::dump_Global_Extend_Op_Un(const ast::Global_Extend_Op_Un& n) noexcept
{
  std::string s;
  s += "extend ";
  s += n.extended_type.dump();
  s += " op ";
  s += ast::EOp_Unary_to_str(n.unary_op);
  s += "\n" + n.codeblock.dump();
  return s;
}
std::string utils::Dump::dump_Global_Extend_Op_Subscript(const ast::Global_Extend_Op_Subscript& n) noexcept
{
  std::string s;
  s += "extend ";
  s += n.extended_type.dump();
  s += " op ";
  s += ast::EOp_Subscript_to_str(n.subscript_op);
  s += "\n" + n.codeblock.dump();
  return s;
}
std::string utils::Dump::dump_Global_Extend_Op_Transfert(const ast::Global_Extend_Op_Transfert& n) noexcept
{
  std::string s;
  s += "extend ";
  s += n.extended_type.dump();
  s += " op ";
  s += ast::ETransfertType_to_str(n.transfert_op);
  s += "\n" + n.codeblock.dump();
  return s;
}
std::string utils::Dump::dump_Global_Extend_Op_Other(const ast::Global_Extend_Op_Other& n) noexcept
{
  std::string s;
  s += "extend ";
  s += n.extended_type.dump();
  s += " op ";
  s += magic_enum::enum_name(n.other_op).substr(1);
  s += "\n" + n.codeblock.dump();
  return s;
}
std::string utils::Dump::dump_Global_Module(const ast::Global_Module& n) noexcept
{
  std::string s;
  s += "mod ";
  s += n.name;
  s += n.codeblock.dump();
  return s;
}
std::string utils::Dump::dump_Global_Extern(const ast::Global_Extern& n) noexcept
{
  std::string s;
  s += "extern ";
  s += "\"" + n.abi + "\"";
  s += n.codeblock.dump();
  return s;
}
std::string utils::Dump::dump_Global_Export(const ast::Global_Export& n) noexcept
{
  std::string s;
  s += "export ";
  s += n.codeblock.dump();
  return s;
}
std::string utils::Dump::dump_Global_Reexport(const ast::Global_Reexport& n) noexcept
{
  std::string s;
  s += "reexport ";
  s += n.regex.dump();
  s += " as ";
  s += n.alias;
  return s;
}
std::string utils::Dump::dump_Global_Enum(const ast::Global_Enum& n) noexcept
{
  std::string s;
  s += "enum ";
  s += n.name;
  s += "{\n";
  for (auto elem : n.variants) {
    s += "  " + elem.dump() + ",\n";
  }
  s += "}";
  return s;
}
std::string utils::Dump::dump_Global_Flag(const ast::Global_Flag& n) noexcept
{
  std::string s;
  s += "flag ";
  s += n.name;
  s += "{\n";
  for (auto elem : n.flags) {
    s += " " + elem.dump() + ",\n";
  }
  s += "}";
  return s;
}
std::string utils::Dump::dump_Global_Union(const ast::Global_Union& n) noexcept
{
  std::string s;
  s += "union ";
  s += n.name;
  s += "{\n";
  for (auto elem : n.variants) {
    s += " " + elem.dump() + ",\n";
  }
  s += "}";
  return s;
}
std::string utils::Dump::dump_Global_Alias_Type(const ast::Global_Alias_Type& n) noexcept
{
  std::string s;
  s += "type ";
  s += n.alias;
  s += " ";
  if (n.type)
    s += n.type.dump();
  else
    s += "opaque";
  return s;
}
std::string utils::Dump::dump_Global_Alias_Module(const ast::Global_Alias_Module& n) noexcept
{
  std::string s;
  s += "mod ";
  s += n.alias;
  s += " = ";
  s += n.regex.dump();
  return s;
}
std::string utils::Dump::dump_Global_Generic(const ast::Global_Generic& n) noexcept
{
  NOT_DEFINED;
}
std::string utils::Dump::dump_Enum_Field(const ast::Enum_Field& n) noexcept
{
  std::string s;
  s += n.name;
  if (n.type) {
    s += n.type.dump();
  }
  return s;
}
std::string utils::Dump::dump_Flag_Field(const ast::Flag_Field& n) noexcept
{
  std::string s;
  s += n.name;
  return s;
}
std::string utils::Dump::dump_Union_Field(const ast::Union_Field& n) noexcept
{
  std::string s;
  s += n.name;
  s += ": ";
  s += n.type.dump();
  return s;
}
std::string utils::Dump::dump_CodeBlock(const ast::CodeBlock& n) noexcept
{
  std::string s;
  s += "{\n";
  for (auto elem : n.elements) s += "  " + elem.dump() + "\n";
  s += "}";
  return s;
}
std::string utils::Dump::dump_Local_Lambda(const ast::Local_Lambda& n) noexcept
{
  NOT_DEFINED;
}
std::string utils::Dump::dump_Local_Lambda_Capture(const ast::Local_Lambda_Capture& n) noexcept
{
  NOT_DEFINED;
}
std::string utils::Dump::dump_Local_Parameter(const ast::Local_Parameter& n) noexcept
{
  std::string s;
  s += ast::EPassMode_to_str(n.passmode);
  s += " ";
  s += n.name;
  s += ":";
  s += n.type.dump();
  if (n.default_value) s += " = " + n.default_value.dump();
  return s;
}
std::string utils::Dump::dump_Local_Gen_Param_Elem(const ast::Local_Gen_Param_Elem& n) noexcept
{
  NOT_DEFINED;
}
std::string utils::Dump::dump_Local_Gen_Params(const ast::Local_Gen_Params& n) noexcept
{
  NOT_DEFINED;
}
std::string utils::Dump::dump_Local_Pattern_Element(const ast::Local_Pattern_Element& n) noexcept
{
  NOT_DEFINED;
}
std::string utils::Dump::dump_Local_Pattern_Enum(const ast::Local_Pattern_Enum& n) noexcept
{
  NOT_DEFINED;
}
std::string utils::Dump::dump_Local_Pattern_Tuple(const ast::Local_Pattern_Tuple& n) noexcept
{
  NOT_DEFINED;
}
std::string utils::Dump::dump_Local_Pattern_Form(const ast::Local_Pattern_Form& n) noexcept
{
  NOT_DEFINED;
}
std::string utils::Dump::dump_Local_Pattern_Rule_Facet(const ast::Local_Pattern_Rule_Facet& n) noexcept
{
  NOT_DEFINED;
}
std::string utils::Dump::dump_Local_Pattern_Facet(const ast::Local_Pattern_Facet& n) noexcept
{
  NOT_DEFINED;
}
std::string utils::Dump::dump_Local_Binding(const ast::Local_Binding& n) noexcept
{
  NOT_DEFINED;
}
std::string utils::Dump::dump_Local_Tuple_Destructuring(const ast::Local_Tuple_Destructuring& n) noexcept
{
  std::string s;
  s += ast::EVariableKind_to_str(n.kind);
  s += "(";
  for (auto elem : n.bindings) s += elem.dump() + ", ";
  s = s.substr(0, s.size() - 2);
  s += ") = ";
  s += n.expression.dump();
  return s;
}
std::string utils::Dump::dump_Local_Variable(const ast::Local_Variable& n) noexcept
{
  std::string s;
  s += ast::EVariableKind_to_str(n.kind);
  s += " ";
  s += n.name;
  if (n.type) s += ": " + n.type.dump();
  if (n.expression)
    s += std::string(ast::ETransfertType_to_str(n.assignment)) + " " + n.expression.dump();
  else if (n.is_uninit)
    s += " = uninit";
  return s;
}
std::string utils::Dump::dump_Local_Capability(const ast::Local_Capability& n) noexcept
{
  std::string s;
  s += ast::ECapability_to_str(n.kind);
  s += " ";
  s += n.name;
  if (n.type) s += ": " + n.type.dump();
  s += " = " + n.expression.dump();
  return s;
}
std::string utils::Dump::dump_SFM_Facet(const ast::SFM_Facet& n) noexcept
{
  std::string s;
  s += "facet ";
  s += n.name;
  s += " {\n";
  for (auto elem : n.fields) s += "  " + elem.dump() + ",\n";
  s += "}";
  return s;
}
std::string utils::Dump::dump_SFM_Facet_Field(const ast::SFM_Facet_Field& n) noexcept
{
  std::string s;
  if (n.capability != ast::ECapability::NONE) {
    s += ast::ECapability_to_str(n.capability);
    s += " ";
  }
  s += n.name;
  s += ": ";
  s += n.type.dump();
  s += " = ";
  s += n.default_value.dump();
  return s;
}
std::string utils::Dump::dump_SFM_View(const ast::SFM_View& n) noexcept
{
  std::string s;
  s += "view ";
  s += n.name;
  s += " {\n";
  for (auto elem : n.facets) s += "  " + elem.dump() + ",\n";
  s += "}";
  return s;
}
std::string utils::Dump::dump_SFM_Form(const ast::SFM_Form& n) noexcept
{
  std::string s;
  s += "form ";
  s += n.name;
  s += " {\n";
  for (auto elem : n.facets) s += "  " + elem.dump() + ",\n";
  s += "}";
  return s;
}
std::string utils::Dump::dump_SFM_Rule(const ast::SFM_Rule& n) noexcept
{
  std::string s;
  s += "rule ";
  s += n.name;
  s += "(";
  for (auto param : n.parameters) {
    s += param.dump();
  }
  if (n.is_explicit_ret) {
    const auto* proto = n.prototype.as<type::Prototype>();
    assert(proto);
    s += ") -> " + proto->ret.dump();
  } else {
    s += ")";
  }

  s += "{\n";
  for (auto elem : n.cases) s += "  " + elem.dump() + "\n";

  return s;
}
std::string utils::Dump::dump_SFM_Rule_Case(const ast::SFM_Rule_Case& n) noexcept
{
  std::string s;
  for (auto elem : n.bindings) {
    s += elem.dump() + " + ";
  }
  s = s.substr(0, s.size() - 2);
  s += "=> ";
  s += n.codeblock.dump();
  return s;
}
std::string utils::Dump::dump_Generic_Type(const ast::Generic_Type& n) noexcept
{
  NOT_DEFINED;
}
std::string utils::Dump::dump_Generic_Cast(const ast::Generic_Cast& n) noexcept
{
  NOT_DEFINED;
}
std::string utils::Dump::dump_Generic_Op(const ast::Generic_Op& n) noexcept
{
  NOT_DEFINED;
}
std::string utils::Dump::dump_Generic_View(const ast::Generic_View& n) noexcept
{
  NOT_DEFINED;
}
std::string utils::Dump::dump_Generic_Facet(const ast::Generic_Facet& n) noexcept
{
  NOT_DEFINED;
}
std::string utils::Dump::dump_Generic_Extension(const ast::Generic_Extension& n) noexcept
{
  NOT_DEFINED;
}
std::string utils::Dump::dump_Generic_Rule(const ast::Generic_Rule& n) noexcept
{
  NOT_DEFINED;
}
std::string utils::Dump::dump_Literal_Boolean(const ast::Literal_Boolean& n) noexcept
{
  return n.val ? "true" : "false";
}
std::string utils::Dump::dump_Literal_NullPtr(const ast::Literal_NullPtr& n) noexcept
{
  static std::string s = "nullptr";
  return s;
}
std::string utils::Dump::dump_Literal_Integral(const ast::Literal_Integral& n) noexcept
{
  return std::string(n.header.start_tokid.str());
}
std::string utils::Dump::dump_Literal_Fixed_Point(const ast::Literal_Fixed_Point& n) noexcept
{
  return std::string(n.header.start_tokid.str());
}
std::string utils::Dump::dump_Literal_Floating_Point(const ast::Literal_Floating_Point& n) noexcept
{
  return n.val.float128_to_string();
}
std::string utils::Dump::dump_Literal_Cune(const ast::Literal_Cune& n) noexcept
{
  return "\"" + std::to_string(n.val) + "\"cune";
}
std::string utils::Dump::dump_Literal_Rune(const ast::Literal_Rune& n) noexcept
{
  return "\"" + n.code_points + "\"rune";
}
std::string utils::Dump::dump_Literal_Text_Pure(const ast::Literal_Text_Pure& n) noexcept
{
  std::string s;
  s += "\"" + n.val + "\"";
  s += magic_enum::enum_name(n.text_type).substr(1);
  return s;
}
std::string utils::Dump::dump_Literal_Text_Interpolation(const ast::Literal_Text_Interpolation& n) noexcept
{
  NOT_DEFINED;
}
std::string utils::Dump::dump_Literal_Textual_Format(const ast::Literal_Textual_Format& n) noexcept
{
  NOT_DEFINED;
}
std::string utils::Dump::dump_Literal_Format_Specifier(const ast::Literal_Format_Specifier& n) noexcept
{
  NOT_DEFINED;
}
std::string utils::Dump::dump_Literal_Table(const ast::Literal_Table& n) noexcept
{
  std::string s;
  s += "{";
  for (auto elem : n.values) s += elem.dump() + ", ";
  s = s.substr(0, s.size() - 2);
  s = "}";
  return s;
}
std::string utils::Dump::dump_Literal_Table_Population(const ast::Literal_Table_Population& n) noexcept
{
  NOT_DEFINED;
}
std::string utils::Dump::dump_Literal_Map(const ast::Literal_Map& n) noexcept
{
  NOT_DEFINED;
}
std::string utils::Dump::dump_Literal_Tuple(const ast::Literal_Tuple& n) noexcept
{
  std::string s;
  s += "(";
  for (auto elem : n.fields) s += elem.value.dump() + ", ";
  s = s.substr(0, s.size() - 2);
  s += ")";
  return s;
}
std::string utils::Dump::dump_Literal_Range(const ast::Literal_Range& n) noexcept
{
  std::string s;
  s += n.start.dump();
  s += n.endInclude ? "..=" : "..";
  s += n.end.dump();
  return s;
}
std::string utils::Dump::dump_Literal_Record(const ast::Literal_Record& n) noexcept
{
  std::string s;
  s += n.name.dump();
  s += "{\n";
  for (size_t i = 0; i < n.fields_names.size(); i++) {
    const auto& name = n.fields_names[i];
    const auto  val  = n.fields_args[i];

    s += "." + name + "= " + val.dump() + ",\n";
  }
  s = s.substr(0, s.size() - 2);
  s += "}";
  return s;
}
std::string utils::Dump::dump_Expression_If_Ternary(const ast::Expression_If_Ternary& n) noexcept
{
  NOT_DEFINED;
}
std::string utils::Dump::dump_Expression_Member_Access(const ast::Expression_Member_Access& n) noexcept
{
  std::string s;
  s += n.left_expression.dump();
  s += ".";
  s += n.right_identifier.dump();
  return s;
}
std::string utils::Dump::dump_Expression_Self(const ast::Expression_Self& n) noexcept
{
  static const std::string s = "self";
  return s;
}
std::string utils::Dump::dump_Expression_Other(const ast::Expression_Other& n) noexcept
{
  static const std::string s = "other";
  return s;
}
std::string utils::Dump::dump_Expression_Invocation(const ast::Expression_Invocation& n) noexcept
{
  std::string s;
  s += n.callee.dump();
  s += "(";
  for (auto elem : n.arguments) s += elem.dump() + ", ";
  s = s.substr(0, s.size() - 2);
  s += ")";
  return s;
}
std::string utils::Dump::dump_Expression_Invocation_Arg(const ast::Expression_Invocation_Arg& n) noexcept
{
  std::string s;
  if (!n.explicit_name.empty()) s += n.explicit_name + "= ";
  s += n.expression.dump();
  return s;
}
std::string utils::Dump::dump_Expression_Invocation_Extend(const ast::Expression_Invocation_Extend& n) noexcept
{
  NOT_DEFINED;
}
std::string utils::Dump::dump_Expression_Invocation_Rule(const ast::Expression_Invocation_Rule& n) noexcept
{
  NOT_DEFINED;
}
std::string utils::Dump::dump_Expression_Table_Access(const ast::Expression_Table_Access& n) noexcept
{
  std::string s;
  s += n.target.dump();
  s += n.bounded ? "?" : "";
  s += "[" + n.selector.dump() + "]";
  return s;
}
std::string utils::Dump::dump_Expression_Ptr_Val(const ast::Expression_Ptr_Val& n) noexcept
{
  std::string s;
  s += "val'";
  s += n.target.dump();
  return s;
}
std::string utils::Dump::dump_Expression_Mut_Of(const ast::Expression_Mut_Of& n) noexcept
{
  std::string s;
  s += "mut'";
  s += n.target.dump();
  return s;
}
std::string utils::Dump::dump_Expression_Ref_Of(const ast::Expression_Ref_Of& n) noexcept
{
  std::string s;
  s += "ref'";
  s += n.target.dump();
  return s;
}
std::string utils::Dump::dump_Expression_Move_Of(const ast::Expression_Move_Of& n) noexcept
{
  std::string s;
  s += "move'";
  s += n.target.dump();
  return s;
}
std::string utils::Dump::dump_Expression_Copy_Of(const ast::Expression_Copy_Of& n) noexcept
{
  std::string s;
  s += "copy'";
  s += n.target.dump();
  return s;
}
std::string utils::Dump::dump_Expression_Addr_Of(const ast::Expression_Addr_Of& n) noexcept
{
  std::string s;
  s += "addr'";
  s += n.target.dump();
  return s;
}
std::string utils::Dump::dump_Expression_Size_Of(const ast::Expression_Size_Of& n) noexcept
{
  std::string s;
  s += "size'";
  s += n.target.dump();
  return s;
}
std::string utils::Dump::dump_Expression_GetBits(const ast::Expression_GetBits& n) noexcept
{
  std::string s;
  s += n.target.dump();
  s += "~[" + n.range.dump() + "]";
  return s;
}
std::string utils::Dump::dump_Expression_New_Ptr(const ast::Expression_New_Ptr& n) noexcept
{
  std::string s;
  s += "new ptr'";
  s += n.type.dump();
  s += "(" + n.expression.dump() + ")";
  return s;
}
std::string utils::Dump::dump_Expression_Get_Type(const ast::Expression_Get_Type& n) noexcept
{
  std::string s;
  s += "meta::typeof(" + n.target.dump() + ")";
  return s;
}
std::string utils::Dump::dump_Statement_If(const ast::Statement_If& n) noexcept
{
  std::string s;
  s += n.is_else ? "else" : n.is_elif ? "elif" : "if";
  s += n.is_else ? "" : n.evaluator.dump();
  s += " ";
  s += n.codeblock.dump();
  return s;
}
std::string utils::Dump::dump_Statement_For(const ast::Statement_For& n) noexcept
{
  std::string s;
  s += "for ";
  if (n.index) s += n.index.dump();
  if (n.index && !n.items.empty()) s += ", ";
  for (auto elem : n.items) s += elem.dump();
  s += " ";
  s += n.codeblock.dump();
  return s;
}
std::string utils::Dump::dump_Statement_Loop(const ast::Statement_Loop& n) noexcept
{
  std::string s;
  s += "loop ";
  s += n.codeblock.dump();
  return s;
}
std::string utils::Dump::dump_Statement_While(const ast::Statement_While& n) noexcept
{
  std::string s;
  s += "while ";
  s += n.evaluator.dump();
  s += " ";
  s += n.codeblock.dump();
  return s;
}
std::string utils::Dump::dump_Statement_GoTo(const ast::Statement_GoTo& n) noexcept
{
  std::string s;
  s += "goto ";
  s += n.label;
  s += " ";
  return s;
}
std::string utils::Dump::dump_Statement_GoTo_Label(const ast::Statement_GoTo_Label& n) noexcept
{
  std::string s;
  s += "label ";
  s += n.label;
  s += " ";
  return s;
}
std::string utils::Dump::dump_Statement_Return(const ast::Statement_Return& n) noexcept
{
  std::string s;
  s += "return ";
  s += n.value.dump();
  return s;
}
std::string utils::Dump::dump_Statement_Break(const ast::Statement_Break& n) noexcept
{
  static const std::string s = "break";
  return s;
}
std::string utils::Dump::dump_Statement_Continue(const ast::Statement_Continue& n) noexcept
{
  static const std::string s = "continue";
  return s;
}
std::string utils::Dump::dump_Statement_Match(const ast::Statement_Match& n) noexcept
{
  std::string s;
  s += "match ";
  s += n.base.dump();
  s += " {\n";
  for (auto elem : n.cases) s += elem.dump();
  if (n.other_case) s += n.other_case.dump();
  s += "\n}";
  return s;
}
std::string utils::Dump::dump_Statement_Match_Case(const ast::Statement_Match_Case& n) noexcept
{
  std::string s;
  s += n.evaluator.dump();
  s += " => ";
  s += n.codeblock.dump();
}
std::string utils::Dump::dump_Operation_Cast_As(const ast::Operation_Cast_As& n) noexcept
{
}
std::string utils::Dump::dump_Operation_Is(const ast::Operation_Is& n) noexcept
{
}
std::string utils::Dump::dump_Operation_In(const ast::Operation_In& n) noexcept
{
}
std::string utils::Dump::dump_Operation_Transfert(const ast::Operation_Transfert& n) noexcept
{
}
std::string utils::Dump::dump_Operation_Binary(const ast::Operation_Binary& n) noexcept
{
}
std::string utils::Dump::dump_Operation_Unary(const ast::Operation_Unary& n) noexcept
{
}
std::string utils::Dump::dump_Operation_Interval(const ast::Operation_Interval& n) noexcept
{
}
std::string utils::Dump::dump_Memory_Del(const ast::Memory_Del& n) noexcept
{
}
std::string utils::Dump::dump_Memory_Align(const ast::Memory_Align& n) noexcept
{
}
std::string utils::Dump::dump_Memory_Drop(const ast::Memory_Drop& n) noexcept
{
}

#undef NOT_DEFINED
#undef PROHIBIED