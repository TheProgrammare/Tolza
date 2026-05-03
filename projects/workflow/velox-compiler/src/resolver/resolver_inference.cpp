
#include "resolver_inference.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <memory>

#include "ast/ast_base.hpp"
#include "compiler/compiler.hpp"
#include "nexus/ast/ast.hpp"

#include "nexus/forward.hpp"
#include "nexus/scope.hpp"
#include "nexus/symbol.hpp"
#include "nexus/type.hpp"
#include "nexus/inference.hpp"
#include "nexus/type.hpp"
#include "nexus/script.hpp"
#include "nexus/resolved.hpp"

#include "nexus/ast/ast.hpp"
#include "ast/ast_declaration_global.hpp"
#include "ast/ast_declaration_local.hpp"
#include "ast/ast_declaration_cop.hpp"
#include "ast/ast_generic.hpp"
#include "ast/ast_literal.hpp"
#include "ast/ast_memory.hpp"
#include "ast/ast_operation.hpp"
#include "ast/ast_statement.hpp"
#include "ast/ast_expression.hpp"

ast::Node& resolver::Inference::Inference::get_node(ast::_id n) const
{
  return scr_info.nodes->get(n);
}

ast::Node& resolver::Inference::Inference::get_node(ast::_gnid n) const
{
  return compiler::COMPILER.nodes.get(n);
}

ast::_gnid resolver::Inference::Inference::make_gnid(ast::Node& n) const
{
  return ast::GNID_Factory::make_gnid(scr_info.id, n.node_id.get_node_id());
}

bool resolver::Inference::Inference::is_inferred(ast::Node& n) const
{
  return compiler::COMPILER.inference.is_inferred(make_gnid(n));
}

bool resolver::Inference::Inference::is_inferred(ast::_gnid n) const
{
  return compiler::COMPILER.inference.is_inferred(n);
}


type::_id resolver::Inference::Inference::get_type_id(ast::Node& n) const
{
  return compiler::COMPILER.inference.get_inference(make_gnid(n));
}

type::_id resolver::Inference::Inference::get_type_id(ast::_gnid n) const
{
  return compiler::COMPILER.inference.get_inference(n);
}

const type::Type& resolver::Inference::Inference::get_type(ast::Node& n) const
{
  auto id = get_type_id(make_gnid(n));
  return compiler::COMPILER.types.get(id);
}

const type::Type& resolver::Inference::Inference::get_type(ast::_gnid n) const
{
  auto id = get_type_id(n);
  return compiler::COMPILER.types.get(id);
}


void resolver::Inference::Inference::add_inference(ast::Node& n, type::_id type) const
{
  compiler::COMPILER.inference.add(make_gnid(n), type);
}

void resolver::Inference::Inference::resolve_Node(ast::_gnid& n, bool mandatory)
{
  if (!n) return;
  auto& node = scr_info.nodes->get(n.get_node_id());
  resolve_Node(node, mandatory);
}


void resolver::Inference::Inference::resolve_Node(ast::Node& n, bool mandatory)
{
#define n_case(kind)                                                                                                   \
  case ast::ENodeKind::kind: resolve_##kind(*scr_info.nodes->get_as<ast::kind>(n.node_id.get_node_id())); break

  switch (n.kind()) {
    n_case(ID);
    n_case(ID_Qualified);
    n_case(ID_Typed);
    n_case(Global_Variable);
    n_case(Global_Function);
    n_case(Local_Pattern_Enum);
    n_case(Local_Pattern_Tuple);
    n_case(Local_Pattern_Entity);
    n_case(Local_Pattern_Sys_Comp);
    n_case(Local_Pattern_Comp);
    n_case(Local_Binding);
    n_case(Local_Variable);
    n_case(Literal_Boolean);
    n_case(Literal_Integral);
    n_case(Literal_Fixed_Point);
    n_case(Literal_Floating_Point);
    n_case(Literal_Cune);
    n_case(Literal_Rune);
    n_case(Literal_Text_Pure);
    n_case(Literal_Text_Interpolation);
    n_case(Literal_Textual_Format);
    n_case(Literal_Table);
    n_case(Literal_Map);
    n_case(Literal_Tuple);
    n_case(Literal_Range);
    n_case(Literal_Iterator);
    n_case(Literal_Enum);
    n_case(Literal_Structured_Data);
    n_case(Literal_Entity);
    n_case(Expression_If_Ternary);
    n_case(Expression_Member_Access);
    n_case(Expression_Self);
    n_case(Expression_Other);
    n_case(Expression_Call);
    n_case(Expression_Call_Argument);
    n_case(Expression_Call_System);
    n_case(Expression_Call_Pipe);
    n_case(Expression_Table_Access);
    n_case(Expression_Ptr_Val);
    n_case(Expression_Mut_Of);
    n_case(Expression_Ref_Of);
    n_case(Expression_Move_Of);
    n_case(Expression_Copy_Of);
    n_case(Expression_Addr_Of);
    n_case(Expression_Size_Of);
    n_case(Expression_GetBits);
    n_case(Expression_New_Ptr);
    n_case(Expression_Get_Type);
    n_case(Statement_For);
    n_case(Statement_Return);
    n_case(Operation_Is);
    n_case(Operation_In);
    n_case(Operation_Assignment);
    n_case(Operation_Binary);
    n_case(Operation_Unary);
    n_case(Operation_Interval);
  default: assert(!mandatory);
  }

#undef n_case
}


