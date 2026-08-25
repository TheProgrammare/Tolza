# Primitive tables
All primitive tables are fat pointers, there is no raw table like in C

| type | syntax | literal |  info |
|-|-|-|-|
| static array | `[T; N]` | `[ 1, 2, 3, 4 ]`,<br> `[ 0..4 => 8 ]` (4 elements equals to 8) | compile time table size
| dynamic array | `[T; _]` | `[ 1, 2, 3, 4 ]_`,<br> `[ 0..4 = 8 ]_` | dynamic table size
| static tensor | `[T; N, N, ...]` | `[0,0,0;0,0,0;0,0,0]` | compile time dimension size
| dynamic tensor | `[T; N, _]` | `[0,0,0;0,0,0;0,0,0]_` | dynamic dimension size
| slice | `[T; ..]` | `ref'<variable>[]` `mut'<variable>[]` or standard literal array | view of any table
| static map | `[T: U; N]` | `[a: 0, b: 1, ...]` | 
| dynamic map | `[T: U; _]` | `[a: 0, b: 1, ...]_` | 

## Primitive table fields
Access to any table field by the suffix operator like `my_table'size` 
table overhead structures are designed by the same order

| table | `'data` | `'size` | `'capa` |
|-|-|-|-|
| static array | `ptr'T` | `usize` array | NO |
| dynamic array | `ptr'T` | `usize` array | `usize` array |
| static tensor | `ptr'T` |  `usize` dimension | NO |
| dynamic tensor | `ptr'T` | `usize` dimension | `usize` dimension |

> e.g. get tensor 3rd sub table capacity  `my_dy_tensor'data'at(2)` for the `'at()` syntax, check ![Pointer Operations](#pointer-operations)

> e.g. get tensor dimension size `my_tensor'size`,


# Table Population
when you create a table instance `[ a, b, ... ]` you can avoid the explicit value affectation and use an table population syntax to put in table literal !

| type | syntax | e.g. | explicit form | info |
|-|-|-|-|-|
| table population | `<range> => <expression>` | `[ 0..3 => 5 ]`,<br> `[ 0..3 => rand::uniform() ]` | `[ 5, 5, 5 ]`,<br> `[ 0.25f, 0.777f, 0.05f ]` | the most simple case |
| table population index relative | `start..end(i)`, `start..end(j)`, ... | `[ 0..3(i) => i + 10 ]` | `[ 10, 11, 12 ]` | useful for position aware in population | 
| map population index relative | `<range> => <expression_keys> : <expression_values>` | `[ 0..3(i) => [i: number_text[i]] ]` | `[ 0: "zero", 1: "one", 2; "two" ]` |  | 
| matrix population | `[ <range1>, <range2>, ... => <expression> ]`, | `[ 0..3(i), 0..3(j) => i + j + 10 ]` | `[10,11,12;11,12,13]` | a good level of abstraction |

# Range
Borned with a start integral, end integral (and optional step only for `for` loop).

range can be use to extract slice from collection with special ranges.

| range type | syntax | info |
|-|-|-|
| range end exclude | `0..10` | will go from 0 to 9 |
| range end include | `0..=10` | will go from 0 to 10 |
| range all | `collection[..]` | will return the collection, not very useful there |
| range from 0 | `..11` | will go from 0 or the start of the collection to the 10th index (so 11 elements slice) |
| range to max | `5..` | will go from 5 to the end of the collection or the max value `i64` |

# Slice
Returns a view according to specified range, can be mutable and immutable

| slice type | syntax |
|-|-|
| immutable slice | `ref'collection[<range>]` |
| mutable slice | `mut'collection[<range>]` | 

slice body
basically a fat pointer
```
slice { first_elem: ptr'T, length: usize }
```


