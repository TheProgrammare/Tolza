#pragma once

#include <memory>

#include "AST/AST_Forward.hpp"

namespace PAR {
	struct Parser_Context;

	struct Parser_Declaration_ECS {
		Parser_Declaration_ECS(Parser_Context& ctx) : ctx(ctx) {}

        [[nodiscard]] std::shared_ptr<AST::Declaration::ECS::Component>     component();
        [[nodiscard]] std::shared_ptr<AST::Declaration::ECS::Role>          role();
        [[nodiscard]] std::shared_ptr<AST::Declaration::ECS::Entity>        entity();
        void                                                                parse_entity_declaration(std::shared_ptr<AST::Declaration::ECS::Entity> inEntity);
        [[nodiscard]] std::shared_ptr<AST::Declaration::ECS::Entity_Op>         _entity_op(std::shared_ptr<AST::Declaration::ECS::Entity> inEntity);
        [[nodiscard]] std::shared_ptr<AST::Declaration::ECS::Entity_Cast>       _entity_cast(std::shared_ptr<AST::Declaration::ECS::Entity> inEntity);
        [[nodiscard]] std::shared_ptr<AST::Declaration::ECS::System>        system();
        [[nodiscard]] std::shared_ptr<AST::Declaration::ECS::System_Case>       _system_case();

        Parser_Context &ctx;
    };
}