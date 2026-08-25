# Multi-threading
Multi-threading is handled natively.

The threading logic is simple:
- Pure `async` function
- Thread invocation via an `async` or `await` prefix then function call
- The function result is automatically packed as Future<T>
- Access intermediate values during async execution via messages
- Access the final result after async via `await`

## Async Function
A `async` function is **always pure**, without side effects.
- All externals values must be passed as parameters.
- Only global constants can be used inside the function.
- You can specify a **message stack** to handle messages during thread execution
```
// thread compatible function
// + message handled function
# pure // a compilation check argument
fn calculate(copy a: i32, copy b: i32) -> i32 // return i32 packed as Future<i32> when called with async or await
->tmsg<str> // enable messages binding, typed as string
{
  let c = a / b - a * b
  send "{c}" // put text to out 1st received
  let d = c * b
  send "{d}" // put text to out 2nd received
  return d // returns the d value
}
```

## Thread Invoking
A thread is invoked using the `async` or `await` prefixes before a function call

async execution
```
let result = async my_fn()   // declaration required to use await later
// async execution
await result                 // mandatory after async
println(result)              // retrieve the final value
```

await (sync) execution
```
let result = await my_fn() // or simply `my_fn()`, declaration optional
// synchronous execution by nature
println(result) // final value guaranteed
```
> Note: `await` does **not** make the thread synchronous. The thread remains async internally; onlu the caller is blocked untile the final result is available.

## Messages
A message is a FIFO stack of values
- Designed for async execution.
- Still useful during synchronous exeuction for debugging or unrolling thread execution.

Async syntax
```
result = async calculate(args...)
->msg(out) { // optional message binding, executed on each new message
  println(out)
}
await result // end of async
```
Sync syntax
```
result = await calculate(args...) // or directly await calculate(args...)
->msg(out) { // optional message binding
}
// end of sync
```
> Note: to bind on messages, you must specify the prefix `sync` or `await` to avoid non thread logic
> Messages are considered as a threading tweak, not a regular coding case

## Key Points
- async always launches an asynchronous thread.
- await blocks the caller, but the thread remains async internally.
- FIFO messages + future for the final result provide a predictable and readable flow.
- The message queue is automatically managed by the runtime and attached to each async thread.
- There is no need for a specialized “thread fn” declaration: any normal function can be spawned via async or await.

