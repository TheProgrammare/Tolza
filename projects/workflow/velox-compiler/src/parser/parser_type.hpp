#pragma once

#include <vector>

#include "nexus/forward.hpp"

namespace parser
{
struct Parser_Type final {
  Parser_Type(Parser_Context& p_ctx);

  struct Param final {
    token::_id     name_tok;
    std::string    name;
    ast::EPassMode passmode;
    type::_id      type;
    ast::_gnid     default_val;
  };

  struct Params final {
    std::vector<Param> params;
    bool               is_variadic = false;

    std::vector<type::_id> to_type_params() const;
  };

  struct Proto final {
    Params    params;
    type::_id ret;
    bool      is_explicit_ret;
  };

  [[nodiscard]] type::_id parse_type();

private:
  [[nodiscard]] type::_id table(const type::Decorator& decorator);
  [[nodiscard]] type::_id pointer(const type::Decorator& decorator);
  [[nodiscard]] type::_id primitive(const type::Decorator& decorator);
  [[nodiscard]] type::_id id_type(const type::Decorator& decorator);
  [[nodiscard]] type::_id tuple(const type::Decorator& decorator);
  [[nodiscard]] type::_id function_proto(const type::Decorator& decorator);
  void                    get_decorator(type::Decorator& decorator);


public:
  Proto                                parse_and_mount_local_callable(type::_id& prototype_id, bool& is_explicit_ret);
  [[nodiscard]] std::vector<type::_id> explicit_tuple();
  [[nodiscard]] ast::_gnid             get_type();
  [[nodiscard]] Proto                  explicit_function_proto(bool p_is_lam = false);
  [[nodiscard]] Params                 parameters();

  Parser_Context& p;
};
} // namespace parser