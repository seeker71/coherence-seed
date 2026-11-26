# Floor

What stands, as observed on **2026-11-26** on one Apple M4 Max running macOS.
[`NORTH_STAR.md`](NORTH_STAR.md) says where this is going.

## What runs

| Piece | Where | State |
| --- | --- | --- |
| Node encoding | [`spec/node-encoding.md`](spec/node-encoding.md) | written, with 146 conformance vectors kept as a node |
| Bootstrap | [`bootstrap/`](bootstrap) | 1,462 lines of C under the 2,000-line budget. A numeric port runs resolved images. The authoring tool also reads the canonical print |
| Framebuffer | [`spec/framebuffer.md`](spec/framebuffer.md) | all of the bootstrap's memory is one shared region, each record with its blueprint and writer |
| Walker | [`spec/core-operations.md`](spec/core-operations.md) | 75 operations: control with backtracking, parts given a place, recipes closed over values, arithmetic and the bits of an integer, a node's parts, cells, binding from inside the run and from outside it, calls held back until a search settles, the step itself, the region's bytes, and any function of the host by name |
| In form | [`form/`](form) | four operations as recipes and as machine code, the whole encoding (reader and printer of the print, canonical bytes, SHA-256 and the id, the reader of bytes), a traced step, the watcher, and the emitter: a module of named definitions that compiles definitions over numbers and nodes, with what they call, to one machine-code leaf |
| Lens | [`spec/lens.md`](spec/lens.md), [`lens/`](lens) | written in form: words beside the numbers, names beside the nodes they name, signs between two, recipes that see the outer names, and rows that turn a surface into the core and back |
| Ice | [`ice/`](ice) | written in form: a node frozen by one process thaws in another |
| Objects | [`spec/objects.md`](spec/objects.md), [`objects/`](objects) | written in form: parents composed into one shape, a reference as a cell and a view, send, super, and a view through an interface a shape fits by structure |
| Engine | [`spec/nums.md`](spec/nums.md), [`nums/`](nums) | written in form: the NUMS catalogue at its numbers, cells made from blueprints, conversions between every numeric layout, templates and genetics, a generic runtime that runs declared functions as written, arithmetic on every float layout as one exact recipe, and a compiler that writes AArch64 for whole functions and what they call, casts included, from rows of instructions |
| Region | [`region/`](region) | written in form: the layout of the region as a node, from which the C header is made, and programs that show a region, ask a running program to evaluate a node, say where it spends its time, and have it compile what it is in most |
| Programs | [`ground/`](ground) | 72 entries in the ground expectation table, exercised by a program in Form |
| Observer | [`tools/fb.c`](tools/fb.c) | in C, from a second process: records a run in motion and replays it |
| Claim graph | [`graph/`](graph) | 1,426 nodes: the ground, the design, what is open, the path, and the evidence they stand on |

## Observed

Each line here is held by a program that runs under `make check`, or names how it was seen.

**The walker**

- **`make check` passes.** It compares the header with its nodes and exercises 72 ground
  programs and 146 encoding vectors. It checks ice across processes, unsigned and hardened
  native execution, recording and querying a live run, a deferred write on the chosen path,
  the budget for bare numeric reads and a resolved image run by id in a separate process.
- **Fail undoes.** A fail, the end of an `all` and of an `outcome` put back cells, bindings and
  the bytes of rooms; a note is kept. A call held back with `after` is made when every choice open at the time is
  closed, and never if a fail returns past it.
- **The engine runs by id.** `seed-run` loads a numeric image and accepts recipe and argument
  ids. The generic engine with its float definition and both compilers packs as 10,609 nodes with zero text leaves.
  In a single process a constructor returned a cell id, a reader returned zero, and a writer
  and reader used that id to store and recover 42. On this machine the AArch64 compiler
  returned 244 bytes for a recipe that gave 42 both before and after compilation. The image,
  requests and checked replies are in `ground/nums-image.ck`, `ground/nums-requests.ck`,
  `ground/nums-native-requests.ck` and `ground/nums-replies.ck`.
- **One word recipe serves signed and unsigned subtraction.** On this host the Form compiler
  attached 252 bytes of code to the resident word instance. The framebuffer showed 18 walker
  steps for the direct call and 5 after compilation, with the same result id at overflow.
  The image also ran that instance through a numeric token and through signed and unsigned
  eight-bit callers. Token input came from the example vocabulary.
