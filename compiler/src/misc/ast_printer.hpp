#pragma once

#include "ast/ast_literal.hpp"
#include "nexus/ast/forward.hpp"
#include "nexus/type/forward.hpp"

#include <string_view>

struct AST_Printer final {
  std::string out_print;

  /*
  // identifiers
  void print_ID(const ast::ID& n);
  void print_Symbol_Qualified(const ast::Symbol_Qualified& n);
  void print_Symbol_Type(const ast::Symbol_Type& n);

  void print_Path_Regex(const ast::Path_Regex& n);

  void print_Root(const ast::Root& n);

  void print_Import(const ast::Import& n);

  // declarations
  // globals
  void print_Global_Variable(const ast::Global_Variable& n);
  void print_Global_Function(const ast::Global_Function& n);
  void print_Global_Module(const ast::Global_Module& n);
  void print_Global_Extern(const ast::Global_Extern& n);
  void print_Global_Export(const ast::Global_Export& n);
  void print_Global_Reexport(const ast::Global_Reexport& n);
  void print_Global_Enum(const ast::Global_Enum& n);
  void print_Global_Flag(const ast::Global_Flag& n);
  void print_Global_Union(const ast::Global_Union& n);
  void print_Global_Alias_Type(const ast::Global_Alias_Type& n);
  void print_Global_Alias_Module(const ast::Global_Alias_Module& n);
  void print_Global_Generic(const ast::Global_Generic& n);

  void print_Enum_Field(const ast::Enum_Field& n);
  void print_Flag_Field(const ast::Flag_Field& n);
  void print_Union_Field(const ast::Union_Field& n);

  // declarations
  // locals
  void print_CodeBlock(const ast::CodeBlock& n);
  void print_Local_Lambda(const ast::Local_Lambda& n);
  void print_Local_Lambda_Capture(const ast::Local_Lambda_Capture& n);
  void print_Local_Parameter(const ast::Local_Parameter& n);
  void print_Local_Gen_Param_Elem(const ast::Local_Gen_Param_Elem& n);
  void print_Local_Gen_Params(const ast::Local_Gen_Params& n);
  void print_Local_Pattern_Element(const ast::Local_Pattern_Element& n);
  void print_Local_Pattern_Enum(const ast::Local_Pattern_Enum& n);
  void print_Local_Pattern_Tuple(const ast::Local_Pattern_Tuple& n);
  void print_Local_Pattern_Form(const ast::Local_Pattern_Form& n);
  void print_Local_Pattern_Rule_Facet(const ast::Local_Pattern_Rule_Facet& n);
  void print_Local_Pattern_Facet(const ast::Local_Pattern_Facet& n);
  void print_Local_Binding(const ast::Local_Binding& n);
  void print_Local_Tuple_Destructuring(const ast::Local_Tuple_Destructuring& n);
  void print_Local_Variable(const ast::Local_Variable& n);
  void print_Local_Capability(const ast::Local_Capability& n);

  // compositional oriented paradigm
  void print_SFM_Facet(const ast::SFM_Facet& n);
  void print_SFM_View(const ast::SFM_View& n);
  void print_SFM_Form(const ast::SFM_Form& n);
  void print_SFM_Form_New(const ast::SFM_Form_New& n);
  void print_SFM_Form_Del(const ast::SFM_Form_Del& n);
  void print_SFM_Form_Cast(const ast::SFM_Form_Cast& n);
  void print_SFM_Form_Op(const ast::SFM_Form_Op& n);
  void print_SFM_Form_Access_Op(const ast::SFM_Form_Access_Op& n);
  void print_SFM_Form_Transfert(const ast::SFM_Form_Transfert& n);
  void print_SFM_Rule(const ast::SFM_Rule& n);
  void print_SFM_Facet_Field(const ast::SFM_Facet_Field& n);
  void print_SFM_Rule_Case(const ast::SFM_Rule_Case& n);

  // generics
  void print_Generic_Type(const ast::Generic_Type& n);
  void print_Generic_Can_Cast(const ast::Generic_Can_Cast& n);
  void print_Generic_Have_Op(const ast::Generic_Have_Op& n);
  void print_Generic_View(const ast::Generic_View& n);
  void print_Generic_Facet(const ast::Generic_Facet& n);
  void print_Generic_Rule(const ast::Generic_Rule& n);

  // literals
  void print_Literal_Boolean(const ast::Literal_Boolean& n);
  void print_Literal_Integral(const ast::Literal_Integral& n);
  void print_Literal_Fixed_Point(const ast::Literal_Fixed_Point& n);
  void print_Literal_Floating_Point(const ast::Literal_Floating_Point& n);
  void print_Literal_Cune(const ast::Literal_Cune& n);
  void print_Literal_Rune(const ast::Literal_Rune& n);
  void print_Literal_Text_Pure(const ast::Literal_Text_Pure& n);
  void print_Literal_Text_Interpolation(const ast::Literal_Text_Interpolation& n);
  void print_Literal_Textual_Format(const ast::Literal_Textual_Format& n);
  void print_Literal_Format_Specifier(const ast::Literal_Format_Specifier& n);
  void print_Literal_Table(const ast::Literal_Table& n);
  void print_Literal_Table_Population(const ast::Literal_Table_Population& n);
  void print_Literal_Map(const ast::Literal_Map& n);
  void print_Literal_Tuple(const ast::Literal_Tuple& n);
  void print_Literal_Range(const ast::Literal_Range& n);
  void print_Literal_Iterator(const ast::Literal_Iterator& n);
  void print_Literal_Enum(const ast::Literal_Enum& n);
  void print_Literal_Record(const ast::Literal_Record& n);

  // expressions
  void print_Expression_If_Ternary(const ast::Expression_If_Ternary& n);
  void print_Expression_Member_Access(const ast::Expression_Member_Access& n);
  void print_Expression_Self(const ast::Expression_Self& n);
  void print_Expression_Other(const ast::Expression_Other& n);
  void print_Expression_Invocation(const ast::Expression_Invocation& n);
  void print_Expression_Invocation_Argument(const ast::Expression_Invocation_Argument& n);
  void print_Expression_Invocation_Rule(const ast::Expression_Invocation_Rule& n);
  void print_Expression_Invocation_Pipe(const ast::Expression_Invocation_Pipe& n);
  void print_Expression_Table_Access(const ast::Expression_Table_Access& n);
  void print_Expression_Ptr_Val(const ast::Expression_Ptr_Val& n);
  void print_Expression_Mut_Of(const ast::Expression_Mut_Of& n);
  void print_Expression_Ref_Of(const ast::Expression_Ref_Of& n);
  void print_Expression_Move_Of(const ast::Expression_Move_Of& n);
  void print_Expression_Copy_Of(const ast::Expression_Copy_Of& n);
  void print_Expression_Addr_Of(const ast::Expression_Addr_Of& n);
  void print_Expression_Size_Of(const ast::Expression_Size_Of& n);
  void print_Expression_GetBits(const ast::Expression_GetBits& n);
  void print_Expression_New_Ptr(const ast::Expression_New_Ptr& n);
  void print_Expression_Get_Type(const ast::Expression_Get_Type& n);

  // statements
  void print_Statement_If(const ast::Statement_If& n);
  void print_Statement_For(const ast::Statement_For& n);
  void print_Statement_Loop(const ast::Statement_Loop& n);
  void print_Statement_While(const ast::Statement_While& n);
  void print_Statement_GoTo(const ast::Statement_GoTo& n);
  void print_Statement_GoTo_Label(const ast::Statement_GoTo_Label& n);
  void print_Statement_Return(const ast::Statement_Return& n);
  void print_Statement_Break(const ast::Statement_Break& n);
  void print_Statement_Continue(const ast::Statement_Continue& n);
  void print_Statement_Match(const ast::Statement_Match& n);
  void print_Statement_Match_Case(const ast::Statement_Match_Case& n);

  // operations
  void print_Operation_Cast_As(const ast::Operation_Cast_As& n);
  void print_Operation_Is(const ast::Operation_Is& n);
  void print_Operation_In(const ast::Operation_In& n);
  void print_Operation_Transfert(const ast::Operation_Transfert& n);
  void print_Operation_Binary(const ast::Operation_Binary& n);
  void print_Operation_Unary(const ast::Operation_Unary& n);
  void print_Operation_Interval(const ast::Operation_Interval& n);

  // memory
  void print_Memory_Del(const ast::Memory_Del& n);
  void print_Memory_Align(const ast::Memory_Align& n);
  void print_Memory_Drop(const ast::Memory_Drop& n);

  std::string get_file_path() const;
  */
};