void resolver::Inference::ensure_expression_resolution(ast::Node& p_expr, type::_id p_type_inference)
{
  // standalone expression inference
  if (!p_type_inference) {
    resolve_Node(p_expr);
    return;
  }

  if (ast::ENodeKind_is_literal(p_expr.kind()) && is_lazy_literal(p_expr)) {
    // primitive literal can be lazy type inference
    if (auto ty = compiler::COMPILER.types.get_as<type::Type::PrimitiveData>(p_type_inference)) {
      if (p_expr.kind() == ast::ENodeKind::Literal_Integral && !type::EPrimitiveTypeKind_is_integral(ty->primitive))
        goto bad_inference;
      else if (p_expr.kind() == ast::ENodeKind::Literal_Floating_Point
               && !type::EPrimitiveTypeKind_is_floating(ty->primitive))
        goto bad_inference;
      else if (p_expr.kind() == ast::ENodeKind::Literal_Fixed_Point
               && !type::EPrimitiveTypeKind_is_fixed(ty->primitive))
        goto bad_inference;
      else
        goto bad_inference;
    } else if (auto ty = compiler::COMPILER.types.get_as<type::Type::StringData>(p_type_inference)) {
      if (p_expr.kind() != ast::ENodeKind::Literal_Text_Pure) goto bad_inference;
    } else
      goto bad_inference;

    add_inference(p_expr, p_type_inference);
  } else {
    resolve_Node(p_expr);
  }

  // no type to infer
  if (!p_type_inference) return;

  if (!is_inferred(p_expr)) {
    add_error(234, p_expr, "Impossible to define the expression type.", "");
    return;
  }

bad_inference:
  if (!p_type_inference) return;

  // check if type inferred is compatible to the expected inference (or expected type)
  if (get_type_id(p_expr) != p_type_inference) {
    auto& type = compiler::COMPILER.types.get(p_type_inference);
    if (type::ETypeKind_is_user_defined(type.kind)) {
      auto& sym = compiler::COMPILER.symbols.get(type.get_sym_id());
      auto& n   = compiler::nodes.get(sym.gnid);
      add_error_two_nodes(230, p_expr, n, "Illegal type inference.", "");
    } else {
      add_error(230, p_expr, "Illegal type inference.", "");
    }
  }
}

bool resolver::Inference::is_lazy_literal(ast::Node& n) const
{
  if (auto ptr = scr_info.nodes->get_as<ast::Literal_Integral>(n.node_id.get_node_id())) {
    return ptr->type == type::EPrimitiveTypeKind::NONE;
  } else if (auto ptr = scr_info.nodes->get_as<ast::Literal_Floating_Point>(n.node_id.get_node_id())) {
    return ptr->type == type::EPrimitiveTypeKind::NONE;
  } else if (auto ptr = scr_info.nodes->get_as<ast::Literal_Fixed_Point>(n.node_id.get_node_id())) {
    return ptr->raw_type == type::EPrimitiveTypeKind::NONE;
  } else if (auto ptr = scr_info.nodes->get_as<ast::Literal_Text_Pure>(n.node_id.get_node_id())) {
    return ptr->text_type == type::ETextType::NONE;
  } else
    return false;
}


bool resolver::Inference::start_resolver()
{
  auto root = scr_info.nodes->get_as<ast::Root>(scr_info.root_node_id.get_node_id());
  assert(root);

  for (auto& item : root->global_nodes) {
    resolve_Node(item);
  }

  return true;
}


