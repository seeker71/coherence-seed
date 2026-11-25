---
name: reference-hot-closure-over-locals-stops-at-heat-mark
description: "fkwu 2026-11-24 — a map over a closure that holds locals and calls a def answering nothing() in one branch goes wrong at its 1024th call in a process; symptom is a caught stop, lost work, no row"
metadata:
  node_type: memory
  type: reference
  originSessionId: 080016d2-0080-4364-9610-e52692dc495b
  modified: 2026-11-24T10:11:08.984Z
---

Observed 2026-11-24 on main's `./fkwu`: `flp-near-names` ranked with
`map(n => [n, n == name ? nothing() : flp-near-key(n, weighed, own)], all)` (317 names per call). The first three calls in a
process answered the same eight names; the fourth (the closure's ~1024th call, FK_F64_HEAT) stopped with
`order: only numbers have an order`. The same pieces called at top level, six times each, stayed right; only the closure
inside the def, holding `name`, `weighed`, `own`, failed. Rewritten as a recursion with every value a parameter
(`flp-near-rows`), it answers ten times of ten. The kernel fault itself is not repaired (see
[[reference-hot-recipe-inline-drops-arguments]]).

**How it shows:** an option of a planner walk ends before its row is written, the next packet is byte-identical to the
first, and the only trace is a `form-organ health ... "aspect":"stop"` line on stderr. A multi-walk process (a drain loop,
a harness running ten requests) is where it appears; a one-walk process never reaches the heat mark.

**How to apply:** when a result differs between the first few calls and later ones in one process, loop the single call
8–10 times in a fresh process before reading the logic. Reproduce a walk-level stall without the voice by replaying a kept
reply through `flp-walk` several times in one process.
