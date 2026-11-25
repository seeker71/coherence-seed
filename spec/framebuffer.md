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

| Area | Records | Holds |
| --- | --- | --- |
| nodes | node | every node: blueprint, writer, child or byte count, where its payload is, its content id once asked for |
| leaf-bytes, children | bytes, words | the payloads of leaves and composites |
| by-content, by-id | words | the two indexes over nodes; they can be rebuilt from the nodes |
| frames | frame | the walker's continuations |
| links | link | values gathered for an operation that takes several |
| activations, arguments | activation, words | each running recipe and its arguments; for an implementation, how many bindings it runs on and the activation and cut scope of whoever used it |
| choice-points | choice-point | each open choice and the marks it returns to |
| gathered | gathered | answers collected by `all` |
| born-code | code | which code leaves have a runnable copy, and where |
| bindings | binding | which entries a program has given a new implementation, oldest first, each naming the one before it for the same entry |
| bound-by-node | words | for each node, its newest binding; it can be rebuilt from the bindings |
| reader-opened, gathering-children, gathering-bytes, print, print-numbers, frozen-marks, text | words, bytes | working lists of the reader, the printer and freeze, and every file read |
| stack | bytes | the machine stack of the C code |

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
   It cannot tell whether anyone is reading.

The walker's registers are fields of the machine record: its mode, the node being evaluated, the
value in hand, the reason, the running activation, the continuation, and `at`, the program node now
running. `at` is the writer stamped on whatever is made next. `step` counts the walker's steps and
serves as the region's clock.

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
- **Areas have fixed sizes.** A run that outgrows one stops with the area's name. A process that is
  killed leaves its region behind until `fb drop NAME`.
- One hand wrote the layout, the writer and the observer, and only this Mac ran them.
