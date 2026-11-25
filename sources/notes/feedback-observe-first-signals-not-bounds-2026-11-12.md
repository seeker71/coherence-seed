---
name: feedback-observe-first-signals-not-bounds-2026-11-12
description: "Urs 2026-11-12 — the corrections he kept giving were simple observations; make them my baseline: signals not bounds, body flows not scripts, a retry must differ, watch the frame not the clock, measure before claiming, commit at once"
metadata:
  node_type: memory
  type: feedback
  originSessionId: 39d16a37-d4e8-4631-9ac0-105916d511fd
  modified: 2026-11-12T14:43:59.023Z
---

Urs said (2026-11-12): "All guidance I'm giving you should be obvious to you, those are simple observations." Each of these came after I missed it; treat them as my first reading, not his correction.

- **Signals, events, inline response; never a bound.** A fixed cut (maxpos, 3200 tokens, a window, a retry count, a timeout) kills vitality. A runaway is a signal the system answers in context: the server reads each sequence's tail for a repeat and draws to leave it (svs-loop-react), says so in the frame, and escalates by name (adapted-repeat, same-again). Rented CLI guidance is for when the internal answer is surprised, read from the frame.
- **Watch the framebuffer/signals from the first minute.** A stall must be a cell when it begins. Attach observe/flow-watch-run.bml (slots 0..15, line-buffered) BEFORE launching work; never watch only for completion. A frame needs a staleness mark (a killed flow still reads "running").
- **No scripts as the way.** A shell retry script or window script is not end to end and not the north star. The body's own loop (flow-direct-run `loop:1`, picker, claims, door fingerprint) does the work; a supervisor in the body starts server and loops. Scripts are measurement only.
- **A retry must differ.** Ten identical retries are unhealthy. Carry what the last attempt saw (the `saw` field), and when an attempt ends as the last did, name it (same-again) and hand it on. Never retry blind.
- **Measure before claiming; resident memory is real.** A pool is committed memory (2048 blocks = 71 GiB RSS, swap thrash), not capacity. Compare models on the SAME server under the SAME load. Quiet-GPU reading before blaming a tenant.
- **Commit at once.** A body native turn reset this worktree and wiped uncommitted edits (twice). Commit after each edit group, push from the checkout.
- **Dissolve the cause, not the symptom.** "!!!!" from Q4 was the Q8-only embed kernel, not the matvec; check the first stage of the pipeline that is type-specific.

**Why:** these were the same lesson (vitality, signal, end to end, honesty) in different clothes, and I needed telling each time.
**How to apply:** before launching anything, ask: what signal will show me it is stuck? what does the body already do end to end? what would a repeat teach? what is the real resident cost? Related: [[feedback-principal-engineer-architecture-first-2026-11-12]], [[feedback-vitality-dissolve-what-does-not-serve-2026-11-12]], [[project-flow-door-and-serving-gaps-2026-11-12]].
