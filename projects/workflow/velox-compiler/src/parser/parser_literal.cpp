#include "parser_literal.hpp"

#include <llvm/ADT/APInt.h>

#include <llvm/ADT/APFloat.h>
#include <string>
#include <string_view>
#include <vector>


#include "ast/ast_literal.hpp"
#include "ast/ast_expression.hpp"

#include "ast/ast_numeric_128_bits.hpp"
#include "compiler/compilation_unit.hpp"
#include "nexus/forward.hpp"
#include "nexus/lexer/token.hpp"
#include "nexus/type/type.hpp"
#include "parser/parser_base.hpp"
#include "parser_context.hpp"
#include "parser_expression.hpp"

ast::ID parser::Parser_Literal::try_literal(bool p_is_silent_error)
{
  switch (p.peek().kind) {
  case token::ETokenKind::TRUE:
  case token::ETokenKind::FALSE:
    return literal_boolean();
    // literal
    // float/decimal
  case token::ETokenKind::L_D:
    return literal_decimal();
    // literal
    // integer
  case token::ETokenKind::L_BIN:
  case token::ETokenKind::L_OCT:
  case token::ETokenKind::L_HEX:
  case token::ETokenKind::L_I:
    return literal_numeric();
    // literal
    // character
  case token::ETokenKind::L_CUNE:
    return literal_cune();
    // literal
    // string
  case token::ETokenKind::L_TEXTUAL:
    return literal_textual();
    // literal
    // range
  case token::ETokenKind::RANGE:
  case token::ETokenKind::RANGE_INCLUSIVE:
    return literal_range();
    // literal
    // collection
  case token::ETokenKind::L_CURLY: return literal_table();
  case token::ETokenKind::L_PAREN:
    if (p.check_at(2, token::ETokenKind::COMMA) || p.check_at(2, token::ETokenKind::COLON)) {
      return literal_tuple();
    }
  default: break;
  }

  if (!p_is_silent_error) {
    p.add_error(80, "Expected literal value", "");
    THROW_BAD_NODE;
  }

  return BAD_NODE_ID;
}

ast::ID parser::Parser_Literal::literal_boolean()
{
  auto& literal = p.add_get_node<ast::Literal_Boolean>(p.peek().tokid);
  literal.val   = p.match(token::ETokenKind::TRUE);
  return literal.nodeid;
}

ast::ID parser::Parser_Literal::literal_numeric()
{
  auto& tok = p.next();
  auto  val = p.tok_to_str(tok.tokid);

  if (p.check_any(token::k_type_integral)) return literal_integral(tok);
  if (p.check_any(token::k_type_fixed_point) || p.check_val("d") || p.check_val("ud")) return literal_fixed_point(val);
  if (p.check_any(token::k_type_floating_point) || p.check_val("f")) return literal_floating_point(val);

  return literal_integral(tok);
}


ast::ID parser::Parser_Literal::literal_decimal()
{
  auto val = p.tok_to_str(p.next().tokid);

  if (p.check_any(token::k_type_fixed_point)) return literal_fixed_point(val);
  if (p.check_any(token::k_type_floating_point)) return literal_floating_point(val);
  if (p.check_val("f")) return literal_floating_point(val);
  if (p.check_val("d")) return literal_fixed_point(val);
  if (p.check_val("ud")) return literal_fixed_point(val);

  return literal_floating_point(val);
}

ast::ID parser::Parser_Literal::literal_fixed_point(std::string_view p_val)
{
  auto& literal = p.add_get_node<ast::Literal_Fixed_Point>(p.peek().tokid);

  size_t scale_pos    = p_val.find('.');
  literal.scale       = scale_pos == 0 ? 0 : p_val.size() - scale_pos;
  std::string raw_val = std::string(p_val);
  std::erase(raw_val, '.');

  (void)p.next(); // consume literal

  // post literal type like 99.999ud or 99.999d32
  if (p.match_any(token::k_type_fixed_point))
    literal.raw_type = type::ETokenKind_to_EPrimitiveTypeKind(p.peek(-1).kind);
  else if (p.match_val("ud"))
    literal.raw_type = type::EPrimitiveTypeKind::_udsize;
  else if (p.match_val("d"))
    literal.raw_type = type::EPrimitiveTypeKind::_dsize;

  // post decimal explicit scale
  if (p.match(token::ETokenKind::COLON) && p.check(token::ETokenKind::L_I)) {
    literal.scale = std::stoi(std::string(p.tok_to_str(p.next().tokid)));
  }

  auto api = llvm::APInt(128, raw_val, 10);

  literal.val = Int128(api);

  return literal.nodeid;
}

