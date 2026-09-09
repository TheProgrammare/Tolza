#pragma once

#include <cstdint>
template <typename TYPE>
struct id_query;


namespace ast
{
struct NodeHeader;
struct ID;
} // namespace ast


namespace token
{
struct Token;
class _file_pos;
struct ID;
} // namespace token

namespace metacode
{
struct MetacodeHeader;
struct ID;
} // namespace metacode

namespace type
{
struct TypeHeader;
struct ID;
} // namespace type

namespace module
{
struct Module;
struct ID;
} // namespace module

namespace scope
{
struct Scope;
struct ID;
} // namespace scope

namespace definition
{
struct Definition;
struct ID;
} // namespace definition


#define DEF_QUERY(IDENTIFIER, TYPE)                                                                                    \
  template <>                                                                                                          \
  struct id_query<TYPE> {                                                                                              \
    static TYPE& get(const IDENTIFIER& id);                                                                            \
  };

DEF_QUERY(ast::ID, ast::NodeHeader);
DEF_QUERY(token::ID, token::Token);
DEF_QUERY(metacode::ID, metacode::MetacodeHeader);
DEF_QUERY(type::ID, type::TypeHeader);
DEF_QUERY(module::ID, module::Module);
DEF_QUERY(scope::ID, scope::Scope);
DEF_QUERY(definition::ID, definition::Definition);
