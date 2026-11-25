---
name: reference-fkwu-import-lane-vs-flat
description: "fkwu runs a band two ways: the import lane (./fkwu band.fk and validate's fkwu leg, once dep .fkb images are ITS OWN) and the flat compile (any refused/foreign image). A node field the .fkb remap doesn't shift breaks only the lane, only for images after the first."
metadata: 
  node_type: memory
  type: reference
  originSessionId: de70343c-34cf-4cc9-92b9-5d1df43ed9e0
  modified: 2026-09-11T23:15:05.945Z
---

Found and healed 2026-09-12 (receipts 26-27). Two compile paths on fkwu:
- **import lane**: `./fkwu band.fk` — and validate.sh's fkwu leg ("runtime fkwu source/JIT door",
  not a flattened file; `-plain.fk` is the Go/Rust/TS legs') — imports each direct dep's .fkb when
  THIS binary wrote it, carries transitive deps as text (fk_src_* ~20800-20930).
- **flat**: any refused image (builder id differs — every rebuild changes it —, version stale, unit
  not importable standalone) falls back to the whole-program compile.

The bug class: fk_fkb_remap_field shifts a node's fields by the image's node_base only up to the arity
fk_fkb_node_arity_for_tag gives (explicit lists, else the optab row). A node that carries a child
beyond that arity keeps pointing into the FIRST image's node range — harmless at node_base 0, wrong in
every later image, silent. Instance healed: tag 201 (float_leaf, arity 2) also serves substring as
mode 9 `(201 LIT-9 s (cons start end))`; field 3 unshifted → substring answered nothing → string-case
0 / xpath 1 (since before 7b35a49b). Earlier instance: 199 method_invoke (explicit arity 3).

**How to apply:** after adding a rewrite row or a node shape, check that every field the walker reads
as a node is shifted on import. To witness the lane: remove the chain's untracked .fkb/.sym (bash
script — zsh won't split `$list`), run `./fkwu band.fk` twice. Scratch builds see foreign images and
compile flat, so they can't confirm an import-lane fix. Controls against older commits: build from
`git show <rev>:runtime/fkwu-uni.c` and let THAT binary write its own images first.
See [[reference-fkwu-unit-lets-stop-at-transitive-prelude]].
