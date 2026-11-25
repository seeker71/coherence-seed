# Floor

What stands, as observed on **2026-11-25** on one Apple M4 Max running macOS.
[`NORTH_STAR.md`](NORTH_STAR.md) says where this is going.

## What runs

| Piece | Where | State |
| --- | --- | --- |
| Node encoding | [`spec/node-encoding.md`](spec/node-encoding.md) | written; the bootstrap's reader holds its 142 vectors |
| Bootstrap | [`bootstrap/`](bootstrap) | 923 lines of C under a 2,000-line budget |
| Framebuffer | [`spec/framebuffer.md`](spec/framebuffer.md) | all of the bootstrap's memory is one shared region, each record with its blueprint and writer |
| Walker | [`spec/core-operations.md`](spec/core-operations.md) | 37 operations: control with backtracking, arithmetic, a node's parts, native code, replacing what runs, reading, printing, files and the clock, and the step itself |
| Swapping | [`form/`](form) | any operation, control included, any recipe, and the walker's step can be given another implementation while a program runs; add, sub, mul and lt exist in form and as machine code, the step as a marking one in form and a small one in machine code, and `compile` only in form: an emitter that writes a recipe's machine code |
| Programs as nodes | [`ground/`](ground) | 31 programs that witness the above |
| Ice | `ck freeze`, `ck thaw` | a node frozen in one process thaws in another to the same id |
| Observer | [`tools/fb.c`](tools/fb.c) | shows a running region, records it and replays it |
| Claim graph | [`graph/`](graph) | 1,333 nodes: the ground, the design, what is open, the path, and the evidence they stand on |

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
- A program reads another program from a file, turns its print into a node and runs it. A program
  times one workload on two implementations of `add`, 36 ms in form and 15 ms as machine code,
  and binds the faster.
- The walker's step is replaced while a program runs. A step in form marks every node and hands
  it on: 17 marks for the 17 distinct nodes of the small recursion, the same answer, and a fail
  takes the marking away. A step of eleven machine instructions reads each node's record in the
  region, answers the leaves and declines the rest to the step beneath it. The long recursion
  takes 0.80 s on the walker alone, 1.39 s under that code step and 8.6 s under the marking step.
  A `cut` reaches the program that used an implementation from inside a helper recipe.
- An emitter written in form compiles a recipe: for a*a + b*3 - 7 it writes one leaf of 34
  instructions and binds the recipe to it. The answers are the same, an overflow is still out of
  range, and a million calls take 0.27 s where the walker takes 0.62 s. Compiling takes about
  1,500 walker steps. A recipe it does not know stays on the walker.
- Keeping the walker's registers and each record's writer in the region costs about a quarter of
  its speed; recording every millisecond costs up to about 4%.
- `./kg check`: no errors over 1,155 claims, none of them reviewed by a second reader.

## Not there

- **Everything swappable** (path: graph node 7.21). Operations, recipes and the step are. The
  store, ice and the other host functions are C with no binding, and so is what carries a run
  between nodes: the continuation, the choice points and the undo. A binding comes only from the
  running program and has no scope of its own.
- **Form everywhere.** Most operations exist only in C. The Makefile's shell, the two C tools,
  these pages, the graph's blocks and the tables are not form nodes.
- **A compiler worth the name.** The emitter in form knows five kinds of node, compiles a recipe
  whole or not at all, and runs only when a program asks. It cannot hand a node back to the
  walker, so it compiles no call and no control, and the walker is still on every running path.
- **A way to write form.** Programs are written by hand in the canonical print; there is no home
  lens, and no check or observer written in form.
- **A second reader** of the encoding, and a second hand to review any claim (6.1.30).
- **An answer to what a node's id is.** SHA-256 was chosen without being asked for (6.1.31).
- **Cells and the journal**, recipes that name each other, threads, symbol lookup.
- **Bounds that give way.** The region's areas have fixed capacities; a wrong recursion runs until
  one is full.

## Guidance

Held together and kept short (graph node 7.13): only the axioms are fixed; form first, with C as
the bootstrap only; every part replaceable; real flows watched live before checks; fail undoes
everything back to the last choice; several parents are one composed shape; ice, water and gas
are states of any node; all memory is the framebuffer; pages say only what is.
