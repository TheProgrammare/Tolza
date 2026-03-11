#include "ast_statement.hpp"

#include "ast_declaration_local.hpp"

#include "visitor/visitor_base.hpp"
#include "visitor/visitor_codegen.hpp"


void ast::statement::If::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::statement::For::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::statement::Loop::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::statement::While::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::statement::GoTo::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::statement::GoTo_Label::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::statement::Return::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::statement::Break::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::statement::Continue::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::statement::Match_Case::accept(Visitor_Base& v)
{
  v.visit(*this);
}
void ast::statement::Match::accept(Visitor_Base& v)
{
  v.visit(*this);
}
std::string ast::statement::For::debug_str() const
{
  std::string str_index = index ? "index: " + index->debug_str() : "";
  std::string str_items;
  for (auto& item : items) {
    str_items += ", " + item->debug_str();
  }

  return "FOR[" + str_index + str_items + "]";
}


ast::statement::If::~If()                 = default;
ast::statement::For::~For()               = default;
ast::statement::Loop::~Loop()             = default;
ast::statement::While::~While()           = default;
ast::statement::Match_Case::~Match_Case() = default;
