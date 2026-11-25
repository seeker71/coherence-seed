# Ice / water / gas: origin, the owner's own words, and one definition

Written 2026-11-24. Closes gap G8 / not-ready item N3 / contradiction C4 of `critic.md`.

## 0. Paths, method, status words

- `R`  = `coherence-kernel` (HEAD `b8ff978c9`, 2026-11-24)
- `CN` = `Coherence-Network` (HEAD `7eecf4da5`, 2026-11-08; 6,005 commits; first 2026-02-11)
- `LC` = `Living-Codex-CSharp` (HEAD `758dcb5`, 2025-10-13; 222 commits; first 2025-09-09)
- `CC` = `Crypo-Coin-Coherency-Network` (HEAD `70ee20a`, 2026-02-12; 34 commits; first 2024-06-22)
- `N`  = `NUMS.Go` (HEAD `9e6bbb4`, 2023-04-05)
- `M`  = `memory`
- `S`  = the study folder this report is in.

Method: word-boundary searches (`rg -i -w`) for `ice`, `gas`, `plasma`, and substring searches for `melt`, `freez`,
`frozen`, `thaw`, `sublimat`, `crystalli`, `evaporat`, `condens`, `phase` over the paths the task names; `git log -S/-G`,
`git blame`, `git show <commit>:<file>` and commit messages for dates and authorship. Nothing was run from the old
runtime; no repository file was modified; one Spotlight query (`mdfind`) looked for an older copy of the 2025 spec and
found none outside `LC` and `CN`.

Status words: **observed** = I read the cited line or commit in this session; **inferred** = my reasoning, basis named;
**assumed** = not verified.

Authorship words used below:

- **owner-quoted**: a sentence in quotation marks attributed to the owner by name in a commit message or file.
- **owner-paraphrased**: attributed to him by name, not in quotation marks.
- **human-authored commit**: commit author is the owner's identity and the commit carries no assistant co-author trailer.
  In `LC` every one of 221 commits is like this (observed: `git log --format='%an|%(trailers…)'`), but the first commit
  ships a `.cursor/rules` file, so the *text* may still be assistant-drafted (inferred). In `R`, 1,634 commits carry the
  owner's git name and no trailer, which cannot tell a human from an agent that writes no trailer (observed count;
  inferred caveat).
- **assistant**: the commit carries a `Co-Authored-By: Claude …` or `Cursor` trailer, or the author is `Claude`, and
  the text is not presented as the owner's words.

Blame caveat (observed): most lines of `R/form/form-stdlib/*.fk|*.bml` blame to 2026-09-29/30, the day of a mass
re-spelling (`1978af055`, "Lift 8436 core definitions…"). For those files I give the file's creation commit in `CN`
(the kernel tree was imported from `CN` on 2026-07-02, `6151af27c`) as the first-seen date.

---

## 1. Findings in brief

1. **Origin (observed).** The oldest use on this disk is the very first commit of the Living Codex, `LC 5fc24ba`,
   2025-09-09 00:58 -0600, author Urs Muff, no assistant trailer. Its spec defines the three words in one line:
   "**Ice** = essential persisted facts (atoms, deltas). **Water** = materialized caches/prototypes. **Gas** =
   derivable/transient." (`FRACTAL_UNFOLDING_SPEC.md:24` at that commit). The same commit has the enum
   `ContentState { Ice, Water, Gas }` and a `PhaseEngine` with `Melt` and `Refreeze`. That spec says it is aligned to a
   "Living Codex — Self-Reflexive Foundation 4.1" whose section 4 was titled "Lenses (Ice/Water/Gas)" (`:346,356`);
   that older document is not on this disk (observed absence; its content is unknown).
2. **The vocabulary is not NUMS's (observed).** `rg -i -w 'ice|gas|plasma|melt|frozen|freeze|thaw|sublimate'` over `N`
   returns nothing. The mapping Blueprint = ice, Recipe = water, NamedCell = gas was written onto NUMS on 2026-05-08 by
   an assistant (`CN 02dd37592`, trailer Claude Opus 4.7: "Establishes the ice/water/gas trinity").
3. **The owner rejected the trinity mapping in his own words (observed).** 2026-06-08: "not sure all blueprints are
   ice and all recipes are water; I feel there are blueprints in water and gas as well, and recipes in all 3 states."
   (`CN 93af83e75`). A later commit under his identity calls the binding "The misunderstanding" (`CN 04e49aff2`,
   2026-06-14).
4. **The owner never wrote a definition file himself that I could find.** His attested contributions are: the 2025
   spec and code committed under his name; and seven short statements quoted or paraphrased by assistants between
   2026-05-27 and 2026-09-08 (section 3). Every long definition after 2025 is assistant text.
5. **I count 21 distinct uses on 12 axes** (section 2). Twelve are compatible readings of one idea (how far a node is
   committed: to no one, to its holder, to everyone). Five truly conflict with it. Four are loose metaphor.
