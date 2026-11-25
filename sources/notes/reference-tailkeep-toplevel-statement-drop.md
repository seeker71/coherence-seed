---
name: tailkeep-toplevel-statement-drop
description: "tailkeep (row 1246, re-seated from 1244): fkwu ran ONLY the last top-level form of a unit — ON MAIN since 2026-09-03 (#566, df79bd86)"
metadata: 
  node_type: memory
  type: reference
  originSessionId: d7d59036-9d39-41bd-8886-a5189bc4eb63
  modified: 2026-09-02T15:16:21.476Z
---

Until 2026-09-02, fkwu's `fk_parse_top` overwrote `fk_root` per top-level form: only the LAST top-level statement of a unit ever ran (earlier `print_str`/`write_file` parsed then dropped, green exit). This numbed every direct-run .bml organ reporting via print_str (the lowering is a statement sequence). Go arm always ran them all (implicit do, `readRootFromSource`) — it was a four-way divergence, not a design.

Heal (ON MAIN 2026-09-03 via #566): every top-level form chains into the root sequence (tag 69); top-level `(do ...)` value chains too; top-level `let` joins via its ONE shared hold node — **top-level lets are now EAGER at their textual position** (was lazy-on-first-reference), matching Go. Band: `form/form-stdlib/tests/toplevel-sequence-band.fk` → 15 (pre-heal kernel: 14, the missing bit exactly the dropped statement). Same commit re-applied the stranded read_file/sock/tls absence heals ([[emptymask-family-and-regen-lanes]]).

A cell relying on "only the last form runs" was always broken on the Go arm; a cell with effectful top-level prelude statements newly RUNS them on every dependent run after the heal.

Receipt: receipts/2026-09-02-every-statement-joins-the-root.md. Corpus row 1246 "tailkeep" (re-seated from 1244 at the 2026-09-03 reunion).