- **Unsigned ordering runs by id.** The four unsigned order relations in the generic runtime
  share one NUMS word comparison. On this host its result agreed with unsigned order on
  1,296 operand pairs before and after compilation. The image run also compared two unsigned
  64-bit values and placed 2⁶³ above 2⁶³ − 1.
- **A cell can hold its bytes alone.** `take` makes a room in the region under a blueprint, and
  `load` and `store` read and change 1, 2, 4 or 8 of its bytes in place; the room's head names
  its blueprint and its last writer. Constructed object views share one room. Its fixed-width
  fields survive a failed write with their previous bits (`ground/rooms.ck`, `ground/nums-cells.ck`).
- **Floats are one recipe.** Sum, difference, product, quotient and comparison of every float
  layout are computed exactly from sign, significand and power, and rounded to nearest even by
  the layout's own encoder. A sweep run in the session over zeros, subnormals, the largest
  numbers, infinities and NaNs matched this M4's hardware bit for bit, NaN payloads included,
  in 10,800 binary16, 32 and 64 cases and 6,264 comparisons. bfloat16, E5M2 and both E4M3
  kinds matched the hardware's binary32 result rounded back.
- **A declared function becomes machine code as it runs.** At a function's first call the
  runtime asks the compiler, which writes AArch64 for the whole function from rows of
  instructions: parameters and locals unboxed in registers, integers kept at their width,
  floats in their own registers. Where the runtime would answer nothing, the code declines and
  the call runs on frames. fib to 30 took 3.4 ms with its compiling, Newton's root of 2 and
  a harmonic sum in binary32 gave the frames' own bits (`ground/nums-compiled.ck`); 27,744
  integer cases answered the same both ways in a session sweep.
- **A function and what it calls are one leaf.** A declaration compiles together with every
  declaration it calls, and a call is a branch between them; two declarations that call each
  other through declared entries stay in machine code, and a new declaration of an entry makes
  the leaves compiled before it compile again at their next call. The area under x² by a
  million rectangles, a call of a second declaration each, took 5.9 ms on this M4; a thousand
  took 156 ms on frames, with the same answer (`ground/nums-compiled.ck`). In session sweeps,
  11,520 operator cases through a call and 12,690 casts between nine layouts by all five rules
  answered the same compiled and on frames; the machine code answered 8,354 of the casts
  itself and declined the rest, where the hardware's answer would not be the runtime's.
- **A language can produce the graph.** Go and BML byte literals round-trip through frozen
  nodes, preserving composite blueprints and shared children (`ground/nums-portable.ck`).
- **BML runs on a BMF character cursor.** In `ground/nums-lenses.ck` an alternative retries
  across an emoji and newline. Crossing a cut reports position 2, immediate stop reports 1,
  and a parameterized BMF list reads two words. The thesis expression `10 - 3 - 2` gives 9.
  A `form.bml` method with the same subtraction gives 5. The same ground program takes the
  base branch of two mutually recursive rules and returns both captures from a bounded
  repetition of an empty rule. It also varies one range matcher for a trial and observes the
  original matcher on return. Reading, printing and rereading
  produced identical syntax trees for 124 Angelic BML sources and all 28 previous-kernel
  `bml-*.bml` fixtures. One additional Angelic file with a BML extension contains Java.
  A separate enumerator agreed on the captures, their order and the cursor positions for 92
  small patterns on all 15 strings of length zero through three over `a` and `b`.
- **A model layout carries its meaning.** Fixed-point, affine, codebook and block layouts
  round values into packed bytes and recover them using numeric blueprints
  (`ground/nums-quantized.ck`).
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
  Twenty-nine recipes over numbers, eighty over nodes, thirty-six that test, fail and choose,
  thirty-three that search and twenty-five that search other definitions end the same both
  ways; forty-six faults planted in the emitter and the bootstrap turned that red.
  Definitions that call each other are compiled into one leaf, and a recursion written as a loop
  runs as one: thirty million rounds, each calling a second definition, in 0.4 s.
