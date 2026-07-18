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
macro print(val: ast::Expression) {
  quote { std::console::log(val) }
}

macro error(val: ast::Expression) {
  quote { std::console::error(val) }
}

macro panic(val: ast::Expression) {
  ...
}

macro log_async(timeout_: f32) -> fn {
  fn.attributes.add(async)
  fn.attributes.add(pure)
  fn.call.before(
    quote {
      var timer = std::time::start()
    }
  )
  fn.code.after(
    quote {
      #print("run time: " + timer as str)
    }
  )
  fn.call.after(
    quote {
      if timer > timeout_ {
        #error("execution takes too long time")
        #panic()
      }
    }
  )
}

#log_async(10)
fn fonc_example() {};
```

metacode block can be exported
```
export {
  macro metacode_reused() -> fn {...}
}
```

macro parameters can be simple types resolved at compilation time

or macro parameters can be ast types

## AST Types
unique types to configure the compiler ast nodes

| node | fields
