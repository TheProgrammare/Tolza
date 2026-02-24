
#include "Parser_Type.hpp"

#include "AST/AST_Base.hpp"
#include "AST/AST_Data.hpp"
#include "AST/AST_Headers.hpp"
#include "Parser_Headers.hpp"
#include <sys/types.h>

// const, optional, volatile
std::tuple<bool, bool, bool> PAR::Parser_Type::get_type_annotation()
{
  bool type_isConst;
  bool type_isOptional;
  bool type_isVolatile;

  for (size_t i = 0; i < 3; i++) {
    if (ctx.tok_v.match(TokTy::DOLLAR)) type_isConst = true;
    if (ctx.tok_v.match(TokTy::INTERROGATIVE)) type_isOptional = true;
    if (ctx.tok_v.match(TokTy::NOT)) type_isVolatile = true;
  }

  return {type_isConst, type_isOptional, type_isVolatile};
}

std::unique_ptr<AST::Type::Table> PAR::Parser_Type::table(bool isConst, bool isOptional, bool isVolatile)
{
  auto table             = ctx.Create_Node<AST::Type::Table>(ctx.tok_v.peek(-1));
  table->type_isConst    = isConst;
  table->type_isOptional = isOptional;
  table->type_isVolatile = isVolatile;

  ctx.tok_v.match(TokTy::OPEN_SQUARE);

  table->inner = ctx.p_type->parse_type();

  // static table (sized)
  if (ctx.tok_v.match(TokTy::ARROW)) {
    table->sizeSymbol = ctx.p_expr->parse_expression();
  }

  if (ctx.tok_v.match(TokTy::CLOSE_SQUARE)) {
    auto [isConstP, isOptionalP, isVolatileP] = get_type_annotation();

    table->type_isConst    = isConst ? true : isConstP;
    table->type_isOptional = isOptional ? true : isOptionalP;
    table->type_isVolatile = isVolatile ? true : isVolatileP;
  }
  return table;
}

std::unique_ptr<AST::Type::Ptr> PAR::Parser_Type::pointer(bool isConst, bool isOptional, bool isVolatile)
{
  auto ptr          = ctx.Create_Node<AST::Type::Ptr>(ctx.tok_v.peek());
  ptr->pointer_type = TokTy_to_EPtrType(ctx.tok_v.peek().type);
  ctx.tok_v.next(); // consume ptr

  ptr->type_isConst    = isConst;
  ptr->type_isOptional = isOptional;
  ptr->type_isVolatile = isVolatile;

  auto [isConstP, isOptionalP, isVolatileP] = get_type_annotation();

  ptr->type_isConst    = isConst ? true : isConstP;
  ptr->type_isOptional = isOptional ? true : isOptionalP;
  ptr->type_isVolatile = isVolatile ? true : isVolatileP;
  ctx.tok_v.expect<124>(TokTy::TICK, "Expected tick ''' after pointer specification.",
                        "define pointer like:"
                        "  - `ptr'T` `std::unique_ptr'T` `std::shared_ptr'T` `std::weak_ptr'T`");
  ptr->inner = ctx.p_type->parse_type();
  return ptr;
}

std::unique_ptr<AST::Type::Primitive> PAR::Parser_Type::primitive(bool isConst, bool isOptional, bool isVolatile)
{
  auto tok             = ctx.tok_v.next();
  auto pri             = ctx.Create_Node<AST::Type::Primitive>(tok);
  pri->type_isConst    = isConst;
  pri->type_isOptional = isOptional;
  pri->type_isVolatile = isVolatile;
  pri->type            = TokTy_to_EPrimType(tok.type);

  auto [isConstP, isOptionalP, isVolatileP] = get_type_annotation();

  pri->type_isConst    = isConst ? true : isConstP;
  pri->type_isOptional = isOptional ? true : isOptionalP;
  pri->type_isVolatile = isVolatile ? true : isVolatileP;
  return pri;
}

