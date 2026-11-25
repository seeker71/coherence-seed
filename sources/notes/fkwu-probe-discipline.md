---
name: fkwu-probe-discipline
description: "Hard-won rules for debugging fkwu programs — preludes, probe validation, source-size awareness"
metadata: 
  node_type: memory
  type: project
  originSessionId: faf3891e-9ee2-4b5f-b8e7-1f46da6262f1
  modified: 2026-09-11T13:44:53.461Z
---

Three traps that cost hours on 2026-07-02 while chasing the "N=100 cliff" in coherence-kernel:

1. **Never run a sliced .fk program bare.** Speech/feature programs need `cat form/form-stdlib/core.fk model/trig.fk model/mel-frame.fk observe/wav-sense.fk model/transformer-numerics.fk <program>` — fsin/fcos/flog10 are Form defns in trig.fk, not natives. A missing defn lowers to silent `nothing`, and `(nothing>>1)` arithmetic yields plausible-looking garbage (e.g. −6.75e17 = nothing×0.15). `count`/`nth` don't exist (use `len`, head/tail chains).

2. **Validate every probe on a synthetic known truth inside the same program** before trusting its verdict on real data (probe3/probe11 pattern: a distinct-vector pair that must report "distinct").

3. **fkwu --src used to silently truncate programs over 262,144 bytes** (fixed 2026-07-02: FK_SOURCE_TEXT_CAP now 8MB and fk_run_src dies loudly on overflow — commit 11cd6108). The permissive reader auto-closes parens at EOF, so an amputated program *runs* and returns confident wrong numbers. If a generated program produces an inexplicable deterministic wrong answer, check `wc -c` against the cap era first.

4. **Verify a band by running the band FILE directly** — `./fkwu --src learn/tests/<x>-band.fk` — because fkwu resolves the `; preludes:` comment and loads core+corpus ONCE. The old habit `cat core.fk corpus.fk band.fk > /tmp/x.fk && fkwu --src /tmp/x.fk` DOUBLE-LOADS: the concatenated file PLUS fkwu re-resolving the preludes named inside it. On 2026-07-09 the homecoming corpus reached 113KB (77 rows); 133KB concat + ~130KB prelude-reload crossed the cap and fkwu printed nothing (empty verdict), which reads as a broken band but is a method artifact. Running the band file directly gave the true 511. FK_SOURCE_TEXT_CAP varies by checkout (262144 in some, 8388608 in others, e.g. the turboquant worktree 2026-07-16 has 8MB at fkwu-uni.c:7129) — verify per checkout. UPDATE 2026-07-16: at 160KB corpus the direct lane ALSO dies (`fk_fkb: truncated string`, see item 6); the current honest lane is concat WITH `; preludes:` lines stripped (`grep -v '^; preludes:'`), which returned the true 511.

5. **`*.fkb` / `*.sym` are NOT all build artifacts — `find -name "*.fkb" -delete` destroys tracked tissue.** fkwu emits a `.fkb`+`.sym` beside every cell it runs, and `.gitignore` lists `*.fkb`, so "fkb = regenerable artifact" reads as safe. It is false: `form/form-samples/cross-modal/03-recipe-as-compression/payload.fkb` (2166 bytes) is **tracked** — committed sample tissue. Deleted it on 2026-07-16 with exactly that reasoning. Always `git ls-files | grep '\.fkb$'` before any bulk clean, or clean with `git clean -fdX` (which honors tracked-ness) instead of `find -delete`. Verified 2026-07-16: exactly one tracked `.fkb`, zero tracked `.sym`.

6. **`fk_fkb: truncated string` is usually a misleading dependency/path diagnostic, not a corrupt artifact.** Same source runs via bare relative name from its own dir but dies via a prefixed path (`./fkwu --src form/x.fk` from root); missing `; preludes:` deps sometimes report clearly ("dependency source is missing"), sometimes die as truncated-string. Also: **.fkb emission refuses source integer literals > 2³¹−1** (`fk_fkb_write_signed`) with the unrelated-looking "fk_run_src: failed to write .fkb/.sym artifacts" — computed 64-bit values are fine, so write big constants as products, e.g. `(mul 65536 32768)` for 2³¹. Full repro matrix: receipts/2026-07-16-frontier-ingest-turboquant.md "Also found"; repair chip spawned that day.

7. **Cross-kernel integer identity requires every product to stay within ±2⁵³** — the TS sibling's numbers are JS doubles, so int64-exact arithmetic on Go/Rust/fkwu silently diverges on TS once a multiply exceeds 2⁵³. Caught 2026-07-16: a glibc-style LCG (state×1103515245, ~2⁶¹) passed its own band 127 four-way because all checks were RELATIVE within one kernel; the first band that pinned an ABSOLUTE cross-kernel value exposed TS returning different packs. Repair: Park–Miller minstd (16807×state mod 2³¹−1, worst product ≈3.6e13) — the same bounded-product law rag-embed's small-modulus hashes already obey. When writing any Form cell with integer arithmetic, bound the worst product; when a band passes four-way, ask whether it pinned anything absolute. Receipt: receipts/2026-07-16-turboquant-rag-lane.md.

8. **A band can read stdin; a background run must end it** (2026-09-11). binary-freshness-band's
   EOF canary calls `(read_line)`; in a background Bash, stdin is a socket that never closes, and the
   band slept 90 minutes at 0 CPU (`sample <pid>` showed fk_walk parked; `lsof` fd 0 a unix socket).
   Run every fkwu and validate.sh call in a background script with `exec < /dev/null` first. And a
   scratch probe that compiled with errors is served from its cached `.fkb`/`.sym` on the next run
   ("cached image was compiled with errors"), so every answer reads the same garbage: clear the pair or
   rename the probe after fixing it.

9. **Measure a seed change's reach with a scratch seed that names what moved** (2026-09-11). Copy
   runtime/fkwu-uni.c to the scratchpad, add one `fk_diag(FK_DIAG_ERR, s, "MARK %.*s", ...)` where the
   new rule departs from the old, and build it from the body root with `cc -O2 -I runtime` (the seed
   includes "fkwu-optable.h" and carries no stdio, so fprintf does not compile). Run it over the
   manifest's bands and the FOURTH-ARM ONLY lane (983 bands, four workers, a 20 s cap, about four
   minutes) and grep stderr for the mark: the capture-order fix (9f99bc48) reached exactly one band.
   fkwu reads no cache directory from the environment, so never run a scratch seed beside
   validate.sh; both rewrite the same .fkb files.

**Why:** all five share one failure shape — a true rule applied past its measured edge, or a partial thing accepted as whole. The instrument had the same disease as the patient. Corpus row 743 names the general form: **scrupulosity**, a real law extended past its scope until it does damage (in either direction — too little healing, or too much deleting); the cure is casuistry, i.e. actually measuring the boundary, which is usually one grep.

**How to apply:** before accusing the kernel, ground the probe; before scaling a codegen program, check its byte size; run band files directly (don't hand-cat preludes); always cat preludes for ad-hoc programs; before any bulk delete, list what's tracked. When a rule tells you something is safe to skip or safe to destroy, spend the one grep on its edge — see [[feedback-inspect-manufactured-blockers]] and [[feedback-frequency-check-before-change]].
