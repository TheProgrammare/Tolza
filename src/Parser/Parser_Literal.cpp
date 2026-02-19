#include "Parser_Literal.hpp"

#include <codecvt>
#include <llvm/ADT/APFloat.h>
#include <memory>
#include <stdio.h>

#include "AST/AST_Base.hpp"
#include "AST/AST_Headers.hpp"
#include "AST/AST_Literal.hpp"
#include "AST/AST_Expression.hpp"
#include "Parser_Headers.hpp"

std::optional<std::unique_ptr<AST::ALiteral>> PAR::Parser_Literal::try_literal(bool is_silent_error)
{
  switch (ctx.tok_v.peek().type) {
  case TokTy::TRUE:
  case TokTy::FALSE:
    return literal_boolean();
    // literal decimal
  case TokTy::L_DECIMAL:
  case TokTy::L_UDECIMAL:
    return literal_decimal();
    // literal float
  case TokTy::L_F:
    return literal_floating_point();
    // literal integer
  case TokTy::L_BIN:
  case TokTy::L_OCT:
  case TokTy::L_HEX:
  case TokTy::L_I:
  case TokTy::L_U:
    return literal_integral();
    // literal character
  case TokTy::L_ASCII:
    return literal_ascii();
    // literal string
  case TokTy::L_TEXTUAL:
    return literal_textual();
    // literal range
  case TokTy::RANGE:
  case TokTy::RANGE_INCLUSIVE:
    return literal_range(nullptr);
    // literal collection
  case TokTy::OPEN_BRACE: return literal_table();
  case TokTy::OPEN_PAREN:
    if (ctx.tok_v.peek(2).type == TokTy::COMMA || ctx.tok_v.peek(2).type == TokTy::COLON) {
      return literal_tuple();
    }
  default: break;
  }

  if (!is_silent_error) {
    ctx.tok_v.add_error<80>("Expected literal value", "");
  }

  return std::nullopt;
}

std::unique_ptr<AST::Literal::Boolean> PAR::Parser_Literal::literal_boolean()
{
  auto literal = ctx.Create_Node<AST::Literal::Boolean>(ctx.tok_v.peek());
  literal->val = ctx.tok_v.match(TokTy::TRUE);
  return literal;
}

std::unique_ptr<AST::Literal::Decimal> PAR::Parser_Literal::literal_decimal()
{
  auto literal = ctx.Create_Node<AST::Literal::Decimal>(ctx.tok_v.peek());

  size_t      decimal_pos = ctx.tok_v.peek().val.find('.');
  std::string before_comma;
  std::string after_comma;

  if (decimal_pos != std::string::npos) {
    // Partie before comma
    before_comma = ctx.tok_v.peek().val.substr(0, decimal_pos);
    // Partie after comma (no point)
    after_comma  = ctx.tok_v.peek().val.substr(decimal_pos + 1);
  } else
    ctx.tok_v.add_error<81>("Expected an point '.' in lietral decimal.",
                            "define literal decimal like:\n  - `0000.00d`\n  - `520.15d`\n  - "
                            "`10.544ud`\n  - `10.25deci`\n  - `20.54udeci`");

  literal->integral_num = before_comma.length();
  literal->decimal_num  = after_comma.length();
  literal->is_unsigned  = literal->_token.type == TokTy::L_UDECIMAL;

  ctx.tok_v.next(); // consume literal

  ETokenType deciKind;

  // post literal type like 99.999udeci or 99.999deci
  if (ctx.tok_v.check_any(kDecimalTypeTokens))
    deciKind = ctx.tok_v.next().type;
  else
    deciKind = ctx.tok_v.peek().type == TokTy::L_DECIMAL ? TokTy::T_DECIMAL : TokTy::T_UDECIMAL;

  return literal;
}

