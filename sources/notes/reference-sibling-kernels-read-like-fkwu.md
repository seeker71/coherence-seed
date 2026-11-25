---
name: reference-sibling-kernels-read-like-fkwu
description: "Since 2026-09-11 Go/Rust read defn params as names and all three siblings give record_new(0)/missing fields fkwu's 0; the shadowed-primitive guard is released; probe hygiene for four-kernel surveys"
metadata: 
  node_type: memory
  type: reference
  originSessionId: de70343c-34cf-4cc9-92b9-5d1df43ed9e0
  modified: 2026-09-12T07:16:50.640Z
---

- **defn parameter lists are names on every kernel** (8e399c04). Go and Rust used to read `(params tok i)`
  as an expression, so a first parameter spelled like one of their reader verbs (params, match, fail,
  list, if, …) fell out of the arity. fkwu's `[shadowed-primitive]` refusal guarded that; measured on
  all four kernels (18 names, value and call position) it no longer diverged and was released
  (bcb3c157). walkers/rust reads names too; walkers/go since 49a4cbe4 (2026-09-12). In call position
  the primitive still wins, identically everywhere.
- **A present native answers its name on all four kernels** (4eb94988, 2026-09-12). Go and Rust used
  to let a user defn shadow a same-named native (except eight byte-string/table names on a bypass
  list), so fourth-shim/form-fs/form-ontology-bp bodies covered working natives there. Env-aware
  natives still yield to a frame binding, as TS does. A Form defn of a native's name is a fallback.
  Call heads read one way on all seven arms since 2026-09-12: fkwu's reserved heads (op rows,
  rewrite rows, defn/do/let/if; gate/reserved-heads.bml writes the header's list into Go/Rust/TS and
  the walkers, a drift-door row compares; when fkwu-optable.h moves, `write` on its stdin re-carries it) answer as the primitive, or the walkers' shared-Form
  recipe, under any same-named local; every other head reads the nearest local binding (any frame
  but the root) first, then the native, then the global. call-head-reading-band 255 four-way.
- **No reader keeps seq/params/match/choose/fail/stop** (released 2026-09-12). BML `match` lowers
  through source-compiler's `fsc-source-emit-match` to an if-chain over value_eq (`_` last wherever it
  stands, a non-name subject bound once as `fsc-match-subject`); source-language-match-switch-band 7
  four-way (R77 released). The ontology gate's `sequence`/`match` rows accept no source verb.
  source-compiler.fk is a form-cli stamped source: after editing it run
  `zsh scripts/regen_form_cli_bootstrap.sh` from form/, else TestFkwuFormCliCanonicalCarrier reds.
- **Ask a value its kind, never measure it** (2026-09-12, 64c29fcd/3479af57/326d1849). `value_kind`
  answers string/list/int/null the same on all four. `str_len` on a list, `node_value` of a composite,
  `node_eq` on lists/strings answer (0 or a compare) on fkwu and STOP Go/Rust/TS — a guard built on
  them passes fkwu by accident. Use value_kind for kind, `value_eq` for structure (it treats nodes as
  node_eq does, probe 1 0 1 1), `(node_eq x (intern_trivial_string "PARSE-FAIL"))` for sentinels
  (interning is content-addressed on all four).
- **List doors, one contract** (a61bdbb9, 2026-09-12): head/tail/nth on a non-list, or with no element
  to give (empty head, index past the end or below zero), answer nothing on fkwu and null on
  Go/Rust/TS; the tail of a list is a list. Before it, fkwu read an int receiver as a pair index
  ((head 3) answered another list's cell, (nth xs -1) the first element), Go's tail of a non-list
  answered (), TS threw. fkwu words: int even, list odd non-negative, string/nothing odd-negative —
  read the tag bit before walking. `(list ...)` walks every element, so a `(not (nothing? x))`
  beside a read of x guards nothing.
- **Memory follows list sharing, and TS's heap follows the machine** (859c3c74, 2026-09-12). On the
  heldout band Go holds 375 MiB resident, Rust 14 GiB, TS 11 GiB (10.1 GiB used heap): Rust/TS copy
  on tail. V8 caps an isolate near 4 GiB (4192 MiB here, worker too), so TS alone ran out of heap on
  turnwheel, heldout, family-mastery; main.ts's worker now takes half of physical memory
  (maxOldGenerationSizeMb). A TS OOM is a ceiling question before it is a leak question: measure
  Rust's RSS on the same band first.
- **records**: `record_new(0)` is a plain field map (blueprint reads back 0) and a missing field reads 0
  on fkwu, Go, Rust and TS (2c715d97). The body tests misses with `eq(v, 0)`.
- **The compare family is one meaning on all four** (2026-09-12, c855d85a0..03a03f6f4): `eq`/`ne` over
  a non-number answer `value_eq` (content: lists by items however built, NaN = NaN, records and
  closures by place); `lt/le/gt/ge` and add/sub/mul/div/mod over a non-number stop by name; truth is
  the 0/1 ints everywhere (no bool kind on any sibling or walker); a branch on nothing stops ("ask
  nothing? before branching"), a float zero is 0. fkwu used to answer these from its words — a cell
  that passes on a word order is a wordlean (row 1493); trace a scratch fkwu before moving a lane.
- **defn frames see only their parameters on fkwu** (2026-09-11): a defn inside a band's `(do ...)`
  that reads an enclosing `let` compiles to `[unbound-name]` on fkwu and prints nothing, while Go,
  Rust and TS close over it. Write bands with parameters, nullary defns or plain lets (25b071a8
  healed field-choice, observe-genealogy, reachability, thought-forming).
- **fkwu's `bp` (op 45) is identity**: it returns the name; the siblings resolve names from
  `bp_table.{go,rs,ts}` (generated by scripts/gen_bp_table.py). A band that reads a `bp` category's
  instance (channel-query, 1712-1716) answers 0 on fkwu. Open as of 2026-09-11.
- **Healed the bp-identity class by minting** (2026-09-11): channel-query, symbols, url-encode, uuid now
  use `make_nodeid` at registry coordinates. xpath-band still reads fkwu 1 against the siblings' 9 after
  SYMBOL-TABLE/BIND were minted, so its cause is elsewhere (the evaluator on fkwu), not bp. Still open:
  the glass family (FORM-GLASS-*-V1/V2) is in no registry or table, so every glass row stops Go/TS at bp.
- **validate.sh legs**: fkwu runs from the body root, the siblings from `form/`; a band that walks a
  relative path diverges by directory alone — pick the dir that exists with `fs-is-dir?` (four-way).
  A cell not listed in fourth-arm-bands.txt gets a ✓ that speaks for three kernels ("fourth arm: 0
  band(s)"); run `./fkwu cell.fk` directly, since the seq divergence showed only there (2026-09-12).
  See [[validate-proof-level-lanes]].
- **failure-taxonomy-band's prompt** is condition E's frozen closed-book prompt, rebuilt by
  `fked-base-prompt` (sha256 c3870f18ea91f1bd…); never a /tmp fixture.
- **Probe hygiene** (paid for twice on 2026-09-11): zsh does not word-split `for n in $N` — use a
  literal list or `${=N}`; and BML probes need real newlines (a one-line brace block reads numb and
  throws "unbound function"). When every sibling crashes on a generated probe, read the generated
  file before believing the kernels.
