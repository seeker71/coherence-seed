# coherence-seed

One numeric space of immutable nodes, one grammar over it, and a small C bootstrap that the system
replaces with machine code it compiles for itself. It is built from five axioms, and only they are
fixed: [`axioms/core-axioms.form`](axioms/core-axioms.form).

- [`NORTH_STAR.md`](NORTH_STAR.md): where this is going.
- [`FLOOR.md`](FLOOR.md): what stands today, with the date it was observed.
- [`AGENTS.md`](AGENTS.md): where a new session starts.

It builds with a C compiler and `make`, and nothing else.

## Build and run

```bash
make
```

```bash
./ck run ground/recursion.ck
```

`ck` is the bootstrap: [`bootstrap/ck.c`](bootstrap/ck.c) with the memory layout
[`bootstrap/fb.h`](bootstrap/fb.h), under a budget of 2,000 lines that the build enforces. It holds
a node store, a reader and printer of the canonical print, a walker over the operations in
[`spec/core-operations.md`](spec/core-operations.md), and the birth of machine-code pages. The authoring commands are
`ck run` and `ck text`. The standalone `seed-run` executable and `ck execute` accept a frozen
image and calls by content id. A client can reuse a returned cell id or invoke the compiler
in the same session. The wire format and a runnable example are in [the NUMS specification](spec/nums.md).

## Read it and write it

What is stored is the canonical print, numbers and brackets, and beside it the names a person
gave to arguments. The lens ([`spec/lens.md`](spec/lens.md)), itself a program in form, shows a
stored node in words and names:

```bash
./ck text lens/to-words.ck lens/all.ck < lens/lens.ck
```

`ground/recursion.ck`, which is stored as `(#37 (#36 (#40 (#39 (#42 (#44 (#50 ..`, shows as

```
(call {n | (if (n < 2) n ((call (self) (n - 1)) + (call (self) (n - 2))))} 10)
```

The lens turns what is written that way into a node to store:

```bash
./ck text lens/to-nodes.ck lens/all.ck < words.txt > new.ck
```

## Watch it

All of the bootstrap's memory is one shared region, the **framebuffer**
([`spec/framebuffer.md`](spec/framebuffer.md)): every node, every frame, the walker's registers and
the machine stack. Each record starts with its blueprint and the program node that last wrote it.
A second process, [`tools/fb.c`](tools/fb.c), reads the region without the bootstrap knowing.

```bash
make watch
```

That runs a long recursion while a second process records its memory every 5 ms, replays the
recording, and reads from the last frame the answer and the node that wrote it. To look at any run:

```bash
CK_FB=/ck.demo ./ck run ground/recursion.ck
```

```bash
./fb show /ck.demo
```

or, from a program written in form, with every binding shown in the lens's words:

```bash
printf /ck.demo | ./ck text region/watch.ck
```

and where a run that is still going spends its time, by looking at it once a millisecond:

```bash
printf /ck.demo | ./ck text region/profile.ck
```

A running program can also be asked. A second process hands it a node, written in the lens's
words, and gets back how it ended, while the run goes on: put a counter on every addition, read
the count, take the counter off again.

```bash
printf '/ck.demo\n(held #1002)' | ./ck text region/ask.ck
```

It can be told to make itself faster. Start a recursion that takes a quarter of a minute, and from
a second terminal have it looked at and asked to compile what it is in most; it ends within a
second.

```bash
printf '(call {n | (if (lt n 2) n (add (call (self) (sub n 1)) (call (self) (sub n 2))))} 38)' | ./ck text lens/to-nodes.ck lens/all.ck > slow.ck
```

```bash
CK_FB=/ck.demo ./ck run slow.ck
```

```bash
printf /ck.demo | ./ck text region/warm.ck
```

That looks at the run three hundred times, finds the recipes it is in most and asks it to compile
them.

## Check it

```bash
make check
```

Eight lines: the header against the nodes it is made from; the ground programs, the encoding's 142
vectors among them, run and judged by a program written in form (`ground/check.ck`); a freeze and
a thaw, both in form, across two processes; a machine-code leaf both unsigned and under the
hardened runtime; a live run recorded from a second process; a run asked from a second process
to count its own additions while it goes on; a search that writes a file only for the path
that held; and the count of reads by a bare number left in the stored form, against its budget.

## Replace an operation

The bootstrap's C is one implementation of each operation. [`form/`](form) holds the same
operations written as nodes, and a running program can put any of them, or a new one, in place:

```bash
./ck run ground/recursion.ck form/add.ck form/sub.ck
```

`ground/on-form.ck` and `ground/on-code.ck` do it from inside a program, with `bind`. The reader
and the printer of the stored form are operations like any other, and they too are written in
form; this runs every ground program on them:

```bash
./ck run ground/check.ck form/read.ck form/print.ck
```

`ground/compiles-a-recipe.ck` loads an emitter written in form (`form/compile.aarch64.ck`), has it
write machine code for a recipe, and runs the recipe on that code. `ground/warms-itself.ck` does
it without being asked: a watcher counts the calls of a running recursion, has the hot recipe
compiled, binds the code and takes itself away, and the run ends on machine code.

The walker's step is replaced the same way. This marks every node the program evaluates:

```bash
./ck run ground/recursion.ck form/step.traced.ck
```

## The knowledge graph

What is known, decided and open is held as a graph of claims under [`graph/`](graph). Each claim
carries its source, its date and how it is known. `make` builds its checker, `kg`.

```bash
./kg brief | less
```

`graph/0.kg` is the root: ten hubs, each a one-line summary of everything under it.

| Hub | Holds |
| --- | --- |
| 1 ground | the five axioms word for word, their theorems, what they force on a kernel |
| 2 sources | every file, repository, paper and page a kept claim cites, with dates |
| 3 references read | NUMS.Go, Form, BML and an earlier kernel, as far as the design refers to them |
| 4 prior art | what systems outside this one show, as far as the design refers to it |
| 5 design | the architecture, decision by decision; 5.13 and 5.14 are what is built |
| 6 open | questions, assumptions and limits: everything that can move |
| 7 path | the stages ahead |
| 8 graph | how this graph is written, checked and reviewed |
| 9 reviews | independent readings that confirm single claims; none at present |

A node is one block of `key: value` lines. Its address (`n`) is the handle other nodes cite.
`status` says how the claim is known: `given` (the guide's word), `observed`, `relayed`, `inferred`,
`assumed` or `open`.

| Command | Does |
| --- | --- |
| `./kg check` | enforces the graph's rules and counts confirmed and unreviewed claims |
| `./kg link N` | prints where the sources of node N can be read |
| `./kg release` | takes out every source, piece of evidence and review that nothing living refers to |

## Layout

| Path | Holds |
| --- | --- |
| `axioms/` | the five axioms, the only fixed thing |
| `bootstrap/` | the C bootstrap and the framebuffer's layout |
| `spec/` | the node encoding and its vectors, the core operations, the framebuffer, the lens, and objects |
| `lens/` | the lens as nodes: its reader, printer and matcher, its words, its rows, and two programs that run it on standard input |
| `ice/` | freeze and thaw as nodes |
| `region/` | the layout of the region's records as a node, and the programs that write the C header from it, show a region, ask a running program, and say where it spends its time |
| `objects/` | shapes, cells and references as nodes |
| `ground/` | programs that witness the floor; `check.ck` runs them against the outcomes in `expect.ck` |
| `form/` | operations written in form and as machine code, the reader and printer of the stored form, the emitter, and the watcher |
| `tools/` | the graph checker `kg`, the observer `fb`; both are C until generated code replaces them |
| `graph/` | the claims |