std::unique_ptr<AST::Literal::Floating> PAR::Parser_Literal::literal_floating_point()
{
  auto literal = ctx.Create_Node<AST::Literal::Floating>(ctx.tok_v.peek());
  literal->val.string_to_f128(ctx.tok_v.peek().val);

  ETokenType floatKind;

  // post literal type like 9.99f32 9.99f64
  if (ctx.tok_v.check_any(kFloatingTypeTokens)) {
    floatKind = ctx.tok_v.next().type;
  } else {
    ctx.tok_v.next();

    auto       &apf = literal->val.val;
    const auto &sem = apf.getSemantics();

    llvm::APFloat maxF32(sem);
    llvm::APFloat maxF64(sem);

    // Initialisation depuis des constantes IEEE natives
    llvm::APFloat f32max(std::numeric_limits<float>::max());
    llvm::APFloat f64max(std::numeric_limits<double>::max());

    bool losesInfo;

    // Conversion vers les semantics de apf (probablement f128)
    maxF32.convertToFloat();

    maxF64.convertToDouble();

    // Comparaisons sûres
    if (!apf.isFinite())
      floatKind = TokTy::T_F128;
    else if (apf.compare(maxF32) != llvm::APFloat::cmpGreaterThan)
      floatKind = TokTy::T_F32;
    else if (apf.compare(maxF64) != llvm::APFloat::cmpGreaterThan)
      floatKind = TokTy::T_F64;
    else
      floatKind = TokTy::T_F128;
  }

  literal->type = TokTy_to_EPrimType(floatKind);

  return literal;
}

std::unique_ptr<AST::Literal::Integral> PAR::Parser_Literal::literal_integral()
{
  auto literalTok = ctx.tok_v.next();
  auto literal    = ctx.Create_Node<AST::Literal::Integral>(literalTok);

  auto &api = literal->val.val;

  // Par défaut, on essaye 64 bits
  unsigned bitWidth = 64;

  try {
    switch (literalTok.type) {
    case TokTy::L_BIN:
      api           = llvm::APInt(bitWidth, literalTok.val.substr(2), 2);
      literal->type = EPrimType::b64;
      break;

    case TokTy::L_OCT:
      api           = llvm::APInt(bitWidth, literalTok.val.substr(2), 8);
      literal->type = EPrimType::b64;
      break;

    case TokTy::L_HEX:
      api           = llvm::APInt(bitWidth, literalTok.val, 16);
      literal->type = EPrimType::b64;
      break;

    case TokTy::L_I:
      api           = llvm::APInt(bitWidth, literalTok.val, 10);
      literal->type = EPrimType::b64;
      break;

    default: throw std::runtime_error("Token literal non supporté");
    }

    // Si la valeur dépasse 64 bits, on passe à 128 bits
    if (api.getBitWidth() > 64) {
      api           = llvm::APInt(128, literalTok.val,
                                  (literalTok.type == TokTy::L_BIN   ? 2
                                   : literalTok.type == TokTy::L_OCT ? 8
                                   : literalTok.type == TokTy::L_HEX ? 16
                                                                     : 10));
      literal->type = EPrimType::i128;
    }
  } catch (const std::invalid_argument &) {
    ctx.tok_v.add_error<82>("Impossible to parse literal integral",
                            "define literal integral like:\n  - decimal: 1234\n  - bin: "
                            "0b10011010010\n  - oct: 0o2322\n  - hex: 0x4d2");
    throw std::runtime_error("Impossible to parse APInt literal");
  } catch (const std::out_of_range &) {
    ctx.tok_v.add_error<83>("Integral literal too big for 128 bits",
                            "define literal integral like:\n  - decimal: 1234\n  - bin: "
                            "0b10011010010\n  - oct: 0o2322\n  - hex: 0x4d2");
  }

  // post literal type like 10i8 0u32
  if (ctx.tok_v.check_any(kIntegerTypeTokens)) {
    literal->type = TokTy_to_EPrimType(ctx.tok_v.peek().type);
    ctx.tok_v.next(); // consume type
  }

  return literal;
}

std::unique_ptr<AST::Literal::ASCII> PAR::Parser_Literal::literal_ascii()
{
  auto literal = ctx.Create_Node<AST::Literal::ASCII>(ctx.tok_v.peek());
  literal->val = ctx.tok_v.next().val[0];
  return literal;
}

std::unique_ptr<AST::Literal::Textual_Format> PAR::Parser_Literal::literal_textual()
{
  auto ftext = ctx.Create_Node<AST::Literal::Textual_Format>(ctx.tok_v.peek());

  while (!ctx.tok_v.is_end()) {
    if (ctx.tok_v.check(TokTy::L_TEXTUAL)) {
      auto           text = ctx.Create_Node<AST::Literal::Text>(ctx.tok_v.peek());
      std::u32string val =
          std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>{}.from_bytes(ctx.tok_v.next().val);
      text->val = val;
      ftext->values.push_back(std::move(text));
      continue;
    } else if (ctx.tok_v.match(TokTy::S_TEXTUAL_EXPR_START)) {
      auto lerp        = ctx.Create_Node<AST::Literal::Text_Lerp>(ctx.tok_v.peek(-1));
      lerp->expression = ctx.p_expr->parse_expression();

      // if format specifier detected
      if (ctx.tok_v.match(TokTy::COLON)) {
        lerp->spec = format_specifier();
      }
      ftext->values.push_back(std::move(lerp));

      ctx.tok_v.expect<84>(TokTy::S_TEXTUAL_EXPR_END, "Expected end expression '}' in format string.",
                           "define format string like: `f\"you age is {now - birthday} years\"`.");
      continue;
    }
    break;
  }

  return ftext;
}

