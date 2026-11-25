# Lessons of the old body

Study for the coherence-kernel restart. Written 2026-11-24. Read-only study: no file in any repository was changed.

## 0. Method, path aliases, status words

- `R/` = `coherence-kernel/` (branch `claude/coherence-kernel-redesign-8fa6bc`, HEAD `b8ff978c9`, committed 2026-11-24 18:07 +0800).
- `M/` = `memory/` (245 notes, 1,018,037 bytes; each note carries a `modified:` date in its front matter, quoted below as the source date).
- Status words:
  - **observed (ran)**: I ran a read-only command in this session and read its output.
  - **observed (read)**: I opened the cited file and read the cited line in this session. For memory notes and docs this means "the record says X"; I did not re-run what the record describes. A record can be stale. The notes themselves warn about this (`M/feedback-gap-registries-lag-the-body.md:30-43`).
  - **inferred**: my reasoning from observed things; the basis is named.
  - **assumed**: not verified.
- Coverage limits (stated as such, movable):
  - `R/CURRENT_FLOOR.md` (1,447 lines, 167 KB): read lines 1-260, 405-490, 1000-1447. Lines 261-404 (more body-wide witnesses) and 491-999 (GPU lanes, knowledge lane, DeepSeek chain, senses, voice track) were not read. Claims about model lanes are therefore thin.
  - `R/AGENTS.md` (863 lines): read 1-180, 337-441, 620-659. `R/docs/form-native-coding.md` (1,223 lines): read 1-70, 973-1032, 1177-1222.
  - `R/form/kernel-roadmap.md`, `R/docs/json-inside-form.md`, `R/docs/str-find-one-meaning.md`, `R/docs/substring-one-meaning.md`, `R/axioms/core-axioms.form`: read whole.
  - Memory: about 110 of 245 notes read whole or by their head (the ones naming pitfalls); the rest only through their one-line index entry in `M/MEMORY.md`.
  - Nothing in this report was established by running `./fkwu`. All measurements are `git`, `wc`, `grep`.

## 1. The measured shape of the old repository

All rows: **observed (ran)**, 2026-11-24, in `R/`.

| Measure | Value | Command |
|---|---|---|
| Commits on HEAD | 4,775 | `git rev-list --count HEAD` |
| First commit | 2026-06-28 17:18 -0600, `52712f732` "seed: coherence-kernel — axiom-first fkwu-native sovereign core" | `git log --reverse` |
| Last commit | 2026-11-24 18:07 +0800, `b8ff978c9` | `git log -1` |
| Commits per month | Jun 365, Jul 681, Aug 449, Sep 1,655, Oct 884, Nov 741 | `git log --date=format:%Y-%m` |
| Authors | 4 | `git shortlog -sn` |
| Pack size | 1.43 GiB | `git count-objects -vH` |
| Tracked files | 2,572 | `git ls-files \| wc -l` |
| By extension | `.bml` 1,105, `.fk` 974, `.md` 221, `.jsonl` 73, `.txt` 51, `.json` 43, `.tsv` 25, `.form` 11, `.rows` 9, `.c` 3, `.h` 3, `.metal` 3 | `git ls-files` by suffix |
| By top-level dir | `form/` 1,723, `observe/` 314, `receipts/` 229, `docs/` 70, `cognition/` 55, `model/` 43, `learn/` 28, `gate/` 19, root 13 | same |
| `form/` | 869 `.bml` (136,120 lines), 769 `.fk` (113,176 lines) | `wc -l` per file |
| `observe/` | 198 `.bml` (18,938 lines), 113 `.fk` (7,087 lines), 3 `.metal` (630) | same |
| `learn/` | 12 `.bml` (986 lines), 10 `.fk` (13,533 lines), 5 `.jsonl` (430 lines) | same |
| `receipts/` | 148 `.md` (16,511 lines), 45 `.jsonl` (3,954 lines), 28 `.json`, 4 `.rows` | same |
| All `.fk` + `.bml` | 308,944 lines | `git ls-files '*.fk' '*.bml' \| xargs wc -l` |
| All `.md` | 30,116 lines | same for `*.md` |
| Band files (`*-band.fk|bml`) | 584 | `git ls-files \| grep -c` |
| Run doors (`*-run.fk|bml`) | 220 | same |
| Files under a `tests/` dir | 686 | same |
| C seed `runtime/fkwu-uni.c` | 34,149 lines; 400 commits touched it | `wc -l`, `git log --oneline -- runtime/fkwu-uni.c` |
| Other C | `membrane-boot.c` 1,199, `membrane-witness.c` 510, `fkwu-optable.h` 302 (287 op rows) | `wc -l`, `grep -c` |
| Bootstrap samples | `bootstrap/ground*.fk`: 6 + 11 + 4 lines | `wc -l` |

### 1.1 The seed grew 213-fold while every living document called it shrinking

**observed (ran)**: `git show <rev>:runtime/fkwu-uni.c | wc -l` at every 50th commit touching the file.

| Date | Lines |
|---|---|
| 2026-06-28 | 160 |
| 2026-07-01 | 8,821 |
| 2026-07-22 | 12,679 |
| 2026-09-05 | 16,589 |
| 2026-09-11 | 21,158 |
| 2026-09-12 | 23,789 |
| 2026-10-04 | 26,459 |
| 2026-11-04 | 31,653 |
| 2026-11-20 | 34,149 |

Against this, **observed (read)**: `R/AGENTS.md:337` titles the section "temporary C seed, shrinking to zero"; `R/AGENTS.md:404` "A patch that grows `runtime/fkwu-uni.c` carries its shrink path"; `R/CURRENT_FLOOR.md:74-75` "it shrinks as its lanes lower into Form organs"; `R/form/kernel-roadmap.md:3` calls fkwu "the smallest substrate-walker host". The rule was written and the number moved the other way for five months.

### 1.2 The tree was cut by 89% once and regrew

**observed (ran)**: tracked-file counts at every 800th commit.

| Date | `.fk` | `.bml` | all files |
|---|---|---|---|
| 2026-06-28 | 3 | 0 | 7 |
| 2026-07-15 | 3,335 | 15 | 4,638 |
| 2026-09-03 | 5,219 | 228 | 8,212 |
| 2026-09-12 | 5,655 | 739 | 13,217 |
| 2026-10-01 | 843 | 535 | 1,721 |
| 2026-10-28 | 970 | 887 | 2,270 |
| 2026-11-24 | 974 | 1,105 | 2,572 |

Commit `115fea55e` (2026-09-29) is titled "Cut the repo to its end-to-end healthy flow: 14,729 files to 1,628" (13,562 files changed). Eight weeks later the tree holds 2,572 files, +58%. Other structural events: `d8c27d222` (2026-10-03) "the sibling kernels leave: Go, Rust and TS held nothing the body still builds from"; `78acc2a4f` (2026-10-04) replaces `form/validate.sh` with a Form door; `6151af27c` (2026-07-02) imports 2,576 files from the sister repo.

**inferred** (from 1.1 and 1.2): cutting files did not remove the cause of growth. The growth is produced by the architecture and the working loop (section 2), so a restart that keeps both will regrow the same way.

