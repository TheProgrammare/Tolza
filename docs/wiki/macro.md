# macro 
macro are AST transformers

syntax:
```
macro name(<params>) -> <target> {...}
```
Explanation:
Target permit to precise the object applied `fn|type|lam|var|let|gen|...`, `_` means any

If the target is not specified the macro is a inplace code constructor

the target can be modified by some fields
> e.g. `macro name() -> fn { fn.name fn.code fn.attributes ... }`

macro e.g.
```
macro log_async(timeout_: f32) -> fn {
  fn.attributes.add(async)
  fn.attributes.add(pure)
  fn.code.before {
    var timer = std::time::start()
  }
  fn.code.after {
    std::console::log("run time: " + timer as str)
  }
  fn.on_call {
    call timeout timeout_
  }
}

# log_async(10)
fn fonc_example() {};
```

metacode block can be exported
```
export {
  macro metacode_reused() -> fn
}
```

macro parameters can be simple types resolved at compilation time

or macro parameters can be ast types
