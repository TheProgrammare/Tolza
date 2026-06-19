#pragma once

#include <initializer_list>
#include <string_view>

#include "nexus/ast/ast.hpp"
#include "nexus/forward.hpp"
#include "nexus/ids.hpp"
#include "parser/parser_declaration_extension.hpp"

namespace parser
{

// necessary macro to avoid any header dependence and have a minimal of boilerplate inside the parsing code


#define THROW_BAD_NODE throw std::runtime_error("Bad Node ID");

#define BAD_NODE_ID ast::ID::invalid();


// necessary macro to avoid any header dependence and have a minimal of boilerplate inside the parsing code
// factory can't be forwarded (nested structure)

#define parser_type_factory p.CU.types->factory

bool is_gen_args(token::Viewer& tok_v) noexcept;


struct Parser_Context final {
  explicit Parser_Context(cu::ID _cuid);
  ~Parser_Context();

  cu::ID               cuid;
  cu::CU&              CU;
  metacode::Graph&     meta;
  module::ID           current_modid;
  scope::ID            current_scpid;
  token::Viewer* const tok_v;
  size_t               node_count = 0;

  size_t scope_depth = 0;

  std::string extern_abi;
  EVisibility current_visibility = EVisibility::File_Scope;

  [[nodiscard]] bool start_parsing();


  // debug purpose on error
  void                      attempt_recovery();
  // match separator, or end instruction or and error return true if end is encounter
  [[nodiscard]] bool        match_field_separator(token::ETokenKind separator, token::ETokenKind end) const noexcept;
  // return true if end is encounter
  [[nodiscard]] bool        match_field_any_separator(token::ETokenKind                        p_separator,
                                                      std::initializer_list<token::ETokenKind> p_end) const noexcept;
  [[nodiscard]] std::string parse_name(std::string_view msg = "", std::string_view hint = "") const noexcept;
  // MetablockManager shortcut for ASTNode
  [[nodiscard]] bool        metablock_contains(size_t file_pos, std::string_view s) const;
  [[nodiscard]] bool        metablock_contains(size_t file_pos, token::ETokenKind t) const;

  [[nodiscard]] const metacode::Instruction* get_instruction(size_t                                  file_pos,
                                                             std::initializer_list<std::string_view> pattern) const;
  [[nodiscard]] const metacode::Metacode*    get_metacode(size_t                                  file_pos,
                                                          std::initializer_list<std::string_view> pattern) const;

  void enter_scope(ast::Node& node, std::string_view debug_name);
  void exit_scope();

  void enter_module(ast::Node& node, std::string_view debug_name);
  void exit_module();

  [[nodiscard]] std::string_view tok_to_str(token::ID id) const noexcept;
  [[nodiscard]] size_t           tok_to_pos(token::ID id) const noexcept;

  // token viewer navigation

  [[nodiscard]] token::Token& expect(ErrorCode code, token::ETokenKind tok, std::string_view msg,
                                     std::string_view hint) const noexcept;
  [[nodiscard]] token::Token& expect_any(ErrorCode code, const std::initializer_list<token::ETokenKind>& types,
                                         std::string_view msg, std::string_view hint) const noexcept;
  [[nodiscard]] token::Token& next() const noexcept;
  [[nodiscard]] bool          is_end() const noexcept;
  [[nodiscard]] bool          match(token::ETokenKind tok) const noexcept;
  [[nodiscard]] bool          match_val(std::string_view val) const noexcept;
  [[nodiscard]] bool          match_any(std::initializer_list<token::ETokenKind> toks) const noexcept;
  [[nodiscard]] bool          match_chain(std::initializer_list<token::ETokenKind> l) const noexcept;

  [[nodiscard]] bool          check(token::ETokenKind tok) const noexcept;
  [[nodiscard]] bool          check_at(size_t offset, token::ETokenKind tok) const noexcept;
  [[nodiscard]] bool          check_val(std::string_view val) const noexcept;
  [[nodiscard]] bool          check_any(std::initializer_list<token::ETokenKind> toks) const noexcept;
  [[nodiscard]] bool          check_chain(std::initializer_list<token::ETokenKind> l) const noexcept;
  [[nodiscard]] token::Token& peek(size_t offset = 0) const noexcept;
  void                        rewind(size_t pos) const noexcept;

  template <typename T>
  [[nodiscard]] T& add_get_node(token::ID tokid) noexcept;

  [[nodiscard]] symbol::ID add_symbol(ast::ID nodeid);

  void add_error(ErrorCode code, std::string_view msg, std::string_view hint) const;
  void add_error_tok(ErrorCode code, const token::Token& tok, std::string_view msg, std::string_view hint) const;

  [[nodiscard]] module::Module& get_current_module();
  [[nodiscard]] scope::Scope&   get_current_scope();

  // sub parsers accessible to all

  Parser_Expression* const            p_expr;
  Parser_Type* const                  p_type;
  Parser_Literal* const               p_lit;
  Parser_Declaration_Local* const     p_loc;
  Parser_Operator* const              p_op;
  Parser_Memory* const                p_mem;
  Parser_Declaration* const           p_decl;
  Parser_Declaration_Extension* const p_extend;
  Parser_Declaration_SFM* const       p_sfm;
  Parser_Statement* const             p_state;
  Parser_Base* const                  p_base;
};

} // namespace parser
