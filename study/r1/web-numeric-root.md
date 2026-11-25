# Prior art for "one numeric space is the root of every language; symbols are a replaceable lens"

Study date: 2026-11-24. Scope: web prior art (programming side, natural-language side, the absent-value lineage),
read against the five axioms and the NUMS.Go reference. No repository was modified.

## 0. Method, status legend, and the limits of this study

Status marks used on every claim:

- **[O] observed** — fetched or read in this session on 2026-11-24. Web pages were read through the WebFetch tool,
  which converts the page and answers through a small summarising model. So an [O] web claim means "the fetched page,
  as relayed by that tool, says this". Short quotes are the tool's relay of the page text; I did not verify them byte
  for byte. Where the tool said "the page does not say", I recorded that and did not fill the gap.
- **[I] inferred** — my reasoning; the observed claims it rests on are named.
- **[A] assumed** — model memory, unverified in this session (knowledge cutoff mid-2026). Treat as a lead, not a fact.

Limits of this study (movable):

- Fetches that failed and stayed unverified: Babelfish UAST spec (`doc.bblf.sh`, DNS failure), Dion (`dion.systems`,
  DNS failure), SCIP design doc (404), a Unison conflict-resolution doc (404), a Truffle host-compilation doc (404),
  Dhall safety page (HTTP 521), Lamdu blog (403), the source{d} Wikipedia page (404), a Microsoft Excel support page (404).
- Not fetched at all (covered only as [A]): Smalltalk images, Red, APL/K specifics, VerbNet, PropBank detail,
  Option/Maybe and domain-theory bottom, LLVM `undef`/`poison` detail, Racket `#lang` expansion detail.
- Long pages were read only in their first 100,000 characters unless an offset is noted.
- One search for HVM3/HVM4 status returned only unrelated results; the 2026 state of that line is known only from the
  two repository front pages cited below.

Local grounding read in this session:

- Axioms: `coherence-kernel/axioms/core-axioms.form`
  lines 47-93 (the five statements), 99-104 (trinity, names are 0..many), 116 (undo is repoint), 212-219
  (absence is not a verdict), 174-183 (held offer). File header states "acknowledged 2026-11-08" (line 17). [O]
- NUMS.Go: last commit `9e6bbb4` dated 2023-04-05 (`git log`). [O] `README.md` is empty (0 lines). [O]
  `NUMS.Go/nums/nums_symbols.go:9-15` keeps a per-recipe symbol table mapping strings to
  `NodeInstance` (locals, parameters, integers, decimals, strings, literals, labels) beside blueprint-id maps: the
  symbols are a side table, the nodes are numeric. [O] `NUMS.Go/http/model/language.go:10`
  imports `github.com/smacker/go-tree-sitter` and lines 15-43 switch over twelve language ids (C, C++, C#, Java,
  JavaScript, Kotlin, Go, PHP, Python, Ruby, Scala, TypeScript). [O] So the reference already sits on the same
  front end (tree-sitter) that the industrial "many languages into one node space" efforts use (section 2.5). [I]

## 1. The thesis, split into claims that can be tested separately

The request's sentence bundles six claims. Prior art supports them very unevenly, so they are kept apart here.

| id | claim | verdict from prior art |
|----|-------|------------------------|
| T1 | Any program or sentence can be *encoded* in a numeric node space. | Settled since 1931; by itself it carries no information (2.2). |
| T2 | Identity of a node can be its content, with names as free metadata. | Proven in production for pure code (Unison 1.0, Nov 2025); costs are known (2.1). |
| T3 | Surface symbols can be swapped per human language without changing the stored thing. | Proven for a system's *own vocabulary* (Scratch, Excel, Wikidata, Wikifunctions, MPS). Not proven for user-chosen names or embedded strings (2.4, 3.2). |
| T4 | Many programming languages can share one node space. | Structurally yes; semantically every serious attempt fell back to per-language schemas plus a thin shared layer (2.5, 2.6). |
| T5 | Natural languages share one meaning space with per-language rendering. | Works inside a controlled abstract syntax (GF). Open-domain attempts are contested or stalled as of 2026 (3.1-3.6). |
| T6 | Three states {0, 1, nothing} are a sufficient ground. | The absent-value lineage shows one marker with several readings is the classic failure; the axioms already name the separation that avoids it (4). |

## 2. Programming side

### 2.1 Unison — content hash as identity, names as metadata

Proves:

- "Each Unison definition is identified by a hash of its syntax tree"; 512-bit SHA3; "names are just separately stored
  metadata that don't affect the function's hash"; bound names become positional references and dependencies are
  replaced by their hashes. [O] https://www.unison-lang.org/docs/the-big-idea/ (page shows only "(c) 2026", no date).
- A hash is "a 512-bit SHA3 digest of a term or a type's internal structure, excluding all names"; shown base32Hex;
  prefixes are accepted when unambiguous; a member of a mutually recursive cycle is `#x.n` with the cycle given "a
  canonical order determined by sorting all the members of the cycle by their individual hashes"; a constructor is
  `#x#c`; builtins are `##Nat`. [O] https://www.unison-lang.org/docs/language-reference/hashes/
- Claimed consequences: a parse/typecheck cache "which is never invalidated", "instant non-breaking renames", two
  versions of a type coexisting, values and functions serialisable and movable between machines. [O] big-idea page.
- It shipped: Unison 1.0, November 2025; company founded Feb 2018, first alpha Aug 2019. [O]
  https://www.unison-lang.org/unison-1-0 (a search result gives 25 November 2025 for the day [O, secondary]).

Limits:

- The store itself had to change: Apr 2021 moved from a "git-style, filesystem-based" codebase to SQLite "for a 100x
  size reduction". [O] unison-1-0 page. One file per hash did not scale. [I from that]