ast::ID parser::Parser_Literal::literal_floating_point(std::string_view p_val)
{
  auto& literal = p.add_get_node<ast::Literal_Floating_Point>(p.peek().tokid);
  literal.val.string_to_f128(p_val);

  // post literal type like 9.99f32 9.99f64
  if (p.match_any(token::k_type_floating_point)) {
    literal.type = type::ETokenKind_to_EPrimitiveTypeKind(p.peek(-1).kind);
  }
  (void)p.match_val("f");

  return literal.nodeid;
}

ast::ID parser::Parser_Literal::literal_integral(const token::Token& p_tok)
{
  constexpr std::string_view hint =
      "define literal integral like:\n  - decimal: 1234\n  - bin: 0b10011010010\n  - oct: 0o2322\n  - hex: 0x4d2";

  auto& literal = p.add_get_node<ast::Literal_Integral>(p_tok.tokid);
  auto& api     = *literal.val.val;

  auto tok_val = p.tok_to_str(p_tok.tokid);

  unsigned bitWidth = 64;

  try {
    switch (p_tok.kind) {
    case token::ETokenKind::L_BIN: {
      api = llvm::APInt(bitWidth, tok_val.substr(2), 2);
      break;
    }
    case token::ETokenKind::L_OCT: {
      api = llvm::APInt(bitWidth, tok_val.substr(2), 8);
      break;
    }
    case token::ETokenKind::L_HEX: {
      api = llvm::APInt(bitWidth, tok_val, 16);
      break;
    }
    case token::ETokenKind::L_I: {
      api = llvm::APInt(bitWidth, tok_val, 10);
      break;
    }
    default: throw std::runtime_error("Litearal token not supported");
    }

    if (api.getBitWidth() > 64) {
      api          = llvm::APInt(128, tok_val,
                                 (p_tok.kind == token::ETokenKind::L_BIN   ? 2
                                  : p_tok.kind == token::ETokenKind::L_OCT ? 8
                                  : p_tok.kind == token::ETokenKind::L_HEX ? 16
                                                                           : 10));
      literal.type = type::EPrimitiveTypeKind::_s128;
    }
  } catch (const std::invalid_argument&) {
    p.add_error(82, "Impossible to parse literal integral", hint);
  } catch (const std::out_of_range&) {
    p.add_error(83, "Integral literal too big for 128 bits", hint);
  }

  // post literal type like 10i8 0u32
  if (p.match_any(token::k_type_integral)) {
    literal.type = type::ETokenKind_to_EPrimitiveTypeKind(p.peek(-1).kind);
  } else if (p.match_val("i")) {
    literal.type = type::EPrimitiveTypeKind::_ssize;
  } else if (p.match_val("u")) {
    literal.type = type::EPrimitiveTypeKind::_usize;
  } else if (p.match_val("b")) {
    literal.type = type::EPrimitiveTypeKind::_bsize;
  }

  return literal.nodeid;
}

ast::ID parser::Parser_Literal::literal_cune()
{
  auto& literal = p.add_get_node<ast::Literal_Cune>(p.peek().tokid);
  literal.val   = p.tok_to_str(p.next().tokid)[0];
  return literal.nodeid;
}

