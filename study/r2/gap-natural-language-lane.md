# Gap G3: the old body's natural-language lane

Written 2026-11-24. Closes gap G3 of `critic.md` (section 2, "The old body's own natural-language lane was not
opened"). Builds on `web-numeric-root.md` section 3 (prior art: GF, Abstract Wikipedia, AMR/UMR/UNL, WordNet/CILI,
NSM) and on `form-language.md` section 5.2 (how `intern_node` gives identity); neither is redone here.

## 0. Paths, method, status words

- `R` = `coherence-kernel`
  (HEAD `b8ff978c9`, 2026-11-24).
- `S` = `scratch`
- **observed** = I opened the cited line or ran the cited read-only command in this session.
  **inferred** = reasoning from observed lines; the basis is named. **assumed** = not verified.
- Dates are `git log -1 --format=%cs -- <file>` unless a `git blame` date is given.
- I ran no Form or BML program and did not start `fkwu`. Every "verdict" number below is a number **recorded in a
  file**, not a number I saw produced. I ran `git` read commands, `grep`, `wc`, and host `awk`/`sort` counts over
  two data files; the count work files are in `S/nl-lane-work/`. I modified nothing in `R`.
- Read whole: `nl-translate.fk`, `nl-lexicon-grow.fk`, `nl-lexicon-data.fk`, `pivot-coverage.bml`,
  `core-lexicon.fk`, `core-lexicon-derived.fk`, `core-lexicon-domains.fk`, `core-grounding.fk`, `meaning-codes.bml`,
  the four bands that stand, `nl-lenses-13.fk` and its data, `concept-nl-semantic-13.fk` with runtime, metadata
  and manifest, `language-neutral-symbol-identity.fk`, `language-neutral-inquiry-induction.fk`,
  `form-neutral-word-field.fk`. Read in part (lines named where cited): `core-frequency-lift-dictionary.fk`
  (1-45, 160-195, 250-339 of 470), `core-frequency-lift-form-neutral-crossing.fk` (1-141 of 142),
  `response-word-field-dictionary.fk` (1-50 of 128), `pl-lenses-13.fk` (1-40), `bmf-grammar.fk` (1-60, 1090-1131
  of 1,138), `bmf-core.fk` (88-108, 326-373), the four `learn/nl-meaning-*.fk` heads, `sanskrit-locale-baseline.fk`
  (1-40), the model-side heads. Not read: `ear-tongue.fk` past line 24 (it is a speech-model lane, not a pivot
  lane), `model/form-neutral-language-world-model.fk` past line 30, its 268-line band past line 16.

## 1. Verdict in brief

1. There is not one natural-language lane. There are **seven separate "neutral meaning" schemes**, each with its
   own id space, and no file joins any two of them except one crossing cell that joins three by hand for one
   meaning (section 8).
2. The lane NORTH_STAR names (`nl-translate.fk`) is 70 lines: **two tongues** (English, Indonesian), **one
   sentence construction** ("the X is Y" / "X adalah Y"), **two one-word slots**, **52 lexicon rows**, ASCII
   letters only (section 3, 4).
3. The band that proved NORTH_STAR's own example sentence was **deleted on 2026-09-29, in the same commit that
   edited that NORTH_STAR line**. At HEAD one band (`pivot-coverage-band.fk`) still exercises the translator, on
   five other words. Its recorded verdict is dated 2026-09-28 (section 5).
4. Two tongues do reach one node, by construction: both lexicon directions return the same **symbol string**
   (`"n0"`), and the pivot is `intern_node(blueprint 1.2.99.33, [string "n0", string "n1"])`. The identity is over
   strings that are arrival-order labels. The file's comment says the symbol is "a door over the numeric NodeID
   (identity invariant under renaming)"; the code makes the label the content, so renaming a symbol changes the
   node (section 6).
5. A lexicon miss passes the surface word through unchanged into the pivot, and a parse failure returns the string
   leaf `"PARSE-FAIL"` as a value. Neither is a fault (section 3.5).
6. The 10,000-row "OMW" table is English-frequency-anchored, 57% machine-translated, 0 rows reviewed, 25% of rows
   without a synset, and its first rows show wrong senses (`i` -> iodine). Its band was deleted (section 7.4).
7. The piece worth carrying is not the translator. It is `cognition/core-frequency-lift-dictionary.fk`: a meaning
   as a content-interned recipe of numeric role edges rooted in the five axioms, surfaces as separate nodes, a
   mapping that carries context, provenance and a claim state, parse answering zero-to-many candidates, render
   starting from a chosen meaning (section 7.5).
8. On this repository's evidence, "the numeric space is the root of all natural languages" is a direction with a
   toy witness: one copular template in two tongues over 50 words per tongue. It is below the smallest thing
   `web-numeric-root.md` calls working (a controlled abstract syntax with data-driven linearisation), because here
   the generation side is hand-written code per tongue (section 10).

## 2. Inventory

### 2.1 What stands at HEAD (observed: `git ls-files`, `wc -l`, `git log`)

