#include "parser_literal.hpp"

#include <llvm/ADT/APInt.h>

#include <llvm/ADT/APFloat.h>
#include <string>
#include <vector>

#include "nexus/ast/ast.hpp"
#include "nexus/lexer/token_viewer.hpp"
#include "nexus/script.hpp"

#include "ast/ast_literal.hpp"
#include "ast/ast_expression.hpp"

#include "ast/ast_numeric_128_bits.hpp"
#include "compiler/compiler.hpp"
#include "nexus/forward.hpp"
#include "nexus/lexer/token.hpp"
#include "misc/error_output.hpp"
#include "nexus/type.hpp"
#include "parser/parser_base.hpp"
#include "parser_context.hpp"
#include "parser_expression.hpp"

ast::_gnid parser::Parser_Literal::try_literal(bool p_is_silent_error)
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
  case token::ETokenKind::OPEN_BRACE: return literal_table();
  case token::ETokenKind::OPEN_PAREN:
    if (p.check_at(2, token::ETokenKind::COMMA) || p.check_at(2, token::ETokenKind::COLON)) {
      return literal_tuple();
    }
  default: break;
  }

  if (!p_is_silent_error) {
    p.add_error(80, "Expected literal value", "");
  }

  return BAD_NODE_ID;
}

ast::_gnid parser::Parser_Literal::literal_boolean()
{
  parser_add_node(literal, Literal_Boolean, p.peek().id);
  literal->val = p.match(token::ETokenKind::TRUE);
  return literal->node_id;
}

ast::_gnid parser::Parser_Literal::literal_numeric()
{
  auto tok = p.next();
  auto val = p.tok_to_str(tok.id);

  if (p.check_any(token::k_type_integral)) {
    return literal_integral(tok);
  } else if (p.check_any(token::k_type_fixed_point)) {
    return literal_fixed_point(val);
  } else if (p.check_any(token::k_type_floating_point)) {
    return literal_floating_point(val);
  } else if (p.check_val("f")) {
    return literal_floating_point(val);
  } else if (p.check_val("d")) {
    return literal_fixed_point(val);
  } else if (p.check_val("ud")) {
    return literal_fixed_point(val);
  }

  return literal_integral(tok);
}


ast::_gnid parser::Parser_Literal::literal_decimal()
{
  auto val = p.tok_to_str(p.next().id);

  if (p.check_any(token::k_type_fixed_point)) {
    return literal_fixed_point(val);
  } else if (p.check_any(token::k_type_floating_point)) {
    return literal_floating_point(val);
  } else if (p.check_val("f")) {
    return literal_floating_point(val);
  } else if (p.check_val("d")) {
    return literal_fixed_point(val);
  } else if (p.check_val("ud")) {
    return literal_fixed_point(val);
  }

  return literal_floating_point(val);
}

ast::_gnid parser::Parser_Literal::literal_fixed_point(std::string_view p_val)
{
  parser_add_node(literal, Literal_Fixed_Point, p.peek().id);

  size_t scale_pos    = p_val.find('.');
  literal->scale      = scale_pos == 0 ? 0 : p_val.size() - scale_pos;
  std::string raw_val = std::string(p_val);
  std::erase(raw_val, '.');

  p.next(); // consume literal

  // post literal type like 99.999ud or 99.999d32
  if (p.match_any(token::k_type_fixed_point))
    literal->raw_type = type::ETokenKind_to_EPrimitiveTypeKind(p.peek(-1).kind);
  else if (p.match_val("ud"))
    literal->raw_type = type::EPrimitiveTypeKind::udSize;
  else if (p.match_val("d"))
    literal->raw_type = type::EPrimitiveTypeKind::dSize;

  // post decimal explicit scale
  if (p.match(token::ETokenKind::COLON) && p.check(token::ETokenKind::L_I)) {
    literal->scale = std::stoi(std::string(p.tok_to_str(p.next().id)));
  }

  auto api = llvm::APInt(128, raw_val, 10);

  literal->val = Int128(api);

  return literal->node_id;
}

ast::_gnid parser::Parser_Literal::literal_floating_point(std::string_view p_val)
{
  parser_add_node(literal, Literal_Floating_Point, p.peek().id);
  literal->val.string_to_f128(p_val);

  // post literal type like 9.99f32 9.99f64
  if (p.match_any(token::k_type_floating_point)) {
    literal->type = type::ETokenKind_to_EPrimitiveTypeKind(p.peek(-1).kind);
  }
  p.match_val("f");

  return literal->node_id;
}

