#pragma once

#include <memory>
#include <string>

#include "ast/ast_declaration.hpp"
#include "lexer/token_viewer.hpp"
#include "misc/module_manager.hpp"

#include "ast/ast_base.hpp"

struct ScriptInfo;

template <typename NodeType>
concept DerivedFromNode = std::is_base_of_v<ast::Node, NodeType> && !std::is_base_of_v<ast::ADeclaration, NodeType>;

template <typename NodeType>
concept DerivedFromDecl = std::is_base_of_v<ast::ADeclaration, NodeType> || std::is_base_of_v<ast::ALocal, NodeType>;

template <typename NodeType>
concept DerivedFromType = std::is_base_of_v<ast::AType, NodeType>;


struct TokenViewer;

namespace meta
{
struct Manager;
struct MetaInstruct;
struct Metablock;
} // namespace meta

namespace symbol
{
struct Manager;
}

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
  explicit Parser_Context(std::shared_ptr<ScriptInfo> p_scr_info);
  ~Parser_Context();

  std::shared_ptr<ScriptInfo>     scr_info_sptr;
  ScriptInfo&                     scr_info;
  std::shared_ptr<meta::Manager>  meta_m;
  std::shared_ptr<module::Module> current_module;
  TokenViewer                     tok_v;
  size_t                          node_count = 0;

  std::shared_ptr<ast::declaration::cop::Entity> current_entity;
  std::shared_ptr<ast::Node>                     current_other;
  ast::ACallable*                                current_function = nullptr;

  bool in_extern = false;
  bool in_export = false;

  [[nodiscard]] std::vector<std::string> start_parsing();


  // debug purpose on error
  void                      attempt_recovery();
  // match separator, or end instruction or and error return true if end is encounter
  [[nodiscard]] bool        match_field_separator(TokTy separator = TokTy::COMMA, TokTy end = TokTy::CLOSE_BRACE);
  // return true if end is encounter
  [[nodiscard]] bool        match_field_any_separator(TokTy                        p_separator = TokTy::COMMA,
                                                      std::initializer_list<TokTy> p_end       = {TokTy::CLOSE_BRACE});
  [[nodiscard]] std::string parse_name(const std::string& msg = "", const std::string& hint = "");
  // MetablockManager shortcut for ASTNode
  [[nodiscard]] bool        metablock_contains(const ast::Node& n, const std::string& s) const;
  [[nodiscard]] bool        metablock_contains(const ast::Node& n, TokTy t) const;
  [[nodiscard]] std::string get_export_name(const ast::Node& n) const;

  [[nodiscard]] const meta::MetaInstruct* get_instruct(const ast::Node&                          n,
                                                       const std::initializer_list<std::string>& pattern) const;
  [[nodiscard]] const meta::Metablock*    get_metablock(const ast::Node&                          n,
                                                        const std::initializer_list<std::string>& pattern);

  void enter_module(std::shared_ptr<ast::ADeclaration> p_decl, std::string debug_name);
  void exit_module();

  // sub parsers accessible to all
  std::unique_ptr<Parser_Expression>        p_expr;
  std::unique_ptr<Parser_Type>              p_type;
  std::unique_ptr<Parser_Literal>           p_lit;
  std::unique_ptr<Parser_Declaration_Local> p_loc;
  std::unique_ptr<Parser_Operator>          p_op;
  std::unique_ptr<Parser_Memory>            p_mem;
  std::unique_ptr<Parser_Declaration>       p_decl;
  std::unique_ptr<Parser_Declaration_COP>   p_cop;
  std::unique_ptr<Parser_Statement>         p_state;
  std::unique_ptr<Parser_Base>              p_base;


  // to create node, set some data, store in resolvers
  template <DerivedFromNode NodeType, typename... Args>
  inline std::unique_ptr<NodeType> Create_Node(Token token, Args&&... args)
  {
    static_assert(!std::is_abstract_v<NodeType>, "Create_Node cannot instantiate abstract AST nodes");

    auto node        = std::make_unique<NodeType>(std::forward<Args>(args)...);
    node->node_token = token;
    node_count++;
    node->node_scr_info = scr_info_sptr;
    current_module->add_item(node);
    return node;
  }
  template <DerivedFromType NodeType, typename... Args>
  inline std::shared_ptr<NodeType> Create_Type(Token token, Args&&... args)
  {
    static_assert(!std::is_abstract_v<NodeType>, "Create_Rype cannot instantiate abstract AST nodes");

    auto node        = std::make_shared<NodeType>(std::forward<Args>(args)...);
    node->node_token = token;
    node_count++;
    node->node_scr_info = scr_info_sptr;
    current_module->add_item(node);
    return node;
  }
  template <DerivedFromDecl NodeType, typename... Args>
  inline std::shared_ptr<NodeType> Create_Decl(Token token, Args&&... args)
  {
    auto node        = std::make_shared<NodeType>(std::forward<Args>(args)...);
    node->node_token = token;
    node_count++;
    node->node_scr_info           = scr_info_sptr;
    node->declaration_is_exported = in_export;
    node->declaration_is_external = in_extern;
    current_module->add_item(node);
    return node;
  };
};
} // namespace parser