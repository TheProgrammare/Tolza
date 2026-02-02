#include "AST_Declaration_ECS.hpp"

#include "AST_Declaration.hpp"

bool AST::Declaration::ECS::System_Case::manage_entity(const Entity &entity) const
{
    return false;
}

bool AST::Declaration::ECS::System_Case::manage_component(const Component &comp) const
{
    return false;
}