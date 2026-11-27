# Floor

What stands, as observed on **2026-11-27** on one Apple M4 Max running macOS.
[`NORTH_STAR.md`](NORTH_STAR.md) says where this is going.

## What runs

| Piece | Where | State |
| --- | --- | --- |
| Node encoding | [`spec/node-encoding.md`](spec/node-encoding.md) | written, with 146 conformance vectors kept as a node |
| Bootstrap | [`bootstrap/`](bootstrap) | 1,604 lines of C under the 2,000-line budget. A numeric port runs resolved images. The authoring tool also reads the canonical print |
| Framebuffer | [`spec/framebuffer.md`](spec/framebuffer.md) | all of the bootstrap's memory is one shared region sized by the machine's memory, each record with its blueprint and writer |
| Walker | [`spec/core-operations.md`](spec/core-operations.md) | 75 operations: control with backtracking, parts given a place, recipes closed over values, arithmetic and the bits of an integer, a node's parts, cells, binding from inside the run and from outside it, calls held back until a search settles, the step itself, the region's bytes, and any function of the host by name |
| In form | [`form/`](form) | four operations as recipes and as machine code, the whole encoding (reader and printer of the print, canonical bytes, SHA-256 and the id, the reader of bytes), a traced step, the watcher, and the emitter: a module of named definitions that compiles definitions over numbers and nodes, with what they call, to one machine-code leaf |
| Lens | [`spec/lens.md`](spec/lens.md), [`lens/`](lens) | written in form: words beside the numbers, names beside the nodes they name, signs between two, recipes that see the outer names, and rows that turn a surface into the core and back |
| Ice | [`ice/`](ice) | written in form: a node frozen by one process thaws in another |
| Objects | [`spec/objects.md`](spec/objects.md), [`objects/`](objects) | written in form: parents composed into one shape, a reference as a cell and a view, send, super, and a view through an interface a shape fits by structure |
| Engine | [`spec/nums.md`](spec/nums.md), [`nums/`](nums) | written in form: the NUMS catalogue at its numbers, cells made from blueprints, conversions between every numeric layout, templates and genetics, a generic runtime that runs declared functions as written, arithmetic on every float layout as one exact recipe, and a compiler written as components that writes AArch64 for whole functions and what they call, casts included, through an encoder written as components over rows of instructions |
| Writer | [`spec/nums.md`](spec/nums.md), [`nums/write.ck`](nums/write.ck) | a component package that writes any component package as Python, C#, TypeScript, Java or BML from rows per language, with names kept beside the nodes and methods called on their receivers |
| Region | [`region/`](region) | written in form: the layout of the region as a node, from which the C header is made, and programs that show a region, ask a running program to evaluate a node, say where it spends its time, and have it compile what it is in most |
| Programs | [`ground/`](ground) | The ground expectation table is exercised by a program in Form under `make check-full` |
| Observer | [`tools/fb.c`](tools/fb.c) | in C, from a second process: records a run in motion and replays it |
| Claim graph | [`graph/`](graph) | 1,444 nodes: the ground, the design, what is open, the path, and the evidence they stand on |

## Observed

Each line here is held by a program in `make check-full`, or names how it was seen.

**The walker**

- **`make check` runs three flows.** A BML expression passes through the source lens and the
  runtime. A standalone process receives a packed image and answers requests by id. A live
  recursion accepts an addition counter from another process and finishes with the expected answer.
  `make check-full` also runs all 77 ground programs separately and together, compares the
  generated header and checks encoding, freeze and thaw, hardened native execution, framebuffer
  replay, deferred writes and the budget for bare numeric reads.
- **Fail undoes.** A fail, the end of an `all` and of an `outcome` put back cells, bindings and
  the bytes of rooms, and give back the nodes and rooms made since; a note is kept, with what it
  holds. A call held back with `after` is made when every choice open at the time is
  closed, and never if a fail returns past it.
- **A fail gives back what it returns past.** As a backtracking machine gives back its heap,
  every node made since a choice point goes when a fail returns to it, except what no fail
  undoes and what that holds, which moves down in the order it was made, its id with it. The
  sweep of compiled code against frames, of 37,822 cases and 3,742 encodings, ends holding
  57,464 nodes, and runs with the other ground programs in one process. `ground/gives-back.ck` makes more than four
  million nodes in forty thousand outcomes and ends with a note from a failed branch, an all's
  answers, an outcome's answer and a kept node's own id standing. Giving back costs 2 to 4
  percent of the time.
