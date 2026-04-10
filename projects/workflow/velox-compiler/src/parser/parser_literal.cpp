#include "parser_literal.hpp"

#include <codecvt>
#include <memory>
#include <stdio.h>

#include <llvm/ADT/APFloat.h>
#include <string>
#include <vector>

#include "ast/ast_base.hpp"
#include "ast/ast_data.hpp"
#include "ast/ast_inferred_type_singleton.hpp"
#include "ast/ast_literal.hpp"
#include "ast/ast_expression.hpp"

#include "ast/ast_numeric_128_bits.hpp"
#include "ast/ast_type.hpp"
#include "lexer/token.hpp"
#include "parser_context.hpp"
#include "parser_expression.hpp"

std::unique_ptr<ast::ALiteral> parser::Parser_Literal::try_literal(bool is_silent_error)
{
  switch (ctx.tok_v.peek().type) {
  case TokTy::TRUE:
  case TokTy::FALSE:
    return literal_boolean();
    // literal
    // decimal
  case TokTy::L_DECIMAL:
  case TokTy::L_UDECIMAL:
    return literal_decimal();
    // literal
    // float
  case TokTy::L_F:
    return literal_floating_point();
    // literal
    // integer
  case TokTy::L_BIN:
  case TokTy::L_OCT:
  case TokTy::L_HEX:
  case TokTy::L_I:
  case TokTy::L_U:
    return literal_integral();
    // literal
    // character
  case TokTy::L_CUNE:
    return literal_cune();
    // literal
    // string
  case TokTy::L_TEXTUAL:
    return literal_textual();
    // literal
    // range
  case TokTy::RANGE:
  case TokTy::RANGE_INCLUSIVE:
    return literal_range(nullptr);
    // literal
    // collection
  case TokTy::OPEN_BRACE: return literal_table();
  case TokTy::OPEN_PAREN:
    if (ctx.tok_v.peek(2).type == TokTy::COMMA || ctx.tok_v.peek(2).type == TokTy::COLON) {
      return literal_tuple();
    }
  default: break;
  }

  if (!is_silent_error) {
    ctx.tok_v.add_error(80, "Expected literal value", "");
  }

  return nullptr;
}

std::unique_ptr<ast::literal::Boolean> parser::Parser_Literal::literal_boolean()
{
  auto literal = ctx.Create_Node<ast::literal::Boolean>(ctx.tok_v.peek());
  literal->val = ctx.tok_v.match(TokTy::TRUE);
  return literal;
}

std::unique_ptr<ast::literal::Decimal> parser::Parser_Literal::literal_decimal()
{
  auto literal = ctx.Create_Node<ast::literal::Decimal>(ctx.tok_v.peek());

  std::string decimal_val = ctx.tok_v.peek().val;
  size_t      decimal_pos = decimal_val.find('.');
  literal->scale          = decimal_pos == 0 ? 0 : decimal_val.size() - decimal_pos;
  std::string raw_val     = decimal_val;
  std::erase(raw_val, '.');

  ctx.tok_v.next(); // consume literal

  // post literal type like 99.999ud or 99.999d32
  if (ctx.tok_v.check_any(kDecimalTypeTokens)) literal->raw_type = TokTy_to_EPrimType(ctx.tok_v.next().type);

  // post decimal explicit scale
  if (ctx.tok_v.check(TokTy::L_I)) {
    literal->scale = std::stoi(ctx.tok_v.next().val);
  }
  if (ctx.tok_v.match(TokTy::COLON) && ctx.tok_v.check(TokTy::L_I)) {
    size_t bits_size = std::stoi(ctx.tok_v.next().val);

    if (bits_size == 32)
      literal->raw_type = EPrimType::i32;
    else if (bits_size == 64)
      literal->raw_type = EPrimType::i64;
    else if (bits_size == 128)
      literal->raw_type = EPrimType::i128;
    else
      ctx.tok_v.add_error_tok(220, ctx.tok_v.peek(-1), "Illegal decimal type size.",
                              "define a decimal type size like: `<decimal>:S`, S must be equals to 32, 64 or 128.");

    auto api = llvm::APInt(bits_size, raw_val, 10);

    literal->val = Int128(api);
  }

  return literal;
}

