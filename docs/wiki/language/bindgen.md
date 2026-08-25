# Bindgen
Bindgen is external binds auto generated scripts when a user use `import bind::<lang>::<lib> as <alias>`

The bindgen will store the importations of the external lib and generate all the wrapper needed when the user use the external lib in his script

user script: 
```
// form_messages.tlz

import ext: C::stdio

fn speak(s: str) {
  C::printf(s)
}
```

bindgen script:
```
// file: bindings/C/stdio.tlzbind

export {
extern "C" {
  fn printf(_Format: str, args: addr...) -> u0;
}
}
```
LLVM will mark these functions externals (`extern`) and search in C ABI
