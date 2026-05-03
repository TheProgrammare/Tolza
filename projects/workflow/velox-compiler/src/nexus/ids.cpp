#include "ids.hpp"


ast::_gnid ast::_id::get_gnid(script::_id scr) const noexcept
{
  return _gnid((static_cast<size_t>(scr.value()) << 32) | id);
}

script::_id ast::_gnid::get_script_id() const noexcept
{
  return script::_id(id >> 32);
}

ast::_id ast::_gnid::get_node_id() const noexcept
{
  return _id(id & 0xFFFFFFFF);
}