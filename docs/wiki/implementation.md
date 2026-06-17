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

> Static implementation functions are type constructor like 

> Avoid implementation functions when a more elaborated data managment is required, use COP paradigm instead

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

# Pipe-call 
Implementations permit to use pipe call to show the data flow intention

To make a pipe-call, use the pipe operator `|`

The call start by a value and a implementation function, then the next call must be a compatible implementation on the returned type of the anterior call.

The execution flow is left to right 

e.g.
```
var filtered = my_collection | sort() | exclude("NONE") | normalize()

// this is equivalent to

var filtered = normalize(exclude(sort(my_collection), "NONE"))
```

> Avoid pipe-call when there is only one call

