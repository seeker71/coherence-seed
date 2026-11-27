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
- Set `CK_SHARE` to a name and the process is a member of the substrate of that name (below). It
  reserves 16 TiB of addresses once, keeps its own region at their start and maps the substrate at
  byte 2^43 of them, as every member does. `CK_SHARE_BYTES` is that substrate's allowance, set by
  whoever makes it.
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

The first page is the **head**: a magic word, the layout version (22), the writer's process id, the
walker's registers (the **machine**), one **area** entry per table, the doors, and the **member** part: where
the substrate lies (`share`), how many pages the tables keyed by handle have charged, the
process's lane, and the name of the substrate it is a member of, empty for a process alone. An area entry says what its records are, how large one is, the most it has held,
where the table starts, how many fit and how many are in use; the last four are counted by 64 bits.

Machine code reads the region too, and three places in the head are kept for it whatever else
moves: at byte 28 the **floor**, the lowest page the machine stack may reach, counted in pages of
16,384 bytes from the region's start; at byte 128 the machine's `serve`, the address code calls
to have the bootstrap do an operation on values in hand (`spec/core-operations.md`, Native
code); and at byte 160 where the nodes area starts. After the last area entry lie the **doors**:
addresses the host gave this process, for code that has no walker to ask. The first is `symbol`,
the address of the host's lookup of a name (`dlsym`). They come after the area table so that the
three places above do not move when a door is added; a reader finds them at the end of the area
table, by the layout's rows, and the member part after them. A leaf that reads any other place is tied
to one layout version: the step leaf of `ground/step-in-code.ck` and `form/step.leaf.aarch64.ck`
reads a node's record, 32 bytes with whether it is a composite at byte 12, as version 22 lays it
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
| seen | words | for each lane, how far it was settled when the oldest open choice was made: what a search sees of it |
| touched | bytes | a bit for each page of cells, bound-by-node and print-numbers, set when the page is first used |

## The substrate

A node's handle is a place in a node table that lies in a **substrate**. A process alone makes its
own, a part of its region after its own areas, with one **lane**. With `CK_SHARE=name` the
substrate is an object of that name which every **member** maps at the same offset `share`, 2^43,
of the addresses it reserved, so a place in it is the same offset from each member's first byte
and rule 3 holds across members. A named substrate has 256 lanes that divide the handles between
them: lane k holds the handles from k·2^24 up to (k+1)·2^24, and a handle names the same node in
every member. Lane 0 belongs to whoever made the substrate and holds nothing at handle 0 and the
seeded catalogue; a process that joins seeds nothing.

A member's nodes, their parts and their ids lie in its lane. Its own area entries for those four
tables point at the substrate's tables, with its lane's part as their count and their room, so the
head's byte 160 still says where node h lies: at that offset plus 32·h, in every member. The
substrate begins with its own head: a magic word, the layout version, how many areas and lanes it
has, its maker's process id and the instant the host started that process, the instant it was made,
`share`, its size, its allowance and how much of it is charged, and `buried`; then one area entry
per table: lanes, nodes, leaf-bytes, children, ids, by-content and by-id.

- **A lane's row** is 64 bytes: `who`, the member's process id in its upper half and below it 1
  claimed, 2 live or 3 gone; `start`, the instant the host started that process; `n`, how far the
  lane's records are made, stored with release as the member makes a node or a fail gives nodes
  back; `sw`; what the lane has charged; and how many of its nodes a commit forwarded.
- **Settled and working.** Every node of a lane below its `sw` was settled there or forwarded, and
  a settled record's `settled` byte is 1. A node made while no choice is open, when no working node
  stands, is settled as it is made. Any other is working: it is found by the process's own
  by-content and by-id, no other member can hold it, and a fail gives it back as it always did. A
  settled node lies below every choice point's mark, so no fail reaches it, and the substrate's
  by-content and by-id find it. That holds because no choice opens again once closed: a cut stops
  at a running outcome's choice point, which the outcome's end fails into. A fail that would reach
  a settled node ends the process.
- **Entering the index.** In a named substrate the two indexes are made once, for twice the nodes
  the allowance can hold, and never grow. A settled record is written whole above the lane's
  count, its `settled` byte first, and then one compare-and-swap puts its handle in an empty place
  of by-content. There is no lock and no place marked as being filled. A swap that finds the place
  taken looks at what took it: equal content answers instead, and the record just written is
  simply not counted, since nobody saw it; other content sends the probe on. Then `n` and `sw` rise
  past the record, stored with release. An index place is loaded as a whole, in sequence with the
  swaps. A node's id is a record appended whole to the lane of whoever asked for it, given to the
  node by one swap of its `ida`, and then found by one swap into by-id; whoever asks later makes
  sure of that last step, so a member that died between the two leaves nothing unfound.