void resolver::Inference::resolve_ID(ast::ID& n)
{
  if (is_inferred(n)) return;

  auto sym_id = compiler::COMPILER.resolved.get_symbol(make_gnid(n));
  assert(sym_id);

  auto& sym   = compiler::COMPILER.symbols.get(sym_id);
  auto& n_sym = compiler::COMPILER.nodes.get(sym.gnid);

  resolve_Node(n_sym);

  assert(!ast::ENodeKind_is_declaration(n_sym.kind()));

  add_inference(n, sym.type);
}
void resolver::Inference::resolve_ID_Qualified(ast::ID_Qualified& n)
{
  if (is_inferred(n)) return;

  auto sym_id = compiler::COMPILER.resolved.get_symbol(make_gnid(n));
  assert(sym_id);

  auto& sym   = compiler::COMPILER.symbols.get(sym_id);
  auto& n_sym = compiler::COMPILER.nodes.get(sym.gnid);

  resolve_Node(n_sym);

  assert(!ast::ENodeKind_is_declaration(n_sym.kind()));

  add_inference(n, sym.type);
}
void resolver::Inference::resolve_ID_Typed(ast::ID_Typed& n)
{
  if (is_inferred(n)) return;

  auto sym_id = compiler::COMPILER.resolved.get_symbol(make_gnid(n));
  assert(sym_id);

  auto& sym   = compiler::COMPILER.symbols.get(sym_id);
  auto& n_sym = compiler::COMPILER.nodes.get(sym.gnid);

  resolve_Node(n_sym);

  assert(!ast::ENodeKind_is_declaration(n_sym.kind()));

  add_inference(n, sym.type);
}

// expression inferred type;
void resolver::Inference::resolve_Local_Pattern_Enum(ast::Local_Pattern_Enum& n)
{
  /*
  if (is_inferred(n)) return;

  resolve_Node(n.expression);

  auto& expr_ty = get_type(n.expression);

  if (expr_ty.kind != type::ETypeKind::Enum) {
    add_error(156, get_node(n.expression), "Expected enum type.", "");
    return;
  }

  auto& enum_data = std::get<type::Type::EnumData>(expr_ty.data);
  auto& enum_sym  = compiler::COMPILER.symbols.get(expr_ty.sym);
  auto  enum_decl = compiler::COMPILER.nodes.get_as<ast::Global_Enum>(enum_sym.node_id);

  auto enum_target = compiler::COMPILER.resolved.get_symbol(make_gnid(get_node(n.name)));


  if (enum_sym.id != enum_target) {
    add_error_two_nodes(245, *enum_decl, get_node(n.name),
                        "Different enum used in pattern between name and expression type.", "");
    return;
  }

  size_t max_payload = 0;
  size_t min_payload = -1;

  for (auto elem : enum_data.variants) {
    max_payload = elem.size() > max_payload ? max_payload = elem.size() : max_payload;
    min_payload = elem.size() < min_payload ? min_payload = elem.size() : min_payload;
  }

  if (max_payload < n.pattern_elements.size())
    add_error_two_nodes(243, get_node(n.expression), *enum_decl,
                        "Too much pattern elements specified for any enum variants.", "");
  else if (min_payload > n.pattern_elements.size())
    add_error_two_nodes(244, n, *enum_decl, "Too few pattern elements specified for any enum variants.", "");

  size_t count = 0;
  for (auto elem : n.pattern_elements) {
    resolve_Node(elem);

    auto n_elem = scr_info.nodes->get_as<ast::Local_Pattern_Element>(elem);
    assert(!n_elem);

    switch (n_elem->kind) {
    case ast::Local_Pattern_Element::Kind::Ignore:  count++; continue;
    case ast::Local_Pattern_Element::Kind::Binding: {
      resolve_Node(n_elem->bind);
      ensure_expression_resolution(get_node(n_elem->bind, enum_data.variants[0]),
    }
    case ast::Local_Pattern_Element::Kind::Literal: break;
    }
    if (n_elem->kind) auto& elem_ty = get_type(elem);

    if (elem_ty !=) }

  if (auto ptr = std::dynamic_pointer_cast<ast::Enum_Element>(n.name->expression_inferred_type->resolve())) {
    if (n.mapping.size() != ptr->types.size()) {
      add_error_two_nodes(155, n, *ptr, "Inequal types binded on " + n.name->debug_str(), "");
    } else {
      for (size_t i = 0; i < n.mapping.size(); i++) {
        auto& map = n.mapping[i];
        auto& ty  = ptr->types[i];

        if (map->kind != ast_loc::Pattern_Element::Kind::Binding) continue;

        if (auto map_ptr = dynamic_cast<ast_loc::Variable_Binding*>(map->node())) {
          map_ptr->type = ty;
        }
      }
    }
  }
  add_error(156, *n.name, "Unexpected reference encounted, expected Enum element", "");
  */
}
void resolver::Inference::resolve_Local_Pattern_Tuple(ast::Local_Pattern_Tuple& n)
{
  /*
  if (is_inferred(n)) return;

  if (auto ptr = std::dynamic_pointer_cast<ast::type::Tuple>(n.right->expression_inferred_type->resolve())) {
    if (n.mapping.size() != ptr->types.size()) {
      add_error_two_nodes(157, n, *ptr, "Inequal types binded on " + ptr->debug_str(), "");
    } else {
      for (size_t i = 0; i < n.mapping.size(); i++) {
        auto& map = n.mapping[i];
        auto& ty  = ptr->types[i];

        if (map->kind != ast_loc::Pattern_Element::Kind::Binding) continue;

        if (auto map_ptr = dynamic_cast<ast_loc::Variable_Binding*>(map->node())) {
          map_ptr->type = ty;
        }
      }
    }
  } else {
    add_error(158, *n.right, "Expected tuple type", "");
  }
    */
}
void resolver::Inference::resolve_Local_Pattern_Entity(ast::Local_Pattern_Entity& n)
{
  /*
  if (is_inferred(n)) return;

  if (auto ptr = std::dynamic_pointer_cast<ast::cop::Entity>(n.name->expression_inferred_type->resolve())) {
    for (auto& pat_comp : n.mapping) {
      bool found = false;
      for (auto& et_comp : ptr->comps) {
        if (et_comp->name == pat_comp->name) {
          pat_comp->expression_inferred_type = et_comp->expression_inferred_type->resolve();
          found                              = true;
          break;
        }
      }

      if (!found) add_error(159, *pat_comp, "Component is not in the entity composition", "");
    }
  } else {
    add_error(160, *n.name, "Expected entity type", "");
  }
    */
}
void resolver::Inference::resolve_Local_Pattern_Comp(ast::Local_Pattern_Comp& n)
{
  /*
  if (is_inferred(n)) return;

  if (auto ptr = std::dynamic_pointer_cast<ast::cop::Component>(n.name->expression_inferred_type)) {
    for (auto& [field_name, pattern] : n.mapping) {
      bool found = false;
      for (auto& comp_field : ptr->fields) {
        if (field_name == comp_field->declaration_name) {

          if (pattern->kind != ast_loc::Pattern_Element::Kind::Binding) continue;

          if (auto map_ptr = dynamic_cast<ast_loc::Variable_Binding*>(pattern->node())) {
            map_ptr->type = comp_field->type;
          }

          found = true;
          break;
        }
      }

      if (!found) add_error(161, *n.name, "Field \"" + field_name + "\" not in the component definition", "");
    }
  } else {
    add_error(162, *n.name, "Expected component type", "");
  }
    */
}

