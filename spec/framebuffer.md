# The framebuffer, floor version

The guide's definition (graph nodes 5.7.6 and 5.7.7): all memory of every object, node and stack
is read and written in one shared memory space, which can be recorded like video with no cost to
the runtime and played back to see what happened, when and where. The blueprint is stored next to
each object, and a record can name who last wrote it.

This page fixes how the bootstrap does that. `bootstrap/fb.h` is the layout in C, `bootstrap/ck.c`
is the only writer, and `tools/fb.c` is the observer.

## The region

- The bootstrap reserves one region when it starts and never moves or grows it. It is POSIX shared
  memory, named `/ck.<pid>`, or by the variable `CK_FB` when that is set.
- A region named by `CK_FB` is kept after the run ends, so its last state can be read afterwards.
  One named by pid is removed when the process ends.
- The bootstrap keeps no heap. Its tables, its working lists, the files it reads and its machine
  stack are all inside the region. It starts on the host's stack only long enough to move onto the
  region's.
- Pages are taken from the host as they are first touched, so reserving costs nothing: the region
  is about 2 GiB of addresses and a small run touches a few hundred kilobytes.

## What is in it

The first page is the **head**: a magic word, the layout version, the writer's process id, the
walker's registers (the **machine**), and one **area** entry per table. An area entry says what its
records are, how large one is, how many are in use, how many fit, and where the table starts.

Machine code reads the region too, and three places in the head are kept for it whatever else
moves: at byte 28 the **floor**, the lowest page the machine stack may reach, counted in pages of
16,384 bytes from the region's start; at byte 128 the machine's `serve`, the address code calls
to have the bootstrap do an operation on values in hand (`spec/core-operations.md`, Native
code); and at byte 160 where the nodes area starts. A leaf that reads any other place is tied
to one layout version.

| Area | Records | Holds |
| --- | --- | --- |
| nodes | node | every node: blueprint, writer, child or byte count, where its payload is, its content id once asked for |
| leaf-bytes, children | bytes, words | the payloads of leaves and composites |
| by-content, by-id | words | the two indexes over nodes; they can be rebuilt from the nodes |
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
| trail | trail | while a choice is open, what a cell held before each change, so a fail puts it back |
| reader-opened, gathering-children, gathering-bytes, print, print-numbers, text | words, bytes | working lists of the reader and the printer, and every file read |
| stack | bytes | the machine stack of the C code and of code leaves |
| asked, answered | bytes | the print of a node another process asks the run to evaluate, and the print of its outcome |
| after | words | calls held back until every open choice is closed: for each, how many words follow, the recipe and its values |

## Rules

1. **Every record starts with two handles**: its blueprint, and the program node that last wrote it.
   A writer of nothing means the bootstrap's own reader wrote it, before any program ran.
2. **A record names its kind in the system's own numbers.** The kinds of the floor's records are
   catalogue entries 53 to 63 and 72 (`spec/core-operations.md`), so a reader needs no table that
   is not itself in the catalogue.
3. **A reference is a handle or an offset from the region's first byte, never an address.** A copy
   of the region reads the same in another process and at a later time.
4. **A record is complete before the count that makes it reachable moves past it.** A reader that
   takes the counts first never sees a record half made.
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
`make header` writes it, and `make check` holds the stored header to what the nodes generate. So
the catalogue's numbers and the records' layouts are said once, in nodes, and the C follows.

A program reads its own region with `peek` (`spec/core-operations.md`, entries 94 to 96), and
`region/region.ck`, written in form, reads it through the description: `(fb.get "area" "n"
base)` is a field of a record, `(fb.show)` the text of a view of the region with every binding
in words, and `(fb.check)` the list of places where the description and the running region
disagree: the size of each kind of record against the size its area states, the count of areas,
and the three places code relies on. `ground/region-describes-itself.ck` runs that check; a
description with a field widened or two fields changed over is reported.

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

- `fb show NAME` takes one frame of a running or kept region and prints it.
- `fb record NAME FILE MS` takes a frame every MS milliseconds until the writer is done. The file
  keeps, for each frame, only the 16 KiB pages that differ from the frame before.
- `fb play FILE` prints one line per frame; `fb play FILE N` prints frame N in full; with a handle
  added it prints that node and the program node that wrote it.

A frame taken while the writer runs is marked `moving`: the step counter changed during the copy,
so its parts may belong to slightly different moments. The counts were read first, so every record
in it was complete when its count was read.

## Observed on the first machine, 2026-11-25

- A 2 GiB region is reserved in about 74 µs.
- The long recursion in `ground/long-recursion.ck` (158.6 million walker steps) took 0.71 to
  0.72 s alone in five runs.
- Recorded by a second process every 5 ms it took 0.71 to 0.76 s in three runs, and every 1 ms
  0.72 to 0.75 s in five. The 1 ms recording held 481 frames.
- A recording of 117 frames was replayed. Its last frame holds the answer 2178309 and names the
  `add` node of the recipe as its writer.
- A build of the same walker with its registers in C locals and its tables on a private heap ran
  the same program in 0.56 s in two of three runs.

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
- **Areas have fixed sizes.** A program that outgrows one has a fault, `full`, with the area's
  name; a `recover` that catches it gets the frames, values and activations back, and no nodes.
  A process that is killed leaves its region behind until `fb drop NAME`.
- One hand wrote the layout, the writer and the observer, and only this Mac ran them.
- **One asker at a time.** Two processes asking at once can overwrite each other's request. The
  asker waits by looking, not by an event. A request is answered between two steps of the walker,
  so code that runs long without returning to it, emitted or the host's, delays the answer; a
  program that has ended answers nothing, and the asker gives up after five seconds.
- **Asking is not gated.** Any process that can map the region can ask it anything a program can
  do. The region is the owner's alone, created with mode 0600.