ast::ID parser::Parser_Literal::literal_textual()
{

  std::vector<ast::ID> values;

  while (!p.is_end()) {

    if (p.check(token::ETokenKind::L_TEXTUAL)) {
      auto& text = p.add_get_node<ast::Literal_Text_Pure>(p.peek().tokid);
      text.val   = p.tok_to_str(p.next().tokid);

      // type inference
      if (p.match(token::ETokenKind::T_TEXT) || p.match_val("t")) {
        text.text_type = type::ETextType::_text;
        if (p.match(token::ETokenKind::T_STRING) || p.match_val("s")) {
          text.text_type = type::ETextType::_str;
        }
      } else if (p.match(token::ETokenKind::T_C_STRING) || p.match_val("c")) {
        text.text_type = type::ETextType::_c_str;
      } else if (p.match(token::ETokenKind::T_CUNE) || p.match_val("cu")) {
        text.text_type = type::ETextType::_cune;
      } else if (p.match(token::ETokenKind::T_RUNE) || p.match_val("r")) {
        text.text_type = type::ETextType::_rune;
      }

      values.emplace_back(text.nodeid);
      continue;
    }

    // interpolation
    if (p.match(token::ETokenKind::S_INTERPOLATION_START)) {
      auto& lerp      = p.add_get_node<ast::Literal_Text_Interpolation>(p.peek().tokid);
      lerp.expression = p.p_expr->parse_expression();

      if (p.match(token::ETokenKind::COLON)) {
        lerp.specifier = format_specifier();
      }

      values.emplace_back(lerp.nodeid);

      (void)p.expect(84, token::ETokenKind::S_INTERPOLATION_END, "Expected end interpolation '}' in string formatted.",
                     "define format string like: `f\"you age is {now - birthday} years\"`.");

      continue;
    }

    break;
  }

  if (values.size() == 1) {
    if (const auto* node = values[0].as<ast::Literal_Text_Pure>()) {
      return node->nodeid;
    }
  }

  auto& f_text  = p.add_get_node<ast::Literal_Textual_Format>(p.peek().tokid);
  f_text.values = values;
  return f_text.nodeid;
}

