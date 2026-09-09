#include "ast/dumper.hpp"

#include "ast/data.hpp"
#include "ast/forward.hpp"
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
#include "ast/tool.hpp"
#include "compiler/file_info.hpp"
#include "id/nodeid.hpp"
#include "nexus/forward.hpp"
#include "resolver/dumper.hpp"
#include "type/data.hpp"
#include "type/dumper.hpp"
#include "type/type.hpp"

#include <cassert>
#include <cstddef>
#include <format>
#include <iterator>
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
    case_n(Call_Contract);
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
    case_n(Expression_New_Ptr);
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
    case_n(Operation_Mem);
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
  return dump(n.name);
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
  return std::format("{} as {}", dump(n.regex), n.alias);
}
std::string utils::Dump::dump_Global_Variable(const ast::Global_Variable& n) noexcept
{
  if (n.expression)
    return std::format("{} {}: {} {} {}", ast::EVariableKind_to_str(n.kind), n.name, dump(n.type),
                       ast::ETransfertType_to_str(n.assignment), dump(n.expression));
  if (n.is_uninit) return std::format("{} {}: {} = uninit", ast::EVariableKind_to_str(n.kind), n.name, dump(n.type));

  assert(false);
}
std::string utils::Dump::dump_Global_Function(const ast::Global_Function& n) noexcept
{
  std::string params;
  params.reserve(n.parameters.size() * 32);
  for (auto param : n.parameters) params += dump(param);

  return std::format("fn {}({}){} -> {}\n{}", n.name, params, n.contract ? "\n" + dump(n.contract) : "",
                     dump(n.prototype.as<type::Prototype>()->ret), dump(n.codeblock));
}
std::string utils::Dump::dump_Call_Contract(const ast::Call_Contract& n) noexcept
{
  if (n.pre && n.post)
    return std::format("pre {} -> {}\npost {} -> {}", dump(n.pre), ast::ECallContract_to_str(n.pre_mode), dump(n.post),
                       ast::ECallContract_to_str(n.post_mode));
  if (n.pre) return std::format("pre {} -> {}", dump(n.pre), ast::ECallContract_to_str(n.pre_mode));
  if (n.post) return std::format("post {} -> {}", dump(n.post), ast::ECallContract_to_str(n.post_mode));

  return {};
}

