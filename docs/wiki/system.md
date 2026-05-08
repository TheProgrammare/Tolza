# System
A system is a static match case based only on the evaluation of entity components.<br>
A system guarantees that any entity allowed to execute it will follow at least one valid case path derived from its composition, case path evaluation resolved at compile time.
```
sys name<gen_args>(<params>) {...}
```

Systems are callable only by an entity instance by the instruction `my_entity::>my_system()`
```
var my_player = Player::{CId.name = "Marc"}
my_player::>Jump(100)
```

> Note: if none match occurs, a compilation error occurs

To declare a composition, use a match composition evaluator `CCompType(binding_name)`

Match composition e.g.
```
CCompType1(c1) + CCompType2(c2) + ... => {...}
```
Explanation: the compiler will check if the entity have the specified composition.<br>
The components fileds are accessible by the binding name `c1` `c2`

> Note: A `role` can be used in a composition evaluator, his components fields stay accessible by the binding (so components fields in role are accessibles)  

Like `match` statement, a default behaviour can be specified `_ => ...`
> Note: But the system will be executable by any entity

full entity definition e.g.
```
sys name<generic_parameters>(parameters) {
  CType1(ct1) + CType2(ct2) => { // case A
    call(ct1, param..., ct2)
    call(ct2)
  }
  Role_Type(rt) => { // case B
    call(rt)
    call(rt.CType1, rt.CType3)
    return
  }
  _ => call() // case C
}
```
Flow explanation :   
- Entity have A + B + C composition -> run A + B and stop (return)
- Entity have A + C composition -> run A + C and stop (no more statement)
- Entity have A composition -> run A and stop (no other compatible statement)
- Entity have C composition -> run C and stop (no more statement)
- Entity have B + C composition -> run B and stop (return)

> Note: It's possible to define any instructions, but it's not recommended to keep the Single Responsibility Principle. Systems must be considered like a function dispatcher on entities  


## Systems Simplification
```
sys move(copy new_pos: xyz_pos, copy vel: f32) {
  CPosition(pos) + CPhysic(phy) {
    move_entity(pos, phy, new_pos, vel) 
  }
}
```
calling
```
player::>move((10.0, 20.0, 30.0), 5.0)
```

