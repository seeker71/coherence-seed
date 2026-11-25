# Starting a session in coherence-seed

This repository is a restart of the coherence-kernel from its five axioms. It stands on its own:
everything a session needs is here, and it builds with a C compiler and `make`.

## Ground first

1. Read [`README.md`](README.md).
2. Build and look at the present state before asserting anything about it:

   ```bash
   make && cc -O2 -o kg tools/kg.c && make check && ./kg check
   ```

3. Read the design you are about to touch: `./kg brief | grep '^5\.'` for the design,
   `'^7\.'` for the path, `'^6\.'` for what is open. `./kg link N` shows where a node's sources
   can be read.

## What the owner has given

These are the owner's own words, recorded as `status: given` with `by: owner`. They are not
proposals; a design that contradicts one is wrong.

| Node | Given |
| --- | --- |
| 1 | only the five axioms in `axioms/core-axioms.form` are fixed |
| 5.4.2 | fail means backtrack, undo and reverse all changes until a new choice or a sequence is tried; an offer whose every option failed receives nothing |
| 5.5.4 | several parents mean one composed shape |
| 5.6.2 | ice is the small essential part that must persist, water a held working form, gas transient and derivable; each is a state of any node |
| 5.7.6, 5.7.7 | the framebuffer: all memory of every object, node and stack lives in one shared space that is recorded like video at no cost to the runtime and played back; the blueprint sits next to each object, with a pointer to who last wrote it |
| 7.10 | doors, gates, receipts and bands were a distraction; follow real end-to-end flows, watched live |
| 7.11 | this is a new repository; the old kernel and its runtime are not used to build it |
| 8.17 | an organisation that no longer exists is not used or named here; where NUMS.Go's own identifiers carried its name, the text writes `[org]` |

When the owner answers a question or gives a direction, record the words verbatim in the node they
decide, with `status: given` and `by: owner`, and write your own reading in a separate node with
`status: inferred`.

## How to work here

- **Follow a real flow.** Pick work that makes something run end to end and be watched through
  the framebuffer (`make watch`, `./fb show`, `./fb record`, `./fb play`). A check, a table or a
  written record comes after the run it describes.
- **The framebuffer is the memory.** Anything the runtime keeps goes in the shared region with its
  blueprint and its writer (`spec/framebuffer.md`, `bootstrap/fb.h`). Do not add a private heap, a
  log, a counter or an event channel for an observer: the observer reads the memory.
- **One movement.** A change to behaviour changes the code, its specification under `spec/`, the
  graph nodes that describe it and the README together. `./kg check` and `make check` both pass
  before a commit.
- **Say how each claim is known.** An observation names a source and what was run; an inferred
  claim names its premises; a limit says what was not tested. A check counts once a planted fault
  has turned it red.
- **Keep the floor small.** The bootstrap has a 2,000-line budget the build enforces. Only what
  cannot be derived again is committed; build outputs are not.
- **The old body is evidence.** Hubs 2 and 3 record what was read there. Nothing from it is run,
  linked or imported, and its vocabulary of doors, gates, receipts and bands is not carried.
- **Plain words.** Write nodes and pages so that someone who has not seen the session can follow
  them; a statement over 600 bytes is split.

## Where it stands

Stage 2, the floor, stands on the framebuffer (graph hub 5.14, round 9.11). Next is stage 3 (7.4):
the home lens as rows written in the grammar. Still unwritten on the floor: the host functions for
symbol lookup, the framework trampoline, threads and the clock (5.14.9). Nothing has been reviewed
by a second hand since review round one, and hubs 1, 5, 6, 7 and 8 were never reviewed (9.7).

Open questions the owner has not decided are the nodes with `status: open` under 6.1.