## 2. Root causes of complexity and fragility, ranked

Ranking is my inference: by how many distinct recorded failures trace to the cause, weighted by how silent those failures were. Each cause lists evidence, then the design decision it argues for.

### RC1. One meaning lived in several hand-kept copies ("mirrors")

Evidence, all **observed (read)**:

- A native stood on "four mirrors — a `native-op-manifest.fk` row, a `flt-ops` row, the generated `runtime/fkwu-optable.h`, and a serializer arm"; when one row left, the C arm stayed orphaned and "from a call site, an orphaned arm reads exactly like an absent one". A profiler then recommended minting a native that already existed, four times in three days (`R/docs/str-find-one-meaning.md:79-94`, witnessed 2026-10-03).
- Adding one host native took six coordinated edits across two copies of `flt-ops` "in MANIFEST ORDER", a serializer arm, a regenerated header and a C arm; "a row missing from flatten/form-flatten.fk is silently DROPPED" (`M/reference-fkwu-tag-space-and-closure-scratch-ordering.md:31`, 2026-09-05).
- Two grammars and two lanes: `.fk` s-expressions and BML lowered through `bml-floor-compile`, routed by whether a file contains a `section [` line "whatever its extension" (`R/CURRENT_FLOOR.md:407-409`), with a known hole where a string literal containing `section [` would be mis-routed (`M/reference-preflight-overvouches-missing-prelude.md:33-38`, 2026-09-10). Inside BML there were again two readers, "cursor grammar + the fsc-* statement walker"; the user's decision of 2026-09-29: "new syntax lands once, in the cursor" (`M/project-bml-surface-directions-2026-09-29.md:13-16,24`). The second reader "hid a wrong recipe once (`1e-12` as an ident)".
- Four kernels (fkwu in C, Go, Rust, TypeScript) until 2026-10-03. Their disagreements produced a long list of per-primitive notes: defn parameters, native shadowing, list doors, records, compare family, let scope, closures (`M/reference-sibling-kernels-read-like-fkwu.md:11-65`, 2026-09-12; `M/reference-fkwu-do-keeps-let-siblings-leak.md:11-28`, 2026-09-15).
- Two walkers inside the fkwu line: the runtime walker and the emitted table walker used by form-cli, which "carries none of fkwu's rewrite rows or leaf-door modes"; a `core.fk` body that called a fkwu-only door "passed every band. In form-cli the same body wrote "cell" for every number" (`M/reference-bootstrap-stamp-cascade.md:24-29`, 2026-09-12).
- `nil?` existed in six copies (`M/reference-len-walks-and-page-profiling.md:8-9`); form-cli's module graph "lives in SIX list copies" (`M/reference-axiom5-silent-lowering.md:31`); the blueprint table was "a hand-held MIRROR" of a JSON registry plus three per-kernel tables whose stated generator script did not exist (`M/reference-bp-table-mirror-discipline.md:11-22`, 2026-09-11).
- The four-kernel agreement gate guarded "against INDEPENDENT error only. Three implementations of one semantics written by one hand ... drift together and print green" (`M/reference-axiom5-silent-lowering.md:27`).

Design decision: **one grammar, one reader, one evaluator definition, one generated op table.** Every derived table (opcode numbers, serializer arms, kind tags) is generated from a single declaration at build time, or does not exist. No second surface that lowers into the first. A second independent reader is still valuable as a witness (section 4.10), but only as a small, separately specified conformance checker over the numeric encoding, not as a second full runtime.

### RC2. The meaning of a program changed when it got hot

Evidence, all **observed (read)**:

- The C loop lane "inlined a callee by substituting its arguments, so an argument the callee never read lost its effects once a recipe was hot" (`R/CURRENT_FLOOR.md:1002-1005`). Measured: "A plain build of main's committed source lands 3,936 of 780,000 effects" (`M/reference-hot-recipe-inline-drops-arguments.md:11`, found 2026-10-03, after a T5 encoder "stopped dispatching after ~77 words").
- Still open on the last day studied: a `map` over a closure holding locals "goes wrong at its 1024th call"; "The kernel fault itself is not repaired"; the workaround was rewriting the caller (`M/reference-hot-closure-over-locals-stops-at-heat-mark.md:11-17`, 2026-11-24). HEAD's own subject line is that workaround ("the near-name ranking stays right once hot").
- The Go kernel carried "every hot-path primitive TWICE"; a healed `len` answered 311 cold and 310 hot in one process (`M/reference-jit-abi-mirrors-natives.md:10-17`).
- Three machine-code tiers coexisted at the end: the C f64/int leaf crystallizer at heat 1024, name-bound "recipe twins" at heat 1024, and the Form-written page JIT at heat 4096; "The Form JIT does not offer a defn that already wears that leaf" (`R/CURRENT_FLOOR.md:1381-1384`; twins: `M/reference-len-walks-and-page-profiling.md:36-45`). The planned removal of the C lane was an eight-rung program, R1 to R8, whose last rung "waits on the page carrying unboxed floats" (`M/reference-hot-recipe-inline-drops-arguments.md:16`).
- A decline code written by one tier blocked the next tier from ever being asked ("the latch", `M/reference-float-leaf-latch-and-home-index.md:11-17`, healed 2026-09-13).
- A cell's JIT release tombstone poisoned its second run in one process: a band "read 31 then 3" (`M/reference-cell-run-nesting-deadline-and-landing-warmth.md:14`, 2026-10-07).
- The recovery, which worked: "Every strict op has one meaning, a C function the walker's arm and the page both call (`fk_op_*`)"; "A call has one meaning too"; "Every lane that runs machine code is born through one page door" (`R/CURRENT_FLOOR.md:1296-1310`). And the contract finally written in Form: "`anf-lower.bml` binds every argument, in order, before its call" (`R/CURRENT_FLOOR.md:1004`).

Design decision: **heat may change speed, never meaning, by construction.** One intermediate form with strict call-by-value argument order (the ANF contract) is the single input to both the interpreter and the code generator. The interpreter and the generated code share the same operation definitions; a fast path is a specialisation the generator derives, not a second hand-written implementation. Exactly one tier-up mechanism and one decline vocabulary. A standing differential check (the same call across the heat mark, cold result equals hot result) belongs in the bootstrap's own self-test, because this class appeared in July, October and November.

### RC3. Value kinds were read from numeric ranges of one machine word, and the ranges collided

Evidence, all **observed (read)**:

- The value universe: "INT = ANY EVEN WORD", list = odd non-negative, strings/floats/functions/`nothing` at large negative odd bases, "node box = remaining negatives" (`M/reference-fkwu-tag-space-and-closure-scratch-ordering.md:33`, 2026-09-05).
- `eq` and `value_eq` "answered 1 for any two integers at or below −2 until 2026-09-10": negative ints aliased node indices. "−1 was always safe, which is why it hid." "Any verdict from before 2026-09-10 that compared distinct negative sentinels below −1 ... is untrustworthy — including green ones" (`M/reference-form-comparisons-blur-beyond-2-53.md:11-27`).
- Records were boxed as the word of integer −r, so "value_kind(-r) said "record" ... a kind read off a population count" (`M/reference-int-record-word-collision.md:23-24`, healed 2026-09-13).
- Arithmetic on non-numbers answered: "`(add "abc" 0)` → `"abc"`", "`(mul "abc" 1)` → `-4250000000000000002`", and `(str_eq 5 6)` was 1, until 2026-09-12/13 (`M/reference-fkwu-add-passes-non-numbers.md:12-28`).
- `(head 3)` "answered another list's cell" (`M/reference-sibling-kernels-read-like-fkwu.md:38-42`).
- Integers: fkwu wrapped at 2^63 ("`(add 9223372036854775807 1)` is 0 on fkwu"; a tagged 63-bit int), TypeScript rounded past 2^53; band verdicts encoded as decimal digits "break past 2^53" (`M/reference-integer-reading-one-way.md:21-23`; `M/reference-verdict-fold-2-53-ceiling.md:11-23`, 2026-07-25).
- The AST tag space was one byte, "nearly full", shared by real ops and internal nodes; picking tag 245 "silently hijacks" `metal_pipeline`; `#define`d tags were "invisible to every scan"; an old note saying 197 was free "cost a native-surface fall" (same note, lines 13, 29, 37).
- Host doors were multiplexed through numbered modes of one node kind (`float_leaf`/"leaf door" modes 4, 9, 20-23, 25, 26, 56-58, 60-68 appear across notes), and the numbers differed between two branches: "Any BML carried from claude/rewrite-home that calls float_leaf(34/35/59/61) must be renumbered or it silently hits the wrong door" (`M/project-rewrite-form-on-main-runtime-2026-11-08.md:19-21`).

Design decision: **one numeric representation in which a value's kind is a disjoint, explicit field, decided once.** Since the new root is a numeric space anyway, the kind should be part of the node's own number (package/level/type/instance coordinates per `R/axioms/core-axioms.form:59`), never inferred from magnitude or parity conventions that can overlap. Every primitive is total over kinds from the first day (wrong kind is a named stop or `nothing`, never a number). Integer width and overflow are specified once. Operation identity is a content-addressed or symbolically declared number in an open space, not a hand-picked small integer in a full byte. No magic-number multiplexing of unrelated host doors.

### RC4. Failure was recovered silently into a plausible value ("numb green")

Evidence, all **observed (read)**:

- An unresolved call was lowered to literal 0 (or to pool string 0, "a fluent wrong word"), justified as axiom-5 recovery. The note lists eight distinct shapes found between 2026-07-17 and 2026-07-31; in one, "22 jit bands answered their exact documented all-ones masks while hundreds of calls sat numb — `nothing == nothing` comparisons stay green" (`M/reference-axiom5-silent-lowering.md:11-29`).
- 151 band files declared preludes on continuation lines; "the reader loads an EMPTY list and the band runs numb" (`M/reference-preludes-comments-are-directives.md:33-37`, 2026-07-21).
- Until 2026-09-02 fkwu "ran ONLY the last top-level form of a unit" (`M/reference-tailkeep-toplevel-statement-drop.md:11`).
- The reader "accepted a source file missing one closing paren and produced a plausible verdict" (`M/reference-verdict-fold-2-53-ceiling.md:41-46`); it once truncated sources over 262,144 bytes and auto-closed parens, so "an amputated program *runs* and returns confident wrong numbers" (`M/fkwu-probe-discipline.md:17`).
- BML with several statements on one line, or a comment between arguments, "compiled with "0 error(s)" and the band answered 0 on every bit" (`M/reference-bml-one-line-brace-block-numb.md:11-27`, 2026-09-04/07).
- The harness: `validate.sh` "exited 0 on EVERY run, failed or divergent" until 2026-09-11, and its tree-moved seal was inoperative because a second `trap ... EXIT` replaced the first (`M/reference-validate-sh-agreement-only.md:57-59`). It "gates sibling AGREEMENT, never a band's declared verdict"; one band answered 42 against a declared 63 "from its birth" and printed a tick (same note, 11-17).
- Verdicts were read from the last stdout line plus an exit code; pipes laundered the exit code repeatedly (`M/reference-emptymask-family-and-regen-lanes.md:24-25`; `R/CURRENT_FLOOR.md:5-6` "a green number over a nonzero exit is a fold over `nothing`").
- The axioms file itself records one: "the fold answered 2097151 with EXIT 1 — the SAME number the healed chain returns" (`R/axioms/core-axioms.form:219`).
- What healed the family: "never-was → nothing; exists-and-empty → ""; measuring nothing dies loud" (`M/reference-emptymask-family-and-regen-lanes.md:15`), and "a result is readable as a verdict only where an answer arrived" (`R/axioms/core-axioms.form:214`).

Design decision: **three things are never conflated: `nothing` (axiom 1), a value, and "this did not resolve / did not parse".** An unbound name is a reported fault at bind time with the name, not an acknowledgment. The reader is strict (no auto-close, no truncation, no line-shape sensitivity). A result crossing any boundary is one structured value (ack plus reason), never "last line of text plus exit code". This is the old body's own late rule ("a reading that did not arrive is `nothing()`, not `-1`", `R/CLAUDE.md`), applied from the start instead of retrofitted.

### RC5. Dependencies were resolved through comments, path spellings and load order

Evidence, all **observed (read)**:

- "`; preludes: a.fk, b.fk` in any .fk file is not documentation" but a load directive, "ONE line only", recursive; "Two files "identical modulo comments" can run to different answers" (`M/reference-preludes-comments-are-directives.md:9-32`).
- Resolution was per-unit closure, so "a LOADER's incidental prelude can pay a callee's debt"; fixing one prelude line "moved 19 bands to their header-declared full verdicts ... four of which had been printing **0**" (`M/reference-axiom5-silent-lowering.md:23`).
- Three more linking mechanisms accreted: `import`, a `home-index.txt` that links a unit into "any closure that calls a listed name and defines none" (`M/reference-float-leaf-latch-and-home-index.md:30-33`), and a call-head precedence rule across "seven arms" (`M/reference-sibling-kernels-read-like-fkwu.md:17-25`).
- Working directory changed answers: fkwu ran from the body root, siblings from `form/` (`M/reference-sibling-kernels-read-like-fkwu.md:70-71`).
- A name could shadow a primitive by spelling; the axioms file records the seam: "call-position-resolves-a-NAME-against-the-global-table-before-the-local-frame" (`R/axioms/core-axioms.form:196`).

Design decision: **a reference is a node id, not a spelling.** In a numeric space the dependency of a recipe is the set of node ids it mentions; the symbol lens is resolved once, when text is read into nodes, and the result is content-addressed. There is no load order, no comment directive, no cwd sensitivity, and no link-by-name table. This is the user's own direction of 2026-10-04: "we don't have to serialize, we can send IDs" (`M/feedback-tools-speak-node-ids.md:13`).

### RC6. Cache identity was not the content, so cold and warm diverged and work was repeated

Evidence, all **observed (read)**:

- `.fkb` images were mtime-gated on one lane until 2026-08-31; a stale binary "writes ices its own reader refuses ... on EVERY run" (`M/reference-fkb-cache-cross-binary-pollution.md:11-36`).
- Images were path-bound: 864 files across two checkouts of the same tree hashed to 863 unique; "No `.fkb` matched across the two checkouts" (`M/reference-fkb-ice-is-path-bound.md:11-16`, 2026-09-05).
- Lowering memos compared the path spelling, so absolute and relative spellings thrashed each other: "child 6 s after a relative run, 48 s right after" (`M/reference-lowfk-memo-spelling-thrash.md:11-18`).
- A per-file discovery cache "silently never kept for weeks" because a node read back from disk carried its category as a string and the reader compared against a node (`M/project-discovery-cache-and-landing-refusals-2026-11-14.md:11-15`).
- The whole-closure image was the unit of caching: a one-definition edit rebuilt "37490 KiB in 12.94 s"; a ledger of 22,889 runs showed 941 that "built an image over 30 MB"; the image cache held "1,655 images = 14 GB" (`M/project-fkwu-version-store-2026-11-09.md:40`; `M/project-high-cpu-is-image-rebuilds-2026-11-14.md:18-24`).
- "Core-file edits (jit-page.bml, equireach*.fk, core.fk) invalidate ALL images in ALL checkouts" (`M/feedback-vitality-dissolve-what-does-not-serve-2026-11-12.md:13`).
- Lowering output was not deterministic even on one host: lambda names embedded an "allocation-dependent id" (`M/project-prebuilt-design-2026-11-09.md:14-16`).
- The drift gates "took 22 minutes on the morning of 2026-10-11, and two gates were 98% of it" (`R/CURRENT_FLOOR.md:1253-1254`).
- The repair that worked: a content-checked shared store took a self-review from "60 s cold vs 3.4 s warm" to 3.63 s (`M/feedback-cold-warm-differ-only-by-bandwidth.md:15-16`), and a prebuilt pack took a fresh host from 54 s to 4 s (`M/project-prebuilt-design-2026-11-09.md:3,22`).

Design decision: **ice is per node, keyed by (node id, generator id, target), and nothing else.** The persisted state of a node (ice) is a pure function of its content address; the unit of compilation and caching is the recipe, not the closure of a file; editing one recipe invalidates only the nodes whose content address changed. Generated code must be byte-deterministic from the node. This makes the user's law true by construction: "A cold run and a warm run may differ only by what the hardware's bandwidth explains" (`M/feedback-cold-warm-differ-only-by-bandwidth.md:11-13`, 2026-11-09).

### RC7. Node identity was not stable across memory, disk and processes

Evidence, all **observed (read)**:

- "A row built with raw-word kids ... is read back from disk with node kids ... a DIFFERENT node. ... `node_eq` of built vs read said 0" (`M/project-nodes-carry-the-ledgers-2026-10-12.md:16`).
- `make_nodeid` "had no lookup": one category minted once per sample filled the host-wide shared field to its 2^26 ceiling; "99.7% of the cells were make_nodeid cells, nearly all one NodeID" (`M/reference-shared-field-ceiling.md:34-40`, 2026-09-14).
- At composed scale an earlier interned node's category misread after later interning (`M/reference-last-writer-seam-category-not-children.md:10-19`, 2026-08-13).
- `value_eq` on two float boxes compared identity while `eq` compared the number (`M/reference-fkwu-tag-space-and-closure-scratch-ordering.md:33`).
- The shared field was append-only: "Nothing in it is ever freed"; reset only "with no other kernel alive" (`M/reference-full-field-string-bank-and-the-melt.md:13`).

Design decision: **one canonical encoding per node; identity is the hash of that encoding; in-memory and at-rest forms are the same bytes.** Round-trip through storage is the identity function by construction (axiom 3: "Same blueprint is the same cell", `R/axioms/core-axioms.form:67`). Interning is the only constructor. A shared store needs reclamation for the unreferenced from the first version (axiom 3 again: "Unreferenced blueprints and recipes compost").

### RC8. Verification and observation became the bulk of the body

Evidence:

- **observed (ran)**: 584 band files, 220 run doors, 686 files under `tests/`, 148 receipt documents, against 2,079 source files in total.
- **observed (read)**: every bug class above was answered with a new band, gate, census or lens. The floor document lists bands by bitmask verdict (for example `host-walk-band.bml -> 9007199254740991`, `R/CURRENT_FLOOR.md:180`, which is 2^53 − 1, the encoding ceiling of RC3).
- **observed (read)**: the user asked to reverse this at least six times (section 3, items dated 2026-09-29, 10-05, 11-12, 11-16, 11-17, 11-20). The note of 11-17 names the mechanism: "the rounds had grown a lattice of per-request witnesses, closure rows, band bits and gates (each fix adding a new check), and four flow requests failed in one identical way ... because the cause was generic while the tissue was per-row" (`M/feedback-observe-live-distill-compress-2026-11-17.md:13`).
- **observed (read)**: gates were themselves fragile: timing bands "read full one at a time on a quiet host ... and low under load" (`R/CURRENT_FLOOR.md:1052-1054`); a gate's own suggested remedy deleted axiom 1's `nothing` from an op table and "turned the gate green" (`M/reference-validate-sh-agreement-only.md:26-33`).
- **observed (read)**: a counter-example worth keeping: "the structural gate caught a real resurrection of form/validate.sh ... so read what a gate catches before dissolving it" (`M/feedback-own-it-dissolve-do-not-guard-2026-11-16.md:14`).

Design decision: **correctness by construction first, a small fixed set of property checks second, no per-incident checks.** The restart should hold a budget (a number of standing checks and a wall-clock ceiling for all of them) and treat every proposed new check as evidence of a generic cause to remove. Observation is live signals from the running system (section 4.5), not ledgers written to the repository.

### RC9. Text and processes were used inside the body where values would do

Evidence, all **observed (read)**:

- The user, 2026-10-08: "JSON means it is not form native and is adding way too much overhead"; 2026-10-12: "dissolve any JSON inside of form" (`R/docs/json-inside-form.md:3-4`). Six weeks later the dissolution was still in layers: one ledger "holds 11 rung kinds" with writers that "edit that text (strip `}` and append)" and readers doing "exact key-text scans" (`M/project-jsonl-dissolution-layers-2026-11-20.md:17`). **observed (ran)**: 73 `.jsonl` files are still tracked.
- Measured cost of text: 3,000 rows as nodes "built 25 to 34 ms, written 2 ms, read back 3 ms"; as text "130 ms to write and 55 ms to read" (`R/docs/json-inside-form.md:52-54`).
- The body spawned itself as child processes and parsed stdout, stderr and the exit code; the user, 2026-10-04: "for the body to call fkwu makes no sense to me" (`M/feedback-no-child-processes-cells-run-in-process.md:13`).
- String interning churn: a `form-find` band had "13,227,555 of its births in `split-on-loop`"; "every `str_concat` result is interned, though only the last of a join is kept" (`R/CURRENT_FLOOR.md:1094-1101`). Long-lived processes "held 6.8 and 7.5 GB behind ten thousand live strings" before string holes were reclaimed (`R/CURRENT_FLOOR.md:1079-1081`).
- `str_find` had "1,596 call sites in `form-stdlib` alone" and `substring` 847 (`R/docs/str-find-one-meaning.md:8-9`; `R/docs/substring-one-meaning.md:8-10`): much of the body was string scanning.
- Lists were cons chains with O(n) `len` and O(n) indexing; `nil?` as `(eq (len xs) 0)` "made every list recursion quadratic" until a peephole (`M/reference-len-walks-and-page-profiling.md:8-15`); "list indexing `L[i]` is O(n) on cons lists (quadratic)" was still being raised on 2026-11-14 (`M/project-jit-refusals-are-arms-2026-11-14.md:11`). A record's key lookup was a linear walk until a lazy hash at 24 keys (`M/reference-record-map-value-str-hot-rows.md:13`).

