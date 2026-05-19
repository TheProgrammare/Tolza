# Design Proposal (RFC) Example

**Author:** Foz Florian
**Date:** 2026-05-19  
**Domain:**
- [X] Compiler
- [ ] Toolchain
- [ ] VS Code plugin

## 1. Title
Explicit in out parameter proposal

## 2. Motivation
- What problem does this solve?
-- Les paramètres mutables sont par nature contraire à la logique de paramètre qui sont principalement entrées modifiant le résultat de sortie d'une fonction
-- Les conventions modernes tendent vers une logique stricte de paramètre comme uniquement une interface d'entrée et pas de sortie
- Why is the current design insufficient?
-- les lifetimes et les effets de bords sont trop implicites et sont la source majeur de bug runtime
- Who benefits from this change?
-- everyone !

## 3. Background
Relevant context:
- Current behavior
-- les paramètres peuvent être mutable et donc violé la convention d'interface d'entrée des fonctions
- Architectural constraints
- Interaction with LLVM 19
-- déjà logique proche avec le SSA, mais usage obligé des pointeurs de fonction
- Components affected (parser, AST, IR, runtime, etc.)
-- parser, ast

## 4. Proposed Design

### 4.1 High-Level Overview
Grande visibilité des flux d'entrée et sortie des fonctions dans leur signature et dans leurs appels

### 4.2 Syntax Changes (if applicable)
ancien design
```
// def
fn getline(addr line: c_str, mut size: usize, mut file: FILE) -> isize
// call
let r = getline(line, size, file)
```
nouveau design
```
// def
fn getline(ref line: c_str, ref size: usize, ref file: FILE) -> (isize, 'line, 'ref, 'size)
// call
let r = getline(my_line, my_size, my_file)->(my_line, my_size, my_file)
// alt call
let r = getline(mut'my_line, mut'my_size, mut'my_file)
```

### 4.3 Semantic Rules
- Type rules
-- l'annotation ' en retour indique un binding sur paramètre et donc interprêter l'id comme un id et non un type
- Scope rules
- Lifetime rules
-- les binding sont la poursuite de la durée de vie du paramètre
- Multithreading impact
-- permet de rendre explicite les effets de bords

### 4.4 IR / Code Generation Impact
- Changes in IR generation
-- nothing or better optimiztaion
- New instructions or passes
- Optimization considerations
-- better lifetime manage

## 5. Alternatives Considered
- List alternative approaches
- Explain why they were rejected

## 6. Risks & Trade-offs
- Performance impact
- Increased complexity
- Backward compatibility
- Maintenance cost

## 7. Implementation Plan
- Parser updates
- AST modifications
- IR layer updates
- Runtime changes
- Tests
- Documentation

## 8. Backward Compatibility
- Breaking changes?
- Migration strategy if required

## 9. Open Questions
Unresolved design questions for discussion.
