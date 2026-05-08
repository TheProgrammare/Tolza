# Union
> Use `union` keyword to declare a union

Union is similar to an C union, so a non discriminant container who will consider the data as a type from the index in the input and output in the discretion of the user, no indexation or flag used to determine wich type is active.
```
union UNumber {
  i: isize,
  f: fsize,
}
```
> Note: Useful for C interop

