---
name: bml-grammar-and-no-run-lower-check
description: The live BML surface is grammars/form-bml.fk (not bml.fk); how to prove a .bml still lowers without running it; duplicate def names across a prelude closure hang or stray-paren
metadata:
  node_type: memory
  type: reference
  originSessionId: f2d2d90b-8dbd-4bdf-8bb6-254813d8bd0f
  modified: 2026-10-02T03:15:23.395Z
---

The grammar fkwu really uses for `.bml` is `form/form-stdlib/grammars/form-bml.fk` (cursor rules, `rule3`, `t-emit`, `p-kw`), lowered by `form-bml-lower.fk` and `source-compiler.fk`. `form/form-stdlib/bml.fk` is an older Java-ish grammar that `bml-band` still reads. `form-bml-intrinsics` in form-bml.fk maps a source word to an existing Form owner with a fixed arity. A captured name spliced with `(t-splice "n")` is a string leaf, so a grammar rule can pass an identifier as a string constant with no new builder.

**No-run lowering proof** for a file that drives a live flow (night walk, planner):
`printf 'path/to/file.bml\n-\n' | ./fkwu form/form-stdlib/bml-floor-compile.fk >out 2>err` then read `err`. rc 0 and empty stderr means it lowers clean; an unresolved call shows as `unresolved-call ... Recovered to nothing` on stderr. Never run the file itself to check it.

**Duplicate def names** across a file and its prelude closure do not error cleanly: the later arity wins, callers lower with `stray ')' in value position`, and the program can spin forever with no output. Grep `def name(` for the new public names before adding them (`bl-some(s,k)` vs `bl-some(v)` cost an hour).

**Regression set for a grammar change:** `form/form-stdlib/tests/bml-band.fk` 268435455, `bml-generics-band.fk` 16777215, `bml-float-literal-band.fk` 2047, `cell-serialize-band.fk` 1023, and `observe/bml-cursor-coverage-run.bml` (files=1439 sections=1085 read=1085 stops=0 refused=0 on 2026-10-02; ~3-5 min, run in background).

Related: [[reference-fkwu-import-lane-vs-flat]], [[reference-preludes-comments-are-directives]].
