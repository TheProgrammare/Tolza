# Implementation
An implementation is a function callable by the dot access '.' on any data with the same type of the first special parameter function

To define a implementation, use the keyword `impl` then function syntax with the type before the name use as a path access, the compiler will allow to call this function on any data with the same type

Syntax: 
```
impl type::name([params]) [-> <return_type>] {...}
```

> Note: to use the targered type instance, use the keyword `self`

| type | syntax | consequence | call |
|-|-|-|-|
| immuable implementation | `impl str::print_message(ref self, lvl: EMsgLvl) {...}` | the implementation is marked constant | `"hello world".print_message(EMsgLvl::log)` `msg.print_message(EMsgLvl::log)` |
| mutable implementation | `impl usize::incr(mut self) { self += 1 }` | the implementation is marked mutable | `var count: usize = 0`</br>`count.incr()` |
| static implementation | `impl fsize::PI() { return 3.14159265359 }` | the implementation is marked static (no self), always returns the type implemented | `let my_pi = fsize::PI()` |
| cast from implementation | `impl str as ssize {`</br>`  var end: ptr'cune`</br>`  return C::strtoull(self, end, 10) as ssize`</br>`}` (self is the origin, return the value converted) | the implementation is marked cast from | `let my_size: ssize = "12345" as ssize` |
| validation implementation | `impl MyType as bool {`</br>`  return self.is_genreated and self.not_empty()`</br>`}` (self is the origin, return boolean) | the implementation is marked validation | `if my_type {...}` |
| immuable operator implementation | `impl MyType op + { return self + other }` | the two terms are always the same types, returns the type | `my_type1 + my_type2`
| mutable operator implementation | `impl MyType op += { self = self + other }` | the two terms are always the same types, returns the type | `my_type1 += my_type2`
| comparison operator implementation | `impl MyType op <=> { return self.member <=> other.member }` | the two terms are always the same types, returns boolean value | `my_type1 > my_type2`
> Note: the `# pure` metacode can be used to forbidding any border effects

An implementation have a type namespace, the module importation will allow the usage but the path will be on the type (like a static implementation)

e.g.
```
impl usize::print() { C::printf("%u", self) }
impl ssize::print() { C::printf("%d", self) }

let a: usize = 10
let b: ssize = -5
a.print()
b.print() 
```

> Avoid implementation functions when a more elaborated data managment is required, use COP paradigm instead



## Generics
It's possible to set generics on implementation parameters, even on `self`.

e.g.
```
impl T::incr<T: op+>() { self += 1 }

let a: usize = 10
let b: ssize = -5
a.incr()
b.incr() 
```

> The most specific implementation always win (generics are the least selected)

generic on static implementation: the type namespace fix the generic type

e.g.
```
impl T::PI<T: floating>(self: T) { self = 3.14159265359 }

let my_pi = f32::PI()
```

# Pipe-call 
Implementations permit to use pipe call to show the data flow intention

The call start by a value and a implementation function, then the next call must be a compatible implementation on the returned type of the anterior call.

The execution flow is left to right 

e.g.
```
var filtered = MyCollection::normalize(MyCollection::exclude(my_collection.sort(), "NONE"))

// this is equivalent to
var filtered = my_collection.sort().exclude("NONE").normalize()
```

> Avoid pipe-call when there is only one call

## Pipe-call with COP systems
Systems call are also a pipeline, it's possible to combine system pipes with implementation pipes 

e.g.
```
let success = my_entity->sys_operate()->sys_get_sub_collection().is_empty()
//            ^--------^----------------------------------------^-----------
//            |        systems pipelines                        implementation pipeline
//            entity call
```