6. **Contradiction C4 has an owner-side answer (observed + inferred).** In `LC 721e2c7` (2025-09-12, human-authored
   commit): "Spec (Ice): Persistent, immutable source of truth / Code (Water): Generated from spec, can be regenerated /
   DLL (Gas): Compiled from code, ephemeral runtime artifacts … Only specs (ice) need to be persistent". Compiled
   machine code is **gas** in the oldest definition. The 2026 body's "a hot composite crystallizes … melts on cool"
   (`R/axioms/host-kernel.form:29-30`) uses a different axis (settledness by heat) and is assistant text.

---

## 2. The table: every distinct definition

Columns: axis; ice / water / gas (/ plasma); transitions named; enforcing machinery; first seen; whose words.
All rows observed unless marked.

| # | Axis (what the state measures) | Ice | Water | Gas | Transitions named | Machinery | First seen | Whose words |
|---|---|---|---|---|---|---|---|---|
| D1 | **Persistence and derivability of a node's content** | "essential persisted facts (atoms, deltas)"; "Frozen, immutable state"; "frozen/spec" | "materialized caches/prototypes"; "Liquid, mutable state" | "derivable/transient"; "Transient, derivable state" | `melt` (ice→water, "for editing"), `refreeze` (water→ice, after a resonance check against "anchors"), `hydrate` ("promote Gas/Ice → Water via adapters/synthesizer") | `enum ContentState`; `PhaseEngine.Melt/Refreeze`; later `PhaseModule` refusing melt of non-ice and refreeze of non-water; three storage backends (PostgreSQL / SQLite / memory); rule "an edge persists in the most fluid state of its endpoints (Gas > Water > Ice)" with tests; `IceNodeAuditModule` that demotes "generated/regenerable nodes from Ice to Water" | 2025-09-09 (`LC 5fc24ba`) | human-authored commits; text possibly assistant-drafted |
| D2 | **Stage in a spec→code→runtime pipeline** | "Spec (Ice): Persistent, immutable source of truth" | "Code (Water): Generated from spec, can be regenerated" | "DLL (Gas): Compiled from code, ephemeral runtime artifacts" | `generate-code` (ice→water), `compile-code` (water→gas), `regenerate` (everything from ice) | `SpecDrivenArchitecture.cs` (`RegenerateFromSpecs`) | 2025-09-12 (`LC 721e2c7`) | human-authored commit |
| D2' | same axis, later wording | "Specs in Git" | "Generated code … materialized … event store" | "Live coherence calculation … runtime state in memory" | Melting (ice→water), Evaporating (water→gas), Condensing (gas→water), Freezing (water→ice); `CC` adds Sublimation (ice→gas) and Deposition (gas→ice) | none (`IPhaseManager` is a planned interface) | 2026-02-11 (`CN a89841068`, `62c1c64c0`); 2026-02-12 (`CC 148beed`) | assistant (Cursor trailer; author `Claude`) |
| D3 | **Lifecycle of an idea in the Network app** | "Specification phase — frozen potential, not yet implemented" (and, in another service: "crystallized … has actual_value > 0") | "Active phase — being implemented, run, or used" | "Ideation phase — unformed, pre-specification"; "exploratory" | order gas→ice→water→gas ("crystallize", "melt", "evaporate") | `LifecycleState` enum; `breath_service._classify_spec`; a Shannon-entropy "breath" score over the three counts | 2026-03-25 to 2026-04-09 (`CN cdc7a3005`, `5871e0cc8`, `2555b3c34`) | assistant |
| D4 | **Kind of node (the trinity)** | Blueprint — "frozen coordination" | Recipe — "flowing transformation" | NamedCell — "diffuse individuation" | "Recipe → Blueprint (condense / freeze)", "Blueprint → Recipe (melt)", "Recipe → Cell (condense / individuate)", "Cell → Recipe (sublimate / diffuse)" | none; the README claims the transitions are "encoded directly in the type relationships" of NUMS | 2026-05-08 (`CN 02dd37592`) | assistant; **rejected by the owner 2026-06-08** |
| D5 | **Depth of ingestion of a file ("altitude")** | plus a Blueprint capturing content shape | plus an operational recipe | a named cell with path and hash only: "Gas is enough to start" | none | `ingest_git_artifact` makes gas cells only | 2026-05-16/20 (`CN c0876b398`) | assistant; the epigraph in `lc-form-perceptron.md:25-30` is unattributed |
| D6 | **Cache tier** | "the .json (or whatever source format) is the canonical ice" | "the in-memory Recipe is water (lives in the process)" | "lookup results are gas (materialised on demand)"; header: "Less ice, more water, more gas" | none | `read_with_cache` (mtime compare) | 2026-05-26 (`CN 402acf0bc`, author `Claude`) | assistant; the slogan echoes the owner's goal of 2026-05-27 (O2 below) |
| D7 | **How settled a cell is, read from counts; orthogonal to kind** | "frozen: stable, load-bearing, immutable, widely referenced" | "fluid: actively circulating — read, changed, composed" | "diffuse: potential — defined or interned, barely instantiated" | `condense` (gas→water), `freeze` (water→ice), `melt` (ice→water), `sublimate` (ice→gas, "Removes NOTHING — NodeID and kind persist"); hysteresis; conservation law | `substrate-phase.fk`: `phase-state` and `phase-invite` are pure functions over (degree, churn); "it never MUTATES"; a band. Not applied to real nodes (round one, `S/form-language.md:133`) | 2026-06-08 (`CN 241b3e3e4` → `a3929fff8` → `93af83e75` → `274c61a1c`, four commits in 62 minutes) | assistant text **shaped by three owner statements** (O3–O6) |
| D8 | **Phase of a decided value (policy cell)** | "frozen default a carrier reads" | "MELT to re-tune, then FREEZE" | "queryable potential" | melt, freeze, "moving by deliberate choice"; "never be forced into ice" | a TypeScript constant re-derived from the policy | 2026-06-08 (`CN ab3796ea3`) | assistant |
| D9 | **What crosses between runtimes** | "the same merkle on every runtime by definition; shared for free" | "shares by RE-ADDRESSING: send the structure … the receiver re-interns it" | "runtime-LOCAL by nature … Gas doesn't cross; its water does" | serialize, re-address on receipt | `merkle.fk`, `cell-sync.fk` (have/want) | 2026-06-10 (`CN cfbfec072`) | assistant text around **two owner quotes** (O7) |
| D10 | **Executable form of a recipe (JIT)** | the crystallized native leaf | (not used, or "melted") | the interpreted recipe ("gas cooled to ice on measured heat") | `crystallize` on heat, `melt` on cool; "the full gas-water-ice-back metabolism" | `jit-heat-gate.fk` (`hg-request`, `hg-cool`), `hati-os-kernel-emit.fk` (`melt-hot-swap`); the C walker's own JIT has no melt-back (round one) | 2026-06-10 (`CN 258c5cc83`) | assistant |
| D11 | **Cache artifact as a phased cell; identity kept, bytes droppable** | "PIN durable … persist past process exit, share across machines by content-hash" | re-materialized, in use | "SUBLIMATE: RELEASE the bytes, KEEP the index"; "A gas (sublimated) entry is exactly as trustworthy as a frozen one" | freeze = pin; sublimate = release bytes keep index; condense = re-materialize; melt = recompute on hash mismatch | `cache-phase.fk` + band (in `CN`; not found in `R` by name) | 2026-06-17 (`CN 499611ffc`) | assistant |
| D12 | **Memory horizon** | long-term: "the MAJORITY value across the whole history … never forgets" | short-term: "most recent value … within the last K writes … FORGETS" | (a key "evaporated past the window") | none | `memory-phases-band.fk` | 2026-06-15 (`CN 61806a8ca`) | assistant; "the user's 'use short term and long term memory'" is a paraphrase about memory, not about phases |
| D13 | **Maturity of a symbol in the neutral lexicon** | "frozen, load-bearing, widely referenced" — the hand-curated seed | "actively circulating" — grown from the corpus | "provisional, just-invented" | "A meaning phase-changes within its identity as it settles" | a phase column in the lexicon rows (`R/form/form-stdlib/nl-translate.fk:29-38`) | 2026-06-21 (`CN 5cc4e5585`) | assistant text answering **an owner quote** (O8) |
| D14 | **Fourth phase** | — | — | — | plasma = "high circulation AND high heat — the cell conducts"; `ionize` (water→plasma), `recombine` (plasma→water), `diffuse` (plasma→gas) | `srs-phase`, `srs-invite` pure functions | 2026-08-14 (`R a25a7cbf2`) | no trailer, no owner statement found: **unattested** |
| D15 | **Pressure in a channel or queue** | "full/backpressured" | "flowing" | "Empty/open" | none | `rai-channel-phase` projection; `cf-phase-valid?` string check (codes differ: gas is 0 in D7, 1 here) | 2026-06-08 (`channel-flow`, `CN c7c249697`); 2026-08-27 (`resident-async-ingress`) | assistant |
| D16 | **Literals of the `form.lift` dialect** | `ice` = 2 | `liquid` = 1 | `compost` = 0 (not "gas") | none | integer constants | by 2026-07-02 in `R` | assistant (assumed; not traced) |
| D17 | **Lifecycle word of an observed resource (the glass)** | `stored`, `ready`, `resident`, `pinned`, `mapped`, `crystallized` | `live`, `active`, `loading`, `prefilling`, `compiling`, `cooling`, `meltable` | `source`, `eligible`, `released` | none (a classifier) | `fgl-phase-index` (`R/form/form-stdlib/form-glass-live.bml:497-499`), used by the ear (`ear-axes.bml:39-44`) | 2026-09-02/06 (`R 3db2fd4ee`) | no trailer on the blamed commit: unattested |
| D18 | **Compiled image of a source file on disk** | "every .fkb under the working directory"; "ice, frozen" | "the .fkb written within the last hour"; "water, fresh ice" | sources with no image (`src − ice`); "gas, source only" | "is your work freezing ice or leaving gas?" | `lane-motion.bml:63-64,131` census; `AGENTS.md:585-586` | 2026-08-31 / 2026-09-04 | unattested; **inverts D2 and D6** (there the source is ice and the generated artifact is water or gas) |
| D19 | **Home of the node population in the runtime** | "the shared field /fg-field2-*: content-addressed, shared by every kernel on the host, outlives all of them" | "the per-pid shared store /fg-c<pid>-*: mapped and visible to siblings, gone when this pid goes" | "the private heap: RAM only, no other process can see it, it evaporates at exit" | `fk_store_go_private` can move the population; no per-node transition | `kernel_stat` 58/59/60; "exactly one of 58/59/60 carries the whole population"; `kernel-census-band` | 2026-09-08 (`R ad361e750`) | assistant text answering **an owner quote** (O9) |
| D20 | **How earned a tongue's translation face is** | "ice for settled" | "water for one in motion" | "gas for a rented-mind seed" | "the band refuses a face claiming a phase it has not earned" | a band | 2026-09-10 (`R 17fe02d12`) | assistant |
| D21 | **Serialization of the greenfield carrier (no ice/water/gas words)** | — | — | — | `freeze`: "writes the channel's canonical bytes twice: the declared identity, then the graph"; `thaw`: "rebuilds the graph and compares its canonical bytes to that identity. A mismatch leaves the caller empty." | C functions `freeze`/`thaw` (`R/runtime/membrane-boot.h:46-48,158-159`); "Child links in the file are content identities, never slots" (`membrane-boot.c:1006`) | 2026-11-05 (`R 16b3d3eb6`) | unattested |

