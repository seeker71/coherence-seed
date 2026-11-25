# Floor

What stands, as observed on **2026-11-25** on one Apple M4 Max running macOS.
[`NORTH_STAR.md`](NORTH_STAR.md) says where this is going.

## What runs

| Piece | Where | State |
| --- | --- | --- |
| Node encoding | [`spec/node-encoding.md`](spec/node-encoding.md) | written; the bootstrap's reader holds its 142 vectors, which are a node checked by a program in form |
| Bootstrap | [`bootstrap/`](bootstrap) | 909 lines of C under a 2,000-line budget; two commands, `run` and `text` |
| Framebuffer | [`spec/framebuffer.md`](spec/framebuffer.md) | all of the bootstrap's memory is one shared region, each record with its blueprint and writer |
| Walker | [`spec/core-operations.md`](spec/core-operations.md) | 46 operations: control with backtracking and a program's own fault, cells with a trail, arithmetic with division, a node's parts and its canonical bytes, native code, replacing what runs, reading, printing, files and the clock, the step itself, and observing an outcome |
| Swapping | [`form/`](form) | any operation, control included, any recipe, and the walker's step can be given another implementation while a program runs; add, sub, mul and lt exist in form and as machine code, the step as a marking one in form and a small one in machine code, and `compile` only in form: an emitter that writes a recipe's machine code |
| Programs as nodes | [`ground/`](ground) | 36 programs that witness the above, the encoding's conformance check among them; one more, `check.ck`, runs them and compares how each ends with the outcomes kept as a node in `expect.ck` |
| Lens | [`spec/lens.md`](spec/lens.md), [`lens/`](lens) | the first lens, written in form: words beside the numbers, and rows that turn a surface into the core and back; it reads its own definition |
| Ice | [`ice/`](ice) | written in form: a node frozen by one process thaws in another to the same node |
| Observer | [`tools/fb.c`](tools/fb.c) | shows a running region and what is bound in it, records it and replays it |
| Claim graph | [`graph/`](graph) | 1,347 nodes: the ground, the design, what is open, the path, and the evidence they stand on |

## Observed

- `make check` passes: the ground programs and the encoding's vectors, judged by programs in
  form; freeze and thaw, both in form, across two processes; native code unsigned and hardened;
  and a live run recorded from a second process.
- The encoding's conformance check, freeze and thaw were C commands and are programs in form. The
  form freeze wrote the same 1,875 bytes as the C freeze it replaced. The check written in form
  found the two vectors a careless conversion had spoiled, and reports planted wrong ones.
- A program keeps what it changes in cells, and a fail, the end of an `all` and of an `outcome`
  put back what a cell held. With its words in cells the lens's round trip takes 0.20 s where it
  took 0.37 s, and freezing the lens 0.04 s where it took 0.18 s, the file the same.
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
- A lens written in form shows its own stored definition, through its own rows, as 382 lines of
  text and reads that text back to the same node, byte for byte. A program written in it does the
  same for each of the 59 files of nodes here, in 0.2 s. Seven rows (if, first, when, not, let,
  def, tuple) are read and shown by one matcher; the matcher and the rows were written in the lens.
  Text it cannot read is a fault that names what and where. Its definitions call each other by
  name because they were bound as one layer.
- Keeping the walker's registers and each record's writer in the region costs about a quarter of
  its speed; recording every millisecond costs up to about 4%.
- `./kg check`: no errors over 1,168 claims, none of them reviewed by a second reader.

## Not there

- **Everything swappable** (path: graph node 7.21). Operations, recipes and the step are, and
  the encoding's bytes and ice are operations and programs. The store's own making of nodes and
  the other host functions are C with no binding, and so is what carries a run between nodes:
  the continuation, the choice points and the undo. A binding comes only from the
  running program and has no scope of its own.
- **Form everywhere.** Most operations exist only in C. Stored here are the bootstrap (two C
  files and an entitlements list) and 60 files of nodes in the canonical print, each of which
  reads and prints back to itself. Everything else stored is neither: the observer and the graph's
  checker (664 lines of C under `tools/`), the Makefile's 59 lines, nine pages, the claim graph
  as 51 text files, and the axioms' text.
- **A compiler worth the name.** The emitter in form knows numbers, one shape of condition and
  calls of the recipe itself. It compiles a recipe whole or not at all, runs only when a program
  asks, and cannot hand a node back to the walker, so it compiles no call of another recipe, no
  backtracking and nothing that builds a node.
- **The home lens.** The first lens's shapes are brackets only: no pattern over text, nothing
  infix, no names for a recipe's arguments, and no way to name a part and use it twice. Most
  stored programs were composed in numbers before it existed; they show through it and were not
  rewritten in it.
- **Checks in form.** The ground programs and the vectors are judged in form. That the ice
  crosses two processes, the hardened run and the recording are still lines of the Makefile's
  shell. There is no observer written in form.
- **A second reader** of the encoding, and a second hand to review any claim (6.1.30).
- **An answer to what a node's id is.** SHA-256 was chosen without being asked for (6.1.31).
- **Objects.** A cell holds a node and has a trail; it has no declared blueprint, no access or
  constructor recipe, no view, and nothing exempt from undo. Composed parents, threads and symbol
  lookup are not built.
- **Bounds that give way.** The region's areas have fixed capacities; a wrong recursion runs until
  one is full. Emitted code that runs out of machine stack declines and the walker runs the
  call, so the work is done twice.

## Guidance

Held together and kept short (graph node 7.13): only the axioms are fixed; form first, with C as
the bootstrap only; every part replaceable; real flows watched live before checks; fail undoes
everything back to the last choice; several parents are one composed shape; ice, water and gas
are states of any node; all memory is the framebuffer; pages say only what is.