std::unique_ptr<ast::literal::Floating> parser::Parser_Literal::literal_floating_point()
{
  auto literal = ctx.Create_Node<ast::literal::Floating>(ctx.tok_v.peek());
  literal->val.string_to_f128(ctx.tok_v.peek().val);

  // post literal type like 9.99f32 9.99f64
  if (ctx.tok_v.match_any(kFloatingTypeTokens)) {
    literal->type = TokTy_to_EPrimType(ctx.tok_v.peek(-1).type);
  } else if (ctx.tok_v.match_val("f")) {
    literal->type = EPrimType::fSize;
  } else {
    ctx.tok_v.next();

    auto&       apf = *literal->val.val;
    const auto& sem = apf.getSemantics();

    llvm::APFloat maxF32(sem);
    llvm::APFloat maxF64(sem);

    llvm::APFloat f32max(std::numeric_limits<float>::max());
    llvm::APFloat f64max(std::numeric_limits<double>::max());

    bool losesInfo;

    maxF32.convertToFloat();

    maxF64.convertToDouble();

    // post literal type like 10f32 0f
    if (ctx.tok_v.match_any(kFloatingTypeTokens)) {
      literal->type = TokTy_to_EPrimType(ctx.tok_v.peek(-1).type);
    } else if (ctx.tok_v.match_val("f")) {
      literal->type = EPrimType::fSize;
    }
  }

  return literal;
}

std::unique_ptr<ast::literal::Integral> parser::Parser_Literal::literal_integral()
{
  static const char* hint =
      "define literal integral like:\n  - decimal: 1234\n  - bin: 0b10011010010\n  - oct: 0o2322\n  - hex: 0x4d2";

  auto literalTok = ctx.tok_v.next();
  auto literal    = ctx.Create_Node<ast::literal::Integral>(literalTok);

  auto& api = *literal->val.val;

  unsigned bitWidth = 64;

  try {
    switch (literalTok.type) {
    case TokTy::L_BIN: {
      api           = llvm::APInt(bitWidth, literalTok.val.substr(2), 2);
      literal->type = EPrimType::bSize;
      break;
    }
    case TokTy::L_OCT: {
      api           = llvm::APInt(bitWidth, literalTok.val.substr(2), 8);
      literal->type = EPrimType::bSize;
      break;
    }
    case TokTy::L_HEX: {
      api           = llvm::APInt(bitWidth, literalTok.val, 16);
      literal->type = EPrimType::bSize;
      break;
    }
    case TokTy::L_I: {
      api           = llvm::APInt(bitWidth, literalTok.val, 10);
      literal->type = EPrimType::iSize;
      break;
    }
    default: throw std::runtime_error("Token literal non supporté");
    }

    if (api.getBitWidth() > 64) {
      api           = llvm::APInt(128, literalTok.val,
                                  (literalTok.type == TokTy::L_BIN   ? 2
                                   : literalTok.type == TokTy::L_OCT ? 8
                                   : literalTok.type == TokTy::L_HEX ? 16
                                                                     : 10));
      literal->type = EPrimType::i128;
    }
  } catch (const std::invalid_argument&) {
    ctx.tok_v.add_error(82, "Impossible to parse literal integral", hint);
    throw std::runtime_error("Impossible to parse APInt literal");
  } catch (const std::out_of_range&) {
    ctx.tok_v.add_error(83, "Integral literal too big for 128 bits", hint);
  }

  // post literal type like 10i8 0u32
  if (ctx.tok_v.match_any(kIntegerTypeTokens)) {
    literal->type = TokTy_to_EPrimType(ctx.tok_v.peek(-1).type);
  } else if (ctx.tok_v.match_val("i")) {
    literal->type = EPrimType::iSize;
  } else if (ctx.tok_v.match_val("u")) {
    literal->type = EPrimType::uSize;
  } else if (ctx.tok_v.match_val("b")) {
    literal->type = EPrimType::bSize;
  }

  return literal;
}

std::unique_ptr<ast::literal::CUNE> parser::Parser_Literal::literal_cune()
{
  auto literal = ctx.Create_Node<ast::literal::CUNE>(ctx.tok_v.peek());
  literal->val = ctx.tok_v.next().val[0];
  return literal;
}

