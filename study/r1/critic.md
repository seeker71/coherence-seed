# Completeness critique of the Understand round

Written 2026-11-24 by the completeness critic. Twelve reports were requested and twelve arrived; I read all twelve
whole (7,136 lines, 640 KB): `nums-core`, `nums-model`, `axioms-north-star`, `form-language`, `bml`, `runtime-jit`,
`lessons`, `web-bootstrap-language`, `web-jit-and-performance`, `web-knowledge-representation`, `web-numeric-root`,
`web-semantics-machines`.

## 0. Method, paths, status words

- `R` = `coherence-kernel` (HEAD `b8ff978c9`,
  2026-11-24 18:07 +0800; `git status --short` empty before and after).
- `N` = `NUMS.Go` (HEAD `9e6bbb4`, 2023-04-05; `git status --short` shows only the two
  pre-existing `.DS_Store` files after my checks).
- `S` = the session scratch directory of the round (not kept)
- `T` = `Coherence-Network/docs/field/urs/artifacts/master-thesis-2000`
- Status words: **observed** = I opened the cited line, ran the command, or fetched the URL in this session;
  **inferred** = my reasoning, the basis is named; **assumed** = not verified.
- Web checks went through the same summarising fetch tool the study agents used. A web "confirmed" therefore means
  "the page, relayed a second time independently, says the same", not a byte comparison.
- I modified nothing in any repository. I ran one program: the scratch binary `S/numscopy/tracebin` that the
  `nums-core` agent built (it reads two files in the scratchpad and prints). Everything else was `git` read commands,
  `wc`, `grep`, `shasum`, the Read tool, and eleven web fetches of public pages.

## 1. Verdict in brief

1. The round is strong on what the old body **is** (form-language, runtime-jit, lessons, axioms-north-star are careful,
   mutually consistent on the big facts, and survive spot-checking almost without error).
2. The round corrects the request's premise in two places, and both corrections hold under my checks:
   NUMS.Go is a 2023 prototype that represents a Go subset in a numeric space and executes nothing; "BML" is four
   different things, and two of the named "proven" features (multiple inheritance; objects as data pointer plus
   interface pointer) were not seen running in any era.
3. Of 47 claims I spot-checked, 41 are confirmed, 4 are wrong or stale (none of them overturns a conclusion, one of
   them matters: NUMS does have a cell record), 2 are unverifiable as cited.
4. Five sources that bear directly on the request were opened by nobody (section 2, G1 to G5). One of them is an
   earlier in-house study of NUMS.Go with a runnable miniature, two directories away from a file a report did open.
5. The reports contradict each other on four design questions that sit close to the axioms: what `fail` is among the
   acknowledgment values, what a node id is, what ice / water / gas mean, and what the object model is
   (delegation or flattening). These are not ready to be decided by more reading; three need the owner.
6. Enough evidence exists to decide: one grammar with nodes as the only interchange; names as a lens over a name-free
   structure; identity from canonical content with mutation only in names; C as the bootstrap language with a line
   budget; the Apple-silicon JIT host contract; "heat changes speed, never meaning"; the recipe as the unit of
   compilation and caching; a first text format for the claim graph.

---

## 2. What is missing, thin or unread

Ranked by how much the request leans on it. Each gap states what I saw that shows it is a gap.

### G1. An earlier study of NUMS.Go, with a runnable miniature, was not opened (unread source)

`Coherence-Network/docs/field/urs/artifacts/nums-go-2023/` holds `README.md` (194 lines),
`study-notes.md` (584), `network-substrate-design.md` (264) and `mini-nums/` (`core.py` 355 lines, `calc.py` 125,
`test_mini.py` 165, `jsonschema.py` 42, `README.md` 73); last commit touching the folder `5fd7b8a0b`, 2026-06-15
(observed: `ls`, `wc -l`, `git log`). The `bml` agent read the sibling folder `master-thesis-2000` in full and quoted
its README line about `NamedCell{Recipe access, Base blueprint, Name, CTOR recipe}` as living "in another repository's
substrate" (bml.md 2.13, marked assumed). It is `N/nums/recipe.go:497-502` (observed).

I read only `README.md:1-60`. It (a) calls NUMS.Go "a working model", (b) states the trinity as Blueprint = ice,
Recipe = water, NamedCell = gas, (c) says the phase transitions are "encoded directly in the type relationships",
and (d) says "It supports 14 languages already wired through the parser.go dispatch". Point (d) is refuted by
`nums-core` (only Go has handlers; `N/nums/parser.go:451-461`) and the list it gives has 13 names. Point (b) is the
likely origin of definition E in form-language's ice/water/gas table. Whether `mini-nums` executes recipes, which
would be the only executable NUMS on this disk, is **unknown**: nobody ran `test_mini.py`.

### G2. NUMS has a cell record; `nums-core` says it has none (feature missed)

`nums-core` 0.1 and 8: "`Cell_t` ... is a two-field struct that nothing uses", "Cells: ... There is no value, no
instance, no state", "Blueprint and recipe are represented. Cell ... is not." `Cell_t` is indeed unused
(`N/nums/reflect.go:3-6`, observed). But `NamedCell_t { Recipe_t; Base BlueprintNodeID_t; Name string; CTOR
RecipeNodeID_t }` at `N/nums/recipe.go:497-502` is used by `N/nums/blueprint.go:73,93,172-184` (observed, grep). The
report describes the mechanism it belongs to (the `CTOR:<name>` recipe, 5.2) without naming it as the cell. Accurate
statement: NUMS represents a cell as a **declared named slot** (an access recipe, a base blueprint, a name, a
constructor recipe); it has no run-time instance or value. `nums-model` has the matching second-generation shape
(`CellDeclaration_i`, kinds Global / Parameter / Local / Register / Resource / Sync / Literal).

