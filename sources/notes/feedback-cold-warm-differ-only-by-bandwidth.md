---
name: feedback-cold-warm-differ-only-by-bandwidth
description: "Urs 2026-11-09 — any cold vs warm claim must be justified by hardware bandwidth difference alone; any other order-of-magnitude gap is a defect to heal, not a fact to report"
metadata:
  node_type: memory
  type: feedback
  originSessionId: 39d16a37-d4e8-4631-9ac0-105916d511fd
  modified: 2026-11-09T00:58:25.281Z
---

A cold run and a warm run may differ only by what the hardware's bandwidth explains (bytes read from disk vs page
cache, a clone vs a copy). Any other gap (recompiling, relowering, rebuilding an image another checkout already
built, a cache tied to a path spelling or cwd) makes no tight engineering sense; it is a dark spot to heal.

**Why:** Urs, 2026-11-09, after self-review measured 60 s cold vs 3.4 s warm. Then 3.63 s once lowerings and images
lived in a content-checked shared store ($HOME/.cache/fkwu/lowfk, /fkb); the leftover 0.09 s was ~300 memo writes.

**How to apply:**
- Never report "cold N s, warm M s" as a plain fact when N/M is far beyond what the byte count explains. Name the
  extra work and treat it as a gap.
- Justify a cold figure by bytes: the size read or written, divided by the disk's bandwidth.
- When measuring, clear only the local caches; a store keyed by content stays warm across checkouts.

Related: [[reference-fkb-ice-is-path-bound]], [[reference-lowfk-memo-spelling-thrash]], [[feedback-witnessed-facts-only]]
