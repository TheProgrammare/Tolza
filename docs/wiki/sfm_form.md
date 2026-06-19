# Form
> Use `form` to declare a form

Entites have a static composition of facets who define his behaviour for rules and the accepted parameter arugment of facet/role in fuctions.

Declaration:
```
form name<gen_args> {...}
```

 ### Form Members

| member | syntax | info |
|-|-|-|
| facet default override | `use name { field: value }` | with default value |
| facet | `use name` | default value from facet | 


examples:
```
facet Specie { name: str = "animal" }
facet Position { x: f32 = 0, y: f32 = 0, z: f32 = 0 }
facet Job { name: str = "no job" }

form Animal {
  use Specie 
  use Position
}

form Human {
  use Specie {.name= "animal"}
  use Position
  use Job
  use Job // ERROR
}
```

# Unique Facets inside forms 
Facets members are a unique set inside forms
- NOT RECOMMENDED: SEMANTIC COMPOSITION VIOLATION: To use two same facets inside one form, create a custom facet with all fields needed,</br>or use a form reference inside a facet to have two facets simulated
- RECOMMENDED: Use explicit facet behaviour or sub forms designed to be used with another form context, or inside a contextual facet for specific form

e.g. Good design
```
mod game_form_data {
  facet Position { x: fsize = 0, y: fsize = 0, z: fsize = 0 }
  facet Velocity { speed: fsize = 0.0f }
  facet Vitality { health: fsize = 0.0f, stamina: fsize = 0.0f, mana: fsize = 0.0f, max_mana: fsize = 0.0f }
  facet Combat {strength: fsize = 0.0f, protection: fsize = 0.0f}

  extend Velocity::calculate_speed(mut pos1: Position, mut pos2: Position) {
    self.speed = ((pos1.x - pos2.x) + (pos1.y - pos2.y) + (pos1.z - pos2.z)) / 3
  }

  role Movable {Position, Velocity,}
  role Fightable {Vitality, Combat,}

  form Player {
    use Position,
    use Velocity,
    use Vitality{.health= 2.0f},
    use Combat {.strength= 10.0f},
  }

 form NPC {
   use Position,
   use Vitality{.health= 1.0f},
   use Combat {.strength= 5.0f},
 }

  rule move(x: fsize = 0, y: fsize = 0, z: fsize = 0) {
    Position(p) + Velocity(v) => {
      v.calculate_speed(p, Position{.x= x, .y= y, .z= z}) // extension called
      p->move(x, y, z) // rule called only on facet position
      return
    }
    Position(p) => {
      p.x += x
      p.y += y
      p.z += z
    }
  }

  rule apply_damage(ref combat: Combat) {
    Vitality(v) => {
      v.health -= combat.strength
    }
    Vitality(v) + Combat(c) => {
      if self->is_dead() { // self is the form, OK vitality binded 
        c.damage = 0
        c.protection = 0
      }
    }
    Vitality(v) + Velocity(vel) => {
      if self->is_dead() { // idem
        vel.speed = 0
      }
    }
  }

  rule is_dead() {
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

## Literal Form

| meber type | usage syntax | note |
|-|-|-|
| call native constructor | `var cat = Cat{ CAnimal{.name= "Ted", .age= 2 }}` |  not recommended |
| call native constructor | `var cat = Cat{ CAnimal{.name= "Ted", .age= 2 }}` |  not recommended |
| call custom constructor | `var cat = Cat::new("Ted", 2)` | from static extension on Cat type|

## Facet Access
Facets are statically defined inside forms and roles interface, to access to an facet inside an form or role use the at keyword `@`

this keyword is a sugar syntax for the compiler to check at compilation time if it's a valid facet query or a valid role query:
```
facet Position { x: f32 = 0, y: f32 = 0, z: f32 = 0 }
facet Velocity { vel: f32 = 0 }
facet AirVelocity { vel: f32 = 0 }
role Movable{ Position, Velocity }
role Flyable{ Position, Velocity, AirVelocity }

form Player { use Position, use Velocity }

var p: Player
Player@Position = Position{.x= 10, .y= 10, .z= 0} // OK Position is a facet of Player
Player@Movable@Velocity.vel = 10 // OK Movable is a compatible composition of Player, then Velocity is a composition of Movable
// can be directly Player@Velocity.vel = 10.0f
Player@Flyable@Position.z = 1000.0f  // ERROR AirVelocity is a incompatible composition of Player (dosen't have AirVelocity facet)
```

## Form Extension
You can extend any function, operation, cast, predicat to an form like any other native type

## Generics
Forms can be generics to be reused on specific types : 

```
facet Buffer<T> {data: ptr'T = nullptr, .size: usize = 0, .capacity: usize = 0 }

form LinkedList<T> {
  use Buffer<T>
}
```

Or forms can fixed facets generics for specific usage :
```
form StringList {
  use Buffer<str>
}
```
