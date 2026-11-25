---
name: validate-sh-agreement-only
description: "validate.sh gates sibling AGREEMENT, never a band's declared verdict — a three-way-agreed wrong answer prints green; drift gates must be wired into phase-0 or they are bells nobody rings"
metadata: 
  node_type: memory
  type: reference
  originSessionId: 8fcf55ee-6906-4da3-84ac-a681904761bb
  modified: 2026-09-11T07:50:50.652Z
---

form/validate.sh compares the three kernels' outputs byte-for-byte and prints ✓ on
agreement — it never checks a band's answer against the verdict its header declares.
Witnessed 2026-07-17: primitive-registry-band.fk answered 42 (declared 63) three-way
from its birth at #231 and every run printed `✓ → 42`; the stale verified-pin (166)
exactly absorbed the one false row (form_table_text declared 15, kernels answer 13),
so the count alone could not say whether the world held 166 honest claims or 167 with
one false — corpus row 802, underdetermination.

**Why:** consensus is not correctness; a pinned count that drifts in lock-step with a
real failure reads as design. And a checker script that no harness invokes catches
nothing — validate_primitive_registry.py sat unrung until wired into validate.sh
phase-0 (`--quiet`: one PASS line on success, full report on drift).

**How to apply:** when a band's bare answer disagrees with its declared verdict, probe
per-row/per-bit before trusting either; when adding a drift gate, grep for its invoker
— if nobody calls it, wire it into validate.sh phase-0. **Never run a drift gate's suggested
remedy without reading its diff first**: witnessed 2026-07-22, phase-0's `flt-ops drifted from
native-op-manifest.fk` blocked EVERY band, and its own advice
(`gen_flt_ops_from_manifest.py --write`) deleted `nothing` 0 137 / `nothing?` 1 138 from
form-flatten.fk — axiom-1's first-class nothing, declared in runtime/fkwu-optable.h and
implemented in fkwu-uni.c, live at form-cli-inquiry.fk. The manifest was the wrong side; the
amputation ran clean, silent, one line, and turned the gate green. **HEALED ON MAIN** at
`5e91e284d` (PR #364, merged 2026-07-22) with class `"value"` — origin/main carries it. **The
trap to know: a worktree cut from a stale base re-presents this exact wound as if it were
fresh.** Witnessed 2026-07-30 off `cb4de4c89`, which was 134 commits behind origin/main: the
nom-rows were absent, phase-0 offered the same amputation, and a re-heal was written before the
base was checked. **Check `git log --oneline HEAD..origin/main | wc -l` BEFORE diagnosing any
"missing" heal** — `git log -S'<the row>' --all` finds the elder commit in seconds and
`git merge-base --is-ancestor <it> origin/main` says whether it landed. Re-deriving a heal that
already exists costs a duplicate row and a merge conflict; the elder's word wins on reunion.
**HEALED 2026-07-25**: the
refusal was correct but the gate was then LEFT red for three days, which is its own harm —
refusing a bad remedy is only half the work. Fixed by adding `(nom-row "nothing" 0 137
"host-pool")` / `(nom-row "nothing?" 1 138 "host-pool")` to native-op-manifest.fk between div
and mod (the generator compares ORDERED lists, so insert at flt-ops' own index — index 120 —
and form-flatten.fk needs no rewrite at all). Both gates then green at 132 rows. The sync
gate's own closing line states the direction: "edit manifest first, then regenerate flt-ops —
not the reverse". Probe that settles which side lies: run `(nothing? (nothing))` per arm —
fkwu answers 1, **Go says unbound, Rust refuses**; axiom-1's ground state is fkwu-only, and
the manifest's scope is the fkwu surface, not the shared one. Also: a bad `; preludes:` token
aborts the ENTIRE suite at sweep setup under set -e (lineage-discounted-vote.fk did this
from #266 until 2026-07-18, healed both ways at #315: token → form-stdlib twin, plus
main's `../$token` repo-root door in fk_resolve_dep_path). Related:
[[silent-partial-list-family]], [[preludes-comments-are-directives]],
[[frequency-check-before-change]].

- 2026-09-10: validate.sh's void-reading seal was INOPERATIVE. `trap cleanup EXIT` (line 312) replaced the seal's `trap _validate_seal EXIT` (line 65) — bash REPLACES an EXIT trap, never chains — so every verdict since that handler landed ran unsealed and a tree that moved mid-run printed clean. Healed by one handler carrying both. The general lesson: a guard's silence is indistinguishable from its working, so prove it by making it FIRE (dirty a tracked file one second into a run, demand rc=3). See corpus row 1406, `inoperative`.

- 2026-09-11: validate.sh exited 0 on EVERY run, failed or divergent: `_validate_exit` set `local rc=0` and handed the seal an empty code. Healed (24fca730, on main) by carrying `$?` through cleanup into `_validate_seal "$rc"`; witnessed midi-bmf (before its bp admission) rc=1 "0 ok, 1 failed or divergent", number-band rc=0. No script in the tree gated on its rc (grep found only comments), which is how it stayed quiet. Read validate.sh's rc directly (`out=$(...); rc=$?`), never through a pipe, and when a guard "passes", check that it can fail.

- 2026-09-10 (same day, second lesson): with the seal ARMED, validate.sh cannot be run while sub-agents are editing the tree — it voids (rc=3) correctly. Sequence long runs after agents land, not beside them. Also: macOS has no `timeout`; `timeout N ./validate.sh ... | grep` fails 127 and the pipeline reports grep's 0, so the run silently never happened.
