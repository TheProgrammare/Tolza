# Reflexion

To get a code item info, use the prefix operator `@`, the type returned will be a builtin special metainfo according to the compiler ast nodes

To splice a info, use the prefix operator `\`, the compiler will investigate the metainfo and inject the target data

flags reflexion:
```tolza
flags FPermissions { Read, Write, Execute }
let first_flag = @FPermission.Flags[0]

fn main() {
  let current_permission = \first_flag b.and FPermission::Execute
  let val_metainfo = @current_permission.value

  print("all permissions: ")

  for fname : @FPermissions.flags_name {
    print(fname)
  }

  println()

  print("current permission: ${val_metainfo.dump()}")
} 
```