std::unique_ptr<ast::ALiteral> parser::Parser_Literal::literal_textual()
{
  auto ftext = ctx.Create_Node<ast::literal::Textual_Format>(ctx.tok_v.peek());

  bool first_elem = true;
  while (!ctx.tok_v.is_end()) {
    if (ctx.tok_v.check(TokTy::L_TEXTUAL)) {
      auto text = ctx.Create_Node<ast::literal::Text_Pure>(ctx.tok_v.peek());
      text->val = ctx.tok_v.next().val;

      if (ctx.tok_v.match(TokTy::T_STRING) || ctx.tok_v.match_val("s")) {
        text->inferred_type = ast::type::get_str_type();
        text->text_type     = EPrimType::str;
      } else if (ctx.tok_v.match(TokTy::T_C_STRING) || ctx.tok_v.match_val("c")) {
        text->inferred_type = ast::type::get_c_str_type();
        text->text_type     = EPrimType::c_str;
      } else if (ctx.tok_v.match(TokTy::T_CUNE) || ctx.tok_v.match_val("cu")) {
        text->inferred_type = ast::type::get_cune_type();
        text->text_type     = EPrimType::cune;
      } else if (ctx.tok_v.match(TokTy::T_RUNE) || ctx.tok_v.match_val("r")) {
        text->inferred_type = ast::type::get_rune_type();
        text->text_type     = EPrimType::rune;
      } else {
        text->inferred_type = ast::type::get_text_type();
        text->text_type     = EPrimType::text;
      }
      if (first_elem) ftext->inferred_type = text->inferred_type;
      first_elem = false;

      auto txt_elem = ast::literal::Textual_Element(std::move(text));

      ftext->values.push_back(std::move(txt_elem));
      continue;
    } else if (ctx.tok_v.match(TokTy::S_TEXTUAL_EXPR_START)) {
      auto lerp        = ctx.Create_Node<ast::literal::Text_Interpolation>(ctx.tok_v.peek(-1));
      lerp->expression = ctx.p_expr->parse_expression();

      // if format specifier detected
      if (ctx.tok_v.match(TokTy::COLON)) {
        lerp->spec = format_specifier();
      }
      ftext->values.push_back(std::move(lerp));

      ctx.tok_v.expect(84, TokTy::S_TEXTUAL_EXPR_END, "Expected end expression '}' in format string.",
                       "define format string like: `f\"you age is {now - birthday} years\"`.");
      continue;
    }
    break;
  }

  if (ftext->values.size() == 1) {
    if (auto ptr = dynamic_cast<ast::literal::Text_Pure*>(ftext->values[0].val)) {
      return std::unique_ptr<ast::literal::Text_Pure>(ptr);
    }
  }

  return ftext;
}