// %0 tolza-compiler version
// %1 file path
// %2 ast view
constexpr std::string_view PRINT_FILE_HTLM_TEMPLATE =
    R"(
<!DOCTYPE html>

<html lang="en">
<head>
<meta charset="UTF-8">
<title>Tolza AST View</title>


<style>
  body {
    font-family: Consolas, "Courier New", monospace;
    font-size: 12px;
    background: #1E1E1E;
    color: #D4D4D4;
    margin: 20px;
  }

  h1 {
    font-size: 18px;
    margin-bottom: 10px;
  }

  h2 {
    font-size: 14px;
    margin-bottom: 5px;
  }

  #controls {
    margin-bottom: 10px;
    display: flex;
    flex-direction: row;
    gap: 10px;
  }

  button {
    padding: 6px 10px;
    font-size: 12px;
    cursor: pointer;
    border: 1px solid #5c5c5c;
    border-radius: 15px;
    background: #333333;
    width: 150px;
    color: #D4D4D4;
  }

  ul {
    list-style-type: none;
    padding-left: 1em;
  }

  li {
    margin: 2px 0;
    line-height: 2em;
  }

  .node {
    cursor: pointer;
    font-weight: 600;
    position: relative;
    padding-left: 1em;
    transition: background 0.2s;
    border-radius: 15px;
  }

  .node:hover {
    background: #1e1e1e27;
  }

  .leaf {
    cursor: default;
    font-weight: normal;
    padding-left: 1em;
    background: #22123b2d;
    border-radius: 15px;
  }

  .children {
    display: none;
    margin-left: 1em;
  }

  .expanded > .children {
    display: block;
  }

  .node::before {
    content: "▶";
    display: inline-block;
    width: 1em;
    transition: transform 0.2s;
  }

  .expanded::before {
    content: "▼";
  }

  .leaf::before {
    content: "● ";
  }

  #ast-container {
    background: #333333;
    padding: 10px;
    border-radius: 15px;
    box-shadow: 0 2px 6px rgba(0,0,0,0.1);
    max-height: 80vh;
    overflow: auto;
  }
</style>
</head>
<body>

<h1>Tolza AST View - %1</h1>
<h2>Auto generated .html file by tolza-compiler version %0</h2>

<div id="controls">
  <button id="expand-all">Expand all</button>
  <button id="collapse-all">Collapse all</button>
</div>

<div id="ast-container">
<ul id="ast">
  %2
</ul>
</div>

<script>
  const astNodes = document.querySelectorAll('#ast li');

  astNodes.forEach(node => {
    const childrenUl = node.querySelector(':scope > .children');
    const hasChildLi = childrenUl && childrenUl.querySelector('li');
    if (hasChildLi) {
      node.addEventListener('click', e => {
        e.stopPropagation();
        node.classList.toggle('expanded');
      });
    } else {
      node.classList.remove('node');
      node.classList.add('leaf');
    }
  });

  document.getElementById('expand-all').addEventListener('click', () => {
    astNodes.forEach(node => {
      if (node.classList.contains('node')) node.classList.add('expanded');
    });
  });

  document.getElementById('collapse-all').addEventListener('click', () => {
    astNodes.forEach(node => {
      if (node.classList.contains('node')) node.classList.remove('expanded');
    });
  });
</script>

</body>
</html>
)";