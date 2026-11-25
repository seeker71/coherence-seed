# Gap G1: the earlier in-house study of NUMS.Go (`nums-go-2023`) and its miniature

Written 2026-11-24. Closes gap G1 of `critic.md` (section 2, lines 50-64) and the first act of its section 5.3.

## 0. Paths, method, status words

- `F` = `Coherence-Network/docs/field/urs/artifacts/nums-go-2023`
- `CN` = `Coherence-Network` (branch `main`, HEAD `7eecf4da5`, 2026-11-08)
- `N` = `NUMS.Go` (HEAD `9e6bbb4`, 2023-04-05)
- `R` = `coherence-kernel`
- `S` = `scratch`
- `E` = `S/exp-mini-nums` (my scratch copy and probes)
- Round-one reports: `S/nums-core.md`, `S/nums-model.md`, `S/critic.md`.

Status words: **observed** (I read the line or ran the command in this session), **inferred** (reasoning, basis named),
**assumed** (not verified).

What I ran: `git` read commands in `CN`, `N`, `R`, `Living-Codex-CSharp`,
`<product checkout>`; `python3 -I -B` on the scratch copy `E/mini-nums` (Python 3.15.0); my own probe
`E/probe.py` on that copy; and the round-one scratch binary `S/../numscopy/tracebin` (the `nums-core` agent's dump
program linked against a copy of `N/nums`) on one new Go file, `E/numsgo-probe/twostructs.go`. I did not run `fkwu`.
I modified nothing in any repository: `git status --short` on `F` is empty after my work; on `N` it shows only the two
pre-existing `.DS_Store` files (observed). The four `.py` files in the copy have the same SHA-256 as the originals
(observed, `shasum -a 256`). No web request was made.

## 1. Verdict in brief

1. The folder is an **assistant-written study from 2026-05-08**, not a 2023 document and not the owner's own prose.
   It holds a README (narrative), study notes (a source walk of `N`), a design for a Coherence-Network "substrate",
   and `mini-nums`, a 687-line Python miniature.
2. **mini-nums does not execute recipes.** It builds and interns recipe trees and stops. There is no evaluator, no
   value, no state. `let x = 2 + 3 * 4` never yields 14. It is therefore *not* "the only executable NUMS on this
   disk" that the critic hoped for; no executable NUMS exists in anything I found.
