# The framebuffer, floor version

The definition (graph nodes 5.7.6 and 5.7.7): all memory of every object, node and stack
is read and written in one shared memory space, which can be recorded like video with no cost to
the runtime and played back to see what happened, when and where. The blueprint is stored next to
each object, and a record can name who last wrote it.

This page fixes how the bootstrap does that. `bootstrap/fb.h` is the layout in C, `bootstrap/ck.c`
is the only writer, and `tools/fb.c` is the observer.

## The region

- The bootstrap reserves a shared region with stable addresses. An ordinary run uses an
  anonymous mapping, which the OS releases even if the process is killed. Set `CK_FB` to
  publish a run by name. A published run remains available after exit until `fb drop NAME`.
- The runtime gives all tables one allowance for touched memory. It charges their page-rounded
  high-water marks, the two heads, the thread's entire stack and the executable copy of code.
  Cells, bound-by-node and print-numbers are keyed by handle, so they are charged by the pages
  they use instead: a page is charged when it is first used, one bit for it in `touched`, and a
  page not yet used is never read, since reading it would take it.
  Backtracking can reuse charged pages. Each table retains space to grow into the common pool.
  `fb show NAME` reports both the virtual reservation and the shared memory allowance.
- At startup, available memory is divided by the number of online CPUs plus one, leaving room
  for other work. On macOS this uses free and inactive pages under normal pressure, free pages
  at warning, and zero at critical pressure. Other supported Unix builds use available physical
  pages. Linux also consults free backing space for a named region. A finite address-space
  limit constrains the layout. `CK_MEMORY_BYTES` can lower the allowance in decimal bytes.
- A request past the allowance follows Form's existing `full` fault for the growing area.
  As an area's high-water mark crosses a MiB boundary, the runtime samples the host again
  before admitting growth. File input is read in bounded chunks and reports a full text area
  if the complete input will not fit.
- Each area but the stack begins as many lines of the data cache into its first page as there are
  areas before it, so the first records of no two areas fall on one set of that cache; the host
  says how long a line is (`hw.cachelinesize`, 128 bytes here), and where it does not say, every
  area begins on a page. A reader finds each area by its offset in the head.

## What is in it

The first page is the **head**: a magic word, the layout version (20), the writer's process id, the
walker's registers (the **machine**), one **area** entry per table, and the **member** part: where
the substrate lies (`share`), how many pages the tables keyed by handle have charged, and the
process's lane. An area entry says what its records are, how large one is, the most it has held,
where the table starts, how many fit and how many are in use; the last four are counted by 64 bits.

Machine code reads the region too, and three places in the head are kept for it whatever else
moves: at byte 28 the **floor**, the lowest page the machine stack may reach, counted in pages of
16,384 bytes from the region's start; at byte 128 the machine's `serve`, the address code calls
to have the bootstrap do an operation on values in hand (`spec/core-operations.md`, Native
code); and at byte 160 where the nodes area starts. A leaf that reads any other place is tied
to one layout version: the step leaf of `ground/step-in-code.ck` and `form/step.leaf.aarch64.ck`
reads a node's record, 32 bytes with whether it is a composite at byte 12, as version 20 lays it
out.

