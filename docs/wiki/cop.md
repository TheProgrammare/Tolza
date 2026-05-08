# COP Paradigm 
**(Compositional Oriented Programming)**
COP, short for Compositional Oriented Programming, is a programming paradigm where entities are built through the static composition of components, without inheritance, without polymorphism, and without dynamic components
it's itended to be flexible with a deterministic syntax

An entity is defined by composition; components exist only within the entity, and systems operate by recognizing that composition.
- component is a contigous list of variables
- system is a behaviour who contains block of statement according to component combinaison


# COP in Function Parameters
The COP paradigm can be used in functions to simplify the code and avoid the generic boilerplate

Keep in mind that any component and role type in parameter is values garantee.



# COP and Function Generics and Function Calls
> Usage is the same as rust generic call type args
> It's possible tu specify a generic in the args

generic function call (turbofish used in calling)
```
<name>::'<'type[, type, ...]'>'([<parameters>])
```
e.g.
```
add::<GNumeric, i32>(10, 20)
```

entity with generic fixed component:
```
entity TCity {
  use CArray<THouse>
}
```

entity with generic component:
```
entity TAnimal<T> {
  use CLife<T>
}
```

component with generic fixed:
```
comp CNames { names: TArray<str> }
```

component with generic:
```
comp CLife<T: GDecimalScaled, U> { render: U, metabolism: Map<str, T> }
```
