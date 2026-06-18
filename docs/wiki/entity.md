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
  use Job // ERROR
}
```

# Unique Components inside entities 
Components members are a unique set inside entities
- NOT RECOMMENDED: SEMANTIC COMPOSITION VIOLATION: To use two same components inside one entity, create a custom component with all fields needed,</br>or use a entity reference inside a component to have two components simulated
- RECOMMENDED: Use explicit component behaviour or sub entities designed to be used with another entity context, or inside a contextual component for specific entity

e.g. Good design
```
// public std lib comp CBuffer<T> {...} // generic buffer data component
// private std lib entity List<T> {...} // generic buffer data formalization (entity with composition and all operations)
// public std lib comp CList<T> { mut list: List<T> } // List entity component wrapper

comp CatalogueItem { name: str= "", price: f32= 0 } // user component
 
entity Catalogue {
  use CList<CatalogueItem>
}

```

## Literal Entity

| meber type | usage syntax | note |
|-|-|-|
| call native constructor | `var cat = Cat{ CAnimal{.name= "Ted", .age= 2 }}` |  not recommended |
| call native constructor | `var cat = Cat{ CAnimal{.name= "Ted", .age= 2 }}` |  not recommended |
| call custom constructor | `var cat = Cat::new("Ted", 2)` | from static implementation on Cat type|

## Component Access
Components are statically defined inside entities and roles interface, to access to an component inside an entity or role use the at keyword `@`

this keyword is a sugar syntax for the compiler to check at compilation time if it's a valid component query or a valid role query:
```
comp Position { x: f32 = 0, y: f32 = 0, z: f32 = 0 }
comp Velocity { vel: f32 = 0 }
comp AirVelocity { vel: f32 = 0 }
role Movable{ Position, Velocity }
role Flyable{ Position, Velocity, AirVelocity }

entity Player { use Position, use Velocity }

var p: Player
Player@Position = Position{.x= 10, .y= 10, .z= 0} // OK Position is a component of Player
Player@Movable@Velocity.vel = 10 // OK Movable is a compatible composition of Player, then Velocity is a composition of Movable
// can be directly Player@Velocity.vel = 10.0f
Player@Flyable@Position.z = 1000.0f  // ERROR AirVelocity is a incompatible composition of Player (dosen't have AirVelocity component)
```

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