- **The commit.** When no choice is open and working nodes stand, they are settled where they
  stand. A process alone does this at the top of the next evaluation step, the newest first as the
  working index forgets them, and ends if it finds equal content already settled, which nothing in
  it can cause. A member does it at once where its last choice closes, and oldest first, because
  another member may have settled equal content since the search began: such a node is forwarded
  to the settled one. Its record stays where it is, unsettled and found by nothing; the nodes made
  after it have their blueprint, writer and children forwarded before they are settled in turn;
  its cell, its bindings and its id go to the settled node; and everything the run holds is
  forwarded: the machine's registers, frames, links, activations, arguments, what was gathered,
  bindings, born code, held-back calls, the heads of rooms and every cell. The lane's row counts
  the nodes forwarded.
- **What a search sees.** While a choice is open or working nodes stand, a member sees of another
  lane only what lay below that lane's `sw` when its oldest choice opened: `seen` keeps each lane's
  `sw` from that moment. A settled node of another lane beyond it is, for the maker of nodes and
  for finding by id, not there, so the search makes its own working node and the commit forwards
  it; `at` on its number is the fault `irreversible`. So going an option again meets the same
  nodes, and no run holds two handles for one content. A node in the index is settled together
  with everything below it in its lane, since a lane publishes in order. So whoever takes a node
  of another lane from the index outside a search first raises that lane's `sw` past it, by
  compare-and-swap, when the lane's member has not yet done so itself. A member killed between
  publishing a node and raising `sw` is finished that way by whoever next takes the node, and what
  a published node names is always below its lane's `sw`.
- **Members.** A member is a process id and the instant the host started that process. It joins
  by claiming the first free row with one compare-and-swap that writes its process id, stores its
  start and marks the row live. Each process that joins, and each that leaves, looks at every row:
  a row whose process is gone, or was started at another instant, is retired by compare-and-swap,
  once. A member that exits retires its own row. When no row has a member the substrate ends: one
  process writes itself in `buried` by compare-and-swap, looks at the rows once more, and unlinks
  the name, or takes `buried` back when someone joined meanwhile; a process that claimed a row
  and then finds `buried` set gives the row up and joins again. So after every member was killed
  the object stays until the next process opens it, buries it and makes a new one. A process waits
  for nobody: a substrate whose maker has not finished seeding it is tried again for at most ten
  seconds, and buried if its maker is gone.
- **The allowance.** `CK_SHARE_BYTES`, or else what the host could spare when the substrate was
  made, is in its head and never changes. The two indexes and the head are charged in full when it
  is made, and each lane's first pages when it is claimed. A lane's growth is charged by whole
  pages, added to the head's count and the lane's with a fetch-and-add; a growth past the allowance
  is taken back and is the fault `full` with the area's name, and the host is asked again at each
  MiB step as for any area. A member's own allowance (`CK_MEMORY_BYTES`) no longer counts those
  four tables. Nothing a lane charged is given back while the substrate lives.
- `fb.check` reads the substrate's head, its areas and the member's own lane row through the
  description, and checks that `share` and the substrate's nodes area put the nodes where the head
  says. It runs inside a choice, so it also checks that `seen` holds the lane's `sw`, that no node
  above `sw` has its `settled` byte set, and, in a process alone, that every node below has.
  `fb.one-handle` walks every lane that was ever claimed, makes each settled node again from its
  blueprint and its parts, and answers how many it made and which were not answered their own
  handle: none, when each content has one handle.

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
  each area how many records are in use, their bytes, and the most it has held. For a member of a
  named substrate it maps that object too, reads the nodes in place, the member's own and what any
  lane has settled, and prints the substrate as `fb share` does.
- `fb share NAME` maps a named substrate alone and prints its head and each lane that was ever
  claimed: its state, its process and whether that runs, its nodes, how far they are settled, how
  many a commit forwarded and what it charged.
- `fb record NAME FILE MS` takes a frame every MS milliseconds until the writer is done. The file
  keeps, for each frame, only the 16 KiB pages that differ from the frame before.
- `fb play FILE` prints one line per frame; `fb play FILE N` prints frame N in full; with a handle
  added it prints that node and the program node that wrote it.

A frame taken while the writer runs is marked `moving`: the step counter changed during the copy,
so its parts may belong to slightly different moments. The counts were read first, so every record
in it was complete when its count was read.

## Observed on the first machine

- 2026-11-27, version 22 with the named substrate against version 21 (the build of 01db529), a
  process alone, in alternating rounds under `/usr/bin/time -l`, one process at a time, medians
  of seven, seven and three rounds: the long recursion 27.427 G instructions against 27.423 G
  (+0.01%) and 3.865 G cycles against 3.848 G (+0.43%); forty thousand outcomes given back 31.544 G
  against 31.485 G (+0.19%) and 5.076 G against 4.971 G (+2.12%; least of the rounds +1.42%); the
  compiled sweep 209.69 G against 209.54 G (+0.07%) and 34.32 G against 34.82 G (−1.44%). Getting
  there took three findings, each by building one change at a time. A commit at the top of every
  answer step cost the long recursion 1.6% of its instructions, so only a member commits where its
  last choice closes. One maker of nodes that tests whether the substrate is named cost 0.8%: it is
  called so often that a few instructions in it show, and carrying the place of a refusal across
  its exit to the member's maker alone cost 0.5%. A process alone now runs a copy of the maker with
  the test folded away, which a member leaves at a miss in a settled index of one empty place.
  Where the compiler puts the walker's loop moves the cycles by two to four percent between builds
  with equal instruction counts.
