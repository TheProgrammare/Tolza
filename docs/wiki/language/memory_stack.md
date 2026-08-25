# Memory Managment: Capabilities
Access to a variable wihout any copy/clone/pointers, use the **explicit capabilities** for better code safety.
| capability | operation | declaration syntax |
|-|-|-|
| `ref` | shared reading | `ref a = x` |
| `mut` | exclusive r/w | `mut a = x` |
| `ref` returned | from function return | `fn return_ref(ref a: T) -> ref'T(a)`</br>`ref b = return_ref(a)` 
| `mut` returned | from function return | `fn return_mut(mut a: T) -> mut'T(a)`</br>`mut b = return_mut(a)` 

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
| Mut/ref index on slice | `ref index = mut_slice[4]` | only in deterministic slice |

> note: all revoked capabilities **cannot be reused**

## Revocation of Capabilities (other cases)
| Case | syntax | consequence |
|-|-|-|
| **Explicit drop** | `drop a_ref` | the capability is removed |
| **End of scope** | `var a = 10 {`<br>`mut m = a`</br>`m += 10 }` | automatic revocation |
| **Revocable parameter** | `fn revoke_mut(mut a: i32) -> mut'i32(a) {...}`</br>`mut m = a`</br>`mut out_mut = revoke_mut(a) }` | revocation from function lifetime signature |
| **Parent data handling** | `mut p_name = player.name`</br>`player = Player::new("marc", 25)` |  r/w a parent revokes children's capabilities. But fields are considered separate.
| **Parent Collection operations** | `mut slice_mut = a[0..10]`</br>`a = {10, 20, 30}` | slice/index capabilites follow the parent's (collection base) operations.
| **Move instruction** | `ref a_ref = a`</br>`b move= a` or move parameter -> all capabilities are revoked and the origin is removed.

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
