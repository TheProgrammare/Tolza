#pragma once

#include <common/enum_lite.hpp>
#include <cstdint>
#include <string_view>

using ErrorCode = short;


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
class ID;
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
class ID;
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
enum class ECallContract : uint8_t;
enum class EVisibility : uint8_t;
enum class EPathAnchor : uint8_t;

class ID;
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

class ID;
struct Arena;
struct Qualifier;
struct TypeHeader;


struct Prototype_Param;


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
class ID;
struct Arena;
struct Symbol;
} // namespace definition

namespace cu
{
enum class EFileSource : uint8_t;

class ID;
struct CU;
} // namespace cu

namespace module
{
class ID;
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
class ID;
struct Graph;
struct Preprocessor;
struct MetacodeHeader;
struct Root;
struct Expand;
struct If;
struct Instruction;
struct Metablock;
struct Binary_Cond;
struct Unary_Not_Cond;

} // namespace metacode

namespace parser
{
struct Parser_Base;
struct Parser_Expression;
struct Parser_Type;
struct Parser_Literal;
struct Parser_Declaration_Local;
struct Parser_Operator;
struct Parser_Declaration;
struct Parser_Declaration_SFM;
struct Parser_Statement;
struct Parser_Context;
} // namespace parser


namespace semantic
{
DEFINE_ENUM(EBuiltin_Member, uint8_t, //
            _data, 1,                 //
            _len, 2,                  //
            _capa, 3,                 //

)

class ID;
struct Arena;
struct Metadata;
} // namespace semantic
