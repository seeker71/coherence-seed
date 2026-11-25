---
name: reference-fkwu-heat-boards-burial
description: "fkwu writes .fkwu-heat.<pid> / .fkwu-boxing.<pid> in its cwd; since e79a2c0a a board-placing exit buries ended pids' boards per kind; which runs are hot enough to witness it"
metadata: 
  node_type: memory
  type: reference
  originSessionId: de70343c-34cf-4cc9-92b9-5d1df43ed9e0
  modified: 2026-09-11T23:47:09.840Z
---

fk_heat_write places `.fkwu-heat.<pid>` (recipes dispatched ≥100000 times) and `.fkwu-boxing.<pid>` (recipes minting ≥1024 floats) in the process's working directory, at the live pulse and at exit (fk_heat_report). Before e79a2c0a nothing removed them: on 2026-09-12 they were 7329 of a worktree root's 7403 entries. Since e79a2c0a an exit that PLACED a board of a kind buries the boards of that kind, and their .tmp files, whose pid fk_pid_gone reads as ended or zombie. A board whose pid answers (any owner) stays, so a living resident's board is never touched.

**Why:** the per-pid name (337620be) exists so a short run cannot erase the glass's resident board; the burial asks the roster's liveness question, never an age.

**How to apply:**
- Not every run is hot: a 3M-call spin cell and transformer-numerics' band place no board and sweep nothing. Witness heat with learn/tests/homecoming-distillation-corpus-band.fk, boxing with form/form-stdlib/tests/format-arith-band.fk; plant a board for a `sleep` pid (kept) and an ended pid (buried).
- Boards left after a sweep are held by pids other programs took over (suggestd, homed, ...); each goes at the next hot exit after its holder ends.
- Readers: lane-motion.bml reads the hearth pid's board while it stands; source-counsel.bml reads a board you point it at.
- A checkout's boards clear only once it runs a binary built from e79a2c0a or later ([[reference-merge-carries-source-not-binary]]).

Related: [[reference-live-page-burial-guard]], [[reference-roster-lost-kernels-three-binaries]].
