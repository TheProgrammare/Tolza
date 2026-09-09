#include "pipeline/resolver.hpp"

#include "compiler/compiler.hpp"
#include "compiler/file_info.hpp"
#include "compiler/io.hpp"
#include "id/cuid.hpp"
#include "pipeline/pipeline.hpp"
#include "resolver/eval.hpp"
#include "resolver/inference.hpp"
#include "resolver/semantic.hpp"
#include "resolver/symbol.hpp"

#include <chrono>
#include <cstddef>
#include <functional>
#include <ratio>


inline double timing(const std::function<void()>& f) noexcept
{
  auto start = std::chrono::high_resolution_clock::now();

  f();

  auto end = std::chrono::high_resolution_clock::now();

  double milli = std::chrono::duration<double, std::milli>(end - start).count();

  return milli;
}

bool resolver::resolve_cu(cu::ID cuid) noexcept
{
  size_t err_count  = COMPILER.errors.size();
  auto   have_error = [&]() { return COMPILER.errors.size() > err_count; };

  auto& cu = cuid.get();

  size_t sym = 0;
  size_t ty  = 0;
  size_t sem = 0;

  IO::print("Resolve \"{}\" >> ", cu.file_info.path);

  static size_t count        = 1;
  auto          sym_duration = timing([&]() { sym = symbol_resolution_cu(cuid); });
  IO::print_raw("sym {} refs {:.2f} ms >> ", sym, sym_duration);
  if (have_error()) {
    IO::println(stderr, IO_PASS::resolver_symbol, "\"{}\" {:.2} ms", cu.file_info.path, sym_duration);
    return false;
  }
  auto ty_duration = timing([&]() { ty = inference_resolution_cu(cuid); });
  IO::print_raw("ty {} infs {:.2f} ms >> ", ty, ty_duration);
  if (have_error()) {
    IO::println(stderr, IO_PASS::resolver_type, "\"{}\" {:.2} ms", cu.file_info.path, ty_duration);
    return false;
  }
  auto sem_duration = timing([&]() { sem = semantic_resolution_cu(cuid); });
  IO::print_raw("sem {:.2f} ms\n", sem_duration);
  if (have_error()) {
    IO::println(stderr, IO_PASS::resolver_semantic, "\"{}\" {:.2} ms", cu.file_info.path, sem_duration);
    return false;
  }

  count++;

  if (!have_error()) PIPELINE.analyzed_compilation_units.insert(cuid);
  return !have_error();
}
size_t resolver::symbol_resolution_cu(cu::ID cuid) noexcept
{
  resolver::Symbol sym(cuid.get());
  return sym.start_resolver();
}
size_t resolver::inference_resolution_cu(cu::ID cuid) noexcept
{
  resolver::Inference inf(cuid.get());
  return inf.start_resolver();
}
size_t resolver::semantic_resolution_cu(cu::ID cuid) noexcept
{
  resolver::Semantic sem(cuid.get());
  return sem.start_resolver();
}
size_t resolver::eval_resolution_cu(cu::ID cuid) noexcept
{
  resolver::Evaluator eval(cuid.get());
  return eval.start_resolver();
}