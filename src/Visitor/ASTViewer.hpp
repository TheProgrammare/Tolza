#pragma once

#include <unordered_map>

#include "AST/AST_Forward.hpp"
#include "Visitor_Default.hpp"

// escape bad characters for dot
static inline std::string escapeDot(const std::string &s)
{
  std::string out;
  out.reserve(s.size() + 8);
  for (char c : s) {
    switch (c) {
    case '\\': out += "\\\\"; break;
    case '"':  out += "\\\""; break;
    case '\n': out += "\\n"; break;
    case '.':  out += "_"; break;
    case ' ':  out += "_"; break;
    case '\r': break;
    default:   out += c; break;
    }
  }
  return out;
}

struct ScriptInfo;

struct AST_Viewer : public Visitor_Default {
public:
  AST_Viewer(ScriptInfo &scrInfo, std::ostream &os) : Visitor_Default(scrInfo), os_(os) {}

  void parent_dot(const AST::Node &n, const std::string &context = "");

  void child_dot(const AST::Node &parent, const AST::Node &child, const std::string &context = "");

private:
  size_t                                      lastLine = 0;
  std::ostream                               &os_;
  std::unordered_map<const AST::Node *, int>  id_;
  std::unordered_map<const AST::AType *, int> id_ty_;
  int                                         next_ = 0;

