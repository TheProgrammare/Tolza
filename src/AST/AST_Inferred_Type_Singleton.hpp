#pragma once

#include "AST_Base.hpp"
#include "AST_Type.hpp"

namespace AST {
namespace Type {


inline INFERRED_TYPE get_bool_type() 
{
    static Primitive *prim = new Primitive(EPrimType::boolean);
    return prim;
}

inline INFERRED_TYPE get_i8_type() 
{
    static Primitive *prim = new Primitive(EPrimType::i8);
    return prim;
}

inline INFERRED_TYPE get_i16_type() 
{
    static Primitive *prim = new Primitive(EPrimType::i16);
    return prim;
}

inline INFERRED_TYPE get_i32_type() 
{
    static Primitive *prim = new Primitive(EPrimType::i32);
    return prim;
}

inline INFERRED_TYPE get_i64_type() 
{
    static Primitive *prim = new Primitive(EPrimType::i64);
    return prim;
}

inline INFERRED_TYPE get_i128_type() 
{
    static Primitive *prim = new Primitive(EPrimType::i128);
    return prim;
}

inline INFERRED_TYPE get_isize_type() 
{
    static Primitive *prim = new Primitive(EPrimType::iSize);
    return prim;
}

// unsigned integers
inline INFERRED_TYPE get_u8_type() 
{
    static Primitive *prim = new Primitive(EPrimType::u8);
    return prim;
}

inline INFERRED_TYPE get_u16_type() 
{
    static Primitive *prim = new Primitive(EPrimType::u16);
    return prim;
}

inline INFERRED_TYPE get_u32_type() 
{
    static Primitive *prim = new Primitive(EPrimType::u32);
    return prim;
}

inline INFERRED_TYPE get_u64_type() 
{
    static Primitive *prim = new Primitive(EPrimType::u64);
    return prim;
}

inline INFERRED_TYPE get_u128_type() 
{
    static Primitive *prim = new Primitive(EPrimType::u128);
    return prim;
}

inline INFERRED_TYPE get_usize_type() 
{
    static Primitive *prim = new Primitive(EPrimType::uSize);
    return prim;
}

// booleans
inline INFERRED_TYPE get_b8_type() 
{
    static Primitive *prim = new Primitive(EPrimType::b8);
    return prim;
}

inline INFERRED_TYPE get_b16_type() 
{
    static Primitive *prim = new Primitive(EPrimType::b16);
    return prim;
}

inline INFERRED_TYPE get_b32_type() 
{
    static Primitive *prim = new Primitive(EPrimType::b32);
    return prim;
}

inline INFERRED_TYPE get_b64_type() 
{
    static Primitive *prim = new Primitive(EPrimType::b64);
    return prim;
}

inline INFERRED_TYPE get_b128_type() 
{
    static Primitive *prim = new Primitive(EPrimType::b128);
    return prim;
}

inline INFERRED_TYPE get_bsize_type() 
{
    static Primitive *prim = new Primitive(EPrimType::bSize);
    return prim;
}

// floats
inline INFERRED_TYPE get_f32_type() 
{
    static Primitive *prim = new Primitive(EPrimType::f32);
    return prim;
}

inline INFERRED_TYPE get_f64_type() 
{
    static Primitive *prim = new Primitive(EPrimType::f64);
    return prim;
}

inline INFERRED_TYPE get_f128_type() 
{
    static Primitive *prim = new Primitive(EPrimType::f128);
    return prim;
}

inline INFERRED_TYPE get_fsize_type() 
{
    static Primitive *prim = new Primitive(EPrimType::fSize);
    return prim;
}

// decimals
inline INFERRED_TYPE get_deci_type() 
{
    static Primitive *prim = new Primitive(EPrimType::deci);
    return prim;
}

inline INFERRED_TYPE get_udeci_type() 
{
    static Primitive *prim = new Primitive(EPrimType::udeci);
    return prim;
}

// other types
inline INFERRED_TYPE get_void_type() 
{
    static Primitive *prim = new Primitive(EPrimType::Void);
    return prim;
}

inline INFERRED_TYPE get_ascii_type() 
{
    static Primitive *prim = new Primitive(EPrimType::ASCII);
    return prim;
}

inline INFERRED_TYPE get_utf32_type() 
{
    static Primitive *prim = new Primitive(EPrimType::UTF32);
    return prim;
}

inline INFERRED_TYPE get_str_type() 
{
    static Primitive *prim = new Primitive(EPrimType::str);
    return prim;
}

inline INFERRED_TYPE get_text_type() 
{
    static Primitive *prim = new Primitive(EPrimType::text);
    return prim;
}

}

}