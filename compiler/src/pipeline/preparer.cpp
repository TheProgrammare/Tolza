#include "pipeline/preparer.hpp"

#include "compiler/compiler.hpp"
#include "compiler/io.hpp"
#include "lexer/lexer.hpp"
#include "metacode/preprocessor.hpp"
#include "metacode/token_generator.hpp"
#include "parser/parser_context.hpp"
#include "pipeline/pipeline.hpp"
#include "pool/link/definition.hpp"

#include <chrono>
#include <common/compiler_options.hpp>
#include <print>


inline double timing(const std::function<void()>& f) noexcept
{
  auto start = std::chrono::high_resolution_clock::now();

  f();

  auto end = std::chrono::high_resolution_clock::now();

  double milli = std::chrono::duration<double, std::milli>(end - start).count();

  return milli;
}


bool preparer::prepare_cu(cu::ID cuid) noexcept
{
  IO::print("Prepare \"{}\" >> ", cuid.get().file_info.path);
  const bool lexer_success        = lexing_cu(cuid);
  const bool preprocessor_success = preprocessing_cu(cuid);
  const bool parser_success       = parsing_cu(cuid);

  auto [_, success] = PIPELINE.prepared_compilation_units.insert(cuid);
  assert(success);

  return lexer_success && preprocessor_success && parser_success;
}

bool preparer::lexing_cu(cu::ID cuid) noexcept
{
  auto& cu = cuid.get();

  static size_t count = 1;
  if (cu.file_info.data.empty()) {
    IO::println(stderr, IO_PASS::lexer, "The file \"{}\" is empty.", cu.file_info.path);
    return false;
  }

  bool  success = false;
  Lexer lex(cu);

  auto duration = timing([&]() { success = lex.tokenize(); });

  if (success) {
    IO::print_raw("fs {:.2} ms >> ", duration);
  }
  if (!success) {
    IO::println(stderr, IO_PASS::lexer, "\"{}\" {:.2} ms", cu.file_info.path, duration);
  }

  count++;

  return success;
}
bool preparer::preprocessing_cu(cu::ID cuid) noexcept
{
  auto& cu = cuid.get();

  bool                   pre_success = false;
  bool                   gen_success = false;
  metacode::Preprocessor pre(cu);
  metacode::Generator    gen(cu, pre);

  auto pre_duration = timing([&]() { pre_success = pre.start_preprocessor(); });
  auto gen_duration = timing([&]() { gen_success = gen.start_generator(); });

  // put the final generated tokens to the script tokens
  cu.file_info.tokens->tokens = gen.tokens_generated;

  static size_t count = 1;
  if (pre_success && gen_success) {
    IO::print_raw("lex {:.2} ms >> ", pre_duration + gen_duration);
  }
  if (!pre_success) {
    IO::println(stderr, IO_PASS::preprocessor, "\"{}\" {:.2} ms", cu.file_info.path, pre_duration);
  }
  if (!gen_success) {
    IO::println(stderr, IO_PASS::preprocessor, "\"{}\" {:.2} ms", cu.file_info.path, gen_duration);
  }

  count++;

  return pre_success && gen_success;
}
bool preparer::parsing_cu(cu::ID cuid) noexcept
{
  bool                   success = false;
  parser::Parser_Context parser(cuid);

  auto duration = timing([&]() {
    success = parser.start_parsing();
    parser.CU.definitions->inject_to_resolved_def();
  });

  auto& cu = cuid.get();

  static size_t count = 1;
  if (success) {
    IO::print_raw("par {:.2} ms\n", duration);
  }

  if (!success) {
    IO::println(stderr, IO_PASS::parser, "\"{}\" {:.2} ms", cu.file_info.path, duration);
  }

  count++;

  return success;
}