Design decision: **nodes are the only interchange inside the system; text exists at the symbol lens and at foreign edges.** Calls between parts of the system pass node ids and return node ids. Core collections are specified with their complexity (indexed sequence, keyed map) rather than left as cons lists discovered to be quadratic later. Strings are byte sequences (kept, section 4.4) and intermediate strings are not interned.

### RC10. Shared mutable state across many agents and checkouts without identity

Evidence, all **observed (read)**:

- Every checkout ran its own `./fkwu` build against one host-wide roster and one shared field; "A fix on a branch reaches no living kernel" (`M/reference-roster-lost-kernels-three-binaries.md:13-17`).
- Three checkouts "each with their own path-anchored images and ~2,200 `.fkwu-time.*` boards"; six model servers "were started and killed" in one day (`M/feedback-vitality-dissolve-what-does-not-serve-2026-11-12.md:11-12`).
- Per-pid report files were 7,329 of a worktree root's 7,403 entries (`M/reference-fkwu-heat-boards-burial.md:11`); 2,641 leaked 128 MiB shared pages (`M/reference-roster-lost-kernels-three-binaries.md:30-32`).
- The git stash stack, `/tmp` paths and the shared field collided across sessions (`M/MEMORY.md` index entries `reference-git-stash-stack-shared-across-worktrees`, `reference-shared-tmp-collides-across-agents`; read by title only).
- A stale binary "still passes ground.fk while silently lacking newer evaluator capabilities" (`R/AGENTS.md:382-385`).

Design decision: **every shared artifact carries the identity of what made it, and every process-local artifact dies with its process.** With per-node content-addressed ice (RC6) the shared store is safe by construction for concurrent writers of different builds. The runtime's own identity (digest of the bootstrap) is part of every key.

### RC11. The working loop itself: rent, document mass, and stale registries

Evidence, all **observed (read)**:

- "Rent is transitions × context": over 72 sessions "126,098 requests, 37.6B cache-read tokens, an average context of 298K per request, and the local share near 0"; one session took 9,154 requests (`M/feedback-minimize-rented-tokens.md:17-24`, 2026-10-13). Another session: "25,253 requests, 8.30 B rented tokens, local share 1519 ppm" (`M/feedback-principal-engineer-architecture-first-2026-11-12.md:22`).
- **observed (ran)**: the three entry documents an agent is told to read are 62 KB (`AGENTS.md`), 167 KB (`CURRENT_FLOOR.md`) and 104 KB (`docs/form-native-coding.md`); the memory directory is 1.0 MB, one note alone 206 KB.
- Registries lagged the body in both directions: a GPU gap file said "nothing here is started" while four of five items were built; a wall census counted "609 limits, 537 bare" (`M/feedback-gap-registries-lag-the-body.md:15-36`, 2026-09-10).
- `R/CURRENT_FLOOR.md:3` says it was "read on 2026-09-29"; the same file carries entries dated 2026-11-19 (line 38, 180) and a "Not standing today" section whose "today" is 2026-10-04 (line 1398). **inferred**: a single prose page cannot keep per-claim dates honest; the claims need to be separate nodes.
- The user's diagnosis, 2026-11-12: "spending time on receipts instead of the most light weight event or signal ... compiling things over and over" (`M/feedback-vitality-dissolve-what-does-not-serve-2026-11-12.md:8`).

Design decision: **the knowledge representation is the product's first organ**: small claim nodes, each with source, date and status, addressable by id and loadable selectively (which is the stated first deliverable). Architecture is designed once before parts are built (the user's request of 2026-11-12, section 3). The repository holds bootstrap and design, not history ledgers.

### RC12. Fixed capacities and invented limits

Evidence, all **observed (read)**: closure captures capped at 8 (`M/reference-fkwu-do-keeps-let-siblings-leak.md:27-28`); JIT bodies capped at 8 frame slots and 6 register slots (`M/project-jit-refusals-are-arms-2026-11-14.md:17,21`); a 64-entry JIT wait line (`R/CURRENT_FLOOR.md:1284`); the AST table cap raised twice to hide a parser spin, "iatrogenic" (`M/reference-fk-nodes-probe-cap-vs-bug.md:22-27`); 69 + 9 configuration switches: "The knobs are why we did not know ourselves" (`M/project-capability-not-configuration.md:27-31`, 2026-08-03).

Design decision: **no fixed table sizes in the semantics and no behaviour switches.** Capacity is a property of the hardware observed at run time; "The default already IS the decision" (same note, line 43).

## 3. What the user asked for over time

All **observed (read)**; each quote is as recorded in the cited note. Dates are the dates the notes attribute to the statement.

