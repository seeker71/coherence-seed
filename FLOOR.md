# Floor

What stands, as observed on **2026-11-25** on one Apple M4 Max running macOS.
[`NORTH_STAR.md`](NORTH_STAR.md) says where this is going.

## What runs

| Piece | Where | State |
| --- | --- | --- |
| Node encoding, version 1 | [`spec/node-encoding.md`](spec/node-encoding.md), 142 vectors | written; two readers hold the vectors |
| Second reader | [`tools/second.c`](tools/second.c) | written from the page without sight of the bootstrap; reads, prints, freezes and thaws; a witness beside the system, with its nodes on the C heap |
| Bootstrap | [`bootstrap/ck.c`](bootstrap/ck.c), [`bootstrap/fb.h`](bootstrap/fb.h) | 800 lines of a 2,000-line budget the build enforces |
| Framebuffer | [`spec/framebuffer.md`](spec/framebuffer.md) | all of the bootstrap's memory is one shared region; it keeps no heap |
| Walker | [`spec/core-operations.md`](spec/core-operations.md) | 21 operations; choice is re-entered after it has answered; its registers are a record in the region |
| Native code | catalogue entry 52 | an AArch64 leaf runs unsigned and under the hardened runtime |
| Ice | `ck freeze`, `ck thaw` | a node frozen in one process thaws in another to the same id |
| Observer | [`tools/fb.c`](tools/fb.c) | shows a running region, records it as frames, replays a recording |
| Claim graph | [`graph/`](graph), [`tools/kg.c`](tools/kg.c) | 1,321 nodes: the ground, the design, what is open, the path, and only the evidence and sources they refer to; every source resolves |

## Observed

- `make check` prints seven passing lines: the 142 vectors through the bootstrap's own reader, the
  14 ground programs, freeze and thaw across two processes, native code unsigned and hardened, a
  live run recorded from a second process, the 142 vectors through the second reader, and the two
  hands together.
- The two hands: each of the 14 ground programs frozen by either reader thaws in the other to the
  same id and print, and the two ice files are the same bytes. The second reader writes 200,000
  variations of the vectors with its verdict on each, and all 200,049 lines hold in both readers.
- `make hands`: the bootstrap reads those lines in about 30 ms while a second process records its
  region. The node table grows from 65 records to 8,788: its 64 entries, the record for nothing,
  and the 8,723 nodes the second reader holds.
- Each line has been turned red by a planted fault: a wrong expected answer, a changed and a
  truncated ice file, a hardened binary without the JIT entitlement, a run too short to be seen in
  motion, and a 100-line budget; in the second reader a loosened integer rule, children frozen
  before their blueprint, a node counted twice and a run with no variations.
- A 32-bit integer read as eight bytes wide passed the vectors and was caught only by the
  variations. An overlong three-byte text passed both and is now a vector.
- Run on the same bytes, the bootstrap's reader refused with a word the page did not have,
  `unknown`: a blueprint or child the reader does not hold. It is the page's seventeenth refusal.
- `make watch`: the long recursion (158.6 million walker steps) runs in 0.71 to 0.72 s. A second
  process records it every 5 ms, replays it, and reads from the last frame the answer 2178309 and
  the `add` node that wrote it.
- Recording every 1 ms slows that run by up to about 4%. Keeping the walker's registers and every
  record's writer in the region costs it about a quarter of its speed, measured against a build
  with registers in C locals.
- `./kg check`: 0 errors, 0 warnings over 1,143 claims. None is confirmed by an independent
  reading, and 64 pieces of evidence are marked assumed because no source for them is kept here.
- `./kg release` finds nothing to take out: every source and piece of evidence is referred to by
  the design, a question or a limit.

## Not there

- The host functions for symbol lookup, the framework trampoline, thread start and the clock.
- A cell a program can change in place, and the journal that undoes it.
- Recipes that name each other; operations that build or take apart nodes.
- A hand of another kind: both readers of the encoding were written by sessions of one language
  model on one machine. No person and no other model has read a page here, and no claim in the
  graph is reviewed. Whether the second reader closes stage 1 is question 6.1.30.
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
| 8.14 | documents keep only what is; history lives in git; a north star and a dated floor; what is not needed to close the gap between them is released |
