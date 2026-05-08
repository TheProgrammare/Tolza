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