| File | Lines | Date | Role |
|---|---|---|---|
| `R/form/form-stdlib/nl-translate.fk` | 70 | 2026-10-03 (born 2026-07-02) | two grammars, seed lexicon, normalise, decode |
| `R/form/form-stdlib/nl-lexicon-data.fk` | 19 | 2026-10-03 | 40 `c` rows + 5 `m` rows |
| `R/form/form-stdlib/nl-lexicon-grow.fk` | 83 | 2026-10-03 | projector: two parallel JSON texts -> rows text |
| `R/form/form-stdlib/pivot-coverage.bml` | 119 | 2026-09-30 | counts corpus words with / without a pivot row |
| `R/form/form-stdlib/tests/pivot-coverage-band.fk` | 82 | 2026-10-01 | 16 bits; the only band calling `tr-en-id` |
| `R/form/form-stdlib/tests/nl-lexicon-grow-band.fk` | 60 | 2026-09-28 | 7 bits; the projector's pure fold |
| `R/form/form-stdlib/core-lexicon.fk` | 263 | 2026-11-15 | 64 English words defined from themselves |
| `R/form/form-stdlib/core-lexicon-derived.fk` | 137 | 2026-09-30 | 4 derived words |
| `R/form/form-stdlib/core-lexicon-domains.fk` | 131 | 2026-09-30 | 36 domain words |
| `R/form/form-stdlib/core-grounding.fk` | 66 | 2026-10-03 | status table derived / sensed / open (not a language cell) |
| `R/form/form-stdlib/meaning-codes.bml` | 166 | 2026-10-01 | 36 symbols, codes per tongue from row files |
| `R/form/form-stdlib/tests/meaning-codes-table-band.fk` | 137 | 2026-10-01 | 8 bits |
| `R/form/form-stdlib/locale-rows/symbol-*.rows` | 3 files | 2026-09-07 | 8 + 20 + 1 phrase rows |
| `R/cognition/nl-lenses-13.fk`, `nl-lenses-13-data.fk` | 78, 24 | 2026-10-03, 09-30 | 13 locales x 4 phrase meanings |
| `R/cognition/tests/nl-lenses-13-band.fk` | 31 | 2026-10-03 | 13 bits |
| `R/cognition/concept-nl-semantic-13.fk` (+runtime, metadata, offsets) | 94, 29, 8, 10,007 | 2026-10-03 | access to the table |
| `R/cognition/concept-nl-semantic-13-omw.tsv` | 10,001 | 2026-07-18 | 10,000 rows x (synset + 13 locales) |
| `R/cognition/concept-nl-semantic-13-sources.dat` | 130,000 bytes | 2026-07-17 | one provenance byte per cell |
| `R/cognition/core-frequency-lift-dictionary.fk` | 470 | 2026-10-07 | numeric meaning recipes, surfaces, mappings |
| `R/cognition/core-frequency-lift-form-neutral-crossing.fk` | 142 | 2026-09-30 | joins the table, the phrase packs, PL fixtures |
| `R/cognition/language-neutral-symbol-identity.fk`, `-inquiry-induction.fk` | 71, 57 | 2026-09-30 | alias rows; 4 meanings over 11 locales |
| `R/cognition/form-neutral-word-field.fk`, `response-word-field-dictionary.fk` | 105, 128 | 2026-09-30 | 13 authored axes; response words grouped |
| `R/learn/nl-meaning-{corpus,net,inquiry-learning,error-adaptation}.fk` | 29, 83, 53, 85 | 2026-09-30 / 10-07 | a 4-class trained classifier |
| `R/learn/sanskrit-locale-baseline.fk` | 78 | 2026-09-30 | 4 meanings (301-304) x 11 locale columns |
| `R/model/form-neutral-{language-world-model,token-space}.fk` | 212, 143 | 2026-09-30 | graph over anchors and recipes |
| `R/form/form-samples/cross-modal/05-nl-to-recipe/nl-arithmetic-demo.fk` | 191 | 2026-09-29 | 4 English sentences -> arithmetic recipes |

### 2.2 What the body had and removed (observed: `git show --diff-filter=D --name-only 115fea55e`)

Commit `115fea55e` (2026-09-29, "Cut the repo to its end-to-end healthy flow: 14,729 files to 1,628") removed,
among others: `form/form-stdlib/nl-tongues.fk` (207 lines: eleven more tongues for the same one construction,
seven meanings each; read at `115fea55e^`, lines 1-70), `tests/nl-translate-band.fk`, `tests/nl-many-band.fk`,
`tests/nl-many-latin-band.fk`, `translate-lane.fk`, `translation-engine.fk`, `translation-invariant.fk`,
`translation-learn.fk`, `translation-quality.fk` with their bands, `pivot-mint.fk`, `nl-emit.fk`, `nl-emitter.fk`,
`nl-reason.fk`, `locale-registry.fk`, `cognition/nl-pivot-de.fk`, `nl-pivot-es.fk`, `nl-grammar-induction.fk`,
`cognition/tests/concept-nl-semantic-13-band.fk`, the per-source licence files of the table, the
`core-lexicon-*-band.fk` files, and four `learn/tests/nl-meaning-*-band.fk` files. Commit `980de319e`
(2026-10-18) removed `nl-translate-band.fk` and `translate-lane.fk` again and restored `pivot-coverage-band.fk`.
Commit `824e7a0c7` (2026-11-10) removed `nl-wisdom-row.fk` and its band; `R/receipts/2026-08-04-wisdom-is-tended-pivot-row.md:16-23`
still describes both as landed. **Status: observed.**

Consequence (**inferred** from the two lists): the many-tongue shelf and nearly every band on this lane are
history. What stands is the two-tongue core, one coverage band, and data tables whose bands are gone.

## 3. The mechanism in `nl-translate.fk`

All line numbers in this section are `R/form/form-stdlib/nl-translate.fk` unless noted. **Observed** throughout.

### 3.1 A tongue's grammar as data

A grammar is a list value built by ordinary functions of the one grammar engine
(`R/form/form-stdlib/bmf-grammar.fk:51,58,1109-1131`): `grammar(start, rules)`, `rule3(name, pattern, template)`,
patterns `p-lit`, `p-ref`, `p-run(class)`, `p-cap(name, pattern)`, `["seq", ...]`, templates `t-emit(op, kids)`,
`t-splice(name)`. The whole of English (line 22):

```
grammar("s", [
  rule3("s",    p-cap("v", p-ref("prop")), t-splice("v")),
  rule3("prop", ["seq", p-lit("the"), p-cap("s", p-run("alpha")), p-lit("is"), p-cap("a", p-run("alpha"))],
                t-emit("property", [t-splice("s"), t-splice("a")]))])
```

