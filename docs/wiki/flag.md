# Flag
> Use `flag` keyword to declare a flag

Flag is a optimized named bits, max 255 elements, use only 8 bits
```
flag FFileMode {
  Read, Write, Read_Write, Lock
}
```
Explicit byte
```
flat FFileMode {
  Read => 0x0, Write => 0x1, Read_Write => 0x2, Lock => 0x3
}
```

> bit enum type can be explicit cast with bytes, integrals : `0x1 as FName` or `1 as FName` => return FName::elem2