ast::ID parser::Parser_Literal::format_specifier()
{
  constexpr std::string_view hint =
      "define format specifier like:"
      "\n  - 1 [fill][align]   : fill character and alignment('<', '>', '=', '^', '~')"
      "\n  - 2 [sign]          : '+', '-', or 's' for space"
      "\n  - 3 [flags]         : '0', '#', 'z'"
      "\n  - 4 [width]         : minimum field width(digits)"
      "\n  - 5 [grouping]      : ',' or '_'"
      "\n  - 6 ['.' precision] : precision for floats or strings"
      "\n  - 7 [type]          : 'b', 'c', 'd', 'e', 'E', 'f', 'F', 'g', 'G', 'n', 'o', 's', 'x', 'X', '%'"
      "\n  - e.g. `{:*^10.2f}`, `{:+08d}`, `{:,_10d}`, `{:.5s}`";

  auto& format = p.add_get_node<ast::Literal_Format_Specifier>(p.peek().tokid);

  // fill + align
  if (std::find(token::k_op_format.begin(), token::k_op_format.end(), p.peek(1).kind) != token::k_op_format.end()) {
    auto  tok1 = p.tok_to_str(p.peek(0).tokid);
    auto& tok2 = p.peek(1);

    format.fill = tok1[0];
    switch (tok2.kind) {
    case token::ETokenKind::L_ANGLE: {
      format.align = ast::Literal_Format_Specifier::EAlign::Left;
      break;
    }
    case token::ETokenKind::R_ANGLE: {
      format.align = ast::Literal_Format_Specifier::EAlign::Right;
      break;
    }
    case token::ETokenKind::OP_CIRCUMFLEX: {
      format.align = ast::Literal_Format_Specifier::EAlign::Center;
      break;
    }
    case token::ETokenKind::TILDE: {
      format.align = ast::Literal_Format_Specifier::EAlign::Justify;
      break;
    }
    default: break;
    }

    (void)p.next();
    (void)p.next();
  }

  // sign
  if (p.match_any({token::ETokenKind::OP_PLUS, token::ETokenKind::OP_MINUS, token::ETokenKind::SPACE})) {
    switch (p.peek(-1).kind) {
    case token::ETokenKind::OP_PLUS: {
      format.sign = ast::Literal_Format_Specifier::ESign::Pos;
      break;
    }
    case token::ETokenKind::OP_MINUS: {
      format.sign = ast::Literal_Format_Specifier::ESign::Neg;
      break;
    }
    case token::ETokenKind::SPACE: {
      format.sign = ast::Literal_Format_Specifier::ESign::Space;
      break;
    }
    default: break;
    }
  }

  // prefix numeric
  if (p.match(token::ETokenKind::HASHTAG)) {
    (void)p.expect(85, token::ETokenKind::L_CUNE, "Expected integral prefix 'x', 'X', 'o' or 'b'.", hint);
    char prefix = p.tok_to_str(p.peek(-1).tokid)[0];

    switch (prefix) {
    case 'x': format.prefix = ast::Literal_Format_Specifier::EPrefix::Hex; break;
    case 'X': format.prefix = ast::Literal_Format_Specifier::EPrefix::HEX; break;
    case 'b': format.prefix = ast::Literal_Format_Specifier::EPrefix::Bin; break;
    case 'o': format.prefix = ast::Literal_Format_Specifier::EPrefix::Oct; break;
    default:  break;
    }
  }

  // fill with 0 + sign before fill
  if (p.tok_to_str(p.peek().tokid) == "0") {
    format.zero_pad = true;
    (void)p.next();

    if (p.match(token::ETokenKind::ASSIGN)) {
      format.signBeforeFill = true;
    }
  }

  // width from variable
  if (p.match(token::ETokenKind::L_CURLY)) {
    format.width = p.p_expr->parse_expression_term();
    (void)p.expect(86, token::ETokenKind::L_CURLY, "Expected close variable width '}'.", hint);
  }
  // width from literal
  else if (p.check(token::ETokenKind::L_I)) {
    auto width   = p.p_expr->parse_expression_term();
    format.width = width;
  }

  // grouping char
  if (p.check_any({token::ETokenKind::COMMA, token::ETokenKind::UNDERSCORE, token::ETokenKind::TICK})) {
    format.grouping_char = p.tok_to_str(p.next().tokid)[0];
  }

  // precision
  if (p.match(token::ETokenKind::DOT)) {
    // precision from variable
    if (p.match(token::ETokenKind::L_CURLY)) {
      format.precision = p.p_expr->parse_expression_term();
      (void)p.expect(87, token::ETokenKind::L_CURLY, "Expected close variable width '}'.", hint);
    }
    // precision from literal
    else if (p.check(token::ETokenKind::L_I)) {
      auto width       = p.p_expr->parse_expression_term();
      format.precision = width;
    }
  }

  // display format
  if (p.match_any({token::ETokenKind::L_CUNE, token::ETokenKind::PERCENTAGE})) {
    switch (p.tok_to_str(p.peek(-1).tokid)[0]) {
    case 's': format.display_format = ast::Literal_Format_Specifier::EDisplayFormat::String; break;
    case 'b': format.display_format = ast::Literal_Format_Specifier::EDisplayFormat::Binary; break;
    case 'c': format.display_format = ast::Literal_Format_Specifier::EDisplayFormat::Character; break;
    case 'd': format.display_format = ast::Literal_Format_Specifier::EDisplayFormat::Decimal; break;
    case 'o': format.display_format = ast::Literal_Format_Specifier::EDisplayFormat::Octal; break;
    case 'x': format.display_format = ast::Literal_Format_Specifier::EDisplayFormat::Hex; break;
    case 'X': format.display_format = ast::Literal_Format_Specifier::EDisplayFormat::HEX; break;
    case 'n': format.display_format = ast::Literal_Format_Specifier::EDisplayFormat::Number; break;
    case 'e': format.display_format = ast::Literal_Format_Specifier::EDisplayFormat::e; break;
    case 'E': format.display_format = ast::Literal_Format_Specifier::EDisplayFormat::E; break;
    case 'f': format.display_format = ast::Literal_Format_Specifier::EDisplayFormat::Fixed; break;
    case 'F': format.display_format = ast::Literal_Format_Specifier::EDisplayFormat::FIXED; break;
    case 'g': format.display_format = ast::Literal_Format_Specifier::EDisplayFormat::g; break;
    case 'G': format.display_format = ast::Literal_Format_Specifier::EDisplayFormat::G; break;
    case '%': format.display_format = ast::Literal_Format_Specifier::EDisplayFormat::Percentage; break;
    default:  break;
    }
  }

  if (!p.check(token::ETokenKind::S_INTERPOLATION_END)) {
    p.add_error(88, "Unexpected token '" + std::string(p.tok_to_str(p.peek().tokid)) + "' in format specifier.", hint);
  }

  return format.nodeid;
}

