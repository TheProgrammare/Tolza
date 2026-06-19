#include "forward.hpp"

#include "compiler/compiler.hpp"

#include <cstdlib>
#include <iostream>
#include <ostream>


void common::compiler::DEBUG_VELOX_ICE(std::string_view msg)
{
#ifdef VELOX_CODE_VALID
  std::cerr << "[velox] Assume velox code is valid\n";
  std::cerr << "[velox:ICE] Internal compiler error: " << msg << "\n";
  ::compiler::COMPILER.print_errors();
  std::cerr << std::flush;
  std::abort();
#else
  (void)msg;
#endif
}
