# Literal asm Instruction
> To make inline assembly code directly added in the final generation

Useful for low-level coding like asm { } in C
```
asm {...}
```
Rules:
- Any in variable must be marked `# asm in`
- Any out/inout variable result must be marked `# asm out`
- The function who contains the `asm` instruction must be marked `# unsafe`

e.g.
```
# unsafe
fn add(copy a: i32, copy b: i32) -> i32 {
  # asm in
  let _a: i32 = a
  # asm in
  let _b: i32 = b
  # asm out
  let result: i32
  
  asm {
    mov eax, _a
    mov ebx, _b
    add eax, ebx
    mov _result, eax
  }
  return result
}
```

>Note: Considered unsafe by nature. Syntax error will creates a compilation error.