ast::ID parser::Parser_Literal::literal_range(ast::ID p_start)
{
  constexpr std::string_view hint =
      "define range like:"
      "\n  - absolute range: `0..10` `0..=9`"
      "\n  - relative range: `a..b` `a..=b - 1`"
      "\n  - slice all: `..`"
      "\n  - slice from 0: `..end` `..=end`"
      "\n  - slice to max: `start..` `start..=`";

  auto& range = p.add_get_node<ast::Literal_Range>(p.peek().tokid);

  if (!p_start) {
    auto& zero  = p.add_get_node<ast::Literal_Integral>(p.peek().tokid);
    zero.val    = Int128(0);
    range.start = zero.nodeid;
  } else
    range.start = p_start;

  auto& range_tok = p.expect_any(89, {token::ETokenKind::RANGE, token::ETokenKind::RANGE_INCLUSIVE},
                                 "Expected range kind '..' or '..='", hint);

  range.endInclude = range_tok.kind == token::ETokenKind::RANGE_INCLUSIVE;

  if (!p.check_any(token::k_args_ending)) {
    range.end = p.p_expr->parse_expression();
  } else {
    auto& zero = p.add_get_node<ast::Literal_Integral>(p.peek().tokid);
    zero.val   = Int128(0);
    range.end  = zero.nodeid;
  }

  return range.nodeid;
}

ast::ID parser::Parser_Literal::literal_table()
{
  constexpr std::string_view hint =
      R"(define literal table like:
  - table { 1, 2, 3, 4 }
  - table population { 0..4 => @i + 1 }
  - matrix { 1, 2, 3, 4 }*3
  - matrix {{ 1, 2 },{ 3, 4 }}
  - matrix population { [0..4] => @i + 1 }*3
  - matrix population { [0..4, 0..4] => @i + 1 + @j }
  - map table { a: 1, b: 2, c: 3 }
  - map table population { [0..4] => text_number[@i] : @i })";

  (void)p.match(token::ETokenKind::L_CURLY);

  if (p.match(token::ETokenKind::R_CURLY)) {
    auto& node = p.add_get_node<ast::Literal_Table>(p.peek(-1).tokid);
    return node.nodeid;
  }

  std::vector<ast::Literal_Map::Association> associations;
  std::vector<ast::ID>                       values;
  std::vector<ast::ID>                       map_values;

  while (!p.is_end()) {

    auto    val = p.p_expr->parse_expression();
    ast::ID map_val;

    if (p.match(token::ETokenKind::COLON)) map_val = p.p_expr->parse_expression();

    associations.emplace_back(ast::Literal_Map::Association{.key = val, .value = map_val});

    if (p.match_field_separator(token::ETokenKind::COMMA, token::ETokenKind::R_CURLY)) break;
  }

  // is a literal table population
  if (associations.size() == 1) {
    if (const auto* tbl_pop = associations[0].key.as<ast::Literal_Table_Population>()) {
      // is a map population
      if (tbl_pop->map_expression_value) {
        auto& map      = p.add_get_node<ast::Literal_Map>(tbl_pop->node_token_id);
        map.population = tbl_pop->nodeid;
        return map.nodeid;
      }

      auto& tbl      = p.add_get_node<ast::Literal_Table>(tbl_pop->node_token_id);
      tbl.population = tbl_pop->nodeid;
      return tbl.nodeid;
    }
  }

  if (!associations.empty() && associations[0].value) {
    auto& map = p.add_get_node<ast::Literal_Map>(p.peek().tokid);

    map.associations = associations;
    return map.nodeid;
  }

  auto& tbl = p.add_get_node<ast::Literal_Table>(p.peek().tokid);
  tbl.values.reserve(associations.size());
  for (auto [key, val] : associations) tbl.values.emplace_back(key);


  return tbl.nodeid;
}

