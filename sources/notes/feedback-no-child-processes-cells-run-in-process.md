---
name: feedback-no-child-processes-cells-run-in-process
description: Urs 2026-10-04 — the body must not spawn child processes (least of all ./fkwu) to run its own cells and parse stdout/stderr/exit; cells run in-process and return values; spawning is only for a declared foreign exception
metadata:
  node_type: memory
  type: feedback
  originSessionId: 533ece4b-c058-4748-885c-284ca4160f07
  modified: 2026-10-04T03:44:53.469Z
---

Urs, 2026-10-04, mid-session (voice pipeline, after the host_signal / membrane-census work):

> "for the body to call fkwu makes no sense to me, can you design and architect a different direct integration that does not spawn a new child process and then has to parse stdout, stderr and exit code which is highly inefficient. and update all documentation and memory so that in the future we no longer spawn child processes"
> earlier: "grep shall be an internal tool and not a host tool, and we need to look at each host-exec membrane cross to see what else we need to internalize"

**Why:** a spawn re-loads the runtime and its preludes, re-lowers what a resident process already holds, and then the caller reads a text protocol (the last line of stdout as the verdict, stderr as diagnostics, an exit code) that the callee could have returned as a Form value. It is the membrane crossing at its most wasteful, and between the body and itself it crosses nothing foreign.

**How to apply (the direction, until the docs say what is built):**
- Never write `host_spawn_at(["./fkwu", cell])`, `host-exec("./fkwu ...")` or a wrapper of them to compute a value, run a band, call another door, or read a verdict. Call the cell in-process and take its VALUE, captured output and diagnostics as data (the design and its status live in `docs/in-process-cells.md`; if it is not there yet the architecture is being built — check the receipts of 2026-10-04 and ask before adding a new spawn).
- Parallelism is not a reason to spawn: shard the work (an operator or the OS starts the shards), or hand typed values between already-living peers over the shared field (`cell_map`, `shm_*`, the cell channel/mesh) — never parse a child's stdout.
- A process that must exist because the OS must isolate it (a worker that can wedge in an uninterruptible GPU wait) is a service the host's own manager starts and restarts; the body talks to it through the shared field and `host_signal`, and reads its progress as values.
- Foreign programs are only the declared exceptions (the `cc` that builds the seed; `git` until Urs decides). `grep`, `head`, `tail`, `tr`, `cut`, `sort`, `uniq`, `wc`, `cat`, `rg`, `jq`, `sed`, `awk` are internal tools of the body (docs/form-native-agent-tools.md), never host programs.
- Before starting any fkwu, the older standing preference still holds: route to a living residence ([[project-process-field-coordination]]).

Related: [[feedback-form-native-only-external-models-ok]], [[reference-gpu-lease-and-host-signal]], [[project-native-voice-pipeline]], [[reference-stop-is-a-backtrack-point]].