| Area | Records | Holds |
| --- | --- | --- |
| nodes | node | every node: blueprint, writer, where its content id lies once asked for, whether it is a composite, whether it is settled, its child or byte count and where its payload is; the table lies in the process's lane of the substrate |
| leaf-bytes, children | bytes, words | the payloads of leaves and composites, in the lane |
| by-content, by-id | words | the working index: the two indexes over the nodes not yet settled; they can be rebuilt from the nodes |
| frames | frame | the walker's continuations |
| links | link | values gathered for an operation that takes several |
| activations, arguments | activation, words | each running recipe and its arguments; for an implementation, the limit it runs under, which counts the program's own bindings and those made from outside, and the activation and cut scope of whoever used it |
| choice-points | choice-point | each open choice and the marks it returns to, among them how many calls were held back |
| gathered | gathered | answers collected by `all` |
| born-code | code | which code leaves have a runnable copy, and where |
| bindings | binding | which entries a program has given a new implementation, oldest first, each naming the one before it for the same entry, and the code that is its faster form when it has one |
| bound-by-node | words | for each node, its newest binding; it can be rebuilt from the bindings |
| outside | binding | what was bound from outside the run: seen by every layer of the program, and not undone by a fail |
| cells | words | for each node used as a cell, the node it holds |
| trail | trail | while a choice is open, what a cell held before each change, so a fail puts it back; and, kept, each note, faster form and born code, so a fail leaves them and keeps what they hold |
| reader-opened, gathering-children, gathering-bytes, print, print-numbers, text | words, bytes | working lists of the reader and the printer, and every file read |
| stack | bytes | the machine stack of the C code and of code leaves |
| asked, answered | bytes | the print of a node another process asks the run to evaluate, and the print of its outcome |
| after | words | calls held back until every open choice is closed: for each, how many words follow, the recipe and its values |
| noted | words | notes written by code that has not answered yet: a cell and what it will hold, in pairs |
| rooms | bytes | rooms: each a head, its blueprint, the node that last stored into it, how many stores it has had and its size, then its bytes, changed in place (`spec/core-operations.md`, entries 113 to 117) |
| room-trail | room-trail | while a choice is open, the four bytes of a room as they were before each store, and its writer and count, so a fail puts them back |
| gives | words | how many times a fail gave nodes back, and the choice, the mark and the top of the last time |
| ids | bytes | each content id once asked for: the node's handle, then the id's 32 bytes; when a fail gives nodes back, the ids of those that stay move down with them; in the lane |
| values | words | the values in hand: those of a node's children while its operation runs, and what a call is given; all given back at the walker's next step |
| seen | words | for each lane, how far it was settled when the oldest open choice was made |
| touched | bytes | a bit for each page of cells, bound-by-node and print-numbers, set when the page is first used |

## The substrate

A node's handle is a place in a node table that lies in a **substrate**: a part of the region,
after the process's own areas at the offset `share` its head records, that is laid out to be one
object several processes could map. A process alone makes
its own, with one **lane**, lane 0, and its nodes, their parts and their ids lie in that lane; the
process's area entries for those four tables point there, so the head's byte 160 still says where
the nodes are. The substrate begins with its own head: a magic word, the layout version, how many
areas and lanes it has, its maker's process id, `share`, its size, and how many places its index
by id holds; then one area entry per table: lanes, nodes, leaf-bytes, children, ids, by-content
and by-id. Its tables divided into lanes keep their counts in the process's own entries.

- **Settled and working.** A lane's row holds its process's id and `sw`: every node below `sw` is
  settled, and its record's `settled` byte is 1. A node made while no choice is open, when no
  working node stands, is settled as it is made. Any other is working: it is found by the process's
  own by-content and by-id, and a fail gives it back as it always did. A settled node lies below
  every choice point's mark, so no fail reaches it, and the substrate's by-content and by-id find it.
  That holds because no choice opens again once closed: a cut stops at a running outcome's choice
  point, which the outcome's end fails into. A fail that would reach a settled node ends the
  process. The machine's `ids` counts the places taken in the working by-id, the substrate head's
  `ids` those in the settled one.
- **The commit.** At the top of each evaluation step of the walker, when no choice is open and
  working nodes stand, each is settled where it stands, the newest first, as the working index forgets it, and
  `sw` rises past them. Both settled indexes are made large enough before the first node is
  settled, so a `full` there leaves every node where it was found. In a process alone nothing else settles while working nodes stand, so no
  settled node can equal one; were one found, the walker's state would have to be moved onto it,
  which is not built, and the process ends saying so.
- **The snapshot.** When the first choice point opens, `seen` keeps each lane's `sw`.
- `fb.check` reads the substrate's head, its areas and the lane row through the description, and
  checks that `share` and the substrate's nodes area put the nodes where the head says. It runs
  inside a choice, so it also checks that `seen` holds the lane's `sw`, and that every node below
  `sw` has its `settled` byte at 1 and every other at 0.

