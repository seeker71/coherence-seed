# Floor

What stands, as observed on **2026-11-25** on one Apple M4 Max running macOS.
[`NORTH_STAR.md`](NORTH_STAR.md) says where this is going.

## What runs

| Piece | Where | State |
| --- | --- | --- |
| Node encoding | [`spec/node-encoding.md`](spec/node-encoding.md) | written, with 142 conformance vectors kept as a node |
| Bootstrap | [`bootstrap/`](bootstrap) | 983 lines of C under a 2,000-line budget, with two commands: run a program, and run it and write its answer as text |
| Framebuffer | [`spec/framebuffer.md`](spec/framebuffer.md) | all of the bootstrap's memory is one shared region, each record with its blueprint and writer |
| Walker | [`spec/core-operations.md`](spec/core-operations.md) | 54 operations: control with backtracking and counting, arithmetic, a node's parts and bytes, cells, binding what runs, reading and printing, the step itself, the bytes of its own region and of another's, and any function of the host by name |
| Swapping | [`form/`](form) | any operation, any recipe and the walker's step can be given another implementation, and have it taken away, while a program runs |
| Emitter | [`form/compile.aarch64.ck`](form/compile.aarch64.ck) | written in form: compiles a recursion over numbers to one machine-code leaf |
| Lens | [`spec/lens.md`](spec/lens.md), [`lens/`](lens) | written in form: words beside the numbers, and rows that turn a surface into the core and back |
| Ice | [`ice/`](ice) | written in form: a node frozen by one process thaws in another |
| Programs | [`ground/`](ground) | 40 that witness the above; one more runs them and judges how each ends |
| Region | [`region/`](region) | the layout of the region's records is a node: the C header is made from it, and a program in form shows its own region or another process's through it |
| Observer | [`tools/fb.c`](tools/fb.c) | in C, from a second process: records a run in motion and replays it |
| Claim graph | [`graph/`](graph) | 1,355 nodes: the ground, the design, what is open, the path, and the evidence they stand on |

## Observed

- **`make check` passes.** The ground programs and the encoding's vectors are judged by programs
  in form; freeze and thaw, in form, cross two processes; native code runs unsigned and hardened;
  a live run is recorded from a second process.
- **What runs can be exchanged while it runs.** An operation is replaced by a recipe or by machine
  code and the programs answer the same; a replacement runs on what it replaced; a fail or an
  `unbind` takes it away. A diagnostic sits on every use of an operation, or on every step, and
  the answers do not change. The long recursion takes 0.75 s on the bootstrap's own operations,
  0.90 s on four machine-code leaves and 9.4 s on four written in form.
- **A run makes itself faster.** A watcher written in form counts calls, has the emitter compile
  a recipe at its hundredth call and takes its place again above the code. Under it the long
  recursion starts on the walker and ends on machine code with the same answer, in 0.03 s and
  about 1.6 million walker steps for three recursions, where the walker takes 158.6 million for
  the first alone. It stays for the next hot recipe and rests when nothing new comes.
- **Emitted code is the walker or faster, never different.** It answers a number or declines,
  and the walker runs what it declines: through overflows, missing and wrong arguments and a
  recursion deeper than the machine stack, the answer, the reason and the fault were the
  walker's own in seventeen trials by hand.
- **The lens reads its own definition.** It shows its stored node, through its own rows, as 382
  lines and reads them back to the same node, byte for byte, and does the same for each of the 70
  files of nodes here in 0.2 s. Its rows and their matcher were written in it. Text it cannot read
  is a fault that names what and where.
- **A program has memory it can change.** A fail, the end of an `all` and of an `outcome` put back
  what a cell held; a note is kept. With its words in cells the lens's round trip takes 0.20 s
  where it took 0.37 s.
- **What were C commands are programs.** The conformance check, freeze and thaw are written in
  form; the form freeze wrote the same bytes as the C freeze it replaced.
- **The region describes itself.** The layout of its records is a node. The C header is made
  from it and from the lexicon, and the stored header equals what they generate. A program in
  form shows its own region through it, or another process's, with every binding in words.
- **The host is reached by name.** With no choice open, a program in form wrote a file through
  the host's own functions; while a choice is open the call is refused, since it cannot be undone.
- **A crash inside code is a fault.** Machine code that reads a bad address or runs off its
  stack, or a host function that faults, gives the program a fault it can catch, and the run
  goes on.
- **Being seen costs little.** Keeping the walker's registers and each record's writer in the
  region costs about a quarter of its speed; recording every millisecond costs up to about 4%.
- **The graph holds.** `./kg check`: no errors over 1,176 claims, none of them reviewed by a second
  reader.

## Not there

- **Everything swappable** (path: graph node 7.21). The store's own making of nodes, the host
  functions, and what carries a run between nodes, the continuation, the choice points and the
  undo, are C with no binding. A binding has no scope of its own and comes only from the running
  program.
- **A compiler worth the name.** The emitter knows numbers, one shape of condition and calls of
  the recipe itself. It compiles a recipe whole or not at all and cannot hand a node back to the
  walker, so it compiles no call of another recipe, no backtracking and nothing that builds a
  node.
- **The home lens.** The first lens's shapes are brackets only: nothing infix, no names for a
  recipe's arguments, no way to name a part and use it twice. Most stored programs were composed
  in numbers before it existed; they show through it and were not rewritten in it.
- **Form everywhere.** Stored here are the bootstrap, whose header is made from nodes, and 70
  files of nodes in the canonical print. Everything else stored is neither: the recorder and the
  graph's checker (659 lines of C under `tools/`), the Makefile's 65 lines, nine pages, the claim
  graph as 51 text files, and the axioms' text. Three of the checks are still lines of the
  Makefile's shell.
- **Objects.** A cell holds a node and has a trail. It has no declared blueprint, no access or
  constructor recipe and no view; composed parents are not built.
- **Threads, and effects inside a search.** One walker runs on one thread. What cannot be undone
  is refused while a choice is open; nothing holds an effect back until the choice is closed.
- **A second reader** of the encoding, and a second hand to review any claim (6.1.30).
- **An answer to what a node's id is.** SHA-256 was chosen without being asked for (6.1.31).
- **Bounds that give way.** The region's areas have fixed capacities: a wrong recursion runs until
  one is full, and a loop written as a recursion keeps an activation for every round until it
  returns.

## Guidance

Held together and kept short (graph node 7.13): only the axioms are fixed; form first, with C as
the bootstrap only; every part replaceable; real flows watched live before checks; fail undoes
everything back to the last choice; several parents are one composed shape; ice, water and gas
are states of any node; all memory is the framebuffer; pages say only what is.