ast::_gnid parser::Parser_Literal::literal_integral(const token::Token& p_tok)
{
  static const char* hint =
      "define literal integral like:\n  - decimal: 1234\n  - bin: 0b10011010010\n  - oct: 0o2322\n  - hex: 0x4d2";

  parser_add_node(literal, Literal_Integral, p_tok.id);
  auto& api = *literal->val.val;

  auto tok_val = p.tok_to_str(p_tok.id);

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
      api           = llvm::APInt(128, tok_val,
                                  (p_tok.kind == token::ETokenKind::L_BIN   ? 2
                                   : p_tok.kind == token::ETokenKind::L_OCT ? 8
                                   : p_tok.kind == token::ETokenKind::L_HEX ? 16
                                                                            : 10));
      literal->type = type::EPrimitiveTypeKind::i128;
    }
  } catch (const std::invalid_argument&) {
    p.add_error(82, "Impossible to parse literal integral", hint);
  } catch (const std::out_of_range&) {
    p.add_error(83, "Integral literal too big for 128 bits", hint);
  }

  // post literal type like 10i8 0u32
  if (p.match_any(token::k_type_integral)) {
    literal->type = type::ETokenKind_to_EPrimitiveTypeKind(p.peek(-1).kind);
  } else if (p.match_val("i")) {
    literal->type = type::EPrimitiveTypeKind::iSize;
  } else if (p.match_val("u")) {
    literal->type = type::EPrimitiveTypeKind::uSize;
  } else if (p.match_val("b")) {
    literal->type = type::EPrimitiveTypeKind::bSize;
  }

  return literal->node_id;
}

ast::_gnid parser::Parser_Literal::literal_cune()
{
  parser_add_node(literal, Literal_Cune, p.peek().id);
  literal->val = p.tok_to_str(p.next().id)[0];
  return literal->node_id;
}

ast::_gnid parser::Parser_Literal::literal_textual()
{

  std::vector<ast::_gnid> values;

  while (!p.is_end()) {

    if (p.check(token::ETokenKind::L_TEXTUAL)) {
      parser_add_node(text, Literal_Text_Pure, p.peek().id);
      text->val = p.tok_to_str(p.next().id);

      // type inference
      if (p.match(token::ETokenKind::T_TEXT) || p.match_val("t")) {
        text->text_type = type::ETextType::text;
        if (p.match(token::ETokenKind::T_STRING) || p.match_val("s")) {
          text->text_type = type::ETextType::str;
        }
      } else if (p.match(token::ETokenKind::T_C_STRING) || p.match_val("c")) {
        text->text_type = type::ETextType::c_str;
      } else if (p.match(token::ETokenKind::T_CUNE) || p.match_val("cu")) {
        text->text_type = type::ETextType::cune;
      } else if (p.match(token::ETokenKind::T_RUNE) || p.match_val("r")) {
        text->text_type = type::ETextType::rune;
      }

      values.push_back(text->node_id);
      continue;
    }
    // interpolation
    else if (p.match(token::ETokenKind::S_INTERPOLATION_START)) {
      parser_add_node(lerp, Literal_Text_Interpolation, p.peek().id);
      lerp->expression = p.p_expr->parse_expression();

      if (p.match(token::ETokenKind::COLON)) {
        lerp->specifier = format_specifier();
      }

      values.push_back(lerp->node_id);

      p.expect(84, token::ETokenKind::S_INTERPOLATION_END, "Expected end interpolation '}' in string formatted.",
               "define format string like: `f\"you age is {now - birthday} years\"`.");

      continue;
    }

    break;
  }

  if (values.size() == 1) {
    if (auto node = p.scr_info.nodes->get_as<ast::Literal_Text_Pure>(values[0].get_node_id())) {
      return node->node_id;
    }
  }

  parser_add_node(f_text, Literal_Textual_Format, p.peek().id);
  f_text->values = values;
  return f_text->node_id;
}