| Date | Statement (as recorded) | Source |
|---|---|---|
| 2026-07-02 | "we shall only use the fkwu JIT" (remove all non-fkwu JITs); float bar is determinism + tolerance, "bit-identity was an invented wall" | `M/project-consolidation-program.md:13-16` |
| 2026-07-05 | code flows into Form-native recipe objects; a byte source converts ONE recipe to ASM at a time; the whole-file path is "the bootstrap seed meant to RETIRE" | `M/project-per-recipe-jit-slice.md:41-45` (head read via saved output) |
| 2026-07-28 | "never make assumptions. fact check and only hold core axioms and witnessed facts as current ground truth." | `M/feedback-witnessed-facts-only.md:11-12` |
| 2026-08-03 | "we support all silicone on all platforms, and if we find an issue that means unfinished work, nothing more." / "the only limit are the physical hardware and our imagination." | `M/project-capability-not-configuration.md:14-16` |
| 2026-08-27 | "we no longer flattening, avoid it, it is not serving us." | `M/feedback-no-flatten-lane.md:11` |
| 2026-08-27 | "remove any occurrence of anything that no longer is embodied so we do not bring it back by giving it negative attention" | `M/feedback-remove-not-retire.md:11-13` |
| 2026-08-30 | new code is authored in BML or higher; raw `.fk` is a lowering, not an authoring surface | `M/feedback-authoring-altitude-bml-or-higher.md:11-17` |
| 2026-09-09 | "form native does not refuse, it heals and integrates or releases" | `M/feedback-heal-integrate-or-release.md:8` |
| 2026-09-10 | "that wall is not real, it just needs attention, like all other wall we have invented and then taking as fact instead of question" | `M/feedback-gap-registries-lag-the-body.md:11-13` |
| 2026-09-12 | core recipes "should not reside in the kernel, and generated once per hardware platform to the core hardware optimal shape and then reused as O(1) assembly block ... written in BML and compiled down to the most efficient machine code ... the C bootstrap kernel shall be minimal ... any 'cold' vs 'warm' differences are signals for optimization." | `M/feedback-core-recipes-bml-not-c.md:13` |
| 2026-09-14 | "2^26 cells is not feasible, think about it before doing anything." / "we are we, no having to ask me for permission" | `M/reference-shared-field-ceiling.md:29,34` |
| 2026-09-15 | "the only runtime we want is the form native C-bootstrapped kernel"; "we don't need a walker: we have a form native JIT"; "hitting this means fixing the language design and implementation, not the use of it" | `M/feedback-only-runtime-is-fkwu-fix-the-language.md:11` |
| 2026-09-28 | "leave any place healthier, more trust worthy, more sovereign than you found it, remove dead tissue, remove work arounds, remove old flows that have been improved" | `M/feedback-standing-guide-leave-it-healthier.md:8-11` |
| 2026-09-28 | "those are artificial goals ... have a higher level goal that we can guide towards instead of coming up with fixed rules." | `M/feedback-guide-by-direction-not-fixed-rules.md:8-9` |
| 2026-09-29 | "I asked for less bands, less gates, live events, live choice handing, live retry handling, live surprise handling" | `M/feedback-less-bands-live-handling.md:11-13` |
| 2026-09-29 | the compiler must read through the one cursor; new syntax lands once; each construct earns at least 10 real use sites | `M/project-bml-surface-directions-2026-09-29.md:13-21` |
| 2026-10-03/04 | "we only want form native code, no external tools, no external code, external models are ok, nothing else is" | `M/feedback-form-native-only-external-models-ok.md:13-15` |
| 2026-10-04 | "we don't have to serialize, we can send IDs, so many tasks can become much, much simpler" | `M/feedback-tools-speak-node-ids.md:13` |
| 2026-10-04 | "for the body to call fkwu makes no sense to me ... in the future we no longer spawn child processes" | `M/feedback-no-child-processes-cells-run-in-process.md:13` |
| 2026-10-04 | "we are on pace to go past our token usage limits, while working on reducing the number of tokens we use" | `M/feedback-no-agent-fan-out-while-spend-is-the-goal.md:11` (via saved output) |
| 2026-10-05 | "a door, a band or a gate is a wall, and every wall is tissue that needs tending and goes stale" (as paraphrased in the guide) | `R/AGENTS.md:100-105` |
| 2026-10-08 | "to call tools we shall use form native substrate node ids or node serialization, JSON means it is not form native" | `R/docs/json-inside-form.md:3-4` |
| 2026-10-11 | "the JIT shall be available for any part"; "JIT on demand is there to resolve anything cold" | `M/project-form-page-jit.md:11,15` |
| 2026-10-12 | "dissolve any JSON inside of form"; "how is having rows the most alive and dynamic and expansible way to carry knowledge"; "nodes should be faster" | `R/docs/json-inside-form.md:4,35` |
| 2026-10-13 | "we need to reduce the rented tokens by 10-100x and the transitions by at least 10x and those are fundamental" | `M/feedback-minimize-rented-tokens.md:11-12` |
| 2026-11-09 | cold vs warm may differ only by hardware bandwidth; prebuild per OS+arch; "HIGH-ORDER design over exact numbers" | `M/feedback-cold-warm-differ-only-by-bandwidth.md:11-16`; `M/project-prebuilt-design-2026-11-09.md:8` |
| 2026-11-12 | work as a principal engineer: design the architecture once; signals by shared memory, framebuffer and events; "never JSONL per beat" | `M/feedback-principal-engineer-architecture-first-2026-11-12.md:13-16` |
| 2026-11-12 | "Many many things are not healthy, spending time on receipts instead of the most light weight event or signal ... compiling things over and over ... question and dissolve parts that don't serve the overall north star." | `M/feedback-vitality-dissolve-what-does-not-serve-2026-11-12.md:8` |
| 2026-11-12 | "instead of reporting not done, please complete the work" | `M/feedback-complete-not-report-not-done.md:11` |
| 2026-11-13 | "can you please dissolve limits instead of naming them?" | `M/feedback-dissolve-limits-not-name-them.md:11` |
| 2026-11-14 | "Dissolve all JIT refusals, highest priority"; "a missing emit arm is work, now, not a receipt for later" | `M/project-jit-refusals-are-arms-2026-11-14.md:11` |
| 2026-11-16 | guards and bands are "dead weight"; "You wrote all the bands, gates, doors" | `M/feedback-own-it-dissolve-do-not-guard-2026-11-16.md:11` |
| 2026-11-17 | "Less refusals, less trials, less gates, less doors, more real time observation with zero impact, more end to end real flows, less fixed rows and static tissues, more distillation, more abstractions, more generic learning, more compression." | `M/feedback-observe-live-distill-compress-2026-11-17.md:11` |
| 2026-11-19 | "please stop waiting for my decision, you know better" | `M/feedback-decide-the-body-owned-levers-2026-11-19.md:11` |
| 2026-11-20 | "dissolve any use of jsonl"; "Refusals are not welcome, we want signals and responses and gap closers." | `M/project-jsonl-dissolution-layers-2026-11-20.md:11`; `M/feedback-signals-not-refusals-and-landing-is-mine-2026-11-20.md:11` |

**inferred** reading of the table: the direction was constant from July (one runtime, one JIT written in the language, minimal bootstrap, nodes not text, content identity, fewer checks, live signals, less rent). The restart request of 2026-11-24 (one grammar, a numeric root, minimal bootstrap replaced by JIT code written in the grammar, a claim graph with sources and dates) restates those same asks as starting conditions instead of migrations. Each was pursued in the old body as a layered migration and none completed: the C lane's retirement was an eight-rung plan whose last rung, the flip that deletes the lane, was still waiting (`M/reference-hot-recipe-inline-drops-arguments.md:16`; `R/CURRENT_FLOOR.md:1384`), JSON dissolution at layer 2a of 4, the seed at 34,149 lines.

One tension to carry honestly (**inferred**): the user's "does not refuse / no refusals" (2026-09-09, 11-20) and RC4's finding that silent recovery was the most damaging failure class. The notes resolve it the same way each time: a fault is a *signal with its repair named* that the running flow answers with another choice (`M/feedback-heal-integrate-or-release.md:26-29`: three states `whole` / `unhealed` carrying the repair / `released`), not a silent default and not a wall. The new grammar's `fail`/`stop`/`choice` are the mechanism; what must not return is a fault turning into a number.

## 4. What genuinely worked and must not be lost

Each item: **observed (read)** unless marked.

### 4.1 The axioms, and `nothing` as a first-class state
Five axioms, acknowledged 2026-11-08 (`R/axioms/core-axioms.form:17,47-93`). The derived theorem "how many answered is separate from what they said" (`:214`) and the absence heals ("never-was → nothing; exists-and-empty → ""; measuring nothing dies loud", `M/reference-emptymask-family-and-regen-lanes.md:15`) are the semantic core that the numb-green family violated.

