---
name: reference-roster-lost-kernels-three-binaries
description: "fkwu processes on this Mac run DIFFERENT binaries (main checkout, Codex/Grok worktrees, this worktree) against ONE /fg-kernels roster; ask a pid's binary with lsof, never infer it; zombies need sysctl not proc_pidinfo; kernel_roster_adopt heals lost kernels"
metadata: 
  node_type: memory
  type: reference
  originSessionId: f784a9b1-bfab-4fa2-a520-95b2ae087e9c
  modified: 2026-09-11T02:45:15.207Z
---

Witnessed 2026-09-11 (commits 64d26582, 19f5aa25 on reunion-2026-09-10):

- **Which binary is a pid?** `lsof -a -p PID -d txt -Fn` (and `-d cwd`). Every checkout runs its OWN
  `./fkwu` — ~/source/coherence-kernel, ~/.codex/worktrees/*/coherence-kernel,
  ~/.grok/worktrees/..., .claude/worktrees/* — and they all share the one `/fg-kernels` roster shm.
  I inferred "3604 runs the CAS binary" from start time > build time and chased a race that binary
  did not have; it ran the main checkout's pre-CAS build. A fix on a branch reaches no living kernel.
- **Pre-CAS roster claim loses kernels in a burst**: main checkout's fkwu read 13/15 on
  roster-register-band three times (1-2 of 8 lost); CAS seed 15/15/15. Heal from outside:
  `kernel_roster_adopt pid` / `kernel_roster_forget pid` (float_leaf modes 20-21, no tag);
  `./fkwu observe/roster-census-run.fk` counts host vs roster, names untracked/stale/orphans, adopts.
- **Zombies**: `kill(pid,0)` succeeds; `proc_pidinfo` flavors 3 and 13 answer ESRCH (so a pbi_status
  check silently never fires — zombie-alive-band caught it at 13); `sysctl {1,14,1,pid}` →
  kinfo_proc (648 B), `p_stat` byte @36 == 5 is SZOMB. Linux: /proc/<pid>/stat state 'Z'.
- **Orphaned glass organs** (ppid 1) come from sessions that `kill -TERM` the carrier by pid: a signal
  skips fgr-end, and host_spawn_quiet children survive. Codex worktree 3457 did it across many launches
  (organs 1377-1379, 5.5 h, one at ~25% of a core). Heal 5def95b3: born-under.bml (`bun-parent` at
  birth, `bun-held?`), asked every ~2 s by machine/sensors/organs/ear; band born-under-band 31. Running
  organs must never be started to test this — compile via a copy whose loop call is replaced.
- **Release-at-exit orphaned the pages**: a kernel that gives its slot back is never swept, so its 128 MiB
  /fg-k<pid> page (+ store, + A/S/D program surface) stood forever — 2641 by 09:51, 49 MiB resident. Heal:
  49adda3a (exit buries its own) + ee84e4e7 (lucid-lehmann): any OTHER page only by `kernel_page_bury pid`
  (float_leaf 22, one pid) through the fk_live_bury_ended guard — see [[reference-live-page-burial-guard]].
  The census READS (rce-ended, pages-ended) and never buries: Urs asked to be asked before any page another
  kernel leaked is unlinked (relayed 10:19). I buried 2616 host-wide at 09:56 before that reached me.
- **The host names /bin/sh `bash`** here (/private/var/select/sh -> /bin/bash): host_processes "sh" is empty;
  read shells as sh/bash/zsh/dash. Orphans hide behind a form-run shell whose parent is pid 1 (39711 -> 39740).
- **Errandsight door** `host_process pid` (float_leaf 23) -> (pid ppid exe cwd argv); census `rce-kernels`
  -> (pid cell cwd exe owner owner-pid), owner = first ancestor not fkwu/shell/login. bsdinfo (flavor 3)
  gives a non-root user NO ppid for root processes (login) -> read ppid from sysctl kinfo_proc e_ppid @560
  of 648; a 0 parent once walked Urs's Terminal glass to "launchd". band host-process-band 63.
- **Main checkout has no `form/native/metal/fk-metal-carrier.dylib`** beside its fkwu → the ear's live
  lane dies at birth every 120 s ("dynamic carrier artifact unavailable"). Healing it starts the mic
  lane — that is Urs's word, not mine.

Related: [[reference-fkwu-host-doors-no-shell]], [[project-process-field-coordination]],
[[reference-merge-carries-source-not-binary]], [[reference-zero-reading-is-over-nothing]].