The whole of Indonesian (line 23) is the same with `["seq", p-cap("s", ...), p-lit("adalah"), p-cap("a", ...)]`.
So a tongue's grammar is: two rules, one of them a fixed token template with two capture slots, emitting one node
kind `property` with two children. The class `alpha` is ASCII letters and underscore
(`R/form/form-stdlib/bmf-core.fk:330-333`). A wider class `letter` (ASCII letter or any byte >= 128) exists for
other scripts (`bmf-core.fk:352-359`); no file at HEAD uses it (observed: `grep 'p-run("letter")'` finds nothing).

### 3.2 The lexicon

Rows are four-element lists `[symbol, phase, en, id]` (line 46: "columns: 0=neutral symbol, 1=phase, 2=en,
3=id"). Seed rows, line 35: `n0 source/sumber`, `n1 native/asli`, `n2 kernel/inti` (phase `ice`), `n3 body/tubuh`,
`n4 cell/sel`, `n5 recipe/resep` (`water`), `n6 witness/saksi` (`gas`). The full lexicon is seed ++ corpus rows
(line 41), the corpus rows being `R/form/form-stdlib/nl-lexicon-data.fk:15-17`: `c0`..`c39` (phase `water`) and
`m1257`..`m1261` (phase `gas`).

Lookup is a linear scan for the first row whose column `i` equals the word, returning column `j`; **an unknown
word is returned unchanged** (lines 43-45: `nlt-pick(r, j, w) = nil?(r) ? w : nth(r, j)`).

### 3.3 The pivot

`norm-en(n)` (line 58) reads the two captured words out of the raw parse node, maps each through the lexicon to
its symbol, and builds

```
intern_node(bp("property"), [intern_trivial_string(symbol-of-slot-0), intern_trivial_string(symbol-of-slot-1)])
```

`bp` is a native operation (`R/runtime/fkwu-optable.h:37`); the name `property` resolves to the coordinate
`1.2.99.33` (`R/form/form-stdlib/form-ontology-bp.fk:943`, mirrored at `bmf-core.fk:104` with a comment at
`bmf-core.fk:93-102` explaining that the two hand-kept tables must agree or "children identical, identity never
meeting"). `intern_node` interns by content: same category and same children give the same node
(`S/form-language.md:110`, observed there by probe). So **the pivot of a sentence is one composite node whose
category is a numeric blueprint coordinate and whose two children are string leaves holding symbol labels.**

### 3.4 Back out in another tongue

`dec-id(p)` (line 62) reads the two children's strings, maps symbol -> Indonesian surface, and concatenates
`slot0 + " adalah " + slot1`. `dec-en(p)` (line 61) concatenates `"the " + slot0 + " is " + slot1`. The
translators (lines 67-68) are `tr-en-id(x) = dec-id(norm-en(g-parse(nl-en(), str-normalize(x))))` and its mirror.
**The decoder is a hand-written function per tongue. It does not read the grammar.** Parsing is data-driven;
generation is not. (Observed; the header's "Adding a tongue is adding a (grammar, lexicon-direction, decoder)",
lines 10-12, says the same thing without marking the asymmetry.)

`str-normalize` lowercases A-Z and drops `. , ! ? ; :` (`R/form/form-stdlib/string-case.fk:12-28`). It is
ASCII-only by its own header ("A 26-letter map").

### 3.5 Failure arms

- **Lexicon miss**: passes the surface through (3.2). "the cat is black" would intern
  `property("cat","black")`, English strings inside the pivot, and decode to "cat adalah black". **Inferred** from
  lines 43-45 and 58-62; not run. The removed receipt-era band knew this:
  `R/receipts/2026-08-04-wisdom-is-tended-pivot-row.md:21-23` describes two bits added "so a lexicon miss that
  passes a surface through unchanged cannot round-trip green in silence". That band is gone (2.2).
- **Parse failure**: `g-parse` answers `intern_trivial_string("PARSE-FAIL")`, a value
  (`bmf-grammar.fk:1094-1096`). `norm-en` then takes children of a leaf. What `tr-en-id` answers for a sentence
  outside the template was **not observed** (not run).
- **Trailing input**: `g-parse` is the prefix matcher; the whole-source door is `g-parse-full`
  (`bmf-grammar.fk:3-5,1094-1107`). The translator uses `g-parse`, so words after the second slot are not
  required to be absent. **Inferred**; not run.

This is the pattern `critic.md` 5.1 item 6 says to end ("An unresolved name or an unparsable text is a reported
fault, never `nothing` and never a value").

## 4. The exact fragment

**Constructions: one.** Copular predication with a single-word subject and a single-word predicate. No verb other
than the copula, no plural, tense, negation, question, modifier, multi-word slot, agreement or reordering
(observed: the two grammars, lines 22-23, are the whole grammar set).

**Tongues: two** at HEAD (English, Indonesian). Thirteen once shared this construction (2.2).

**Words** (observed counts over `nl-translate.fk:35` and `nl-lexicon-data.fk:15-17`, work file
`S/nl-lane-work/lex.txt`): 52 rows = 7 seed + 40 corpus + 5 minted. 50 distinct English words, 50 distinct
Indonesian words. Collisions: `home` is three rows (`c16` rumah, `c21` rumah, `c23` beranda); `karya` is two
(`c1` work, `c15` works); `rumah` is two. Because lookup is first match, `home` always answers `c16`, and
`works` -> `karya` -> `work` does not round-trip (**inferred** from first-match, lines 43-45). Seven rows have the
same spelling in both columns (`episode`, `podcast`, `film`, `video`, `album`, `email`, `keloid`).

**Where the 40 corpus rows came from**: single-word, all-ASCII-letter string leaves at the same key path in two
parallel UI message files (`nl-lexicon-grow.fk:33-56`; the files are `web/messages/en.json` and `id.json`, which
are not in this repository: `ls R/web` fails; they exist in the parent repository at
`Coherence-Network/web/messages/`, observed by `ls`). They are interface labels aligned by
key, not sense-aligned vocabulary. The runner that would regenerate the file "is still not built"
(`nl-lexicon-grow.fk:18-19`).

**Coverage of the body's own minted vocabulary** (`pivot-coverage.bml`): the homecoming corpus holds 1,070 rows
with 1,065 distinct fresh words (observed: `grep -o 'hdc-row([0-9]+,'` over
`R/learn/homecoming-distillation-corpus.fk`, 2026-11-20; ids 601..1683). Five of them have a lexicon row
(`afterwall`, `keloid`, `seatrace`, `seedshell`, `twicegrown`): 5 pivoted, 1,065 missing, 0.47% (**inferred**: my
static join of the two files, `S/nl-lane-work/`; not the lens's own run). The lens's field code
`words*10^6 + pivoted*10^3 + missing` is declared "injective while pivoted and missing each stay under 1000"
(`pivot-coverage.bml:23-25`); with 1,065 missing it no longer is (**inferred**).

## 5. Proven by a test versus asserted in prose

| Claim | Where asserted | Witness at HEAD | Status |
|---|---|---|---|
| "the source is native" and "sumber adalah asli" intern to the same node | `R/NORTH_STAR.md:112-113`; `nl-translate.fk:10` | **None.** It was bit 1 of `tests/nl-translate-band.fk`, deleted in `115fea55e` (2026-09-29) and again in `980de319e` (2026-10-18). `git blame` shows `NORTH_STAR.md:111` was last edited by `115fea55e` itself | observed (the deletion, the blame) |
| Some en/id sentence pair interns to one node | same | `tests/pivot-coverage-band.fk:73-75` bit 4096: "the afterwall is keloid" vs "pascatembok adalah keloid". Only the first slot differs across tongues; `keloid` is spelled the same in both | observed (file); the band was not run by me |
| Translation both ways | `nl-translate.fk:63-68` | `pivot-coverage-band.fk:76-77` bits 8192, 16384, two sentences over the five minted words | observed (file) |
| Symbol <-> surface both ways | `pivot-coverage.bml:7-14` | `pivot-coverage-band.fk:44-48,67-71`, five rows | observed (file) |
| The pivot child is the meaning-id, not a lemma | `nl-translate.fk:24-27` | `pivot-coverage-band.fk:78-79`: child 0 equals the string `"m1259"` | observed (file) |
| Band verdict 65535 | `pivot-coverage-band.fk:14,39` | `R/CURRENT_FLOOR.md:598`, line dated 2026-09-28 by blame: before the cut, before the band file's present commit | observed (record); not re-run |
| Projector walk, filter, cap, carry of `m` rows | `nl-lexicon-grow.fk:14-27` | `tests/nl-lexicon-grow-band.fk:36-60`, 7 bits; `CURRENT_FLOOR.md:597` records 127 (line dated 2026-10-04) | observed (record) |
| The seed words (`n0`..`n6`) translate | `nl-translate.fk:33-35` | none at HEAD (bits 2-128 of the deleted band) | observed |
| Corpus-grown words (`c` rows) translate | `nl-lexicon-data.fk:4-5` | none at HEAD (bits 2048-8192 of the deleted band) | observed |
| Noisy input normalises ("The Source Is Native.") | `nl-translate.fk:64-66` | none at HEAD (bit 16384 of the deleted band) | observed |
| Phase tags ice / water / gas on symbols | `nl-translate.fk:29-31,55` | none at HEAD (bits 512, 1024 of the deleted band). No code reads the phase to decide anything | observed (grep: `nlt-phase` occurs only at its definition) |
| "the self-authored-symbol-space MDL loop refines it toward minimal" | `nl-translate.fk:27-28` | no such cell exists (observed: the phrase occurs only in that comment) | observed |
| "reusable BODY behind the translate band (proven on fkwu) and the live carrier" | `nl-translate.fk:2-3` | the band and `translate-lane.fk` are deleted | observed |
| "Each natural language enters as grammar data ... and the engine stays as it is" | `NORTH_STAR.md:110-111` | true for parsing of two tongues; the decoder is code (3.4) | inferred |
| "every tongue, English included, is a projection column" | `NORTH_STAR.md:112` | true of the 52-row table's shape; 1,065 of 1,070 minted words exist in the English column only (section 4) | inferred |

The deleted `nl-translate-band.fk` (48 lines, read at `115fea55e^`) had fifteen bits and claimed 32767 "across
Go / Rust / TS / fkwu". Three of those four readers no longer exist (`critic.md` R15).

## 6. How names attach to nodes, and whether two tongues intern to one node

**Words are not nodes.** A lexeme is a string in a column of a row of a list value returned by a function
(`nlt-lexicon()`). Neither the row nor the word is interned as a node with identity; the only interned things are
the two symbol-string leaves and the `property` composite (3.3). **Observed.**

**The "neutral symbol" is a label minted three ways**: `n<i>` by hand in the seed, `c<i>` by the projector as a
running counter capped by an argument (`nl-lexicon-grow.fk:52-54`), `m<mid>` copied from the homecoming corpus
row number (`nl-lexicon-data.fk:6-10`). All three are arrival-order ordinals. `critic.md` 5.1 item 3 names that
as the thing an id must never be.

**Do two tongues intern to the same node?** Yes, mechanically, and trivially: `norm-en` and `norm-id` produce the
same node exactly when both lexicon directions answer the same symbol string for each slot. The equality tested
is equality of two pairs of strings. Three limits follow (**inferred** from 3.2-3.3):

1. The identity sits on the label. Rename `n0` to anything else and every pivot containing it becomes a different
   node. The comment at lines 24-28 asserts the opposite ("identity invariant under renaming").
2. It holds only where a hand-made row exists for both tongues. With no row, each tongue's surface word enters
   the pivot and the two nodes differ (3.5).
3. It is equality between sentences of one shape. Nothing relates `property(n0, n1)` to any other statement about
   `n0`; `n0` has no definition, no blueprint, no relation to the core lexicon's 64 words (section 8).

So the old body shows the **shape** the new design wants (category = numeric blueprint; children by content; the
tongue outside the identity) and gets the **content** wrong (children are label strings, not concept nodes).

**Phase as a row attribute.** Column 1 tags a row `ice` / `water` / `gas`, glossed as frozen and load-bearing /
circulating / just-invented (`nl-translate.fk:29-31`; `nl-lexicon-data.fk:9-10` adds "gas ... until a native
speaker or the corpus settles it"). This is one more meaning of ice / water / gas to add to `critic.md` C4's
list: **settledness of a name binding**. It is the one that fits a lens model directly: the node is fixed, the
attachment of a surface to it has a trust state.

## 7. The neighbours

### 7.1 `core-lexicon.fk`: 64 words closed over themselves (English only)

Mechanism (observed, `R/form/form-stdlib/core-lexicon.fk`): 64 rows `(word, family 0..7, index 0..7,
definition-token-list)` (lines 92-110); twelve declared glue words (line 114); `cl-closed?` checks that every
token of every definition is one of the 64 or glue (lines 115-123); 494 definition tokens asserted (line 261).
Each word has an address `family*1000 + index` and a node id `make_nodeid(1, family*1000, 0, index)` (lines
99-100) which nothing else in the repository uses (observed: grep for `cl-nid`). Eight "query lanes" are index
arithmetic: `why(i)` = same index one family down, `which(i)` = next index in the family ring, `what` always
answers `thing`, `whether` answers a truth by family (lines 148-160). The header's conclusions ("every why-regress
bottoms out in nothing", "exactly one lane is reversible") are properties of that arithmetic on an 8 x 8 grid,
not of the words (**inferred**: `cl-lane-why` never consults a definition).

Extensions: 4 derived words (`name`, `done`, `give`, `hold`) with a depth and a well-foundedness check
(`core-lexicon-derived.fk:80-121`); 36 words across 11 domains with **declared depth recomputed and compared**
(`core-lexicon-domains.fk:62-110`; my count of the rows). 104 words in all.

Witness: `cl-check()` folds 18 readings to 262143 (line 261). **No file at HEAD calls it** (observed: grep), and
`tests/core-lexicon-band.fk`, `-derived-band.fk`, `-domains-band.fk` were deleted in `115fea55e`.
`cognition/form-neutral-word-field.fk:82-84` and `response-word-field-dictionary.fk` reuse `cl-ok-token?` as a
closure test.

What is real here: definitions as token lists with a mechanical closure check; a depth ladder that is computed;
and a recorded lesson, `core-lexicon-domains.fk:88-95`: the first grounding check "could not fail" because an
unreachable marker of -1 was folded into a max. What is not here: any second tongue, any link from a word to a
pivot symbol, any grammar. The definitions are uninflected controlled English ("a self make a thing be"), close
in kind to the semantic-primes approach `S/web-numeric-root.md:463-468` describes as contested even at 65 primes
(**inferred** resemblance; the file cites no such source).

### 7.2 `meaning-codes.bml`: one symbol, many codes

Mechanism (observed): a roster of 36 symbols (line 50), each with an English phrase from `symbol-voice.bml`
(`sv-fixed`, e.g. `release` -> "released"); the codes of a symbol are `symbol`, `nl` (the English phrase), `node`
(first 8 hex digits of SHA-256 of the symbol text, lines 56-57) and one code per column of every
`locale-rows/symbol-*.rows` file whose row matches the English phrase (lines 68-89). `mc-resolve-all(code)`
answers **every** symbol carrying the code (lines 102-107); round trip is membership, not equality (148);
collisions are listed (154-159); an unknown token renders as `[token?]` (100).

Fragment (observed, the three row files): `symbol-words.rows` 8 rows x persian / german / french / portuguese /
romanian (the last row has German only); `symbol-perception.rows` 20 phrase rows x the same five;
`symbol-jungle.rows` 1 row (Indonesian, empty Balinese, German, French, Persian). The band pins one collision:
the Persian word for "said" resolves to both `say` and `tell` (`tests/meaning-codes-table-band.fk:128`).

Two limits (**inferred**): the join key from symbol to row is the English phrase (line 87: `mc-table-codes(tbl,
nl)`), so English is the hub in practice although line 55 says "no tongue privileged"; and the `node` anchor is a
hash of an English-like token, and the file itself says "The short anchor belongs to this codebook; kernel
composition identity has its own representation" (lines 15-16).

Witness: 8 bits, recorded 255 on fkwu and 195 on "the three proof walkers" (band lines 17-21;
`CURRENT_FLOOR.md:1070`, line dated 2026-09-28). The band's header records that `value_eq` answered false for
equal fresh strings "about 1 in 700 on sha256 output" on that kernel (lines 11-15).

### 7.3 `nl-lenses-13`: a phrase table

Mechanism (observed, `R/cognition/nl-lenses-13.fk`): 13 rows, one per locale (en id es fr pt-br sw de ru zh ja ar
hi tr), each holding four strings for "meanings" 1 presence, 2 enquiry, 3 affirmation, 4 a framing prefix and
suffix around a caller's detail (data file line 23). Detection is exact string equality scored 8/4/2/1 (line 40);
transformation looks up the same column in the target row (lines 56-70). Unknown text or locale answers the
empty string (line 11).

The file states its own boundary (lines 9-11): "deterministic phrase semantics. It is not learned language
identification, morphological generation, general translation, or fluency." Four rows are
`seed-pinned-unreviewed` (sw, ru, hi, tr; data lines 4-8, 23). The selection is "a repository-readiness proxy,
not a demographic ranking" (data lines 12-16). The meaning "presence" is rendered by phrases that do not say the
same thing across rows (English "I am.", Swahili "Niko hapa.", Hindi a greeting) (**assumed**: my own reading of
those languages, unreviewed).

Witness: `tests/nl-lenses-13-band.fk`, 13 bits, header says 8191; not listed in `CURRENT_FLOOR.md` (observed:
grep). Not run.

### 7.4 `concept-nl-semantic-13-omw.tsv`: the wordnet table

Shape (observed by count; every line has 14 tab fields): 10,000 rows. Column 0 is a Princeton WordNet 3.0
`offset-pos` synset key or empty; column 1 is an English word from a frequency list ("English is the exact
frequency-model anchor", `concept-nl-semantic-13.fk:7`); columns 2-13 are one label per locale.

Measured (observed, host `awk` over the file):

- 2,521 rows (25.2%) have **no synset key** (the first data row is `you`, key empty).
- 6,765 distinct synset keys over the other 7,479 rows; 577 keys are shared by more than one English anchor
  (`04928903-n` carries `way style fashion manner mode`).
- No label cell is empty; 10,000 distinct English labels.
- Provenance bytes (`sources.dat`, 130,000 = 10,000 x 13): `F` frequency anchor 10,000; `G` Google machine
  translation 68,370; `W` Wiktionary-derived 34,941; `D` dedicated wordnet 16,621; `C` CLDR 68. Of the 120,000
  non-English cells, 57.0% are machine translation. Code letters per `concept-nl-semantic-13.fk:42-52`.
- Reviewed rows: zero by construction (`cnl13-review-summary` returns a constant 0 in the reviewed position,
  lines 87-91; metadata state `automatic-mapped-unreviewed`).
- Cells identical to the English cell: id 754, es 806, fr 970, pt-br 519, sw 1,503, de 455, tr 604, ru 2, zh 19,
  ja 28, ar 40, hi 17.
- First rows show the join choosing a wrong sense: line 3 `i` -> `14641397-n` (iodine: `yodium`, `yodo`, `iode`),
  line 6 `a` -> `13658027-n` (angstrom), line 8 `it` -> `06134510-n` (information technology).

Sources are pinned by hash (`concept-nl-semantic-13-source-manifest.txt:1-40`: omw-data commit `406bf83b`, per
language files). The file header is careful: "state=mapped-unreviewed ... no claim that the first lemma is
preferred, canonical, fluent, or reviewed" (`concept-nl-semantic-13.fk:8-11`). The per-source licence files it
points to (`metadata.fk:4`) were deleted in `115fea55e`, as were both bands
(`cognition/tests/concept-nl-semantic-13-band.fk`, `-live-band.fk`).

Use at HEAD (observed): the crossing cell reads **one row**, index 1400 `necessary` / `01580050-a`
(`core-frequency-lift-form-neutral-crossing.fk:61-71`; file line 1402 confirms the row).

This is a label table keyed by English word frequency with a wordnet key attached where a first-sense join
found one. It is not a concept inventory and not an interlingual index. `S/web-numeric-root.md:448-455` covers
what a real one (CILI) is.

### 7.5 `core-frequency-lift-dictionary.fk`: the one design worth carrying

Observed in the lines read (1-45, 160-195, 250-339):

- **Meaning** = `intern_node(cat-meaning, sorted list of edges)`, an edge = `intern_node(cat-edge, [role,
  value])`, roles unique within a meaning or the constructor answers `nothing()` (lines 168-180). Roles, axioms and
  "gap" cells are `intern_node(category, [int])` with categories `make_nodeid(31, 2, 0, k)` (lines 17-40). The five
  axioms are roots 1..5 (lines 43-45). Derived meanings compose them: `observation`, `acknowledgement`, `choice`,
  `trust`, `sovereignty` (lines 185-195). Header line 10: "Meanings contain numeric cells only. NL strings live
  only in surface cells. Unresolved semantic atoms are explicit grounding gaps, not secret English identities."
- **Surface** = `intern_node(cat-surface, [language string, text string])` (line 256).
- **Occurrence**, **corpus**, **surface projection** (occurrence surface -> lexeme surface with a mode: 0 direct,
  1 case-fold, "further modes may carry inflection, script, abbreviation") and **provenance** with a claim state
  1 authored candidate / 2 surface relation attested / 3 participant-bound selection attested (lines 260-290).
- **Mapping** = `(arriving surface, context, meaning, faithful surface, provenance)` (lines 302-305).
- **Parse** returns the list of every mapping whose surface and context match: zero to many candidates (lines
  319-323). **Render** starts from a selected meaning and a language and returns candidate faithful surfaces
  (lines 327-333): "Rendering begins from an explicitly selected meaning, never a source word."
- The crossing cell adds a **residue** bit set on every projection (mapped-unreviewed, machine source,
  lexical-kind shift, enquiry/formality, inherited pivot) and an explicit **alignment** node
  `(external anchor, candidate recipe, residue)` described as "an inspectable proposal, not identity equality"
  (`...-crossing.fk:16-22,44-46`).

Fragment (observed, line 316): 28 mappings over 17 English words (`safety` x3, `control` x3, `authority` x2,
`compliance` x2, `enforcement` x3, `requirement` x4, and eleven single-sense words), all through `cfd-en`. One
mapping carries attested provenance (`authority`, line 299); 27 are authored candidates. Other tongues reach it
only through the crossing's three hand-chosen anchors (1400, 2001, 3001).

Witness: header "witnessed: 2026-08-13 -> ... 134217727"; no band for it stands in `R/cognition/tests/`
(observed: `ls` shows 21 band files there, none for this file; `grep -l core-frequency-lift-dictionary` finds only
the file itself and the crossing cell). The model-side band
`R/model/tests/form-neutral-language-world-model-band.fk` (2026-08-14, header "65535") imports it; I did not read
that band past line 16.

### 7.6 The rest, briefly (observed heads only)

- `language-neutral-symbol-identity.fk:13`: a node is the decimal number
  `package*10^9 + level*10^6 + type*10^3 + instance`; alias rows `[plane, relation, dialect, surface, node]`,
  plane 1 natural language, plane 2 programming language (lines 14-20); a miss answers `nothing()`.
  `language-neutral-inquiry-induction.fk:13-14` resolves tokens of one of 11 locales to one of four meanings
  301-304 from `learn/sanskrit-locale-baseline.fk` (four Sanskrit lines with renderings).
- `learn/nl-meaning-net.fk` with `nl-meaning-corpus.fk`: a trained two-block classifier over hashed bag-of-words
  for those same four meanings, about 50 hand-authored English examples per class; the file reports held-out
  accuracy "32/50 -> 36/50" after length normalisation (`nl-meaning-net.fk` header). Its bands were deleted.
- `form-neutral-word-field.fk`: 13 authored words with 13-number affinity vectors; the header says the values are
  authored seeds, "not a claim that culture has one universal geometry" (lines 3-10).
- `nl-arithmetic-demo.fk`: four English sentences ("the square of 7") parsed by a character grammar into
  arithmetic recipes and compared by `node_eq` with the hand-built recipe (lines 1-21). This is the one place a
  natural-language sentence becomes an **executable** recipe node. Sample, not a band; not run.
- `pl-lenses-13.fk:4-16` states that no parse/emit walkers for Python, TypeScript, Go or Rust are carried in this
  body and that it detects by marker strings and generates only add / subtract / multiply.

## 8. Seven neutral-id schemes, unjoined

Observed, one line each with its id rule:

| # | Scheme | What a "neutral id" is | Source |
|---|---|---|---|
| 1 | translation pivot | a string `n<i>` / `c<i>` / `m<mid>` | `nl-translate.fk:34-35`, `nl-lexicon-data.fk:15-17` |
| 2 | core lexicon | `family*1000 + index`, or `make_nodeid(1, family*1000, 0, index)` | `core-lexicon.fk:99-100` |
| 3 | meaning codes | 8 hex digits of SHA-256 of the symbol's own spelling | `meaning-codes.bml:56-57` |
| 4 | phrase lenses | the integers 1..4 | `nl-lenses-13.fk:3-4` |
| 5 | wordnet table | row index 0..9999, and a WordNet 3.0 offset string where present | `concept-nl-semantic-13.fk:22,36-40` |
| 6 | symbol identity | a decimal packing of four fields; meanings 301..304 | `language-neutral-symbol-identity.fk:13`; `...-inquiry-induction.fk:13` |
| 7 | frequency-lift dictionary | an interned composite of numeric role edges | `core-frequency-lift-dictionary.fk:168-180` |

No file imports `nl-translate.fk` together with `core-lexicon.fk`, `meaning-codes.bml` or anything under
`cognition/` (observed: grep; the only files naming both are `NORTH_STAR.md` and a corpus comment). The word
`body` is `n3` in scheme 1, a depth-1 domain word in scheme 2 (`core-lexicon-domains.fk:76`), absent from 3 and
4, a frequency row in 5. Only scheme 7 is content-derived; only the crossing cell relates 4, 5 and 7, for three
anchors. **Inferred conclusion**: the old body never had a meaning space for natural language; it had several
tables that each call their key neutral.

## 9. What a numeric-space design should take, precisely

Each item names its source in the old body. All are **inferred** recommendations from the observed lines.

**Take the shape**

1. **A sentence meaning is a composite: numeric blueprint as category, concept nodes as children, identity by
   content.** Source: `nl-translate.fk:58-59` with `form-ontology-bp.fk:943`. Six natural-language blueprints are
   already reserved as coordinates `1.2.99.32-37` (`fact`, `property`, `isa`, `relation`, `question`, `meaning`,
   `bmf-core.fk:103-108`); only `property` has a user at HEAD.
2. **A tongue enters as data over one engine.** Source: `nl-translate.fk:22-23` over `bmf-grammar.fk`. Extend it to
   the half the old body left as code: the linearisation rule must be data too (3.4), or "adding a tongue" stays a
   programming task. `S/web-numeric-root.md:336-349` shows the prior art that does both directions from one
   concrete-syntax description.
3. **A lexeme is a lens entry, not part of the node.** Model it on scheme 7, not scheme 1: surface node
   `(lens, text)`; mapping `(surface, context, concept, provenance)`; many surfaces per concept, many concepts per
   surface. Source: `core-frequency-lift-dictionary.fk:256,302-305`.
4. **Text to node answers zero, one or many candidates, and says which.** Sources:
   `core-frequency-lift-dictionary.fk:319-323`; `meaning-codes.bml:102-107,154-159` (resolve-all, collisions
   listed as "the price of compression, never hidden"). Matches `critic.md` 5.1 item 2.
5. **Node to text starts from a chosen concept and a lens, and may answer nothing.** Source:
   `core-frequency-lift-dictionary.fk:327-333`.
6. **Every attachment carries provenance and a claim state; "reviewed" is a count that starts at zero.** Sources:
   `core-frequency-lift-dictionary.fk:260-263` (states 1/2/3); `concept-nl-semantic-13.fk:42-52,87-91` (one
   provenance byte per cell; constant-zero reviewed); `nl-lenses-13-data.fk:3-8` (`seed-pinned-unreviewed`).
7. **A crossing keeps its residue.** Source: `core-frequency-lift-form-neutral-crossing.fk:16-22`: what a
   projection lost (sense, formality, kind shift, machine source) is a recorded bit set, and an alignment between
   an outside inventory and an inside concept is a separate proposal node, never an equality.
8. **Ungrounded atoms are explicit gap cells.** Source: `core-frequency-lift-dictionary.fk:10-11,21,160-166`.
9. **A concept can carry a definition as a composition of other concepts, checked mechanically for closure and
   for depth.** Source: `core-lexicon.fk:115-123`; `core-lexicon-domains.fk:96-110`. Take the check and the lesson
   that a check must be shown able to fail (`core-lexicon-domains.fk:88-95`). In a numeric space the definition is
   a recipe over concept ids, and the English token list is one lens of it.
10. **The settledness of a name binding is a state of the binding.** Source: the phase column,
    `nl-translate.fk:29-31`. If ice / water / gas is to be "respected", this is one concrete place it has a
    reading: a lens row is gas when invented, water in use, ice when reviewed and referenced; the node underneath
    does not change.
11. **State the boundary in the cell.** Source: `nl-lenses-13.fk:9-11`, `concept-nl-semantic-13.fk:8-11`,
    `pl-lenses-13.fk:10-15`. These headers say what the cell is not; the north-star sentence built on them does
    not repeat it.

**Do not take**

1. Label strings as pivot children and ordinals as symbols (`n<i>`, `c<i>`, `m<mid>`): section 6.
2. The lexicon as rows with one column per tongue. Adding a tongue reshapes every row; the removed many-tongue
   shelf had already switched to `(symbol, surface)` triples per tongue for that reason
   (`nl-tongues.fk:28-29` at `115fea55e^`).
3. Pass-through on a lexicon miss and a string value on parse failure (3.5).
4. English as the hidden hub: the join key in `meaning-codes.bml:87`, the anchor column of the table
   (`concept-nl-semantic-13.fk:7`), `cfd-en` in every base mapping (`core-frequency-lift-dictionary.fk:259,316`),
   the 64 words themselves.
5. Machine-translated cells as lexicon content (7.4).
6. ASCII-only normalisation and letter classes as the text floor (`string-case.fk:12-28`, `bmf-core.fk:330-333`).
7. Several id schemes for one idea (section 8).
8. Lane arithmetic presented as findings about meaning (7.1).
9. The word "proven" in a header whose band has been deleted (section 5).

## 10. The honest present limit of "the numeric space is the root of all natural languages"

On the evidence in this repository, at HEAD, as of 2026-11-24:

- **Demonstrated in a standing test file** (not re-run by me; recorded verdict dated 2026-09-28): one sentence
  template; two tongues; five invented words translated both ways; one cross-tongue node equality in which one of
  the two slots is spelled identically in both tongues.
- **Present as code without a standing test**: the same template over 47 more rows; the NORTH_STAR example
  sentence itself.
- **Present as tables**: 4 phrase meanings x 13 locales by exact string match; 36 symbols with up to five
  tongue codes per row file over 29 phrase rows; a 10,000-row label table that is 57% machine translation, unreviewed, a quarter of
  it without a concept key.
- **Present as a design with 28 English mappings**: numeric meaning recipes with surfaces, provenance and residue.
- **In history only**: thirteen tongues over that one template with seven meanings.

What no file at HEAD does (observed by reading the files in section 2.1): inflect or agree a word; reorder
constituents by rule; handle a multi-word slot, a verb other than the copula, tense, negation, quantification or
a question as a parsed sentence; choose between senses by context in the translator (it is first match);
segment a script that does not use spaces; produce a surface in a tongue from a grammar description rather than
from hand-written concatenation; relate a word's concept to the concepts in its definition and to the same word
in a sentence.

And the root itself is not numeric where it matters. The sentence category is a numeric coordinate. The things
the sentence is about are strings.

So the supportable statement is: **the old body shows that a content-interned node can be the meeting point of
two surface strings when a hand-made table maps both to one label, for one fixed sentence shape.** That is the
controlled-fragment result `S/web-numeric-root.md:51` (row T5) already grants from prior art, at close to the
smallest possible size, and without the property that makes the prior art scale to a second construction
(linearisation as data). "Root of all natural languages" has, in this repository, no evidence beyond that
sentence. `critic.md` 5.1 item 11 ("the natural-language claim is staged last and kept separable") and 5.2 N9
stand; this report gives N9 its missing fact: the fragment is one construction, two tongues, 50 words.

## 11. Open questions

1. Does `pivot-coverage-band.fk` still answer 65535 on today's `fkwu`? Its record predates the cut and the
   1,024th-call fault (`critic.md` R14). I did not run it.
2. What does `tr-en-id` answer for a sentence outside the template, and for trailing words (3.5)?
3. Is the 28-mapping dictionary's band (134217727, 2026-08-13) still reproducible, and does the model-side band
   exercise it? Not read past line 16.
4. Which of the three first-class candidates is the concept inventory for a first natural-language lens: the 64
   closed words, the axiom-rooted role recipes, or an outside index (CILI) attached by alignment nodes? The old
   body tried all three and joined none.
5. Is a concept's definition (7.1) part of its identity (then two tongues can only share a node if they share
   the decomposition) or a claim about a nominal node? This is `critic.md` N2's nominal-distinction question, met
   again in language.
6. What counts as success for the first lens (`critic.md` N9)? A measurable candidate from this reading: one
   abstract construction set, two tongues with different word order and one with agreement, linearisation as
   data, every miss a fault, round trip checked by a test that is shown able to fail.
7. Are `web/messages/*.json` in the parent repository meant to be lexicon evidence at all, given they are UI
   labels aligned by key?
8. The licence files for the wordnet table were deleted from this tree (2.2); the table remains. Whether the
   table may be carried forward is the owner's call.

## 12. Surprise, and what stayed uncomfortable

Most surprising: the commit that cut the repository to "its end-to-end healthy flow" edited the NORTH_STAR line
naming `nl-translate.fk` and, in the same commit, deleted the only band that tested the sentence that paragraph
quotes. The claim has stood without its witness for eight weeks.

Second: the file whose comment says the symbol's identity is "invariant under renaming" makes the symbol's
spelling the identity.

Third: the careful part of this lane is not the part the north star cites. `core-frequency-lift-dictionary.fk`
and the crossing cell carry provenance, claim states, residue and "this is a proposal, not equality" in the code;
`nl-translate.fk` carries none of it.

Uncomfortable:

1. I ran nothing. Every verdict here is a recorded number; two of the three recorded numbers are dated
   2026-09-28.
2. The 5-of-1,070 coverage figure and the overflow of the lens's field code are my static join, not the lens's
   run.
3. My reading of the phrase packs' cross-language meaning (7.3) rests on my own knowledge of those languages.
4. I read 173 of 470 lines of the dictionary that I recommend as the model.
5. The wordnet table's first rows carry a wrong sense that is visible on sight (`i` as iodine), and the body's
   world-model band opens that table (`R/model/tests/form-neutral-language-world-model-band.fk:16`).