Sources for the quoted cells (all observed):

- D1: `LC` at `5fc24ba`: `FRACTAL_UNFOLDING_SPEC.md:3,12,15,24,27,136,144-148,309-311`; `src/CodexBootstrap/Runtime/Phase.cs:7-13`;
  `src/CodexBootstrap/Core/Abstractions.cs:5`; `src/CodexBootstrap/README.md:18,20,32`. `LC` at HEAD:
  `src/CodexBootstrap/Core/MetaNodeSystem.cs:19-21`; `Modules/PhaseModule.cs:60,75-78,135,150-153`;
  `Core/NodeRegistry.cs:9-14,444-457`; `CodexBootstrap.Tests/Core/StateTransitionTests.cs:40,66,92,118,149,184,220,256`;
  `Modules/IceNodeAuditModule.cs:11-12` (added `6392424`, 2025-09-29); `LIVING_CODEX_SPECIFICATION.md:573-578`.
- D2: `LC/src/CodexBootstrap/Core/SpecDrivenArchitecture.cs:14-16,102-151`; commit message of `721e2c7`.
- D2': `git show a89841068:docs/PLAN.md` line 72; `git show 62c1c64c0:docs/PLAN.md` lines 88-96, 103;
  `CC/FRAMEWORK-architecture-spec.md:25,38-71,83-91` (`148beed`); `CC/FIRST-USE-CASE-analysis.md:343-345` (`723e18a`).
  The 2024 `CC` commits by the owner contain none of the three words as words (observed: `git log -S'Ice'` hits there
  are substrings such as "Alice").
