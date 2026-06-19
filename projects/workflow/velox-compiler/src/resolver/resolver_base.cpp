#include "resolver_base.hpp"

#include "compiler/compiler.hpp"
#include "misc/error_output.hpp"

#include "nexus/ast/ast.hpp"
#include "compiler/compilation_unit.hpp"
#include "nexus/lexer/token.hpp"


void resolver::Base::add_error(ErrorCode code, const ast::Node& n, std::string_view msg, std::string_view hint) const
{
  auto& tok   = CU.file_info.tokens->get(n.node_token_id);
  auto  error = Error_Diagnostic(CU.cuid, code, tok.begin, tok.begin + tok.length, current_EPhase(), msg, hint);
  compiler::COMPILER.add_error(std::move(error));
}

void resolver::Base::add_error_two_nodes(ErrorCode code, const ast::Node& first, const ast::Node& second,
                                         std::string_view msg, std::string_view hint) const
{
  auto& first_tok  = CU.file_info.tokens->get(first.node_token_id);
  auto& second_tok = CU.file_info.tokens->get(second.node_token_id);
  auto  err_first =
      Error_Elem(CU.cuid, code, first_tok.begin, first_tok.begin + first_tok.length, current_EPhase(), msg, hint);
  auto err_second =
      Error_Elem(CU.cuid, code, second_tok.begin, second_tok.begin + first_tok.length, current_EPhase(), msg, hint);
  Error_Diagnostic err(err_first, err_second);
  compiler::COMPILER.add_error(err);
}

compiler::EPhase resolver::Base::current_EPhase() const
{
  return compiler::EPhase::resolver_symbol;
}
