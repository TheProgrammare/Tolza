#include "forward.hpp"

#include "compiler/compiler.hpp"

#include <cstdlib>
#include <print>


void common::compiler::DEBUG_TOLZA_ICE(std::string_view msg)
{
#ifdef TOLZA_CODE_VALID
  std::println(stderr,
               "[tolza] Assume tolza code is valid\n"
               "[tolza:ICE] Internal compiler error: {})",
               msg);
  ::compiler::COMPILER.print_errors();
  std::abort();
#else
  (void)msg;
#endif
}
