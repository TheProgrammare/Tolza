#pragma once

#include "ast/ast_declaration_global.hpp"
#include "ast/ast_declaration_cop.hpp"
#include <string_view>

struct AST_Printer final {
  std::string out_print;
  /*
    // ============ AST ============
    void visit(ast::Node& n) override;

    void visit(ast::AType& n) override;
    void visit(ast::ALiteral& n) override;
    void visit(ast::ADeclaration& n) override;
    void visit(ast::ALocal& n) override;
    void visit(ast::AExpression& n) override;
    void visit(ast::AIdentifier& n) override;
    void visit(ast::Expr_ID& n) override;
    void visit(ast::Expr_ID_Qualified& n) override;
    void visit(ast::Expr_ID_Type& n) override;

    void visit(ast::Root& n) override;

    // ============ DECLARATION ============
    void visit(ast::declaration::Global_Variable& n) override;
    void visit(ast::declaration::Function& n) override;

    void visit(ast::declaration::Mod& n) override;
    void visit(ast::declaration::Export& n) override;
    void visit(ast::declaration::Extern& n) override;

    void visit(ast::declaration::Enum& n) override;
    void visit(ast::declaration::Enum_Element& n) override;

    void visit(ast::declaration::Flag& n) override;
    void visit(ast::declaration::Union& n) override;

    void visit(ast::declaration::Module_Alias& n) override;
    void visit(ast::declaration::Type_Alias& n) override;

    void visit(ast::declaration::Global_Generic& n) override;

    // ============ LOCAL ============
    void visit(Local_CodeBlock& n) override;

    void visit(Local_Lambda& n) override;
    void visit(Local_Lambda_Capture& n) override;
    void visit(Local_Capture_Member& n) override;

    void visit(Local_Parameter& n) override;
    void visit(Local_Generic_Parameter_Element& n) override;
    void visit(Local_Generic_Parameters& n) override;

    void visit(Local_Pattern& n) override;
    void visit(Local_Pattern_Enum& n) override;
    void visit(Local_Pattern_Tuple& n) override;
    void visit(Local_Pattern_Entity& n) override;
    void visit(Local_Pattern_System_Component& n) override;
    void visit(Local_Pattern_Component& n) override;

    void visit(Local_Variable_Binding& n) override;
    void visit(Local_Tuple_Destructuring& n) override;
    void visit(Local_Variable& n) override;

    void visit(Local_Capability& n) override;

    // ============ COP ============
    void visit(ast::declaration::cop::Component& n) override;
    void visit(ast::declaration::cop::Component_Field& n) override;

    void visit(ast::declaration::cop::Role& n) override;

    void visit(ast::declaration::cop::Entity& n) override;
    void visit(ast::declaration::cop::Entity_New& n) override;
    void visit(ast::declaration::cop::Entity_Del& n) override;
    void visit(ast::declaration::cop::Entity_Cast& n) override;
    void visit(ast::declaration::cop::Entity_Op& n) override;
    void visit(ast::declaration::cop::Entity_Access_Op& n) override;
    void visit(ast::declaration::cop::Entity_Transfert& n) override;

    void visit(ast::declaration::cop::System& n) override;
    void visit(ast::declaration::cop::System_Case& n) override;

    // ============ GENERIC ============
    void visit(ast::generic::Is_Type& n) override;
    void visit(ast::generic::Can_Cast& n) override;
    void visit(ast::generic::Have_Op& n) override;
    void visit(ast::generic::Have_Role& n) override;
    void visit(ast::generic::Use_Component& n) override;
    void visit(ast::generic::Compatible_System& n) override;

    // ============ TYPE ============
    void visit(ast::type::Ptr& n) override;
    void visit(ast::type::Table& n) override;
    void visit(ast::type::Primitive& n) override;
    void visit(ast::type::Tuple& n) override;
    void visit(ast::type::Function_Proto& n) override;

    void visit(ast::type::Get_Expr_Type& n) override;

    // ============ LITERAL ============
    void visit(ast::literal::Boolean& n) override;
    void visit(ast::literal::Integral& n) override;
    void visit(ast::literal::Fixed_Point& n) override;
    void visit(ast::literal::Floating_Point& n) override;

    void visit(ast::literal::CUNE& n) override;
    void visit(ast::literal::RUNE& n) override;

    void visit(ast::literal::Text_Pure& n) override;
    void visit(ast::literal::Text_Interpolation& n) override;
    void visit(ast::literal::Textual_Format& n) override;
    void visit(ast::literal::Format_Specifier& n) override;

    void visit(ast::literal::Table& n) override;
    void visit(ast::literal::Table_Population& n) override;

    void visit(ast::literal::Map& n) override;

    void visit(ast::literal::Tuple& n) override;

    void visit(ast::literal::Range& n) override;
    void visit(ast::literal::Iterator& n) override;

    void visit(ast::literal::Enum& n) override;

    void visit(ast::literal::Structured_Data& n) override;
    void visit(ast::literal::Entity& n) override;

    // ============ Expression ============
    void visit(ast::expression::If_Ternary& n) override;

    void visit(ast::expression::Member_Access& n) override;

    void visit(ast::expression::Self& n) override;
    void visit(ast::expression::Other& n) override;

    void visit(ast::expression::Call& n) override;
    void visit(ast::expression::Call_Argument& n) override;
    void visit(ast::expression::Call_System& n) override;
    void visit(ast::expression::Call_Pipe& n) override;

    void visit(ast::expression::Table_Access& n) override;

    void visit(ast::expression::Ptr_At& n) override;
    void visit(ast::expression::Ptr_Offset& n) override;
    void visit(ast::expression::Ptr_Val& n) override;
    void visit(ast::expression::Mut_Of& n) override;
    void visit(ast::expression::Ref_Of& n) override;
    void visit(ast::expression::Addr_Of& n) override;
    void visit(ast::expression::Size_Of& n) override;
    void visit(ast::expression::GetBits& n) override;

    void visit(ast::expression::Move& n) override;
    void visit(ast::expression::New_Ptr& n) override;

    // ============ STATEMENT ============
    void visit(ast::statement::If& n) override;

    void visit(ast::statement::For& n) override;
    void visit(ast::statement::Loop& n) override;
    void visit(ast::statement::While& n) override;
    void visit(ast::statement::GoTo& n) override;
    void visit(ast::statement::GoTo_Label& n) override;

    void visit(ast::statement::Return& n) override;
    void visit(ast::statement::Break& n) override;
    void visit(ast::statement::Continue& n) override;

    void visit(ast::statement::Match& n) override;
    void visit(ast::statement::Match_Case& n) override;

    // ============ OPERATION ============
    void visit(ast::operation::Cast_As& n) override;
    void visit(ast::operation::Is& n) override;
    void visit(ast::operation::In& n) override;
    void visit(ast::operation::Assignment& n) override;
    void visit(ast::operation::Binary& n) override;
    void visit(ast::operation::Unary& n) override;
    void visit(ast::operation::Interval& n) override;
    void visit(ast::operation::Ptr_Dist& n) override;

    // ============ MEMORY ============
    void visit(ast::memory::Del& n) override;
    void visit(ast::memory::Align& n) override;
    void visit(ast::memory::Drop& n) override;

    std::string get_file_path() const;
    */
};

// %0 velox-compiler version
// %1 file path
// %2 ast view
constexpr std::string_view PRINT_FILE_HTLM_TEMPLATE =
    R"(
<!DOCTYPE html>

<html lang="en">
<head>
<meta charset="UTF-8">
<title>Velox AST View</title>


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

<h1>Velox AST View - %1</h1>
<h2>Auto generated .html file by velox-compiler version %0</h2>

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