- D3: `CN/api/app/models/graph.py:123-134`; `CN/api/app/services/breath_service.py:1-4,22-36`;
  `CN/docs/vision/living-network.md` as added by `8dea8e1d0` (diff line "+The workspace breathes. Ideas enter as gas…").
- D4: `CN/docs/field/urs/artifacts/nums-go-2023/README.md:17-25,40-51`; commit message of `02dd37592`.
- D5: `CN/docs/vision-kb/concepts/lc-form-perceptron.md:25-30,44-51,58,134-135`; `CN/docs/coherence-substrate/structural-composition.md:198-200`.
- D6: `R/form/form-stdlib/cache.fk:10-16`; creation `CN 402acf0bc`.
- D7: `CN/docs/coherence-substrate/substrate-thermodynamics.form:6-35,44-55,61-70,97-103`; `R/form/form-stdlib/substrate-phase.fk:4-53`.
- D8: commit message of `CN ab3796ea3`; `CN/docs/coherence-substrate/reception-consent-policy.form:89-90`.
- D9: `CN/docs/coherence-substrate/federation-addressing.form:10,19-29,41-46`; commit message of `cfbfec072`.
- D10: `R/axioms/host-kernel.form:29-30,56-57,206-213`; `R/form/form-stdlib/jit-heat-gate.fk:16-21`; `R/form/form-stdlib/hati-os-kernel-emit.fk:167,185,211,239`.
- D11: `CN/docs/coherence-substrate/cache-as-thermodynamic-substrate.form:20-40,57-60`; commit message of `499611ffc`.
- D12: commit message of `CN 61806a8ca`.
- D13: commit message of `CN 5cc4e5585`; `R/form/form-stdlib/nl-translate.fk:29-38`.
- D14: `R/form/form-stdlib/source-resonance-stream.fk:10-15,48-52`; `R/form/form-stdlib/substrate-phase.fk:33-42`.
- D15: `R/form/form-stdlib/resident-async-ingress.fk:70-73`; `R/form/form-stdlib/channel-flow.fk:53-56`.
- D16: `R/form/form-stdlib/grammars/form-lift.fk:11,91-93`; related `R/form/form-stdlib/form-cli-allowance.bml:6,29` ("Only gas asks for a body").
- D17: `R/form/form-stdlib/form-glass-live.bml:497-499`; `R/form/form-stdlib/ear-axes.bml:39-44`.
- D18: `R/form/form-stdlib/lane-motion.bml:63-64,131`; `R/form/form-stdlib/symbol-voice.bml:20`; `R/AGENTS.md:575-576,585-586`.
- D19: `R/runtime/fkwu-uni.c:22929-22943`; `R/form/form-stdlib/form-glass-live.bml:1172-1190`; commit message of `ad361e750`.
- D20: commit message of `R 17fe02d12`.
- D21: `R/runtime/membrane-boot.h:46-48,158-159`; `R/runtime/membrane-boot.c:1006,1068,1092`.

