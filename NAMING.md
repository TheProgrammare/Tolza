# Naming Convention (recommended)
Types : entity, component, role, system, enum, type, union, generic, named metacode

## Naming

| type | rule | e.g. |
|-|-|-|
| function, lambda | always verbal and on first word if possible | `find_player` `add_health` |
| variable, member | `<englober>_<attribute>_<capacity>_<parameter>` | `car_speed_max`<br> `player_health_max_color` |
| component | decrypt a capacity<br> verbal with `able` suffix | `CPrintable` |
| role | decrypt a list of capacity so a role<br> adjective on the main behaviour | `RMerchant` (`CViable` `CMovable` `CContainable` `CValuable` `CInteractable`) |
| system | decrypt a capacity application<br> verbal with `able` suffix and the application | `viable_add_health` |
| system with element | decrypt a compoent or role, so the action name | `w_movement` (CMovable), `w_merchant` (`RMerchant`)
| generic | decrypt a filter, so a capacity or a nomenclature<br> verbal with `able` suffix or nomenclature | `GMovable` `GIntegral` |

>Note: naturally, you can encounter some exceptions or unexistent word, pay attention to the clarity first

## Formatation

| type | format | e.g. |
|-|-|-|
| namespace | PascalCase | `CityEurope` |
| type alias | PascalCase | `Map3Array` |
| entity | PascalCase, prefix `T` | `TAnimal` |                              
| component | PascalCase, prefix `C`<br> suffix `able` | `CMovable` |
| component member | snake_case | `max_health` |
| role | PascalCase<br> prefix `R` | `REnnemy` |
| enumerator | PascalCase<br> prefix `E` | `ESpecies` |
| union | PascalCase<br> prefix `U` | `UView` |
| generic | PascalCase<br> prefix `G` suffix `able`<br> or a nomenclature word | `GPrintable` `GIntegral` |
| reusable metacode | PascalCase<br> prefix `M` | `MStaticConst` |
| function | snake_case | `math_foo` |
| system | snake_case<br> prefix major component used with `able` prefix and the main purpose of the system | `movable_jump`<br> (CMovable is the major component to jump any entity, and the main purpose of the system is to jump) |
| variable | snake_case | `health_max` |
| local variable | snake_case<br> prefix `_` | `_temp_vector` |
| parameter | snake_case<br> prefix `p_` | `p_first` |
| lambda | snake_case<br> prefix `_` | `_precalculate_array` |
| lambda local variable | snake_case<br> prefix `_` | `_temp_vector` |
| lambda parameter | snake_case<br> prefix `p_` | `p_temp_vector` |
| constant | snake_case<br> prefix `k_` | `k_player_max` |
| global | CAPITAL | `COMPILATION_ARGS` |
| generic typename | one letter uppercase | `T ` `U ` `V ` |
| metacode placeholder | one letter uppercase, prefix `_` | `_T` `_U` `_V` |

## abbreviation and truncation

use this for too long names and for most used local variable or temporary variables

| type | rule | e.g. |
|-|-|-|
| temporary | prefix `tmp_`<br> if copy of var, peek first letter on syllab<br> or first letters (for one word)<br> or first words letters, or standard convention (`lhs`, `rhs`, ...) | `temp_buff` `temp_lhs` |
| most used variable | peek first letter on syllab<br> or first letters (for one word)<br> or first words letters<br> or standard convention (`lhs`, `rhs`, ...) | `expansion_meta->placeholders_pos` : `phs_pos` |

# Mangling 

## Primitives and Types

- optional: `[]`
- name: `<size><name>`

| type | mangling |
|------|----------|
| bool | `b` |
| i8 - i128 - isize | `i8` - `i128` `isz` |
| u8 - u128 - isize| `u8` - `u128` `usz` |
| b8 - b128 - bsize | `b8` `b128` `bsz` |
| f32 - f64 - fsize | `f32` `f64` `fsz` |
| ascii | `aii` |
| utf32 | `utf` |
| ptr  | `p`  |
| sptr | `sp` |
| uptr | `wp` |
| deci | `d_<integral_size>_<decimal_size>` ->   `102.56d`->    `d_3_2` |
| udeci | `ud_<integral_size>_<decimal_size>`  ->  `50.2555ud` ->  `ud_2_4` |
| static array | `arr<size>_<type>` |
| dynamic array | `list_<type>` |
| tuple |`tu<size>_<type>`
| string | `str` |
| range | `rng_<typern>` |
| iterator | `iter_<type>` |
| slice | `sli_<type>` |
| function | `fn<param_size>_<param>_<return>` |

## Items

| type | pattern | mangling |
|-|-|-|
| velox mangling symbol |                         `_V_` |                                               |
| <path> |                                `[<module>]<namespaces>` |
| <gentys> instance | `<gentys>`: `G<count>_<types>` |           `G2_str_i32` |
| <gentys> symbol | `<gentys>`: `S<count>_<types>` |           `S2_T_U` |
| namespace       | `[<module>][<namespace>]<name>[<namespace>]` |  `_v_6Forest5Trees` |
| local var         | `[<path__>]loc_<name>[<_gentys>]` | `_V_6Forest__loc_5Count` |
| global var       | `[<path__>]glo_<name>[<_gentys>]` | `_V_6Forest__glo_7MAX_POP` |
| function         | `[<path__>]fn_<name>[<_gentys>]` | `_V_6Forest__fn_10new_forest` |
| function type   |   `[<path__>]fn<param_size>_<param>_<return>` |  `_V_6Forest__fn2_i8_f32_tu2_f64_b` |
| typealias        | `[<path__>]ty_<name>` | `_V_6Forest__ty_6Health` |
| system          |       `[<path__>]sy_<name>` | `_V_6Forest__sy_4Move` |
| component       |       `[<path__>]cp_<name>` | `_V_6Forest__cp_8Position` |
| entity         | `[<path__>]et_<name>[<_gentys>]` | `_V_6Forest__et_6Animal` |
| entity component |     `<entity__>cp_<name>` | `_V_6Forest__et_6Animal_cp_5Speed ` |
| entity constructor |   `<entity__>nw_<types>` | `_V_6Forest__et_6Animal_nw_str_u16` |
| enum           | `[<path__>]en_<name>` | `_V_6Forest__en_4Race` |
| enum elem     |  `[<path__>]en_<name>` | `_V_6Forest__en_4Race_3Dog` |
| flag           | `[<path__>]fg_<name>` | `_V_6Forest__fg_7Weather` |
| role           | `[<path__>]rl_<name>` | `_V_6Forest__rl_8Moveable` |
| generic       | `[<path__>]gn_<name>` | `_V_6Forest__gn_9Life_From` |
| metacode       | `[<path__>]mc_<name>` | `_V_6Forest__mc_10ConstAsync` |
