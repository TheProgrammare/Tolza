#pragma once

#include <variant>

#include "AST_Base.hpp"

namespace AST {

struct CodeBlock_instruction {
    std::variant<
        std::shared_ptr<ALocal>,
        std::unique_ptr<Node>
    > data;

    Node* node() const {
        return std::visit([](auto const& v) -> Node* {
            return v.get();
        }, data);
    }
};


}