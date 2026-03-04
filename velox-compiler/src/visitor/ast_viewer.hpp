#pragma once

#include <string>
#include <unordered_map>

#include "ast/ast_forward.hpp"
#include "visitor_default.hpp"

// escape bad characters for dot
static inline std::string escapeDot(const std::string& s)
{
  std::string out;
  out.reserve(s.size() + 8);
  for (char c : s) {
    switch (c) {
    case '\\': out += "\\\\"; break;
    case '"':  out += "\\\""; break;
    case '\n': out += "\\n"; break;
    case '.':  out += "_"; break;
    case ' ':  out += "_"; break;
    case '\r': break;
    default:   out += c; break;
    }
  }
  return out;
}

struct ScriptInfo;

struct AST_Viewer : public Visitor_Default {
public:
  AST_Viewer(ScriptInfo& scrInfo, std::ostream& os)
    : Visitor_Default(scrInfo)
    , os_(os)
  {
  }

  void parent_dot(const ast::Node& n, const std::string& context = "");

  void child_dot(const ast::Node& parent, const ast::Node& child, const std::string& context = "");

private:
  size_t                                     lastLine = 0;
  std::ostream&                              os_;
  std::unordered_map<const ast::Node*, int>  id_;
  std::unordered_map<const ast::AType*, int> id_ty_;
  int                                        next_ = 0;

  int get_id(const ast::Node& n)
  {
    auto it = id_.find(&n);
    if (it != id_.end()) return it->second;
    int id  = next_++;
    id_[&n] = id;
    return id;
  }
  int get_id_type(const ast::AType& n)
  {
    auto it = id_ty_.find(&n);
    if (it != id_ty_.end()) return it->second;
    int id     = next_++;
    id_ty_[&n] = id;
    return id;
  }

public:
};

// format with title
constexpr const char* DIAGRAPH_VIEW_PARAM_AST_begin =
    R"(
digraph AST {
  
  graph [
    rankdir=LR,
    bgcolor="#1e1e1e",
    nodesep=0.1,
    ranksep=0.2,
    label=""
)";

// format with title
constexpr const char* DIAGRAPH_VIEW_PARAM_LINKAGE_begin =
    R"(
digraph AST {

  splines=line;

  graph [
    rankdir=LR,
    bgcolor="#1e1e1e",
    nodesep=1,
    ranksep=1,
    label=""
)";

constexpr const char* DIAGRAPH_VIEW_PARAM_end =
    R"(
  ",
    labelloc=t,
    splines=spline,
    fontname="Cascadia Mono",
    fontcolor=white,
    fontsize=25
  ]

  node [
    style="rounded,filled",
    fillcolor="#2d2d2d",
    shape=rect,
    fontname="Cascadia Mono",
    fontcolor=white,
    fontsize=10
  ]

  edge [
    color="#bbbbbb",
    fontname="Cascadia Mono",
    fontcolor=white,
    fontsize=10
  ]
)";
