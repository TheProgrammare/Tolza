#pragma once

#include <memory>
#include <string>

#include "visitor/symbol_manager.hpp"

#include "lexer/token.hpp"
#include "lexer/token_viewer.hpp"

#include "ast/ast_base.hpp"

struct ScriptInfo;
struct Symbol_Manager;

template <typename NodeType>
concept DerivedFromNode = std::is_base_of_v<ast::Node, NodeType> && !std::is_base_of_v<ast::ADeclaration, NodeType>;

template <typename NodeType>
concept DerivedFromDecl = std::is_base_of_v<ast::ADeclaration, NodeType> || std::is_base_of_v<ast::ALocal, NodeType>;

namespace meta
{
struct MetablockManager;
struct MetaInstruct;
struct Metablock;
} // namespace meta

struct Symbols_Manager;

namespace parser
{
bool is_gen_args(TokenViewer& tokView);
struct Parser_Base;
struct Parser_Expression;
struct Parser_Type;
struct Parser_Literal;
struct Parser_Declaration_Local;
struct Parser_Operator;
struct Parser_Memory;
struct Parser_Declaration;
struct Parser_Declaration_COP;
struct Parser_Statement;

struct Parser_Context {
  explicit Parser_Context(ScriptInfo& _scr_info);
  ~Parser_Context();

  ScriptInfo&             scr_info;
  Symbols_Manager*        m_sym  = nullptr;
  meta::MetablockManager* m_meta = nullptr;
  TokenViewer             tok_v;
  size_t                  node_count = 0;

  std::shared_ptr<ast::declaration::cop::Entity> current_entity;
  std::shared_ptr<ast::Node>                     current_other;

  bool in_extern = false;

  // debug purpose on error
  void                      attempt_recovery();
  // match separator, or end instruction or and error return true if end is encounter
  [[nodiscard]] bool        match_field_separator(TokTy separator = TokTy::COMMA, TokTy end = TokTy::CLOSE_BRACE);
  // return true if end is encounter
  [[nodiscard]] bool        match_field_any_separator(TokTy                        separator = TokTy::COMMA,
                                                      std::initializer_list<TokTy> end       = {TokTy::CLOSE_BRACE});
  [[nodiscard]] std::string parse_name(const std::string& msg = "", const std::string& hint = "");
  // MetablockManager shortcut for ASTNode
  [[nodiscard]] bool        metablock_contains(const ast::Node& n, const std::string& s) const;
  [[nodiscard]] bool        metablock_contains(const ast::Node& n, TokTy t) const;
  [[nodiscard]] std::string get_export_name(const ast::Node& n) const;

  [[nodiscard]] const meta::MetaInstruct* get_instruct(const ast::Node&                          n,
                                                       const std::initializer_list<std::string>& pattern) const;
  [[nodiscard]] const meta::Metablock*    get_metablock(const ast::Node&                          n,
                                                        const std::initializer_list<std::string>& pattern);

  // sub parsers accessible to all
  Parser_Expression*        p_expr;
  Parser_Type*              p_type;
  Parser_Literal*           p_lit;
  Parser_Declaration_Local* p_loc;
  Parser_Operator*          p_op;
  Parser_Memory*            p_mem;
  Parser_Declaration*       p_decl;
  Parser_Declaration_COP*   p_cop;
  Parser_Statement*         p_state;
  Parser_Base*              p_base;


  // to create node, set some data, store in resolvers
  template <DerivedFromNode NodeType, typename... Args>
  inline std::unique_ptr<NodeType> Create_Node(Token token, Args&&... args)
  {
    static_assert(!std::is_abstract_v<NodeType>, "Create_Node cannot instantiate abstract AST nodes");

    auto node    = std::make_unique<NodeType>(std::forward<Args>(args)...);
    node->_token = token;
    node->_scope = m_sym->get_current_path();
    node_count++;
    node->_scr_info = &scr_info;
    return node;
  }
  template <DerivedFromDecl NodeType, typename... Args>
  inline std::shared_ptr<NodeType> Create_Decl(Token token, Args&&... args)
  {
    auto node    = std::make_shared<NodeType>(std::forward<Args>(args)...);
    node->_token = token;
    node->_scope = m_sym->get_current_path();
    node_count++;
    node->_scr_info = &scr_info;
    return node;
  };
};
} // namespace parser