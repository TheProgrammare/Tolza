#include "parser_type.hpp"

#include <memory>
#include <sys/types.h>

#include "ast/ast_base.hpp"
#include "ast/ast_type.hpp"
#include "ast/ast_declaration_local.hpp"
#include "ast/ast_inferred_type_singleton.hpp"

#include "parser_context.hpp"
#include "parser_expression.hpp"

// const, optional, volatile
std::tuple<bool, bool, bool> parser::Parser_Type::get_type_annotation()
{
  bool type_isConst    = false;
  bool type_isOptional = false;
  bool type_isVolatile = false;

  for (size_t i = 0; i < 3; i++) {
    if (ctx.tok_v.match(TokTy::DOLLAR)) type_isConst = true;
    if (ctx.tok_v.match(TokTy::INTERROGATIVE)) type_isOptional = true;
    if (ctx.tok_v.match(TokTy::NOT)) type_isVolatile = true;
  }

  return {type_isConst, type_isOptional, type_isVolatile};
}

std::shared_ptr<ast::type::Table> parser::Parser_Type::table(bool p_is_const, bool p_is_optional, bool p_is_volatile)
{
  auto table              = ctx.Create_Type<ast::type::Table>(ctx.tok_v.peek(-1));
  table->type_is_constant = p_is_const;
  table->type_is_optional = p_is_optional;
  table->type_is_volatile = p_is_volatile;

  ctx.tok_v.match(TokTy::OPEN_SQUARE);

  table->inner = ctx.p_type->parse_type();

  // static table (sized)
  if (ctx.tok_v.match(TokTy::ARROW)) {
    table->size_sym = ctx.p_expr->parse_expression();
  }

  if (ctx.tok_v.match(TokTy::CLOSE_SQUARE)) {
    auto [a_is_const, a_is_optional, a_is_volatile] = get_type_annotation();

    table->type_is_constant = p_is_const ? true : a_is_const;
    table->type_is_optional = p_is_optional ? true : a_is_optional;
    table->type_is_volatile = p_is_volatile ? true : a_is_volatile;
  }
  return table;
}

std::shared_ptr<ast::type::Ptr> parser::Parser_Type::pointer(bool p_is_const, bool p_is_optional, bool p_is_volatile)
{
  auto ptr          = ctx.Create_Type<ast::type::Ptr>(ctx.tok_v.peek());
  ptr->pointer_type = TokTy_to_EPtrType(ctx.tok_v.peek().type);
  ctx.tok_v.next(); // consume ptr

  ptr->type_is_constant = p_is_const;
  ptr->type_is_optional = p_is_optional;
  ptr->type_is_volatile = p_is_volatile;

  auto [a_is_const, a_is_optional, a_is_volatile] = get_type_annotation();

  ptr->type_is_constant = p_is_const ? true : a_is_const;
  ptr->type_is_optional = p_is_optional ? true : a_is_optional;
  ptr->type_is_volatile = p_is_volatile ? true : a_is_volatile;
  ctx.tok_v.expect(124, TokTy::TICK, "Expected tick ''' after pointer specification.",
                   "define pointer like:"
                   "  - `ptr'T` `std::unique_ptr'T` `std::shared_ptr'T` `std::weak_ptr'T`");
  ptr->inner = ctx.p_type->parse_type();
  return ptr;
}

std::shared_ptr<ast::type::Primitive> parser::Parser_Type::primitive(bool p_is_const, bool p_is_optional,
                                                                     bool p_is_volatile)
{
  auto tok              = ctx.tok_v.next();
  auto pri              = ctx.Create_Type<ast::type::Primitive>(tok);
  pri->type_is_constant = p_is_const;
  pri->type_is_optional = p_is_optional;
  pri->type_is_volatile = p_is_volatile;
  pri->type             = TokTy_to_EPrimType(tok.type);

  auto [a_is_const, a_is_optional, a_is_volatile] = get_type_annotation();

  pri->type_is_constant = p_is_const ? true : a_is_const;
  pri->type_is_optional = p_is_optional ? true : a_is_optional;
  pri->type_is_volatile = p_is_volatile ? true : a_is_volatile;
  return pri;
}

