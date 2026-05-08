# Variable
Variables are statically typed or type inferred

A variable always must be declared with a value before any read

| type | syntax |
|-|-|
| mutable typed | `var name: T` |
| mutable typed setted | `var name: T = ...` |
| mutable inferred setted | `var name = ...` |
| immutable typed | `let name: T = ...` |
| immutable inferred | `let name = ...` |
| compiletime typed | `const name: T = ...` |
| compiletime inferred | `const name = ...` |

## Assignation
variables are managed by a explicit move mode and capacity assignation
the default affectation `=` is a move semantic except for primitives who are a copy (performance reason)
> See Capabilities

| assignation mode | syntax | note |
|-|-|-|
| Copy | `lhs copy= rhs`    | For complex types: check for a copy method. For primitives: copy |
| Clone | `lhs clone= rhs`     | For complex types: check for a clone method. For primitives: copy |
| Move | `lhs move= rhs` or `lhs = rhs`   | Revokes all previous capabilities (ref + mut) see explicit Capabilities |

## Operation Assignation
variables can be mofied directly by a arithmetic operation (read/write) operation
`+=` `-=` `*=` `/=` `%mod%=` `%quo%=` `%rem%=` `**=` 