## Rules

1. **Every record starts with two handles**: its blueprint, and the program node that last wrote it.
   A writer of nothing means the bootstrap's own reader wrote it, before any program ran.
2. **A record names its kind in the system's own numbers.** The kinds of the floor's records are
   catalogue entries 53 to 63, 72, 91, 113, 114, 126 and 127 (`spec/core-operations.md`), so a reader needs no table that
   is not itself in the catalogue.
3. **A reference is a handle or an offset from the region's first byte, never an address.** A copy
   of the region reads the same in another process and at a later time.
4. **A record is complete before the count that makes it reachable moves past it.** A reader that
   takes the counts first never sees a record half made. A count moves down before the records it
   leaves are changed: when a fail gives nodes back, the nodes count drops to the mark, what stays
   moves down, and the count rises past it. A place in the nodes can then hold another node, so a
   reader that keeps nodes by handle reads the count in `gives` and keeps them under it. Two
   working areas, `text` and `values`, take their room by moving the count first and are written
   after: what lies under their counts is whole only between two steps.
5. **The writer does nothing for a reader.** It makes no call, takes no lock and waits for no one.
   It cannot tell whether anyone is reading. What it does look at, once a step, is whether
   someone has asked it something (below).

The walker's registers are fields of the machine record: its mode, the node being evaluated, the
value in hand, the reason, the running activation, the continuation, and `at`, the program node now
running. `at` is the writer stamped on whatever is made next. `step` counts the walker's steps and
serves as the region's clock.

## The region describes itself

What each record looks like is a node, `region/layout.ck`: for the head, the machine, an area
entry and each kind of record, its fields in order with their widths and whether each holds a
handle, a number or bytes; and the areas by name in the order of the table. `bootstrap/fb.h` is
made from it and from the lexicon (`lens/words.ck`) by a program in form, `region/header.ck`:
`make header` writes it, and `make check-full` compares the stored header with what the nodes generate. So
the catalogue's numbers and the records' layouts are said once, in nodes, and the C follows.

A program reads its own region with `peek` (`spec/core-operations.md`, entries 94 to 96), and
`region/region.ck`, written in form, reads it through the description: `(fb.get "area" "n"
base)` is a field of a record, `(fb.show)` the text of a view of the region with every binding
in words, and `(fb.check)` the list of places where the description and the running region
disagree: the size of each kind of record against the size its area states, the count of areas,
the three places code relies on, the substrate's head and areas, the lane row, and where `share`
puts the nodes. `ground/region-describes-itself.ck` runs that check; a description with a field
widened or two fields changed over is reported.

The same program reads another process's region. `look` maps it to be read only, and
`region/watch.ck` shows it: every handle it meets is a place in the other process's node table,
so the node is built again here from its record, its blueprint first, and what is shown is the
node and not the number. The address of the mapping is the machine's `watch`, the one address
kept in the region beside where code pages lie.

## Where a run spends its time

The walker's registers are in the region, so looking at them at intervals is a profile.
`region/profile.ck`, written in form, looks at another process's region once a millisecond until
it ends, at most two thousand times, and counts two things: the node the walker was at, and the
recipe of the activation that was running. It then shows the most frequent of each, in the lens's
words and with the names the recipes' arguments have:

```bash
printf /ck.demo | ./ck text region/profile.ck
```

The run that is looked at does nothing for it. A look is a sample: what is met often is seen,
what is met rarely is not.

## Asking a running program

A second process can ask a running program to evaluate a node. It writes the node's print in the
area `asked` and then that area's count; between two steps the walker sees the count, evaluates
the node, writes the outcome's print in the area `answered` with its count, and sets the count
of `asked` back to nothing. The run goes on where it was: the node it was about to evaluate, its
activation, its cut scope and the reason in hand are kept in a frame and taken up again.

