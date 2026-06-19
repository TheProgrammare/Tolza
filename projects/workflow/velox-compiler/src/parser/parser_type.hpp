#pragma once

#include <vector>

#include "nexus/forward.hpp"

namespace parser
{
struct Parser_Type final {
  Parser_Type(Parser_Context& p_ctx);

  struct Param final {
    token::ID      name_tok;
    std::string    name;
    ast::EPassMode passmode;
    type::ID       type;
    ast::ID        default_val;
    bool           is_restrict = false;
  };

  struct Params final {
    std::vector<Param> params;
    bool               is_variadic = false;

    [[nodiscard]] std::vector<type::ID> to_type_params() const;
  };

  struct Proto final {
    Params   params;
    type::ID ret;
    bool     is_explicit_ret;
  };

  [[nodiscard]] type::ID parse_type();

private:
  [[nodiscard]] type::ID table(const type::Qualifier& qualifier);
  [[nodiscard]] type::ID pointer(const type::Qualifier& qualifier);
  [[nodiscard]] type::ID primitive(const type::Qualifier& qualifier);
  [[nodiscard]] type::ID id_type(const type::Qualifier& qualifier);
  [[nodiscard]] type::ID tuple(const type::Qualifier& qualifier);
  [[nodiscard]] type::ID function_proto(const type::Qualifier& qualifier);
  void                   get_qualifier(type::Qualifier& qualifier);


public:
  [[nodiscard]] Proto                 parse_and_mount_local_callable(type::ID& prototype_id, bool& is_explicit_ret);
  [[nodiscard]] std::vector<type::ID> explicit_tuple();
  [[nodiscard]] ast::ID               get_type();
  [[nodiscard]] Proto                 explicit_function_proto(bool p_is_lam = false);
  [[nodiscard]] Params                parameters();

  Parser_Context& p;
};
} // namespace parser