# Modules

Modules are namespace and script interface representation.

There are two types of modules:
- inline module: `mod <name> {...}`
- file module: `my_script.vlx`

# Access logic

Access resolution order:
1. lexical scope
2. file scope
3. cross-file scope

**Lexical scope:**
- Private to the block where it is defined
- Accessible only within its own sub-scopes
- Cannot be accessed from outside its lexical boundary

**File scope:**
- Accessible anywhere within the same file module
- Access often requires explicit path resolution (e.g. `a::b::c`)
- Not visible outside the file unless explicitly exported

**Cross-file scope:**
- Represents visibility between file modules
- Only exported members are visible across files
- Non-exported members are never reachable from other files

# General rule

Almost all members are file scope by default.

To enforce file-scope members to become lexical-scope members, use: `# private`

# Inline module

Inline modules are classic namespaces.
They are file scope by default.
Modules names are in a different namespace of values and types.

**Usage:**
- namespace creation: `mod <name> {...}`
- namespace access: `My_Namespace::my_function`

## Access rules

| access type         | definition                                                      | example                        | mode                 |
|---------------------|-----------------------------------------------------------------|--------------------------------|----------------------|
| vertical ascending  | parent module accessing child elements                          | `fn z() { a::x() }`            | explicit only        |
| vertical descending | child accessing parent or sibling modules                       | `fn x() { b::z() }`            | explicit only |
| horizontal          | sibling-to-sibling access (are in same module)                  | `fn x() { y() }`               | explicit or relative |

Unqualified identifiers are resolved only within:
1. lexical scope
2. current module scope

They MUST NOT resolve to parent, sibling, or cross-file modules implicitly.


## Special access

| mode   | syntax                         | info                          |
|--------|--------------------------------|-------------------------------|
| root   | `::my_module::my_fn()`         | resolves from root scope      |
| self   | `self::my_sub_module::my_fn()` | resolves from current scope   |
| parent | `super::my_module::my_fn()`    | resolves from parent scope    |

## Module alias

You can simplify module access:
```
mod fs = core::filerule
```
**Usage:**
```
fs::path
```

Module aliases are file scope.

# File module

All files are file modules.

Only exported file members are cross-file scope.

## Export file

To expose a file module, define a unique export scope:
```
export { ... }
```
This is the file interface.

**Rules:**
- Cross-file scope is strictly derived from the export block
- Only file-scope members can be exported
- Lexical-scope members are never exported (including import results)

## Import file

To use other file code you must indicate explicitly a importation:
```
import <path> [as <alias>]
```
It will make a shortcut to the last identifier(s) (or alias) in the current scope 

special path prefix:
- `std::` search on standard library
- `src::` search on the root of the project source (filerule structure)
- `pkg::` search on packages installed
- `bind::` search on bindings
- none search first on relative file (filerule structure)
  - then try other path -> src -> std -> pkg -> bind


**Rules:**
- Import is lexical scope only
- Import remains private even inside export scope
- Imports can reference:
  - file modules
  - inline modules
  - specific elements

**Examples:**
Import module
```
import std::math::scientific

fn sin() {
  return scientific::sin(10)
}
```

Import items
```
import std::math::{ foo, cos, sin }
import std::math::scientific::{ foo, cos, sin } as { sfoo, scos, ssin }
import std::random::linear_rand as lrand

fn main() {
  let result = cos(10)
  let w = lrand(1)
  let a = sfoo(10)
}
```

**Note:**
A path import can resolve to:
- a sub file module
- an inline module
depending on parent module structure

## Export imported file

To expose an imported module, use:
```
reexport <path> [as <alias>]
```

> Trick: it's like a export import of module

**Rules:**
- reexport is file scope
- it can be included in export { ... }
- it exposes a sub file module as part of the current file interface




