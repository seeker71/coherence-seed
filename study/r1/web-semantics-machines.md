# Semantic machinery one grammar must carry natively

Study for the coherence-kernel restart. Written 2026-11-24. All web sources were fetched on 2026-11-24 unless a line says otherwise.

## 0. How to read this report

Status legend, used on every claim:

- **observed**: I read the source (or ran the command) in this session. The source and its own date are given.
- **observed-snippet**: I saw the statement only as a search-engine summary of a primary source that I could not open directly. Weaker than observed.
- **inferred**: my reasoning from observed things; the premises are named.
- **assumed**: from model memory, not verified in this session. Source is "model memory, unverified".

PDF sources were fetched by the web tool, saved by it as binary, and converted locally with `pdftotext`; the text copies are in
`scratch/pdftext/` (`verse.txt`, `rr.txt`, `icon.txt`, `schulte.txt`, `sel4.txt`, `traits.txt`, `bacon.txt`). Line numbers below that name one of these files refer to those text copies.

The fixed ground this study is measured against is `coherence-kernel/axioms/core-axioms.form` (status line 17: acknowledged 2026-11-08): axiom 1 states (line 48), axiom 2 cell (line 58), axiom 3 content-addressing (line 67), axiom 4 boundary (line 78), axiom 5 offer (line 88). All observed.

