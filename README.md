# coherence-seed

A restart of the [coherence-kernel](https://github.com/seeker71/coherence-kernel) from its five
axioms. The aim is one numeric space of immutable nodes, one grammar over it, and a small C
bootstrap that the system replaces with machine code it compiles for itself.

Only the axioms are fixed: [`axioms/core-axioms.form`](axioms/core-axioms.form), carried byte for
byte. Everything else can move, and says how it is known.

This repository builds with a C compiler and `make`, and nothing else. The old kernel and its
runtime are evidence that was read; they are not a tool, a dependency or a step of any build here.

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
[`spec/core-operations.md`](spec/core-operations.md), and the birth of machine-code pages.

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

## Check it

```bash
make check
```

Five lines: the 127 encoding vectors through the bootstrap's own reader, the 14 ground programs, a
freeze and thaw across two processes, a machine-code leaf both unsigned and under the hardened
runtime, and a live run recorded from a second process.

## The knowledge graph

What is known, decided and open is held as a graph of claims under [`graph/`](graph). Each claim
carries its source, its date and how it is known.

```bash
cc -O2 -o kg tools/kg.c
```

```bash
./kg brief | less
```

`graph/0.kg` is the root: ten hubs, each a one-line summary of everything under it.

| Hub | Holds |
| --- | --- |
| 1 ground | the five axioms word for word, their theorems, what they force on a kernel |
| 2 sources | every file, repository, paper and page a claim cites, with dates |
| 3 references read | NUMS.Go, Form, BML, the old runtime and its lessons, as read and run |
| 4 prior art | what systems outside this body show |
| 5 design | the architecture, decision by decision; 5.13 and 5.14 are what is built |
| 6 open | questions, assumptions and limits: everything that can move |
| 7 path | the stages from here to the hardware's speed |
| 8 graph | how this graph is written, checked and reviewed |
| 9 rounds | the study, design, build and review rounds, and the reviews of single nodes |

A node is one block of `key: value` lines. Its address (`n`) is the handle other nodes cite.
`status` says how the claim is known: `given` (the owner's word), `observed`, `relayed`, `inferred`,
`assumed` or `open`.

```bash
./kg check
```

The last line counts the claims a review has confirmed against their present wording, the ones
disputed, the ones changed since their review, and the ones nobody has reviewed. An unreviewed
claim is not a refuted one.

```bash
./kg link 5.14.13
```

prints where the sources of a node can be read: a file in this repository, or a web address pinned
to the revision that was read. The checker refuses a source that does not resolve.

## Where it stands

Beside the axioms, the owner has given these, each recorded word for word:

- `fail` is backtracking with undo, and an offer whose every option failed receives nothing (5.4.2)
- several parents mean one composed shape (5.5.4)
- ice, water and gas are states of any node (5.6.2)
- the framebuffer is all memory in one shared space, recorded and replayed at no cost (5.7.6, 5.7.7)
- the work follows real end-to-end flows, watched live (7.10)
- this repository is built with a C compiler alone (7.11)

Stage 1, the node encoding ([`spec/node-encoding.md`](spec/node-encoding.md), 127 vectors), is
written; it closes when a second hand has written a reader from the page alone. Stage 2, the floor,
stands on the framebuffer. Stage 3 is next: the home lens, written as rows in the grammar.

Not established: no second hand has reviewed the design, the specifications or the code, and only
one machine has built them. Of 1,687 claims, 666 are confirmed by a review, 114 changed after their
review and 907 are unreviewed.

## Layout

| Path | Holds |
| --- | --- |
| `axioms/` | the five axioms, the only fixed thing |
| `bootstrap/` | the C bootstrap and the framebuffer's layout |
| `spec/` | the node encoding, the core operations, the framebuffer, and the vectors |
| `ground/` | programs that witness the floor, with their expected answers |
| `tools/` | the graph checker `kg`, the encoding's reference reader `enc`, the observer `fb` |
| `graph/` | the claims |
| `study/` | the full reports of the study rounds of 2026-11-24 |
| `sources/notes/` | working notes of the old body's sessions that claims cite |
| `AGENTS.md` | where a new session starts |
