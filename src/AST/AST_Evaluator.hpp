#pragma once

#include <variant>
#include <memory>

#include "AST_Base.hpp"
#include "AST_Forward.hpp"
#include "AST_Declaration_Local.hpp"

namespace AST {

struct Evaluator {
    enum class EKind { None, Pattern, Condition };
    EKind kind = EKind::None;

    std::unique_ptr<Declaration::Local::Pattern>    pattern;
    std::unique_ptr<Node>                           condition;

    Node* node() const {
        if (kind == EKind::Pattern) return dynamic_cast<Node*>(pattern.get());
        else return condition.get();
    }

    Evaluator() = default;

    Evaluator(std::unique_ptr<Declaration::Local::Pattern> _pattern)
        : pattern(std::move(_pattern))
        , kind(EKind::Pattern) {}
    
    Evaluator(std::unique_ptr<Node> _condition)
        : condition(std::move(_condition))
        , kind(EKind::Condition) {}
};

}