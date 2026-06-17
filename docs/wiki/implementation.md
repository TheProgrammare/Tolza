# Implementation
An implementation is a function callable by the dot access '.' on any data with the same type of the first special parameter function

To define a implementation, use the function syntax, then name the first parameter with the keyword `self`, the compiler will allow to call this function on any data with the same type

Syntax: 
```
fn name([<passmode>] self: <type> [, ...]) [-> <return_type> {...}
```

The passmode is either `ref` or `mut` or nothing:

| passmode | syntax | consequence | call |
|-|-|-|-|
| `ref` | `fn print_message(ref self: str, lvl: EMsgLvl) {...}` | the implementation is marked constant | `"hello world".print_message(EMsgLvl::log)` `msg.print_message(EMsgLvl::log)` |
| `mut` | `fn incr(mut self: usize) { self += 1 }` | the implementation is marked mutable | `var count: usize = 0`</br>`count.incr()` |
| nothing | `fn PI(self: fsize) { self = 3.14159265359 }` | the implementation is marked static | `let my_pi = fsize::PI()` |

An implementation have a type namespace, the module importation will allow the usage but the path will be on the type (like a static implementation)

So implementation functions can have the same name on different `self` parameter type

e.g.
```
fn print(ref self: usize) { C::printf("%u", self) }
fn print(ref self: ssize) { C::printf("%d", self) }

let a: usize = 10
let b: ssize = -5
a.print()
b.print() 
```
 

## Generics
It's possible to set generics on implementation parameters, even on `self`.

e.g.
```
fn incr<T: op+>(self: T) { self += 1 }

let a: usize = 10
let b: ssize = -5
a.incr()
b.incr() 
```

generic on static implementation: the type namespace fix the generic type

e.g.
```
fn PI<T: floating>(self: T) { self = 3.14159265359 }

let my_pi = f32::PI()
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
