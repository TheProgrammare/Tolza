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
| component default | `use name { field: value }` | with default value | | |
| component | `use name` | default value from component | | |


examples:
```
comp Specie { name: str = "animal" }
comp Position { x: f32 = 0, y: f32 = 0, z: f32 = 0 }
comp Job { name: str = "no job" }

entity Animal {
  use Specie 
  use Position
}

entity Human {
  use Specie {.name= "animal"}
  use Position
  use Job
}
```

## Member Usage

| meber type | usage syntax | note |
|-|-|-|
| call native constructor | `var cat = Cat{ CAnimal{.name= "Ted", .age= 2 }}` |  not recommended |
| call native constructor | `var cat = Cat{ CAnimal{.name= "Ted", .age= 2 }}` |  not recommended |
| call custom constructor | `var cat = Cat::new("Ted", 2)` | from static implementation on Cat type|

## Entity Implementation
You can implement any function, operation, cast, predicat to an entity like any other native type

## Generics
Entities can be generics to be reused on specific types : 

```
comp Buffer<T> {data: ptr'T = nullptr, .size: usize = 0, .capacity: usize = 0 }

entity LinkedList<T> {
  use Buffer<T>
}
```

Or entities can fixed components generics for specific usage :
```
entity StringList {
  use Buffer<str>
}
```