- After 1.0 the format still moved: version 1.1.0 (January 2026) changed the codebase in a non-backwards-compatible
  way to sync comments. [O, search-result summary of https://www.unison-lang.org/blog/ucm110/, not fetched directly]
- "C FFI support" is still listed as future work at 1.0. [O] unison-1-0. Seven years from founding to 1.0 without a
  foreign-function door: the world outside the hash space is the hard edge. [I]
- Cycles need a special rule (above). Identity by content cannot name "a thing that changes"; names and branches are
  the mutable pointers ("We can change what address a name points to" [O] big-idea).
- Nominal ("unique") types carry a salt so two structurally equal types can differ. [A] model memory, unverified.
- Humans still edit text: definitions are printed into a scratch file with current names, edited, and parsed back. [A]
  model memory; the page fetched for it did not cover the mechanism.
- Upgrades that break dependents open the affected terms in an editor until "the file typechecks". [O]
  https://www.unison-lang.org/docs/tooling/project-workflows/

Lesson for the one grammar: axiom 3 (line 67 of core-axioms.form) and "names are 0..many free query keys" (line 104)
are Unison's exact design, and it is the only one on this list that reached a production 1.0. Keep: hash of the
name-free structure; a separate name table per lens; a canonical rule for cycles decided on day one; a compact store
(not a file per node). Decide early: how a nominal distinction is expressed (a salt cell inside the blueprint), and
how effects and the host world cross (axiom 4's interface), because that was Unison's longest-standing gap.

### 2.2 Goedel numbering — encoding is free, and arbitrary

- Arithmetisation maps symbols, formulas and proofs to natural numbers; "there are many possible ways of
  accomplishing this, and the details do not really matter"; the one requirement is that the mapping be effective in
  both directions. It is what lets a system speak about its own proofs (diagonal lemma). [O]
  https://plato.stanford.edu/entries/goedel-incompleteness/ (first published 2013-11-11, substantive revision 2025-10-08).

Lesson: T1 is true and cheap. A numeric space being *possible* is not evidence for the thesis; what matters is which
numbering makes equal things equal (2.1, 2.3) and which makes self-description usable. The same entry notes the
diagonal construction yields a provable equivalence, "not sameness of meaning" [O]: a system describing itself in
its own numbers inherits incompleteness, so "the grammar fully describes itself" should be read as executable
self-description, never as a completeness claim. [I]

### 2.3 de Bruijn indices and locally nameless — how to take names out of binders

- de Bruijn (1972): a variable is the count of binders between occurrence and binder; terms are "invariant with
  respect to alpha-conversion", so alpha-equivalence is syntactic equality. Cost: substitution needs shifting. Levels
  count from the other end; nominal techniques and higher-order abstract syntax are the listed alternatives. Systems
  using indices show users named variables. [O] https://en.wikipedia.org/wiki/De_Bruijn_index
- Locally nameless: "de Bruijn indices to represent bound variables while retaining names for free variables";
  gives "a unique representation of terms" and "a very simple implementation of substitution"; used in Coq, Lego,
  Isabelle, HOL4, Epigram. The idea is already in de Bruijn's 1972 paper. The paper generalises to multiple,
  recursive and mutually recursive binders and patterns. [O] Chargueraud, "The Locally Nameless Representation",
  https://chargueraud.org/research/2009/ln/main.pdf, pages 1-2 read as PDF (manuscript; cited works date it 2009+).
- Dhall alpha-normalisation "renames all bound variables within an expression to use De Bruijn indices", then
  beta-normalises and encodes to CBOR. [O] https://github.com/dhall-lang/dhall-lang/blob/master/standard/README.md
  That the SHA-256 of that encoding is the "semantic integrity check" is [A] (the page fetched did not state it).

Lesson: the canonical form that is hashed must be binder-name-free (indices or levels), and free references must be
hashes, not names. That is locally nameless with "free name" replaced by "node id". Dhall hashes the *normal form*,
so refactorings that normalise equal hash equal; Unison hashes the *syntax tree*. This is a real design choice:
hash-of-normal-form gives more equalities but requires total normalisation (Dhall is total; a language with choice,
stop and effects is not). The one grammar should hash structure, and treat "these two blueprints behave the same" as
an observed, recorded relation between two ids, never as identity. [I from the three sources above]

### 2.4 Symbols as a lens over stored structure — where it already works

| system | stored form | lens | where the lens stops |
|--------|-------------|------|----------------------|
| Excel | canonical English function names in .xlsx; the older .xls stored numeric function codes | names and argument separators swapped per display language "each time the file is opened" | quoted text in formulas "will not translate"; TEXT format codes "partially localized"; CSV; "The Visual Basic editor only speaks English"; several scripts get no localisation |
| Scratch | block objects keyed by id with an `opcode` string; variables keyed by id with the name stored beside | block text translated into "more than 75 languages" | "Changing the editor language does not translate variable names" |
| Wikidata | Q and P numbers | "a label, a description and any number of aliases" per language | labels are not a grammar: sentences need lexemes, forms, senses (3.2) |
| Wikifunctions | ZObjects, keys like `Z1K1`; "something like LISP in JSON" | labels "in any of the more than 300 languages" | labelized forms "are often ambiguous and cannot always be converted back" |
| JetBrains MPS | AST nodes each "with a unique ID (UID)", references "based on actual pointers" | "several different editors" for the same concept | "Special support is needed for infrastructure integration" (diff, merge) |
| Lamdu | not text files (storage format not stated on the pages fetched) | "the natural language of the code just a display preference" | "technology preview", v0.8.1, started 2011 |
| Hedy | textual language | keywords in 47 human languages | a teaching language; mixing rules not stated on the page |
| ALGOL 68 | — | standard published in Russian, German, French, Bulgarian | historical |

Sources, all [O]: Excel — https://office-watch.com/2026/excel-formulas-different-languages-how-it-works/ (published
2026-05-31; a secondary source, not Microsoft); Scratch — https://en.scratch-wiki.info/wiki/Scratch_File_Format and
https://en.scratch-wiki.info/wiki/Translating_Scratch and
https://en.wikipedia.org/wiki/Non-English-based_programming_languages; Wikidata —
https://www.wikidata.org/wiki/Wikidata:Introduction (last edited 2026-10-21); Wikifunctions —
https://www.wikifunctions.org/wiki/Wikifunctions:Function_model (last edited 2026-02-25); MPS —
https://www.jetbrains.com/help/mps/mps-faq.html and https://www.jetbrains.com/help/mps/basic-notions.html (MPS 2026.2
help, page dated 2026-08-06); Lamdu — https://www.lamdu.org/; Hedy — https://hedy.org/.

Lesson (the sharpest one on the programming side): the lens is proven exactly as far as the *system owns the
vocabulary*. Every deployed system translates its own keywords, opcodes and labels, and every one of them stops at
(a) names the user chose, (b) strings and format codes embedded in data, and (c) the inverse direction, label back to
id, which is ambiguous. A design that wants user-chosen names to be a lens too must make a user name a node with a
label set per language (the Wikidata move), not a string. And it must treat text-to-node as a *choice* that can
return several candidates or nothing, never as a function. [I from the table]

Note the identity scheme: none of these label-lens systems is content-addressed. They use assigned ids (Q42, Z10,
UUIDs, opcodes). Only Unison and Dhall combine content identity with names-as-metadata. This tension is taken up in
section 5.1.

### 2.5 "Many languages into one node space" — the industrial record

- tree-sitter: a parser generator and incremental parsing library; goals "General enough to parse any programming
  language", "Fast enough to parse on every keystroke", "Robust enough to provide useful results even in the presence
  of syntax errors". [O] https://tree-sitter.github.io/tree-sitter/ Node types are per grammar (`node-types.json`,
  identified by `type` and `named`); nothing standardises names across languages. [O]
  https://tree-sitter.github.io/tree-sitter/using-parsers/6-static-node-types.html
- GitHub semantic: built shared cross-language syntax types ("data types a la carte"), then moved to per-language
  types generated from each tree-sitter grammar because the shared ones were "essentially untyped", made it "hard for
  us to make our analysis sensitive to minor linguistic differences", and the hand-mapping was "super time-consuming,
  very developer-intensive, error-prone, and extremely tedious". [O]
  https://github.com/github/semantic/blob/main/docs/codegen.md The repository was archived on 2025-04-01, "no longer
  supported or updated by GitHub". [O] https://github.com/github/semantic
- CodeQL: "Each language has its own unique database schema"; one database per language; libraries give classes
  "over the database tables". [O] https://codeql.github.com/docs/codeql-overview/about-codeql/
- Glean: "Facts are immutable terms described by user-defined schemas, and form a DAG", deduplicated; "There can be
  arbitrary amounts of language-specific detail in the schema for each language"; a language-neutral layer
  (`codemarkup.angle`) is *derived* on top. It stores what clients need, not full expressions. [O]
  https://glean.software/docs/introduction/
- Kythe: hub-and-spoke to turn "a worst-case of O(LxCxB)" into O(L+C+B); "By design, the Kythe graph schema is
  liberal and extensible", extended "without recourse to a central authority". [O]
  https://kythe.io/docs/kythe-overview.html A node name (VName) is a five-field vector: signature ("opaque ...
  generated by the analyzer"), corpus, root, path, language. [O] https://kythe.io/docs/kythe-storage.html