### G3. The old body's own natural-language lane was not opened (unread source)

`R/NORTH_STAR.md:110-113` (observed) names running files for "every tongue is a projection column":
`form/form-stdlib/nl-translate.fk` and `nl-lexicon-grow.fk`, and claims that "the source is native" and "sumber adalah
asli" intern to the same node. Files present and unread by every report (observed: `git ls-files`, `wc -l`):
`form/form-stdlib/nl-translate.fk` (70 lines, 2026-10-03), `nl-lexicon-grow.fk` (83), `nl-lexicon-data.fk` (19),
`core-lexicon.fk` (263), `core-lexicon-derived.fk` (137), `core-lexicon-domains.fk` (131),
`tests/nl-lexicon-grow-band.fk` (60), `cognition/nl-lenses-13.fk` (78), `cognition/concept-nl-semantic-13.fk` (94) with
a 10,001-line `concept-nl-semantic-13-omw.tsv`, `learn/nl-meaning-*.fk` (four files),
`form/form-samples/cross-modal/05-nl-to-recipe/nl-arithmetic-demo.fk` (191). I read `nl-translate.fk:1-22` only: two
tongue grammars as data over the one BMF engine, a lexicon normalising onto one pivot node.

Consequence: the natural-language half of the request rests, in this round, on prior art alone
(`web-numeric-root` section 3, whose verdict is "works inside a controlled abstract syntax; open-domain contested").
The body's own two-tongue demonstration was neither read nor run, so nobody can say how small its fragment is.

### G4. The old body's host-language grammars were not opened (unread source)

`R/form/form-stdlib/grammars/go-bmf.fk` (223 lines), `python-bmf.fk` (1,118), `rust-bmf.fk` (114),
`typescript-bmf.fk` (252), `prolog-bmf.fk` (135), all dated 2026-10-01 to 10-03 (observed). `bml.md` 2.15 lists them
"not read"; `form-language.md` 17 quotes a port table ("Python in-motion, TypeScript/Go/Rust seed"). So "one numeric
space under Go, Python, TypeScript, Rust, C, C++, Java" has this evidence in the round: NUMS for a Go subset
(recorded, half of it printable), a config worksheet for five more languages (`nums-model` 8), and these unread
files. No report put a second programming language through either reference.

### G5. The emitter written in the language was counted, not read (feature not inventoried)

The working reference for "JIT machine code written in the one grammar" is `R/form/form-stdlib/bml/jit-page.bml`
(396 lines), `form-asm.fk` (306), `form-lower.fk` (678 per runtime-jit), `form-macho.fk` (74), `bml/anf-lower.bml`,
`form-cli-jit.bml`. `runtime-jit` read the C host side line by line and gives the page ABI; it gives line counts for
the Form side and says it could not run the Form page lane (its open question 2). `web-bootstrap` read
`form-macho.fk:1-20`. Nobody listed: which node kinds `jit-page.bml` emits, how an AArch64 word is expressed in the
grammar, its register and frame convention, what it hands back to the walker. The new design's central mechanism has
a 400-line working ancestor that the round did not open.

### G6. Nothing BML was executed, and two named features have no running witness (claim not verified)

`bml.md` states it ran nothing. For the features the request calls proven:

- Multiple inheritance by delegation: the 2000 thesis marks the `delegate` inheritance attribute
  "[Not supported yet]" (`T/backtracking-model-languages.txt:656`, observed); the 2026 thesis-lane compiler has the
  code path but no fixture exercises it (bml.md 2.12, its own grep); `R/form/form-stdlib/tests/bml-full-class-model-proof.fk`
  (248 lines) was not read in full.
- Objects as (data, interface): defined in the companion thesis (`T/companion/sgb-bml-objects.txt:486,493,642-644`,
  observed) with the two-base call rule; the 2026 lowering uses one record plus a class id and one receiver
  (bml.md 2.13, inferred there from three lines, not run).
- The 2000 virtual machine sources under `~/Downloads/Angelic` (`BMCPU`, `Java`) were sampled (five
  files); the choice-point and undo implementation is known from documents only.