std::unique_ptr<AST::Expr_ID_Generic> PAR::Parser_Type::id_type(bool isConst, bool isOptional, bool isVolatile)
{
  std::unique_ptr<AST::Expr_ID_Generic> result;

  auto base_tok = ctx.tok_v.peek();
  auto id       = ctx.p_expr->identifier();

  ctx.m_sym->add_external_symbol(*id.get(), Extern_Item::Kind::Type);

  if (ctx.tok_v.match_any({TokTy::OPEN_BRACKETS, TokTy::TURBO_FISH})) {
    result       = ctx.p_expr->identifier_typed();
    result->name = std::move(id);

  } else {
    result       = ctx.Create_Node<AST::Expr_ID_Generic>(base_tok);
    result->name = std::move(id);
  }

  result->type_isConst    = isConst;
  result->type_isOptional = isOptional;
  result->type_isVolatile = isVolatile;

  auto [isConstP, isOptionalP, isVolatileP] = get_type_annotation();

  result->type_isConst    = isConst ? true : isConstP;
  result->type_isOptional = isOptional ? true : isOptionalP;
  result->type_isVolatile = isVolatile ? true : isVolatileP;
  return result;
}

std::unique_ptr<AST::Type::Tuple> PAR::Parser_Type::tuple(bool isConst, bool isOptional, bool isVolatile)
{
  auto tu             = explicit_tuple();
  tu->type_isConst    = isConst;
  tu->type_isOptional = isOptional;
  tu->type_isVolatile = isVolatile;

  auto [isConstP, isOptionalP, isVolatileP] = get_type_annotation();

  tu->type_isConst    = isConst ? true : isConstP;
  tu->type_isOptional = isOptional ? true : isOptionalP;
  tu->type_isVolatile = isVolatile ? true : isVolatileP;
  return tu;
}

std::unique_ptr<AST::Type::Function_Proto> PAR::Parser_Type::function_proto(bool isConst, bool isOptional,
                                                                            bool isVolatile)
{
  auto proto             = explicit_function_proto();
  proto->type_isConst    = isConst;
  proto->type_isOptional = isOptional;
  proto->type_isVolatile = isVolatile;

  auto [isConstP, isOptionalP, isVolatileP] = get_type_annotation();

  proto->type_isConst    = isConst ? true : isConstP;
  proto->type_isOptional = isOptional ? true : isOptionalP;
  proto->type_isVolatile = isVolatile ? true : isVolatileP;
  return std::unique_ptr<AST::Type::Function_Proto>(proto.get());
}

std::unique_ptr<AST::AType> PAR::Parser_Type::parse_type()
{
  auto [isConst, isOptional, isVolatile] = get_type_annotation();

  switch (ctx.tok_v.peek().type) {
  case TokTy::OPEN_SQUARE: return table(isConst, isOptional, isVolatile);
  case TokTy::PTR:
  case TokTy::UPTR:
  case TokTy::SPTR:
  case TokTy::WPTR:        return pointer(isConst, isOptional, isVolatile);
  case TokTy::OPEN_PAREN:  return tuple(isConst, isOptional, isVolatile);
  case TokTy::FUNCTION:    return function_proto(isConst, isOptional, isVolatile);
  default:                 break;
  }

  if (ctx.tok_v.check_any(kPrimitiveTypeTokens)) return primitive(isConst, isOptional, isVolatile);

  if (ctx.tok_v.check(TokTy::IDENTIFIER)) return id_type(isConst, isOptional, isVolatile);

  ctx.tok_v.add_error<126>("Unexpected type definition '" + ctx.tok_v.peek().val + "'.",
                           "define type like:"
                           "  - primitives `i32`, `f32`, `bool`, `char`, `addr`, ..."
                           "  - user type `identifier`, `_id32`, `T`, ..."
                           "  - generic args `T<i32, U>`, `T<gen_args>`, `T<gen_args>::U`, ..."
                           "  - from module/namespace `A::B::T`, `A::B<U, V>`, `A::B<U, V>::T`, ...");
  return nullptr;
};

std::unique_ptr<AST::Type::Tuple> PAR::Parser_Type::explicit_tuple()
{
  static const std::string hint = "define named tuple like `(filed1: i32, ...)`.";

  auto tuple        = ctx.Create_Node<AST::Type::Tuple>(ctx.tok_v.peek());
  bool endByParen   = ctx.tok_v.match(TokTy::OPEN_PAREN);
  bool isNamedTuple = ctx.tok_v.peek(1).type == TokTy::COLON; // (name: type, ...) or (type, ...)

  while (!ctx.tok_v.is_end()) {
    if (isNamedTuple) {
      tuple->name_fields.push_back(ctx.parse_name("", hint));
      ctx.tok_v.expect<128>(TokTy::COLON, "Expected a name type separator ':' after an filed name keyword.", hint);
      tuple->types.push_back(ctx.p_type->parse_type());
    } else {
      tuple->types.push_back(ctx.p_type->parse_type());
    }

    if (!endByParen && ctx.match_field_any_separator(TokTy::COMMA, kEndArgsListokens)) {
      ctx.tok_v.rewind(ctx.tok_v.position() - 1);
      break;
    }
    if (ctx.match_field_separator(TokTy::COMMA, ETokenType::CLOSE_PAREN)) break;
  }

  return tuple;
}

