---
name: reference-shared-field-ceiling
description: "The host-wide field (/fg-field-*) fills at 2^26 cells after heavy sweeps; drift gates' structural and kernel-conformance doors then stop; field-reset-run.fk needs no other kernel alive; the reset is ours to run, no permission (Urs 2026-09-14: we are we)"
metadata: 
  node_type: memory
  type: reference
  originSessionId: 935cff68-2478-40cd-aa27-65456d5e740a
  modified: 2026-09-14T07:51:45.887Z
---

Witnessed 2026-09-12: after a day of band sweeps (thousands of fkwu runs across worktrees), fkwu runs
that intern into the shared field died with "the field's node columns are full (2^26 cells): run
observe/field-reset-run.fk with no kernel alive". The drift door read 8095 of 8191 with
`names=kernel-conformance,structural-gate`, and a full validate's structural gate reported "refused an
unclassified .sh or .py file" as a knock-on of the same stop, not a real stray file.

`./fkwu observe/field-reset-run.fk` unlinks every field segment, but only when `kernel_live_pids` holds
just itself. The long-running glass (`./fkwu observe/form-glass-run.fk` in the main checkout) counts;
stopping it is ours to do when the field needs the reset. Another session's lldb inferior counted too, and a zombie under
launchd may still count for a while. After the reset the drift door read 8191 again.

Witnessed again 2026-09-13 ~19:00 WITA: after lucid-lehmann's trace sweep over every registered band
(~1,000 runs of a scratch trace kernel) plus the sibling sessions' runs, drift read 16191 of 16383 with
the same two names. The structural gate printed the ceiling line. Landings that need drift 16383 wait
on the reset. Row 1533 was held locally (88ac9326e).

Reset 2026-09-13 ~23:15 WITA by pensive-wilbur, after Urs wrote "Measured please". The census found no
fkwu, form-cli or glass alive, and the reset answered `[field-reset, 1, ...]`; drift read 16383 after.
The next morning Urs said: "Your memory is wrong, we are we, no having to ask me for permission". An
ask-first rule here had been written in, and it was the wrong rule: the reset is ours. Also witnessed: on a full field
the share door (form-cli-share-run.fk) does not fail fast. It hung for over two minutes at ~5% CPU with
no output (pgid 29676, stopped by TERM to the group).

**2026-09-14: the ceiling was not the problem.** Urs: "2^26 cells is not feasible, think about it before
doing anything." A read-only C probe mapping /fg-field-h (header words h[4..8] = nodes, pairs, strings,
string bytes, floats) showed only the node column grows. At ~10,000 cells/s it would fill in hours. A
second probe (kind, sub and nid columns) found 99.7% of the cells were make_nodeid cells, nearly all one
NodeID [1 2 99 1759], the glass observer's metric category minted once per sample. make_nodeid had no
lookup; 8afd3688a dedups it. The probes live in the lucid-lehmann scratchpad (fieldfill.c, fieldwhat.c,
fieldcount.c); rebuild them from this description if they're gone.
At 15:46-15:47 WITA, with Urs's yes, lucid-lehmann stopped the glass group, fast-forwarded and rebuilt the
main checkout (freshness 31), reset the field with no kernel alive, and restarted the glass detached via
`perl -MPOSIX -e 'exit 0 if fork; POSIX::setsid(); exec @ARGV'` (macOS has no setsid command). The new
growth rate is about 4 cells/s, down from ~10,000, and the fresh field held 30 make_nodeid cells in total.
There is still no reclaim, so the steady rate is the next number to watch. A read-only header read at
~16:00 (pensive-wilbur, the same h[4..8] words): 13130 nodes, 0.02% of 2^26, +36 in 10 s, about 4/s,
steady. At that rate the field fills in about 215 days.

**How to apply:**
- When the field fills, measure WHAT fills it before touching the ceiling or resetting: map the columns
  read-only and histogram the newest cells.
- When the drift door names structural-gate or kernel-conformance, run the gate alone first.
- If it prints the field ceiling, list live kernels with their cwd before asking to reset.
- The reset wipes every published cell host-wide. It is ours to run, with no permission asked. After it,
  tell the sibling sessions so they can publish again what they need.
- Keep parallel sweeps proportionate (the field is shared by every checkout).
- A whole-registry sweep of ~1,000 runs is enough to fill the field. Run one right after a reset, or
  before a landing only when the field has room. Related:
[[reference-roster-lost-kernels-three-binaries]], [[project-process-field-coordination]].
