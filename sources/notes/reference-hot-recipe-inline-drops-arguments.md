---
name: reference-hot-recipe-inline-drops-arguments
description: "fkwu's native loop lane inlined callees by argument-expression substitution (call by name) and silently dropped effects of ignored arguments once a recipe was hot; C guard dd397a892, contract now in Form (anf-lower); judge detector bands against a PLAIN build"
metadata:
  node_type: memory
  type: reference
  originSessionId: 533ece4b-c058-4748-885c-284ca4160f07
  modified: 2026-10-04T02:49:38.217Z
---

Found 2026-10-03 (native-t5 encoder layers stopped dispatching after ~77 words). Mechanism: past FK_F64_HEAT (1024 calls) `fk_f64_inline_try` (runtime/fkwu-uni.c) substitutes each argument EXPRESSION at the callee's parameter reads. A parameter the callee never reads loses its argument (effects gone), one read in a branch runs only there. A plain build of main's committed source lands 3,936 of 780,000 effects on `.hearth/jit-probe/repro-scale.fk`; the guarded build lands all.
- Guard: commit dd397a892 (trial-admits each argument; declines the inline if it carries a call/closure/accumulator/C call). Its C comment carries the SHRINK PATH (AGENTS.md requires one). Measured cost: none (±1 ms).
- The contract lives in Form now: `form/form-stdlib/bml/anf-lower.bml` binds every argument, in order, before the call row; `tests/arg-order-lane-parity-band.fk` compares effect ORDER across cold walker, Form evaluator and hot C lane (16383 guarded, 16356 on a plain build); `tests/hot-recipe-effects-band.fk` (63 guarded / 52 plain); `observe/ignored-argument-scan-run.fk` lints the bug class (4 real sites found and repaired, e.g. `nws-free-after`).
- JUDGE BY A PLAIN BUILD OF THE COMMITTED SOURCE. The main checkout's `~/source/coherence-kernel/fkwu` is unguarded and buggy; a worktree's `./fkwu` can be a symlink to it or a regular file a sibling swapped in (it was swapped to a guarded build at 09:10 on 10-04 and made me misread the situation for an hour). Check `ls -la fkwu` and run the 100x repro before trusting a band number.
- Form lowering holes healed alongside: `lo-node` answered `[]` for an uncovered tag (silent wrong answers; now `nothing()`), `lo-div` was UNSIGNED (-7/2 = 2147483644; now sdiv), results now sign-extended in the epilogue (`lo-sign`), division lowers only by a nonzero literal, tag 5 is MUL in value position and LE in a COND (the form route had it wrong).
- Retirement path for the C lane (Urs's goal: JIT generation in Form, kernel minimal): R1 ANF front end (done) -> R2 shadow install of Form-built pages beside the walker -> R3 heat event + install door -> R4 typed loops -> R5 calls with a real ABI (retires the inliner) -> R6 strings/lists in frames -> R7 effectful natives via trampolines (or leave walked) -> R8 flip default, delete `fk_f64_*`. `lo-map`, `fsim`, `lo-rec-fn` are kept (banded) as the float/recursion rungs' material.