void resolver::Inference::resolve_Local_Pattern_Sys_Comp(ast::Local_Pattern_Sys_Comp& n)
{
}


void resolver::Inference::resolve_Global_Variable(ast::Global_Variable& n)
{
  if (is_inferred(n)) return;

  if (!n.type) {
    resolve_Node(n.expression);
    n.type = get_type_id(n.expression);
  } else {
    ensure_expression_resolution(get_node(n.expression), n.type);
  }

  add_inference(n, n.type);
}

void resolver::Inference::resolve_Global_Function(ast::Global_Function& n)
{
  if (is_inferred(n)) return;

  auto proto = compiler::COMPILER.types.get_as<type::Type::PrototypeData>(n.prototype);

  if (n.name == "main") {
    if (!proto->ret || proto->ret == type::TYPEID_u0) {
      proto->ret = type::TYPEID_u0;
    } else if (proto->ret != type::TYPEID_i32) {
      add_error(218, n, "Violation of the main function convention, main must return i32 or u0 type.", "");
    }
  }

  // potential function return type inference
  if (proto->ret == type::TYPEID_u0 && !n.is_explicit_ret_type) {
    type::_id return_ty;

    auto cb = scr_info.nodes->get_as<ast::Local_CodeBlock>(n.codeblock.get_node_id());

    for (auto& elem : cb->elements) {
      if (auto ret = scr_info.nodes->get_as<ast::Statement_Return>(elem.get_node_id())) {

        resolve_Node(*ret);
        return_ty = get_type_id(*ret);
        continue;
      }
    }

    proto->ret = return_ty;
  }

  add_inference(n, proto->ret);
}

void resolver::Inference::resolve_Local_Variable(ast::Local_Variable& n)
{
  if (is_inferred(n)) return;

  if (!n.type) {
    resolve_Node(n.expression);
    n.type = get_type_id(n.expression);
  } else {
    ensure_expression_resolution(get_node(n.expression), n.type);
  }

  add_inference(n, n.type);
}
void resolver::Inference::resolve_Local_Binding(ast::Local_Binding& n)
{
  if (is_inferred(n)) return;

  if (!n.type) {
    resolve_Node(n.expression);
    n.type = get_type_id(n.expression);
  } else {
    ensure_expression_resolution(get_node(n.expression), n.type);
  }

  add_inference(n, n.type);
}