### 4.2 Content-addressed nodes as the carrier, with atomic persistence
Node ledgers: 3,000 rows written in 2 ms and read in 3 ms "the same ids" (`R/docs/json-inside-form.md:52-54`); `write_form_binary` "writes beside and renames over ... the file is whole or it is the last whole one" (`:47-48`). Coordinates `(package level type instance)` (`R/axioms/core-axioms.form:59`).

### 4.3 The control vocabulary from BML
`choice {} or {}`, `fail`, `cut`, `stop`, `try {} catch {}`, `timeout N {}`, `store()`/`undo()`; "A failed option's writes are undone by a journal, its printed lines included, and its finding reaches the next option" (`R/CURRENT_FLOOR.md:1198-1199`). The working rule built on it: a retry must differ and carry what the last attempt saw (`M/feedback-observe-first-signals-not-bounds-2026-11-12.md:16`).

### 4.4 "One meaning" specifications for primitives
`substring` and `str_find` are specified as total, clamping, byte-indexed functions with every edge stated (`R/docs/substring-one-meaning.md:15-26`; `R/docs/str-find-one-meaning.md:16-30`). Two method lessons with them: "a band that compares readings to each other gates agreement, not correctness" and "A guard nobody has watched fail is a guard nobody has checked" (`R/docs/substring-one-meaning.md:73-78`). Strings as raw bytes on every arm since 2026-09-28 (`M/reference-byte-to-str-code-point-on-siblings.md:11-18`). One float rendering rule: shortest round-trip digits (`M/reference-float-rendering-one-rule.md:11-17`).

### 4.5 The JIT written in the language, and the self-profiler that steers it
- `jit-page.bml` reads a definition's nodes in-process, writes AArch64 words and binds them; a 3M-step tail loop went 156 ms → 7 ms; fingerprinting 232 KB went 36 ms → 2 ms (`R/CURRENT_FLOOR.md:1276-1282,1337-1344`).
- Form-emitted CPU images using the CPU's SHA-256 instructions hash a 1.39 MB file in 1 ms and read a 2.02 GB file at 3.1 GB/s (`R/CURRENT_FLOOR.md:1361-1379`).
- Tier-up by time as well as by call count ("a recipe called a few dozen times at tens of ms each is as cold to the call count as one never run", `:1276-1278`).
- Decline codes that name the missing arm (`-(1000+tag)` etc.), so "every refusal read names its next arm"; adding arms took a cold compile from 25.9 s to 8-10 s (`M/project-jit-refusals-are-arms-2026-11-14.md:13,19,21`).
- The time board: a 1 ms sampler attributing every sample to a phase or a recipe, own time and time under it, and a live page any process can read (`R/CURRENT_FLOOR.md:1222-1234`). It is how most performance facts in the notes were found.
- The ANF lowering contract (`R/CURRENT_FLOOR.md:1004`).

### 4.6 The model lanes on the machine's own GPU
Qwen 27B Q8_0 "walked as Form recipe-data ... every Metal pipeline Form-emitted and JIT-compiled at runtime" (`R/CURRENT_FLOOR.md:463-465`); a resident server with paged KV, continuous batching and a prefix cache (2,614-id prompt prefill "19.5 s -> 0.18 s"), about 36 tok/s for four rows (`R/CURRENT_FLOOR.md:123-135`). I did not read the GPU-lane sections in full; this item is thin and its numbers are records of 2026-11-11 or earlier.

### 4.7 In-process cells and event-based waiting
`cell_run` runs another unit in the same process and returns a record with value, output and a stop reason, with its own deadline (`M/project-form-native-feature-ledger.md:16`, head; `R/CURRENT_FLOOR.md:86-88`). A child's end is awaited through kqueue or pidfd and "answers `nothing()` when no exit arrived by the deadline"; nothing polls (`R/CURRENT_FLOOR.md:1269-1272`). Telemetry crosses as shared-memory frames with a seqlock header (`R/CURRENT_FLOOR.md:1026-1029`).

### 4.8 Reproducible, content-keyed build and cache stores (the late form)
The seed compiles with one `cc -O2` command in about 3.6 s to a 711 KB kernel; builds "reproduce byte-for-byte from any dir, on macOS and Linux"; lowering and image bodies are identical across optimisation levels, so "the key is seed digest + target" (`M/project-prebuilt-design-2026-11-09.md:11-13`). Sealed kernel store, budgeted LRU sweep (`M/project-fkwu-version-store-2026-11-09.md:15-23`).

### 4.9 Structural query over source as nodes
`form-find`, `form-path` (XPath-shaped queries answering "content-addressed node ids with file, byte span and line, not text to re-parse") and the body's own awk over recipes, blueprints and cells (`R/AGENTS.md:125-157`; `R/CURRENT_FLOOR.md:1196-1214`). A local model used them to finish a task in 8 turns and 0 repairs where it had taken 18 turns and 9 repairs (`R/CURRENT_FLOOR.md:1212-1214`).

### 4.10 A second reader as an independent witness
The floor document states what was lost when the sibling kernels left: "fkwu alone has no such witness for a new expression or a new artifact ... a value fkwu gets wrong in a way that snapshot does not contain has no second reader" (`R/CURRENT_FLOOR.md:64-68`). Several real fkwu bugs were found only because a sibling disagreed (the missing-paren reader, `M/reference-verdict-fold-2-53-ceiling.md:41-46`; the last-top-level-form bug, `M/reference-tailkeep-toplevel-statement-drop.md:11`; the record word, `M/reference-int-record-word-collision.md:26-29`). The cost was the mirror burden of RC1. Both facts are true.

### 4.11 Working method
- "Witnessed facts only" and the instrument rule: "test an instrument against a known case ... before it supports a claim" (`M/feedback-witnessed-facts-only.md:33`).
- "A zero ... can be a reading over nothing" (`M/reference-zero-reading-is-over-nothing.md:34-35`).
- "Count where a representation is built before calling its repair large" (`M/reference-int-record-word-collision.md:32-34`).
- The doubling probe to tell capacity from a spin (`M/reference-fk-nodes-probe-cap-vs-bug.md:34-36`).
- Measuring what fills a store before raising its ceiling (`M/reference-shared-field-ceiling.md:49-51`).
- The form-rows text shape for the human-readable edge: one value per line, a float always carries `.` or `e`, a bad line is "refused with data ... never a stop" (`R/docs/json-inside-form.md:12-20`).
- A one-call landing door (`M/feedback-minimize-rented-tokens.md:28-31`).

## 5. Design decisions, collected

