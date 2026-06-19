# Memory Managment
The memory use a fine managment, there is no GC

There is two types of management of memory:
- Pointers -> memory on heap
- Capabilities -> memory on stack

# Memory Managment: Pointers
| name | syntax | info |
|-|-|-|
| non typed memory address | `ptr'u0` | useful for C interop (`void*`) |
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

## Pointer Operations
No standard arithmetic notation is permitted to avoid any ambiguous memory manipulation

| operation | syntax | return type | info |
|-|-|-|-|
at | `my_ptr'at(i)` | `ptr'T` on the index offset caculated | dangerous on raw ptr, safe on fat ptr (value indexed on pointer, not on the fat ptr) |
get val | `val'my_ptr` | `T` | get ptr value at 0, can be combined with `val'my_ptr'at(i)` |
size | `size'my_ptr` | `usize` | only on fat ptr, get the value on the index 1 of the fat ptr |
offset | `my_ptr'offset(+-i)` | `ptr'T` | add/remove offset of type size * index in the ptr address, moves the pointer position | 
diff | `my_ptr1 <-> my_ptr2` | `ptrdiff` | distance between two pointers |


## Pointer usage
Pointers are handled by the language, despite of the capability rule.
It's designed for low-level operations
You can use them for some operations: (non-exhaustive list)
| usage | syntax | info |
|-|-|-|
| arbitrary memory location | `let my_console_input: $ptr'i32! = 0xFFFFFF as! $ptr'i32!` | classic const pointer of volatile value from a input for embbed rules |
| change memory address | `fn swap_ptr(addr a: i32, addr b: i32) {...} swap(my_ptr1, my_ptr2)` | any addr parameter is considered aliased by security |
| complex function return | `fn get_hardware_output(copy hardware_ty: EHardwareOutType) -> ptr'i32` |  |

