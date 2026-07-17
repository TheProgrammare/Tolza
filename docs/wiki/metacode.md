
# Metaprogrammation
metaprogrammation is a compilation time behaviour designed to add some information to the compiler, change the ast from compile time expressions, macros, and for code reflexion

# meta statement
Meta statements are block of code designed to modify the ast from compilation time conditions

> use the keyword `meta` to indicate a meta statement

> they can be used in global scope and local scopes

| statement | syntax | effect |
|-|-|-|
| if | `meta if <compiletime cond> {...} meta elif {...} meta else {...}` | only code inside the valid statement will be parsed |
| for | `meta for i in <range/collection/enum/union> {}` | will duplicate code inside his scope, the index/element can be used to inject token inside the scope. Can iterate on enum and union. |
| match | `meta match <compiletime expr> {}` | only code inside the valid case will be parsed |

> condiition can be a defined macro

# meta expression
Meta expressions are defined at compilation time, consider as special function like inside the module `meta`

> Note: the `const` variable is _de facto_ a meta expression and usable inside meta statment and expressions

There is some builtin meta expressions
| expression | syntax | effect |
|-|-|-|
| size | `meta::size(...)` | returns the memory size of the node: variable/definition/type... |
| type | `meta::type(...)` | returns the type of the expression |
| name | `meta::name(...)` | returns the symbolic name of the node: variable/field/definition/identifier/type... |

# meta declaration
Consider as compiler helper ans tips. Resolvers (type/symbol/semantic) will use them to check the user intention declared thought his meta declarations. 

> Use the keyword `#` to indicate a meta declaration

> Meta declarations can be cumulates on same line or next lines

## behaviour
If the new line don't have `#`, the metacode is no longer cumulative
```
# const
# extern

fn function<T>(a: T) -> i32 {...}
```
Explanation: 
- The function will only be generic and not exported and not promise const because an new line without `#` separate them

e.g.
```
# const
fn functionOnThreads() {}

# align 8
facet City {
  code: i32 = 0,
  name: str = "",
}

# const 
fn sum() {} 
```

Cumulative metacode can be in a unique line:
```
# const # extern
fn sum() {}
```

## Block
You can reuse metacode with names, parameters can be passed
```
# // metacode instructions
meta name(<params>) -> <target>
```
Explanation:
Target permit to precise the object applied `fn|type|lam|var|let|gen|...`
If the target is not specified a warning will be triggered

```
# timeout timeout_
# pure
meta metacode_reused_name(timeout_: f32) -> fn

# metacode_reused_name(10)
fn fonc_example() {};
```

metacode block can be exported
```
export {
  # pure
  meta metacode_reused() -> fn
}
```

## Scoped Metacode
The replication of the metacode is permitted by:
- `meta {...}` metacode scope 
- `meta name {...}` named metacode scode

>The scope is the end of the metacode block influence

It's possible to filter metacode parent/child scoping with:
- `# exclude` `# exclude all` -> disable all parent scopes
- `# exclude name` -> disable the named parent scope

> Note: Scopes are not interdependent, an exclusion of an specific scope disable only the named scope, the superiors or inferiors scopes will not be impacted.

e.g.
```
meta if os == windows or os == linux {

# async
meta {
export operations {
  fn add() {...}
  facet CMap_pos { 
    var lat: i32 = 0
    var long: i32 = 0 
  }

  form THouse {
    use CMap_pos
  }

  rule get_map_pos -> (lat: i32, long: i32) {
    CMap_pos(pos) {
      return pos.lat, pos.long    
    } 
  }
  
  meta if os == linux {
  fn lin_convert_pos(lat: i32, long: i32) { ... }
  } meta elif os == windows {
  fn win_convert_pos(lat: i32, long: i32) { ... }
  } meta elif other { // compilation macro 
  ...
  }

  # exclude // meta scope excluded
  fn update_pos_ui() { ... }
}
} // end meta scope
} // end if os == windows or os == linux
```

## builtin declarations
### Script Target
| name | syntax | info
|-|-|-|
| author name | `# author "name"` | declare the script author. |
| title | `# title "name"` | declare the script title. |
| code version | `# version "0.0.0"` | script version. |
| description | `# description "text"` | provide a script description. |
| creation date | `# created "date"` | provide a creation date. |
| last update date| `# updated "date"` | provide a update date. |
| language version | `# language_version "0.0.0"` | code language version. |
| encodign script | `# encoding "UTF-8"` | file encoding. |
| license | `# license ""` | |
| contributors | `# contributors ""` | | 
| contact | `# contact ""` | |
| copyright| `# copyright ""` | |
| wiki | `# wiki ""` | |
| documentation | `# doc ""` | |
| operating rule used | `# os ""` | |

### Function / Lambda Target (`fn`, `lam`)
| name | syntax | info 
|-|-|-|
| asynchrone execution | `# async` | mark the function as asynchronous. |
| parallel execution | `# parallel` | mark the function for parallel execution. |
| pure function | `# pure` | the function has no side-effects. |
| timeout execution (async) | `# timeout seconds` | maximum execution time. |
| inline | `# inline` | suggest compiler inlining. |
| unit test | `# test` | the function is a unit test. |
| for performance | `# benchmark` | the function is for performance benchmarking. |

### Form Target
| name | syntax | info 
|-|-|-|
|  | `# align N` | enforce memory alignment (e.g. `# align(8)`). |
|  | `# serializable` | allow automatic serialization. |

### Enum Target
| name | syntax | info 
|-|-|-|
|  | `# repr(type)` | define underlying representation (e.g. `i8`, `u32`). |
