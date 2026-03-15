#pragma once

#include "ast/ast_forward.hpp"
#include <string>
#include <expected>

enum class EBinOpType;

namespace llvm
{
class APFloat;
}

struct LLVM_Tools;
struct Visitor_Codegen;


struct Static_Evaluator {
  Static_Evaluator(Visitor_Codegen& _v);

  Visitor_Codegen& v;
  LLVM_Tools*      tools;

  std::expected<ast::ALiteral*, std::string> evaluate_expression(ast::AExpression& value);

  bool float_almost_eq_ULP(const llvm::APFloat& L, const llvm::APFloat& R, unsigned maxULP = 4);

  std::expected<ast::ALiteral*, std::string> integral(const ast::literal::Integral& L, const ast::literal::Integral& R,
                                                      EBinOpType op);
  std::expected<ast::ALiteral*, std::string> floating(const ast::literal::Floating& L, const ast::literal::Floating& R,
                                                      EBinOpType op);
  std::expected<ast::ALiteral*, std::string> decimal(const ast::literal::Decimal& L, const ast::literal::Decimal& R,
                                                     EBinOpType op);
  std::expected<ast::ALiteral*, std::string> boolean(const ast::literal::Boolean& L, const ast::literal::Boolean& R,
                                                     EBinOpType op);
  std::expected<ast::ALiteral*, std::string> boolean_not(const ast::ALiteral& term);
  std::expected<ast::ALiteral*, std::string> scalar_minus(const ast::ALiteral& term);
  std::expected<ast::ALiteral*, std::string> scalar_plus(const ast::ALiteral& term);
};