# Entity
> Use `entity` to declare a entity

Entites have a static composition of components who define his behaviour for systems and the accepted parameter arugment of component/role in fuctions.

Declaration:
```
entity name<gen_args> {...}
```

 ### Entity Members

| member | syntax | info | method | return |
|-|-|-|-|-|
| component | `use name { field: value }` | with default value | | |
| component | `use name` | default value from component | | |
| cast | `cast self as T { ... }` | cast entity to antoher type, reserved key `self` and `other` used, permit to use `my_var as T` | `const` | `T` |
| cast | `cast T as self { ... }` | cast entity from another type, reserved key `self` and `other` used, permit to use `my_val as Type(my_entity)` | `const` | `self` |
| constructor | `new(params) { ... }` | overloading possible, must returns the same entity type | `const` | `self` | 
| output | `output(ref output_mode: io::output::mode, args: io::arguments...) { ... }` | overloading output I/O operation | `const` | `u0` |
| open | `open(ref path: str, args: io::argument...) { ... }` | overloading open I/O operation |  | `self` |
| close | `close { ... }` | overloading close I/O operation |  | `self` |
| read | `read(ref read_mode: io::argument, args: io::argument...) -> T { ... }` | overloading read I/O operation |  | `T` |
| write | `write(args: io::arguments...) -> T { ... }` | overloading read I/O operation |  | `self` |
| copier* | `copy { ... }` | must returns the same entity type | `const` | `self` | 
| cloner** | `clone { ... }` | must returns the same entity type | `const` | `self` | 
| deleter*** | `del { ... }` | no parameter, reserved key `self` used | | |
| arithmetic operator | `op + { ... }` | all operators handled but type are restrictives | `mutable` only if with a operation assignation `+=` | if operation assignation: in-place modification, otherwise copy |
| comparison operator | `op == { ... }` | all operators handled `self` `other` are same type | `const` | `bool` |
| logical operator | `op and { ... }` | all operators handled `self` `other` are same type | `const` | `bool` |

>\* Without copier, the compiler will copy each components fileds, if ref/ptr/sptr fields -> call copy on type, copy ptr address, share pointer</br>
>** Without cloner, the compiler will clone each components fields, if ref/ptr/sptr fields -> call clone on type, new ptr address then call clone on type, share pointer</br>
>*** Without deleter, the compiler will delete each components fields, if ref/ptr/sptr fields -> call del on type, free ptr, decrement share pointer


examples:
```
comp Specie { name: Str = "" }
comp Position { x: f32 = 0, y: f32 = 0, z: f32 = 0 }
comp Job { name: Str = "no job" }

entity Animal {
  use Specie 
  use Position
  cast self to Human {
    var man = Human{ Specie.name= "Human", Position= self.Position }
    return man
  }
}

entity Human {
  use Specie
  use Position
  use Job
  cast self to Animal {
    var animal = Animal{ Specie.name= "monkey", Position= self.Position}
    return animal
  }
}

```

## Member Usage

| meber type | usage syntax | note |
|-|-|-|
| call native constructor | `var cat = Cat{ CAnimal.name = "Ted", CAnimal.age = 2 }` |  not recommended |
| call native constructor | `var cat = Cat{ CAnimal.name = "Ted", CAnimal.age = 2 }` |  not recommended |
| call custom constructor | `var cat = Cat::new("Ted", 2)` | clean constructor |

## Entity Operator Overloading
there is some restrictions in operator definition:

| operator | other term | return type | syntax |
|-|-|-|-|
| `==` `!=` `===` `!==` `<` `>` `<=` `>=` `is` | self entity type | `bool` | `op <operator> { ... }` |
| `+` `-` `*` `/` `%mod%` `%quo%` `%rem%` `**` `<<[0]` `<<[1]` `<<[a]` `<<[r]` `<<[rc]` `[0]>>` `[1]>>` `[a]>>` `[r]>>` `[rc]>>` | self entity type | self entity type copy | `op <operator> { ... }` |
| `+=` `-=` `*=` `/=` `%mod%=` `%quo%=` `%rem%=` `**=` | custom type T | self entity type ref | `op <operator> <T> { ... }` |
| `Iter` iterator | none | `Iter<T>` | `op Iter<T> { ... }` |
| `[]` index | none | index always i64, custom type U | `op [i] -> U { ... }` |
| `[..]` range | none | custome slice U | `op [r: ..] -> mut'[U]` |


