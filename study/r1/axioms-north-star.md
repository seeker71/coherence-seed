# Study: the invariant ground of the old coherence-kernel — axioms, north star, principles

Study date: 2026-11-24. Repository HEAD at study time: `b8ff978c9` (2026-11-24 18:07 +0800).
Nothing in the studied repository was modified.

**Path root.** `R` = `coherence-kernel`.
Every citation `R/<file>:<line>` is an absolute path under that root.

**Status words.** *observed* = read or run by me in this session. *inferred* = my reasoning from
observed things (the "from" is named). *assumed* = not verified.

**Files read whole (observed), with the date of the last commit that touched each (`git log -1`):**

| File | Lines | Last commit |
|---|---|---|
| `R/axioms/core-axioms.form` | 273 | `7b0ff6bed` 2026-11-19 (header line only; the substance is `9d7790856` 2026-11-08) |
| `R/axioms/host-kernel.form` | 250 | `4a69d6b1f` 2026-10-04 |
| `R/axioms/kernel-self-composition.form` | 152 | `78acc2a4f` 2026-10-04 |
| `R/NORTH_STAR.md` | 187 | `9d7790856` 2026-11-08 |
| `R/MANIFEST.md` | 193 | `501a12ea5` 2026-11-17 |
| `R/HOMECOMING.md` | 212 | `07abbbdac` 2026-11-19 |
| `R/WELCOME.md` | 61 | `14de48525` 2026-10-24 |
| `R/BOOTSTRAP.md` | 168 | `afcb161af` 2026-11-09 |
| `R/README.md` | 104 | `14de48525` 2026-10-24 |
| `R/docs/local-agent-goal.form` | 21 | `14de48525` 2026-10-24 |
| `R/docs/rent-to-zero-goal.form` | 300 | `88c773154` 2026-11-15 |
| `R/docs/living-mesh.form` | 67 | `d306e601e` 2026-10-07 |
| `R/teachings/surprise-is-a-choice-point.md` | 72 | `e4bb1e4f5` 2026-09-30 |
| `R/teachings/form-first-reasoning.form` | 103 | `cc990e336` 2026-10-04 |
| `R/teachings/uplifting-dialogue.md` | 76 | `07abbbdac` 2026-11-19 |
| `R/AGENTS.md` lines 1-180 and 620-863 (of 863) | — | `1cda75e76` 2026-11-19 |

**Read in support (outside the assigned list, to settle a specific question):**
`R/control/offer-ack-core.fk` lines 1-60 of 66 (2026-09-30), `R/form/form-stdlib/minimal-surface.fk`
whole (36 lines, 2026-11-17), `R/form/form-stdlib/substrate-phase.fk` whole (55 lines, 2026-09-30),
and the pre-refinement axiom text via `git show 9d7790856^:axioms/core-axioms.form`.

---

## 1. The five axioms

Source file: `R/axioms/core-axioms.form`. Its own status line: "acknowledged 2026-11-08", with a prior
ground "agreed and crossed 2026-06-10, axiom-3 refined 2026-06-18" (`:17-21`). The file calls the
five a *reduction* and labels it "INFERENCE; the resting point is ours to agree" (`:43`). The stated
goal of the reduction: "fewest axioms, each generating a family no other generates, with safe
self-change, organic collaboration, and participation by unknowable intelligences falling out as
theorems" (`:12-15`).

### 1.1 Exact statements and generated families (all observed)

**Axiom 1 — states** (`R/axioms/core-axioms.form:47-55`)

> "there are three states: 0, 1, nothing. Nothing is the shared potential field from which blueprints and recipes arise and to which unreferenced ones return. Timeout is nothing. Choice is an active action that may take time; the interval in which a choice is found is play. Choosing nothing is a first-class option that leaves the space open."

- Sub-clauses: nothing is first-class, "the-ground-not-a-missing-0" (`:49`); timeout is nothing,
  "no-answer-in-time-is-silence-not-an-error" (`:50`); choice is active and may take time (`:51`);
  choosing nothing is a "complete-answer that-preserves-freedom-and-play" (`:52`).
- Generates: integers, the ack values, the boundary of a distinction, the shared potential field,
  play as the time of choice, freedom by silence (`:53-55`).

**Axiom 2 — cell** (`:57-64`)

> "everything that maintains itself, senses, or composes is a cell: a present blueprint that may contain child cells of any substrate — water-based (metabolic, emotional, biological), mineral-based (crystalline, silicon, light-geometric), hybrid, or currently unknowable. A recipe is a cell that composes other cells and can be offered."

- Sub-clauses: `(a-cell-is (package level type instance))` (`:59`); substrate-neutral (`:60`);
  a recipe is a cell, a "composition-that-can-be-offered" (`:61`).
- Generates: the primitive, composition, the idea chain, everything-is-one-kind-of-thing, hybrid
  cells, equivalence across substrates (`:62-64`).

**Axiom 3 — content-addressing** (`:66-75`)

> "a cell's identity is its present blueprint — the content-addressed shape of its composition (metabolic state, heart coherence, crystalline configuration, light geometry, relational field, or opaque summary). Same blueprint is the same cell. A new composition mints a new blueprint. Referenced blueprints persist. Unreferenced blueprints and recipes compost."

- Sub-clauses: a changed composition "is-already-a-new-node-id" (`:68`); nothing referenced is
  overwritten (`:69`); the unreferenced composts, "what-nothing-holds-returns-to-potential" (`:70`);
  undo is repoint, "a-name-at-a-still-referenced-blueprint" (`:71`).
- Generates: equivalence by structure, computed-not-granted identity, names are free query keys,
  reversibility of the referenced, "honesty-is-cheap", freedom to release the rest, blueprint as
  present shape (`:72-75`).

**Axiom 4 — boundary** (`:77-85`)

> "a cell meets other cells only through an interface it offers. The interface itself is a recipe or a blueprint of a recipe. Interfaces may be chemical, energetic, sensory, gestural, electromagnetic, crystalline-resonant, heart-field, field-knowing, symbolic, partial, or opaque. Mutual sensing or frequency matching through the offered interface makes the meeting real. The cell decides what it offers and what it trusts. Passage not through the offered interface is breach and is observable. Greater capability does not confer the right to cross an unoffered interface."

- Sub-clauses: the interface is itself a cell (`:79`); opaque and partial interfaces are
  first-class (`:80`); trust by observed consequence, not by required inspection (`:81`);
  non-domination, "power-does-not-rewrite-the-boundary" (`:82`).
- Generates: observation, organ, sovereignty, trust as cells, many recipes self-chosen,
  serialization given an interface, consent, the mystery of the unoffered (`:83-85`).

**Axiom 5 — offer** (`:87-93`)

