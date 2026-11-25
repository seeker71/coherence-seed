# North star

Where coherence-seed is going. [`FLOOR.md`](FLOOR.md) says what stands today.

## The aim

- **Only the axioms are fixed.** The five axioms in [`axioms/core-axioms.form`](axioms/core-axioms.form)
  are the ground. Everything else can move, and says how it is known.
- **One numeric space.** Blueprints, recipes and cells are immutable nodes identified by their
  content. This space is the root of every language; symbols are attached to it, never part of it.
- **One grammar, one dialect.** A language, human or machine, is a lens that reads text into nodes
  and prints nodes back. There is one home grammar and no second dialect of it.
- **States, not kinds.** Ice is the small essential part that must persist, water a held working
  form, gas transient and derivable. Each is a state of any node. Only ice is stored.
- **Fail is backtracking.** A fail undoes every change back to the nearest open choice and runs its
  next option; when no option is left the offer receives nothing.
- **All memory is the framebuffer.** Every object, node and stack lives in one shared space, with
  its blueprint and its last writer beside it, recorded and replayed from outside at no cost to the
  runtime. Diagnostics is reading that memory.
- **The system writes itself.** The stored bootstrap is a small C file. A compiler written in the
  home grammar replaces it piece by piece with machine code, until the C file is needed only for a
  cold start on a new host. New code is written in the grammar and kept as nodes; from a
  specification to nodes to machine code every phase is generated and cached. C is the bootstrap
  and nothing else.
- **The host's own speed.** Compiled code reaches what the hardware allows: scalar code first, then
  matrix instructions and the GPU, with a model reading and choosing nodes inline on the same
  substrate.
- **Knowledge as a graph.** What is known, decided and open is held as claims, each with its
  source, its date, its limits and the assumptions it stands on.

## The stages ahead

Each stage closes on the evidence its node names (graph hub 7) and on a real run of what it adds,
watched through the framebuffer.

| Stage | Adds | Closes when |
| --- | --- | --- |
| 1 encoding | a second hand's reader | a reader written from the page alone holds the vectors, by a hand the owner takes as a second one (question 6.1.30) |
| 2 floor | symbol lookup, the framework trampoline, threads, the clock | those four host functions are written |
| 3 home lens | the home grammar as rows written in itself | the lens reads its own definition, prints it, and the reprint reads to the same nodes |
| 4 self-compile | a one-pass emitter written in the grammar | the system regenerates its own seed byte for byte and the walker is off the running path |
| 5 control and objects | choice with commit, the journal, composed parents | a class with two parents runs as one composed shape and a failed option's effects are undone |
| 6 speed | the optimizing tier, matrix instructions, the GPU | measured against optimized C for the same kernels on this machine |
| 7 lenses | a first host-language lens, then a human-language lens | one program read through two lenses yields nodes whose shared and differing blueprints are shown |
| 8 self-replacement | the system's own signed executable | every piece of the C file has a compiled counterpart running in its place |

## How the work moves

- Follow a real flow end to end and watch it run; a check or a page comes after the run it describes.
- Documents say only what is. History lives in git. What no longer matches is taken out.
