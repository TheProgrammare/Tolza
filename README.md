# Velox

A hybrid-level programming language designed to emphasize safety and simplicity through innovative syntax and paradigms.

This project has been under design and experimentation since 2024.  
The public repository reflects the current implementation.

Velox is currently in an experimental stage and should be considered a work in progress.

This README presents the vision and core ideas behind the language.  
Some features and syntax are still under development.

Velox is a next-generation programming language that aims to combine performance, simplicity, and extensibility. Inspired by modern systems programming practices and component-oriented architectures, Velox enables developers to write efficient, maintainable, and scalable code with minimal boilerplate.


# type
## primitives
|      type      |      syntax      |            literal           |      size      |
|-|-|-|-|
| boolean        | `bool`           | `true` `false`               | 1 bit          |
| binary         | `bsize`          | `0b10010010` `0x0F` `0bsize` | 8/16/32/64 bit |
| binary         | `b8`           | `0b10010010` `0x0F` `0b8`    | 8 bit          |
| binary         | `b16`           | `0b10010010` `0x0F` `0b16`   | 16 bit         |
| binary         | `b32`           | `0b10010010` `0x0F` `0b32`   | 32 bit         |
| binary         | `b64`           | `0b10010010` `0x0F` `0b64`   | 64 bit         |
| binary         | `b128`           | `0b10010010` `0x0F` `0b128`  | 128 bit        |
| integral       | `isize`          | `0 ` `-1` `10isize`         | 8/16/32/64 bit |
| integral       | `i8`           | `0 ` `-1` `10i8`            | 8 bit          |
| integral       | `i16`           | `0 ` `-1` `10i16`           | 16 bit         |
| integral       | `i32`           | `0 ` `-1` `10i32`           | 32 bit         |
| integral       | `i64`           | `0 ` `-1` `10i64`           | 64 bit         |
| integral       | `i128`           | `0 ` `-1` `10i128`          | 128 bit        |
| unsigned       | `usize`          | `0 ` `10usize`              | 8/16/32/64 bit |
| unsigned       | `u8`           | `0 ` `10u8`                 | 8 bit          |
| unsigned       | `u16`           | `0 ` `10u16`                | 16 bit         |
| unsigned       | `u32`           | `0 ` `10u32`                | 32 bit         |
| unsigned       | `u64`           | `0 ` `10u64`                | 64 bit         |
| unsigned       | `u128`           | `0 ` `10u128`               | 128 bit        |
| floating       | `fsize`          | `0.0f` `-1.0f` `10fsize`   | 32/64 bit      |
| floating       | `f32`           | `0.0f` `-1.0f` `10f32`     | 32 bit         |
| floating       | `f64`           | `0.0f` `-1.0f` `10f64`     | 64 bit         |
| floating       | `f128`           | `0.0f` `-1.0f` `10f128`    | 128 bit        |
| decimal        | `deci`           | `0.0` `-1.0` `10d` default | numbers*8  bit |
| udecimal       | `udeci`           | `10ud`                       | numbers*8  bit |
| decimal constructor    | `<size>d<size>`  | `3d2` -> `000.00`            | numbers*8  bit |
| no type        | `u0`, `void`   |  | 
| ascii          | `ascii`          | `"a"ascii`                   | 8 bits (latin1) |
| utf32      | `utf32`           | `"⚜"utf32` `"⚜"` default         | 32 bits  |                   
| string         | `str`            | `"hello"s`                   | ascii*len + 2*bsize (fat pointer) bit (latin1) |
| text | `text`           | `"hello"t` `"world"` default | utf32*len + 2*bsize (fat pointer) bit (utf32) |
| opaque ptr     | `addr`           | `...`                      | bsize bit      |
| static table   | `[T; N]`       | `{ 1, 2, 3, 4}`,<br> `{ 0..4 = 8 }` (4 elements equals to 8) | N*size + bisize (pointer) |
| static matrix   | `[T; N, N, ...]`,<br> `[T; N]*D` | `{{0,0,0},{0,0,0},{0,0,0}}` `{ 1, 2, 3, 4}*3` (make 3d matrix of 4 elements for each dimension) | N*size + bisize (pointer) |
| dynamic  table  | `[T]`            | same of static table, but literal is instanciation only | List entity |
| dynamic matrix  | `[T]*D` | same of static matrix, but literal is instanciation only  | Matrix entity    |

### implicit cast
no memory loss allowed 

|      type      |                   cast to                 |
|----------------|-------------------------------------------|
| binary         | `b8` -> `b16` -> `b32` -> `b64` -> `b128` |
| binary         | `bsize` -> `b32` or `bsize` -> `b64`      |
| integral       | `i8` -> `i16` -> `i32` -> `i64` -> `i128` |
| integral       | `isize` -> `i32` or `isize` -> `i64`      |
| integral       | integral -> floating                      |
| unsigned       | `u8` -> `u16` -> `u32` -> `u64` -> `u128` |
| unsigned       | `usize` -> `u32` or `usize` -> `u64`      |
| unsigned       | unsigned -> integral                      |
| unsigned       | unsigned -> floating                      |
| floating       | `f32` -> `f64` -> `f128`                  |
| floating       | `fsize` -> `f32` or `fsize` -> `f64`      |
| decimal        | inferior decimal -> superior decimal      |
| udecimal       | udecimal -> decimal                       |
| ascii          | `ascii` -> `char`                         |
| string         | `str` -> `text`                           |

### forbid/allow implicit cast

```
# no cast implicit
# scope
  let a: i32 = 10
  var c: i64 = a // compilation error
# end
var d: i64 = a // no compilation error
```

## native types (or in core lib)
Handled by the compiler

| type | syntax | description |
|-|-|-|
| unique ptr           | `uptr'T`                | unique pointers                  |
| shared ptr     | `sptr'T`                | shared pointer                   |
| enum           | `enum name {}`          | enumerator typed                 |
| flag           | `flag name {}`          | named bit                        |
| entity         | `entity name {}`        | entity                           |
| fn             | `fn name() -> T {}`     | function                         |
| fn ty          | `fn () -> ()`           | function signature               |
| Str            | `Str`                   | String type mutable (Latin-1)    |
| Text           | `Text`                  | Text type mutable (UTF32) access O(1) size `4*Str` |
| Decimal        | `100.00` or `5d2`       |                                  |
| List           | `List<T>`               | Dynamic array                    |