- The same day, two members making the same content at once, `ground/region-describes-itself.ck`,
  medians of five rounds: each took 6.28 G instructions and 1.01 G cycles; a process alone 6.88 G
  and 1.04 G; one member with nobody beside it 7.26 G and 1.20 G. A member alone pays for the
  commit's forwarding table, the index made once and the charge; two together make each node once
  between them, and 21,675 of their nodes were forwarded in the last round.
- The same day, what this Mac allows a member (two probes written and run in the session, not
  kept in the repository): reservations of 1 to 64 TiB with no access, 16 TiB anonymous and shared for reading
  and writing, a sparse object of 16 TiB, a 3 TiB object mapped at a fixed place 2^43 bytes into
  the reserved range, and a second process with another base that saw the first's bytes at the
  same offset and won a compare-and-swap there.
- 2026-11-27, version 21 against the bootstrap without the substrate (version 20), in three
  alternating rounds under `/usr/bin/time -l`, one process at a time, medians of cycles: the long
  recursion 3.85 G against 4.21 G (−8.6%; instructions −6.5%), forty thousand outcomes given back
  5.23 G against 5.51 G (−5.1%; −4.7%), and the compiled sweep of 37,822 cases 34.6 G against
  35.0 G (−1.2%; −2.0%). As first laid out the substrate took 4.45 G, 5.73 G and 40.4 G (+5.7%,
  +4.1%, +15.5%). Where those cycles went was found by building one change at a time and by
  sampling both programs every millisecond, and it was three choices of the compiler:
  - `room`, asked at every growth of a table, was a call. It now lies in the walker's loop. This
    gain does not come from the substrate: the same change alone gives the version without it
    −6.7%, −6.1% and −8.2% cycles.
  - The commit lay inside the walker's loop. It is now a call, made only when nodes settle.
  - The page-bit test lay inside the lookup of a binding, and the lookup then no longer lay in
    the walker's loop. The test is now a call, made only for a key past the dense prefix.

  What the substrate itself still costs is seen against version 20 with the same `room`: the long
  recursion +3.5% instructions and −2.0% cycles, the outcomes +4.2% and +1.1%, the sweep +7.3%
  and +7.6%. Of the sweep's instructions, 4.7 points go with the page-bit test, since the lookup
  of a binding is still a call from the walker's most used place (putting it in the loop gives
  back 3.6 points of instructions and no cycles); 1.1 with the second index and the test of
  what is settled, since of 81.7 million nodes made under a choice 52.7 million ask the settled
  index too and 44.3 million are found there; and 1.2 with the commit's test at each of 517
  million evaluation steps. Of its cycles, 2.5 points go with the second index and the commit's
  test and 1.4 with the page bits. Where the lane's areas start moves the sweep's cycles by up to
  3 points either way with the same instructions, so a smaller difference is not told apart from
  placement. The working index keeps its largest size beside the settled one: the peak footprint
  is 5.67 MB against 4.98 MB for the outcomes, 7.55 MB against 7.18 MB for the sweep and 1.93 MB
  against 1.87 MB for the long recursion.
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
  region records the leaf, the offset of its copy and the address of that mapping. On macOS
  a process uses one `MAP_JIT` reservation. Linux x86-64 publishes code by changing the touched
  pages from writable to executable. Linux reserves anonymous framebuffer addresses with
  `MAP_NORESERVE` and charges touched pages to the runtime's shared allowance.
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
- **A named substrate is not yet a channel.** Members share settled nodes and can name them by
  handle, but nothing carries a handle from one to another: shared rooms, `swap`, the inbox and the
  bell are not built, nor the side front. A lane is never used again, so a substrate has 256
  members in its life; its indexes never grow; and what a forwarded node occupied stays occupied.
  The numeric port does not answer by handle. A member is shown but not recorded. The counts of
  nodes settled in place and refused are not kept, only those forwarded. A commit that forwards
  charges the page of cells a forwarded cell moves onto without asking the allowance first. A
  process that dies while it buries a substrate is taken over by the next opener when its process
  id is gone; an id the host has given again meanwhile holds the name until `fb drop`.
- **Two statics lie outside the region**: where each keyed table's page bits begin and the key
  below which all its pages are used. Kept at the start of `touched` they cost the long recursion
  1.7% more instructions, measured, so they stay where they are.
- **One asker at a time.** Two processes asking at once can overwrite each other's request. The
  asker waits by looking, not by an event. A request is answered between two steps of the walker,
  so code that runs long without returning to it, emitted or the host's, delays the answer; a
  program that has ended answers nothing, and the asker gives up after five seconds.
- **Asking is not gated.** Any process that can map the region can ask it anything a program can
  do. Only the user that made the region can map it: it is created with mode 0600.
