---
name: reference-fkb-ice-is-path-bound
description: ".fkb ice never deduplicates across checkouts — its stored source identity is bound to the working directory, so identical source compiled in two worktrees yields different bytes; a content-addressed (source digest + builder) identity is the substrate stone Urs named (2026-09-05)"
metadata: 
  node_type: memory
  type: reference
  originSessionId: f784a9b1-bfab-4fa2-a520-95b2ae087e9c
  modified: 2026-09-04T19:24:18.413Z
---

Witnessed 2026-09-05 03:22 with observe/substrate-reclaim-run.fk: 864 files (adapters, and
every `.fkb` over 64 KiB under form/form-stdlib and observe/ in BOTH the main checkout and
the epic-edison worktree) addressed by sha256 — 863 unique, ONE duplicate (an adapter
copy). No `.fkb` matched across the two checkouts although the sources are the same tree.
fkwu's own warning says why: "stale .fkb (stored source identity does not match, e.g.
written from a different working directory)".

**Why it matters:** Urs's substrate idea — resolve by node-id + update time to one unique
path and reclaim caches and duplicate shards — works today for adapters and model shards
(content-addressed) but not for ice, which is the largest cache on the host and is
recompiled per worktree.

**How to apply:** don't expect `substrate-reclaim-run` to reclaim `.fkb`; the stone is in the
seed's image identity (address a unit's image by source-byte digest + builder digest only,
not by path) — then worktrees share ice by content and the registry
(.hearth/substrate-index.rows: address|mtime|bytes|path) resolves it. See
[[reference-fkb-cache-cross-binary-pollution]] and [[reference-agent-worktrees-and-linear-main]].