3. All seven tests pass (15 assertions) once the copy's directory is on the module path. Plain `python3 -I
   test_mini.py` fails at import, because `-I` removes the script directory from the path.
4. The headline test ("cross-language equivalence", test 4) is a tautology: it compares the constant
   `BID_integer()` with itself and stays green when the JSON-Schema field is a string.
5. "Blueprint = ice, Recipe = water, NamedCell = gas" first appears in this folder on 2026-05-08, written by the
   assistant. NUMS.Go has none of the three words. The older in-house meaning (2025-09) is a persistence state of
   any node, and the body itself corrected the mapping on 2026-06-08: "STATE is orthogonal to SUBSTANCE".
6. The README makes four concrete claims that runs contradict: 14 languages wired; same-shape structs share one
   blueprint id; two globals with the same initializer share one constructor id; an unknown member "can't be
   expressed". Two of the four are contradicted by NUMS.Go itself on a run today.
7. The C++ NUMS is not on disk under `~/source`. What exists: 51 commented-out C++ lines in
   `N/nums/nums_db.go:224-272` and one commit subject. The tree-sitter fork is not on disk either; a document in
   `<product checkout>` gives its remote name.

---

## 2. What the folder is: files, dates, authorship

### 2.1 Files (observed: `find`, `wc -l`)

| File | Lines | What it is |
|---|---|---|
| `F/README.md` | 194 | narrative: "trinity", phase circulation, "AI grounding", lineage, table mapping NUMS to the Coherence Network |
| `F/study-notes.md` | 584 | "phase 1a": a walk of `N/nums` and `N/http` with type listings and pseudo-code |
| `F/network-substrate-design.md` | 264 | "phase 2": a category vocabulary and Postgres/Neo4j schema for a NUMS-shaped store of ideas, specs, memories |
| `F/mini-nums/core.py` | 355 | the miniature kernel |
| `F/mini-nums/calc.py` | 125 | a `let x = expr` front end |
| `F/mini-nums/jsonschema.py` | 42 | a JSON-Schema-to-blueprint front end |
| `F/mini-nums/test_mini.py` | 165 | seven tests |
| `F/mini-nums/README.md` | 73 | what the miniature claims to validate |
| `F/mini-nums/.gitignore` | 2 | `__pycache__/`, `*.pyc` |

### 2.2 History (observed: `git log --stat -- docs/field/urs/artifacts/nums-go-2023`)

Two commits touch the folder.

| Commit | Date | What |
|---|---|---|
| `02dd37592` | 2026-05-08 18:45 -06:00 | squash of pull request #1421, 57 files, 12,945 insertions; creates all nine files of the folder (1,804 lines) |
| `5fd7b8a0b` | 2026-06-15 17:38 +08:00 | pull request #3181; replaces the absolute home path with `~/source/NUMS.Go` in 13 README lines and 1 study-notes line; no other change to the folder |

### 2.3 Whose words

- **Git author** of both commits is the owner's GitHub account; **committer** of both is "GitHub" (a squash merge).
  (observed)
- The squash message of `02dd37592` contains the sub-commit messages. Each of the sub-commits that created these
  files ends with `Co-Authored-By: Claude Opus 4.7 (1M context)`. `5fd7b8a0b` carries co-author lines for the owner's
  second identity and for `Claude Opus 4.8 (1M context)`. (observed: `git show -s --format=%B`)
- The text speaks in the assistant's first person throughout: "any future session, mine or another presence's"
  (`F/README.md:3`); "Any agent operating on this body — me, Codex, Gemini" (`F/README.md:186`); "the gaps where my
  understanding might still be wrong" (`F/mini-nums/README.md:67`); "If a test fails, my understanding of NUMS is wrong
  at exactly that point" (`F/mini-nums/test_mini.py:4`); "Open design questions for Urs" (`F/network-substrate-design.md:245`).
  (observed)
- **Inferred** from the three points above: every sentence of prose in the folder was written by an assistant. No
  sentence in it is marked as a quotation of the owner.
- **The owner's own material inside the folder** is only the Go code the README quotes from `N` (all 60 commits of
  `N` are by the owner under two spellings of his name; observed, `git log --format=%an | sort | uniq -c` gives 51 + 9).
  Even there the README adds words the source does not have: `N/nums/recipe.go:497-502` is four uncommented fields;
  the comments "the cell IS a recipe (its access expression)", "(its tissue)", "(its seed)" at `F/README.md:31-34`
  and the numbered comments 1-6 at `F/README.md:62-89` are the README's. The source of `Make_Field`
  (`N/nums/emit_module.go:182-200`) has one comment, a commented-out line. (observed, both files)
- The README cites the struct at `nums/recipe.go:497-506`; the struct is lines 497-502 and `NewField` is 504-506.
  (observed)

### 2.4 The owner's own words about cells and states, found elsewhere (for contrast)

| Date | Source | Text | Status |
|---|---|---|---|
| 2023-02-22 | `N/query api and todo.txt:101-107` (commit `45322b7`) | a label list: "blueprint / built-in / declaration / implementation / recipe / cell / named" | observed |
| 2023-03-17 or earlier | `N/nums/reflect.go:3-6` | `type Cell_t struct { *Blueprint_t; Value any }` — a cell as a blueprint plus a value; unused | observed |
| 2023-04-02 to 04-05 | `N/test/sample.go:17,23,24` | builder comments `rp.cell("a")`, `rp.Select(rp.cell("a"), "a")` | observed |
| 2025-09-12 | `Living-Codex-CSharp/LIVING_CODEX_SPECIFICATION.md:572-575` (commit `6ea184a`, authored under the owner's name) | "Everything is a Node ... Ice (Persistent): Essential atoms in SQLite/PostgreSQL; Water (Semi-persistent): Materialized views with caching; Gas (Transient): Derivable, ephemeral state" | observed; whether typed by the owner or by an assistant is **assumed** unknown |
| 2026-03-23 | `CN` commit `4304a9856` (authored under the owner's name) | "Everything becomes a Node with TypeId, State (Ice/Water/Gas), Content, Meta."; "Ice/Water/Gas determines persistence." | observed (diff lines 25, 71) |
| 2026-05-29 | `CN/kernels/OBJECT_MODEL_BML_NUMS.md:3-6` | quoted as the owner: "nums.go has cells with methods on cells as reference implementation." | observed as a quotation in a document; the original utterance not seen |

---

## 3. mini-nums: what it is

### 3.1 The id scheme (observed, `F/mini-nums/core.py`)

- `NodeID` is a frozen dataclass of four integers `(package, level, type_, instance)`, printed `P.L.T.I`
  (`core.py:92-107`). `package` defaults to 1 and is 1 everywhere, for catalogue ids and minted ids alike.
- Levels: 1 trivial, 2 basic, 3 to 6 "complex" (`core.py:23-30`). Catalogue ids are hand-written constructor
  functions, for example integer blueprint `1.1.2.2`, object container `1.2.1.2`, math plus `1.2.1.1`, write-assign
  `1.2.2.5`, block-compound `1.2.3.2` (`core.py:112-133`).
- A composite's level is `max(child levels, category level) + 1`, floored at 3 (`core.py:138-144`, `:179-181`).
- A minted id is `1.<level>.<category's type>.<len(entries)+1>` (`core.py:158-168`, `:183`). The instance is an
  **arrival-order ordinal per level**; the key that makes two entries the same is the string
  `category + "+" + child + ...` (`core.py:147-149`).
- Two id spaces share one number format. `Module` holds `blueprint_db` and `recipe_db` (`core.py:192-193`).
  The run shows blueprint `1.3.1.1` (an object) and recipe `1.3.1.1` (`2 + 3`) in the same module
  (`E/probe.out`, P9; also visible across tests 2 and 5 of the test log).
- Literal values are not in the tree. `emplace_integer("5")` returns an ordinal from a per-module table
  (`core.py:201-204`); the leaf is `1.1.3.<ordinal>`.

Differences from NUMS.Go's scheme, each observed on both sides:

| Point | NUMS.Go | mini-nums |
|---|---|---|
| Package | 0 for the fixed catalogue, 1 for minted (`nums-core` 3; my run: `0.2.2.3+0.1.2.2^0`, minted `1.3.0.1`) | 1 for both |
| Type of a minted id | always 0 (`nums-core` 3; my run: every key is `0:n`) | the category's type (`core.py:183`) |
| Literal tables | per recipe (`nums-core` 4) | per module (`core.py:197-199`) |
| Structs / objects | nominal: each declaration gets its own id (section 6, D3) | structural: same child list, same id (`core.py:236-242`) |
| Edge bitmask on blueprint children | present (`ID^mask`) | absent (declared so in `F/mini-nums/README.md:50`) |
| Unresolved name | auto-declared (`nums-core` 5.1 item 6; `F/study-notes.md:252-271`) | `NameError` (`calc.py:120`; `E/probe.out` P10) |

### 3.2 What it says a blueprint, a recipe and a cell are