std::unique_ptr<AST::Literal::Format_Specifier> PAR::Parser_Literal::format_specifier()
{
  static const std::string hint =
      "define format specifier like:"
      "\n  - 1 [fill][align]   : fill character and alignment('<', '>', '=', '^', '~')"
      "\n  - 2 [sign]          : '+', '-', or 's' for space"
      "\n  - 3 [flags]         : '0', '#', 'z'"
      "\n  - 4 [width]         : minimum field width(digits)"
      "\n  - 5 [grouping]      : ',' or '_'"
      "\n  - 6 ['.' precision] : precision for floats or strings"
      "\n  - 7 [type]          : 'b', 'c', 'd', 'e', 'E', 'f', 'F', 'g', 'G', 'n', 'o', 's', 'x', "
      "'X', '%'"
      "\n  - e.g. `{:*^10.2f}`, `{:+08d}`, `{:,_10d}`, `{:.5s}`";

  auto format = ctx.Create_Node<AST::Literal::Format_Specifier>(ctx.tok_v.peek());

  // fill + align
  if (std::find(kFormatSpecAlign.begin(), kFormatSpecAlign.end(), ctx.tok_v.peek(1).type) != kFormatSpecAlign.end()) {
    auto tok1 = ctx.tok_v.peek(0);
    auto tok2 = ctx.tok_v.peek(1);

    format->fill = tok1.val[0];
    switch (tok2.type) {
    case TokTy::OPEN_BRACKETS: {
      format->align = AST::Literal::Format_Specifier::EAlign::Left;
      break;
    }
    case TokTy::CLOSE_BRACKETS: {
      format->align = AST::Literal::Format_Specifier::EAlign::Right;
      break;
    }
    case TokTy::OP_CIRCUMFLEX: {
      format->align = AST::Literal::Format_Specifier::EAlign::Center;
      break;
    }
    case TokTy::TILDE: {
      format->align = AST::Literal::Format_Specifier::EAlign::Justify;
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
      format->sign = AST::Literal::Format_Specifier::ESign::Pos;
      break;
    }
    case TokTy::OP_MINUS: {
      format->sign = AST::Literal::Format_Specifier::ESign::Neg;
      break;
    }
    case TokTy::SPACE: {
      format->sign = AST::Literal::Format_Specifier::ESign::Space;
      break;
    }
    default: break;
    }
  }

  // prefix numeric
  if (ctx.tok_v.match(TokTy::HASHTAG)) {
    ctx.tok_v.expect<85>(TokTy::L_ASCII, "Expected integral prefix 'x', 'X', 'o' or 'b'.", hint);
    char prefix = ctx.tok_v.peek(-1).val[0];

    switch (prefix) {
    case 'x': format->prefix = AST::Literal::Format_Specifier::EPrefix::Hex; break;
    case 'X': format->prefix = AST::Literal::Format_Specifier::EPrefix::HEX; break;
    case 'b': format->prefix = AST::Literal::Format_Specifier::EPrefix::Bin; break;
    case 'o': format->prefix = AST::Literal::Format_Specifier::EPrefix::Oct; break;
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
    ctx.tok_v.expect<86>(TokTy::OPEN_BRACE, "Expected close variable width '}'.", hint);
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
      ctx.tok_v.expect<87>(TokTy::OPEN_BRACE, "Expected close variable width '}'.", hint);
    }
    // precision from literal
    else if (ctx.tok_v.check(TokTy::L_U)) {
      auto width        = ctx.p_expr->parse_expression_term();
      format->precision = std::move(width);
    }
  }

  // display format
  if (ctx.tok_v.match_any({TokTy::L_ASCII, TokTy::PERCENTAGE})) {
    switch (ctx.tok_v.peek(-1).val[0]) {
    case 's': format->display_format = AST::Literal::Format_Specifier::EDisplayFormat::String; break;
    case 'b': format->display_format = AST::Literal::Format_Specifier::EDisplayFormat::Binary; break;
    case 'c': format->display_format = AST::Literal::Format_Specifier::EDisplayFormat::Character; break;
    case 'd': format->display_format = AST::Literal::Format_Specifier::EDisplayFormat::Decimal; break;
    case 'o': format->display_format = AST::Literal::Format_Specifier::EDisplayFormat::Octal; break;
    case 'x': format->display_format = AST::Literal::Format_Specifier::EDisplayFormat::Hex; break;
    case 'X': format->display_format = AST::Literal::Format_Specifier::EDisplayFormat::HEX; break;
    case 'n': format->display_format = AST::Literal::Format_Specifier::EDisplayFormat::Number; break;
    case 'e': format->display_format = AST::Literal::Format_Specifier::EDisplayFormat::e; break;
    case 'E': format->display_format = AST::Literal::Format_Specifier::EDisplayFormat::E; break;
    case 'f': format->display_format = AST::Literal::Format_Specifier::EDisplayFormat::Fixed; break;
    case 'F': format->display_format = AST::Literal::Format_Specifier::EDisplayFormat::FIXED; break;
    case 'g': format->display_format = AST::Literal::Format_Specifier::EDisplayFormat::g; break;
    case 'G': format->display_format = AST::Literal::Format_Specifier::EDisplayFormat::G; break;
    case '%': format->display_format = AST::Literal::Format_Specifier::EDisplayFormat::Percentage; break;
    }
  }

  if (!ctx.tok_v.check(TokTy::S_TEXTUAL_EXPR_END)) {
    ctx.tok_v.add_error<88>("Unexpected token '" + ctx.tok_v.peek().val + "' in format specifier.", hint);
  }

  return format;
}