| name | syntax |
|-|-|
| dynamic table       | `List<T>`                                      |
| pointers            | `ptr'T` `uptr'T` `sptr'T`         |
| enumerator          | `enum name { a, b, c }` or typed `enum name<T> { Valid(T), Invalid }` |
| flag | `flag name { a, b, c }` -> it's a named byteset |
| entity | `entity name : parent { ... }` |
| fn | `fn name(a: T, b: U) -> (T, U) { ... } ` |
| fn type | `fn(T, U) -> (T, U, V)` |
| string | `str` -> encoding utf8, literal `"Hello World!"str`, format `f"Hello {name}"` |
| other string | `utf8`(same as `str`) `utf16` `utf32` -> good for traduction, literal `"Hello World!"utf16`, format `f"Hello {name}"utf32` |
| decimal | `deci<3, 2>`, literal/type definition `123.45` (default) or `1'234.5deci` |
| unsigned decimal | `udeci<3, 2>`, literal/type definition `123.45udeci` `100'000.000'000udeci` |
| tuple | `tuple<T, U, V>` |
| named tuple | `tuple<a: T, b: U>` |
| variant | `variant<T, U, V>` |

# sugar syntax
you can use some sugar to avoid the heavy standard syntax

| name | syntax |
|-|-|
| pointer  | `ptr'T` `uptr'T` `sptr'T`       |
| static table | `[T -> static_size]` |
| dynamic table | `[T]`,<br> `new ptr'[T -> variant_size]` |
| static matrix | `[T -> static_size]*N`,<br> `[T -> static_size, static_size, ...]` |
| dynamic matrix | `[T]*N`<br> `[T]`<br> `new ptr'[T -> variant_size, variant_size, ...]`,<br> `new ptr'[T -> variant_size]*N` |
| const | `$T` |
| special cases |
| table of const | `$[T]`,<br> `$[T -> size]`,<br> `[$T]`,<br> `[$T -> size]` |
| const ptr | `$ptr'T` T mutable |
| const pointee | `ptr'$T` ptr mutable |
| optional | `T?` |
| optional | special cases |
| optional table | `[T]?`,<br> `[T -> size]?` |
| table of optional | `[T?]`,<br> `[T? -> size]` |
| optional ptr |`ptr?'T` |
| optional pointee | `ptr'T?` |

# operators
|      name      |    Syntax    |       description       |
|----------------|--------------|-------------------------|
| memory | | |
| copy           | `=` or `copy=`  | assignation by copy ()   |
| ref            | `ref=`       | assignation by ref borrow immutable |
| mut            | `mut=`       | assignation by mutable borrow |
| move           | `move=`      | assignation by move     |
| arithmetic (consider version of assignation operation)| | |
| add            | `+`          | works ont pointers too  |
| subtract       | `-`          | works ont pointers too  |
| multiply       | `*`          |                         |
| matrix multiplication | `@` | only on matrices |
| divide         | `/`          | result always f32/f64   |
| remain         | `%rem%`      | remainder singed with dividend |
| modulo         | `%mod%`      | euclidian remainder (modulo not negative if divisor>0) |
| quotient       | `%quo%`      | euclidian quotient      |
| power          | `**`         |                         |
| increment      | `++`         |                         |
| decrement      | `--`         |                         |
| comparator | | |
| greater        | `>`          |                         |
| lower          | `<`          |                         |
| greater equal  | `>=`         |                         |
| lower equal    | `<=`         |                         |
| equal          | `==`         |                         |
| equal strictly | `===`        | for string and float    |
| not equal      | `!=`         |                         |
| not eq strictly| `!==`        | for string and float    |
| logical | | |
| and    and.b   |`and` `and.b`  |                         |
| nand   nand.b  |`nand` `nand.b`|                         |
| or     or.b    |`or` `or.b`    |                         |
| xor    xor.b   |`xor` `xor.b`  |                         |
| nor    nor.b   |`nor` `nor.b`  |                         |
| xnor   xnor.b  |`xnor` `xnor.b`|                         |
| binary | | |
| shift left 0   | `<<[0]`      | fill right with 0       |
| shift left 1   | `<<[1]`      | fill right with 1       |
| shift right 0  | `[0]>>`      | fill left with 0        |
| shift right 1  | `[1]>>`      | fill left with 1        |
| shift left a   | `<<[a]`      | fill right with MSB (arithmetic) |
| shift right a  | `[a]>>`      | fill left with MSB (arithmetic) |
| rotate left    | `<<[r]`      | rotate bits to the left |
| rotate right   | `[r]>>`      | rotate bits to the right |
| slice bits     | `:[0..8]`    | get bits from range     |


# type alias
designed to reuse parametred type

Syntax:
```
type <name> = <expression>
```

e.g.
```
type tResult = future::Future<i32>

# async
fn calculation() -> tResult {
  // operation
}

var future_result = calculation()
var future_result: tResult = calculation()
``` 

# table population
when you create a table instance `{ a, b, ... }` you can avoid the explicit value affectation and use an table population syntax to put in table literal !

| type | syntax | e.g. | explicit form | info |
|-|-|-|-|-|
| table population | `[<range>] => <expression>` | `{ [0..3] => 5 }`,<br> `{ [0..3] => rand::uniform() }` | `{ 5, 5, 5 }`,<br> `{ 0.25f, 0.777f, 0.05f }` | the most simple case |
| table population index relative | `@i`, `@j`, ... | `{ [0..3] => @i + 10 }` | `{ 10, 11, 12 }` | useful for position aware in population | 
| map population index relative | `[<range>] => <expression_keys> : <expression_values>` | `{ [0..3] => @i : number_text[@i] }` | `{ 0: "zero", 1: "one", 2; "two" }` |  | 
| matrix population | `{ [<range1>, <range2>, ...] => <expression> }`,<br> `{ [<range>] => <expression> }*N` | `{ [0..3, 0..3] => @i + @j + 10 }`,<br> `{ [0..3] => @i + @j + 10}*N` | `{{10,11,12},{11,12,13}}` | a good level of abstraction |

table population use compiler reserved indentifier to use the indexation during the table population:

- `@i` first dimension
- `@j` second dimension
- ... 

# reserved values
to set default value or uninit use: `null`

to set invalid address memory use: `nullptr`

# metaprogrammation
metaprogrammation is behaviour declarative who starts with `#`
can define some behaviour : module exportation, async, parallel, contigous memory alignment, etc...

## cumulative metacode
cumulative metacode union behaviours when the new line have # 