ast::_gnid parser::Parser_Literal::format_specifier()
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

  parser_add_node(format, Literal_Format_Specifier, p.peek().id);

  // fill + align
  if (std::find(token::k_op_format.begin(), token::k_op_format.end(), p.peek(1).kind) != token::k_op_format.end()) {
    auto  tok1 = p.tok_to_str(p.peek(0).id);
    auto& tok2 = p.peek(1);

    format->fill = tok1[0];
    switch (tok2.kind) {
    case token::ETokenKind::OPEN_BRACKETS: {
      format->align = ast::Literal_Format_Specifier::EAlign::Left;
      break;
    }
    case token::ETokenKind::CLOSE_BRACKETS: {
      format->align = ast::Literal_Format_Specifier::EAlign::Right;
      break;
    }
    case token::ETokenKind::OP_CIRCUMFLEX: {
      format->align = ast::Literal_Format_Specifier::EAlign::Center;
      break;
    }
    case token::ETokenKind::TILDE: {
      format->align = ast::Literal_Format_Specifier::EAlign::Justify;
      break;
    }
    default: break;
    }

    p.next();
    p.next();
  }

  // sign
  if (p.match_any({token::ETokenKind::OP_PLUS, token::ETokenKind::OP_MINUS, token::ETokenKind::SPACE})) {
    switch (p.peek(-1).kind) {
    case token::ETokenKind::OP_PLUS: {
      format->sign = ast::Literal_Format_Specifier::ESign::Pos;
      break;
    }
    case token::ETokenKind::OP_MINUS: {
      format->sign = ast::Literal_Format_Specifier::ESign::Neg;
      break;
    }
    case token::ETokenKind::SPACE: {
      format->sign = ast::Literal_Format_Specifier::ESign::Space;
      break;
    }
    default: break;
    }
  }

  // prefix numeric
  if (p.match(token::ETokenKind::HASHTAG)) {
    p.expect(85, token::ETokenKind::L_CUNE, "Expected integral prefix 'x', 'X', 'o' or 'b'.", hint);
    char prefix = p.tok_to_str(p.peek(-1).id)[0];

    switch (prefix) {
    case 'x': format->prefix = ast::Literal_Format_Specifier::EPrefix::Hex; break;
    case 'X': format->prefix = ast::Literal_Format_Specifier::EPrefix::HEX; break;
    case 'b': format->prefix = ast::Literal_Format_Specifier::EPrefix::Bin; break;
    case 'o': format->prefix = ast::Literal_Format_Specifier::EPrefix::Oct; break;
    }
  }

  // fill with 0 + sign before fill
  if (p.tok_to_str(p.peek().id) == "0") {
    format->zero_pad = true;
    p.next();

    if (p.match(token::ETokenKind::ASSIGN)) {
      format->signBeforeFill = true;
    }
  }

  // width from variable
  if (p.match(token::ETokenKind::OPEN_BRACE)) {
    format->width = p.p_expr->parse_expression_term();
    p.expect(86, token::ETokenKind::OPEN_BRACE, "Expected close variable width '}'.", hint);
  }
  // width from literal
  else if (p.check(token::ETokenKind::L_I)) {
    auto width    = p.p_expr->parse_expression_term();
    format->width = std::move(width);
  }

  // grouping char
  if (p.check_any({token::ETokenKind::COMMA, token::ETokenKind::UNDERSCORE, token::ETokenKind::TICK})) {
    format->grouping_char = p.tok_to_str(p.next().id)[0];
  }

  // precision
  if (p.match(token::ETokenKind::DOT)) {
    // precision from variable
    if (p.match(token::ETokenKind::OPEN_BRACE)) {
      format->precision = p.p_expr->parse_expression_term();
      p.expect(87, token::ETokenKind::OPEN_BRACE, "Expected close variable width '}'.", hint);
    }
    // precision from literal
    else if (p.check(token::ETokenKind::L_I)) {
      auto width        = p.p_expr->parse_expression_term();
      format->precision = std::move(width);
    }
  }

  // display format
  if (p.match_any({token::ETokenKind::L_CUNE, token::ETokenKind::PERCENTAGE})) {
    switch (p.tok_to_str(p.peek(-1).id)[0]) {
    case 's': format->display_format = ast::Literal_Format_Specifier::EDisplayFormat::String; break;
    case 'b': format->display_format = ast::Literal_Format_Specifier::EDisplayFormat::Binary; break;
    case 'c': format->display_format = ast::Literal_Format_Specifier::EDisplayFormat::Character; break;
    case 'd': format->display_format = ast::Literal_Format_Specifier::EDisplayFormat::Decimal; break;
    case 'o': format->display_format = ast::Literal_Format_Specifier::EDisplayFormat::Octal; break;
    case 'x': format->display_format = ast::Literal_Format_Specifier::EDisplayFormat::Hex; break;
    case 'X': format->display_format = ast::Literal_Format_Specifier::EDisplayFormat::HEX; break;
    case 'n': format->display_format = ast::Literal_Format_Specifier::EDisplayFormat::Number; break;
    case 'e': format->display_format = ast::Literal_Format_Specifier::EDisplayFormat::e; break;
    case 'E': format->display_format = ast::Literal_Format_Specifier::EDisplayFormat::E; break;
    case 'f': format->display_format = ast::Literal_Format_Specifier::EDisplayFormat::Fixed; break;
    case 'F': format->display_format = ast::Literal_Format_Specifier::EDisplayFormat::FIXED; break;
    case 'g': format->display_format = ast::Literal_Format_Specifier::EDisplayFormat::g; break;
    case 'G': format->display_format = ast::Literal_Format_Specifier::EDisplayFormat::G; break;
    case '%': format->display_format = ast::Literal_Format_Specifier::EDisplayFormat::Percentage; break;
    }
  }

  if (!p.check(token::ETokenKind::S_INTERPOLATION_END)) {
    p.add_error(88, "Unexpected token '" + std::string(p.tok_to_str(p.peek().id)) + "' in format specifier.", hint);
  }

  return format->node_id;
}