void resolver::Inference::resolve_Statement_Return(ast::Statement_Return& n)
{
  if (is_inferred(n)) return;

  auto check_ret = [&](ast::_gnid gnid) {
    resolve_Node(get_node(gnid));

    if (!n.value && get_type_id(gnid) == type::TYPEID_u0) return;
    if (!n.value && get_type_id(gnid) != type::TYPEID_u0) {
      add_error_two_nodes(246, n, get_node(gnid),
                          "Invalid void return, expression needed, the function returns values.", "");
      return;
    }

    ensure_expression_resolution(get_node(n.value), get_type_id(gnid));
    add_inference(n, get_type_id(gnid));
  };

  if (auto fn_gnid = compiler::COMPILER.scopes.tools.get_scope_node(ast::ENodeKind::Global_Function, n.scope_id)) {
    check_ret(fn_gnid);
  } else if (auto lam_gnid = compiler::COMPILER.scopes.tools.get_scope_node(ast::ENodeKind::Local_Lambda, n.scope_id)) {
    check_ret(lam_gnid);
  } else if (auto sys_gnid = compiler::COMPILER.scopes.tools.get_scope_node(ast::ENodeKind::COP_System, n.scope_id)) {
    check_ret(sys_gnid);
  } else
    assert(false);
}


void resolver::Inference::resolve_Expression_If_Ternary(ast::Expression_If_Ternary& n)
{
  if (is_inferred(n)) return;

  resolve_Node(n.evaluator);
  resolve_Node(n.statement_true);
  resolve_Node(n.statement_false);

  ensure_expression_resolution(get_node(n.left), get_type_id(n.statement_true));
  if (n.statement_false) ensure_expression_resolution(get_node(n.left), get_type_id(n.statement_false));

  add_inference(n, get_type_id(n.statement_true));
}