if the new line don't have # the metacode is no longer cumulative
```
# export
# async

# where T
fn function(a: T) -> i32 {

}
```
the function will only be generic and not exported and not async because an new line without # separate them
example:
```
# async
fn functionOnThreads() {}

# align 8
class City {
  var code: i32;
  var name: str;
}

// cumulate behaviour
# export
# async 
fn sum() {} 
```
cumulative metacode can be in a unique line
```
# export # async
fn sum() {}
```

## metacode block
you can reuse metacode with names, parameters can be passed 
```
# meta name(parameters) -> fn|var|let|class|trait|method|...
# // sub metacode
```
target permit to precise the object applied
if the target is not specified a warning will be triggered
```
# meta metacode_reused_name(timeout_: f32) -> func
# export
# async
# timeout timeout_

# metacode_reused_name
fn fonc_example() {};
```

metacode block can be exported
```
# export
# meta metacode_reused() -> fn
# export
# async
```

## metacode scoped
replication of the metacode specification to all objects in the scope 
use simply `# scope ... # end` or named scode `# scope name ... # end`.
the scope is the end of the metacode block

you can disable all metacode superior scope with: `# exclude` or for a specific scope: `# exclude name`
scopes are not interdependent, an exclusion of an specific scope disable only the named scope, the superiors or inferiors scopes will not be impacted.

example:
```
# if os == windows | linux

# async main
# scope
namespace operations {
  fn add() {}
  comp CMap_pos { 
    var lat: i32 = 0
    var long: i32 = 0 
  }

  entity THouse {
    use CMap_pos
  }

  sys get_map_pos -> (lat: i32, long: i32) {
    where m_pos: CMap_pos {
      return m_pos.lat, m_pos.long    
    } 
  }
  
  # if os == linux
  fn lin_convert_pos(lat: i32, long: i32) { ... }
  # elif os == windows
  fn win_convert_pos(lat: i32, long: i32) { ... }
  # end if // elif

  # exclude
  # export
  fn update_pos_ui() { ... }
}
# end if // scope
```

## metacode conditional
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
| operating system condition | `# if os == ...` | `# if os == linux` | indicate the operating system (Linux/Windows/MacOS/...) |
| mode condition | `# if debug` | `# if debug` | for the compilation in debug mode |

## metacode expansion
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
use `# expand if ...` `# expand elif` `# expand else` `# end`
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
 
## module alias
you can give an alias to an module (hightly not recommended)
`use fs = core::file_system`

# cast
| type | syntax | note
|-|-|-|
| static cast | `<value> as <type>` | always successful |
| safe cast | `<value> as? <type>` | optional return `T?` |
| reinterpret cast | `<value> as! <type>` | getelementptr on speficied type 


# type check
polymorphism/variant check
will return only boolean

| nature | syntax | description |
|-|-|-|
| composing check | `<comp> in <entity>` | check if entity have a component | 
| composing check | `<role> in <entity>` | check if entity have a role (sum of specific components) | 

# format string
formatted text literal:
```
"text{variable} and other {variable}"
```
same as: "text" + variable + "and other" + variable

## format specifier
you can use some parameter to specify the out formatation of variable formatted
```
[[<fill>]<align>][<sign>]["#"]["0"][<width>][<grouping_option>]["."<precision>][<type>]

```

use in format string like:
```
"duration {<expression>:<format_specifier>} s"
```


## format conditional (experimental)
you can specify some reaction with a comparison from the expression returned value:

Syntax:
```
(<comparison_op><value>:<return_text> [, <comparison_op><value>:<return_text>, ...])
```
special comparison operator:
```
other:<return_text>
```

In format string:
```
f"Gender: {<expression>}:<format_conditional>"
```

e.g.
```
var messages = 3
f"You have {messages}:(==0:no messages,==1:one message,>1:{messages} messages)"
var gender = "F"
f"{gender}:(=='F':'She is online', =='M':'He is online', other:'online')"
```

# function
> Use `fn` keyword to declare a function

syntax:
```
fn name([<pass_mode> <name>: <type> [= <default_value>] [, ...]]) [-> <return_type>] { ... }
```

| type | syntax | info |
|-|-|-|
| no return no param | `fn name() { ... }`
| one return | `fn name() -> T { ... }`
| multiple return | `fn name() -> (T, U) { ... }` | it's a tuple 
| multiple return | `fn name() -> (field1: T, field2: U) { ... }` | it's a named tuple 
| one param | `fn name(p_name: str) { ... }`
| multiple param | `fn name(p_name1: i32, p_name2: i32) { ... }`
| variant | `fn name(p_name: i32, p_args: addr...) { ... }`

> overloading prohibed

calling:

| type | syntax | info |
|-|-|-|
| discarded | `name()` | | 
| no discarded | `var result_name = name()` | |
| unpack result | `var (a, _, c) = name()` | `_` is for ignore field

## local variables
variables are declared by a kind:

- use `const` to make a compile time constant
- use `let` to make a constant variable
- use `var` to make a mutable variable

A variable always must be declared with a value

Syntax:
```
<kind> <name> [: <type>] = <value>
```

A variable can have unique and persistent memory address between calls with `# static`

e.g.
```
fn add_counter() -> i32 {
  # static
  var counter: i32 = 0; // will init at 0 but not reset to 0 between calls
  counter++;
  return counter;
}
```

## Pipe-call (experimental)
functions can call mutiple of times with differents parameters without too many syntax

- pure pipe-call begins by `callee |` return a tuple of each call 
- mutable pipe-call begins by `callee <-|` return a unique value concatened with all possibles operators (priority calculation only from left to right!)

Syntax pure pipe-call:
```
<callee_name> | <args1> [| <args2> | ...]
```

Syntax mutable pipe-call:
```
<callee_name> <-| <args1> [|<operator> <args2> | ...]
```

you can call nested functions in pipe-call

the type verification is static

| type | syntax |
|-|-|
| mutable pip-call | `let result: f32 = sum <-| 10 |+ 2.0 |+ avg(a, b, c) |+ k |+ "100" as f32 |+ 10.5;` |
| pure pip-call | `let position3D: (f32, f32, f32) = offset | x | y | z;`
| mutable pip-call generic args | `let result: f32 = sum <-| <i32> 10 |+ <f32> 2.0 |+ <i32> avg(a, b, c) |+ <i32> k |+ <f32> "100" as f32 |+ <f32> 10.5;` |
| pure pip-call generic args | `let position3D: (f32, f32, f32) = offset<f32> | x | y | z;` |

