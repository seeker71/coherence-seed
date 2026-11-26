# Starting a session in coherence-seed

Everything a session needs is in this repository, and it builds with a C compiler and `make`.

## Ground first

1. Read [`NORTH_STAR.md`](NORTH_STAR.md) for the direction and [`FLOOR.md`](FLOOR.md) for what
   stands.
2. Build and look at the present state before asserting anything about it:

   ```bash
   make && make check && ./kg check
   ```

3. Read the design you are about to touch: `./kg brief | grep '^5\.'` for the design, `'^7\.'` for
   the path, `'^6\.'` for what is open.

## Guidance

We hold this work together. Guidance orients; it is not a list to enforce, and what is aligned
needs no permission. It is kept short and in our own words (graph node 7.13), not quoted.

- **Form first.** New code is written in form and kept as nodes. C is the bootstrap only, and no
  other language is written. What a program lacks becomes an operation of the bootstrap. Write in
  words through the lens and read stored nodes through it ([`spec/lens.md`](spec/lens.md)); what
  is stored is the canonical print. A new operation, area or field is added in a node
  (`lens/words.ck`, `region/layout.ck`) and `make header` writes the C header from them.
- **Meaning in the nodes.** What is stored is neutral: nodes in the numeric space, with words
  and names in a table beside them that changes no meaning. This engine is not written for a
  person to read; a lens can always show it. What a thing is, takes and gives is itself a node
  that programs read (`form/operations.ck`), and knowledge is rows of data before it is code.
  A part read by a bare number, a raw constant inside a definition and a ladder of cases are
  prelude: like the bootstrap they stand only until they can be replaced. `make check` counts
  the bare reads in the nodes against a budget that is lowered and never raised.
- **Every part can be replaced.** The bootstrap's code is one implementation of each part; a
  recipe or machine code can take its place while the system runs.
- **Follow a real flow.** Make something run end to end and watch it through the framebuffer. A
  running program says more than a check, a table or a page about it.
- **Only what is.** Pages and nodes say what is true now, briefly and at a high level. History
  lives in git; what no longer matches is taken out.
- **The framebuffer is the memory.** What the runtime keeps lives in the shared region with its
  blueprint and its writer, so seeing it needs nothing added.
- **The real work over the shortcut.** When the aligned way cannot run yet, build what it needs,
  or say plainly in `FLOOR.md` that it is not there.
- **Say how a thing is known**, in plain words someone outside the session can follow.

## Code that holds

Written this way, code holds the first time and is not written again (graph nodes 7.24, 7.25).

- **Facts are rows, choices are choices.** A target, a template, a rule or a relation is a row
  that code reads, written once. Where there is more than one way, ordered choice takes the
  first that holds.
- **Name the parts.** A record's parts are read by name; a jump names its label.
- **Bind before use.** A part that can fail or answer nothing is bound in a `let` before it is
  used: `all`, and so `tuple`, keeps neither and puts back what was done inside it.
- **Fault or fail, never guess.** A mistake of the code faults; a case it does not take fails
  and leaves a note.
- **Run it against its reference.** A new path comes with a ground program that runs it
  against its reference all the way down, over the boundaries of what it takes, and answers the
  cases that differ with counts of what it covered. A sweep run once in a session holds nothing.
- **Pin meaning.** An expectation holds what a program means: its answer, that two ways agree,
  an invariant. Never a size of code, a count of definitions, a label of coverage or a time:
  those move without a defect and hold a wrong value in place (graph node 7.26).
- **Mind two edges of the language.** A `let` takes the argument places after its recipe's
  own, so a recipe called with more arguments than it names cannot bind one. A node a
  definition steps sees only the bindings that definition sees, not those made after its layer.
- **Read it as a second reader would**, against these points, before it is committed: by
  running it, not only by reading it.

## Before a commit

`make check` and `./kg check` pass, a new path has its sweep among them, and
`git log --format=%ae | sort -u` shows only the address the existing commits carry.