- **Code works on nodes by asking the bootstrap.** One address in the region does any operation
  on values in hand, under the contract by which code itself is called, so emitted code reads
  and builds nodes and the store stays the bootstrap's. A node it names is named by its id, not
  by where one run keeps it. Counting the lens's own definition as a
  tree, 5,711 nodes, three hundred times took 0.67 s on the walker and 0.04 s compiled; a chain
  of a million pairs was built and summed in 0.13 s against 0.53 s.
- **A search inside a definition runs as loops.** An `all` or a `first` over ranges, choices,
  tests and operations on their answers is compiled: the primes below 3,000 by trial division
  and the pairs below 400 were counted in 79 ms on the walker and 2 ms compiled. A definition
  that answers more than once hands each answer to the search that called it, so a search
  crosses definitions: one that calls itself handed up all 5,711 nodes of the lens's definition.
- **The emitter compiles part of the system, itself included, and is faster for it.** Of the 377
  definitions the repository's modules bind it compiles 207, each the same alone and after all
  the others. Having compiled 95 of its own 149, it compiles the other 228 again on that code:
  every leaf byte for byte what it wrote walked, in 0.5 s where walked took 2.1 s. The reader of
  the print written in form read the lens's 19,780 bytes in 20 ms on the walker and in 7 ms
  after.
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

- **The lens reads its own definition**: 984 lines, shown and read back to the file it is, byte
  for byte, as is each of the 148 files of nodes here. Text it cannot read is a fault
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

- **The graph holds.** `./kg check`: no errors over 1,243 claims, none reviewed by a second reader.

## Not there

- **The rest of the NUMS engine.** The catalogue, constructed cells, conversion between scalar
  layouts, typed method dispatch and specialization, a genetic population and two expression
  lenses run, and so does a generic runtime for declared functions: frames in rooms, control as
  written, integer arithmetic of every width, arithmetic on every float layout, and machine
  code for whole functions. Whole-package BML lowering, the full Go grammar, GPU code and
  the recipes for switch, foreach, try, defer and go are open. Members and objects run neither on frames nor in compiled
  code, and the compiler takes no more than six parameters at a call from outside and eight
  between declarations. A nothing inside a declared function is not carried out of it on
  frames: returned, a division by zero answers 1; written to a place, it faults `arity`.
  Compiled code declines there, so both ways end alike. The ground programs and supported
  layouts are listed in [the engine specification](spec/nums.md).
- **A neutral store.** What is stored should be meaning in the numeric space with every word
  beside it. One step is taken: the 74 operations describe themselves as nodes, what each
  takes, gives and does to the world (`form/operations.ck`), and the emitter reads the kind of
  a value, which places are numbers and what to ask of the bootstrap from them; arithmetic's
  instructions are rows of data. The rest is not there. The nodes read a part by a bare number
  475 times, the budget `make check` holds: the shape of a part is said only in the name
  beside the node, where the lens shows `mine.ints`, and not in the node. A definition is
  still an entry numbered by hand in one lexicon, not its content. The files store the
  lowered core, so an `if` is four nested nodes. The emitter's other cases, the lens and the
  reader are still programs and not rows.

- **Everything swappable** (path: graph node 7.21). The store's own making of nodes, the host
  functions, and what carries a run between nodes, the continuation, the choice points and the
  undo, are C with no binding.
- **A compiler worth the name.** The emitter compiles a definition whole or not at all: numbers,
  nodes, tests, lets, searches, notes, and calls among definitions compiled together. It
  compiles no mark but a note, no call of a recipe held as a value, no search as the condition
  of an `if`, and nothing nested deeper than seven registers; a definition that answers more
  than once is compiled only where another searches it. 170 of the system's 377 definitions
  wait on those. A third of the emitter itself is among them, the part that searches. Code for a recipe that
  is no entry's does not see what the program binds after it was made. Each operation on a node is a call into
  the bootstrap and not instructions.
- **The home lens.** Every form of the first lens is in brackets: a sign stands between two and
  no more. The BML lens has character patterns and precedence. The home lens still uses brackets.
- **Form everywhere.** Stored here are the bootstrap, whose header is made from nodes, and 148
  files of nodes in the canonical print. Everything else stored is neither: the recorder and the
  graph's checker (662 lines of C under `tools/`), the Makefile's 98 lines, eleven pages, the claim
  graph as 55 text files, and the axioms' text. Five of the checks are still lines of the
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
