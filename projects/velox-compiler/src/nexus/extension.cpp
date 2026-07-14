#include "extension.hpp"

#include "ast/ast_declaration_extension.hpp"

ast::Global_Extend_Cast* extension::get_cast(type::ID extended_tyid, type::ID cast_to_tyid) noexcept
{
  const auto& exts = extended_tyid.extensions();
  if (exts.empty()) return nullptr;

  for (auto nodeid : exts) {
    auto* node = nodeid.as<ast::Global_Extend_Cast>();
    if (!node) continue;
    if (node->as_type == cast_to_tyid) return node;
  }

  return nullptr;
}

ast::Global_Extend_Op_Bin* get_op_bin(type::ID tyid, ast::EOp_Bin opty) noexcept
{
  const auto& exts = tyid.extensions();
  if (exts.empty()) return nullptr;

  for (auto nodeid : exts) {
    auto* node = nodeid.as<ast::Global_Extend_Op_Bin>();
    if (!node) continue;
    if (node->bin_op == opty) return node;
  }

  return nullptr;
}
ast::Global_Extend_Op_Un* get_op_unary(type::ID tyid, ast::EOp_Unary opun) noexcept
{
  const auto& exts = tyid.extensions();
  if (exts.empty()) return nullptr;

  for (auto nodeid : exts) {
    auto* node = nodeid.as<ast::Global_Extend_Op_Un>();
    if (!node) continue;
    if (node->unary_op == opun) return node;
  }

  return nullptr;
}
ast::Global_Extend_Op_Subscript* get_op_subscript(type::ID tyid, ast::EOp_Subscript opsub) noexcept
{
  const auto& exts = tyid.extensions();
  if (exts.empty()) return nullptr;

  for (auto nodeid : exts) {
    auto* node = nodeid.as<ast::Global_Extend_Op_Subscript>();
    if (!node) continue;
    if (node->subscript_op == opsub) return node;
  }

  return nullptr;
}
ast::Global_Extend_Op_Transfert* get_op_transfert(type::ID tyid, ast::ETransfertType optr) noexcept
{
  const auto& exts = tyid.extensions();
  if (exts.empty()) return nullptr;

  for (auto nodeid : exts) {
    auto* node = nodeid.as<ast::Global_Extend_Op_Transfert>();
    if (!node) continue;
    if (node->transfert_op == optr) return node;
  }

  return nullptr;
}
ast::Global_Extend_Op_Other* get_op_other(type::ID tyid, ast::EOp_Other opot) noexcept
{
  const auto& exts = tyid.extensions();
  if (exts.empty()) return nullptr;

  for (auto nodeid : exts) {
    auto* node = nodeid.as<ast::Global_Extend_Op_Other>();
    if (!node) continue;
    if (node->other_op == opot) return node;
  }

  return nullptr;
}
