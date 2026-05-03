#pragma once

#include <initializer_list>
#include <string_view>

#include "nexus/ast/ast.hpp"
#include "nexus/forward.hpp"

namespace parser
{

// necessary macro to avoid any header dependence and have a minimal of boilerplate inside the parsing code

// add node to the current script linked to the parser context
#define parser_add_node(name, type, token_id)                                                                          \
  static_assert(std::is_base_of_v<ast::Node, ast::type>, "The node type must inherit from ast::Node");                 \
  auto __id_##name    = p.scr_info.nodes->add<ast::type>();                                                            \
  auto name           = p.scr_info.nodes->get_as<ast::type>(__id_##name);                                              \
  name->node_token_id = token_id;

#define BAD_NODE_ID ast::GNID_Factory::make_gnid(p.scr_info.id, ast::_id(0));


// necessary macro to avoid any header dependence and have a minimal of boilerplate inside the parsing code
// factory can't be forwarded (nested structure)

#define parser_type_factory compiler::COMPILER.types.factory

bool is_gen_args(token::Viewer& tokView);


struct Parser_Context final {
  explicit Parser_Context(script::_id p_scr_info);
  ~Parser_Context();

  script::_id            scr_id;
  script::ScriptInfo&    scr_info;
  ast::Arena* const      compilation_nodes;
  metacode::ScriptGraph& meta;
  module::_id            current_module_id;
  scope::_id             current_scope_id;
  token::Viewer* const   tok_v;
  size_t                 node_count = 0;

  bool in_extern = false;
  bool in_export = false;

  [[nodiscard]] bool start_parsing();


  // debug purpose on error
  void                           attempt_recovery();
  // match separator, or end instruction or and error return true if end is encounter
  [[nodiscard]] bool             match_field_separator(token::ETokenKind separator, token::ETokenKind end);
  // return true if end is encounter
  [[nodiscard]] bool             match_field_any_separator(token::ETokenKind                        p_separator,
                                                           std::initializer_list<token::ETokenKind> p_end);
  [[nodiscard]] std::string_view parse_name(std::string_view msg = "", std::string_view hint = "");
  // MetablockManager shortcut for ASTNode
  [[nodiscard]] bool             metablock_contains(size_t file_pos, std::string_view s) const;
  [[nodiscard]] bool             metablock_contains(size_t file_pos, token::ETokenKind t) const;

  [[nodiscard]] const metacode::Instruction* get_instruction(size_t                                  file_pos,
                                                             std::initializer_list<std::string_view> pattern) const;
  [[nodiscard]] const metacode::Metacode*    get_metacode(size_t                                  file_pos,
                                                          std::initializer_list<std::string_view> pattern) const;

  void enter_scope(ast::Node& node, std::string_view debug_name);
  void exit_scope();

  void enter_module(ast::Node& node, std::string_view debug_name);
  void exit_module();

  std::string_view tok_to_str(token::_id id) const;
  size_t           tok_to_pos(token::_id id) const;

  // token viewer navigation

  token::Token& expect(ErrorCode code, token::ETokenKind tok, std::string_view msg, std::string_view hint);
  token::Token& expect_any(ErrorCode code, const std::initializer_list<token::ETokenKind>& types, std::string_view msg,
                           std::string_view hint);
  token::Token& next();
  [[nodiscard]] bool          is_end();
  bool                        match(token::ETokenKind tok);
  bool                        match_val(std::string_view val) const;
  bool                        match_any(std::initializer_list<token::ETokenKind> toks);
  [[nodiscard]] bool          check(token::ETokenKind tok) const;
  [[nodiscard]] bool          check_at(size_t offset, token::ETokenKind tok) const;
  [[nodiscard]] bool          check_val(std::string_view val) const;
  [[nodiscard]] bool          check_any(std::initializer_list<token::ETokenKind> toks) const;
  [[nodiscard]] token::Token& peek(size_t offset = 0) const;
  void                        rewind(size_t pos);

  symbol::_id add_symbol(ast::_id n_id);

  void add_error(ErrorCode code, std::string_view msg, std::string_view hint) const;
  void add_error_tok(ErrorCode code, const token::Token& tok, std::string_view msg, std::string_view hint) const;

  [[nodiscard]] module::Module& get_current_module() const;
  [[nodiscard]] scope::Scope&   get_current_scope() const;

  // sub parsers accessible to all

  Parser_Expression* const        p_expr;
  Parser_Type* const              p_type;
  Parser_Literal* const           p_lit;
  Parser_Declaration_Local* const p_loc;
  Parser_Operator* const          p_op;
  Parser_Memory* const            p_mem;
  Parser_Declaration* const       p_decl;
  Parser_Declaration_COP* const   p_cop;
  Parser_Statement* const         p_state;
  Parser_Base* const              p_base;
};

} // namespace parser