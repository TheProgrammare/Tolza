#pragma once

#include <common/enum_lite.hpp>
#include <cstdint>

namespace ast
{

DEFINE_ENUM(ENodeKind, uint8_t,
            // identifiers
            Symbol_Id, 1,        //
            Symbol_Qualified, 2, //
            Symbol_Type, 3,      //

            Path_Regex, 4, //

            Root, 5, //

            Import, 6, //

            // declarations
            // globals
            Global_Variable, 7,             //
            Global_Function, 8,             //
            Global_Extend_Fn, 9,            //
            Global_Extend_Cast, 10,         //
            Global_Extend_Op_Bin, 11,       //
            Global_Extend_Op_Un, 12,        //
            Global_Extend_Op_Subscript, 13, //
            Global_Extend_Op_Transfert, 14, //
            Global_Extend_Op_Other, 15,     //
            Global_Module, 16,              //
            Global_Extern, 17,              //
            Global_Export, 18,              //
            Global_Reexport, 19,            //
            Global_Enum, 20,                //
            Global_Flag, 21,                //
            Global_Union, 22,               //
            Global_Alias_Type, 23,          //
            Global_Alias_Module, 24,        //
            Global_Generic, 25,             //

            Enum_Field, 26,    //
            Flag_Field, 27,    //
            Union_Field, 28,   //
            Call_Contract, 29, //

            CodeBlock, 30, //

            // declarations
            // locals
            Local_Lambda, 31,              //
            Local_Lambda_Capture, 32,      //
            Local_Parameter, 33,           //
            Local_Gen_Param_Elem, 34,      //
            Local_Gen_Params, 35,          //
            Local_Pattern_Element, 36,     //
            Local_Pattern_Enum, 37,        //
            Local_Pattern_Tuple, 38,       //
            Local_Pattern_Form, 39,        //
            Local_Pattern_Rule_Facet, 40,  //
            Local_Pattern_Facet, 41,       //
            Local_Binding, 42,             //
            Local_Tuple_Destructuring, 43, //
            Local_Variable, 44,            //
            Local_Capability, 45,          //

            // compositional oriented paradigm
            SFM_Facet, 46,       //
            SFM_Facet_Field, 47, //
            SFM_View, 48,        //
            SFM_Form, 49,        //
            SFM_Rule, 50,        //
            SFM_Rule_Case, 51,   //

            // generics
            Generic_Type, 52,      //
            Generic_Cast, 53,      //
            Generic_Op, 54,        //
            Generic_View, 55,      //
            Generic_Facet, 56,     //
            Generic_Extension, 57, //
            Generic_Rule, 58,      //

            // literals
            Literal_Boolean, 59,            //
            Literal_NullPtr, 60,            //
            Literal_Integral, 61,           //
            Literal_Fixed_Point, 62,        //
            Literal_Floating_Point, 63,     //
            Literal_Cune, 64,               //
            Literal_Rune, 65,               //
            Literal_Text_Pure, 66,          //
            Literal_Text_Interpolation, 67, //
            Literal_Textual_Format, 68,     //
            Literal_Format_Specifier, 69,   //
            Literal_Table, 70,              //
            Literal_Tuple, 73,              //
            Literal_Range, 74,              //
            Literal_Record, 75,             //

            // expressions
            Expression_If_Ternary, 76,        //
            Expression_Member_Access, 77,     //
            Expression_Self, 78,              //
            Expression_Other, 79,             //
            Expression_Invocation, 80,        //
            Expression_Invocation_Arg, 81,    //
            Expression_Invocation_Extend, 82, //
            Expression_Invocation_Rule, 83,   //
            Expression_Table_Access, 84,      //
            Expression_New_Ptr, 93,           //

            // statements
            Statement_If, 95,          //
            Statement_For, 96,         //
            Statement_Loop, 97,        //
            Statement_While, 98,       //
            Statement_GoTo, 99,        //
            Statement_GoTo_Label, 100, //
            Statement_Return, 101,     //
            Statement_Break, 102,      //
            Statement_Continue, 103,   //
            Statement_Match, 104,      //
            Statement_Match_Case, 105, //

            // operations
            Operation_Cast_As, 106,   //
            Operation_Is, 107,        //
            Operation_In, 108,        //
            Operation_Transfert, 109, //
            Operation_Binary, 110,    //
            Operation_Unary, 111,     //
            Operation_Interval, 112,  //
            Operation_Mem, 113,       //
)


DEFINE_ENUM(EOp_Unary, uint8_t,
            // arithmetic
            _not, 1,         // not !
            _plus, 2,        // as positive +
            _minus, 3,       // as negative -
            _invert_sign, 4, // invert sign
)


DEFINE_ENUM(EOp_Subscript, uint8_t, _index, 1, // index[i] -> T
            _index_bound, 2,                   // index?[i] -> T?
            _slice, 3,                         // slicing[start..end] -> Slice<T>
            _slice_bound, 4,                   // slicing?[start..end] -> Slice<T>?
            _b_index, 5,                       // index bit ~[i] -> bool
            _b_slice, 6,                       // slicing bits ~[start..end] -> bsize
)


DEFINE_ENUM(EOp_Bin, uint8_t,
            // arithmetic
            _add, 1,    // add +
            _sub, 2,    // substract -
            _mul, 3,    // multiply *
            _div, 4,    // divide /
            _mod, 5,    // modulo %mod% not signed if divided > 0
            _quo, 6,    // quotien %quo%
            _rem, 7,    // remain %rem% signed with dividend
            _divrem, 8, // quotien + remainder in one operation %divrem%
            _pow, 9,    // power **
            // comparator
            _ordering, 10, // ordering <=>
            _gre, 11,      // greater >
            _low, 12,      // lower <
            _gre_eq, 13,   // greater equal >=
            _low_eq, 14,   // lower equal <=
            _eq, 15,       // equal ==
            _in, 16,       // inside in
            _nin, 17,      // not inside !in or nin
            _is, 18,       // is
            _nis, 19,      // not is !is or nis
            _neq, 20,      // not equal !=
            _eqs, 21,      // equal strictly ===
            _neqs, 22,     // not equal strictly !==
            // logical
            _and, 23,    // and logical and
            _nand, 24,   // not and logical
            _or, 25,     // or logical
            _xor, 26,    // xor logical
            _nor, 27,    // not or logical
            _xnor, 28,   // not xor logical
            _b_and, 29,  // b.and bitwise
            _b_nand, 30, // b.nand not and bitwise
            _b_or, 31,   // b.or bitwise
            _b_xor, 32,  // b.xor bitwise
            _b_nor, 33,  // b.nor not or bitwise
            _b_xnor, 34, // b.xnor not xor bitwise

            _b_shl_0, 35, // b.shl.0 left shift fill 0
            _b_shl_1, 36, // b.shl.1 left shift fill 1
            _b_shl_a, 37, // b.shl.a left shift arithmetic (fill with MSB)
            _b_shr_0, 38, // b.shr.0 right shift fill 0
            _b_shr_1, 39, // b.shr.1 right shift fill 1
            _b_shr_a, 40, // b.shr.a right shift arithmetic (fill with MSB)
            _b_rol, 41,   // b.rol left rotate
            _b_ror, 42,   // b.ror right rotate

            _mem_add, 43,  // mem.add
            _mem_sub, 44,  // mem.sub
            _mem_dist, 45, // mem.dist
)

DEFINE_ENUM(EOp_Mem, uint8_t, //
            _val, 1,          // pointer deref
            _mut, 2,          // mutable borrow
            _ref, 3,          // immutable borrow
            _move, 4,         // move semantic
            _copy, 5,         // copy semantic
            _addr, 6,         // get memory address
            _del, 7,          // delete pointer
            _drop, 8,         // borrow explicit drop (usable ???)
            _out, 9,          // declare and extract value from parameter
)

DEFINE_ENUM(EInvocationKind, uint8_t, //
            fn_call, 1,               //
            enum_bind, 2,             //
            pattern, 3,               //
)

// name op like : _name
DEFINE_ENUM(EOp_Other, uint8_t, //
            _del, 1,            //
            _predicat, 2        //
)


DEFINE_ENUM(ECapability, uint8_t, //
            ref, 1,               //
            mut, 2,               //
            copy, 3,              //
            move, 4               //
)


DEFINE_ENUM(EPassMode, uint8_t, //
            mut, 1,             //
            ref, 2,             //
            copy, 3,            //
            move, 4,            //
            addr, 5,            //
            _const, 6,          //
)


DEFINE_ENUM(EExprPassMode, uint8_t, //
            mut, 1,                 //
            ref, 2,                 //
            copy, 3,                //
            move, 4                 //
)


DEFINE_ENUM(EVariableKind, uint8_t, //
            _const, 1,              //
            _let, 2,                //
            _var, 3                 //
)

DEFINE_ENUM(ETransfertType, uint8_t, //
            copy, 1,                 //
            move, 2                  //
)

DEFINE_ENUM(ECallContract, uint8_t, //
            Static, 1,              //
            Assert, 2,              //
            Result, 3,              //
            Panic, 4,               //
)

enum class EVisibility : uint8_t { Lexical_Scope, File_Scope, Cross_File_Scope };

DEFINE_ENUM(EPathAnchor, uint8_t, //
            src, 1,               // user scripts
            vendor_lib, 2,        // 3rd party scripts
            stdlib, 3,            // standard library
            pkg_lib, 4,           // package library
            binding, 5,           // binding library
            relative_self, 6,     // relative current module
            relative_super, 7,    // relative parent module
            relative_root, 8,     // relative script root module
)


} // namespace ast