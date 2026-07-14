
#include "parser_declaration_global.hpp"

#include <string>
#include <string_view>

#include "ast/ast_base.hpp"
#include "ast/ast_declaration_extension.hpp"
#include "ast/ast_declaration_global.hpp"
#include "ast/ast_declaration_local.hpp"
#include "compiler/compilation_unit.hpp"
#include "nexus/ast/ast.hpp"
#include "nexus/ast/data.hpp"
#include "nexus/forward.hpp"
#include "nexus/lexer/token.hpp"
#include "nexus/type/type.hpp"
#include "nexus/metacode/metacode.hpp"

#include "parser_context.hpp"
#include "parser_type.hpp"

#include "ast/ast_generic.hpp"

#include "parser_expression.hpp"
#include "parser_declaration_local.hpp"
#include "parser_declaration_sfm.hpp"
#include "parser_base.hpp"


ast::ID parser::Parser_Declaration::parse_declaration()
{
  (void)p.match(token::ETokenKind::SEMICOLON);

  auto& tok = p.peek();

  switch (tok.kind) {
  case token::ETokenKind::MOD:       return _module();
  case token::ETokenKind::UNION:     return _union();
  case token::ETokenKind::FLAG:      return flag();
  case token::ETokenKind::ENUM:      return enumeration();
  case token::ETokenKind::VAR:
  case token::ETokenKind::LET:
  case token::ETokenKind::CONST:     return global_variable();
  case token::ETokenKind::FUNCTION:  return function();
  case token::ETokenKind::EXTENSION: return p.p_extend->parse_extension();
  case token::ETokenKind::GENERIC:   return generic();
  case token::ETokenKind::TYPE:      return type_alias();
  case token::ETokenKind::FACET:     return p.p_sfm->facet();
  case token::ETokenKind::RULE:      return p.p_sfm->rule();
  case token::ETokenKind::FORM:      return p.p_sfm->form();
  case token::ETokenKind::REEXPORT:  return p.p_base->parse_reexport();
  case token::ETokenKind::EXPORT:    return p.p_base->parse_export();
  case token::ETokenKind::EXTERN:    return p.p_base->parse_extern();
  case token::ETokenKind::IMPORT:    return p.p_base->parse_import();
  default:                           break;
  }

  p.add_error(60, "Illegal instruction '" + std::string(p.tok_to_str(tok.tokid)) + "' in global.",
              "you can define in global: namespace, variable, function, form, facet, rule");

  (void)p.next();

  THROW_BAD_NODE;
}


ast::ID parser::Parser_Declaration::parse_codeblock_declaration(bool no_import, bool no_export)
{
  (void)p.expect_any(46, {token::ETokenKind::L_CURLY, token::ETokenKind::INJECT}, "Expected start code block '{'", "");

  bool  inline_code = p.peek(-1).kind == token::ETokenKind::INJECT;
  auto& cb          = p.add_get_node<ast::CodeBlock>(p.peek().tokid);

  while (!p.is_end()) {
    (void)p.match(token::ETokenKind::SEMICOLON);

    if (p.match_field_separator(token::ETokenKind::S_END_OF_FILE, token::ETokenKind::R_CURLY)) break;

    if (no_export && p.check_any({token::ETokenKind::EXPORT})) {
      p.add_error_tok(10, p.peek(), "Illegal nested export module instruction.", "");
    }

    if (no_import && p.check_any({token::ETokenKind::IMPORT})) {
      p.add_error_tok(10, p.peek(), "Illegal import module instruction.", "");
    }

    if (auto decl = parse_declaration())
      cb.elements.emplace_back(decl);
    else
      p.add_error(255, "Unexpected token '" + std::string(p.peek().tokid.str()) + "' inside codeblock.",
                  "A codeblock is ended by '}'");

    // one instruction
    if (inline_code) break;
  }

  return cb.nodeid;
}

ast::ID parser::Parser_Declaration::_module()
{
  constexpr std::string_view hint =
      R"(define module like:"
  - module `mod myName { ... }`"
  - module alias `mod Vec = core::container::vector`"
  - module to root `mod root = core::container::vector`)";

  (void)p.match(token::ETokenKind::MOD);

  auto name = p.parse_name("", hint);

  if (p.match(token::ETokenKind::ASSIGN)) {
    auto& node = p.add_get_node<ast::Global_Alias_Module>(p.peek().tokid);
    node.alias = name;
    node.regex = p.p_base->identifier();


    (void)p.add_definition(node.nodeid);

    return node.nodeid;
  }

  if (p.match(token::ETokenKind::L_CURLY)) {
    auto& node = p.add_get_node<ast::Global_Module>(p.peek(-2).tokid);

    p.enter_scope(node, name);
    node.name = name;

    node.codeblock = p.p_decl->parse_codeblock_declaration();

    p.exit_scope();

    return node.nodeid;
  }

  p.add_error_tok(61, p.peek(), "Expected '{' or '=' after module name.", hint);

  THROW_BAD_NODE;
}