ast::_gnid parser::Parser_Literal::literal_range(ast::_gnid p_start)
{
  constexpr std::string_view hint =
      "define range like:"
      "\n  - absolute range: `0..10` `0..=9`"
      "\n  - relative range: `a..b` `a..=b - 1`"
      "\n  - slice all: `..`"
      "\n  - slice from 0: `..end` `..=end`"
      "\n  - slice to max: `start..` `start..=`";

  parser_add_node(range, Literal_Range, p.peek().id);

  if (!p_start) {
    parser_add_node(zero, Literal_Integral, p.peek().id);
    zero->val    = Int128(0);
    range->start = zero->node_id;
  } else
    range->start = p_start;

  auto range_tok = p.expect_any(89, {token::ETokenKind::RANGE, token::ETokenKind::RANGE_INCLUSIVE},
                                "Expected range kind '..' or '..='", hint);

  range->endInclude = range_tok.kind == token::ETokenKind::RANGE_INCLUSIVE;

  if (!p.check_any(token::k_args_ending)) {
    range->end = p.p_expr->parse_expression();
  } else {
    parser_add_node(zero, Literal_Integral, p.peek().id);
    zero->val  = Int128(0);
    range->end = zero->node_id;
  }

  return range->node_id;
}

ast::_gnid parser::Parser_Literal::literal_table()
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

  p.match(token::ETokenKind::OPEN_BRACE);

  if (p.match(token::ETokenKind::CLOSE_BRACE)) {
    parser_add_node(node, Literal_Table, p.peek(-1).id);
    return node->node_id;
  }

  std::vector<ast::Literal_Map::Association> associations;
  std::vector<ast::_gnid>                    values;
  std::vector<ast::_gnid>                    map_values;

  while (!p.is_end()) {

    auto       val = p.p_expr->parse_expression();
    ast::_gnid map_val;

    if (p.match(token::ETokenKind::COLON)) map_val = p.p_expr->parse_expression();

    associations.emplace_back(ast::Literal_Map::Association{.key = val, .value = map_val});

    if (p.match_field_separator(token::ETokenKind::COMMA, token::ETokenKind::CLOSE_BRACE)) break;
  }

  // is a literal table population
  if (associations.size() == 1) {
    if (auto tbl_pop = p.scr_info.nodes->get_as<ast::Literal_Table_Population>(associations[0].key.get_node_id())) {
      // is a map population
      if (tbl_pop->map_expression_value) {
        parser_add_node(map, Literal_Map, tbl_pop->node_token_id);
        map->population = tbl_pop->node_id;
        return map->node_id;
      } else {
        parser_add_node(tbl, Literal_Table, tbl_pop->node_token_id);
        tbl->population = tbl_pop->node_id;
        return tbl->node_id;
      }
    }
  }

  if (!associations.empty() && associations[0].value) {
    parser_add_node(map, Literal_Map, p.peek().id);

    map->associations = associations;
    return map->node_id;
  }

  parser_add_node(tbl, Literal_Table, p.peek().id);
  tbl->values.reserve(associations.size());
  for (auto [key, val] : associations) tbl->values.push_back(key);


  return tbl->node_id;
}

