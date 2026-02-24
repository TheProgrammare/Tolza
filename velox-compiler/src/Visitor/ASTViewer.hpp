#pragma once

#include <unordered_map>

#include "AST/AST_Forward.hpp"
#include "Visitor_Default.hpp"

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

  void parent_dot(const AST::Node& n, const std::string& context = "");

  void child_dot(const AST::Node& parent, const AST::Node& child, const std::string& context = "");

private:
  size_t                                     lastLine = 0;
  std::ostream&                              os_;
  std::unordered_map<const AST::Node*, int>  id_;
  std::unordered_map<const AST::AType*, int> id_ty_;
  int                                        next_ = 0;

  int get_id(const AST::Node& n)
  {
    auto it = id_.find(&n);
    if (it != id_.end()) return it->second;
    int id  = next_++;
    id_[&n] = id;
    return id;
  }
  int get_id_type(const AST::AType& n)
  {
    auto it = id_ty_.find(&n);
    if (it != id_ty_.end()) return it->second;
    int id     = next_++;
    id_ty_[&n] = id;
    return id;
  }

public:
};