Sources I tried and could not read (so nothing below rests on them): the microKanren paper PDF at webyrd.net (connection refused; I read the reference source code instead), the Mozart online manual (certificate error; I read Schulte's thesis instead), erights.org (certificate error), Epic's AutoRTFM tech-blog (HTTP 403; only a search summary), the ACM page of the unified-GC paper (403; I read a PDF copy instead), the Tübingen page for the 2025 multi-shot handler paper (connection refused; only a search summary), the Koka book body (the page is 834 KB and mostly inline image data; I read the tour source file instead), the Verse "failure" documentation page (rendered empty).

---

## 1. Control: choice, fail, stop, cut, undo, store/restore, timeout

### 1.1 What each reference system actually has

**Warren Abstract Machine (Prolog).**
- Original: D. H. D. Warren, "An abstract Prolog instruction set", SRI, October 1983. Memory areas: heap (global stack) for compound terms, local stack for environments and choice points, trail for bindings to undo. Source: https://en.wikipedia.org/wiki/Warren_Abstract_Machine (page last edited 2025-08-11). observed.
- Choice point frame, as laid out in Aït-Kaci's "Warren's Abstract Machine: A Tutorial Reconstruction" (the Wikipedia page cites the edition dated 1999-02-18; first publication in 1991 is assumed from model memory), p. 38: arity n; argument registers A1..An; CE (continuation environment); CP (continuation pointer); B (previous choice point); BP (next clause to try); TR (trail pointer); H (heap pointer). That is n+7 machine words. Source: text of the tutorial at https://studfile.net/preview/429183/page:6/ (a third-party copy of the book; pages 36-41). observed.
- Trail: an array of addresses of variables that must be reset to unbound on backtracking; only *conditional* bindings are trailed, meaning a heap variable with address below HB or a stack variable with address below B (pp. 36-37, same source). observed. Cost reading: one compare per binding, one word pushed only when the variable is older than the newest choice point. inferred from the quoted condition.
- `try_me_else` pushes the frame and sets HB to H; `retry_me_else` restores arguments, E, CP, H, unwinds the trail to the saved TR, and updates BP in place; `trust_me` does the same restore and pops the frame (pp. 39-41, same source). observed.
- The decisive property: backtracking never copies a stack. The frames below a choice point are simply not popped until the choice point is gone; each alternative is entered once, in sequence, from the saved registers. inferred from the frame layout and the three instructions above.
- Cut, as specified for users: "Discard all choice points created since entering the predicate in which the cut appears"; meta-call is opaque to cut; `;`, `->`, `*->` are transparent to it; if-then-else commits to the condition's first solution but keeps the predicate's own clause choice point. Source: SWI-Prolog manual, https://www.swi-prolog.org/pldoc/man?section=control (no date on page). observed. The WAM implementation of cut (a saved cut barrier register, one assignment to B) is assumed (model memory, unverified; the page of the tutorial that covers it was not fetched).

**Icon (and SNOBOL before it).**
- SNOBOL, 1962-1967, Bell Labs (Farber, Griswold, Polonsky): patterns are first-class data, every statement succeeds or fails and can branch on either, matching backtracks. Source: https://en.wikipedia.org/wiki/SNOBOL (last edited 2026-05-19). observed.
- Icon: "Failure during the evaluation of an expression causes previously evaluated generators to produce additional values. This is called goal-directed evaluation" (Griswold & Griswold, *The Icon Programming Language*, 3rd ed., ch. 2, p. 18; `icon.txt` lines 1399-1402). Source PDF: https://www2.cs.arizona.edu/icon/ftp/doc/lb1up.pdf. observed.
- Icon separates **control backtracking** (resuming a suspended generator; `icon.txt` 1430-1437) from **data backtracking** (undoing a change). Only a few operations undo data: the scanning functions `tab` and `move` restore the position, and there is a reversible assignment `<-`; ordinary `:=` is "irreversible" (ch. 7, `icon.txt` 4386-4432). observed.
- Icon's cut is the **bounded expression**: "If an expression is bounded and produces a result, all suspended generators in it are discarded. It is therefore impossible to backtrack into a bounded expression." The stated reasons are meaning (the condition of an `if` must not be resumed) and space (suspended generators would otherwise live forever) (ch. 7, p. 90; `icon.txt` 4440-4462). observed.
- Overview dated 1996-03-02 confirms success/failure instead of booleans, generators, alternation `|`, `every`: https://www2.cs.arizona.edu/icon/docs/ipd266.htm. observed.

**The Verse calculus (Epic, ICFP 2023).**
- Augustsson, Breitner, Claessen, Jhala, Peyton Jones, Shivers, Steele, Sweeney, "The Verse Calculus: A Core Calculus for Deterministic Functional Logic Programming", Proc. ACM Program. Lang. 7, ICFP, Article 203, August 2023. Sources: https://simon.peytonjones.org/verse-calculus/ and the extended PDF https://simon.peytonjones.org/assets/pdfs/verse-icfp23.pdf. observed.
- Whole core syntax (Fig. 1, p. 203:4): values are variables or head-normal forms (integers, primitive operators, tuples, lambdas); expressions are `v`, `eq; e`, `∃x. e`, `fail`, `e1 | e2`, `v1 v2`, `one{e}`, `all{e}`; an `eq` is an expression or an equation `v = e`. Seven expression forms. observed.
- Meaning (section 2.2, p. 203:6): an expression "evaluates to zero, one, or many values; or it can get stuck, which is different from producing zero values". `fail` yields none, a value yields one, `e1 | e2` yields all of e1 then all of e2, order kept, duplicates kept. observed. This is the cleanest published match to axiom 1's distinction between an explicit no and silence: fail is "zero results", stuck is "not yet". inferred from that sentence and axiom 1 line 48.
- `one{e}` fails if e fails, else returns e's first value; `if` is not primitive: `if (∃x. e1) then e2 else e3` means `(one{(∃x. e1; λ⟨⟩. e2) | (λ⟨⟩. e3)})⟨⟩` (Fig. 1 and section 2.5, p. 203:8). `all{e}` reifies choice into a tuple; indexing a tuple with an unbound variable turns the tuple back into a choice (section 2.6, p. 203:9). `for` desugars into `all` plus `map` (section 2.8, p. 203:10). One equality operator serves both testing and binding, "following the lead of Icon" (section 2.5). observed.
- A single more general operator can replace both `one` and `all`: `split{e}⟨f, g⟩` calls `f⟨⟩` if e fails and `g⟨v, λ⟨⟩. rest⟩` if e yields a first value v with the rest suspended (section 5.3, p. 203:23; the name comes from Kiselyov et al. 2005). observed. So the choice-consuming side reduces to one primitive.
- Choices are "laid out in space" (in the term) "rather than ... in time (via non-deterministic rewrites and backtracking)" (p. 203:3, and section 6.1). The calculus is deterministic and confluent for well-behaved terms. observed.
- Effects: "The full Verse language also has a statically checked effect system, including both mutable references and input/output. All these effects must be transactional; for example, when the condition of an if fails, any store effects in the condition must be rolled back" (conclusion, `verse.txt` 1449-1452). Appendix F adds `alloc`, `read`, `write`; under `split` "the store is duplicated" (rule st-split-dup, `verse.txt` 4403-4424). observed.
- Epic's documentation calls failure contexts "a form of speculative execution": on success the effects are kept, on failure they are rolled back as if the expression never ran. Source: https://dev.epicgames.com/documentation/fortnite/speculative-execution (undated). observed.
- Epic's March 2024 tech blog "Bringing Verse Transactional Memory Semantics to C++" describes failure contexts as nested transactions, a compiler-instrumented C++ runtime (AutoRTFM) that keeps "a log of memory locations and their old values", and admits that Verse runtime errors did not yet roll back correctly. URL https://www.unrealengine.com/tech-blog/bringing-verse-transactional-memory-semantics-to-c returned 403 to me. observed-snippet. I found nothing dated 2025 or 2026 on the state of this.

**microKanren.**
- The reference implementation is 51 lines of Scheme: `var`, `var?`, `var=?`, `walk`, `ext-s`, `==`, `unit`, `mzero`, `unify`, `call/fresh`, `disj`, `conj`, `mplus`, `bind`. A state is a pair (substitution . counter); the substitution is an association list; a goal maps a state to a stream; an immature stream is a thunk, and `mplus` swaps its arguments when it forces one, which interleaves the two branches and makes the search complete. Source: https://raw.githubusercontent.com/jasonhemann/microKanren/master/microKanren.scm (copyright 2013 Hemann and Friedman). observed.
- Four user-level primitives suffice: `==`, `call/fresh`, `disj`, `conj`. Because the substitution is a persistent value, there is no trail at all: a branch is abandoned by dropping its state. inferred from the source just cited. Cost: variable lookup is linear in the association list. inferred.

**Mozart/Oz computation spaces.**
- Schulte, *Programming Constraint Services*, doctoral thesis, Saarbrücken, 2000: https://www.ps.uni-saarland.de/Publications/documents/Schulte_PHD_2000.pdf. Operations (section 4.8 summary, `schulte.txt` 2014-2066): `NewSpace: Script → Space`, `Inject`, `Merge: Space → Any`, `Ask: Space → Status` (waits until the space is stable), `Clone: Space → Space`, `Commit: Space × Alternative`. States: running, stable (failed, succeeded, distributable), merged. observed.
- A search engine is ordinary code: ask; if distributable, clone, commit the original to alternative 1 and the clone to the rest (`schulte.txt` 1946-1952). observed.
- "search is based on copying rather than on trailing"; the thesis "establishes the competitiveness of copying by a rigid comparison with trailing"; recomputation "saves space, possibly at the expense of increased runtime" and with copying gives "a fundamental improvement over trailing-based search for large problems" (abstract, `schulte.txt` 96-105). observed.

**Mercury and Curry.**
- Mercury gives every mode of every predicate a determinism: det (exactly one solution), semidet (zero or one), multi (one or more), nondet (zero or more), failure, erroneous; plus committed-choice cc_multi and cc_nondet where the implementation commits to the first solution and can never be asked for another, which "allow[s] the compiler to generate much more efficient code". Determinism is inferred for local procedures and checked against declarations. Source: https://mercurylang.org/information/doc-latest/mercury_reference_manual/Determinism.html (undated "latest"). observed.
- Curry describes itself as "a truly integrated functional logic programming language" with non-determinism, free variables and built-in search; implementations listed: PAKCS, KiCS2, KMCC, Curry2Go, MCC. Source: https://www.curry-lang.org/ (footer 1995-2025). observed. Curry's laziness is what makes its encapsulated search subtle, according to the Verse paper section 6.2. observed (Verse paper).

**amb, PEG, packrat.**
- SICP section 4.3: `(amb e1 ... en)` returns one of the values, `(amb)` fails; the evaluator does depth-first "chronological backtracking"; it is implemented by giving every execution procedure a success continuation and a failure continuation; ordinary assignment is undone on failure (the failure continuation restores the old value), and exercise 4.51 asks for a `permanent-set!` that is not undone. The footnote credits McCarthy (described 1961, cited as McCarthy 1967). Source: https://mitp-content-server.mit.edu/books/content/sectbyfn/books_pres_0/6515/sicp.zip/full-text/book/book-Z-H-28.html. observed.
- PEG: Ford, POPL 2004. Where CFGs have nondeterministic choice, PEGs "use prioritized choice"; "a linear-time parser can be built for any PEG". Source: https://bford.info/pub/lang/peg/. observed. Packrat: Ford, ICFP 2002: backtracking with unlimited lookahead and guaranteed linear time. Source: https://bford.info/pub/lang/packrat-icfp02/. observed. That the linear time is bought by memoizing every (rule, position) result, at storage proportional to input size, is assumed (model memory, unverified; the abstract page did not say it).
- Reading: PEG's ordered choice is exactly `one{e1 | e2}`, and packrat is tabling keyed by (recipe, position). A grammar is then just a recipe that uses ordered choice; no separate parsing machinery is needed. inferred from the PEG abstract and the Verse definition of `one`.

**Delimited continuations.**
- Racket (reference section 10.4, docs for version 9.3): prompts carry tags; `call-with-composable-continuation` captures up to the nearest prompt and applying the result "extends the current continuation"; continuation barriers and `dynamic-wind` interact with jumps. Source: https://docs.racket-lang.org/reference/cont.html. observed. Composable continuations are not consumed by application, so they are multi-shot. inferred from that text.
- SWI-Prolog `reset/3` and `shift/1` implement delimited continuations (Schrijvers et al. 2013) and are what its tabling is built on; `shift` captures the forward continuation but not choice points, and cut and if-then-else inside a saved continuation do not behave as expected. Source: https://www.swi-prolog.org/pldoc/man?section=delcont. observed. Reading: continuations and choice points are two different machine structures even inside one mature system, and combining them has sharp edges. inferred.

**Algebraic effects and handlers.**
- Plotkin and Pretnar, "Handling Algebraic Effects", LMCS 9(4), December 2013 (arXiv 1312.1399): handlers generalise exception handlers to any algebraic effect; the abstract lists exceptions, state, nondeterminism, interactive I/O and time as effects, and timeout, rollback and stream redirection as handler examples. Source: https://arxiv.org/abs/1312.1399. observed.
- Koka (tour source, repository at v3.2.9 dated 2026-09-17): a `ctl` operation will "yield to its innermost handler, unwind the stack, and only then evaluate the operation definition"; `resume` returns to the call site; a handler may return without resuming (exceptions); "it is also possible to invoke it more than once", which "can be used to implement backtracking or probabilistic programming models"; `fun` operations are tail-resumptive by construction and have "a performance cost very similar to virtual method calls". Sources: https://raw.githubusercontent.com/koka-lang/koka/master/doc/spec/tour.kk.md and https://github.com/koka-lang/koka. observed. Koka compiles "directly to C code" without a garbage collector or runtime system, by evidence passing plus Perceus (README). observed. The README calls v3 "a research language ... not quite ready for production use". observed.
- Xie and Leijen, "Generalized Evidence Passing for Effect Handlers", ICFP 2021 (MSR-TR-2021-5): a chain of refinements, multi-prompt delimited control, evidence passing, yield bubbling, ending in a monadic translation to plain lambda calculus that compiles to C. Source: https://www.microsoft.com/en-us/research/publication/generalized-evidence-passing-for-effect-handlers/. observed.
- OCaml 5 (manual 5.3, effects since 5.0): continuations must be resumed "exactly once"; a second resume raises `Continuation_already_resumed`; an unresumed continuation leaks its fiber unless discontinued; there is no static effect safety. Source: https://ocaml.org/manual/5.3/effects.html. observed. The implementation paper (Sivaramakrishnan et al., PLDI 2021, arXiv 2104.00250, submitted 2021-04-01) reports a mean 1% overhead on programs that do not use effects. Source: https://arxiv.org/abs/2104.00250. observed.
- Multi-shot on OCaml 5 exists only as a library that clones the continuation before each resume and warns that compiler optimisations (heap-to-stack conversion) can break multi-shot semantics. Source: https://github.com/dhil/ocaml-multicont. observed.
- WebAssembly stack switching (typed continuations: `cont.new`, `cont.bind`, `suspend`, `resume`, `resume_throw`, `switch`): continuations are single-shot, resuming twice traps, and "none of our critical use-cases requires multi-shot continuations". Source: https://github.com/WebAssembly/stack-switching/blob/main/proposals/stack-switching/Explainer.md. observed. Phase 3 (implementation) as listed at https://github.com/WebAssembly/proposals on 2026-11-24. observed.
- One-shot versus multi-shot cost: Bruggeman, Waddell, Dybvig, "Representing Control in the Presence of One-Shot Continuations", PLDI 1996: a segmented-stack implementation that serves both and avoids the copying multi-shot normally incurs when the use is one-shot. observed-snippet (search summary; DOI 10.1145/231379.231395).
- Kobayashi and Kameyama, APLAS 2025 (arXiv 2509.11901, 2025-09-15): one-shot effect handlers and one-shot delimited control can be macro-expressed by asymmetric coroutines, but not the reverse. Source: https://arxiv.org/abs/2509.11901. observed.
- Muhcu, Schuster, Steuwer, Brachthäuser, "Multiple Resumptions and Local Mutable State, Directly", ICFP 2025: lexical effect handlers with multiple resumptions and stack-allocated mutable state, constant-time capture and resume for one-shot use, noting that GHC and Scala Native defensively copy the continuation even for one-shot use. observed-snippet (search summary; the page refused connections).
- Effekt describes itself as "a language with lexical effect handlers and lightweight effect polymorphism", with a static effect system where only programs with no remaining effects can run. Source: https://effekt-lang.org/. observed.

**Transactional memory and persistent structures (cheap store/restore).**
- Harris, Marlow, Peyton Jones (the authors the page lists; a fourth author, Herlihy, is assumed from model memory), "Composable Memory Transactions", PPoPP 2005: transactional memory with "modular forms of blocking and choice". Source: https://www.microsoft.com/en-us/research/publication/composable-memory-transactions/. observed.
- GHC `stm` 2.5.3.1: `retry` abandons the transaction until a read `TVar` changes; `orElse a b` runs b if a retries, and "if both actions retry then the orElse as a whole retries"; `catchSTM`: "any changes made by m are rolled back, but changes prior to m persist". Source: https://hackage.haskell.org/package/stm/docs/Control-Monad-STM.html. observed. Reading: `retry` is fail, `orElse` is ordered choice with rollback, `atomically` is the delimiter; this is the same algebra as Verse's failure contexts, arrived at independently. inferred.
- Hardware transactions are not a foundation to build on: Intel TSX was disabled by microcode on Haswell/Broadwell (2014), HLE was disabled on Skylake (2018), a June 2021 microcode update disabled TSX on many Skylake-to-Coffee-Lake parts, and client processors from 2020 on do not support it; it survives on some Xeons. Source: https://en.wikipedia.org/wiki/Transactional_Synchronization_Extensions (last edited 2026-11-08). observed.
- Clojure: "All of the Clojure collections are immutable and persistent", with structural sharing; vector index and hash-map lookup take "log32N hops". Source: https://clojure.org/reference/data_structures. observed.

**Reversible execution and replay.**
- Janus (Caltech 1982; formalised by Yokoyama and Glück 2007): reversibility by construction, using only invertible updates (`+=`, `-=`, `^=`), conditionals with an exit assertion, loops with an entry assertion, and `call`/`uncall`. Source: https://en.wikipedia.org/wiki/Janus_(time-reversible_computing_programming_language) (last edited 2026-08-15). observed.
- rr (O'Callahan et al., arXiv 1705.05937, 2017-05-16): record all nondeterministic inputs at the user/kernel boundary, run one thread at a time, time asynchronous events with the "retired conditional branches" hardware counter, the one deterministic counter on Intel CPUs; recording slowdown is under 2x on the measured workloads excluding `make`; the approach "cannot be implemented on ARM CPUs" as of that report (`rr.txt` lines 71, 90-95, 313-320, 727-729). Source: https://arxiv.org/pdf/1705.05937. observed. Reading: time travel needs only a boundary and a log of what crossed it, which is axiom 4 read as an engineering rule. inferred.

### 1.2 The smallest known-sufficient primitive set

All inferred, from the observations in 1.1.

Every system above is a combination of five things:

1. **A result that may be zero, one, or many, or not yet.** Verse states it outright. Axiom 5's four acknowledgments (nothing, 0, 1, node) are this plus one distinction Verse leaves to its "stuck" state: 0 is an answered no (fail, safe to backtrack on), nothing is no answer (stuck, timeout, held offer; not a verdict).
2. **Ordered choice.** `e1 | e2`. Prolog clauses, Icon alternation, PEG `/`, STM `orElse`, `amb`, microKanren `disj`.
3. **A delimiter that consumes choice.** Verse `one`/`all` (or the single `split`), Icon bounded expressions, Prolog cut and `->`, `findall`, a handler, a prompt, an Oz space, `atomically`. Cut, once, if-then-else, negation, every/for, and collect-all are all this one construct.
4. **A store discipline tied to that delimiter.** What happens to writes when a branch is abandoned. Four known implementations: trail (WAM, Icon's reversible operations), old-value log (STM, AutoRTFM), copy (Oz spaces), persistent value (microKanren's substitution, Clojure). Verse's rule is the simplest statement: every effect inside a failure context is transactional.
5. **Suspension with resumption.** One-shot is enough for generators-by-coroutine, held offers, timeouts and async; multi-shot is only needed if a resumption escapes as a value and is run more than once.

Derivations:

| asked-for construct | from the set |
|---|---|
| fail | zero results (ack 0) |
| choice | ordered `|` |
| cut / once | `one{e}`; discard remaining alternatives at the delimiter |
| if / not | `one{(c; then-thunk) | else-thunk}` applied (Verse Fig. 1) |
| every / all / for | `all{e}`, or `split` in a loop |
| try | `one` plus a handler for the raised operation |
| stop | abort to a delimiter without resuming (Koka handler that does not resume; OCaml `discontinue`) |
| store / restore / undo | capture the store root at the delimiter; restore on failure. With a content-addressed store this is "re-point a name" (axiom 3 line 71) |
| timeout | the clock delivers "nothing" to a delimiter; the suspended branch is discontinued or left held |
| hold (not-yet) | suspension: the offer stays unconsumed (axioms file lines 174-183) |

Algebraic effect handlers with multi-shot resumptions generate all rows from one mechanism (Plotkin-Pretnar; Koka's tour shows exceptions, state and backtracking as handlers). That makes them the best *semantic* single primitive. They are not the cheapest *machine* primitive for choice: see 1.3.

### 1.3 Machine cost

Observed costs are cited; the rest of each row is inferred from the cited mechanism.

| mechanism | to open a choice | per write | to backtrack | to commit (cut) | source |
|---|---|---|---|---|---|
| WAM choice point + trail | push n+7 words | one compare; push one word if the variable is older than the choice point | restore n+4 registers, pop trail resetting each entry | drop frames by one register assignment (assumed) | Aït-Kaci pp. 36-41 |
| value trail / undo log | mark log top | push (address, old value) | replay log backwards | discard or keep log to parent | STM docs; AutoRTFM snippet |
| copying (Oz) | clone a stable space, proportional to its size | none | drop the clone | merge | Schulte ch. 4, abstract |
| recomputation | nothing | none | re-run from an ancestor copy | nothing | Schulte abstract |
| persistent store | keep the root reference, O(1) | path copy, log32 N nodes | repoint, O(1) | nothing | Clojure reference; microKanren source |
| one-shot continuation | allocate a stack segment | none | discontinue (unwind) | nothing | OCaml manual; 1% mean overhead on non-effect code (PLDI 2021 abstract) |
| multi-shot continuation | as above | none | clone the captured stack per extra resume, proportional to its depth | nothing | ocaml-multicont README |
| tail-resumptive operation | none | none | not applicable | not applicable | "very similar to virtual method calls" (Koka tour) |
| record/replay | log inputs at the boundary | none inside | re-execute from a checkpoint | nothing | rr: under 2x recording slowdown, single-threaded |

Two findings matter most:

- **Chronological backtracking does not need multi-shot continuations.** The WAM re-enters each alternative from saved registers and protects the frames below; nothing is copied. Mainstream control substrates (OCaml 5, WebAssembly stack switching) are deliberately one-shot. So "build choice on the platform's continuations" would be the expensive route; a choice-point stack is the cheap one. inferred from the WAM frame, the OCaml manual and the Wasm explainer.
- **Determinism information erases the cost where it is not needed.** Mercury's categories let det code compile as plain calls and semidet as a branch, and committed choice "generate[s] much more efficient code". inferred from the Mercury manual plus the frame cost above.

### 1.4 Design recommendation for the one grammar

All inferred; each is a proposal, not a finding.

- R1. Make "an offer yields nothing, 0, 1 or node(s)" the evaluation rule of every expression, not a library. Booleans are not needed for control (Icon, Verse). Keep 0 (answered no) and nothing (no answer) distinct all the way down to the machine: fail triggers backtracking; nothing triggers hold or timeout handling and is never read as a verdict (axioms file lines 212-219).
- R2. Surface primitives: `fail`, ordered choice, and one choice-consuming delimiter (`split`-shaped, with `one` and `all` as its two common forms). Define `if`, `not`, `every`, `try`, `cut` by desugaring, as Verse does.
- R3. Operations plus handlers as the general control form, with three resume classes fixed by the operation's declaration and checked: never resumes (stop, raise), resumes exactly once in tail position (compiled as an indirect call), may resume later or more than once (general). This is Koka's `final ctl` / `fun` / `ctl` split used as a cost contract.
- R4. All effects inside a choice-consuming delimiter are transactional (Verse's rule). Implementation by state of the node: immutable content-addressed nodes need no undo at all (restore is repoint); live mutable cells get a value trail scoped to the innermost choice point; effects that crossed the boundary of the body cannot be undone and must be either forbidden inside a failure context or declared with a compensating recipe.
- R5. Machine substrate: a choice-point stack (frame, trail mark, allocation mark), a value trail, and one-shot stack switching for suspension. A resumption that escapes as a value is frozen into a node (copied once, then content-addressed), which is where multi-shot is paid for, and only there.
- R6. Infer determinism per recipe (Mercury's lattice) and store it on the blueprint. Det recipes compile with no choice machinery; semidet as a conditional branch; only nondet recipes touch the choice stack.
- R7. Timeout is not an exception type. It is the clock acknowledging "nothing" to a delimiter, after which the branch is discontinued or held (Erlang's `receive ... after` is the working model: section 3.1).
- R8. Tabling (memo keyed by recipe id and argument ids) comes for free from content addressing and subsumes packrat parsing.

---

## 2. Object model, multiple inheritance, generics

### 2.1 What each reference system actually has

**The fat pair (data pointer, table pointer).**
- Go (Russ Cox, "Go Data Structures: Interfaces", 2009-12-01): an interface value is two words, a pointer to an itable and a pointer to the data. The itable belongs to the pair (interface type, concrete type) and holds only that interface's methods. The runtime builds it on first use by walking the two sorted method tables in O(ni + nt) and caches it. A call is `s.tab->fun[0](s.data)`: two loads and an indirect call. Go is "halfway between" static tables (C++, Java) and per-call lookup (Smalltalk): the lookup moves to the point where the value is stored into the interface. Word-sized values can sit directly in the data slot. Source: https://research.swtch.com/interfaces. observed.
- Rust: a trait object pointer is a data pointer plus a vtable pointer; the vtable holds size, alignment, a `drop_in_place` pointer, and the methods of the trait and its supertraits. Sources: https://doc.rust-lang.org/reference/types/trait-object.html and https://doc.rust-lang.org/std/ptr/struct.DynMetadata.html (nightly API). observed. Vtable pointer equality is unreliable because vtables are duplicated or merged across codegen units (DynMetadata docs). observed. Coercing `&dyn Trait` to `&dyn Supertrait` became stable in Rust 1.86.0, 2025-04-03: https://blog.rust-lang.org/2025/04/03/Rust-1.86.0/. observed.
- Swift: an opaque existential container is a three-pointer inline value buffer, a type metadata pointer, and one witness table pointer per protocol conformance; class-constrained existentials are just the object pointer plus witness tables. Source: https://github.com/swiftlang/swift/blob/main/docs/ABI/TypeLayout.rst. observed. Every type has a value witness table that says how to lay out and manipulate its values; generic code that is not specialised works on opaque layouts through metadata and witness tables; specialisation is described as an optimisation. Source: https://github.com/swiftlang/swift/blob/main/docs/ABIStabilityManifesto.md (undated). observed.

**Embedded vptr and C++ multiple inheritance.**
- Itanium C++ ABI: a virtual table holds virtual call offsets, virtual base offsets, an offset-to-top, a typeinfo pointer and the function pointers (section 2.5.2); a class with several bases has a virtual table group of a primary table plus secondary tables; a thunk is code "called instead of the target function for the purpose of modifying parameters" such as `this` (section 1.1); construction with virtual bases needs a table of table pointers, the VTT (section 2.6). Source: https://itanium-cxx-abi.github.io/cxx-abi/abi.html. observed.
- Reading: everything in that list beyond the function pointers exists because the table pointer lives inside the object and each base needs its own view of `this`. With a fat pair, the table is chosen per (shape, interface) and `this` is always the one data pointer, so none of it is needed for interface inheritance. inferred from the ABI text and the Go layout.

**Linearisation, explicit resolution, traits.**
- C3 (from "A Monotonic Superclass Linearization for Dylan", DOI 10.1145/236337.236343): `L[C(B1..BN)] = C + merge(L[B1]..L[BN], B1..BN)`; it guarantees local precedence order and monotonicity, and it refuses inconsistent hierarchies (the X,Y / Y,X example raises an error). Python adopted it in 2.3. Source: https://docs.python.org/3/howto/mro.html. observed. The paper's year (OOPSLA 1996) is assumed (model memory, unverified).
- Eiffel resolves by the programmer's statement, not by an order: `rename` for clashes ("Two different features of a class may not have the same final name"), `undefine` and join, and for repeated inheritance "a feature inherited multiply under one name will be shared ... under different names will be replicated", with `select` to settle dynamic binding. Source: https://www.eiffel.org/doc/eiffel/ET-_Inheritance. observed.
- Traits (Schärli, Ducasse, Nierstrasz, Black, ECOOP 2003): "A trait is essentially a group of pure methods"; traits "do not specify any state variables"; they have provided and required methods; composition "enjoys the flattening property", so `super` has no special meaning and a composed class equals the flat class with the same methods; conflicts are resolved explicitly, with aliases and exclusion. The paper opens by quoting "Multiple inheritance is good, but there is no good way to do it." Source: https://scg.unibe.ch/archive/papers/Scha03aTraits.pdf (`traits.txt` lines 1-40, 94-111, 375-381, 478). observed.

**Prototypes, maps, inline caches.**
- Self (Chambers, Ungar, Lee, OOPSLA 1989): maps group objects cloned from the same prototype and carry their layout; the compiler makes a separate version of each method per receiver map ("customized compilation"), which lets sends to self be bound and inlined. Source: https://bibliography.selflanguage.org/implementation.html. observed.
- Polymorphic inline caches (Hölzle, Chambers, Ungar, ECOOP 1991): a call site caches several (receiver type, target) results; median speedup 11%, and 27% when the recorded types also feed the recompiler. Source: https://bibliography.selflanguage.org/pics.html. observed.
- V8: a Map (hidden class) is the first word of every object and "describes the exact location of each property"; adding a property follows or creates a transition edge to another Map; descriptor arrays are shared along a transition chain. Source: https://v8.dev/docs/hidden-classes. observed.

**Generics and templates.**
- Go 1.18 "dictionaries and gcshape stenciling": one compiled body per GC shape (same underlying type, or any pointer), plus a dictionary passed as a hidden first argument holding the type arguments, derived types, sub-dictionaries and itabs; a method call on a type parameter goes through an itab from the dictionary; types that cannot be enumerated at compile time are rejected. Source: https://go.googlesource.com/proposal/+/refs/heads/master/design/generics-implementation-dictionaries-go1.18.md. observed.
- Go 1.27 (August 2026) adds generic methods, but "methods of interfaces may not declare type parameters nor can interface methods be implemented by generic methods". Source: https://go.dev/doc/go1.27. observed. Reading: the restriction follows from building itabs and dictionaries statically; a system that instantiates at run time by content hash does not have this wall. inferred.
- Zig: "Types are values that must be known at compile-time"; "a generic data structure is simply a function that returns a type"; reflection by `@typeInfo`; "no preprocessor, and no macros". Source: https://ziglang.org/learn/overview/. observed.
- BER MetaOCaml N153 (May 2025, for OCaml 5.3): typed quotation and splicing; a well-typed generator yields well-typed code; generated code can be compiled and linked into the running program. Source: https://okmij.org/ftp/ML/MetaOCaml.html. observed.
- Copy-and-patch (Xu and Kjolstad, OOPSLA 2021, arXiv 2011.13127): code generation by copying pre-compiled binary "stencils" with holes and patching the holes; about two orders of magnitude faster to compile than LLVM -O0 with code about 14% faster than LLVM -O0 on TPC-H; 4.9x to 6.5x faster compile than Chrome's Liftoff with 39-63% faster code. Source: https://arxiv.org/abs/2011.13127. observed. CPython's JIT uses it: templates are generated from the same DSL as the interpreter, LLVM is a build-time dependency only, the result was "about as fast as the existing specializing interpreter" with 10-20% more memory. Source: PEP 744, https://peps.python.org/pep-0744/ (created 2024-04-11, last modified 2025-02-01, still Draft). observed.
- Dictionary passing as the meaning of constrained polymorphism (Wadler and Blott 1989) and Haskell type classes: assumed (model memory, unverified).

### 2.2 Smallest known-sufficient set

All inferred from 2.1.

1. **Shape**: a description of a value's layout and size (Self map, V8 Map, Swift value witness table, Rust vtable header, Go type descriptor).
2. **Interface**: an ordered set of operation signatures.
3. **Witness**: the table that maps one interface onto one shape's recipes (Go itab, Rust vtable, Swift protocol witness table). It is a derived, cacheable fact about the pair, not part of either.
4. **Reference**: the pair (data, witness). One indirect call per send; no adjustment of the data pointer, ever.
5. **Parameterised recipe**: a recipe that takes shapes or witnesses as ordinary arguments (Zig's "types are values", Go's dictionary, Swift's hidden metadata). Its meaning is dictionary passing; specialising it for particular arguments is an optimisation that must not change meaning.

How this is "blueprint, recipe, cell" (inferred, with axiom 2 line 58 and axiom 4 line 78 as premises): the shape and the interface are blueprints; methods and generic functions are recipes; a cell is the pair (data, blueprint); a witness is the memoised answer to "does this blueprint offer that interface", keyed by the two content addresses. Axiom 4 says the interface "is itself a recipe or a blueprint of a recipe", which is the fat pair's second word exactly. Because the data word carries no table pointer, the data's bytes are the same whichever interface it is seen through, so its content address is stable. An embedded vptr would put an address inside the hashed content. That alone is a reason to prefer the fat pair in a content-addressed body.

Multiple inheritance then splits into two separate questions:

- Several interfaces on one shape: several witnesses. No layout problem.
- Several parents contributing state and code: compose into a new flat shape (a new blueprint, as axiom 3 says a new composition mints one) and require conflicts to be resolved in the text (traits' alias/exclusion, Eiffel's rename/select). C3 is the fallback when an implicit order is wanted, and it must be allowed to refuse.

### 2.3 Machine cost

| mechanism | per object | per reference | per call | per (type, interface) | source |
|---|---|---|---|---|---|
| fat pair | 0 words | 2 words | 2 loads + indirect call | one table, built lazily O(ni+nt), cached | Cox 2009 |
| Swift opaque existential | 0 | 5 words for one protocol (3 buffer + metadata + witness) | load + indirect call, value may be inline | one witness table | TypeLayout.rst |
| embedded vptr, single inheritance | 1 word | 1 word | 2 loads + indirect call | none | Itanium ABI (inferred count) |
| embedded vptr, multiple/virtual | 1 word per base with virtual functions | 1 word | as above plus a thunk or vcall-offset adjustment | secondary tables, VTT | Itanium ABI 2.5-2.6 |
| per-site inline cache | 0 | 1 word | compare + direct jump on hit | cache entries per site | PICs 1991 |
| shape-stenciled generic | 0 | n/a | dictionary load per type-dependent operation | one body per shape, one dictionary per instantiation | Go design doc |
| monomorphised generic | 0 | n/a | direct call | one body per instantiation | Zig overview (inferred) |
| stencil JIT | 0 | n/a | direct code | copy + patch per instantiation, far cheaper than an optimising compile | arXiv 2011.13127 |

### 2.4 Design recommendation

All inferred.

- R9. References are (data, witness) pairs. Data never contains a table pointer.
- R10. Witnesses are computed on demand and memoised by (shape id, interface id). With content addresses this cache is global, shareable and never invalid: a changed shape is a different id.
- R11. Conformance is structural and can be added after the fact (Go's model); a named conformance is only a cached proof.
- R12. Composition of several parents is flattening into a new shape with explicit conflict resolution. No subobjects, no pointer adjustment, no construction-time table juggling.
- R13. One mechanism for templates and generics: recipes that take blueprints as arguments, evaluated either by passing the dictionary (always valid, the slow but general path) or by specialising and caching machine code keyed by (recipe id, argument shape ids). Self's customisation and Go's shape stenciling are the two ends of the same dial; the body can move along it per recipe from observed heat.
- R14. Emit specialised code by stencil copy-and-patch first; reserve optimising compilation for what stays hot.
- R15. Dynamic shapes (objects that gain fields) use shape transitions: adding a field moves the cell to a successor shape, found by a transition table on the shape.

---

## 3. Content-addressed identity, reclamation, boundaries, messages

### 3.1 What each reference system actually has

**Identity by content.**
- Hash consing: a value is shared with any structurally equal existing value, so equality is pointer comparison in constant time; usually a hash table of weak references; Ershov 1958, Goto 1974, Filliâtre and Conchon 2006. Source: https://en.wikipedia.org/wiki/Hash_consing (last edited 2025-10-03). observed.
- Unison: each definition is identified by a 512-bit SHA3 hash of its syntax tree with named arguments made positional and dependencies replaced by their hashes; names are separate metadata, so a rename changes no hash; consequences listed are no builds, no dependency conflicts, typed durable storage, and moving computations between machines with missing dependencies synced on demand; the codebase is a database, not text files. Source: https://www.unison-lang.org/docs/the-big-idea/. observed. Unison 1.0 shipped November 2025 ("the language, distributed runtime, and developer workflow have stabilized"; "we never compile the same code twice"); a C FFI was still in progress. Source: https://www.unison-lang.org/unison-1-0. observed. 25 November 2025 as the exact day and a 1.1.0 release on 2026-01-29: observed-snippet.
- Git: "a content-addressable filesystem"; the key is the SHA-1 of a header (`type size\0`) plus the content; blobs, trees, commits. Source: https://git-scm.com/book/en/v2/Git-Internals-Git-Objects. observed.
- Merkle DAG: a node's identifier is the hash of its payload plus its children's identifiers; "any change in a node would alter its identifier and thus affect all the ascendants"; "every node ... is the root of a (sub)Merkle DAG itself"; they "can only be constructed from the leaves"; cycles cannot practically be created. Source: https://docs.ipfs.tech/concepts/merkle-dag/. observed.
- BLAKE3: one algorithm (hash, PRF, MAC, KDF, XOF), 256-bit default output, a Merkle tree internally, which gives parallelism, verified streaming and incremental update; "much faster" than SHA-1, SHA-2, SHA-3, BLAKE2 (no figures in the text). Source: https://github.com/BLAKE3-team/BLAKE3. observed.
- How Unison hashes mutually recursive definitions (as one unit): assumed (model memory, unverified).

**Reclamation ("the unreferenced composts").**
- Bacon, Cheng, Rajan, "A Unified Theory of Garbage Collection", OOPSLA 2004: tracing and reference counting "are in fact duals"; tracing works on live objects ("matter"), counting on dead ones ("anti-matter"); "all high-performance collectors ... are in fact hybrids". Source: PDF copy at https://web.eecs.umich.edu/~weimerw/2012-4610/reading/bacon-garbage.pdf (`bacon.txt` abstract). observed. The year and venue are assumed (not on the page I read).
- Perceus (Reinking, Xie, de Moura, Leijen, PLDI 2021; MSR-TR-2020-42): precise reference counting that makes programs "garbage free" (only live references are retained), which enables reuse analysis and guaranteed in-place update, the "functional but in-place" style; starts from a functional core with explicit control flow. Source: https://www.microsoft.com/en-us/research/publication/perceus-garbage-free-reference-counting-with-reuse/. observed.
- Lean 4's scheme (Ullrich and de Moura, IFL 2019, arXiv 1908.05647): reference counting for a pure language, reuse of non-shared values, borrowed references inferred to cut count updates. Source: https://arxiv.org/abs/1908.05647. observed.
- LXR (Zhao, Blackburn, McKinley, PLDI 2022, arXiv 2210.17175): reference counting on the Immix heap, tracing only for cycles; on Lucene about 6x the throughput and 30x lower 99.9th-percentile latency than Shenandoah. Source: https://arxiv.org/abs/2210.17175. observed.
- Regions: MLKit 4.7.17 (2025-12-16) infers allocation and deallocation at compile time and "integrates reference-tracing garbage collection with region-based memory management"; the collector can be switched off. Source: https://elsman.com/mlkit/. observed. Tofte and Talpin 1997 as the origin: assumed.
- Generational collection (Ungar 1984): assumed (model memory, unverified).

**Boundaries and capabilities.**
- seL4 (whitepaper rev. 1.4.1, 2026-07-17): "a capability is an object reference ... (implementation of capabilities are often referred as 'fat pointers')"; it is "an encapsulation of an object reference and the rights it conveys"; "invoking a capability is the one and only way of performing an operation on a system object"; the kernel is about 10 kSLOC against Linux's roughly 20 MSLOC; "the world's fastest microkernel". Source: https://sel4.systems/About/seL4-whitepaper.pdf (`sel4.txt` lines 170, 240-250, 850-860). observed.
- CHERI: architectural capabilities replace integer addresses; tagged memory protects them; strong monotonicity (a capability cannot be made more permissive); supports compartmentalisation; ISA version 9 is UCAM-CL-TR-987, September 2023; CHERI-RISC-V is the reference platform, Arm Morello the prototype; UK-funded projects since 2024 aim to bring CHERI-RISC-V to market; a CHERI Alliance exists. Sources: https://www.cl.cam.ac.uk/research/security/ctsrd/cheri/ and https://www.cl.cam.ac.uk/techreports/UCAM-CL-TR-987.html. observed. The 128-bit-plus-tag capability width: assumed (model memory, unverified).
- Pony reference capabilities: `iso` (the only reference; may be handed to another actor), `val` (immutable; may be shared), `ref` (mutable, one actor), `box` (read-only view), `trn` (writable, becoming `val`), `tag` (identity only: compare and send to, never read or write). "By sharing only immutable data and exchanging only isolated data we can have safe concurrent programs without locks." Source: https://tutorial.ponylang.io/reference-capabilities/reference-capabilities.html. observed.
- E and the object-capability model: Miller, *Robust Composition*, Johns Hopkins, 2006: how separately written programs can cooperate without destructively interfering, extended to concurrent and potentially malicious components; presents E, "a distributed, persistent, secure programming language". Source: https://papers.agoric.com/papers/robust-composition/abstract/. observed. Eventual sends, promises, vats and promise pipelining as E's mechanisms: assumed (model memory, unverified).
- WebAssembly component model: a component interacts "only through its imports and exports", may not export a memory, and so cannot communicate through shared memory; interfaces are written in WIT and lowered by a canonical ABI. Source: https://component-model.bytecodealliance.org/design/why-component-model.html. observed. WASI 0.3.0 was ratified and announced 2026-06-11: `async func`, `stream<T>` and `future<T>` are first-class in the canonical ABI, `wasi:io` is removed, and the host runs one event loop shared by all components where 0.2 had one per component. Source: https://bytecodealliance.org/articles/WASI-0.3. observed. The component model itself is listed at phase 1 at https://github.com/WebAssembly/proposals. observed.

**Messages and actors.**
- Kay, email of 2003-07-23: "OOP to me means only messaging, local retention and protection and hiding of state-process, and extreme late-binding of all things"; objects were conceived like biological cells "only able to communicate with messages". Source: https://userpage.fu-berlin.de/~ram/pub/pub_jf47ht81Ht/doc_kay_oop_en. observed.
- Actor model (Hewitt, Bishop, Steiger 1973): on a message an actor can send messages, create actors, and designate the behaviour for its next message; it can only address actors it was told about, already knew, or created. Source: https://en.wikipedia.org/wiki/Actor_model. observed.
- Erlang (OTP 29.1.2 docs): processes are lightweight; send (`!`) to a pid never fails, even if the process is gone; `receive` scans the queue for the first message matching a clause (cost O(N) in queue length); `receive ... after T` yields its timeout body if nothing matches in T milliseconds, with `0` and `infinity` as the two ends; links are bidirectional, monitors one-way. Sources: https://www.erlang.org/doc/system/ref_man_processes.html and https://www.erlang.org/doc/system/expressions.html. observed.

### 3.2 Smallest known-sufficient set

All inferred from 3.1 and the axioms.

1. **A canonical numeric form and one hash function.** Identity = hash(canonical form with names removed and children replaced by their identities). Unison is the working proof for code; Git and IPFS for data.
2. **A separate name table.** Names map to identities, zero or many per identity. This is axiom 3's "names are free query keys" (line 74) and is the same split as NUMS's replaceable symbol lens, as the mission text describes it.
3. **Reference counts on the DAG.** A Merkle DAG has no cycles, so counting alone finds everything unreferenced; no tracing pass is required for the frozen part of the body. This follows from the IPFS property (built from leaves, no cycles) plus Bacon's duality (counting handles everything except cycles). Mutable named cells are the roots.
4. **Unforgeable references that carry their interface.** The (data, witness) pair of section 2 is already a capability if (a) it cannot be fabricated from numbers and (b) the witness lists only what was offered. Narrowing the witness is attenuation; wrapping it is a membrane.
5. **One act: offer.** `offer(recipe-or-blueprint, args) → nothing | 0 | 1 | node`. A local call, a send to another cell, and a request across a channel differ only in how long the acknowledgment may take and whether the caller waits.

### 3.3 Machine cost

| mechanism | cost | source |
|---|---|---|
| identity of a frozen node | one hash over its canonical bytes; children contribute fixed-size ids, so cost is proportional to the node, not the subtree | Merkle DAG docs (inferred) |
| equality of two frozen nodes | compare ids (or pointers under hash consing), constant time | hash consing article |
| rename | zero rehash | Unison big idea |
| edit of one node | rehash it and every ancestor up to each root that should see it | "affect all the ascendants" (IPFS docs) |
| snapshot / undo of a persistent structure | keep or repoint one id | axiom 3 line 71; Clojure |
| update of a persistent map | copy about log32 N nodes | Clojure reference |
| reclaiming | count updates on reference creation and drop; reuse in place when the count is 1 | Perceus abstract |
| capability check, software | one validation per invocation at the boundary | seL4 whitepaper |
| capability check, hardware | per-pointer tag and bounds metadata checked by the processor | CHERI page (cost figures not fetched) |
| message send | enqueue; receive is a scan, O(queue length) for selective receive | Erlang expressions |

One caution, inferred: hashing every intermediate value at run time would be ruinous. Hash consing in memory is a hash-table probe per allocation and pays off only for immutable, frequently compared structure. Identity should be computed when a node freezes, not while it is being worked on.

### 3.4 Design recommendation

All inferred.

- R16. Three representations for the three node states, with identity computed only at the solid end. Frozen (the body calls this ice; `AGENTS.md` lines 575 and 585 of the current worktree use "ice" for the persisted compiled artifact and "gas" for what a movement leaves unfrozen, observed; a precise definition of the three states was not found in the files I read, so the mapping here is assumed): immutable, content-addressed, reference-counted, shareable across cells and machines. Live: mutable, owned by one cell, addressed by location, protected by the value trail of section 1; it gets an identity when it is frozen or offered across a boundary. Transient: in registers and on stacks, never named, gone when the movement ends.
- R17. Canonical form is the numeric space itself: a node is (kind number, child ids, literal bytes). Symbols live in name tables per language. Hashing must be defined on this form and nothing else, so that every surface language and every natural-language lens of the same structure gets the same id.
- R18. Use a tree hash (BLAKE3-class) so that large leaves hash in parallel and incrementally. The choice of function is an assumption until measured on the target machines.
- R19. Reclaim frozen nodes by precise reference counting with reuse (Perceus), which also gives in-place update of uniquely held structure without giving up immutability semantics. Use regions for transient allocation inside one movement (reset at the delimiter that also bounds backtracking: the allocation mark in a choice-point frame is already a region).
- R20. References across a cell boundary are capabilities: (data id or handle, witness). Three sharing modes cover what Pony proves safe: frozen (share freely), unique live (move, sender loses it), opaque handle (may be compared and offered to, never read). The third is axiom 4's "opaque interface" made concrete.
- R21. No ambient authority: a cell reaches only what it holds a reference to. A crossing that did not use an offered witness must fail at a check and leave a record, which is what "breach is observable" (axiom 4) requires. In software this is a check at the boundary recipe; on CHERI-class hardware it can be the processor.
- R22. One offer primitive. Inside a cell with a det recipe it compiles to a direct call. Across cells it is an enqueue plus a future; the acknowledgment arrives as nothing (timeout or held), 0, 1 or a node. Streams and futures in the boundary ABI (as WASI 0.3 now does) rather than per-cell event loops.

---

## 4. One table: the axioms against the machinery

All inferred. Premises are the axiom lines and the sections above.

| axiom | semantic machinery | machine structure |
|---|---|---|
| 1 states: 0, 1, nothing; timeout is nothing; choosing nothing is first-class | results are zero / one / many / not-yet; fail versus stuck (Verse 2.2); `receive ... after` | a status in the return path separate from the value; the choice stack reacts to 0, the scheduler to nothing |
| 2 cell: blueprint, recipe, child cells | shape, parameterised recipe, (data, witness) pair; flattening composition | records without embedded table pointers; witnesses memoised |
| 3 content addressing; unreferenced composts; undo is repoint | hash of canonical numeric form; names apart; persistent structures | Merkle DAG, reference counts, snapshot by id |
| 4 boundary: only through an offered interface; breach observable | capability = reference + rights; attenuation; shared-nothing components | fat pair as capability; boundary check; optional hardware tags |
| 5 offer: run and speak are one act; ack is nothing, 0, 1 or node | message send = call; handler-delimited operations; futures | direct call when det and local; enqueue plus one-shot suspension otherwise |

The three areas turn out to share structures: the witness of section 2 is the capability of section 3; the allocation mark of a choice point (section 1) is a region (section 3); a store snapshot (section 1) is an id (section 3); tabling (section 1) and specialisation caching (section 2) are both memo tables keyed by content addresses (section 3).

---

## 5. Developments through 2026 that I checked

- Go 1.27, August 2026: generic methods, still not on interfaces. https://go.dev/doc/go1.27. observed.
- WASI 0.3.0, 2026-06-11: async, streams and futures in the canonical ABI. https://bytecodealliance.org/articles/WASI-0.3. observed. WASI 0.3.1 dated 2026-08-06 and Wasmtime 46 enabling it by default: observed-snippet.
- WebAssembly stack switching at phase 3; JS Promise Integration standardised (phase 5); component model at phase 1. https://github.com/WebAssembly/proposals. observed.
- Unison 1.0, November 2025. observed.
- Rust 1.86, 2025-04-03: trait object upcasting stable. observed.
- Koka v3.2.9, 2026-09-17, still labelled research. observed.
- OCaml 5.3: handler syntax for effects; continuations remain one-shot. observed.
- BER MetaOCaml N153, May 2025. observed.
- ICFP 2025 multi-shot lexical handlers (Effekt group) and OOPSLA 2025 tracing JIT for handlers. observed-snippet.
- APLAS 2025 expressiveness result for one-shot control. observed.
- seL4 whitepaper revised 2026-07-17. observed.
- Erlang/OTP 29.1.2 documentation current. observed.
- CPython JIT: PEP 744 still Draft and last modified 2025-02-01; status in 3.14/3.15 not checked. observed for the PEP, open for the rest.
- Verse: nothing newer than the 2023 paper and the 2024 blog found. open.

---

## 6. Limits of this study

- Costs in the tables are mostly counts of words and loads derived from published layouts, not measurements on the target hardware. Every "inferred" row should be measured before it decides a design.
- The WAM frame comes from a third-party copy of the tutorial; the cut mechanism itself was not read.
- Several abstracts were read without the paper body (Perceus, evidence passing, LXR, Counting Immutable Beans, copy-and-patch, composable memory transactions). Claims are limited to what the abstracts say.
- I did not study CLOS method combination, Datalog/tabling engines, Scala or Haskell type-class implementations, Java's virtual threads, or Nix's content-addressed store. They are absent, not rejected.
- The mapping of ice, water, gas to representations (R16) rests on two lines of `AGENTS.md` and my reading; it needs the body's own definition.
