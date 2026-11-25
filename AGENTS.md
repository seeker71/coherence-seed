# Starting a session in coherence-seed

Everything a session needs is in this repository, and it builds with a C compiler and `make`.

## Ground first

1. Read [`NORTH_STAR.md`](NORTH_STAR.md) for the direction and [`FLOOR.md`](FLOOR.md) for what
   stands and what the owner has given.
2. Build and look at the present state before asserting anything about it:

   ```bash
   make && cc -O2 -o kg tools/kg.c && make check && ./kg check
   ```

3. Read the design you are about to touch: `./kg brief | grep '^5\.'` for the design, `'^7\.'` for
   the path, `'^6\.'` for what is open. `./kg link N` shows where a node's sources can be read.

## The owner's word

What the owner has given is listed in `FLOOR.md` and recorded in the graph as `status: given` with
`by: owner`. It is not a proposal; a design that contradicts it is wrong. When the owner answers a
question or gives a direction, put the words verbatim in the node they decide, and write your own
reading in a separate node with `status: inferred`.

## How to work here

- **Only what is.** Every document and node says what is true now. Do not write what changed, what
  was corrected, what came before, who did which round, or a list of updates: git holds that. When
  something stops being true, change it in place or take it out. The tree always reads as if it
  had been written today on a clean sheet.
- **Only what is needed.** The repository holds what closes the gap from the floor to the north
  star: the code, its specifications, the design, what is open, and the evidence and sources the
  design refers to. Notes, reports, closed questions and measurements nothing refers to are taken
  out; `./kg release` does this for the graph.
- **Two living pages.** `NORTH_STAR.md` is the direction. `FLOOR.md` is what stands, with the date
  it was observed; when you observe again, replace its content and its date.
- **Follow a real flow.** Pick work that makes something run end to end and be watched through
  the framebuffer (`make watch`, `./fb show`, `./fb record`, `./fb play`). A check, a table or a
  page comes after the run it describes.
- **The framebuffer is the memory.** Anything the runtime keeps goes in the shared region with its
  blueprint and its writer (`spec/framebuffer.md`, `bootstrap/fb.h`). Do not add a private heap, a
  log, a counter or an event channel for an observer: the observer reads the memory.
- **One movement.** A change to behaviour changes the code, its specification under `spec/`, the
  graph nodes that describe it, and `FLOOR.md` together. Run `./kg release` after rewording
  claims; `./kg check` and `make check` both pass before a commit.
- **Say how each claim is known.** An observation names a source and what was run; an inferred
  claim names its premises; a limit says what was not tested. A check counts once a planted fault
  has turned it red.
- **New code is nodes.** Only the bootstrap is C (node 7.13). The C files under `tools/` stand
  until generated code replaces them; add no other. When a program cannot be written as nodes the
  walker runs, the missing operation goes into the bootstrap and the program is written on it.
- **No committed limits.** A number in code is the size of a run or a starting size that grows. A
  capacity that stops a run is a limit, and is named in `FLOOR.md` (node 7.14).
- **Keep the floor small.** The bootstrap has a 2,000-line budget the build enforces. Only what
  cannot be derived again is committed; build outputs are not.
- **Nothing outside is a tool.** No other kernel, runtime or repository is run, linked or imported
  to build or check this one. What was read elsewhere is evidence, held in hubs 2 to 4.
- **Check the author before a push.** `git log --format=%ae | sort -u` must show only the address
  this repository's commits already carry.
- **Plain words.** Write nodes and pages so that someone who has not seen the session can follow
  them; a statement over 600 bytes is split.
