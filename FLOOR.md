# Floor

What stands, as observed on **2026-11-25** on one Apple M4 Max running macOS.
[`NORTH_STAR.md`](NORTH_STAR.md) says where this is going.

## What runs

| Piece | Where | State |
| --- | --- | --- |
| Node encoding | [`spec/node-encoding.md`](spec/node-encoding.md) | written; the bootstrap's reader holds its 142 vectors |
| Bootstrap | [`bootstrap/`](bootstrap) | 979 lines of C under a 2,000-line budget |
| Framebuffer | [`spec/framebuffer.md`](spec/framebuffer.md) | all of the bootstrap's memory is one shared region, each record with its blueprint and writer |
| Walker | [`spec/core-operations.md`](spec/core-operations.md) | 42 operations: control with backtracking and a program's own fault, arithmetic with division, a node's parts, native code, replacing what runs, reading, printing, files and the clock, the step itself, and observing an outcome |
| Swapping | [`form/`](form) | any operation, control included, any recipe, and the walker's step can be given another implementation while a program runs; add, sub, mul and lt exist in form and as machine code, the step as a marking one in form and a small one in machine code, and `compile` only in form: an emitter that writes a recipe's machine code |
| Programs as nodes | [`ground/`](ground) | 33 programs that witness the above; one more, `check.ck`, runs them and compares how each ends with the outcomes kept as a node in `expect.ck` |
| Lens | [`spec/lens.md`](spec/lens.md), [`lens/`](lens) | the first lens, written in form: words beside the numbers, read into nodes and shown back; it reads its own definition |
| Ice | `ck freeze`, `ck thaw` | a node frozen in one process thaws in another to the same id |
| Observer | [`tools/fb.c`](tools/fb.c) | shows a running region and what is bound in it, records it and replays it |
| Claim graph | [`graph/`](graph) | 1,343 nodes: the ground, the design, what is open, the path, and the evidence they stand on |

## Observed

- `make check` passes: the vectors, the ground programs, freeze and thaw across two processes,
  native code unsigned and hardened, and a live run recorded from a second process.
- `make watch`: the long recursion, 158.6 million walker steps, runs in about 0.75 s while a second
  process records it and reads the answer and the node that wrote it from the last frame.
- A program takes a node apart to its bytes and builds it again; the answer is the same node. A
  rule of the encoding runs as nodes. A build the encoding would refuse answers nothing.
- A program replaces an operation while it runs, by a recipe or by machine code, and the programs
  answer the same. A replacement runs on what it replaced, and a fail takes it away again.
- A diagnostic is put on every use of `choice` and the answers do not change; the marks it leaves
  are read from the region. One recipe is tracked on each of its calls, then released. A control
  operation and a new kind of implementation are brought into being by a program. The
  long recursion takes 9.4 s on four form operations, 0.90 s on four machine-code leaves, and
  0.75 s on the bootstrap's own. A fault inside an `all` reaches the `recover` around it.
- A program reads another program from a file, turns its print into a node and runs it. A program
  times one workload on two implementations of `add`, 36 ms in form and 15 ms as machine code,
  and binds the faster.
- The walker's step is replaced while a program runs. A step in form marks every node and hands
  it on: 17 marks for the 17 distinct nodes of the small recursion, and the same answer. A step
  that reads every 2 as 3 changes the answer inside the recipes a program calls, and a fail takes
  it away. A step of ten machine instructions reads each node's record in the region, answers the
  leaves and declines the rest to the step beneath it. The long recursion takes 0.75 s on the
  walker alone, 1.29 s under that code step and 8.1 s under the marking step.
  A `cut` reaches the program that used an implementation from inside a helper recipe.
- An emitter written in form compiles a recipe and binds it to the code. For the recursion it
  writes one leaf of 67 instructions; on it the long recursion answers the same in 0.02 s and
  7,945 walker steps, loading and compiling included, where the walker takes 0.75 s and 158.6
  million. The leaf answers a number or declines, and the walker runs what it declines: in
  seventeen trials by hand the answer, the reason and the fault were the walker's own, through
  overflows, missing and wrong arguments and a recursion a million deep. A recipe the emitter
  does not know stays on the walker.
- A lens written in form shows its own stored definition as text and reads that text back to the
  same node, byte for byte. A program written in words does the same for each of the 50 files of
  nodes here, in 0.08 s. Text it cannot read is a fault that names what and where. Its 32
  definitions call each other by name because they were bound as one layer.
- Keeping the walker's registers and each record's writer in the region costs about a quarter of
  its speed; recording every millisecond costs up to about 4%.
- `./kg check`: no errors over 1,164 claims, none of them reviewed by a second reader.

## Not there

- **Everything swappable** (path: graph node 7.21). Operations, recipes and the step are. The
  store, ice and the other host functions are C with no binding, and so is what carries a run
  between nodes: the continuation, the choice points and the undo. A binding comes only from the
  running program and has no scope of its own.
- **Form everywhere.** Most operations exist only in C. Stored here are the bootstrap (two C
  files and an entitlements list) and 51 files of nodes in the canonical print, each of which
  reads and prints back to itself. Everything else stored is neither: the observer and the graph's
  checker (664 lines of C under `tools/`), the Makefile's 59 lines, nine pages, the claim graph
  as 50 text files, the table of the encoding's vectors and the axioms' text.
- **A compiler worth the name.** The emitter in form knows numbers, one shape of condition and
  calls of the recipe itself. It compiles a recipe whole or not at all, runs only when a program
  asks, and cannot hand a node back to the walker, so it compiles no call of another recipe, no
  backtracking and nothing that builds a node.
- **The home lens.** The first lens is words and leaves. It has no rule rows of pattern and
  template, no names for a recipe's arguments, and no way to name a part and use it twice; it
  shows on one line. Most stored programs were composed in numbers before it existed.
- **Checks in form.** One check is in form, the ground programs'; the vectors, the ice, the
  hardened run and the recording are still checked by C and the Makefile's shell. There is no
  observer written in form.
- **A second reader** of the encoding, and a second hand to review any claim (6.1.30).
- **An answer to what a node's id is.** SHA-256 was chosen without being asked for (6.1.31).
- **Cells and the journal**, recipes that name each other, threads, symbol lookup.
- **Bounds that give way.** The region's areas have fixed capacities; a wrong recursion runs until
  one is full. Emitted code that runs out of machine stack declines and the walker runs the
  call, so the work is done twice.

## Guidance

Held together and kept short (graph node 7.13): only the axioms are fixed; form first, with C as
the bootstrap only; every part replaceable; real flows watched live before checks; fail undoes
everything back to the last choice; several parents are one composed shape; ice, water and gas
are states of any node; all memory is the framebuffer; pages say only what is.
