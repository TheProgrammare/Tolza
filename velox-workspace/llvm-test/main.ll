; ModuleID = 'main'
source_filename = "main"

@.cstr = private unnamed_addr constant [13 x i8] c"hello world!\00"

define i32 @main() {
entry:
  %tmp_call = call i32 (ptr, ...) @printf(ptr @.cstr)
  ret i32 0
}

declare i32 @printf(ptr, ...)
