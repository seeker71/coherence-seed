---
name: reference-float-leaf-latch-and-home-index
description: "since 298ce6017 an all-float body the float leaf can't take stays untried for the typed leaf (latchno, row 1530); ops come home via form-stdlib/home-index.txt + lane arms, never C arms keyed to text"
metadata: 
  node_type: memory
  type: reference
  originSessionId: 17df6750-d2e6-4967-9450-acbd045b17b6
  modified: 2026-09-14T07:52:38.708Z
---

**The latch (healed 298ce6017, 2026-09-13).** The box ledger (fk_fbox, every 1024 boxes) reaches
`fk_f64_pulse` — the all-float leaf, no let/if/conversion arms — before the call-entry heat pulse.
Its decline used to be written into `fk_fn_native` as -(1000+tag), and `fk_f64_loop_pulse_in`
returns on any nonzero state, so the typed leaf (which compiles lets and ifs over float params) was
never asked. A float-param recipe reading -1109 looked like "lets are a wall" when the typed leaf
could take it. Now `fk_f64_pulse` is a wrapper: a decline leaves state 0 (only -1, no body, stays),
and `fk_f64_expr_pulse` falls through to the typed leaf when all params are float.
**How to apply:** a -(1000+tag) on an all-float defn is the typed leaf's verdict now; before
298ce6017 it may have been the float leaf's. Probe with an int param to tell them apart.

**Its reach, measured by lucid-lehmann (9659557c0 vs b2ebcf743, same cells, second of two rounds, per
rendering):** core.fk's float-repr — one float param, so latched before — 0.1 4.3→1.22 µs; 17-digit tie
2.1→1.63 µs (now under the C writer's 2.20); 123456.789 3.4→1.32; 1.5e-7 13.5→9.0; extremes 610-740→
540-700 µs; C natives unchanged. The speedup came through the helpers, not float-repr itself. Native
state (8th hot-row field) after heating, 53a58d319 then 8afd3688a: d4-mant -1 → 2, fr-me -1109 → 1,
fr-whole? 0 → 1, while float-repr still walks (-1109 → -1027). A mixed-param helper unlatched too, so
it is the latch fix's reach (any float-minting defn) and not the all-float fallthrough (their row 1537).
A latch fix is worth re-timing everything that mints floats, not just the recipe it was aimed at.

**How an op comes home now (the rounding family, 298ce6017):** one BML home unit, its names in
`form/form-stdlib/home-index.txt` (every kernel links the unit into any closure that calls a listed
name and defines none — no prelude lines at callers), the head rows out of manifest/flt-ops ×2/fkc
or-group/fourth-arm hati arm, the C arm deleted, sibling natives deleted, registry rows + pins, then
teach the loop lane whatever arm the body lacks (float_to_int = kind 36: FCVTZS + LSL#1/ASR#1 so the
63-bit tagged wrap matches the walker). Measured: floor compiled 0.42 s vs old C arm 0.40 s per 5M
calls; walked 0.95 s. A first cut that bound C arms by an FNV of lowered text worked and was the
wrong layer — Urs: "BML compiler to generate form nodes and then using the JIT to produce assembly".

**Bootstrap tier rule:** a unit in FOURTH_EMIT_CHAIN / the BML floor compiler chain must not call a
home name (link-by-name would pull a .bml into a bootstrap). hati-os-kernel.fk's arm 51 did; removed.
The lowering door finds bml-floor-compile.fk from form/ too (`fk_bml_floor_path`).

Related: [[reference-loop-lane-string-slots]], [[project-core-recipes-leave-the-kernel]],
[[feedback-core-recipes-bml-not-c]].