std::unique_ptr<ast::literal::Format_Specifier> parser::Parser_Literal::format_specifier()
{
  static const std::string hint =
      "define format specifier like:"
      "\n  - 1 [fill][align]   : fill character and alignment('<', '>', '=', '^', '~')"
      "\n  - 2 [sign]          : '+', '-', or 's' for space"
      "\n  - 3 [flags]         : '0', '#', 'z'"
      "\n  - 4 [width]         : minimum field width(digits)"
      "\n  - 5 [grouping]      : ',' or '_'"
      "\n  - 6 ['.' precision] : precision for floats or strings"
      "\n  - 7 [type]          : 'b', 'c', 'd', 'e', 'E', 'f', 'F', 'g', 'G', 'n', 'o', 's', 'x', 'X', '%'"
      "\n  - e.g. `{:*^10.2f}`, `{:+08d}`, `{:,_10d}`, `{:.5s}`";

  auto format = ctx.Create_Node<ast::literal::Format_Specifier>(ctx.tok_v.peek());

  // fill + align
  if (std::find(kFormatSpecAlign.begin(), kFormatSpecAlign.end(), ctx.tok_v.peek(1).type) != kFormatSpecAlign.end()) {
    auto tok1 = ctx.tok_v.peek(0);
    auto tok2 = ctx.tok_v.peek(1);

    format->fill = tok1.val[0];
    switch (tok2.type) {
    case TokTy::OPEN_BRACKETS: {
      format->align = ast::literal::Format_Specifier::EAlign::Left;
      break;
    }
    case TokTy::CLOSE_BRACKETS: {
      format->align = ast::literal::Format_Specifier::EAlign::Right;
      break;
    }
    case TokTy::OP_CIRCUMFLEX: {
      format->align = ast::literal::Format_Specifier::EAlign::Center;
      break;
    }
    case TokTy::TILDE: {
      format->align = ast::literal::Format_Specifier::EAlign::Justify;
      break;
    }
    default: break;
    }

    ctx.tok_v.next();
    ctx.tok_v.next();
  }

  // sign
  if (ctx.tok_v.match_any({TokTy::OP_PLUS, TokTy::OP_MINUS, TokTy::SPACE})) {
    switch (ctx.tok_v.peek(-1).type) {
    case TokTy::OP_PLUS: {
      format->sign = ast::literal::Format_Specifier::ESign::Pos;
      break;
    }
    case TokTy::OP_MINUS: {
      format->sign = ast::literal::Format_Specifier::ESign::Neg;
      break;
    }
    case TokTy::SPACE: {
      format->sign = ast::literal::Format_Specifier::ESign::Space;
      break;
    }
    default: break;
    }
  }

  // prefix numeric
  if (ctx.tok_v.match(TokTy::HASHTAG)) {
    ctx.tok_v.expect(85, TokTy::L_CUNE, "Expected integral prefix 'x', 'X', 'o' or 'b'.", hint);
    char prefix = ctx.tok_v.peek(-1).val[0];

    switch (prefix) {
    case 'x': format->prefix = ast::literal::Format_Specifier::EPrefix::Hex; break;
    case 'X': format->prefix = ast::literal::Format_Specifier::EPrefix::HEX; break;
    case 'b': format->prefix = ast::literal::Format_Specifier::EPrefix::Bin; break;
    case 'o': format->prefix = ast::literal::Format_Specifier::EPrefix::Oct; break;
    }
  }

  // fill with 0 + sign before fill
  if (ctx.tok_v.peek().val == "0") {
    format->zero_pad = true;
    ctx.tok_v.next();

    if (ctx.tok_v.match(TokTy::ASSIGN)) {
      format->signBeforeFill = true;
    }
  }

  // width from variable
  if (ctx.tok_v.match(TokTy::OPEN_BRACE)) {
    format->width = ctx.p_expr->parse_expression_term();
    ctx.tok_v.expect(86, TokTy::OPEN_BRACE, "Expected close variable width '}'.", hint);
  }
  // width from literal
  else if (ctx.tok_v.check(TokTy::L_U)) {
    auto width    = ctx.p_expr->parse_expression_term();
    format->width = std::move(width);
  }

  // grouping char
  if (ctx.tok_v.check_any({TokTy::COMMA, TokTy::UNDERSCORE, TokTy::TICK})) {
    format->grouping_char = ctx.tok_v.next().val[0];
  }

  // precision
  if (ctx.tok_v.match(TokTy::DOT)) {
    // precision from variable
    if (ctx.tok_v.match(TokTy::OPEN_BRACE)) {
      format->precision = ctx.p_expr->parse_expression_term();
      ctx.tok_v.expect(87, TokTy::OPEN_BRACE, "Expected close variable width '}'.", hint);
    }
    // precision from literal
    else if (ctx.tok_v.check(TokTy::L_U)) {
      auto width        = ctx.p_expr->parse_expression_term();
      format->precision = std::move(width);
    }
  }

  // display format
  if (ctx.tok_v.match_any({TokTy::L_CUNE, TokTy::PERCENTAGE})) {
    switch (ctx.tok_v.peek(-1).val[0]) {
    case 's': format->display_format = ast::literal::Format_Specifier::EDisplayFormat::String; break;
    case 'b': format->display_format = ast::literal::Format_Specifier::EDisplayFormat::Binary; break;
    case 'c': format->display_format = ast::literal::Format_Specifier::EDisplayFormat::Character; break;
    case 'd': format->display_format = ast::literal::Format_Specifier::EDisplayFormat::Decimal; break;
    case 'o': format->display_format = ast::literal::Format_Specifier::EDisplayFormat::Octal; break;
    case 'x': format->display_format = ast::literal::Format_Specifier::EDisplayFormat::Hex; break;
    case 'X': format->display_format = ast::literal::Format_Specifier::EDisplayFormat::HEX; break;
    case 'n': format->display_format = ast::literal::Format_Specifier::EDisplayFormat::Number; break;
    case 'e': format->display_format = ast::literal::Format_Specifier::EDisplayFormat::e; break;
    case 'E': format->display_format = ast::literal::Format_Specifier::EDisplayFormat::E; break;
    case 'f': format->display_format = ast::literal::Format_Specifier::EDisplayFormat::Fixed; break;
    case 'F': format->display_format = ast::literal::Format_Specifier::EDisplayFormat::FIXED; break;
    case 'g': format->display_format = ast::literal::Format_Specifier::EDisplayFormat::g; break;
    case 'G': format->display_format = ast::literal::Format_Specifier::EDisplayFormat::G; break;
    case '%': format->display_format = ast::literal::Format_Specifier::EDisplayFormat::Percentage; break;
    }
  }

  if (!ctx.tok_v.check(TokTy::S_TEXTUAL_EXPR_END)) {
    ctx.tok_v.add_error(88, "Unexpected token '" + ctx.tok_v.peek().val + "' in format specifier.", hint);
  }

  return format;
}

