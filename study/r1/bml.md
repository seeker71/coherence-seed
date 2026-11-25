# Study: BML (Backtracking Model Languages) as the higher grammar

Study date: 2026-11-24. Kernel worktree head: `b8ff978c9` (2026-11-24).
Nothing in any repository was modified. Nothing was executed: `fkwu` is not built in this worktree
(`ls fkwu` answered "No such file or directory"), and I did not run the main checkout's binary because a run
writes cache files beside the sources it reads. Every "observed" below therefore means **read in this session**,
never "ran".

## 0. Legend, paths, and what was read

Status words:

- **observed** — I read the cited lines myself in this session.
- **inferred** — my reasoning from observed lines; the lines are named.
- **assumed** — not verified; said so.

Path abbreviations (all absolute):

- `T` = `Coherence-Network/docs/field/urs/artifacts/master-thesis-2000`
- `TT` = `T/backtracking-model-languages.txt` (the 2000 thesis text; footer date "July 5, 2000", `TT:1072`)
- `K` = `coherence-kernel`
- `A` = `~/Downloads/Angelic` (the 2000 source archive on local disk, outside any repo)
- `M` = `memory`

Read in full: `TT` (1081 lines), `T/INDEX.md`, `T/README.md`, `T/EXTERNAL.md`, all of `T/companion/`
(`angelic-assembler.txt`, `bml-search-algorithms.txt`, `sgb-bml-objects.txt`,
`sgb-thesis-another-object-model.txt`, the five source samples), `K/form/form-stdlib/bml.fk`,
`bmf-core.fk`, `bmf-mini.fk`, `bml-floor-compile.fk`, `bml-demand-jit.fk`, `bml/bml-lang.bml`,
`K/control/offer-ack-core.fk`, `K/control/choice-lane-core.fk`, `K/docs/native-bml-admission.md`,
`K/docs/bml-direct-nodes-jit.md`, `K/grammars/bml-native-north-star.form`,
`K/grammars/control-invite-grammar.fk`, `K/grammars/host-effect-vocabulary.bmf`,
`K/teachings/surprise-is-a-choice-point.md`, the four memory notes.

