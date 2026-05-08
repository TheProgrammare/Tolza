# Operators
## Memory
> See Memory section

| memory | syntax | info |
|-|-|-|
| move           | `move=`, complex type default: `=` | assignation by move semantic
| copy           | `copy=`, primitive default: `=` | assignation by copy method, otherwise clone method used
| clone          | `clone=`  | assignation by clone method, otherwise copy method used
| drop           | `drop my_var` | revoke `ref`/`mut`
| new            | `new ptr'T()` | memory allocation on heap
| delete         | `del my_ptr` | free pointer
| val of ptr     | `val'my_ptr` | get pointer value (return always optional -> None if ptr is None)
| address of     | `addr'my_val` | get the memory address of value -> returns raw ptr'T of val type| 

## Arithmetic
> See COP section

| arithmetic | syntax | info |
|-|-|-|
| add            | `+`          | |
| subtract       | `-`          | |
| multiply       | `*`          | |
| matrix multiplication | `@` | only on matrices |
| divide         | `/`          | result always f32/f64   |
| remain         | `%rem%`      | remainder singed with dividend |
| modulo         | `%mod%`      | euclidian remainder (modulo not negative if divisor>0) |
| quotient       | `%quo%`      | euclidian quotient      |
| quotient       | `%divrem%`   | euclidian quotient      |
| power          | `**`         |                         |
| increment      | `++`         |                         |
| decrement      | `--`         |                         |

## Comparison
> See COP section

| comparison | syntax | info |
|-|-|-|
| greater        | `>`          |                         |
| lower          | `<`          |                         |
| greater equal  | `>=`         |                         |
| lower equal    | `<=`         |                         |
| equal          | `==`         |                         |
| equal strictly | `===`        | for string and float    |
| not equal      | `!=`         |                         |
| not eq strictly| `!==`        | for string and float    |

## Logical
> See COP section

| logical | bitwise | syntax | syntax bitwise |
|-|-|-|-|
| and   | and.b   |`and`  | `b.and`  |
| nand  | nand.b  |`nand` `!and` | `b.nand` `!b.and` |
| or    | or.b    |`or`   | `b.or`   |
| xor   | xor.b   |`xor`  | `b.xor`  |
| nor   | nor.b   |`nor` `!or`  | `b.nor` `!b.or` |
| xnor  | xnor.b  |`xnor` `!xor` | `b.xnor` `!b.xor` |

## Binary
> Only on `flag` and `b8`-`b128`

| binary | syntax | info |
|-|-|-|
| left shift 0   | `b.shl.0` | fill right with 0       |
| left shift 1   | `b.shl.1` | fill right with 1       |
| right shift 0  | `b.shr.0` | fill left with 0        |
| right shift 1  | `b.shr.1` | fill left with 1        |
| left shift a   | `b.shl.a` | fill right with MSB (arithmetic) |
| right shift a  | `b.shr.a` | fill left with MSB (arithmetic) |
| left rotate    | `b.rol` | rotate bits to the left |
| right rotate   | `b.ror` | rotate bits to the right |