std::unique_ptr<ast::literal::Range> parser::Parser_Literal::literal_range(std::unique_ptr<ast::AExpression> start)
{
  static const std::string hint =
      "define range like:"
      "\n  - absolute range: `0..10` `0..=9`"
      "\n  - relative range: `a..b` `a..=b - 1`"
      "\n  - slice all: `..`"
      "\n  - slice from 0: `..end` `..=end`"
      "\n  - slice to max: `start..` `start..=`";

  auto range = ctx.Create_Node<ast::literal::Range>(ctx.tok_v.peek());

  range->start = std::move(start);

  auto range_tok =
      ctx.tok_v.expect_any(89, {TokTy::RANGE, TokTy::RANGE_INCLUSIVE}, "Expected range kind '..' or '..='", hint);

  range->endInclude = range_tok.type == TokTy::RANGE_INCLUSIVE;

  if (!ctx.tok_v.check_any(kEndArgsListokens)) {
    range->end = ctx.p_expr->parse_expression();
  }

  return range;
}

std::unique_ptr<ast::ALiteral> parser::Parser_Literal::literal_table()
{
  static const std::string hint =
      "define literal table like:"
      "\n  - table { 1, 2, 3, 4 }"
      "\n  - table population { 0..4 => @i + 1 }"
      "\n  - matrix { 1, 2, 3, 4 }*3"
      "\n  - matrix {{ 1, 2 },{ 3, 4 }}"
      "\n  - matrix population { [0..4] => @i + 1 }*3"
      "\n  - matrix population { [0..4, 0..4] => @i + 1 + @j }"
      "\n  - map table { a: 1, b: 2, c: 3 }"
      "\n  - map table population { [0..4] => text_number[@i] : @i }";

  ctx.tok_v.match(TokTy::OPEN_BRACE);

  if (ctx.tok_v.match(TokTy::CLOSE_BRACE)) return ctx.Create_Node<ast::literal::Table>(ctx.tok_v.peek(-1));
  ;

  std::vector<std::unique_ptr<ast::AExpression>> values;
  std::vector<std::unique_ptr<ast::AExpression>> map_values;

  while (!ctx.tok_v.is_end()) {
    values.push_back(ctx.p_expr->parse_expression());

    if (ctx.tok_v.match(TokTy::COLON)) {
      map_values.push_back(ctx.p_expr->parse_expression());
    }

    if (ctx.match_field_separator(TokTy::CLOSE_BRACE)) break;
  }

  // is a literal table population
  if (values.size() == 1) {
    if (dynamic_cast<ast::literal::Table_Population*>(values[0].get())) {
      auto pop_ptr = dynamic_cast<ast::literal::Table_Population*>(values[0].release());

      // is a map population
      if (pop_ptr->map_expression_value) {
        auto map = ctx.Create_Node<ast::literal::Map>(ctx.tok_v.peek());
        map->population.reset(pop_ptr);
        return map;
      } else {
        auto tbl = ctx.Create_Node<ast::literal::Table>(ctx.tok_v.peek());
        tbl->population.reset(pop_ptr);
        return tbl;
      }
    }
  }

  if (map_values.size() > 0) {
    auto map    = ctx.Create_Node<ast::literal::Map>(ctx.tok_v.peek());
    map->keys   = std::move(values);
    map->values = std::move(map_values);
    return map;
  }

  auto tbl    = ctx.Create_Node<ast::literal::Table>(ctx.tok_v.peek());
  tbl->values = std::move(values);

  return tbl;
}

std::unique_ptr<ast::literal::Table_Population> parser::Parser_Literal::literal_table_population()
{
  ctx.tok_v.match(TokTy::OPEN_SQUARE);

  auto pop = ctx.Create_Node<ast::literal::Table_Population>(ctx.tok_v.peek(-1));

  while (!ctx.tok_v.is_end()) {
    pop->ranges.push_back(ctx.p_expr->parse_expression());

    if (ctx.match_field_any_separator(TokTy::INJECT)) break;
  }

  pop->expression = ctx.p_expr->parse_expression();

  // mapping population
  if (ctx.tok_v.match(TokTy::COLON)) {
    pop->map_expression_value = ctx.p_expr->parse_expression_term();
  }

  return pop;
}

