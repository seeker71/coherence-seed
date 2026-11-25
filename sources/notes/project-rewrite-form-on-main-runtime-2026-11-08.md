---
name: project-rewrite-form-on-main-runtime-2026-11-08
description: "The codex-4523 runtime rewrite landed (7ed6ad7b2) as its Form half on main's fast C; its own 10k-line evaluator stays on claude/rewrite-home. Seams and the float_leaf numbering split."
metadata:
  node_type: memory
  type: project
  originSessionId: 39d16a37-d4e8-4631-9ac0-105916d511fd
  modified: 2026-11-08T14:28:35.570Z
---

2026-11-08: the rewrite's evaluator was ~40x slower than main's, so its Form work landed on main's C instead
(7ed6ad7b2; gates pass=524287, 287 s vs main's 266 s; warm bands at or under main's times). Main's C gained only:
float_leaf 60 (scoped file leases for native-posix.bml), 62/63 (int64/float64 <-> 8 LE bytes), and record_get
answering nothing (not 0) for an absent field.

**Why:** Urs asked for the rewrite without a speed regression and with minimal C change.

**How to apply:**
- float_leaf mode numbers DIFFER between the rewrite and main: on main 34-48 are host doors (35 = host_chmod).
  Any BML carried from claude/rewrite-home that calls float_leaf(34/35/59/61) must be renumbered or it silently
  hits the wrong door. 41/42/58 agree.
- Rewrite BML habits main's cursor/compiler refuse: `def f() = ... { ... }` without a trailing `;`; a primitive
  passed as a value (`map(byte_to_str, xs)`, `metal_buf_alloc` as an argument) — wrap it in a lambda.
- The BML field (native-field.bml), kernel-ast.bml checker and the field-only publisher stay on the rewrite
  branch (they need door 61 and the rewrite evaluator).
- Find seams fast: probe each changed file with `import core.fk; import X; section [form.bml] {print_str("ok");}`,
  and delete stale `*.fkb`/`*.lowfk` caches first; a refused image is cached beside its source.

Related: [[reference-history-rewrite-and-the-cut]], [[feedback-native-substrate-before-local-model]]