std::unique_ptr<AST::Type::Get_Expr_Type> PAR::Parser_Type::expr_get_expr_type()
{
  auto node = ctx.Create_Node<AST::Type::Get_Expr_Type>(ctx.tok_v.peek(-1));
  ctx.tok_v.expect<129>(TokTy::OPEN_PAREN, "Expected start arg '('.",
                        "define get type at compilation time like: `comptime::type(var)`");
  node->target = ctx.p_expr->parse_expression();
  ctx.tok_v.expect<130>(TokTy::OPEN_PAREN, "Expected end arg ')'.",
                        "define get type at compilation time like: `comptime::type(var)`");
  return node;
}

std::shared_ptr<AST::Type::Function_Proto> PAR::Parser_Type::explicit_function_proto(bool isLam)
{
  static const std::string hint =
      "define function like:"
      "\n  - `fn myName() { ... }`"
      "\n  - with return `fn myName() -> (i32, ...) { ... }`";

  ctx.tok_v.match_any({TokTy::FUNCTION, TokTy::LAMBDA});

  auto type = ctx.Create_Node<AST::Type::Function_Proto>(ctx.tok_v.peek());

  // check
  // parameters
  ctx.tok_v.expect<131>(TokTy::OPEN_PAREN, "Expected start parameter defintion '(' after function declaration.", hint);
  type->parameters = parameters();
  if (!type->parameters.empty() && type->parameters.back()->isVariadic) {
    type->isVariadic    = true;
    type->variadic_type = type->parameters.back()->type;
  }

  // check
  // return
  if (ctx.tok_v.match(TokTy::ARROW)) {
    type->returnType = ctx.p_type->explicit_tuple();
  }

  return type;
}

std::vector<std::shared_ptr<AST::Declaration::Local::Parameter>> PAR::Parser_Type::parameters()
{
  static const std::string hint =
      "define parameter like:"
      "\n  Rule: <pass_mode> <name>: <type> [= <default_value>]"
      "\n  - with a specific pass mode `[mut/copy/clone/move/addr] myName: i32`."
      "\n  - Note: copy pass mode can have a default value like `copy myName: i32 = 0`."
      "\n  - variadic mode `[mut/copy/clone/move/addr] myName: T...`";

  static const std::string hint_passmode =
      "Parameters default by pass mode:\n  - Allowed default: `copy`, `clone`, `ref`\n  - Prohibied default: `mut`, "
      "`move`";

  if (ctx.tok_v.match(TokTy::CLOSE_PAREN)) return {};

  std::vector<std::shared_ptr<AST::Declaration::Local::Parameter>> params;

  while (!ctx.tok_v.is_end()) {
    auto param      = ctx.Create_Decl<AST::Declaration::Local::Parameter>(ctx.tok_v.peek());
    param->passMode = TokTy_to_EPassMode(ctx.tok_v.next().type);
    if (param->passMode == EPassMode::NONE)
      ctx.tok_v.add_error<132>("Expected parameter pass mode before the parameter name.", hint);

    param->name = ctx.parse_name("", hint);
    // check pointer parameter type
    ctx.tok_v.expect<133>(TokTy::COLON, "Expected type definition ':' after parameter name.", hint);
    param->type = ctx.p_type->parse_type();

    param->isVariadic = ctx.tok_v.match(TokTy::VARIADIC);

    ctx.m_sym->add_decl(param);

    // check parameter default value
    if (ctx.tok_v.match(TokTy::ASSIGN)) {
      if (!EPassMode_Can_Default(param->passMode))
        ctx.tok_v.add_error_tok<134>(
            ctx.tok_v.peek(-1), "Unexpected defaut value for pass mode '" + EPassMode_to_str(param->passMode) + "'.",
            hint_passmode);
      param->defaultValue = ctx.p_expr->parse_expression();
    }

    params.push_back(std::move(param));

    if (ctx.match_field_separator(TokTy::COMMA, TokTy::CLOSE_PAREN)) break;
  }

  return params;
}