std::shared_ptr<ast::Expr_ID_Type> parser::Parser_Type::id_type(bool p_is_const, bool p_is_optional, bool p_is_volatile)
{
  std::shared_ptr<ast::Expr_ID_Type> result;

  auto base_tok = ctx.tok_v.peek();
  auto id       = ctx.p_expr->identifier();

  if (ctx.tok_v.match_any({TokTy::OPEN_BRACKETS, TokTy::TURBO_FISH}))
    result = ctx.p_expr->identifier_typed();
  else
    result = ctx.Create_Type<ast::Expr_ID_Type>(base_tok);

  result->name = id.release();

  result->type_is_constant = p_is_const;
  result->type_is_optional = p_is_optional;
  result->type_is_volatile = p_is_volatile;

  auto [a_is_const, a_is_optional, a_is_volatile] = get_type_annotation();

  result->type_is_constant = p_is_const ? true : a_is_const;
  result->type_is_optional = p_is_optional ? true : a_is_optional;
  result->type_is_volatile = p_is_volatile ? true : a_is_volatile;
  return result;
}

std::shared_ptr<ast::type::Tuple> parser::Parser_Type::tuple(bool p_is_const, bool p_is_optional, bool p_is_volatile)
{
  auto tu              = explicit_tuple();
  tu->type_is_constant = p_is_const;
  tu->type_is_optional = p_is_optional;
  tu->type_is_volatile = p_is_volatile;

  auto [a_is_const, a_is_optional, a_is_volatile] = get_type_annotation();

  tu->type_is_constant = p_is_const ? true : a_is_const;
  tu->type_is_optional = p_is_optional ? true : a_is_optional;
  tu->type_is_volatile = p_is_volatile ? true : a_is_volatile;
  return tu;
}

std::shared_ptr<ast::type::Function_Proto> parser::Parser_Type::function_proto(bool p_is_const, bool p_is_optional,
                                                                               bool p_is_volatile)
{
  auto proto              = explicit_function_proto();
  proto->type_is_constant = p_is_const;
  proto->type_is_optional = p_is_optional;
  proto->type_is_volatile = p_is_volatile;

  auto [a_is_const, a_is_optional, a_is_volatile] = get_type_annotation();

  proto->type_is_constant = p_is_const ? true : a_is_const;
  proto->type_is_optional = p_is_optional ? true : a_is_optional;
  proto->type_is_volatile = p_is_volatile ? true : a_is_volatile;
  return std::unique_ptr<ast::type::Function_Proto>(proto.get());
}

std::shared_ptr<ast::AType> parser::Parser_Type::parse_type()
{
  auto [isConst, p_is_optional, p_is_volatile] = get_type_annotation();

  switch (ctx.tok_v.peek().type) {
  case TokTy::OPEN_SQUARE: return table(isConst, p_is_optional, p_is_volatile);
  case TokTy::PTR:
  case TokTy::UPTR:
  case TokTy::SPTR:
  case TokTy::WPTR:        return pointer(isConst, p_is_optional, p_is_volatile);
  case TokTy::OPEN_PAREN:  return tuple(isConst, p_is_optional, p_is_volatile);
  case TokTy::FUNCTION:    return function_proto(isConst, p_is_optional, p_is_volatile);
  default:                 break;
  }

  if (ctx.tok_v.check_any(k_type_primitive)) return primitive(isConst, p_is_optional, p_is_volatile);

  if (ctx.tok_v.check(TokTy::IDENTIFIER)) return id_type(isConst, p_is_optional, p_is_volatile);

  ctx.tok_v.add_error(126, "Unexpected type definition '" + ctx.tok_v.peek().val + "'.",
                      "define type like:"
                      "  - primitives `i32`, `f32`, `bool`, `char`, `addr`, ..."
                      "  - user type `identifier`, `_id32`, `T`, ..."
                      "  - generic args `T<i32, U>`, `T<gen_args>`, `T<gen_args>::U`, ..."
                      "  - from module/namespace `A::B::T`, `A::B<U, V>`, `A::B<U, V>::T`, ...");
  return nullptr;
};

std::shared_ptr<ast::type::Tuple> parser::Parser_Type::explicit_tuple()
{
  static const std::string hint = "define named tuple like `(filed1: i32, ...)`.";

  auto tuple        = ctx.Create_Type<ast::type::Tuple>(ctx.tok_v.peek());
  bool endByParen   = ctx.tok_v.match(TokTy::OPEN_PAREN);
  bool isNamedTuple = ctx.tok_v.peek(1).type == TokTy::COLON; // (name: type, ...) or (type, ...)

  while (!ctx.tok_v.is_end()) {
    if (isNamedTuple) {
      tuple->name_fields.push_back(ctx.parse_name("", hint));
      ctx.tok_v.expect(128, TokTy::COLON, "Expected a name type separator ':' after an filed name keyword.", hint);
      tuple->types.push_back(ctx.p_type->parse_type());
    } else {
      tuple->types.push_back(ctx.p_type->parse_type());
    }

    if (!endByParen && ctx.match_field_any_separator(TokTy::COMMA, k_args_ending)) {
      ctx.tok_v.rewind(ctx.tok_v.position() - 1);
      break;
    }
    if (ctx.match_field_separator(TokTy::COMMA, ETokenType::CLOSE_PAREN)) break;
  }

  return tuple;
}

