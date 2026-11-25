---
name: project-fkwu-version-store-2026-11-09
description: "The cache stack as landed 2026-11-09 (550078396, 8c7d12c37, 9584bddb5, 9217f5e74): sealed reproducible kernel store (./fkwu is a CLONE), lowering+image stores keyed by kernel build, hygiene, ledger+budget sweep, cache-run door, header-aware steward"
metadata:
  node_type: memory
  type: project
  originSessionId: 39d16a37-d4e8-4631-9ac0-105916d511fd
  modified: 2026-11-09T02:35:06.439Z
---

2026-11-09, Urs: cold vs warm may differ only by bandwidth ([[feedback-cold-warm-differ-only-by-bandwidth]]); decisions of
vitality/trust/sovereignty are mine, never his to approve ([[feedback-decide-with-discernment-and-siblings]]).

**What stands (all in runtime/fkwu-uni.c unless noted):**
- Kernel store `$HOME/.cache/fkwu/bin/<digest>/fkwu` + `fkwu.seal` (SHA-256). Digest = FNV of the 3 seed files + flags. The
  seed is read ONCE, digested and compiled from a private snapshot in a fixed layout (reproducible: macOS signs a binary with
  its FILE NAME as identifier, so name+layout are part of the build). `./fkwu` is a COW clone (a symlink dangled when pruned).
  `kernel_stat 90` = 1 when running the seed's version (87-89 are the GPU lease). Binary-freshness band reads it.
- Lowering memos carry the kernel build in the P row's digest slot; store rows are spoken from the checkout root (`@`).
  Image path ids anchor at the checkout root (fk_anchor). Stores: dir owned by us, mode 0700, O_EXCL temps, regular-file reads
  only (FIFO answers a miss), damaged entries unlinked, refused images never stored.
- Ledger `$HOME/.cache/fkwu/ledger` (one `R` line per run, `E` per sweep); sweep to 85% of budget LRU, once per 10 min;
  `FKWU_CACHE=off|ro|rw`, `FKWU_CACHE_REPORT=1`, `FKWU_CACHE_BUDGET_MB`. Door: `./fkwu observe/cache-run.bml`.
- Steward (form/form-stdlib/bml/disk-steward.bml): float_leaf 64 = first n bytes of a file; image header magic FKPIFB1 +
  BE u32 version; `dead-image` = a version != the checkout's OWN kernel version; FORMBIN* is kept; `reclaim 1` reaches every
  worktree. The "40 GB dead" figure of a design agent was relative to the CURRENT loader: judged per checkout only 5.1 GiB.

**Decisions:** no prebuilt kernel downloads (the cloud session derives the kernel from the source it cloned: `cc` once,
~8 s); remote cache tier stays a design (sha256 objects behind a signed manifest, fetched one at a time) until measured need.

**Open:** C4 `destination-changed` stop in the cell-image publisher under concurrent stale locals (pre-existing);
image format v10 (drop the 42% node-symbol rows the loader discards; wide hash for the 40 ms byte-serial FNV; store-direct
load so no local copy); resident cell processes never re-observe home-index.txt (II-5); LI-4 floor pin/discovery caches never
publish. door-links is red on main since #415 (nl-tongues.fk missing, two differing copies of it in old checkouts).

**Process lessons:** never `rm -rf "$VAR/..."` without `${VAR:?}` (the safety check refuses); I deleted a design agent's
worktree before copying its prototype out (it lived only there) - copy deliverables out before cleaning; host `sed -n` slipped
in for reads twice (use head/tail).

2026-11-13 measurement (FKWU_CACHE_REPORT=1, a one-def edit to observe/local-plan-run.bml): lowerings local 351 built 0 (per-unit packets reuse), but the door's IMAGE built 1 = 37490 KiB in 12.94 s wall (~2.9 MB/s CPU); the next run read the same image in 0.18 s. So the cold cost after an edit is whole-image assembly, the one stage that is not per-unit; a comment-only edit did not rebuild it (key ignores comments). Lever: assemble/link the image per unit or make the image build ~50x faster; fkwu-uni.c is the place (sibling-asking kernel change).

2026-11-13 live profile of a cold image build (observe/page-watch-run.bml reads a running kernel's live page, kernel_page_hot pid 0; 40 ms beats, one-def edit to the planner door, ~13 s): load "a unit's top level" 129 beats (~40%, fn 0: digests, link by name, symbol rows, top-level lets; no unit named), bml-source-quote-state 28, parse top level 24, bml-source-comment-at 14, then a long tail (gx, fsc-bml-*, char_at). Tried and reverted: segment-wise fsc-quote-loop (identical bytes, 36 KB string 1.5 s -> 0.1 s) gave NO gain on the door's cold build (15.2 s vs 12.9 s) and cost a one-time 105 s re-lowering of everything; so strings quoted at build are short. The lever is the unit top-level evaluation at load (walked cold), not the quote scan.
