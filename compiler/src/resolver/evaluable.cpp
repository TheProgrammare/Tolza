#include "resolver/evaluable.hpp"

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
#include "ast/node/operation.hpp"
#include "ast/node/statement.hpp"
#include "ast/tool.hpp"
#include "id/nodeid.hpp"

#include <cassert>


#define RESOLUTION_GUARD                                                                                               \
  if (evaluated.contains(n.nodeid())) return true;


bool resolver::Evaluable::is_evaluable(ast::ID nodeid) noexcept
{
  assert(nodeid && "Must be valid id");

#define case_n(_kind)                                                                                                  \
  case ast::ENodeKind::_kind: return is_evaluable_##_kind(*nodeid.as<ast::_kind>());

  const auto kind = nodeid.kind();

  switch (kind) {
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
  case ast::ENodeKind::NONE: return false;
  }
}

bool resolver::Evaluable::add_evaluation(ast::ID nodeid, bool is_evaluable) noexcept
{
  if (!is_evaluable) return false;
  evaluated.insert(nodeid);
  return true;
}

bool resolver::Evaluable::is_evaluable_Symbol_Id(const ast::Symbol_Id& n) noexcept
{
  RESOLUTION_GUARD;

  return add_evaluation(n.nodeid(), is_evaluable(n.nodeid().def().node()));
}
bool resolver::Evaluable::is_evaluable_Symbol_Qualified(const ast::Symbol_Qualified& n) noexcept
{
  RESOLUTION_GUARD;

  return add_evaluation(n.nodeid(), is_evaluable(n.nodeid().def().node()));
}
bool resolver::Evaluable::is_evaluable_Symbol_Type(const ast::Symbol_Type& n) noexcept
{
  return false;
}
bool resolver::Evaluable::is_evaluable_Path_Regex(const ast::Path_Regex& n) noexcept
{
  return false;
}
bool resolver::Evaluable::is_evaluable_Root(const ast::Root& n) noexcept
{
  return false;
}
bool resolver::Evaluable::is_evaluable_Import(const ast::Import& n) noexcept
{
  return false;
}
bool resolver::Evaluable::is_evaluable_Global_Variable(const ast::Global_Variable& n) noexcept
{
  RESOLUTION_GUARD;

  if (n.expression) return add_evaluation(n.nodeid(), is_evaluable(n.expression));

  return false;
}
bool resolver::Evaluable::is_evaluable_Global_Function(const ast::Global_Function& n) noexcept
{
  RESOLUTION_GUARD;

  (void)is_evaluable(n.contract);

  return add_evaluation(n.nodeid(), is_evaluable(n.codeblock));
}
bool resolver::Evaluable::is_evaluable_Call_Contract(const ast::Call_Contract& n) noexcept
{
  RESOLUTION_GUARD;

  if (n.pre_mode != ast::ECallContract::Static && n.pre_mode != ast::ECallContract::NONE) return false;
  if (n.post_mode != ast::ECallContract::Static && n.post_mode != ast::ECallContract::NONE) return false;

  bool pre_evaluable  = is_evaluable(n.pre);
  bool post_evaluable = is_evaluable(n.post);

  return add_evaluation(n.nodeid(), pre_evaluable && post_evaluable);
}
bool resolver::Evaluable::is_evaluable_Global_Extend_Fn(const ast::Global_Extend_Fn& n) noexcept
{
  RESOLUTION_GUARD;

  (void)is_evaluable(n.contract);

  return add_evaluation(n.nodeid(), is_evaluable(n.codeblock));
}
bool resolver::Evaluable::is_evaluable_Global_Extend_Cast(const ast::Global_Extend_Cast& n) noexcept
{
  RESOLUTION_GUARD;

  (void)is_evaluable(n.contract);

  return add_evaluation(n.nodeid(), is_evaluable(n.codeblock));
}
bool resolver::Evaluable::is_evaluable_Global_Extend_Op_Bin(const ast::Global_Extend_Op_Bin& n) noexcept
{
  RESOLUTION_GUARD;

  (void)is_evaluable(n.contract);

  return add_evaluation(n.nodeid(), is_evaluable(n.codeblock));
}
bool resolver::Evaluable::is_evaluable_Global_Extend_Op_Un(const ast::Global_Extend_Op_Un& n) noexcept
{
  RESOLUTION_GUARD;

  (void)is_evaluable(n.contract);

  return add_evaluation(n.nodeid(), is_evaluable(n.codeblock));
}
bool resolver::Evaluable::is_evaluable_Global_Extend_Op_Subscript(const ast::Global_Extend_Op_Subscript& n) noexcept
{
  RESOLUTION_GUARD;

  (void)is_evaluable(n.contract);

  return add_evaluation(n.nodeid(), is_evaluable(n.codeblock));
}
bool resolver::Evaluable::is_evaluable_Global_Extend_Op_Transfert(const ast::Global_Extend_Op_Transfert& n) noexcept
{
  RESOLUTION_GUARD;

  (void)is_evaluable(n.contract);

  return add_evaluation(n.nodeid(), is_evaluable(n.codeblock));
}
bool resolver::Evaluable::is_evaluable_Global_Extend_Op_Other(const ast::Global_Extend_Op_Other& n) noexcept
{
  RESOLUTION_GUARD;

  (void)is_evaluable(n.contract);

  return add_evaluation(n.nodeid(), is_evaluable(n.codeblock));
}
bool resolver::Evaluable::is_evaluable_Global_Module(const ast::Global_Module& n) noexcept
{
  return false;
}
bool resolver::Evaluable::is_evaluable_Global_Extern(const ast::Global_Extern& n) noexcept
{
  return false;
}
bool resolver::Evaluable::is_evaluable_Global_Export(const ast::Global_Export& n) noexcept
{
  return false;
}
bool resolver::Evaluable::is_evaluable_Global_Reexport(const ast::Global_Reexport& n) noexcept
{
  return false;
}
bool resolver::Evaluable::is_evaluable_Global_Enum(const ast::Global_Enum& n) noexcept
{
  return false;
}
bool resolver::Evaluable::is_evaluable_Global_Flag(const ast::Global_Flag& n) noexcept
{
  return false;
}
bool resolver::Evaluable::is_evaluable_Global_Union(const ast::Global_Union& n) noexcept
{
  return false;
}
bool resolver::Evaluable::is_evaluable_Global_Alias_Type(const ast::Global_Alias_Type& n) noexcept
{
  return false;
}
bool resolver::Evaluable::is_evaluable_Global_Alias_Module(const ast::Global_Alias_Module& n) noexcept
{
  return false;
}
bool resolver::Evaluable::is_evaluable_Global_Generic(const ast::Global_Generic& n) noexcept
{
  return false;
}
bool resolver::Evaluable::is_evaluable_Enum_Field(const ast::Enum_Field& n) noexcept
{
  return false;
}
bool resolver::Evaluable::is_evaluable_Flag_Field(const ast::Flag_Field& n) noexcept
{
  return false;
}
bool resolver::Evaluable::is_evaluable_Union_Field(const ast::Union_Field& n) noexcept
{
  return false;
}
bool resolver::Evaluable::is_evaluable_CodeBlock(const ast::CodeBlock& n) noexcept
{
  RESOLUTION_GUARD;

  bool is_const = true;

  for (const auto& elem : n.elements) {
    if (!is_evaluable(elem)) is_const = false;
  }

  return is_const;
}
bool resolver::Evaluable::is_evaluable_Local_Lambda(const ast::Local_Lambda& n) noexcept
{
  RESOLUTION_GUARD;

  (void)is_evaluable(n.contract);

  return add_evaluation(n.nodeid(), is_evaluable(n.codeblock));
}
bool resolver::Evaluable::is_evaluable_Local_Lambda_Capture(const ast::Local_Lambda_Capture& n) noexcept
{
  return false;
}
bool resolver::Evaluable::is_evaluable_Local_Parameter(const ast::Local_Parameter& n) noexcept
{
  RESOLUTION_GUARD;

  return n.passmode == ast::EPassMode::_const;
}
bool resolver::Evaluable::is_evaluable_Local_Gen_Param_Elem(const ast::Local_Gen_Param_Elem& n) noexcept
{
  return false;
}
bool resolver::Evaluable::is_evaluable_Local_Gen_Params(const ast::Local_Gen_Params& n) noexcept
{
  return false;
}
bool resolver::Evaluable::is_evaluable_Local_Pattern_Element(const ast::Local_Pattern_Element& n) noexcept
{
  RESOLUTION_GUARD;

  return add_evaluation(n.nodeid(), is_evaluable(n.literal));
}
bool resolver::Evaluable::is_evaluable_Local_Pattern_Enum(const ast::Local_Pattern_Enum& n) noexcept
{
  RESOLUTION_GUARD;

  return add_evaluation(n.nodeid(), is_evaluable(n.expression));
}
bool resolver::Evaluable::is_evaluable_Local_Pattern_Tuple(const ast::Local_Pattern_Tuple& n) noexcept
{
  RESOLUTION_GUARD;

  return add_evaluation(n.nodeid(), is_evaluable(n.expression));
}
bool resolver::Evaluable::is_evaluable_Local_Pattern_Form(const ast::Local_Pattern_Form& n) noexcept
{
  RESOLUTION_GUARD;

  return add_evaluation(n.nodeid(), is_evaluable(n.expression));
}
bool resolver::Evaluable::is_evaluable_Local_Pattern_Rule_Facet(const ast::Local_Pattern_Rule_Facet& n) noexcept
{
  RESOLUTION_GUARD;

  return add_evaluation(n.nodeid(), is_evaluable(n.expression));
}
bool resolver::Evaluable::is_evaluable_Local_Pattern_Facet(const ast::Local_Pattern_Facet& n) noexcept
{
  RESOLUTION_GUARD;

  return add_evaluation(n.nodeid(), is_evaluable(n.expression));
}
bool resolver::Evaluable::is_evaluable_Local_Binding(const ast::Local_Binding& n) noexcept
{
  RESOLUTION_GUARD;

  return add_evaluation(n.nodeid(), is_evaluable(n.expression));
}
bool resolver::Evaluable::is_evaluable_Local_Tuple_Destructuring(const ast::Local_Tuple_Destructuring& n) noexcept
{
  RESOLUTION_GUARD;

  return add_evaluation(n.nodeid(), is_evaluable(n.expression));
}
bool resolver::Evaluable::is_evaluable_Local_Variable(const ast::Local_Variable& n) noexcept
{
  RESOLUTION_GUARD;

  return add_evaluation(n.nodeid(), is_evaluable(n.expression));
}
bool resolver::Evaluable::is_evaluable_Local_Capability(const ast::Local_Capability& n) noexcept
{
  RESOLUTION_GUARD;

  return add_evaluation(n.nodeid(), is_evaluable(n.expression));
}
bool resolver::Evaluable::is_evaluable_SFM_Facet(const ast::SFM_Facet& n) noexcept
{
  RESOLUTION_GUARD;

  return false;
}
bool resolver::Evaluable::is_evaluable_SFM_Facet_Field(const ast::SFM_Facet_Field& n) noexcept
{
  RESOLUTION_GUARD;

  return false;
}
bool resolver::Evaluable::is_evaluable_SFM_View(const ast::SFM_View& n) noexcept
{
  RESOLUTION_GUARD;

  return false;
}
bool resolver::Evaluable::is_evaluable_SFM_Form(const ast::SFM_Form& n) noexcept
{
  RESOLUTION_GUARD;

  return false;
}
bool resolver::Evaluable::is_evaluable_SFM_Rule(const ast::SFM_Rule& n) noexcept
{
  RESOLUTION_GUARD;

  return false;
}
bool resolver::Evaluable::is_evaluable_SFM_Rule_Case(const ast::SFM_Rule_Case& n) noexcept
{
  RESOLUTION_GUARD;

  return false;
}
bool resolver::Evaluable::is_evaluable_Generic_Type(const ast::Generic_Type& n) noexcept
{
  RESOLUTION_GUARD;

  return false;
}
bool resolver::Evaluable::is_evaluable_Generic_Cast(const ast::Generic_Cast& n) noexcept
{
  RESOLUTION_GUARD;

  return false;
}
bool resolver::Evaluable::is_evaluable_Generic_Op(const ast::Generic_Op& n) noexcept
{
  RESOLUTION_GUARD;

  return false;
}
bool resolver::Evaluable::is_evaluable_Generic_View(const ast::Generic_View& n) noexcept
{
  RESOLUTION_GUARD;

  return false;
}
bool resolver::Evaluable::is_evaluable_Generic_Facet(const ast::Generic_Facet& n) noexcept
{
  RESOLUTION_GUARD;

  return false;
}
bool resolver::Evaluable::is_evaluable_Generic_Extension(const ast::Generic_Extension& n) noexcept
{
  RESOLUTION_GUARD;

  return false;
}
bool resolver::Evaluable::is_evaluable_Generic_Rule(const ast::Generic_Rule& n) noexcept
{
  return false;
}
bool resolver::Evaluable::is_evaluable_Literal_Boolean(const ast::Literal_Boolean& n) noexcept
{
  return true;
}
bool resolver::Evaluable::is_evaluable_Literal_NullPtr(const ast::Literal_NullPtr& n) noexcept
{
  return true;
}
bool resolver::Evaluable::is_evaluable_Literal_Integral(const ast::Literal_Integral& n) noexcept
{
  return true;
}
bool resolver::Evaluable::is_evaluable_Literal_Fixed_Point(const ast::Literal_Fixed_Point& n) noexcept
{
  return true;
}
bool resolver::Evaluable::is_evaluable_Literal_Floating_Point(const ast::Literal_Floating_Point& n) noexcept
{
  return true;
}
bool resolver::Evaluable::is_evaluable_Literal_Cune(const ast::Literal_Cune& n) noexcept
{
  return true;
}
bool resolver::Evaluable::is_evaluable_Literal_Rune(const ast::Literal_Rune& n) noexcept
{
  return true;
}
bool resolver::Evaluable::is_evaluable_Literal_Text_Pure(const ast::Literal_Text_Pure& n) noexcept
{
  return true;
}
bool resolver::Evaluable::is_evaluable_Literal_Text_Interpolation(const ast::Literal_Text_Interpolation& n) noexcept
{
  RESOLUTION_GUARD;

  return add_evaluation(n.nodeid(), is_evaluable(n.expression));
}
bool resolver::Evaluable::is_evaluable_Literal_Textual_Format(const ast::Literal_Textual_Format& n) noexcept
{
  RESOLUTION_GUARD;

  bool is_const = true;

  for (const auto& interpolation : n.values) {
    if (!is_evaluable(interpolation)) return false;
  }

  return add_evaluation(n.nodeid(), is_const);
}
bool resolver::Evaluable::is_evaluable_Literal_Format_Specifier(const ast::Literal_Format_Specifier& n) noexcept
{
  RESOLUTION_GUARD;

  const bool is_const_precision = is_evaluable(n.precision);
  const bool is_const_width     = is_evaluable(n.width);

  return add_evaluation(n.nodeid(), is_const_precision && is_const_width);
}
bool resolver::Evaluable::is_evaluable_Literal_Table(const ast::Literal_Table& n) noexcept
{
  RESOLUTION_GUARD;

  bool is_const_ranges = true;
  for (const auto& range : n.ranges) {
    if (!is_evaluable(range)) is_const_ranges = false;
  }

  bool is_const_vals = true;
  for (const auto& vals : n.values) {
    for (const auto& val : vals) {
      if (!is_evaluable(val)) is_const_vals = false;
    }
  }

  bool is_const_map_vals = true;
  for (const auto& vals : n.map_value) {
    for (const auto& val : vals) {
      if (!is_evaluable(val)) is_const_map_vals = false;
    }
  }

  return add_evaluation(n.nodeid(), is_const_ranges && is_const_vals && is_const_map_vals);
}
bool resolver::Evaluable::is_evaluable_Literal_Tuple(const ast::Literal_Tuple& n) noexcept
{
  RESOLUTION_GUARD;

  bool is_const_vals = true;
  for (const auto& f : n.fields) {
    if (!is_evaluable(f.value)) is_const_vals = false;
  }

  return add_evaluation(n.nodeid(), is_const_vals);
}
bool resolver::Evaluable::is_evaluable_Literal_Range(const ast::Literal_Range& n) noexcept
{
  RESOLUTION_GUARD;

  bool is_const_start = n.start ? is_evaluable(n.start) : true; // if no start: start = 0
  bool is_const_end   = n.end ? is_evaluable(n.end) : true;     // if no end: end = ssize::max

  return add_evaluation(n.nodeid(), is_const_start && is_const_end);
}
bool resolver::Evaluable::is_evaluable_Literal_Record(const ast::Literal_Record& n) noexcept
{
  RESOLUTION_GUARD;

  bool is_const_vals = true;
  for (const auto& f : n.fields_args) {
    if (!is_evaluable(f)) is_const_vals = false;
  }

  return add_evaluation(n.nodeid(), is_const_vals);
}
bool resolver::Evaluable::is_evaluable_Expression_If_Ternary(const ast::Expression_If_Ternary& n) noexcept
{
  RESOLUTION_GUARD;

  bool is_const_cond  = is_evaluable(n.evaluator);
  bool is_const_true  = is_evaluable(n.statement_true);
  bool is_const_false = n.statement_false ? is_evaluable(n.statement_false) : true;

  return add_evaluation(n.nodeid(), is_const_cond && is_const_true && is_const_false);
}
bool resolver::Evaluable::is_evaluable_Expression_Member_Access(const ast::Expression_Member_Access& n) noexcept
{
  return false;
}
bool resolver::Evaluable::is_evaluable_Expression_Self(const ast::Expression_Self& n) noexcept
{
  return false;
}
bool resolver::Evaluable::is_evaluable_Expression_Other(const ast::Expression_Other& n) noexcept
{
  return false;
}
bool resolver::Evaluable::is_evaluable_Expression_Invocation(const ast::Expression_Invocation& n) noexcept
{
  RESOLUTION_GUARD;

  bool is_const_fn   = is_evaluable(n.callee.def().node());
  bool is_const_args = true;
  for (const auto& arg : n.arguments) {
    if (!is_evaluable(arg)) is_const_args = false;
  }

  return add_evaluation(n.nodeid(), is_const_fn && is_const_args);
}
bool resolver::Evaluable::is_evaluable_Expression_Invocation_Arg(const ast::Expression_Invocation_Arg& n) noexcept
{
  RESOLUTION_GUARD;

  return add_evaluation(n.nodeid(), is_evaluable(n.expression));
}
bool resolver::Evaluable::is_evaluable_Expression_Invocation_Extend(const ast::Expression_Invocation_Extend& n) noexcept
{
  RESOLUTION_GUARD;

  bool is_const_fn   = is_evaluable(n.callee.def().node());
  bool is_const_args = true;
  for (const auto& arg : n.arguments) {
    if (!is_evaluable(arg)) is_const_args = false;
  }

  return add_evaluation(n.nodeid(), is_const_fn && is_const_args);
}
bool resolver::Evaluable::is_evaluable_Expression_Invocation_Rule(const ast::Expression_Invocation_Rule& n) noexcept
{
  RESOLUTION_GUARD;

  bool is_const_fn   = is_evaluable(n.callee.def().node());
  bool is_const_args = true;
  for (const auto& arg : n.arguments) {
    if (!is_evaluable(arg)) is_const_args = false;
  }

  return add_evaluation(n.nodeid(), is_const_fn && is_const_args);
}
bool resolver::Evaluable::is_evaluable_Expression_Table_Access(const ast::Expression_Table_Access& n) noexcept
{
  RESOLUTION_GUARD;

  bool is_const_table    = is_evaluable(n.target);
  bool is_const_selector = is_evaluable(n.selector);

  return add_evaluation(n.nodeid(), is_const_table && is_const_selector);
}
bool resolver::Evaluable::is_evaluable_Expression_New_Ptr(const ast::Expression_New_Ptr& n) noexcept
{
  RESOLUTION_GUARD;

  return false;
}
bool resolver::Evaluable::is_evaluable_Statement_If(const ast::Statement_If& n) noexcept
{
  RESOLUTION_GUARD;

  return false;
}
bool resolver::Evaluable::is_evaluable_Statement_For(const ast::Statement_For& n) noexcept
{
  RESOLUTION_GUARD;

  return false;
}
bool resolver::Evaluable::is_evaluable_Statement_Loop(const ast::Statement_Loop& n) noexcept
{
  RESOLUTION_GUARD;

  return false;
}
bool resolver::Evaluable::is_evaluable_Statement_While(const ast::Statement_While& n) noexcept
{
  RESOLUTION_GUARD;

  return false;
}
bool resolver::Evaluable::is_evaluable_Statement_GoTo(const ast::Statement_GoTo& n) noexcept
{
  RESOLUTION_GUARD;

  return false;
}
bool resolver::Evaluable::is_evaluable_Statement_GoTo_Label(const ast::Statement_GoTo_Label& n) noexcept
{
  RESOLUTION_GUARD;

  return false;
}
bool resolver::Evaluable::is_evaluable_Statement_Return(const ast::Statement_Return& n) noexcept
{
  RESOLUTION_GUARD;

  return false;
}
bool resolver::Evaluable::is_evaluable_Statement_Break(const ast::Statement_Break& n) noexcept
{
  RESOLUTION_GUARD;

  return false;
}
bool resolver::Evaluable::is_evaluable_Statement_Continue(const ast::Statement_Continue& n) noexcept
{
  RESOLUTION_GUARD;

  return false;
}
bool resolver::Evaluable::is_evaluable_Statement_Match(const ast::Statement_Match& n) noexcept
{
  RESOLUTION_GUARD;

  return false;
}
bool resolver::Evaluable::is_evaluable_Statement_Match_Case(const ast::Statement_Match_Case& n) noexcept
{
  RESOLUTION_GUARD;

  return false;
}
bool resolver::Evaluable::is_evaluable_Operation_Cast_As(const ast::Operation_Cast_As& n) noexcept
{
  RESOLUTION_GUARD;

  return false;
}
bool resolver::Evaluable::is_evaluable_Operation_Is(const ast::Operation_Is& n) noexcept
{
  RESOLUTION_GUARD;

  return false;
}
bool resolver::Evaluable::is_evaluable_Operation_In(const ast::Operation_In& n) noexcept
{
  RESOLUTION_GUARD;

  return false;
}
bool resolver::Evaluable::is_evaluable_Operation_Transfert(const ast::Operation_Transfert& n) noexcept
{
  RESOLUTION_GUARD;

  return false;
}
bool resolver::Evaluable::is_evaluable_Operation_Binary(const ast::Operation_Binary& n) noexcept
{
  RESOLUTION_GUARD;

  return false;
}
bool resolver::Evaluable::is_evaluable_Operation_Unary(const ast::Operation_Unary& n) noexcept
{
  RESOLUTION_GUARD;

  return false;
}
bool resolver::Evaluable::is_evaluable_Operation_Interval(const ast::Operation_Interval& n) noexcept
{
  RESOLUTION_GUARD;

  return false;
}
bool resolver::Evaluable::is_evaluable_Operation_Mem(const ast::Operation_Mem& n) noexcept
{
  RESOLUTION_GUARD;

  return false;
}
