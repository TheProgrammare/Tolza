#pragma once

#include <cstddef>
#include <cstdint>
#include <deque>
#include <string>
#include <string_view>
#include <vector>

#include "ids.hpp"


using ErrorCode = short;


namespace scope
{
struct Scope;
struct Graph;
} // namespace scope

namespace pattern_constants
{
// any token valid
constexpr std::string_view wildcard    = "<*>";
// identifier token valid
constexpr std::string_view identifier  = "<a>";
// numeric token valid
constexpr std::string_view numeric     = "<0>";
// end token valid
constexpr std::string_view end         = "<!>";
// can be alternative token
constexpr std::string_view alternative = "<_>";

} // namespace pattern_constants

namespace compiler
{
enum class EPhase;
}

namespace token
{
enum class ETokenKind;
struct Arena;
struct FileArena;
struct Viewer;
struct Token;

} // namespace token

namespace ast
{
enum class ECapability;
enum class EPassMode;
enum class EBinOpType;
enum class EUnaryOpType;
enum class EExprPassMode;
enum class EVariableKind;
enum class ETransfertType;
enum class EPathSource;

struct ScriptArena;
struct Arena;
struct Node;

enum class ENodeKind : uint8_t;


struct COP_Entity;
} // namespace ast

namespace type
{
enum class EPrimitiveTypeKind;
struct Arena;
struct Decorator;
struct Type;

struct Decorator final {
  bool is_optional = false;
  bool is_volatile = false;
  bool is_constant = false;
};

} // namespace type

namespace inference
{
struct Arena;
}

namespace symbol
{
struct Arena;
struct Symbol;
} // namespace symbol

namespace script
{
enum class EFileSource;
struct ScriptInfo;
} // namespace script

namespace module
{
struct Module;
struct Graph;
enum class EVisibility;
} // namespace module

namespace common
{
struct FastRNG;
}

namespace pipeline
{
struct Pipeline;
}

namespace resolved
{
struct Arena;
struct Binding;
} // namespace resolved

namespace unresolved
{
struct Arena;
} // namespace unresolved

namespace metacode
{
struct ScriptGraph;
struct Preprocessor;
struct Metacode;
struct Root;
struct Expand;
struct If;
struct Instruction;
struct Metablock;
struct Binary_Cond;
struct Unary_Not_Cond;
using Env = std::vector<token::_id>;

} // namespace metacode

namespace parser
{
struct Parser_Base;
struct Parser_Expression;
struct Parser_Type;
struct Parser_Literal;
struct Parser_Declaration_Local;
struct Parser_Operator;
struct Parser_Memory;
struct Parser_Declaration;
struct Parser_Declaration_COP;
struct Parser_Statement;
struct Parser_Context;
} // namespace parser


// llvm convention (dot separation)
// hello -> hello
static std::string mangle_id(std::string_view id)
{
  return std::string(id);
}

// llvm convention (dot separation)
// { hello, world } -> hello.world
static std::string mangle_path(const std::vector<std::string_view>& id)
{
  std::string out;
  for (auto elem : id) out += std::string(elem) + ".";
  return out.substr(0, out.size() - 1);
}
