# Lambda
> use `lam` keyword to declare a lambda, threated like c++ : anonym functions
```
lam [<name>]['['<capture>']'][(<params>)] [-> <return_type>] { ... }
```

| type | syntax |
|-|-|
| named lambda | `lam l_name[capture](parameters) -> return {...}` |
| anonym lambda for predicate purpose | `lam [capture](parameters) -> return {...}` |
| simple lambda | `lam {...}` |

> same calling as function

## Lambda Capture
to capture variables in scopes

| target | syntax | extension with exceptions |
|-|-|-|
| to modify all variables | `[mut]` | `[mut, copy a, copy b]` |
| to copy all variables | `[copy]` | `[copy, mut a, mut b]` |
| to get instance | `[self]` | `[..., self,...]` |