  int get_id(const AST::Node &n)
  {
    auto it = id_.find(&n);
    if (it != id_.end()) return it->second;
    int id  = next_++;
    id_[&n] = id;
    return id;
  }
  int get_id_type(const AST::AType &n)
  {
    auto it = id_ty_.find(&n);
    if (it != id_ty_.end()) return it->second;
    int id     = next_++;
    id_ty_[&n] = id;
    return id;
  }

public:
  //	// ============ AST ============
  //	void visit(AST::Node &n) override;
  //
  //  void visit(AST::AType &n) override;
  //  void visit(AST::ALiteral &n) override;
  //  void visit(AST::ADeclaration &n) override;
  //  void visit(AST::ALocal&n) override;
  //  void visit(AST::AReference &n) override;
  //  void visit(AST::Identifier_Reference &n) override;
  //  void visit(AST::Type_Reference &n) override;
  //  void visit(AST::Type_Arguments &n) override;
  //
  //	void visit(AST::Root &n) override;
  //
  //	void visit(AST::ID &n) override;
  //
  //	// ============ DECLARATION ============
  //	void visit(AST::Declaration::Global &n) override;
  //	void visit(AST::Declaration::Function &n) override;
  //
  //	void visit(AST::Declaration::Mod &n) override;
  //	void visit(AST::Declaration::Export &n) override;
  //
  //	void visit(AST::Declaration::Enum &n) override;
  //	void visit(AST::Declaration::Enum_Element &n) override;
  //
  //	void visit(AST::Declaration::Flag &n) override;
  //
  //	void visit(AST::Declaration::Type_Alias &n) override;
  //
  //	void visit(AST::Declaration::Generic &n) override;
  //
  //	// ============ LOCAL ============
  //	void visit(AST::Declaration::Local::CodeBlock &n) override;
  //
  //	void visit(AST::Declaration::Local::Lambda &n) override;
  //	void visit(AST::Declaration::Local::Lambda_Capture &n) override;
  //	void visit(AST::Declaration::Local::Capture_Member &n) override;
  //
  //	void visit(AST::Declaration::Local::Parameter &n) override;
  //	void visit(AST::Declaration::Local::Generic_Parameter &n) override;
  //
  //	void visit(AST::Declaration::Local::Pattern &n) override;
  //	void visit(AST::Declaration::Local::Pattern_Enum &n) override;
  //	void visit(AST::Declaration::Local::Pattern_Tuple &n) override;
  //	void visit(AST::Declaration::Local::Pattern_Entity &n) override;
  //	void visit(AST::Declaration::Local::Pattern_Component &n) override;
  //
  //	void visit(AST::Declaration::Local::Variable_Binding &n) override;
  //	void visit(AST::Declaration::Local::Variable_Unpack &n) override;
  //	void visit(AST::Declaration::Local::Variable &n) override;
  //
  //	void visit(AST::Declaration::Local::Capability &n) override;
  //
  //	// ============ COP ============
  //	void visit(AST::Declaration::COP::Component &n) override;
  //	void visit(AST::Declaration::COP::Component_Field &n) override;
  //
  //	void visit(AST::Declaration::COP::Role &n) override;
  //
  //	void visit(AST::Declaration::COP::Entity &n) override;
  //	void visit(AST::Declaration::COP::Entity_Cast &n) override;
  //	void visit(AST::Declaration::COP::Entity_Op &n) override;
  //	void visit(AST::Declaration::COP::Entity_OpIndex &n) override;
  //
  //	void visit(AST::Declaration::COP::System &n) override;
  //	void visit(AST::Declaration::COP::System_Case &n) override;
  //
  //	// ============ GENERIC ============
  //	void visit(AST::Generic::Is_Type &n) override;
  //	void visit(AST::Generic::Can_Cast &n) override;
  //	void visit(AST::Generic::Have_Op &n) override;
  //	void visit(AST::Generic::Have_Role &n) override;
  //	void visit(AST::Generic::Use_Component &n) override;
  //	void visit(AST::Generic::Compatible_System &n) override;
  //
  //	// ============ TYPE ============
  //	void visit(AST::Type::Ptr &n) override;
  //	void visit(AST::Type::Table &n) override;
  //	void visit(AST::Type::Primitive &n) override;
  //	void visit(AST::Type::Tuple &n) override;
  //	void visit(AST::Type::Function_Proto &n) override;
  //
  //	void visit(AST::Type::Get_Expr_Type &n) override;
  //
  //	// ============ LITERAL ============
  //	void visit(AST::Literal::Boolean &n) override;
  //	void visit(AST::Literal::Integral &n) override;
  //	void visit(AST::Literal::Decimal &n) override;
  //	void visit(AST::Literal::Floating &n) override;
  //
  //	void visit(AST::Literal::ASCII &n) override;
  //	void visit(AST::Literal::UFT32 &n) override;
  //
  //	void visit(AST::Literal::Text &n) override;
  //	void visit(AST::Literal::Text_Lerp &n) override;
  //	void visit(AST::Literal::Textual_Element &n) override;
  //	void visit(AST::Literal::Textual_Format &n) override;
  //	void visit(AST::Literal::Format_Specifier &n) override;
  //
  //	void visit(AST::Literal::Table &n) override;
  //	void visit(AST::Literal::Table_Population &n) override;
  //
  //	void visit(AST::Literal::Map &n) override;
  //
  //	void visit(AST::Literal::Tuple &n) override;
  //
  //	void visit(AST::Literal::Range &n) override;
  //
  //	void visit(AST::Literal::Component &n) override;
  //	void visit(AST::Literal::Entity &n) override;
  //
  //	// ============ REFERENCE ============
  //	void visit(AST::Reference::Enum &n) override;
  //
  //	void visit(AST::Reference::Member_Access &n) override;
  //
  //	void visit(AST::Reference::Self &n) override;
  //	void visit(AST::Reference::Other &n) override;
  //
  //	void visit(AST::Reference::Call &n) override;
  //	void visit(AST::Reference::Call_Argument &n) override;
  //	void visit(AST::Reference::Call_System &n) override;
  //	void visit(AST::Reference::Call_Pipe &n) override;
  //
  //	void visit(AST::Reference::Table_Access &n) override;
  //
  //	// ============ STATEMENT ============
  //	void visit(AST::Statement::If &n) override;
  //	void visit(AST::Statement::If_Ternary &n) override;
  //
  //	void visit(AST::Statement::For &n) override;
  //	void visit(AST::Statement::Loop &n) override;
  //	void visit(AST::Statement::While &n) override;
  //	void visit(AST::Statement::GoTo &n) override;
  //	void visit(AST::Statement::GoTo_Label &n) override;
  //
  //	void visit(AST::Statement::Return &n) override;
  //	void visit(AST::Statement::Break &n) override;
  //	void visit(AST::Statement::Continue &n) override;
  //
  //	void visit(AST::Statement::Match &n) override;
  //	void visit(AST::Statement::Match_Case &n) override;
  //
  //	// ============ OPERATION ============
  //	void visit(AST::Operation::Cast_As &n) override;
  //	void visit(AST::Operation::Is &n) override;
  //	void visit(AST::Operation::In &n) override;
  //	void visit(AST::Operation::Assignment &n) override;
  //	void visit(AST::Operation::Binary &n) override;
  //	void visit(AST::Operation::Unary &n) override;
  //	void visit(AST::Operation::Interval &n) override;
  //
  //
  //	// ============ MEMORY ============
  //	void visit(AST::Memory::Move &n) override;
  //	void visit(AST::Memory::New &n) override;
  //	void visit(AST::Memory::Del &n) override;
  //	void visit(AST::Memory::Val_Of_Ptr &n) override;
  //	void visit(AST::Memory::Addr_Of_Ref &n) override;
  //	void visit(AST::Memory::Dist &n) override;
  //	void visit(AST::Memory::Size &n) override;
  //	void visit(AST::Memory::Align &n) override;
  //	void visit(AST::Memory::GetBits &n) override;
  //	void visit(AST::Memory::Drop &n) override;
};
