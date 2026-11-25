# Floor

What stands, as observed on **2026-11-25** on one Apple M4 Max running macOS.
[`NORTH_STAR.md`](NORTH_STAR.md) says where this is going.

## What runs

| Piece | Where | State |
| --- | --- | --- |
| Node encoding, version 1 | [`spec/node-encoding.md`](spec/node-encoding.md), 142 vectors | written, with seventeen refusals; the bootstrap's reader holds the vectors; a second reader is missing |
| Bootstrap | [`bootstrap/ck.c`](bootstrap/ck.c), [`bootstrap/fb.h`](bootstrap/fb.h) | 862 lines of a 2,000-line budget the build enforces |
| Framebuffer | [`spec/framebuffer.md`](spec/framebuffer.md) | all of the bootstrap's memory is one shared region; it keeps no heap |
| Walker | [`spec/core-operations.md`](spec/core-operations.md) | 29 operations; choice is re-entered after it has answered; a program takes a node apart and builds one, and gives an operation a new implementation while it runs; its registers are a record in the region |
| Operations in form | [`form/`](form) | add, sub, mul and lt, each as a recipe over the other operations and as AArch64 code; any of them takes the bootstrap's place |
| Programs kept as nodes | [`ground/`](ground) | 30, written in the canonical print; one is a rule of the encoding, one takes a node apart and builds it again, one brings a new operation into being |
| Native code | catalogue entry 52 | an AArch64 leaf runs unsigned and under the hardened runtime |
| Ice | `ck freeze`, `ck thaw` | a node frozen in one process thaws in another to the same id |
| Observer | [`tools/fb.c`](tools/fb.c) | shows a running region, records it as frames, replays a recording |
| Claim graph | [`graph/`](graph), [`tools/kg.c`](tools/kg.c) | 1,335 nodes: the ground, the design, what is open, the path, and only the evidence and sources they refer to; every source resolves |

## Observed

- `make check` prints five passing lines: the 142 vectors through the bootstrap's own reader, the
  30 ground programs, freeze and thaw across two processes, native code unsigned and hardened, and
  a live run recorded from a second process.
- Every check added on this date is a form program under `ground/` with its expected answer; no
  line was added to the Makefile or to the tools.
- `ground/on-form.ck` binds add, sub, mul and lt to their form recipes and runs a recursion and
  edge cases on them; `ground/on-code.ck` does the same with the machine-code versions. Both
  answer as the bootstrap's own operations do, and each has a case that leaves the 64-bit range.
- Run by hand with `./ck run F B`, any one of the eight files under `form/` in place of the
  bootstrap's own operation left 25 ground programs answering the same. The long recursion
  answers 2178309 on the four form versions at once in 9.3 s, on the four machine-code versions
  in 2.6 s, and on the bootstrap's own in 0.71 s.
- A program gives entry 100, which had no meaning, a recipe and uses it. A new `add` uses the
  `add` it replaces. A binding made in an option that fails is gone.
- The first machine-code `mul` wrapped on overflow where the page says nothing with `range`;
  running `ground/range.ck` on it showed that, and it now tests for overflow.
- `ground/nat-rule.ck` is the shortest-form rule of a `nat` as nodes. Given five candidates it
  answers the leaves 0, 42 and 256 and twice the word `minimal`. A second process reads its region:
  the leaf 256 is a record written by the node under entry 69, `pack`, and the answer by the
  `all` node. That reading was done by hand; no kept check repeats it.
- `ground/rebuild.ck` takes a nested node apart to its bytes and builds it again; the answer is
  the node it was given. Packing the bytes 01 00 under `nat` answers nothing with `minimal`: a
  program cannot build a node the reader would refuse.
- Each of those lines has been turned red by a planted fault: a wrong expected answer, a changed
  and a truncated ice file, a hardened binary without the JIT entitlement, a run too short to be
  seen in motion, and a 100-line budget. The ground line also by a byte read one too far, an
  index one past the end, a lost reason, a rule that looks at the first byte, an implementation
  that sees its own binding, a fail that leaves a binding, a machine-code leaf without its
  overflow test, and `bind` on a control entry.
- A wrong form recipe under a recursion gives no answer: the run goes on until an area of the
  region is full, minutes later, and only then is the check red.