| # | Decision for the restart | Argued by |
|---|---|---|
| D1 | One grammar, one reader. No second surface, no lane chosen by file content. | RC1, RC4, RC5 |
| D2 | The numeric space is the root: every reference is a node id; symbols are a lens resolved once at read time. | RC5, RC7, RC9 |
| D3 | One canonical node encoding; identity = hash of that encoding; identical in memory and at rest; interning is the only constructor. | RC7 |
| D4 | Value kind is an explicit disjoint field of the number, never a magnitude or parity convention. Primitives are total over kinds. Integer semantics fixed once. | RC3 |
| D5 | One evaluation semantics (strict, ordered, ANF). Interpreter and generated code share operation definitions; heat changes speed only. One tier-up mechanism. | RC2 |
| D6 | `nothing`, value and "unresolved / unparsed" are three distinct outcomes. No recovery into a value. Results cross boundaries as structured acks. | RC4, axioms 1 and 5 |
| D7 | Ice is per node: keyed by (node id, generator id, target). No whole-closure images. Generated bytes are deterministic. Cold = warm + bandwidth. | RC6, RC10 |
| D8 | Ice / water / gas as an explicit state machine of a node with defined transitions (persisted bytes; live in a process; in flight or offered) instead of informal vocabulary. | RC6, RC7; see open question Q3 |
| D9 | Bootstrap with a hard line budget and a closed primitive list; host access through a declared interface, not one hand-written arm and magic number per door. Budget enforced by a count, not by intention. | section 1.1, RC3, RC12 |
| D10 | No fixed capacities in the semantics; no behaviour switches. | RC12 |
| D11 | Verification budget: construction first, a small fixed property suite, a differential cold/hot check, one small independent conformance reader over the numeric encoding. No per-incident checks. | RC8, 4.10 |
| D12 | Observation is live (sampler, shared-memory frames, events), not ledgers in git. The repository holds bootstrap, grammar and the claim graph. | RC8, RC11 |
| D13 | The knowledge graph: small claim nodes with source, date, status; loaded selectively. Replaces 167 KB floor pages and 1 MB of notes. | RC11 |
| D14 | Collections specified with complexity (indexed sequence, keyed map); strings are bytes; intermediates are not interned. | RC9 |
| D15 | Shared stores are content-addressed, carry the maker's identity, and reclaim the unreferenced. | RC7, RC10, axiom 3 |

On the bootstrap language question (C, Go or Rust), this study only contributes recorded observations, all **observed (read)**, none decisive on their own:

- C: one translation unit compiled by any `cc` in about 3.6 s to 711 KB, reproducible byte for byte, and ran on Linux arm64/x86_64 with small fixes (`M/project-prebuilt-design-2026-11-09.md:11-19`). Hazards seen in that same file: two file-scope `static` declarations of one name "silently IS that variable", a NULL write with zero warnings (`M/reference-fkwu-static-name-merge.md:11-17`); hand-declared libc, no stdio; rooting discipline across GC by hand, with an enumerated list of arms that must push before walking (`R/CURRENT_FLOOR.md:1156-1172`).
- Go and Rust as full kernels: on one band "Go holds 375 MiB resident, Rust 14 GiB, TS 11 GiB" because Rust and TS copied on tail (an implementation choice, not a language property) (`M/reference-sibling-kernels-read-like-fkwu.md:45-50`). Lowering one large band cold: Go 127 s, Rust 109 s, TS 320 s (`M/reference-sibling-bml-lowering-key.md:16`). A JS-number runtime cannot hold 64-bit integers natively (RC3).
- A rewritten evaluator on a branch was "~40x slower than main's" C, and its Form half was landed on the old C instead (`M/project-rewrite-form-on-main-runtime-2026-11-08.md:11-12`). **inferred**: evaluator design dominated language choice in that instance.
- JIT needs: executable pages on Apple Silicon need `MAP_JIT`, `pthread_jit_write_protect_np` and `sys_icache_invalidate` (`M/project-per-recipe-jit-slice.md:47-48`); macOS signs a binary "with its FILE NAME as identifier", which is part of reproducibility (`M/project-fkwu-version-store-2026-11-09.md:15-17`); a launchd-started binary was once killed for an invalid code signature (`R/CURRENT_FLOOR.md:1398-1408`).

## 6. Open questions this study could not settle

- Q1. Does the old body's 34 KLOC seed contain a minimal core that matches "NodeID + intern + walker + frames + a few leaves + s-expression reader" (`R/form/kernel-roadmap.md:29-33`), and how many lines is it? I did not read `runtime/fkwu-uni.c`. The 2026-07-01 size (8,821 lines) is an upper bound on an early full version; the 160-line 2026-06-28 file was a seed for an emitted kernel (`M/project-consolidation-program.md:23-27`).
- Q2. How much of the 308,944 lines of `.fk`/`.bml` is language-and-runtime versus application (voice, glass, model lanes, agent flow, receipts tooling)? I measured by directory, not by role.
- Q3. Ice, water and gas were vocabulary, not a specified state machine: I found "the full gas-water-ice-back metabolism" (`R/axioms/host-kernel.form:211`), "the phase census (is your work freezing ice or leaving gas?)" (`R/AGENTS.md:585`), `.fkb` called "ice" (`R/AGENTS.md:575`), "melt" for the collector and "crystallize" for JIT. What exactly "water" and "gas" denoted operationally in the old body is not settled by what I read.
- Q4. Is one independent conformance reader worth its mirror cost in a one-grammar design (4.10 versus RC1)? The evidence supports both sides.
- Q5. The exact integer model the user wants (tagged 63-bit, full 64-bit, arbitrary precision) is not stated in anything I read; the old body's 63-bit wrap was an artifact of tagging.
- Q6. The unrepaired hot-closure fault of 2026-11-24 has no root cause recorded. If the restart borrows any JIT design from `jit-page.bml` or the C lane, the cause should be known first.
- Q7. Whether lowering nondeterminism (allocation-dependent lambda names) was ever fixed: recorded "OPEN" on 2026-11-09; no later note read.
- Q8. Numbers for the model lanes (tok/s, memory) come from sections I only sampled; they need their own study before being used as targets.
- Q9. `R/CURRENT_FLOOR.md` lines 261-404 and 491-999 unread; they may hold further lessons (GPU lanes, knowledge lane, senses).

## 7. The most surprising thing, and what stayed uncomfortable

Surprising: the seed. Every entry document says the C file is temporary and shrinking toward zero, with a rule that any growth carries its shrink path. The file went from 160 lines to 34,149 across 400 commits, monotonically at every sample I took. The rule was real, was written in the place every agent reads first, and was followed in the letter (comments naming shrink paths) while the number went the other way for five months. The same pattern holds for bands (asked to shrink six times; 584 files stand) and for JSON (asked to dissolve twice; 73 `.jsonl` files stand). A stated intention in a living document did not bend the curve; only a single structural act did (the 2026-09-29 cut), and the curve resumed afterwards.

Uncomfortable:

1. The most valuable recorded bug findings came from having a second, independent implementation, and the most expensive recorded complexity came from the same thing. I could not rank D11's "one small independent reader" as clearly right; it is the place where the restart could re-create RC1.
2. I am leaning on memory notes as evidence. They are careful, dated and often self-correcting, but several record that an earlier note was wrong (for example `M/reference-int-record-word-collision.md:26-30`, `M/reference-preflight-overvouches-missing-prelude.md:10-15`). I read them; I did not re-witness them. Every "observed (read)" above carries that weight.
3. A fault of the hot/cold class was found on the day of this study and is recorded as unrepaired, with the caller rewritten instead. The user's 2026-09-15 rule is the opposite ("fixing the language design and implementation, not the use of it"). The old body's last commit is a workaround for its own runtime.
4. The "no refusals" direction and the "no silent recovery" lesson are compatible only if faults are loud, named and answerable inside the flow. That is a design obligation on the new grammar's `fail`/`stop`/`choice`, not something this study verified can be met.