# lambda
> use `lam` keyword to declare a lambda, threated like c++ : anonym functions

Syntax:
```
lam [<name>]['['<capture>']'][(<params>)] [-> <return_type>] { ... }
```

| type | syntax |
|-|-|
| named lambda | `lam l_name[capture](parameters) -> return {...}` |
| anonym lambda for predicate purpose | `lam [capture](parameters) -> return {...}` |
| simple lambda | `lam {...}` |

> same calling as function

## lambda capture
to capture variables in scopes

| target | syntax | extension with exceptions |
|-|-|-|
| to modify all variables | `[mut]` | `[mut, copy a, copy b]` |
| to copy all variables | `[copy]` | `[copy, mut a, mut b]` |
| to get instance | `[self]` | `[..., self,...]` |

# parameters
parameters are managed by a pass mode and a type base

## parameter arrangement rule
call ordering: positional -> named -> variadic args

| behaviour | definition | call |
|-|-|-|
| obligatory param with positional args | `fn add(a: i32, b: str)` | `add(10, val)` |
| obligatory param with named args | `fn add(a: i32, b: str)` | `add(b= 10, a= val)` |
| optional param with no or positional args | `fn rand(s: f32 = 0.0)` | `rand()` or `rand(2.5)` |
| optional/obligatory param with positional and named args | `fn lerp(x: f32, a: f32 = 0, b: f32 = 1)` | `lerp(val)` or `rand(val, b= 100)` or `rand(b= 100, x= val)` |
| variadic param (variadic name is obligatory to specify the start of the variadic args). In a function call, variadic arguments must be prefixed by ... once, before the first variadic value. All subsequent values are considered part of the variadic list |  `fn sum(args: T...)` | `sum()` or `sum(... 10 as i32, 2.5 as f32)` | 
| optional/obligatory param with positional and named args and variadic args | `fn msg_add(msg: str, left: f32 = 0, right: f32 = 0, args: T...)` | `msg_add("result")` or `msg_add("result", 10.0)` or `msg_add("result", 10.0, right= 10 as f32, ... 10, 5.0, 6)`


## pass mode
default pass mode:

- primitive types : pass by copy (optimisation)
- complex types : pass by reference const

user pass mode:

| type | key | behaviour | syntax | info |
|-|-|-|-|-|
| reference pass mode | `ref` | borrow immutable, designed to avoid copy cost (for non primitive) | `foo(ref a: T)` | default value permitted (become optional argument) |
| mutable pass mode | `mut` | borrow mutable, designed to transfer modifications from function | `foo(mut a: T)` | default value prohibied (not optional argument) |
| copy pass mode | `copy` | force the copy (e.g. avoid threading cocurrency) you can specify | `foo(copy a: T)` | default value permitted (become optional argument) |
| move pass mode | `move` | move semantic : ref and invalidate origin | `foo(move a: T)` | default value prohibied (not optional argument) |
| address pass mode | `addr` | designed to modify the address of the pointer (pointers accepted only) | `foo(addr a: T)` | default value prohibied |
| variadic pass mode | `...` | variadic parameter (always the last parameter) you can specify a general pass mode | `sum(copy term: T...)` | default value prohibied (but optional argument)

# returns
multiple returns are handled (but the compiler pack them as a tuple)
complex types pass by copy or move semantic if local variables and primitives pass by copy by default
you can specify the pointer type in return

## calling
there is multiple ways:

| type | syntax |
|-|-|
| positional affectation | `add(value1, value2)` |
| positional + named affectation | `window("Title", length = 10, height = 10)` |
| positional + named affectation + variadic args | `scale2D_sum(scaleVal1, scale_dimension2 = scaleVal2, ... val1, val2, val3, val4)` |

if a value is passed without parameter specification, the standard order of affection will be used
you can use both as long as there is no parameter conflict and named affectation are at the end
variadic args are always at the end of arguments and after named parameters, the first non named parameter after named parameter is the begining of variadic arguments
if the function is not variadic, an error will occur because only named parameters are at the end

parameters no specified needs to have a default value 

# generics
> Use `gen` keyword to declare a generic type

technically, generic restriction return true (valid type) or false (invalid type) you can reuse generic with named generic

Syntax:
```
gen <name>'<'<typename> [, <typename2>, ...]'>' { .. }
```

## generics conditions:

syntax:
```
<typename> <kind> <symbol>
```

| filter kind | kind | syntax |
|-|-|-|
| operator filter | `op` | `T op *` |
| component filter | `comp` | `T comp Position` |
| role filter | `role` | `T role Merchant` |
| system filter | `sys` | `T sys Move` |
| cast filter | `cast to`<br> `cast from` | `T cast to i32` `T cast from i32` `T cast to U` (commutative : valid if at least one cast is compatible) |
| generic filter | `is` | `T is gen::base_of<CAnimal> | ...` or `T is Integral | Signed | i128 | ...` (first arg is left of `is`) can have alternative |

named generic example: 
```
gen GMoveable<T> { 
  T comp CPosition
  T use op +
  T use op - 
} 

gen GVelocity_Applied<T> {
  T is GMoveable
  T comp CVelocity
}
```

## generic use
generic usage on functions/component/entity/lambda: use metacode 

e.g.
```
# gen T, U
# where T is Numeric
# where U op +
fn add(a: T, b: U) {
  return a + b;
}
```

generic without condition is possible (not recommended)
```
# gen T
comp item { N: usize, value: T }
```

generic type usage is possible (not recommended)
```
fn add(a: GNumeric, b: GNumeric) {
  return a + b;
}
```

## generic use in COP and call
> Usage is the same as rust generic call type args

> It's possible tu specify a generic in the args

generic function call (turbofish used in calling)

syntax:
```
<name>::'<'type[, type, ...]'>'([<parameters>])
```

```
add::<GNumeric, i32>(10, 20)
```

entity with generic fixed component:
```
entity TCity {
  use CArray<THouse>
}
```

entity with generic component:
```
entity TAnimal<T> {
  use CLife<T>
}
```

component with generic fixed:
```
comp CNames { names: TArray<str> }
```

component with generic:
```
comp CLife<T: GDecimalScaled, U> { render: U, metabolism: Map<str, T> }
```

# flag
> Use `flag` keyword to declare a flag

Flag is a optimized named bits, max 255 elements, use only 8 bits

syntax:
```
flag FFileMode {
  Read, Write, Read_Write, Lock
}
```

# union
> Use `union` keyword to declare a union