ast::ID parser::Parser_Declaration::enumeration()
{
  constexpr std::string_view hint =
      "define enum like:"
      "\n  - typed enum `enum name { field_name1(T), field_name2 }"
      "\n  - typed enum `enum Option { Valid(T), Invalid }";
  (void)p.match(token::ETokenKind::ENUM);

  auto& enu  = p.add_get_node<ast::Global_Enum>(p.peek().tokid);
  enu.name   = p.parse_name("", hint);
  auto defid = p.add_definition(enu.nodeid);
  p.enter_scope(enu, "enum " + std::string(enu.name));

  (void)p.expect(63, token::ETokenKind::L_CURLY, "Expected start enum block '{' after enum name declaration.", hint);
  std::vector<type::ID> factory_variants;

  size_t count = 0;
  while (!p.is_end()) {
    auto& field    = p.add_get_node<ast::Enum_Field>(p.peek().tokid);
    field.name     = p.parse_name("", hint);
    field.position = count++;

    field.type = p.p_type->parse_type();
    factory_variants.emplace_back(field.type);

    enu.variants.emplace_back(field.nodeid);
    if (p.match_field_separator(token::ETokenKind::COMMA, token::ETokenKind::R_CURLY)) break;
  }

  p.exit_scope();

  (void)parser_type_factory.make_enum(factory_variants, defid);

  return enu.nodeid;
}

ast::ID parser::Parser_Declaration::_union()
{
  constexpr std::string_view hint = "define union like: `union name { field_name1: T, field_name2: U, ... }";
  (void)p.match(token::ETokenKind::UNION);

  auto& _union = p.add_get_node<ast::Global_Union>(p.peek().tokid);
  _union.name  = p.parse_name("", hint);
  auto defid   = p.add_definition(_union.nodeid);
  p.enter_scope(_union, "union " + std::string(_union.name));

  (void)p.expect(182, token::ETokenKind::L_CURLY, "Expected start union block '{' after union name declaration.", hint);

  std::vector<type::ID> factory_types;

  while (!p.is_end()) {
    auto& field = p.add_get_node<ast::Union_Field>(p.peek().tokid);
    field.name  = p.parse_name("", hint);

    (void)p.expect(183, token::ETokenKind::COLON, "Expected colon ':' after union field name declaration.", hint);

    field.type = p.p_type->parse_type();

    _union.variants.emplace_back(field.nodeid);
    factory_types.emplace_back(field.type);

    if (p.match_field_separator(token::ETokenKind::COMMA, token::ETokenKind::R_CURLY)) break;
  }

  p.exit_scope();

  (void)parser_type_factory.make_union(factory_types, defid);

  return _union.nodeid;
}

ast::ID parser::Parser_Declaration::flag()
{
  constexpr std::string_view hint = "define flag like: `flag name { flag1, flag2, ... }";
  (void)p.match(token::ETokenKind::FLAG);

  auto& flag = p.add_get_node<ast::Global_Flag>(p.peek().tokid);
  flag.name  = p.parse_name("", hint);

  auto defid = p.add_definition(flag.nodeid);
  p.enter_scope(flag, "flag " + std::string(flag.name));

  (void)p.expect(184, token::ETokenKind::L_CURLY, "Expected start flag block '{' after flag name declaration.", hint);

  while (!p.is_end()) {
    auto& field = p.add_get_node<ast::Flag_Field>(p.peek().tokid);
    field.name  = p.parse_name("", hint);

    flag.flags.emplace_back(field.nodeid);

    if (p.match_field_separator(token::ETokenKind::COMMA, token::ETokenKind::R_CURLY)) break;
  }

  p.exit_scope();

  (void)parser_type_factory.make_flag(flag.flags.size(), defid);

  return flag.nodeid;
}

