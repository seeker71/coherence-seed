# Form the language, as it stands — a feature inventory for the restart

Study date: 2026-11-24. Author: a study subagent of the coherence-kernel redesign workflow.

## 0. Scope, method, snapshot

- **Repository read** (never modified): `ROOT = coherence-kernel`, head commit `b8ff978c9`, committed 2026-11-24T18:11:03+08:00 (observed: `git log -1`).
- **Runtime used for probes**: `coherence-kernel/fkwu` (the main checkout's built binary, 745,448 bytes, built 2026-11-20 16:12). The seed source `ROOT/runtime/fkwu-uni.c` was last committed 2026-11-20T16:06:27+08:00, so the binary is consistent with the seed at this head (inferred from the two timestamps; not proven by hash).
- **Probes**: 40 one-line Form programs, written to and run from `<scratchpad>/study/probes/` (scratchpad = the directory holding this report), never from inside the repository. Each probe result below is marked `observed (probe <name>)`. Running a probe wrote `.fkb`, `.fkb.make_ms` and `.sym` beside the probe file in the scratchpad, and read/wrote the host's shared node field (see section 6).
- **Status words**: *observed* = read in a file or run in this session; *inferred* = my reasoning from observed things (the basis is named); *assumed* = not verified.
- **Classification words** per feature: **essential** (the new design needs the idea), **accidental** (an artifact of history, replaceable), **wart** (a known or newly observed defect or trap).

Source dates carried where known (observed by `git log -1 --format=%cI -- <file>`): `axioms/core-axioms.form` 2026-11-19; `docs/kernel-interface.md` 2026-11-17; `form/form-stdlib/core.fk` 2026-10-29; `runtime/fkwu-node-word.h` 2026-11-12; `CURRENT_FLOOR.md` 2026-11-20.

## 1. The picture in one paragraph

"Form" today is three stacked things that share one name. (a) A tiny S-expression language with exactly **four special forms** (`defn`, `do`, `let`, `if`), two literal words (`true`, `false`), and a table of ~287 call heads (221 primitive ops + 66 reader rewrites) that the C seed answers. (b) A node store: every value that matters can be a **NodeID**, a 64-bit word `(package, level, type, instance)`, with composites interned by `(category, children)` into a host-wide shared-memory field. (c) A large body of recipes — 974 `.fk` and 1,105 `.bml` tracked files, about 309,000 lines — of which most "Form" files are in fact BML inside an `.fk` wrapper (575 of 974 `.fk` files carry a `section [form.bml]`). The C seed that is called "the smallest substrate-walker host" is 34,149 lines. BML's `fail`, `stop`, `try/catch`, `offer` and lambdas all lower onto **two** Form facts: the value `nothing` and the recover point `attempt`. Proof is by "bands": programs that answer a bit-sum integer held to a pin in their own header.

Sources: `ROOT/runtime/fkwu-uni.c:25033-25169` (the four forms), `ROOT/runtime/fkwu-optable.h:7-301` (the tables), counts by `git ls-files`/`git grep` (observed 2026-11-24), `ROOT/form/kernel-roadmap.md:3` ("smallest substrate-walker host"), `wc -l` on the seed (observed).

## 2. Surface syntax

| Feature | Definition | Proof | Class |
|---|---|---|---|
| S-expression call form `(head arg...)` | `ROOT/runtime/fkwu-uni.c:25018-25030` (`fk_sparse`: `(` then a head symbol) | `ROOT/bootstrap/ground.fk:6` answers 42 — observed (probe ground) | accidental as *the* surface; essential as a canonical printable form of a node tree |
| Integer, float, string literals; `true`/`false` | float = token with `.` or `e/E` (`ROOT/grammars/form-eval.fk:8`); `true`/`false` at `fkwu-uni.c:25672-25675` | observed (probe lits): `-5 1e3 2.50 "a\nb"` read as `-5, 1000, 2.5, a<LF>b` | essential (literals), wart: `true`/`false` read as the ints 1/0 — `(value_kind true)` answers `int` (observed, probe nothing/kinds) |
| Comments `;` to end of line; `//` in sectioned files; `#` in `.form` spec files | `ROOT/form/form-stdlib/form-parse.fk:14-16`; `.form` files use `#` (`ROOT/axioms/core-axioms.form:1-25`) | all sources read | accidental (three comment spellings) |
| Header directives inside comments: `; preludes: a b`, `; Verdict n`, `; PROOF LEVEL: FKWU-STAGED`, `; STAGED CARRIER:`, `; REQUIRES:` | `ROOT/form/kernel-roadmap.md:10-19`, `ROOT/form/form-stdlib/AUTHORING.md:71-79,144-148` | band sweep reads them (`ROOT/form/kernel-roadmap.md:8-13`) | wart: semantics carried in comments (a comment that changes what loads) |
| `import path;` (any spelling) | `ROOT/docs/kernel-interface.md:10-12`; `fkwu-uni.c:27247,27281` | `ROOT/form/form-stdlib/boot.fk:5-7` | essential idea (declared dependency), accidental spelling |
| Multi-dialect sections `section [dialect] { ... }` inside one file | `ROOT/form/form-stdlib/compiler.fk:12,219,234`; BMF rule `section ::= "section" "[" $dialect:name "]" "{" ... "}"` at `compiler.fk:239` | observed counts: 1,641 `section [form.bml]`, 88 `[public]`, 3 `[form.action]`, 2 `[form.lift]`, one each for `python.bmf go.bmf rust.bmf typescript.bmf prolog.bmf bml.bmf bmf.bmf form.route` | accidental (it exists because there are two grammars); the *idea* that one file carries a grammar plus its recipes is worth keeping |
| Multiple top-level forms without `do`; last value is the answer | observed (probe toplevel: `(defn a ..) (let y (a 1)) (a y)` answers 3) | probe | essential |
| No quote, no macro, no anonymous-function form in the S-expression reader | the reader compares only `defn do let if true false` (`fkwu-uni.c:25033,25116,25130,25161,25672,25675`; `grep fk_sym_eq` shows no others) | observed (grep, 2026-11-24) | essential fact about the floor; BML lambdas lower to a named nested `defn` (section 10) |

Stale documentation found here: `ROOT/form/form-samples/README.md:24` lists `(seq <stmt>...)` as a verb; the runtime refuses it as an unresolved call with exit 1 (observed, probe seq).

## 3. Special forms (complete list)

All four are parsed by hand in C; everything else is table rows.

1. **`(defn name (params...) body)`** — registers a function before its body is parsed, so self-recursion and forward references resolve (`fkwu-uni.c:25031-25115`, comment at 25315-25317; forward reference observed, probe undef answers 2). A `defn` in value position yields a function value (AST tag 243). A nested `defn` captures free variables of the enclosing frame (capture prologue, `fkwu-uni.c:25001-25016`). **Essential.**
2. **`(do stmt...)`** — sequence, last value; `let` and `defn` inside bind for the rest of the `do` (`fkwu-uni.c:25116-25118`, 25120-25129). **Essential.**
3. **`(let name val)`** — two-argument: binds for the rest of its `do` and evaluates to `val`. A three-argument `(let n v body)` survives as "a pre-existing value-position convenience" (`fkwu-uni.c:25120-25156`). Re-`let` of the same name shadows (observed, probe rebind answers 2). **Essential** (binding); the two shapes are a **wart**.
4. **`(if c t e)`** — the only control form (`fkwu-uni.c:25158-25170`). Branch law, observed by probe: `0` is false; any other int, any string (including `""`), and any list (including the empty list) is true; **`nothing` stops the program**: `if: nothing is neither 0 nor 1 -- ask nothing? before branching` (probes empt1, empt2, empt3, ifnoth). **Essential**, with a design tension recorded in section 8.

Reader rewrites (66 rows, `ROOT/runtime/fkwu-optable.h:233-300`): `gt ge lt eq and or not ne abs` are RPN templates over `le`/tag-102/`if`; `float_value make_float32 make_float64 math_pi value_kind recipe_to_bytes bytes_to_recipe read_form_binary write_form_binary substring value_str read_file_bytes write_file_bytes attempt cell_run cell_call cell_input cell_path jit_fn_bind jit_node jit_cpu_run host_*` and the speaker doors all rewrite to one overloaded op, **`float_leaf` (tag 201) with a mode number** (`fkwu-optable.h:242-299`). The reason is stated: "The AST tag space is full, so the door rides tag 201 as every leaf door does" (`ROOT/docs/in-process-cells.md:380-381`). **Wart** (an 8-bit tag space that filled; `attempt`, the core of the error model, is "mode 28 of the float leaf").

`and`/`or` are binary only; a third argument is a compile error ("stray ')' in value position"), the run still prints a value and exits 1, and the next run reads a cached image "compiled with errors" (observed, probe and3; rule at `ROOT/form/form-stdlib/AUTHORING.md:107-108`). **Wart.**

Call-position resolution order: reserved primitive names win over local bindings and over user `defn`s of the same name. `(do (defn str_len (s) "fallback-shadow") (str_len "abc"))` answers 3 (observed, probe shadow; pinned as a conformance vector at `ROOT/form/conformance/canonical-s-expression-vectors.json:65-69`; the seed prints a `[shadowed-call]` warning for local bindings, `fkwu-uni.c:25172-25183`, which records "the trap that cost ... 13 hours"). The axioms file itself names this "the live seam: call position resolves a NAME against the global table before the local frame" (`ROOT/axioms/core-axioms.form:196`). **Wart** against axiom 3's "names are free query keys".

## 4. Core primitives

### 4.1 The pure core (the table the interface doc commits to)

Source: `ROOT/docs/kernel-interface.md:19-36` (2026-11-17). Status: observed (read); spot-checked by probes.

| Family | Names |
|---|---|
| numbers | `add sub mul div mod le lt eq` (`gt ge ne not and or` are rewrites) |
| floats | `float_to_int str_to_float math_sqrt make_float32 make_float64 float_value intern_trivial_float` |
| equality | `value_eq str_eq` (and `node_eq`, same tag 80 as `value_eq`: `fkwu-optable.h:47-48`) |
| bits | `band bor bxor shl_u32 shr_u32` (+ measured `rotr_u32 add_u32 bnot_u32`) |
| strings | `str_len str_byte_at byte_to_str str_concat` — the "narrow waist"; a string is bytes (`ROOT/form/form-stdlib/core.fk:15-36`) |
| lists | `cons head tail nth len list empty` |
| absence, control | `nothing nothing? attempt form_error` |
| kind | `value_kind` |
| records | `record_new record_get record_set record_has record_keys record? record_blueprint` |
| nodes | `make_nodeid intern_trivial_int intern_trivial_string intern_trivial_bool intern_node node_children node_value node_category node_type node_inst node_pkg node_level` |
| methods | `method_define method_has method_invoke` ("measured natives", `kernel-interface.md:39-41`) |

Ten laws pin the core (`kernel-interface.md:47-65`), witnessed by `ROOT/form/form-stdlib/tests/kernel-laws-band.fk` (header pins verdict 17179869183, 34 bits; observed by reading lines 1-60). Probed here: law 1, 63-bit wrap — `(add 4611686018427387903 1)` answers `-4611686018427387904`; law 2 — `(attempt (div 1 0))` answers `nothing`; law 8 — `(str_byte_at "" 0)` answers `-1`; `(nth (list 1 2) 5)` answers `nothing` (probes ints, attempt).

Class: the *families* are **essential**; the specific spellings and the split "primitive vs rewrite vs float_leaf mode" are **accidental**.

### 4.2 The Form-side core library

`ROOT/form/form-stdlib/core.fk` (776 lines, pure S-expression): `identity`; strings built over the waist (`substring char_at ord int_to_str str_to_int str_find float_to_str`, lines 106-333); `intern_node_at` (344); Num vocabulary (348-359); List vocabulary `nil? walk-end? map filter foldl foldr range append take drop reverse sum product maximum minimum any? all?` (377-480); `Cell<T>` and `Task<T>` as tagged lists (483-525); `assert` (529-530); a self-describing class/method model (564-662); and an exact shortest-round-trip float printer (bignum kit + Dragon4, 664-774). Each native that later replaced a recipe keeps its recipe as "the body's statement of what the door means", with an equivalence band (e.g. `core.fk:90-98`). **Essential pattern** (native = measured replacement of a recipe that stays as the spec); the 300-odd lines of commentary inside the file are history.

### 4.3 The whole op surface is far larger than the pure core

`fkwu-optable.h` carries 221 op rows: files, sockets, TLS, HTTP, shared memory, process spawn/wait/kill, Metal GPU (`metal_*`), CUDA, microphone/camera/speaker/Wi-Fi/Bluetooth sensing (`sense_*`), mesh (`mesh_*`), terminal, JIT (`jit_arm64_u32_leaf jit_leaf_inram`), kernel introspection (`kernel_stat kernel_ast kernel_hot ...`) (`fkwu-optable.h:8-228`; observed). Many names are duplicated aliases on one tag (`host_dir_mkdir`/`fs_mkdir`, `host_file_read_text`/`read_file`, `write_file`/`write_file_text`; lines 57-72, 120-130). The interface doc says one host-door table "replaces the mirrors that disagree today" and is "not built yet" in full (`kernel-interface.md:98-104,183-186`). **Accidental** (the breadth lives in the C seed, contrary to the stated small kernel).

## 5. The data model

### 5.1 Value kinds

`value_kind` names in the seed: `unknown null bool float string closure record node_id list int` (`fkwu-uni.c:19436`). Observed by probe (nothing, kinds): `(value_kind (nothing))` = `null`; `1` = `int`; `1.5` = `float`; `"s"` = `string`; `(list)` = `list`; a NodeID = `node_id`; a record = `record`; a function value (capturing or not) = `closure`; `true`, `false` and comparison results = `int`.

- **int**: 63-bit two's complement `[-2^62, 2^62)`, tagged (`kernel-interface.md:50-51`).
- **float**: binary64; prints shortest round-trip (`core.fk:664-774`); a whole-number float prints without a point — `(mul 1.5 2)` prints `3` and has kind `float` (observed, probe float32). **Wart** (documented: "whole-number floats still display inconsistently", `AUTHORING.md:117-119`).
- **string**: bytes; every index is a byte offset; cuts clamp and never stop (`core.fk:67-88`).
- **list**: cons pairs; "a pair keeps its tail word" (dotted pairs exist; `kernel-interface.md:37,48`).
- **record**: keyed, **mutable in place** — `(record_set r "a" 5)` then `(record_get r "a")` answers 5 (observed, probe rec); "the record table has no collector" (`ROOT/docs/in-process-cells.md:388`). Carries a blueprint/class id (`record_blueprint`). This is BML's object carrier.
- **closure**: function value; prints `<closure>`.
- **nothing**: one absence value; `value_kind` says `null`; `print` renders it `nothing` and `value_str` renders it `null` inside a list (`kernel-interface.md:95-96`). **Wart** (two names for one thing).
- **node_id**: section 5.2.

### 5.2 NodeID — the 64-bit identity word

Generated header `ROOT/runtime/fkwu-node-word.h:1-37` (2026-11-12; "Generated by Form: observe/native-node-word-generate.bml"):

- 1 mode bit + **package 6 bits + level 13 bits + type 12 bits + instance 32 bits** = 64 (`:6-14`).
- Mode bit clear = the coordinate `1.1.1.i`, a **trivial int** whose instance is a 63-bit signed integer stored directly in the word (`:16,24,28-31`). So an integer *is* its own NodeID; `(make_nodeid 1 1 1 7)` equals `(intern_trivial_int 7)` (`ROOT/form/form-stdlib/tests/nodeid-interning-band.fk:28-34`).
- Rendered `@pkg.level.type.inst` (conformance vector `canonical-nodeid-render`, `canonical-s-expression-vectors.json:51-54`).

Leaf types under package 1, level 1 (observed by probe nid2/float32 and by `ROOT/form/form-stdlib/bml/formbin-codec.bml:26-36`): type 1 int, 2 string, 3 bool, 5 int64 payload, 6 float32 (**the instance is the IEEE-754 bit pattern**: `(make_float32 0.1)` renders `@1.1.6.1036831949`, and 1036831949 = 0x3DCCCCCD), 7 float64. For strings and float64 the instance is a **kernel-local table index** (`@1.1.2.196514`; "the inst of a 1.1.2/1.1.7 leaf is kernel-local", `kernel-interface.md:36`).

Level-2 categories ("blueprints") are plain coordinates in a hand-kept table: `do 1.2.9.1`, `let 1.2.9.3`, `if 1.2.11.2`, `fndef 1.2.31.1`, `fncall 1.2.32.1`, `ident 1.2.33.1`, `add 1.2.12.1` ... `not 1.2.14.3`; user blueprints at type 99 (`JSON-OBJECT 1.2.99.10`, `OAC-ZERO 1.2.99.1947`); other packages appear (`BMF-DOMAIN-CONTRACT 8.45.3.1`) (`ROOT/form/form-stdlib/form-ontology-bp.fk:11-70`). `host-kernel.form` gives package a meaning: "NodeID.package: 1 = body, 2+ = branches/sandboxes" (`ROOT/axioms/host-kernel.form:150`).

**Composite nodes.** `(intern_node category children)` interns by content: two calls with the same category and children answer the same word (observed, probe nodeid: `eq` answers 1). The composite's word is `@0.2.99.21759736`: package 0, the category's level and type, and an **instance that is a running index in the host's shared field** — the same index came back from a second process (observed, two runs of probe nodeid), and `kernel_stat 60` (nodes in the shared field) read 21,759,736 (probe nid2). So:

> A composite NodeID is **not** a content hash. It is an intern-table index, stable only while the same shared field lives. Content addressing is a property of the intern table, not of the identity word. (observed; the docs agree: "Composite identity remains local to the kernel that admits an artifact", `ROOT/docs/native-form-artifacts.md:5-6`; "the content digest crosses the wire, not the local node id", `ROOT/axioms/kernel-self-composition.form:91`.)

This is the single most important structural fact for the restart. Class: the four-field coordinate for *issued* identities is **essential** (it is the numeric space); the local-index composite identity is **accidental** and in tension with axiom 3.

Level means category level: `node_level` of a composite is its category's level (probe nodeid: 2); `node_children` of a bare coordinate is `[]`; `node_category` of a leaf is itself (probe nid2).

### 5.3 How blueprint, recipe and cell are represented

- **Axiom level**: "a-cell-is (package level type instance)"; trinity: blueprint = axioms 2+3, recipe = axiom 2, named-cell = axiom 3 with 0..many names (`ROOT/axioms/core-axioms.form:59,99-104`).
- **Blueprint** in practice: (a) a level-2 coordinate used as the category of a composite; (b) a name-to-coordinate row. Three authorities coexist: `blueprint-registry.json` ("authoring source", hand-curated), `form-ontology-bp.fk` rows ("current runtime authority"), and the program image `.fkb` ("target executable authority") (`ROOT/form/user-blueprint-registry.md:8-20`). `bp` is native and merely "answers its argument"; coordinates come from `fol-bp` (`kernel-interface.md:43-45,63-65`). **Wart** (three authorities, one of them JSON the kernel does not read).
- **Recipe**: a composite node tree — "a recipe is identity, observed and never walked" (`ROOT/form/kernel-roadmap.md:47-52`). But the thing the walker actually executes is a *separate* AST of 8-bit-tagged C nodes built by `fk_sparse` (`fkwu-uni.c:25099-25169`, `fk_smknode(tag, ...)`). So there are **two tree representations**: NodeID recipes (data, content-interned) and the walker AST (executed). The north-star file states the intended unification — "A semantic Form recipe is the authoring authority ... the native JIT consumes its graph directly" — and admits "general graph execution uses the Form walker, while direct CPU/Metal graph JIT remains the next compiler boundary" (`ROOT/grammars/bml-native-north-star.form:5,9`). **Wart / central open design point**.
- **Cell**, four different shapes: (1) a persisted named cell `CELL-V0(name, domain, blueprint, ctor)` with identity `(domain, name)` and last-write-wins (`ROOT/form/form-stdlib/persistence.fk:20-28,48`); (2) `core.fk`'s `Cell<T>` = the list `("cell" kind value origin inverse)` (`core.fk:483-495`); (3) an "in-process cell" = a runnable unit called with `cell_call(path, argument, deadline_ms)` answering a record `{value out err diag stopped errors resident ms cpu_us}` (`ROOT/docs/in-process-cells.md:17-41`); (4) shared-memory cell maps `cell_map cell_field cell_value cell_ref` (`fkwu-optable.h:106-110`). **Wart** (one word, four carriers); the *call-a-unit-get-a-record* contract of (3) is **essential**.
- **Type-99 allocation hygiene**: principles "composition first ... every new number has a cost"; observed drift: "defensive high numbering in the 7000+ / 7700+ / 8100+ / 8800+ / 9000+ ranges", per-dialect synonyms (`ROOT/form/user-blueprint-registry.md:39-56`). **Wart**, self-diagnosed.

## 6. Node states: ice, water, gas

The words are used with **at least six different definitions**. None is enforced by the language; each is a reading or a convention.

| # | Definition | Where | Enforced? |
|---|---|---|---|
| A | **Home of the node population** (the only definition inside the runtime): ice = the shared field `/fg-field2-*`, content-addressed, shared by every kernel on the host, outlives them; water = the per-pid shared store `/fg-c<pid>-*`, visible to siblings, gone with the pid; gas = the private heap, RAM only. "exactly one of 58/59/60 carries the whole population" | `ROOT/runtime/fkwu-uni.c:22929-22943`; band `ROOT/form/form-stdlib/tests/kernel-census-band.fk:46-57` (verdict 2047) | Read-only census via `kernel_stat 58/59/60`. Observed (probe nid2): gas 0, water 0, ice 21,759,736 |
| B | **Circulation metabolism** of any substance: gas = 0 diffuse, water = 1 fluid, ice = 2 frozen; `phase-state(deg, churn, thresholds)`; moves `condense freeze melt sublimate`; hysteresis; "it never MUTATES — the caller carries the invitation"; "STATE is orthogonal to SUBSTANCE" | `ROOT/form/form-stdlib/substrate-phase.fk:4-53` | A pure function over counts; nothing calls it on real nodes that I found (assumed: no enforcement) |
| B' | A fourth phase, **plasma** = 3 (high circulation and high heat) | `substrate-phase.fk:33-42`; `ROOT/form/form-stdlib/source-resonance-stream.fk:10-14,48-50` | same |
| C | **Cache tiers**: source file = "canonical ice", in-memory recipe = water, lookup results = gas; and elsewhere `.fkb` beside a source is "its ice" | `ROOT/form/form-stdlib/cache.fk:10-16`; `ROOT/AGENTS.md:574-576` | convention; note C calls the *source* ice while AGENTS calls the *cache image* ice |
| D | **JIT metabolism**: interpreted recipe = gas, crystallized native leaf = ice, "melt when it cools"; "the full gas-water-ice-back metabolism" | `ROOT/axioms/host-kernel.form:28-31,206-213`; `ROOT/form/form-stdlib/jit-heat-gate.fk:16-21,79-92` (band `jit-heat-gate-band.fk`, 4095 per `host-kernel.form:228`) | policy in Form (`hg-request`, `hg-cool`); the walker's own JIT has no melt-back that I found (section 14) |
| E | **Trinity mapping**: Blueprint = ice, Recipe = water, NamedCell/beliefs = gas | `ROOT/form/form-stdlib/grammars/living-equation.fk:6-13` | none — and it contradicts B's "state is orthogonal to substance" |
| F | **Literals** in the `form.lift` dialect: `ice / liquid / compost` = 2 / 1 / 0 (compost, not gas) | `ROOT/form/form-stdlib/grammars/form-lift.fk:11,91-93` | integer constants only |
| G | **Channel/OSI layer phase** strings `"gas" "water" "ice"` with codes 1/2/3 | `ROOT/form/form-stdlib/channel-flow.fk:53-56,90` | validity predicate only |
| H | **Queue pressure**: empty/open = gas, flowing = water, full/backpressured = ice | `ROOT/form/form-stdlib/resident-async-ingress.fk:70-72` | projection |

Separately, **"melt"** in the runtime means garbage collection ("fkwu's melt compacts the cons heap and then frees what no root names", `ROOT/CURRENT_FLOOR.md:1073-1075`), and **"freeze"** in the axioms prose means admitting a teaching (`ROOT/axioms/core-axioms.form:203`).

Finding (inferred from the table): the three states are a strong shared *metaphor* and a weak *mechanism*. The restart's requirement that node states "must be respected" has no existing single definition to inherit; definition A (where a node physically lives and who can see it) and definition D (interpreted vs compiled form of the same recipe) are the two that correspond to real machinery. The numeric codes disagree across files (gas is 0 in B, 1 in G; ice is 2 in B and F, 3 in G).

## 7. Offer / acknowledge and choice at the Form level

**The axiom**: "offer a recipe or a blueprint with 0..many argument cells. It is acknowledged by exactly one of nothing, 0, 1, or node" (`ROOT/axioms/core-axioms.form:87-93`).

**The running core** (`ROOT/control/offer-ack-core.fk:1-66`, BML source; band `ROOT/form/form-stdlib/tests/offer-ack-core-band.fk`, header verdict 32767):

- Ack constructors: `oac-nothing` = `nothing`; `oac-zero` = node `OAC-ZERO(0)`; `oac-one(payload)` = node `OAC-ONE(payload)`; `oac-node(counter)` = node `OAC-NODE(counter)`; one discriminator `oac-kind` (lines 5-24). "OAC-NOTHING (1946) remains retired: absence is the reducer's sentinel, not a blueprint" (`form-ontology-bp.fk:14-15`).
- `oac-offer(recipe,args) = try recipe(args) catch fail`; `oac-fail() = fail`; `oac-stop() = stop`; `oac-choice`, `oac-try`, `oac-run-chain` (halts and keeps the last ack), `oac-async`/`oac-await` (a future is the pair `[recipe,args]`), `oac-hold` (re-present with ripened args under a patience count), `oac-read-chain` (holds, refuses to advance), `oac-census` (answered vs silent kept apart) (lines 26-64). These are the theorems `held-offer` and `absence-is-not-a-verdict` (`core-axioms.form:174-219`).
- `ROOT/control/choice-lane-core.fk:7-39`: `oac-cut` (commit to first), `oac-lanes` (walk all), `oac-store`/`oac-restore` (**identity functions** — a checkpoint is just an immutable value), `oac-undo`, `oac-timeout-walk` (a budget count; timeout = nothing), `oac-backtrack` (each option receives the original args plus the memory of what earlier options found). Bands: `ROOT/control/tests/choice-lane-core-band.fk` (1023), `backtrack-band.fk` (255), `attempt-band.fk` (4095) (headers observed).

**What it lowers to in Form** (observed in the lowering memo `coherence-kernel/control/offer-ack-core.fk.lowfk:89-106`, written 2026-11-21):

```
(defn oac-offer (recipe args) (do (let $try (attempt (recipe args))) (if (nothing? $try) (nothing) $try)))
(defn oac-fail () (nothing))
(defn oac-stop () (nothing))
(defn oac-choice (alts args) (if (nil? alts) (nothing) (do (let $try (attempt ...)) (if (nothing? $try) (oac-choice (bml-value-tail alts) args) $try))))
```

So at the Form floor: **fail = stop = timeout = `nothing`; try/catch = `attempt` + `nothing?`; choice = recursion over a list of function values; store/restore = keeping a value.** `fail` and `stop` are indistinguishable as values (`oac-failed?` and `oac-stopped?` are both `oac-nothing?`, `offer-ack-core.fk:28-30`); only the *driver* (choice continues, run-chain halts, read-chain holds) gives silence its meaning. The axioms file says exactly this: "The core had read the nothing-ground twice ... FAIL ... and STOP ... A third was always derivable" (`core-axioms.form:170-173`).

Class: the four-valued ack, the single discriminator, hold/census, and backtrack-with-memory are **essential**. That `0`-ack and "error" and "declined" all collapse into `nothing` at the floor, and that the reason travels out of band (section 8), is the **design tension** to settle. Host effects are not undone by restore ("their restoration does not reverse host effects", `choice-lane-core.fk:1-2`) — an honest **limit**.

A control-token grammar names eight invites a model stream can carry: `<CHOICE> <CUT> <FAIL> <STOP> <UNDO> <STORE> <RESTORE> <TIMEOUT>` (`ROOT/grammars/control-invite-grammar.fk:36`; band `grammars/tests/control-invite-grammar-band.fk`). `form.lift` gives the same words integer codes `nothing cut stop timeout choice undo restore` = 0..6 (`form-lift.fk:14-15,94-100`), which makes `nothing` = `compost` = 0 = the int zero in that dialect — contradicting the kernel, where `nothing` is not 0. **Wart.**

## 8. Error model: nothing versus stops

Three layers, observed:

1. **`nothing`** — an ordinary value: `nth` past the end, `head` of an empty list, `record_get` of a missing key, a host door that could not answer (probes ints, cmp, rec; `kernel-interface.md:122-129`).
2. **A stop** — a named refusal that unwinds to the nearest `attempt`, or ends the program with exit 1. Sources: `form_error`, integer divide by zero, arithmetic or order on a non-number, `str_concat` of a non-string, `len` of nothing, `if` on nothing, a walker stack wall, a cell deadline (probes attempt, strs, subnoth, eqnoth, ifnoth; `in-process-cells.md:38,58-66`). The stop message teaches the fix ("ask value_kind first").
3. **`(attempt x)`** — answers `x`'s value, or `nothing` if `x` stopped; each recovered stop is **voiced as an `organ-health-v1` JSON line** carrying the reason and `"offers":["backtrack"],"selected":"backtrack"` (observed, probe attempt). The reason is therefore *not* a value the program holds; `host_why(verb)` is the stated way to read a door's reason word (`kernel-interface.md:122-125`). A cell deadline jumps "past any `attempt` the cell holds" (`in-process-cells.md:61-63`).

Stated rule: "No negative codes, no `""` or `[]` standing for absence, no `0` standing for refusal, and no door that ends the process" (`kernel-interface.md:125-127`). Counter-examples still standing: `str_byte_at` out of range answers `-1` and `str_find` answers `-1` (law 8, `kernel-interface.md:60-61`); `ord` answers `-1` (`core.fk:119`); `write_form_binary` answers `-1` on a bad encode (`formbin-codec.bml:57`); `lookup-cell` answers a 0-or-1-element list as "the body's optional idiom" (`persistence.fk:75-77`); `fe-look` uses integer 0 as nil (`form-eval.fk:42,50`). **Warts**, each a place the rule has not reached.

Newly observed defects (this session, this binary):

- `core.fk`'s `assert` cannot fail: it relies on `(head (empty))` "provok[ing] the kernel's empty-head failure path" (`core.fk:527-530`), but `(head (empty))` answers `nothing` without stopping, so `(assert 0 "must fail")` answers `nothing` and the program continues (probe assert: `[0, nothing, 99]`).
- `core.fk:102-105` says "(sub 2 nothing) is 8999999999999999999"; the runtime now stops on it (probe subnoth). Stale comment.
- `AUTHORING.md:124-125` "No `let` inside a `defn` body" is stale: it works (probe letdefn answers 8) and `core.fk` itself does it (`core.fk:110,173`).
- `AUTHORING.md:93` "`eq` compares integers and nodes; `str_eq` compares strings. Don't cross them": `(eq "a" "a")` and `(eq (list 1 2) (list 1 2))` both answer 1 (probe lits). And `(eq 1 1.0)` answers 1 while `(value_eq 1 1.0)` answers 0 (same probe) — two equalities with different numeric laws.

Class: `nothing` as first-class, stop-as-backtrack-point, and "a refusal names its fix" are **essential**. The out-of-band reason, the surviving sentinels, and `if` stopping on `nothing` while every non-zero value is true are **warts / open choices**.

## 9. Numeric tower

Observed/sourced facts:

- **int**: 63-bit wrap-around, `[-2^62, 2^62)`; `div` truncates (`(div 7 2)` = 3), `mod` truncates with the dividend's sign (`(mod -7 2)` = -1); by-zero stops (probe ints; laws 1-2, `kernel-interface.md:50-52`). Literals past 2^62 "wrap as read" (`kernel-laws-band.fk:9`). Comparison blur beyond 2^53 is recorded in the user's memory index (assumed; not re-verified here).
- **float64**: IEEE binary64; `0.1 + 0.2` = `0.30000000000000004`; `math_sqrt` correctly rounded; float `mod` truncates; `float_to_int` stops on NaN/out-of-range (probe ints; laws 4-6).
- **Mixed arithmetic promotes**: `(div 7.0 2)` = 3.5, `(div 6 3.0)` = 2 (kind float) (probes ints, float32).
- **float32**: a NodeID leaf `@1.1.6.<bits>`, not a float value; `value_kind` says `node_id` (probe float32). `make_float64` likewise mints a `1.1.7` leaf. So there are two float worlds: unboxed walker floats and NodeID float leaves.
- **u32 bit ops**: `band bor bxor shl_u32 shr_u32 rotr_u32 add_u32 bnot_u32` (`(shl_u32 1 31)` = 2147483648).
- **No native bignum, rational, complex, or unsigned 64**; a base-10^9 bignum kit exists only to print floats (`core.fk:664-694`). Quantized formats "bf16/fp8/nf4/int8/bitnet-158" are named as recipes in `format-arith.fk` (`AUTHORING.md:98-101`; not read here — assumed from that citation).
- The wire format carries full int64 and every float64 bit independently of the 63-bit runtime int ("No number is narrowed through a Form scalar during a wire roundtrip", `native-form-artifacts.md:21-26`).

Class: 63-bit tagged ints are **accidental** (a tagging choice of this seed that leaks into the language as wrap-at-2^62); IEEE floats and exact printing are **essential**; "numbers as coordinates in the node space" (int = `1.1.1.v`, f32 = `1.1.6.bits`) is the **essential** seed of the numeric-space root and is already half-present.

## 10. Functions and closures

- Functions are first-class by name: `(twice inc 5)` answers 7 (probe hof). A nested `defn` returned from its parent captures the parent's frame values at definition time: `((make-adder 10) 5)` = 15 (`ROOT/form/form-samples/closure.fk:7-13`; probe closure).
- No anonymous function in the S-expression reader. A BML lambda `x => e` lowers to `(do (defn $lambda-@0.2.9.784418-0 (params) body) (let name name) name)` — a generated name embedding a NodeID (observed, `offer-ack-core.fk.lowfk:106`).
- Self tail calls run as loops: 3,000,000 iterations answered (probe tco). Non-tail recursion 100,000 deep answered when run as a program (probe deep); inside a cell the documented wall is a stop at 2047 frames (`in-process-cells.md:110-115`). (The two numbers are not reconciled here — open.)
- Known closure hazards recorded by the project: "a duplicate def name across a file and its prelude closure" history (`in-process-cells.md:83-85`); the user's memory index lists "Hot closure over locals stops at the heat mark — a map over a closure holding locals went wrong at its 1024th call" (2026-11-24; assumed, not re-verified).
- Methods and classes are a runtime table (`method_define method_has method_invoke`) plus records with a class id; BML classes lower to Form `defn`s plus a descriptor list `("abstraction" "ChoiceLane" (bases) (methods) ...)` (`choice-lane-core.fk.lowfk:98`).

Class: closures and first-class recipes are **essential**; "lambda = generated global name" and "primitive names beat bindings" are **warts**.

## 11. Modules, imports, preludes, cells

- A unit's **closure** = its `; preludes:` (recursive, deduplicated, `none` sentinel) + every `import` spelling + names listed in `form-stdlib/home-index.txt` (39 lines: `<name> <unit>`; a unit that calls a home name it does not define gets that unit linked in) (`ROOT/form/kernel-roadmap.md:15-19`; `ROOT/docs/kernel-interface.md:10-15`; `home-index.txt:1-4`).
- `./fkwu --closure <unit> <out>` writes the closure as one plain-Form file (`kernel-interface.md:13-15`).
- `.bml` and section-bearing `.fk` are lowered through the "floor compiler" (`bml-floor-compile.fk`, a 23-unit plain-`.fk` closure) into a memo `<unit>.lowfk` (`in-process-cells.md:202-213`). The memo format `fklow2` lists every dependency with a fingerprint (observed at `control/offer-ack-core.fk.lowfk:1-60`).
- **Namespace**: one flat global function table per process, with a per-cell **name window** so two cells may define the same helper (`in-process-cells.md:77-85`). No module-qualified names in Form; the convention is a per-file prefix (`cser-`, `bt-`, `oac-`, `fe-`...). **Wart** (prefix discipline instead of scope).
- Top-level `let` visibility differs by placement ("A unit-top-level let IS visible to a defn inside a later do ... fkwu compiles every name positionally", `ROOT/form/form-stdlib/compiler.fk:204-214`). **Wart.**
- "No effects at import" is a convention (`in-process-cells.md:87-89`).
- Path resolution is repo-root relative ("from `form/` they refuse", `ROOT/form/README.md:27`), and the memo above shows probing of doubled paths (`.../form-stdlib/form-stdlib/core.fk`, `N` rows) — a search-path heuristic. **Accidental.**

**In-process cells** (`ROOT/docs/in-process-cells.md`): `cell_call(path, argument, deadline_ms)` runs a unit in this process and answers one record; `cell_input()` reads the argument; `"check"` as deadline is the compile-only door; residency (parsed once per path+content); arena is append-only for the process's life (lines 17-41, 75-103). Measured: a spawned run costs ~55 ms before the unit does anything; a resident call of `(do 7)` costs 0.1 ms (lines 146-158). **Essential**: unit-as-callable-cell with a typed record result, deadline as `nothing`, and "there is no exit code ... no stderr to re-read: a diagnostic is a row" (lines 43-45).

## 12. Serialization formats (four, plus caches)

| Format | What | Where | Class |
|---|---|---|---|
| **FORMBIN2** | Inert node-tree wire data. Magic `FORMBIN2`; big-endian u32 tags/counts/coordinates; deduplicated UTF-8 string pool; numeric tags 2 (float64) and 3 (int64) carry 8 little-endian bytes; explicit continuation stack (no depth ceiling); admission ceilings 64 MiB / 262,144 strings / 1,000,000 nodes | `ROOT/docs/native-form-artifacts.md:1-33`; codec in Form: `ROOT/form/form-stdlib/bml/formbin-codec.bml`; 13 canonical + 12 malformed vectors: `ROOT/form/conformance/*.json`; gate witness 511 (`native-form-artifacts.md:40-52`) | **essential** (an inert, refusal-complete wire form, owned by the language not the seed) |
| **FKPIFB1 program image** | The per-source compile cache, also named `.fkb`, plus a `.sym` text lens and a `.fkb.make_ms` file. Magic `FKPIFB1 ... fkwu-image-v1 seed <hash>` — bound to the seed build | observed: `xxd` of the probe's `ground.fkb`; `.sym` content `program-image-sym-lens-v1 / source-hash fk-unit-v4|... / dependency 0 mtime ... size ... path ...` | accidental; **wart**: the extension `.fkb` names two unrelated formats; freshness by mtime+size ("older than two seconds", `in-process-cells.md:92-94`) |
| **`.lowfk` (`fklow2`)** | Lowered-BML memo with dependency fingerprints | `in-process-cells.md:204-242` | accidental — exists only because of two grammars |
| **JSON cell dialect** | `{"bp":"<name>","c":[...]}` — blueprint *name* + children, rebuilt by the same `bp name` + `intern_node` | `ROOT/form/form-stdlib/cell-serialize.fk:3-23,107,136-189` | essential idea (identity re-derived from name + structure through a codebook) |
| CORBA-CDR dialect | named only | `cell-serialize.fk:14` | not read |

Round-trip identity, observed: `bytes_to_recipe(recipe_to_bytes n)` returns the **same** node when the category is a coordinate (`@0.2.99.21759736` both ways; probe rtb2). With a **string leaf as category** — the shape `form-samples/float-artifact-roundtrip.fk:27` uses — the original is `@0.1.2.21759737`, the round trip is `@0.0.0.21759738`, `node_eq` answers 0, and the returned node's category is the bare string `weight` rather than the leaf `@1.1.2.871359` (probe rtb3). The sample still answers 0.8125 because it compares only the child. **Wart** (an identity-losing round trip on this binary for string-category composites).

Persistence above the wire: a store is a channel of messages in one `.fkb` file; cells are appended; lookup scans for the last match (`persistence.fk:59-81`); the boot sequence "load image → offer a channel → self-extend by minting a *new* image" is `ROOT/form/form-stdlib/boot.fk:81-107` (band `boot-band.fk`, verdict 9 per `kernel-self-composition.form:95`). **Essential**: extension mints, never mutates; rollback is re-pointing.

## 13. Host doors

- Design target: one table, families `file path process stream clock shared gpu device entropy self`; "A door answers its value or `nothing`. The reason is a word"; handles are `(index, generation)` so "a stale handle answers `nothing`"; `await(deadline-ms, handles)` is "the one place the body blocks" (`ROOT/docs/kernel-interface.md:98-135`). Status per that doc: the unified table and per-verb timing are "not built yet" (lines 183-186).
- Today: ~150 host rows in `fkwu-optable.h` with aliases, plus `float_leaf` modes 17-58.
- Principle from the axioms layer: any host driver/OS API is allowed as a carrier if the kernel "allow[s] its presence" and "measure[s] its health"; native replacement is "CAN-not-MUST" (`ROOT/axioms/host-kernel.form:103-110`); resources are typed ports `(direction, value-shape)` with shapes `bool scalar bytes text pixel key` (lines 86-91).
- A one-rule BMF grammar is declared the "source authority" for the host-effect vocabulary (`ROOT/grammars/host-effect-vocabulary.bmf:8-10`).

Class: the door contract (value-or-nothing, reason word, generation handles, one blocking point) is **essential**; the present op zoo is **accidental**.

## 14. JIT, heat, melt

- Two JITs coexist in the seed: a C "loop lane" (`fk_f64_*`) that "crystallizes pure-float defns and self-tail-call loops at heat 1024", and a Form-written page JIT (`form/form-stdlib/bml/jit-page.bml`) offered a defn "whose heat reaches a multiple of 4096" or 32 time samples; it "writes AArch64 words and binds them with `jit_fn_bind(fn, words)`"; the defn then "wears the crystallized-defn node (tag 194)" (`ROOT/CURRENT_FLOOR.md:1273-1296,1381-1385`). "Every strict op has one meaning, a C function the walker's arm and the page both call" (line 1296).
- A separate Form policy organ counts heat per interned identity at a request door, crystallizes on a threshold and melts on cooling, with hysteresis (`jit-heat-gate.fk:16-31,82-92`).
- The laws are held "hot as cold" (laws 1, 2, 6 warm bits in `kernel-laws-band.fk`); a deadline "cannot end a crystallized hot leaf" (`in-process-cells.md:64-66,388-389`).
- Target contract: "Cold and warm execution of unchanged meaning on the same target may differ at most by storage delivery" (`ROOT/grammars/bml-native-north-star.form:8`).

Class: heat-driven specialization written in the language itself, one-meaning-per-op shared by interpreter and compiled code, and the cold/warm contract are **essential**. Two JITs, AArch64-only pages, and thresholds baked in C are **accidental**.

## 15. The proof system

- A **band** is a program answering a bit-sum integer, each bit one falsifiable claim; its header pins the verdict (`; Verdict n`); "a band's observed verdict, held to the pin its head declares, is the proof; there is no trusted prover" (`AUTHORING.md:5-7,71-79`). 584 band files tracked; 98 rows in `form/band-verdicts.txt` (observed counts).
- The sweep "fails when a band exits nonzero, speaks a diagnostic, or answers other than the verdict its head pins" (`kernel-roadmap.md:8-13`). The axioms file records why the exit code matters: a broken chain answered "2097151 with EXIT 1 — the SAME number the healed chain returns" (`core-axioms.form:219`).
- Conformance gate: 13 canonical expressions and 12 malformed FORMBIN2 vectors (`ROOT/form/conformance/`).
- Limits (inferred): a 34-bit verdict is an opaque integer; a pin proves "same answer as when pinned", not "right answer"; verdicts above 2^53 hit a fold ceiling (user memory index, assumed).

Class: falsifiable claims pinned beside the code and perturbation checks are **essential**; the bit-sum integer encoding is **accidental**.

## 16. The idea → spec → implementation chain

- As a statement it exists in exactly one place: the theorem `(idea-chain axiom-2)  # idea->spec->implementation is composition at the work altitude` (`ROOT/axioms/core-axioms.form:62,106`; `git grep` for the phrase found no other file — observed 2026-11-24).
- As practice it appears as: `.form` spec files that cite running proofs (`axioms/host-kernel.form`, `kernel-self-composition.form`, `grammars/bml-native-north-star.form`), each with `closing-recipes` naming "the parts to compose and the proof band that closes it" (`host-kernel.form:189-221`); the compiler flows `source-scan → lift → normalize → emit → run-observe` and `grammar-source → grammar-parse → grammar-compile → dialect-capsule → band-proof` held as data cells (`ROOT/form/form-stdlib/compiler.fk:58-59`); and the artifact contract `source -> compiler-image -> loadable-table -> native-binary -> ...` (`bml-native-north-star.form:12`).
- `.form` files are S-expression-shaped prose with `#` comments; they are not executed by anything I found (assumed non-executable: `AUTHORING.md:38` calls them "field `.form`").

Class: the chain is **essential** as a graph relation (idea node → spec node → implementation node → proof node). Today it is prose, not nodes: **gap**.

## 17. Grammar machinery already inside Form (relevant to "one grammar")

- A scannerless cursor (no tokenizer), patterns as data (`lit cls run seq alt opt cap`, `cut`), templates that build nodes (`emit splice const`), and **reversible rules** `name ::= pattern => action <= reverse;` (`compiler.fk:235-271`; `host-effect-vocabulary.bmf:9`; `control-invite-grammar.fk:15-17,42`).
- A meta-circular evaluator of Form in ~170 lines of BML (`ROOT/grammars/form-eval.fk`; band verdict 65535 per its header line 32).
- A whole dialect defined in 112 lines of rules (`form-lift.fk:22-108`): infix, `unless/when`, new keywords — evidence that surface syntax is cheap once the node tree is the authority.
- Claimed property: "Content-addressing means two surface syntaxes for one program parse to the same NodeID — one substrate" (`kernel-roadmap.md:56-60`); language ports table lists BMF and BML "proven", Python "in-motion", TypeScript/Go/Rust "seed", Java "legacy-source", C# "target" (`compiler.fk:60`).

## 18. Classification summary

**Essential (carry the idea):** the five axioms; `nothing` as first-class; the four-valued ack with one discriminator; `attempt` as the single recover point; hold / census / backtrack-with-memory; NodeID four-field coordinate with numbers as coordinates; content-interning of composites; names as free query keys via a codebook (`bp`); recipes as data; first-class closures; self tail call as loop; the byte "narrow waist" for strings with clamped total functions; native = measured replacement of a recipe that remains the spec; unit-as-cell call with a typed record; door contract (value-or-nothing + reason word + generation handles); inert wire format with malformed-vector conformance; mint-never-mutate images; bands as pinned falsifiable claims; grammar-as-data with reversible rules; heat-driven JIT written in the language with one meaning per op.

**Accidental (history):** two grammars and the section mechanism; `.lowfk` memos and the floor compiler; S-expression as the *authoring* surface; 8-bit AST tags and `float_leaf` modes; 63-bit int wrap; three blueprint authorities; `.fkb` meaning two formats; mtime freshness; repo-root-relative path heuristics; prefix namespaces; two JITs; alias op names; bit-sum verdict encoding; 34k-line C seed carrying sockets, TLS, Metal, CUDA, sensors and mesh.

**Warts (defects/traps):** primitives beat bindings in call position; `if` stops on `nothing` but treats every non-zero non-nothing value (including `""` and `[]`) as true; `(empty x)` trap; binary-only `and`/`or` with a compile error that still prints a value; `true`/`false` are ints; whole floats print as ints; `nothing` prints as `nothing` or `null`; `eq` vs `value_eq` disagree on `1` vs `1.0`; surviving `-1`/`0`/`[]` sentinels; `assert` that cannot fail; stop reasons out of band; composite NodeIDs are local indices; string-category composites lose identity on round trip; directives in comments; ice/water/gas with 6+ definitions; "cell" with 4 carriers; stale docs (`seq`, no-let-in-defn, eq-only-ints, `(sub 2 nothing)`).

## 19. Documentation vs observed behaviour (this session)

| Doc claim | Source | Observed | Probe |
|---|---|---|---|
| `(seq ...)` is a verb | `form/form-samples/README.md:24` | unresolved call, exit 1 | seq |
| "No `let` inside a `defn` body" | `form/form-stdlib/AUTHORING.md:124-125` | works, answers 8 | letdefn |
| "`eq` compares integers and nodes ... Don't cross" | `AUTHORING.md:93` | `eq` answers 1 on equal strings and equal lists | lits |
| "(sub 2 nothing) is 8999999999999999999" | `form/form-stdlib/core.fk:102-105` | stops: "only numbers add, subtract..." | subnoth |
| `assert` "provokes the kernel's empty-head failure path" | `core.fk:527-530` | answers `nothing`, run continues | assert |
| "The kernel stays small" / "smallest substrate-walker host" | `form/README.md:38`, `form/kernel-roadmap.md:3` | seed is 34,149 lines; 221 op rows | wc, optable |
| `form/form-stdlib/offer-ack-core.fk` "in BOTH homes" | `axioms/core-axioms.form:182-183` | only `control/offer-ack-core.fk` exists | `git ls-files` |
| float artifact round trip preserves identity | `form/form-samples/float-artifact-roundtrip.fk:27-33` | leaf yes; the wrapping string-category composite no | rtb3 |

## 20. Carry forward / let go / open questions

See the structured return for the lists; the reasoning is in sections 5-18. The three decisions the restart cannot avoid:

1. **What a NodeID is.** Keep the four-field numeric coordinate for issued identities (the numeric space), and decide whether composite identity is a true content digest (portable, bigger than 64 bits or a 64-bit handle onto a digest table) instead of a field index.
2. **One tree.** Make the node tree the thing that executes (interpreted or compiled), so the walker AST, the `.lowfk` text and the image cache are states of the same node — which is also the only place ice/water/gas can become a real, enforced property (persisted+shared / in-process / transient; or stored / interpreted / compiled).
3. **What silence carries.** Keep `nothing` as the one absence, and decide where the *reason* lives (a node-valued ack already exists in the axiom: "nothing, 0, 1, or node") so that error, decline, timeout and not-yet are distinguishable by the receiver without reading a side channel.

## 21. Probe log (all observed 2026-11-24 on the binary named in section 0)

```
ground    (add 40 2)                                   -> 42
closure   make-adder 10, then 5                        -> 15
truth     (lt 1 2)(gt 1 2)(eq 4 4)(and 1 0)(or 0 1)(not 0) -> [1,0,1,0,1,1]
nothing   kinds of nothing,1,1.5,"s",(list),nodeid,true -> null,int,float,string,list,node_id,int
attempt   (attempt (div 1 0)) / (attempt (add 1 2)) / (attempt (form_error "boom")) -> nothing / 3 / nothing (+ one organ-health line per stop)
empt1-3   (if (empty) 1 0) (if "" 1 0) (if 5 1 0)       -> 1, 1, 1
ifnoth    (if (nothing) 1 0)                            -> stop, exit 1
nodeid    intern_node twice, same content               -> @0.2.99.21759736, eq 1, category @1.2.99.10, children [@1.1.1.7, @1.1.2.196514], level 2, pkg 0, type 99
nid2      leaves and census                             -> @1.2.12.1 @1.1.1.7 @1.1.2.651661 @1.1.7.1062892 @1.1.3.1; gas 0, water 0, ice 21759736
ints      wrap, div, mod, mixed, 0.1+0.2, byte_at, nth  -> -4611686018427387904, 3, -1, 3.5, 0.30000000000000004, -1, nothing
shadow    user defn str_len vs native                   -> 3 (native wins)
letdefn   let inside defn body                          -> 8
and3      (and 1 1 0)                                   -> compile error, prints 0, exit 1
kinds     closure, plain defn, record, (eq 1 1), false  -> closure, closure, record, int, int
rec       record_set then get; missing key              -> 5; nothing (records mutate in place)
seq       (seq 1 2 3)                                   -> unresolved, exit 1
tco       3,000,000 self tail calls                     -> 3000000
deep      100,000 non-tail frames                       -> 100000
lits      -5 1e3 2.50 "a\nb" (str_len "λ") eq on strings/lists, (eq 1 1.0), (value_eq 1 1.0) -> -5,1000,2.5,a<LF>b,2,1,1,1,1,0
rebind    two lets of x                                 -> 2
hof       (twice inc 5)                                 -> 7
undef     forward reference between defns               -> 2
strs      (str_concat "a" 1)                            -> stop, exit 1
cmp       attempt lt on strings / add "a" 1 / head (empty); (head (list)) (tail (list)) (len "abc") (len 5) -> nothing,nothing,nothing,nothing,[],3,0
rtb2      wire round trip, coordinate category          -> same node, eq 1
rtb3      wire round trip, string category              -> @0.1.2.21759737 vs @0.0.0.21759738, node_eq 0
far       form-samples/float-artifact-roundtrip.fk      -> 0.8125
assert    core.fk assert shape                          -> [0, nothing, 99]
subnoth   (sub 2 (nothing))                             -> stop, exit 1
eqnoth    (len (nothing))                               -> stop, exit 1
float32   (make_float32 0.1), kinds, bits, sqrt         -> @1.1.6.1036831949, node_id, 8, 2147483648, 1.4142135623730951, 3, 3 (kind float), 2
toplevel  three top-level forms                         -> 3
image     xxd ground.fkb                                -> magic "FKPIFB1 ... fkwu-image-v1 seed 8b24c31e58ce4..."
```

## 22. Limits of this study (stated so they can be moved)

- BML's own grammar and class model were read only where they touch Form (lowering memos, `offer-ack-core`, `choice-lane-core`); the BML study is a sibling's.
- The seed (34,149 lines) was read at the reader (`24990-25320`), the census (`22915-22975`) and by grep; the walker arms, the GC ("melt") and the JIT emitters were not read line by line.
- `format-arith.fk`, `jit-page.bml`, `bml-floor-compile.fk`, `channel.fk`, `wire-registry.fk`, `blueprint-registry.json` and `form/band-verdicts.txt` were cited through documents that describe them, not opened.
- Probes ran on one binary on one machine (macOS, arm64) against a shared node field already holding 21.7 million nodes; instance numbers are specific to that field.
- No band was run in the repository (a run writes caches beside sources); band verdicts are quoted from headers and documents, not re-witnessed.
