#include "resolver_base.hpp"

#include "compiler/compiler.hpp"
#include "misc/error_output.hpp"

#include "nexus/ast/data.hpp"
#include "nexus/ast/definition.hpp"
#include "nexus/ast/forward.hpp"
#include "compiler/compilation_unit.hpp"
#include "nexus/lexer/token.hpp"


void resolver::Base::add_error(ErrorCode code, const ast::NodeHeader& n, std::string_view msg,
                               std::string_view hint) const
{
  auto& tok   = CU.file_info.tokens->get(n.start_tokid);
  auto  error = Error_Diagnostic(CU.cuid, code, n.nodeid, current_EPhase(), msg, hint);
  compiler::COMPILER.add_error(std::move(error));
}

void resolver::Base::add_error_two_nodes(ErrorCode code, const ast::NodeHeader& first, const ast::NodeHeader& second,
                                         std::string_view msg, std::string_view hint) const
{
  auto& first_tok  = CU.file_info.tokens->get(first.start_tokid);
  auto& second_tok = CU.file_info.tokens->get(second.start_tokid);
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
