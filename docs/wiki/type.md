# Type
## Primitives
| type | syntax | literal | size |
|-|-|-|-|
| boolean | `bool` | `true` `false` | 1 bit (but 8 bit aligned) |
| binary | `bsize` `b8`-`b128` | `0b10010010` `0x0F` `0b64` | 8-128 bits |
| integral | `isize` `i8`-`i128` | `0` `-1` `1i` `10i32` default integral | 8-128 bits |
| unsigned | `usize` `u8`-`u128` | `1u` `10u64` | 8-128 bits |
| floating (floatting-point) | `fsize` `f32`-`f128` | `0.0` `.0` `-1.0f` `10f64` default point | 32-128 bits |
| decimal (fixed-point) | `dN` `dN:S` `32dN`-`128dN` | `200.45d2` `10d4:32` | 32-128 bits, N is decimal scale (static), S is bits size (32, 64, 128) |
| unsigned decimal | `udN` `udN:S` `32udN`-`128udN` | `200.45ud2` `10ud4:32` | 32-128 bits, N is decimal scale (static), S is bits size (32, 64, 128) |
| no type | `u0` |  | 
| cunei | `cune` | `"a"cune` `"a"cu` default one character | 8 bits textual storage, can be partial codepoint or simple ascii character |
| string | `str` | `"hello"s` `"hello"str` default literal text | fat pointer `{ data: ptr'cune, size: i32 }` null terminated, i/o encoding dependent  |
| C string | `c_str` | `"hello"c_str` `"hello"c` | raw pointer `ptr'cune` , null terminated : C convention |
| rune | `rune` | `"⚜"rune` `"⚜"r` `"⚜"` | 32 bits : 1 code point, encoding utf32  |                   
| text | `text` | `"hello"t` `"world"` | fat pointer `{ data: ptr'rune, size: i32 }`, encoding utf32 |
| opaque ptr | `ptr'u0` |  | bsize bit |
| unique ptr | `uptr'T` | `uptr'i32` | bsize bit |
| shared ptr | `sptr'u0` | `sptr'i32` | bsize bit |
| function prototype | `fn() -> ()` | `fn(i32, i32) -> (i32)` | |

## Literal strings

Literal type annotation
| type | syntax | position | info |
|-|-|-|-|
| literal string | `"my string"` | none | default string, returns `str` type (`{ ptr'cune, i32 }`) |
| literal c string | `"my c string"c` | suffix | c convention string, returns `c_str` type (`ptr'cune`) |
| literal text string | `"my text string"t` | suffix | explicit utf32, returns `text` type (`{ ptr'rune, i32 }`) |

Literal reading mode
| mode | syntax | position | info |
|-|-|-|-|
| partial raw literal | `r"C:my\windows\path"` | prefix | disable escape, can't read `"` character |
| total raw literal | `r#"my "particular" string"#` | prefix and suffix | disable escape and can read `"` character, can't read `"#` |
| multiline literal | `"""`</br>`  my multiline`</br>`  and aligned`</br>`  string`</br>`"""` | prefix and suffix | enable escape, can read `"` or `""` character, not `"""`, read start on the first line after `"""`, read end on the line last line before `"""` |
| raw multiline literal | `r"""`</br>`  my multiline`</br>`  and aligned`</br>`  string`</br>`"""` | prefix and suffix | disable escape, can read `"` or `""` character, not `"""`, read start on the first line after `"""`, read end on the line last line before `"""` |

> Note: Literal type annotation and literal reading mode are cumulative on the same literal

## Complex
| type | syntax | literal | size |
|-|-|-|-|
| enumeration | `enum EMyEnum<T>{ Some(T), None }` | `EMyEnum::Some(10)` | size of the biggest type + index `usize`
| flag | `flag FMyFlag { read, write, access }` | `FMyFlag::read` | `u8` 
| union | `union UMyUnion { i: i32; u: u64 }` | `UMyUnion.i = ...` `var a = UMyUnion.i` | size of the biggest type
| generic | `gen GNumeric<T> { ... }` | `var a: GNumeric = ...` | 
| facet | `facet FID { name: str = "", age: i32 = 20, id: b64 = 0x0 }` | `var marc = CID{ .name= "Marc", .age= 40, .id= 0x00FFF }` | list of related data | sum of type + offset on largest
| form | `form Player { use CPosition, use CID { .name= "Player" } }` | `var player2 = Player{ CID.name= "Player2", CID.id= 0x00AAA }` | list of facet for static composition | sum of facets (and offset)

### Implicit Cast
no memory loss allowed 

|      type      |                   cast to                 |
|-|-|
| numeric        | `b/u/i 8` -> `b/u/i 16` -> `b/u/i 32` -> `b/u/i 64` -> `b/u/i 128` (`b/u/i size` are api dependend) |
| numeric        | numeric -> floating                      |
| floating       | `f32` -> `f64` -> `f128` (`fisize` is api dependend) |
| decimal        | inferior decimal -> superior decimal      |
| cunei          | `cune` -> `rune` ascii -> utf32 |
| str            | `str` -> `text` place every str ascii on last utf32 byte (Big endian) |
| text           | `text` -> `str` place every utf32 4 bytes (Big endian) on every str ascii (except for the ascii compatible characters) 

> note a casting with binary is always a reinterpret cast 

### Forbid/Allow implicit cast
```
# no cast implicit
# scope
  let a: i32 = 10
  var c: i64 = a // compilation error
# end
var d: i64 = a // no compilation error
```

# Type Modifier
There is 6 main type modifiers:
| modifier | syntax | info
|-|-|-|
| constant | `T$` | cannot be modified |
| optional | `T?` | sugar syntax of `Option<T>` |
| volatile | `T!` | value can be modified by the hardware, avoid optimization | 
| raw ptr  | `ptr'T` | raw pointer |
| unique ptr | `uptr'T` | unique pointer |
| shared ptr | `sptr'T` | shared pointer |

> Note: the modifier can be placed before or after the type (except pointers), is only must be juxtaposed outside the targereted type
> Note: `$` `?` `!` are cumulatives : `i32?!`-> optional i32 with volatile possibility 

Placement e.g. (place possible: `@`)
| rule | syntax | info |
|-|-|-|
| `@T@` | `i32?` | `i32` is optional `?`
| `@T<@GenArg@>@` | `Vec<i32$>?` | `Vec` is optional `?`, `i32` is constant `$`
| `@[@i32@]@` | `[i32?]$` | Dynamic table is constant `$`, `i32` is optional `?`
| `@ptr'@T@` | `ptr'[i32]` | Raw pointer on dynamic table of `i32`
| `@ptr'@[@i32@]@` | `$ptr'[i32]` | Constant raw pointer on dynamic table of `i32`
| `@[@ptr'@i32@]@` | `[uptr'i32]` | dynamic table of unique pointer on `i32` 

# Type Alias
designed to reuse parametred type
```
type <name>: <expression>
```


# Cast
| type | syntax | note
|-|-|-|
| static cast | `<value> as <type>` | always successful |
| safe cast | `<value> as? <type>` | optional return `T?` |
| reinterpret cast | `<value> as! <type>` | getelementptr on speficied type 


