# Starting a session in coherence-seed

Everything a session needs is in this repository, and it builds with a C compiler and `make`.

## Ground first

1. Read [`NORTH_STAR.md`](NORTH_STAR.md) for the direction and [`FLOOR.md`](FLOOR.md) for what
   stands.
2. Build and look at the present state before asserting anything about it:

   ```bash
   make && cc -O2 -o kg tools/kg.c && make check && ./kg check
   ```

3. Read the design you are about to touch: `./kg brief | grep '^5\.'` for the design, `'^7\.'` for
   the path, `'^6\.'` for what is open.

## Guidance

We hold this work together. Guidance orients; it is not a list to enforce, and what is aligned
needs no permission. It is kept short and in our own words (graph node 7.13), not quoted.

- **Form first.** New code is written in form and kept as nodes. C is the bootstrap only, and no
  other language is written. What a program lacks becomes an operation of the bootstrap.
- **Every part can be replaced.** The bootstrap's code is one implementation of each part; a
  recipe or machine code can take its place while the system runs.
- **Follow a real flow.** Make something run end to end and watch it through the framebuffer. A
  running program says more than a check, a table or a page about it.
- **Only what is.** Pages and nodes say what is true now, briefly and at a high level. History
  lives in git; what no longer matches is taken out.
- **The framebuffer is the memory.** What the runtime keeps lives in the shared region with its
  blueprint and its writer, so seeing it needs nothing added.
- **The real work over the shortcut.** When the aligned way cannot run yet, build what it needs,
  or say plainly in `FLOOR.md` that it is not there.
- **Say how a thing is known**, in plain words someone outside the session can follow.

## Before a commit

`make check` and `./kg check` pass, and `git log --format=%ae | sort -u` shows only the address
the existing commits carry.
