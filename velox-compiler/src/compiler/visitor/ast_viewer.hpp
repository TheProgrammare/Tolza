#pragma once

#include <unordered_map>

#include "compiler/ast/ast_forward.hpp"
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
