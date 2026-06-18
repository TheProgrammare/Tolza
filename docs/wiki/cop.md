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

# COP in Implementations
Components and Entities, Roles can be implemented by any implementation items (function, cast, op, etc...)

> Tips: implement setter and getter on components, implement cast, op, constructors on entities, keep entities behaviours for systems

# COP in Generics
Components and Entities can be generic, have generics members, use generic type, fixed generic types
