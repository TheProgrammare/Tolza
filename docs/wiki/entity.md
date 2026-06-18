# Entity
> Use `entity` to declare a entity

Entites have a static composition of components who define his behaviour for systems and the accepted parameter arugment of component/role in fuctions.

Declaration:
```
entity name<gen_args> {...}
```

 ### Entity Members

| member | syntax | info |
|-|-|-|
| component default override | `use name { field: value }` | with default value |
| component | `use name` | default value from component | 


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
mod game_entity_data {
  comp Position { x: fsize = 0, y: fsize = 0, z: fsize = 0 }
  comp Velocity { speed: fsize = 0.0f }
  comp Vitality { health: fsize = 0.0f, stamina: fsize = 0.0f, mana: fsize = 0.0f, max_mana: fsize = 0.0f }
  comp Combat {strength: fsize = 0.0f, protection: fsize = 0.0f}

  impl Velocity::calculate_speed(mut pos1: Position, mut pos2: Position) {
    self.speed = ((pos1.x - pos2.x) + (pos1.y - pos2.y) + (pos1.z - pos2.z)) / 3
  }

  role Movable {Position, Velocity,}
  role Fightable {Vitality, Combat,}

  entity Player {
    use Position,
    use Velocity,
    use Vitality{.health= 2.0f},
    use Combat {.strength= 10.0f},
  }

 entity NPC {
   use Position,
   use Vitality{.health= 1.0f},
   use Combat {.strength= 5.0f},
 }

  sys move(x: fsize = 0, y: fsize = 0, z: fsize = 0) {
    Position(p) + Velocity(v) => {
      v.calculate_speed(p, Position{.x= x, .y= y, .z= z}) // implementation called
      p->move(x, y, z) // system called only on component position
      return
    }
    Position(p) => {
      p.x += x
      p.y += y
      p.z += z
    }
  }

  sys apply_damage(ref combat: Combat) {
    Vitality(v) => {
      v.health -= combat.strength
    }
  }

  sys is_dead() {
    Vitality(v) => {
      return v.health <= 0
    }
  }

  var my_player: Player
  my_player->move(1000, 10, 0)
  var ennemy: NPC
  ennemy->move(1000, 10, 0)
  my_player->apply_damage(ennemy) // ennemy deals 5 damage to my_player
  ennemy->apply_damage(my_player) // my_player deals 10 damage to ennemy
  if my_player->is_dead() => println("Player is dead")
  if ennemy->is_dead() => println("Ennemy is dead")
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