ast::_gnid parser::Parser_Literal::literal_table_population()
{
  p.match(token::ETokenKind::OPEN_SQUARE);

  parser_add_node(pop, Literal_Table_Population, p.peek().id);

  while (!p.is_end()) {
    pop->ranges.push_back(p.p_expr->parse_expression());

    if (p.match_field_any_separator(token::ETokenKind::INJECT, {token::ETokenKind::CLOSE_BRACE})) break;
  }

  pop->expression = p.p_expr->parse_expression();

  // mapping population
  if (p.match(token::ETokenKind::COLON)) {
    pop->map_expression_value = p.p_expr->parse_expression_term();
  }

  return pop->node_id;
}

ast::_gnid parser::Parser_Literal::literal_entity(ast::_gnid p_id)
{
  parser_add_node(lit_entity, Literal_Entity, p.peek().id);
  lit_entity->name = p_id;

  p.match(token::ETokenKind::OPEN_BRACE);
  if (p.match(token::ETokenKind::CLOSE_BRACE)) return lit_entity->node_id;

  while (!p.is_end()) {
    auto comp_name = p.p_base->identifier(true);

    if (p.check(token::ETokenKind::OPEN_BRACE)) {
      lit_entity->component_args.push_back(literal_component(comp_name));
    } else if (p.match(token::ETokenKind::DOT)) {
      parser_add_node(lit_comp, Literal_Structured_Data, p.peek(-2).id);
      lit_comp->fields_args.push_back(literal_field());
      lit_entity->component_args.push_back(lit_comp->node_id);
    } else {
      p.add_error_tok(91, p.peek(-1), "Unexpected literal reference",
                      "define literal components only in literal entity");
    }

    if (p.match_field_separator(token::ETokenKind::COMMA, token::ETokenKind::CLOSE_BRACE)) break;
  }

  return lit_entity->node_id;
}

ast::_gnid parser::Parser_Literal::literal_component(ast::_gnid id)
{
  constexpr std::string_view hint =
      R"(define literal component like:
  - no fields `name{.}`
  - normal `name{ .field1= val1, .field2= val2 }`
  - generic `name<gen_args>{ .field1= val1, .field2= val2 }`)";

  parser_add_node(comp, Literal_Structured_Data, p.peek().id);
  comp->name = id;

  p.match(token::ETokenKind::OPEN_BRACE);
  if (!p.match(token::ETokenKind::CLOSE_BRACE)) return comp->node_id;

  while (!p.is_end()) {
    if (p.match(token::ETokenKind::CLOSE_BRACE)) break;

    comp->fields_args.push_back(literal_field());

    if (p.match_field_separator(token::ETokenKind::COMMA, token::ETokenKind::CLOSE_BRACE)) break;
  }

  return comp->node_id;
}

ast::_gnid parser::Parser_Literal::literal_field()
{
  constexpr std::string_view hint =
      "define literal filed like:"
      "\n  - scoped field `name{ .field1= val1, .field2= val2 }`"
      "\n  - direct field `name.field1= val1`";

  p.match(token::ETokenKind::DOT);

  parser_add_node(field_arg, Expression_Call_Argument, p.peek().id);
  field_arg->explicit_name = p.parse_name("", hint);

  p.expect(94, token::ETokenKind::ASSIGN, "Expected field assignation '=' after field name", hint);

  field_arg->expression = p.p_expr->parse_expression();

  return field_arg->node_id;
}

ast::_gnid parser::Parser_Literal::literal_tuple()
{
  constexpr std::string_view hint = "define named tuple instance like `(filed1: value, ...)`.";

  parser_add_node(tuple, Literal_Tuple, p.peek().id);
  bool is_named_tuple = p.check(token::ETokenKind::COLON); // (name: type, ...) or (type, ...)

  while (!p.is_end()) {
    ast::Literal_Tuple::Field field;
    if (is_named_tuple) {
      field.name = p.parse_name("", hint);

      p.expect(96, token::ETokenKind::ASSIGN,
               "Expected field value assignation ':' after filed name in named tuple instance.", hint);

      field.value = p.p_expr->parse_expression();
    } else {
      field.value = p.p_expr->parse_expression();
    }

    tuple->fields.push_back(std::move(field));

    if (p.match(token::ETokenKind::COMMA)) continue;

    break;
  }

  return tuple->node_id;
}
