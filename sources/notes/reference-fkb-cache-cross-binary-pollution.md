---
name: fkb-cache-identity
description: .fkb identity is now content-true on BOTH lanes (v2 hash with per-dep byte digest; .bml lane checked since 2026-08-31); remaining cautions
metadata: 
  node_type: memory
  type: reference
  originSessionId: e9280898-f345-4dff-b42b-5731b3e9f347
  modified: 2026-09-02T04:38:14.772Z
---

The .fkb identity is CONTENT-TRUE now, both lanes. The `.fk` unit lane carries
the v2 hash (`fk-unit-v2|path@mtime:size#digest` per dependency, FNV-1a over
bytes — landed by the field after the 2026-07-30 same-length witness). The
direct-`.bml` lane was mtime-gated and UNCHECKED until 2026-08-31
("byteseal", corpus row 1198): it now lowers in memory every run (~73ms warm)
and warm-loads only through `fk_src_load_fkb_checked`. The July repro class
(edit `111→222` in the same second) answers its own bytes on both lanes.

Still true and worth keeping:
- freshness-band green says SOURCE identity only — it cannot see linkage; a
  plain `cc` rebuild silently sheds the metal carrier (`metal_linked=false`,
  all metal doors answer zeros) — hollowbuild, row 1196. Rebuild with the
  full AGENTS.md recipe (carrier + frameworks), and check `metal_status`
  after building when metal work follows.
- never point fkwu directly at a brace-surface `.fk` (~100 in form-stdlib).
- `git clean -qfX` deletes the root fkwu.
- a 14-minute stale window (2026-08-31) came from an edit storm: land
  multi-file changes as one batch, then clear affected ices ONCE — mid-edit
  inconsistent states make the supervisor loop churn 45s cold compiles.
- a STALE BINARY after a runtime/fkwu-uni.c merge/rebase self-rejects: it
  writes ices its own reader refuses ("unusable .fkb artifact (trailing
  bytes); rebuilding") on EVERY run — the cache never warms and the warning
  looks like ice corruption, not binary staleness (2026-09-02: binary predated
  the fkb symbol-image consumer 58fbf22e while writing the image). After any
  merge touching fkwu-uni.c, rebuild with the full recipe FIRST, then read
  warnings; a repeating rebuild warning on a just-written ice = stale binary.

Related: [[reference-axiom5-silent-lowering]] (aphonia family),
[[feedback-frequency-check-before-change]].
