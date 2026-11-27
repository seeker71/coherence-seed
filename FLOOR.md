# Floor

What stands, as observed on **2026-11-27** on an Apple M4 Max running macOS, including Rosetta and an emulated x86-64 Linux guest.
[`NORTH_STAR.md`](NORTH_STAR.md) says where this is going.

## What runs

| Piece | Where | State |
| --- | --- | --- |
| Node encoding | [`spec/node-encoding.md`](spec/node-encoding.md) | written, with 146 conformance vectors kept as a node |
| Bootstrap | [`bootstrap/`](bootstrap) | The C cold start is counted at each build, 2,159 lines, 28 fewer since another process's region is read in Form, and is to shrink as parts leave it for Form. A numeric port runs resolved images. The authoring tool also reads the canonical print |
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
| Claim graph | [`graph/`](graph) | 1,461 nodes: the ground, the design, what is open, the path, and the evidence they stand on |

## Observed

Each line here is held by a program in `make check-full`, or names how it was seen.

**The walker**

- **`make check` runs three flows.** A BML expression passes through the source lens and the
  runtime. A standalone process receives a packed image and answers requests by id. A live
  recursion accepts an addition counter from another process and finishes with the expected answer.
  `make check-full` also runs the applicable ground programs separately and together (77 on AArch64, 63 on x86-64), compares the
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
  and ran a generated NUMS function directly on macOS AArch64, macOS x86-64 under Rosetta,
  and Linux x86-64 under QEMU. The same image ran a conditional by ID before and after compilation
  and returned 2 and 42 for its two arms. The x86-64 compiler also handles guarded returns and
  nested integer or Boolean selections. Calls, loops and floating-point bodies use frames on
  this target. [Backend interface and coverage](spec/nums.md#execution-and-specialization)
- **Metal runs by ID on both macOS process architectures.** On the M4 Max, one image
  returned identical replies on arm64 and Rosetta x86-64. The image comparison runs the
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
  matched the sampled GGUF weights. Only the selected result ranges cross the port.
  A live worker mounted the Llama model once, kept all 255 weight mappings across turns
  for two distinct KV histories, and released them on retirement while the worker stayed
  alive. Both histories agreed with the previous decoder on both process architectures.
  [Metal batches and resident turns](spec/nums.md#named-metal-batches)
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
  into a syntax tree with the BML reader. Through `nums/write-text.ck` the writer writes itself
  as 2,949 lines of Python, 4,980 of C#, 3,374 of TypeScript, 3,522 of Java and 2,916 of BML.
  Python's parser takes the Python, and ruff finds in it no undefined name, redefinition, unused
  local or comparison with `None`, `True` or `False`. Against a stub of the runtime that types
  every name `any`, Deno's checker refuses 81 places in the TypeScript, 77 of them a value of type
  unknown used where a type is needed; against small stubs, javac 21 refuses 121 and Roslyn 349.
  A member read is cast to the shape it reads where the receiver's static type is another, and a
  value of unknown type to the type its place declares. Numbers the package names are written by
  name, and long argument lists are broken at the language's width. The BML is written as
  `section [form.search]`, which the BML reader takes beside `[form.bml]`, and its first lines say
  it reads as Form reads its nodes, where `choice` answers every option; no reader yet lowers
  those searches to recipes. The encoder's package, `nums/arm.ck`, and the compiler's,
  `nums/compile.ck`, are written by the same writer in all five; the compiler's Python parses and
  its BML reads back. The writer writes no call into another package, so the compiler is handed
  the encoder's functions and the runtime's as recipes.
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
  region, laid out as version 25 and described in `region/layout.ck`. A node made while no choice
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
- **The flow.** `make check-native` runs `ground/host-refused.ck` by itself, with `ck` as built
  and with a copy signed for the hardened runtime: a file is written, read back three ways and
  removed; four words, two of them differing only above bit 31, are sorted by the host through
  a callback that is leaf code; the square root of 4 and of 2 and 2 to the tenth come back as
  floats; two varying values are formatted; a page is mapped, written through an absolute
  store, read and unmapped; a raw supervisor call answers the process id, and one that fails
  answers no. Where entry 98 can carry a call, by name, by address and through the berth
  answer the same. A value of the wrong kind for its slot, a composite where an address goes,
  a call with more values than slots and a function with no row answer nothing. A supervisor
  call and an absolute store inside a search are the fault `irreversible`, as entry 98 is, and
  the store is not made. Forty calls that crash inside the host leave the host's heap with the
  bytes in use it had. An object made by the message `new` is let go by `release`, and a
  message with no row answers nothing. Of the 30 function rows, 22 are run by the flow and
  four more by the Metal flow of `make check-image`; `mprotect`, `dlopen` and
  `dlsym` are outside that comparison. Metal reads VM counters through `sysctlbyname`. The host and Metal flows exercise the message rows they use. Inside the ground catalogue's search the same
  program is the refusal `irreversible`, at its first call, which is by address. By hand, not
  held by a check: two wrong place rows made the run name the five cases they break; a row
  naming a library that is not there is a fault that names the library; a call before the door
  is opened is a fault that names `host.open`.
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
  layout version is refused, where `look` read it as this one. 97 has no operation now: a node
  of it faults, where the bootstrap's arithmetic would have multiplied its two values.
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
- **One allowance covers the runtime's memory.** The framebuffer tables draw from a common
  pool sized from available host memory. The layout does not follow it: each area reserves the
  range `region/layout.ck` gives it, the same in every process, and the allowance is a word of
  its own. The stack and executable pages count toward it.
  The live run under `make check` recovers from a room request beyond its 32 MiB allowance
  and finishes with the expected answer. Ordinary runs use anonymous shared memory. A named
  run can be read after exit. See [the framebuffer](spec/framebuffer.md#the-region).

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

- **The graph holds.** `./kg check`: no errors over 1,265 claims, none reviewed by a second reader.

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
  recipes of `region/rest.ck`, which every start with files before its program reads: a warm lens
  flow takes 29.2 M instructions and 2.26 ms where the gate in C took 18 M and 1.53 ms. Images are kept by default in the user's cache, held
  to 64 MB or a sixteenth of the free disk by removing those read longest ago. A page of
  an image is trusted as written: one byte changed in it is answered from, where a cold start
  answers otherwise; hashing every page of the lens image would cost 0.26 to 0.6 ms of its 1.67
  ms and read every page in. The node an image runs before it is entered may use only the
  operations of the bootstrap and the values it carries, since nothing is bound where it runs, so
  `region/rest.ck` is a `let` of recipes and not a component package, and it is not among the
  files `make check-bare` counts.
- **An area larger than its reservation.** Each area holds at most what `region/layout.ck`
  reserves for it, however large the allowance: 1 GiB for the text, the print, the frames, links,
  activations and most of the walker's tables, 256 MiB for the asked and answered bytes, born code
  and the bindings. A file of more than 1 GiB is the fault `full` with `text` here, where on main
  it was read when the allowance held it.
- **A neutral store.** What is stored should be meaning in the numeric space with every word
  beside it. One step is taken: the 75 operations describe themselves as nodes, what each
  takes, gives and does to the world (`form/operations.ck`), and the emitter reads the kind of
  a value, which places are numbers and what to ask of the bootstrap from them; arithmetic's
  instructions are rows of data. The rest is not there. The nodes read a part by a bare number
  474 times, the budget `make check-full` holds: the shape of a part is said only in the name
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
  graph's checker (681 lines of C under `tools/`), the Makefile, eleven pages, the claim
  graph as 57 text files, and the axioms' text. The Makefile still drives cross-process checks.
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
  as moves and a call.
- **A channel between members.** Members of a named substrate share settled nodes, and a handle
  names the same node in each, but nothing yet carries a handle from one to another: `swap`,
  `share`, shared rooms, the inbox served between steps, the bell and the side front are not
  built, and asking still goes by print through `asked`. The numeric port does not answer by
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
  of its words and numbers; a mismatch was provoked by hand and is held by no check. The writer cannot show `share/share.ck` in a language, nor `host/host.ck`: it
  does not write a call of another package's function. The observer shows a member but does not
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