std::unique_ptr<AST::Literal::Range> PAR::Parser_Literal::literal_range(std::unique_ptr<AST::AExpression> start)
{
  static const std::string hint =
      "define range like: "
      "\n  - absolute range: `0..10` `0..=9`"
      "\n  - relative range: `a..b` `a..=b - 1`"
      "\n  - slice all: `..`"
      "\n  - slice from 0: `..end` `..=end`"
      "\n  - slice to max: `start..` `start..=`";

  auto range = ctx.Create_Node<AST::Literal::Range>(ctx.tok_v.peek());

  range->start = std::move(start);

  auto range_tok =
      ctx.tok_v.expect_any<89>({TokTy::RANGE, TokTy::RANGE_INCLUSIVE}, "Expected range kind '..' or '..='", hint);

  range->endInclude = range_tok.type == TokTy::RANGE_INCLUSIVE;

  range->start      = ctx.Create_Node<AST::Literal::Integral>(ctx.tok_v.peek());
  range->endInclude = ctx.tok_v.match(TokTy::RANGE_INCLUSIVE);

  if (!ctx.tok_v.check_any(kEndArgsListokens)) {
    range->end = ctx.p_expr->parse_expression();
  }

  return range;
}

std::unique_ptr<AST::ALiteral> PAR::Parser_Literal::literal_table()
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

  if (ctx.tok_v.match(TokTy::CLOSE_BRACE)) return ctx.Create_Node<AST::Literal::Table>(ctx.tok_v.peek(-1));
  ;

  std::vector<std::unique_ptr<AST::AExpression>> values;
  std::vector<std::unique_ptr<AST::AExpression>> map_values;

  while (!ctx.tok_v.is_end()) {
    values.push_back(ctx.p_expr->parse_expression());

    if (ctx.tok_v.match(TokTy::COLON)) {
      map_values.push_back(ctx.p_expr->parse_expression());
    }

    if (ctx.match_field_separator(TokTy::CLOSE_BRACE)) break;
  }

  // is a literal table population
  if (values.size() == 1) {
    if (dynamic_cast<AST::Literal::Table_Population *>(values[0].get())) {
      auto pop_ptr = dynamic_cast<AST::Literal::Table_Population *>(values[0].release());

      // is a map population
      if (pop_ptr->map_expression_value) {
        auto map = ctx.Create_Node<AST::Literal::Map>(ctx.tok_v.peek());
        map->population.reset(pop_ptr);
        return map;
      } else {
        auto tbl = ctx.Create_Node<AST::Literal::Table>(ctx.tok_v.peek());
        tbl->population.reset(pop_ptr);
        return tbl;
      }
    }
  }

  if (map_values.size() > 0) {
    auto map    = ctx.Create_Node<AST::Literal::Map>(ctx.tok_v.peek());
    map->keys   = std::move(values);
    map->values = std::move(map_values);
    return map;
  }

  auto tbl    = ctx.Create_Node<AST::Literal::Table>(ctx.tok_v.peek());
  tbl->values = std::move(values);

  return tbl;
}

