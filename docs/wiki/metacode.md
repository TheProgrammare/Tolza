
# Metaprogrammation
metaprogrammation is behaviour declarative who starts with `#`
can define some behaviour : module exportation, async, parallel, contigous memory alignment, etc...

## Cumulative Metacode
Cumulative metacode union behaviours when the new line have `#`

If the new line don't have # the metacode is no longer cumulative
```
# const
# extern

fn function<T>(a: T) -> i32 {...}
```
Explanation: 
The function will only be generic and not exported and not promise const because an new line without # separate them

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

## Metacode Block
You can reuse metacode with names, parameters can be passed
```
# meta name(<params>) -> <target>
# // metacode instructions
```
Explanation:
Target permit to precise the object applied `fn|type|lam|var|let|gen|...`
If the target is not specified a warning will be triggered

```
# meta metacode_reused_name(timeout_: f32) -> fn
# timeout timeout_
# pure

# metacode_reused_name(10)
fn fonc_example() {};
```

metacode block can be exported
```
export {
  # meta metacode_reused() -> fn
  # pure
}
```

## Scoped Metacode
The replication of the metacode is permitted by:
- `# scope ... # end scope` metacode scope 
- `# scope name ... # end scope` named metacode scode

>The scope is the end of the metacode block influence

It's possible to filter metacode parent/child scoping with:
- `# exclude` `# exclude all` -> disable all parent scopes
- `# exclude name` -> disable the named parent scope

> Note: Scopes are not interdependent, an exclusion of an specific scope disable only the named scope, the superiors or inferiors scopes will not be impacted.

e.g.
```
# if os == windows or os == linux

# async main
# scope
export operations {
  fn add() {}
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
  
  # if os == linux
  fn lin_convert_pos(lat: i32, long: i32) { ... }
  # elif os == windows
  fn win_convert_pos(lat: i32, long: i32) { ... }
  # end if // elif

  # exclude
  fn update_pos_ui() { ... }
}
# end scope // async main
# end if // os == windows or os == linux
```

## Metacode Conditional
metacodes can set condition during the compilation to compile or exclude code parts

| name | syntax | e.g. | info |
|-|-|-|-|
| if condition | `# if ...` | `# if some_user_define`,<br> `#  user_define == 10` | |
| elif condition | `# elif ...` | `# elif other_user_define` | |
| else condition | `# else` | `# else` | |
| end condition | `# end if` | `# end if` | to terminate the conditional metacode | |

> there is some native conditions for clean compilation and reading

| name | syntax | e.g. | info |
|-|-|-|-|
| bits condition | `# if bits == ...` | `# if bits == 64` | indicate the architecture bits (8/16/32/64) |
| architecture condition | `# if arch == ...` | `# if arch == x86_64` | indicate the architecture type (ARM64/x86_64/x86/IBM/...) |
| operating rule condition | `# if os == ...` | `# if os == linux` | indicate the operating rule (Linux/Windows/MacOS/...) |
| mode condition | `# if debug` | `# if debug` | for the compilation in debug mode |

## Metacode Expansion
metacodes can generate code before the compilation

- use `# expand` to set the code expansion 
- use `# for _NAME as a | b | c ...` to define the generation placeholder
- use `# each` to define the expansion scope

you can define multiple placeholders (it's became like a dimensions like table)

use placeholders in code with double square like: `[[_U]]`

> Note: the generation will iterate for all placeholders multiply by their alts

e.g.
```
# expand
# for _T as a | b | c
# for _U as 1 | 2
# each
  fn add(val: [[_T]]) -> isize {
    return [[_T]] + [[_U]]
  }
# end each// expand
``` 
will generate for

| `_U` | `_T` | | |
|-|-|-|-|
|   | a  | b  | c  |
| 1 | a1 | b1 | c1 |
| 2 | a2 | b2 | c2 |

You can set some exceptions or conditions during the expansion:
- use `# expand if ...` `# expand elif` `# expand else` `# end`
```
# expand
# for _T as i32 | i64 | i128
# for _U as f32 | f64 | f128
# each
  fn add(a: [[_T]], b: [[_T]]) -> [[_U]] {
    # expand if [[_U]] == f128 or [[_T]] == i128
    log::warning("usage of 128 bytes type, please check your architecture!")
    # end expand if
    
    return a + b;
  }
# end each
```


# Other Metacodes
## Script Target
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

## Function / Lambda Target (`fn`, `lam`)
| name | syntax | info 
|-|-|-|
| asynchrone execution | `# async` | mark the function as asynchronous. |
| parallel execution | `# parallel` | mark the function for parallel execution. |
| pure function | `# pure` | the function has no side-effects. |
| timeout execution (async) | `# timeout seconds` | maximum execution time. |
| inline | `# inline` | suggest compiler inlining. |
| unit test | `# test` | the function is a unit test. |
| for performance | `# benchmark` | the function is for performance benchmarking. |

## Form Target
| name | syntax | info 
|-|-|-|
|  | `# align N` | enforce memory alignment (e.g. `# align(8)`). |
|  | `# serializable` | allow automatic serialization. |

## Enum Target
| name | syntax | info 
|-|-|-|
|  | `# repr(type)` | define underlying representation (e.g. `i8`, `u32`). |

## Scope Target
| name | syntax | info 
|-|-|-|
| scope | `# scope ... # end scope` | define a named scope where metacodes apply. |
| scope | `# scope name ... # end scope` | define a named scope where metacodes apply. |
| exclusion | `# exclude all` | exclude current block from inherited metacodes. |
| exclusion | `# exclude name` | exclude a named scope only. |
