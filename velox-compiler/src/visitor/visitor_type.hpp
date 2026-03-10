#pragma once

#include <type_traits>

#include "ast/ast_base.hpp"
#include "ast/ast_forward.hpp"
#include "ast/ast_literal.hpp"
#include "visitor_default.hpp"
#include "symbol_manager.hpp"

template <typename T>
concept DerivedFromType = std::is_base_of_v<ast::AType, T>;

struct Visitor_Type : public Visitor_Default {
  // keep
  // parent
  // constructor
  using Visitor_Default::Visitor_Default;

  bool                        is_same_type(const ast::AType& p_type_1, const ast::AType& p_type_2);
  std::shared_ptr<ast::AType> resolve_type(ast::Node& n, ast::AType& input_type, bool silentError = false);


  template <DerivedFromType T>
  T* get_inferred_type(ast::INFERRED_TYPE& ty)
  {
    if (auto ptr = dynamic_cast<T*>(ty)) return ptr;
    return nullptr;
  }

  template <typename T>
  T* get_symbol(ast::Node& n, ast::SYM_REF sym_data)
  {
    if (!sym_data) {
      error_add<163>(n, "Unresolved symbol.", "");
      return nullptr;
    }
    if (!sym_data->symbol) {
      error_add<164>(n, "Invalid symbol origin", "");
      return nullptr;
    }

    if (auto ptr = dynamic_cast<T*>(sym_data->symbol.get())) {
      return ptr;
    } else {
      error_add<167>(*sym_data->symbol, "Unexpected symbol type of [" + sym_data->symbol->debug_str() + "]", "");
      return nullptr;
    }
  }


  // expression inferred type;
  void visit(ast::Expr_ID& n) override;
  void visit(ast::Expr_ID_Qualified& n) override;
  void visit(ast::Expr_ID_Type& n) override;

  void visit(ast::declaration::local::Pattern_Enum& n) override;
  void visit(ast::declaration::local::Pattern_Tuple& n) override;
  void visit(ast::declaration::local::Pattern_Entity& n) override;
  void visit(ast::declaration::local::Pattern_Component& n) override;


  void visit(ast::expression::If_Ternary& n) override;
  void visit(ast::expression::Member_Access& n) override;
  void visit(ast::expression::Self& n) override;
  void visit(ast::expression::Other& n) override;
  void visit(ast::expression::Call_Argument& n) override;
  void visit(ast::expression::Call& n) override;
  void visit(ast::expression::Call_Pipe& n) override;
  void visit(ast::expression::Table_Access& n) override;
  void visit(ast::expression::Ptr_At& n) override;
  void visit(ast::expression::Ptr_Offset& n) override;
  void visit(ast::expression::Ptr_Val& n) override;
  void visit(ast::expression::Addr_Of& n) override;
  void visit(ast::expression::Size_Of& n) override;
  void visit(ast::expression::GetBits& n) override;
  void visit(ast::expression::Move& n) override;
  void visit(ast::expression::New_Ptr& n) override;

  void visit(ast::literal::Table& n) override;
  void visit(ast::literal::Map& n) override;
  void visit(ast::literal::Text_Interpolation& n) override;
  void visit(ast::literal::Enum& n) override;
  void visit(ast::literal::Tuple& n) override;
  void visit(ast::literal::Range& n) override;
  void visit(ast::literal::Component& n) override;
  void visit(ast::literal::Entity& n) override;
  void visit(ast::literal::Iterator& n) override;
  // exception: non typed expression
  // void visit(ast::statement::GoTo &n) override;
};
