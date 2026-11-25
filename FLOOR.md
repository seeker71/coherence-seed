# Floor

What stands, as observed on **2026-11-25** on one Apple M4 Max running macOS.
[`NORTH_STAR.md`](NORTH_STAR.md) says where this is going.

## What runs

| Piece | Where | State |
| --- | --- | --- |
| Node encoding, version 1 | [`spec/node-encoding.md`](spec/node-encoding.md), 127 vectors | written; one reader by a second hand is missing |
| Bootstrap | [`bootstrap/ck.c`](bootstrap/ck.c), [`bootstrap/fb.h`](bootstrap/fb.h) | 800 lines of a 2,000-line budget the build enforces |
| Framebuffer | [`spec/framebuffer.md`](spec/framebuffer.md) | all of the bootstrap's memory is one shared region; it keeps no heap |
| Walker | [`spec/core-operations.md`](spec/core-operations.md) | 21 operations; choice is re-entered after it has answered; its registers are a record in the region |
| Native code | catalogue entry 52 | an AArch64 leaf runs unsigned and under the hardened runtime |
| Ice | `ck freeze`, `ck thaw` | a node frozen in one process thaws in another to the same id |
| Observer | [`tools/fb.c`](tools/fb.c) | shows a running region, records it as frames, replays a recording |
| Claim graph | [`graph/`](graph), [`tools/kg.c`](tools/kg.c) | 2,552 nodes; every source resolves |

## Observed

- `make check` prints five passing lines: the 127 vectors through the bootstrap's own reader, the
  14 ground programs, freeze and thaw across two processes, native code unsigned and hardened, and
  a live run recorded from a second process.
- Each of those lines has been turned red by a planted fault: a wrong expected answer, a changed
  and a truncated ice file, a hardened binary without the JIT entitlement, a run too short to be
  seen in motion, and a 100-line budget.
- `make watch`: the long recursion (158.6 million walker steps) runs in 0.71 to 0.72 s. A second
  process records it every 5 ms, replays it, and reads from the last frame the answer 2178309 and
  the `add` node that wrote it.
- Recording every 1 ms slows that run by up to about 4%. Keeping the walker's registers and every
  record's writer in the region costs it about a quarter of its speed, measured against a build
  with registers in C locals.
- `./kg check`: 0 errors, 0 warnings. Of 1,675 claims, 666 are confirmed by an independent reading
  of their present wording and 1,009 are unreviewed.

## Not there

- The host functions for symbol lookup, the framework trampoline, thread start and the clock.
- A cell a program can change in place, and the journal that undoes it.
- Recipes that name each other; operations that build or take apart nodes.
- A second hand: one hand wrote the specifications, the bootstrap and the observer, and one machine
  built them. Hubs 1, 5, 6, 7 and 8 of the graph have never been reviewed.
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
| 8.14 | documents keep only what is; history lives in git; a north star and a dated floor |