ast::ID parser::Parser_Declaration::global_variable()
{
  constexpr std::string_view hint =
      R"(define global variable like:"
  - mutable : `var name: R = expression` `var name = expression`"
  - immutable : `let name: T = expression` `let name = expression`"
  - constant (compiletime value): `const name: T = expression` `const name = expression`"
  - garbage memory value: `var name: T = uninit` `var name = uninit`"
  - external mutable :"
   `# extern"
    var name: T`"
  - external immutable :"
   `# extern"
    let name: T`)";

  auto&              tok  = p.expect_any(65, {token::ETokenKind::LET, token::ETokenKind::VAR, token::ETokenKind::CONST},
                                         "Expected global variable declaration token", hint);
  ast::EVariableKind kind = ast::ETokenKind_to_EVariableKind(tok.kind);

  auto& var      = p.add_get_node<ast::Global_Variable>(p.peek().tokid);
  var.kind       = kind;
  var.name       = p.parse_name("", hint);
  var.extern_abi = p.extern_abi;

  (void)p.add_definition(var.nodeid);

  if (var.name.empty()) {
    p.add_error(66, "Invalid Identifier !", "");
  }

  bool is_inferred_type = false;

  // explicit type case
  if (p.match(token::ETokenKind::COLON)) var.type = p.p_type->parse_type();
  // auto deduce type case
  else
    is_inferred_type = true;

  // check affectation
  if (p.match(token::ETokenKind::ASSIGN)) {
    auto& assign_tok = p.peek(-1);
    var.assignment   = ast::ETokenKind_to_ETransfertType(assign_tok.kind);
    if (p.match(token::ETokenKind::L_UNINIT)) {
      var.is_uninit = true;
    } else {
      auto expr      = p.p_expr->parse_expression();
      var.expression = expr;
    }
  }

  if (var.is_uninit && var.kind != ast::EVariableKind::_var) {
    p.add_error(67, "Illegal uninit variable with a const or immutable status", hint);
  }

  if (var.assignment == ast::ETransfertType::NONE && is_inferred_type)
    p.add_error(68, "Expected assignation '=' in auto inferred variable type.",
                "define auto inferred variable like `let myName = expression;`");


  return var.nodeid;
}

ast::ID parser::Parser_Declaration::function()
{
  constexpr std::string_view hint =
      R"(define function like:
  - definition `fn myName() { ... }`
  - definition with return `fn myName() -> i32 { ... }`.
  - metacode allowed `# pure`)";

  constexpr std::string_view main_hint =
      R"(define main function like:
  - implicit return `fn main() {...}`
  - explicit return `fn main() -> s32 {}`
  - with args `fn main(copy argc: s32, ref argv: cstr)`)";

  (void)p.match(token::ETokenKind::FUNCTION);

  auto tok_pos = p.tok_to_pos(p.peek().tokid);

  auto&      fn        = p.add_get_node<ast::Global_Function>(p.peek().tokid);
  const auto old_ret   = p.current_returnable;
  p.current_returnable = fn.nodeid;

  fn.name    = p.parse_name("", hint);
  fn.is_pure = p.metablock_contains(tok_pos, "pure");

  fn.extern_abi = p.extern_abi;

  (void)p.add_definition(fn.nodeid);
  p.enter_scope(fn, "function " + std::string(fn.name));

  auto [protoid, params] = p.p_type->prototype_from_declaration();
  fn.prototype           = protoid;
  fn.parameters          = params;

  if (fn.name == "main") {
    auto* proto = fn.prototype.as<type::Prototype>();
    if (proto->is_explicit_ret) {
      if (proto->ret != type::TYPEID_s32)
        p.add_error(277, "The program entry function \"main\" must returns only s32 type",
                    "define main function like:\n  - implicit ret `fn main() {...}`\n");
    }
    proto->ret = type::TYPEID_s32;
  }

  // if extern : no definition
  if (!fn.extern_abi.empty() && p.check(token::ETokenKind::L_CURLY))
    p.add_error(69, "Unexpected start code block '{' after a extern function declaration", main_hint);

  if (fn.extern_abi.empty()) fn.codeblock = p.p_loc->parse_codeblock_instruction();

  p.exit_scope();

  p.current_returnable = old_ret;

  return fn.nodeid;
}


