#pragma once

#include "nexus/forward.hpp"

#include <vector>

namespace parser
{
struct Parser_Type final {
  Parser_Type(Parser_Context& p_ctx);

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
  [[nodiscard]] std::vector<type::ID> explicit_tuple();
  [[nodiscard]] ast::ID               get_type();
  // proto, parameters, contract
  [[nodiscard]] std::tuple<type::ID, std::vector<ast::ID>, ast::ID>
                                                      prototype_from_declaration(bool start_at_params = false);
  // parameters
  [[nodiscard]] type::ID                              prototype_from_type();
  [[nodiscard]] type::ID                              prototype_contract();
  // is_variadic, parameters
  [[nodiscard]] std::pair<bool, std::vector<ast::ID>> parameters();

  Parser_Context& p;
};
} // namespace parser