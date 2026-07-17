# Facet
> Use `facet` to declare a facet

is a contigous list of variables, default values are required to avoid any undetermined value

Declaration:
```
facet name<gen_args> {...}
```

Facet Field:
```
field_name: type = default_value,
```

Literal:
```
facet_name{ .field_name1= init_value, .field_name2= init_value }
```

Literal void:
```
facet_name{.}
```
Avoid any type abiguity

> optional metadata annotation for facet members</br>
- no default value requied `# no default` really unsafe

A facet used in function parameter or extended is a guarantee of the presence of the values (or on entitity as long as the form have the facet expected).

## Facet Field form typed
A field is a primtive type or an form reference, no inline facet/form field are accepted. To keep the composition clean

Facets can't handle a nested form/facet value for contigous memory sanity and avoid infinitive structures loop. When an form is specified, it's always a reference to an form instance.

There is differents usage mod of form who define the pointer type.

Filed form/facet typed mode 
| mode | syntax | behaviour |
|-|-|-|
| pointer | `ref name: FormT/CompT` | non-nullable, reference, same lifetime |
| pointer | `mut name: FormT/CompT` | non-nullable, mutable reference, same lifetime |
| pointer | `name: ptr'FormT/CompT` | nullable, raw pointer, independent lifetime |
| pointer | `name: sptr'FormT/CompT` | nullable, reference counted, lifetime shared |

> Tips: extend del on facets to manage pointer lifetime on form type field

## Facet Parameters
CPosition and CPhysic guarantee the members values for the form calling
```
type xyz_pos = (x: f32, y: f32, z: f32)

fn move_form(mut pos: CPosition, mut phy: CPhysic, copy new_pos: xyz_pos, copy vel: f32) {
  pos.x copy= new_pos.x
  pos.y copy= new_pos.y
  pos.z copy= new_pos.z
  phy.vel copy= vel
}
```
calling
```
var player = Player{CPosition.x= 100, CPosition.y= 100, CPosition.z= 100}
move_form(player, player, (10.0, 20.0, 30.0), 5.0)
```
note a role or a rule can simplify the function declaration and call:


# View
> Use `view` to declare a view
 
view are a package of facets to check if forms have some facets
useful for simple generic functions or rule case !

a view cannot be used in an form composition, it's a generic/rule_case/parameter guarantee shortcut
```
view name { facets, ... }
```


## View Parameters
```
View vMovable { CPosition, CPhysic }
extend vMovable::move(mut self, ref new_pos: xyz_pos, copy vel: f32) {
  self.CPosition = CPosition{.x= new_pos.x, .y= new_pos.y, .z= new_pos.z}
  self.CPhysic.vel = vel
}

form Car { use CPosition, use CPhysic }

var my_car: Car
my_car@vMovable.move({.x= 10}, 100)
```

> Tips: use views to extend some cross-facet behaviour without specific form context

calling
```
player.move((10.0, 20.0, 30.0), 5.0) // will check if player is vMovable compile time compatible
vMovable::move(player, (10.0, 20.0, 30.0), 5.0) // explicit move call from vMovable extension, will check if player is vMovable compile time compatible
```


