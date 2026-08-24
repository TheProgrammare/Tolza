# Examples

## Game Entity System

store to a module
```
mod game::system::entity {
```
define some base data
```
  facet Position { x: fsize = 0, y: fsize = 0, z: fsize = 0 }
  facet Velocity { speed: fsize = 0.0f }
  facet Vitality { health: fsize = 0.0f, stamina: fsize = 0.0f, mana: fsize = 0.0f, max_mana: fsize = 0.0f }
  facet Combat {strength: fsize = 0.0f, protection: fsize = 0.0f}
```

use a specific extension for some facets logics
```
  extend Velocity fn calculate_speed(mut pos1: Position, mut pos2: Position) {
    self.speed = ((pos1.x - pos2.x) + (pos1.y - pos2.y) + (pos1.z - pos2.z)) / 3
  }
```

define some roles to reuse them or show a (packaged facets) logic independent of any form context
```
  role Movable {Position, Velocity,}
  role Fightable {Vitality, Combat,}
```

define forms with a composition and some default values
```
  form Entity {
    use Position,
    use Velocity,
    use Vitality{.health= 2.0f},
    use Combat {.strength= 10.0f},
  }
```

define rules designed to execute some pertinent behaviour for any forms with compatible composition

Move is applied for this set of composition : (Positon and velocity) or (Position)
```
  rule move(x: fsize = 0, y: fsize = 0, z: fsize = 0) {
    Position(p) + Velocity(v) => {
      v.calculate_speed(p, Position{.x= x, .y= y, .z= z}) // extension called
      fallthrough
    }
    Position(p) => {
      p.x += x
      p.y += y
      p.z += z
    }
  }
```

complex rule composition graph with some specific design 
```
  rule apply_damage(ref combat: Combat) {
    Vitality(v) => {
      v.health -= combat.strength
      fallthrough
    }
    Vitality(v) + Combat(c) + Velocity(vel) => { // micro optimization to avoid double call on is_dead()
      if self->is_dead() { // self is the form, OK because vitality binded in context 
        c.damage = 0
        c.protection = 0
        vel.speed = 0
      }
      return // not useful, no fallthrough by default
    }
    Vitality(v) + Combat(c) => {
      if self->is_dead() { // self is the form, OK because vitality binded in context 
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

This rule is better than a extension on Vitality facet because "is_dead" is useful in form context
```
  rule is_dead() {
    Vitality(v) => {
      return v.health <= 0
    }
  }
```

logic code permitted
```
  fn main() {
    // the player and ennemy move to the same direction
    var my_player: Entity
    my_player->move(1000, 10, 0)
    var ennemy: Entity
    ennemy->move(1000, 10, 0)

    // some combat !
    my_player->apply_damage(ennemy) // ennemy deals 5 damages to my_player
    ennemy->apply_damage(my_player) // my_player deals 10 damages to ennemy
    // fight results
    if my_player->is_dead() => println("Player is dead")
    if ennemy->is_dead() => println("Ennemy is dead")

    // what if is_dead was a extension on Vitality :
    // if my_player@Vitality.is_dead() => ...
    // if ennemy@Vitality.is_dead() => ...
  }

} // end mod
```


