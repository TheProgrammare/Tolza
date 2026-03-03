# Pull Request

**Author:** [Your Name]  
**Date:** [YYYY-MM-DD]

## 1. Summary
Briefly describe the purpose of this pull request:
- What problem does it solve?
- What feature or behavior is changed or added?

## 2. Scope
Type of change:
- [ ] Bug fix
- [ ] Refactoring
- [ ] Performance improvement
- [ ] Documentation update
- [ ] New feature (requires a Design Proposal)
- [ ] Other (explain)

This PR is:
- [ ] Atomic (single concept)
- [ ] Multi-file but single domain
- [ ] Cross-domain (justify below)

Justification if multi-domain:
> Explain why splitting into multiple PRs was not possible.

## 3. Technical Details
For **each addition or modification**, provide clear explanation:

- Function / method: role and location
- Type / structure: what is added or modified
- Operation: what the function/type does or changes
- Design choices / important decisions
- Ownership / memory model (stack / smart pointers / raw pointers)
- Thread-safety considerations (if applicable)
- Error handling strategy

Example:
> Function `parse_expression` (AST): parses an expression node, uses `unique_ptr` for ownership, adds an overflow check.

## 4. LLVM / IR Impact (if applicable)
- Does this modify IR generation?
- Does it affect optimization passes?
- Is it compatible with LLVM 19?
- Any potential regression risk?

## 5. Safety & Correctness Checklist
- [ ] Compiles with Clang 19
- [ ] No new warnings
- [ ] No undefined behavior introduced
- [ ] No memory leaks
- [ ] No unjustified reinterpret_cast
- [ ] No dangerous macros
- [ ] Code formatted with `.clang-format`
- [ ] Tests added or updated

## 6. Testing
- Unit tests
- Manual tests
- Edge cases covered
- Error cases covered

## 7. Performance Impact (if applicable)
- Measured impact?
- Benchmark results?
- Justification if performance-sensitive code was modified

## 8. Additional Notes
Any additional information for reviewers.
