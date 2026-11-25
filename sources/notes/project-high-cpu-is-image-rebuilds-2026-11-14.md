---
name: project-high-cpu-is-image-rebuilds-2026-11-14
description: "2026-11-14 — the sustained 100% CPU of day-turn's child fkwu is a whole-closure image (37 MiB) rebuilt per plan-option witness band; images are path-bound and the band unit differs per option, so lowerings hit the cache and the image never does"
metadata:
  node_type: memory
  type: project
  originSessionId: 39d16a37-d4e8-4631-9ac0-105916d511fd
  modified: 2026-11-14T05:09:01.923Z
---

Urs 2026-11-14: "High CPU needs understanding." Read from the live page (observe/page-watch-run.bml on the hot pid) and the
fkwu ledger, not guessed.

**What it is:** `observe/day-turn.bml` (the body's own turn) spawns a child `fkwu` per plan-option witness band, from a fresh
scratch dir (`$TMPDIR/local-plan-band-<ms>/owned-source/.../witnesses/<name>-band.bml`). Its page is all `load`/`parse`/`melt`
of the compiler's own units (fbl-lower, fsc-rec-call, fsc-quote-loop…): a cold compile at 97-100% of one core for ~13 s.

**Measured (FKWU_CACHE_REPORT=1, the same 38 MiB-closure band):** path A cold 140 s (lowerings local 3104, image built 1 = 37070
KiB); path A again: image local hit; path B (identical bytes): image built again. So per-unit lowerings are content-keyed and
hit; the whole-closure image is bound to the path AND changes when the band unit changes, so a witness band that differs per
option can never reuse it.

**Weight:** ~/.cache/fkwu ledger 22,889 runs; 941 built an image over 30 MB (~13 s CPU, ~37 MiB written each); 109 of today's
2,730 runs. fkb cache = 1,655 images = 14 GB, mostly such rebuilds.

**Next lever (the design, not built):** a base image for the stable closure (the ~3,100 prelude units, content-addressed) plus a
small overlay for the band's own unit; or lazy linking (only definitions a run reaches). A node walker alone does not help:
measured 7.7x slower than the printer ([[project-fkwu-version-store-2026-11-09]]). Two separate hot things that are NOT ours:
macOS `signpost_reporter` (57-94% for 40 min, with Activity Monitor open) and WindowServer (~30%).

**Addendum 2026-11-14 15:10 (walk page + sampler, observe/jit-sample-run.bml):** the walk (observe/day-turn.bml, launchd, window 12:30-15:00, a started turn may run
past the window by design) held a 54 GiB footprint = the BML lowerer's interned recipe nodes (mint ledger: fsc-rec-fndef/-do/-call, fsc-bml-expand-in) never freed;
RSS read 3.1 GiB (compressor holds the rest), so kernel_stat 91 (phys footprint, landed aef022806) + hw-memory-room (host-walk-window.bml, band 31) now stop new
turns when the walk holds more than the host has available. Its cumulative page: bml-value-equal 3.9e9 calls (native -1080 = the crystallizer refused tag 80,
value_eq), pifbc-proj-* ~5e8 each (the Form-side .fkb byte container run by bands in the learning step). JIT lane facts: the native column is the f64/int leaf
crystallizer (FK_F64_HEAT 1024): -1 declined, -2 frame slot not a number, -1000-tag refused tag; a cold BML build's hottest recipes (gx, fsc-bml-type-find,
fsc-bml-call-arity, fsc-bml-erased) are all declined: there is NO tier for list/string/record bodies and no time-based escalation. Cold build also re-reads
3100 unit sources for closure discovery (fsc-bml-expand-in) even when all lowerings hit. A flow landing for an already-satisfied request (56d9363ae) deleted a
clause and regressed check-witness-pattern 15->14; healed e26b7ac95. Open: gate the flow's landing on the gates its edited file maps to; a post-landing sentinel.