The node is evaluated as `(outcome (outside x))` (`spec/core-operations.md`, entries 82 and
101), with no arguments. So a request can read any cell and call anything the program has
bound; what it binds is bound from outside the run, seen at every layer and kept; and nothing
else it did stays, whether it answered, failed or faulted. A print that cannot be read is the
fault `syntax`. The answer is what `outcome` gives: `(list "node" v)`, `(list "nothing" reason)`
or `(list "fault" reason)`.

`region/ask.ck`, written in form, is the asker. Its first line of input is the region's name and
the rest a node in the lens's words:

```bash
printf '/ck.demo\n(bind add (lazy (seq (note #1002 (add 1 (if (held #1002) (held #1002) 0))) (base $0))))' | ./ck text region/ask.ck
```

puts a counter on every addition the program makes from then on, `(held #1002)` reads it while
the program runs, and `(unbind add)` takes it off. The asker maps the region through the host's
own `shm_open` and `mmap`, called by name, writes with `memcpy`, and waits by looking at the
count once a millisecond.

A request can load a module. Asked

```
(seq (step (read (file "form/compile.aarch64.ck"))) (step (read (file "form/heat.ck"))) (heat.watch))
```

a running program loads the emitter and the watcher from outside, and the watcher then compiles
what runs hot. A recursion that takes 14.3 s on the walker, asked this one second after it
began, ended at 1.3 s with the same answer.

The looking and the asking go together in `region/warm.ck`:

```bash
printf /ck.demo | ./ck text region/warm.ck
```

It asks the run to load the emitter, looks at it three hundred times, and asks it to compile the
three recipes it was in most, saying what became of each. Where a recipe is what an entry is
bound to, it asks for the entry by name, so the code becomes that binding's faster form and
every definition that calls the entry runs it. Nothing is put on the run's calls: what
is hot is seen from outside, and the run is only asked to do what the looking found. The same
recursion, looked at from half a second in, ended at 1.3 s.

## Frames, recording and replay

A **frame** is a copy of the head page and of the used part of every area, taken by a second
process that has mapped the region read-only.

- `fb show NAME` takes one frame of a running or kept region and prints it: the machine, and for
  each area how many records are in use, their bytes, and the most it has held.
- `fb record NAME FILE MS` takes a frame every MS milliseconds until the writer is done. The file
  keeps, for each frame, only the 16 KiB pages that differ from the frame before.
- `fb play FILE` prints one line per frame; `fb play FILE N` prints frame N in full; with a handle
  added it prints that node and the program node that wrote it.

A frame taken while the writer runs is marked `moving`: the step counter changed during the copy,
so its parts may belong to slightly different moments. The counts were read first, so every record
in it was complete when its count was read.

## Observed on the first machine

- 2026-11-27, version 20 against version 19, in three alternating rounds under `/usr/bin/time -l`,
  medians: the long recursion took 1.07 s of CPU against 1.03 s (instructions +4.9%, cycles
  +3.6%), forty thousand outcomes given back 1.39 s against 1.35 s (+3.6%, +3.1%), and the
  compiled sweep of 37,822 cases 9.83 s against 8.59 s (+5.7%, +14.6%). A key below the first
  page not yet used is read without asking its page bit, which took back most of the sweep's
  added instructions but few of its cycles; where those cycles go is not yet known. The working
  index keeps its largest size beside the settled one, so the outcomes' peak footprint is 5.67 MB
  against 4.97 MB.
- 2026-11-27, version 18 against version 17, in seven alternating rounds under `/usr/bin/time -l`,
  medians: the long recursion took 0.99 s of CPU against 1.01 s (cycles −1.6%, instructions
  +0.8%), three million steps of a choice 0.78 s against 0.83 s (cycles −5.9%), a million calls of
  a closure 0.23 s against 0.24 s (cycles −3.2%), and a loop of seven arguments 0.23 s against
  0.24 s (cycles −4.4%). The values in hand cost those instructions; each area beginning on its own
  set of the data cache gives back more cycles than they take, where with every area on a page the
  same walker took up to 11% more cycles.