- LSIF: a dump of language-server answers as a graph of vertices and edges with ids; monikers (scheme + identifier)
  must be "position independent and stable"; "the LSIF therefore doesn't define a symbol database". [O]
  https://microsoft.github.io/language-server-protocol/specifications/lsif/0.6.0/specification/
- SCIP: "a language-agnostic protocol for indexing source code"; indexers for Java/Scala/Kotlin, TypeScript, Rust,
  C/C++, Ruby, Python, C#, Dart, PHP. [O] https://github.com/sourcegraph/scip That SCIP replaced LSIF's opaque
  numeric ids with human-readable symbol strings is [A] (design doc returned 404).
- srcML: XML markup over source for C, C++, C#, Java; "the conversion from source code to srcML is lossless". [O]
  https://www.srcml.org/about.html
- Babelfish/UAST: the repository page describes only "A self-hosted server for source code parsing" with Python and
  Java drivers in a sample. [O] https://github.com/bblfsh/bblfshd That the sponsoring company stopped around 2020 and
  the project went dormant is [A] (spec host no longer resolves [O], which is consistent with it).

What it proves: one *storage substrate* and one *query language* over many languages is routine (Glean, CodeQL,
Kythe run at very large scale [A for the scale]).

Where it hit limits: every project that tried one *shared schema of constructs* either abandoned it (semantic) or
never attempted it (CodeQL, tree-sitter, Glean, Kythe). What is shared in practice is small: definitions,
references, locations (LSIF, SCIP, Kythe, codemarkup). Full expression semantics stays per language.

Lesson: for the new kernel, "Go, Python, Rust, C under one numeric space" should mean one node store and one grammar
in which *each host language is a namespace of blueprints* (its own constructs, kept precise), plus derived,
explicitly witnessed equivalences between them. Promising one universal construct set is the path that failed. The
NUMS.Go design (language id as a field, tree-sitter in front, symbols in a side table) is consistent with the
survivors. [I from the observed items above plus the NUMS.Go reading in section 0]

### 2.6 Multi-language IRs and semantic frameworks

- LLVM IR: three forms, in-memory, bitcode, and readable assembly, and "The three different forms of LLVM are all
  equivalent"; SSA-based; a "'universal IR' of sorts". [O] https://llvm.org/docs/LangRef.html Unnamed values are
  numbered (`%12`). [O, same page, Identifiers section as relayed]
- MLIR: built because a single low-level IR forces a backend to "lift and reconstruct loops"; "modular and pluggable
  dialects" each defining operations, types and invariants; "progressive lowering"; a generic textual form that
  "serves as a safe fallback" even when a dialect is not loaded, plus optional custom forms per operation. [O]
  https://mlir.llvm.org/docs/Rationale/Rationale/
- WebAssembly: fast, safe, well-defined, hardware-, language- and platform-independent, compact; core spec
  "version 3.0 (2026-11-23)". [O] https://webassembly.github.io/spec/core/intro/introduction.html That binary and text
  are two concrete syntaxes of one abstract syntax is [A] (the introduction page did not say it).
- GraalVM Truffle: languages are "interpreters for self-modifying Abstract Syntax Trees"; high-performance code is
  derived "automatically" via partial evaluation; JavaScript, Python, WebAssembly and Java run on it; GraalVM 25.x
  documentation. [O] https://www.graalvm.org/latest/graalvm-as-a-platform/language-implementation-framework/ That this
  is the first Futamura projection and that warm-up and memory are its known costs is [A].
- K framework: "a rewrite-based executable semantic framework" with configurations of "labeled, nestable cells". [O]
  https://kframework.org/ Current semantics: C (2010-), WebAssembly (2015-), EVM (2017-), Plutus-Core, IELE,
  Michelson, Algorand. Archived: LLVM (2011-2018), Python (2012-2013), Java (2012-2016), OCaml, JavaScript
  (2013-2015), JVM, x86-64 (2017-2020), Solidity, and others. [O] https://kframework.org/projects/
- Racket: "a macro can only extend a language, and it can do so only at the expander layer"; separate facilities
  extend "the reader layer" and package "a reader and expander starting point into a conveniently named language". [O]
  https://docs.racket-lang.org/guide/languages.html Rosette (9.3 docs) is a solver-aided language built that way. [O]
  https://docs.racket-lang.org/rosette-guide/index.html
- Haxe: one statically typed source, 13 listed targets in three maintenance tiers; string encoding has
  "target-specifics". [O] https://haxe.org/documentation/introduction/compiler-targets.html

Lessons:

1. MLIR is the closest structural precedent for "one grammar, no second dialect": one node shape (operation, operands,
   regions, attributes), a generic rendering that always works, and namespaces of blueprints at different altitudes
   that lower into each other. Form-plus-BML can become one grammar the same way: BML's higher constructs are
   blueprints that lower to core ones inside the same node space, and every node has a generic printed form. [I]
2. LLVM and WebAssembly show the lens precisely: numeric identity inside, names optional, text and binary equivalent.
3. K's project list is a caution: full formal semantics of large surface languages were written and then archived
   within a few years; the ones still maintained are small virtual machines and C. Maintaining complete semantics
   for Go, Python, Java and friends inside the kernel is not sustainable by a small team; carrying their *structure*
   and delegating execution to the host toolchain through an offered interface is. [I from the K list]
4. Truffle proves "write the interpreter in the framework, get machine code by specialising it" at production
   quality, which is the same shape as "a bootstrap that replaces itself with JIT code written in the one grammar".
   Its runtime is large [A]; the kernel's bet is that the same effect is reachable with far less.

### 2.7 Minimal executable cores

- Interaction nets (Lafont 1989) and interaction combinators (Lafont 1997): three agents (eraser, duplicator,
  constructor); universal for interaction systems; strong confluence, so "no synchronization is needed"; each rule is
  local and constant-time; but nets are "essentially deterministic and cannot model non-deterministic computations
  directly" without an added `amb` agent (Fernandez and Khalil 2003). [O] https://en.wikipedia.org/wiki/Interaction_nets
- HVM1 (2022): "HVM is still to be considered a prototype"; single-thread roughly 50% faster to 3x slower than GHC;
  some terms evaluate under different semantics than the lambda calculus; a "Q1 2024" production target is outdated.
  [O] https://github.com/HigherOrderCO/HVM
- HVM2: "a massively parallel Interaction Combinator evaluator", an IR compiled to C and CUDA; CUDA is "less stable".
  [O] https://github.com/HigherOrderCO/HVM2
- Bend, as the repository reads in 2026: "Bend 2", not compatible with Bend 1 or HVM; targets C, Metal, CUDA,
  JavaScript; values are affine; numbers are Nat, U32, F32; strings are linked lists; the compiler is young and
  "99% AI-written". [O] https://github.com/HigherOrderCO/Bend That Bend 1 offered two surface syntaxes over one core
  is [A]; the current page shows one.
- Forth 2012: the dictionary has "a name space, a code space, and a data space"; names are searched to find a
  definition; "Different definitions may have the same execution token if the definitions are equivalent". [O]
  https://forth-standard.org/standard/usage
- Lisp-family homoiconicity: the term dates to TRAC (1965); there is "no consensus" precise definition; one critic
  calls "syntax is a data structure" a category error. Lisp, Rebol, Red, Prolog, Wolfram are the listed examples. [O]
  https://en.wikipedia.org/wiki/Homoiconicity
