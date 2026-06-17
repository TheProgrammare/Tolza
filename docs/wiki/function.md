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