void resolver::Inference::resolve_Expression_Member_Access(ast::Expression_Member_Access& n)
{
  if (is_inferred(n)) return;

  resolve_Node(n.left_expression);
  resolve_Node(n.right_identifier);

  add_inference(n, get_type_id(n.right_identifier));
}
void resolver::Inference::resolve_Expression_Self(ast::Expression_Self& n)
{
  if (is_inferred(n)) return;

  auto entity_gnid = compiler::COMPILER.scopes.tools.get_scope_node(ast::ENodeKind::COP_Entity, n.scope_id);
  assert(entity_gnid);

  auto entity = compiler::COMPILER.nodes.get_as<ast::COP_Entity>(entity_gnid);
  assert(!entity);

  resolve_Node(*entity);

  add_inference(n, get_type_id(*entity));
}
void resolver::Inference::resolve_Expression_Other(ast::Expression_Other& n)
{
  if (is_inferred(n)) return;
}
void resolver::Inference::resolve_Expression_Call_Argument(ast::Expression_Call_Argument& n)
{
  if (is_inferred(n)) return;
}
void resolver::Inference::resolve_Expression_Call(ast::Expression_Call& n)
{
  if (is_inferred(n)) return;

  resolve_Node(n.callee);
  /*
    auto ty_id = get_type_id(n.callee);

    auto& call_ty = compiler::COMPILER.types.get(ty_id);
    auto  sym_id  = call_ty.get_sym_id();
    assert(sym_id);

    std::vector<ast::Local_Parameter*> parameters;
    type::Type::PrototypeData*         proto = nullptr;

    auto init_types = [&](type::_id type, std::vector<ast::_gnid> params) {
      proto = compiler::COMPILER.types.get_as<type::Type::PrototypeData>(type);
      assert(proto);

      parameters.reserve(params.size());
      for (auto param : params) {
        auto n = compiler::COMPILER.nodes.get_as<ast::Local_Parameter>(param);
        parameters.push_back(n);
      }
    };

    auto& sym = compiler::COMPILER.symbols.get(sym_id);
    if (auto fn_def = compiler::COMPILER.nodes.get_as<ast::Global_Function>(sym.gnid)) {
      init_types(fn_def->prototype, fn_def->parameters);
    } else if (auto lam_def = compiler::COMPILER.nodes.get_as<ast::Local_Lambda>(sym.gnid)) {
    }

    if (auto proto = compiler::COMPILER.nodes.get_as<ast::Global_Function>())


      auto sym_id = compiler::COMPILER.resolved.get_symbol(n.callee);
    auto& sym = compiler::COMPILER.symbols.get(sym_id);

    if (auto fn = compiler::COMPILER.symbols.get(_id id).get_as<ast::Global_Function>(n.callee))

      if (auto fn_ty = dynamic_cast<ast::Function*>(n.function_symbol.get())) {
        n.function_proto           = fn_ty->prototype;
        n.expression_inferred_type = fn_ty->prototype->return_ty->resolve();
      } else {
        add_error(193, *n.callee, "Expected fuction type in inferred type", "");
      }

    // resolve parameters
    size_t arg_count = 0;
    bool   name_mode = false;
    for (auto& arg : n.param_args) {
      // argument inferred type is equal to his expression inferred type
      // argument type on call is checked in semantic resolver

      // unamed argument encounted after a named argument
      if (!arg->name.empty() && name_mode) {
        add_error(212, *arg, "Unexpected unamed argument after a named argument.",
                  "Named arguments are always the latest call arguments");
        continue;
      }

      // named argument encounted
      if (!arg->name.empty()) {
        name_mode = true;

        // find the corresponding parameter
        bool found = false;
        for (auto& param : n.function_proto->parameters) {
          if (param->declaration_name == arg->name) {
            arg->fn_param_type = param;
            arg->accept(*this);

            found = true;
            break;
          }
        }

        if (!found) {
          add_error_two_nodes(211, *arg, n, "The paramater name invoked \"" + arg->name + "\" dosen't exists.", "");
          continue;
        }
      }
      // unamed argument encounted : positional parameter
      else if (n.function_proto->parameters.size() > arg_count) {
        auto param         = n.function_proto->parameters[arg_count++];
        arg->fn_param_type = param;
        arg->accept(*this);
      }
      // unamed argument encounted out of param size but variadic function
      else if (n.function_proto->is_variadic) {
        arg->fn_param_type = nullptr;
        arg->variadic_arg  = true;
        arg->accept(*this);
      }
      // unamed argument encounted out of param size : not variadic function
      else {
        add_error_two_nodes(213, *arg, *n.function_symbol, "Too many arguments invoked.", "");
      }
    }
    for (auto& elem : n.gen_args) elem->accept(*this);
    */
}
void resolver::Inference::resolve_Expression_Call_Pipe(ast::Expression_Call_Pipe& n)
{
  if (is_inferred(n)) return;
}
void resolver::Inference::resolve_Expression_Call_System(ast::Expression_Call_System& n)
{
  if (is_inferred(n)) return;
}
void resolver::Inference::resolve_Expression_Table_Access(ast::Expression_Table_Access& n)
{
  if (is_inferred(n)) return;
}
void resolver::Inference::resolve_Expression_Ptr_Val(ast::Expression_Ptr_Val& n)
{
  if (is_inferred(n)) return;
}
void resolver::Inference::resolve_Expression_Mut_Of(ast::Expression_Mut_Of& n)
{
  if (is_inferred(n)) return;

  resolve_Node(n.target);
  add_inference(n, get_type_id(n.target));
}
void resolver::Inference::resolve_Expression_Ref_Of(ast::Expression_Ref_Of& n)
{
  if (is_inferred(n)) return;

  resolve_Node(n.target);
  add_inference(n, get_type_id(n.target));
}
void resolver::Inference::resolve_Expression_Addr_Of(ast::Expression_Addr_Of& n)
{
  if (is_inferred(n)) return;

  resolve_Node(n.target);
  add_inference(n, get_type_id(n.target));
}
void resolver::Inference::resolve_Expression_Size_Of(ast::Expression_Size_Of& n)
{
  if (is_inferred(n)) return;

  resolve_Node(n.target);
  add_inference(n, get_type_id(n.target));
}
void resolver::Inference::resolve_Expression_GetBits(ast::Expression_GetBits& n)
{
  if (is_inferred(n)) return;

  resolve_Node(n.target);
  add_inference(n, get_type_id(n.target));
}
void resolver::Inference::resolve_Expression_Move_Of(ast::Expression_Move_Of& n)
{
  if (is_inferred(n)) return;

  resolve_Node(n.target);
  add_inference(n, get_type_id(n.target));
}
void resolver::Inference::resolve_Expression_Copy_Of(ast::Expression_Copy_Of& n)
{
  if (is_inferred(n)) return;

  resolve_Node(n.target);
  add_inference(n, get_type_id(n.target));
}
void resolver::Inference::resolve_Expression_Get_Type(ast::Expression_Get_Type& n)
{
  if (is_inferred(n)) return;

  resolve_Node(n.target);
  add_inference(n, get_type_id(n.target));
}
void resolver::Inference::resolve_Expression_New_Ptr(ast::Expression_New_Ptr& n)
{
  if (is_inferred(n)) return;
}
void resolver::Inference::resolve_Literal_Boolean(ast::Literal_Boolean& n)
{
  if (is_inferred(n)) return;

  add_inference(n, type::TYPEID_boolean);
}

