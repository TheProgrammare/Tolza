#pragma once

#include <memory>

#include "ast_base.hpp"

namespace ast
{

namespace generic
{

struct IGenCond : public Node {
  virtual ~IGenCond()                                              = default;
  // for semantic viewer
  [[nodiscard]] virtual bool type_isValid(const AType& type) const = 0;
};

struct Is_Type final : IGenCond {
  std::string                         source_typename;
  std::vector<std::shared_ptr<AType>> in_type;

  std::shared_ptr<ast::declaration::Generic> parent_generic;

  VISTOR_ACCEPT


  bool type_isValid(const AType& type) const override
  {
    for (auto& _type : in_type) {
      // if (*ptr == type) return true;
    }
    return false;
  }
  std::string debug_str() const override
  {
    return "gen is";
  }
};

struct Can_Cast final : IGenCond {
  std::string            source_typename;      // typename
  std::shared_ptr<AType> target;               // cast target
  bool                   is_cast_from = false; // false = cast to | true = cast from

  std::shared_ptr<ast::declaration::Generic> parent_generic;

  VISTOR_ACCEPT


  bool type_isValid(const AType& type) const override
  {
    // return *target == type;
    return false;
  }
  std::string debug_str() const override
  {
    return std::string("gen cast ") + (is_cast_from ? "from" : "to");
  }
};

struct Have_Op final : IGenCond {
  std::string            target_gen_sym;          // typename
  EBinOpType             op_ty = EBinOpType::Add; // operator
  std::shared_ptr<AType> explicit_return_type;    // for indexation/iterator

  std::shared_ptr<ast::declaration::Generic> parent_generic;

  VISTOR_ACCEPT


  bool        type_isValid(const AType& type) const override;
  std::string debug_str() const override
  {
    return "gen op";
  }
};

struct Have_Role final : IGenCond {
  std::string                  target_gen_sym;
  std::unique_ptr<AExpression> role;

  std::shared_ptr<ast::declaration::Generic> parent_generic;

  VISTOR_ACCEPT


  std::shared_ptr<declaration::cop::Role> resolved_role_sym;

  bool        type_isValid(const AType& type) const override;
  std::string debug_str() const override
  {
    return "gen role";
  }
};

struct Use_Component final : IGenCond {
  std::string                  target_gen_sym;
  std::unique_ptr<AExpression> component;

  std::shared_ptr<ast::declaration::Generic> parent_generic;

  VISTOR_ACCEPT


  std::shared_ptr<declaration::cop::Component> resolved_comp_sym;

  bool        type_isValid(const AType& type) const override;
  std::string debug_str() const override
  {
    return "gen component";
  }
};

struct Compatible_System final : IGenCond {
  std::string                  target_gen_sym;
  std::unique_ptr<AExpression> system;

  std::shared_ptr<ast::declaration::Generic> parent_generic;

  VISTOR_ACCEPT


  std::shared_ptr<declaration::cop::System> resolved_system_sym;

  bool        type_isValid(const AType& type) const override;
  std::string debug_str() const override
  {
    return "gen system";
  }
};

} // namespace generic
  // Generic
} // namespace ast
  // AST