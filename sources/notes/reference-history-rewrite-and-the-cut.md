---
name: reference-history-rewrite-and-the-cut
description: "Main's history after 2026-06-29 was rewritten by a binary-stripping filter (old copies show 3000+/4000+ \"divergence\"); the 2026-09-29 cut 115fea55e removed ~13k files — check both before carrying stranded work"
metadata:
  node_type: memory
  type: reference
  originSessionId: 39d16a37-d4e8-4631-9ac0-105916d511fd
  modified: 2026-11-08T08:11:11.933Z
---

- A checkout whose `git merge-base HEAD origin/main` is `a0b97cf00` (2026-06-29) sits on the PRE-REWRITE copy of main's history:
  same subjects, same author/committer dates, same 117 merges, different hashes (39 large binaries such as whisper-tiny .f32
  fixtures and form-cli-darwin-arm64.fkb were filtered out of every commit). rev-list then reports ~3000 ahead / ~4000 behind.
  It is not divergent work: compare subjects (`git log --format=%s | sort -u`, `comm`), never rebase the old line. Move the
  checkout to a fresh branch from origin/main; carry only uncommitted changes.
- 2026-09-29 `115fea55e` "Cut the repo to its end-to-end healthy flow: 14,729 files to 1,628" deleted ~13k files on purpose.
  Before carrying any stranded file, `git log origin/main --diff-filter=D -1 -- <path>`: a file the cut removed comes back only
  if Urs asks; a never-on-main file whose imports point at cut files is dead on arrival.
- Ledger .jsonl diffs in stranded checkouts are append-only: add rows main lacks (grep -qxF per line), in order.
- 2026-11-08: carried primary checkout + 3457 + ledgers (de0044c9f). Left for Urs: the fkwu runtime rewrite line
  (coherence-kernel-rewrite, codex 4523; its receipt claims no landing), codex 3cee (09-04, 61 conflicts/45 vanished of 113),
  ca19 review doors (need the cut provider-usage probe).
- An "echo" checkout (every old-line subject on main, `comm -23` empty) is moved with `git checkout -B <branch> origin/main`;
  its untracked leftovers were cut artifacts and sibling-kernel cargo `target/` (ca19, 2026-11-08: 287 MB removed on Urs's word).
  Agent worktrees can be recycled onto old-line branches: check `rev-list --left-right` first thing.
- The runtime rewrite (codex 4523) was carried to branch claude/rewrite-home (12 commits + 1 commit of its 214 open files);
  main's only runtime change since its base was the stat presence fix, re-applied to offer_path_exists/offer_file_size.
- `git cherry-pick --skip` resets the working tree: never apply a patch while a cherry-pick is pending.
