#include "ast_operation.hpp"

#include "ast_inferred_type_singleton.hpp"

ast::operation::Is::Is()
{
  inferred_type = ast::type::get_bool_type();
}

ast::operation::In::In()
{
  inferred_type = ast::type::get_bool_type();
}

ast::operation::Interval::Interval()
{
  inferred_type = ast::type::get_bool_type();
}

ast::operation::Ptr_Dist::Ptr_Dist()
{
  inferred_type = ast::type::get_ptrdiff_type();
}