#include "resolver_dump.hpp"
#include "Neargye/magic_enum.hpp"
#include "compiler/compilation_unit.hpp"
#include "nexus/ast/data.hpp"

#include "nexus/ast/ast.hpp" // is mandatory, do not remove

#include "nexus/type/definition.hpp"
#include <string>


#define NOT_DEFINED return {};
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
  s.reserve(n.path.size() * 12);
  for (const auto& elem : n.path) std::format_to(std::back_inserter(s), "{}::", elem);
  return s + n.name;
}
std::string utils::Dump::dump_Symbol_Type(const ast::Symbol_Type& n) noexcept
{
  return n.name.dump();
}
std::string utils::Dump::dump_Path_Regex(const ast::Path_Regex& n) noexcept
{
  std::string s = std::format("{}::", cu::EFileSource_to_str(n.source));

  for (const auto& elem : n.path) std::format_to(std::back_inserter(s), "{}::", elem);
  s = s.substr(0, s.size() - 2);

  if (!n.elements.empty()) {
    s += "{";
    for (const auto& elem : n.elements) std::format_to(std::back_inserter(s), "{}, ", elem);
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
  return std::format("{} as {}", n.regex.dump(), n.alias);
}
std::string utils::Dump::dump_Global_Variable(const ast::Global_Variable& n) noexcept
{
  if (n.expression)
    return std::format("{} {}: {} {} {}", ast::EVariableKind_to_str(n.kind), n.name, n.type.dump(),
                       ast::ETransfertType_to_str(n.assignment), n.expression.dump());
  if (n.is_uninit) return std::format("{} {}: {} = uninit", ast::EVariableKind_to_str(n.kind), n.name, n.type.dump());

  assert(false);
}
std::string utils::Dump::dump_Global_Function(const ast::Global_Function& n) noexcept
{
  std::string params;
  params.reserve(n.parameters.size() * 32);
  for (auto param : n.parameters) params += param.dump();

  return std::format("fn {}({}){}\n{}", n.name, params,
                     n.is_explicit_ret ? " -> " + n.prototype.as<type::Prototype>()->ret.dump() : "",
                     n.codeblock.dump());
}
std::string utils::Dump::dump_Global_Extend_Fn(const ast::Global_Extend_Fn& n) noexcept
{
  std::string params;
  params.reserve(n.parameters.size() * 32);
  for (auto param : n.parameters) params += param.dump();

  if (n.is_self_const)
    return std::format("extend {} fn {}(ref self, {}){}\n{}", n.extended_type.dump(), n.name, params,
                       n.is_explicit_ret ? " -> " + n.prototype.as<type::Prototype>()->ret.dump() : "",
                       n.codeblock.dump());
  if (!n.is_static)
    return std::format("extend {} fn {}(mut self, {}){}\n{}", n.extended_type.dump(), n.name, params,
                       n.is_explicit_ret ? " -> " + n.prototype.as<type::Prototype>()->ret.dump() : "",
                       n.codeblock.dump());

  return std::format("extend {} fn {}({}){}\n{}", n.extended_type.dump(), n.name, params,
                     n.is_explicit_ret ? " -> " + n.prototype.as<type::Prototype>()->ret.dump() : "",
                     n.codeblock.dump());
}
std::string utils::Dump::dump_Global_Extend_Cast(const ast::Global_Extend_Cast& n) noexcept
{
  return std::format("extend {} as {}\n{}", n.extended_type.dump(), n.as_type.dump(), n.codeblock.dump());
}
std::string utils::Dump::dump_Global_Extend_Op_Bin(const ast::Global_Extend_Op_Bin& n) noexcept
{
  return std::format("extend {} op {}\n{}", n.extended_type.dump(), ast::EOp_Bin_to_str(n.bin_op), n.codeblock.dump());
}
std::string utils::Dump::dump_Global_Extend_Op_Un(const ast::Global_Extend_Op_Un& n) noexcept
{
  return std::format("extend {} op {}\n{}", n.extended_type.dump(), ast::EOp_Unary_to_str(n.unary_op),
                     n.codeblock.dump());
}
std::string utils::Dump::dump_Global_Extend_Op_Subscript(const ast::Global_Extend_Op_Subscript& n) noexcept
{
  return std::format("extend {} op {}\n{}", n.extended_type.dump(), ast::EOp_Subscript_to_str(n.subscript_op),
                     n.codeblock.dump());
}
std::string utils::Dump::dump_Global_Extend_Op_Transfert(const ast::Global_Extend_Op_Transfert& n) noexcept
{
  return std::format("extend {} op {}\n{}", n.extended_type.dump(), ast::ETransfertType_to_str(n.transfert_op),
                     n.codeblock.dump());
}
std::string utils::Dump::dump_Global_Extend_Op_Other(const ast::Global_Extend_Op_Other& n) noexcept
{
  return std::format("extend {} op {}\n{}", n.extended_type.dump(), magic_enum::enum_name(n.other_op).substr(1),
                     n.codeblock.dump());
}
std::string utils::Dump::dump_Global_Module(const ast::Global_Module& n) noexcept
{
  return std::format("mod {} {}", n.name, n.codeblock.dump());
}
std::string utils::Dump::dump_Global_Extern(const ast::Global_Extern& n) noexcept
{
  return std::format("extend \"{}\"\n{}", n.abi, n.codeblock.dump());
}
std::string utils::Dump::dump_Global_Export(const ast::Global_Export& n) noexcept
{
  return std::format("export {}", n.codeblock.dump());
}
std::string utils::Dump::dump_Global_Reexport(const ast::Global_Reexport& n) noexcept
{
  return std::format("reexport {} as {}", n.regex.dump(), n.alias);
}
std::string utils::Dump::dump_Global_Enum(const ast::Global_Enum& n) noexcept
{
  std::string variants;
  variants.reserve(n.variants.size() * 24);
  for (auto elem : n.variants) std::format_to(std::back_inserter(variants), "  {},\n", elem.dump());
  return std::format("enum {} {{\n{}\n}}", n.name, variants);
}
std::string utils::Dump::dump_Global_Flag(const ast::Global_Flag& n) noexcept
{
  std::string flags;
  flags.reserve(n.flags.size() * 12);
  for (auto elem : n.flags) std::format_to(std::back_inserter(flags), "  {},\n", elem.dump());
  return std::format("flag {} {{\n{}\n}}", n.name, flags);
}
std::string utils::Dump::dump_Global_Union(const ast::Global_Union& n) noexcept
{
  std::string variants;
  variants.reserve(n.variants.size() * 12);
  for (auto elem : n.variants) std::format_to(std::back_inserter(variants), "  {},\n", elem.dump());
  return std::format("union {} {{\n{}\n}}", n.name, variants);
}
std::string utils::Dump::dump_Global_Alias_Type(const ast::Global_Alias_Type& n) noexcept
{
  return std::format("type {} = {}", n.alias, n.type ? n.type.dump() : "opaque");
}
std::string utils::Dump::dump_Global_Alias_Module(const ast::Global_Alias_Module& n) noexcept
{
  return std::format("mod {} = {}", n.alias, n.regex.dump());
}
std::string utils::Dump::dump_Global_Generic(const ast::Global_Generic& n) noexcept
{
  NOT_DEFINED;
}
std::string utils::Dump::dump_Enum_Field(const ast::Enum_Field& n) noexcept
{
  return std::format("{}({})", n.name, n.type ? n.type.dump() : "");
}
std::string utils::Dump::dump_Flag_Field(const ast::Flag_Field& n) noexcept
{
  return n.name;
}
std::string utils::Dump::dump_Union_Field(const ast::Union_Field& n) noexcept
{
  return std::format("{}: {}", n.name, n.type.dump());
}
std::string utils::Dump::dump_CodeBlock(const ast::CodeBlock& n) noexcept
{
  std::string elements;
  elements.reserve(n.elements.size() * 32);
  for (auto elem : n.elements) std::format_to(std::back_inserter(elements), "  {}\n", elem.dump());
  return std::format("{{\n{}}}", elements);
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
  return std::format("{} {}: {}{}", ast::EPassMode_to_str(n.passmode), n.name, n.type.dump(),
                     n.default_value ? " = " + n.default_value.dump() : "");
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
  std::string bindings;
  bindings.reserve(n.bindings.size() * 12);
  for (auto elem : n.bindings) std::format_to(std::back_inserter(bindings), "{}, ", elem.dump());
  bindings = bindings.substr(0, bindings.size() - 2);
  return std::format("{}({}) = {}", ast::EVariableKind_to_str(n.kind), bindings, n.expression.dump());
}
std::string utils::Dump::dump_Local_Variable(const ast::Local_Variable& n) noexcept
{
  if (n.expression)
    return std::format("{} {}{} {} {}", ast::EVariableKind_to_str(n.kind), n.name, n.type ? ": " + n.type.dump() : "",
                       ast::ETransfertType_to_str(n.assignment), n.expression.dump());
  if (n.is_uninit)
    return std::format("{} {}{} = uninit", ast::EVariableKind_to_str(n.kind), n.name,
                       n.type ? ": " + n.type.dump() : "");

  assert(false);
}
std::string utils::Dump::dump_Local_Capability(const ast::Local_Capability& n) noexcept
{
  return std::format("{} {}{} = {}", ast::ECapability_to_str(n.kind), n.name, n.type ? ": " + n.type.dump() : "",
                     n.expression.dump());
}
std::string utils::Dump::dump_SFM_Facet(const ast::SFM_Facet& n) noexcept
{
  std::string facets;
  facets.reserve(n.fields.size() * 12);
  for (auto elem : n.fields) std::format_to(std::back_inserter(facets), "  {},\n", elem.dump());
  return std::format("facet {} {{\n{}\n}}", n.name, facets);
}
std::string utils::Dump::dump_SFM_Facet_Field(const ast::SFM_Facet_Field& n) noexcept
{
  return std::format("{}{}: {} = {}",
                     n.capability != ast::ECapability::NONE ? std::string(ast::ECapability_to_str(n.capability)) + " "
                                                            : "",
                     n.name, n.type.dump(), n.default_value.dump());
}
std::string utils::Dump::dump_SFM_View(const ast::SFM_View& n) noexcept
{
  std::string facets;
  facets.reserve(n.facets.size() * 12);
  for (auto elem : n.facets) std::format_to(std::back_inserter(facets), "  {},\n", elem.dump());
  return std::format("view {} {{\n{}\n}}", n.name, facets);
}
std::string utils::Dump::dump_SFM_Form(const ast::SFM_Form& n) noexcept
{
  std::string facets;
  facets.reserve(n.facets.size() * 12);
  for (auto elem : n.facets) std::format_to(std::back_inserter(facets), "  {},\n", elem.dump());
  return std::format("form {} {{\n{}\n}}", n.name, facets);
}
std::string utils::Dump::dump_SFM_Rule(const ast::SFM_Rule& n) noexcept
{
  std::string parameters;
  parameters.reserve(n.parameters.size() * 24);
  for (auto param : n.parameters) parameters += param.dump();
  std::string cases;
  cases.reserve(n.cases.size() * 64);
  for (auto elem : n.cases) std::format_to(std::back_inserter(cases), "  {}\n", elem.dump());
  return std::format("rule {}({}){} {{\n{}\n}}", n.name, parameters,
                     n.is_explicit_ret ? " -> " + n.prototype.as<type::Prototype>()->ret.dump() : "", cases);
}
std::string utils::Dump::dump_SFM_Rule_Case(const ast::SFM_Rule_Case& n) noexcept
{
  std::string bindings;
  bindings.reserve(n.bindings.size() * 16);
  for (auto elem : n.bindings) bindings += std::format("{} + ", elem.dump());
  bindings = bindings.substr(0, bindings.size() - 3);
  return std::format("{} => {}", bindings, n.codeblock.dump());
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
  return "nullptr";
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
  return std::format(R"("{}"cune)", n.val);
}
std::string utils::Dump::dump_Literal_Rune(const ast::Literal_Rune& n) noexcept
{
  return std::format(R"("{}"rune)", n.code_points);
}
std::string utils::Dump::dump_Literal_Text_Pure(const ast::Literal_Text_Pure& n) noexcept
{
  return std::format(R"("{}"{})", n.val, magic_enum::enum_name(n.text_type).substr(1));
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
  std::string values;
  values.reserve(n.values.size() * 12);
  for (auto elem : n.values) std::format_to(std::back_inserter(values), "{}, ", elem.dump());
  values = values.substr(0, values.size() - 2);
  return std::format("{{{}}}", values);
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
  std::string fields;
  fields.reserve(n.fields.size() * 12);
  for (auto elem : n.fields) std::format_to(std::back_inserter(fields), "{}, ", elem.value.dump());
  fields = fields.substr(0, fields.size() - 2);
  return std::format("({})", fields);
}
std::string utils::Dump::dump_Literal_Range(const ast::Literal_Range& n) noexcept
{
  return std::format("{}{}{}", n.start.dump(), n.endInclude ? "..=" : "..", n.end.dump());
}
std::string utils::Dump::dump_Literal_Record(const ast::Literal_Record& n) noexcept
{
  std::string fields;
  fields.reserve(n.fields_names.size() * 32);
  for (size_t i = 0; i < n.fields_names.size(); i++)
    std::format_to(std::back_inserter(fields), ".{}= {},\n", n.fields_names[i], n.fields_args[i].dump());
  fields = fields.substr(0, fields.size() - 2);
  return std::format("{}{{\n{}\n}}", n.name.dump(), fields);
}
std::string utils::Dump::dump_Expression_If_Ternary(const ast::Expression_If_Ternary& n) noexcept
{
  NOT_DEFINED;
}
std::string utils::Dump::dump_Expression_Member_Access(const ast::Expression_Member_Access& n) noexcept
{
  return std::format("{}.{}", n.left_expression.dump(), n.right_identifier.dump());
}
std::string utils::Dump::dump_Expression_Self(const ast::Expression_Self& n) noexcept
{
  return "self";
}
std::string utils::Dump::dump_Expression_Other(const ast::Expression_Other& n) noexcept
{
  return "other";
}
std::string utils::Dump::dump_Expression_Invocation(const ast::Expression_Invocation& n) noexcept
{
  std::string arguments;
  arguments.reserve(n.arguments.size() * 24);
  for (auto elem : n.arguments) std::format_to(std::back_inserter(arguments), "{}, ", elem.dump());
  arguments = arguments.substr(0, arguments.size() - 2);
  return std::format("{}({})", n.callee.dump(), arguments);
}
std::string utils::Dump::dump_Expression_Invocation_Arg(const ast::Expression_Invocation_Arg& n) noexcept
{
  return std::format("{}{}", n.explicit_name.empty() ? "" : n.explicit_name + "= ", n.expression.dump());
}
std::string utils::Dump::dump_Expression_Invocation_Extend(const ast::Expression_Invocation_Extend& n) noexcept
{
  std::string arguments;
  arguments.resize(n.arguments.size() * 32);
  for (auto elem : n.arguments) std::format_to(std::back_inserter(arguments), "{}, ", elem.dump());
  return std::format("{}.{}({})", n.target_form.dump(), n.callee.dump(), arguments);
}
std::string utils::Dump::dump_Expression_Invocation_Rule(const ast::Expression_Invocation_Rule& n) noexcept
{
  std::string arguments;
  arguments.resize(n.arguments.size() * 32);
  for (auto elem : n.arguments) std::format_to(std::back_inserter(arguments), "{}, ", elem.dump());
  return std::format("{}->{}({})", n.target_form.dump(), n.callee.dump(), arguments);
}
std::string utils::Dump::dump_Expression_Table_Access(const ast::Expression_Table_Access& n) noexcept
{
  return std::format("{}{}[{}]", n.target.dump(), n.bounded ? "?" : "", n.selector.dump());
}
std::string utils::Dump::dump_Expression_Ptr_Val(const ast::Expression_Ptr_Val& n) noexcept
{
  return std::format("val'{}", n.target.dump());
}
std::string utils::Dump::dump_Expression_Mut_Of(const ast::Expression_Mut_Of& n) noexcept
{
  return std::format("mut'{}", n.target.dump());
}
std::string utils::Dump::dump_Expression_Ref_Of(const ast::Expression_Ref_Of& n) noexcept
{
  return std::format("ref'{}", n.target.dump());
}
std::string utils::Dump::dump_Expression_Move_Of(const ast::Expression_Move_Of& n) noexcept
{
  return std::format("move'{}", n.target.dump());
}
std::string utils::Dump::dump_Expression_Copy_Of(const ast::Expression_Copy_Of& n) noexcept
{
  return std::format("copy'{}", n.target.dump());
}
std::string utils::Dump::dump_Expression_Addr_Of(const ast::Expression_Addr_Of& n) noexcept
{
  return std::format("addr'{}", n.target.dump());
}
std::string utils::Dump::dump_Expression_Size_Of(const ast::Expression_Size_Of& n) noexcept
{
  return std::format("size'{}", n.target.dump());
}
std::string utils::Dump::dump_Expression_GetBits(const ast::Expression_GetBits& n) noexcept
{
  return std::format("{}~[{}]", n.target.dump(), n.range.dump());
}
std::string utils::Dump::dump_Expression_New_Ptr(const ast::Expression_New_Ptr& n) noexcept
{
  return std::format("new ptr'{}({})", n.type.dump(), n.expression.dump());
}
std::string utils::Dump::dump_Expression_Get_Type(const ast::Expression_Get_Type& n) noexcept
{
  return std::format("meta::typeof({})", n.target.dump());
}
std::string utils::Dump::dump_Statement_If(const ast::Statement_If& n) noexcept
{
  std::string if_naming = n.is_else ? "else" : n.is_elif ? "elif" : "if";
  return std::format("{}{} {}", if_naming, n.is_else ? "" : n.evaluator.dump(), n.codeblock.dump());
}
std::string utils::Dump::dump_Statement_For(const ast::Statement_For& n) noexcept
{
  std::string items;
  items.reserve(n.items.size() * 13);
  if (n.index) items += n.index.dump();
  if (n.index && !n.items.empty()) items += ", ";
  for (auto elem : n.items) items += elem.dump() + ", ";
  return std::format("for {} in {} {}", items, n.expression.dump(), n.codeblock.dump());
}
std::string utils::Dump::dump_Statement_Loop(const ast::Statement_Loop& n) noexcept
{
  return std::format("loop {}", n.codeblock.dump());
}
std::string utils::Dump::dump_Statement_While(const ast::Statement_While& n) noexcept
{
  return std::format("while {} {}", n.evaluator.dump(), n.codeblock.dump());
}
std::string utils::Dump::dump_Statement_GoTo(const ast::Statement_GoTo& n) noexcept
{
  return std::format("goto {}", n.label);
}
std::string utils::Dump::dump_Statement_GoTo_Label(const ast::Statement_GoTo_Label& n) noexcept
{
  return std::format("label {}", n.label);
}
std::string utils::Dump::dump_Statement_Return(const ast::Statement_Return& n) noexcept
{
  return std::format("return {}", n.value.dump());
}
std::string utils::Dump::dump_Statement_Break(const ast::Statement_Break& n) noexcept
{
  return "break";
}
std::string utils::Dump::dump_Statement_Continue(const ast::Statement_Continue& n) noexcept
{
  return "continue";
}
std::string utils::Dump::dump_Statement_Match(const ast::Statement_Match& n) noexcept
{
  std::string cases;
  cases.reserve(n.cases.size() * 33);
  for (auto elem : n.cases) cases += elem.dump();
  if (n.other_case) cases += n.other_case.dump();
  return std::format("match {} {{\n{}\n}}", n.base.dump(), cases);
}
std::string utils::Dump::dump_Statement_Match_Case(const ast::Statement_Match_Case& n) noexcept
{
  return std::format("{} => {}", n.evaluator.dump(), n.codeblock.dump());
}
std::string utils::Dump::dump_Operation_Cast_As(const ast::Operation_Cast_As& n) noexcept
{
  switch (n.cast_type) {
  case ast::Operation_Cast_As::ECastType::AS: return std::format("{} as {}", n.expression.dump(), n.type.dump());
  case ast::Operation_Cast_As::ECastType::AS_REINTERPRET:
    return std::format("{} as! {}", n.expression.dump(), n.type.dump());
  case ast::Operation_Cast_As::ECastType::AS_SAFE: return std::format("{} as? {}", n.expression.dump(), n.type.dump());
  }
}
std::string utils::Dump::dump_Operation_Is(const ast::Operation_Is& n) noexcept
{
  return std::format("{} is {}", n.left.dump(), n.right.dump());
}
std::string utils::Dump::dump_Operation_In(const ast::Operation_In& n) noexcept
{
  return std::format("{} in {}", n.left.dump(), n.right.dump());
}
std::string utils::Dump::dump_Operation_Transfert(const ast::Operation_Transfert& n) noexcept
{
  if (n.assignment_op != ast::EOp_Bin::NONE)
    return std::format("{} {}= {}", n.left.dump(), ast::EOp_Bin_to_str(n.assignment_op), n.right.dump());

  if (n.assignment_type != ast::ETransfertType::NONE)
    return std::format("{} {}= {}", n.left.dump(), ast::ETransfertType_to_str(n.assignment_type), n.right.dump());

  assert(false);
}
std::string utils::Dump::dump_Operation_Binary(const ast::Operation_Binary& n) noexcept
{
  return std::format("{} {} {}", n.left.dump(), ast::EOp_Bin_to_str(n.op_ty), n.right.dump());
}
std::string utils::Dump::dump_Operation_Unary(const ast::Operation_Unary& n) noexcept
{
  return std::format("{}{}", ast::EOp_Unary_to_str(n.unary_op), n.base.dump());
}
std::string utils::Dump::dump_Operation_Interval(const ast::Operation_Interval& n) noexcept
{
  return std::format("{} {} {} {} {}", n.left.dump(), ast::EOp_Bin_to_str(n.left_comparator), n.center.dump(),
                     ast::EOp_Bin_to_str(n.right_comparator), n.right.dump());
}
std::string utils::Dump::dump_Memory_Del(const ast::Memory_Del& n) noexcept
{
  return std::format("del {}", n.target.dump());
}
std::string utils::Dump::dump_Memory_Align(const ast::Memory_Align& n) noexcept
{
  return std::format("align {}", n.target.dump());
}
std::string utils::Dump::dump_Memory_Drop(const ast::Memory_Drop& n) noexcept
{
  return std::format("drop {}", n.target.dump());
}

#undef NOT_DEFINED
#undef PROHIBIED