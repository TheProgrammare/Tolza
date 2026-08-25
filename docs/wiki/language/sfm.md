# SFM Paradigm 
**(Structural Facet Model)**
SFM, short for Structural Facet Model, is a programming paradigm where forms are built through the static composition of facets, without inheritance, without polymorphism, and without dynamic facets
it's itended to be flexible with a deterministic syntax

An form is defined by composition; facets exist only within the form, and rules operate by recognizing that composition.
- facet is a contigous list of variables
- rule is a behaviour who contains block of statement according to facet combinaison


# SFM in Function Parameters
The SFM paradigm can be used in functions to simplify the code and avoid the generic boilerplate

Keep in mind that any facet and role type in parameter is values garantee.

# SFM in Extensions
Facets and Forms, Roles can be extended by any extension items (function, cast, op, etc...)

> Tips: extend setter and getter on facets, extend cast, op, constructors on forms, keep forms behaviours for rules

# SFM in Generics
Facets and Forms can be generic, have generics members, use generic type, fixed generic types