std::unique_ptr<ast::literal::Entity> parser::Parser_Literal::literal_entity(std::unique_ptr<ast::AIdentifier> id)
{
  auto lit_entity  = ctx.Create_Node<ast::literal::Entity>(ctx.tok_v.peek());
  lit_entity->name = std::move(id);

  ctx.tok_v.match(TokTy::OPEN_BRACE);
  if (ctx.tok_v.match(TokTy::CLOSE_BRACE)) return lit_entity;

  while (!ctx.tok_v.is_end()) {
    auto comp_name = ctx.p_expr->identifier(true);

    if (ctx.tok_v.check(TokTy::OPEN_BRACE)) {
      lit_entity->comp_args.push_back(literal_component(std::move(comp_name)));
    } else if (ctx.tok_v.match(TokTy::DOT)) {
      auto lit_comp = ctx.Create_Node<ast::literal::Structured_Data>(comp_name->_token);
      lit_comp->field_args.push_back(literal_field());

      lit_entity->comp_args.push_back(std::move(lit_comp));
    } else {
      ctx.tok_v.add_error_tok(91, comp_name->_token, "Unexpected literal reference",
                              "define literal components only in literal entity");
    }

    if (ctx.match_field_separator(TokTy::COMMA, TokTy::CLOSE_BRACE)) break;
  }

  return lit_entity;
}

std::unique_ptr<ast::literal::Structured_Data>
parser::Parser_Literal::literal_component(std::unique_ptr<ast::AIdentifier> id)
{
  static const std::string hint =
      "define literal component like:"
      "\n  - no fields `name{.}`"
      "\n  - normal `name{ .field1= val1, .field2= val2 }`"
      "\n  - generic `name<gen_args>{ .field1= val1, .field2= val2 }`";

  auto comp  = ctx.Create_Node<ast::literal::Structured_Data>(ctx.tok_v.peek());
  comp->name = std::move(id);

  ctx.tok_v.match(TokTy::OPEN_BRACE);
  if (!ctx.tok_v.match(TokTy::CLOSE_BRACE)) return comp;

  while (!ctx.tok_v.is_end()) {
    if (ctx.tok_v.match(TokTy::CLOSE_BRACE)) break;

    comp->field_args.push_back(literal_field());

    if (ctx.match_field_separator(TokTy::COMMA, TokTy::CLOSE_BRACE)) break;
  }

  return comp;
}

std::unique_ptr<ast::expression::Call_Argument> parser::Parser_Literal::literal_field()
{
  static const std::string hint =
      "define literal filed like:"
      "\n  - scoped field `name{ .field1= val1, .field2= val2 }`"
      "\n  - direct field `name.field1= val1`";

  ctx.tok_v.match(TokTy::DOT);

  auto field_arg  = ctx.Create_Node<ast::expression::Call_Argument>(ctx.tok_v.peek());
  field_arg->name = ctx.parse_name("", hint);

  ctx.tok_v.expect(94, TokTy::ASSIGN, "Expected field assignation '=' after field name", hint);

  field_arg->expression = ctx.p_expr->parse_expression();

  return field_arg;
}

std::unique_ptr<ast::literal::Tuple> parser::Parser_Literal::literal_tuple()
{
  static const std::string hint = "define named tuple instance like `(filed1: value, ...)`.";

  auto tuple        = ctx.Create_Node<ast::literal::Tuple>(ctx.tok_v.peek());
  bool isNamedTuple = ctx.tok_v.check(TokTy::COLON); // (name: type, ...) or (type, ...)

  while (!ctx.tok_v.is_end()) {
    if (isNamedTuple) {
      tuple->name_fields.push_back(ctx.parse_name("", hint));

      ctx.tok_v.expect(96, TokTy::ASSIGN,
                       "Expected field value assignation ':' after filed name in named tuple instance.", hint);

      tuple->values.push_back(ctx.p_expr->parse_expression());
    } else {
      tuple->values.push_back(ctx.p_expr->parse_expression());
    }

    if (ctx.tok_v.match(TokTy::COMMA)) continue;

    break;
  }

  return tuple;
}