void resolver::Inference::resolve_Literal_Integral(ast::Literal_Integral& n)
{
  if (is_inferred(n)) return;

  switch (n.type) {
  case type::EPrimitiveTypeKind::NONE:
  case type::EPrimitiveTypeKind::iSize: ensure_expression_resolution(n, type::TYPEID_iSize); break;
  case type::EPrimitiveTypeKind::i8:    ensure_expression_resolution(n, type::TYPEID_i8); break;
  case type::EPrimitiveTypeKind::i16:   ensure_expression_resolution(n, type::TYPEID_i16); break;
  case type::EPrimitiveTypeKind::i32:   ensure_expression_resolution(n, type::TYPEID_i32); break;
  case type::EPrimitiveTypeKind::i64:   ensure_expression_resolution(n, type::TYPEID_i64); break;
  case type::EPrimitiveTypeKind::i128:  ensure_expression_resolution(n, type::TYPEID_i128); break;
  case type::EPrimitiveTypeKind::uSize: ensure_expression_resolution(n, type::TYPEID_uSize); break;
  case type::EPrimitiveTypeKind::u8:    ensure_expression_resolution(n, type::TYPEID_u8); break;
  case type::EPrimitiveTypeKind::u16:   ensure_expression_resolution(n, type::TYPEID_u16); break;
  case type::EPrimitiveTypeKind::u32:   ensure_expression_resolution(n, type::TYPEID_u32); break;
  case type::EPrimitiveTypeKind::u64:   ensure_expression_resolution(n, type::TYPEID_u64); break;
  case type::EPrimitiveTypeKind::u128:  ensure_expression_resolution(n, type::TYPEID_u128); break;
  default:                              assert(false);
  }
}
void resolver::Inference::resolve_Literal_Floating_Point(ast::Literal_Floating_Point& n)
{
  if (is_inferred(n)) return;

  switch (n.type) {
  case type::EPrimitiveTypeKind::NONE:
  case type::EPrimitiveTypeKind::fSize: ensure_expression_resolution(n, type::TYPEID_fSize); break;
  case type::EPrimitiveTypeKind::f16:   ensure_expression_resolution(n, type::TYPEID_f16); break;
  case type::EPrimitiveTypeKind::f32:   ensure_expression_resolution(n, type::TYPEID_f32); break;
  case type::EPrimitiveTypeKind::f64:   ensure_expression_resolution(n, type::TYPEID_f64); break;
  case type::EPrimitiveTypeKind::f80:   ensure_expression_resolution(n, type::TYPEID_f80); break;
  case type::EPrimitiveTypeKind::f128:  ensure_expression_resolution(n, type::TYPEID_f128); break;
  default:                              assert(false);
  }
}
void resolver::Inference::resolve_Literal_Fixed_Point(ast::Literal_Fixed_Point& n)
{
  if (is_inferred(n)) return;

  switch (n.raw_type) {
  case type::EPrimitiveTypeKind::NONE:
  case type::EPrimitiveTypeKind::dSize:  ensure_expression_resolution(n, type::TYPEID_dSize); break;
  case type::EPrimitiveTypeKind::d32:    ensure_expression_resolution(n, type::TYPEID_d32); break;
  case type::EPrimitiveTypeKind::d64:    ensure_expression_resolution(n, type::TYPEID_d64); break;
  case type::EPrimitiveTypeKind::d128:   ensure_expression_resolution(n, type::TYPEID_d128); break;
  case type::EPrimitiveTypeKind::udSize: ensure_expression_resolution(n, type::TYPEID_udSize); break;
  case type::EPrimitiveTypeKind::ud32:   ensure_expression_resolution(n, type::TYPEID_ud32); break;
  case type::EPrimitiveTypeKind::ud64:   ensure_expression_resolution(n, type::TYPEID_ud64); break;
  case type::EPrimitiveTypeKind::ud128:  ensure_expression_resolution(n, type::TYPEID_ud128); break;
  default:                               assert(false);
  }
}
void resolver::Inference::resolve_Literal_Cune(ast::Literal_Cune& n)
{
  if (is_inferred(n)) return;

  add_inference(n, type::TYPEID_cune);
}
void resolver::Inference::resolve_Literal_Rune(ast::Literal_Rune& n)
{
  if (is_inferred(n)) return;

  add_inference(n, type::TYPEID_rune);
}