- 2026-11-27, version 17 against version 16, which had fixed capacities, in alternating runs under
  `/usr/bin/time -l`: the region of 2.28 TiB is reserved in 1 to 6 µs, as the one of 2.5 GiB was.
  The long recursion in `ground/long-recursion.ck` (158.6 million walker steps) took 0.99 s of CPU
  against 0.97 s, with 1.2% more instructions, most of them the 64-bit hash's; its largest resident
  set was the same, and its page tables grew from 433 KiB to 673 KiB (`footprint`).
- 2026-11-25: recorded by a second process every 5 ms the long recursion took 0.71 to 0.76 s in
  three runs, and every 1 ms 0.72 to 0.75 s in five. The 1 ms recording held 481 frames.
- 2026-11-25: a recording of 117 frames was replayed. Its last frame holds the answer 2178309 and
  names the `add` node of the recipe as its writer.
- 2026-11-25: a build of the same walker with its registers in C locals and its tables on a private
  heap ran the long recursion in 0.56 s in two of three runs, where the region's took 0.71 s.

## Limits

- **The layout has a cost of its own.** Keeping the walker's registers in memory and stamping a
  writer on each record costs this run about a quarter of its speed (0.71 s against 0.56 s with
  registers in C locals and private tables). Reading costs the writer no instruction, but a reader that copies a page pulls
  its cache lines away from the writer; at 1 ms that was measured as up to about 4%.
- **Machine registers are not memory.** A value held only in a register of compiled code is seen
  once it is stored. The walker keeps its registers in the region for this reason.
- **Executable pages are outside.** macOS refused a shared executable mapping (`EINVAL`, tested).
  A code leaf's bytes are a node in the region and its runnable copy is in a private mapping; the
  region records the leaf, the offset of its copy and the address of that mapping.
- **The C library's own memory is outside**: what the dynamic loader and the thread library keep.
  The process's arguments and its first thread's stack are outside until the move onto the
  region's stack.
- **A frame is a sample.** What a cell held between two frames is not kept, apart from its last
  writer. The journal that undo needs (stage 5) will keep old values inside the region.
- **The stack is recorded as raw bytes**, its newest 256 KiB, and the observer does not yet read it.
- **The allowance covers runtime-managed pages.** The live ground process runs with a 32 MiB
  allowance. It catches a room request that fits the room table's address range but exceeds the
  shared allowance, accepts the observer's binding, and completes its recursion. Multiple
  processes sample the same host independently. Raw host calls and other processes own their
  own allocations.

- **What is still counted by 32 bits or fewer**: a node's handle, so at most 2^32 − 1 nodes; the
  indexes of frames, links, activations, arguments, choice points, gathered values, born code,
  trails, room trails, held-back calls, notes, values in hand, and the nodes the reader opened and
  the children it gathers, so a composite has at most 2^32 − 1 children and a call as many values; the limit word, 2^23 − 1 of the program's
  own bindings and 127 from outside; the offset of born code, 4 GiB of it; and the floor, pages of
  16 KiB, so the stack lies within the region's first 64 TiB. The native stack is sized from
  host thread and stack limits and occupies at most half of the run's allowance.
- The runtime, live reader and observer were exercised together on macOS arm64.
- **One lane.** A handle is still local to its process: the substrate is inside the process's own
  region and nobody else maps it. Joining a substrate of another process, the shared allowance, the
  commit's path for content another lane settled, and the side front are later stages.
- **One asker at a time.** Two processes asking at once can overwrite each other's request. The
  asker waits by looking, not by an event. A request is answered between two steps of the walker,
  so code that runs long without returning to it, emitted or the host's, delays the answer; a
  program that has ended answers nothing, and the asker gives up after five seconds.
- **Asking is not gated.** Any process that can map the region can ask it anything a program can
  do. Only the user that made the region can map it: it is created with mode 0600.
