<div align="center">
  
# JSON Specification for FFI AST

</div>

This document defines the **JSON convention** used to represent an **Abstract Syntax Tree (AST)** for FFI (Foreign Function Interface) bindings.  
The JSON file contains all the necessary information to define **functions, types, structures, entities, enums, unions, flags, and aliases** exposed by an external library.

Each AST section is represented by a JSON object with specific fields, strictly defined to ensure reliable conversion into the corresponding Velox structures (`functions`, `type`, `components`, `entities`, `globals`, `enums`, `unions`, `flags`, `typealiases`).

## Key conventions
- **Types** are described by objects containing the base type (`base_type`) and additional attributes such as pointer, array, const/volatile qualifiers, and atomicity.
- **Functions** (`functions`) include the name, calling convention, prototype, and parameter names.
- **Components** (`components`) contain fields and memory layout information.
- **Entites** (`entities`) contain components.
- **Flags** (`flags`) define his members and underlying types.
- **Enums** (`enums`) define his members with members type list.
- **Unions** (`unions`) define his members with member type. 
- **Global** (`globals`) define global variable with name, type, constant. 
- **TypeAliases** (`typealiases`) define his name and type. 
- All boolean fields are optional and default to `false` if omitted.
- All name fields are mandatory.

This JSON convention enables reliable **serialization and deserialization of the AST** in Velox using functions from the `FFI::JSON` namespace.

## FFI JSON Pipeline
The pipeline permit to generate FFI for velox from any language or lib

The generator must convert the Foreign Code to a valid JSON AST. 
The Velox compiler will inspect JSON files (at `FFI_JSON/` in current project folder) then generate binding scripts for the compilation at EMBBinder phase (at `EMBinder/` in current project folder)

Binding script generation:

`Foreign Code` ► `[User Script/Generator]` ► `JSON AST` ► `[Compiler]` ► `Binding Scripts`

Binding script compilation:

`EMBinder phase` ► `Lexing` ► `Preprocess` ► `Parsing` ► push to the main compilation pipeline...

# JSON root file
``` json
{
  "bind": { <bind> },
  "functions": [ { <function> }, ... ],
  "components": [ { <component> }, ... ],
  "entities": [ { <entity> }, ... ],
  "flags": [ { <flag> }, ... ],
  "enums": [ { <enum> }, ... ],
  "unions": [ { <union> }, ... ],
  "globals": [ { <global> }, ... ],
  "typealiases": [ { <typealias> }, ... ]
}
```

# Bind base
``` json
"bind": {
  "bind_name": "",
  "lang": "",
  "lib": ""
}
```

# Type
``` json
"type": {
  "base_type": "",
  "complex_type_name": "",
  "is_pointer": false,
  "is_pointer_double": false,
  "is_pointer_const": false,
  "is_pointer_volatile": false,
  "is_table": false,
  "is_table_of_pointers": false,
  "is_val_type_const": false,
  "is_val_type_volatile": false,
  "is_atomic": false,
  "table_size": [number, ...]
}
```

## Base Type
It's a string keyword

Velox convention:
- `i8`-`i128`-`isize`: signed integrals
- `u8`-`u128`-`usize`: unsigned integrals
- `b8`-`b128`-`bsize`: binary
- `f32`-`f128`-`fsize`: float
- `ptrdiff`: pointer distance
- `void`: no type
- `bool`: boolean
- `ascii`: ascii character
- `utf32`: utf32 character
- `str`: string (ascii)
- `text`: text (utf32)
- `enum`: enumeration
- `comp`: component
- `entity`: entity
- `union`: union
- `flag`: flag
- `prototype`: function prototype

C convention:
- `schar` `sc` / `uchar` `uc`: character / unsigned character
- `short` `s` / `ushort` `us`: short / unsigned short
- `long` `l` / `ulong` `ul`: long / unsigned long
- `longlong` `ll` / `ulonglong` `ull`: long long / unsigned long long (64 bits)
- `int` `i` / `uint` `ui`: integer / unsigned integer
- `float` `f`: float
- `double` `d`: double
- `longdouble` `ld`: long double (128 bits)

# Param
`pass_mode` must have one string value: `copy` `ref` `mut` `move` `addr`

``` json
[
  {
    "pass_mode",
    { <type> },
    false // is_restrict
  },
  ...
]
```

# Prototype
``` json
"prototype": {
  { <type> }, // return type
  false, // is_variadic
  [ <parameters> ] // parameters
}
```

# Component
``` json
"components": [
  {
    "name": "",
    "fields": [
      [ "field_name", { <type> } ],
      ...
    ],
    "layouts": [
      [ 0, 1, 1 ], // offset, size, align
      ...
    ]
  },
  ...
]
```

# Entity
``` json
"entities": [
  {
    "name": "",
    "components": [
      { <components> },
      ...
    ]
  },
  ...
]
```

# Enum
``` json
"enums": [
  {
    "name": "",
    "members": [
      [ "member_name", { <type> } ],
      ...
    ]
  },
  ...
}
```

# Union
``` json
"unions": [
  {
    "name": "",
    "members": [
      [ "member_name", { <type> } ],
      ...
    ]
  },
  ...
}
```

# Flag
``` json
"flags": [
  {
    "name": "",
    "members": [
      [ "member_name", { <type> } ],
      ...
    ],
    "underlying_type": "<base_type>"
    },
    ...
  },
  ...
]
```

# Global
``` json
"globals": [
  {
    "name": "",
    "type": { <type> },
    "is_const": false
  },
  ...
]
```

# Typealias
``` json
"typealiases": [
  {
    "name": "",
    "type": { <type> }
  },
  ...
]
```

# function
`call_convention` must have one string value: `C` `std_call` `fast_call` `vector_call` `systemv`
`param_names` must have the same size as `prototype` parameter list


``` json
"functions": [
  {
    "name": "",
    "call_convention": "C|std_call|fast_call|vector_call|systemv",
    "prototype": { <prototype> },
    "param_names": [ "param_name", ... ]
  },
  ...
]
```

