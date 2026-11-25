# Floor

What stands, as observed on **2026-11-25** on one Apple M4 Max running macOS.
[`NORTH_STAR.md`](NORTH_STAR.md) says where this is going.

## What runs

| Piece | Where | State |
| --- | --- | --- |
| Node encoding, version 1 | [`spec/node-encoding.md`](spec/node-encoding.md), 142 vectors | written, with seventeen refusals; the bootstrap's reader holds the vectors; a second reader is missing |
| Bootstrap | [`bootstrap/ck.c`](bootstrap/ck.c), [`bootstrap/fb.h`](bootstrap/fb.h) | 800 lines of a 2,000-line budget the build enforces |
| Framebuffer | [`spec/framebuffer.md`](spec/framebuffer.md) | all of the bootstrap's memory is one shared region; it keeps no heap |
| Walker | [`spec/core-operations.md`](spec/core-operations.md) | 21 operations; choice is re-entered after it has answered; its registers are a record in the region |
| Native code | catalogue entry 52 | an AArch64 leaf runs unsigned and under the hardened runtime |
| Ice | `ck freeze`, `ck thaw` | a node frozen in one process thaws in another to the same id |
| Observer | [`tools/fb.c`](tools/fb.c) | shows a running region, records it as frames, replays a recording |
| Claim graph | [`graph/`](graph), [`tools/kg.c`](tools/kg.c) | 1,322 nodes: the ground, the design, what is open, the path, and only the evidence and sources they refer to; every source resolves |

## Observed

- `make check` prints five passing lines: the 142 vectors through the bootstrap's own reader, the
  14 ground programs, freeze and thaw across two processes, native code unsigned and hardened, and
  a live run recorded from a second process.
- Each of those lines has been turned red by a planted fault: a wrong expected answer, a changed
  and a truncated ice file, a hardened binary without the JIT entitlement, a run too short to be
  seen in motion, and a 100-line budget.
- The bootstrap's reader starts with catalogue entries 0 to 63 and refuses bytes as `unknown` when
  their blueprint or a child is the id of no node it holds: a leaf under entry 64 is refused until
  entry 64 itself has been read.
- `make watch`: the long recursion (158.6 million walker steps) runs in 0.71 to 0.72 s. A second
  process records it every 5 ms, replays it, and reads from the last frame the answer 2178309 and
  the `add` node that wrote it.
- Recording every 1 ms slows that run by up to about 4%. Keeping the walker's registers and every
  record's writer in the region costs it about a quarter of its speed, measured against a build
  with registers in C locals.
- `./kg check`: 0 errors, 0 warnings over 1,144 claims. None is confirmed by an independent
  reading, and 64 pieces of evidence are marked assumed because no source for them is kept here.
- `./kg release` finds nothing to take out: every source and piece of evidence is referred to by
  the design, a question or a limit.

## Not there

- Code written in the grammar. Every line that runs is C. Outside the bootstrap that is the graph
  checker and the observer; each is owed a counterpart generated from its specification (7.13,
  7.15).
- A second reader of the encoding. It is owed as nodes written from the page alone (7.2); the
  walker has no operation that takes a node apart or reads a leaf's bytes, so it cannot run one.
- Memory that grows. The framebuffer's areas, the code region and the graph checker's tables have
  fixed capacities, and a run that outgrows one stops (7.16).
- The host functions for symbol lookup, the framework trampoline, thread start and the clock.
- A cell a program can change in place, and the journal that undoes it.
- Recipes that name each other; operations that build or take apart nodes.
- A second hand: one hand wrote the specifications, the bootstrap and the observer, and one machine
  built them. No claim in the graph is reviewed. Who counts as a second hand is question 6.1.30.
- Machine registers, executable pages and the C library's own memory lie outside the framebuffer.

## What the owner has given

Each is recorded word for word in the node named.

| Node | Given |
| --- | --- |
| 1 | only the five axioms are fixed |
| 5.4.2 | fail means backtrack, undo and reverse all changes until a new choice or a sequence is tried; an offer whose every option failed receives nothing |
| 5.5.4 | several parents mean one composed shape |
| 5.6.2 | ice, water and gas are states of any node |
| 5.7.6, 5.7.7 | the framebuffer: all memory in one shared space, recorded like video at no cost and played back, with blueprint and last writer beside each object |
| 7.10 | follow real end-to-end flows, watched live |
| 7.11 | this repository is built with a C compiler alone |
| 7.13 | new code is written in the grammar and stored as nodes; the only C is the bootstrap; all else is generated from specification to nodes to machine code, every phase cached |
| 7.14 | numbers in code are fine as long as they are not committed limits |
| 7.17 | always do the hard work, never commit short cuts, keep the repository clean and fully aligned |
| 8.14 | documents keep only what is; history lives in git; a north star and a dated floor; what is not needed to close the gap between them is released |