- Rebol: "Dialects are sub-languages of REBOL that use the same lexical form for all datatypes"; "the parser rules
  themselves are specified as a dialect" (2005). [O] https://www.rebol.com/docs/core23/rebolcore-15.html
- Wolfram Language: "everything you type into the Wolfram Language is treated as an expression"; head and parts;
  FullForm shows the internal structure and printed forms "do not always correspond directly". [O]
  https://reference.wolfram.com/language/tutorial/Expressions.html
- BQN: chosen over APL for "fewer and less overloaded primitives"; a context-free grammar is listed as a major
  difference. [O] https://mlochbaum.github.io/BQN/commentary/why.html Dense glyph notations as an example that the
  symbol set itself is a lens over array operations is [I]; "Notation as a Tool of Thought" is [A].
- Smalltalk images (the running object memory is the source of truth, text is an export) are [A].

Lessons:

1. A three-rule core is enough for universality, so "minimal bootstrap" is not the risk. The risk is the distance
   from that core to hardware speed, which is where HVM has spent years and several incompatible rewrites. [I]
2. Interaction combinators are deterministic by construction. BML's choice, fail and stop are not. A core chosen for
   the new grammar must have non-determinism (or at least committed choice with backtracking) as a primitive, or the
   reference features become an encoding on top of the core instead of the core. [I from the interaction-nets page]
3. Forth separates name space from code space and identifies behaviour by token; Rebol and Racket show that
   "dialects" can be values of the one language (a block interpreted by a rule set; a reader plus expander) instead
   of a second grammar. That is the precedent for folding BML into one grammar without losing its surface. [I]
4. "Homoiconic" is not a property to claim; "the stored tree is the program and every printed form is derived" is
   the precise version, and LLVM, MPS and Wolfram state it that way.

### 2.8 Projectional and structure editors — when the tree is the source

- MPS: "The editor in MPS directly manipulates AST"; no grammar or parser to define; concepts are themselves AST
  nodes; because every element has an id, a move can be told from delete-plus-create; "it takes a few days for most
  users to become accustomed". [O] MPS 2026.2 help pages cited in 2.4.
- Hazel: "there are no meaningless editor states"; holes mark missing or conflicting parts so every incomplete
  program has a type and a result; tylr adds keyboard-driven "gradual structure editing"; Grove (POPL 2025) is a
  structure-editor calculus "built on commutative edit actions" for collaboration; work continues through POPL 2027.
  [O] https://hazel.org/
- Darklang: the structural editor was dropped (post dated 2024-03-12) because it was "disjointed from our users'
  'normal' development flows", "pretty buggy" so users could not "type the code they wanted", and "~50% of our
  codebase"; replaced by a language server, existing editors and a tree-sitter parser. [O]
  https://blog.darklang.com/an-overdue-status-update/
- Lamdu: twelve-plus years as a "technology preview". [O] https://www.lamdu.org/
- Dion: unreachable (DNS). [O that it is unreachable]; anything about it is [A].

