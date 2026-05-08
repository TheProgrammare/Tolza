# Parameters
parameters are managed by a pass mode and a type base

## Pass Modes
There is 6 pass modes:
| mode | syntax | info |
|-|-|-|
| `ref` | `ref name: T [= default_val]` | by reference (immutable)
| `mut` | `mut name: T` | by mutation (mutable)
| `copy` | `copy name: T [= default_val]` | by copy forced
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
| `copy` | `copy` | call `copy`
| `move` | `copy` | `move` and invalidate origin 
| `addr` | only `b8`-`b128` | only `ptr'T`

> \* Copy and Clone arguments can be overrided during the call by `copy` or `clone` `fn copy_myvar(copy a: MyVar)` `copy_myvar(clone my_var)`<\br>
> The compiler will check if the clone or copy method exists, otherwise he will try to call the other method, if no copy and clone exists, a compilation error occur.

### by Capability
| mode | regular variable | ref | mut
|-|-|-|-|
| `ref` | local ref, no add ref | add ref to origin | X 
| `mut` | local mut, no add mut | X | redirect mut 
| `copy` | read operation, call copy | idem | idem 
| `move` | consumed | ref consumed | mut consumed 
| `addr` | only raw pointer `ptr'T` | X | X

### by Pointer
| mode | ptr | uptr | sptr |
|-|-|-|-|
| `ref` | read only ptr and pointee | X | read only sptr and pointee |
| `mut` | read only ptr, mutable pointee | X | read only sptr, mutable pointee | 
| `copy` | copy ptr address | X | share uptr (sharing > copy) |
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

