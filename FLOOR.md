# Floor

Observations through **2026-11-28** cover an Apple M4 Max running macOS on AArch64, earlier Rosetta runs,
and a separate x86-64 Linux guest. The current seed carries AArch64 machine code and refuses an x86-64 process at startup.
[`NORTH_STAR.md`](NORTH_STAR.md) says where this is going.

## What runs

| Piece | Where | State |
| --- | --- | --- |
| Node encoding | [`spec/node-encoding.md`](spec/node-encoding.md) | written, with 154 conformance vectors kept as a node; every node file is stored as its records, which the bootstrap reads by making each through the store, and is shown and reviewed through its print and the lens; the print is read and written in form only |
| Bootstrap | [`bootstrap/`](bootstrap) | The C cold start is counted at each build, 2,172 lines, and shrinks as the seed carries its parts. A numeric port runs resolved images. It reads stored records only; every process first holds the leaves of the seed `ice/seed.ck` in their cells and prints the answer line of `ck run` by `form/print.ck`, both read from beside the executable |
| Framebuffer | [`spec/framebuffer.md`](spec/framebuffer.md) | all of the bootstrap's memory is one shared region with a budget based on available host memory, each record with its blueprint and writer; its node table is its lane of a substrate: inside it, or with `CK_SHARE` a named one that every member maps at one offset |
| Walker | [`spec/core-operations.md`](spec/core-operations.md) | Control with backtracking, parts given a place, recipes closed over values, arithmetic and the bits of an integer, a node's parts, cells, binding from inside the run and from outside it, calls held back until a search settles, the step itself, the region's bytes, and any function of the host by name |
| In form | [`form/`](form) | four operations as recipes and as machine code, the whole encoding (reader and printer of the print, canonical bytes, SHA-256 and the id, the reader of bytes), a traced step, the watcher, and the emitter: a module of named definitions that compiles definitions over numbers and nodes, with what they call, to one machine-code leaf |
| Lens | [`spec/lens.md`](spec/lens.md), [`lens/`](lens) | written in form: words beside the numbers, names beside the nodes they name, signs between two, recipes that see the outer names, and rows that turn a surface into the core and back |
| Ice | [`ice/`](ice) | written in form: a node frozen by one process thaws in another, and a third names it by its handle |
| Objects | [`spec/objects.md`](spec/objects.md), [`objects/`](objects) | written in form: parents composed into one shape, a reference as a cell and a view, send, super, and a view through an interface a shape fits by structure |
| Engine | [`spec/nums.md`](spec/nums.md), [`nums/`](nums) | written in form: the NUMS catalogue at its numbers, cells made from blueprints, conversions between every numeric layout, templates and genetics, a generic runtime that runs declared functions as written, arithmetic on every float layout as one exact recipe, and a compiler written as components that writes AArch64 for whole functions and what they call, casts included, through an encoder written as components over rows of instructions |
| Writer | [`spec/nums.md`](spec/nums.md), [`nums/write.ck`](nums/write.ck) | a component package that writes any component package as Python, C#, TypeScript, Java or BML from rows per language, with names kept beside the nodes and methods called on their receivers |
| Region | [`region/`](region) | written in form: the layout of the region as a node, from which the C header is made, programs that show a region, ask a running program to evaluate a node, say where it spends its time, and have it compile what it is in most, `region/far.ck`, which maps another process's region by the host's rows and reads it with the load leaf, and `region/rest.ck`, which says of a start saved at rest what it read and whether that still holds |
| Programs | [`ground/`](ground) | The ground expectation table is exercised by a program in Form under `make check-full` |
| Observer | [`tools/fb.c`](tools/fb.c) | in C, from a second process: records a run in motion and replays it. A named substrate and its lanes are shown by `share/show.ck`, in Form |
| Substrate | [`share/`](share) | written in form, a component package over the host's rows: making or joining a named substrate, claiming a lane, telling whether a member stands, retiring, burying, leaving, and listing one |
| Claim graph | [`graph/`](graph) | the ground, the design, what is open, the path and their evidence |

## Observed

Each line here is held by a program in `make check-full`, or names how it was seen.

**The walker**

- **`make check` runs three flows.** A BML expression passes through the source lens and the
  runtime. A standalone process receives a packed image and answers requests by id. A live
  recursion accepts an addition counter from another process and finishes with the expected answer.
  `make check-full` also runs the applicable ground programs separately and together (77 on AArch64), compares the
  generated header and checks encoding, freeze and thaw, hardened native execution, framebuffer
  replay, deferred writes and the budget for bare numeric reads.
- **Fail undoes.** A fail, the end of an `all` and of an `outcome` put back cells, bindings and
  the bytes of rooms, and give back the nodes and rooms made since; a note is kept, with what it
  holds. A call held back with `after` is made when every choice open at the time is
  closed, and never if a fail returns past it.
- **A return and a fail give back the walker's own.** A fail carries no answer, so what the
  failed path last answered is not kept: a room taken and failed over is given back, where it
  stayed when the take was the last thing answered. A return to a frame gives back the frames
  a commit left above it and the values linked since, where only the newest frame was given
  back: a hundred cuts, a hundred host calls and a hundred maps in one recursion end with the
  5 frames and 2 links they began with, where the maps alone had left 432,965 frames. Both
  trails are empty at every step with no choice open. Seen by reading the areas' counts from
  the region; `ground/metal-holds.ck` holds it for the maps. The deepest case of
  `ground/code-on-nodes.ck` needs the stack `make` gives: run from a shell with an 8 MB stack
  it declines, on main as here.
- **A fail gives back what it returns past.** As a backtracking machine gives back its heap,
  every node made since a choice point goes when a fail returns to it, except what no fail
  undoes and what that holds, which moves down in the order it was made, its id with it. The
  sweep of compiled code against frames, of 37,822 cases and 3,742 encodings, ends holding
  57,464 nodes, and runs with the other ground programs in one process. `ground/gives-back.ck` makes more than four
  million nodes in forty thousand outcomes and ends with a note from a failed branch, an all's
  answers, an outcome's answer and a kept node's own id standing. Giving back costs 2 to 4
  percent of the time.