std::string utils::Dump::dump_Global_Extend_Fn(const ast::Global_Extend_Fn& n) noexcept
{
  std::string params;
  params.reserve(n.parameters.size() * 32);
  for (auto param : n.parameters) params += dump(param);

  if (n.is_self_const)
    return std::format("extend {} fn {}(ref self, {}){}\n{}", dump(n.extended_type), n.name, params,
                       n.is_explicit_ret ? " -> " + dump(n.prototype.as<type::Prototype>()->ret) : "",
                       dump(n.codeblock));
  if (!n.is_static)
    return std::format("extend {} fn {}(mut self, {}){}\n{}", dump(n.extended_type), n.name, params,
                       n.is_explicit_ret ? " -> " + dump(n.prototype.as<type::Prototype>()->ret) : "",
                       dump(n.codeblock));

  return std::format("extend {} fn {}({}){}\n{}", dump(n.extended_type), n.name, params,
                     n.is_explicit_ret ? " -> " + dump(n.prototype.as<type::Prototype>()->ret) : "", dump(n.codeblock));
}
std::string utils::Dump::dump_Global_Extend_Cast(const ast::Global_Extend_Cast& n) noexcept
{
  return std::format("extend {} as {}\n{}", dump(n.extended_type), dump(n.as_type), dump(n.codeblock));
}
std::string utils::Dump::dump_Global_Extend_Op_Bin(const ast::Global_Extend_Op_Bin& n) noexcept
{
  return std::format("extend {} op {}\n{}", dump(n.extended_type), ast::EOp_Bin_to_str(n.bin_op), dump(n.codeblock));
}
std::string utils::Dump::dump_Global_Extend_Op_Un(const ast::Global_Extend_Op_Un& n) noexcept
{
  return std::format("extend {} op {}\n{}", dump(n.extended_type), ast::EOp_Unary_to_str(n.unary_op),
                     dump(n.codeblock));
}
std::string utils::Dump::dump_Global_Extend_Op_Subscript(const ast::Global_Extend_Op_Subscript& n) noexcept
{
  return std::format("extend {} op {}\n{}", dump(n.extended_type), ast::EOp_Subscript_to_str(n.subscript_op),
                     dump(n.codeblock));
}
std::string utils::Dump::dump_Global_Extend_Op_Transfert(const ast::Global_Extend_Op_Transfert& n) noexcept
{
  return std::format("extend {} op {}\n{}", dump(n.extended_type), ast::ETransfertType_to_str(n.transfert_op),
                     dump(n.codeblock));
}
std::string utils::Dump::dump_Global_Extend_Op_Other(const ast::Global_Extend_Op_Other& n) noexcept
{
  return std::format("extend {} op {}\n{}", dump(n.extended_type), ast::EOp_Other_to_str(n.other_op).substr(1),
                     dump(n.codeblock));
}
std::string utils::Dump::dump_Global_Module(const ast::Global_Module& n) noexcept
{
  return std::format("mod {} {}", n.name, dump(n.codeblock));
}
std::string utils::Dump::dump_Global_Extern(const ast::Global_Extern& n) noexcept
{
  return std::format("extend \"{}\"\n{}", n.abi, dump(n.codeblock));
}
std::string utils::Dump::dump_Global_Export(const ast::Global_Export& n) noexcept
{
  return std::format("export {}", dump(n.codeblock));
}
std::string utils::Dump::dump_Global_Reexport(const ast::Global_Reexport& n) noexcept
{
  return std::format("reexport {} as {}", dump(n.regex), n.alias);
}
std::string utils::Dump::dump_Global_Enum(const ast::Global_Enum& n) noexcept
{
  std::string variants;
  variants.reserve(n.variants.size() * 24);
  for (auto elem : n.variants) std::format_to(std::back_inserter(variants), "  {},\n", dump(elem));
  return std::format("enum {} {{\n{}\n}}", n.name, variants);
}
std::string utils::Dump::dump_Global_Flag(const ast::Global_Flag& n) noexcept
{
  std::string flags;
  flags.reserve(n.flags.size() * 12);
  for (auto elem : n.flags) std::format_to(std::back_inserter(flags), "  {},\n", dump(elem));
  return std::format("flag {} {{\n{}\n}}", n.name, flags);
}
std::string utils::Dump::dump_Global_Union(const ast::Global_Union& n) noexcept
{
  std::string variants;
  variants.reserve(n.variants.size() * 12);
  for (auto elem : n.variants) std::format_to(std::back_inserter(variants), "  {},\n", dump(elem));
  return std::format("union {} {{\n{}\n}}", n.name, variants);
}
std::string utils::Dump::dump_Global_Alias_Type(const ast::Global_Alias_Type& n) noexcept
{
  return std::format("type {} = {}", n.alias, n.type ? dump(n.type) : "opaque");
}
std::string utils::Dump::dump_Global_Alias_Module(const ast::Global_Alias_Module& n) noexcept
{
  return std::format("mod {} = {}", n.alias, dump(n.regex));
}
std::string utils::Dump::dump_Global_Generic(const ast::Global_Generic& n) noexcept
{
  NOT_DEFINED;
}
std::string utils::Dump::dump_Enum_Field(const ast::Enum_Field& n) noexcept
{
  return std::format("{}({})", n.name, n.type ? dump(n.type) : "");
}
std::string utils::Dump::dump_Flag_Field(const ast::Flag_Field& n) noexcept
{
  return n.name;
}
std::string utils::Dump::dump_Union_Field(const ast::Union_Field& n) noexcept
{
  return std::format("{}: {}", n.name, dump(n.type));
}
std::string utils::Dump::dump_CodeBlock(const ast::CodeBlock& n) noexcept
{
  std::string elements;
  elements.reserve(n.elements.size() * 32);
  for (auto elem : n.elements) std::format_to(std::back_inserter(elements), "  {}\n", dump(elem));
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
  return std::format("{} {}: {}{}", ast::EPassMode_to_str(n.passmode), n.name, dump(n.type),
                     n.default_value ? " = " + dump(n.default_value) : "");
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
  for (auto elem : n.bindings) std::format_to(std::back_inserter(bindings), "{}, ", dump(elem));
  bindings = bindings.substr(0, bindings.size() - 2);
  return std::format("{}({}) = {}", ast::EVariableKind_to_str(n.kind), bindings, dump(n.expression));
}
std::string utils::Dump::dump_Local_Variable(const ast::Local_Variable& n) noexcept
{
  if (n.expression)
    return std::format("{} {}{} {} {}", ast::EVariableKind_to_str(n.kind), n.name, n.type ? ": " + dump(n.type) : "",
                       ast::ETransfertType_to_str(n.assignment), dump(n.expression));
  if (n.is_uninit)
    return std::format("{} {}{} = uninit", ast::EVariableKind_to_str(n.kind), n.name,
                       n.type ? ": " + dump(n.type) : "");

  assert(false);
}
std::string utils::Dump::dump_Local_Capability(const ast::Local_Capability& n) noexcept
{
  return std::format("{} {}{} = {}", ast::ECapability_to_str(n.kind), n.name, n.type ? ": " + dump(n.type) : "",
                     dump(n.expression));
}
std::string utils::Dump::dump_SFM_Facet(const ast::SFM_Facet& n) noexcept
{
  std::string facets;
  facets.reserve(n.fields.size() * 12);
  for (auto elem : n.fields) std::format_to(std::back_inserter(facets), "  {},\n", dump(elem));
  return std::format("facet {} {{\n{}\n}}", n.name, facets);
}
std::string utils::Dump::dump_SFM_Facet_Field(const ast::SFM_Facet_Field& n) noexcept
{
  return std::format("{}{}: {} = {}",
                     n.capability != ast::ECapability::NONE ? std::string(ast::ECapability_to_str(n.capability)) + " "
                                                            : "",
                     n.name, dump(n.type), dump(n.default_value));
}
std::string utils::Dump::dump_SFM_View(const ast::SFM_View& n) noexcept
{
  std::string facets;
  facets.reserve(n.facets.size() * 12);
  for (auto elem : n.facets) std::format_to(std::back_inserter(facets), "  {},\n", dump(elem));
  return std::format("view {} {{\n{}\n}}", n.name, facets);
}
std::string utils::Dump::dump_SFM_Form(const ast::SFM_Form& n) noexcept
{
  std::string facets;
  facets.reserve(n.facets.size() * 12);
  for (auto elem : n.facets) std::format_to(std::back_inserter(facets), "  {},\n", dump(elem));
  return std::format("form {} {{\n{}\n}}", n.name, facets);
}
std::string utils::Dump::dump_SFM_Rule(const ast::SFM_Rule& n) noexcept
{
  std::string parameters;
  parameters.reserve(n.parameters.size() * 24);
  for (auto param : n.parameters) parameters += dump(param);
  std::string cases;
  cases.reserve(n.cases.size() * 64);
  for (auto elem : n.cases) std::format_to(std::back_inserter(cases), "  {}\n", dump(elem));
  return std::format("rule {}({}){} {{\n{}\n}}", n.name, parameters,
                     n.is_explicit_ret ? " -> " + dump(n.prototype.as<type::Prototype>()->ret) : "", cases);
}
std::string utils::Dump::dump_SFM_Rule_Case(const ast::SFM_Rule_Case& n) noexcept
{
  std::string bindings;
  bindings.reserve(n.bindings.size() * 16);
  for (auto elem : n.bindings) bindings += std::format("{} + ", dump(elem));
  bindings = bindings.substr(0, bindings.size() - 3);
  return std::format("{} => {}", bindings, dump(n.codeblock));
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
  return std::format(R"("{}"{})", n.val, type::ETextType_to_str(n.text_type).substr(1));
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

  std::string out;
  out.reserve(n.values.size() * 12);

  if (ast::is_table_population(n.nodeid())) {
    for (const auto& range : n.ranges) out += dump(range) + "; ";

    out = out.substr(0, out.size() - 2);

    std::string injection;
    for (const auto& inject : n.values) injection += dump(inject[0]) + "; ";

    return std::format("[{} => {}]", out, injection);
  }
  if (ast::is_map(n.nodeid())) {
    for (size_t i = 0; i < n.values.size(); i++) {
      const auto& d_key = n.values[i];
      const auto& d_val = n.map_value[i];

      for (size_t j = 0; j < n.values.size(); j++) {
        const auto& key = d_key[j];
        const auto& val = d_val[j];

        out += std::format("{}: {},", dump(key), dump(val));
      }

      out += "; ";
    }
    out = out.substr(0, out.size() - 2);
    return std::format("[{}]", out);
  }

  // regular table
  for (const auto& dim : n.values) {
    for (const auto& val : dim) out += dump(val) + ", ";
    out += "; ";
  }
  out = out.substr(0, out.size() - 2);

  return std::format("[{}]", out);
}
std::string utils::Dump::dump_Literal_Tuple(const ast::Literal_Tuple& n) noexcept
{
  std::string fields;
  fields.reserve(n.fields.size() * 12);
  for (auto elem : n.fields) std::format_to(std::back_inserter(fields), "{}, ", dump(elem.value));
  fields = fields.substr(0, fields.size() - 2);
  return std::format("({})", fields);
}
std::string utils::Dump::dump_Literal_Range(const ast::Literal_Range& n) noexcept
{
  return std::format("{}{}{}", dump(n.start), n.endInclude ? "..=" : "..", dump(n.end));
}
std::string utils::Dump::dump_Literal_Record(const ast::Literal_Record& n) noexcept
{
  std::string fields;
  fields.reserve(n.fields_names.size() * 32);
  for (size_t i = 0; i < n.fields_names.size(); i++)
    std::format_to(std::back_inserter(fields), ".{}= {},\n", n.fields_names[i], dump(n.fields_args[i]));
  fields = fields.substr(0, fields.size() - 2);
  return std::format("{}{{\n{}\n}}", dump(n.name), fields);
}
std::string utils::Dump::dump_Expression_If_Ternary(const ast::Expression_If_Ternary& n) noexcept
{
  NOT_DEFINED;
}
std::string utils::Dump::dump_Expression_Member_Access(const ast::Expression_Member_Access& n) noexcept
{
  return std::format("{}.{}", dump(n.left_expression), dump(n.right_identifier));
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
  for (auto elem : n.arguments) std::format_to(std::back_inserter(arguments), "{}, ", dump(elem));
  arguments = arguments.substr(0, arguments.size() - 2);
  return std::format("{}({})", dump(n.callee), arguments);
}
std::string utils::Dump::dump_Expression_Invocation_Arg(const ast::Expression_Invocation_Arg& n) noexcept
{
  return std::format("{}{}", n.explicit_name.empty() ? "" : n.explicit_name + "= ", dump(n.expression));
}
std::string utils::Dump::dump_Expression_Invocation_Extend(const ast::Expression_Invocation_Extend& n) noexcept
{
  std::string arguments;
  arguments.resize(n.arguments.size() * 32);
  for (auto elem : n.arguments) std::format_to(std::back_inserter(arguments), "{}, ", dump(elem));
  return std::format("{}.{}({})", dump(n.target_form), dump(n.callee), arguments);
}
std::string utils::Dump::dump_Expression_Invocation_Rule(const ast::Expression_Invocation_Rule& n) noexcept
{
  std::string arguments;
  arguments.resize(n.arguments.size() * 32);
  for (auto elem : n.arguments) std::format_to(std::back_inserter(arguments), "{}, ", dump(elem));
  return std::format("{}->{}({})", dump(n.target_form), dump(n.callee), arguments);
}
std::string utils::Dump::dump_Expression_Table_Access(const ast::Expression_Table_Access& n) noexcept
{
  return std::format("{}{}[{}]", dump(n.target), n.bounded ? "?" : "", dump(n.selector));
}
std::string utils::Dump::dump_Expression_New_Ptr(const ast::Expression_New_Ptr& n) noexcept
{
  return std::format("new ptr'{}({})", dump(n.type), dump(n.expression));
}
std::string utils::Dump::dump_Statement_If(const ast::Statement_If& n) noexcept
{
  std::string if_naming = n.is_else ? "else" : n.is_elif ? "elif" : "if";
  return std::format("{}{} {}", if_naming, n.is_else ? "" : dump(n.evaluator), dump(n.codeblock));
}
std::string utils::Dump::dump_Statement_For(const ast::Statement_For& n) noexcept
{
  std::string items;
  items.reserve(n.items.size() * 13);
  if (n.index) items += dump(n.index);
  if (n.index && !n.items.empty()) items += ", ";
  for (auto elem : n.items) items += dump(elem) + ", ";
  return std::format("for {} in {} {}", items, dump(n.expression), dump(n.codeblock));
}
std::string utils::Dump::dump_Statement_Loop(const ast::Statement_Loop& n) noexcept
{
  return std::format("loop {}", dump(n.codeblock));
}
std::string utils::Dump::dump_Statement_While(const ast::Statement_While& n) noexcept
{
  return std::format("while {} {}", dump(n.evaluator), dump(n.codeblock));
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
  return std::format("return {}", dump(n.value));
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
  for (auto elem : n.cases) cases += dump(elem);
  if (n.other_case) cases += dump(n.other_case);
  return std::format("match {} {{\n{}\n}}", dump(n.base), cases);
}
std::string utils::Dump::dump_Statement_Match_Case(const ast::Statement_Match_Case& n) noexcept
{
  return std::format("{} => {}", dump(n.evaluator), dump(n.codeblock));
}
std::string utils::Dump::dump_Operation_Cast_As(const ast::Operation_Cast_As& n) noexcept
{
  switch (n.cast_type) {
  case ast::Operation_Cast_As::ECastType::AS: return std::format("{} as {}", dump(n.expression), dump(n.type));
  case ast::Operation_Cast_As::ECastType::AS_REINTERPRET:
    return std::format("{} as! {}", dump(n.expression), dump(n.type));
  case ast::Operation_Cast_As::ECastType::AS_SAFE: return std::format("{} as? {}", dump(n.expression), dump(n.type));
  }
}
std::string utils::Dump::dump_Operation_Is(const ast::Operation_Is& n) noexcept
{
  return std::format("{} is {}", dump(n.left), dump(n.right));
}
std::string utils::Dump::dump_Operation_In(const ast::Operation_In& n) noexcept
{
  return std::format("{} in {}", dump(n.left), dump(n.right));
}
std::string utils::Dump::dump_Operation_Transfert(const ast::Operation_Transfert& n) noexcept
{
  if (n.assignment_op != ast::EOp_Bin::NONE)
    return std::format("{} {}= {}", dump(n.left), ast::EOp_Bin_to_str(n.assignment_op), dump(n.right));

  if (n.assignment_type != ast::ETransfertType::NONE)
    return std::format("{} {}= {}", dump(n.left), ast::ETransfertType_to_str(n.assignment_type), dump(n.right));

  assert(false);
}
std::string utils::Dump::dump_Operation_Binary(const ast::Operation_Binary& n) noexcept
{
  return std::format("{} {} {}", dump(n.left), ast::EOp_Bin_to_str(n.op_ty), dump(n.right));
}
std::string utils::Dump::dump_Operation_Unary(const ast::Operation_Unary& n) noexcept
{
  return std::format("{}{}", ast::EOp_Unary_to_str(n.unary_op), dump(n.base));
}
std::string utils::Dump::dump_Operation_Interval(const ast::Operation_Interval& n) noexcept
{
  return std::format("{} {} {} {} {}", dump(n.left), ast::EOp_Bin_to_str(n.left_comparator), dump(n.center),
                     ast::EOp_Bin_to_str(n.right_comparator), dump(n.right));
}
std::string utils::Dump::dump_Operation_Mem(const ast::Operation_Mem& n) noexcept
{
  return std::format("{}'{}", ast::EOp_Mem_to_str(n.op), dump(n.target));
}

#undef NOT_DEFINED
#undef PROHIBIED