- **The engine runs by id.** `seed-run` loads a numeric image and accepts recipe and argument
  ids. The generic engine with its float definition and both compilers packs as 14,523 nodes with zero text leaves.
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
  runtime asks the compiler, which writes AArch64 for the whole function. What it knows is
  rows: the instructions and their fields, the target's registers, each family's templates,
  the relations a family admits, the cast rules. Parameters and locals stay unboxed in
  registers, integers at their width, floats in their own registers. Where the runtime would
  answer nothing, the code declines and the call runs on frames; a construct the compiler does
  not take, or a fault while compiling, leaves the function on frames with a note of it. fib
  to 30 took 3.6 ms with its compiling, and Newton's root of 2 and a harmonic sum in binary32
  gave the frames' own bits (`ground/nums-compiled.ck`).
- **A function and what it calls are one leaf.** A declaration compiles together with every
  declaration it calls, and a call is a branch between them; two declarations that call each
  other through declared entries stay in machine code, and a new declaration of an entry makes
  the leaves compiled before it compile again at their next call. The area under x² by a
  million rectangles, a call of a second declaration each, took about 1 ms on this M4 beyond
  loading and compiling; a thousand took 159 ms on frames, with the same answer.
- **Compiled code answers as frames do.** `ground/nums-compiled-agrees.ck` runs 37,822 cases
  as machine code and on frames all the way down: every class, int and Bool among them, as
  result and as parameter over six shapes; each kind of value passed to each class; literal
  operands of every size, NaN, the infinities and −0; every operator on eight layouts through
  a call; every cast between nine layouts by all five rules; and thirteen edges at the
  boundaries of a call. None differ. 30,269 compiled; the machine code answered 27,108 itself
  and declined the rest, where the hardware's answer would not be the runtime's. What a slot
  holds for a value is one recipe on both sides.
- **The encoder is components, held to the one it replaced.** `nums/arm.ck` is a package of
  fields, formats, encodings, conditions and target facts, with the shared fields and formats
  named. The same sweep keeps the rows and recipe it replaced as its reference and runs every
  instruction row with each field at its edges, one past them, every condition and four
  operands that are no field's number: 3,742 encodings, 2,436 the same word, 1,145 refused by
  both and 161 faulting in both on an operand beyond 64 bits, none differing; inversions,
  pieces, bytes and the target's facts agree too. Twelve faults planted in the package, from a
  base word to a byte order, each made cases differ. A word is kept under its encoding and its
  instruction: the sweep asks for 44,877 encodings of 343 instructions. The sweep as it stood
  before the change takes 919.8 million walker steps on the rows and recipe, 900.2 million on
  the package and 976.4 million on the package keeping no words; under `/usr/bin/time -l`, five
  interleaved runs of each took 8.13, 8.00 and 8.42 s of CPU.
- **The compiler is components, held to the one it replaced.** `nums/compile.ck` is a package of
  a compilation's records, a lowering for each kind of node, families of values, an operation
  for each operator, crossings between families and the forms a callee takes, over rows that
  keep themselves where it reads them. The compiler it replaced ran in a second process over
  every function the agreement sweep meets: of 1,465, both compiled the same 924 to the same
  code byte for byte and both declined the other 541; so they did for a quoted callee, two
  declared entries that call each other and a construct no rule takes, with the same note.
  Four rounds of compiling those functions take 284.3 million walker steps where they took
  314.6 million, since each template is specialized to the target when it is kept, an
  instruction with no role left is kept as its word, and the search runtime checks a record's
  fields in the recipe that builds it, not in a second one. The sweep takes 934.0 million steps
  where it took 940.6 million; under `/usr/bin/time -l`, five interleaved runs of each took 8.29
  and 8.34 s of CPU on average.
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
  In `make check-full`, both BMF evaluators match an independent enumeration of spellings and
  capture trees at every scalar boundary of the sample inputs. Reversing the order of
  alternatives in an isolated matcher made the sweep report the two answers in reverse order.
  In `ground/bml-captures.ck`, a BML expression grammar captures the operators in
  `12*3/4` under one numeric label and the digits of each term under an anonymous label.
  Renaming the tags produces identical linked rules.
