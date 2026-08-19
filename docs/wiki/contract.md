# Contract
Contracts are pre and post conditions defined inside functions signature

Contracts are immutables, there are only checker
The default contract condition mode is `static`

Preconditions :
| mode | syntax | info |
|-|-|-|
| static | `pre <cond> -> static` `pre <cond>` | static evaluation mandatory, no runtime check |
| debug | `pre <cond> -> debug` | makes an assertion before the call, only in debug build |
| runtime | `pre <cond> -> runtime` | transform the function return to Result<T, str>, check before the call, condition violation will not execute the function then return the string error |

> on `debug` and `runtime` mode, the compiler will try to avoid runtime checking if the values are statically evaluable

Postcondition :
| mode | syntax | info |
|-|-|-|
| static | `post <cond> -> static` `post <cond>` | static execution mandatory, the function becomes statically evaluable |
| debug | `post <cond> -> debug` | makes an assertion after the call, only in debug build |
| runtime | `post <cond> -> runtime` | transform the function return to Result<T, str>, check after the call, condition violation return the string error |