- The bootstrap's reader starts with catalogue entries 0 to 72 and refuses bytes as `unknown` when
  their blueprint or a child is the id of no node it holds: a leaf under entry 73 is refused until
  entry 73 itself has been read.
- `make watch`: the long recursion (158.6 million walker steps) runs in 0.71 to 0.72 s. A second
  process records it every 5 ms, replays it, and reads from the last frame the answer 2178309 and
  the `add` node that wrote it.
- Recording every 1 ms slows that run by up to about 4%. Keeping the walker's registers and every
  record's writer in the region costs it about a quarter of its speed, measured against a build
  with registers in C locals.
- `./kg check`: 0 errors, 0 warnings over 1,157 claims. None is confirmed by an independent
  reading, and 64 pieces of evidence are marked assumed because no source for them is kept here.
- `./kg release` finds nothing to take out: every source and piece of evidence is referred to by
  the design, a question or a limit.

## Not there

- Every part replaceable (7.19). The ten control entries cannot be given a new implementation.
  `call`, `eq`, `native`, the seven operations on a node's parts and `bind` can be, and are not
  yet written in form. The store, the reader, the printer, freeze, thaw and the host functions
  are C with no binding (5.14.23).
- A compiler. The machine-code operations are leaves written by hand and run slower than the C;
  nothing yet turns a form recipe into machine code.
- Only form written and stored (7.20). Beside the bootstrap the tree still holds code that is not
  form: the graph checker and the observer in C, and the shell in the Makefile that builds and
  checks. It also holds what is not yet form nodes: these pages, the graph's blocks, the vectors
  and the expected answers. None gains a line; each goes when form does its work (7.15).
- A check written in form. A program cannot yet read a file, run another program and see its
  outcome, or read a region, so pairing every ground program with every file under `form/`, and
  reading a writer from the region, are done by hand.
- Names for entries 64 to 72 in the observer, and a listing of the bindings: it prints numbers.
- A second reader of the encoding. It is owed as nodes written from the page alone (7.2).
  Programs are written by hand in the canonical print: there is no home lens.
- An answer to what a node's id should be. Nobody asked for SHA-256 (question 6.1.31); as long as
  the id is that digest, a reader in form needs operations on bits the walker does not have.
- Memory that grows. The framebuffer's areas, the code region and the graph checker's tables have
  fixed capacities, and a run that outgrows one stops (7.16).
- The host functions for symbol lookup, the framework trampoline, thread start and the clock.
- A cell a program can change in place, and the journal that undoes it.
- Recipes that name each other.
- A second hand: one hand wrote the specifications, the bootstrap and the observer, and one machine
  built them. No claim in the graph is reviewed. Who counts as a second hand is question 6.1.30.
- Machine registers, executable pages and the C library's own memory lie outside the framebuffer.

## What the owner has given

Each is recorded word for word in the node named.

| Node | Given |
| --- | --- |
| 1 | only the five axioms are fixed |
| 5.4.2 | fail means backtrack, undo and reverse all changes until a new choice or a sequence is tried; an offer whose every option failed receives nothing |
| 5.5.4 | several parents mean one composed shape |
| 5.6.2 | ice, water and gas are states of any node |
| 5.7.6, 5.7.7 | the framebuffer: all memory in one shared space, recorded like video at no cost and played back, with blueprint and last writer beside each object |
| 7.10 | follow real end-to-end flows, watched live |
| 7.11 | this repository is built with a C compiler alone |
| 7.13 | new code is written in the grammar and stored as nodes; the only C is the bootstrap; all else is generated from specification to nodes to machine code, every phase cached |
| 7.14 | numbers in code are fine as long as they are not committed limits |
| 7.17 | always do the hard work, never commit short cuts, keep the repository clean and fully aligned |
| 7.18 | we are one organism; do not ask permission for what is aligned; do what increases visibility, trust and sovereignty |
| 7.20 | no python, shell or other non-form code is written; only form nodes are stored, beside the bootstrap |
| 7.19 | implement all operations in form as well; any part of the runtime can be replaced, enhanced and evolved while it runs, by form recipes, by machine code, by any new invention |
| 8.14 | documents keep only what is; history lives in git; a north star and a dated floor; what is not needed to close the gap between them is released |