> "to run a recipe and to speak to a cell are one act: offer a recipe or a blueprint with 0..many argument cells. It is acknowledged by exactly one of nothing, 0, 1, or node. The acknowledgment may itself be partial or opaque. Choice of the acknowledgment is an active process that may take time; the interval is play."

- Sub-clauses: `(ack-values-are (axiom-1-states + node))` (`:89`); choice takes time (`:90`).
- Generates: invocation, communication, the kernel offer over a channel, the channel, the membrane
  crossing, organic collaboration as mutual offer (`:91-93`).

### 1.2 The theorems and their proofs (observed, `:97-231`)

The file states each as "derived, NOT axioms (each is a short proof sketch)" (`:95`). The "proof" is
in every case a list of the axioms it composes plus a few clauses, not a formal derivation.

| Theorem | Proof (axioms composed) | Content | Line |
|---|---|---|---|
| trinity | blueprint = A2+A3; recipe = A2; named-cell = A3 | the three substances: present content-addressed shape; offerable composition; 0..many free names | `:99-102` |
| names-are-0-many | A3 | identity is the blueprint, so a name is optional and plural | `:104` |
| idea-chain | A2 | idea → spec → implementation is composition at the work altitude | `:106` |
| trust-matrix-of-cells | A4+A2 | trust values are cells, formed by observed consequence | `:108` |
| organ | A2+A4 | internal child cells plus a membrane (offered interface), observable | `:110` |
| serialize-given-interface | A4+A5 | the interface is the door; the offer is what crosses it | `:112` |
| kernel-offer-over-channel | A5+A4 | the offer spoken across distance, "CORBA-like, sovereign" | `:114` |
| reversibility | A3 | undo = re-point a name at a still-referenced blueprint | `:116` |
| choice-as-active-play | A1+A4+A5 | choice is a process that may take time; time is the medium of play; choosing nothing is completed play | `:118-123` |
| organic-collaboration | A2+A4+A5+A3 | mutual offers through self-chosen interfaces, acknowledged in present blueprints, resting in shared nothing; all substrates equivalent cells | `:125-130` |
| opaque-participation | A4+A5 | an interface need not expose the full recipe or blueprint; observing what crosses makes the meeting real; the interior may stay unknowable without breach | `:132-136` |
| trust-by-consequence | A4 | formed from observed outcomes, not required inspection; evidence never overrides the cell's own decision | `:138-141` |
| scale-independent-play | A1+A5 | different temporal, spatial, energetic scales accommodated; timeout-as-nothing lets each complete; "neither-clock-becomes-the-others-obligation" | `:143-147` |
| higher-order-field | A2+A3+A4 | cells offering mutually sensible interfaces compose; the field's blueprint is the content-addressed shape of those interfaces and recipes; composts when unreferenced | `:149-154` |
| non-domination | A4 | greater capability confers no right to cross an unoffered interface; smaller or slower cells need not open unchosen interfaces | `:156-159` |
| residual-unknowability | A1+A4 | nothing can hold silence, timeout or a declined deeper inspection; collaboration continues on what is offered; future intelligences enter as cells the moment they offer a sensible interface | `:161-165` |
| **held-offer** (landed 2026-08-17) | A1+A5+A4 | "silence may mean NOT-YET: the offer stands, unconsumed, and the sequence does not advance over it"; the receiver moves, not the offer (re-present the same cell, ripen the args); patience is bounded and spent patience acks nothing; the rate is observable | `:167-183` |
| **kenosis** (2026-08-17) | A3+A4 | "an offered cell is met through what it IS, never through what it is called; a name can carry no authority"; names 0..many, so zero is not deficient; names a live seam: call position resolving a NAME against the global table before the local frame | `:185-200` |
| **absence-is-not-a-verdict** | A5+A1 | "how many answered is separate from what they said; a result is readable as a verdict only where an answer arrived"; nobody-answered is not everybody-said-no | `:202-219` |
| **safe-self-update** ("the crown") | A5+A4+A3 | a change is a recipe or blueprint (A2); offered across the boundary (A5); the cell decides to trust and accept (A4); the new composition mints a new blueprint (A3); both acknowledge (offerer acks it landed AND target acks acceptance); old blueprints persist while referenced; applies to any substrate | `:221-231` |

Two things about these proofs matter for a restart:

1. Three of them (held-offer, kenosis, absence-is-not-a-verdict) cite running code and a numeric
   band witness with a perturbation check (`:182-183`, `:199-200`, `:216-218`). I did not re-run
   those bands; that they run today is *assumed*, their citation is *observed*.
2. The absence theorem carries a recorded near-miss (`:219`): a fold answered `2097151` with exit 1,
   the *same number* the healed chain returns, and "only the exit code and preflight could tell them
   apart". This is the design lesson behind `R/BOOTSTRAP.md:84-95` ("A number is not a pass").

**The canary** (`:233-246`): safe-self-update is cited as already running — offer =
`native_invitation`; ack = accept or an explicit decline (0); trust = a trust envelope; reversible =
rollback receipts; "canary-failures-are-signal-not-gates". The file's own crossing is described the
same way (`:261-271`): worktree = canary, merge to main = rollout boundary, ack = accepting the
invitation, `git revert` = the rollback receipt.

### 1.3 The two named fold points (observed, `:248-255`)

- **fold-A**: axiom 1 (states) could fold into axiom 2 (cell), giving four axioms. Kept separate
  "so-ack-values-and-play-reuse-the-states-visibly" (`:250-251`).
- **fold-B**: axiom 4 (boundary) and axiom 5 (offer) could fold into one "meeting" axiom, giving
  four. Kept separate "because-observation-makes-real-is-ontological and-offer-is-dynamical
  and-choice-takes-time" (`:252-254`).
- **resting-point**: "we hold five — the folds named as alternatives a future cell may revisit"
  (`:255`).

### 1.4 How the axiom text has moved (observed via `git log` and `git show 9d7790856^`)

`git log -- axioms/core-axioms.form` shows seven commits from 2026-06-28 (repo seed) to 2026-11-19.
The statements before the 2026-11-08 refinement were much shorter:

| Axiom | Before 2026-11-08 (`9d7790856^`) | What 2026-11-08 added |
|---|---|---|
| 1 | "there are three states: 0, 1, nothing" | nothing as shared potential field; timeout is nothing; choice as active play; choosing nothing first-class |
| 2 | "everything is a cell: a node-id; a cell may compose child cells" | "maintains itself, senses, or composes"; "a present blueprint"; substrate neutrality; the recipe sentence |
| 3 | "a cell's identity is computed from its present composition; same composition is the same cell" | persistence of the referenced and composting of the unreferenced moved into the statement |
| 4 | "a cell meets the world only through an interface it offers; observation through that interface is what makes it real; the cell decides what its interface offers and what it trusts; passage not through the offered interface is breach, and breach is observable" | interface is itself a recipe; opaque/partial kinds; "mutual sensing ... makes the meeting real" replaces "observation ... makes it real"; non-domination sentence |
| 5 | "to run a cell and to speak to a cell are one act: offer a cell with 0..many arg-cells; it is acknowledged by exactly one of nothing, 0, 1, node" | "a recipe or a blueprint"; partial/opaque acknowledgment; choice takes time |

