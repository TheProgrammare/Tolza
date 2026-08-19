# Contract
Contracts are pre and post conditions defined inside functions signature

Contracts are immutables, there are only checker
The default contract condition mode is `static`

Preconditions :
| mode | syntax | info |
|-|-|-|
| static | `pre <cond>` | static proof mandatory, no runtime check |
| debug | `pre <cond> -> assert` | makes an assertion before the call, only in debug build |
| runtime | `pre <cond> -> <error>` | transform the function return to Result<T, E | ...>, check before the call, condition violation will not execute the function then return the error |
| runtime | `pre <cond> -> panic("msg")` | engage a panic behavior before the call, panic can be catched or will terminate the process |

> on `debug` and runtime mode, the compiler will try to avoid runtime checking if the values are statically evaluable

Postcondition :
| mode | syntax | info |
|-|-|-|
| static | `post <cond>` | static contract verification |
| debug | `post <cond> -> assert` | makes an assertion after the call, only in debug build |
| runtime | `post <cond> -> <error>` | transform the function return to Result<T, ... | E>, check after the call, condition violation return the error |
| runtime | `post <cond> -> panic("msg")` | engage a panic behavior after the call, panic can be catched or will terminate the process |

syntax example:
```velox
fn foo(x: ssize) -> ssize // real return Result<ssize, error::bad_sign | error::bad_value>
  pre x > 0 -> error::bad_sign
  post ret >= x -> error::bad_value
{ ... }

fn main() {
  let result = foo(10)
  match result {
  Ok(val) => println(val)
  Err(msg) => println(msg)
  }
}
```
