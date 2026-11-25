---
name: feedback-less-bands-live-handling
description: "Urs 09-29 — \"less bands, less gates, live events, live choice/retry/surprise handling\": move handling into running flows; don't build band machinery"
metadata:
  node_type: memory
  type: feedback
  originSessionId: 90dc3bca-9a6e-4280-bc86-b8df3f128d34
  modified: 2026-09-29T01:29:22.081Z
---

Urs, 2026-09-29, after I answered "turn bands into live events" by making bands speak (a band-head
reader, a runner door, two new bands, repairs on gates): "I asked for less bands, less gates, live
events, live choice handing, live retry handling, live surprise handling".

**Why:** bands and gates read a surprise after the fact; the body should meet it where it happens,
in the running step, and need fewer after-the-fact checks.

**How to apply:** when asked for live/dynamic behaviour, put it in the running flow (walk carry,
turn, landing) via `form/form-stdlib/bml/live.bml` (`live-do(flow step ctx run holds repairs)`,
repairs `live-repair(name meets act)` offered only when `meets` what the step answered). Don't add
a band to prove an organ; its live use is the witness. Gates run only when their ground moved
(`dg-grounds` in gate/drift-gates.bml). If a change adds bands/gates, stop and ask whether the
handling could live in the flow instead. See [[reference-host-dir-door-contracts]],
[[feedback-surprise-is-a-choice-point]].
