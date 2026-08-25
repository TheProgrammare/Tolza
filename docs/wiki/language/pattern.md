# Patterns
the pattern matching can be used from if/elif/while and match statement
```
[if/elif/while] [ref/mut] <pattern> = <expression> [if <condition>] {...}
```
pattern element kind | e.g. | info |
|-|-|-|
| bind value | `Some(a)` | bind value on a |
| matching value | `("Marc", 20)` | compare tuple with value "Marc" and 20 |
| ignore value | `(a, _, b)` | ignore the second element |

| pattern kind | e.g. |
|-|-|
| enum pattern | `MyEnum::Elem(a, _, 10) = <expression>` |
| tuple pattern | `(a, _, 7, "hello") = <expression>` |
| form pattern | `Player{CId.name: name, CId.id 10} = <expression>` |
| form pattern | `Player{CId{name: name, id 10}} = <expression>` |
| facet pattern | `CId{name: name, id: 10} = <expression>` |

## Pattern Binding Mode
| Binding Mode | syntax | info |
|-|-|-|
| Bind | `ref (a)` | Copy all primitives, Ref all complex types |
| Bind | `mut (a)` | Mut all primitives, Mut all complex types |
| Override bind by copy | `(copy a)` | Will read the value and put a copy in binding |
| Override bind by clone | `(clone a)` | Will read the value and put a clone in binding |
| Override bind by mut | `(mut a)` | Will add a mut capability in binding |
| Override bind by ref | `(ref a)` | Will add a ref capability in binding |
| Override bind by move | `(move a)` | Move the value in binding and invalid the origin |

> Note: Override a bind by * will ignore general `ref`/`mut` variables declaration