Union is similar to an C union, so a non discriminant container who will consider the data as a type from the index in the input and output in the discretion of the user, no indexation or flag used to determine wich type is active.

Useful for C interop

syntax:
```
union UNumber {
  i: isize,
  f: fsize,
}
```

# enumerator
> Use `enum` keyword to declare a enumerator

C like enum:
```
enum ESpecices {
  Dog, Cat, Horse, Sheep
}
```
prefer the `flag` type to use C like enum and keep performances

rust like enum (typed):
```
enum EInteractIssue {
  ItemPickUp(Item, isize),
  Damage(isize),
  EarnCoins(isize)
  None,
}
```

to use enumerator

| type | syntax |
|-|-|
| access to a C like element | `EName::elem1` |
| access to a typed element | `EName::elem1` or `EName::elem1(val)` |

> bit enum type can be explicit cast with integrals : `1 as EName` => return EName::elem2

## match enumerator
enums are linked to the match mecanism
```
enum Optional<T> {
  Some(T),
  None,
}

let result = getVal<f32>(); // type Optional<f32>

match result {
  case Some(i) then println(i as f32::string(2));
  case None then println("Failure");
}
```

# COP paradigm (Compositional Oriented Programming)
COP, short for Compositional Oriented Programming, is a programming paradigm where entities are built through the static composition of components, without inheritance, without polymorphism, and without dynamic collections 

inspired by POO and ECS

An entity is defined by composition; components exist only within the entity, and systems operate by recognizing that composition.

- component is a contigous list of variables
- system is a behaviour who contains block of statement according to component combinaison

## component
> Use `comp` to declare a component

is a contigous list of variables, default values are required to avoid any undetermined value

Declaration:
```
comp CMathQuery { 
  name: str = "PI",
  val: f32 = 3.14f,
}
```
Initialization (in entity and/or in variable declaration): 
```
entity MathElement {
  use CMathQuery { name = "Phi", val = 1.618f } // default for this entity instanciation
}

fn start() {
  var math = MathElement { 
   CMathQuery.name = "e", 
    CMathQuery.val = 1.0f
  } // default for this variable declaration
}
```

> optional metadata annotation for component members

- no default value requied `# no default` really unsafe

A component used in function parameter is a guarantee of the presence of the values as long as the entity have the component expected.

### component field
A field is a primtive type or an entity, no nested component field are accepted. To keep the composition clean

Components can't handle a nested entity for contigous memory sanity and avoid infinitive structures loop. When an entity is specified, it's always a reference to an entity instance.

There is differents usage mod of entity who define the pointer type.

Filed entity typed mode 
| mode | syntax | behaviour |
|-|-|-|
| reference (default) | `use ref EntityT` | non nullable, same lifetime |
| optional reference | `use ref EntityT?` | nullable, same lifetime |
| pointer | `use ptr EntityT` | nullable, raw pointer, independent lifetime |
| pointer | `use sptr EntityT` | nullable, reference counted, lifetime shared |

## role
> Use `role` to declare a role
 
roles are a package of components to check if entities have some components
useful for simple generic functions or system case !

a role cannot be used in an entity composition, it's a generic/system_case/parameter guarantee shortcut

declaration:
```
role name { components, ... }
```

## entity
> Use `entity` to declare a entity

entites have a static composition of components who define his behaviour for systems and the accepted parameter arugment of component/role in fuctions.

declaration:
```
entity name { 
  use component ...
}
```

entity can contains:

| entity members | syntax | note |
|-|-|-|
| component | `use name { field: value }` | with default value | 
| component | `use name` | default value from component |
| cast | `cast self as T { ... }` | cast entity to antoher type, reserved key `self` and `other` used | 
| cast | `cast T as self { ... }` | cast entity from another type, reserved key `self` and `other` used |
| constructor | `new(params) { ... }` | overloading possible, must returns the same entity type |
| destructor | `del { ... }` | no parameter, reserved key `self` used | 
| operator | `op == { ... }` | all operators handled but type are restrictives |

examples:
```
comp Specie { name: Str = "" }
comp Position { x: f32 = 0, y: f32 = 0, z: f32 = 0 }
comp Job { name: Str = "no job" }

entity Animal {
  use Specie 
  use Position
  cast self to Human {
    var man = Human {Specie.name= "Human", Position= self.Position}
    return man
  }
}

entity Human {
  use Specie 
  use Position
  use Job
  cast self to Animal {
    var animal = Animal {Specie.name= "monkey", Position= self.Position}
    return animal
  }
}

```

### member usage

| meber type | usage syntax | note |
|-|-|-|
| call native constructor | `var cat = Cat{ CAnimal.name = "Ted", CAnimal.age = 2 }` |  not recommended |
| call custom constructor | `var cat = Cat::new("Ted", 2)` | clean constructor |

### entity operator overloading
there is some restrictions in operator definition:

| operator | other term | return type | syntax |
|-|-|-|-|
| `==` `!=` `===` `!==` `<` `>` `<=` `>=` `is` | self entity type | `bool` | `op <operator> { ... }` |
| `+` `-` `*` `/` `%mod%` `%quo%` `%rem%` `**` `<<[0]` `<<[1]` `<<[a]` `<<[r]` `<<[rc]` `[0]>>` `[1]>>` `[a]>>` `[r]>>` `[rc]>>` | self entity type | self entity type copy | `op <operator> { ... }` |
| `+=` `-=` `*=` `/=` `%mod%=` `%quo%=` `%rem%=` `**=` | custom type T | self entity type ref | `op <operator> <T> { ... }` |
| `Iter` iterator | none | `Iter<T>` | `op Iter<T> { ... }` |
| `[]` index | none | index always i64, custom type U | `op [i] -> U { ... }` |
| `[..]` range | none | custome slice U | `op [r: ..] -> mut'[U]` |


# system

A system is a composition-driven orchestration unit. It does not define behavior itself, but selects and orders behavior executions based on the set of components and roles present in an entity.
A system guarantees that any entity allowed to execute it will follow at least one valid case path derived from its composition, case path evaluation resolved at compile time.

declaration:
```
sys name<generic_parameters>(parameters) {
  CType1(ct1) + CType2(ct2) => { // case A
    call(ct1, param..., ct2)
    call(ct2)
  }
  Role_Type(rt) => { // case B
    call(rt)
    call(rt.CType1, rt.CType3)
    return
  }
  other => call() // case C
}
```

