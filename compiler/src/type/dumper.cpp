#include "type/dumper.hpp"

#include "ast/dumper.hpp"
#include "ast/node/declaration_sfm.hpp"
#include "ast/tool.hpp"
#include "id/typeid.hpp"
#include "type/data.hpp"
#include "type/type.hpp"

#include <cassert>
#include <format>
#include <iterator>
#include <string>


std::string type::dump(ID tyid) noexcept
{
  const auto& ptr_ty = tyid.get();

  switch (tyid.kind()) {
  case ETypeKind::NONE: return {};
  case ETypeKind::Primitive:
    return std::string(EPrimitiveTypeKind_to_str(tyid.as<type::Primitive>()->primitive).substr(1));
  case ETypeKind::String: return std::string(ETextType_to_str(tyid.as<type::String>()->kind).substr(1));
  case ETypeKind::Tuple:  {
    std::string out;
    const auto* tu = tyid.as<type::Tuple>();
    out.reserve(tu->elems.size() * 32);
    for (const auto& tyid : tu->elems) {
      std::format_to(std::back_inserter(out), "{}, ", dump(tyid));
    }

    return std::format("({})", out.substr(0, out.size() - 2));
  }
  case ETypeKind::Array: {
    const auto* ptr = tyid.as<type::Array>();
    return std::format("[{}; {}]", dump(ptr->inner), std::to_string(ptr->size));
  }
  case ETypeKind::Buffer: {
    const auto* ptr = tyid.as<type::Buffer>();
    return std::format("[{}; _]", dump(ptr->inner));
  }
  case ETypeKind::Slice: {
    const auto* ptr = tyid.as<type::Slice>();
    return std::format("[{}; {}]", dump(ptr->inner), ptr->is_c_table ? "c" : "..");
  }
  case ETypeKind::Ptr: {
    const auto* ptr = tyid.as<type::Ptr>();
    return std::format("ptr'{}", dump(ptr->inner));
  }
  case ETypeKind::Prototype: {
    std::string params;
    params.reserve(params.size() * 32);
    const auto* proto = tyid.as<type::Prototype>();
    for (const auto& param : proto->params) {
      std::format_to(std::back_inserter(params), "{} {}, ", ast::EPassMode_to_str(param.passmode), dump(param.type));
    }
    params = params.substr(0, params.size() - 2);

    return std::format("fn({}) -> {}", params, dump(proto->ret));
  }
  case ETypeKind::Facet: {
    const auto* facet = tyid.as<type::Facet>();
    const auto* node  = facet->def.node().as<ast::SFM_Facet>();
    assert(node && "type node must be a facet");

    std::string fields;
    fields.reserve(node->fields.size() * 32);
    for (const auto& field : node->fields) {
      const auto* f_node = field.as<ast::SFM_Facet_Field>();
      assert(f_node && "a facet must have field nodes");

      std::format_to(std::back_inserter(fields), "{}: {}\n", f_node->name, dump(f_node->type));
    }

    return std::format("facet {}{{\n{}}}\n", node->name, fields);
  }
  case ETypeKind::View: {
    const auto* view = tyid.as<type::View>();
    assert(view->def && "a view type must refer to a symbol");
  }
  case ETypeKind::Identifier: {
    const auto* id = tyid.as<type::Identifier>();
    return ast::get_decl_name(id->nodeid);
  }
  case ETypeKind::Form:
  case ETypeKind::Enum:
  case ETypeKind::Flag:
  case ETypeKind::Union: break;
  }

  return {};
}
