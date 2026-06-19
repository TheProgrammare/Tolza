#pragma once

#include <cstdint>
#include <string>

enum class EBinOpType : uint8_t;

namespace llvm
{
class APFloat;
}

struct LLVM_Tools;

namespace resolver
{
struct Codegen;
}


struct Static_Evaluator {
  Static_Evaluator(resolver::Codegen& _v);

  resolver::Codegen& v;
  LLVM_Tools*        tools;
  /*
    void add_error();

    std::expected<ast::ALiteral*, std::string> evaluate_expression(ast::AExpression& value);

    bool float_almost_eq_ULP(const llvm::APFloat& L, const llvm::APFloat& R, unsigned maxULP = 4);

    std::expected<ast::ALiteral*, std::string> integral(const ast::literal::Integral& L, const ast::literal::Integral&
    R, EBinOpType op); std::expected<ast::ALiteral*, std::string> floating(const ast::literal::Floating_Point& L, const
    ast::literal::Floating_Point& R, EBinOpType op); std::expected<ast::ALiteral*, std::string> decimal(const
    ast::literal::Fixed_Point& L, const ast::literal::Fixed_Point& R, EBinOpType op); std::expected<ast::ALiteral*,
    std::string> boolean(const ast::literal::Boolean& L, const ast::literal::Boolean& R, EBinOpType op);
    std::expected<ast::ALiteral*, std::string> boolean_not(const ast::ALiteral& term);
    std::expected<ast::ALiteral*, std::string> scalar_minus(const ast::ALiteral& term);
    std::expected<ast::ALiteral*, std::string> scalar_plus(const ast::ALiteral& term);
    */
};