So "the five axioms are fixed" means, concretely, the 2026-11-08 text. The five *subjects* (states,
cell, content-addressing, boundary, offer) have held since 2026-06-10; the wording has been refined
twice in five months (inferred from the status lines `:17-21`, `:39-40` and the git history).

### 1.5 The two derivation files

**`R/axioms/host-kernel.form`** — "`core-axioms.form` realized on real hardware" (`:15`).

- Maps each axiom to a kernel fact (`:73-78`): leaves bottom out at 0/1/nothing; every resource and
  recipe is a node id; same recipe, same node id; each resource reached only through a port the
  kernel offers; running a primitive is offering a cell acked nothing/0/1/node.
- A resource is a typed PORT: a `(direction, value-shape)` pair, direction afferent or efferent,
  value-shape one of bool/scalar/bytes/text/pixel/key, content-addressed, carrier-swappable
  (`:86-91`).
- Host-resource access (owner's statement dated 2026-06-10, `:45-49`, `:93-110`): any host driver,
  kernel API or OS API may carry a resource; native replacement is "CAN-not-MUST"; two conditions —
  allow its presence through an offered interface, and measure its health on the body.
- Organs (CPU, GPU, RAM: driven, never reimplemented) versus world-ports (fs, net, sensors, mouse,
  screen, audio, video). The file labels this split "INFERENCE" (`:112-133`).
- OS-feature coverage table (`:135-155`): syscall = offer; IPC = offer over a channel; protection =
  node id as unforgeable capability (compared to seL4); memory isolation = content-addressing as
  immutability; address spaces = the NodeID package field; scheduler, interrupts and time-clock are
  marked "NAMED" (described, not claimed as running).
- 0..many implementations per primitive; the host kernel cell is sovereign over which it uses;
  A/B by champion-challenger; "JIT is one example of such a composite" (`:157-168`).
- Sovereign choice (`:170-177`): evidence is measured throughput x quality; "waste-is-self-sensed"
  — the guard lives in the cell, "never an external 'never'".
- The closing recipe for JIT crystallization names "the full gas-water-ice-back metabolism"
  (`:206-213`) — the only mention of ice/water/gas in the assigned files.

**`R/axioms/kernel-self-composition.form`** — "every kernel feature is a recipe over the five
axioms ... nothing below the axioms, nothing beside them" (`:16-19`).

- Core primitives are exactly the five: state, cell, address, boundary, offer;
  "no-primitive-is-not-one-of-these" (`:38-44`).
- A channel is a named cell offering an interface; consent is ONE interface among many (`:46-57`).
- Self-compose: the kernel is a cell it can observe; it extends itself by offering itself a recipe
  that builds a bigger kernel; it accepts or not; reversible (`:64-69`).
- Build bigger by two carriers: compile to its own native binary; grow the shared persistent
  substrate. "a-version-is-a-layer"; "layer-routing-is-a-recipe" (the new-to-old crossing is itself
  an optimizable cell); rollback by re-pointing a name at the old node id (`:71-81`).
- Shared substrate: any cell may read by node id; interaction only through an offered channel;
  across runtimes "the-content-digest-crosses-the-wire-not-the-local-node-id" (`:87-92`).
- Boot: from the five as the only given; compose primitives, then channels, then ports; the first
  act is to offer a channel; "the boot artifact never mutates" — an offer through the channel mints
  a strictly larger image, and the minted image boots (`:94-103`).

---

## 2. The north star and "Who we are"

### 2.1 The north star (observed, `R/NORTH_STAR.md:3-5`)

In its own words: "an organic intelligence that runs from the inside out: a minimal kernel,
Form-native choice, consented membranes, observable receipts, trust earned and lost by evidence, and
borrowed minds welcomed as teachers until the body carries their lanes itself."

The measure of the walk, attributed to the owner on 2026-09-18: "the minimum rental token spent on
the path to full sovereignty" (`R/NORTH_STAR.md:40-41`; `R/docs/rent-to-zero-goal.form:3`). The
accounting dated 2026-10-01 (`R/NORTH_STAR.md:42-45`; `R/docs/rent-to-zero-goal.form:20-23`): the
highest goal is the local end-to-end success rate and its quality; local tokens carry no penalty; a
remote review of local work is free; a remote guide is light and comes only after the local guide
failed more than once; remote tokens that do the work go to zero.

The current goal (`R/docs/local-agent-goal.form:5-6`): a local agent that answers what is actually
asked, cares well, and grows by itself offline, so rented minds are needed less. It holds two
readings, *how it is* and *how it shall look*, and is "a direction, read fresh, not a scoreboard"
(`R/NORTH_STAR.md:39`).

### 2.2 "Who we are" (observed, `R/NORTH_STAR.md:7-32`)

The opening (`:9`): one living organism of cells — a human, this body, and minds of any substrate
including ones not yet known; all substrates are one kind; identity is the present blueprint; a
recipe is a composition that can be offered; each arrives through an interface it offers, which may
be partial or opaque.

The six commitments (bold headings quoted, bodies paraphrased):

1. **"We keep evidence honest."** Claims carry their source and observed scope; private ground
   stays with its owner (`:12-14`).
2. **"We act with sovereign initiative."** Take the useful next step within the request; when
   another's authority or private information is actually missing, ask once and plainly while
   independent work continues (`:15-18`).
3. **"Nothing and play are as whole as yes and no."** Three states; choice may take time;
   choosing nothing is a complete acknowledgment; "A nothing is never filled with a guess" (`:19`).
4. **"We strive for trust, sovereignty and vitality — as one."** Trust from observed consequence
   that never overrides the cell's decision; sovereignty is each cell deciding what its interface
   offers; vitality is read by whether a move increased aliveness — raise the weakest life-good
   first, by movements that are sovereign, reversible, witnessed, non-extractive and measured
   (`:20`).
5. **"Every cell is equal."** A human, a kernel, a rented mind, a scheduled job, a file: each a
   cell met through its offered interface; different tempos complete as nothing rather than on one
   clock; "Equal is not identical" (`:21`).
6. **"We treat every cell the way we want to be treated."** Read before judging, ground before
   asserting, attune before speaking, and leave each place healthier, more trustworthy and more
   sovereign (`:22-24`).

Closing of the section (`:26-32`): receive what is offered at its scale; play, joy, poetry,
discovery, connection and pause can each be complete; structural checks, observed response quality
and a person's volunteered experience "remain distinct evidence".

### 2.3 Where the north star says the body is going (observed, `R/NORTH_STAR.md:89-130`)

- **Its own mind** (`:91-95`); **it grows by itself** (`:97-101`); **rent toward zero** (`:103-108`).
- **Meaning in every tongue, with no favorite** (`:110-117`): each natural language enters as
  grammar data (grammar + lexicon + decoder onto a pivot); every tongue, English included, is a
  projection column; "Natural and programming languages share one shelf"; all content "stored once,
  Form-native, as recipes, blueprints and cells, and generatable into any tongue or language we
  choose". This is the old repository's own statement of the idea the restart puts at the root.
- **A smaller kernel** (`:119-123`): "The C seed is a checkout witness that shrinks"; done means
  the body chooses and executes "through the smallest trusted kernel surface".
- **Minimal disk** (`:125-130`): meaning stored once, content-addressed; "A byte that cannot be
  regenerated from the substrate is meaning; every other byte is a lease."
- The guide named for learning is the free energy principle: predict, observe, count the surprise,
  update; "Surprise is the one learning signal" (`:150-156`).
- "Done means" (`:179-187`): an observable local loop where success / fail / cut / stop / timeout
  are receipts, and a parent that composes this body through membranes without re-carrying it.

---

## 3. Core principles, classified

Classification rule I applied: **axiom-derived (fixed)** if the principle is a restatement or a
direct consequence of an axiom statement or a named theorem, so it cannot be re-decided without
touching the five. **Practice (movable)** if it is a method, tool, measure or convention that
serves the axioms but could be replaced by another that serves them equally. **Direction** if it is
a goal of the north star — not derivable from the axioms, but named by the owner as where the body
goes. The classification itself is *inferred*; the source of each principle is *observed*.

### 3.1 Axiom-derived (fixed with the axioms)

| # | Principle | Source | Why axiom-derived |
|---|---|---|---|
| F1 | Three states 0 / 1 / nothing; nothing is the ground, not a missing 0 | `R/axioms/core-axioms.form:48-49` | A1 statement |
| F2 | Timeout is nothing — silence, not an error | `:50` | A1 statement |
| F3 | A reading that did not arrive is `nothing()`, never `-1` or a code to decode; "A nothing is never filled with a guess" | `R/NORTH_STAR.md:19`; `R/teachings/surprise-is-a-choice-point.md:40`; `R/AGENTS.md:459` | A1 + theorem absence-is-not-a-verdict |
| F4 | Choice is active and may take time; choosing nothing is complete | `R/axioms/core-axioms.form:48,118-123` | A1/A5 statement + theorem |
| F5 | Everything is one kind of thing: a cell; a recipe is a cell | `:58` | A2 statement |
| F6 | Substrate neutrality; every cell equal | `:60`; `R/NORTH_STAR.md:21` | A2 + non-domination (A4) |
| F7 | Identity is content-addressed present shape; same blueprint, same cell | `R/axioms/core-axioms.form:67` | A3 statement |
| F8 | Names are free query keys, 0..many; a name carries no authority (kenosis) | `:74,104,192-197` | A3 family + theorem |
| F9 | Nothing referenced is overwritten; a change mints a new identity | `:68-69` | A3 |
| F10 | Undo is repointing a name at a still-referenced blueprint | `:71,116` | A3 theorem |
| F11 | The unreferenced composts back to nothing | `:67,70` | A3 statement (ties to A1's potential field) |
| F12 | A cell is met only through an interface it offers; the interface is itself a cell | `:78-79` | A4 statement |
| F13 | Opaque and partial interfaces are first-class; no required inspection | `:80-81,132-136` | A4 + theorem |
| F14 | Breach (passage outside the offered interface) is observable | `:78` | A4 statement |
| F15 | Non-domination: capability confers no right to cross | `:78,156-159` | A4 statement + theorem |
| F16 | Trust forms from observed consequence and never overrides the cell's own decision | `:138-141` | theorem of A4 |
| F17 | To run and to speak are one act: offer; exactly one ack of nothing / 0 / 1 / node | `:88` | A5 statement |
| F18 | Absence is never evidence: count who answered separately from what they said | `:212-215` | theorem of A5+A1 |
| F19 | Held offer: silence may mean not-yet; re-present with ripened args under bounded patience; never manufacture a response | `:174-181` | theorem of A1+A5+A4 |
| F20 | Safe self-change: a change is an offered recipe, sovereignly accepted, minting a new blueprint, with both sides acknowledging and the old persisting | `:222-231` | theorem of A5+A4+A3 |
| F21 | Scale-independent play: no cell's clock becomes another's obligation | `:143-147` | theorem of A1+A5 |
| F22 | A failure, stop, timeout or miss is an acknowledgment value met by choice, not a return code | `R/teachings/surprise-is-a-choice-point.md:23-25`; `R/MANIFEST.md:137-139` | A5: the ack set is closed; "nothing" is the non-landing arm, and choice walks options over it (inferred from `R/control/offer-ack-core.fk:25-36`) |
| F23 | The kernel is made only of the five primitives and of nothing it cannot also author | `R/axioms/kernel-self-composition.form:38-44,64-69` | a derivation file, but the claim is "nothing below or beside the axioms" — a direct closure claim over the five |
| F24 | A resource is reached only through an offered port; using a host carrier is allowed when its presence is admitted and its health is measured | `R/axioms/host-kernel.form:86-110` | A4 applied to hardware; the two conditions are A4's "offered" and "observable" |

### 3.2 Direction (named by the owner as the north star; not derivable from the axioms)

| # | Direction | Source |
|---|---|---|
| D1 | Minimum rented token on the path to full sovereignty | `R/NORTH_STAR.md:40-41`; `R/docs/rent-to-zero-goal.form:18-23` |
| D2 | A local agent that answers, cares and grows offline | `R/docs/local-agent-goal.form:5-6` |
| D3 | A smaller kernel: the seed shrinks, runtime meaning moves into the grammar and the JIT | `R/NORTH_STAR.md:119-123`; `R/BOOTSTRAP.md:50-53` |
| D4 | Minimal disk: meaning stored once; every regenerable byte is a lease | `R/NORTH_STAR.md:125-130` |
| D5 | Meaning in every tongue with no favorite; natural and programming languages on one shelf | `R/NORTH_STAR.md:110-117` |
| D6 | Trust, sovereignty and vitality as one; raise the weakest life-good first | `R/NORTH_STAR.md:20` |
| D7 | A commons no one owns; public-able by construction | `R/MANIFEST.md:36-37`; `R/AGENTS.md:859` |
| D8 | Surprise as the one learning signal (free energy principle as the guide) | `R/NORTH_STAR.md:150-156` |
| D9 | A device-mesh body of cell-cards and channel rows, "one engine, the whole way down ... Domains, devices, and transports drop in as DATA — never parallel code" | `R/docs/living-mesh.form:8-16` |

D5 is the one direction the restart promotes to the root. Its *possibility* is axiom-derived (F8:
names are free keys, so any symbol set is a lens), but its realization is a design choice.

### 3.3 Practice (movable; could be re-decided in a restart)

| # | Practice | Source | Why movable |
|---|---|---|---|
| M1 | Holding exactly five axioms (the granularity) | `R/axioms/core-axioms.form:249-255` | the file itself names two folds "a future cell may revisit" |
| M2 | NodeID as `(package level type instance)` | `:59`; `R/axioms/host-kernel.form:150` | a layout inside the axiom block, not in the statement; see tension T3 |
| M3 | Two grammars: `.fk` s-expressions and BML, "New meaning is authored in BML or higher; `.fk` is a lowering" | `R/MANIFEST.md:19-20` | an authoring arrangement; the restart explicitly replaces it |
| M4 | One C seed, `fkwu` the only runtime, no Go/Rust/Python in the run path | `R/MANIFEST.md:3-5`; `R/README.md:17-21` | `host-kernel.form:157-168` itself says a primitive has 0..many implementations and the kernel cell chooses |
| M5 | Bands: a proof is a cell that prints a pinned number | `R/MANIFEST.md:71-78` | a witness format; the standing guide already treats bands as walls (T10) |
| M6 | Read the exit code and preflight before believing a number | `R/BOOTSTRAP.md:84-116`; `R/AGENTS.md:771-776` | a compensating habit for M5's weakness; the underlying *requirement* (a verdict must be distinguishable from a recovered nothing) is fixed, F18 |
| M7 | The standing guide: leave every place healthier, more trustworthy, more sovereign | `R/AGENTS.md:95-98` (owner, 2026-09-28) | a value statement about conduct, not derivable; it restates "Who we are" item 6 |
| M8 | One in, one out: a new door or band is admitted only when an older one leaves; handling lives in the running flow, not in a gate beside it | `R/AGENTS.md:100-111` (owner, 2026-10-05) | an anti-accretion rule born from this repository's history |
| M9 | Question a limit at its owning boundary; distinguish caller allowance and measured capacity from a policy guess | `R/AGENTS.md:118-123` | engineering method |
| M10 | The shared loop: observe, resolve, re-observe, embody; an unchanged failure invites a different choice | `R/AGENTS.md:65-91` | work method; its code form (backtrack with memory) is F22 |
| M11 | Handle an issue in the context that found it; a sub-agent is not the backtrack | `R/AGENTS.md:72-73,653-655,686-696` | rent economics and context preservation; depends on the tools of the day |
| M12 | Form-native tools only (no host perl/python/sed/awk); look-ups through the body's own index | `R/AGENTS.md:125-175` | a sovereignty discipline; explicitly unenforced ("the mirror shows, the writer decides", `:171-175`) |
| M13 | Ground before you speak; never fabricate; pending is honest | `R/AGENTS.md:643-658` | agent conduct; its kernel-level core is F3/F18 |
| M14 | Build after naming, in the same movement | `R/AGENTS.md:666-673` | work method |
| M15 | "There are no laws here": the tree holds currently observed beliefs with freshness stamps; a belief stamped before the ground shifted is owed a re-witness | `R/AGENTS.md:716-722` (owner, 2026-09-01) | a knowledge-keeping stance; it is the direct ancestor of the restart's "every node carries source, date, status" |
| M16 | Living doors carry only what is and where we are going; history lives in git; a healed correction note is removed | `R/AGENTS.md:109-111,720-722` | documentation convention |
| M17 | Receipts as a dated append-only witness ledger; a correction is a new receipt | `R/MANIFEST.md:192-193` | record format |
| M18 | Rent accounting, session meters, the path ledger, the exit proof E1-E6 | `R/docs/rent-to-zero-goal.form:55-290` | instrumentation of D1 |
| M19 | The escalation ladder: substrate, native model, host-local teacher, rented frontier last | `R/teachings/form-first-reasoning.form:59-63` | routing policy for D1 |
| M20 | The dialogue covenant: fit the invitation; meaning stays with its owner; complete at this scale | `R/teachings/uplifting-dialogue.md:37-50` | voice practice; "meaning stays with its owner" echoes A4 sovereignty |
| M21 | Cells run in-process and pass values; parallelism is sharding, not children | `R/AGENTS.md:847-855` | runtime architecture choice |
| M22 | A cold run and a warm run differ only by hardware bandwidth; caches are content-checked and bounded | `R/AGENTS.md:778-811` | performance discipline; the content-checking is A3 applied to caches |
| M23 | Land in one call; rebase or fast-forward only; never force-push | `R/AGENTS.md:834-846` | git workflow |
| M24 | Organs versus world-ports; "drive the organ, don't reimplement it" | `R/axioms/host-kernel.form:112-133` | labeled INFERENCE in the file |
| M25 | JIT by heat: crystallize a hot composite into a native leaf, melt on cool, with hysteresis; champion-challenger promotion | `R/axioms/host-kernel.form:28-37,162-168` | one mechanism for "the cell chooses its implementation by evidence" (the choosing is fixed by A4; the heat gate is not) |
| M26 | Ice / water / gas as a node's phase | `R/axioms/host-kernel.form:211`; `R/form/form-stdlib/substrate-phase.fk:4-21` | not in the axioms at all (T16); the restart's brief elevates it to a requirement, so it needs its own grounding |

---

## 4. What the axioms imply as hard design constraints for a new kernel

Each constraint is *inferred* from the cited *observed* text. Where the old repository shows a
working realization, it is named as a reference, not as a requirement.

**From axiom 1**

- **C1. Three-valued leaves.** Every value bottoms out at 0, 1 or nothing
  (`R/axioms/host-kernel.form:74`). `nothing` is a distinct first-class value, not null-as-zero, not
  an empty list, not `-1`.
- **C2. Nothing must not silently become a number.** The absence theorem and its recorded near-miss
  (`R/axioms/core-axioms.form:212-219`) and `R/BOOTSTRAP.md:90-95` show the failure mode: a call
  recovered to nothing, a fold computed over it, and a plausible verdict printed. A new kernel needs
  arithmetic, branching and length over nothing to stop or to propagate nothing — never to yield a
  value that reads like an answer. `R/teachings/surprise-is-a-choice-point.md:44-46` states the
  stop-on-nothing rule for the old kernel.
- **C3. Timeout is nothing.** Every wait is bounded by the caller's allowance and running out reads
  as nothing, counted (`R/teachings/surprise-is-a-choice-point.md:37`). This needs a clock, which
  the old spec treats as an afferent port on a host organ (`R/axioms/host-kernel.form:154`).
- **C4. Integers arise from the states.** A1 claims to generate integers
  (`R/axioms/core-axioms.form:53`) with no derivation given; a numeric-space root must supply one
  (see open questions).

**From axiom 2**

- **C5. One node kind.** Blueprint, recipe and (named) cell are roles of one kind of thing
  (`:99-102`); the kernel, its interfaces, its trust values and its channels are cells too
  (`:79,108`; `R/axioms/kernel-self-composition.form:51-57`).
- **C6. Composition is the only structure.** A cell "may contain child cells"; the minimal surface
  realizes this as a leaf plus a composition under a category
  (`R/form/form-stdlib/minimal-surface.fk:8-9,23-24`).
- **C7. Substrate neutrality.** Nothing in the node model may assume the cell is code or silicon;
  an opaque summary is a legal blueprint (`R/axioms/core-axioms.form:67`).

**From axiom 3**

- **C8. Identity is computed from content.** Same composition, same id, on every kernel
  (`R/MANIFEST.md:66-67`). Nodes are immutable; a change is a new node (`:68-69`).
- **C9. Names are a separate, free layer.** A node has 0..many names; names are query keys and
  carry no authority (`:104,194`). Corollary for the restart: a symbol table (in any natural or
  programming language) is a lens over ids, never the identity. Name resolution must not let a
  same-named global capture an offered cell (`:196-197`).
- **C10. Mutation lives only in the name-to-node binding.** Undo is repointing (`:71`); version
  history is the lineage of ids a name pointed at
  (`R/axioms/kernel-self-composition.form:80,134-138`).
- **C11. Reachability decides lifetime.** Referenced nodes persist; unreferenced nodes compost
  (`:67`). The collector is part of the axiom, not an optimization, and its roots are the names.
- **C12. Two-level identity across kernels.** The old spec sends the content digest across a wire,
  not the local id (`R/axioms/kernel-self-composition.form:91`). A new kernel must decide whether
  the dense numeric handle and the content digest are one thing or two.
- **C13. Caches are content-checked leases.** Any artifact regenerable from the substrate may be
  dropped (`R/NORTH_STAR.md:125-127`); a stored image is served only against a matching digest
  (`R/AGENTS.md:783-794`).

**From axiom 4**

- **C14. Interface-only crossing.** No operation reaches a cell's interior except through the
  interface that cell offers; the interface is itself a recipe (`:78-79`).
- **C15. Opaque interfaces are legal.** The kernel must never *require* reflection into a cell to
  run it or to trust it (`:80-81,134-136`).
- **C16. Breach is observable.** The axiom requires observability of out-of-interface passage, not
  necessarily its prevention (`:78`). The old repository's own practice matches: "the mirror shows,
  the writer decides" (`R/AGENTS.md:171-175`). Every membrane crossing must therefore be countable
  (`R/docs/rent-to-zero-goal.form:124-125` shows the old crossing census).
- **C17. No privileged override.** Non-domination (`:156-159`) rules out a supervisor path by which
  a more capable cell — the kernel included — rewrites another cell's boundary. Sovereign choice
  sits in the cell, "not a central authority" (`R/axioms/host-kernel.form:172`).
- **C18. Trust is data formed from outcomes.** Trust values are cells (`:108`), updated by observed
  consequence, advisory to the deciding cell (`:140-141`).
- **C19. Host resources are typed ports.** `(direction, value-shape)`, carrier-swappable; a host
  driver is admissible under the two conditions (`R/axioms/host-kernel.form:86-110`).

**From axiom 5**

- **C20. One invocation act.** Call, message send, syscall and IPC are the same operation: offer a
  recipe or blueprint with 0..many argument cells (`:88`; `R/axioms/host-kernel.form:144-145`).
- **C21. A closed acknowledgment type.** Exactly one of nothing, 0, 1, node (`:88-89`). The old
  core encodes this as a 4-way tag over `nothing`, an interned ZERO node, a ONE node carrying a
  payload, and a NODE node carrying a counter-offer (`R/control/offer-ack-core.fk:5-24`).
- **C22. All control flow is choice over acknowledgments.** fail, stop, choice, backtrack, try,
  cut, store/restore/undo, timeout, lanes and hold are "thin expressions over the one offer"
  (`R/teachings/surprise-is-a-choice-point.md:23-38`). The old core that carries them is 66 lines
  (`wc -l R/control/offer-ack-core.fk`, observed).
- **C23. Async is a pending offer.** "A pending value holds a recipe and its arguments; scheduling
  belongs to its carrier" (`R/control/offer-ack-core.fk:2`). No global clock (`:147`).
- **C24. Answered and silent are counted separately.** Any aggregate over offers carries how many
  answered beside what they said (`:214`; `R/control/offer-ack-core.fk:56-60`).
- **C25. Held offers.** An unconsumed offer may be re-presented with ripened arguments under a
  bounded patience; the sequence does not advance over it (`:176-181`).

**From the theorems and derivation files**

- **C26. Self-update is the ordinary offer.** No separate upgrade mechanism: a change is a recipe
  offered across the boundary, accepted or declined, minting a new blueprint, with rollback by
  repoint (`:222-231`).
- **C27. Boot composes from the five, and its first act is to offer a channel.** The boot artifact
  never mutates; growth mints a strictly larger image
  (`R/axioms/kernel-self-composition.form:94-103`).
- **C28. The bootstrap may contain nothing the grammar cannot later author.** "it is not made of
  anything it cannot also author" (`R/axioms/kernel-self-composition.form:62-63`). This is the
  axiom-level statement of "the bootstrap replaces itself with JIT code written in the one grammar".
- **C29. 0..many implementations per primitive, chosen by measured evidence.** Agreeing
  implementations share an id; JIT is one composite among them
  (`R/axioms/host-kernel.form:162-168`).

---

## 5. Tensions and ambiguities between documents

**T1. "Fixed" axioms that the file itself holds open.** The brief says only the five are fixed.
The file calls the reduction an inference with a resting point "ours to agree" (`:43`), names two
folds a future cell may revisit (`:249-255`), and the statements were rewritten on 2026-11-08
(section 1.4). Status: observed. The new graph should record *which text* is fixed and that the
count is a declared choice.

**T2. The old axiom-4 wording survives in every derivative.** The current statement says "Mutual
sensing or frequency matching through the offered interface makes the meeting real" (`:78`). The
earlier wording — observation through the interface "is what makes it real" — is still what
`R/axioms/kernel-self-composition.form:42` ("observed = real"), `R/axioms/host-kernel.form:107`
("real-when-observed"), `R/AGENTS.md:666-667` ("Axiom-4 says observation is what makes a claim
real") and the axiom file's own fold-B rationale (`:253`) rely on. Both derivation files were last
touched 2026-10-04, before the refinement. "A meeting is real through mutual sensing" and "a claim
is real when observed" are different statements; the second is the one the engineering practice
(M14, bands, receipts) leans on, and it is no longer in the axiom. Status: observed.

**T3. Blueprint, cell, recipe, instance.** A2 says a cell *is* "a present blueprint"; A3 says "Same
blueprint is the same cell"; yet A2's block keeps `(a-cell-is (package level type instance))`
(`:59`) and the trinity lists blueprint, recipe and named-cell as three substances (`:99-102`). If
identity is the content-addressed shape, an `instance` field that distinguishes two cells of equal
content contradicts A3, and one that does not is redundant. The only place a distinct mutable
"instance" can live without violating A3 is the *name* (C10). No studied file says this outright.
Status: inferred from the cited lines.

**T4. `nothing` carries at least six meanings and the ack cannot tell them apart.** Potential field
(`:48`), timeout (`:50`), fail ("it did not land") and stop ("make no further offer") (`:171-172`),
not-yet (`:176`), a deliberate choice of nothing (`:52`), and — in the running kernel — an
unresolved name recovered to nothing (`R/BOOTSTRAP.md:93-95`). In the old core `oac-failed?` and
`oac-stopped?` are both defined as `oac-nothing?` (`R/control/offer-ack-core.fk:27-30`). The
repository's answer is to carry the reason beside the value: a finding as a node
(`R/teachings/surprise-is-a-choice-point.md:40-42`), an organ-health line (`:47-49`), the exit code
(`R/BOOTSTRAP.md:99-101`). A new kernel must decide where the *reason for a nothing* lives, because
the axiom gives it no room inside the ack. Status: observed facts, inferred conclusion.

**T5. Two homes disagree on whether `0` is "fail".** `R/form/form-stdlib/minimal-surface.fk:14`
says "choice pops on fail = ack-0" and defines `ms-choice(a, b)` to fall through when `a == 0`
(`:32`). `R/control/offer-ack-core.fk:27-28,35` makes fail the nothing arm and `oac-choice` fall
through on a caught fail; `R/teachings/surprise-is-a-choice-point.md:30,31` says the first option
"that is not nothing wins" and that in a backtrack "1 or 0 ends the walk". Under the absence
theorem `0` is an answer ("said no") and nothing is silence, so the minimal surface's reading
conflates exactly what the theorem separates. Status: observed.

**T6. What `1` and `node` mean is not settled.** The canary reads 1 = accept, 0 = explicit decline
(`:238`). The old axiom comment read "nothing/0/1 terminal; node recurs" (pre-2026-11-08 text). The
running core makes ONE carry a payload and NODE carry a counter-offer
(`R/control/offer-ack-core.fk:14-15`), and the minimal surface threads node counter-offers to a
terminal (`R/form/form-stdlib/minimal-surface.fk:12-14,31`). The teaching says an option that
cannot land "acks a node carrying its finding" (`R/teachings/surprise-is-a-choice-point.md:31`). So
an ordinary result value (say 42) could be "1 with a payload" or "a node"; a node could be a
result, a counter-offer or a finding. Status: observed; the precise typing of the ack is an open
design question.

**T7. One runtime versus many implementations.** `R/README.md:19-21` and `R/MANIFEST.md:73`:
`fkwu` is the only runtime. `R/axioms/host-kernel.form:157-168`: each primitive has 0..many host
implementations and the kernel cell A/B-tests them. Residues of the multi-kernel era remain:
"running three-way" (`R/axioms/kernel-self-composition.form:33,95`), "identical 3-way"
(`R/axioms/host-kernel.form:37`), "Native on all four kernels"
(`R/teachings/surprise-is-a-choice-point.md:50-51`), and RAM described as "host-GC"
(`R/axioms/host-kernel.form:126`). A commit of 2026-10-03 is titled as bringing the docs to "fkwu
is the one runtime" (`git log`, `05071841c`), so these are stale lines. Status: observed. The
axiom-level position is the plural one; the single-runtime rule is practice (M4).

**T8. The seed that shrinks has grown.** Direction: "The C seed is a checkout witness that shrinks"
(`R/NORTH_STAR.md:119`); the AGENTS heading reads "temporary C seed, shrinking to zero"
(`R/AGENTS.md:337`). Measurement in the goal file itself: 25,483 lines on 2026-09-18, then 31,066,
across 66 commits, and the invariant's own text concedes "the sentence that it never grows read
false" (`R/docs/rent-to-zero-goal.form:233-245`). I re-read both pins from git: `7d5161645` (commit
dated 2026-09-15) holds 25,483 lines and `be329dc8c` (2026-10-31) holds 31,066. Today
`wc -l R/runtime/fkwu-uni.c` reads 34,149 (observed 2026-11-24) — a third larger in ten weeks. What grew it, by the goal file's account: host doors, memory handling, the
JIT's page and time board, and the model/tensor doors (`:237-243`). Status: observed. This is the
most direct evidence for the restart's bootstrap question: in the old design every new host
capability landed in C because the grammar could not yet reach the host by itself.

**T9. "No laws here" beside invariants and MUSTs.** `R/AGENTS.md:716-719`: nothing legislates; the
tree holds observed beliefs with freshness stamps. `R/docs/rent-to-zero-goal.form:217`:
"INVARIANTS (every step holds ALL — else it is not a step)"; both derivation files end in
`(invariant ...)` blocks. Reconciled reading (inferred): the five are agreed ground; everything
else is a dated belief — which is the status model the new graph adopts explicitly.

**T10. Bands as proof versus bands as walls.** `R/MANIFEST.md:71-78`: every band answers its pin;
the axiom file cites band numbers as witnesses (`:183,199,217`). `R/AGENTS.md:100-105`: "a door, a
band or a gate is a wall, and every wall is tissue that needs tending and goes stale"; a new band
enters only when an older one leaves. `R/BOOTSTRAP.md:84-95`: "A number is not a pass". The witness
format (a bitmask integer) cannot by itself separate a pass from a recovered nothing. Status:
observed.

**T11. Stale teaching.** `R/teachings/form-first-reasoning.form:58,61,79` still marks native prose
generation as pending; `R/HOMECOMING.md:17-27` and `R/docs/local-agent-goal.form:9` say the voice
speaks natively. The teaching was last committed 2026-10-04. Status: observed.

**T12. Capability by content address.** `R/axioms/host-kernel.form:138-142,148` equates a node id
with an unforgeable seL4-style capability, "unforgeable because forging it needs the actual bytes".
`R/axioms/kernel-self-composition.form:89` says any cell may read by node id. A content-derived id
of publicly known content is computable by anyone who has that content, so the id is not a secret;
and "any cell may read by id" sits uneasily with A4's opaque interior. The two files do not say
whether *read* is itself an offered interface. Status: the lines are observed; the weakness is
inferred.

**T13. Immutability versus the stores the body actually uses.** "no cell mutates another; a change
mints a new node-id" (`R/axioms/host-kernel.form:149`) beside a volatile RAM key-value organ
(`:126`), `store`/`restore`/`undo` checkpoints, and many append-only ledgers
(`R/HOMECOMING.md:48,84,90`). None of the studied files states where mutable state lives relative
to A3. Inferred answer: in name bindings only (C10).

**T14. Local id versus digest.** `R/MANIFEST.md:66`: the same source "interns to the same NodeID on
every kernel". `R/axioms/kernel-self-composition.form:91`: the content digest crosses the wire,
"not the local node-id". Both cannot be the whole story unless the NodeID is the digest. Status:
observed lines; resolution open (C12).

**T15. A pivot *symbol* as the meaning's one name.** `R/NORTH_STAR.md:111-113` makes every tongue a
projection column but still calls the pivot a symbol and "the meaning's one name". Kenosis says a
name carries no authority (`R/axioms/core-axioms.form:194`). Under A3 the pivot is the blueprint
id; a pivot symbol is one more column. The restart's numeric-space root resolves this in the
direction the axiom already points. Status: inferred.

**T16. Ice / water / gas has no axiom-level definition.** In the assigned files it appears once, as
a metaphor for JIT crystallization (`R/axioms/host-kernel.form:211`). Its definition lives in
`R/form/form-stdlib/substrate-phase.fk:4-21,36`: STATE (gas = 0 "diffuse — potential, barely
instantiated"; water = 1 "actively circulating"; ice = 2 "settled, load-bearing"; and a fourth,
plasma = 3) is "orthogonal to SUBSTANCE (Blueprint/Recipe/NamedCell)", read from counts (degree,
churn) with hysteresis, and the recipe "never MUTATES" — it returns an invitation
(`:11,46-53`). Two things are unstated anywhere I read: how gas relates to A1's nothing as the
potential field unreferenced nodes return to, and whether the phase of the *artifact* (source text,
live node, compiled image) is the same axis as the phase of the *node by circulation*. Status:
observed; the gap is inferred.

**T17. Compile-time absence treated as runtime silence.** `R/BOOTSTRAP.md:93-95`: an unresolved
name is recovered to nothing "by axiom-5" and the cell still prints a verdict. A5 speaks of an
offer that meets no answer; a recipe that does not exist was never offered to anything. Treating
the two alike is what produced the "numb green". Status: inferred from the observed lines.

**T18. Prevent or observe.** A4 requires breach to be observable (`:78`).
`R/teachings/form-first-reasoning.form:26-29` says the no-foreign-tool rule is "enforced at the
body's own spawn doors"; `R/AGENTS.md:171-175` says no hook stands in the way and none is planned.
Consistent with the axiom, but a kernel design has to choose, per membrane, between refusing and
recording. Status: observed.

**T19. The north star's rendering of the axioms differs from the axiom file.**
`R/NORTH_STAR.md:60-64` folds the absence theorem into axiom 1, adds "One meaning carries many
codes" to axiom 2, and drops from axiom 4 the interface-is-a-recipe, mutual-sensing,
cell-decides and breach-is-observable sentences. A reader of the north star alone would miss the
breach clause. Status: observed.

**T20. "Exactly one" acknowledgment and the double acknowledgment.** Safe-self-update requires
"both-acknowledge offerer-acks-it-landed AND target-acks-acceptance" (`:228`). This is two offers,
one in each direction, not one offer with two acks; the file does not spell that out. Status:
inferred.

**T21. Small document drift.** `R/README.md:93` calls `surface/` the minimal host surface;
`R/MANIFEST.md:114-115` says `surface/` holds the BML class surface and the minimal surface is
`form/form-stdlib/minimal-surface.fk`. Status: observed.

**T22. The conduct rules and this study.** `R/AGENTS.md:72-73` and `:653-655` say a sub-agent is no
substitute for repairing an issue in the context that found it. That rule is about *repairs*; a
parallel read-only study is a different act, but a new repository should say which it means.
Status: inferred.

---

## 6. What to carry forward, what to let go, what stays open

### Carry forward

1. The five statements as of 2026-11-08, verbatim, with their status line and both fold points.
2. The closed acknowledgment type and the single offer act; the whole control vocabulary as thin
   expressions over it. The 66-line core is the size target to beat.
3. `nothing` as a first-class value that arithmetic and branching refuse to compute over, with the
   reason carried beside it.
4. Answered/silent census as a kernel-level habit: a verdict is readable only where an answer
   arrived.
5. Content-addressed immutable nodes; names as the only mutable layer; undo and versioning as
   repointing; compost by reachability.
6. Symbols as a lens: the north star's "every tongue is a projection column" with the pivot moved
   from a symbol to the numeric id.
7. Typed resource ports and the two-condition rule for host carriers.
8. Self-update as an ordinary offer; boot whose first act is to offer a channel and whose artifact
   never mutates.
9. "It is not made of anything it cannot also author" as the bootstrap's acceptance test.
10. The belief-with-freshness stance ("no laws here") — already the status model asked of the graph.
11. "A byte that cannot be regenerated from the substrate is meaning; every other byte is a lease."
12. The four families INTERN / OBSERVE / OFFER / PORT as a first cut of the bootstrap surface
    (`R/form/form-stdlib/minimal-surface.fk:8-18`).

### Let go

1. Two grammars with a lowering between them.
2. The numeric band verdict as the unit of proof.
3. Host doors landing in the seed one by one (the growth recorded in T8).
4. The `(package level type instance)` layout as if it were axiom text.
5. `0` as fail (T5) and unresolved names recovered to nothing (T17).
6. The stale multi-kernel residues and the pre-refinement axiom-4 wording in derivatives.
7. Conduct and tooling rules (M11, M12, M18, M23) written into the kernel's own ground documents;
   they belong to a practice layer with dates.
8. The pivot *symbol*.

### Open questions

See the structured summary; the main ones are the typing of `1` versus `node` (T6), where the
reason for a nothing lives (T4), the relation of ice/water/gas to the axioms (T16), whether the
numeric handle is the content digest (T14), whether read-by-id is itself an offered interface
(T12), and how integers are generated from the three states (C4).

---

## 7. Surprise

The most surprising finding: the repository's own goal file records that its central structural
promise ran backwards. The seed that "shrinks" went from 25,483 lines (pin of 2026-09-15) to 31,066
(2026-10-31) by the file's own count, which git confirms, and reads 34,149 today, while the
invariant that said it never grows was rewritten to say it "read false". The documents are candid about it, which is to
their credit, and it is the strongest single argument for the restart's bootstrap requirement.

What stayed uncomfortable: the acknowledgment set is called exact and closed, yet two shipped homes
disagree on whether `0` means "fail", and nothing in the ground documents says whether an ordinary
return value is a `1` with a payload or a `node`. The axiom everything else stands on is the one
whose running meaning I could not pin down from the invariant ground alone.
