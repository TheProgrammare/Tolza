# Rule
A rule is a static match case based only on the evaluation of form facets.<br>
A rule guarantees that any form allowed to execute it will follow at least one valid case path derived from its composition, case path evaluation resolved at compile time.
```
rule name<gen_args>(<params>) {...}
```

Rules are callable only by an form instance by the instruction `my_form::>my_rule()`
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
Explanation: the compiler will check if the form have the specified composition.<br>
The facets fileds are accessible by the binding name `c1` `c2`

> Note: A `role` can be used in a composition evaluator, his facets fields stay accessible by the binding (so facets fields in role are accessibles)  

Like `match` statement, a default behaviour can be specified `_ => ...`
> Note: But the rule will be executable by any form

full form definition e.g.
```
rule name<generic_parameters>(parameters) {
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
- Form have A + B + C composition -> run A + B and stop (return)
- Form have A + C composition -> run A + C and stop (no more statement)
- Form have A composition -> run A and stop (no other compatible statement)
- Form have C composition -> run C and stop (no more statement)
- Form have B + C composition -> run B and stop (return)

> Note: It's possible to define any instructions, but it's not recommended to keep the Single Responsibility Principle. Rules must be considered like a function dispatcher on forms  


## Rules Simplification
```
rule move(copy new_pos: xyz_pos, copy vel: f32) {
  CPosition(pos) + CPhysic(phy) {
    move_form(pos, phy, new_pos, vel) 
  }
}
```
calling
```
player::>move((10.0, 20.0, 30.0), 5.0)
```