void resolver::Inference::resolve_Literal_Table(ast::Literal_Table& n)
{
  if (is_inferred(n)) return;
}
void resolver::Inference::resolve_Literal_Map(ast::Literal_Map& n)
{
  if (is_inferred(n)) return;
}
void resolver::Inference::resolve_Literal_Text_Interpolation(ast::Literal_Text_Interpolation& n)
{
  if (is_inferred(n)) return;
}
void resolver::Inference::resolve_Literal_Text_Pure(ast::Literal_Text_Pure& n)
{
  if (is_inferred(n)) return;

  switch (n.text_type) {
  case type::ETextType::c_str: ensure_expression_resolution(n, type::TYPEID_cune); break;
  case type::ETextType::str:   ensure_expression_resolution(n, type::TYPEID_str); break;
  case type::ETextType::text:  ensure_expression_resolution(n, type::TYPEID_text); break;
  default:                     assert(false);
  }
}
void resolver::Inference::resolve_Literal_Textual_Format(ast::Literal_Textual_Format& n)
{
  if (is_inferred(n)) return;
}
void resolver::Inference::resolve_Literal_Enum(ast::Literal_Enum& n)
{
  if (is_inferred(n)) return;
}
void resolver::Inference::resolve_Literal_Tuple(ast::Literal_Tuple& n)
{
  if (is_inferred(n)) return;
}
void resolver::Inference::resolve_Literal_Range(ast::Literal_Range& n)
{
  if (is_inferred(n)) return;

  ensure_expression_resolution(get_node(n.start), get_type_id(n.end));
  ensure_expression_resolution(get_node(n.end), get_type_id(n.start));
}
void resolver::Inference::resolve_Literal_Structured_Data(ast::Literal_Structured_Data& n)
{
  if (is_inferred(n)) return;
}
void resolver::Inference::resolve_Literal_Entity(ast::Literal_Entity& n)
{
  if (is_inferred(n)) return;
}
void resolver::Inference::resolve_Literal_Iterator(ast::Literal_Iterator& n)
{
  if (is_inferred(n)) return;
}


void resolver::Inference::resolve_Statement_For(ast::Statement_For& n)
{
  if (is_inferred(n)) return;

  resolve_Node(n.expression);

  if (n.index) ensure_expression_resolution(get_node(n.index), get_type_id(n.expression));
  for (auto& elem : n.items) ensure_expression_resolution(get_node(elem), get_type_id(n.expression));
}


void resolver::Inference::resolve_Operation_Cast_As(ast::Operation_Cast_As& n)
{
  if (is_inferred(n)) return;

  assert(n.type);

  ensure_expression_resolution(get_node(n.expression), n.type);
}
void resolver::Inference::resolve_Operation_Is(ast::Operation_Is& n)
{
  if (is_inferred(n)) return;
}
void resolver::Inference::resolve_Operation_In(ast::Operation_In& n)
{
  if (is_inferred(n)) return;
}
void resolver::Inference::resolve_Operation_Assignment(ast::Operation_Assignment& n)
{
  if (is_inferred(n)) return;

  resolve_Node(n.left);

  // variable type inference
  ensure_expression_resolution(get_node(n.right), get_type_id(n.left));
  ensure_expression_resolution(n, get_type_id(n.left));
}
void resolver::Inference::resolve_Operation_Binary(ast::Operation_Binary& n)
{
  if (is_inferred(n)) return;

  if (EBinOpType_is_logical(n.op_ty)) {
    ensure_expression_resolution(get_node(n.left), type::TYPEID_boolean);
    ensure_expression_resolution(get_node(n.right), type::TYPEID_boolean);

    add_inference(n, type::TYPEID_boolean);
    return;
  }


  ensure_expression_resolution(get_node(n.left), get_type_id(n.right));
  ensure_expression_resolution(get_node(n.right), get_type_id(n.left));


  if (EBinOpType_is_comparison(n.op_ty)) {
    add_inference(n, type::TYPEID_boolean);
    return;
  } else if (n.op_ty == ast::EBinOpType::Div) {
    // div always return a floating point
    add_inference(n, type::TYPEID_fSize);
    return;
  } else
    add_inference(n, get_type_id(n.left));
}
void resolver::Inference::resolve_Operation_Unary(ast::Operation_Unary& n)
{
  if (is_inferred(n)) return;

  if (n.unary_op == ast::EUnaryOpType::_not) {
    ensure_expression_resolution(get_node(n.base), type::TYPEID_boolean);
    add_inference(n, type::TYPEID_boolean);
    return;
  } else {
    resolve_Node(n.base);
    ensure_expression_resolution(n, get_type_id(n.base));
  }
}
void resolver::Inference::resolve_Operation_Interval(ast::Operation_Interval& n)
{
  if (is_inferred(n)) return;
}
