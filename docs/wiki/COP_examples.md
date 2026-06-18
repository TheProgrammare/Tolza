# Examples

## Game entity system

store to a module
```
mod game::entity_system {
```
define some base data
```
  comp Position { x: fsize = 0, y: fsize = 0, z: fsize = 0 }
  comp Velocity { speed: fsize = 0.0f }
  comp Vitality { health: fsize = 0.0f, stamina: fsize = 0.0f, mana: fsize = 0.0f, max_mana: fsize = 0.0f }
  comp Combat {strength: fsize = 0.0f, protection: fsize = 0.0f}
```

use a specific implementation for some components logics
```
  impl Velocity::calculate_speed(mut pos1: Position, mut pos2: Position) {
    self.speed = ((pos1.x - pos2.x) + (pos1.y - pos2.y) + (pos1.z - pos2.z)) / 3
  }
```

define some roles to reuse them or show a (packaged components) logic independent of any entity context
```
  role Movable {Position, Velocity,}
  role Fightable {Vitality, Combat,}
```

define entities with a composition and some default values
```
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
```

define systems designed to execute some pertinent behaviour for any entities with compatible composition

Move is applied for this set of composition : (Positon and velocity) or (Position)
```
  sys move(x: fsize = 0, y: fsize = 0, z: fsize = 0) {
    Position(p) + Velocity(v) => {
      v.calculate_speed(p, Position{.x= x, .y= y, .z= z}) // implementation called
      fallthrough
    }
    Position(p) => {
      p.x += x
      p.y += y
      p.z += z
    }
  }
```

complex system composition graph with some specific design 
```
  sys apply_damage(ref combat: Combat) {
    Vitality(v) => {
      v.health -= combat.strength
      fallthrough
    }
    Vitality(v) + Combat(c) + Velocity(vel) => { // micro optimization to avoid double call on is_dead()
      if self->is_dead() { // self is the entity, OK because vitality binded in context 
        c.damage = 0
        c.protection = 0
        vel.speed = 0
      }
      return // not useful, no fallthrough by default
    }
    Vitality(v) + Combat(c) => {
      if self->is_dead() { // self is the entity, OK because vitality binded in context 
        c.damage = 0
        c.protection = 0
      }
      fallthrough
    }
    // theorically unreacheable with the micro optimization before
    Vitality(v) + Velocity(vel) => {
      if self->is_dead() { // idem
        vel.speed = 0
      }
    }
  }
```

This system is better than a implementation on Vitality component because "is_dead" is useful in entity context
```
  sys is_dead() {
    Vitality(v) => {
      return v.health <= 0
    }
  }
```

logic code permitted
```
  fn main() {
    // the player and NPC move to the same direction
    var my_player: Player
    my_player->move(1000, 10, 0)
    var ennemy: NPC
    ennemy->move(1000, 10, 0)

    // some combat !
    my_player->apply_damage(ennemy) // ennemy deals 5 damages to my_player
    ennemy->apply_damage(my_player) // my_player deals 10 damages to ennemy
    // fight results
    if my_player->is_dead() => println("Player is dead")
    if ennemy->is_dead() => println("Ennemy is dead")

    // what if is_dead was a implementation on Vitality :
    // if my_player@Vitality.is_dead() => ...
    // if ennemy@Vitality.is_dead() => ...
  }

} // end mod
```


