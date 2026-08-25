# Primitive tables
All primitive tables are fat pointers, there is no raw table like in C

| type | syntax | literal |  info |
|-|-|-|-|
| static array | `[T; N]` | `{ 1, 2, 3, 4}`,<br> `{ 0..4 = 8 }` (4 elements equals to 8) | compile time table size
| dynamic array | `[T]` | `{ 1, 2, 3, 4}d`,<br> `{ 0..4 = 8 }d` | dynamic table size
| static tensor | `[T; N, N, ...]`,<br> `[T; N]*D` | `{{0,0,0},{0,0,0},{0,0,0}}` `{ 1, 2, 3, 4}*3` | compile time dimension size
| dynamic tensor | `[T]*_` | `{{0,0,0},{0,0,0},{0,0,0}}d` `{ 1, 2, 3, 4}d*3` | dynamic dimension size

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
when you create a table instance `{ a, b, ... }` you can avoid the explicit value affectation and use an table population syntax to put in table literal !

| type | syntax | e.g. | explicit form | info |
|-|-|-|-|-|
| table population | `[<range>] => <expression>` | `{ [0..3] => 5 }`,<br> `{ [0..3] => rand::uniform() }` | `{ 5, 5, 5 }`,<br> `{ 0.25f, 0.777f, 0.05f }` | the most simple case |
| table population index relative | `@i`, `@j`, ... | `{ [0..3] => @i + 10 }` | `{ 10, 11, 12 }` | useful for position aware in population | 
| map population index relative | `[<range>] => <expression_keys> : <expression_values>` | `{ [0..3] => @i : number_text[@i] }` | `{ 0: "zero", 1: "one", 2; "two" }` |  | 
| matrix population | `{ [<range1>, <range2>, ...] => <expression> }`,<br> `{ [<range>] => <expression> }*N` | `{ [0..3, 0..3] => @i + @j + 10 }`,<br> `{ [0..3] => @i + @j + 10}*N` | `{{10,11,12},{11,12,13}}` | a good level of abstraction |

table population use compiler reserved indentifier to use the indexation during the table population:

- `@i` first dimension
- `@j` second dimension
- ... 


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
| immutable slice | `collection'ref[start..end]` |
| mutable slice | `collection'mut[start..end]` | 

slice body
basically a fat pointer
```
slice { first_elem: ptr'T, length: usize }
```