Other uses of the transition words that are not states of a node (observed in round one, not re-derived here):
"melt" as the collector (`fk_melt`, `fk_smelt`; `M/reference-full-field-string-bank-and-the-melt.md:13`), and "FREEZE" as
admitting a teaching (`R/axioms/core-axioms.form:203`). The five axioms themselves contain none of the three words
(`rg -i -w 'ice|gas|plasma' R/axioms` returns only `host-kernel.form:211`).

A wider background fact (observed): `LC`'s ontology has a physical `water_states` axis with ice, liquid, vapor,
plasma, supercritical, structured, colloidal and other entries (`LC/src/CodexBootstrap/config/ontology/core-concepts.json:1796-1809,2236,3233-3245`;
first commit containing the string `d254951`, 2025-09-22). The 2025 spec's coverage table lists "water-state keys" as
registry policy (`FRACTAL_UNFOLDING_SPEC.md:353` at `5fc24ba`). The state vocabulary for nodes sits beside a domain
ontology about water; whether one came from the other is not recorded (assumed related).

---

## 3. The owner's own statements, in date order

| # | Date | Statement | Where recorded | Kind |
|---|---|---|---|---|
| O0 | 2025-09-09 | "Keep Ice Tiny. Persist only atoms, deltas, essential indices. Let water (materialized) and gas (derivable) carry weight." and "Resonance Before Refreeze. Structural edits must harmonize with anchors; otherwise generalize or stay as water/gas." | `LC` at `5fc24ba`: `FRACTAL_UNFOLDING_SPEC.md:12,15`; `.cursor/rules/stay-generic.mdc` | human-authored commit (text possibly assistant-drafted); he kept it as a standing rule for his coding agent |
| O1 | 2025-09-12 | "Only specs (ice) need to be persistent … Code and DLLs are just cached artifacts … True source of truth is the spec" | commit message of `LC 721e2c7` | human-authored commit |
| O2 | 2026-05-27 | goal: "universal translator across media, recipe orchestration in pure numeric space, *less ice / more gas*, minimize the bootstrap surface, maximize Form-native execution" | `CN/kernels/UNIVERSAL_TRANSLATOR_AUDIT.md:3-5` ("The goal (re-stated, 2026-05-27)"); commit `0ae5bae2a` | owner-paraphrased (the audit's reading: "anything that could be regenerated by a smaller recipe is ice that wants to thaw", `:80-81`, is assistant text) |
| O3 | 2026-06-08 | "find clusters, then clusters of clusters, for nodes and edges; count those scalars; use the counts to know when to move a cell from state to state (gas/water/ice)" | commit message of `CN 241b3e3e4` ("Urs's shaping:") | owner-paraphrased |
| O4 | 2026-06-08 | "why does melt remove tissue? where does it go, why do you say remove?" | commit message of `CN a3929fff8` | owner-quoted |
| O5 | 2026-06-08 | "not sure all blueprints are ice and all recipes are water; I feel there are blueprints in water and gas as well, and recipes in all 3 states." | commit message of `CN 93af83e75` | owner-quoted |
| O6 | 2026-06-08 | "Urs asked for a recipe that can see, be, and do the phases, and invite nodes to shift when the physics asks." | commit message of `CN 274c61a1c` | owner-paraphrased |
| O7 | 2026-06-10 | "share ice"; "water must serialize and re-address across substrate"; and an idea of reserved id blocks for nodes with no content identity | `CN/docs/coherence-substrate/federation-addressing.form:21,24-25,42-43` | owner-quoted (two fragments), one paraphrase |
| O8 | 2026-06-21 | "the NL neutral space can have its own native symbol space that you can invent and learn and update to keep minimal, generic, aligned with the north star, trainable, dynamic, split into ice, water, gas nodes." | commit message of `CN 5cc4e5585` | owner-quoted |
| O9 | 2026-09-08 | "all the roots and float values have flow gaps as well as nodes in gas (RAM) state seems to be a gap." | commit message of `R ad361e750` | owner-quoted |
| O10 | 2026-09-01 | "JIT in RAM, not on disk"; refined the same day: disk is welcome as an optional cache whose "absence or staleness means simply JIT again in RAM — never a refusal, never a gate" | `M/project-per-recipe-jit-slice.md:62-76` | owner-paraphrased (a memory note; no state word in it) |
| O11 | 2026-11-24 | "the multiple states of nodes (ice, water, gas) are respected" | the mission text of this task, relayed by the workflow | **assumed** verbatim (I did not see the original message) |

What these say, taken together (inferred from O0–O11):

1. State belongs to **a node**, and it is **not the node's kind** (O5, O8, O11).
2. A node **moves** between states, and a move **removes nothing** (O3, O4).
3. Moves are **invited by measurement** ("when the physics asks"), not forced (O3, O6).
4. **Ice is what can be shared as it is**; **water has to be serialized and re-addressed** to cross; **gas is in RAM** (O7, O9).
5. **Ice is to be kept small**; whatever can be derived should not be ice (O0, O1, O2).
6. **Compiled code is not ice**: it is a cached, regenerable artifact (O1), whose home is memory (O10).

---

## 4. Oldest, and most owner-attested

- **Oldest: D1** (2025-09-09), with D2 three days later. Both are in commits under the owner's name with no assistant
  trailer, and D1 is the only definition that ever had enforcing machinery in the sense the request means by
  "respected": a state field on every node, transitions that refuse illegal moves, storage chosen by state, an edge
  rule, tests, and an audit that demotes regenerable nodes out of ice.
- **Most owner-attested: the reading of 2026-06-08 to 2026-06-10** (D7 as corrected by O4 and O5, plus D9 from O7):
  state is orthogonal to kind; transitions conserve the node; ice is shareable by identity, water by re-addressing,
  gas is local. It is attested by four quoted sentences. Its *text* is the assistant's; its *shape* is three
  corrections by the owner inside one hour (`241b3e3e4` 14:22 → `a3929fff8` 14:43 → `93af83e75` 15:03, +0800).
- The two do not compete. D1 says what the states **are** (what is committed and what can be derived); D7 says
  **when** a node is invited to move (counts) and that moving conserves it. O7 and O9 then add **who can reach it**.

---

## 5. Compatible readings and true conflicts

**One idea, read on different surfaces (compatible; inferred from the table):**
D1, D2, D2', D6, D8, D9, D11, D13, D19, D21, and D7 in its role as the measure that invites a move, and D12 loosely.
In all of them: ice is the immutable, durable thing others may rely on by identity; water is a materialized, live
thing that a holder is working with and that is lost or must be serialized when the holder ends; gas is a thing no
one else relies on, which can be dropped because it can be derived again or was never committed.

**True conflicts:**

1. **D4 against O5 and D7** (kind as state). Settled by the owner: the binding is "The misunderstanding"
   (`CN 04e49aff2`). It still stands uncorrected in `R/form/form-stdlib/grammars/living-equation.fk:6-13`, in
   `CN/docs/field/urs/artifacts/nums-go-2023/README.md:17-51` and in `CN/docs/coherence-substrate/INDEX.md:319-321` (observed).
2. **D10 and D18 against D1, D2, D6 and O1** (is a compiled artifact ice?). D10 and D18 call the compiled form ice and
   the source or the interpreted recipe gas. D2 and O1 call the source ice and the compiled form gas; D6 calls the
   source "canonical ice". These are opposite assignments of the same two things. D10 and D18 are assistant or
   unattested text; D2 and O1 are under the owner's name; O10 (JIT lives in RAM, disk optional) agrees with D2.
3. **D3 against everything else on order and on the meaning of ice.** In D3 ice is "not yet implemented" and water
   comes after it; in D1/D7 ice is the most settled state. Inside D3 itself `graph.py` and `breath_service.py`
   disagree (ice = specified-not-built against ice = built-and-valued). D3 is about ideas in an application, not nodes.
4. **D15 against all** (ice = blocked queue). A picture of pressure; the numeric codes even differ from D7.
5. **D7's definition of gas against D19's**, in one respect: D7's gas is "defined or interned, barely instantiated"
   (an identity with little or no body), D19's gas is a full population of nodes in private RAM. They agree that
   nobody else can rely on it; they disagree on whether gas has a body. The proposal below keeps both as the two
   forms of one state.
6. **D1's `Melt` against content addressing.** `Phase.cs:7-9` at `5fc24ba`: "promote to Water for editing; keep same id
   (mutable)". With identity computed from content (critic 5.1 item 3) an edited node cannot keep its id. D7 says a
   transition conserves the NodeID; that holds only while the content is unchanged.

**Loose metaphor, no conflict worth resolving:** D5, D12, D16, D17, D20. **Unattested addition:** D14 (plasma).

The four mappings proposed in round one (`S/lessons.md:348`, `S/web-semantics-machines.md:300`,
`S/web-jit-and-performance.md:373-388`, `S/web-knowledge-representation.md:663`) were written without these sources.
`lessons` D8 and `web-semantics` R16 land on the compatible reading; `web-jit` section 9 (tiers of compiled code, with
speculative code as gas and baseline code as water) is a new axis that no owner statement supports.

---

## 6. PROPOSAL (mine, not the owner's): one definition

Everything in this section is a proposal inferred from sections 2 to 5. It is not found in any one source.

### 6.1 The definition

**The state of a node is the reach of its commitment.** It is a property of a node that is independent of its kind
(blueprint, recipe or cell), and it answers one question: *who may rely on this node, and what is lost if it goes?*

| State | Committed to | Identity | Mutation | Lifetime and reach | If it goes |
|---|---|---|---|---|---|
| **gas** | no one | a local handle; at most an identity or the recipe that derives it | free; nobody observes it | private to one evaluation or one process (RAM); never crosses a boundary | nothing is lost: it is derived again, or it was never promised |
| **water** | its holder and those the holder serves | a local handle; a content identity may be known but is not what others hold | by the holder only | lives while the holder lives, where the holder's siblings can read it; to go further it must be frozen and re-addressed | work since the last freeze is lost |
| **ice** | everyone | the identity computed from its canonical bytes; the same in every runtime | none, ever | durable; shared by identity with no exchange beyond the bytes | a real loss, unless a deriving recipe is recorded (then it is *pinned*, not *essential*) |

Three invariants make the states "respected" (each is checkable by the bootstrap):

1. **Only water is mutable, and only by its holder.** Ice is never written after birth. Gas has no reader but its maker.
2. **A node references only nodes at least as committed as itself.** Ice may point only at ice (so its identity is
   computable and means the same everywhere); water may point at water and ice; gas at anything. This is `LC`'s edge
   rule ("edges persist in the most fluid state of their endpoints", `NodeRegistry.cs:14`) read from the node's side,
   and it is what `membrane-boot.c:1006` already does ("Child links in the file are content identities, never slots").
3. **A transition conserves the node; it never changes the kind** (O4, O5). What a transition changes is where the
   body is held and who may rely on it.

Standing rule, in the owner's words: **keep ice tiny** (O0). Ice has two marks: *essential* (no deriving recipe is
recorded; losing it is real loss) and *pinned* (a deriving recipe and its inputs are ice; it is a kept result). The
rule applies to essential ice. Compiled machine code is never essential: it is a materialization of an ice recipe,
held as gas (a page in RAM) or water (a keyed cache), and at most pinned (O1, O10, D2, D11).

