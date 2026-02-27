#pragma once

namespace AST
{
struct Node; // base

struct AType;             // abstract
struct ADeclaration;      // abstract
struct ALocal;            // abstract : ADeclaration
struct AExpression;       // abstract with inferred type
struct ALiteral;          // abstract with literal value
struct Expr_ID;           // identifier expression
struct Expr_ID_Qualified; // qualified identifier expression
struct Expr_ID_Generic;   // identifier with generic arguments

struct Root; // file root

namespace Declaration
{
struct Global;   // var a: T = ... / let a: T = ... / const a: T = ...
struct Function; // fn add(a: i32, b: i32) -> i32 {...}

struct Mod;    // mod Declaration { entity Global {...} } }
struct Export; // export AST { mod Declaration {...} }

struct Enum;         // enum EItem { House(str, i32), City(str), None() }
struct Enum_Element; // EItem::House(str, i32)

struct Flag; // flag Fautorisation { pr, pw, px, gr, gw, gx, or, ow, og }

struct Type_Alias; // type ull = u64
struct Mod_Alias;  // mod Vec = core::container::Vector

struct Generic; // gen GNumeric<T> { T op +, T op - }

namespace Local
{
struct CodeBlock; // { ... }

struct Lambda;         // lam name[self](a: i32) -> i32 {...} / lam {...}
struct Lambda_Capture; // [self] / [mut] / [copy] / [ref] / [Var1, Var2]
struct Capture_Member; // self / mut / copy / ref / Var1, Var2

struct Parameter;         // a: i32 = ... / copy a: i32 / mut a: i32 / ref a: i32 /  ptr'void...
struct Generic_Parameter; // <T: Movable + Physic, U: Copyable>

struct Pattern;
struct Pattern_Enum;      // let EItem::House(name, number) = building
struct Pattern_Tuple;     // let (a, b, c) = triple
struct Pattern_Entity;    // let Player{ CId.name: name, CId.id: 10 } // let Player{ CId{ name: name, id: 10 } }
struct Pattern_Component; // let CId{ name: name, id: 10 }

struct Variable_Binding; // EItem::House(str, i32) // str and i32 are bindings
struct Variable_Unpack;  // var (a, _, c) = triple / let (a, b) = call_pair()
struct Variable;         // var a: T = ... / let a: T = ... / const a: T = ...

struct Capability; // ref a = origin // mut a = origin
} // namespace
  // Local

namespace COP
{
struct Component;       // comp CPosition { x: f32 = 0, y: f32 = 0, z: f32 = 0 }
struct Component_Field; // x: f32 = 0

struct Role; // role RMovable { CPosition, CPhysic }

struct Entity;         // entity TPlayer { use CPosition, use CPhysic }
struct Entity_Cast;    // cast self to TAnimal {...}
struct Entity_Op;      // op + { self.CPosition.x += other.CPosition.y }
struct Entity_OpIndex; // op[a] { return self.CInventory.items[a] }

struct System;      // sys jump(height: f32) { A(a) + B(b) => ... other => ...}
struct System_Case; // CPosition(pos) + CPhysic(phy) => call_fn(pos, phy)
} // namespace
  // COP
} // namespace
  // Declaration

namespace Generic
{
struct IGenCond; // interface

struct Is_Type;           // T is GCalculable
struct Can_Cast;          // T cast to i32 // T cast from i32
struct Have_Op;           // T op -
struct Have_Role;         // T role RMovable
struct Use_Component;     // T comp CPosition
struct Compatible_System; // T sys jump
} // namespace
  // Generic

namespace Type
{
struct Ptr;            // ptr' // uptr' // sptr' // wptr'
struct Table;          // [T; N] // [T; N, N, ...] // [T; N]*D
struct Primitive;      // i8-i128-isize u8-u128-usize b8-b128-bsize f32-f128-fsize
                       // bool void ascii utf32 str text
struct Tuple;          // (T, U, V) // (a: T, b: U, c: V)
struct Function_Proto; // fn(T, U) -> (T, U, V)

struct Get_Expr_Type; // type(expr)
} // namespace
  // Type

namespace Literal
{
struct Boolean;  // true // false
struct Integral; // int uint binary
struct Decimal;  // deci udeci
struct Floating; // float double longdouble

struct ASCII; // "a"ascii -> 8 bits character
struct UFT32; // "⚜"utf32 "⚜" -> 32 bits character

struct Text;             // fat pointer of utf32 "Hello World!"
struct Text_Lerp;        // "{expression}" "{expression:spec}"
struct Textual_Element;  // Text or Text_Lerp
struct Textual_Format;   // "name is {name}"
struct Format_Specifier; // "{name:format_specifier}"

struct Table;            // { 0, 1, 2 } // {{1, 2},{1, 2}}
struct Table_Population; // { [0..3] => 5 } -> { 5, 5, 5 } // { [0..3] => @i +
                         // 10 } -> { 10, 11, 12 }

struct Map; // { 10 -> "hello", 20 -> "world" }

struct Tuple; // ("my", 10, value)

struct Range; // [0..10] // [1..=10]

struct Component; // CPosition{x= 10, y= 10, z= 10}
struct Entity;    // CPlayer{CPosition.x= 10} // CPlayer{CPosition{x= 10, y= 10, z=
                  // 10}} // CPlayer{.CPosition{10, 10, 10}}
} // namespace
  // Literal

namespace Expression
{
struct If_Ternary; // x = if a > b then a else math::abs(b)

struct Enum; // EItem::House(name, number)

struct Member_Access; // Player.CPosition.x // CPosition.x

struct Self;  // self
struct Other; // other

struct Call;          // foo()
struct Call_Argument; // (a, b, c)
struct Call_System;   // Player::>jump(100)
struct Call_Pipe;     // add | a, b | c, d

struct Table_Access; // index [i] or slice [0..10]

struct Ptr_At;     // my_ptr'at(i)
struct Ptr_Offset; // my_ptr'offset(i)
struct Ptr_Val;    // val'my_ptr
struct Addr_Of;    // addr'a
struct Size_Of;    // size'a
struct GetBits;    // a~[0..8]

struct Move;    // move'a
struct New_Ptr; // new ptr'T(val) // new ptr'T{}
} // namespace
  // Expression

namespace Statement
{
struct If; // if a == 0 {...} elif a == 1 {...} else {...} // if let Some(a) =
           // value {...}

struct For;        // for i in [0..10] {...} // for let (key, val) in map_val {}
struct Loop;       // loop {...}
struct While;      // while a < 10 {...} // while let a = stack.pop()
struct GoTo;       // goto over_error
struct GoTo_Label; // label over_error:

struct Return;   // return a, 0, c
struct Break;    // break
struct Continue; // continue

struct Match;      // match value {...}
struct Match_Case; // case Some(a) {...} // case Some(a, b) if e > 10 {...}
} // namespace
  // Statement

namespace Operation
{
struct Cast_As;    // a as b // a as? b // a as! b
struct Is;         // a is b <=> a == b
struct In;         // a in [0..10] // "Hello" in "Hello World!"
struct Assignment; // a copy= b // a ref= b // a mut= b // a move= b
struct Binary;     // a + b // a - b // a * b // a and b ...
struct Unary;      // ++a // a++ // --a // a-- // !a
struct Interval;   // a > x > b
struct Ptr_Dist;   // a <-> b
} // namespace
  // Operation

namespace Memory
{
struct Del;   // del a
struct Align; // align a
struct Drop;  // drop a
} // namespace
  // Memory
} // namespace
  // AST