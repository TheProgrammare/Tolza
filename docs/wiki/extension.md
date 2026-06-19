# Extension
An extension is a function callable by the dot access '.' on any data with the same type of the first special parameter function

To define a extension, use the keyword `extend` then function syntax with the type before the name use as a path access, the compiler will allow to call this function on any data with the same type

Syntax: 
```
extend type::name([params]) [-> <return_type>] {...}
```

> Note: to use the targered type instance, use the keyword `self`

| type | syntax | consequence | call |
|-|-|-|-|
| immuable | `extend T fn name(ref self, args...) -> U {...}` | the extension is marked constant | `"hello world".print_message(EMsgLvl::log)` `msg.print_message(EMsgLvl::log)` |
| mutable | `extend T fn name(mut self, args...) -> U {...}` | the extension is marked mutable | `var count: usize = 0`</br>`count.incr()` |
| static | `extend T fn name(args...) -> T {...}` | the extension is marked static (no self), always returns the type extended | `let my_pi = fsize::PI()` |
| cast | `extend T as U {...}` (self is the origin, return the value converted) | the extension is marked cast | `let my_size: ssize = "12345" as ssize` |
| operator | `extend T op + {...}` | the two terms are always the same types, returns the type | `my_type1 + my_type2`

> Note: the `# pure` metacode can be used to forbidding any border effects

> *See [operation extension](#operation-extension) section

An extension have a type namespace, the module importation will allow the usage but the path will be on the type (like a static extension)

e.g.
```
extend usize fn print() { C::printf("%u", self) }
extend ssize fn print() { C::printf("%d", self) }

let a: usize = 10
let b: ssize = -5
a.print()
b.print() 
```

> Avoid extension functions when a more elaborated data managment is required, use SFM paradigm instead

## operation extension

| type | syntax | consequence | call |
|-|-|-|-|
| immuable operator | `extend T op + {...}` | the two terms are always the same types, returns the type | `my_type1 + my_type2`
| mutable operator | `extend T op += {...}` | the two terms are always the same types, returns the type | `my_type1 += my_type2`
| ordering | `extend T op <=> {...}` | the two terms are always the same types, returns ordering value, generate boolean version, no equality, can be overrided | `my_type1 > my_type2`
| index operator | `extend T op [index: usize] -> U {...}` | returns the type reference mut/ref | `ref elem = my_type[1]`
| index bound operator | `extend MyType op ?[index: usize] -> T? {...}` | returns optional | `if let elem = my_type?[1]`
| slice operator | `extend T op [start..end] -> Slice<U> {...}` | returns the slice type | `ref elem = my_type[0..8]`
| slice bound operator | `extend T op ?[start..end] -> Slice<U>? {...}` | returns the slice type | `ref elem = my_type?[0..8]`
| slice bits operator | `extend T op ~[start..end] -> U {...}` | returns byte type | `let bits: bsize = my_type~[0..64]`
| transfert copy operator | `extend T op copy -> T {...}` | returns a copy | `let my_copy: MyType copy= my_type`
| transfert move operator | `extend T op move -> T {...}` | move semantic, drop self | `let my_copy: MyType = my_type`
| delete operator | `extend T op del {...}` | drop self | `del my_type`
| predicat | `extend T op pred {...}` (self is the origin, returns boolean) | the extension is marked predicat | `if my_type {...}` |

## Generics
It's possible to set generics on extension parameters, even on `self`.

e.g.
```
extend T fn incr<T: op+>() { self += 1 }

let a: usize = 10
let b: ssize = -5
a.incr()
b.incr() 
```

> The most specific extension always win (generics are the least selected)

generic on static extension: the type namespace fix the generic type

e.g.
```
extend T fn PI<T: floating>(self: T) { self = 3.14159265359 }

let my_pi = f32::PI()
```

# Pipe-call 
Extensions permit to use pipe call to show the data flow intention

The call start by a value and a extension function, then the next call must be a compatible extension on the returned type of the anterior call.

The execution flow is left to right 

e.g.
```
var filtered = MyCollection::normalize(MyCollection::exclude(my_collection.sort(), "NONE"))

// this is equivalent to
var filtered = my_collection.sort().exclude("NONE").normalize()
```

> Avoid pipe-call when there is only one call

## Pipe-call with SFM rules
Rules call are also a pipeline, it's possible to combine rule pipes with extension pipes 

e.g.
```
let success = my_form->rule_operate()->rule_get_sub_collection().is_empty()
//            ^--------^----------------------------------------^-----------
//            |        rules pipelines                        extension pipeline
//            form call
```
