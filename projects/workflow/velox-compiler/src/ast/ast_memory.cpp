#include "ast_memory.hpp"

#include "visitor/visitor_base.hpp"


void ast::memory::Del::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::memory::Align::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::memory::Drop::accept(Visitor_Base& v)
{
  v.visit(*this);
}
