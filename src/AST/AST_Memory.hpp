#pragma once

#include "AST_Base.hpp"

namespace AST {
namespace Memory {

// move'p
struct Move : public Node {
    std::unique_ptr<Node> target;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    [[nodiscard]] std::string debug_str() const override { return "move"; }
};

// new ptr'T(val)
struct New : public Node {
    EPtrType pointer = EPtrType::raw_ptr;
    std::shared_ptr<AType> type;
    std::unique_ptr<Node> expression;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    [[nodiscard]] std::string debug_str() const override {  return "new"; }
};

// del var
struct Del : public Node {
    std::unique_ptr<Node> element;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    [[nodiscard]] std::string debug_str() const override { return "delete"; }
};


// val'my_ptr
struct Val_Of_Ptr : public Node {
    std::unique_ptr<AReference> target;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    [[nodiscard]] std::string debug_str() const override { return "val'"; }
};

// addr'my_val
struct Addr_Of_Ref : public Node {
    std::unique_ptr<AReference> target;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    [[nodiscard]] std::string debug_str() const override { return "addr'"; }
};


// p1 <-> p2
struct Dist : public Node {
    std::unique_ptr<Node> left;
    std::unique_ptr<Node> right;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    [[nodiscard]] std::string debug_str() const override { return "<mem> distance"; }
};

struct Size : public Node {
    std::unique_ptr<Node> target;
    size_t size = 0;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    [[nodiscard]] std::string debug_str() const override { return "<mem> size(" + std::to_string(size) + ")"; }
};

struct Align : public Node {
    std::unique_ptr<Node> target;
    size_t align = 0;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    [[nodiscard]] std::string debug_str() const override { return "<mem> align(" + std::to_string(align) + ")"; }
};

// target~[0..8] | target~[16..24]
struct GetBits : public Node {
    std::unique_ptr<AReference> target;
    std::unique_ptr<Node> range;

    std::shared_ptr<AType> resolved_range_ty;
    // 8, 16, 32, 64, 128
    enum EBitSize { _8, _16, _32, _64, _128 };
    EBitSize bit_size = _8;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    [[nodiscard]] std::string debug_str() const override { return "bits get"; }
};

struct Drop : public Node {
    std::unique_ptr<AReference> target;

    void accept(Visitor_Base& v) override { v.visit(*this); }
    [[nodiscard]] std::string debug_str() const override { return "drop"; }
};

}
}