- `R/surface/core-class-surface.bml` (106 lines; "the core stdlib as its class surface ... generic classes
  Num<T>, List<T>, Cell<T>, Task<T>", lines 1-12 observed) was opened by nobody. It is the 2026 body's own statement
  of classes and generics for the core.

### G7. Sibling checkouts and an earlier rewrite were not opened (unread sources)

`ls ~/source` (observed) shows `coherence-kernel-rewrite`, `coherence-kernel-walk`,
`coherence-kernel-raw-loader`, `coherence-kernel-codex-floor`, `ck-pre`, `ck-turn`, `coherence-seed` (an additional
working directory of this very session), `Living-Codex-CSharp`, `ds4`, `<product checkout>`, `<product checkout>-evidence`. `lessons`
cites one memory note saying a rewritten evaluator was about 40 times slower. No report opened the rewrite itself to
learn why a previous restart did not carry. `<product checkout>` is a different product tree (top level: `Mentor.*`,
`MP-CodeCheckBin-*`; last commit 2026-04-25; I listed the top level only); the tree-sitter fork
`<tree-sitter-fork>` is absent (observed), and no C++ NUMS was found by anyone.

### G8. Ice / water / gas: inventoried, not resolved (thin)

`form-language` lists eight uses, `runtime-jit` three, `axioms-north-star` records that the axioms contain none, and
four reports each propose a new mapping (section 4, C5). Nobody asked the operational question the request implies
by "respected": in which state may a node be mutated, by whom, and which transitions need an offer. G1's README adds
one more definition nobody weighed. `grep -w` for "ice" or "gas" over `N` finds no file (observed): the vocabulary
is not NUMS's.

### G9. No measurement stands behind the new design's cost claims (claims not verified)

`web-semantics-machines` says so itself ("mostly counts of words and loads derived from published layouts").
Unmeasured on this machine: hashing cost per node for any candidate hash and width; a content-addressed store against
the present intern index (the live shared field holds 21.7 M nodes per form-language's probe: a ready benchmark);
choice-point stack against persistent store; dictionary passing against specialisation. `runtime-jit` could not run
the Form page lane, so the one in-language JIT has no speed figure from this round. Its own walker timings spread
2.2 to 4.0 s for one program.

### G10. GPU and matrix path (thin)

"Highest available host efficiency ... GPU" has: `runtime-jit` 8 (the Metal carrier is 2,326 lines of Objective-C,
programs are MSL strings written in Form), `lessons` 4.6 (self-declared thin; `CURRENT_FLOOR.md:491-999` unread),
and `web-jit` 11 (prior art; SME present on this chip). Nobody read how Form emits a GPU kernel, and nothing GPU was
run.

### G11. Concurrency (missing)

No report studied what the new kernel's unit of parallelism is. Facts scattered across reports: JIT write permission
is per thread (Apple page, confirmed below); the old body used one big-stack thread plus a 1 ms sampler; "cells run
in-process, parallelism is sharding" is filed as movable practice; the 2000 VM ran speculation exclusively.
`web-semantics` covers actors and one-shot continuations from literature. There is no local evidence to choose with.

### G12. Form features outside the core (thin, self-declared)

`form-language` 22 lists its limits: the walker arms, the collector and the JIT emitters not read line by line;
`format-arith.fk`, `channel.fk`, `wire-registry.fk`, `blueprint-registry.json` cited through documents. Also not
inventoried by anyone: the dialect sections `form.action`, `form.route`, `public`; the body's own `awk`, `path`,
`form-find` (lessons 4.9 names them as worth keeping); `cell-channel.bml`. "All the features of Form" is met for
the core language and not for the 974-file library.

### G13. The knowledge-graph format was designed, not exercised (thin)

`web-knowledge-representation` measured bytes only; no tokenizer, no local-model write test, `merge=union` untested
(its open questions 1, 2, 6). It was not compared with the body's own line-per-value "form-rows" shape
(lessons 4.11) or with FORMBIN2. I recomputed its example ids (section 3, K1): they are right.

### G14. Declared reading limits that remain

`CURRENT_FLOOR.md` 261-404 and 491-999; `AGENTS.md` 181-336 and 442-619; about 135 of 245 memory notes;
`grammars/bml.fk` beyond its comments and about 200 lines; Windows and Linux hosts read, not run; paper bodies behind
most abstracts in the three web studies.

### G15. Questions only the owner can settle (not a reading gap)

How integers arise from the three states (axiom 1 claims to generate them; no derivation exists); whether `1` or
`node` carries an ordinary result; whether "store" in the request means checkpoint (save / restore) or persistence;
which integer model is wanted; which of the "fixed" axiom texts is fixed (the wording changed on 2026-11-08 and the
file itself calls the count a resting point, `R/axioms/core-axioms.form:43,249-255`, observed).

---

## 3. Spot-checks

Verdicts: **confirmed**, **wrong** (with correction), **stale** (true of the cited file, false at HEAD),
**unverifiable**. All observed by me on 2026-11-24.

### Axioms and control core

| # | Report | Claim | Checked against | Verdict |
|---|---|---|---|---|
| A1 | axioms-north-star 1.1 | The five statements, quoted | `R/axioms/core-axioms.form:47-93` | confirmed, word for word |
| A2 | axioms-north-star 1.2; lessons RC4 | Near-miss: a fold answered 2097151 with exit 1, the same number as the healed chain | `core-axioms.form:219` | confirmed |
| A3 | axioms-north-star 1.3 | Two named fold points; "we hold five" as a resting point | `core-axioms.form:249-255` | confirmed |
| A4 | form-language 19 | The axioms file names `form/form-stdlib/offer-ack-core.fk` "in BOTH homes" but only `control/` exists | `core-axioms.form:182-183`; `git ls-files \| grep offer-ack-core` gives `control/offer-ack-core.fk` and the band only | confirmed (the axioms file carries a dead path) |
| A5 | form-language 7; bml 2.3; axioms-north-star T4 | `oac-failed?` and `oac-stopped?` are both `oac-nothing?`; the core is 66 lines; `oac-choice` falls through on a caught fail | `R/control/offer-ack-core.fk:26-35`; `wc -l` = 66 | confirmed |
| A6 | axioms-north-star T5 | `minimal-surface.fk` says "choice pops on fail = ack-0" and `ms-choice` falls through on `a == 0` | `R/form/form-stdlib/minimal-surface.fk:14,32` | confirmed |
| A7 | form-language 6; axioms-north-star T16 | `substrate-phase.fk`: gas 0, water 1, ice 2, plasma 3; "STATE ... orthogonal to SUBSTANCE"; "never MUTATES" | `R/form/form-stdlib/substrate-phase.fk:4-11,17-21,36` | confirmed |
| A8 | form-language 6 (row E) | `living-equation.fk` maps Blueprint to ice and Recipe to water | `R/form/form-stdlib/grammars/living-equation.fk:7-13` | confirmed |
| A9 | axioms-north-star T8 | Goal file concedes "never grows read false": 25,483 lines then 31,066 | `R/docs/rent-to-zero-goal.form:233-236`; `git show 7d5161645:runtime/fkwu-uni.c \| wc -l` = 25483 | confirmed (commit date is 2026-09-15; the file's text says 2026-09-18) |

### Runtime and JIT

| # | Report | Claim | Checked against | Verdict |
|---|---|---|---|---|
| R1 | runtime-jit 1; form-language 1; lessons 1 | Seed is 34,149 lines; `membrane-boot.c` 1,199; `membrane-boot.h` 169; 400 commits touch the seed | `wc -l`; `git log --oneline -- runtime/fkwu-uni.c \| wc -l` | confirmed |
| R2 | form-language 1 vs runtime-jit 8 | Op table: 221 ops + 66 rewrites (form-language) or 223 + 64 (runtime-jit) | `R/runtime/fkwu-optable.h`: `fk_optab` rows on lines 8-228, `fk_rwtab` from 234; 287 rows total | form-language confirmed; runtime-jit **wrong** by two (221 + 66) |
| R3 | form-language 5.2; runtime-jit 3.2 | NodeID word: 1 mode bit + package 6 + level 13 + type 12 + instance 32; mode clear = immediate 63-bit int | `R/runtime/fkwu-node-word.h:6-14,24,28-31` | confirmed |
| R4 | form-language 6 (row A); runtime-jit 4 | In the runtime, ice / water / gas are the three *homes* of the node population (shared field, per-pid store, private heap) | `R/runtime/fkwu-uni.c:22929-22943` | confirmed |
| R5 | runtime-jit 5.1; web-bootstrap 2.3 | One page birth: `mmap(..., 0x7, 0x1802 on Apple ...)`, per-thread write toggle, `__builtin___clear_cache`; "the shrink path is none" | `fkwu-uni.c:4905-4929` | confirmed |
| R6 | runtime-jit 1 | "BOUNDED -- do not grow this into a full C flattener" stands in the seed | `fkwu-uni.c:23279-23282` | confirmed (it is the banner of the source-runner section, not the file's header) |
| R7 | runtime-jit 5 | `FK_F64_HEAT 1024`, `FK_JIT_HEAT 4096`, `FK_JIT_TIME_SAMPLES 32`, `FK_JIT_ABI 4`, 256 tag arms | `fkwu-uni.c:665,1034,10254,10281,10299` | confirmed |
| R8 | runtime-jit 8 | Seed was 1,954 lines on its third day and could already read Form source | last commit of 2026-06-30 (`766519b6c`) is 1,954 lines and contains `fk_sparse` at line 842 | confirmed (I did not build or run that version) |
| R9 | lessons 1.1 | Seed "2026-07-01: 8,821 lines" | last commit of 2026-07-01 is 1,977 lines; of 2026-07-02, 8,814 | **wrong date by about a day** (time zone or sample); the jump coincides with the 2026-07-02 import of 2,576 files, so it was an import, not organic growth |
| R10 | runtime-jit 11 | `membrane-boot.h`: word is 0, 1, nothing, node; "C does not evaluate a recipe"; "There is no arithmetic in this file"; identity is canonical bytes, a digest only indexes | `R/runtime/membrane-boot.h:5-37` | confirmed. Not surfaced by any report: `:15-16,29` "Held work is not a word: offer sets arrived=0 and writes no ack", and `:26-27` "An unoffered interface acks state 0 ... and records a breach row" (see C3) |
| R11 | bml 2.18 vs runtime-jit 2 | BML lowering runs "in a long-lived child process over a pipe" (bml) or "as a resident cell of this process" (runtime-jit) | `R/form/form-stdlib/bml-floor-compile.fk:1-14` says child; `fkwu-uni.c:33187-33194` says "used to begin a resident `fkwu` ... over two pipes ... Nothing is begun: no pipe, no fork"; `R/docs/bml-direct-nodes-jit.md:7` agrees with the C | bml **stale** (it trusted a file header the C has outgrown); runtime-jit confirmed |
| R12 | bml 2.18; runtime-jit 7.2, 10.1 | 59,464-byte door, 201,998 bytes lowered, 38,391,347-byte image, "645 times its source", closure of 351 units; 0.37 s per MiB cold | `R/docs/bml-direct-nodes-jit.md:5-8` | confirmed |
| R13 | form-language 5.2 | Composite identity is local to a kernel; the digest, not the local id, crosses a wire | `R/docs/native-form-artifacts.md:5-6`; `R/axioms/kernel-self-composition.form:91` | confirmed in documents (I did not re-run the probe: it writes into the host's shared field) |
| R14 | lessons RC2; runtime-jit 10.1 | A closure over locals goes wrong at its 1,024th call; "The kernel fault itself is not repaired" | memory note `reference-hot-closure-over-locals-stops-at-heat-mark.md:11-17`, modified 2026-11-24 | confirmed as a record |
| R15 | lessons 4.10 | With the sibling kernels gone, a value fkwu gets wrong "has no second reader" | `R/CURRENT_FLOOR.md:64-68` | confirmed |
| R16 | lessons 1 | 4,775 commits; 974 `.fk`, 1,105 `.bml`, 73 `.jsonl`, 584 band files | `git rev-list --count`, `git ls-files \| grep -c` | confirmed, all five |

### NUMS.Go

| # | Report | Claim | Checked against | Verdict |
|---|---|---|---|---|
| N1 | nums-core 2.1 | ID is `Package.Level.Type.Instance`, uint16 / uint16 / uint16 / uint32; `DefaultPackage` is 1 | `N/nums/nums_nodes.go:11-24,55-57` | confirmed |
| N2 | nums-core 3 | A minted id is `len(Entries)+1` per level with package 1: an arrival-order ordinal, not a hash; `Update_Entry` overwrites in place | `N/nums/nums_db.go:55-85` | confirmed |
| N3 | nums-core 2.2 | For a composite, level is one more than the largest child or category level | `N/nums/nums_nodes.go:814-830`; my run shows level-6 bodies over level-5 and level-3 children | confirmed |
| N4 | nums-core 7.1; nums-model 9.1 | Does not build as checked in: `replace ... => ../../<tree-sitter-fork>` in `go.mod`, `use ../<tree-sitter-fork>` in `go.work`, directory absent | `N/nums/go.mod:5`, `N/go.work:8`, `ls ~/source` | confirmed (I did not re-run `go build`) |
| N5 | nums-core 7.3; nums-model 9.2 | `<PREFIX>_ASSERT_` and `DEBUGBREAK_` only log | `N/nums/debugging.go:5-18` | confirmed |
| N6 | nums-core 6.4 | Renaming every identifier leaves the node databases identical; numbering varies run to run | re-ran `S/numscopy/tracebin` six times each on `tiny/tiny.go` and `tiny/renamed.go`, hashing the two database sections: tiny 6 of 6 one digest; renamed 4 of 6 that same digest, 2 of 6 a second | confirmed, both halves |
| N7 | nums-core 0, 8 | "There is no value, no instance, no state"; cell "is not" represented | `N/nums/recipe.go:497-502` `NamedCell_t`, used in `blueprint.go` | **wrong as stated**: a cell is represented as a declared named slot; there is no run-time instance (G2) |
| N8 | nums-core 5.1 vs web-numeric-root 0 | Thirteen languages enumerated (nums-core); "twelve language ids" (web-numeric-root) | `N/nums/parser.go:31-47` has 13 (with Verilog); `N/http/model/language.go:15-43` switches over 12 | both confirmed for the file each cites; no contradiction |
| N9 | nums-model 4 | `labels.yaml` 298 lines, `Go.yaml` 282 lines | `wc -l` | confirmed (237 labels not recounted) |
| N10 | nums-core 1 | 60 commits; `nums/` holds 7,045 lines of Go; no tests in `nums/` | `git rev-list --count HEAD`, `wc -l nums/*.go`, `ls nums/*_test.go` | confirmed |

### BML, 2000 and 2026

| # | Report | Claim | Checked against | Verdict |
|---|---|---|---|---|
| B1 | bml 2.1, 2.2 | Thesis definitions of `choice` and `fail` | `T/backtracking-model-languages.txt:625-626,633-634` | confirmed |
| B2 | bml 2.12 | "only single structural inheritance"; more parents means delegation; `delegate` attribute "[Not supported yet]" | same file `:234`, `:656` | confirmed |
| B3 | bml 2.13 | Reference = object id + interface id + native flag; two bases on every call (behavioural, structural) | `T/companion/sgb-bml-objects.txt:486,493,642-644` | confirmed |
| B4 | bml 2.1 | The 2000 test puts `fail;` after a completed `choose`, so choice was re-entrant | `~/Downloads/Angelic/src/Test/Choice.bml` (17 lines) | confirmed as source; the re-entrant reading stays inferred (not run) |
| B5 | bml 2.4 | Fixture pins three cut cases and `Guard` | `R/form/form-stdlib/tests/fixtures/bml-choose-cut.bml:20-24` | confirmed as text (not run) |
| B6 | bml 1.2, 2.2; form-language 7 | In the live dialect `fail` and `stop` are spellings of `nothing`; the grammar reads only `choice` | `R/form/form-stdlib/grammars/form-bml.fk:52,269` | confirmed |
| B7 | bml 1.1 | BML 2000 "does not include a mechanism for unification" | `T/companion/bml-search-algorithms.txt:246` | confirmed |

### Web sources (refetched)

| # | Report | Claim | URL | Verdict |
|---|---|---|---|---|
| W1 | web-bootstrap 2.1; web-jit 10 | `allow-jit` needed only with the Hardened Runtime; then one `MAP_JIT` region; always `sys_icache_invalidate`; allowlist path via a C macro; toggle "isn't available on iOS" | https://developer.apple.com/tutorials/data/documentation/apple-silicon/porting-just-in-time-compilers-to-apple-silicon.json | confirmed, all five quotes |
| W2 | web-bootstrap 5 | Zig, 2022-12-07: about 4,000 lines of C replace 80,000 of C++; `zig1.wasm.zst` 637 KB; `zig3` and `zig4` identical | https://ziglang.org/news/goodbye-cpp/ | confirmed; the "22 WASI imports" count is **unverifiable** (the page lists imports numbered 0 to 22 and states no count) |
| W3 | web-jit 7 | Maru: only `boot-eval.c` is C; "The whole thing is 1750 lines"; about 30% of optimised C; regenerates an identical `eval.s` | https://piumarta.com/software/maru/ | confirmed; caveat: on the page the 1,750 figure describes the *original version*, so it is weaker as a size target for "evaluator plus compiler plus boot" than web-jit 15 uses it |
| W4 | web-bootstrap 3.3 | Go issue 20123: "We don't support JITed code", "unexpected return pc", closed the day it opened | https://github.com/golang/go/issues/20123 | confirmed (2017-04-25) |
| W5 | web-numeric-root 4 | SEP many-valued logic entry dated 2026-01-17, revised 2026-09-29; Kleene's middle value as undefined output | https://plato.stanford.edu/entries/logic-manyvalued/ | confirmed as relayed |
| W6 | web-knowledge-representation 3.2 | KG-LLM-Bench tokens and scores for five textualisations; pseudonyms change the score by 0.2 points | https://arxiv.org/html/2504.07087v1 | confirmed, every number |
| W7 | web-jit 1.2 | PEP 836, created 2026-07-02, Withdrawn, resolution 2026-10-19; targets 5% and 20% | https://peps.python.org/pep-0836/ | confirmed |
| W8 | web-jit 12; web-numeric-root 2.1; web-KR 2.6 | Unison: hash of the syntax tree, 512-bit SHA3, names outside the hash, a cache never invalidated | https://www.unison-lang.org/docs/the-big-idea/ | confirmed |
| W9 | web-jit 1.2 | Python 3.15 released 2026-10-09; JIT 7-8% and 11-12% | https://docs.python.org/3.15/whatsnew/3.15.html | release date confirmed; the percentages are **unverifiable** in my fetch (they lie past the first 100,000 characters) |
| W10 | web-semantics 2.1 | Go 1.27 (August 2026) adds generic methods; interface methods may not declare type parameters | https://go.dev/doc/go1.27 | confirmed |
| W11 | web-semantics 1.1 | Verse calculus: an expression "evaluates to zero, one, or many values; or it can get stuck"; all effects "must be transactional" | `S/study/pdftext/verse.txt:248,1450-1451` (the agent's own text copy of the paper) | confirmed |

### Experiments left by the agents

| # | Report | Claim | Checked against | Verdict |
|---|---|---|---|---|
| X1 | web-bootstrap 2.2, 3.1-3.3 | JIT matrix across signing states; 21 ns toggle against 454 ns `mprotect`; cgo 23.6 ns; a Go collection held 627 ms behind JIT code against 0.31 ms | recorded output `S/study/exp/out.txt:1-34`, `out2.txt:1-28` | recorded output matches the report line for line (not re-run) |
| X2 | web-bootstrap 0; web-jit L1 | go1.27.2, rustc 1.99.1, Apple clang 17.0.0 on this machine | `go version`, `rustc --version`, `cc --version` | confirmed |
| K1 | web-KR 5.2-5.3 | Example ids `c808c09b`, `25d9086d`, `7200e47a`, hub `91673052` are SHA-256 prefixes of the block bodies; 333 / 365 / 407 / 601 bytes | `shasum -a 256 S/study/kgprobe/*.body`; `wc -c` | confirmed |

Tally: 47 checks. Confirmed 41. Wrong or stale 4 (R2 runtime-jit's op split, R9 lessons' date, R11 bml's child
process, N7 nums-core's "no cell"). Unverifiable as cited 2 (W2's import count, W9's percentages). No report was
found fabricating a citation.

---

## 4. Contradictions between reports

**C1. What `fail` is among the four acknowledgments.** `web-semantics` R1: "0 is an answered no (fail, safe to
backtrack on), nothing is no answer"; fail = ack 0. `axioms-north-star` T5 and its let-go list: drop "`0` as fail";
fail is the nothing arm. The running core agrees with the second (`oac-fail() = fail`, lowered to `nothing`; A5); the
minimal surface agrees with the first (A6). `web-numeric-root` 4 adds a third reading (nothing as lattice top, fail
as bottom, distinct). The two recommendations cannot both enter the design.

**C2. What a node id is.** `nums-core` and `form-language` carry forward the four-field coordinate; `lessons` D4 puts
the value kind inside it; `axioms-north-star` lets the layout go "as if it were axiom text"; `web-numeric-root` R1 and
`web-KR` 5.5 make identity a content hash and leave coordinates for in-process handles. Underneath, the two references
disagree with each other on every field: Level is tree height in NUMS (N3) and the category's level in fkwu
(form-language 5.2); Package is 0 for the fixed catalogue and 1 for minted nodes in NUMS (N2), and 1 for the body with
composites at 0 in fkwu (form-language's probe, `@0.2.99.n`); Type is always 0 for minted NUMS nodes. No report set
these side by side.

**C3. How many outcomes an offer has.** The axiom and most reports say exactly four. `web-semantics` 1.2 treats
"not yet" as part of nothing. The greenfield carrier in the tree keeps a fifth situation outside the word: "Held work
is not a word: offer sets arrived=0 and writes no ack", and distinguishes "an arrived nothing" (R10). `axioms-north-star`
T4 counts six meanings inside nothing. Whether not-yet is a value, a missing value, or a reason beside a value is
answered three ways.

**C4. Is compiled native code ice or gas?** Old body: a hot composite "crystallizes into a callable native leaf,
melts on cool" (`R/axioms/host-kernel.form:28-31,206-211`, observed), so compiled code is the solid. `web-jit` 9:
ice is the stored node form and stencils, water is baseline machine code, gas is speculative optimised code that
"can evaporate". `web-semantics` R16 and `lessons` D8: ice = frozen and content-addressed, water = live mutable in a
process, gas = transient. `web-KR` 5.10: committed text / derived index / in-context. Four incompatible axes
(storage home, mutability, compilation tier, circulation), plus the trinity mapping of G1.

**C5. Object model.** `bml` 4.1 and 5.7: keep the reference pair *and* the two-base rule, with multiple inheritance
by delegation to shared base objects. `web-semantics` R9-R12: fat pair with one data pointer, parents composed by
flattening into a new shape with explicit conflict resolution, "No subobjects". Delegation with a moving `this` and
flattening are different semantics (shared base state exists in one and not the other).

**C6. Choice by default.** `bml` 4.1: the general primitive is re-entrant choice with commit. `form-language` and
the running body: first non-absent value, scoped. `web-semantics` R2/R5: ordered choice with a consuming delimiter
on a choice-point stack. These can be one design, but the default and the cost model are stated three ways.

**C7. Where the literal value lives.** NUMS keeps literal values outside the tree, per recipe (nums-core 6.4: `a + 1`
and `b + 2` are one node). `web-numeric-root` R1 removes names only. `web-semantics` R17 hashes "(kind number, child
ids, literal bytes)". If literals are outside, a recipe's identity is the pair; nobody chose.

**C8. The seed's form and size.** `runtime-jit` 12: store the compiler as canonical numeric cells, no parser in C,
about 1,400 to 1,600 lines. `web-jit` 15: a reader for the numeric node form plus a plain evaluator, "a few hundred to
a couple of thousand". `web-bootstrap` 8: under about 3,000 lines with "a tiny evaluator or copy-and-patch tier"; but
`web-jit` 1.1 shows copy-and-patch needs an external optimiser at build time, which conflicts with a minimal stored
bootstrap. The three agree on C and on a budget, not on the number or on what the seed is made of.

**C9. Store.** `web-numeric-root` R12: "a compact database, not a file per node" from the start. `web-KR`: plain text
blocks under git as the stored form of the graph. `runtime-jit` option (c): a diffable serialisation of cells in git.
Compatible only if the claim graph and the node store are different things; the request says the graph should later
be nodes in the one grammar.

**C10. A second reader.** `lessons` RC1 ranks hand-kept second implementations as the first cause of fragility and
D11 then asks for "one small independent conformance reader"; `runtime-jit` 12 wants the bootstrap walker to be the
only second implementation and preferably retired. `lessons` flags the tension itself.

**C11. Smaller factual disagreements.** Op split 221 + 66 against 223 + 64 (R2, resolved). Seed size on 2026-07-01
(R9, resolved). Lowering in a child against in-process (R11, resolved). Seed compile time 3.6 s (memory note,
2026-11-09), 5.52 s (`-c` only, web-bootstrap), 8.5 s (full link, runtime-jit): three conditions, no conflict, but the
graph should carry one with its condition. Binary 711 KB (2026-11-09) against 745 KB today: growth, not conflict.

**C12. Premise against findings (not report against report).** The mission text calls NUMS.Go and BML "100% working";
`nums-core`, `nums-model` and `bml` each find otherwise and agree with one another. The G1 README, written inside the
owner's own repository, calls NUMS.Go "a working model" and overstates language support. The graph should carry the
premise and the finding as two claims with their sources, not pick one silently.

---

## 5. Decisions: what the evidence now supports, and what it does not

### 5.1 Ready to decide

Each line: the decision, then the evidence that makes it ready.

1. **One grammar, one reader; nodes are the only interchange inside the system; text is a lens at the edge.**
   Lessons RC1, RC5, RC9; bml 1.2 (three BML readers with different meanings for the same words); R12 (a 645-fold
   image because strings are the seams). No report argues the other side.
2. **Names are a detachable lens over a name-free structure, and user-chosen names are lens entries too.** N6
   (re-run by me), W8 (Unison), `core-axioms.form:72-74,104`. Carry the stated limit: text to node is a choice that
   may return several candidates or nothing (web-numeric-root 5.2).
3. **Identity is computed from a canonical, name-free encoding; a node is immutable; mutation lives only in name
   bindings; an id must never be an arrival-order index.** N2 and N6 (NUMS ordinals differ between runs), R13 (fkwu
   composite ids are field indices), R10 (the membrane carrier already states "Equality is the bytes"), axiom 3.
   The *shape* of the id is not ready (5.2, N2).
4. **Bootstrap in C: one translation unit, a declared list of OS symbols, a host-door table, and a line budget that a
   count enforces.** Three reports converge independently (runtime-jit 13, web-bootstrap 8, web-jit 14). Evidence
   checked: W1, W2, W3, W4, X1, R5, and the seed's growth from 1,954 to 34,149 lines under a written shrink rule (R1,
   R8, A9). The flip conditions in web-bootstrap 9 belong in the graph beside the decision.
5. **The Apple-silicon JIT host contract from the first line: `MAP_JIT`, per-thread write toggle, icache flush, one
   reserved region, correct with and without the Hardened Runtime.** W1, X1, R5.
6. **An unresolved name or an unparsable text is a reported fault, never `nothing` and never a value; a result
   crossing a boundary is one structured value.** A2, lessons RC4 (eight shapes of numb green), form-language 8.
7. **Heat changes speed, never meaning: one operation definition shared by the interpreter and the emitter, strict
   argument order, one tier-up mechanism, and a cold-equals-hot check in the bootstrap's own self-test.** R14 (a fault
   of this class recorded today, unrepaired), lessons RC2, R7.
8. **The unit of compilation and caching is the recipe, keyed by its content id, the generator and the target.**
   R12, lessons RC6; the old body's own destination (`bml-direct-nodes-jit.md:10`, observed).
9. **A first text format for the claim graph: immutable `key: value` blocks, content-derived ids, supersession by
   addition, derived views.** K1, W6, web-KR 3 and 5. Ready as a starting format because every block is regular and
   can be re-minted mechanically; its open points (does the hash cover provenance; later coordinates) do not block
   the first nodes.
10. **Record the premise corrections as claims.** NUMS.Go: idea demonstrated for a Go subset, not executable, not
    building as checked in, crashes on its three shipped samples (N4, N5, N6, N7 corrected). BML: four things; the
    2026 control words are thin expressions over `nothing` and one recover point (A5, B6); multiple inheritance and
    the reference pair have no running witness (B2, B3, G6).
11. **Host languages enter as namespaces of blueprints with lens tables as data, not as one universal construct
    set; the natural-language claim is staged last and kept separable.** web-numeric-root 2.5 and R13; nums-model 4
    (labels plus a per-language table); G3 and G4 limit how far this can be claimed today.
12. **The acknowledgment set stays closed at nothing / 0 / 1 / node, with one offer act, and answered is counted
    apart from silent.** A1, A5, R10. What each value *means* is not ready (5.2, N1).

### 5.2 Not ready

1. **N1. The meaning of the four acknowledgments**: fail as 0 or as nothing (C1); where the reason for a nothing
   lives; whether not-yet is a value (C3); whether an ordinary result is `1` with a payload or a `node`. This reads
   the axioms; it needs the owner, then a small running model.
2. **N2. The node id's shape**: coordinates, digest, or a handle onto a digest; what Level, Package and Type mean
   (C2); width; hash function; literals in or out (C7); the rule for cycles and self-reference; how a nominal
   distinction is expressed. Needs a side-by-side table of the two references and one measurement (G9).
3. **N3. Ice / water / gas** (C4, G8). Needs one definition chosen and the transitions stated as operations.
4. **N4. Choice**: default strength, cut's scope, and the reversibility declaration per effect (C6). Needs the 2000
   machine read or a model run (G6).
5. **N5. Object model**: delegation with two bases, or flattening (C5). No running reference exists for either in
   this body.
6. **N6. The seed**: source read by a C reader, a cell image, or canonical numeric cells; who regenerates it; whether
   the bootstrap walker survives; whether a second reader exists (C8, C10). G5 should be closed first.
7. **N7. JIT beyond the baseline tier**: trace or method; rewrite engine; stencils without an external optimiser
   (unproven anywhere, web-jit 16.1); relocation and persistence of native code; root finding in native frames.
8. **N8. Numeric tower and machine-level blueprints**: integer width and overflow, float kinds, layout precise
   enough to emit code (nums-core 9.6; form-language 9; lessons Q5).
9. **N9. Natural language**: scope of the first lens and what counts as success (G3; web-numeric-root C2).
10. **N10. Store and reclamation**: database, files, or both; reference counts or tracing; reclamation in a shared
    store (C9; lessons RC7).
11. **N11. Concurrency** (G11).
12. **N12. The verification budget** and whether an independent reader is in it (C10).
13. **N13. The GPU tier** (G10).

### 5.3 The shortest path to make the not-ready ready

Inferred. Six acts, none large: (1) read G1 whole and run `mini-nums/test_mini.py`; (2) read G5's four files and
write the emitter's feature list; (3) read G3 and G4 and state the fragment each covers; (4) put NUMS's and fkwu's
id fields in one table; (5) time one hash candidate over the nodes of one real unit; (6) put N1 to the owner as one
question with the three readings and their sources.

---

## 6. Surprise, and what stayed uncomfortable

Most surprising: the round disproved the "100% working" premise for NUMS.Go carefully and by experiment, and in the
same pass missed that NUMS has a cell record (`NamedCell_t`), and missed an earlier study of NUMS.Go with a runnable
miniature that sits two directories from a README one agent quoted. That unread README is also where the
blueprint = ice, recipe = water, cell = gas mapping is written down, which several reports then met downstream as an
unexplained definition.

Uncomfortable:

1. Two reports give opposite answers to "is fail a 0 or a nothing", each with a running file behind it, and the
   question sits on axiom 5, the one thing declared fixed.
2. The two references disagree on the meaning of every field of the id they share by name, and the axiom block
   carries that same four-word layout.
3. My web checks used the same summarising tool as the reports. Agreement between two relays is weaker than it looks.
4. I confirmed measurements by reading recorded output (X1), not by re-running them, and I did not run any Form or
   BML program: a run writes into the host's shared node field, and I chose not to add to it.
5. "Multiple inheritance" and "objects with a data and interface pointer" are named in the request as proven. I
   found no run of either, in 2000 or in 2026, in anything the round or I opened.

## 7. What I did not do

- I did not read G1 beyond `README.md:1-60`, G3 beyond `nl-translate.fk:1-22`, or G4, G5, G7 at all; I established
  that they exist, how large they are, and that no report opened them.
- I did not recount the 237 labels, re-run `go build` in `N`, rebuild the seed, or re-run any timing.
- I did not check claims from the roughly 110 memory notes beyond one (R14).
- Seven further web claims I considered (Cranelift's percentages, the Wasmtime hardened-runtime issue, Deegen's
  numbers, the Abstract Wikipedia request for comment, TOON's benchmark, the STEPS line counts, the seL4 white
  paper's date) were not refetched.