| Thing | Record | Docstring | Source |
|---|---|---|---|
| Blueprint | `(module, id, name)` | "Structural identity. What something IS (the ice phase)." | `core.py:221-226` |
| Recipe | `(module, category, blueprint, children, _self_id)`; `blueprint` is "the type this expression evaluates to" | "Operational expression. How something HAPPENS (the water phase)." | `core.py:248-259` |
| NamedCell | `(module, name, base, blueprint, access, ctor)`; `access` is a `Recipe`, `ctor` an optional `NodeID` | "Where structure and process meet, with name and seed (the gas phase)." and, on `access`, "reading the cell IS running this recipe" | `core.py:276-284` |

A global cell is made by `make_global_cell` (`core.py:293-317`): take the next global ordinal, build the access leaf
`1.1.8.<ordinal>`, and if there is an initializer intern `Block(access, Write(=, access, init))` and keep its id as
`ctor`. A field cell is constructed inside `make_object_blueprint` and **thrown away**: the local `cell` at
`core.py:331-334` is never stored ("We don't add to cells_by_name here — fields live inside the blueprint",
`core.py:335`), and the blueprint record has no place for it. After building three objects the module's
`cells_by_name` is empty and field names are nowhere (`E/probe.out`, P8). So in the miniature a struct's cells do not
exist after construction; only the ordered list of field blueprint ids does.

### 3.3 Does it execute? No.

- No function in `core.py`, `calc.py` or `jsonschema.py` evaluates anything. Of 49 callables defined in the three
  modules, the only name matching eval / exec / run / interp / value / step / apply / call is `RID_call`, a constant
  id constructor (`E/probe.out`, P1). (observed)
- `Recipe` has no value field; `NamedCell` has no value field (P1). (observed)
- `let x = 2 + 3 * 4` produces a cell `Cell(x: 1.1.2.2, ctor=1.6.3.1)`, four interned strings and the literal table
  `{'2': 1, '3': 2, '4': 3}`. The number 14 is computed nowhere (P2). `let z = 1 / 0` is accepted and interned (P10).
  (observed)
- `F/mini-nums/README.md:45-55` lists what is "intentionally simplified" (no tree-sitter, no edge byte, no
  resolution cascade, no build modes, no functions, no persistence, no labels). Execution is not in the list, because
  NUMS.Go has none to simplify. (observed; the "because" is inferred from `nums-core` 0.1)

mini-nums **represents** recipes. The sentence "reading the cell IS running this recipe" has no code behind it.

### 3.4 The test run (observed)

Copy: `cp -R F/mini-nums E/` then, from `E/mini-nums`:

| Command | Result |
|---|---|
| `python3 -I test_mini.py` | exit 1: `ModuleNotFoundError: No module named 'core'`. Isolated mode drops the script directory from `sys.path`, and the tests import their siblings by bare name. Log: `E/run-isolated-plain.log` |
| `python3 -I -B -c "import sys, runpy; sys.path.insert(0, D); runpy.run_path(D + '/test_mini.py', run_name='__main__')"` with `D` = the copy | exit 0, `ALL TESTS PASSED`. Log: `E/run-isolated-path.log` |

Note that `jsonschema.py` shares its name with a widely installed third-party package; with the copy first on the
path the local file wins. (observed that the local one loaded: the test uses `schema_to_blueprint`, which only the
local file defines)

Exactly what passed, 15 assertions in 7 tests:

| Test | Assertions printed | What the assertion really checks |
|---|---|---|
| 1 trivial dedup | `same '5' string emplaces to same instance: 1 == 1`; `two '5' literals share Recipe ID: 1.1.3.1` | a dict lookup returns the same ordinal twice |
| 2 recipe-tree dedup | `(2+3) shares Recipe ID: 1.3.1.1` | two hand-built `2 + 3` trees intern to one id. The parsed program on line 50 is not inspected |
| 3 cell birth | name, blueprint `1.1.2.2`, base undefined, CTOR not None, access not None (5) | fields were set |
| 4 cross-language | `calc int == json-schema integer: 1.1.2.2 == 1.1.2.2` | `BID_integer() == BID_integer()`; see 3.5 |
| 5 object dedup | `two structurally-identical objects share Blueprint NodeID: 1.3.1.1` | two equal child lists intern to one id |
| 6 composition | CTOR exists; `recipes_total >= 3` (actual 4, levels `{3:1, 4:1, 5:1, 6:1}`) | a count |
| 7 growth | 5 cells created; 5 cells in module | two counts. The comment says the shared `pi * two` is what is verified; it is not asserted |

For test 7 the lattice printed is `{3: 4, 4: 5, 5: 3}`, 12 recipes. By hand: level 3 holds write-pi, write-two, the
product and the quotient; so the product `pi * two` is indeed stored once for `circumference` and `almost`. The
sharing is real; the test does not check it. (observed numbers; the reading is inferred)

### 3.5 The "killer test" is a tautology

`test_mini.py:93-102`: `calc_int_id = cells[0].blueprint` and `json_int_id = BID_integer()`. The first is
`init.blueprint`, which `calc.py:114` sets to the constant `BID_integer()` for every literal. The second is the same
constant, called directly; the comment says "pull it out" but nothing is pulled out of `bp`, which is unused.
Probe P7 changes the schema's field to `{"type": "string"}`: the two ids the test compares are still `1.1.2.2` and
`1.1.2.2`, while the schema's actual blueprint is `1.3.1.1` holding `1.2.1.2+1.1.2.4` (object of string). (observed)

What it would take for the claim to mean something: two front ends each *deriving* a composite from their own
syntax and landing on one id. Test 5 does that for two JSON objects (one front end). No test does it across the two
front ends, and they cannot meet above the leaves: `calc` makes only recipes and global cells, `jsonschema` makes only
blueprints.

