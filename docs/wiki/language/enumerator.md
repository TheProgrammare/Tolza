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