Read in part (outline by `grep` plus the cited line ranges; stated so the reader knows the limit):
`K/form/form-stdlib/bmf-grammar.fk` (1138 lines; header, manifest, `gx-seq/alts/rep/sep`, builders),
`K/form/form-stdlib/bml-source.fk` (706 lines; definition outline),
`K/form/form-stdlib/bml/bml-form-lower.bml` (849 lines; comment narrative and lines 1-152, 524-730),
`K/form/form-stdlib/grammars/bml.fk` (4400 lines, 490 KB; comment narrative and about 200 cited lines),
`K/form/form-stdlib/grammars/form-bml.fk` (334 lines; 1-140 and the rule index),
`K/form/form-stdlib/form-bml-lower.fk` (lines 100-135, 380-406), `K/form/form-stdlib/source-compiler.fk`
(lines 681-696), `K/form/form-stdlib/engine.fk` (28-67), `K/form/form-stdlib/compiler.fk` (1-40, 205-262),
`K/form/form-stdlib/bml/blueprint-literal.bml` (1-60), `K/form/form-stdlib/bml/anf-lower.bml` (1-45),
`K/form/form-stdlib/bml/bml-demand-jit.bml` (1-30), `K/runtime/fkwu-uni.c` (358-370, 1994-2043, grep),
`K/docs/native-bml-execution-learning.md` (1-90), `K/docs/form-native-coding.md` (grep for BML only; this
file is about the native coding controller, not about BML's concepts), `K/axioms/core-axioms.form` (1-140),
five fixtures in `K/form/form-stdlib/tests/fixtures/`, and four files of the 2000 archive
(`A/src/Test/Choice.bml`, `A/src/BMF/primitive/Fail.bml`, `MultiMatch.bml`,
`A/src/BMF/container/Branch.bml`, `Repeat.bml`).

Source dates (git last-commit date of the file, `git log -1 --date=short`, read 2026-11-24):
`bml.fk` 2026-09-03; `grammars/bml.fk` 2026-11-19; `grammars/form-bml.fk` 2026-11-20;
`form-bml-lower.fk`, `bmf-core.fk`, `bmf-grammar.fk` 2026-11-03; `bml/bml-form-lower.bml` 2026-10-11;
`bmf-mini.fk` 2026-10-03; `bml-source.fk` 2026-11-14; `bml-floor-compile.fk` 2026-10-11;
`bml-demand-jit.fk` 2026-09-30; `bml/bml-lang.bml` 2026-09-29; `bml/blueprint-literal.bml` 2026-11-03;
`bml/anf-lower.bml` 2026-10-04; `control/offer-ack-core.fk`, `control/choice-lane-core.fk` 2026-09-30;
`docs/native-bml-admission.md` 2026-10-04; `docs/bml-direct-nodes-jit.md` 2026-11-13;
`grammars/bml-native-north-star.form` 2026-11-04; `engine.fk` 2026-09-29; `source-compiler.fk` 2026-11-14;
`compiler.fk` 2026-10-04; fixtures `bml-choose-cut.bml`, `bml-choose-fields.bml` 2026-09-30,
`bml-class-templates.bml`, `bml-snapshots.bml` 2026-09-15; `axioms/core-axioms.form` 2026-11-19.
2000 sources: `TT` July 5, 2000; `sgb-bml-objects.txt` 2000 (no day given); `bml-search-algorithms.txt`
sources dated 6-Dec-99 (`:281`); source samples 11-Apr-2000; `A/src/Test/Choice.bml` file mtime May 30, 2000;
`A/src/BMF/container/Branch.bml`, `Repeat.bml` mtime Apr 20, 2000.

---

## 1. First finding: "BML" names four different things

A unified grammar has to decide which of these it is unifying. They do not have the same semantics.

### 1.1 The 2000 system (five technologies, two authors)

- **BMF** — "Backtracking Model Form", a top-down, context-sensitive, backtracking parser definition language and
  compiler-construction tool (`TT:6`, `TT:76`, `TT:126`). observed.
- **BML** — the language: "extension from C++, Java, Smalltalk, and Prolog", with "classes, interfaces,
  templates, delegation, exception handling, and backtracking using its own virtual machine" (`TT:90`). observed.
- **BMA** — the abstract machine (instruction set), **BMO** — the object model; both are the second author's
  work (`T/companion/bml-search-algorithms.txt:34-37`; `TT:6` says the object model and the VM were "designed and
  implemented by Steve G. Bjorg"). observed.
- The author's own thesis text is **incomplete as a specification**: the sections Overloading, Templates,
  Exception Handling carry almost no body (`TT:748-764`), Compatibility is empty (`TT:765`), the Conclusion is
  three empty headings (`TT:1018-1023`), and about twenty attributes are marked "[Not supported yet]"
  (`TT:651-685`). observed.
- BML 2000 has **no unification**: "BML does not include a mechanism for unification. Consequently, it does not
  support queries where parameters are unbound" (`bml-search-algorithms.txt:246`). observed.

### 1.2 The 2026 body carries three separate BML readers

| # | Reader | Files | What it is | Status |
|---|---|---|---|---|
| a | "Java-ish" cursor grammar | `K/form/form-stdlib/bml.fk` (135 lines) | Parses class/interface/method/statement text into universal `UE-*` nodes. Types, modifiers, bases and casts are "parsed; incidental type/modifier detail is erased" (`bml.fk:18-20`); bases lower to the constant `"bases"` (`:42`), props to `"props"` (`:45`); **`fail;` emits `UE-THROW`**, the same node as `throw` (`:71-72`); it has no `choose`/`choice`/`cut` rule at all (rule list `:66`). | observed |
| b | Thesis-BML native compiler | `K/form/form-stdlib/grammars/bml.fk` (4400 lines) + `K/form/form-stdlib/bml/bml-form-lower.bml` | A second, much larger compiler that reads the thesis grammar (its own `section [bml.bmf]` rulebook, `grammars/bml.fk:937-1012`), builds a class layout (bases, delegates, interfaces, templates, flags) and lowers to Form `defn` text that `fkwu` runs (`bml-form-lower.bml:1-13`). This is where choose/fail/cut/save/restore with a field journal, classes, delegation and template specialization live. | observed |
| c | `form.bml` executable dialect | `K/form/form-stdlib/grammars/form-bml.fk` + `form-bml-lower.fk` + `source-compiler.fk` | The surface almost all of the 2026 body is written in: `def`, `let`, lambdas, `|>`, `??`, `match`, `try … catch`, `choice { }`, `defer`, `class` as a grouping, `template`, `blueprint`. Here `fail` and `stop` are **the value `nothing`** (`form-bml.fk:52`), `store`/`restore` are identity functions and `undo` is a three-way select (`choice-lane-core.fk:16-18`). | observed |

The memory note says the same of (a) versus (c): "The grammar fkwu really uses for `.bml` is
`form/form-stdlib/grammars/form-bml.fk` … `form/form-stdlib/bml.fk` is an older Java-ish grammar that `bml-band`
still reads" (`M/reference-bml-grammar-and-no-run-lower-check.md:11`, dated 2026-10-02; 53 days old, still
consistent with what I read). observed.

**Consequence (inferred from the three rows):** the 2026 body did not have "Form plus one BML". It had Form
s-expressions, two BML dialects with different meanings for the same words (`fail`, `choice`, `cut`, `class`,
`template`), and a third reader that erases most of it. "One grammar, one dialect" resolves a real, present split.

---

## 2. Concept inventory

For each concept: the 2000 definition, the 2026 state, one real example with its file, and the status.

### 2.1 choice / choose

**2000 (thesis).** `Choice ::= "choice" CodeBlock \\ ","` — "one of the two backtracking statements in BML. It
specifies two or more independent choices the program can take. If the first code block fails, the second code
block will be executed, and so on. If the choice statement is used in a loop statement, each loop can take an
independent path." (`TT:625-626`). Translation to the abstract machine: `jmpf Label2; Statements1; :Label2;
jmpf Label3; Statements2; :Label3; Statements3;` (`TT:927-932`). `choice` is a reserved word (`TT:245`); the
second author's paper and the 2000 sources spell it `choose` (`bml-search-algorithms.txt:49-54`). observed.

**2000 semantics are re-entrant (chronological).** A failure raised *after* the choose statement has completed
returns into the choose and runs its next branch. Three independent observations:

1. `if_fail { … } stmt` is defined as "equivalent to" `choose {}, { … } stmt` — the failing statement stands
   *after* the choose (`bml-search-algorithms.txt:58-66`).
2. The 2000 test `A/src/Test/Choice.bml` (16 lines, mtime May 30, 2000):
   ```
   choose { .PrintLn( "First choice" ); }, { .PrintLn( "Second choice" ); }, { .PrintLn( "Final choice" ); return; }
   }
   fail;
   ```
   The `fail;` is outside the choose; the program only makes sense if it re-enters the second and third branch.
3. `ChoiceSet<T>.Pick()` "returns non-deterministically an element … When a failure occurs, the Pick method
   continues to return another element until every element of the set has been returned once"
   (`bml-search-algorithms.txt:1007-1008`), used as `return Explore( ChoiceSet<State>( hState.Expand()).Pick());`
   (`:686`) where the failure happens deep inside the recursion, after `Pick` returned.

observed (all three); that this is the general rule of the 2000 VM is **inferred** from them plus the DO/UNDO
machine mode (2.5).

**2026, reader (b).** `choice { … } , { … }` (and `choose`) are read by the thesis rulebook
(`grammars/bml.fk:998`). The lowering is **scoped, not re-entrant**: "A choose whose branches assign or fail
partway runs each branch from the names it was entered with, FAILED clear: the first branch that ends unfailed
hands back the incoming flag and answers, and a branch that failed undoes the field sets the journal kept since
the choose opened it … If every branch fails the choose fails and its body stops at the value it stood on"
(`bml-form-lower.bml:525-531`; same text `grammars/bml.fk:2072-2082`). A failure after a completed choose travels
*outward* to the enclosing choose or ends the program (`grammars/bml.fk:1981-1988`). Example, the fixture
`K/form/form-stdlib/tests/fixtures/bml-choose-cut.bml:23`:
```
int Guard() { int r = 0; choice { r = After(); } , { r = 100; } return r; }
```
observed. `if_fail` and `while_success`, which need re-entry, are parsed (`grammars/bml.fk:1003-1004`) but the
only executable emitters I found are two narrow fixed shapes (`:1005-1006`,
`if-fail-fail-return-int`, `while-success-fail-return-int`); that the general forms do not run is **inferred**
from those two lines and from the scoped lowering — I did not find a general lowering and did not run one.

**2026, reader (c).** `choice { e1; e2; … }` "attempts options in order and returns the first non-absent
answer, including zero" (`docs/native-bml-admission.md:105-107`). It is an **expression** over values; lowering:
"every option of a choose is attempted in order; none landing is nothing" —
`(attempt e1)`, test `nothing?`, else the rest (`source-compiler.fk:691-696`). The library form is one line:
```
def oac-choice(alts,args) = nil?(alts) ? fail : try alts[0](args) catch oac-choice(alts[1:],args);
```
(`K/control/offer-ack-core.fk:35`). No state is restored, because nothing was mutated. The live grammar reads
only the word `choice` (`grammars/form-bml.fk:269`); the admission doc's "(also `choose`)" (`:105`) and the
north star's "one choice spelling" (`bml-native-north-star.form:16`) disagree, and the grammar agrees with the
north star. observed.

### 2.2 fail

**2000.** "`Fail ::= "fail";` … the other backtracking statement in BML. It will force the virtual machine to
change its state to fail, and start backtracking, until another path can be found. This will typically be a
choice statement." (`TT:633-634`). It compiles to one instruction, `fail` (`TT:959-964`; "fail — set failure
mode", `angelic-assembler.txt:141`). In BMF, `Fail` is a primitive whose whole body is the statement:
`void Match( Context hContext, Environment hEnv, String strTag ) { fail; }` (`A/src/BMF/primitive/Fail.bml`).
A `loop` may be left by `fail` (`TT:616`). observed. What a program answers when no path is left is **not
defined** in the thesis (2026 comment says so: `K/form/form-stdlib/bml/bml-lang.bml:13-15`). observed.

**2026 (b).** `fail;` sets the run's FAILED cell; "the statements after it stand down"
(`bml-form-lower.bml:525`, `:536`). An unheld fail "ends the program, and the program answers the value of its
last completed statement" (`bml-lang.bml:9-17`; `bml-native-north-star.form:20`). This is a 2026 decision filling
the thesis's gap. observed.

**2026 (c).** `fail`, `stop`, `fail()`, `stop()` are all spellings of canonical `nothing`: "An absence signal
interpreted by its receiving boundary; no imperative abort" (`docs/native-bml-admission.md:173`;
`grammars/form-bml.fk:52`). "production code writes `nothing()`" (`native-bml-admission.md:442`). Example:
`def first-reading(primary,secondary) = choice { primary(); secondary(); fail };` (`:114`). observed.

**2026 (a).** `fail;` → `UE-THROW` with no children (`bml.fk:71`). observed. This reader cannot tell a failure
from an exception.

### 2.3 stop

**2000.** There is no `stop` statement or keyword (`TT:245` keyword list). The word appears only inside Cut:
executed backwards, "an error is reported and the execution undoes anything done so far, and stops after that"
(`TT:161`). The machine has `halt` (`angelic-assembler.txt:137`). observed. So "stop" as a concept is a **2026
addition**.

**2026.** Three layers:

- Kernel: "fk_stop — a STOP is a state a Form expression cannot continue past (a length of nothing, a branch on
  nothing, arithmetic over a non-number), never corruption. It is a backtrack point: inside (attempt x) it
  unwinds to the attempt … and answers nothing … Outside every attempt the program itself was the option … it
  ends" (`K/runtime/fkwu-uni.c:362-367`). The recover point is `_setjmp`/`_longjmp` (`:1996-2008`). observed.
- Dialect (c): `stop` is `nothing` (above). `oac-stop() = stop`; `oac-run-chain` ends a chain on a stopped ack
  (`offer-ack-core.fk:29-34`). `fail` and `stop` are therefore **the same value**; only the reader distinguishes
  "did not land" from "make no further offer" (`teachings/surprise-is-a-choice-point.md:33`). observed.
- Token level: `<STOP>` is one of eight control invites a model stream may carry
  (`K/grammars/control-invite-grammar.fk:5`, `:36`). observed.

### 2.4 cut (and mark)

**2000 (BMF primitive).** "If a cut is executed forward, nothing happens. However, if a cut is executed
backwards, then an error is reported and the execution undoes anything done so far, and stops after that."
(`TT:160-161`). The source: `void Match( … ) { System.Cut( hContext.Argument( 0 )); }`
(`T/companion/source-samples/primitive-Cut.bml:19`) — it calls into the VM, with an argument. "Worst-case
NP-complete → bad language definitions are very slow but there is a cut mechanism" (`TT:1017`). observed.

**2000 (statement, second author).** "The cut statement disable retries when it is encountered during
backtracking. Consequently, all preceding statements are undone as if no alternative existed. This operation
continues until a corresponding mark statement is found. The mark statement enables again the retry operations."
(`bml-search-algorithms.txt:77-78`). `cut` and `mark` are **not** in the thesis keyword list (`TT:245`). observed.
So 2000 has two descriptions (cut = fatal error barrier; cut…mark = retry-disabled span); how they reconcile is
not stated in what I read. open.

**2026 (b).** "A cut commits the choose around it to this branch: a failure after it in the branch fails the
whole choose, one before it still falls through, and its scope is its own choose … Outside any choose it commits
nothing." (`bml-form-lower.bml:568-573`). The fixture pins all three cases
(`tests/fixtures/bml-choose-cut.bml:20-24`): `Before` 2, `Kept` 10, `After` fails and `Guard` answers 100; total
112. This is Prolog-style cut scoped to one choose, not the thesis's error barrier. `mark` survives as a synonym
of `save` (2.6). observed.

**2026 (grammar engine).** `cut` is a first-class word in BMF rule bodies: `package ::= "package" spc cut
package-name ";"` (`grammars/bml.fk:942`); "Once a sequence crosses a `cut`, a subsequent fail becomes a
`fail-cut` that aborts the enclosing choice's remaining alternatives" (`engine.fk:35-42`, with `mk-fail-cut`
`:46`). The newer cursor engine (`bmf-grammar.fk`) has no cut pattern; it prunes with precomputed first-byte sets
(`bmf-grammar.fk:7-11`) and ordered alternatives (`:817-820`). observed.

**2026 (c).** `cut(alternatives,args)` = "Offer only the first alternative":
`def oac-cut(alts,args) = nil?(alts) ? nothing : oac-offer(alts[0],args);` (`choice-lane-core.fk:8`), with a
receipt that counts the pruned lanes (`:9-12`). observed. Same word, a third meaning.

### 2.5 undo

**2000.** Undo is not a statement; it is the **execution mode of the machine**. "Choosing a backtracking virtual
machine solves the problem of undoing already computed attributes and computations while searching for another
path" (`TT:84`); "When the parser backs out, all the attributes already computed have to be undone as well"
(`TT:76`). The VM state has a mode byte printed as `"UNDO"` or `"  DO"`
(`T/companion/source-samples/bmcpu-main.cpp:32`). A native method carries "three pointers to native functions: a
do function, a redo function, and an undo function" (`sgb-bml-objects.txt:634`). Undo reaches object lifetime:
"backtracking can ask to undo the destruction and anytime" (`TT:743`). The opt-out is the `nostate` attribute:
"operations done on this variable are not un-done during backtracking. [Not supported yet]" (`TT:667`), used in
the companion as `double dNewCostLimit [nostate];` (`bml-search-algorithms.txt:697`) and on the error stream so
"a program [can] output a message before it executes the fail statement" (`:1014`). observed.

**2026 (b).** Undo is a **journal of field sets**, open only while a snapshot is open: "While a snapshot is open
(DEPTH above 0) a field set first journals the record, the key and the value it replaces; a rollback undoes the
journal, newest first, back to the count its snapshot took" (`grammars/bml.fk:2084-2087`). The emitted runtime is
three small Form defns: `bfk-jot` (journal an entry), `bfk-undo` (pop n entries, `record_set` each old value),
state cells `j c d` (`bml-form-lower.bml:715-718`). Locals need no journal: each assignment is a fresh Form name
and a branch restarts "from the names it was entered with" (`:4-5`, `:526`). Example:
`int Roll() { choice { this.v = 5; fail; } , { this.v = this.v + 1; } return this.v; }`
(`tests/fixtures/bml-choose-fields.bml:12`). observed. Host effects (print, file write) are not journaled in what
I read — inferred from `bfk-jot` being called only from field stores (`bml-form-lower.bml:618`, `:621`).

**2026 (c).** `undo(ack, checkpoint, memory)` "is the checkpoint when `ack` is absent and `memory` otherwise";
it "evaluates all three arguments first"; "Host effects are not rolled back" (`native-bml-admission.md:176`,
`:375-379`; `choice-lane-core.fk:18`). observed.

**Axiom tie.** Axiom 3 already states the reduced form: "undo-is-repoint a-name-at-a-still-referenced-blueprint"
(`K/axioms/core-axioms.form:71`), theorem "reversibility … undo = re-point a name at a still-referenced
blueprint" (`:116`). The BMF cursor is the working example: "every advance returns a NEW cursor. A checkpoint is
just an earlier cursor value; restore is returning to it. Backtracking leaves no sediment because nothing was
mutated" (`bmf-core.fk:19-21`, `:295-297`). observed.

### 2.6 store / restore (save, mark, restore, discard)

**2000.** The machine has a State group: "save — push n bits; restore — pop n bits; discard — drop n bits"
(`angelic-assembler.txt:173-176`). The thesis has no save/restore statement; the companion has `mark`
(2.4). `with` saves and restores a register by hand: `push REG15; Expression -> REG15; Statements; pop REG15`
(`TT:984-989`). observed.

**2026 (b).** Statements `save;` / `mark;` / `restore;` / `discard;`: "The state is the mutable locals and the
value the statement list stands on … the frame keeps a stack of snapshots under __marks: save pushes the live
slots and __top, restore writes them back and pops, discard pops. A field set between save and restore is undone
too" (`grammars/bml.fk:1941-1948`). Example: `int Kept() { int x = 1; mark; x = 5; discard; return x; }`
(`tests/fixtures/bml-snapshots.bml:11`); nested snapshots stack (`:12`). A choose takes "the snapshot save takes"
(`grammars/bml.fk:2073-2074`) — so in this reader **choose is built on save/restore**. observed.

**2026 (c).** `store(memory)` and `restore(saved)` "are the identity, so a checkpoint is a value the caller holds"
(`native-bml-admission.md:367-368`; `choice-lane-core.fk:16-17`). observed.

### 2.7 timeout

**2000.** No timeout. The nearest thing is the BMF primitive **Multi match**: "The first argument is the element
to match, and the second argument to maximum number of retries … you can stop the backtracking process and fail
after it reached the specified maximum number of retries" (`TT:168-169`). The 2000 source of it only forwards the
match (`A/src/BMF/primitive/MultiMatch.bml`: `((ParseItem) hContext.Argument( 0 )).Match( … )`) — the retry bound
is not visible in the BML source I read; whether the VM enforced it is unknown. The companion's "A thread with a
non-zero DF (i.e. degree of freedom) is executed until a zero DF is reached again. No other threads are executed
during this speculation phase" (`angelic-assembler.txt:4`) means speculation was *not* preemptible. observed.

**2026.** Axiom 1: "Timeout is nothing" (`core-axioms.form:48`, `:50`). There is no surface word:
"`backtrack` and `timeout` have no surface word, and their owners stay `oac-backtrack-walk` and
`oac-timeout-walk`" (`native-bml-admission.md:443-444`). `oac-timeout-walk(alts,args,budget)` counts **attempts**,
not time, and answers `[ack, budget-left, alts-left]`; `oac-timed-out?` distinguishes a spent budget from
exhausted alternatives (`choice-lane-core.fk:19-27`). `oac-hold(recipe,args,ripen,patience)` re-offers with
changed arguments under a patience count (`offer-ack-core.fk:43-46`). Clock time enters only through helpers
(`wait-until`, `retry-first`, `native-bml-admission.md:142-149`) and through reader (b)'s run budget ("Budget: N"
ms, an offer of more time, not a wall — `bml-form-lower.bml:771-779`). `<TIMEOUT>` is a control invite
(`control-invite-grammar.fk:36`). observed.

### 2.8 try / catch / throw (and attempt, `??`, defer)

**2000.** `Try ::= "try" Statement;`, `Catch ::= "catch" "(" Parameter ")" Statement;`, `Throw ::= "throw"
Expression ";"` — "Any type of objects can be thrown"; the parameter type filters; a throw inside a handler goes
to the next outer block; uncaught terminates the program (`TT:627-632`). "The termination model of exceptions
handling is provided" (`TT:762`). Lowering uses a **separate channel** from failure: `jerr exHandlerLabel` …
`call REG0, IsInstance` … `throw` (`TT:967-981`) versus `jmpf`/`fail`. Methods can declare `exception = …`
(`TT:644`). The thesis does not say whether a throw undoes state. observed; the last sentence is an absence I
looked for and did not find.

**2026 (b).** `try`/`throw`/`catch` lower through a control-mode cell: "break and continue set the mode their
target reads; throw and return hold their value and set the mode when none is set"; "try: the body, then, on the
mode a catch takes, the catch's local from the thrown value and the handler" (`bml-form-lower.bml:500-512`). The
thrown value is kept (cell `t`, `:56`). Failure is a different cell (`f`). observed.

**2026 (c).** "`try e catch h` (also `try e else h`) attempts `e`; an absent answer or a caught runtime stop
selects `h`" (`native-bml-admission.md:104-105`). Lowering: `(attempt e)` then `nothing?` → `h`
(`form-bml-lower.fk:392-393`). There is **no thrown value** — the handler learns only that the body was absent.
`x ?? y` evaluates `y` only when `x` is absent and "does not add an attempt" (`:107-108`). "Only a genuine stop
selects the handler: `x / 0` is the integer 0 and `len(5)` is 0, so neither stops" (`:312-314`). Example:
`def size(x) = try len(x) catch 0;` (`:113`). `defer e;` is derived from attempt: the rest of the block runs under
`attempt`, then `e` runs, then a held stop is re-raised (`form-bml-lower.fk:126-135`). observed.

Carrying a reason forward is done by a fourth acknowledgement value, a node: `oac-backtrack-walk` hands each
option `[args, memory]`, "an absent answer is skipped, a node answer appends its payload to memory and the walk
goes on, and any other answer … ends it" (`choice-lane-core.fk:29-33`; `native-bml-admission.md:318-322`). This is
the 2026 equivalent of a caught exception's payload, and it matches axiom 5's four acknowledgements
(`core-axioms.form:88`). observed.

### 2.9 templates

**2000.** Two different template mechanisms:

- **BML templates**: "Templates are restricted to class and interface definitions" (`TT:234`). "A class template
  defines the layout and operations for an unbounded set of related types. An interface template is the same
  except it is restricted on operations" (`TT:755`). Syntax: `Template ::= "template" "<" [ TemplateParameter \\
  "," ] ">" ( Class | Interface ); TemplateParameter ::= InterfaceRef FieldName [ "=" InterfaceRef ] | Const |
  "interface" FieldName [ "=" InterfaceRef ];` (`TT:757`) — parameters are *typed by an interface* (a bound), may
  be a constant, and may have a default. Each instance can be given its own GUID: `"<" Signature ">" "=" "["
  Properties "]"` (`TT:756`). Example: `template<Object TYPE, Comparable KEY> class Queue { … }`
  (`bml-search-algorithms.txt:1061-1070`). observed.
- **BMF template rules**: "In BMF, rules can have arguments … An argument can be used as a parse element, a
  method, an argument to a method, or an argument to another rule call." `List( item, separator ) ::= item {
  separator item };` (`TT:148-152`). Arguments are "bound on run-time with the actual context" (`TT:177`), stored
  in the rule's Context (`TT:224`). observed.

**2026 (b).** "A template is BML-AST-TEMPLATE (parameter names, class or interface). Every Name<args> a unit
names instantiates it: type positions (field, parameter, const, local, new, base, super) are substituted, a
generic method's own type parameters shadow the template's, and the instance enters the layout as a class named
by the whole type." (`grammars/bml.fk:4095-4100`). It is **specialization by substitution**; `Box<int>` and
`Box<String>` are two classes, and overload choice happens per instance. Example
(`tests/fixtures/bml-class-templates.bml:13-14`):
```
template <K, V> class Pair : Object { section [public] { K first; V second; … } }
template <T> interface Source { section [public] { T Next(); } }
```
Parameter bounds ("which kinds a template may wrap (757)") are listed as unsupported
(`bml-native-north-star.form:32-33`). observed.

**2026 (c).** `template Name<T, …> { member name: Type; … }` and `class Name : Template<A, …> { … }`
(`grammars/form-bml.fk:86-102`, `:135-140`). The doc is explicit about the limit: "Templates describe parameters
and members; instance accessors, interface checking and specialization are not implied"
(`native-bml-admission.md:504-505`). observed.

**2026 (grammar).** A rule's action is itself a *template* in another sense: data (`emit`/`splice`/`const`) that
builds a node from captures (`bmf-core.fk:512-532`). observed. Three meanings of "template" stand side by side.

### 2.10 generics

**2000.** There is no separate "generics" concept; templates are it. Arrays are generic by construction:
"`int[]` creates an instance of `Array<int>`" (`bml-search-algorithms.txt:1004`). A parameter attribute `cast`
weakens an argument type with a checked cast (`TT:679`; used as `State hState [cast=CityState]`,
`bml-search-algorithms.txt:880`). observed.

**2026.** Generic class headers `class Box<T>` and generic methods `name<T>(…)` are read (`bml.fk:39`, `:54`;
`grammars/bml.fk:168`, `:3949`); in (b) they specialize (2.9). In (c) the only typing that executes is the
**comparison contract**: `String`, `Number`, `Comparable`, `Value` on parameters, lets and returns "select
[a] relationship without runtime wrappers … These are comparison contracts, not a general static type checker"
(`native-bml-admission.md:187-205`). `Disposable<H>` is a generic protocol that hoists a release function
(`:127-138`). Composite generic *data* types exist as blueprints: `list<T> set<T> option<T> map<K,V> tuple<A,B>`
(`grammars/form-bml.fk:14-21`; `bml/blueprint-literal.bml:7-10`). observed.

### 2.11 classes, interfaces, sections

**2000.** "A class is a structural specification with a list of interfaces to describe its behavior … An
interface is a list of methods and can be based on another interface. Blocks are code fragments that have the
context of the caller, but are being executed in the callee." (`TT:234`). Syntax (`TT:695`, `TT:701`):
`Class ::= "class" ClassName Properties [ ":" SuperClass \\ "," ] ClassBlock`; a class block may hold templates,
sections, **syntax** blocks, class interfaces, code blocks, inner classes, constants, methods, fields.
Method *definition* (name, parameters, properties, GUID) is separate from method *implementation*
(`TT:702-703`). Members have GUIDs (`TT:263`, `TT:687`). Operators are method calls by naming convention
(`TT:356`, `TT:418`). Constructors are not inherited; there are no destructors (`TT:738-743`). Coercion is an
attribute on a constructor or cast operator (`TT:279-280`, `TT:745-747`). Inner classes carry "a hidden field
that allows it to access fields from the outer class" (`TT:654`). `section [attrs] { … }` applies attributes to
a group and may be named and nested (`bml-search-algorithms.txt:219-229`). The compiler runs seven phases
(Register Main, Register Main 2, Resolve Main, Register Field, Register Other, Resolve, Resolve Offset —
`TT:769-846`). observed.

Class-level data lives in a **common object**: "the BML object model has basically removed the distinction
between classes and instances … the information contained in a custom class object is now stored in a common
object" reached through the instance's **instantiator**; two instantiators of one type may point at different
common objects (`sgb-bml-objects.txt:660-675`). observed.

**2026 (b).** "A record is a Form record carrying bmlClass, bmlClassId and every field. A member is one defn over
the packed (self a0 a1 ...) list … a method table reads self's class id and hands the same list to the member
that answers it." (`bml-form-lower.bml:598-602`). Construction: one record, fields at their keys with literal
initializers, each class's init member base-first, then the chosen constructor (`:639-641`). Class fields: "one
slot per class in the program's state; its initializer runs at the first read" (`:652-653`). Inner classes: a
hidden `__outer` field (`:648-651`). Access levels private/protected/public/package are enforced at lowering
(`grammars/bml.fk:3554-3559`). Example: `class A : Object, T { section [public] { int N() { return 1; } } }`
(`tests/fixtures/bml-class-interface-dispatch.bml:5`). The north star lists as **unsupported**: inner classes
(partly), class methods, template kind constraints, `Application.Main` as entry, section properties on methods,
deferred-method meaning (`bml-native-north-star.form:32-33`). observed.

**2026 (c).** A class is a **grouping**: "Classes group APIs" (`native-bml-admission.md:504`);
"form-bml-lower hoists the members into the section and binds the class name to its abstraction descriptor"
(`grammars/form-bml.fk:114-116`). No instances, no dispatch; "the cursor refuses a class nested inside another"
(`native-bml-admission.md:446-447`). Members may be `def`, `field`, `ref`, `thought` (`form-bml.fk:2-4`).
Attribute lists `[public, observable, jit]` are read (`form-bml.fk:158`; used at
`bml/bml-demand-jit.bml:28`). observed.

### 2.12 multiple inheritance

**2000.** "Each class can be based on one or more parent classes. The object model used allows only single
structural inheritance. If a class is derived from more than one class, the object model allows supports
delegation. This will behave like multiple inheritances, but one class has only one structural parent."
(`TT:234`). "If a class has more than one structural parent, each additional parent will generate a delegated
field. Each delegated field will add all its interfaces to the class. For all the methods that are not
implemented in the delegated interface, delegate deferred calls will be generated." (`TT:722-723`). The
inheritance attribute `delegate` is nonetheless marked "[Not supported yet]" (`TT:656`), as is `shared`
(`TT:657`). observed. The mechanism is specified in the companion: "Delegation inheritance does not include the
fields defined by the base classes; rather it uses a reference to the instances defined by them"
(`sgb-bml-objects.txt:711`); worked example `class C : A, B {}` … `class E : C, D { void goo() { … } }`
(`:719`) with the walk of `self`/`this` (`:721-738`). By-products: **shared inheritance** (two true objects share
one base object, `:744`) and **tagging** (`class ContainerItem : ITEM [tag]`, `:771`), the latter "only supports
at this point shared inheritance without the necessary normalization mechanism" (`:775`). Cost is named: "the
gained flexibility has also come at the cost of considerable performance loss during runtime" (`:796`).
observed.

**2026 (b).** "The first super is the structural base unless it names an interface. Of the others, an interface
is one the class declares and a class is a delegation base" (`grammars/bml.fk:4254-4257`). Lookup order: "A
class, then each of its supers in declaration order, depth first: the structural base's line, then each
delegation base's; each class once" (`:3339-3342`). "Delegation lends methods and subtype, not fields"
(`:3363-3365`). A field name declared by two classes that can share an object gets the key `Class.name`
(`:3352-3356`). A call the receiver's class does not answer "goes to a field marked delegate whose type answers
it" (`:3634-3637`; `bml-form-lower.bml:632`). The north star lists `multiple-class-bases` and
`delegate-field-slots` as supported (`bml-native-north-star.form:22`). observed in the compiler source.
**Not witnessed by a fixture I could find**: `grep` for `delegate`, `strict`, `[get`, `abstract`, `final`,
`inner` over `K/form/form-stdlib/tests/fixtures/bml-class-*.bml` returned nothing, and the only multi-super
fixtures have the shape `class X : Object, Interface` (`bml-class-interface-dispatch.bml:5-6`,
`bml-class-templates.bml:15`). The band `bml-full-class-model-proof` is registered at 2147483647
(`K/form/band-verdicts.txt:28`) and tests the class *model* (`tests/bml-full-class-model-proof.fk:73`, `:167`),
which I did not read in full. So "multiple inheritance runs end to end in 2026" is **assumed**, not observed.

**2026 (a), (c).** No inheritance. (a) erases bases (`bml.fk:42`); (c) has `class Name : Template<…>` only
(`form-bml.fk:117-140`). observed.

### 2.13 objects as a (data pointer, interface pointer) pair

**Where it is defined.** Not in the author's thesis text (`TT` never describes the reference layout). It is the
companion thesis: "A reference is composed of three parts: the object identifier, the interface identifier, and
the native flag. Both identifiers are indices into the object look-up table" (`sgb-bml-objects.txt:486`).
"two object references with the same object identifier could use two different interface identifiers. Hence, the
same object data would be used by different implementations" (`:493`). The façade alternative was rejected
because it "required the creation of many more instances" (`:493`). observed.

**Exact 2000 semantics** (all observed in `sgb-bml-objects.txt`):

- Object = fixed part (object fields, appended on structural inheritance) + optional custom part that is either a
  dynamic list of fields or binary data, selected by two descriptor flags (`:467-483`, `:528-546`).
- Native values (int, float, bool) live *in* the reference with the native flag set: "no-objects", cannot be
  aliased (`:489-490`). References are 64-bit (two 32-bit ids), native values 32-bit, max 2^31 objects (`:495`).
- Every object has a descriptor in the Object Look-Up Table: Counter (reference count), Size, FixedCount, InUse,
  Binary, Dynamic, Persistent, Data pointer, Instantiator (`:498-526`). Indirection allows moving an object.
- **Two bases on every call**: "The first argument refers to the behavioral base and the second argument refers
  to the structural base. The behavioral base is always used to dispatch methods … The structural base is always
  used for structural access." (`:642-644`). Named `self` (true object) and `this` (current structure)
  (`:721-738`); "calling a method on an object sets self and this to the foreign object's reference; forwarding a
  method to a delegation pointer sets only this" (`sgb-thesis-another-object-model.txt:103`).
- Dispatch is **indexed**, possible because methods are grouped per interface (`sgb-bml-objects.txt:640`).
- **Interface cast** = "a new object reference composed of the found interface and the original structure"
  (`:649-651`), found by walking the instantiator's interface list, O(n) (`:821`).
- **Detached interfaces**: an interface defined on top of an existing class can be attached at cast time if
  prerequisites hold; "the current language implementation does not have the necessary constructs" (`:787-791`).
- Identity by GUID (128-bit) with runtime RUIDs (20-bit) and file-local LUIDs (`:551`, `:680`), "to allow
  renaming" (`:555`).
- Instantiator kinds: multiple, **Singleton**, **Unique** — "instances that are created with the same data have
  the same reference … used, for example, for string objects" (`:778-784`).

**2026.** The (b) lowering does **not** use a pair. An object is one record with a `bmlClassId` key; an interface
call lowers to a stub defn that reads the class id and compares it against each table row
(`bml-form-lower.bml:684-685`: `bfk-stub` / `bfk-disp`, a chain of `(if (eq k N) …)`); `self` is `(head a0)`
(`:604`); there is one receiver, not two bases. Delegation is done by re-invoking on the delegate field's value
(`:632`), which makes the delegate the receiver — the thesis's rule that `self` stays on the true object while
only `this` moves is **not preserved** by that line, as far as I can read it. inferred from `:604`, `:632`,
`:684-685`; I did not run a case that would show it (a delegated method calling an overridden method).

The lineage README claims the pair lives on in another repository's substrate (`NamedCell{Recipe access, Base
blueprint, Name, CTOR recipe}`, `T/README.md:125`). I did not read that code. assumed.

### 2.14 field and class flags (attributes)

**2000 rule.** "every definition (i.e. class, method, variable) can be enhanced with attributes"
(`sgb-bml-objects.txt:153`); they stand in square brackets right after the declaration, in seven contexts: class,
inheritance, method, field, argument, variable, function (`bml-search-algorithms.txt:99-127`). Grammar:
`TT:640-648`. The table lists every attribute I found, with the thesis's own support mark.

| Context | Attribute | Meaning (2000) | 2000 mark | 2026 reader (b) |
|---|---|---|---|---|
| class | `struct` | fields are binary; get/put generated; gains `BML.lang.Structure` (`TT:648`) | — | not found |
| class | `dynamic=binary` / `dynamic=object` | resizable binary part / dynamic field part, gains `Indexed` (`TT:649-650`) | — | not found |
| class | `noobject` | value stored in the pointer itself (`TT:651`) | "Not fully supported yet" | not found |
| class | `unique` | no two equal instances (`TT:652`) | "Not supported yet" | not found |
| class | `singleton` | one instance (`TT:653`) | "Not supported yet" | not found |
| class | `outer` / (`inner`) | normal nested class / hidden outer field (`TT:654`; `bml-search-algorithms.txt:135-136`) | — | hidden `__outer` field (`bml-form-lower.bml:648-651`); reader work pending (`north-star:33`) |
| class | `abstract` | cannot be instantiated (`TT:655`) | — | `new` of abstract refused (`bml-form-lower.bml:642`) |
| class | `alias=`, `guid=`, `hidden`, `library=` | (`TT:641-643`, `TT:687-688`) | — | not found |
| inheritance | `delegate` | no structural inheritance; hidden field; calls delegated (`TT:656`) | "Not supported yet" | later supers are delegation bases (`grammars/bml.fk:4254-4257`) |
| inheritance | `shared` | delegated base supplied at creation, shareable (`TT:657`) | "Not supported yet" | flag read (`:4057`) |
| inheritance | `tag` | tagging (`sgb-bml-objects.txt:771`) | no normalization (`:775`) | not found |
| field | `name=` | external name (`TT:658`) | — | not found |
| field | `get[=props]`, `put[=props]` | generate read/write method (`TT:659-660`) | — | honoured (`grammars/bml.fk:3593-3594`) |
| field | `delegate` | "delegates all method calls to the default interface" (`TT:661`) | — | honoured (`:3634-3647`) |
| field | `inline` | embed the instance instead of a reference (`TT:662`) | — | not found |
| field | `exclusive` | reachable only through this field (`TT:663`) | "Not supported yet" | not found |
| field | `relaxed` (default) | may hold derived types (`TT:664`) | "Not supported yet" | default (`:3612-3615`) |
| field | `shared` (default) | reachable from many places (`TT:665`) | "Not supported yet" | flag read |
| field | `strict` | only the exact type (`TT:666`) | "Not supported yet" | static refusal plus a run-time check and trap (`bml-form-lower.bml:619-621`, `:724-725`) |
| field/member | `class` | belongs to the common instance (`TT:686`) | — | class field slot (`bml-form-lower.bml:652-657`); class methods unsupported (`north-star:32`) |
| local | `nostate` | not undone on backtracking (`TT:667`) | "Not supported yet" | not found |
| method | `extern=`, `library=`, `native`, `forwardAs=`, `forwardTo=` | external implementation (`TT:641`, `TT:644`, `TT:667-668`) | — | not found |
| method | `abstract`, `final` | (`TT:669`, `TT:674`) | final: "Not supported yet" | both read (`grammars/bml.fk:3792-3803`) |
| method | `const` | does not modify its base (`TT:670`) | "Not supported yet" | not found |
| method | `coercion`, `explicit`, `cost=` | implicit-conversion chain and its cheapest path (`TT:671-672`, `TT:644`) | cost: "Not supported yet" | not found |
| method | `default` | callable without its name (`TT:673`) | "Both not supported yet" | not found |
| method | `reassign` | result reassigns the call base (`TT:675`) | — | not found |
| method | `name=`, `inline`, `exception=` | (`TT:676-677`, `TT:644`) | inline: "Not supported yet" | not found |
| method | `deferred` | implemented by a delegating derived class (`bml-search-algorithms.txt:157-158`) | not in the thesis grammar (`TT:644`) | "runs as written, and its flag is voiced" (`grammars/bml.fk:3651-3653`) |
| parameter | `assign` | last parameter stands right of `=` (`TT:678`) | — | not found |
| parameter | `cast`, `in`, `out`, `inout` | (`TT:679-682`) | cast/out/inout: "Not supported yet" | not found |
| access | `public`, `protected`, `private`, (package) | (`TT:683-685`, `TT:726-732`) | "Not supported yet" (`TT:683-685`) | enforced (`grammars/bml.fk:3554-3559`) |
| array | `strict`, `relaxed`, `exclusive`, `shared` | (`TT:645`) | — | not found |
| function | `const`, `inline`, `nocontext` | local function may outlive its method (`bml-search-algorithms.txt:212-218`) | — | not found |

"not found" = my `grep` of `grammars/bml.fk` comment lines and of `bml-form-lower.bml` showed no handling; it is
an observation about what I searched, not a proof of absence. The 2026 rule for an unknown-meaning flag is
"where the thesis gives a flag no behaviour, the flag is carried and voiced until Urs gives it one"
(`bml-native-north-star.form:21`). observed.

Reader (c) keeps only bare-word class attribute lists, with no semantics attached by the grammar
(`form-bml.fk:158`). Reader (a) erases them (`bml.fk:44-46`). observed.

### 2.15 the model / grammar duality (BMF)

**2000.** Four statements define it:

1. A grammar is a list of single-definition named rules, one is top (`TT:130`). Elements: **containers** (rule
   definition, rule reference, template rule, branch = OR-list in order, sequence = AND-list, repeat with lower
   and optional upper bound), **primitives** (Cut, Fail, EOF, EOL, Multi match), **terminals** (character range,
   literal with two kinds of white space), plus Argument and Tag (`TT:137-179`).
2. Parsing builds a parse tree with the grammar's shape; after a rule is parsed, its **process** — "a method that
   gets a parse tree for that rule as input and returns an object" — replaces the tree by any object
   (`TT:131-136`). **Methods** inside a rule are semantic predicates: "If a method fails, then that means the
   parser will search for another path" (`TT:182`).
3. **The grammar is an object model**: "Each non-terminal symbol is represented as a class and can be derived
   from classes and support interfaces" (`TT:86`). The grammar elements themselves are BML classes — the runtime
   manifest lists `Branch`, `MethodCall`, `Repeat`, `Rule`, `RuleCall`, `Sequence`, `Tag`, `Cut`, `EndOfFile`,
   `EndOfLine`, `Fail`, `MultiMatch`, `Nil`, `CharChain`, `CharRange` (`source-samples/BMF-includes.bml:13-31`).
   Each has `Match( Context, Environment, String tag )`, `AsString()` (prints itself as BMF text) and
   `AsCreateString()` (prints the constructor call that rebuilds it) — `source-samples/container-Rule.bml:62-131`.
   One object, three renderings: executable matcher, grammar text, construction code.
4. **The grammar is editable while it runs**: users can add parsing constructs "even at run-time" and "modify,
   add, and delete rules from the currently used grammar during parsing" (`TT:80-81`); rules can take parameters
   "from the currently reading input file" (`TT:82`); no lexer, so binary formats and context-dependent white
   space work (`TT:82`).

And the dual direction: **any class may carry syntaxes** — "every object can have an arbitrary number of syntaxes
attached to it. The most common syntaxes found in objects are text and binary"
(`bml-search-algorithms.txt:239`). Example (`:760-771`):
```
syntax [text] {
   CitySearch ::= m_strName:word "," longitude:double "," latitude:double;
   ( m_dX, m_dY, m_dZ ) { … }
}
```
used as `in.Parse( City())` (`:940`). A rule's tags bind directly to the class's fields. observed.

**The combinators are derived from choose and fail.** The 2000 `Branch.Match` is
`for each item: choose { item.Match(…); return; }, {}` followed by `fail;`, and `Repeat.Match` is
`for( iCount < m_iMax ) choose { m_hItem.Match(…); }, { if ( iCount < m_iMin ) fail; return; }`
(`A/src/BMF/container/Branch.bml`, `Repeat.bml`, both read in this session). The thesis's description of repeat —
"starts out with the maximum of successful matches. If this fails, it will back off one element and try again"
(`TT:158`) — is not coded anywhere; it *falls out* of re-entrant choose. observed for the code; the "falls out"
is inferred from the code plus 2.1.

**2026.** Grammar-as-data survives; the object model around it does not.

- `bmf-core.fk`: "surface → cursor → match(pattern) → build(template) → NodeID" (`:7`); patterns are data
  (`lit cls run seq alt opt cap`, `:433-441`), templates are data (`emit splice const`, `:512-518`), a rule is
  `(pattern, template)` (`:540`). Scannerless, streaming, immutable cursor (`:41-50`).
- `bmf-grammar.fk`: named rules, recursive `ref`, `rep`/`sep`, `not`, `str`/`char`/`num`, `infix` (precedence
  table), `chain` (member/call/index), `ternary`, memoised rules (`rule3m`), and a compiled form with first-byte
  sets (`:1-11`, `:1109-1137`).
- **Semantics changed to ordered choice without re-entry**: `gx-alts` returns the first alternative that matches
  and never revisits it; `gx-rep` is greedy and never backs off (`bmf-grammar.fk:817-826`). The 2000 engine could
  revisit both (above). This is PEG behaviour, not Prolog behaviour. observed in the code; the PEG label is mine.
- BMF **text** surface: `name ::= pattern => emit-action <= reverse-action;`
  (`compiler.fk:235-236`; real rule: `host-effect ::= "effect" $domain:string … => host-effect-emit <=
  host-effect-reverse;`, `K/grammars/host-effect-vocabulary.bmf:9`). The `<= reverse` half is the 2026 form of
  the duality: one rule names both the parse action and its inverse (source from object). Many thesis rules carry
  both (`package-simple … => bml-emit-package-simple <= bml-source-package-simple;`, `grammars/bml.fk:949`).
- Grammar swap as data: "a different grammar compiles the SAME tokens into a DIFFERENT program (the
  compiler-compiler property…)" (`bmf-mini.fk:13-16`).
- One grammar shared by two consumers: "The executable reader and cursor grammar share the operator table in
  `grammars/form-bml.fk`" (`native-bml-admission.md:49-50`); and one literal syntax for programs and stored data:
  "the same grammar reads them out of stored text, so a literal has one syntax and there is no second parser"
  (`bml/blueprint-literal.bml:23-26`).
- Other languages are BMF grammars beside it: `go-bmf.fk`, `python-bmf.fk`, `rust-bmf.fk`, `typescript-bmf.fk`,
  `prolog-bmf.fk` (`ls K/form/form-stdlib/grammars`; not read).

Not found in 2026: rules with arguments (BMF template rules), run-time grammar mutation during a parse, the
`syntax [text] { }` block attached to a class as an executable construct (it is parsed as
`syntax-section`, `grammars/bml.fk:977`, with the generic emitter), methods-as-predicates inside rules. observed
as "not found by my reading".

### 2.16 streams

**2000.** `ParseStream` wraps an `InputStream` and "adds the backtracking capabilities"; methods `read`,
`getPosition`, `setPosition` (`TT:187-191`). `Location` (position, line, stream name) and `Range` (start line,
parse text) are position records (`TT:192-200`). `ParseStack` holds the tagged result tree
(push/pop/peek/drop/reduce, find by class or tag — `TT:201-222`). `Context` nests per rule and holds resolved
template arguments (`TT:223-231`). Evaluating attributes while parsing "will cut down the running parse tree in a
way, that even infinitive input streams can be supported" (`TT:76`). Text and binary streams
(`bml-search-algorithms.txt:1025-1028`); the error stream is `nostate` (`:1014`, `:1117`). "Multi-syntax support
in a single stream" (`TT:1014`). observed.

**2026.** A **surface** (`string`, `file-window`) and an immutable **cursor** `(surface, pos, state)`
(`bmf-core.fk:170-205`); "socket/cell/channel are the next surfaces" (`:10-12`). Positions are **bytes** on both
surfaces, after a repair dated 2026-07-26 (`:220-239`). White space and C-family comments are skipped before every
primitive (`:378-404`) — the thesis's two white-space kinds became one fixed skip. A stream of model tokens is
scanned by the same engine for control invites (`control-invite-grammar.fk:45-55`). observed.

### 2.17 how backtracking meets state and side effects

**2000: everything is undone unless exempted.**

- The reason BML exists: "It is best to write this parser in a language that supports backtracking from ground
  up. That is because parse attributes will be evaluated during the parsing phase … When the parser backs out,
  all the attributes already computed have to be undone as well." (`TT:76`).
- Mechanism: machine mode DO/UNDO (`bmcpu-main.cpp:32`); native methods supply do/redo/undo (`sgb-bml-objects.txt:634`).
- Scope: local variables, fields, the stream position, console output (the companion needs `nostate` on the error
  stream precisely because ordinary output *is* undone, `bml-search-algorithms.txt:1014`), generated search
  states ("the backtracking approach takes automatically care of removing the generated states", `:273`), even
  object destruction (`TT:743`).
- Exemption: `nostate` (variable, stream).
- Memory: reference counting, "fails with circular references"; a collector "is not an easy task because
  backtracking can ask to undo the destruction" (`TT:743`).
- Concurrency: speculation is exclusive — no other thread runs while a thread has degrees of freedom
  (`angelic-assembler.txt:4`).
- Unknown from what I read: how I/O to a real file or device is undone, and whether exceptions unwind the trail.

**2026: nothing needs undoing because most values are immutable; the rest is journaled or declared not rolled
back.**

- Values and cursors are immutable; a checkpoint is a value (`bmf-core.fk:19-21`; `native-bml-admission.md:367`).
- Identity is content-addressed, so the old state still exists as long as something references it
  (`core-axioms.form:66-71`).
- Reader (b): locals are single-assignment names, fields are journaled only inside an open snapshot (2.5).
- Reader (c): "Host effects are not rolled back" (`native-bml-admission.md:176`); "A host effect comes back only
  when the caller writes the saved value back" (`:378-379`). `defer`/`Disposable` release on every exit including
  a stop (`:124-138`).
- Evaluation-order rule that matters for effects: `undo` "evaluates all three arguments first; give it values,
  never a call that must not run on the absent branch" (`:377-378`); `choice`, `try`, `??`, `? :` and `match`
  evaluate only the selected branch (`:109`, `:54`).
- Every caught stop is voiced as one organ-health line (`fkwu-uni.c:364`; `native-bml-admission.md:109-110`), so a
  backtrack is observable — the 2000 system had a debugger for that (`TT:998-1004`), not a signal.

**The difference in one sentence (inferred from the two lists):** 2000 made the *machine* reversible and exempted
what must persist; 2026 made the *data* persistent and declared what cannot be reversed. The first loses nothing
but pays on every instruction; the second is free but cannot express "undo this effect" for a host effect.

### 2.18 lowering: to nodes, to machine code

**2000.** Seven phases over the parse tree (`TT:769-846`), then code generation per expression kind with four
result locations "Stack, Local, Register, and Void" (`TT:862`), and per statement into BM assembler with labels:
loop/while/for (`TT:872-896`), if (`:906-907`), switch/select (`:914-924`), choice (`:931-932`), break/continue/
return with "Clean up with blocks" (`:939-956`), fail (`:963-964`), try/catch (`:971-972`), throw (`:980-981`),
with (`:988-989`). Operators become method calls (`TT:356`). The first compiler "is written using the Java
implementation of BMF. Later versions … will be written using BML itself" (`TT:90`). The target is an interpreter:
"Interpreter → easy to debug", "Multi-platform capable (based on a virtual machine)" (`TT:1011`, `:1015`). No
native code generation. The instruction set has `jsuccess`/`jfailure`, `fail`, `raise`/`resume`, `save`/
`restore`/`discard`, `invokevirtual/special/state/interface`, `cast`, `count`/`loop` with low/high registers
(`angelic-assembler.txt:104-195`). observed.

**2026, reader (c) — the path the body actually runs** (`docs/bml-direct-nodes-jit.md:7`, measured 2026-11-13):
"BML text → grammar templates (`bmf-grammar.fk` `template-emit`, strings) → Form text → C parse (about 17% of the
beats) → node rows → a flat image written at 9 bytes a value". Details:

- The cursor grammar interns recipe nodes (`gx-build`), `fbl-lower` rebuilds them as source-compiler recipes, and
  "the text hop is only the printer `fsc-source-emit-recipe`" (`bml-direct-nodes-jit.md:24`).
- Lowering runs in a long-lived child process over a pipe (`bml-floor-compile.fk:1-14`), with a lowering memo
  (`.lowfk`) and a native image (`.fkb`) (`native-bml-admission.md:217-228`).
- Cost: "A 59,464-byte door becomes 201,998 bytes of lowered Form text (`.lowfk`) and a 38,391,347-byte image
  (`.fkb`), 645 times its source, because the closure of 351 units is assembled whole" (`bml-direct-nodes-jit.md:5`);
  cold build 12.94 s for the planner door, 7.63 s for a trivial door importing 12 units (`:6`, `:12`); "Strings are
  the seams between stages and also the cache keys, so one edit invalidates a whole closure while the unit of
  meaning is the recipe" (`:8`).
- Direction: "BML parses to nodes, each recipe is content-addressed by its own NodeID, the kernel JITs a recipe on
  first call and keeps the page under that id" (`:10`). Step 1 exists as the `jit_node` door (leaf-door mode 57)
  and `bml/source-node-emit.bml` (`:23-25`). Measured: the node walker costs 116 µs a recipe against 15 µs for the
  text printer, "so the text hop is not where the cold build spends its time" (`:26`). The lever is the cache
  keyed on recipe content id (`:26`).
- Machine code: the kernel writes words onto `MAP_JIT` pages (`fkwu-uni.c:4891-4926`, `:14432`); the north star
  names tag families "lowered-jit-tags-70-79 value-native-jit-tags-81-86 numeric-native-tags-87-90" and a loop
  lane (`bml-native-north-star.form:22`). `bml/bml-demand-jit.bml` specializes "an ordinary lowerable scalar
  recipe graph" on demand: the first request is answered by the evaluator, a later bounded step compiles, checks at
  the request argument, and publishes (`bml-demand-jit.bml:3-15`). Not yet: "general graph execution uses the
  Form walker, while direct CPU/Metal graph JIT remains the next compiler boundary" (`north-star:9`). observed in
  docs and code; no JIT output was produced or inspected by me.
- `bml/anf-lower.bml` gives every defn an ordered single-assignment form in which "An argument is evaluated once,
  in order, before the call" is a property of the rows, with a reference interpreter (`anf-lower.bml:1-18`).

**2026, reader (b).** Thesis BML → layout + scope readings (`grammars/bml.fk`) → Form `defn` text
(`bml-form-lower.bml:3-10`) → a child `fkwu` compiles and runs it and prints one answer line
`@bfk <kind>\t<unheld>\t<value>` (`:754`, `:810`). Control lowers through a mode cell and guards; loops become
tail-recursive defns; a loop without calls that can transfer control, without fail/return/throw, becomes the
shape "that fkwu's loop lane crystallizes" (`:198-202`, `:331-338`). An older table lowering to "Hati" tables also
exists in `grammars/bml.fk` (`:2088-2105` use `fk-*` builders) and "names cut as a need it cannot meet"
(`bml-form-lower.bml:574`). observed.

**2026, reader (a).** Emits `UE-*` universal nodes through `bmf-bp` coordinates (`bmf-core.fk:118-151`); a name
missing from that table silently becomes category `0.0.0.0` (`:93-99`, `:109-113`). observed.

### 2.19 2026 additions with no 2000 counterpart

All from `docs/native-bml-admission.md` unless cited otherwise; observed.

- Expression-bodied `def name(params) = expr;`, lexical lambdas `x => e`, closures that escape (`:84-89`).
- Pipeline `input |> stage(args)` — input evaluated once, supplied as the last argument (`:91-95`).
- `x ?? y` absence-coalescing (`:107`), `match x { p | q => e, _ => d }` by exact value (`:99-102`, `:156-161`).
- List literals, negative index, half-open slices, shared by lists and strings (bytes) (`:68-75`).
- `a ++ b` string join, right-associative; hyphen is part of a name unless spaced (`:43-45`).
- Comparison contracts as the only types (`:187-205`).
- `defer`, `Disposable<H>` (`:122-138`).
- `repeat` / `again` / `retry` on an event queue (`:177-185`, `:404-422`).
- `blueprint Name { field: type }` and content-addressed composite literals (`grammars/form-bml.fk:14-21`).
- Offer/acknowledge with four answers nothing/0/1/node (`offer-ack-core.fk:1-24`); lanes that keep every answer
  (`choice-lane-core.fk:13-15`); census that keeps answered and silent apart (`offer-ack-core.fk:56-61`).
- Choice receipts as data (`bml-source.fk:504-535`; `bml.fk:122`).
- Absence is first class and distinct from zero: "Zero, an empty list and an empty string are present answers"
  (`:259-260`).

2000 features with no 2026 counterpart that I found: `with (e) { .member }` exists in (b) only
(`bml-form-lower.bml:521`); blocks "executed in the callee" with the caller's context and local functions nested
to depth 8 (`TT:234`; `bml-search-algorithms.txt:85`); `select` (switch with implicit break) and case ranges
`lo .. hi` — in (b) (`grammars/bml.fk:1647`); binary types `binary Identifier [ size=…, mapping=… ]` and `struct`
classes for binary layout (`TT:271-274`); `asm` methods (`TT:689`); functor-notation operator tables with
user-definable operators of 1-3 characters (`TT:247-248`, `TT:289`); coercion chains with cost
(`TT:671-672`); the component database (`TT:87-88`); the visual browser and debugger (`TT:992-1004`).

---

## 3. 2000 versus 2026 at a glance

| Concept | 2000 definition | 2026 reader (b) thesis lane | 2026 reader (c) form.bml |
|---|---|---|---|
| choice | statement; re-entrant; VM choice point (`jmpf`) | statement; scoped, first unfailed branch | expression; first non-absent value |
| fail | statement; machine enters failure mode and undoes | sets FAILED cell; unheld fail ends program with standing value | the value `nothing` |
| stop | none (only inside Cut's description) | — | `nothing`; kernel `fk_stop` unwinds to `attempt` |
| cut | BMF primitive (backward = error + undo + stop); statement cut…mark | commits the enclosing choose | "offer only the first alternative" |
| undo | machine mode; do/redo/undo per native | field journal while a snapshot is open | select between checkpoint and memory |
| store/restore | VM `save/restore/discard` n bits; `mark` | `save`/`mark`/`restore`/`discard` statements, stacked | identity functions |
| timeout | none; Multi match bounds retries | run budget in ms, as an offer | attempt-count budget; "timeout is nothing" |
| try/throw | exceptions with typed catch; separate channel | mode cell, thrown value kept | attempt → absence; no payload |
| templates | class/interface templates with bounded params; BMF rule arguments | specialization by substitution | declarative template + language class |
| generics | = templates; `Array<int>` | generic classes and methods specialize | comparison contracts; blueprint type args |
| classes | structure + interface list; GUIDs; common object | record + class id + method table | namespace grouping |
| multiple inheritance | one structural base + delegation | first super structural, later supers delegation bases | none |
| object reference | (object id, interface id, native flag), two bases per call | one record, one receiver | none |
| flags | ~45 attributes in 7 contexts, about 20 unsupported | ~12 honoured | bare words, no meaning |
| grammar | objects with Match/AsString/AsCreateString; editable at run time; full backtracking | BMF text rulebook with `cut` and `<= reverse` | data patterns + data templates; ordered choice, greedy repeat |
| stream | ParseStream with position get/set, undone on backtrack | — | immutable byte cursor over a surface |
| effects | undone unless `nostate` | fields journaled; others not | not rolled back |
| target | BM assembler for an interpreting VM | Form defns run by fkwu | recipe nodes → Form text → C parse → node rows → walker and JIT pages |

All cells are summaries of section 2, with the citations there.

---

## 4. Orthogonal primitives and what is derivable

This section is **inferred** throughout; each line names what it rests on.

### 4.1 Primitives (none can be built from the others)

1. **Three-valued answer with a fourth for "here is something instead"**: nothing / 0 / 1 / node. Rests on axiom
   1 and axiom 5 (`core-axioms.form:48`, `:88`) and `offer-ack-core.fk:5-24`. `fail`, `stop`, `timeout`, an
   unanswered offer are all `nothing`; what differs is who reads it.
2. **A recover point that a failure unwinds to** (`attempt` in 2026, the choice point in 2000). Rests on
   `fkwu-uni.c:362-367` and `TT:931-932`. Everything else in the control family is written over it:
   `try` (`form-bml-lower.fk:392`), `choice` (`source-compiler.fk:692-696`), `defer` (`form-bml-lower.fk:128-135`),
   `oac-offer` (`offer-ack-core.fk:26`).
3. **Re-entry**: whether a recover point stays live after its body has succeeded. This is the one real semantic
   fork between 2000 and 2026 (2.1). Scoped choice is re-entrant choice plus an automatic commit at exit;
   re-entrant choice cannot be recovered from scoped choice without rewriting the rest of the program as the
   continuation of each branch. So the *general* primitive is the re-entrant one, with commit (`cut`) as its
   companion. Rests on `bml-search-algorithms.txt:58-66`, `A/src/Test/Choice.bml`, `bml-form-lower.bml:525-531`.
4. **Commit** (`cut`): discard recover points back to a mark. Not derivable from choice; it is the only way to
   say "do not come back here". Rests on `TT:160-161`, `bml-search-algorithms.txt:77-78`,
   `bml-form-lower.bml:568-573`.
5. **State with a known inverse.** Either state is a name bound to an immutable content-addressed value (then
   restore is re-pointing, axiom 3, `core-axioms.form:71`) or it is an effect that declares its own
   do/redo/undo (`sgb-bml-objects.txt:634`) or declares itself irreversible (`nostate`, `TT:667`). These are three
   declarations of one thing — *how this state behaves when a recover point is taken* — and the property must be
   attachable to any cell.
6. **A reference as a pair (structure, behaviour)** with dispatch on behaviour and field access on structure, and
   the two-base call rule (`self` fixed on the true object, `this` moving). Rests on `sgb-bml-objects.txt:486-493`,
   `:642-644`, `:721-738`. Not derivable from records and functions without re-inventing the pair.
7. **A cursor over a surface** with terminals (byte/character range, literal) and capture. Rests on
   `TT:171-179`, `bmf-core.fk:170-297`, `:433-441`.
8. **Attributes on any declaration** (a property list attached to a node, readable by compiler and runtime).
   Rests on `sgb-bml-objects.txt:153`, `TT:640-648`.
9. **Parameterised declarations** (a declaration that takes declarations or values and yields a declaration).
   Class templates, interface templates, generic methods and BMF rule arguments are the same primitive at four
   sites. Rests on `TT:148-152`, `TT:755-757`, `grammars/bml.fk:4095-4100`.
10. **Identity independent of name** (GUID in 2000, NodeID in 2026). Rests on `sgb-bml-objects.txt:551-555`,
    `core-axioms.form:66-70`.

### 4.2 Derivable (shown derived in the sources)

- `fail` = produce nothing at a recover point (`A/src/BMF/primitive/Fail.bml`; `form-bml.fk:52`).
- `if_fail`, `while_success` = re-entrant choose with an empty first branch / choose in a loop
  (`bml-search-algorithms.txt:58-74`).
- Branch, Sequence, Repeat, Optional, Zero-to-many, One-to-many, EOF, EOL = choose + fail + loop over a cursor
  (`A/src/BMF/container/Branch.bml`, `Repeat.bml`; `TT:158`).
- `ChoiceSet.Pick` = re-entrant choose over a list (`bml-search-algorithms.txt:1007-1008`).
- `try/catch` = recover point + payload; `throw` = failure carrying a node (`choice-lane-core.fk:29-33`;
  `form-bml-lower.fk:392`). The 2000 separate `jerr` channel (`TT:971-972`) is an implementation choice.
- `defer`, `Disposable` = attempt + re-raise (`form-bml-lower.fk:126-135`).
- `store`/`restore`/`discard`/`mark` = primitive 5 when state is immutable (`choice-lane-core.fk:16-17`); a stack
  of snapshots when it is not (`grammars/bml.fk:1941-1948`).
- timeout / budget / hold / retry = recursion with a counter over offers (`choice-lane-core.fk:19-27`;
  `offer-ack-core.fk:43-46`). Wall-clock timeout additionally needs a host clock reading.
- Structural inheritance = layout concatenation of the fixed part (`sgb-bml-objects.txt:469-475`).
- Multiple inheritance, shared inheritance, tagging, inner classes = delegation with a hidden field under the
  two-base rule (`sgb-bml-objects.txt:711-775`; `TT:654`).
- Interface cast, detached interface, "view" = build a new pair from the same structure and another behaviour
  (`sgb-bml-objects.txt:649-651`, `:787`).
- Class fields and class methods = a second object (the common object) reached through the instantiator
  (`sgb-bml-objects.txt:660-669`).
- Singleton, Unique = instantiator kinds (`sgb-bml-objects.txt:778-784`). **Unique is content addressing**: same
  data, same reference — axiom 3 already gives it for every cell (`core-axioms.form:67`).
- get/put properties, operator methods, coercion, array indexing, `instanceof` = method calls by naming
  convention (`TT:332`, `TT:344`, `TT:356`, `TT:659-660`).
- `with`, `select`, `switch`, `loop`, `for`, `do` = sugar over jump and compare (`TT:872-989`).
- `|>`, `??`, `match`, slices, `++` = sugar over call, `nothing?`, compare (`native-bml-admission.md`; 2.19).
- access levels, `final`, `abstract`, `strict` = attribute checks (`grammars/bml.fk:3554-3559`, `:3612-3618`).

---

## 5. What a single unified grammar must keep so nothing BML can express is lost

Each item is a requirement inferred from the cited observation.

1. **One meaning for each control word.** Today `fail`, `cut`, `choice`, `class`, `template`, `store`, `undo`
   each have two or three meanings (section 1.2, 2.1-2.9). The new grammar needs one definition per word, and the
   re-entrant reading must be expressible (2.1), since it is the one that makes the parser combinators and
   `if_fail`/`while_success`/`Pick` one-liners (4.2).
2. **Both strengths of choice, named apart**: "first that answers" (commit at exit; cheap, what 2026 uses
   everywhere) and "stay open" (re-entrant; what the thesis defines). One is the other plus a commit, so one
   construct with an explicit commit suffices — but the default must be stated.
3. **`cut` with a scope rule** (which recover points it discards) and its partner `mark` (2.4).
4. **A reversibility declaration per state/effect**: immutable-and-repointable, journaled, has-inverse
   (do/redo/undo), or irreversible (`nostate`). 2026 has only the first two and a sentence in a doc for the
   fourth (2.5, 2.17).
5. **A failure that can carry a reason** (node acknowledgement / thrown value) so `try … catch (T x)` and
   backtrack-with-memory are the same construct (2.8).
6. **Absence distinct from zero, empty and false** in every position (axiom 1; `native-bml-admission.md:259`).
7. **The reference pair and the two-base call rule** as the object primitive, with cast = re-pairing, so that
   multiple inheritance by delegation, shared bases, tagging, detached interfaces and views are library patterns
   (2.12, 2.13). The 2026 record-plus-class-id lowering cannot express a view of the same data through a second
   interface without copying or wrapping (inferred, 2.13).
8. **Attributes on every declaration**, user-extensible, readable at compile time and at run time; a `section`
   form that applies a set to many members (2.14). The 2026 rule "a flag without a meaning is carried and
   voiced" (`north-star:21`) is a good default.
9. **Parameterised declarations at every site**: blueprints (classes), interfaces, recipes (methods) and grammar
   rules, with optional bounds and defaults (`TT:757`), and constants as parameters (2.9).
10. **Grammar rules as first-class cells of the same language**: a rule is a recipe with a pattern, an action and
    optionally an inverse; rules take arguments; a blueprint may carry any number of named syntaxes (text,
    binary, …) whose captures bind its fields; the rule set is a value that can be extended during a parse
    (2.15). If rules are written with the language's own choice/fail/cut, the grammar engine is not a second
    interpreter.
11. **A cursor/surface abstraction** that is immutable (so backtracking costs nothing), byte-exact, and open to
    non-text surfaces (binary, token streams, sockets) (2.16).
12. **Identity independent of name** for every declaration and every instance (GUID → NodeID), which also gives
    `unique` for free and makes renaming safe (2.13, 4.2).
13. **Value-level types beyond comparison contracts** if the thesis's overload resolution, coercion chains,
    `strict`/`relaxed` fields and `cast` parameters are to stay expressible (2.10, 2.14). In 2026 these exist
    only in reader (b).
14. **Binary layout declarations** (`binary` types with size and mapping, `struct` classes, `inline` fields) —
    the thesis's bridge to foreign memory (`TT:271-274`, `TT:662`); absent in 2026 BML and directly relevant to a
    bootstrap that emits machine code.
15. **Observable backtracking**: every recover-point unwind leaves a reading (2.17).

What can be dropped without losing expressiveness (inferred): the separate exception channel; the seven compiler
phases as a fixed list; functor-notation operator tables (a precedence table as data does it,
`form-bml.fk:41-48`); the text hop between grammar and nodes (`bml-direct-nodes-jit.md:8`); reader (a); the
second statement walker the memory note already wanted gone ("new syntax lands once, in the cursor",
`M/project-bml-surface-directions-2026-09-29.md:13-16`); `store`/`restore` as words when state is immutable.

---

## 6. Assumptions, limits of this study, open questions

Assumptions (not verified):

- That the 2000 implementation worked as its documents say. I read sources and documents; the binaries in
  `A/BMCPU`, `A/Java` were not run. The thesis itself marks about twenty attributes unsupported and three
  chapters unwritten.
- That the 2026 fixtures answer what their heads declare. I read `Answer: 112`, `8`, `20`, `133`, `21`; I did not
  run them.
- That multiple inheritance by delegation and field flags run end to end in reader (b) (no fixture found, 2.12).
- That the Coherence-Network substrate carries the reference pair (`T/README.md:125`).

Limits:

- `grammars/bml.fk` is 490 KB; I read its comment narrative and about 200 lines. Statements of the form "not
  found" about it are bounded by that.
- `form-native-coding.md` and `native-bml-execution-learning.md` are about the coding controller and a learning
  curriculum; they add nothing to BML's concept definitions beyond what is cited.
- `sgb-bml-objects.txt` is a plain-text conversion; figures are missing, so the delegation walk is read from
  prose only.

Open questions are listed in the structured summary.

---

## 7. Citation index (file → what it grounds)

- `TT:6`, `:76-90` — design goals, why backtracking, why a backtracking VM.
- `TT:126-182` — BMF elements, primitives, terminals, methods.
- `TT:186-231` — ParseStream, Location, Range, ParseStack, Context.
- `TT:234` — class/interface/block/template definitions; single structural inheritance + delegation.
- `TT:245` — keywords. `TT:271-275` — binary types, structures, lvalues.
- `TT:279-594` — coercions, expressions, operator tables.
- `TT:596-636` — statements, including Choice (`:625-626`), Try/Catch/Throw (`:627-632`), Fail (`:633-634`).
- `TT:640-689` — properties grammar and every attribute.
- `TT:695`, `:701-723` — class and interface syntax, delegation.
- `TT:738-747` — constructors, destructors and backtracking, conversion.
- `TT:755-757` — templates. `TT:762` — exceptions.
- `TT:769-989` — compiler phases and statement translations.
- `TT:1006-1017` — advantages, disadvantages.
- `T/companion/sgb-bml-objects.txt:467-555` — object structure, references, descriptor, GUID/RUID.
- `…:569-634` — instantiator, definitions, method with do/redo/undo.
- `…:640-675` — dispatch with two bases, casting, common objects.
- `…:688-791` — structure-behaviour separation, delegation, shared inheritance, tagging, instantiators, detached
  interfaces. `…:794-821` — costs and future work.
- `T/companion/bml-search-algorithms.txt:49-78` — choose, if_fail, while_success, fail, cut and mark.
- `…:99-229` — attributes and sections. `…:239-246` — syntaxes on objects; no unification.
- `…:667-717` — backtracking search with `nostate`. `…:760-771` — `syntax [text]` on a class.
- `…:1007-1014` — ChoiceSet, nostate error stream.
- `T/companion/angelic-assembler.txt:4`, `:104-195` — speculation rule, instruction set.
- `T/companion/source-samples/*` — Cut primitive, Rule container, VM main with DO/UNDO, BMF manifest.
- `A/src/Test/Choice.bml`, `A/src/BMF/primitive/Fail.bml`, `MultiMatch.bml`, `A/src/BMF/container/Branch.bml`,
  `Repeat.bml` — 2000 sources read from local disk.
- `K/form/form-stdlib/bml.fk` — reader (a). `K/form/form-stdlib/grammars/bml.fk`,
  `K/form/form-stdlib/bml/bml-form-lower.bml` — reader (b). `K/form/form-stdlib/grammars/form-bml.fk`,
  `form-bml-lower.fk`, `source-compiler.fk:681-696` — reader (c).
- `K/form/form-stdlib/bmf-core.fk`, `bmf-grammar.fk`, `bmf-mini.fk`, `compiler.fk:234-262`, `engine.fk:35-58`,
  `K/grammars/host-effect-vocabulary.bmf` — BMF in 2026.
- `K/control/offer-ack-core.fk`, `K/control/choice-lane-core.fk`, `K/teachings/surprise-is-a-choice-point.md`,
  `K/runtime/fkwu-uni.c:362-367`, `:1996-2008` — control primitives in 2026.
- `K/docs/native-bml-admission.md` — the form.bml surface and control patterns.
- `K/docs/bml-direct-nodes-jit.md`, `K/grammars/bml-native-north-star.form`,
  `K/form/form-stdlib/bml/bml-demand-jit.bml:1-30`, `K/form/form-stdlib/bml/anf-lower.bml:1-18` — lowering, JIT,
  direction.
- `K/axioms/core-axioms.form:48-88`, `:116` — axioms 1, 3, 5 and the reversibility theorem.