### 3.6 Properties the tests do not state, measured by probe (all observed, `E/probe.out`)

| Probe | Finding |
|---|---|
| P3 | `let a = 5` and `let b = 5`: `a.ctor = 1.4.3.1`, `b.ctor = 1.4.3.2`. Constructors of two globals with the same initializer are **different** nodes (the access leaf carries the global's ordinal) |
| P4 | Two modules, `let x = 5` and `let x = 7`: the node dumps are **identical**; only the literal tables differ (`{'5': 1}` against `{'7': 1}`). A recipe's meaning is tree plus table, as in NUMS.Go |
| P5 | The same two statements in swapped order give the same shapes different ids (`2 + 3` is `1.3.1.1` in one order and `1.3.1.2` in the other). Ids are arrival order |
| P6 | Renaming every identifier (German names) leaves the node dump identical. Names are a lens here too |
| P8 | `{a:int, b:str}` and `{x:int, y:str}` are one blueprint `1.3.1.1`; `{b:str, a:int}` is another, `1.3.1.2`. Field names are outside identity, field order is inside |
| P9 | Recipe and blueprint ids collide numerically (`1.3.1.1` in both) |
| P11 | `let x = 1`, `let x = 2`, `let y = x`: the name `x` is re-pointed to a new cell with a new global ordinal; the first constructor's nodes stay in the store. Rebinding moves a name, never a node |

P11 is the one behaviour in the miniature that matches the direction round one calls ready ("mutation lives only in
name bindings", `critic.md` 5.1 item 3). It is a side effect of `emplace_global` ignoring its `name` argument
(`core.py:216-218`), not a stated design. (observed code; "side effect" is inferred)

---

## 4. "Blueprint = ice, Recipe = water, NamedCell = gas": text and provenance

### 4.1 Exact text (observed)

- `F/mini-nums/README.md:61`: "- The trinity (Blueprint = ice, Recipe = water, NamedCell = gas)". This is the only
  place with that exact wording.
- `F/README.md:21-25`, a table with a "Phase" column: Blueprint "**ice** — frozen coordination"; Recipe "**water** —
  flowing transformation"; NamedCell "**gas** — diffuse individuation".
- `F/README.md:42`: "Phase transitions are encoded directly in the type relationships, not metaphorically", followed
  by four rows (`:44-49`): Recipe to Blueprint "condense / freeze" at `r.BlueprintEdgeNodeID_t`; Blueprint to Recipe
  "melt" at `Declare_Function`; Recipe to Cell "condense / individuate" at the CTOR pattern; Cell to Recipe
  "sublimate / diffuse" because `NamedCell_t` embeds `Recipe_t`.
- `F/mini-nums/core.py:223,250,278`: the three docstrings quoted in 3.2.
- `F/network-substrate-design.md:222`: "water phase — a flowing realization-recipe ... ice phase — frozen
  problem-identities".

### 4.2 Provenance (observed unless marked)

1. First appearance of the mapping: commit `02dd37592`, 2026-05-08, assistant-written (2.3). Its message says the
   commit "Establishes the ice/water/gas trinity (Blueprint/Recipe/NamedCell)".
2. NUMS.Go does not contain it. `grep -rniw -E 'ice|water|gas|phase|frozen|melt'` over the `.go`, `.txt`, `.md` and
   `.yaml` files of `N` returns nothing (my run; agrees with `critic.md` G8).
3. `CN/CLAUDE.md` at the parent of `02dd37592` has no line matching ice / water / gas / phase, although
   `F/README.md:53` says "This is the same physics as the body-tending practice in `CLAUDE.md`".
4. The three words were already in the house with another meaning: a **persistence state of any node** (Living
   Codex spec, 2025-09-12; `CN` commit `4304a9856`, 2026-03-23; both quoted in 2.4). A spec commit `d91ddded2`,
   2026-04-26, "Ice/Water/Gas phase states + breath cycle lifecycle for every entity", exists on side branches of
   `CN` and not on `main`; I did not read its body.
5. **Inferred** from 1 to 4: the mapping of the three *kinds* onto the three *states* is the assistant's
   2026-05-08 synthesis of two vocabularies that had been separate (NUMS's kinds, the Living Codex's states).
6. One month later the body corrected it: `CN` commit `93af83e75`, 2026-06-08, "STATE is orthogonal to SUBSTANCE —
   a Blueprint can be gas, a Recipe can be ice"; `CN/docs/coherence-substrate/INDEX.md:134` now reads "The body's
   'Blueprint=ice' names each kind's resting tendency — the DIAGONAL of a 3×3 grid". The kernel carries the corrected
   form in `R/form/form-stdlib/substrate-phase.fk:4-14` (first commit 2026-07-02): "STATE (ice/water/gas) is orthogonal
   to SUBSTANCE (Blueprint/Recipe/NamedCell) ... A phase change moves a cell along the state axis WITHIN its
   substance".
7. The uncorrected form also survives in the kernel: `R/form/form-stdlib/grammars/living-equation.fk:7-13`
   ("structure ↔ Blueprint (ice) ... choices ↔ Recipe (water) ... the gas/NamedCell layer"). This is "definition E"
   of `form-language`'s table (`critic.md` A8, G1). **Inferred**: it descends from this folder.
8. The fixed axioms contain none of the three words (`grep -n -i 'ice\|water\|gas' R/axioms/core-axioms.form` returns
   only lines where "choice" matches "ice"; observed).

### 4.3 Is "encoded directly in the type relationships" grounded? No.

Each of the four rows points at an ordinary structural fact of `N`: an expression carries its type
(`BlueprintEdgeNodeID_t` inside `Recipe_t`); a function is recorded as a signature blueprint and a body recipe
(`nums-core` 5.2); a global has a named slot and an initializer recipe; `NamedCell_t` embeds `Recipe_t`
(`N/nums/recipe.go:497-498`). None of them is a transition: nothing in `N` or in mini-nums changes a node from one
kind to another, nothing is run, and "reading the cell IS running its access-recipe" (`F/README.md:49`) has no
reader. (observed absence of any evaluator in both code bases; the judgement is inferred)

---

## 5. Claims about other languages and natural language

| # | Claim | Where | Grounded? | Evidence |
|---|---|---|---|---|
| L1 | "It supports 14 languages already wired through the `parser.go` dispatch", then 13 names | `F/README.md:9-11`; "14" again at `:127,145,174`; `F/study-notes.md:98,328` | **No** | `N/nums/parser.go:31-45` enumerates 13 (observed). `NewFileParserFromFile` has one case, `Language_Go`; every other language returns "NUMS Language: '%s' not supported" (`N/nums/parser.go:453-460`, observed). Grammars are linked; handlers exist for Go only |
| L2 | "Verilog is in the list, which is the tell: the ... vocabulary is general enough to hold hardware description" | `F/README.md:13` | **No** | Verilog is an enum entry and a grammar import from the private fork (`nums-core` 7.1); no handler |
| L3 | "Adding a 15th language = writing a `RegisterPython()`"; "The kernel doesn't know about any specific language" | `F/study-notes.md:328` | Half | The dispatch table is per-language (true). But the core hard-codes Go spellings and Go built-ins (`nums-core` 4, last paragraph; `nums-model` 8 "compiled in and Go-specific") |
| L4 | "71 visitors" | `F/study-notes.md:28,328` | Miscount | `RegisterGo` fills 75 entries (`N/nums/go.go:1425-1502`; my count of `NodeProcess[` lines; `nums-core` 5.1) |
| L5 | "A Python class and a Go struct with the same field-shapes hash to the same Blueprint ID ... Translation preserves meaning by construction" | `F/README.md:127` | **No**, and contradicted | No Python handler exists. Two *Go* structs with the same field shapes get **different** ids in NUMS.Go (section 6, D3) |
| L6 | "Two functions that do the same thing in different surface syntax retrieve as one memory because their interned Recipe IDs match" | `F/README.md:128` | **No** | Sharing is by identical tree shape only; literal values are outside the tree, so `a + 1` and `b + 2` also match (`nums-core` 6.4; my P4) |
| L7 | "Ill-formed code becomes literally unproduceable"; "Hallucination becomes architecturally impossible ... If it doesn't, the proposal can't be expressed in the lattice" | `F/README.md:126,129` | **No**, and contradicted inside the folder | `F/study-notes.md:271`: "The substrate auto-grows when an unknown identifier is encountered." An unknown `user.email` is auto-declared as a member (`N/nums/emit_recipe.go:247-304` per `nums-core` 5.1 item 6) |
| L8 | "the model is universal"; "The kernel is universal" | `F/README.md:186`; `F/network-substrate-design.md:5`; `F/mini-nums/calc.py:13` | Asserted | One Go subset in `N`; a four-operator calculator and a six-type schema reader in the miniature |
| L9 | "cross-language structural equivalence" demonstrated by test 4 | `F/mini-nums/README.md:35-43`; commit message of `02dd37592` ("Proves cross-language structural equivalence") | **No** | Tautology (3.5) |
| L10 | NUMS "scaled out from code-language to human-meaning"; concept stories as ice, guides as water, encounters as gas | `F/README.md:136-149` | Analogy only | A two-column table; no code in the folder reads prose |
| L11 | "Two specs that mean the same thing but were written by different agents at different times surface as the same node"; "Cross-document semantic search by structural shape" | `F/network-substrate-design.md:210`, `:214-216` | **No** | The design's own identity rule is "the spec's identity *is* its frontmatter shape" (`:45`) and it concedes "a Memory and an Idea with identical frontmatter shape would technically share an identity" (`:47`). The commit message of `02dd37592` reports the built result: of 40 memory files "38 are structurally equivalent — they share Blueprint NodeID 1.5.4.1 because their frontmatter (name + description + type + body) has identical shape". Same keys, not same meaning (observed as text; the built substrate not run by me) |
| L12 | Natural language as such | whole folder | **No claim made** | The words "natural language" do not occur. The folder's reach beyond code is markdown files with YAML frontmatter, treated as records with named fields |

Summary: for programming languages the folder asserts generality that NUMS.Go's code and runs do not show; for
natural language it makes no technical claim at all, only an analogy between a struct's fields and a document's
frontmatter keys. The round-one finding stands: nothing on this disk puts a second programming language, or any
natural language, through a NUMS-shaped store (`nums-core` 9.14; `nums-model` 8; `critic.md` G3, G4).

---

## 6. Where the folder disagrees with the two round-one NUMS reports

For each: what the folder says, what round one says, and what I observed today.

| # | Topic | 2026-05 folder | Round one | Settled by |
|---|---|---|---|---|
| D1 | Is it working? | "a **working model** — not a design document, not aspirational architecture" (`F/README.md:9`) | Does not build as checked in; panics on its three shipped samples (`nums-core` 7.1, 7.3) | Round one's runs. The folder records no run of `N` |
| D2 | Was it ever built and run by the earlier study? | Commit message of `02dd37592`: "Phase 1b: built and ran NUMS.Go against sample.go. Real lattice produced: 44 named blueprints, 97 unique structural shapes, 479 unique recipe shapes. Confirms every invariant documented in phase 1a." | `sample/sample.go` "records 36 functions and 547 nodes, then **panics**" (`nums-core` 7.3) | **Unverifiable.** No phase-1b output is in the folder; `F/study-notes.md:579-582` still lists 1b as "next"; `grep` for "479 unique" or "Phase 1b" under `CN/docs` and `CN/kernels` finds only that heading. A build needs the absent fork or the patch round one made. The two sets of counts are not comparable as stated |
| D3 | Struct identity | "The struct's identity includes its cells — change one, get a different Blueprint ID" (`F/README.md:97`); "identity = serialized(Category + ordered child Blueprint IDs)" (`F/study-notes.md:311`) | Structs, interfaces, packages and function declarations are **nominal** and mutated in place under a stable id (`nums-core` 3) | **My run** of the round-one dump on `E/numsgo-probe/twostructs.go` (`type A struct{x, y int}`, `type B struct{p, q int}`): `A -> 1.3.0.1`, `B -> 1.3.0.2`, both storing `0.2.2.3+0.1.2.2^0+0.1.2.2^0`. Same shape, two ids. Mechanism: `Should_Reserve` is true for object, interface, package (`N/nums/nums_nodes.go:719-721`) and for function declarations (`:272-274`); the empty reserved entry is stored as `value=key` so it can never be found by value, then overwritten by `Update_Entry` (`N/nums/nums_db.go:69-72`, `:82-85`). Round one is right; the folder (and mini-nums test 5) is wrong about NUMS.Go |
| D4 | Constructor sharing | "**Two globals with the same constructor shape share the same RecipeNodeID.**" (`F/README.md:95`) | not addressed directly; `nums-core` 5.2 says the function recipe has "a forced unique ID" | **My run**, same file, `var g1 = 5` and `var g2 = 5`: `CTOR:g1 -> recipe 1.3.0.2`, `CTOR:g2 -> recipe 1.3.0.4`; the bodies differ in the access leaf (`0.1.13.1` against `0.1.13.2`). They share only the signature blueprint `1.3.0.3` (count 2). The miniature agrees with NUMS.Go here and disagrees with its own README's parent (P3; and `test_mini.py:149` says "The CTORs are different (different LHS)") |
| D5 | "Content-addressed" | used throughout: "content-addressed numeric lattice", "content-addressed, permanent, re-derivable" (`F/README.md:1,81`) | an id is "the Nth distinct shape of this height seen in this module ... local to one in-memory module and one run"; numbering varies run to run (`nums-core` 6.4, 9.1) | Round one. `F/study-notes.md:117-121` itself describes `len(Entries) + 1`. Three runs of my probe file gave three different whole-output digests; the one diff I inspected is a reordered line in a map dump, so I do not claim the node numbering moved in this file (observed digests; cause not isolated). mini-nums P5 shows order dependence directly |
| D6 | Languages | 14 wired (L1) | 13 enumerated, Go only has handlers (`nums-core` 5.1; `critic.md` N8) | Observed today: 13, one case |
| D7 | The cell | NamedCell is one of three primitives, with membrane, cytoplasm, nucleus (`F/README.md:25-38`) | `nums-core` 0.1 and 8: "Cells: ... There is no value, no instance, no state"; `nums-model` 1: Cell is one of three label roots (storage: globals, locals, parameters, members) | Both partly right. The folder is right that the record exists and is used (`N/nums/recipe.go:497-502`; uses at `blueprint.go:73,93,172-184`, `emit_module.go:35,198,243,320`, `parser.go:496`; observed). `nums-core` is right that there is no value or instance. The critic's corrected wording (G2: "a declared named slot") is the accurate one. The owner's own unused `Cell_t {*Blueprint_t; Value any}` (`N/nums/reflect.go:3-6`) is the only place a cell holds a value, and neither the folder nor mini-nums mentions it |
| D8 | "There are **no strings of identity** anywhere in the body" | `F/README.md:114` | names and literal text live in side tables; the store is keyed by strings of numbers (`nums-core` 3, 4) | Agreement in substance. Exception neither notes together: `NamedCell_t.Name` is a string inside the cell record (observed) |
| D9 | Level | "Levels emerge bottom-up ... no manual annotation" (`F/study-notes.md:169`) | adds the exception: reserved nodes are level 3 whatever their depth (`nums-core` 2.2) | Round one is more complete; my run shows function declarations at `1.3.0.n` over level-4 and level-5 bodies |
| D10 | Build modes | "The 16 `BuildMode` values", then 11 names (`F/study-notes.md:205`) | "declaration, body, parameter, return, call, init, var-decl, key" (`nums-core` 5.2) | The enum has 11 values plus `None` (`N/nums/emit_module.go:9-22`, observed). Both are loose; neither matters |
| D11 | Size of a port | "Roughly 4500 LoC for the kernel + emit layer" (`F/study-notes.md:565`) | `nums/` is 7,045 lines of which `go.go` is 1,502 (`nums-core` 1) | Compatible (the folder's table omits `go.go`, `parser.go`, `emit.go` remainder) |
| D12 | Second generation (`http/`) | "Labels are a *separate hierarchy*" describing "what code means"; "The label assignment is data-driven"; "REST API ... read-only" (`F/study-notes.md:510-548`) | agrees, and adds what the folder lacks: two models side by side with the new one regressed at HEAD, ids that are counters or byte positions, engine code that is Go-specific behind the config (`nums-model` 1, 8, 11) | No contradiction; the folder is a listing of types, round one ran it |
| D13 | Open gaps | persistence, concurrency, import stub, grammar-version-bound symbol ids, label rules (`F/study-notes.md:569-577`) | the same five, plus determinism, literals, nominal mutation, two id spaces, no semantics, no cells with state (`nums-core` 9) | Agreement on the five. The folder's line "none is an architectural unknown" (`:577`) does not survive round one's list |

What the folder has that round one did not state:

- The cell-birth sequence written out step by step (`F/README.md:55-97`; `F/study-notes.md:273-311`). It matches
  `N/nums/emit_module.go:182-200` line for line apart from the added comments (observed).
- The link from `NamedCell{access recipe, base blueprint}` to BML's reference pair (data id, interface id). It is made
  in the commit message of `02dd37592` ("BML-style Views") and in `CN/kernels/OBJECT_MODEL_BML_NUMS.md:81-110`
  (2026-05-29), not in the folder's files. That later document names only this folder as its NUMS source
  (`:14`) and repeats D3, D4 and L5 as facts ("Two globals with the same init share one CTOR NodeID", `:103-104`;
  "Two structs with the same field+method shape content-address to the *same* Blueprint", `:98-100`). It also
  describes a dispatch NUMS.Go does not have: "`obj.m()` → `obj.Base` → `NamedRecipes["m"]` → invoke with obj as
  receiver" (`:93-95`); nothing in `N` invokes. (observed text; the absence is `nums-core` 0.1)

---

## 7. What happened next (short, because it explains why the folder matters)

Observed from the squash message and file list of `02dd37592` and from `CN` today:

- The same pull request that added this folder added `api/app/services/substrate/` (a Python kernel, ORM, markdown
  front end), `docs/coherence-substrate/form-language.md`, and `form.py` with a lexer, parser and evaluator. The
  message says: "Form, the substrate-native language for agent ↔ lattice interaction. NodeIDs first-class; @1.5.4.1
  is a literal" and "Form is now executable." So **Form's first appearance and this NUMS study are one commit**.
- `CN/docs/coherence-substrate/form-language.md` exists since `02dd37592` (2026-05-08); its current head says
  "`fkwu` is its sole live kernel".
- `api/app/services/substrate/kernel.py` is 798 lines today; the `form*.py` files of the original commit are no
  longer in that directory. I did not read or run the Python substrate.
- **Inferred**: the four-word id `(package level type instance)` in the fixed axioms (`R/axioms/core-axioms.form:59`,
  per `nums-core` 2.1) reached the kernel through this folder's reading of NUMS.Go, including its errors about what
  the fields mean (package 1 everywhere, type from the category), not from NUMS.Go directly. This would explain the
  field-by-field disagreement the critic lists under C2. I did not trace the intermediate commits.

---

## 8. The C++ NUMS and the tree-sitter fork

Searched under `~/source` (observed, 2026-11-24):

| Looked for | How | Found |
|---|---|---|
| A directory or file named like a C++ NUMS | `find -maxdepth 4 -iname '*nums*'` | only `N` itself, `CN/kernels/OBJECT_MODEL_BML_NUMS.md` (and a worktree copy), nine `<product checkout>/docs/apis/dif-nums-*.md` files, and unrelated `sync-numstat` files |
| NUMS identifiers outside `N` | `grep -rIl 'RecipeNodeID\|BlueprintNodeID\|TreeLevelDB\|NamedCell_t\|Make_SelfID'` | this folder; `CN/kernels/*.md`; `CN/api/app/services/substrate/orm.py`; three `.bml` files repeated in each coherence-kernel checkout (`operator-type-id.bml`, `bml/native-model-token-flow.bml`, `bml/form-node-category.bml`); no C or C++ file |
| Mentions of a C++ NUMS | `grep -rIl -i 'C++ NUMS\|ported from C++\|nums.cpp'` over `N`, `<product checkout>/docs`, `CN/kernels`, `CN/docs/field`, `CN/docs/coherence-substrate` | no file. In `N`'s history: commit `b508f08`, 2023-02-08, "translate C++ NUMS Go handlers" |
| C++ remnants in `N` | read | `N/nums/nums_db.go:224-272`: two commented-out C++ member functions, `TreeDB_t::get_TypeInfo(Module_t const&, id_t const&)` and `TreeDB_t::get_AllIDs()`, using `type.category_id_`, `type.nodes_`, `type.member_ids`, `level_entries`. They show the C++ original had the same `TreeDB_t` / level-entries design with "Type" where Go says "Blueprint" |
| `<product checkout>` | top-level `ls`; `grep -w 'NUMS\|nums'` over `.md .cpp .hpp .h .cc .go .txt` excluding `.git-root` | twelve documents and no source file. `docs/architecture/repos/nums-go.md` (2026-02-18): "Responsibility: numeric tooling experiments ... Legacy / experimental ... Maintained: No", remote `<remote>/NUMS.Go`. `docs/apis/dif-nums-technical.md` (2026-03-20/21) describes an API "in `MP-CodeCheck` using NUMS-style semantic types rather than raw parser node labels", internally "tree-sitter and `CtrlExpr`", and links to `debugging/MP-CodeCheck/src/HTTP.cpp`. The directory `<product checkout>/debugging/MP-CodeCheck` exists; my grep found the word NUMS in none of its C++ files |
| The tree-sitter fork `<tree-sitter-fork>` | `ls ~/source`; `find -maxdepth 6 -type d -iname '*tree-sitter*'` | **absent**. The only matches are data directories `<product checkout>/Mentor.Bridge/.git-root/repo(N)/topics/tree-sitter`. `<product checkout>/docs/architecture/repos/go-tree-sitter.md` names a remote, `<remote>/go-tree-sitter`, "tree-sitter Go bindings mirror ... Maintained: No" (not fetched). The Go module cache holds upstream `smacker/go-tree-sitter` at the pinned 2023-01-13 version and a 2024-08-27 one, no fork |

So: **no C++ NUMS source and no fork on disk under `~/source`**. Whether `MP-CodeCheck`'s `CtrlExpr`
and "NUMS-style semantic types" are the C++ line that `N` was translated from is **assumed** possible and unread: it
is a product tree outside this study, and I opened one document of it. Two home-level directories with the company's
name exist outside `~/source` (seen in a directory listing); I did not look inside them, as the task
bounded the search.

---

## 9. What this changes for the new design

Carry forward:

1. **The cell as a four-part declared slot**: access recipe, base blueprint, name, constructor recipe, with the
   constructor stored as a recipe and referred to by id. This is NUMS.Go's own record (`N/nums/recipe.go:497-502`)
   and the one thing the miniature reproduces faithfully and tests (test 3).
2. **Name rebinding re-points a name and leaves nodes in place** (P11). Make it a stated rule.
3. **Field names outside identity, field order inside** (P8), as a *decision to make consciously*: it is what both
   NUMS.Go and the miniature do, and it means `{x, y}` and `{p, q}` are one shape.
4. **The miniature as a test-writing lesson**: seven green tests, of which one is a tautology and two assert only
   counts. A claim like "same id across two surfaces" needs a test where each side derives the id and where changing
   one side turns it red.
5. **State orthogonal to kind**: the body's own correction of 2026-06-08, carried in `substrate-phase.fk`.
6. **The earlier persistence meaning of ice / water / gas** (2025-09) as a candidate for the one definition the
   critic asks for (N3): it is the only one of the nine that names an operational property (where a node is stored
   and whether it can be re-derived).
7. The owner's unused `Cell_t {*Blueprint_t; Value any}` as the seed of a run-time cell: a blueprint plus a value.

Let go:

1. "Blueprint = ice, Recipe = water, NamedCell = gas" as a definition. It is an assistant's 2026-05 analogy, absent
   from NUMS.Go, absent from the axioms, and already withdrawn by the body.
2. "Phase transitions are encoded directly in the type relationships." Nothing transitions.
3. "NUMS.Go is a working model" and "14 languages".
4. "Content-addressed" as a word for arrival-order interning. Keep the word for ids computed from content.
5. "Same shape, same id, across languages, for free." Not true of structs in NUMS.Go; not shown across two
   surfaces anywhere.
6. "Hallucination becomes architecturally impossible." The reference auto-declares what it does not find.
7. Treating this folder, or `CN/kernels/OBJECT_MODEL_BML_NUMS.md` built on it, as a source about NUMS.Go. Cite
   `N` and round one.
8. A struct's cells that exist only during construction (the miniature's `make_object_blueprint`).

---

## 10. Open questions

1. Was NUMS.Go really built and run on 2026-05-08 ("44 named blueprints, 97 ... 479 ...")? No artifact remains. If
   it was, with which tree-sitter binding, and why did it not panic where round one's build does?
2. Where is the C++ NUMS? Candidates not opened: `<product checkout>/debugging/MP-CodeCheck` (the `CtrlExpr` code), the
   remote `<product checkout>` repositories, the two company-named directories in the home folder. Does it execute anything?
3. Did the owner ever state the trinity-to-phase mapping himself, or only the persistence meaning? Only he can say.
   The quotation of 2026-05-29 ("cells with methods on cells") suggests his own picture of a cell is richer than
   either NUMS.Go's record or the miniature.
4. Should struct identity be nominal (NUMS.Go as built), structural (the miniature, and the README's belief), or
   structural with a separate nominal name layer (`nums-core` 9.4)? The folder shows how easily the two are confused.
5. Are field names part of a blueprint's identity? Both references say no; the consequence (P8) was never stated.
6. What does the 2026-04-26 breath-cycle spec (`d91ddded2`, side branches only) define? Unread.
7. How much of the kernel's present id layout came through this folder rather than from NUMS.Go (section 7,
   inferred)? Needs a walk of `CN` history between 2026-05-08 and the kernel's split.
8. What does the Python substrate that followed (`CN/api/app/services/substrate/kernel.py`, 798 lines) do today, and
   did its `form.py` evaluator ever evaluate a recipe, or only queries over the store? Unread.

## 11. Surprise, and what stayed uncomfortable

Most surprising: the document that named NUMS.Go "a working model", gave the kernel its ice / water / gas trinity and
introduced Form in the same commit, was written by an assistant in one day from a reading of the source, and its
proof of "cross-language equivalence" is a test that compares a constant with itself. Two of its stated facts about
NUMS.Go (same-shape structs share an id; same-initializer globals share a constructor) are false on a run of
NUMS.Go today, and a later design document copied them as the specification "the kernel's object primitive must
converge to".

Uncomfortable:

1. I cannot verify or refute the commit message's claim that NUMS.Go was built and run in May.
2. I judged authorship from co-author lines and first-person voice. That is strong evidence, not a signature; the
   owner may have dictated or approved any sentence.
3. My NUMS.Go check used the round-one scratch build (four build lines removed, upstream grammar instead of the
   fork). It is the same caveat round one carries.
4. The inference in section 7 (the axioms' id words arrived through this folder) is a hypothesis with no commit
   trail behind it yet.
5. The search for the C++ original stopped at the boundary I was given. The most likely places were not opened.

## 12. Files produced (all in the scratchpad)

- `S/exp-mini-nums/mini-nums/`: the copy (byte-identical `.py` files).
- `S/exp-mini-nums/run-isolated-plain.log`: `python3 -I -B test_mini.py`, exit 1.
- `S/exp-mini-nums/run-isolated-path.log`: the passing run, exit 0.
- `S/exp-mini-nums/probe.py`, `probe.out`: probes P1 to P11.
- `S/exp-mini-nums/numsgo-probe/twostructs.go`, `run1.out`, `run2.out`, `run3.out`: the NUMS.Go check for D3 and D4.