std::unique_ptr<AST::Literal::Table_Population> PAR::Parser_Literal::literal_table_population()
{
  ctx.tok_v.match(TokTy::OPEN_SQUARE);

  auto pop = ctx.Create_Node<AST::Literal::Table_Population>(ctx.tok_v.peek(-1));

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

std::unique_ptr<AST::Literal::Entity> PAR::Parser_Literal::literal_entity(std::unique_ptr<AST::AIdentifier> id)
{
  auto lit_entity  = ctx.Create_Node<AST::Literal::Entity>(ctx.tok_v.peek());
  lit_entity->name = std::move(id);

  ctx.tok_v.match(TokTy::OPEN_BRACE);
  if (ctx.tok_v.match(TokTy::CLOSE_BRACE)) return lit_entity;

  while (!ctx.tok_v.is_end()) {
    auto expr = ctx.p_expr->parse_expression();

    if (dynamic_cast<AST::Literal::Component *>(expr.get())) {
      lit_entity->comp_args.push_back(
          std::unique_ptr<AST::Literal::Component>(static_cast<AST::Literal::Component *>(expr.release())));

    } else if (auto comp_member = dynamic_cast<AST::Expression::Member_Access *>(expr.get())) {
      ctx.tok_v.expect<90>(TokTy::ASSIGN, "Expected component field initialisation '='.",
                           "define literal component member like: `CPosition.x= 10, CPosition.y = 15`");

      auto lit_comp = ctx.Create_Node<AST::Literal::Component>(expr->_token);
      lit_comp->name =
          std::move(std::unique_ptr<AST::AIdentifier>(static_cast<AST::AIdentifier *>(comp_member->left.release())));
    } else {
      ctx.tok_v.add_error_tok<91>(expr->_token, "Unexpected literal reference",
                                  "define literal components only in literal entity");
    }

    if (ctx.match_field_separator(TokTy::COMMA, TokTy::CLOSE_BRACE)) break;
  }

  return lit_entity;
}

std::unique_ptr<AST::Literal::Component> PAR::Parser_Literal::literal_component(std::unique_ptr<AST::AIdentifier> id)
{
  static const std::string hint =
      "define literal component like:"
      "\n  - no fields `name{.}`"
      "\n  - normal `name{ .field1: val1, .field2: val2 }`"
      "\n  - generic `name<gen_args>{ .field1: val1, .field2: val2 }`";

  auto comp  = ctx.Create_Node<AST::Literal::Component>(ctx.tok_v.peek());
  comp->name = std::move(id);

  ctx.tok_v.match(TokTy::OPEN_BRACE);
  if (!ctx.tok_v.match(TokTy::CLOSE_BRACE)) return comp;

  while (!ctx.tok_v.is_end()) {
    if (ctx.tok_v.match(TokTy::CLOSE_BRACE)) break;

    ctx.tok_v.expect<92>(TokTy::DOT, "Expected contextual field access '.' in literal component", hint);

    auto field_arg  = ctx.Create_Node<AST::Expression::Call_Argument>(ctx.tok_v.peek());
    field_arg->name = ctx.parse_name("", hint);
    ctx.tok_v.expect<94>(TokTy::COLON, "Expected field assignation ':' after field name", hint);

    field_arg->expression = ctx.p_expr->parse_expression();
    comp->field_args.push_back(std::move(field_arg));

    if (ctx.match_field_separator(TokTy::COMMA, TokTy::CLOSE_BRACE)) break;
  }

  return comp;
}

std::unique_ptr<AST::Literal::Tuple> PAR::Parser_Literal::literal_tuple()
{
  static const std::string hint = "define named tuple instance like `(filed1: value, ...)`.";

  auto tuple        = ctx.Create_Node<AST::Literal::Tuple>(ctx.tok_v.peek());
  bool isNamedTuple = ctx.tok_v.check(TokTy::COLON); // (name: type, ...) or (type, ...)

  while (!ctx.tok_v.is_end()) {
    if (isNamedTuple) {
      tuple->name_fields.push_back(ctx.parse_name("", hint));

      ctx.tok_v.expect<96>(TokTy::ASSIGN,
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