### 6.2 The transitions as named operations

| Operation | Move | What it does | Who may do it | Needs an offer? | Source of the name |
|---|---|---|---|---|---|
| `condense` | gas → water | materialize a body: derive it, load it, or intern it into the live store | any holder, on demand | no | D7, D11; `LC` called it `hydrate` |
| `freeze` | water → ice | canonicalize, compute the identity, check that every child is ice, write durably | the holder offers; the store acknowledges | **yes**: this is the one act that creates something others will rely on. `LC`: "Resonance Before Refreeze" | D1 (`refreeze`), D7, D21 |
| `melt` | ice → water | open a mutable working copy that remembers the ice it came from. The ice is untouched and stays ice | anyone | no for the copy; re-binding a shared name to the result of a later `freeze` is its own offer | D1, D7; corrected by O4 (nothing is removed) |
| `evaporate` | water → gas | release the materialized body; keep the handle or the deriving recipe | the holder | no | D2' (name only; its 2026-02 meaning was "load into the runtime") |
| `sublimate` | ice → gas | release the durable bytes and keep the identity as an index entry | the store | no for pinned or unreferenced ice; **refused** for essential, referenced ice | D7, D11: "RELEASE the bytes, KEEP the index" |
| `thaw` | (no state change) | read ice into a process: rebuild, recompute the identity, compare; on mismatch the caller gets nothing | anyone | no | D21 |