Lesson: "the tree is the source" survives when text stays a full citizen (Unison's scratch files [A], Darklang's
return to text [O], tylr's text-like editing [O]). It fails when the tree is editable *only* through a bespoke
editor. For a body whose main writers are models and command-line tools, the required shape is: node space is the
truth; text is a lens with a total printer and a partial parser; and incomplete input has a first-class
representation. Hazel's hole is that representation, and it lines up with axiom 1: an unfilled position is
`nothing`, the program still has a shape, and it can still be offered. [I]

### 2.9 Content hashing outside code

- Nix: input-addressed outputs depend on the whole derivation; content-addressed ones "only" on name, data and
  method, so a change that "cannot have a real effect" stops forcing "massive rebuilds". Self-references need a
  provisional path, "an impurity, since it is an arbitrary choice". Floating content addressing is still experimental
  (`ca-derivations`). [O] https://nix.dev/manual/nix/latest/store/derivation/outputs/content-address
- CUE: "`bool` is just as much a value as `true` and `false`"; every value sits in one lattice with top `_` and
  bottom `_|_` ("an instance of every value"); unification is "the greatest lower bound". [O]
  https://cuelang.org/docs/concept/the-logic-of-cue/

Lesson: Nix distinguishes two identities that the kernel will also have: the identity of a *recipe plus its inputs*
(what was asked) and the identity of the *result* (what came out). Early cutoff comes only from the second. A cell
that contains its own address is the hard case in both Nix and Unison; decide the rule once. CUE's lattice is taken
up in section 4. [I]

## 3. Natural-language side

### 3.1 Grammatical Framework — one abstract syntax, many concrete syntaxes

- Abstract syntax defines "what meanings can be expressed in the grammar"; each concrete syntax maps them to strings;
  parsing "takes a string into an abstract syntax tree"; translation is parse then linearise; resource grammars give
  "the grammatically correct combinations of words, whatever their meanings are" while application grammars carry the
  domain. [O] https://www.grammaticalframework.org/doc/tutorial/gf-tutorial.html
- GF 3.12 released 2025-08-08; the Resource Grammar Library lists 46 languages, some partial; adding a language
  "takes 3 to 9 months"; machine translation is "usually applied for limited fragments of language". [O]
  https://www.grammaticalframework.org/
- Ranta, Angelov, Gruzitis, Kolachina, "Abstract Syntax as Interlingua", Computational Linguistics 46(2), 2020: GF
  began in 1998 for controlled languages; the last decade's focus was "scaling up GF to wide-coverage language
  processing" by linking to UD, WordNets, FrameNets, AMR. [O] https://aclanthology.org/2020.cl-2.6/

Proves: within a defined abstract syntax, one stored tree renders correctly, with agreement and word order, in dozens
of languages, and the same machinery is a compiler front end (the abstract's own comparison). This is the cleanest
working instance of the thesis for natural language.

Limits: the symbol lens is not a label table. A concrete syntax is a *program* per language (records, tables,
inflection), months of expert work each. Wide coverage needed statistical help. Parsing is ambiguous.

Lesson: an NL lens in the new kernel is a set of linearisation recipes written in the one grammar, not a dictionary
attached to nodes. The dictionary (3.2) feeds it. The pair is asymmetric: linearise is total, parse is a choice.

### 3.2 Wikidata, lexemes, Wikifunctions, Abstract Wikipedia (status through November 2026)

- Wikidata: items "uniquely identified by a `Q` followed by a number", properties by `P`; "Data entered in any
  language is immediately available in all other languages". [O] Wikidata:Introduction, last edited 2026-10-21.
- Lexemes: `L` ids with forms (`L…-F…`) and senses (`L…-S…`); a sense links to a concept through "item for this
  sense (P5137)"; modelling practice varies by language. [O]
  https://www.wikidata.org/wiki/Wikidata:Lexicographical_data/Documentation (last edited 2026-04-12).
- Wikifunctions: functions with multilingual descriptions, tests, and implementations "in different programming
  languages" that can call each other; all functions are pure. [O]
  https://www.wikifunctions.org/wiki/Wikifunctions:About (last edited 2026-07-13). ZObjects are "basically abstract
  syntax trees"; a normal form for the evaluator and a canonical form for storage "have the same meaning". [O]
  Function_model page. A status-update headline dated 2026-09-03 reads "5,000 Functions, passing Z40000". [O]
  https://www.wikifunctions.org/wiki/Wikifunctions:Status_updates
- Abstract Wikipedia: content "in a language-independent way", rendered by Wikifunctions from Wikidata; timeline:
  approved May 2020, Wikifunctions production July 2023, embedded calls April 2025, first sentence-generating
  functions late 2025, "soft launch" 2026 with Malayalam, Dagbani and Igbo pilots. [O]
  https://meta.wikimedia.org/wiki/Abstract_Wikipedia (last edited 2026-09-29).
- 2022 evaluation by a group of Google.org fellows: Wikifunctions' scope is wider than the task needs; it "invents a
  completely new ad-hoc system"; a pure-functional model is a poor fit for generation that needs shared state; they
  recommended decoupling, defining the abstract content specification first, and using an existing system "such as
  Grammatical Framework". [O] https://meta.wikimedia.org/wiki/Abstract_Wikipedia/Google.org_Fellows_evaluation
- A request for comment opened 2026-07-14 and still open: a sample of 13 random abstract pages found four showing
  only internal errors, one failing after two sentences, one showing raw HTML, and none acceptable as an article; it
  proposes pausing rollout, publishing success and termination criteria, or archiving. The team replied on 2026-08-20
  that the system is in early beta, that many cited errors were fixed, that the composition interface is too
  complicated, and that each language community will decide on opt-in. Commenters raised that concepts do not map
  cleanly across languages (one word for clock and watch in Russian), that interlingual machine translation has
  decades of mixed results, and that the payoff requires renderer functions and lexemes per language. [O]
  https://meta.wikimedia.org/wiki/Requests_for_comment/The_future_of_Abstract_Wikipedia (first 200,000 of about
  356,000 characters read; this is a live dispute and a one-sided opening statement plus replies, not a neutral
  evaluation).

Proves: language-neutral numeric identity for *things and relations* with labels in hundreds of languages works at
planetary scale (Wikidata). A function library keyed by numbers with labels in 300+ languages exists and runs
implementations in several programming languages against shared tests (Wikifunctions). This is, structurally, the
thesis: Z-objects are a numeric node space with a label lens, executing.

Where it hit limits: the step from labelled nodes to *sentences* is where six years of funded work stands contested
in 2026. The costs named by both critics and team are per-language grammar functions, per-language lexeme coverage,
tooling that humans cannot read fluently, and no demonstrated case of one contribution improving many languages.
Also, by the system's own documentation, the labelled view is not invertible.

Lessons:

1. Reuse rather than re-mint: Q/P/L ids are an existing, open, multilingual numeric vocabulary. A kernel node can
   carry a Wikidata id as one of its names (axiom 3: names are free query keys) without adopting Wikidata's identity
   model. [I]
2. Wikifunctions' "one function, many implementations, shared testers" is a working precedent for "a recipe's
   blueprint is its interface plus witnesses; implementations in Go, C or the one grammar are interchangeable cells
   behind it" (axiom 4: the interface is itself a cell). [I]
3. The 2022 evaluation's core criticism is aimed at exactly the move this project is making: building a new general
   programming system as the precondition for a language goal. The honest response is to keep the two bets separable
   so the kernel's value does not depend on the natural-language claim. [I]

### 3.3 Meaning representations: AMR, UMR, Universal Dependencies, FrameNet, UNL

- AMR 1.2.6 (2019-05-01): one graph "may represent any number of natural language sentences"; it is "geared toward
  English and the vocabulary of English" and "is not an interlingua"; it "drops grammatical number, tense, aspect,
  quotation marks, etc.". [O] https://github.com/amrisi/amr-guidelines/blob/master/amr.md
- UMR 0.9 (2022-08-08): "a cross-lingual annotation framework with a shared set of abstract concepts and relations";
  to serve languages with fewer resources "other annotation domains are structured as lattices" (finer relations
  under a general one); languages without frame files use general roles (`:actor`, `:undergoer`, `:theme`); adds
  document-level coreference, temporal and modal dependencies, scope, aspect. [O]
  https://raw.githubusercontent.com/umr4nlp/umr-guidelines/master/guidelines.md Project activity through 2025
  (summer school June 2025). [O] https://umr4nlp.github.io/web/
- Universal Dependencies: release 2.19 dated 2026-11-15, "over 200 treebanks", "over 150 languages". [O]
  https://universaldependencies.org/ Design: "a universal inventory of categories and guidelines" with
  "language-specific extensions when necessary"; six competing goals, and "It's easy to come up with a proposal that
  improves UD on one of these dimensions". [O] https://universaldependencies.org/introduction.html
- FrameNet (1997-): frames, frame elements, lexical units; sister FrameNets for Spanish, Japanese, German, Polish,
  Chinese, Swedish and others, built on the original. [O] https://en.wikipedia.org/wiki/FrameNet How well English
  frames transfer is not stated there; that transfer is partial is [A]. VerbNet and PropBank: [A] only.
- UNL (1996, United Nations University; UNDL Foundation from 2001): hypergraphs of "universal words" with
  English-based headwords, 46 relations, attributes; the article shows archived links and no current activity. [O]
  https://en.wikipedia.org/wiki/Universal_Networking_Language (flagged there for sourcing problems).

Proves: a shared category inventory with per-language extension is sustainable for *syntax* at the scale of 150+
languages (UD), and meaning graphs abstract over paraphrase within a language (AMR).

Limits: the most used meaning representation says of itself that it is not an interlingua. The one built to be
cross-lingual (UMR) got there by giving up a single fixed granularity. UNL, the explicit universal pivot with English
headwords, did not reach adoption.

Lesson: UMR's lattice is the mechanism the kernel needs for cross-language mismatch. A concept node sits in a
subsumption order; a language's lens attaches symbols at the level it actually lexicalises; translation between two
lenses may move up the lattice and lose detail, and that loss is observable, not hidden. This is content-addressable:
a finer node references the coarser one. Do not label pivot nodes with one language's words (UNL, AMR); UD shows the
pattern that lasts, a small universal core plus named extensions per language. [I]

### 3.4 Lexical inventories: WordNet, CILI, BabelNet

- A synset is "a group of words that can all refer to the same concept"; wordnets in many languages are "all linked
  through the Collaborative Interlingual Index (CILI)" with ids like `i69570`; some relations exist because "English
  does not really use this relationship, but it is common in many languages". [O]
  https://globalwordnet.github.io/gwadoc/ CILI (Bond, Vossen, McCrae, Fellbaum, 2016) is "designed to make possible
  coordination between multiple loosely coupled wordnet projects". [O] https://aclanthology.org/2016.gwc-1.9/
- BabelNet 6.0: about 21 million entries, 3,086 languages, built by automatic integration of WordNet, Wikipedia,
  Wikidata, Wiktionary and others (now with an LLM-assisted pipeline); non-commercial research licence. [O]
  https://babelnet.org/about

Lesson: a language-neutral numeric index of word senses exists twice over; the open one (CILI) is small and
federated, the large one is licence-restricted. The kernel does not need to mint a sense inventory; it needs a place
for such ids as names on nodes, and a rule that an external id is a name and never the identity. [I]

### 3.5 Designed universals: Lojban, Natural Semantic Metalanguage, Montague

- Lojban (1987; baseline 1997): grammar "tested to be unambiguous using computers"; the claim covers "standard
  syntactic (not semantic) constructs"; compounds remain ambiguous; root words drawn algorithmically from six
  languages; a very small speaker community. [O] https://en.wikipedia.org/wiki/Lojban
- NSM: 65 primes (from 14 in 1972), claimed to "have the same translation in every language"; explications are
  reductive paraphrases in primes; semantic molecules are intermediate. [O]
  https://en.wikipedia.org/wiki/Natural_semantic_metalanguage The universality of at least one prime (HAVE PARTS) is
  recurrently contested, as proponents' own reviews acknowledge. [O, search-result summaries of
  https://journals.rudn.ru/linguistics/article/view/26795; article not fetched]
- Montague: "no important theoretical difference" between natural and formal languages; "Syntax is an algebra,
  semantics is an algebra, and meaning is a homomorphism between them"; limits: indexicals and context, logically
  equivalent sentences collapsing to one meaning (hyperintensionality), cross-sentence anaphora, idioms. [O]
  https://plato.stanford.edu/entries/montague-semantics/ (first published 2011-11-07, revised 2025-02-05).
- Linguistic relativity: weak effects of language on processing are supported; the entry rejects any "general or
  global ineffability problem", since speakers coin words or use workarounds. [O]
  https://plato.stanford.edu/entries/linguistics/whorfianism.html

Lesson: Montague is the strongest theoretical ally of the thesis (one algebraic treatment for both kinds of
language), and his homomorphism is exactly "lens = structure-preserving map from the node algebra to a surface
algebra". The limits listed are the things a homomorphism from sentence structure cannot carry: who is speaking,
when, and what was said before. In the kernel's terms these are argument cells of the offer (axiom 5), not properties
of the sentence node. Lojban shows that removing syntactic ambiguity leaves semantic ambiguity untouched. NSM offers
65 candidate ground concepts that could be tried as seed nodes; they are a contested hypothesis, not a floor. The
relativity literature supports "everything can be said in every language, with workarounds", which is what a lens
with paraphrase fallback needs, and does not support "one word per node in every language". [I]

## 4. The absent-value lineage and the state set {0, 1, nothing}

- Kleene's strong three-valued tables: the middle value was "meant to represent a situation where the output of a
  partial recursive function is undefined"; in the constant-free fragment no sentence is valid. Lukasiewicz used a
  third value for possibility (future contingents). Bochvar's/weak Kleene's third value is a "non-sensical or 'junk'
  value" that infects any compound. The same tables support two readings of the middle value, "Neither True nor
  False" (Kleene, Kripke) and "Both True and False" (Priest), with different consequence relations. [O]
  https://plato.stanford.edu/entries/logic-manyvalued/ (first published 2026-01-17, revised 2026-09-29).
- SQL NULL: a marker for absent data; comparisons give Unknown and WHERE keeps only True; one NULL stands for both
  "missing" and "inapplicable"; Codd's 1990 proposal of two markers (and four-valued logic) was not adopted; known
  traps include `= NULL`, `<> 1` silently dropping rows, aggregates skipping NULLs, and UNION treating NULLs as equal
  against the comparison rules; Date and Darwen call it "inherently flawed". [O]
  https://en.wikipedia.org/wiki/Null_(SQL)
- CUE: a single lattice with top (anything) and bottom (conflict). [O] section 2.9.
- Hazel: holes give incomplete programs a type and a result. [O] section 2.8.
- Option/Maybe (absence made a typed value so it cannot be confused with a member), domain-theory bottom
  (non-termination as the least element), LLVM `undef`/`poison` (deferred undefined behaviour): [A].

Reading against the axioms:

1. Axiom 1 gives `nothing` several roles at once: "the shared potential field", "Timeout is nothing", and "Choosing
   nothing is a first-class option" (core-axioms.form line 48). [O] In the lineage these are distinct things: Kleene's
   undefined (no answer arrived), an explicit decline, and unconstrained potential. SQL is the worked example of what
   happens when one marker carries two readings and each operator picks its own. [I]
2. The axioms file already contains the repair, as theorems: "nobody-answered IS-NOT everybody-said-no" (line 215)
   and held-offer, "silence may mean NOT-YET" (line 176). [O] The lesson from SQL and Codd is to make that
   distinction *observable beside* the state (a census cell, a reason cell), and not to grow the state set to four
   or five values. [I]
3. Lattice position is unsettled and matters. In CUE the least specific value is top and conflict is bottom. Axiom 1
   describes `nothing` as the field "from which blueprints and recipes arise", which reads as top (unconstrained
   potential), while BML's `fail` reads as bottom (no value satisfies this). If the one grammar gives `nothing` and
   `fail` the same representation, it repeats NULL. If it keeps them apart, unification-style composition (order
   independent, as CUE states) becomes available. [I; this is an open question, section 8]
4. Propagation must be one rule. Strong Kleene (a definite operand can decide the result despite an absent one) and
   weak Kleene (absence infects everything) are both coherent; mixing them per operator is what SQL did. [I]
5. Hazel's hole and axiom 1 agree: an incomplete cell is still a cell that can be typed, offered and run. That is a
   concrete, published semantics for "pending is honest". [I]

## 5. Cross-cutting findings

### 5.1 Three identity schemes, and where each one wins

| scheme | examples [all O above] | wins at | loses at |
|--------|------------------------|---------|----------|
| computed from content | Unison, Dhall, Nix content-addressed outputs, Glean's deduplicated facts | no registry, free deduplication, permanent caches, safe sharing between strangers | "the same thing after an edit"; cycles and self-reference; anything nominal; hashing cost per node |
| assigned by a registry | Wikidata Q/P/L, Wikifunctions Z, MPS node ids, Scratch block and variable ids, Excel function codes, LSIF vertex ids | stable under edits; cheap; short | needs an authority or a counter; two parties cannot mint the same id independently |
| descriptive vector | Kythe VName, LSIF monikers, SCIP symbols | independent tools agree on the name of an *external* thing without coordination | long; only as stable as the thing's coordinates |

Axiom 3 commits to the first row. Every large multilingual label system sits in the second row. This is not a
contradiction but it is a design obligation: the kernel needs the second and third as *names over* the first. A
"named cell" (trinity, line 102) is the mutable pointer that gives continuity across edits; an external coordinate
(a Go function at a path in a repository, a Wikidata item) is a descriptive name. The node id stays the hash.
Unison is the existence proof that this layering works. [I]

### 5.2 The lens is asymmetric

Node to text is a function. Text to node is a search. Evidence [O]: Wikifunctions (labelized forms cannot always be
converted back), GF (parsing is ambiguous; translation is parse then linearise), AMR (one graph, many sentences),
Lojban (syntax unambiguous, meaning not), Excel (pasted text is not a formula). For programming languages the
inverse is tractable because grammars are designed to be unambiguous; for natural language it is not. A design that
treats both directions as "just swap the symbols" will be wrong in the second direction. BML's choice and fail are
the right primitives for it: a parse yields a choice among candidate nodes, or nothing. [I]

### 5.3 Shared substrate yes, shared schema no

Section 2.5 and 2.6: the systems that last share storage, identity and a query or rewrite language, and keep
constructs per language (dialects in MLIR, schemas in CodeQL and Glean, grammars in tree-sitter, concrete syntaxes in
GF, language-specific extensions in UD). The systems that tried one shared construct inventory narrowed (LSIF to
navigation facts), abandoned it (semantic), or stalled (UNL). [I from those observed items]

### 5.4 What actually took the time

Unison: seven years to 1.0, and the store format changed twice (2021, 2026). Abstract Wikipedia: six years to a
contested soft launch. GF: three to nine months per language. K: most large-language semantics archived. Darklang:
half the codebase in an editor that was then removed. HVM: three incompatible generations. In each case the core
idea worked early and the time went into the edge with the existing world: foreign functions, version control,
editors, per-language grammar, performance on real hardware. [I from the observed dates]

## 6. Design recommendations

Each is a proposal resting on the findings above; none is verified by building.

R1. **Identity = hash of the name-free structure** (Unison shape): binders by index or level, free references by
node id, names excluded. Fix the cycle rule and the self-reference rule in the first design round. Choose the hash
width deliberately: Unison uses 512 bits; the efficiency goal argues for a short in-memory handle with the full hash
only at the storage and exchange boundary (assumption to be measured).

R2. **Names are a side table per lens**, keyed by (node id, lens id) with zero or more entries. A lens id is itself a
node (a human language, a programming-language surface, a house style). User-chosen names are entries in that table
too, so they can be translated or left untranslated visibly. This is NUMS.Go's symbol table made multilingual.

R3. **A lens is a pair of recipes in the one grammar**: `linearise` (total) and `parse` (a choice that may yield
several nodes or nothing). Labels alone are the degenerate lens that suffices for keywords and opcodes.

R4. **One grammar by the MLIR pattern**: one node shape, one generic printed form that always round-trips, and
namespaces of blueprints. What BML adds over Form (choice, fail, stop, store, templates, generics, inheritance,
objects) becomes higher-altitude blueprints that lower to core ones in the same space. Surface sugar is a lens.

R5. **Host languages are blueprint namespaces, not a universal AST.** Import Go, C, Python as their own precise
constructs (tree-sitter in front, as NUMS.Go does). Record cross-language equivalences as explicit, witnessed
relation cells. Do not write full semantics for host languages; reach their toolchains through offered interfaces.

R6. **Keep three identities distinct**: the blueprint (content), the named cell (a pointer that moves; undo is
repoint), and the external coordinate (Kythe-style vector, Wikidata id). Never let an external id be the identity.

R7. **Concept nodes live in a lattice of granularity** (UMR, WordNet hypernymy, CUE). A natural-language lens
attaches words at the level that language lexicalises; crossing lenses may generalise, and the generalisation step is
recorded.

R8. **Context is an argument cell of the offer.** Speaker, time, discourse and locale are passed, never baked into
the sentence node (Montague's known limits; Excel's embedded format codes).

R9. **Incomplete is first class.** A hole is `nothing` in a typed position; every state of the node space can be
printed, hashed and offered (Hazel). Text stays a full editing lens (Darklang's lesson).

R10. **One `nothing`, one propagation rule, reasons beside it.** Keep the state set at three. Decide whether
`nothing` is lattice top and `fail` lattice bottom. Carry "why nothing" (timeout, declined, unreferenced, held) as an
optional observable cell, as the absence-is-not-a-verdict theorem already does.

R11. **Two-level recipe identity** (Nix): the id of recipe-plus-arguments and the id of the result are both kept, so
unchanged results cut recomputation short.

R12. **The store is a compact database, not a file per node**, from the start (Unison's 100x). The repository holds
the bootstrap and, at most, an exported seed.

R13. **Keep the bets separable.** Order: (a) content-addressed node space with name lenses for the kernel's own
vocabulary; (b) host programming languages as namespaces; (c) natural-language lenses for controlled fragments (GF
scale); (d) open natural language, where learned models, not grammars, currently do the work. Each stage has value
without the next. State (d) as a direction, with the 2026 Abstract Wikipedia record as its current limit.

R14. **Borrow open numeric vocabularies** (Wikidata Q/P/L, CILI, UD categories) as names on nodes instead of minting
parallel ones.

## 7. The three strongest counter-arguments to the whole thesis

**C1. "Numeric root" is either trivially true or unproven.** Any formal object can be numbered, and the numbering is
arbitrary (2.2). So "one numeric space underlies all languages" says nothing until it means "one space in which
*equal meanings get equal or relatable identities across languages*". On that reading the industrial record is
against it: the one team that built shared cross-language syntax types replaced them with per-language types and the
project is archived (2.5); CodeQL, Glean, tree-sitter keep schemas per language; K's semantics for large languages
were archived (2.6). Content hashing makes it sharper: two programs with the same behaviour and different structure
get different ids, and behavioural equivalence is undecidable in general, so structure-identity never reaches
meaning-identity. NUMS.Go working for Go shows the encoding, which was never in doubt; it does not show the shared
meaning. A fair defender must say which reading is claimed.

**C2. Natural language is not a lens over a fixed meaning space, and the working alternative does not use one.** The
field's own instruments say so: AMR "is not an interlingua"; UNL's universal words are English headwords and the
project went quiet; UMR had to make granularity a per-language choice; NSM's primes are contested even at 65; Lojban
removed syntactic ambiguity and kept semantic ambiguity; the largest funded attempt at "abstract content plus
per-language renderers" is, in November 2026, the subject of an open request to pause or archive it, after a sample
of 13 pages found none acceptable (3.2). Meaning in use depends on speaker, time and prior discourse (3.5), which no
node for a sentence contains. Meanwhile translation and paraphrase between hundreds of languages is done well today
by learned models with no explicit interlingua [A for the quality claim]. So the evidence suggests the bridge
between languages is a learned, continuous, context-dependent mapping, and a discrete symbolic node space is the
wrong shape for it, however elegant.

**C3. The costs of leaving text and names have repeatedly outweighed the benefits, and the efficiency goal makes it
worse.** The ecosystem (version control, review, search, editors, every model's training data) is text with names.
Projects that made the tree the source paid for it: Unison needed seven years, two store formats and still lists a
C FFI as future work; Darklang spent half its codebase on an editor and then removed it; MPS needs special merge
support and remains a niche; Lamdu is a preview after more than a decade (2.1, 2.8). Content identity also has
structural costs: every edit re-identifies all dependents, cycles and self-reference need special rules, mutable and
external things (files, devices, time) are not content, and hashing every node spends exactly the CPU and memory the
project wants to save. A critic can add that a kernel which is simultaneously a new language, a new store, a new JIT
and a theory of natural language multiplies four risks, which is the same objection the 2022 evaluation raised
against coupling Abstract Wikipedia to Wikifunctions.

What each counter-argument does not defeat: C1 leaves T2 and T3 standing (identity by content and vocabulary lenses
work). C2 leaves controlled-fragment rendering standing (GF) and says nothing against programming languages. C3 is
about cost and sequencing, which R12 and R13 address, and Unison 1.0 shows the cost can be paid.

## 8. Open questions this study could not settle

1. Is `nothing` the top of a value lattice (unconstrained potential) or the bottom (no value), and is `fail` the
   same state or a different one? The axioms' wording suggests top; the BML reference behaviour was not examined here.
2. Hash of structure or hash of a normal form? Normal form gives more equalities (Dhall) but needs totality.
3. How do nominal distinctions, effects and host resources enter a content-addressed space (Unison's salt and
   abilities are [A] here)?
4. What is the measured cost of hashing at node granularity versus recipe granularity on the target hardware?
5. Can a user-chosen identifier be a translatable label in practice, or will it stay an untranslated string as in
   Scratch and Excel?
6. Does one contribution to a concept lattice ever improve many language lenses at once? Abstract Wikipedia's
   commenters say no example exists yet; GF's functor pattern suggests yes inside a domain.
7. What exactly did Babelfish/UAST, Dion and SCIP's design notes conclude? All three were unreachable today.
8. What is the present state of the interaction-combinator runtime line after Bend 2's break with HVM?
9. Do interaction-net style cores admit choice and backtracking without losing strong confluence, or must the core
   be a different calculus?
10. How far does the Wikifunctions pattern (many implementations behind one function id, shared testers) carry to
    JIT-generated machine code as "one more implementation"?

## 9. Source index (all fetched 2026-11-24 unless marked)

| source | own date | used in |
|--------|----------|---------|
| https://www.unison-lang.org/docs/the-big-idea/ | (c) 2026, undated | 2.1 |
| https://www.unison-lang.org/docs/language-reference/hashes/ | undated | 2.1 |
| https://www.unison-lang.org/unison-1-0 | Nov 2025 | 2.1 |
| https://www.unison-lang.org/docs/tooling/project-workflows/ | undated | 2.1 |
| https://plato.stanford.edu/entries/goedel-incompleteness/ | 2013-11-11, rev. 2025-10-08 | 2.2 |
| https://en.wikipedia.org/wiki/De_Bruijn_index | undated | 2.3 |
| https://chargueraud.org/research/2009/ln/main.pdf (pages 1-3) | manuscript, 2009+ | 2.3 |
| https://github.com/dhall-lang/dhall-lang/blob/master/standard/README.md | undated | 2.3 |
| https://office-watch.com/2026/excel-formulas-different-languages-how-it-works/ | 2026-05-31 | 2.4 |
| https://en.scratch-wiki.info/wiki/Scratch_File_Format ; .../Translating_Scratch | undated | 2.4 |
| https://en.wikipedia.org/wiki/Non-English-based_programming_languages | undated | 2.4 |
| https://www.jetbrains.com/help/mps/basic-notions.html ; .../mps-faq.html | 2026-08-06 (MPS 2026.2) | 2.4, 2.8 |
| https://www.lamdu.org/ | v0.8.1, undated | 2.4, 2.8 |
| https://hedy.org/ | undated | 2.4 |
| https://tree-sitter.github.io/tree-sitter/ (+ static node types page) | undated | 2.5 |
| https://github.com/github/semantic ; .../docs/codegen.md | archived 2025-04-01 | 2.5 |
| https://codeql.github.com/docs/codeql-overview/about-codeql/ | undated | 2.5 |
| https://glean.software/docs/introduction/ | undated | 2.5 |
| https://kythe.io/docs/kythe-overview.html ; .../kythe-storage.html | undated | 2.5 |
| https://microsoft.github.io/language-server-protocol/specifications/lsif/0.6.0/specification/ | v0.6.0 | 2.5 |
| https://github.com/sourcegraph/scip | undated | 2.5 |
| https://www.srcml.org/about.html | undated | 2.5 |
| https://github.com/bblfsh/bblfshd | undated | 2.5 |
| https://llvm.org/docs/LangRef.html (first 100k chars) | living doc | 2.6 |
| https://mlir.llvm.org/docs/Rationale/Rationale/ | undated | 2.6 |
| https://webassembly.github.io/spec/core/intro/introduction.html | v3.0 draft 2026-11-23 | 2.6 |
| https://www.graalvm.org/latest/graalvm-as-a-platform/language-implementation-framework/ | GraalVM 25.x | 2.6 |
| https://kframework.org/ ; https://kframework.org/projects/ | undated | 2.6 |
| https://docs.racket-lang.org/guide/languages.html ; .../rosette-guide/index.html | Racket 9.3 docs | 2.6 |
| https://haxe.org/documentation/introduction/compiler-targets.html | undated | 2.6 |
| https://en.wikipedia.org/wiki/Interaction_nets | undated | 2.7 |
| https://github.com/HigherOrderCO/HVM ; /HVM2 ; /Bend | (c) 2026, undated | 2.7 |
| https://forth-standard.org/standard/usage | Forth 2012 | 2.7 |
| https://en.wikipedia.org/wiki/Homoiconicity | undated | 2.7 |
| https://www.rebol.com/docs/core23/rebolcore-15.html | 2005-04-15 | 2.7 |
| https://reference.wolfram.com/language/tutorial/Expressions.html | undated | 2.7 |
| https://mlochbaum.github.io/BQN/commentary/why.html | undated | 2.7 |
| https://hazel.org/ | news through Nov 2026 | 2.8, 4 |
| https://blog.darklang.com/an-overdue-status-update/ | 2024-03-12 | 2.8 |
| https://nix.dev/manual/nix/latest/store/derivation/outputs/content-address | living doc | 2.9 |
| https://cuelang.org/docs/concept/the-logic-of-cue/ | undated | 2.9, 4 |
| https://www.grammaticalframework.org/ ; .../doc/tutorial/gf-tutorial.html | GF 3.12, 2025-08-08 | 3.1 |
| https://aclanthology.org/2020.cl-2.6/ | 2020 | 3.1 |
| https://www.wikidata.org/wiki/Wikidata:Introduction | edited 2026-10-21 | 2.4, 3.2 |
| https://www.wikidata.org/wiki/Wikidata:Lexicographical_data/Documentation | edited 2026-04-12 | 3.2 |
| https://www.wikifunctions.org/wiki/Wikifunctions:About | edited 2026-07-13 | 3.2 |
| https://www.wikifunctions.org/wiki/Wikifunctions:Function_model | edited 2026-02-25 | 2.4, 3.2 |
| https://www.wikifunctions.org/wiki/Wikifunctions:Status_updates | headlines to 2026-11-23 | 3.2 |
| https://meta.wikimedia.org/wiki/Abstract_Wikipedia | edited 2026-09-29 | 3.2 |
| https://meta.wikimedia.org/wiki/Abstract_Wikipedia/Google.org_Fellows_evaluation | written 2022; edited 2026-07-24 | 3.2 |
| https://meta.wikimedia.org/wiki/Requests_for_comment/The_future_of_Abstract_Wikipedia | opened 2026-07-14, open | 3.2 |
| https://github.com/amrisi/amr-guidelines/blob/master/amr.md | AMR 1.2.6, 2019-05-01 | 3.3 |
| https://raw.githubusercontent.com/umr4nlp/umr-guidelines/master/guidelines.md ; https://umr4nlp.github.io/web/ | UMR 0.9, 2022-08-08 | 3.3 |
| https://universaldependencies.org/ ; .../introduction.html | UD 2.19, 2026-11-15 | 3.3 |
| https://en.wikipedia.org/wiki/FrameNet | undated | 3.3 |
| https://en.wikipedia.org/wiki/Universal_Networking_Language | undated | 3.3 |
| https://globalwordnet.github.io/gwadoc/ ; https://aclanthology.org/2016.gwc-1.9/ | 2016 (CILI) | 3.4 |
| https://babelnet.org/about | BabelNet 6.0, 2026 dumps | 3.4 |
| https://en.wikipedia.org/wiki/Lojban | undated | 3.5 |
| https://en.wikipedia.org/wiki/Natural_semantic_metalanguage | undated | 3.5 |
| https://plato.stanford.edu/entries/montague-semantics/ | 2011-11-07, rev. 2025-02-05 | 3.5 |
| https://plato.stanford.edu/entries/linguistics/whorfianism.html | (c) 2026, undated | 3.5 |
| https://plato.stanford.edu/entries/logic-manyvalued/ | 2026-01-17, rev. 2026-09-29 | 4 |
| https://en.wikipedia.org/wiki/Null_(SQL) | undated | 4 |
| coherence-kernel/axioms/core-axioms.form | acknowledged 2026-11-08 | 0, 4, 5 |
| NUMS.Go/nums/nums_symbols.go ; NUMS.Go/http/model/language.go | last commit 2023-04-05 | 0, 2.5 |