ast::ID parser::Parser_Literal::literal_table_population()
{
  (void)p.match(token::ETokenKind::L_SQUARE);

  auto& pop = p.add_get_node<ast::Literal_Table_Population>(p.peek().tokid);

  while (!p.is_end()) {
    pop.ranges.emplace_back(p.p_expr->parse_expression());

    if (p.match_field_any_separator(token::ETokenKind::INJECT, {token::ETokenKind::R_CURLY})) break;
  }

  pop.expression = p.p_expr->parse_expression();

  // mapping population
  if (p.match(token::ETokenKind::COLON)) {
    pop.map_expression_value = p.p_expr->parse_expression_term();
  }

  return pop.nodeid;
}

ast::ID parser::Parser_Literal::literal_form(ast::ID nodeid)
{
  auto& lit_form = p.add_get_node<ast::Literal_Form>(p.peek().tokid);
  lit_form.name  = nodeid;

  (void)p.match(token::ETokenKind::L_CURLY);
  if (p.match(token::ETokenKind::R_CURLY)) return lit_form.nodeid;

  while (!p.is_end()) {
    auto facet_name = p.p_base->identifier(true);

    if (p.check(token::ETokenKind::L_CURLY)) {
      lit_form.facet_args.emplace_back(literal_facet(facet_name));
    } else if (p.match(token::ETokenKind::DOT)) {
      auto& lit_facet = p.add_get_node<ast::Literal_Structured_Data>(p.peek(-2).tokid);
      lit_facet.fields_args.emplace_back(literal_field());
      lit_form.facet_args.emplace_back(lit_facet.nodeid);
    } else {
      p.add_error_tok(91, p.peek(-1), "Unexpected literal reference", "define literal facets only in literal form");
    }

    if (p.match_field_separator(token::ETokenKind::COMMA, token::ETokenKind::R_CURLY)) break;
  }

  return lit_form.nodeid;
}

ast::ID parser::Parser_Literal::literal_facet(ast::ID nodeid)
{
  constexpr std::string_view hint =
      R"(define literal facet like:
  - no fields `name{.}`
  - normal `name{ .field1= val1, .field2= val2 }`
  - generic `name<gen_args>{ .field1= val1, .field2= val2 }`)";

  auto& facet = p.add_get_node<ast::Literal_Structured_Data>(p.peek().tokid);
  facet.name  = nodeid;

  (void)p.match(token::ETokenKind::L_CURLY);
  if (!p.match(token::ETokenKind::R_CURLY)) return facet.nodeid;

  while (!p.is_end()) {
    if (p.match(token::ETokenKind::R_CURLY)) break;

    facet.fields_args.emplace_back(literal_field());

    if (p.match_field_separator(token::ETokenKind::COMMA, token::ETokenKind::R_CURLY)) break;
  }

  return facet.nodeid;
}

ast::ID parser::Parser_Literal::literal_field()
{
  constexpr std::string_view hint =
      "define literal filed like:"
      "\n  - scoped field `name{ .field1= val1, .field2= val2 }`"
      "\n  - direct field `name.field1= val1`";

  (void)p.match(token::ETokenKind::DOT);

  auto& field_arg         = p.add_get_node<ast::Expression_Call_Argument>(p.peek().tokid);
  field_arg.explicit_name = p.parse_name("", hint);

  (void)p.expect(94, token::ETokenKind::ASSIGN, "Expected field assignation '=' after field name", hint);

  field_arg.expression = p.p_expr->parse_expression();

  return field_arg.nodeid;
}

ast::ID parser::Parser_Literal::literal_tuple()
{
  constexpr std::string_view hint = "define named tuple instance like `(filed1: value, ...)`.";

  auto& tuple          = p.add_get_node<ast::Literal_Tuple>(p.peek().tokid);
  bool  is_named_tuple = p.check(token::ETokenKind::COLON); // (name: type, ...) or (type, ...)

  while (!p.is_end()) {
    ast::Literal_Tuple::Field field;
    if (is_named_tuple) {
      field.name = std::string(p.parse_name("", hint));

      (void)p.expect(96, token::ETokenKind::ASSIGN,
                     "Expected field value assignation ':' after filed name in named tuple instance.", hint);

      field.value = p.p_expr->parse_expression();
    } else {
      field.value = p.p_expr->parse_expression();
    }

    tuple.fields.emplace_back(field);

    if (p.match(token::ETokenKind::COMMA)) continue;

    break;
  }

  return tuple.nodeid;
}