std::shared_ptr<ast::type::Get_Expr_Type> parser::Parser_Type::expr_get_expr_type()
{
  auto node = ctx.Create_Type<ast::type::Get_Expr_Type>(ctx.tok_v.peek(-1));
  ctx.tok_v.expect(129, TokTy::OPEN_PAREN, "Expected start arg '('.",
                   "define get type at compilation time like: `comptime::type(var)`");
  node->target = ctx.p_expr->parse_expression();
  ctx.tok_v.expect(130, TokTy::OPEN_PAREN, "Expected end arg ')'.",
                   "define get type at compilation time like: `comptime::type(var)`");
  return node;
}

std::shared_ptr<ast::type::Function_Proto> parser::Parser_Type::explicit_function_proto(bool p_is_lam)
{
  static const std::string hint =
      "define function like:"
      "\n  - `fn myName() { ... }`"
      "\n  - with return `fn myName() -> (i32, ...) { ... }`";

  ctx.tok_v.match_any({TokTy::FUNCTION, TokTy::LAMBDA});

  auto proto = ctx.Create_Type<ast::type::Function_Proto>(ctx.tok_v.peek());

  // check
  // parameters
  ctx.tok_v.expect(131, TokTy::OPEN_PAREN, "Expected start parameter defintion '(' after function declaration.", hint);
  proto->parameters = parameters();
  if (!proto->parameters.empty() && proto->parameters.back()->is_variadic) {
    proto->is_variadic = true;
    proto->parameters.pop_back();
  }

  // check
  // return
  if (ctx.tok_v.match(TokTy::ARROW)) {
    proto->is_explicit_return_type = true;
    if (ctx.tok_v.check(TokTy::OPEN_PAREN)) {
      proto->return_ty = ctx.p_type->explicit_tuple();
    } else {
      proto->return_ty = ctx.p_type->parse_type();
    }
  } else {
    proto->is_explicit_return_type = false;
    proto->return_ty               = ast::type::get_void_type();
  }

  return proto;
}

std::vector<std::shared_ptr<ast::declaration::local::Parameter>> parser::Parser_Type::parameters()
{
  static const std::string hint =
      "define parameter like:"
      "\n  Rule: <pass_mode> <name>: <type> [= <default_value>]"
      "\n  - with a specific pass mode `[mut/copy/clone/move/addr] myName: i32`."
      "\n  - Note: copy pass mode can have a default value like `copy myName: i32 = 0`."
      "\n  - variadic (only in externs for interop): `...`";

  static const std::string hint_passmode =
      "Parameters default by pass mode:\n  - Allowed default: `copy`, `clone`, `ref`\n  - Prohibied default: `mut`, "
      "`move`";

  if (ctx.tok_v.match(TokTy::CLOSE_PAREN)) return {};

  std::vector<std::shared_ptr<ast::declaration::local::Parameter>> params;

  while (!ctx.tok_v.is_end()) {
    auto param = ctx.Create_Decl<ast::declaration::local::Parameter>(ctx.tok_v.peek());

    if (ctx.tok_v.match(TokTy::VARIADIC)) {
      param->is_variadic = true;

      ctx.tok_v.expect(154, TokTy::CLOSE_PAREN,
                       "Unexpected token after a variadic mark, the variadic must be the last parameter.", hint);
      params.push_back(std::move(param));
      break;
    }

    param->passmode = TokTy_to_EPassMode(ctx.tok_v.next().type);
    if (param->passmode == EPassMode::NONE)
      ctx.tok_v.add_error(132, "Expected parameter pass mode before the parameter name.", hint);

    param->declaration_name = ctx.parse_name("", hint);
    // check pointer parameter type
    ctx.tok_v.expect(133, TokTy::COLON, "Expected type definition ':' after parameter name.", hint);
    param->type = ctx.p_type->parse_type();

    ctx.current_module->add_item(param);

    // check parameter default value
    if (ctx.tok_v.match(TokTy::ASSIGN)) {
      if (!EPassMode_Can_Default(param->passmode))
        ctx.tok_v.add_error_tok(134, ctx.tok_v.peek(-1),
                                "Unexpected defaut value for pass mode '" + EPassMode_to_str(param->passmode) + "'.",
                                hint_passmode);
      param->defaultValue = ctx.p_expr->parse_expression();
    }

    params.push_back(std::move(param));

    if (ctx.match_field_separator(TokTy::COMMA, TokTy::CLOSE_PAREN)) break;
  }

  return params;
}
