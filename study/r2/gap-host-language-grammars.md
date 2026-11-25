# Gap G4: the old body's host-language grammars, and the smallest mechanism for one definition that reads, prints and translates

Study round 2, written 2026-11-24. Closes gap G4 of `critic.md` (lines 93-100): "The old body's host-language
grammars were not opened".

Path abbreviations used below:

- `R` = `coherence-kernel` (HEAD `b8ff978c9`)
- `R@cut^` = the tree one commit before `115fea55e` ("Cut the repo to its end-to-end healthy flow: 14,729 files to
  1,628", 2026-09-29 22:23 +0800), read with `git show '115fea55e^:<path>'`. Nothing was checked out.
- `N` = `NUMS.Go`
- `S` = `scratch`

Status words: **observed** (I read it or ran it in this session), **inferred** (reasoned from observed things,
named), **assumed** (not verified). I did not run `fkwu`. The only program I ran is my own probe,
`S/exp-lens/lens.c`, compiled with the system `cc`.

Built on, not repeated: `S/bml.md` section 2.15 (the 2000 and 2026 BMF engines), `S/nums-core.md` sections 2.3, 4,
5, `S/nums-model.md` sections 4 and 8, `S/web-numeric-root.md` sections 2.5, 2.6, 3.1.

---

## 0. The short version

1. The five host-language files are **phrase books, not grammars**. Every rule in `go-bmf.fk`, `rust-bmf.fk`,
   `typescript-bmf.fk`, `prolog-bmf.fk` and `python-bmf.fk` is one flat sequence of fixed tokens and typed holes
   (`name`, `int`, `string`). No rule refers to another rule; none of the five files defines the rule-reference
   constructor the compiler would call. A Go function is readable only if its body is empty or is exactly
   `return a + b`. (observed)
2. They emit into **per-language node categories** (`GO-BMF-*` 631-653, `RS-BMF-*` 661-675, `PL-BMF-*` 676-684,
   `PY-BMF-*` from 501, 75 of them, `TS-BMF-*`, 37 of them). Two languages' nodes therefore cannot share identity
   above the leaf. (observed for the numbers; inferred for the consequence)
3. The one live test, `language-packs-band.fk`, proves shared identity of **two integer literals** (10 and 3)
   read out of five languages, not of any program. (observed)
4. A test of the real thing existed and was deleted in the cut of 2026-09-29: `lang-convergence-band.fk` parsed the
   same method in Go, Java and BML through three recursive grammars that shared 13 rules and asserted one node.
   Go and Java cost 32 lines each. It converged by **not keeping** what differed (types). (observed in git history;
   not run by me)
5. "Reversible" in the old body means: each rule names two hand-written closures, one building the node from
   captures, one rebuilding the token list from the node. The grammar rule line is 12-19% of each file's bytes; the
   closures and their registration are the rest. The reverse yields tokens, not text. (observed)
6. The old body has the two halves of the answer in two different engines and never joined them: the object engine
   (`engine.fk`) has both directions but as code; the cursor engine (`bmf-grammar.fk`) has pattern and template as
   pure data but one direction only. (observed)
7. Joining them works. A 360-line C probe reads one rule set (pattern + template, data) forwards as a reader and
   backwards as a printer. Go, Java and Python texts of one method read to the **same node number**; that node
   prints through each lens and re-reads to itself; parentheses appear only where needed; a meaning the target has no
   row for is refused by name; a rule that reads without keeping cannot print. Cost: 9 shared rules, 7 or 8 rules
   plus 2 lexicon rows per language. (observed, `S/exp-lens/out.txt`, verdict 31)
8. The limits are known and are not syntax: operator meaning that depends on operand type, scoping and name
   resolution, comments and layout fidelity, identifier collisions with keywords, and every construct a target
   language has no row for.

---

## 1. Inventory: what exists today

All dates are the last commit touching the file (`git log -1 --format=%ci`). (observed)

| File | Lines | Bytes | Date | Role |
|---|---|---|---|---|
| `R/form/form-stdlib/grammars/go-bmf.fk` | 223 | 21,091 | 2026-10-03 | Go phrase rules |
| `R/form/form-stdlib/grammars/rust-bmf.fk` | 114 | 12,019 | 2026-10-03 | Rust phrase rules |
| `R/form/form-stdlib/grammars/typescript-bmf.fk` | 252 | 26,809 | 2026-10-03 | TypeScript phrase rules + token scanner |
| `R/form/form-stdlib/grammars/prolog-bmf.fk` | 135 | 12,786 | 2026-10-03 | Prolog phrase rules |
| `R/form/form-stdlib/grammars/python-bmf.fk` | 1,118 | 107,012 | 2026-10-01 | Python scanner, layout, statement grouper, phrase rules |
| `R/form/form-stdlib/engine.fk` | 2,581 | | 2026-09-29 | object-stream rule engine, both directions |
| `R/form/form-stdlib/source-compiler.fk` | 2,362 | | 2026-11-14 | compiles `section [x.bmf] { ... }` text into rule lists (lines 436-528) |
| `R/form/form-stdlib/compiler.fk` | 279 | | 2026-10-04 | BMF meta-grammar, port table |
| `R/form/form-stdlib/dialect-categories.fk` | 209 | | 2026-09-30 | generated per-language category numbers |
| `R/form/form-stdlib/bmf-core.fk` | 553 | | 2026-11-03 | cursor, pattern matcher, template builder |
| `R/form/form-stdlib/bmf-grammar.fk` | 1,138 | | 2026-11-03 | recursive grammar-as-data engine |
| `R/form/form-stdlib/bmf-mini.fk` | 35 | | 2026-10-03 | token fold used by the swap-the-grammar band |
| `R/form/form-stdlib/lang-common.fk` | 21 | | 2026-09-30 | 13 shared imperative rules; no caller left |
| `R/form/form-stdlib/grammar-loader.fk` | 52 | | 2026-09-30 | named registry of grammar values |
| `R/form/form-stdlib/tests/language-packs-band.fk` | 119 | | 2026-10-03 | the only band that loads the five files |
| `R/cognition/pl-lenses-13.fk` | 86 | | 2026-10-03 | 13-language detector and one-function generator, with its own audit |

Not host-language material, checked and set aside (observed): `R/grammars/` holds `form-eval.fk`,
`control-invite-grammar.fk`, `host-effect-vocabulary.bmf`, `bml-native-north-star.form`; `R/form/form-stdlib/adapters/`
holds model adapters (`llama-3.2-3b-*`, `qwen-teach-overlay.lora`); `native-op-emitter.bml` writes the kernel's op
table. There is no `c-bmf`, `java-bmf` or `cpp` grammar in the tree. `compiler.fk:60` lists Java as
"legacy-source" (pointing at a path outside the repository) and CSharp as "target".

The body's own description of where this stands (observed):

- `R/form/form-stdlib/compiler.fk:60`: Python "in-motion" with gap "f-strings, decorators, try/catch, modules, ORM
  runtime"; TypeScript, Go, Rust "seed" with gap "full ... grammar and semantic lift".
- `R/cognition/pl-lenses-13.fk:4-16`: "no substrate language cells or parse/emit walkers for Python, TypeScript, Go,
  or Rust are carried here"; "It does not claim thirteen parsers, AST round-tripping, name escaping, type checking,
  compilation, execution, or arbitrary transpilation."; capability flags
  `(lexical-detect, bounded-generate, full-parse, AST-roundtrip, typecheck, compile, execute, arbitrary-transpile,
  identifier-escape) = [1, 1, 0, 0, 0, 0, 0, 0, 0]` (line 85).
- `R/NORTH_STAR.md:113-114` (2026-11-08) states the intent only: "grammar + lexicon + decoder onto the pivot, grammar
  + blueprint family + emitter onto BMF nodes". It names no file for the programming-language half.

### 1.1 What the cut removed (observed in `R@cut^`, line counts by `git show | wc -l`; I read only the files marked "read")

| Path in `R@cut^` | Lines | Note |
|---|---|---|
| `form/form-stdlib/go.fk` | 32 | read. Recursive Go method grammar on the cursor engine |
| `form/form-stdlib/java.fk` | 32 | read. Same for Java |
| `form/form-stdlib/tests/lang-convergence-band.fk` | 49 | read. Go = Java = BML node equality |
| `form/form-samples/cross-modal/02-cross-language-content-addressing/{README.md,converge.fk,factorial.py,factorial.ts,factorial.fk}` | | read. Hand-built trees |
| `form/form-stdlib/python-bmf-lift.fk` | 1,214 | not read. Header of its band says it walked a parsed module tree into `PY-BMF-*` recipes |
| `form/form-stdlib/python-bmf-eval.fk` | 1,258 | not read. A closure interpreter over those recipes |
| `form/form-stdlib/grammars/python-bmf-compiler.fk` | 777 | not read |
| `form/form-stdlib/emits/python-native.fk` | 534 | not read |
| `form/form-stdlib/typescript-bmf-lift.fk`, `typescript-bmf-eval.fk`, `prolog-bmf-eval.fk` | 196, 217, 211 | not read |
| `form/form-kernel-ts/src/lang-go.ts`, `lang-rust.ts`, `lang-typescript.ts`, `languages.ts` | 2,018, 2,377, 1,898, 741 | not read. Language front ends written in TypeScript |
| 21 `tests/python-bmf-*` bands, `go-bmf-reversible-band.fk` (128), `prolog-bmf-reversible-band.fk` (105) | | head of two read |
| `docs/native-python-compiler.md` | 26 | read. A `.py` to `.fk` specimen compiler |

The five surviving grammar files were then in s-expression form and three times longer in lines: python 3,028,
typescript 716, go 560, prolog 351, rust 309. The present files are the same rules in the BML surface with very long
lines, so bytes are the honest measure today. (observed)

---

## 2. Per language: what is read, what is written back

Method: I read every rule line (`name ::= pattern => emit <= source;`) in each `section [<lang>.bmf]` block and the
closures they name. Counts by `grep -c '::='` and `grep '::=' | grep -c '<='`. (observed)

Shape common to all five: a pattern is a flat sequence of quoted tokens and `$name:kind` holes with kind in
`name | int | string` (Python adds `float | bytes | fstring`). There is no alternation, repetition, optional part or
reference inside any host rule.

### Go: `R/form/form-stdlib/grammars/go-bmf.fk:181-212`: 32 rules, 32 with a reverse

Read (each as one fixed shape): `package x`; `import "p"`; `func f(){}`; `func f(a,b){}`;
`func f(a T, b T) T { return a + b }`; `func (r) f(){}`; `type T struct{}`; `type T interface{}`; `var x = 1`;
`const x = "s"`; `return x`; `return 1`; `x = y`; `x = 1`; `x := y`; `x := 1`; `f()`; `f(x)`; `a.b`; `a[1]`;
`if c {}`; `for k, v := range xs {}`; `go f()`; `defer f()`; `type A = B`; `switch v { case c: }`;
`select { case x := <-ch: }`; `ch <- v`; `<-ch`; `T{}`; `T{f: v}`; `*T`.

Written back: the same 32 shapes as a list of token objects (30 `gobmf-source-*` closures; the two `:=` rules reuse
the `=` closures because the operator is stored as a child, lines 86-97). Node categories: 23 (`GO-BMF-PACKAGE` ..
`GO-BMF-PTR-TYPE`, `R/form/form-stdlib/dialect-categories.fk:40-63`).

Not readable: any nested expression, any non-empty body except the one `return a + b`, any file.

### Rust: `rust-bmf.fk:84-103`: 20 rules, 10 with a reverse

Read: `use "p";` and `pub use`; `mod m;` and `pub mod`; `fn f(){}`; `fn f(x){}`;
`fn f(a: T, b: T) -> T { return a + b; }`; empty `struct`, `enum`, `impl`, `trait`; `name!("s");`; `let x = y;`;
`let x = 1;`; `return x;`; `match v { a => r }`; `if c {}`; `loop {}`; `async fn f(){}`;
`fn f() -> R<O, E> { C(v) }`.

Written back (10): fn-return-add, macro-call, let-ident, let-int, return-ident, match-default-ident, if-pass,
loop-empty, async-fn0, fn-result-ok. The other ten have no reverse and fall to the default, which returns the
token span the object remembered from its own parse (`source-compiler.fk:487-489`, `engine.fk:1924-1930`). Node
categories: 15 (`dialect-categories.fk:155-170`).

### TypeScript: `typescript-bmf.fk:195-227`: 33 rules, 33 with a reverse

Read and written back: named and default import; `export const`; empty `function` and `export function` (parameter
list as one string hole); the typed `return a + b` function; empty `class`, `interface`, `enum`; `type A = B;`;
`f(x);`; `await x;`; `<Name/>`; `return x;`, `return "s";`, `return c ? a : b;`; `c ? a : b`; `x = y;`; `x = 1;`;
`a.b`; `a[1]`; `a?.b`; `if (c) {}`; `if (c) { return v; }` with one and two `else` arms; `for (const x of xs) {}`;
`x => y`; `[]`; `{}`; one generic identity-shaped function; an interface with one property; a template string with
one hole. A token scanner is present (lines 237-249), built from a keyword list and an operator list over the shared
scanner (`source-compiler.fk:1980`, `:2147`). Node categories: 37.

### Prolog: `prolog-bmf.fk:110-124`: 15 rules, 15 with a reverse

Read and written back: facts of arity 0, 1, 2; a rule with one goal; a rule with two binary goals; `h :- g, !.`;
`:- g.` and `:- g(a).`; `?- g(a, b).`; `X is A + B` with name or integer; `[H|T]`; `[]`; `X = Y`; `h --> a, b.`.
Node categories: 9 (`dialect-categories.fk:69-78`).

### Python: `python-bmf.fk:926-1101`: 176 rules, 139 with a reverse

This file is different in kind in its first 200 lines, the same in its rules.

- Token scanner as data plus code (lines 29-118): keyword list, operator list (longest first), prefixed and
  triple-quoted strings, integers with `_`, floats of the form `N.M` only ("Exponents (1e9) and leading-dot floats
  (.5) are a later breath", lines 83-93).
- Layout (lines 119-139): NEWLINE, INDENT, DEDENT, ENDMARKER objects derived from line and column. The dedent step is
  the constant 4 (line 126). (observed)
- Statement grouper (lines 140-201): groups tokens into logical statements by line and bracket depth, nests them by
  indentation, and labels each with a CPython rule name chosen from its first tokens (line 161).
- 176 phrase rules. By family: 12 import forms; 8 decorator forms; `def` with 0 or 2 parameters and several
  annotation variants, bodies absent or `pass`; `class` with 0 to 3 bases, a metaclass keyword, a decorator, body
  `pass`; assignment of int, name, string; annotated, augmented (`+=` only), attribute, subscript, unpack-2, walrus;
  `return` of int, name, string, and `return a + b`; `raise` in 5 forms; `pass break continue global nonlocal del
  assert`; `if/elif/else`, `while`, `for`, `with`, `async` variants, all with `pass` bodies; 4 `try` shapes with
  `pass` and 4 with a single name as body; calls with 0 or 1 argument, one keyword, one star; attribute, 2-step
  attribute chain, method call with 0 to 2 arguments, one chained call; subscript and 6 slice shapes; list/tuple/set/
  dict literals of a fixed size; 10 comprehension shapes; 11 comparison rules (one a fixed chain `a < b <= c`) and
  11 binary-operator rules, each `name OP name` (lines 1059-1080); `and`, `or`, `not`, unary minus and plus on an
  int; lambda with one argument;
  `await`, `yield`, `yield from`; `match`/`case` headers; float, bytes, f-string with 0, 1 or 2 holes; `...`;
  and leaf rules `int`, `string`, `ident`.
- One rule, `sample-module` (line 1098), matches the complete token sequence of one specific sample file: two
  imports, a from-import, a constant, a two-parameter `def` returning a sum, and a `main` with two assignments and
  two calls. It is a grammar for one program. (observed)

Node categories: 75. The operators of a binary expression are stored as string children of one `PY-BMF-BINOP` node,
for example `intern_node(PY-BMF-BINOP, [left, "+", right])` (line 276). (observed)

What connected the statement grouper to the rules, `python-bmf-lift.fk`, is gone (1.1). In the present tree nothing
calls `python-parse-module-*` or any `pybmf-*` name outside the grammar file itself (repository-wide search, observed).

---

## 3. How a rule is held as data: four representations

(observed for all four)

**A. BMF text.** `name ::= pattern => action <= reverse;` inside `section [<lang>.bmf] { }`. It is read by a line
scanner, not by a grammar: `fsc-rule-line-recipe` finds `"::="`, `" => "`, `" <= "` and `";"` by substring search
(`R/form/form-stdlib/source-compiler.fk:477-499`). A quoted part becomes a call of `<lang>-kw` or `<lang>-op`
depending on whether it is word-shaped (`:143-148`); `$name:kind` becomes a call of `<lang>-name`, `<lang>-int-lit`,
`<lang>-string-lit` and so on (`:155-167`); a bare word becomes a call of `<lang>-ref` (`:463-465`), a function none
of the five host files defines.

**B. Object rule (what A compiles to).** `[name, ["sequence", item, ...], action, bmf-identity-inverse, reverse]`
with item = `["object", kind, value]` (value `""` matches any object of that kind) or `["capture", name, item]`
(`engine.fk:2102`, `go-bmf.fk:17-21`). The engine's pattern tags are `object sequence choice capture star opt cut
stop` (`engine.fk:2127-2143`). `action` and `reverse` are **closures**: Form functions, not data. The input is a
list of token objects from "a sensing layer" (`engine.fk:5-7`), never text.

**C. Cursor rule.** `(list name pattern template)` with an optional `"memo"` mark (`bmf-grammar.fk:58-59`).
Pattern tags, all lists: `lit` (with `kw` variants), `ref`, `run`, `cap`, `opt`, `not`, `rep`, `sep`, `str`, `char`,
`num`, `infix` (operand + operator table), `chain`, `leaf`, `ternary`, `seq`, `alt` (`:1109-1127`). Template tags:
`emit op kids`, `splice`, `splice-int`, `splice-neg`, `bind`, `splice*`, `wrap-rest`, `const`, `const-int`,
`const-bool` (`:1128-1137`). A grammar is `(list "G" start rules index)` (`:51`). The data is compiled once per rule
set per process into numbered ops with first-byte sets (`:327-331`, `:541-573`). Ordered choice, greedy repetition,
no re-entry (`:817-826`; see `S/bml.md` 2.15). **No reverse direction exists for C.** This is the engine the home
language uses: `R/form/form-stdlib/grammars/form-bml.fk` has 84 lines naming `rule3` and one 17-row operator table
(`:41-48`: symbol, node name, precedence, optional `"call"`, optional `"right"`).

**D. Token-fold rule.** `(token, op, operand)` as interned cells; `bmf-compile` folds a list of token ints
(`bmf-mini.fk:20-30`). Used only to show that swapping the rule list changes the compiled program.

Two things in C are not general although the engine is presented as general (observed):

- White space and comments are fixed to the C family: `//` and `/* */` are skipped by every primitive
  (`bmf-core.fk:390-404`, `bmf-grammar.fk:582-594`); the file says "Python `#` / Form `;` are a future per-grammar
  parameterization".
- BML's own sugar is inside the engine: the keyword edge uses the `bmlname` class (`bmf-grammar.fk:338`), the chain
  op hard-codes `.`, `[`, `(`, `<...>`, `++`, `--` and builds `$bml-slice` and `$bml-typed` calls
  (`:930-979`, `:1053-1059`).

One lesson recorded in the engine itself: the blueprint name table is kept in two hand-held copies, and a name
missing from the copy in `bmf-core.fk` silently receives category `0.0.0.0`, "children identical, identity never
meeting" (`bmf-core.fk:93-102`, `:161-168`). (observed)

---

## 4. How `=> action <= reverse` works, with a real rule

The rule with a live round-trip check is Rust's `let-int`.

```
rust-bmf.fk:97   let-int ::= "let" $name:name "=" $value:int ";" => rsbmf-emit-let-int <= rsbmf-source-let-int;
rust-bmf.fk:49   def rsbmf-emit-let-int(objects) = intern_node(RS-BMF-LET, [rsbmf-string(objects, "name"), rsbmf-int(objects, "value")]);
rust-bmf.fk:50   def rsbmf-source-let-int(object) = {
rust-bmf.fk:51       let kids = rsbmf-node-kids(object);
rust-bmf.fk:52       [rust-src-keyword("let"), rust-src-name(node_value(kids[0])), rust-src-op("="), rust-src-int(node_value(kids[1]).str()), rust-src-op(";")];
rust-bmf.fk:53   };
```

Forward, `apply-object-rule` (`engine.fk:2444-2464`): match the pattern against the token list; turn the captures
into a collection; call the action on it to get the node; wrap node, rule name, the matched token span and the
inverse into one object. Backward, `apply-object-rule-reverse` (`:2556-2559`): call the rule's fifth element on the
object. `bmf-roundtrip` (`:2561-2571`) does forward, backward, forward again, and `bmf-roundtrip-node-eq?`
(`:2577-2579`) compares the two nodes. The band feeds it tokens, not text
(`tests/language-packs-band.fk:86-90`). (observed)

What this shows:

1. The two closures are each other's mirror by hand. `kids[0]` is `"name"`, `kids[1]` is `"value"`, and the literal
   tokens are typed a second time. Nothing checks that they agree except a round-trip band per rule. (observed for
   the code; the "nothing checks" is inferred from the engine having no other consumer of the pair)
2. The reverse returns a **token list**. I found no function that joins tokens into text with spacing or layout in
   `engine.fk`, `source-compiler.fk` or `compiler.fk` (search by name for `to-text`, `source-text`, `render`,
   `objects-text`: none). Go, Rust and Prolog also have no scanner from text to tokens in their files (0 matches for
   `scan`). So for those three, neither end touches text. (observed absence, limited to the searches named)
3. A rule without `<=` gets `bmf-default-reverse-emitter`, which returns the span the object was parsed from. That
   reverses only an object that came from a parse; a node built any other way has no source. (inferred from
   `engine.fk:92-97`, `:1924-1930`)
4. Measured share: the rule lines are 3,602 of 21,091 bytes in Go (17%), 1,912 of 12,019 in Rust (16%), 4,203 of
   26,809 in TypeScript (16%), 1,577 of 12,786 in Prolog (12%), 20,042 of 107,012 in Python (19%). The remainder is
   closures, the `bmf-native(...)` registration list, the `compiler-rule(...)` list and helper wrappers. (observed,
   `head | tail | wc -c` over the rule line ranges)

---

## 5. Does parsing two languages' equivalent programs give nodes with shared identity?

### 5.1 Today: no test, and the design prevents it above the leaf

`tests/language-packs-band.fk` is titled "five tongues, one numeric shape". What it does (observed, lines 20-77):
each language parses one statement holding an integer (`step = 10` in Python, `var step = 10` in Go,
`let step = 10;` in Rust, `Step is Acc + 10` in Prolog, `step = 10;` in TypeScript); `last-kid-int` takes the
integer **value** out of the last child; the two values 10 and 3 become the operands of a two-row `bmf-mini` rule
list; the same token list `[1,1,2]` is folded through it; `ms-same` compares the five resulting program cells. The
five cells are equal because 10 = 10 and 3 = 3. The statement nodes themselves are never compared, and could not
be equal: they carry `PY-BMF-ASSIGN` (505), `GO-BMF-VAR` (637), `RS-BMF-LET` (669), `PL-BMF-IS` (680) and a
`TS-BMF-*` category, and they have different child lists (Prolog's has three children). Verdict 31 is pinned in
`R/CURRENT_FLOOR.md:448` with the date 2026-10-01. (observed for the code and the pin; "could not be equal" inferred
from the category numbers and from node identity being category plus children, `bmf-core.fk:527-528`)

### 5.2 Before the cut: yes, a test existed, at method depth, for Go, Java and BML

`R@cut^:form/form-stdlib/tests/lang-convergence-band.fk` (49 lines; history of this file, `go.fk` and `java.fk`
taken together: added 2026-07-02, at least one touched 2026-08-09, all deleted 2026-09-29). It builds the expected
tree by hand and asserts `node_eq` of three parses against it:

```
(node_eq (g-parse gg "func f ( x int ) { y = a + b ; return y ; }") want-f)
(node_eq (g-parse jg "int f ( int x ) { y = a + b ; return y ; }")  want-f)
(node_eq (g-parse bg "int f ( int x ) { y = a + b ; return y ; }")  want-f)
```

and a second method with an `if` and a call (Java with parentheses, Go without). Expected total 300.

How it converged (observed in `R@cut^:form/form-stdlib/go.fk`, `java.fk`, and `R/form/form-stdlib/lang-common.fk:20`
which survives without a caller):

- Each language supplies three rules: `method`, `param`, `ifs`. Everything from `stmt` down to `atom`, 13 rules,
  is one shared list appended to each grammar.
- All three emit the same universal names: `fndef`, `do`, `let`, `if-then`, `add`, `fncall`, `UE-JUMP-RETURN`.
- **The types are read and dropped.** Go's `param` is `(seq (cap n (run alpha)) (run alpha))` with template
  `(splice n)`; Java's is `(seq (run alpha) (cap n (run alpha)))`. The return type is an uncaptured `(run alpha)`
  too. `func f(x int)` and `func f(x string)` are the same node.

Evidence that it ran: `R@cut^:receipts/2026-07-17-ontology-band-triage.md` lists `lang-convergence` as DIVERGENT
before a table fix (first missing name `UE-JUMP-RETURN`) and "NO-FOURTH (0 divergent; fourth silent)" after, meaning
the three sibling kernels then agreed and `fkwu` did not report. I did not run it. (observed in the receipt; the
value 300 is not stated there)

Two defects in the shared rules that a tree-equality test cannot see (observed in the rule text, consequence
inferred): `addx` is `term "+" expr` and `subx` is `term "-" expr`, `divx` is `factor "/" term`. These are
right-recursive, so `a - b - c` reads as `a - (b - c)` and `a / b / c` as `a / (b / c)`. And `call` emits
`fncall[name-leaf, args...]`, while the engine's own call builders emit `fncall[ident[name], ...]`
(`bmf-grammar.fk:906-909`): two shapes for a call under one category name.

### 5.3 Before the cut: the sample named "cross-language content addressing" was hand-built

`R@cut^:form/form-samples/cross-modal/02-cross-language-content-addressing/converge.fk` interns the same tree twice
through two helper routes, calls one "the Python author's" and the other "the TypeScript author's", and prints that
the NodeIDs match. Its README says under "What's not reachable today": "Python → NodeID end-to-end via
python-bmf.fk ... requires lighting up the whole BMF object pipeline inside the sample". `factorial.py` and
`factorial.ts` sit beside it and are not read by anything. (observed)

---

## 6. Cost per language

| What | Size | Covers | Status |
|---|---|---|---|
| Go phrase file today | 223 lines, 21 KB | 32 fixed shapes, both directions on tokens | observed |
| Rust phrase file today | 114 lines, 12 KB | 20 shapes, 10 reversible | observed |
| TypeScript phrase file today | 252 lines, 27 KB | 33 shapes + scanner | observed |
| Prolog phrase file today | 135 lines, 13 KB | 15 shapes | observed |
| Python phrase file today | 1,118 lines, 107 KB | scanner, layout, grouper, 176 shapes | observed |
| Python lift + eval + compiler + emitter before the cut | 1,214 + 1,258 + 777 + 534 lines | the path that actually ran Python text (arithmetic, def, call, recursion per its band header) | observed sizes; behaviour from band header only |
| Go recursive method grammar before the cut | 32 lines over 13 shared rules (21-line file) | methods with params, assignment, `+ - * /`, calls, `if`, `return`; one direction; types dropped | observed |
| Java, same | 32 lines | same | observed |
| TypeScript-hosted front ends before the cut | 1,898 to 2,377 lines per language | not read | observed sizes only |
| NUMS Go front end | `N/nums/go.go` 1,502 lines + `parser.go` 657 + tree-sitter grammar | 75 handlers (`S/nums-core.md` 5.1) | sizes observed here; handler count from the report |
| NUMS Go label lens | `N/http/config/Go.yaml` 282 lines over `labels.yaml` 298 lines | classification only (`S/nums-model.md` 8) | sizes observed here |
| Probe lens, this session | common 17 lines (9 rules); go 13 lines (7 rules, 2 lexicon rows); java 13; python 19 (8 rules) | methods with typed params, assignment, four operators with precedence, calls, `if`, `return`; **both directions on text**; types kept | observed, ran |

The old phrase-book cost is 600 to 850 bytes per shape (file bytes over rule count) and grows with the number of
shapes (Python needed 11 rules for 11 binary operators between two names). The recursive form costs a rule per
construct and a row per operator.

---

## 7. Comparison with NUMS.Go

From `S/nums-core.md` and `S/nums-model.md`, with three facts re-observed here (file sizes; 81 `_Op:` templates in
`N/http/config/labels.yaml`, for example line 79 `Equal : { _Op: '{0} == {1}' }`, line 151
`Ternary : { _Op: '{0} ? {1} : {2}' }`, line 200 `For : { _Op: 'for {0}; {1}; {2} {3}' }`; and 0 occurrences of
`_Op` in `Go.yaml`; both files last committed 2023-04-05).

| Question | Old body (present tree) | Old body (before the cut) | NUMS.Go |
|---|---|---|---|
| Who reads text | hand scanner per language (Python, TypeScript) or nobody (Go, Rust, Prolog) | the cursor engine, scannerless | tree-sitter, an external parser per language (`nums-core` 5.1) |
| Node vocabulary | one category family **per language** | one shared family (`fndef`, `add`, ...) for the shared rules | one **fixed catalogue** for all languages: 19 level-2 recipe types, operators drawn from C, C#, Python, C++ (`nums-core` 2.3) |
| What the per-language part is | rules + two closures per rule | 3 rules | a table from parser symbol to neutral label, plus naming and scoping rules in a small selector language, plus Go code (`nums-model` 4.2, 8) |
| Where the surface template lives | inside each reverse closure | nowhere (one direction) | on the **neutral** label, one `_Op` string for all languages; the Go lens has none |
| Printer | 209 hand-written source closures across five files (30 + 10 + 33 + 15 + 121), tokens out | none | hand-written Go switch for a subset of function bodies; it does not use the `_Op` strings (`nums-core` 4, 5.3) |
| Types | stored as plain name strings (Go, TS, Rust `return-add`), or absent | dropped | resolved while building; all Go integer widths collapse to one blueprint (`nums-core` 5.1 item 7) |
| Names | string leaves inside the node | string leaves inside the node | ordinals into a side table; renaming moves no node (`nums-core` 4) |
| Second language | five phrase books | Go, Java, BML to one node at method depth | none; twelve languages enumerated, only Go has handlers (`nums-core` 5.1 item 2) |

Reading the three together (inferred):

- NUMS has the right **root** (a fixed neutral catalogue; names outside the node) and an **embryo of the right
  lens**: `'{0} + {1}'` is a pattern and a template at once, a literal sequence with numbered holes bound to the
  children of one neutral label. It was used one way (hole count as arity when classifying) and attached to the
  wrong side (the neutral tree, so one spelling for all languages).
- The old body before the cut had the right **engine shape** (rules as data, shared core, language supplies only
  where its surface differs) with no printer and with names inside nodes.
- The old body today has the right **word** ("reversible", `=> <=`) on the wrong carrier (closures over tokens,
  per-language categories).

The mechanism in section 8 is the `_Op` idea, moved to the per-language side, with typed holes, and run by one
interpreter in both directions.

---

## 8. The smallest mechanism for one definition that reads, prints and translates

### 8.1 Statement

A **lens** for a host language is a list of rows. A row is

```
(rule NAME PATTERN TEMPLATE)
```

- `PATTERN` is data over a closed set of forms: literal token; a run of a character class kept as a leaf; a
  reference to another rule; capture; sequence; ordered alternatives; repetition; separated list; an operator table
  `(symbol, blueprint, precedence)`; three layout marks (line end, indent, dedent). This is the old body's
  representation C (section 3) minus the BML-specific ops.
- `TEMPLATE` is a **constructor expression** over the captures: `(emit BLUEPRINT child...)`, `(splice name)`,
  `(splice* name)`, `(const leaf)`. Also representation C.
- A second kind of row, `(lex NEUTRAL "spelling")`, maps a neutral leaf to this language's word for it (type names
  here; operator spellings are already rows of the operator table).

One interpreter walks a row in two directions:

- **Read**: match `PATTERN` against text, collect captures, build `TEMPLATE` with `intern`. Ordered choice, as the
  old engine.
- **Print**: take a node; run `TEMPLATE` **as a destructor** (does the node have this blueprint and this many
  children? bind each capture to the child in its place); then walk `PATTERN` emitting literals and printing each
  capture through the sub-pattern it was captured by.
- **Translate**: read with lens A, print with lens B. Before printing, the set of blueprints in the tree is compared
  with the set lens B has rows for; a blueprint with no row is reported by number and the translation is refused.

Why this is the minimum (inferred from sections 3 to 5 and from the paper in 8.3): the reverse of a template is free
**because nodes are content-addressed**. `intern(blueprint, children)` and `(blueprint-of node, children-of node)`
are already a pair of mutually inverse functions; a template built only from them is invertible wherever each
capture is used once. The old body's `gobmf-emit-x` / `gobmf-source-x` pairs are that inverse written twice by hand.
Nothing else needs adding to representation C except the second walk.

Three conditions a row must meet to be printable, each checkable when the lens is loaded (inferred; the first is
demonstrated in 8.2):

1. Every part of the pattern that consumes variable text is captured (otherwise there is nothing to print from).
2. Every capture appears exactly once in the template (linear).
3. Alternatives that add text without adding a node (parentheses) are reachable in print only when the plain
   alternatives fail. In the probe this is one guard: a rule that is asked to print the node it is already printing
   answers no.

### 8.2 Evidence that it works: the probe

Files (all in `S/exp-lens/`): `lens.c` (360 lines: s-expression reader, hash-consing node store, reader, layout
pre-pass, printer, coverage check, test driver), `common.lens`, `go.lens`, `java.lens`, `py.lens`, `golossy.lens`,
four source texts, `out.txt`. Built with `cc -O1 -Wall -Wextra -fsanitize=address,undefined`; no warning, no
sanitizer report; exit 0; verdict 31. (observed)

1. **One node from three surfaces.** `scale.go.txt`, `scale.java.txt`, `scale.py.txt` (a typed two-parameter method
   with an assignment, a parenthesised sum times a name, an `if` with a call, and `return y - k / 2`, written
   `k // 2` in Python) each read to node `#22`. The space holds 22 nodes after all three reads: the second and third
   reads created nothing. The type words `int64`, `long`, `int` meet at the leaf `i64` through the lexicon rows.
2. **Print and re-read.** Node `#22` printed through each lens and read again through the same lens gives `#22`.
   Go to Java and Go to Python are therefore the same operation as printing. Output, Python lens:
   ```
   def scale ( x : int , k : int ) -> int :
       y = ( x + 1 ) * k
       if y :
           log ( y , k )
       return y - k // 2
   ```
3. **Parentheses from the rules.** `(a + b) * c` prints with its parentheses; `a + b * c` prints without;
   `a - b - c` and `a - (b - c)` are different nodes (`#31`, `#33`) and each prints back to its own form. No
   parenthesis rule was written for printing: the operator table refuses a looser operator where a tighter one is
   required, and the operand rule's `"(" expr ")"` alternative then applies.
4. **Refusal by name.** `def half(x: float) -> float: return x / 2` reads to a node holding blueprint `truediv`
   (Python's `/` row). The Go lens has no row for it: the coverage check prints "no row for blueprint 13 (truediv)"
   and the print is refused. The Python lens prints it.
5. **Reading without keeping cannot print.** `golossy.lens` restores the two rules of the deleted `go.fk` (types
   read, not captured). It reads `scale.go.txt` to a different node (`#44`), and printing is refused with the reason
   that an uncaptured run has nothing to print from. The convergence of 5.2 was bought with exactly this.
6. **Cost.** Shared: 9 rules. Go: 7 rules (method header, parameter, block open, block close, statement end, two
   empty parenthesis rules) and 2 lexicon rows. Java: 7 and 2. Python: 8 and 2 (the eighth is its operator table,
   which differs in one row). Python also sets `(layout)`.

What the probe does not show: any language feature beyond those listed; speed (alternatives are retried, nodes are
found by linear search); comments; the same thing written in the one grammar instead of C. It also has no error
report: its first run answered node 0 for the Go text with no position and no reason (my lens read type names with
the letters-only class, and `int64` has digits). The old engine's traced mode, which keeps the farthest place a
rule could not begin and the rules that were wanted there (`bmf-grammar.fk:102-125`), is the part to carry for
that. (observed)

### 8.3 Evidence from outside

- Rendel and Ostermann, "Invertible Syntax Descriptions: Unifying Parsing and Pretty Printing", Haskell Symposium,
  30 September 2010. https://www.informatik.uni-marburg.de/~rendel/unparse/rendel10invertible.pdf, fetched
  2026-11-24, pages 1-12 read directly. (observed) The paper builds one description usable as parser or printer on
  **partial isomorphisms**: a pair of partial functions, constructor one way and destructor the other, generated
  mechanically per data constructor (section 3.1). Alternatives are tried in order in both directions (3.3, 4.2).
  White space is three marks: allowed (read any, print none), desired (read any, print one), required (read at least
  one, print one) (6.3). Operator priorities are handled by one partial isomorphism that accepts only operators of
  a given priority, so the printer falls through to the parenthesis alternative exactly where the parser would need
  it; their example prints `(7 + 8) * 9` (6.4). Stated limits: the round trip holds up to equivalence classes
  (many spaces read as one printed space) (3.1); the proof-of-concept parser has exponential worst case and a space
  leak from unlimited backtracking (4.4); a faster parser library may not have the same choice semantics (4.4).
  This is the same construction as 8.1, with the content-addressed node playing the role of their generated
  constructor/destructor pairs. The probe's result 3 reproduces their parenthesis result by the same means.
- BNFC. https://bnfc.digitalgrammars.com/ and https://bnfc.readthedocs.io/en/latest/lbnf.html, fetched 2026-11-24
  through a summarising fetch, so the wording reached me second-hand. (observed at that remove) From one labelled
  grammar it generates abstract syntax, a lexer, a parser and a pretty-printer for Haskell, Agda, C, C++, Java and
  OCaml; latest release listed 2.9.6.3, 2026-03-15. Documented asymmetries: the printer normalises away redundant
  separators; comments are not passed to the parser; layout syntax is implemented only by the Haskell-family back
  ends; the printer assumes numbered categories mean precedence; constructors marked internal can be printed and
  not parsed.
- Spoofax SDF3 template productions. https://spoofax.dev/references/sdf3/templates/, fetched 2026-11-24 through a
  summarising fetch. (observed at that remove) A production written as a template carries its own layout; without
  templates the generated printer puts everything on one line separated by spaces; with them the template's layout
  is what is printed. This is the industrial form of the probe's `(nl) (in) (out)` hints.
- Grammatical Framework (already in `S/web-numeric-root.md` 3.1): one abstract syntax, a concrete syntax per
  language, translation is parse then linearise. Same shape; its concrete syntaxes are programs, not tables.
- tree-sitter (already in `S/web-numeric-root.md` 2.5): node types are per grammar. That it generates no printer is
  **assumed** from my own knowledge, not fetched. NUMS stands on it and has a hand-written printer
  (`S/nums-core.md` 5.3).

### 8.4 Limits

Each line says what kind of limit it is and what it rests on.

1. **Operator meaning that depends on types.** Go's `/` is integer division on integers and real division on
   floats; a row `(op "/" div 2)` cannot tell. The probe's Go lens is wrong for `float64` operands. Choosing the
   blueprint needs the operand's blueprint, which is name resolution and typing, not syntax. NUMS resolves types
   while building (`S/nums-core.md` 5.1 item 6); `R/cognition/pl-lenses-13.fk:13-14` excludes division for this
   reason. (inferred; the exclusion is observed)
2. **Scope and names.** The probe keeps names as string leaves inside nodes, like the old body. Then `f(x)` and
   `f(y)` are different nodes and renaming changes identity. NUMS keeps names in a side table so that it does not
   (`S/nums-core.md` 4). The lens mechanism is neutral on this, but the reader must then resolve each name to a
   declaration, and that part is per-language engine work in NUMS (`S/nums-model.md` 8). (inferred)
3. **Round trip is on nodes, not on text.** read(print(n)) = n is what holds. print(read(text)) gives the canonical
   spelling: one space between tokens, the lens's layout, no redundant parentheses. Comments are skipped by the
   reader and are gone. A lossless text round trip needs the original spans kept beside the node (srcML does this,
   `S/web-numeric-root.md` 2.5; the old object engine kept a span per object, `engine.fk:2450-2459`). (observed in
   the probe for spacing and parentheses; inferred for comments)
4. **Layout is a pre-pass.** Python needed indentation turned into three marks before reading. It is 15 lines in
   the probe and is itself reversible, but it is one more thing outside the rule rows. The old Python file hard-codes
   a step of 4 (`python-bmf.fk:126`); the probe's reader keeps a stack and accepts any step, its printer writes 4.
   (observed)
5. **No row, no translation.** Constructs with no counterpart (Go's `select`, Rust's `match` with ownership,
   Python's comprehensions into Java) are refused by name. To translate them, something must rewrite the node into
   blueprints the target has rows for. That is a node-to-node recipe, a third artefact beside the two lenses, and
   it can lose or change meaning. The lens mechanism does not supply it. (inferred)
6. **Printing a leaf that collides with a keyword.** A name `func` or `def` printed through a lens where it is a
   keyword re-reads wrongly. Needs an escape row per lens. `pl-lenses-13.fk:85` marks identifier-escape as 0.
   (inferred)
7. **Ordered choice.** The reader commits to the first alternative that matches and never revisits. A host grammar
   that needs more look-ahead than the rule order gives must be reordered or factored by hand; the old engine
   already had to do this (`form-bml.fk:61-62`, "def-block BEFORE def-expr"). Left-recursive rules cannot be
   written; the operator table is the escape for the common case. (observed in the old engine; inferred for host
   grammars at scale)
8. **Desugaring templates.** A template that builds a call from a keyword form (as `form-bml.fk:70-74` does for
   `blueprint Name { ... }`) inverts mechanically, but then two surfaces lead to one node and the printer picks the
   first in rule order. Acceptable, and it must be stated per lens. (inferred)
9. **Scale is unmeasured.** The probe covers perhaps 15 constructs. Go's specification has several hundred
   productions (assumed, not counted). The old body's full BML grammar on the same representation is about 84
   rules (`form-bml.fk`, 84 lines naming `rule3`); nothing here measures a full host language. (the 84 is observed;
   the gap is stated, not estimated)

---

## 9. For the new design

Carry forward

- Rules as data in the cursor engine's shape: named rule, pattern list, template list, operator table rows.
  (`bmf-grammar.fk:51-59`, `:1109-1137`; `form-bml.fk:41-48`)
- The shared-core arrangement: one list of rules for what imperative languages have in common, a language supplying
  only the rules where its surface differs. (`lang-common.fk:1-11`; `R@cut^:form/form-stdlib/go.fk`)
- The round-trip check as the test of a lens: read, print, read, compare nodes. (`engine.fk:2561-2579`)
- Scannerless reading with ordered choice and first-byte pruning. (`bmf-grammar.fk:170-181`)
- NUMS's fixed neutral catalogue as the vocabulary rows emit into, and its numbered-hole template as the picture of
  what a row is. (`S/nums-core.md` 2.3; `N/http/config/labels.yaml:79-207`)
- The honest audit style of `pl-lenses-13.fk`: a capability vector that says 0 where it is 0.

Let go

- Per-language node categories (`GO-BMF-*`, `PY-BMF-*`, ...). They make shared identity impossible.
- Emit and source closures per rule. One template read both ways replaces both.
- Token objects as the grammar's input and output. Text in, text out.
- Phrase rules (`return-plus-ident`, `sample-module`, one rule per operator between two names).
- Two hand-held copies of the blueprint name table with a silent zero for a missing name.
- Engine ops that belong to one language's sugar (`$bml-slice`, `$bml-typed`, the `bmlname` keyword edge, C-family
  comments for everyone).
- Convergence by dropping what differs. Keep it and map it through a lexicon row, or do not claim the node is the
  program.
- Counting a language as "supported" from the presence of a grammar file.

---

## 10. Open questions

1. Was `lang-convergence-band` ever green on `fkwu` itself? The only receipt found says the three sibling kernels
   agreed and `fkwu` was silent (2026-07-17).
2. What did `python-bmf-lift.fk` (1,214 lines) do that the 176 phrase rules did not, and did it use the rules at
   all? Not read.
3. How many of the roughly 84 rules of `form-bml.fk` meet the three printability conditions of 8.1 as written? If most do,
   the home language gets its printer from its existing grammar; I did not count.
4. Where do names live in the new node: inside (identity changes on rename, lenses stay simple) or in a side table
   as in NUMS (identity survives rename, the reader must resolve scope)? The lens mechanism works either way; the
   cost moves.
5. How is type-dependent operator meaning chosen at read time without a per-language type checker? One candidate is
   to read to a language-scoped blueprint first (`go:/`) and let a separate, witnessed recipe map it to `div` or
   `truediv` once operand blueprints are known. Not tried.
6. Can the layout pre-pass be expressed as rows too, or does it stay a primitive of the reader?
7. What is the cost curve past the probe's fragment: rows per language for statements, declarations, generics,
   pattern matching? No measurement exists in either reference.
8. Does the printing guard ("a rule asked to print the node it is already printing answers no") stay sufficient
   with mutually recursive wrapping rules, or is a static order needed? One grammar tested.

---

## 11. Sources read

Repository `R` (observed): `form/form-stdlib/bmf-core.fk` (all), `bmf-grammar.fk` (all), `bmf-mini.fk` (all),
`grammars/go-bmf.fk` (all), `grammars/rust-bmf.fk` (all), `grammars/prolog-bmf.fk` (all),
`grammars/typescript-bmf.fk` (lines 5-40, 195-252), `grammars/python-bmf.fk` (definition index of lines 1-923, rule
lines 925-1104, samples of emitters, lines 1104-1118), `grammars/form-bml.fk` (1-80), `engine.fk` (1-125,
1840-2170, 2380-2470, 2540-2581), `source-compiler.fk` (120-180, 400-550), `compiler.fk` (56-62, 225-262),
`dialect-categories.fk` (1-30 and all `GO/RS/PL/NATURAL` rows), `lang-common.fk` (all), `grammar-loader.fk` (all),
`nl-translate.fk` (all), `tests/language-packs-band.fk` (all), `cognition/pl-lenses-13.fk` (all),
`grammars/host-effect-vocabulary.bmf` (all), `NORTH_STAR.md:104-116`, `CURRENT_FLOOR.md:440-450`,
`form/kernel-roadmap.md:60-80`.

History `R@cut^` (observed through `git show`): `form/form-stdlib/go.fk`, `java.fk`,
`tests/lang-convergence-band.fk`, the five files of `form/form-samples/cross-modal/02-cross-language-content-addressing/`,
`docs/native-python-compiler.md`, heads of `tests/python-bmf-lift-band.fk` and `tests/python-bmf-reversible-band.fk`,
parts of `receipts/2026-07-17-ontology-band-triage.md`.

NUMS `N` (observed): `http/config/labels.yaml` (lines with `_Op`), `http/config/Go.yaml` (search only), file sizes.

Web (fetched 2026-11-24): the Rendel and Ostermann PDF (read directly); bnfc.digitalgrammars.com,
bnfc.readthedocs.io/en/latest/lbnf.html, spoofax.dev/references/sdf3/templates/ (through a summarising fetch).
Failed: `ps.cs.uni-tuebingen.de` (DNS), `spoofax.dev/references/syntax/templates/` (404).

Probe (written and run this session): `S/exp-lens/lens.c`, `common.lens`, `go.lens`, `java.lens`, `py.lens`,
`golossy.lens`, `scale.go.txt`, `scale.java.txt`, `scale.py.txt`, `half.py.txt`, output `out.txt`.