- **A package reads in five languages.** `nums/write.ck` is written as components and writes
  itself, from the same nodes, as Python, C#, TypeScript, Java and BML: generators, LINQ and
  `function*` for searches in the first three, `Stream` with `flatMap` in Java, Form's own
  `choice` and `fail` in BML. `ground/lens-round-trip.ck` writes all five and reads the BML back
  into a syntax tree with the BML reader. Python's parser takes the Python and ruff finds no
  undefined name in it. Against loose stubs of the runtime, the hosts' own checkers still refuse
  parts of the typed renderings: TypeScript's strict checker 77 errors, javac 131 and Roslyn 343
  (29 distinct messages). Most are a value typed loosely in the package (any, a list of any) used
  where a host wants a narrower type; in C# about half are an entry compared with `is`, which
  takes only a constant. A member read is cast to the shape it reads, and a value of unknown
  type to the type its place declares. Numbers the package names are written by name, and long
  argument lists are broken at the language's width. The old
  kernel's `choice { a; b }` takes the first option that is not nothing, not every answer, so the
  BML rows do not yet keep a search's meaning where a choice is a search. The encoder's package,
  `nums/arm.ck`, and the compiler's, `nums/compile.ck`, are written by the same writer in all
  five; the compiler's Python parses and its BML reads back. The writer writes no call into
  another package, so the compiler is handed the encoder's functions and the runtime's as recipes.
