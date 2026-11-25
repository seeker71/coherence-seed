---
name: reference-fkwu-do-keeps-let-siblings-leak
description: "One let rule four-way: nested do lexical, unit top-level do flat, a let answers its value, a closure keeps its binding (f5ebbf502, e1fe2f0b3, aee227573); fkwu caps captures at 8"
metadata: 
  node_type: memory
  type: reference
  originSessionId: d7d665d0-ee49-4194-b96b-f85d64a45006
  modified: 2026-09-15T08:32:24.121Z
---

`(do (let t 0) (do (let t 5)) t)` reads 0 on all four kernels since f5ebbf502 (witnessed by
form/form-stdlib/tests/let-scope-band.fk, 511 four-way). Go, Rust and TS open a child frame at a
do's first let or defn; a let that is not a do's own form (an if arm, an argument) binds nothing
past itself; the unit's top-level do reads flat through walk_unit / walkUnit; the readers mark an
explicit single-do root apart from the implicit do around several forms.

Since e1fe2f0b3 and aee227573 (landed 2026-09-15, receipt
receipts/2026-09-15-a-let-answers-its-value.md) the two older differences are closed four-way:
- a let answers the value it binds on fkwu too (a let ending its do reads its own slot, tag 110;
  `fk_parse_do`, `fk_sparse`, `fk_parse_top_do_value`);
- a closure keeps the bindings in scope where it was made on Go, Rust and TS: a let after a
  closure binds in a fresh frame, unit-level defns carry a view frame stamped with the unit
  version, and a unit let that rebinds a name keeps history.
TS and Rust take tail calls in place, so string-join no longer holds every list alive (TS passed
48 GB before; origin's Rust 22 GB in 3 s). TS voices list/copy-budget past 2^28 copied elements.

Still differing: fkwu captures at most 8 names per function (FK_CLOSURE_CAP_MAX); a ninth is
declined with [closure-scope], rc 1 (an organ row since 0a95061ee), where the siblings answer.

**How to apply:** never reshape a caller's output around a kernel's scoping; mutation's home is
`native-mutable-local-frame` in `grammars/bml-native-north-star.form`. A unit let could rebind in a
function's own frame only through walk_recipe_here, which left the siblings with 2622299f6.

Related: [[feedback-only-runtime-is-fkwu-fix-the-language]] [[reference-sibling-kernels-read-like-fkwu]] [[reference-landing-on-a-busy-main]]
