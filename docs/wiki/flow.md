# Flow

## Conditions: if elif else
control flow by condition

| flow | syntax codeblock | syntax inline |
|-|-|-|
| if | `if <condition> { ... }` | `if <condition> => ...` |
| elif | `elif <condition> { ... }` | `elif <condition> => ...` |
| else | `else { ... }` | `else ...` |

### Ternary if
Ternary if is a control flow for value fields
```
if <condition> => <true statement>
if <condition> => <true_statement> else <false_statement>
```

## Match
Match statement control flow for code logic by matching comparison on value. 

Execution stop to the case executed. Or use metacode `# fallthrough`
```
match <value> {
  // case states ...
}
```

### Case Statement
Define inside match

| statement case | syntax |
|-|-|
| case codeblock | `<evaluator> => { ... }` |
| case inline | `<evaluator> => ...` |

e.g.
| type | syntax |
|-|-|
| compare literal integral | `10 =>` |
| compare literal string | `"hello" =>` |
| compare value | `if val > 100 =>` |
| compare in range | `10..=30 =>` |
| match typed enum and bind value | `Some(a) =>` |
| match typed enum, bind value and compare | `Some(a) if a > 10 =>` |
| match on enum | `EEnum::Elem =>` |
| other case | `_ =>` | 

## Loop
Highlty not recommended

| statement type | syntax |
|-|-|
| codeblock | `loop { ... }` |
| inline | `loop => ...` |

## While
| statement type | syntax |
|-|-|
| codeblock | `while <condition> { ... }` |
| inline | `while <condition> => ...` |

## Do-while Statement
| statement type | syntax |
|-|-|
| codeblock | `do { ... } while <condition>;` |
| inline | `do => ... while <condition>;` |

## For Loop Statement
For loop can be used on range, slice, collection and map

Syntax:

| statement type | syntax | info |
|-|-|-|
| for index | `for <index> in <range> [step <constant>] { ... }` | can use `step` after `<range>` |
| for item | `for [<index>] mut/ref/copy/move <item> in <slice/collection> { ... }` |  |
| for item | `for [<index>] mut/ref/copy/move (<item1>, <item2>, ...) in <slice/collection> { ... }` | useful for map or tuple array  |

>Inline case: `for ... => <expression>`

# Goto Statement
The goto statement is scoped, you can use it without fear.

| state | syntax |
|-|-|
| go to and label | `goto name` |
| label definition | `label name {...}` `label name => ...` |

