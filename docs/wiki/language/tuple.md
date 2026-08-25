# Tuple
tuples are implicit they a deduced most of the time in `( ... )`

tuple cases:

| name | syntax | info
|-|-|-|
| tuple type | `(T, U, ...)` | |
| tuple instance | `(val1, val2, ...)` | |
| named tuple type | `(name1: T, name2: U, ...)` | |
| named tuple instance | `(name1= val1, name2= val2)` | |  
| static access | `let first = a.0` | |
| static access named | `let first = a.name` | |
| dynamic access | `let first = a.get(k)` | |
| function return | `fn name() -> (T, U, ...)` | tuple return |
| function return | `fn name() -> (name1: T, name2: U, ...)` | named tuple return | 
| unpack tuple from call | `var (a, b, c) = name()` | |
| unpack tuple from variable | `var (a, _, c) = var_tuple` | |

## Tuple Cast
Tuples and named tuples are the same for the compilator but need to de distinguished for users to makes proprer conversion.

Conversions:

| cast type | base | syntax | result | info |
|-|-|-|-|-|
| positional | `let t1 = (10, 2.0, "hello")` | `t2 = t1 as (2, 1, 0)` | `t2 = ("hello", 2.0, 10)` | in each position, set the new position by the index |
| nomenclature | `let t1 = (a= 10, b= 2.0, c= "hello")` | `t2 = t1 as (c, b, a)` | `t2 = ("hello", 2.0, 10)` | in each position, set the new position by the field name | 