Flow explanation :   
- Entity have A + B + C composition -> run A + B and stop (return)
- Entity have A + C composition -> run A + C and stop (no more statement)
- Entity have A composition -> run A and stop (no other compatible statement)
- Entity have C composition -> run C and stop (no more statement)
- Entity have B + C composition -> run B and stop (return)

# COP in functions parameter
The COP paradigm can be used in functions to simplify the code and avoid the generic boilerplate

Keep in mind that any component and role type in parameter is values garantee.

## with components
CPosition and CPhysic guarantee the members values for the entity calling
```
type xyz_pos = (x: f32, y: f32, z: f32)

fn move_entity(mut pos: CPosition, mut phy: CPhysic, copy new_pos: xyz_pos, copy vel: f32) {
  pos.x copy= new_pos.x
  pos.y copy= new_pos.y
  pos.z copy= new_pos.z
  phy.vel copy= vel
}
```
the calling
```
var player = Player{CPosition.x= 100, CPosition.y= 100, CPosition.z= 100}
move_entity(player, player, (10.0, 20.0, 30.0), 5.0)
```
note a role or a system can simplify the function declaration and call:

## with role
```
role RMovable { CPosition, CPhysic }
fn move_entity_role(mut mov: RMovable, copy new_pos: xyz_pos, copy vel: f32) {
  mov.CPosition.x copy= new_pos.x
  mov.CPosition.y copy= new_pos.y
  mov.CPosition.z copy= new_pos.z
  mov.CPhysic.vel copy= vel
}
```
call with role
```
move_entity_role(player, (10.0, 20.0, 30.0), 5.0)
```

## with system
```
sys move(copy new_pos: xyz_pos, copy vel: f32) {
  CPosition(pos) + CPhysic(phy) {
    move_entity(pos, phy, new_pos, vel) 
  }
}
```
call with system
```
player::>move((10.0, 20.0, 30.0), 5.0)
```


# modules
modules permit to avoid naming collision, it's possible to use native types as modules

declaration:
```
mod name { ... }
```

call:
```
City::House::new(N: 37.0379f, E: 27.4241f)
```

to export module, use the instruction `export <name> { ... }` and use as a module

# variable
Variables are statically typed or type inferred

| type | syntax |
|-|-|
| variable typed | `var name: T` |
| variable typed setted | `var name: T = ...` |
| variable inferred setted | `var name = ...` |
| constant typed | `let name: T = ...` |
| constant inferred | `let name = ...` |

# variable affectation

| type | syntax | note |
|-|-|-|
| copy value    | `lvalue copy= rvalue / lvalue`     |  |
| reference value (no mutable)   | `lvalue ref= lvalue`   | multiple ref permitted but mutable ref are prohibied until all ref are removed before |
| mutable ref value | `lvalue mut= lvalue` | only one mutable ref permitted
| move semantic   | `lvalue move= lvalue`   | remove all ref and mutable ref anterior

# operations borrowing

| type | syntax | note |
|-|-|-|
| always borrow


# memory managment
The memory use a fine managment, there is no GC

Memory types:

| name | syntax | info |
|-|-|-|
| non typed memory address | `ptr'void` | useful for C interop (`void*`) |
| raw pointer | `ptr'T` | if no escape in the scope, will delete |
| unique pointer | `uptr'T` | use `move` to change his position and invalidate his last position |
| shared pointer | `sptr'T` | Use `mut` to add his reference to the counter and new position. A `move` will keep the counter, a `mut` will keep the counter if on another lvalue by the borrow rule

## pointer creation (on heap)
To use a memory space by pointer creation use `new` key, uses the C malloc

Allocate new memory space like:
```
new <ptr>'<type>(<value>)
```

e.g.
```
var halicarnassus: ptr'City::Monument = new ptr'City::Monument()
```

# tuple
tuples are implicit they a deduced most of the time in `( ... )`

tuple cases:

| name | syntax | info
|-|-|-|
| tuple type | `(T, U, ...)` | |
| tuple instance | `(val1, val2, ...)` | |
| named tuple type | `(name1: T, name2: U, ...)` | |
| named tuple instance | `(name1= val1, name2= val2)` | |  
| static access | `let first = a.0` | |
| static access named | `let first = a.name` | |
| dynamic access | `let first = a.get(k)` | |
| function return | `fn name() -> T, U, ...` | tuple return |
| function return | `fn name() -> name1: T, name2: U, ...` | named tuple return | 
| unpack tuple from call | `var a, b, c = name()` | |
| unpack tuple from variable | `var a, _, c = var_tuple` | |

## tuple cast
Tuples and named tuples are the same for the compilator but need to de distinguished for users to makes proprer conversion.

Conversions:

| cast type | base | syntax | result | info |
|-|-|-|-|-|
| positional | `let t1 = (10, 2.0, "hello")` | `t2 = t1 as (2, 1, 0)` | `t2 = ("hello", 2.0, 10)` | in each position, set the new position by the index |
| nomenclature | `let t1 = (a= 10, b= 2.0, c= "hello")` | `t2 = t1 as (c, b, a)` | `t2 = ("hello", 2.0, 10)` | in each position, set the new position by the field name | 

# if else elif statement
control flow by condition

| flow | syntax codeblock | syntax inline |
|-|-|
| if | `if <condition> { ... }` | `if <condition> then ...` |
| elif | `elif <condition> { ... }` | `elif <condition> then ...` |
| else | `else { ... }` | `else ...` |

# ternary if
Ternary if is a control flow for value

syntax:
```
if <condition> then <true statement>
if <condition> then <true_statement> else <false_statement>
```

# match statement
Match statement control flow for code logic by matching comparison on value. 

Execution stop to the case executed. Or use metacode `# fallthrough`

syntax:
```
match <value> {
  // case states ...
}
```

## case statement
Define inside match

| statement case | syntax |
|-|-|
| case codeblock | `<evaluator> => { ... }` |
| case inline | `<evaluator> => ...` |

e.g.

| type | syntax |
|-|-|
| compare literal integral | `10 =>` |
| compare literal string | `"hello" =>` |
| compare value | `if val > 100 =>` |
| compare in range | `if val in 10..=30 =>` |
| match typed enum and extract value | `Some(a) =>` |
| match on enum | `EEnum::Elem =>` |
| other case | `other =>` | 

# loop statement
Highlty not recommended
syntax:

| statement type | syntax |
|-|-|
| codeblock | `loop { ... }` |
| inline | `loop then ...` |

# while statement
syntax:

