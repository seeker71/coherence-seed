# Floor

What stands, as observed on **2026-11-26** on one Apple M4 Max running macOS.
[`NORTH_STAR.md`](NORTH_STAR.md) says where this is going.

## What runs

| Piece | Where | State |
| --- | --- | --- |
| Node encoding | [`spec/node-encoding.md`](spec/node-encoding.md) | written, with 142 conformance vectors kept as a node |
| Bootstrap | [`bootstrap/`](bootstrap) | 1,215 lines of C under a 2,000-line budget, with two commands: run a program, and run it and write its answer as text |
| Framebuffer | [`spec/framebuffer.md`](spec/framebuffer.md) | all of the bootstrap's memory is one shared region, each record with its blueprint and writer |
| Walker | [`spec/core-operations.md`](spec/core-operations.md) | 66 operations: control with backtracking, parts given a place, recipes closed over values, arithmetic and the bits of an integer, a node's parts, cells, binding from inside the run and from outside it, calls held back until a search settles, the step itself, the region's bytes, and any function of the host by name |
| In form | [`form/`](form) | four operations as recipes and as machine code, the whole encoding (reader and printer of the print, canonical bytes, SHA-256 and the id, the reader of bytes), a traced step, the watcher, and the emitter: a module of named definitions that compiles definitions over numbers and nodes, with what they call, to one machine-code leaf |
| Lens | [`spec/lens.md`](spec/lens.md), [`lens/`](lens) | written in form: words beside the numbers, names beside the nodes they name, signs between two, recipes that see the outer names, and rows that turn a surface into the core and back |
| Ice | [`ice/`](ice) | written in form: a node frozen by one process thaws in another |
| Objects | [`spec/objects.md`](spec/objects.md), [`objects/`](objects) | written in form: parents composed into one shape, a reference as a cell and a view, send, super, and a view through an interface a shape fits by structure |
| Region | [`region/`](region) | written in form: the layout of the region as a node, from which the C header is made, and programs that show a region, ask a running program to evaluate a node, say where it spends its time, and have it compile what it is in most |
| Programs | [`ground/`](ground) | 53 that witness the above; one more runs them and judges how each ends |
| Observer | [`tools/fb.c`](tools/fb.c) | in C, from a second process: records a run in motion and replays it |
| Claim graph | [`graph/`](graph) | 1,384 nodes: the ground, the design, what is open, the path, and the evidence they stand on |

## Observed

Each line here is held by a program that runs under `make check`, or names how it was seen.

**The walker**

- **`make check` passes**, seven lines: the header against its nodes; 53 ground programs and the
  142 vectors, judged by a program in form; ice across two processes; native code unsigned and
  hardened; a run recorded from a second process; a run asked from one; and a search that writes
  a file only for the path that held.
- **Fail undoes.** A fail, the end of an `all` and of an `outcome` put back cells and bindings; a
  note is kept. A call held back with `after` is made when every choice open at the time is
  closed, and never if a fail returns past it.
- **A loop runs in the room of one round.** A call or a `let` in the last position gives back the
  activations nothing can return to: three million rounds ended with 5 in use.
- **A recipe inside another sees the outer names**, by being closed over them: over two arguments
  and a part a `let` named, over nine at once, through its own recursion, as a handler.
- **The host is reached by name**, and a crash inside code or a host function is a fault the
  program can catch. So is an area of the region that is full: a recursion that never returns
  is caught, twice, and the program goes on.

**Replacing what runs**

- **While it runs.** An operation, a recipe or the step is given another implementation and has
  it taken away; a replacement runs on what it replaced. The long recursion takes 0.8 s on the
  bootstrap's operations, 1.0 s on four machine-code leaves and 9.9 s on four written in form.
- **From outside the run.** A binding made outside is seen at every layer and is not undone by
  a fail. A second process asks a running program to evaluate a node and gets back how it
  ended: it put a counter on every addition, read it twice as it rose and took it off.
- **The encoding.** All of it is written in form from its page: the reader and printer of the
  print, the canonical bytes, SHA-256 and the id, and the reader of bytes. Bound in place of the
  bootstrap's own they hold the 142 vectors, ids among them; on the reader and printer every
  ground program runs.
- **Emitted code is the walker or faster, never different.** It answers a number or a node, or declines. For
  an entry it is a faster form of the entry's own binding, used wherever the recipe would run,
  and it steps aside while something from outside watches an operation it replaced.
  Twenty-nine recipes over numbers and sixty over nodes end the same both ways; seventeen
  faults planted in the emitter turned that red.
  Definitions that call each other are compiled into one leaf, and a recursion written as a loop
  runs as one: thirty million rounds, each calling a second definition, in 0.4 s.
