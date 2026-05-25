<div align="center">
  
# JSON Specification for FFI AST

</div>

This document defines the **JSON convention** used to represent an **Abstract Syntax Tree (AST)** for FFI (Foreign Function Interface) bindings.  
The JSON file contains all the necessary information to define **functions, types, structures, entities, enums, unions, flags, and aliases** exposed by an external library.

Each AST section is represented by a JSON object with specific fields, strictly defined to ensure reliable conversion into the corresponding Velox structures (`functions`, `type`, `components`, `entities`, `globals`, `enums`, `unions`, `flags`, `typealiases`).

## Key conventions
- **Types** are described by objects containing the base type (`base_type`) and additional attributes such as pointer, array, const/volatile qualifiers, and atomicity.
- **Imports** (`imports`) contain name, path, importation type.
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
  "abi": "",
  "lang": "",
  "lib": ""
}
```

# Import
`type` must have one of this value: `user` `stdlib` `lib` `any`

```json
"imports": [
  "my_module_name": {
    "path": [
      "module_path_elem_1",
      ...
    ],
    "type": "user"
  },
  ...
]
```

# Type
`kind` must have one of this value: `primitive` `textual` `Tuple` `static_array` `ptr` `dynamic_array` `prototype` `identifier`

`primitive` must have one of this value: `u0` `bool` `cune` `rune` `ssize` `s8` `s16` `s32` `s64` `s128` `usize` `u8` `u16` `u32` `u64` `u128` `bsize` `b8` `b16` `b32` `b64` `b128` `ptrdiff` `fsize` `f16` `f32` `f64` `f80` `f128` `dsize` `d32` `d64` `d128` `udsize` `ud32` `ud64` `ud128` `ptr` 

`decorator` must have some or nothing of this value: `const` `volatile` `optional` (concatenate with `,`)

`str_kind` must have one of this value: `c_str` `str` `rune` `cune` `text`

`type_data` must be the type name or the type json structure 

> if you have already define a named type by his declaration (`comp` `role` `entity` `flag` `enum` `union`), use directly `Identifier` type

## base type
``` json
"type": {
  "kind": "my_kind",
  "decorator": "volatile,const",
  "data": { <type_data> }
}
```
## primitive type
``` json
"data": {
  "primitive": "bool"
}
```
## Ptr type
``` json
"data": {
  "inner": { <type_data> }
}
```
## Textual type
``` json
"data": {
  "kind": "str"
}
```
## Static array type
``` json
"data": {
  "inner": { <type_data> }
"size": 10
}
```
## dynamic array type
``` json
"data": {
  "inner": { <type_data> }
}
```
## Tuple type
``` json
"data": {
  "types": [
    { <type_data> }
  ]
}
```
## Prototype type
for params, check function section

``` json
"data": {
  "ret_type": { <type> },
  "is_variadic": false, 
  "params": [ <parameters> ]
}
```
## Identifier type
``` json
"data": {
  "name": "my_role" 
}
```




# Component
``` json
"components": [
  "my_comp_name": {
    "fields": [
      <field>
      ...
    ],
  },
  ...
]
```

## Field
`kind` must have one of this value `var` `ref` `mut` 
```json
"field_name": {
  "kind": "",
  "type": { <type> }
}
```

# Entity
``` json
"entities": [
  "my_entity_name": {
    "components": [
      "my_component_name",
      ...
    ]
  },
  ...
]
```

# Enum
``` json
"enums": [
  "my_enum_name": {
    "variants": [
      { "my_field_name": { <type> } },
      ...
    ]
  },
  ...
}
```

# Union
``` json
"unions": [
  "my_union_name": {
    "variants": [
      { "my_field_name": { <type> } },
      ...
    ]
  },
  ...
}
```

# Flag
``` json
"flags": [
  "my_flag_name": {
    "fields": [
      "my_field_name",
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
  "my_global_name": {
    "type": { <type> },
    "is_const": false
  },
  ...
]
```

# Typealias
``` json
"typealiases": [
  "my_typealias_name": {
    "type": { <type> }
  },
  ...
]
```

# function
`call_convention` must have one string value: `cdecl` `stdcall` `fastcall` `thiscall` `sysv` `win64` `aapcs` `aapcs_vfp` `vectorcall` `custom` `unknown`

`param_names` must have the same size as `prototype` parameter list


``` json
"functions": [
  "my_func_name": {
    "call_convention": "unknown",
    "prototype": { <prototype> },
    "param_names": [ "param_name", ... ]
  },
  ...
]
```


## Parameter
`pass_mode` must have one string value: `copy` `ref` `mut` `move` `addr`

``` json
[
  {
    "pass_mode": "my_passmode",
    "type": { <type> },
    "is_restrict": false
  },
  ...
]
```

