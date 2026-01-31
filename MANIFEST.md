<div align="center">

![Velox logo title](logo/velox-logo-tilted.svg) 

# Velox</br> TECHNICAL IMPLEMENTATION

</div>

A hybrid-level programming language designed to emphasize safety and simplicity through innovative syntax and paradigms.

This project has been under design and experimentation since 2024.  
The public repository reflects the current implementation.

Velox is currently in an experimental stage and should be considered a work in progress.

This MANIEST presents the vision and core ideas behind the language.  
Some features and syntax are still under development.

Velox is a next-generation programming language that aims to combine performance, simplicity, and extensibility. Inspired by modern systems programming practices and component-oriented architectures, Velox enables developers to write efficient, maintainable, and scalable code with minimal boilerplate.

> Explicit Inspirations: ![Rust](https://github.com/rust-lang/rust) - ![C](https://github.com/llvm/llvm-project) - ![C++](https://github.com/llvm/llvm-project) - ![Python](https://github.com/python) - ![COBOL](https://github.com/dscobol/Cobol-Projects) - ![FORTAN](https://github.com/fortran-lang) - ![Lisp](https://github.com/topics/common-lisp?l=scheme&o=desc&s=) - ![Zen-C](https://github.com/z-libs/Zen-C)</br>
The Zen-C programming language demonstrates a clear commitment to modernizing C, offering a safer language that aligns with modern programming practices. We are following the progress of this project with great interest, as it shares some of the goals we are pursuing in our own language.

### Copiler Status
![Compiler](https://img.shields.io/badge/Compiler-Work_in_progress-brightgreen)</br>
![Compilation_Pipeline](https://img.shields.io/badge/Compilation_Pipeline-Done-darkgreen)
![Lexer](https://img.shields.io/badge/Lexer-Done-darkgreen)
![Preprocessor](https://img.shields.io/badge/Preprocessor-Done-darkgreen)
![Parser](https://img.shields.io/badge/Parser-Work_in_progress-brightgreen)
![EMBinder](https://img.shields.io/badge/EMBinder-Done-darkgreen)
![Exporter](https://img.shields.io/badge/Exporter-Done-darkgreen)
![ResolverSymbol](https://img.shields.io/badge/Resolver_Symbol-Work_in_progress-brightgreen)
![ResolverType](https://img.shields.io/badge/Resolver_Type-Work_in_progress-brightgreen)
![ResolverSemantic](https://img.shields.io/badge/Resolver_Semantic-Work_in_progress-brightgreen)
![LLVMGeneration](https://img.shields.io/badge/LLVM_Generation-Work_in_progress-brightgreen)</br>
### Tools Status
![Highlighter](https://img.shields.io/badge/Highlighter-Done-darkgreen)
![Snippet](https://img.shields.io/badge/Snippet-Done-darkgreen)
![Compilation_Dot](https://img.shields.io/badge/Compilation_debug_dot_graph-Done-darkgreen)

# Type
## Primitives
|      type      |      syntax      |            literal           |      size      |
|-|-|-|-|
| boolean        | `bool`           | `true` `false`               | 1 bit (but 8 bit aligned)          |
| binary         | `bsize` `b8-b128` | `0b10010010` `0x0F` `0bsize` | 8-128 bits |
| integral       | `isize` `i8-i128` | `0 ` `-1` `10isize`         | 8-128 bits |
| unsigned       | `usize` `u8-u128` | `0 ` `10usize`              | 8-128 bits |
| floating       | `fsize` `f32-f128` | `0.0f` `-1.0f` `10fsize`   | 32-128 bits |
| decimal        | `deci`           | `0.0` `-1.0` `10d` default | numbers*8  bit |
| udecimal       | `udeci`           | `10ud`                       | numbers*8  bit |
| decimal constructor    | `<size>d<size>`  | `3d2` -> `000.00`            | numbers*8  bit |
| no type        | `u0`, `void`   |  | 
| ascii          | `ascii`          | `"a"ascii`                   | 8 bits (latin1) |
| utf32      | `utf32`           | `"⚜"utf32` `"⚜"` default         | 32 bits  |                   
| string         | `str`            | `"hello"s`                   | ascii*len + 2*bsize (fat pointer) bit (latin1) |
| text | `text`           | `"hello"t` `"world"` default | utf32*len + 2*bsize (fat pointer) bit (utf32) |
| opaque ptr     | `ptr'void`           | `...`                      | bsize bit      |
| unique ptr     | `uptr'T`           | `uptr'i32`                      | bsize bit      |
| shared ptr     | `sptr'void`           | `sptr'i32`                      | bsize bit      |
| function prototype | `fn() -> ()`           | `fn(i32, i32) -> (i32)` |       |
| static table   | `[T; N]`       | `{ 1, 2, 3, 4}`,<br> `{ 0..4 = 8 }` (4 elements equals to 8) | N*size + bisize (pointer) |
| static matrix   | `[T; N, N, ...]`,<br> `[T; N]*D` | `{{0,0,0},{0,0,0},{0,0,0}}` `{ 1, 2, 3, 4}*3` (make 3d matrix of 4 elements for each dimension) | N*size + bisize (pointer) |
| dynamic  table  | `[T]`            | same of static table, but literal is instanciation only | List entity |
| dynamic matrix  | `[T]*D` | same of static matrix, but literal is instanciation only  | Matrix entity    |

### Implicit Cast
no memory loss allowed 

|      type      |                   cast to                 |
|----------------|-------------------------------------------|
| numeric        | `b/u/i 8` -> `b/u/i 16` -> `b/u/i 32` -> `b/u/i 64` -> `b/u/i 128` (`b/u/i size` are api dependend) |
| numeric        | numeric -> floating                      |
| floating       | `f32` -> `f64` -> `f128` (`fisize` is api dependend) |
| decimal        | inferior decimal -> superior decimal      |
| ascii          | `ascii` -> `utf32` latin1 -> utf32 |
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

# Operators
## Memory
> See Memory section

| memory | syntax | info |
|-|-|-|
| move           | `move=`, complex type default: `=` | assignation by move semantic
| copy           | `copy=`, primitive default: `=` | assignation by copy method, otherwise clone method used
| clone          | `clone=`  | assignation by clone method, otherwise copy method used
| drop           | `drop my_var` | revoke `ref`/`mut`
| new            | `new ptr'T()` | memory allocation on heap
| delete         | `del my_ptr` | free pointer
| val of ptr     | `val'my_ptr` | get pointer value (return always optional -> None if ptr is None)
| address of     | `addr'my_val` | get the memory address of value -> returns raw ptr'T of val type| 

## Arithmetic
> See COP section

| arithmetic | syntax | info |
|-|-|-|
| add            | `+`          | |
| subtract       | `-`          | |
| multiply       | `*`          | |
| matrix multiplication | `@` | only on matrices |
| divide         | `/`          | result always f32/f64   |
| remain         | `%rem%`      | remainder singed with dividend |
| modulo         | `%mod%`      | euclidian remainder (modulo not negative if divisor>0) |
| quotient       | `%quo%`      | euclidian quotient      |
| power          | `**`         |                         |
| increment      | `++`         |                         |
| decrement      | `--`         |                         |

## Comparison
> See COP section

| comparison | syntax | info |
|-|-|-|
| greater        | `>`          |                         |
| lower          | `<`          |                         |
| greater equal  | `>=`         |                         |
| lower equal    | `<=`         |                         |
| equal          | `==`         |                         |
| equal strictly | `===`        | for string and float    |
| not equal      | `!=`         |                         |
| not eq strictly| `!==`        | for string and float    |

## Logical
> See COP section

| logical | bitwise | syntax | syntax bitwise |
|-|-|-|-|
| and   | and.b   |`and`  | `and.b`  |
| nand  | nand.b  |`nand` | `nand.b` |
| or    | or.b    |`or`   | `or.b`   |
| xor   | xor.b   |`xor`  | `xor.b`  |
| nor   | nor.b   |`nor`  | `nor.b`  |
| xnor  | xnor.b  |`xnor` | `xnor.b` |

## Binary
> Only on `flag` and `b8`-`b128`

| binary | syntax | info |
|-|-|-|
| shift left 0   | `<<[0]`      | fill right with 0       |
| shift left 1   | `<<[1]`      | fill right with 1       |
| shift right 0  | `[0]>>`      | fill left with 0        |
| shift right 1  | `[1]>>`      | fill left with 1        |
| shift left a   | `<<[a]`      | fill right with MSB (arithmetic) |
| shift right a  | `[a]>>`      | fill left with MSB (arithmetic) |
| rotate left    | `<<[r]`      | rotate bits to the left |
| rotate right   | `[r]>>`      | rotate bits to the right |
| slice bits     | `~[0..8]`    | get bits from range     |


# Type Alias
designed to reuse parametred type
```
type <name> = <expression>
```

# Table Population
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

# Cast
| type | syntax | note
|-|-|-|
| static cast | `<value> as <type>` | always successful |
| safe cast | `<value> as? <type>` | optional return `T?` |
| reinterpret cast | `<value> as! <type>` | getelementptr on speficied type 

# Format String
formatted text literal:
```
"text{variable} and other {variable}"
```
same as: "text" + variable + "and other" + variable

## Format Specifier
you can use some parameter to specify the out formatation of variable formatted
```
[[<fill>]<align>][<sign>]["#"]["0"][<width>][<grouping_option>]["."<precision>][<type>]

```

use in format string like:
```
"duration {<expression>:<format_specifier>} s"
```

## Format Conditional (experimental)
you can specify some reaction with a comparison from the expression returned value:
```
(<value>:"<return_text>", ...)
```
special comparison operator:
- other case `_:"<return_text>"`
- positive `+:"<return_text>"`
- negative `-:"<return_text>"`

In format string:
```
"Gender: {lerp}:<format_conditional>"
```

e.g.
```
var messages = 3
"You have {messages}:(0:"no messages",1:"one message",_:{messages} "messages")"
var gender = "F"
"{gender}:("F":"She is online", "M":"He is online", _:"online")"
var account_balance = -2.0f
"{balance}:(+:"no depts", -:"some depts")"
```

# Variable
Variables are statically typed or type inferred

A variable always must be declared with a value before any read

| type | syntax |
|-|-|
| mutable typed | `var name: T` |
| mutable typed setted | `var name: T = ...` |
| mutable inferred setted | `var name = ...` |
| immutable typed | `let name: T = ...` |
| immutable inferred | `let name = ...` |
| compiletime typed | `const name: T = ...` |
| compiletime inferred | `const name = ...` |

## Assignation
variables are managed by a explicit move mode and capacity assignation
the default affectation `=` is a move semantic except for primitives who are a copy (performance reason)
> See Capability
| assignation mode | syntax | note |
|-|-|-|
| Copy | `lhs copy= rhs`    | For complex types: check for a copy method. For primitives: copy |
| Clone | `lhs clone= rhs`     | For complex types: check for a clone method. For primitives: copy |
| Move | `lhs move= rhs` or `lhs = rhs`   | Revokes all previous capabilities (ref + mut) see explicit Capabilities |

## Operation Assignation
variables can be mofied directly by a arithmetic operation (read/write) operation
`+=` `-=` `*=` `/=` `%mod%=` `%quo%=` `%rem%=` `**=` 


# Memory Managment
The memory use a fine managment, there is no GC

There is two types of management of memory:
- Pointers -> memory on heap
- Capabilities -> memory on stack

# Memory Managment: Pointers
| name | syntax | info |
|-|-|-|
| non typed memory address | `ptr'void` | useful for C interop (`void*`) |
| raw pointer | `ptr'T` | if no escape in the scope, will delete |
| unique pointer | `uptr'T` | only `move=` to change his position and invalidate his last position |
| shared pointer | `sptr'T` | only `=` to add his reference to the new position and increment his counter |

> Note: the capability checker will ignore all operations on pointers excepts the `move=` operation to invalidate the origin

## Pointer Creation (on heap)
To use a memory space by pointer creation use `new` key, uses the C malloc

Allocate new memory space like:
```
new <ptr>'<type>(<value>)
```

e.g.
```
var halicarnassus: ptr'City::Monument = new ptr'City::Monument()
```

## Pointer on variables
Not recommended but permitted to avoid capabilities:
```
var a: i32 = 100
var my_ptr: ptr'i32 = addr'a
var b: i32 = val'my_ptr       // get pointer value
```

> Pointer on `ref` and `mut` are prohibied !


## Pointer usage
Pointers are handled by the language, despite of the capability system.
It's designed for low-level operations
You can use them for some operations: (non-exhaustive list)
| usage | syntax | info |
|-|-|-|
| arbitrary memory location | `let my_console_input: $ptr'i32! = 0xFFFFFF as! $ptr'i32!` | classic const pointer of volatile value from a input for embbed systems |
| change memory address | `fn swap_ptr(addr a: i32, addr b: i32) {...} swap(my_ptr1, my_ptr2)` | any addr parameter is considered aliased by security |
| complex function return | `fn get_hardware_output(copy hardware_ty: EHardwareOutType) -> ptr'i32` |  |

# Memory Managment: Capabilities
Access to a variable wihout any copy/clone/pointers, use the **explicit capabilities** for better code safety.
| capability | operation | declaration syntax |
|-|-|-|
| `ref` | shared reading | `ref a = x` |
| `mut` | exclusive r/w | `mut a = x` |
| `ref` returned | from function return | `fn return_ref(ref a: T) -> ref'T(a)</br>ref b = return_ref(a)` 
| `mut` returned | from function return | `fn return_mut(mut a: T) -> mut'T(a)</br>mut b = return_mut(a)` 

capabilities is based on xor reference/mutable:
- multiple ref are allowed
- exclusive mut is allowed

## Origin Usage: Revocation of Previous Capabilities
| Operation on origin | e.g. | consequence |
|-|-|-|
| Read only / copy | `println(player.CId.name)` | **revokes any previous mut** |
| Write | `x += 10` | **revokes any previous ref/mut** |
| Move | `b move= a` | **revokes any previous ref/mut** (+ origin moved) |
| New ref | `ref r = a` | **revokes any previous mut** |
| New mut | `mut m = a` | **revokes all previous ref** |
| Mut indexations | `mut index = a[i]` | cannot overlap |
| Mut slices | `mut col_slice = a[0..10]` | cannot overlap |
| Mut/ref index on slice | ref index = mut_slice[4] | only in deterministic slice |

> note: all revoked capabilities **cannot be reused**

## Revocation of Capabilities (other cases)
| Case | syntax | consequence |
|-|-|-|
| **Explicit drop** | `drop a_ref` | the capability is removed |
| **End of scope** | `var a = 10 {<br>mut m = a</br>m += 10 }` | automatic revocation |
| **Revocable parameter** | `fn revoke_mut(mut a: i32) -> mut'i32(a)</br>mut m = a</br>mut out_mut = revoke_mut(a) }` | revocation from function lifetime signature |
| **Parent data handling** | `mut p_name = player.name</br>player = Player::new("marc", 25)` |  r/w a parent revokes children's capabilities. But fields are considered separate.
| **Parent Collection operations** | `mut slice_mut = a[0..10]</br>a = {10, 20, 30}` | slice/index capabilites follow the parent's (collection base) operations.
| **Move instruction** | `ref a_ref = a<br>b move= a` or move parameter -> all capabilities are revoked and the origin is removed.

> note: for parameter passage, see parameters section

### Mut capability returned revoke all potential mut parameters 
Remined: capabilities are determined at compilation time -> static resolution.

Axiom: if a `mut`/`ref` is returned, the function have at least the same number of `mut`/`ref` as parameters. 
Because capabilities are never null (except packed in enum) but they can be revoked.

Problematic: which parameter is revoked during the call ?

Rule: when a `mut`/`ref` parameter is returned, the argument passed is always revoked.

Signature mandatory: it's mandatory to specify revocables parameters for signature only static checking

The compiler will helps to specify the lifetime matching

e.g.
```
fn one_return_explicit(mut a: i32, mut b: i32) -> mut'i32(a) {
  a += b
  return a
}
```
Consequence:
- `a` argument is revoked on call, `b` argument stay valid
```
fn one_return<>(mut a: i32, mut b: i32) -> mut'i32(a, b)? {
  if cond1 => return a
  elif cond2 => return b
  else => return None
}
```
Consequence:
- `a` and `b` arguments are revoked on call
```
fn two_return(mut a: i32, mut b: i32, mut c: i32) -> (mut'i32(a, b), mut'i32(a, b)?) {
  if cond1 => return (a, b)
  elif cond2 => return (b, None)
  else => return (a, b)
}
```
Consequence:
- `a` and `b` arguments are revoked on call
- `c` is not revoked

> Note: arguments revocation is reserved on `mut` and `ref` capabilities, simples variables are not concerned

#### Single Responsibility Principle (SRP)
As the `mut` arguments can be consumed by a `mut` return, the language requires a single resposibility principle design in his functions 

# Function
> Use `fn` keyword to declare a function
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
| unpack result | `var (a, _, c) = name()` | `_` is for ignore field, see patterns section 

## Local Variables
No name shadowing permitted
```
<kind> <name> [: <type>] = <value>
```

A variable can have unique and persistent memory address between calls with `# static`

e.g.
```
fn add_counter() -> i32 {
  # static
  var counter: i32 = 0 // will init at 0 but not reset to 0 between calls
  counter += 1
  return counter
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
| mutable pip-call | `let result: f32 = sum <-\| 10 \|+ 2.0 \|+ avg(a, b, c) \|+ k \|+ "100" as f32 \|+ 10.5`
| pure pip-call | `let position3D: (f32, f32, f32) = offset \| x \| y \| z`
| mutable pip-call generic args | `let result: f32 = sum <-\| <i32> 10 \|+ <f32> 2.0 \|+ <i32> avg(a, b, c) \|+ <i32> k \|+ <f32> "100" as f32 \|+ <f32> 10.5`
| pure pip-call generic args | `let position3D: (f32, f32, f32) = offset<f32> \| x \| y \| z`

# Lambda
> use `lam` keyword to declare a lambda, threated like c++ : anonym functions
```
lam [<name>]['['<capture>']'][(<params>)] [-> <return_type>] { ... }
```

| type | syntax |
|-|-|
| named lambda | `lam l_name[capture](parameters) -> return {...}` |
| anonym lambda for predicate purpose | `lam [capture](parameters) -> return {...}` |
| simple lambda | `lam {...}` |

> same calling as function

## Lambda Capture
to capture variables in scopes

| target | syntax | extension with exceptions |
|-|-|-|
| to modify all variables | `[mut]` | `[mut, copy a, copy b]` |
| to copy all variables | `[copy]` | `[copy, mut a, mut b]` |
| to get instance | `[self]` | `[..., self,...]` |

# Parameters
parameters are managed by a pass mode and a type base

## Pass Modes
There is 6 pass modes:
| mode | syntax | info |
|-|-|-|
| `ref` | `ref name: T [= default_val]` | by reference (immutable)
| `mut` | `mut name: T` | by mutation (mutable)
| `copy` | `copy name: T [= default_val]` | by copy forced
| `clone` | `clone name: T [= default_val]` | by clone forced
| `move` | `move name: T` | by move semantic
| `addr` | `addr name: T` | only pointer address manipulation
| `...` | `<pass_mode> args: T...` | Variadic, non type (C convention) : `ptr'u0`

>Note: variadic is technically not a pass mode, it's permit multiple parameters with a pass mode and type 
>any pass mode can be optional with the type modifier `?`, not necessary for parameters with a default value

### by Type
| mode | primitive | complex type |
|-|-|-|
| `ref` | `copy` | add `ref`
| `mut` | local `mut` capbility, no calling scope `mut`/`ref` revoked | idem
| `copy` | `copy` | call `copy`, fallback `clone`*
| `clone` | `clone` | call `clone`, fallback `copy`**
| `move` | `copy` | `move` and invalidate origin 
| `addr` | only b8-b128 | only `ptr'T`

> * Copy and Clone arguments can be overrided during the call by `copy` or `clone` `fn copy_myvar(copy a: MyVar)` `copy_myvar(clone my_var)`

### by Capability
| mode | regular variable | ref | mut
|-|-|-|-|
| `ref` | local ref, no add ref | add ref to origin | X 
| `mut` | local mut, no add mut | X | redirect mut 
| `copy` | read operation, call copy | idem | idem 
| `clone` | read operation, call clone | idem | idem 
| `move` | consumed | ref consumed | mut consumed 
| `addr` | only raw pointer `ptr'T` | X | X

### by Pointer
| mode | ptr | uptr | sptr |
|-|-|-|-|
| `ref` | read only ptr and pointee | X | read only sptr and pointee |
| `mut` | read only ptr, mutable pointee | X | read only sptr, mutable pointee | 
| `copy` | copy ptr address | X | share uptr (sharing > copy) |
| `clone` | new ptr address affected, call clone on pointee put in new ptr address | X | share sptr (sharing > copy) |
| `move` | ptr consumed | uptr consumed | uptr consumed | 
| `addr` | mutable ptr, mutable pointee | X | X

## Parameter Arrangement Rules

call parameter ordering left to right: positional -> named -> variadic args
> Note: optional arguments are from parameter with optional type modifier `?` of with a default value

| case | case info | call | call info |
|-|-|-|-|
| `fn(mut a: i32, mut b: i32)` | Mandatory arguments (no default value) must be specified by their position or name | `(value_a, b= value_b)` | value_a on a, value_b on b |
| `fn(copy a: i32 = 0, mut b: i32)` | If a optional argument is before a mandatory argument, you mut discriminate arguments proprely | `(None, valueb)` or `(b= valueb)` | Optional arguments can be used without None | 
| `fn(mut a: i32, mut args: i32...)` | The variadic argument case is always optional despite of his pass mode | `(value_a ...value_arg1, value_arg2)` or `(value_a)` | Variadic args list must begins with `...` token |

## Return
multiple returns is handled with the tuple logic 
```fn() -> (a, b, c, ...)```

return element kind:
| kind | syntax | out mode | info |
|-|-|-|-|
| Primitive type | `fn() -> i32` | `copy` | Local variables can't survive, a copy is the convention |
| Complex type | `fn() -> MyEntity` | `move` | A move semantic always make because local variables can't survive |
| mut | `fn(mut a: i32, mut b: i32) -> mut'i32` | `move` | but explicit lifetime is required here (mut is exclusive) see Return mut matching
| ref | `fn(ref a: i32, ref b: i32) -> ref'i32` | add `ref` | no explicit lifetime required because ref are cumulatives
| ptr | `fn() -> ptr'i32` | `move` | pointer is escaped
| ptr | `fn(addr my_ptr: i32) -> ptr'i32` | `move` | ambiguous return: can be a new ptr or a transfert from `my_ptr`


# Generics
> Use `gen` keyword to declare a generic type

technically, generic restriction return true (valid type) or false (invalid type) you can reuse generic with named generic
```
gen <name>'<'<typename> [, <typename2>, ...]'>' { .. }
```

Generics are usable on:
- functions
- parameters
- entity
- systems
- components
- roles (for generic components)
- type-alias


## Generics Conditions
```
<typename> <kind> <symbol>
```
Conditions are cumulatives and not alternatives

| filter kind | kind | syntax |
|-|-|-|
| operator filter | `op` | `T op *` |
| component filter | `comp` | `T comp Position` |
| role filter | `role` | `T role Merchant` |
| system filter | `sys` | `T sys Move` |
| cast filter | `cast to`<br> `cast from` | `T cast to i32` `T cast from i32` `T cast to U` (commutative : valid if at least one cast is compatible) |
| generic filter | `is` | `T is gen::base_of<CAnimal> \| ...` or `T is Integral \| Signed \| i128 \| ...` (first arg is left of `is`) can have alternative |

Named generic example: 
```
gen GMoveable<T> { 
  T comp CPosition,
  T use op +,
  T use op -,
} 

gen GVelocity_Applied<T> {
  T is GMoveable
  T comp CVelocity
}
```

## Generic Usage
generic usage on functions/component/entity/lambda 

function:
```
fn add<T: is i32 + is i64, U: op + + is integral>(a: T, b: U) {
  return a + b;
}
```
> Note: `U: op + + integral` the first `+` is the operator, the second `+` is the cumulative generic filter with `is integral`
> It's a very bad syntax: use generic declaration instead to use like fn<T: Numeric, U: Numeric>(copy a: T, copy b: U)

generic without condition is possible (not recommended)
```
comp items<T> { size: usize, value: T }
```

generic type usage is possible (not recommended)
```
fn add(a: GNumeric, b: GNumeric) {
  return a + b;
}
```
# Literal asm Instruction
> To make inline assembly code directly added in the final generation

Useful for low-level coding like asm { } in C
```
asm {...}
```
Rules:
- Any in variable must be marked `# asm in`
- Any out/inout variable result must be marked `# asm out`
- The function who contains the `asm` instruction must be marked `# unsafe`

e.g.
```
# unsafe
fn add(copy a: i32, copy b: i32) -> i32 {
  # asm in
  let _a: i32 = a
  # asm in
  let _b: i32 = b
  # asm out
  let result: i32
  
  asm {
    mov eax, _a
    mov ebx, _b
    add eax, ebx
    mov _result, eax
  }
  return result
}
```

>Note: Considered unsafe by nature. Syntax error will creates a compilation error.


# Literal LLVM Instruction
> To make inline LLVM code directly added in the code generation

Useful for low-level coding like asm { } in C
```
llvm {...}
```
Rules:
- Any in variable must be marked `# llvm in`
- Any out/inout variable result must be marked `# llvm out`
- The function who contains the `llvm` instruction must be marked `# unsafe`

e.g.
```
# unsafe
fn add(copy a: i32, copy b: i32) -> i32 {
  # llvm in
  let _a: i32 = a
  # llvm in
  let _b: i32 = b
  # llvm out
  let result: i32
  
  llvm {
    %va = load i32, ptr %_a
    %vb = load i32, ptr %_b
    %sum = add i32 %va, %vb
    store i32 %sum, ptr %result
  }
  return result
}
```

>Note: Considered unsafe by nature. Syntax error handled by llvm back-end directly 

# Flag
> Use `flag` keyword to declare a flag

Flag is a optimized named bits, max 255 elements, use only 8 bits
```
flag FFileMode {
  Read, Write, Read_Write, Lock
}
```
Explicit byte
```
flat FFileMode {
  Read => 0x0, Write => 0x1, Read_Write => 0x2, Lock => 0x3
}
```

> bit enum type can be explicit cast with bytes, integrals : `0x1 as FName` or `1 as FName` => return FName::elem2

# Union
> Use `union` keyword to declare a union

Union is similar to an C union, so a non discriminant container who will consider the data as a type from the index in the input and output in the discretion of the user, no indexation or flag used to determine wich type is active.
```
union UNumber {
  i: isize,
  f: fsize,
}
```
> Note: Useful for C interop

# Enumerator
> Use `enum` keyword to declare a enumerator

rust like enum (typed):
```
enum EInteractIssue {
  ItemPickUp(Item, isize),
  Damage(isize),
  EarnCoins(isize)
  None,
}
```
>Note: for untyped enum prefer the `flag` to keep performances

Enum usage
| type | syntax |
|-|-|
| access to a C like element | `EName::elem1` |
| access to a typed element | `EName::elem1` or `EName::elem1(val)` |


## Match Enumerator
enums are linked to the match mecanism
```
enum Optional<T> {
  Some(T),
  None,
}

let result = Optional(10.0) // type Optional<f32>

match result {
  Some(i) => println("{i}")
  None => println("Failure")
}
```

# Modules
modules permit to avoid naming collision, it's possible to use native types as modules
```
mod name { ... }
```

call:
```
City::House::new(N: 37.0379f, E: 27.4241f)
```

to export module, use the instruction `export <name> { ... }` and use as a module

## Module Alias
you can give an alias to an module (hightly not recommended)
`use fs = core::file_system`


# Tuple
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
| function return | `fn name() -> (T, U, ...)` | tuple return |
| function return | `fn name() -> (name1: T, name2: U, ...)` | named tuple return | 
| unpack tuple from call | `var (a, b, c) = name()` | |
| unpack tuple from variable | `var (a, _, c) = var_tuple` | |

## Tuple Cast
Tuples and named tuples are the same for the compilator but need to de distinguished for users to makes proprer conversion.

Conversions:

| cast type | base | syntax | result | info |
|-|-|-|-|-|
| positional | `let t1 = (10, 2.0, "hello")` | `t2 = t1 as (2, 1, 0)` | `t2 = ("hello", 2.0, 10)` | in each position, set the new position by the index |
| nomenclature | `let t1 = (a= 10, b= 2.0, c= "hello")` | `t2 = t1 as (c, b, a)` | `t2 = ("hello", 2.0, 10)` | in each position, set the new position by the field name | 

# Flow

## Conditions: if elif else
control flow by condition

| flow | syntax codeblock | syntax inline |
|-|-|-|
| if | `if <condition> { ... }` | `if <condition> => ...` |
| elif | `elif <condition> { ... }` | `elif <condition> => ...` |
| else | `else { ... }` | `else ...` |

### Ternary if
Ternary if is a control flow for value fields
```
if <condition> => <true statement>
if <condition> => <true_statement> else <false_statement>
```

## Match
Match statement control flow for code logic by matching comparison on value. 

Execution stop to the case executed. Or use metacode `# fallthrough`
```
match <value> {
  // case states ...
}
```

### Case Statement
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
| compare in range | `10..=30 =>` |
| match typed enum and bind value | `Some(a) =>` |
| match typed enum, bind value and compare | `Some(a) if a > 10 =>` |
| match on enum | `EEnum::Elem =>` |
| other case | `_ =>` | 

## Loop
Highlty not recommended

| statement type | syntax |
|-|-|
| codeblock | `loop { ... }` |
| inline | `loop => ...` |

## While
| statement type | syntax |
|-|-|
| codeblock | `while <condition> { ... }` |
| inline | `while <condition> => ...` |

## Do-while Statement
| statement type | syntax |
|-|-|
| codeblock | `do { ... } while <condition>;` |
| inline | `do => ... while <condition>;` |

## For Loop Statement
For loop can be used on range, slice, collection and map

Syntax:

| statement type | syntax | info |
|-|-|-|
| for index | `for <index> in <range> [step <constant>] { ... }` | can use `step` after `<range>` |
| for item | `for [<index>] mut/ref/copy/move <item> in <slice/collection> { ... }` |  |
| for item | `for [<index>] mut/ref/copy/move (<item1>, <item2>, ...) in <slice/collection> { ... }` | useful for map or tuple array  |

>Inline case: `for ... => <expression>`

# Goto Statement
Highlty not recommended, designed for flexibility and code specific behaviour.

| state | syntax |
|-|-|
| go to and label | `goto name` |
| label definition | `label name =>` |

# Range Statement
Borned with a start integral, end integral and optional step (only for `for` loop).

range can be use to extract slice from collection with special ranges.

| range type | syntax | info |
|-|-|-|
| range end exclude | `0..10` | will go from 0 to 9 |
| range end include | `0..=10` | will go from 0 to 10 |
| range all | `collection[..]` | will return the collection, not very useful there |
| range from 0 | `..11` | will go from 0 or the start of the collection to the 10th index (so 11 elements slice) |
| range to max | `5..` | will go from 5 to the end of the collection or the max value `i64` |

# Slice
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

# Patterns
the pattern matching can be used from if/elif/while and match statement
```
[if/elif/while] [let/var] <pattern> = <expression> [if <condition>] {...}
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
| entity pattern | `Player{CId.name: name, CId.id 10} = <expression>` |
| entity pattern | `Player{CId{name: name, id 10}} = <expression>` |
| component pattern | `CId{name: name, id: 10} = <expression>` |

## Pattern Binding Mode
| Binding Mode | syntax | info |
|-|-|-|
| Bind | `let (a)` | Copy all primitives, Move all complex types |
| Bind | `var (a)` | Mut all primitives, Mut all complex types |
| Override bind by copy | `(copy a)` | Will read the value and put a copy in binding |
| Override bind by clone | `(clone a)` | Will read the value and put a clone in binding |
| Override bind by mut | `(mut a)` | Will add a mut capability in binding |
| Override bind by ref | `(ref a)` | Will add a ref capability in binding |
| Override bind by move | `(move a)` | Move the value in binding and invalid the origin |

> Note: Override a bind by * will ignore let/var variables declaration

# Module Import / Export
The import and exportation of the code use the LLVM declare/extern
- Exports are also modules 
- To export the module in the root module, use `# native \n export <name> {...}`)

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
entity, role, component, system, generic, function, global, enum, metacode, type


# COP Paradigm (Compositional Oriented Programming)
COP, short for Compositional Oriented Programming, is a programming paradigm where entities are built through the static composition of components, without inheritance, without polymorphism, and without dynamic components
it's itended to be flexible with a deterministic syntax

An entity is defined by composition; components exist only within the entity, and systems operate by recognizing that composition.
- component is a contigous list of variables
- system is a behaviour who contains block of statement according to component combinaison

## Component
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

### Component Field
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

## Role
> Use `role` to declare a role
 
roles are a package of components to check if entities have some components
useful for simple generic functions or system case !

a role cannot be used in an entity composition, it's a generic/system_case/parameter guarantee shortcut

declaration:
```
role name { components, ... }
```

## Entity
> Use `entity` to declare a entity

entites have a static composition of components who define his behaviour for systems and the accepted parameter arugment of component/role in fuctions.
```
entity name { 
  use component ...
}
```

entity can contains:

| entity members | syntax | info | method | return |
|-|-|-|
| component | `use name { field: value }` | with default value | | |
| component | `use name` | default value from component | | |
| cast | `cast self as T { ... }` | cast entity to antoher type, reserved key `self` and `other` used, permit to use `my_var as T` | `const` | `T` |
| cast | `cast T as self { ... }` | cast entity from another type, reserved key `self` and `other` used, permit to use `my_val as Type(my_entity)` | `const` | `self` |
| constructor | `new(params) { ... }` | overloading possible, must returns the same entity type | `const` | `self` | 
| copier* | `copy { ... }` | must returns the same entity type | `const` | `self` | 
| cloner** | `clone { ... }` | must returns the same entity type | `const` | `self` | 
| deleter*** | `del { ... }` | no parameter, reserved key `self` used | | |
| arithmetic operator | `op + { ... }` | all operators handled but type are restrictives | `mutable` only if with a operation assignation `+=` | if operation assignation: in-place modification, otherwise copy |
| comparison operator | `op == { ... }` | all operators handled `self` `other` are same type | `const` | `bool` |
| logical operator | `op and { ... }` | all operators handled `self` `other` are same type | `const` | `bool` |

>* Without copier, the compiler will copy each components fileds, if ref/ptr/sptr fields -> call copy on type, copy ptr address, share pointer
>** Without cloner, the compiler will clone each components fields, if ref/ptr/sptr fields -> call clone on type, new ptr address then call clone on type, share pointer
>*** Without deleter, the compiler will delete each components fields, if ref/ptr/sptr fields -> call del on type, free ptr, decrement share pointer


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

### Member Usage

| meber type | usage syntax | note |
|-|-|-|
| call native constructor | `var cat = Cat{ CAnimal.name = "Ted", CAnimal.age = 2 }` |  not recommended |
| call custom constructor | `var cat = Cat::new("Ted", 2)` | clean constructor |

### Entity Operator Overloading
there is some restrictions in operator definition:

| operator | other term | return type | syntax |
|-|-|-|-|
| `==` `!=` `===` `!==` `<` `>` `<=` `>=` `is` | self entity type | `bool` | `op <operator> { ... }` |
| `+` `-` `*` `/` `%mod%` `%quo%` `%rem%` `**` `<<[0]` `<<[1]` `<<[a]` `<<[r]` `<<[rc]` `[0]>>` `[1]>>` `[a]>>` `[r]>>` `[rc]>>` | self entity type | self entity type copy | `op <operator> { ... }` |
| `+=` `-=` `*=` `/=` `%mod%=` `%quo%=` `%rem%=` `**=` | custom type T | self entity type ref | `op <operator> <T> { ... }` |
| `Iter` iterator | none | `Iter<T>` | `op Iter<T> { ... }` |
| `[]` index | none | index always i64, custom type U | `op [i] -> U { ... }` |
| `[..]` range | none | custome slice U | `op [r: ..] -> mut'[U]` |


# System

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

# COP in Function Parameters
The COP paradigm can be used in functions to simplify the code and avoid the generic boilerplate

Keep in mind that any component and role type in parameter is values garantee.

## Component Parameters
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
calling
```
var player = Player{CPosition.x= 100, CPosition.y= 100, CPosition.z= 100}
move_entity(player, player, (10.0, 20.0, 30.0), 5.0)
```
note a role or a system can simplify the function declaration and call:

## Role Parameters
```
role RMovable { CPosition, CPhysic }
fn move_entity_role(mut mov: RMovable, copy new_pos: xyz_pos, copy vel: f32) {
  mov.CPosition.x copy= new_pos.x
  mov.CPosition.y copy= new_pos.y
  mov.CPosition.z copy= new_pos.z
  mov.CPhysic.vel copy= vel
}
```
calling
```
move_entity_role(player, (10.0, 20.0, 30.0), 5.0)
```

## Systems Simplification
```
sys move(copy new_pos: xyz_pos, copy vel: f32) {
  CPosition(pos) + CPhysic(phy) {
    move_entity(pos, phy, new_pos, vel) 
  }
}
```
calling
```
player::>move((10.0, 20.0, 30.0), 5.0)
```

# COP and Function Generics and Function Calls
> Usage is the same as rust generic call type args
> It's possible tu specify a generic in the args

generic function call (turbofish used in calling)
```
<name>::'<'type[, type, ...]'>'([<parameters>])
```
e.g.
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

# Multi-threading
Multi-threading is handled natively.

The threading logic is simple:
- Pure `async` function
- Thread invocation via an `async` or `await` prefix then function call
- The function result is automatically packed as Future<T>
- Access intermediate values during async execution via messages
- Access the final result after async via `await`

## Async Function
A `async` function is **always pure**, without side effects.
- All externals values must be passed as parameters.
- Only global constants can be used inside the function.
- You can specify a **message stack** to handle messages during thread execution
```
// thread compatible function
// + message handled function
# pure // a compilation check argument
fn calculate(copy a: i32, copy b: i32) -> i32 // return i32 packed as Future<i32> when called with async or await
->tmsg<str> // enable messages binding, typed as string
{
  let c = a / b - a * b
  send "{c}" // put text to out 1st received
  let d = c * b
  send "{d}" // put text to out 2nd received
  return d // returns the d value
}
```

## Thread Invoking
A thread is invoked using the `async` or `await` prefixes before a function call

async execution
```
let result = async my_fn()   // declaration required to use await later
// async execution
await result                 // mandatory after async
println(result)              // retrieve the final value
```

sync execution
```
let result = await my_fn() // or simply `my_fn()`, declaration optional
// synchronous execution by nature
println(result) // final value guaranteed
```
> Note: `await` does **not** make the thread synchronous. The thread remains async internally; onlu the caller is blocked untile the final result is available.

## Messages
A message is a FIFO stack of values
- Designed for async execution.
- Still useful during synchronous exeuction for debugging or unrolling thread execution.

Async syntax
```
result = async calculate(args...)
->msg(out) { // optional message binding, executed on each new message
  println(out)
}
await result // end of async
```
Sync syntax
```
result = await calculate(args...) // or directly await calculate(args...)
->msg(out) { // optional message binding
  
}
// end of sync
```
> Note: to bind on messages, you must specify the prefix `sync` or `await` to avoid non thread logic
> Messages are considered as a threading tweak, not a regular coding case

# Key Points
- async always launches an asynchronous thread.
- await blocks the caller, but the thread remains async internally.
- FIFO messages + future for the final result provide a predictable and readable flow.
- The message queue is automatically managed by the runtime and attached to each async thread.
- There is no need for a specialized “thread fn” declaration: any normal function can be spawned via async or await.

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

export C {

# extern 
fn printf(_Format: str, args: addr...) -> void;

}
```
LLVM will mark these functions externals (`# extern`) and search in C ABI (`export C {...}`)


# Metaprogrammation
metaprogrammation is behaviour declarative who starts with `#`
can define some behaviour : module exportation, async, parallel, contigous memory alignment, etc...

## Cumulative Metacode
cumulative metacode union behaviours when the new line have # 

if the new line don't have # the metacode is no longer cumulative
```
# export
# async

fn function<T>(a: T) -> i32 {

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

## Metacode Block
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

## Metacode Scoped
replication of the metacode specification to all objects in the scope 
use simply `# scope ... # end` or named scode `# scope name ... # end`.
the scope is the end of the metacode block

you can disable all metacode superior scope with: `# exclude` or for a specific scope: `# exclude name`
scopes are not interdependent, an exclusion of an specific scope disable only the named scope, the superiors or inferiors scopes will not be impacted.

example:
```
# if os == windows or os == linux

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
  # export
  fn update_pos_ui() { ... }
}
# end if // scope
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
| operating system condition | `# if os == ...` | `# if os == linux` | indicate the operating system (Linux/Windows/MacOS/...) |
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
 

# Naming Convention (recommended)
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

# Mangling 

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
| operating system used | `# os ""` | |

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

## Entity Target
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
