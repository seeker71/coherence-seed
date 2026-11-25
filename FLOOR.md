# Floor

What stands, as observed on **2026-11-25** on one Apple M4 Max running macOS.
[`NORTH_STAR.md`](NORTH_STAR.md) says where this is going.

## What runs

| Piece | Where | State |
| --- | --- | --- |
| Node encoding | [`spec/node-encoding.md`](spec/node-encoding.md) | written; the bootstrap's reader holds its 142 vectors |
| Bootstrap | [`bootstrap/`](bootstrap) | 884 lines of C under a 2,000-line budget |
| Framebuffer | [`spec/framebuffer.md`](spec/framebuffer.md) | all of the bootstrap's memory is one shared region, each record with its blueprint and writer |
| Walker | [`spec/core-operations.md`](spec/core-operations.md) | 31 operations: control with backtracking, arithmetic, a node's parts, native code, and replacing what runs |
| Swapping | [`form/`](form) | any operation, control included, and any recipe can be given another implementation while a program runs; add, sub, mul and lt exist in form and as machine code |
| Programs as nodes | [`ground/`](ground) | 25 programs that witness the above |
| Ice | `ck freeze`, `ck thaw` | a node frozen in one process thaws in another to the same id |
| Observer | [`tools/fb.c`](tools/fb.c) | shows a running region, records it and replays it |
| Claim graph | [`graph/`](graph) | 1,329 nodes: the ground, the design, what is open, the path, and the evidence they stand on |

## Observed

- `make check` passes: the vectors, the ground programs, freeze and thaw across two processes,
  native code unsigned and hardened, and a live run recorded from a second process.
- `make watch`: the long recursion, 158.6 million walker steps, runs in about 0.71 s while a second
  process records it and reads the answer and the node that wrote it from the last frame.
- A program takes a node apart to its bytes and builds it again; the answer is the same node. A
  rule of the encoding runs as nodes. A build the encoding would refuse answers nothing.
- A program replaces an operation while it runs, by a recipe or by machine code, and the programs
  answer the same. A replacement runs on what it replaced, and a fail takes it away again.
- A diagnostic is put on every use of `choice` and the answers do not change; the marks it leaves
  are read from the region. One recipe is tracked on each of its calls, then released. A control
  operation and a new kind of implementation are brought into being by a program. The
  long recursion takes 9.3 s on four form operations, 0.80 s on four machine-code leaves, and
  0.75 s on the bootstrap's own.
- Keeping the walker's registers and each record's writer in the region costs about a quarter of
  its speed; recording every millisecond costs up to about 4%.
- `./kg check`: no errors over 1,151 claims, none of them reviewed by a second reader.

## Not there

- **Everything swappable** (path: graph node 7.21). Operations and recipes are; the store, the
  reader, the printer, ice, the host functions and the walker's own step are C with no binding.
  A binding comes only from the running program and has no scope of its own.
- **Form everywhere.** Most operations exist only in C. The Makefile's shell, the two C tools,
  these pages, the graph's blocks and the tables are not form nodes.
- **A compiler.** Machine code is written by hand, one leaf an operation; the walker still drives
  every step.
- **A way to write form.** Programs are written by hand in the canonical print; there is no home
  lens, and no check or observer written in form.
- **A second reader** of the encoding, and a second hand to review any claim (6.1.30).
- **An answer to what a node's id is.** SHA-256 was chosen without being asked for (6.1.31).
- **Cells and the journal**, recipes that name each other, threads, the clock, symbol lookup.
- **Bounds that give way.** The region's areas have fixed capacities; a wrong recursion runs until
  one is full.

## Guidance

Held together and kept short (graph node 7.13): only the axioms are fixed; form first, with C as
the bootstrap only; every part replaceable; real flows watched live before checks; fail undoes
everything back to the last choice; several parents are one composed shape; ice, water and gas
are states of any node; all memory is the framebuffer; pages say only what is.