- **The engine runs by id.** `seed-run` loads a numeric image and accepts recipe and argument
  ids. Its packed image contains zero text leaves. One process constructed a cell and stored
  and read back 42 using its ID. An image packed on Apple silicon compiled a scalar recipe by ID
  and ran a generated NUMS function directly on macOS AArch64. The NUMS compiler writes x86-64
  too, but no process runs there now: `ck` and `seed-run` refuse that processor at start. The same image ran a conditional by ID before and after compilation
  and returned 2 and 42 for its two arms. The x86-64 compiler also handles guarded returns and
  nested integer or Boolean selections. Calls, loops and floating-point bodies use frames on
  this target. [Backend interface and coverage](spec/nums.md#execution-and-specialization)
- **Metal runs by ID.** The image comparison runs the
  65-lane word map directly, through an attached Form recipe under an open choice, and
  through two dependent stages. All three answers agree with NUMS frames. A fault while
  binding an encoder's buffer releases the encoder and permits the next map. Both runtime
  executables link only libSystem.
- **Compiled Metal libraries run through a numeric batch.** Views carry byte offsets.
  Stages choose thread or threadgroup grids, constants, dynamic threadgroup memory and
  concurrent buffer barriers. The old kernel's float and quantized fills and Llama argmax
  reductions ran in one batch on both process architectures. The complete readbacks matched
  CPU references. Thirteen Q8 matrix products and a recurrent delta update also matched
  their CPU results on both ABIs. Page-aligned file mappings supply model weights directly
  to Metal. A 28-layer Llama pass matched the old decoder, and a four-row Qwen projection
  matched the sampled GGUF weights. Image construction resolves Q8_0, Q4_K and Q6_K
  projections from the stored matrix layout. The two-row Q8_0 kernel read a complete
  Qwen projection directly from its file region on both Mac process architectures.
  A resident Llama model can present compact activation panels to the GPU. Q4_K
  and Q6_K projections share packed weights across streams within a dispatch.
  Numeric function constants select Metal pipelines. A resident eight-stream Llama
  image ran llama.cpp’s packed matrix kernels and weighted RMS normalization by ID.
  Packed matrix panels select the upstream tile and can add a residual in the
  same store. On this model all 197 matrix dispatches matched llama.cpp down
  to the GPU instruction bytes. A four-history replay preserved every logit on both
  process architectures. The upstream weighted RMS kernel also matched all 96
  reference values on a 2 × 3 × 4 grid.
  One resident plan now handles changing stream positions and token feedback by ID.
  Its 56 indexed Float16 KV writes and 28 flash-attention/reduction pairs per turn
  match the upstream Metal trace. Eight streams generated eight successive tokens
  each for shared and distinct starts, with all 128 IDs agreeing with llama.cpp.
  [Decode by operation](spec/nums.md#llama-decode-by-operation)
  The resident worker reads the eight token IDs in one copy and returns eight ordered
  results by ID. A second turn can reuse the layout and deliver fresh values. Only
  the selected result ranges cross the port.
  A live worker mounted the Llama model once, kept all 255 weight mappings across turns
  for two distinct KV histories, and released them on retirement while the worker stayed
  alive. Both histories agreed with the previous decoder on both process architectures.
  [Metal batches and resident turns](spec/nums.md#named-metal-batches)
  One resident worker also serves independent local client processes. A new request
  can receive its first token while an earlier request is still generating, and a
  paused client can resume its own context. Eight clients on nine requests matched
  all 106 predictions from llama.cpp at the same batch width on arm64 and Rosetta.
  On an M4 Max, eight local clients decoding eight positions on Llama 3.2 3B Q4_K_M
  reached 647 tokens/s when the worker submitted the first 48 of 468 dispatches early.
  The single-buffer service reached 615 and llama.cpp reached 740 on the same weights
  and inputs. All three returned the same 64 tokens in each of 15 measured warm runs.
  In 40 traced turns the GPU ran the prefix while the CPU encoded the suffix, with a
  median overlap of 0.79 ms.
  [Independent local decode streams](spec/nums.md#independent-local-decode-streams)
  On Apple Silicon, a generation request can run a model-selected Form recipe by
  ID. Two local clients elicited different operators from the same mounted Llama
  model. Each recipe assembled a Metal frame, returned its result by content ID
  and fed a token into its own next prediction. Both predictions matched llama.cpp.
  Ready tools that name the same panel planner share a Metal submission. Eight
  add/argmax calls and their next decode used two command buffers. Their eight
  replies were byte-identical to the nine-buffer scalar run.
  [Recipe tokens and live variants](spec/nums.md#recipe-tokens-and-live-variants)
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
  into a syntax tree with the BML reader. For the cross-object search example, Seed reads
  back the emitted BML interface, implementing class and free function, invokes the concrete
  receiver, and matches six native value and callback traces. The other four emitted languages
  ran the same cases. The encoder and compiler also project in all five
  languages. Reading those larger packages as syntax trees and executing a supported method are
  separate checks. For a linked projection, the writer follows referenced declarations to
  determine whether each call can yield several answers. [Component views](spec/nums.md)
- **Twelve packages read in five languages, and the Java stands apart.** `nums/write.ck`, `arm`,
  `compile`, `part`, `resolve`, `classes`, `bmf/items`, `bml-methods`, `bml-link`, `view-patterns`,
  `bml-alphabet` and `lens/identifier-model` are each written in Python, C#, TypeScript, Java and BML
  without a fault (`ground/checks/packages.mk`, in `make check-held-back`). Each Java file is in its own
  package and class, public, and every record, interface and function has a Javadoc line; a package that
  links another imports its class and does not copy it. The Java of these packages is not yet compiled:
  the only runtime it is run against is the one for `ground/search-package.ck`, its parameters are
  `Object` and `List<?>` (so a receiver of either does not compile in any runtime), and about one line in
  a hundred runs past 120 columns.
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
- **A member read of a known shape is one operation.** A package's `(r.member S k v)` with an
  entry for S and a number for k is lowered to the field `(child v k S)`, and the Form of
  `nums/` and the host reads its records the same way (347 reads in 27 files); the bootstrap
  checks the blueprint and the place in that one step, and the emitter inline. The shape is in
  the node, so the stored form says which record's part is read. `ground/record-fields.ck`
  holds 210 reads, of seven records by five shapes at six places, and a nothing in each of the
  three parts, alike walked, compiled, as a field and through `nums.member-access`, to a
  reference that is the recipe it replaced, and a lowered package read to the same; five faults
  planted by hand each turned it red: the shape not checked, the place one past, a nothing
  answered rather than faulted, the emitter's blueprint check left out, and the lowering
  reading the wrong place. Measured with a region of 256 MiB, instructions retired, on main and
  after: a named read walked 35,200 against 4,300 (a child read 2,850), compiled 480 against
  210 (a child 205); `getpid` through the berth 1.98 M against 985,000 a call, through a door
  cell 54,000 and 56,000; loading `host/all.ck` cold 713 M against 468 M; a member's start
  cold 1,629 M against 1,092 M, warm 229 M against 136 M.
- **A grammar can supply a new parse item.** `bmf/items.ck` declares the item interface,
  immutable fields, constructor templates and typed search methods. `ground/bmf-items.ck`
  defines another item and a capture action in a separate package. Direct execution and
  specialization give the same ordered answers and restore the action's write on a failed
  branch. The numeric port runs a grammar with a bounded repetition by id.
  On arm64 it compiles the cursor's UTF-8 width recipe and calls it again through the same id.
- **A model layout carries its meaning.** Fixed-point, affine, codebook and block layouts
  round values into packed bytes and recover them using numeric blueprints
  (`ground/nums-quantized.ck`).
- **Two processes, one substrate.** With `CK_SHARE=name`, a process that runs after the prelude
  `share/all.ck` is a member of a named substrate that every member has at byte 2^43 of its
  region's addresses. The substrate has 256 lanes of 2^24 handles each, so the handle of a
  settled node names it in every member. A settled record enters the one index by a
  compare-and-swap of its handle, with no lock. A search sees of another lane what was settled at
  the one moment it began, and a commit forwards a working node onto equal content another member
  settled meanwhile and gives its place back.
  Becoming a member is written in Form: `share/share.ck` makes or joins the substrate through
  rows of host functions, claims a lane by one compare-and-swap in a machine-code leaf, knows a
  member as a process id together with the instant the host started it, in one word, asks the
  host for any process's start, retires the rows of members that are gone, buries a substrate
  with no member, unlinking a name only where the object of that name holds its own word, and
  leaves at the run's end, also where the bootstrap ends the process from inside a run. The bootstrap keeps one
  function of it, `adopt`, reached through a door in the head: the mapped substrate becomes the
  process's, every node it had made alone is made again there, and all the run holds is
  forwarded. A member's start, `share/all.ck`, loads the two packages (`share/load.ck`), which is
  saved at rest, and leaves the opening of the door and the joining to run after it, so a start
  with `CK_IMAGES` enters the loaded packages and opens and joins from there.
  `make check-full` holds this with three flows. In the first, all 77 ground programs ran as
  members of one substrate, six at a time, each ending as expected, while one more member, the
  long recursion, was killed with `kill -9` in mid-run and its lane was retired by those that
  joined after; the member that held the substrate open then made every settled node of all 79
  lanes again from its parts, each answered its own handle; and when the last member left, the
  name was gone. Then a second substrate had its burying and one lane written in the name of a
  process that runs but was started at another instant, as a process id the host gave again
  leaves them, and every member of it was killed; the next process to open it took the burying
  over, retired the lane, buried it, made a new one and answered 42. Then a process kept a third
  substrate mapped while its last member left and another made one of the same name, and buried
  through its stale mapping: the live one's name stood, where before it was unlinked. In the second, a member thaws
  a frozen node and answers its handle, and the member that made the substrate prints the same
  node from that number alone. In the third, under `make check`, two numeric ports that are
  members of one substrate answer between them, byte for byte, what one port alone answers, one
  of the answers a node in a lower lane than its part.
  Seen with a debugger and held by no check (`spec/framebuffer.md`, What a search sees): a member
  stopped between its reads of two lanes, while one settled a node and the other a node holding
  it, saw both or neither when it went on; before the tide it answered that one content had two
  handles. Members killed just after a record entered the index, in the middle of a forwarding
  commit, and between writing `buried` and unlinking left nothing the next members or the next
  opener did not finish. A member that outgrew a substrate of 8 MiB had the fault `full` with
  `nodes`, the substrate's charge standing at its allowance and not above it.
  On this Mac a process alone starts as main's does, 13.8 M instructions and 1.50 MB; the maker of a
  substrate starts in 108 M instructions and 6.4 ms where the start in C took 135 M and 14 ms, and
  a joiner in 92 M and 6.8 ms against 133 M and 14 ms; their peak footprints rose, to 12.7 MB
  and 6.5 MB from 5.2 MB and 3.1 MB, with an allowance of 64 MiB (`spec/framebuffer.md`, Observed).
- **A lone process settles.** The node table is lane 0 of a substrate inside the process's own
  region, laid out as version 26 and described in `region/layout.ck`. A node made while no choice
  is open is settled as it is made; one made under a choice is working, found by the process's
  own index, and settled where it stands at the first evaluation step after the last choice
  closes. A second process recording the long recursion saw lane 0 settled below 273 of its 273
  nodes mid-run and below 274 at the end, the answer node 273 among them. A cut stops at a
  running outcome's choice point, so no choice opens again over settled nodes, and a fail that
  would reach one ends the process. `ground/gives-back.ck` holds that a node made before its forty
  thousand outcomes keeps its handle, that the lane's settled watermark stands while they run and
  is past their answer once they close, and that a note on a key the fail moves to a page of cells
  not used before still stands. `ground/region-describes-itself.ck` checks the substrate head, its
  areas, the lane row, where `share` puts the nodes, that `seen` holds the watermark while a
  choice is open, and every node's `settled` byte against it; five faults planted in the
  description, a walker planted to keep a wrong `seen` and one planted to leave the byte unset
  were each reported. A fault planted to move the watermark ended the run at the commit, which
  found content already settled, rather than going on. A `full` planted where the
  commit and a fail grow an index by id left every node found by content and by id. Cells,
  bound-by-node and print-numbers are charged by the pages they use. A fail that has no memory for
  the pages of cells its kept nodes move onto keeps its nodes instead of giving them back, as one
  without text for its forwarding table already did. Measured against the bootstrap without the
  substrate in three alternating rounds under `/usr/bin/time -l`, medians of cycles: the long
  recursion 3.85 G against 4.21 G (−8.6%), `ground/gives-back.ck` as it stood before 5.23 G
  against 5.51 G (−5.1%), and `ground/nums-compiled-agrees.ck` 34.6 G against 35.0 G (−1.2%).
  As first laid out it took +5.7%, +4.1% and +15.5%: the compiler had left `room` a call, and had
  put the commit and the page-bit test into the walker's loop. `room` in the loop is a gain the
  version without the substrate would have too; against that, the substrate costs the sweep 7.3%
  more instructions and 7.6% more cycles, the long recursion +3.5% and −2.0%, the outcomes +4.2%
  and +1.1% (`spec/framebuffer.md`, Observed, says where). The peak footprint is 5.67 MB against
  4.98 MB for the outcomes, since the working index keeps its largest size beside the settled one.
- **A loop runs in the room of one round.** A call or a `let` in the last position gives back the
  activations nothing can return to: three million rounds ended with 5 in use.
- **A recipe inside another sees the outer names**, by being closed over them: over two arguments
  and a part a `let` named, over nine at once, over fifty, through its own recursion, as a
  handler. A call passes as many values as memory holds: forty in `ground/closes-over.ck`.
- **The host is reached through one door, and its functions are rows** (graph node 5.14.60).
  Entry 98 calls a name or an address, and the address of the host's own lookup is a word of the
  region's head. The package `host/` holds as rows: 30 host functions, each with a slot for
  every value and the kind of its result; Objective-C messages, each a selector, its slots
  and its result; the calling convention of this host, which is Apple's
  variant of the arm64 convention (a varying value goes on the stack); which blueprint a slot
  takes and how it becomes a word; how each kind of result is read, a one-byte truth among them; the host's two lookup
  constants; seven instruction encodings; and six small machine-code leaves as instruction
  rows for `nums/arm.ck`. One leaf, the berth, runs any function row. Its layout is one row,
  and the leaf's offsets, the places of the convention and the recipes are all derived from
  it. The berth is one room of the region, taken once when the door is opened, so it lies
  inside the allowance and a call that faults midway strands nothing.
- **Door cells: a row called through a word of the head.** The head's `doors` record holds
  64 cells, one word each, the process's own and saved as 0 at rest. `host.call` takes a cell
  for a row the first time it meets one whose slots all go as words to integer registers,
  at most five, and whose result is a word, an integer, an address, a truth or nothing: the
  address is resolved once and stored in the cell, and the row's own cell (`held` of the row)
  keeps a closure of the cell's number, the leaf for the kind of result, the count of slots
  and whether it leaves nothing. Every later call is that closure: the count and that every
  value is an integer are checked, then the guard for a row that may leave something, then
  one `native` of a door leaf, which reads the cell through its place in the head
  (`ldr x9, [region + 8k + place]; blr x9`; the code holds no address) and narrows or reads
  the result as its kind says. A leaf that finds its cell empty declines, and the call is the
  fault `(list host.door row)`. The five door leaves are instruction rows, assembled the first
  time a row of their kind takes a door. Any other row, a value that is not an integer, or a
  count that differs goes through the berth as before (`host.berth-call`), so it fails where
  the berth fails. Measured against the berth in one run, a loop of 1,000 calls against 2,000
  less the loop: `getpid` about 23,000 instructions a call against 915,000, `close` of -1
  with its guard 49,000 against 1.31 M, `lseek` of three values 85,000 against 2.4 M. The
  parts a row's code reads by name are fields, one operation each (above); what is left of the
  door's cost is the walking of the closure and its checks.
- **The flow.** `make check-native` runs `ground/host-refused.ck` by itself, with `ck` as built
  and with a copy signed for the hardened runtime: a file is written, read back three ways and
  removed; four words, two of them differing only above bit 31, are sorted by the host through
  a callback that is leaf code; the square root of 4 and of 2 and 2 to the tenth come back as
  floats; two varying values are formatted; a page is mapped, written through an absolute
  store, read and unmapped; a raw supervisor call answers the process id, and one that fails
  answers no. Where entry 98 can carry a call, by name, by address, through the berth and
  through a door cell answer the same; `close` of -1 through the cell answers -1. A value of
  the wrong kind for its slot, a composite where an address goes, a call with more values than
  slots, through the berth or a cell, and a function with no row answer nothing. A supervisor
  call and an absolute store inside a search are the fault `irreversible`, as entry 98 is, and
  the store is not made; so is a row that may leave something behind, `close`, prepared or run
  on an address. A row whose column says it leaves nothing, `getpid` and the clock, runs inside
  a search once its address is noted, and the clock read there is later than one read before. Forty calls that crash inside the host leave the host's heap with the
  bytes in use it had. An object made by the message `new` is let go by `release`, and a
  message with no row answers nothing. Of the 30 function rows, 22 are run by the flow and
  four more by the Metal flow of `make check-image`; `mprotect`, `dlopen` and
  `dlsym` are outside that comparison. Metal reads VM counters through `sysctlbyname`. The host and Metal flows exercise the message rows they use. Inside the ground catalogue's search the same
  program is the refusal `irreversible`, at its first call, which is by address. By hand, not
  held by a check: two wrong place rows made the run name the five cases they break; a row
  naming a library that is not there is a fault that names the library; a call before the door
  is opened is a fault that names `host.open`. The flow also empties every cell and calls
  `getpid` again, which is the fault that names its row; a text where an integer goes and a
  call with more values than slots fail over to the next option through a cell as through the
  berth. Five faults planted in the door by hand each made the flow fail: values not checked,
  the count not checked, the column that says a row leaves nothing read inverted, the wrong
  cell read, and an empty cell answering. A door leaf that does not narrow an integer result
  made no case differ: no row the flow calls leaves the high word of an integer result unset.
- **Limits of a row.** A text or bytes value in an address slot is copied into the berth with a
  zero after it and may hold at most 127 bytes; a longer one answers nothing. At most eight
  values go to the integer registers, eight to the float registers and eight to the stack, and
  sixteen in all. Of the results only the first integer word or the first float is read; the
  second result word and the other three floats are stored by the leaf and read by no row. A
  failed supervisor call answers no and does not give the host's error number. The berth is
  one per process, so a callback cannot itself call a row.
- **On the numeric port a call by address ran**, by hand, in eight small images under
  `seed-run`: the address of `getpid`, found through the `symbol` word, answered the process id
  in the region's head, with no values and with eight; address 0 answered nothing, `no-symbol`;
  an address with no code was the fault `crash` and could be recovered from; a call inside an
  `all` was the fault `irreversible`; nine values were the fault `arity`. No check holds this.
- **A crash inside code or a host function is a fault the program can catch** when the
  processor faults: a bad address, a bad instruction, a recursion in machine code that falls
  off the stack. A host function that ends the process itself is not caught: `(host "abort")`
  and `free` of an address the host never gave end the process with exit 134. A request no
  area of the region can hold is caught as well: a room larger than memory, asked for a
  thousand calls deep, is caught twice, the frames come back each time, and the program goes on.

- **A start at rest is saved once and entered by every later start** (`spec/framebuffer.md`, A
  region at rest). When the files run before a program, or the first part of the image `seed-run`
  opens, took nothing from the host or the process, the region's pages are saved under a name of
  the start and its directory, with a node from `region/rest.ck` that checks each file the start
  read by one `stat`; a later start runs that node in a fresh region and, if it holds, maps the
  pages copy-on-write instead of running the start. The builder goes on from its own image, so a
  cold and a warm start meet the program in the same region. `make check` holds that the image
  flow replies the same bytes cold, while saving and entered, and that the image was entered and
  not made again; `make check-full` runs every ground program in one process entered from the
  image of `region/all.ck`, and in `check-ice` two builds of one start are the same bytes and
  each case that must not be entered is not: a start file changed, changed with its size and
  times put back, another executable, another layout version, an allowance smaller than the image
  charges, `CK_SHARE`, an image cut short, a builder's file left mid-save, and a start that read
  the clock, which saves nothing, and so do a start that reads the stack, one that runs code
  that reads the processor's counter, one that catches `full` at an allowance of 40 MiB, whose
  image a start at 60 MiB would have entered and answered as at 40, and one that catches code
  falling off the stack; a start whose last file reads the clock saves and later
  enters the image of the files before it, the same bytes as that part alone saves, and once that
  file is pure the whole start is saved from the part it entered, and is not entered after its
  first file changes. Measured on this M4 Max under `/usr/bin/time -l` (medians of
  five) and from outside (median wall of thirty), cold against warm: the lens flow 117.7 M
  instructions and 6.8 ms against 28.2 M and 2.29 ms, where main took 269.4 M and 12.8 ms cold:
  the reader of the print marks each node it meets, so a node met twice is one look where it was
  a search of all those met before. The image flow of `check-image` cold 946.4 M and 40.0 ms
  against 114.0 M and 5.6 ms warm (measured with the gate in C), and its start with no request
  851.0 M and 35.4 ms against 15.7 M and 1.37 ms. A process that does nothing takes 13.6 M
  instructions where main took 14.7 M, and 13.37 M at 64 MiB where main took 13.59 M.
- **A region is laid out alike on every host.** Each area reserves what `region/layout.ck` says
  and is offset into its page by `fb.hue`, a row, where it was clamped to the allowance and offset
  by the cache line the host reported. So a start's image is the same pages at any allowance. `check-ice`
  holds it, holds that the executable built for x86-64 is refused at start under Rosetta and saves
  nothing, and a clamp or an offset that follows the processor planted in the
  layout is reported. A process alone takes 14.0 M instructions as before; it reserves 49 GB of
  addresses at every allowance.

**Replacing what runs**

- **While it runs.** An operation, a recipe or the step is given another implementation and has
  it taken away; a replacement runs on what it replaced. The long recursion takes 0.8 s on the
  bootstrap's operations, 1.0 s on four machine-code leaves and 9.9 s on four written in form.
- **From outside the run.** A binding made outside is seen at every layer and is not undone by
  a fail. A second process asks a running program to evaluate a node and gets back how it
  ended: it put a counter on every addition, read it twice as it rose and took it off. The
  asker takes `region/all.ck` as its start, so where an image folder is found an ask after the
  first enters its image: about
  240 M instructions an ask against 2,370 M when it stepped the file itself, the same answers.
- **The encoding.** All of it is written in form from its page: the reader and printer of the
  print, the canonical bytes, SHA-256 and the id, and the reader of bytes. Bound in place of the
  bootstrap's own they hold the 142 vectors, ids among them; on the reader and printer every
  ground program runs.
- **The print left C.** The bootstrap reads stored records only; the reader and the printer of
  the canonical print are `form/read.ck` and `form/print.ck`, and entry 77 unbound is the fault
  `unbound`. Machines exchange records: the asked service, the check an image keeps, the BML and
  ice outputs fed to `ck run`. `region/places.ck` and `form/print.ck` are written by their
  programs as records (`bin.write`) and `ck text` writes them out, so no print is read back to
  make them: measured with a region of 64 MiB, 330 M instructions for the places against 638 M
  (112 M to run and print, 526 M to read the print), and 6.8 G for the printer against 9.4 G;
  `check-header` holds the same bytes.
  The answer line of `ck run` is printed by the recipe `form/print.ck` answers, read from beside
  the executable (a link followed) and run on the bootstrap's code alone, so what the program bound
  does not change it; a copy of `ck` takes `form/print.ck` with it, and `region/rest.ck`'s saver
  `form/nodes.ck`, without which it saves nothing. Before the C went, every answer line of
  `make check-full` was printed by both and compared: no line differed; the printer as it stands
  was held to the C of 5bfb416 over 28 answers, bindings of `pack`, `leaf`, `choice` and the step
  among them, and three of 1,000 to 20,000 nodes, byte for byte. The printer writes into a
  room in two passes, so it is linear in the size of the print at any depth. Where the processor
  is AArch64 it runs as machine code (below); walked, a list of 3,000 numbers takes 1,555 M
  instructions in all where the C took 21 M. The answer line may take 16 MiB past the allowance, so a run that ends with an area
  full still says its fault: `(all (range 0 300000000))` at 4, 8, 16 and 32 MiB ended with
  `fault (full ...)`, where at 4 to 32 MiB the C ended with no room left for the print. The printer's file is read with that allowance too, by hand at 1 to
  64 MiB, since a larger one no longer fit in what the run had left. `ck` finds
  its folder from the host (`_NSGetExecutablePath`, `/proc/self/exe`), through `PATH` and links. An
  asked answer is written by `bin.write` beneath the asked run's bindings, and its marks are holds
  that the outcome takes back; an outside binding of `pack` or `leaf` no longer changes it.
- **The answer line runs code the emitter made of the printer.** `form/printer.ck` holds the
  walked printer and a second one written for the emitter, which keeps the nodes it has met in a
  table in a room where the walked one notes them; `make header` compiles it to a leaf of 16 KB
  and writes `form/print.ck` with it, the layout version it was made for and the nodes whose ids
  it names, and `make check-full` holds that the file is what the nodes make now. The leaf runs
  only on AArch64 and only under its layout version, by `native`, so where it declines the walked
  printer answers. Measured as whole `ck run`s (instructions retired): answering 42, 16.0 M against
  15.1 M walked and 14.0 M for the C of 5bfb416; 3,000 numbers, 123 M against 1,555 M and 21 M; a
  chain 10,000 deep, 343 M against 3,587 M and 83 M; 20,000 triples, 1.8 G against 24.0 G and
  204 M; a chain 200,000 deep declines at the machine stack's floor and is walked, 72.4 G against
  71.6 G and 1.4 G. The three printed every case byte for byte alike. `ground/encoding-in-form.ck`
  holds the two printers alike over every kind of leaf, shared parts, a recursion 300 deep and a
  table that grows, and that the code answered. Under Rosetta the printer is walked.
- **Code reads the head at places the layout's rows give.** `make header` writes them as numbers
  into `region/places.ck`; the emitter's call of `serve`, its stack check, the step leaf and the
  room operations take them there, and the machine's `pcount` and `unused` left the layout
  (version 28) with every compiled ground program unchanged. A room's `load` and `store` compile to
  the C's checks as instructions and its bytes read or written in place; a case they do not take
  calls `serve` as before. A `take` or `store` is compiled only into a leaf `native` runs, of a
  recipe that calls no other definition and has none under a search inside it, where a fail
  would return past it with no choice of the walker's to undo it; so where compiled code would
  keep a store the walker undoes, the emitter refuses. `ground/encoding-in-form.ck` runs both room
  operations at every width over rooms of six sizes at and past their edges, compiled and walked,
  and a narrowed bound and a wrong width planted in the emitter each made cases differ. Code
  trails every word it writes in a room that stood before it began, under a choice or not, and a
  leaf that declines has those put back and the rooms it took given back whole before the walker
  runs it again (the machine's `fresh` says where its own rooms begin), so a declined leaf leaves
  no room effect: `ground/rooms.ck` bumps a room at each width and offset 0 to 8 and then
  declines, and the room's bytes, writer and count of writes are the walker's, and a leaf that
  takes a room and declines leaves one room taken, not two; with either putting back taken out
  the count rose twice, the bumped value was bumped twice, or two rooms stood. Under an open choice
  code trails its own rooms too, since a fail may keep one a note names; where the trail cannot
  grow, code declines and the walker writes.
- **Code reads and writes the region's areas, and swaps a word of the substrate in one step.**
  `load`, `store` and `cas` (entry 97) take an area's number where they take a room: the region's
  own bytes at an offset, within what the area holds at its peak or its count, never trailed. Where the area and
  the width are constants the emitter writes the head's place of the area's offset, peak and size
  from the layout's rows (`fb.area-at` and the rest in `region/places.ck`), a bound and an
  alignment check, and one `ldr`/`str` at base plus offset; a substrate's bytes as `ldar`, `stlr`
  and `casal`, the orderings the C's own reads, writes and swaps of the substrate have. A word read
  inline costs 95 instructions a step of a compiled loop, against 1,649 through `serve` and 10,070
  walked (`.kit/jit/area-cost.sh`, same answers). Code writes an area only inline and only as the
  last act of a leaf `native` runs, where a room's store may compile: asked of `serve`, a `store`
  or `cas` on an area answers nothing unwritten, so code that would go on declines before it
  writes and the walker writes once; such a leaf calls no definition, itself included, so a
  declined leaf run again never writes twice. The heads, which place the areas, are only read; the
  bytes of other areas are the bootstrap's own records, and a program that rewrites a node's offset
  makes the next read of that node fault the process, as a `host` call of `memset` can: area writes
  are as trusted as `host` until only the store's own leaf writes them. `ground/rooms.ck`
  reads seven areas at four widths at and past their edges, writes and swaps a word of a room and
  of the lane's own record at every width over nine values, walked and as code, and answers no case
  that differs; a counter swapped and then a read past the edge, and a recursion that swaps and then
  stores, count once either way. A wrong
  width, a wrong swap width and the alignment check taken out, each planted in the emitter, made it
  answer some; with `serve` writing, or the recursion allowed, a word was written twice.
- **The store's find runs in Form, compiled, and answers what the C's make finds.**
  `form/store.ck` asks the region for the one node with a content: FNV-1a over the blueprint and the
  bytes or children (eight at a time where aligned), the process's working index (area 3) unless
  the last open choice is closed and all is settled, then the settled index (area 39) where the C
  asks it: alone where every part is settled, in a member by the rule `make_far` keeps, with a node
  of another lane counted only where this run may see it. It writes nothing: where the C's make
  would raise another lane's settled mark, or where an index grew while it looked, it answers -1,
  for the C to decide; a member reads another lane's records through the substrate's own areas
  (35 to 37), whose peak a named substrate sets to its cap. It says whether the content is there,
  not whether the rule would take it: refused content is answered 0. The places it reads are
  the layout's (`fb.settled-area`, `fb.lane-sw` and the rest in `region/places.ck`), folded to
  numbers by `emit.placed` before it is compiled. `ground/region-describes-itself.ck` finds every
  node of the region by its content with the compiled find, under a choice and alone or as a member
  beside others making nodes, makes no node while it does, finds the settled ones again by the path
  taken when no choice is open, and agrees with the walked find and with
  the C's make on content made by giving a node another's blueprint; a wrong multiplier, a compare
  that skips the blueprint, a byte or child compare that always holds, and a probe that stops at
  its first slot, each planted, made cases differ. A find from walked code costs about 20,000
  instructions for a 32-byte leaf where the C's make costs 1,000 to 2,000 (`.kit/jit/r4/idc.sh`:
  2,000 and 4,000 finds less the loop alone; runs vary); of a find, about 1,100 are the walker's
  call of code and about 57 each call inside it, and so the C's make stays the store.
- **A node is made in Form as code, and only as code.** `store.joined` makes what `join` makes:
  it finds the content by the list's own children, and only to make it copies them past the
  children's count and writes the record past the node count, and then writes the children's
  count, the index's place and the node count in one act. Walked it cannot be right, since its own integers are nodes made at the place it writes
  (seen: the record it wrote was an integer's); compiled it makes none. So it runs only as a leaf
  `native` runs, where it answers `no` for what only the C can do (a settling make of a process
  alone, an index or an area that must grow, content the rule may refuse, another lane's node
  this run may not yet see), and the C's `join` makes it. In a member it makes as the C does: under
  a choice into its working index, raising its lane's count; with no choice open it publishes
  by one run: a swap of the substrate's tide, a swap of the settled index's empty place from 0 to
  the node, then the counts, the lane's count and its settled mark. The place is where the probe
  of the index stopped; equal content met before it declines, and so does either swap answering
  another value, before any count is written (the record past the count stays unseen, and a
  bumped tide only makes a look at the lanes repeat). The
  emitter takes three things for it: a store in such a leaf that is not its last act writes only
  past the count of a lane's area (nodes, leaf bytes, children), checked as it runs, where walked
  code and `serve` neither read nor write, so a decline after it changes nothing they can see; a
  sequence of stores as the leaf's last act, each an argument or a number and the head only at a
  lane area's count, raised and within its peak, and the lane's own count and mark only with the
  value the node count takes, is checked whole before any is written; such a run may begin with
  swaps of the settled index (from 0 only) and the tide, each of which must answer what it
  expected or none of what follows is written; and
  such a leaf may call definitions that take, swap and store in rooms nothing, but not itself
  again. A lane's area holds the rest of the page it was charged for, so there is room past its
  count. A leaf may so count records it never wrote: it is as trusted as `host`. `ground/region-describes-itself.ck`
  makes 1,800 composites of every 61st node's children under three blueprints both ways, the C's
  `join` and the leaf (1,560 made or found by the leaf), and a list under each of the first 33
  blueprints, and every one agrees: refused, new or the same node, the count after it, the
  children's count, blueprint, kind, length, place, children, the record's flags, and that the
  index finds it; an index place one past, a children's count not raised, a length one more, a
  settled flag and a rule that takes the catalogue's blueprints, each planted, made cases differ.
  It also makes 300 composites with no choice open, compared with the C's records of their
  siblings (a member's leaf makes them and publishes them, and its index then has no empty place
  for content already settled), keeps the lane's count with the node count, and swaps a place of
  the settled index and the tide with values they do not hold, two swaps whose first fails, and
  a store to another lane's record, each of which declines: a swap whose answer is not checked, a
  swap from any value, later swaps run after a failed one, a lane store not checked, a place taken
  over equal content, a lane count not raised and a record with the wrong settled flag, each
  planted, made it differ. Not seen by it: a settled mark left unraised (the next commit settles
  it), a tide not moved (the C's own settled makes move it), and a seek that takes another lane's
  node before that lane marked it settled (only another member can show it). `ground-isolated` runs it once more as a
  member with no choice open, in a substrate of its own. Members racing to make the same content (the
  round probes race, fwd and fwd2 with every list made by the leaf, and 3,000 makes of the same
  content in three members at once, `.kit/jit/r6`) answered what one member answers, and the
  holder made every settled node again as itself. A make from walked code costs 2,700 to 5,200
  instructions against 400 to 2,400 for the C's `join` over four runs (2,000 and 4,000 makes of
  new content, less the loop; runs vary), so the C's make stays the store.
- **The walker returns and restores a fail by compiled Form the seed carries; the C has neither.**
  `ice/seed.ck` beside the executable is node records of one list: the layout version it was
  made for, then rows of a cell and the leaf of AArch64 it holds. The walker cannot return
  before the seed is in place, so the seed is not walked: the C checks the version and holds
  each row's leaf in its cell before anything runs, and again when a look at a saved image
  leaves the region fresh; a start entered from an image has them from the image, and the seed
  is among the files that image checks. Both leaves are `form/walk.ck`, compiled by
  `emit.floor`, the emitter's floor mode: stores anywhere in the process's own areas and the
  head, bounded by what each holds, a load or store of the head at a constant place as one
  instruction, and no entry check, since the leaves lie beneath every binding.
  - In floor mode a comparison of values that cannot be nothing (a constant, an argument, a
    let of one, a head place, the handle of a seeded entry, which is known when the code is
    made) is a compare and a branch, with no status kept around it, and an `if`'s test is a
    chain of branches. A call is skipped by one `cbnz` while the status is set, which ends the
    process. The rows after the first are called only by the leaf itself: they keep x27 and x28
    as the caller has them, save x19 to x26 and the arguments they read, and give the status as
    it is. An area's base and bound are read once for a run of loads and stores of it in a
    `seq` or `let`, and again after the slow path, and it passes through a call to an inlined row
    whose arguments are all put in place. An offset of a small constant added to or taken from
    a value is a plain add or sub: a wrapped sum is negative or beyond every bound, so the
    check sends it to C, which refuses it as before. An area access in every mode checks sign
    and alignment with one `tst`. An inlined callee's constant or argument is put in place of
    its slot.
  - In every mode, a part is spilled only when the registers it can use do not fit, a constant
    area and width are put in registers only on the slow path, and add and sub of a constant
    under 4,096 take it as an immediate.
  - The cell of `frame` holds `walk.ret`, the walker's return: it pops the frame as the C did
    (the frames and links above it given back when no open choice returns to them, the
    activations made since given back), takes a sequence's next step, links an argument's
    value and starts the next, and takes `fail`, `need`, `recover` and `layer`, popping frame
    after frame in a loop while the run keeps returning. What it leaves, it answers by the
    frame's place: a cut, an all, a let, an outcome, a handler, an aside, and a call or an
    operation whose values are all linked, which the C goes on with. An answer that is no frame
    it popped, or a frame it takes itself, ends the process.
  - The cell of `choice-point` holds `walk.restore`, the restore of a choice: the frames, links,
    activations, arguments, held-back calls and the machine's `env` and `k` set from the
    choice's marks, the bindings made since undone, cells put back from the trail, the trail
    kept of what no fail undoes, and room words put back. The walker's fail calls it and goes on
    with the giving back of nodes, ranges, outcomes and gathering in C; a leaf that leaves the
    frames, bindings, room trail or trail off the choice's marks ends the process.
  - A cell with no code this processor runs ends the process at its first use; a processor fault
    in a leaf is the walker's own. A process with no seed beside it, a seed made for another
    layout version, or a processor other than AArch64 is refused at start with a plain line.
  - The seed is built by the system: `form/seed.ck` reads `form/walk.ck`, which compiles both
    leaves, and writes the list with `bin.write`. `make header` writes it, and `check-header`
    builds it again on the seed itself, walking with its own return, and compares the bytes:
    the same 22,167 bytes. `check-held-back` holds that leaves the
    run compiles again and holds mid-run return and restore as the seed's do.
  - Held: the catalogue answered alike under the C walker of `733d27a` and under the seed's
    return, each of the 77 programs' outcome and its verdict, one process each at the default
    8 MB stack, `cmp` of 154 answers but one. The one is `ground/code-on-nodes.ck`'s
    100,000-deep compiled recursion, which meets the stack's floor there: under `733d27a` the
    calling row read the refusal as an answer and gave a wrong one ("differs"); now the refusal
    reaches it and the walker answers 100,000. Under `check-full`'s 64 MB stack both pass. `ground/need-moves-on.ck` returns through 200,000 frames of `need` in a row:
    the loop of returns is a jump, so it runs in a constant stack. Planted in `walk.ret`, each
    made a program differ or the process end: `no` let through a `need` (the answer `no`
    where 7 is pinned, the lens loops, and `ground/check-one.ck` names the path it judged in
    place of `#33`: its verdict is picked by the handle of one comparison, not by a branch, so
    it does not stand on the `need` it checks), a pushed frame's link mark taken from the
    activations (no answer line), a frame dropped though a choice can return to it (faults of
    kind), a layer's bindings seeing one fewer (`ground/loop-in-place.ck`), and the loop of
    returns made a call that is not a jump (the stack's floor is met, and the C ends the
    process). A refusal at the stack's floor inside code is now said to the caller, where
    before the caller read the argument it gave as the answer.
  - Cost, measured with `/usr/bin/time -l`, N rounds against 2N: a round of a counting loop
    `{n | (if (n = 0) 0 (call (self) (n - 1)))}` takes about 7,100 instructions against 4,420
    with the C return of `733d27a`, and a call of `fib` about 8,300 against 4,650 (10,000 and
    11,800 before the floor mode above). The return is the walker's step and costs about twice
    the C until `eval` is in compiled code too and a run stays inside it.
  When the seed is lost or broken: every seed is in git, and `make header` with the previous one
  builds the current. With none that runs, the last commit whose C walker returns and restores a
  fail alone, with no seed (`4d7b155`), builds a seed from the nodes, and that seed builds the next.
- **A node's parts are read by instructions, and code checks at entry what it assumed.** `count`,
  `byte`, `child`, `blueprint`, `leaf`, `handle` and `at` compile to reads of the node's record
  (`at` of a handle of the process's own lane, in any lane), and any case they do not take calls
  `serve` as before; `handle` keeps what `adopt` asks of it in
  two words of the machine (layout 29), not in C. A leaf's first word branches to a check that
  declines, so the walker answers, while a binding from outside or of the program's own sits on an
  operation the leaf runs, or a definition it carries is bound to another recipe than the one it
  was compiled from or by a binding made since, or the leaf carries definitions and runs in another
  process than the one that made it, or is itself bound from outside; calls inside the leaf go past
  it. The emitter compiled by itself writes the same leaves as walked (`ground/compiles-its-own.ck`),
  measured once at 2.4 s against 8.5 s for the definitions it does not compile. `ground/code-on-nodes.ck` binds `count` and
  a callee inside a run, under a fail and kept, and from outside, bound and taken away, while two
  compiled definitions and a leaf run by `native` are live: every answer is the walker's, and the
  emitter with either check taken out made cases differ. The check costs about 90 instructions at
  each entry of the printer; node reads and `handle` took the printer from 152 M to 123 M on 3,000
  numbers.
- **Another process's region is read in Form, and the C that read it is gone.** Entry 97,
  `look`, and the host's `watch` left the bootstrap; `region/far.ck` maps the region by the
  host's rows that `share/share.ck` holds and reads it with the load leaf, which runs under an
  open choice where a host call may not. Before the C went, both read a finished run's region
  at 24 offsets (page edges, its last bytes, one past, -1) and 12 lengths: the 288 answers were
  the same, `range` included, and three objects that are no region (none, 100 bytes, 64 KiB of
  zeros) were refused with `no-region` by both; `region/watch.ck` showed a lens run's region,
  148 lines with every binding, the same bytes through both, in 3.4 s where `look` took 2.1 s.
  `make check-live` reads its live region from outside and holds the magic, the size read two
  ways, the last word, an empty read, three reads past an edge and a name that is no region; a
  byte mask, a shift and a bound planted in the package each made it fail. A region of another
  layout version is refused, where `look` read it as this one.
- **Emitted code is the walker or faster, never different.** It answers a number or a node, or declines. For
  an entry it is a faster form of the entry's own binding, used wherever the recipe would run,
  and it steps aside while something from outside watches an operation it replaced.
  Twenty-nine recipes over numbers, eighty over nodes, thirty-six that test, fail and choose,
  thirty-three that search and twenty-five that search other definitions end the same both
  ways; forty-six faults planted in the emitter and the bootstrap turned that red.
  Definitions that call each other are compiled into one leaf, and a recursion written as a loop
  runs as one: thirty million rounds, each calling a second definition, in 0.4 s.
- **A small callee is emitted in place, and stays a row.** A call by name of a definition the leaf
  carries, of at most six arguments and a body under 64 nodes with no search, no `self`, no recipe
  and no store, is emitted where it is called, up to four deep: each argument kept at a place
  free there, in the kind the callee's row wants, and the callee's body with its places moved
  past the caller's. The callee keeps its row, so the entry check sees a swap of it and the
  caller declines. A call of a definition by itself in last place is a loop: the arguments are
  kept in their places and the code branches back past the start, where the start's check of how
  many arguments came is kept by taking the loop only with as many as the body reads. Where
  registers run out, a part is emitted at the first register with the ones below it kept in free
  places, so a let may take any of 32 places and an `if` deep in a call's arguments compiles; a
  row's `gives` is settled a second time from a number, so a self recursion does not stay a node.
  `ground/code-or-walker.ck` holds these against the walker (lets at places 31 and 32, a
  condition as a sixth argument two calls deep, inlined callees given a node, text, a failing
  and a missing argument, a self loop given too few), and `ground/code-on-nodes.ck` swaps an
  inlined callee under a failing choice and from outside; places moved one too far, a spill that
  does not restore, a loop keeping its arguments in the wrong places, the callee checks taken out,
  arguments not converted to the wanted kind and the loop taken with too few arguments, each
  planted, made them fail. It costs code: the store's find is a leaf of 9,785 instructions where it
  was 6,381, and the make 16,945 where it was 10,240. A find of a 32-byte leaf called 20,000 and
  40,000 times from walked code took 6,000 to 7,200 instructions each against 9,100 to 9,400
  before, where the C's make takes about 1,000 (runs vary); most of what is left is the walker's
  call of code, the entry check and the hash, whose loop takes about 47 instructions a byte.
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
- **One allowance covers the runtime's memory.** The framebuffer tables draw from a common
  pool sized from available host memory. The layout does not follow it: each area reserves the
  range `region/layout.ck` gives it, the same in every process, and the allowance is a word of
  its own. The stack and executable pages count toward it.
  The live run under `make check` recovers from a room request beyond its 32 MiB allowance
  and finishes with the expected answer. Ordinary runs use anonymous shared memory. A named
  run can be read after exit. See [the framebuffer](spec/framebuffer.md#the-region).

**Writing**

- **The lens reads its own definition.** It shows and reads back the records listed in
  `ground/files.ck`, including their names, byte for byte. Text it cannot read is a fault
  that names what and where.
- **Names lie beside the nodes.** Arguments are numbers in a node and names in records stored after
  the node's in its file. Every module and ground program was named; naming changed no node. A sign stands
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

- **A small model can continue from a tool result.** In `make check-image`, nine examples fit a
  categorical model. Its predictions select subtraction or comparison. A separate process reads
  the fitted model from the image and uses the actual answers to make the next prediction across
  eighteen calls. A sibling changed one learned continuation and observed the image comparison
  fail. The categorical model is separate from the fitted language model below.
- **A bilingual reading can be shared under an exact grant.** English “We may share” gives two
  actor/action readings. `ground/scoped-delivery.ck` selects the exclusive one and sends “Kami
  boleh berbagi” to an anonymous local stream. The same recipient and bytes under the other
  reading are declined. The ground also withdraws permission after queueing and compares the
  bytes left by complete, empty and partial writes. A sibling substituted a writer that claimed
  success without writing. The physical stream comparison caught it. [Scoped delivery](spec/core-operations.md#scoped-delivery)
- **Two local roles can disagree and still exchange one authorized letter.** A listener proposes
  the inclusive reading of “We may share”. The speaker selects the exclusive reading. A refusal
  and an explicit silence retain both positions. Separate processes publish “Kami boleh berbagi”
  only after the owner grants that delivery and the listener invites it. A retry after a lost
  acknowledgment finds the same file. Eight sibling-run receivers racing for another occurrence
  produced one acceptance and seven replay receipts. This witnesses local process behavior.
  Two independently operated cells also exchanged different readings under fresh
  consent in each direction. A human account of trust or vitality would have to
  come from the person reporting it.
  [Local inbox and its boundaries](spec/core-operations.md#local-inbox)
- **A receiving broker can attest what arrived across machines.** A Linux guest signed its
  invitation using an independently enrolled listener key. A signed owner grant and listener
  invitation permitted publication of a 137-byte letter. The guest signed its observation of
  those bytes, and the macOS sender verified the 591-byte receipt against the original
  expectation. [Signed controls](spec/core-operations.md#signed-consent-controls) and
  [receipt verification](spec/mesh-receipts.md)
- **What a broker knows depends on which controls reached it.** A broker with a withdrawal
  refused a delivery that a disconnected peer still allowed. Retaining the highest observed time
  stopped a clock rollback at that broker. A separate witness that retained the latest control
  revisions also stopped a restored receiver from publishing under a superseded grant. A
  withdrawal still held only by its owner cannot inform the receiver or witness.
  [Time and control revisions](spec/mesh-receipts.md#independent-time-and-control-revisions)
- **Seed can train its own causal language model and generate with the saved weights.**
  Eighteen synthetic sentences trained a 15-token model. A new process generated the
  expected verb and an end token for all six withheld adjective and noun combinations.
  A separate scalar implementation matched every fitted weight. [Local language learning](spec/local-learning.md)
- **A resident 3B model consumed the result of a Form recipe.** Changing the subtraction
  result from token 20 to 22 changed the next prediction from 14 to 56012. A separate
  local decoder reproduced both. The run used a pinned native Metal worker and GGUF.
  [Resident inference](spec/resident-inference.md)
- **Better token loss alone can select the wrong adapter.** On independently written
  paired dialogues, two locally fitted adapters failed to answer both owners correctly
  on any pair. The unchanged model given a public codebook and the selected owner's
  statement succeeded on 16 of 18 fresh pairs. [Measured trials](spec/local-learning.md#measured-3b-adapter-trials)
- **An owner can grant teaching and still withhold use of the fitted artifact.**
  The local consumer required an admitted teaching round, a current derivative grant,
  a passing comparison and the exact selected bytes. A grant for the first fitted 3B
  adapter left the active selection empty because its evaluation was held. A separate
  run corrected a false claim against its cited sources and retained the original
  wording alongside the correction. [Witness and teaching](spec/witness-and-teaching.md)
- **An observed event can overturn a forecast and change the next one.** A local
  instrument contradicted the preferred rule at sequence 11. A fresh process retained
  that forecast, the measured bytes and both rules' error counts. The next reading
  confirmed the newly preferred rule. [Predictions and observed events](spec/observations.md)
- **A recipient assembled two spoken turns after a correction.** Withdrawing the
  first Indonesian offer kept it out of the stream. After both roles accepted
  the correction, local transcription of the received PCM recovered the English
  sentence and the corrected Indonesian pronoun. [Spoken turns](spec/voice.md)
- **The graph holds.** `./kg check` reports structural errors, warnings and the review state
  of the claims. The sibling experiments above are behavioral comparisons.

The same delivery and evidence boundaries now support these observed runs.

| Question | Observation |
| --- | --- |
| Can someone restore an old receiver and release a superseded grant? | A time-only witness let an old grant deliver 137 bytes. A separate witness retaining both control revisions refused that replay. A Linux guest signed its actual arrival, and the sender reconciled receipts out of order. [Mesh receipts](spec/mesh-receipts.md) |
| Does the model respect evidence and a stop request? | On four synthetic prompts, the local 3B model accepted a false premise and invented a motive. The hosted session identified the superseded source. Both followed the source switch and respected the stop. [Prompts and observed answers](spec/model-behavior.md) |
| Can a locally trained model reach a consumer under explicit permission? | The owned model improved from 0/6 to 6/6 held-out completions, reached inference through an independent derivative grant and became unavailable to a fresh consumer after withdrawal. Three further 3B comparisons improved aggregate results while each lost a protected owner pair. [Local learning](spec/local-learning.md) |
| Can a gauge reading keep its uncertainty? | Two independent readers agreed on 12,090 mV in eight overlapping windows. The journal retained both readers' raw values and time brackets, including when the declared allowance crossed the interpretation boundary. [Instrument evidence](spec/observations.md) |
| Can a fitted adapter improve its task and preserve unrelated behavior? | A step-by-step gate let generated text activate the adapter during an unrelated request. Binding the single adapted decision to the original caller context produced 22/22 expected results, including longer protected continuations. Its parent and matched control produced 16/22 each. Withdrawing the grant prevented a fresh use. [Native rank adapter](spec/rank-adapter.md) |
| Who can replace a lost key? | An owner can enroll an independent delegate and later cancel that authority. A chartered transition established a new episode after a worker interruption and refused old controls. A disconnected copy still acted on the old episode. [Key recovery](spec/key-recovery.md) |
| Can a recipient pause during a word or sentence and recover from a lost reply? | The recipient paused a synthetic English turn at 13,486 frames. Local recognition recovered “We may listen before.” Two resumed processes produced the complete bilingual file once. [Spoken turns](spec/voice.md#pausing-inside-an-utterance) |
| How do learned and transparent readers interpret the same owner's words? | On 32 synthetic statements the cue reader got 6 correct with 25 abstentions, the native specialist 20, the unchanged 3B model 22, and each raw LoRA 28. Each LoRA predicted permission for three statements about stopping, indecision or correction. On a separate undecided statement the native specialist predicted permission. The broker refused that prediction and delivered the excerpt only after the owner signed the grant for the waiting listener. [Intent comparison](spec/intent-specialist.md) |
| Can a multilingual correction reach a listener under fresh consent? | The corrected Indonesian wording changed the exact review scope. A queued superseded position failed. After the original grant was withdrawn, the corrected text reached the receiving file only under a new listener invitation. [Translation and consent](spec/translation.md#offering-chosen-wording-to-a-reader) |
| How is first enrollment distinguished from permission to share? | Both roles signed statements about the same roster and their own roles. Another validly endorsed episode could open a new broker. The advanced broker refused it and preserved its two consent records and delivered letter. [Participant commands](spec/participant-circle.md) |
| Can two independent cells disagree, keep private accounts and stop delivery? | Birch and Reed each controlled their signer, journal and decisions on one host. A second invitation remained unanswered. Reed's signed pause blocked the next forward read and preserved the letter it had already received. Fresh consent in the reverse direction delivered Reed's different reading to Birch. An explicit Satsang silence turn still awaits an independent-cell witness. [Participant study](spec/participant-circle.md#what-two-independent-cells-observed) |

## Not there

- **The rest of the NUMS engine.** The catalogue, constructed cells, conversion between scalar
  layouts, typed method dispatch and specialization, a genetic population and two expression
  lenses run, and so does a generic runtime for declared functions: frames in rooms, control as
  written, integer arithmetic of every width, arithmetic on every float layout, and machine
  code for whole functions. Whole-package BML lowering, the full Go grammar, floating-point GPU reductions and
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
  a fail returns past it, so a loop that counts long enough can spend the run's allowance. A handle
  kept as a number across a fail back past where its node was made can name another node
  afterwards.
- **Expectations of meaning only.** Some ground programs still answer values that move without a
  change of meaning: sizes of compiled code, counts of definitions and of a lens's nodes, labels
  of what the emitter compiles, step thresholds and two comparisons of clock time.
- **More of the warm start.** Only macOS has it: the executable's identity is its `LC_UUID`, and
  on Linux no build id is read, so every start there is cold. The Metal package, the Metal
  image's first part and `share/all.ck` load, are saved, and leave their opening to run after the
  start; the opening itself is not saved, so a warm Metal image start still asks the device (839 M
  instructions and 29.7 ms against 1,360 M and 51.7 ms cold). A start that runs any code is cold,
  compiled code too: nothing yet tells code the compiler made, which reads only what it is given,
  from code that reads anything. The name, the gate, the saver and the directory's size are
  recipes of `region/rest.ck`, which every start with files before its program reads, and the saver
  reads the writer of records, `form/nodes.ck`, from beside the executable: a warm lens
  flow takes 29.2 M instructions and 2.26 ms where the gate in C took 18 M and 1.53 ms. Images are kept by default in the user's cache, held
  to 64 MB or a sixteenth of the free disk by removing those read longest ago. A page of
  an image is trusted as written: one byte changed in it is answered from, where a cold start
  answers otherwise; hashing every page of the lens image would cost 0.26 to 0.6 ms of its 1.67
  ms and read every page in. The node an image runs before it is entered may use only the
  operations of the bootstrap, the values it carries and the files beside the executable, since
  nothing is bound where it runs, so
  `region/rest.ck` is a `let` of recipes and not a component package, and it is not among the
  files `make check-bare` counts.
- **An area larger than its reservation.** Each area holds at most what `region/layout.ck`
  reserves for it, however large the allowance: 1 GiB for the text, the frames, links,
  activations and most of the walker's tables, 256 MiB for the asked and answered bytes, born code
  and the bindings. A file of more than 1 GiB is the fault `full` with `text` here, where on main
  it was read when the allowance held it.
- **A neutral store.** What is stored should be meaning in the numeric space with every word
  beside it. One step is taken: the 75 operations describe themselves as nodes, what each
  takes, gives and does to the world (`form/operations.ck`), and the emitter reads the kind of
  a value, which places are numbers and what to ask of the bootstrap from them; arithmetic's
  instructions are rows of data. The rest is not there. The nodes read a part by a bare number
  464 times, the budget `make check-full` holds: the shape of a part is said only in the name
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
  is no entry's does not see what the program binds after it was made. Each operation on a node but a room's `load`
  and `store` is a call into the bootstrap and not instructions; `take`, and a room's operations
  anywhere but in a leaf `native` runs, are not compiled. The emitter writes AArch64 only: on
  x86-64 the answer line is walked.
- **The home lens.** Every form of the first lens is in brackets: a sign stands between two and
  no more. The BML lens has character patterns and precedence. The home lens still uses brackets.
- **Form everywhere.** Form source and its names are stored as records. The remaining host sources
  include the bootstrap with its generated header, the recorder and graph checker, and the
  Makefile that drives the cross-process checks. The graph, documentation and axioms are text.
- **More of objects.** Attributes are not built, and the activation does not carry the reference: a
  method is handed it.
- **Threads.** One walker runs on one thread. A fault on a thread the host made ends the process
  with a message and does not enter the walker: provoked once by hand, with a leaf as the entry
  of `pthread_create`; no check holds it.
- **More of the host.** Rows and leaves exist for `aapcs64` only; on any other target
  `ground/host-refused.ck` does not apply. A structure passed by value in more than two words,
  the indirect-result register (its place is in the berth, no row uses it), sockets, processes
  and signals have no rows; a structure of more than two words goes by reference from a room.
  Memory a host function hands out is outside the allowance. What the host
  ties to the process's first thread cannot be reached: the walker is not that thread. Of the
  bootstrap's own host functions Metal and those a member needed have left C; the eleven a
  process alone starts with are still there. Metal prepares its message signatures when it opens. On the M4 Max, the same 481-stage
  Llama graph with layout-matched kernels took about 108 ms to encode with prepared
  calls and prebound buffer views. The general row path took about 874 ms. The two
  paths returned byte-identical logits across four turns in two stream histories.
  The native call observes the host choice scope (`ground/host-refused.ck`). A program that lays the berth out by hand and runs its leaf is not refused.
  A dispatch is not reentrant: entering the door's guard from inside it is a fault.
  A library leaf bound as an entry's implementation does not run the map, where the C ran it
  for operands of the right shape: it declines, and the map is reached by `native`. A map makes
  its device and pipeline anew each time, as the C did. The compiler does not yet emit a row
  as moves and a call; code reaches a row only through `native` of a door leaf. A door is
  taken outside a search only, since resolving the address calls the host. The 64 cells are
  never given back: a row past them goes through the berth.
- **A channel between members.** Members of a named substrate share settled nodes, and a handle
  names the same node in each, but nothing yet carries a handle from one to another: `swap`,
  `share`, shared rooms, the inbox served between steps, the bell and the side front are not
  built, and asking goes by records through `asked`. The numeric port does not answer by
  handle.
- **More of the substrate.** A lane is never used again, so a named substrate has 255 members
  after its maker in its life, and the next to come is refused; its indexes never grow, and a
  member's first thousands of nodes touch nearly every page of them; what a lane charged is never
  given back, so one member can use up the allowance for all that come after. These three are the
  design's stage 5. A process becomes a member only where the host's package has rows: macOS on
  Apple silicon. A member the host kills outright does not leave; its row and, if it was the
  last, the name stay until the next process looks. A member that leaves a full substrate takes
  about a page above the allowance to do it.
  A member's first start loads the two packages through the search, about 545 M instructions,
  where reading the door as one bound node took 109 M; every later one enters their image and
  makes a substrate in 98.9 M and 5.4 ms, or joins one in 77.5 M and 5.4 ms, against 109 M and 93 M
  on main. A process that has given out a handle above the catalogue is refused
  when it would join as other than the maker, since that number would name the maker's node at
  that place after; a number computed and not given by `handle` has no such claim and may name
  another node, as a number kept across a fail can. The joiner's catalogue is compared by a digest
  of its words and numbers; a mismatch was provoked by hand and is held by no check. The writer follows calls in supplied dependency packages. Projection of the full substrate and host packages
  still requires known answer contracts for their reachable operations. The observer shows a member but does not
  record one. The lane rows count the nodes forwarded, not those settled in place or refused.
- **A dispatch makes nodes of addresses.** What the host answers a row is an integer node, an
  address among them, different in each run; a map leaves one whenever the host hands it an
  address it had not met. `ground/metal-holds.ck` holds main's expectation, that twice the maps
  leave what half as many do, and beside it that what they leave are numbers only; the first
  failed one run in twenty here. Keeping addresses in the door's room and out of the nodes is
  not built.
- **A stack limit above the default.** The case of `ground/code-on-nodes.ck` that recurses a
  hundred thousand deep in machine code declines under the 8 MiB stack limit this session's shell
  gave, at 01db529 as well, and then differs from its expectation; the catalogue was run under
  `ulimit -s 65520`.
- **A second reader** of the encoding, and a second hand to review any claim (6.1.30). The
  encoding in form was written by the hand that wrote the page; nodes are still made by the
  bootstrap's store, and the id in form is too slow to be the one in daily use.
- **An answer to what a node's id is.** SHA-256 was chosen without being asked for (6.1.31).
- **Limits of host accounting.** Each process samples headroom independently. The allowance
  covers runtime-managed pages. Foreign code can allocate memory through the host. The
  observer's request count uses four bytes of the eight in the layout; the walker keeps
  the high four at zero.

## Guidance

Held together and kept short (graph node 7.13): only the axioms are fixed; form first, with C as
the bootstrap only; every part replaceable; real flows watched live before checks; fail undoes
everything back to the last choice; several parents are one composed shape; ice, water and gas
are states of any node; all memory is the framebuffer; pages say only what is.