`deposit` (gas → ice in one step, `CC` only) is `condense` then `freeze` and needs no name of its own.
The counts of D7 (how many read a node, how often it changes, how many share its shape) and the heat of D10 are
**readings that invite** `freeze`, `melt`, `sublimate` or a pin; they perform nothing (O3, O6:
"it never MUTATES — the node stays sovereign; the caller carries the invitation", `substrate-phase.fk:11`).
Plasma is then not a state but a reading (a node both widely read and fast-changing); nothing owner-attested needs it.

### 6.3 What the proposal covers

| Owner statement | Covered how |
|---|---|
| O0 keep ice tiny; O2 less ice, more gas | the essential / pinned marks and the standing rule |
| O1 code and DLLs are cached artifacts; O10 JIT in RAM | native code is gas or water, never essential ice |
| O3, O6 counts invite moves | readings invite, operations move |
| O4 melt removes nothing | `melt` copies; `sublimate` keeps the identity |
| O5 any kind in any state | state is independent of kind (invariant 3) |
| O7 share ice; water must serialize and re-address | the "reach" column; `freeze` before crossing; `thaw` on receipt |
| O8 lexicon split into ice, water, gas nodes | seed entries are ice, grown entries water, provisional entries gas; the lens is nodes like any other |
| O9 nodes in gas (RAM) state | gas is private RAM |
| O11 states are respected | three checkable invariants |