ast::ID parser::Parser_Declaration::generic()
{
  constexpr std::string_view kHint_gen = "define geneneric like `gen name<T, ...> { ... }`.";
  constexpr std::string_view kHint_filter =
      R"(define geneneric filter like:"
  - operator       `T op +;`"
  - cast to        `T cast to U;`"
  - cast from      `T cast from U;`"
  - view           `T view Printable;`"
  - extension `T extend name;`"
  - rule         `T rule Move;`"
  - facet      `T facet Position;`"
  - typealias      `T type len;`"
  - nested filter  `T is gen::base_of<Animal>;`)";

  auto& gen = p.add_get_node<ast::Global_Generic>(p.peek().tokid);
  (void)p.match(token::ETokenKind::GENERIC);

  gen.name = p.parse_name("", kHint_gen);
  (void)p.add_definition(gen.nodeid);
  p.enter_scope(gen, "generic " + std::string(gen.name));
  (void)p.expect(70, token::ETokenKind::L_ANGLE, "Expected start type '<' after generic name.", kHint_gen);

  while (!p.is_end()) {
    gen.typenames.emplace_back(p.parse_name("", kHint_gen));
    if (p.match_field_separator(token::ETokenKind::S_END_OF_FILE, token::ETokenKind::R_ANGLE)) break;
  }

  (void)p.expect(72, token::ETokenKind::L_CURLY, "Expected start code block '{' after generic declaration.", kHint_gen);

  while (!p.is_end()) {
    if (p.match(token::ETokenKind::R_CURLY)) break;
    auto firstok = p.parse_name("", kHint_filter);

    // case: T op ...
    if (p.match(token::ETokenKind::OP)) {
      auto& gen_op          = p.add_get_node<ast::Generic_Op>(p.peek().tokid);
      gen_op.target_gen_sym = firstok;
      auto& tok_op          = p.expect_any(74, token::k_operator,
                                           "Expected operator after 'op' keyword in generic filter argument.", kHint_filter);
      gen_op.op_ty          = ast::ETokenKind_to_EOp_Bin(tok_op.kind);

      gen.gen_conds.emplace_back(gen_op.nodeid);
      (void)p.match(token::ETokenKind::SEMICOLON);
    }
    // case: T facet ...
    else if (p.match(token::ETokenKind::FACET)) {
      auto& facet          = p.add_get_node<ast::Generic_Facet>(p.peek().tokid);
      facet.target_gen_sym = firstok;
      facet.facet          = p.p_expr->parse_expression();

      gen.gen_conds.emplace_back(facet.nodeid);
    }
    // case: T view ...
    else if (p.match(token::ETokenKind::VIEW)) {
      auto& view          = p.add_get_node<ast::Generic_View>(p.peek().tokid);
      view.target_gen_sym = firstok;
      view.view           = p.p_expr->parse_expression();

      gen.gen_conds.emplace_back(view.nodeid);
    }
    // case: T extend ...
    else if (p.match(token::ETokenKind::EXTENSION)) {
      auto& extend          = p.add_get_node<ast::Generic_Extension>(p.peek().tokid);
      extend.target_gen_sym = firstok;
      extend.extension      = p.p_expr->parse_expression();

      gen.gen_conds.emplace_back(extend.nodeid);
    }
    // case: T rule ...
    else if (p.match(token::ETokenKind::RULE)) {
      auto& rule          = p.add_get_node<ast::Generic_Rule>(p.peek().tokid);
      rule.target_gen_sym = firstok;
      rule.rule           = p.p_expr->parse_expression();

      gen.gen_conds.emplace_back(rule.nodeid);
    }
    // case: T is i32 | type::floating | ...
    else if (p.match(token::ETokenKind::IS)) {
      auto& nested           = p.add_get_node<ast::Generic_Type>(p.peek().tokid);
      nested.source_typename = firstok;

      while (!p.is_end()) {
        nested.in_type.emplace_back(p.p_type->parse_type());

        if (p.match(token::ETokenKind::PIPE)) continue;
        break;
      }

      gen.gen_conds.emplace_back(nested.nodeid);
    }
    // case: T AS to/from ...
    else if (p.match(token::ETokenKind::AS)) {
      auto& castNode           = p.add_get_node<ast::Generic_Cast>(p.peek().tokid);
      castNode.source_typename = firstok;

      if (!p.check_val("to") && !p.check_val("from")) {
        p.add_error(75, "Expected cast way 'to' or 'from' in generic filter argument.", kHint_filter);
      }

      castNode.target = p.p_type->parse_type();

      gen.gen_conds.emplace_back(castNode.nodeid);
    } else
      p.add_error(76, "Expected generic condition 'use' or 'is' in generic filter argument.", kHint_filter);

    (void)p.match(token::ETokenKind::SEMICOLON);
  }

  p.exit_scope();
  return gen.nodeid;
}

ast::ID parser::Parser_Declaration::type_alias()
{
  constexpr std::string_view hint =
      R"(define type alias like:
  - type `type myAlias = i32`.
  - opaque `type myAlias = opaque`.
  - generic `type Vec<T> = core::container::vector<T>`.
  - generic `type StrList = core::container::vector<str>`.)";

  auto& type_alias = p.add_get_node<ast::Global_Alias_Type>(p.peek().tokid);

  (void)p.match(token::ETokenKind::TYPE);

  type_alias.alias = p.parse_name();

  // if alias, else it's a opaque type
  (void)p.expect(211, token::ETokenKind::ASSIGN, "expected typealias assignation '='.", hint);

  type_alias.type = p.p_type->parse_type();

  auto defid = p.add_definition(type_alias.nodeid);

  // define canonical alias
  p.CU.types->add_canon(type_alias.alias, type_alias.type);

  return type_alias.nodeid;
}