| statement type | syntax |
|-|-|
| codeblock | `while <condition> { ... }` |
| inline | `while <condition> then ...` |

# do-while statement
syntax:

| statement type | syntax |
|-|-|
| codeblock | `do { ... } while <condition>;` |
| inline | `do ... while <condition>;` |

# for loop statement
For loop can be used on range, slice, collection and map

Syntax:

| statement type | syntax | info |
|-|-|-|
| for index | `for <index> in <range> { ... }` | can use `step` after `<range>` |
| for mutable ref item | `for var <item> in <slice/collection> { ... }` | can be immutable with `let` instead of `var` |
| for key/val | `for let <key>, let <item> in <slice/collection> { ... }` | can be each immutable with `let` instead of `var` |

# goto statement
Highlty not recommended, designed for flexibility and code specific behaviour.

| state | syntax |
|-|-|
| go to and label | `goto name` |
| label definition | `label name:` |

# range statement
Borned with a start integral, end integral and optional step (only for `for` loop).

range can be use to extract slice from collection with special ranges.

| range type | syntax | info |
|-|-|-|
| range exclusive | `0..10` | will go from 0 to 9 |
| range inclusive | `0..=10` | will go from 0 to 10 |
| range all | `collection[..]` | will return the collection, not very useful there |
| range no begin | `..11` | will go from 0 or the start of the collection to the 10th index (so 11 elements slice) |
| range no end | `5..` | will go from 5 to the end of the collection or the max value `i64` |

# slice
Returns a view according to specified range, can be mutable and immutable

| slice type | syntax |
|-|-|
| immutable slice | `collection[start..end]` |
| mutable slice | `mut'collection[start..end]` | 

slice body
basically a fat pointer
```
slice { first_elem: ptr'T, length: usize }
```

# module import/export
The import and exportation of the code use the LLVM declare/extern
The modules are also modules namespaces to avoid any name collision (to avoid the module exported with his namespace, use `# native \n export <name> {...}`)

| type | syntax | e.g. | info |
|-|-|-|-|
| export module | optional: `[#native]` <br> `export <name> {...}` | `export math {...}` | Exports the current code as a module under its own namespace. If native is used, importing it will expose all symbols directly in the file’s root scope.  |
| export also the imported module (mirror) | `[#native]` <br> `export import <name>` | `export import math` | Declares that when this module is imported, the specified module(s) will also be imported automatically. If native is used, those symbols are also imported into the file’s root scope. | 
| export to other language | `export <name> extern <lang> {...}`  | `export math extern C {...}` | callable from another language | 
| import from module | `import <name>` | `import city` | import the code be the module name (by default is import user, else, will import from the standard lib) |
| import from standard module | `import @<name>` | `import @core` | import from the standard lib |
| import from user module | `import $<name>` | `import $math` | import from user script and imported lib |
| import from external language lib | `import extern <lang>::<lib>` | `import extern C::stdio` | C is natively handled, will generate automatically a parallel bind folder and imported in the script with the wrapper used |

> Elements exportable :
mod, entity, role, component, system, generics, function, global, enum, metacode

# Bindgen
Bindgen is external binds auto generated scripts when a user use `# import extern lang::lib`

The bindgen will store the importations of the external lib and generate all the wrapper needed when the user use the external lib in his script

user script: 
```
// entity_messages.vlx

import extern C::stdio

fn speak(s: str) {
  C::printf(s)
}
```

bindgen script:
```
// C_bind.vlxb

export module C 
extern {

fn printf(_Format: str, args: addr...) -> void;

}
```
LLVM will mark these functions externals (`# extern`) and search in C ABI (`# export module C`)

# naming convention (recommended)
Types : entity, component, role, system, enum, type, union, generic, named metacode

Naming rules:

| type | rule | e.g. |
|-|-|-|
| function, lambda | always verbal and on first word if possible | `find_player` `add_health` |
| variable, member | `<englober>_<attribute>_<capacity>_<parameter>` | `car_speed_max`<br> `player_health_max_color` |
| component | decrypt a capacity<br> verbal with `able` suffix | `CPrintable` |
| role | decrypt a list of capacity so a role<br> adjective on the main behaviour | `RMerchant` (`CViable` `CMovable` `CContainable` `CValuable` `CInteractable`) |
| system | decrypt a capacity application<br> verbal with `able` suffix and the application | `viable_add_health` |
| system with element | decrypt a compoent or role, so the action name | `w_movement` (CMovable), `w_merchant` (`RMerchant`)
| generic | decrypt a filter, so a capacity or a nomenclature<br> verbal with `able` suffix or nomenclature | `GMovable` `GIntegral` |

>Note: naturally, you can encounter some exceptions or unexistent word, pay attention to the clarity first

Formatation rule:

| type | format | e.g. |
|-|-|-|
| namespace | PascalCase | `CityEurope` |
| type alias | PascalCase | `Map3Array` |
| entity | PascalCase, prefix `T` | `TAnimal` |                              
| component | PascalCase, prefix `C`<br> suffix `able` | `CMovable` |
| component member | snake_case | `max_health` |
| role | PascalCase<br> prefix `R` | `REnnemy` |
| enumerator | PascalCase<br> prefix `E` | `ESpecies` |
| union | PascalCase<br> prefix `U` | `UView` |
| generic | PascalCase<br> prefix `G` suffix `able`<br> or a nomenclature word | `GPrintable` `GIntegral` |
| reusable metacode | PascalCase<br> prefix `M` | `MStaticConst` |
| function | snake_case | `math_foo` |
| system | snake_case<br> prefix major component used with `able` prefix and the main purpose of the system | `movable_jump`<br> (CMovable is the major component to jump any entity, and the main purpose of the system is to jump) |
| variable | snake_case | `health_max` |
| local variable | snake_case<br> prefix `_` | `_temp_vector` |
| parameter | snake_case<br> prefix `p_` | `p_first` |
| lambda | snake_case<br> prefix `_` | `_precalculate_array` |
| lambda local variable | snake_case<br> prefix `_` | `_temp_vector` |
| lambda parameter | snake_case<br> prefix `p_` | `p_temp_vector` |
| constant | snake_case<br> prefix `k_` | `k_player_max` |
| global | CAPITAL | `COMPILATION_ARGS` |
| generic typename | one letter uppercase | `T ` `U ` `V ` |
| metacode placeholder | one letter uppercase, prefix `_` | `_T` `_U` `_V` |

abbreviation and truncation:

use this for too long names and for most used local variable or temporary variables