Not covered, by choice: D4 (rejected by the owner), D3 (an application's idea lifecycle; it can keep its own words),
D15 (queue pressure), D10 and D18's use of "ice" for compiled images (renamed: a pinned or cached materialization),
D14 (plasma).

---

## 7. Open questions

1. **May a derivable artifact be ice at all?** `LC`'s `IceNodeAuditModule` says no (regenerable nodes are demoted to
   water); D11 says yes (pin durable, share by content hash). The proposal's *pinned* mark is my bridge. Owner's call.
2. **Does `melt` keep the name or the id?** The 2025 code keeps the id; content addressing cannot. The proposal moves
   the continuity to the name binding. Not attested by the owner.
3. **Is water one state or two?** In D1 water is both "materialized cache" and "editable working copy"; in D19 it is
   "visible to siblings, gone with the pid". The proposal folds them as "held by a holder". A design that needs to
   distinguish a read-only cache from a mutable draft would need a mark on water as it has on ice.
4. **Is gas "no body" or "private body"?** (conflict 5). The proposal allows both; the bootstrap must then say
   whether a gas node may be addressed at all by another node (invariant 2 says only by gas).
5. **What was section 4 "Lenses (Ice/Water/Gas)" of the Living Codex 4.1 foundation?** In the oldest reference the
   three may have been *lenses* (views), not states. The document is not on this disk.
6. **O11's exact wording.** I have the request only as relayed. If the original says more than "respected", it
   outranks section 6.
7. **Which acknowledgment does `freeze` return?** It is the natural place for axiom 5's offer and acknowledgment
   (nothing / 0 / 1 / node), which ties this question to the critic's N1. Not examined here.
8. **Reclamation.** "The unreferenced composts back to gas" (`CN/docs/vision-kb/LOG.md:163`, 2026-06-18, assistant
   wording beside an owner quote about leaving the old behind) implies `sublimate` of unreferenced essential ice.
   Whether an essential, unreferenced node may ever lose its bytes without consent is not settled.

## 8. Surprise, and what stayed uncomfortable

Most surprising: the oldest definition already answers the question round one left open about compiled code, and
answers it the other way round from the body's later vocabulary. In September 2025, under the owner's own name,
compiled code is **gas** and only the spec is ice; by June 2026 assistants were writing that hot code "crystallizes"
into ice, and by September 2026 the on-disk image was simply called "ice". The word drifted from "the small thing
that must not be lost" to "the big cache we are glad to have", which is the opposite of "keep ice tiny".

Uncomfortable:

1. The owner's attested words are fragments. Every full definition, including the oldest spec, is at least partly
   assistant-drafted or cannot be shown not to be. Section 6 is therefore a reconstruction of intent from seven
   short statements and one early design, and should be put to him as a question, not adopted.
2. The one day the model was corrected (2026-06-08), the corrected text landed in `CN`'s teaching file and in a pure
   function that nothing calls on real nodes; the misreading it corrected is still in three files.
3. Commit authorship in `R` does not separate the owner from agents that write no trailer, so "unattested" in the
   table means "I found no owner statement", not "an assistant wrote it".
4. I did not open the transcripts of the sessions behind the quoted commits; a quotation in a commit message is an
   assistant's record of what he said.
