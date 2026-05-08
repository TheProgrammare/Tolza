# Component
> Use `comp` to declare a component

is a contigous list of variables, default values are required to avoid any undetermined value

Declaration:
```
comp name<gen_args> {...}
```

Component Field:
```
field_name: type = default_value,
```

Literal:
```
comp_name{ .field_name1= init_value, .field_name2= init_value }
```

Literal void:
```
comp_name{.}
```
Avoid any type abiguity

> optional metadata annotation for component members</br>
- no default value requied `# no default` really unsafe

A component used in function parameter is a guarantee of the presence of the values as long as the entity have the component expected.

## Component Field entity typed
A field is a primtive type or an entity, no inline component/entity field are accepted. To keep the composition clean

Components can't handle a nested entity/component for contigous memory sanity and avoid infinitive structures loop. When an entity is specified, it's always a reference to an entity instance.

There is differents usage mod of entity who define the pointer type.

Filed entity/component typed mode 
| mode | syntax | behaviour |
|-|-|-|
| pointer | `ref name: EntityT/CompT` | non-nullable, reference, same lifetime |
| pointer | `mut name: EntityT/CompT` | non-nullable, mutable reference, same lifetime |
| pointer | `name: ptr'EntityT/CompT` | nullable, raw pointer, independent lifetime |
| pointer | `name: sptr'EntityT/CompT` | nullable, reference counted, lifetime shared |

## Component Parameters
CPosition and CPhysic guarantee the members values for the entity calling
```
type xyz_pos = (x: f32, y: f32, z: f32)

fn move_entity(mut pos: CPosition, mut phy: CPhysic, copy new_pos: xyz_pos, copy vel: f32) {
  pos.x copy= new_pos.x
  pos.y copy= new_pos.y
  pos.z copy= new_pos.z
  phy.vel copy= vel
}
```
calling
```
var player = Player{CPosition.x= 100, CPosition.y= 100, CPosition.z= 100}
move_entity(player, player, (10.0, 20.0, 30.0), 5.0)
```
note a role or a system can simplify the function declaration and call:


# Role
> Use `role` to declare a role
 
roles are a package of components to check if entities have some components
useful for simple generic functions or system case !

a role cannot be used in an entity composition, it's a generic/system_case/parameter guarantee shortcut
```
role name { components, ... }
```


## Role Parameters
```
role RMovable { CPosition, CPhysic }
fn move_entity_role(mut mov: RMovable, copy new_pos: xyz_pos, copy vel: f32) {
  mov.CPosition.x copy= new_pos.x
  mov.CPosition.y copy= new_pos.y
  mov.CPosition.z copy= new_pos.z
  mov.CPhysic.vel copy= vel
}
```
calling
```
move_entity_role(player, (10.0, 20.0, 30.0), 5.0)
```