| type | rule | e.g. |
|-|-|-|
| temporary | prefix `tmp_`<br> if copy of var, peek first letter on syllab<br> or first letters (for one word)<br> or first words letters, or standard convention (`lhs`, `rhs`, ...) | `temp_buff` `temp_lhs` |
| most used variable | peek first letter on syllab<br> or first letters (for one word)<br> or first words letters<br> or standard convention (`lhs`, `rhs`, ...) | `expansion_meta->placeholders_pos` : `phs_pos` |

# mangling 

- `[]` optional
- <name> : `<size><name>`

| type | mangling |
|------|----------|
| bool | `b` |
| i8 - i128 - isize | `i8` - `i128` `isz` |
| u8 - u128 - isize| `u8` - `u128` `usz` |
| b8 - b128 - bsize | `b8` `b128` `bsz` |
| f32 - f64 - fsize | `f32` `f64` `fsz` |
| ascii | `aii` |
| utf32 | `utf` |
| ptr  | `p`  |
| sptr | `sp` |
| uptr | `wp` |
| deci | `d_<integral_size>_<decimal_size>` ->   `102.56d`->    `d_3_2` |
| udeci | `ud_<integral_size>_<decimal_size>`  ->  `50.2555ud` ->  `ud_2_4` |
| static array | `arr<size>_<type>` |
| dynamic array | `list_<type>` |
| tuple |`tu<size>_<type>`
| string | `str` |
| range | `rng_<typern>` |
| iterator | `iter_<type>` |
| slice | `sli_<type>` |
| function | `fn<param_size>_<param>_<return>` |

entities:

| type | pattern | mangling |
|-|-|-|
| velox mangling symbol |                         `_V_` |                                               |
| <path> |                                `[<module>]<namespaces>` |
| <gentys> instance | `<gentys>`: `G<count>_<types>` |           `G2_str_i32` |
| <gentys> symbol | `<gentys>`: `S<count>_<types>` |           `S2_T_U` |
| namespace       | `[<module>][<namespace>]<name>[<namespace>]` |  `_v_6Forest5Trees` |
| local var         | `[<path__>]loc_<name>[<_gentys>]` | `_V_6Forest__loc_5Count` |
| global var       | `[<path__>]glo_<name>[<_gentys>]` | `_V_6Forest__glo_7MAX_POP` |
| function         | `[<path__>]fn_<name>[<_gentys>]` | `_V_6Forest__fn_10new_forest` |
| function type   |   `[<path__>]fn<param_size>_<param>_<return>` |  `_V_6Forest__fn2_i8_f32_tu2_f64_b` |
| typealias        | `[<path__>]ty_<name>` | `_V_6Forest__ty_6Health` |
| system          |       `[<path__>]sy_<name>` | `_V_6Forest__sy_4Move` |
| component       |       `[<path__>]cp_<name>` | `_V_6Forest__cp_8Position` |
| entity         | `[<path__>]et_<name>[<_gentys>]` | `_V_6Forest__et_6Animal` |
| entity component |     `<entity__>cp_<name>` | `_V_6Forest__et_6Animal_cp_5Speed ` |
| entity constructor |   `<entity__>nw_<types>` | `_V_6Forest__et_6Animal_nw_str_u16` |
| enum           | `[<path__>]en_<name>` | `_V_6Forest__en_4Race` |
| enum elem     |  `[<path__>]en_<name>` | `_V_6Forest__en_4Race_3Dog` |
| flag           | `[<path__>]fg_<name>` | `_V_6Forest__fg_7Weather` |
| role           | `[<path__>]rl_<name>` | `_V_6Forest__rl_8Moveable` |
| generic       | `[<path__>]gn_<name>` | `_V_6Forest__gn_9Life_From` |
| metacode       | `[<path__>]mc_<name>` | `_V_6Forest__mc_10ConstAsync` |


# other metacodes
## Script target
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
| operating system used | `# os ""` | |

## Function / Lambda target (`fn`, `lam`)
| name | syntax | info 
|-|-|-|
| asynchrone execution | `# async` | mark the function as asynchronous. |
| parallel execution | `# parallel` | mark the function for parallel execution. |
| pure function | `# pure` | the function has no side-effects. |
| timeout execution (async) | `# timeout seconds` | maximum execution time. |
| inline | `# inline` | suggest compiler inlining. |
| unit test | `# test` | the function is a unit test. |
| for performance | `# benchmark` | the function is for performance benchmarking. |

## Entity target
| name | syntax | info 
|-|-|-|
|  | `# align N` | enforce memory alignment (e.g. `# align(8)`). |
|  | `# serializable` | allow automatic serialization. |

## Enum target
| name | syntax | info 
|-|-|-|
|  | `# repr(type)` | define underlying representation (e.g. `i8`, `u32`). |

## Scope target
| name | syntax | info 
|-|-|-|
| scope | `# scope ... # end scope` | define a named scope where metacodes apply. |
| scope | `# scope name ... # end scope` | define a named scope where metacodes apply. |
| exclusion | `# exclude all` | exclude current block from inherited metacodes. |
| exclusion | `# exclude name` | exclude a named scope only. |

# script structure recommandation
```
// script definition section (COBOL inspiration)
# author ""
# title "" 
# version ""
# description ""
# localisation "" // file path
# platform "" // platform target designed

// export import section
import math
import timer

export Forest {
// constant definition section
let PI: f32 = 3.14159265359

// reusable metacode definition section
# meta threadSafeFn() -> fn | lam 
# pure
# async

// reusable generic definition section
gen sizeable<T> {
  T is integral;
  T is unsigned;
}

// enum definition section
enum opt<T> {
  Valid<T>,
  None,
}

// components definition
comp Position { x: f32 = 0, y: f32 = 0 }
comp Velocity { dx: f32 = 0, dy: f32 = 0 }
comp Health { life: f32 = 0 }

// entity definition
entity Animal {
  use Position
  use Velocity
  use Health { life: 100 }
}

// systems definition
sys Move {
  Position(pos) + Velocity(vel) => {
    pos.x += vel.dx
    pos.y += vel.dy
  }
}

// main function
fn main() {
  var animals = new_forest()
  var _timer: f32 = 0.0

  while timer < 60 {
    for item in animals {
      move() on item
    }

    _timer += timer::delta
  }
}


fn new_forest() -> List<Animal> {
  var animals: List<Animal>(10)

  for i in 0..10 {
      animals.add(Animal{Position= {math::rand(), math::rand()}})
    }
}

} // end export
```
