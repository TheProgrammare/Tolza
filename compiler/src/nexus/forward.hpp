#pragma once

#include "ids.hpp"

#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

using ErrorCode = short;
enum class EVisibility : uint8_t { Lexical_Scope, File_Scope, Cross_File_Scope };
enum class ECallContract : uint8_t { Static, Assert, Result, Panic };

struct StringHash {
  using is_transparent = void;

  size_t operator()(std::string_view s) const noexcept
  {
    return std::hash<std::string_view>{}(s);
  }

  size_t operator()(const std::string& s) const noexcept
  {
    return (*this)(std::string_view{s});
  }

  size_t operator()(const char* s) const noexcept
  {
    return (*this)(s);
  }
};

template <typename T>
using StringMap = std::unordered_map<std::string, T, StringHash, std::equal_to<>>;


enum class EPathAnchor : uint8_t {
  src,            // user scripts
  vendor_lib,     // 3rd party scripts
  stdlib,         // standard library
  pkg_lib,        // package library
  binding,        // binding library
  relative_self,  // relative current module
  relative_super, // relative parent module
  relative_root,  // relative script root module
};

namespace evaluated
{
struct Arena;
}

namespace common::compiler
{

// tolza compiler invariant violation : Internal Compiler Error
void DEBUG_TOLZA_ICE(std::string_view msg);
enum class FPass : uint16_t;
struct Manifest;
struct Profile;

} // namespace common::compiler


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
enum class EPhase : uint8_t;
}

namespace token
{

enum class ETokenKind : uint8_t;

struct Arena;
struct Viewer;
struct Token;

} // namespace token

namespace ast
{
enum class EInvocationKind : uint8_t;
enum class ECapability : uint8_t;
enum class EPassMode : uint8_t;
enum class EOp_Bin : uint8_t;
enum class EOp_Unary : uint8_t;
enum class EOp_Subscript : uint8_t;
enum class EOp_Other : uint8_t;
enum class EExprPassMode : uint8_t;
enum class EVariableKind : uint8_t;
enum class ETransfertType : uint8_t;

struct Arena;
struct NodeHeader;

enum class ENodeKind : uint8_t;


struct SFM_Form;
} // namespace ast

namespace type
{

enum class ETextType : uint8_t;
enum class EPrimitiveTypeKind : uint8_t;
enum class ETypeKind : uint8_t;
struct Arena;
struct Qualifier;
struct TypeHeader;


struct Prototype_Param final {
  ast::EPassMode passmode;
  ID             type;
  ast::ID        nodeid;
  bool           is_restrict = false;

  auto operator<=>(const Prototype_Param&) const = default;
};


struct Qualifier final {
  bool is_optional = false;
  bool is_volatile = false;
  bool is_constant = false;
  bool is_opaque   = false;

  [[nodiscard]] bool is_pure() const noexcept
  {
    return !is_optional && !is_volatile && !is_constant && !is_opaque;
  }
};

} // namespace type

namespace extension
{
struct Arena;
}

namespace inference
{
struct Arena;
}

namespace definition
{

struct Arena;
struct Symbol;
} // namespace definition

namespace cu
{
enum class EFileSource : uint8_t;
struct CU;
} // namespace cu

namespace module
{
struct Module;
struct Graph;
} // namespace module

namespace common
{
struct FastRNG;
}

namespace common::env
{
enum class ECallConvention : uint8_t;
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
struct Graph;
struct Preprocessor;
struct Metacode;
struct Root;
struct Expand;
struct If;
struct Instruction;
struct Metablock;
struct Binary_Cond;
struct Unary_Not_Cond;
using Env = std::vector<token::ID>;

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
struct Parser_Declaration_SFM;
struct Parser_Statement;
struct Parser_Context;
} // namespace parser


namespace semantic
{
enum class EBuiltin_Member : uint8_t {
  NONE,
  _data,
  _len,
  _capa,
};

struct Arena;
struct Metadata;
} // namespace semantic


// llvm convention (dot separation)
// hello -> hello
static std::string mangle_id(std::string_view id)
{
  return std::string(id);
}

// llvm convention (dot separation)
// { hello, world } -> hello.world
static std::string mangle_path(const std::vector<std::string_view>& ids)
{
  std::string out;
  out.reserve(ids.size() * 12);
  for (auto elem : ids) std::format_to(std::back_inserter(out), "{}.", elem);
  return out.substr(0, out.size() - 1);
}