- **Code works on nodes by asking the bootstrap.** One address in the region does any operation
  on values in hand, under the contract by which code itself is called, so emitted code reads
  and builds nodes and the store stays the bootstrap's. A node it names is named by its id, not
  by where one run keeps it. Counting the lens's own definition as a
  tree, 5,711 nodes, three hundred times took 0.67 s on the walker and 0.04 s compiled; a chain
  of a million pairs was built and summed in 0.13 s against 0.53 s.
- **A run makes itself faster, or is made faster.** A watcher in form compiles a recipe at its
  hundredth call. From a second process, a program in form looked at a recursion that takes
  14.3 s, found the recipe it was in, and asked the run to compile it: it ended at 1.3 s.

**Seeing**

- **The region describes itself.** The layout of its records is a node; the C header is made from
  it. Programs in form show their own region or another's, with bindings in words and names.
- **The framebuffer is a profiler.** Looked at once a millisecond, a run says which recipes it is
  in. That found the walker scanning every enclosing `all`'s answers at the end of each small
  one; gathering only its own took the lens's showing of itself from 0.28 s to 0.09 s.
- **Being seen costs little.** Registers and writers kept in the region cost about a quarter of
  the walker's speed; being recorded every millisecond, up to about 4%.

**Writing**

- **The lens reads its own definition**: 853 lines, shown and read back to the file it is, byte
  for byte, as is each of the 93 files of nodes here, in 0.8 s. Text it cannot read is a fault
  that names what and where.
- **Names lie beside the nodes.** Arguments are numbers in a node and names on a second line of
  its file. Every module and ground program was named; naming changed no node. A sign stands
  between two: `(n < 2)`.
- **Objects.** Two parents that share an ancestor are one shape; super moves the view; a slot
  outside the view is refused; a cell is seen through an interface it fits by structure.

**Found by running real flows**, each older than the flow that found it: a lazy bound while an
operation's arguments were being evaluated was handed their answers (found by asking a running
program); `step` evaluated its node in the activation of whatever computed it (found by running
everything on the reader in form); the emitter trusted a recipe's own name where it did not mean
the recipe (found reading it back); a witness compared the walker with the walker because its
helper could not see the code (found by planting faults that nothing noticed).

- **The graph holds.** `./kg check`: no errors over 1,202 claims, none reviewed by a second reader.

## Not there

- **Everything swappable** (path: graph node 7.21). The store's own making of nodes, the host
  functions, and what carries a run between nodes, the continuation, the choice points and the
  undo, are C with no binding.
- **A compiler worth the name.** The emitter compiles what answers once: numbers, the parts of a
  node and the building of one, tests, lets, and calls among definitions compiled together. It
  compiles a definition whole or not at all, and nothing that searches (`choice`, `fail`, an
  `all` over a search) and nothing that leaves a mark. The lens, the region, the objects and
  the emitter itself are written with both, so none of them is compiled. Code for a recipe that
  is no entry's does not see what the program binds after it was made. Each operation on a node is a call into
  the bootstrap and not instructions.
- **The home lens.** Every form of the first lens is in brackets: a sign stands between two and
  no more, with no precedence, and there is no pattern over text.
- **Form everywhere.** Stored here are the bootstrap, whose header is made from nodes, and 93
  files of nodes in the canonical print. Everything else stored is neither: the recorder and the
  graph's checker (662 lines of C under `tools/`), the Makefile's 81 lines, ten pages, the claim
  graph as 54 text files, and the axioms' text. Five of the checks are still lines of the
  Makefile's shell.
- **More of objects.** Attributes are not built, and the activation does not carry the reference: a
  method is handed it.
- **Threads.** One walker runs on one thread.
- **A second reader** of the encoding, and a second hand to review any claim (6.1.30). The
  encoding in form was written by the hand that wrote the page; nodes are still made by the
  bootstrap's store, and the id in form is too slow to be the one in daily use.
- **An answer to what a node's id is.** SHA-256 was chosen without being asked for (6.1.31).
- **Bounds that give way.** The region's areas have fixed capacities. A program that fills one
  has a fault it can catch, and gets its frames back; no node a run made is given back, so a
  loop that counts to ten million fills the store, and that stays full.

## Guidance

Held together and kept short (graph node 7.13): only the axioms are fixed; form first, with C as
the bootstrap only; every part replaceable; real flows watched live before checks; fail undoes
everything back to the last choice; several parents are one composed shape; ice, water and gas
are states of any node; all memory is the framebuffer; pages say only what is.