- **Five views of the identifier alphabet run.** Java, C#, TypeScript and Python executed
  the projected predicates. The BML view parsed with Seed's grammar and its declarations
  ran after reading the view. All five agreed with Seed on 266 inputs, including the signed
  64-bit endpoints. A standalone image with zero text leaves answered five calls to those
  predicates by id. The BML blueprint reader reconstructed the range fields and constructor
  from parsed syntax. See [the component projection](spec/nums.md#viewing-a-component-declaration).
- **BML methods run on their receiver.** The class linker resolves field reads, typed
  arguments and sequential local bindings into NUMS methods. The portable ground extends
  the BMF range with a BML predicate, compares it with the existing range predicate on all
  128 ASCII values, and parses with the extended range in both search evaluators. The
  general BML writer and reader recover the same class declaration, including qualified
  receiver fields, method-local bindings and nested applied types.
- **A grammar can supply a new parse item.** `bmf/items.ck` declares the item interface,
  immutable fields, constructor templates and typed search methods. `ground/bmf-items.ck`
  defines another item and a capture action in a separate package. Direct execution and
  specialization give the same ordered answers and restore the action's write on a failed
  branch. The numeric port runs a grammar with a bounded repetition by id.
  On arm64 it compiles the cursor's UTF-8 width recipe and calls it again through the same id.
- **A model layout carries its meaning.** Fixed-point, affine, codebook and block layouts
  round values into packed bytes and recover them using numeric blueprints
  (`ground/nums-quantized.ck`).
- **A loop runs in the room of one round.** A call or a `let` in the last position gives back the
  activations nothing can return to: three million rounds ended with 5 in use.
- **A recipe inside another sees the outer names**, by being closed over them: over two arguments
  and a part a `let` named, over nine at once, over fifty, through its own recursion, as a
  handler. A call passes as many values as memory holds: forty in `ground/closes-over.ck`.
- **The host is reached by name**, and a crash inside code or a host function is a fault the
  program can catch, a recursion in machine code that falls off the stack among them. So is a
  request no area of the region can hold: a room larger than memory, asked for a thousand calls
  deep, is caught twice, the frames come back each time, and the program goes on.

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
- **Only memory sizes the region, and the host what it takes.** Each area has room for as many
  records as the machine's memory holds, or as its count of 32 bits numbers where that is fewer:
  with 128 GiB the region is 2.3 TiB of addresses, reserved in a few microseconds, and an area's
  entry keeps the most it has held. As the areas grow, the host is asked what it can spare, and a
  growth it cannot spare is `full`: a room of all the machine's memory less a page is refused
  untouched (`ground/full-is-a-fault.ck`). A region without a name goes with its process: a run
  killed by a signal gave back its 2.9 GB at once, where before its region kept them until the
  machine restarted. Against the same bootstrap with fixed capacities, under `/usr/bin/time -l`, the long
  recursion took 0.99 s of CPU where it took 0.97, the sweep of compiled code 8.29 s where it took
  8.15, and every ground program in one process 57.5 s where it took 56.7: the 64-bit hash and
  counts. The sweep held 6.6 MB at most where it held 7.3, a node's record being 32 bytes where it
  was 52; the page tables of the wider range of addresses add a quarter of a megabyte.

**Writing**

- **The lens reads its own definition**: 984 lines, shown and read back to the file it is, byte
  for byte, as is each of the 152 files of nodes here. Text it cannot read is a fault
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
helper could not see the code (found by planting faults that nothing noticed); on frames a
return whose value was nothing answered yes, and a call's arguments lost what they changed,
because a list built with `all` drops a nothing and undoes what was done inside it (found by
running compiled code against frames all the way down).

- **The graph holds.** `./kg check`: no errors over 1,261 claims, none reviewed by a second reader.

## Not there

- **The rest of the NUMS engine.** The catalogue, constructed cells, conversion between scalar
  layouts, typed method dispatch and specialization, a genetic population and two expression
  lenses run, and so does a generic runtime for declared functions: frames in rooms, control as
  written, integer arithmetic of every width, arithmetic on every float layout, and machine
  code for whole functions. Whole-package BML lowering, the full Go grammar, GPU code and
  the recipes for switch, foreach, try, defer and go are open. Members and objects run neither on frames nor in compiled
  code, and the compiler takes no more than six parameters at a call from outside and eight
  between declarations. A function the compiler refuses keeps a note of the construct no rule
  takes; refused for another reason, it notes only that it was refused. The ground programs and supported
  layouts are listed in [the engine specification](spec/nums.md).
  Component code is slow where it reads a part: a member is a checked access and a method call
  looks up the receiver's key, so an encoding the package has not kept costs about 1,660 walker
  steps where the rows and recipe took 585. A kept word is keyed by its encoding and its
  instruction and not by the conditions' rows, so a condition kept again later is not seen by a
  word kept before.
- **A store bounded by what is live.** A run with no choice open keeps every node it makes until
  a fail returns past it, so a loop that counts long enough spends the machine's memory. A handle
  kept as a number across a fail back past where its node was made can name another node
  afterwards.
- **Expectations of meaning only.** Some ground programs still answer values that move without a
  change of meaning: sizes of compiled code, counts of definitions and of a lens's nodes, labels
  of what the emitter compiles, step thresholds and two comparisons of clock time.
- **A neutral store.** What is stored should be meaning in the numeric space with every word
  beside it. One step is taken: the 74 operations describe themselves as nodes, what each
  takes, gives and does to the world (`form/operations.ck`), and the emitter reads the kind of
  a value, which places are numbers and what to ask of the bootstrap from them; arithmetic's
  instructions are rows of data. The rest is not there. The nodes read a part by a bare number
  475 times, the budget `make check-full` holds: the shape of a part is said only in the name
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
- **Form everywhere.** Stored here are the bootstrap, whose header is made from nodes, and 152
  files of nodes in the canonical print. Everything else stored is neither: the recorder and the
  graph's checker (663 lines of C under `tools/`), the Makefile, eleven pages, the claim
  graph as 55 text files, and the axioms' text. The Makefile still drives cross-process checks.
- **More of objects.** Attributes are not built, and the activation does not carry the reference: a
  method is handed it.
- **Threads.** One walker runs on one thread.
- **A second reader** of the encoding, and a second hand to review any claim (6.1.30). The
  encoding in form was written by the hand that wrote the page; nodes are still made by the
  bootstrap's store, and the id in form is too slow to be the one in daily use.
- **An answer to what a node's id is.** SHA-256 was chosen without being asked for (6.1.31).
- **A bound on the machine stack.** The areas together take only what the host can spare, but the
  machine stack is not counted with them, so a recursion in emitted code can still take what the
  host cannot spare before emitted code's check of the floor. A
  handle and the indexes of the walker's records are 32 bits, so at most 2^32 − 1 nodes and as
  many frames or children; the limit word holds 2^23 − 1 bindings and 127 from outside; born code
  is reached by a 32-bit offset, 4 GiB. The stack holds as much as memory, so no program reaches
  emitted code's check of the floor. `region/asking.ck` writes a request's count as four bytes
  of the eight the layout gives it, which holds for a request below 4 GiB, since the walker keeps
  the high four at zero.

## Guidance

Held together and kept short (graph node 7.13): only the axioms are fixed; form first, with C as
the bootstrap only; every part replaceable; real flows watched live before checks; fail undoes
everything back to the last choice; several parents are one composed shape; ice, water and gas
are states of any node; all memory is the framebuffer; pages say only what is.
