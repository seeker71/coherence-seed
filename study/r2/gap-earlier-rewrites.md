# Gap G7: earlier restart attempts and the greenfield carrier

Written 2026-11-24. Closes critic gap G7 (`critic.md:129-137`: "No report opened the rewrite itself to learn why a
previous restart did not carry") and deepens `runtime-jit.md` section 11 (the carrier) and section 12 (the size
estimate), which I build on and in two places correct.

## 0. Method, paths, status words

- `R` = `coherence-kernel` (HEAD `b8ff978c9`,
  2026-11-24 18:07 +0800; `git status --short` empty before and after my work).
- `K` = `coherence-kernel` (the main checkout; `origin/main` = `b8ff978c9`).
- `W/<name>` = `<name>` for the five sibling checkouts.
- `M` = `memory`.
- `S` = `scratch`;
  `P` = `S/probes` (files I extracted with `git show` or built; nothing outside `S` was written).
- **observed** = I read the cited line, ran the cited git command, or ran the cited scratch program in this session.
  **inferred** = my reasoning; the basis is named. **assumed** = not verified.
- I did not run `fkwu` anywhere. I ran: `git` read commands; `cc` and `otool` on copies inside `P/membrane-run`;
  the two resulting scratch binaries (`bboot`, `limits-probe`). The scratch copy of `membrane-witness.c` differs from
  the repository file in exactly seven string literals (`"/tmp/greenfield-` became `"./greenfield-`) so that the run
  wrote only inside `P/membrane-run`. I ran the witness without `--produce` (that flag compiles and runs the old seed).
- All five sibling checkouts are git worktrees of the one repository `K` (`git worktree list` in `K`, observed), not
  separate repositories. So "an earlier rewrite" is a branch, and its fate is readable from `git`.

## 1. Findings in brief

1. Only **two** of the five checkouts were restart attempts. `coherence-kernel-rewrite` (with its continuation in
   codex worktree 4523 and the branches `claude/rewrite-home`, `claude/rewrite-on-main-runtime`) is the C rewrite of
   the seed of 2026-11-06 to 11-08. The greenfield carrier (`runtime/membrane-boot.c`, one commit, 2026-11-05) is the
   other; it lives on main, not in a separate checkout. `coherence-kernel-raw-loader` is a performance lane inside the
   old seed (merged), `coherence-kernel-codex-floor` is the measuring organ built for the rewrite (merged), `ck-pre`
   and `ck-turn` are short-path worktrees for A/B and for model turns (no commits of their own).
2. The C rewrite did not carry for a recorded reason: its evaluator stayed "about 40x slower than main's (op-manifest
   39 s against 1 s)" after three optimisation rounds (commit `803ce20d8`, 2026-11-08 18:01). Its Form-side work was
   landed on the old C four hours later with a 143-line change to the old seed (`7ed6ad7b2`, 2026-11-08 22:23).
3. The shape of that failure is the lesson: the rewrite started as a clean 1,574-line reader plus evaluator with 24
   operations and, because its acceptance test was "run the existing body", reached 10,302 lines and 254 operations
   in 56 hours, re-implementing a BML reader in C on the way.
4. The greenfield carrier did not carry for a structural reason: it has no reader and no walker by design ("C does
   not evaluate a recipe"), nothing in the tree produces an image for it, and its "produces fkwu" proof bypasses the
   membrane entirely: the witness program forks `cc` on the old 32,016-line seed with its own `fork`/`execvp`.
5. The carrier's witness (31 checks) passes on this Mac today, built from an unmodified copy in 0.19 s (71,400-byte
   binary, no warnings under `-Wall -Wextra`). It proves the word encoding, content identity within one store,
   freeze/thaw with refusal of a damaged image, publication without entry, a machine-code leaf answering 42, port
   discipline, and a fault that keeps its signal. It does not prove x86-64 (the embedded x86 leaf returns 40, by
   disassembly), and its "branch" check cannot tell the two arms apart.
6. Three hard limits of the carrier, measured: the store is full after 3,947 top-level offers and never frees a slot
   for reuse; a node that names the same child twice doubles its canonical bytes, so 16 levels of sharing reach the
   4 MiB refusal; a file over 1 MiB reads as nothing.
7. About 460 lines of `membrane-boot.c` can be inherited nearly unchanged. To stand alone without the old seed it
   lacks an entry, a scalable store, a seed-image format for shared graphs, a walker (or a binary seed), most doors,
   and a code arena. My grounded estimate for a standalone bootstrap is 1,800 to 2,200 lines of C with a walker, or
   1,350 to 1,500 without one (binary seed per target). Both are inferred, and the rewrite's history says such a
   budget holds only if the walker's one job is to run the compiler once.

---

## 2. Dated history

### 2.0 Timeline (all times +0800; every row observed with `git log`, `git reflog` or `ls -la` unless marked)

| When | What | Where | Outcome |
|---|---|---|---|
| 2026-06-28/29 | Seed `runtime/fkwu-uni.c` first committed (`c02dae6b4`, 2026-06-28 17:21 -0600) | `K` | start of the old body |
| 2026-06-29 | History after `a0b97cf00` later rewritten by a binary-stripping filter (39 large binaries removed from every commit; same subjects and dates, new hashes) | note `M/reference-history-rewrite-and-the-cut.md:11-15` (2026-11-08) | observed as a record only |
| 2026-09-29 22:15 | The cut: `115fea55e` "Cut the repo to its end-to-end healthy flow: 14,729 files to 1,628" (13,562 files changed, 1,132,646 deletions) | `K` | a reset by deletion, not by rewrite |
| 2026-10-03 | The Go, Rust and TS sibling kernels leave (`d8c27d222`) | `K` | fkwu becomes the only reader (see `critic.md` R15) |
| 2026-11-04 15:54 to 11-05 01:00 | Raw cell image lane: 30 commits in the range `776943730~1..7d85e5660` (27 without the scheduled "Native turn/reading" rows) on `perf/raw-cell-disk-reconstruction` | `W/coherence-kernel-raw-loader` | merged (0 commits ahead of main) |
| 2026-11-05 15:17 | Greenfield carrier: one commit `16b3d3eb6`, 1,898 lines in four files | `R/runtime/membrane-*.{c,h}`, `R/observe/greenfield-produce-run.bml` | on main; never touched again |
| 2026-11-06 10:05 | "The runtime source is the rewrite." `b21a77483`: seed 32,016 lines replaced by 1,574 | `W/coherence-kernel-rewrite`, branch `perf/optimization-book-jit` | branch never merged |
| 2026-11-06 17:59 | Candidate floor door `7208c8963` (landed as `d30dcd880`, #676, 18:04) | `W/coherence-kernel-codex-floor` | merged |
| 2026-11-06 19:58 / 20:50 | Last rewrite commit `2673d5392` (7,222 lines); last uncommitted edit of the worktree's seed 20:50 | `W/coherence-kernel-rewrite` | worktree left with a 145-line uncommitted diff |
| 2026-11-06 19:35 | Codex continuation `c2020efad` (7,857 lines) | `~/.codex/worktrees/4523/coherence-kernel` | carried to `claude/rewrite-home` on 11-08 |
| 2026-11-08 16:10 to 18:01 | Three more commits on `claude/rewrite-home`; seed 10,302 lines; "remains about 40x slower" | branch `claude/rewrite-home` (`803ce20d8`) | branch ends here |
| 2026-11-08 22:18 / 22:23 | "The rewrite's Form work runs on main's runtime" (`2ee36f831` on its branch, `7ed6ad7b2` on main) | `K` | the Form half lands; the C rewrite does not |
| 2026-11-09 15:15 | `ck-turn` created (detached, follows `origin/main` until 2026-11-12 14:15) | `W/ck-turn` | workspace; 8 uncommitted files |
| 2026-11-09 17:18 | `ck-pre` created at `de0044c9f`, never moved | `W/ck-pre` | A/B baseline; clean |
| 2026-11-09 | Sealed kernel store, warm packs, Linux build (`550078396`, `afcb161af`, others) | notes `M/project-fkwu-version-store-2026-11-09.md`, `M/project-prebuilt-design-2026-11-09.md` | on main (records, not re-verified) |
| 2026-11-24 | This restart (new repository, one grammar) | mission text | pending |

### 2.1 `coherence-kernel-raw-loader` (2026-11-04 to 11-05): a perf lane, carried

- Branch `perf/raw-cell-disk-reconstruction`, head `7d85e5660` (2026-11-05 01:00). `git rev-list --left-right --count
  HEAD...origin/main` gives `0 653`: every commit is an ancestor of main. Status: observed.
- What it was for, from its own subjects (observed, `git log -30`): "A matching raw-cell image is admitted and not
  parsed again" (`28e1c258b`, 16:24), "A raw-cell row stays in the image until a call or a read reaches it"
  (`08a55ae91`), "Reconstruct node reads and catalog exports on demand" (`e0ce2eabf`), "Preserve raw image identity and
  reconstruct callees on demand" (`041465b7d`). It taught the old seed to admit a stored cell image and rebuild rows
  lazily.
- Cost, observed: `git diff --stat 776943730~1..7d85e5660 -- runtime` = `runtime/fkwu-uni.c | 1423 ++++...`, 1,035
  insertions and 388 deletions in about nine hours.
- It is not a restart and did not stop. Why it matters here (inferred from the two facts above plus the note below):
  it is the old body's answer to "load cells instead of reading text", and it was paid for inside the C seed. Nine
  days later the same author measured that the cost after a one-definition edit was still one whole-image assembly:
  "the door's IMAGE built 1 = 37490 KiB in 12.94 s wall ... the next run read the same image in 0.18 s"
  (`M/project-fkwu-version-store-2026-11-09.md:40`, entry dated 2026-11-13; status: observed as a record).

### 2.2 The greenfield carrier (2026-11-05): a floor with nothing standing on it

- One commit, `16b3d3eb6`, 2026-11-05 15:17:34: "The greenfield carrier produces fkwu and form-cli. It compiles
  runtime/fkwu-uni.c the way the old bootstrap does. That fkwu and the standing fkwu both answer ground 42, and both
  produce a form-cli." Files: `runtime/membrane-boot.c` 1,199, `membrane-boot.h` 169, `membrane-witness.c` 510,
  `observe/greenfield-produce-run.bml` 20. Status: observed (`git show --stat`).
- No later commit touches any of the three C files (`git log --follow`), no other branch has a commit whose subject
  mentions it (`git log --all -i --grep=greenfield` returns this commit and two unrelated July rows), and the only
  file in the tree that names it is its own door (`git grep -i 'greenfield\|FBOOT\|bboot'` outside `runtime/membrane-*`
  returns only `observe/greenfield-produce-run.bml`). No memory note mentions it (`grep` over `M/*.md`). Status: observed.
- Why it did not carry. Evidence, each observed; the conclusion is inferred from them:
  1. It cannot run any Form by design: "C does not evaluate a recipe" (`R/runtime/membrane-boot.h:31`), and the only
     thing `thaw` accepts is its own `FBOOT3` format (`R/runtime/membrane-boot.c:1105`).
  2. Nothing produces an `FBOOT3` image except the witness's own three-cell example (`membrane-witness.c:138-143`;
     the image is 176 bytes, see 3.11).
  3. The "produce" proof does not pass through the membrane. `gf_produce` (`R/runtime/membrane-witness.c:427-499`)
     calls a private `spawn_tail` (`:357-413`: `pipe`, `fork`, `dup2`, `execvp`) with `cc -O2 -o
     /tmp/sema-greenfield-boot/fkwu runtime/fkwu-uni.c` (`:449-455`), then runs that binary and `./fkwu` on
     `bootstrap/ground.fk` and on `gate/form-cli-build-run.bml` (`:461-490`). The membrane's own `process` port is
     exercised only with `/usr/bin/true` (`:249-255`). This corrects `runtime-jit.md:423-424`, which says the proof
     spawns `cc` "through the process port".
  4. The next commit on this theme, 19 hours later, took the opposite route: a new source interpreter in C
     (`b21a77483`, 2.3).
  So the carrier is a correct, small implementation of the axioms' floor with no bridge from the body's source to an
  image it could thaw. Its commit title claims more than its code does.

### 2.3 The C rewrite of the seed (2026-11-06 to 11-08): did not carry, cause recorded

Lineage (observed, `git log`): `perf/optimization-book-jit` in `W/coherence-kernel-rewrite` (10 commits over main
`475057533`, co-authored with the Cursor lane) -> the same commits plus three in codex worktree 4523
(`codex/native-response-rewrite-completion-20261106`) -> carried on 2026-11-08 to `claude/rewrite-home` ("12 commits
+ 1 commit of its 214 open files", `M/reference-history-rewrite-and-the-cut.md:26`) -> `claude/rewrite-on-main-runtime`
(`2ee36f831`) -> main `7ed6ad7b2`. Neither `2673d5392` nor `803ce20d8` is an ancestor of `origin/main`
(`git merge-base --is-ancestor`, observed).

What it was (observed, `P/rewrite-b21a77483-fkwu-uni.c`, the file at `b21a77483`): "The Form runtime reads one source
file and offers its forms. A word is nothing, an integer, a real, text, a list, a function, or a node." (`:1-5`). A
value is a struct passed by value (`:196-207`), the reader is an S-expression parser (`:462-646`), the evaluator is an
explicit frame stack with six frame kinds (`:237-243`, `:1191-1490`), and 24 operations are registered (`:1157-1190`).
It keeps every named constant as a `static const` (`:22-78`).

Growth (observed: `git show <c>:runtime/fkwu-uni.c | wc -l` and `grep -c 'register_operation("'`):

| Commit | Time | Lines | Operations |
|---|---|---|---|
| `b21a77483` | 11-06 10:05 | 1,574 | 24 |
| `b6f6f1e3a` | 11-06 12:00 | 3,875 | 79 |
| `2c9a98115` | 11-06 17:22 | 5,574 | 121 |
| `fd17fac83` | 11-06 17:34 | 6,760 | 179 |
| `2673d5392` | 11-06 19:58 | 7,222 | 186 |
| `92eec0e86` (codex) | 11-06 19:35 | 7,857 | 201 |
| `f2399070c` | 11-08 16:10 | 10,139 | 254 |
| `803ce20d8` | 11-08 18:01 | 10,302 | 254 |

Main's seed for comparison: 32,016 lines at the base `475057533`, 32,178 at the landing `7ed6ad7b2`, 34,149 today.

How far it got, from its own receipt `W/coherence-kernel-rewrite/receipts/2026-11-06-rewrite-floor.md` (observed):

- 16:50 on day one: 3,875 lines, `cc -O2` in 0.4 s, 124 KB binary; answers the three ground files and two bands
  (`:14-27`). Every body-wide door "ends at its first unbound host name" (`:31-38`). Against the body's contract "the
  rewrite registers 121 names ... the registry names 124; 58 registry names the rewrite lacks" (`:54-63`). Two
  surface forms unread: typed `let` and `try X else Y` (`:40-46`).
- 17:xx: "the blocker has moved from unbound primitive names to reader coverage in the BML lane" (`:117-119`).
- 18:xx: `primitive-registry.fk` answers 0, `door-options-band` 1023, but `observe/door-link-health-run.bml` gives
  "no output after >120s" (`:141-157`).
- 19:xx round four: canonical conformance "FAIL 11/13 canonical expressions" (`:246`); the long door still runs past
  180 s "with high allocation pressure and large process footprint (~1.5 GB)" (`:261`).

The companion receipt (observed, `P/rewrite-integration.md`, from `claude/rewrite-home:receipts/2026-11-06-native-response-rewrite-integration.md`):
"This is an integration checkpoint, **not native response parity or a release**" (`:9`); it stops on
`class NativeAffineBuffer : Disposable<Int>` (`:95-100`) and on "an unbound name was offered: make_nodeid" (`:109`);
"node interning/introspection, `form_error`, and the `cell_run`/`cell_call`/`cell_input` contracts absent" (`:113-115`);
and it names the debt it created: "move the reader back onto that authority and remove the duplicate C parsing paths"
(`:131-132`). By the last commit the file's own header says "A form.bml section lowers its definitions into the same
forms" (`P/rewrite-home-fkwu-uni.c:5`) and it holds 37 `parse_*` functions and 14 `step_*` functions (observed, `grep -c`).

Why it stopped (observed, in the author's own words):

1. Speed. Commit `803ce20d8` (2026-11-08 18:01), after per-unit check images, in-place frame reads, cached call
   resolution and borrowed string literals: "Gate rows: 11 of 19 pass under a 30-60 s parallel cap (9 before). The
   evaluator remains about 40x slower than main's (op-manifest 39 s against 1 s), which is the remaining regression."
   The same message names the mechanism it had just reduced: "step_eager and enter_field_owner read the 552-byte frame
   in place instead of copying it two or three times per evaluated argument."
2. Its Form-side store. Commit `757ae59ca` (2026-11-08 17:23): "Every process that interned a node replayed the whole
   native-field journal in Form, and the journal grew by a roster row per run without end (58 MB: three interns took
   over ten minutes)"; and "Startup re-ran the Form binding checker over every loaded unit on every start (3 s for an
   empty program)".
3. The decision. `M/project-rewrite-form-on-main-runtime-2026-11-08.md:11-16`: "the rewrite's evaluator was ~40x slower
   than main's, so its Form work landed on main's C instead ... **Why:** Urs asked for the rewrite without a speed
   regression and with minimal C change." Landing commit `7ed6ad7b2`: 200 files, `runtime/fkwu-uni.c | 143 +-`; main's C
   gained three small things (file leases, an 8-byte number codec, `record_get` answering nothing for an absent field).
4. What stayed behind on the branch: "the BML field (native-field.bml), kernel-ast.bml checker and the field-only
   publisher ... need door 61 and the rewrite evaluator" (same note, `:24-25`); in the landing diff
   `form/form-stdlib/bml/native-field.bml` (977 lines) and `kernel-ast.bml` (503 lines) are removed again (`2ee36f831`
   stat, observed).
5. A silent seam created by having two runtimes for two days: "float_leaf mode numbers DIFFER between the rewrite and
   main ... must be renumbered or it silently hits the wrong door" (same note, `:19-21`).

Inferred cause, one sentence (from 1 to 5 and the growth table): the rewrite replaced the C and kept the contract
(the 124-name primitive registry, two surface grammars, the band pins), so every hour went into re-covering the old
surface in new C, and the one thing that was genuinely new, a plain boxed-value frame interpreter, was slower than the
tagged-word walker it replaced.

### 2.4 `coherence-kernel-codex-floor` (2026-11-06): the second witness, carried

- Branch `codex/candidate-floor-door`, one commit `7208c8963` (17:59), on main as `d30dcd880` (#676, 18:04); the files
  stand today (`R/observe/candidate-floor-run.bml`, `R/form/form-stdlib/bml/candidate-floor.bml`, observed `ls`).
- Purpose, from the commit body (observed): "The body has one kernel, so a rewritten runtime had no second witness:
  the band sweep runs each band in process on the resident. This organ begins the candidate by argv as a child on
  each band the sweep keeps, with its own deadline, reads the last line and exit, and compares the reading to the
  head's pin: full, short, stop or deadline".
- It did not stop; it was built in eight hours because the rewrite needed it. I found no receipt of a full candidate
  sweep of the rewrite (the receipts in 2.3 list doors run by hand). Status of that absence: observed for the files I
  read, not exhaustive.

### 2.5 `ck-pre` and `ck-turn` (2026-11-09 to 11-12): workspaces, not attempts

- `W/ck-pre`: detached at `de0044c9f` since 2026-11-09 17:18 (reflog has one entry), clean except ignored caches.
  Purpose in the author's words: "build the real pre-rewrite commit apart in a short-path worktree (~/source/ck-pre,
  de0044c9f, its own seed+dylib) and run the same real check ... there and in the current tree with ONE thing swapped
  each time ... Mixed-file reverts do not work (51 compile errors): the rewrite changed the stack together"
  (`M/project-local-loop-goal-2026-11-09.md:55-56`). Status: observed as a record; the worktree facts observed.
- `W/ck-turn`: created 2026-11-09 15:15, fast-forwarded to `origin/main` until `656762fe9` (2026-11-12 14:15), eight
  uncommitted or untracked files (flow and serve work). Purpose: "Run physical turns from `~/source/ck-turn` (a
  short-path worktree of main I made ...)", made because a scratch checkout path of about 165 bytes overflowed a
  128-byte buffer in the seed (`gift_roster_register`, `nm166[128]`): "The same fatal would hit any user with a
  checkout path over ~100 bytes" (same note, `:37-39`).
- Both exist because of the landing of 2.3: the day after it, a model lane decoded only zeros and the cause was found
  by building the pre-landing commit apart (same note, `:53-56`). That is the measured price of landing 200 files at
  once.

### 2.6 What the five memory notes add (records dated in their own frontmatter; I re-verified only the commit facts)

- `M/reference-history-rewrite-and-the-cut.md` (2026-11-08): two earlier resets of a different kind. The history
  filter of 2026-06-29 (old copies show "~3000 ahead / ~4000 behind" without being divergent work) and the cut of
  2026-09-29 (`115fea55e`, verified above). Line 20-22: the rewrite line was "Left for Urs ... its receipt claims no
  landing".
- `M/reference-cut-moves-kernel-files-adds-conformance-gate.md` (2026-09-29): deleting kernel source files changed
  the gate set itself ("`full` goes 8191 to 16383 ... three red gates, not a broken cut", `:11-14`). A verdict whose
  meaning depends on which files moved.
- `M/project-rewrite-form-on-main-runtime-2026-11-08.md`: quoted in 2.3.
- `M/project-fkwu-version-store-2026-11-09.md`: the kernel became a sealed, reproducible store entry. Two facts a
  self-replacing bootstrap inherits: "macOS signs a binary with its FILE NAME as identifier, so name+layout are part
  of the build" (`:16-17`), and the decision "no prebuilt kernel downloads (the cloud session derives the kernel from
  the source it cloned: `cc` once, ~8 s)" (`:28`).
- `M/project-prebuilt-design-2026-11-09.md`: "the key is seed digest + target, not exe bytes" (`:12`); and a
  determinism fault that argues against arrival-order ids: lowering units "differ only in `$lambda-@..<n>` names
  (source-compiler.fk:671 embeds value_str of a recipe node = allocation-dependent id): lowering bytes are NOT
  deterministic even on one host" (`:14-16`).

### 2.7 Causes of not carrying, across attempts (inferred; each from the observed items named)

1. **Parity with the old surface was the acceptance test** (2.3 growth table, receipt `:54-63`). A new floor judged
   by 536 old bands inherits the old shape.
2. **Two readers again within a day** (2.3 integration receipt `:131-132`), the same root cause `lessons.md` ranks
   first.
3. **No speed budget stated before the first line** (2.3 item 1): the regression was measured on day three.
4. **The floor and the body were never joined** (2.2): a carrier without an image, and a body without a way to become
   one.
5. **Landing everything together** (2.5): 200 files in one commit, then a bisect by parallel worktree.
6. **Second witness built after the fact** (2.4).

---

## 3. `membrane-boot`: a precise description

Sources: `R/runtime/membrane-boot.h` (169 lines), `R/runtime/membrane-boot.c` (1,199), `R/runtime/membrane-witness.c`
(510), all from commit `16b3d3eb6` (2026-11-05), read whole. SHA-256 of the three files as read:
`71d05a73...`, `e263b54a...`, `47979738...` (observed, `shasum`). Everything in 3.1 to 3.12 is observed by reading
unless marked; 3.13 and 3.14 include runs.

### 3.1 Build and size

- Build line: `cc -O2 -o <dir>/bboot runtime/membrane-boot.c runtime/membrane-witness.c`
  (`R/observe/greenfield-produce-run.bml:11`), then `<dir>/bboot --produce` (`:12`). `membrane-boot.c` has no `main`;
  the witness supplies it (`membrane-witness.c:501-510`).
- My run on unmodified `membrane-boot.c`/`.h` and the path-patched witness: `cc -O2 -Wall -Wextra` in 0.19 s real, no
  diagnostics, binary 71,400 bytes (`P/membrane-run/bboot`).
- Host dependencies: 13 libc/POSIX headers (`membrane-boot.c:3-15`), `pthread.h` and `sys_icache_invalidate` on Apple
  (`:17-20`), and one `dlopen` of a hard-coded Metal framework path (`:488`).

### 3.2 The word (64 bits)

- `0` = state 0, `1` = state 1, `2` = nothing, `(slot << 2) | 3` = a node in this carrier (`membrane-boot.h:10-15`;
  `membrane-boot.c:94-103`). `word_class` returns `CLASS_STATE` for 0 and 1, `CLASS_NOTHING` for 2, `CLASS_NODE` when
  the low two bits are 3 and the slot is non-zero, and -1 for anything else (`:105-110`). `word_state(2)` returns
  nothing (`:96-99`).
- A node word carries a **slot**, a process-local index, not an identity. "The driver's pointer is not a cell
  identity" (`h:44`) is stated for drivers; the same holds for the word itself (inferred from `:101-103`).
- The C API mostly passes `uint32_t` slots, not words; words appear at two places only: the `Outcome.ack`
  (`h:103-106`) and the ABI-1 leaf boundary (3.8).

### 3.3 Cell layout and store

- `struct Cell` (`membrane-boot.c:31-44`): `id` (u64 digest), `kind`, `nchild`, `nbytes`, `refs`, `live`,
  `child[32]` (inline slots), `payload` (malloc), `canon` (malloc), `canon_len`, `page`, `page_bytes` (the executable
  mapping of a leaf).
- `struct Machine` (`:62-76`): `cells[8192]` inline, `ncells`, `channel`, `frame[4096]`, `nframe`, `held[128]`,
  `carriers[24]`, `facts`, `depth`, a self-pipe (`event_r`, `event_w`). One `calloc` (`:182`). Slot 0 is never a cell
  (`:185`).
- Limits: `MAX_CELLS 8192`, `MAX_CHILDREN 32`, `MAX_PAYLOAD 1 MiB`, `MAX_FRAME 4096`, `MAX_HELD 128`,
  `MAX_CARRIERS 24` (`:22-27`).
- `cell_compose` (`:213-281`): validates, builds the canonical bytes, then **linearly scans every live cell** for a
  byte-equal canonical string (`:245-250`) and returns that slot; otherwise takes `slot = m->ncells++` (`:256`). A
  slot is never taken twice.
- Refs: a parent increments each child's `refs` at compose (`:266-269`); `cell_pin`/`cell_unpin` add and remove one
  (`:283-289`); `machine_release_unpinned` frees every cell whose `refs` is 0 and recurses into its children
  (`:291-318`). Nothing else releases. A freshly composed value (an ack, a file's bytes) has `refs` 0 until someone
  pins it.

### 3.4 Canonical bytes and the identity function

- Canonical bytes of a cell (`:224-243`): `u32 kind`, `u32 nchild`, `u32 nbytes` (little-endian, `wr_u32` at
  `:1008-1015`), then for each child `u32 child_canon_len` followed by **the child's whole canonical bytes**, then the
  payload bytes. It is the full tree serialised in place, recursively; children are not referenced by digest.
- Refusals: kind 0, more than 32 children, payload over 1 MiB, a dead child, or a total over 4 MiB minus 4 (`:223-229`).
- Identity: "Equality is the bytes" (`h:20`). `id` is FNV-1a 64 over the canonical bytes (`:28-29`, `:81-92`, `:244`,
  `:259`). The digest is computed and stored but **not used to find anything**: the dedup is the linear `memcmp` scan,
  and line `:251` is `(void)index;`. `cell_find` by id is also a linear scan (`:371-376`) and has no caller in the
  witness.
- Consequence, checked by formula: scalar = 12 + 8 = 20 bytes; text "add" = 15; leaf with 8 code bytes = 12 + 1 + 8 =
  21; export = 12 + (4+15) + (4+21) = 56; channel "boot" with that export = 12 + (4+56) + 4 = 76. The witness prints
  `image_bytes=176` = 8 + 8 + 76 + 8 + 76 (3.11). Status: observed (run) and matches the reading.

### 3.5 Kinds and makers

- Kinds (`h:68-78`): scalar 1, bytes 2, text 3, port 5, leaf 6, channel 7, reading 8, frame 10, export 11. Numbers 4
  and 9 are unused (inferred: removed kinds).
- Makers (`:384-433`): scalar = 8 little-endian bytes of an `int64` payload; bytes; text (payload without terminator);
  leaf = payload `[abi byte][code bytes]` with `abi` 0 or 1; export = children `(name, leaf)`; port = payload
  `[direction][shape][name length][name]`, name at most 32 bytes; channel = payload name, children any cells.
- There is no kind for a recipe, a blueprint, a list, a float, or a symbol. `KIND_FRAME` and `KIND_TEXT` are also
  used by the witness as generic "node with children" (`membrane-witness.c:119`).

### 3.6 `offer`, the only act

- `Outcome offer(Machine*, uint32_t slot, const uint32_t *args, uint32_t nargs)` (`h:156`; `:543-556`). `Outcome` is
  `{int arrived; uint64_t ack;}` (`h:103-106`).
- Dispatch on the target's kind (`offer_inner`, `:914-989`):
  - scalar, bytes, text, reading, frame: with no args the ack is the node itself; with args it is a breach (`:920-924`).
  - port: `run_carrier` (3.7).
  - leaf or export (export enters its second child): `enter_leaf` (3.8).
  - channel with no args: **publication**: walk the whole graph and map every leaf's code executable, enter none,
    ack state 1, or nothing if any mapping failed (`:945-965`). Channel with args: compose a **new** channel whose
    children are the old children plus the args; if the old one was the machine's channel, re-point it; ack the new
    node (`:966-985`). The old channel is unchanged (immutability; witness check `channel_mints`).
  - any other kind: breach.
- Depth: `depth` is counted per nested offer and refused with nothing at `max_depth` (`:916`), where `max_depth` =
  stack limit / 8192, at least 32 (`:156-158`); 1,022 on this host (witness output).
- Breach (`:135-140`): pushes a frame row with phase 4 and acks **state 0** "from the membrane"; the `why` argument is
  discarded (`(void)why;`), and of the four `BREACH_*` names only `BREACH_SHAPE` is ever passed (observed, `grep`).

### 3.7 Carriers (ports)

- A port is a cell `(direction, shape, name)`; shapes bool, scalar, bytes, text, pixel, key; directions afferent,
  efferent (`h:39-41`, `:80-92`), the same six shapes and two directions as `R/axioms/host-kernel.form:86-91`.
- Registered at open (`:476-495`): `clock.monotonic`, `clock.realtime`, `entropy`, `file.read`, `file.write`,
  `cpu.leaf`, `event.wait`, `event.signal`, `process`, `identity.eq`, `gpu.admit`. "video, audio, pixel, and key stay
  unregistered until a driver is discovered" (`:494`).
- Resolution (`run_carrier`, `:874-912`): unknown name -> nothing; known name with a different direction or shape ->
  breach (state 0); otherwise a `strcmp` chain selects the C function (`:897-908`). Each carrier counts `offers` and
  `nothings` (`:896`, `:909`), readable through `carrier_health` (`:991-1003`).
- The doors:
  - `file.read(text path)` -> bytes node, whole file, path under 512 bytes (`:674-717`).
  - `file.write(text path, bytes)` -> state 1 after temp file, `fsync`, `rename`; mode 0600 (`:719-749`).
  - `clock.*()` -> a `reading` node with children (count scalar, domain text) (`:751-764`).
  - `entropy([n])` -> 1 to 32 bytes from `/dev/urandom` (`:766-786`).
  - `identity.eq(a, b)` -> state 1 or 0 by `memcmp` of canonical bytes (`:788-798`).
  - `event.signal()` writes one byte to the self-pipe; `event.wait(ns)` polls it without blocking (`:800-828`).
  - `process(text path, text args...)` -> `fork`, `execv`, blocking `waitpid`; ack = exit status scalar; at most 8
    arguments of under 256 bytes; no stdin, stdout or environment handling (`:830-867`).
  - `gpu.admit()` -> state 1 if `dlopen` of Metal succeeded at open, else nothing (`:869-872`, `:488-493`).
  - `cpu.leaf(bytes code [, scalars...])` -> makes an ABI-0 leaf from the bytes; with scalars, enters it (`:657-672`).

### 3.8 Leaves: machine code as cell payload

- A leaf's payload is one ABI byte and raw machine code (`:400-411`). "A leaf is bytes the image supplied. Publication
  maps those bytes and does not enter them" (`h:32-33`).
- Mapping (`map_executable`, `:558-584`): the code must fit one page (`n > page` refuses, `:562`); one
  `mmap(PROT_READ|WRITE|EXEC, MAP_PRIVATE|MAP_ANON|MAP_JIT)` **per leaf**; on Apple arm64 the per-thread write toggle
  around the `memcpy`, then `sys_icache_invalidate`; elsewhere `__builtin___clear_cache`. Unmapped when the cell is
  released or the machine closes (`:190-195`, `:302`).
- ABI 0 (`:619-633`): `int64_t f(int64_t, int64_t, int64_t, int64_t)`; at most four scalar args; the result is
  composed as a scalar node.
- ABI 1 (`:634-653`): `uint64_t f(const uint64_t *words, uint64_t argc, OfferFn offer)`; at most eight args; each arg
  is passed as a node word, **except** that a scalar cell holding 0 or 1 is passed as the state word 0 or 1 (`:643-645`);
  the result must be a valid word. The leaf calls back through the function pointer it is handed
  (`leaf_offer`, `:588-600`, via a file-static `call_m`, `:586`), so leaf code holds no absolute address.
- "Branch and hold are that leaf, or an outstanding host flight. They are not opcodes" (`h:34-35`). The witness's
  branch is 16 AArch64 words (`membrane-witness.c:89-94`); disassembled (`P/membrane-run/leaves-arm64.s`, `otool -tv`,
  observed): save frame, `x19 = words`, `x20 = offer`, `if words[0] == 1 then x0 = words[1] else x0 = words[2]`,
  `x1 = &words[3]`, `x2 = 2`, `blr x20`, restore, `ret`. That is `if` as a leaf: choose a target, offer it the two
  remaining args.

### 3.9 Held work: "arrived=0"

- Stated: "Held work is not a word: offer sets arrived=0 and writes no ack" (`h:15-16`); "An outstanding host operation
  stays open: arrived=0, no ack" (`h:29`); "Timeout and a missing driver are an arrived nothing" (`h:26`).
- Implemented (`:124-129`): `held_out()` returns `{arrived = 0, ack = 0}`. It has **one** caller: `event.wait` with a
  non-zero argument and an empty pipe (`:813`). With argument 0 the same situation is an arrived nothing. The argument
  is never used as a duration; `poll` is called with timeout 0 (`:811`).
- The `Held` table (`:56-60`, `:68`) is declared and zeroed in `thaw` (`:1142`) and otherwise never read or written
  (observed, `grep`). There is no flight identity, no resume, no scheduler.
- Across the machine-code boundary the distinction is lost: `leaf_offer` returns `word_nothing()` when the inner offer
  did not arrive (`:598`). So a leaf sees four outcomes; only C callers see five. This is the concrete form of critic
  contradiction C3 (`critic.md:312-316`).
- The frame row for a held offer records class 3 (`:552`, `:910`).

### 3.10 The frame

- A row is a `KIND_FRAME` cell with four scalar children: phase (1 = a top-level offer, 3 = a carrier crossing,
  4 = a breach), byte count of the args' payloads, duration in ns, ack class (`:504-517`, `:553`, `:893`, `:910`,
  `:138`). Rows are pinned and listed in `frame[]`; payload text is not copied (witness check `frame_keeps_text_out`).
- The rows are composed into the same store as everything else (see 3.14, limit 1).

### 3.11 Freeze and thaw

- `freeze` (`:1068-1090`): magic `"FBOOT3\0"` padded to 8 bytes, `u64 n`, the channel's canonical bytes, `u64 n`, the
  same canonical bytes again. "the declared identity, then the graph" (`h:46-47`). The block comment above still says
  "magic FBOOT2 ... Child links in the file are content identities, never slots" (`:1005-1006`); the code writes FBOOT3
  and children are inlined bytes, not links.
- `thaw` (`:1092-1162`): check magic and lengths; open a fresh machine; `parse_canon` (`:1039-1066`) rebuilds the graph
  bottom-up through `cell_compose` and compares each rebuilt cell's canonical bytes with the bytes it was read from;
  the root must be a channel and its canonical bytes must equal the declared identity; only then are the caller's
  cells replaced (a `memcpy` of the whole cell table, `:1138`). On any mismatch the caller is left as it was.
- Because the identity is the canonical bytes themselves, the check is "two copies in one file agree and re-compose to
  themselves". It detects damage to one copy. It is not a digest pinned outside the file (inferred from `:1076-1086`).

### 3.12 Fault handling

- `fault_arm(fd)` (`:1180-1195`): alternate stack, `sigaction` for SIGSEGV, SIGBUS, SIGTRAP, SIGILL. The handler
  (`:1165-1178`) writes the signal number to the fd, restores the default action and re-raises: "It does not resume the
  offer" (`h:53-54`).

### 3.13 The witness: what it proves, and how

Run: `P/membrane-run/bboot` (no arguments), exit 0, `fails=0`, output in `P/membrane-run/witness.out`. Host facts
printed: `page=16384`, `arch=aarch64`, `depth_limit=1022`, `entropy_present=1`, `gpu_present=1`. 31 checks, all 1:

| Check(s) | Proves | Lines in `membrane-witness.c` |
|---|---|---|
| `nothing_is_not_zero`, `state_zero_is_the_word_zero`, `state_rejects_other` | the word encoding of 3.2 | 100-102 |
| `old_survives`, `composted`, `remint_same_canon` | a cell a pinned parent names survives a release; an unreferenced one is freed; composing the same content again gives the same canonical bytes | 116-136 |
| `freeze`, `thaw`, `tamper_refused` | a three-cell channel freezes to 176 bytes, thaws, and an image with its last byte flipped is refused with the target machine unchanged | 138-147, 165-174 |
| `publish_does_not_enter`, `answer_is_node`, `observed_42` | offering the thawed channel acks state 1; offering its export with scalars 40 and 2 enters the machine-code leaf (`add w0, w0, w1; ret`) and acks a node holding 42 | 148-163 |
| `missing_is_nothing`, `shape_mismatch_is_membrane_zero` | an unregistered port name acks nothing; a registered name with the wrong shape acks state 0 and leaves a phase-4 row | 176-189 |
| `clock_domain`, `file_durable`, `entropy_bytes`, `gpu_admitted`, `process_exit_is_node`, `eq_same`, `eq_different` | each door once | 190-273 |
| `timeout_is_nothing`, `held_is_not_an_ack`, `event_woke` | 3.9 | 226-239 |
| `branch_one_arm` | an ABI-1 leaf calls back through `offer` and a file appears | 275-297 |
| `frame_has_rows`, `frame_keeps_text_out` | rows exist and none contains the written payload text | 299-306 |
| `channel_mints` | offering the channel an arg yields a new channel with one more child; the old one is byte-identical | 308-323 |
| `fault_pipe`, `fault_preserves_signal` | a child that enters `brk #0` dies of SIGTRAP and the parent reads 5 from the evidence pipe | 325-346 |

`--produce` then runs `gf_produce` (2.2 item 3). I did not run it.

What the witness does not prove (observed by reading and by disassembly, conclusions inferred):

- **x86-64.** The x86 add leaf is `01 fe 89 f8 c3` (`:110`), which disassembles to `addl %edi,%esi; movl %edi,%eax;
  retq` (`P/membrane-run/leaves-x64.s`, `otool -tv`): it returns the first argument, 40. The x86 fault leaf reads
  address 0 (`:65`), which raises SIGSEGV where the check demands SIGTRAP (`:344`). `branch_one_arm` is hard-coded to
  fail off aarch64 (`:297`). So on x86-64 at least three checks would fail; the witness has only ever been green on
  Apple arm64 (inferred; not run on x86-64).
- **The two arms of the branch.** Both arm slots are the same port, `args[1] = write_p; args[2] = write_p`, and the
  trailing args are the "yes" path and body in both cases; `no_path` and `no_body` are discarded with `(void)`
  (`:285-291`). Either arm writes the "yes" file; no arm can write the "no" file. The check shows the callback works,
  not that the leaf chose.
- **Any scale.** No check composes more than a few dozen cells, nests offers, or thaws an image larger than 176 bytes.
- **Held work resumed.** Nothing ever arrives later.

### 3.14 Limits and defects, measured or read

Measured with `P/membrane-run/limits-probe.c` linked against the unmodified `membrane-boot.c` (output
`P/membrane-run/limits.out`, observed):

1. **The store fills with its own observations.** 3,947 top-level offers of `clock.monotonic` and the next acks
   nothing: `live_cells=8191 frame_rows=4096`. After `machine_release_unpinned` 296 cells are live, yet a new
   `make_scalar` still returns 0: slots are never reused (`:256`).
2. **Sharing doubles.** A node with the same child twice: canonical bytes 60, 140, 300, 620, ... 2,621,420 at level
   16; level 17 is refused. Seventeen cells exhaust the 4 MiB identity limit.
3. **Large payloads vanish.** A 1,048,577-byte file through `file.read` arrives as nothing (`MAX_PAYLOAD`, `:24`,
   `:223`), indistinguishable from a missing file.

Read, not run:

4. `carrier_lookup` skips carriers that are not present (`:447`), while `carrier_name_known` does not (`:456-463`), so
   a registered but absent carrier takes the breach path (`:885-887`) and acks state 0. The header promises nothing
   for a missing driver (`h:26`, `h:43-44`) and the witness's `gpu_absent` and `entropy_absent` branches expect
   nothing (`membrane-witness.c:223`, `:245`). On a host without Metal those checks would fail (inferred; this Mac has
   both).
5. `machine_arm_carriers` (`:1197-1199`) calls `carriers_init` a second time on a machine that `machine_open` already
   armed (`:186`), which would register every carrier twice. It has no caller in the tree.
6. `offer` dereferences a null machine on its own breach path (`:548` -> `:138` -> `frame_push(m, ...)`).
7. The ABI-0 arm64 leaf in the witness adds 32-bit registers (`add w0, w0, w1`), so the "scalar" it returns is not an
   `int64` sum (inferred from the disassembly).
8. ABI 1 turns a scalar cell holding 0 or 1 into a state word (`:643-645`): the integer 1 and state 1 are the same
   thing to a leaf. This is critic question N1 ("whether an ordinary result is `1` ... or a `node`") decided silently.
9. Not thread-safe: one file-static `call_m` (`:586`), one file-static `fault_fd` (`:1163`).

---

## 4. The two specs beside the axioms: what they ask of a bootstrap

`R/axioms/host-kernel.form` (250 lines; last commit `4a69d6b1f`, 2026-10-04) and
`R/axioms/kernel-self-composition.form` (152 lines; last commit `78acc2a4f`, 2026-10-04); both first added in
`52712f732`, 2026-06-28 (`git log --follow --diff-filter=A`). Read whole. Status: observed.

- Host kernel: the kernel "IS an implementation of the core spec" (`host-kernel.form:62`, `:72-78`); every resource is
  "reached ONLY through a typed PORT the kernel offers", a `(direction value-shape)` pair with shapes
  `bool scalar bytes text pixel key` (`:80-91`). Any host driver or OS API is an allowed carrier under two conditions:
  "allow-its-presence" and "measure-its-health-on-the-body" (`:103-110`). The syscall ABI is the offer (`:144`); boot
  is "compose-the-kernel-from-the-axioms" (`:155`). A primitive has "0..many-host-recipes" and the cell chooses
  (`:162-168`).
- Ice in this file means compiled code: a hot composite "crystallizes into a callable native leaf, melts on cool"
  (`:28-31`) and the closing recipe names "the full gas-water-ice-back metabolism" (`:211`). The membrane carrier
  implements the leaf half only: bytes become a callable leaf at publication; nothing decides heat and nothing melts
  except release.
- Self-composition: "nothing-below-these no-primitive-is-not-one-of-these" (`kernel-self-composition.form:44`); a
  channel is "a-named-cell-offering-an-interface" (`:51-57`); boot is "primitives-then-channels-then-ports", "the first
  act is to offer a channel for input", "a kernel boots a bigger kernel it authored" (`:99-103`); growth is by "its OWN
  native binary" and by "the persistent lattice" (`:71-81`); and "the-content-digest-crosses-the-wire-not-the-local-node-id"
  (`:91`).
- The running Form counterpart is `R/form/form-stdlib/boot.fk` (117 lines): "load a .fkb image, offer a channel,
  self-extend by MINTING the next image (the boot artifact itself never mutates)" (`:1-3`). I read lines 1-45 only.
- Correspondence (inferred from the readings in section 3): the carrier is a C transcription of exactly this boot
  theorem. `thaw` = load the image; the first `offer` of the channel = publication; `offer(channel, arg)` = mint a
  larger channel and leave the old one intact (`membrane-boot.c:966-985`). The 24 cited Form files I checked all exist
  at HEAD (observed, `git ls-files`; I did not check every cited band).
- What neither spec nor the carrier states: how the first image comes to exist.

---

## 5. `bootstrap/ground*.fk` and `BOOTSTRAP.md`

Tracked files in `R/bootstrap` (observed, `git ls-files`): three, 21 lines in all.

| File | Lines | First commit | Content | Expected answer |
|---|---|---|---|---|
| `ground.fk` | 6 | `3630bd29c`, 2026-06-30 | one form, `(add 40 2)`; the comment: "This is not a flattened table seed. It proves that the freshly built fkwu runner executes Form source directly." | `42` |
| `ground-recursive.fk` | 11 | `3630bd29c`, 2026-06-30 | `(defn ground-fib (n) (if (lt n 2) n (add (ground-fib (sub n 1)) (ground-fib (sub n 2)))))` then `(ground-fib 10)` | `55` |
| `ground-numeric-list.fk` | 4 | `444238554`, 2026-07-15 | `(do (defn ground-numeric-list () (list 1 2.5 (list 3 4))) (ground-numeric-list))` | `[1, 2.5, [3, 4]]` |

- Together they exercise: reading S-expressions, integer and float literals, `defn`, a call, recursion, `if`, `lt`,
  `add`, `sub`, `do`, `list`, and value printing. Nothing else. The trailing `10` on the second command is not read
  ("direct-source Form does not yet read argv without a table entry", `ground-recursive.fk:4`, `R/BOOTSTRAP.md:81-82`).
- `R/BOOTSTRAP.md` (168 lines; last commit `afcb161af`, 2026-11-09): the bootstrap is "one C compiler invocation"
  `cc -O2 -o fkwu runtime/fkwu-uni.c` (`:7-16`); four verification commands with expected output `42`, `55`, `31`,
  the list (`:57-71`); the third is a freshness canary because "a stale binary still answers `42` while lacking newer
  evaluator capabilities" (`:78-80`); "These commands are the temporary checkout witness, not the destination ... the C
  seed shrinks toward that native walker and the per-target Form emitters" (`:50-53`); and the warning section "A
  number is not a pass": a cell with an unresolved name "still prints a plausible verdict ... while exiting 1"
  (`:84-95`). `BOOTSTRAP.md` does not mention the membrane carrier (observed, `git grep`).
- The ground files are the only artifacts both restart attempts were measured against on their first hour: the
  rewrite's receipt opens with them (`rewrite-floor.md:21-23`) and `gf_produce` compares `ground.fk` on two binaries
  (`membrane-witness.c:461-473`). A 1,574-line file passed them; that is how little they pin (inferred).
- An untracked `ground.fk.cell.fkb` (578 bytes, 2026-11-20) sits in `K/bootstrap`: a cache the old runtime writes
  beside the source it runs (ignored by `.gitignore:59`). It is the reason this study runs no `fkwu`.

---

## 6. What a new minimal C bootstrap can inherit from `membrane-boot.c`

Line numbers are `R/runtime/membrane-boot.c` unless marked `h:`. "Nearly unchanged" means the code is correct for its
stated job as read, is exercised by a passing check, and depends on nothing I list as needing replacement.

### 6.1 Nearly unchanged (about 460 lines)

| Lines | Count | What | Note |
|---|---|---|---|
| `h:10-55` | 46 | the written contract | keep as the header's text; fix the points in 3.14 |
| 94-115 | 22 | word constructors and classifier | the only encoding in either tree that maps one-to-one onto axiom 1 (`runtime-jit.md:99`) |
| 117-131 | 15 | `Outcome` constructors (`arrived_ack`, `held_out`, `ack_nothing`) | |
| 150-177 | 28 | host discovery: page size, stack depth, arch, entropy, self-pipe | |
| 384-433 | 50 | makers for scalar, bytes, text, leaf, export, port, channel | adjust to the final kind set |
| 497-502 | 6 | monotonic and realtime clock | |
| 558-584 | 27 | executable mapping with `MAP_JIT`, write toggle, icache flush | matches the contract confirmed in `critic.md` W1, R5 |
| 586-600, 612-655 | 59 | the offer callback and leaf entry (ABI 1 is the one to keep) | drop ABI 0, or keep it as the 4-integer fast path |
| 674-717, 719-749 | 75 | whole-file read; atomic write with `fsync` and `rename` | lift the 512-byte path and 1 MiB caps |
| 751-764, 766-786, 788-798 | 46 | clock reading, entropy, identity equality | |
| 800-828 | 29 | event signal and wait over the self-pipe | |
| 830-867 | 38 | process door | |
| 1008-1037 | 30 | little-endian readers and writers | |
| 1163-1195 | 33 | fault handler on an alternate stack | |

### 6.2 Keep the contract, replace the body (530 lines today)

| Lines | Count | What | Why it must change |
|---|---|---|---|
| 22-76 | 55 | fixed limits, `Cell` with `child[32]`, `Machine` with inline arrays | 3.14 limits 1 to 3 |
| 213-281 | 69 | `cell_compose` | linear dedup; canonical bytes inline the whole subtree |
| 283-382 | 100 | pin, release, accessors | slots never reused; accessors are fine but follow the layout |
| 443-495, 874-912 | 92 | carrier table and dispatch | `strcmp` chain; 3.14 item 4 |
| 504-517 | 14 | frame rows | rows live in the store they observe |
| 914-989 | 76 | `offer_inner` | publication puts 8 KB + 32 KB arrays on the stack; channel append is the only "program" |
| 1039-1162 | 124 | `parse_canon`, `freeze`, `thaw` | tree format; identity written as a second copy; table `memcpy` |

### 6.3 Drop

`Held` and `MAX_HELD` (`:26`, `:56-60`, `:68`) unless a flight table is really built; `cell_find` (`:371-376`);
`machine_arm_carriers` (`:1197-1199`); `BREACH_CYCLE`, `BREACH_DEPTH`, `BREACH_IMAGE` (`h:96-98`); the hard-coded Metal
`dlopen` (`:488-493`), which belongs behind a general door.

---

## 7. What is missing for it to stand alone, with line estimates

All estimates are **inferred**. Each names the observed size it is scaled from. Measured reference sizes:

- the carrier's own pieces (section 6 counts);
- the first rewritten seed, `P/rewrite-b21a77483-fkwu-uni.c`: S-expression reader with numbers and strings `:462-646`
  (185 lines), source file read `:647-690` (44), frame evaluator for six frame kinds without closures or recovery
  `:1191-1490` (300), 24 operations `:775-1190` (416, of which the shared-memory bus and framebuffer take about half;
  `offer_add` is 11 lines, `:775-785`), value printing `:1493-1554` (62);
- the old seed ran Form source at 1,954 lines on its third day (`critic.md` R8);
- `runtime-jit.md:449-462`, whose budget of 1,400 to 1,600 I use as a cross-check, not as a source.

| # | Missing piece | In the carrier today | Estimate (lines of C) | Reasoning |
|---|---|---|---|---|
| 1 | **Entry**: `main`, argv and environment as cells, find and thaw the seed image, publish, offer the entry export, map the ack to an exit status | none (`main` is in the witness, 10 lines) | 40-60 | witness `main` 10 + `machine_open` call sequence at `membrane-witness.c:146-157` (12) + argv to text cells + exit mapping |
| 2 | **Store that scales**: growable columns, children as a span not `child[32]`, a hash index probed by digest then compared, slot reuse | 224 lines (`:22-76`, `:213-382`) that must be replaced | 200-240 (net change about zero) | present compose + refs + accessors = 169; add open-addressing index (about 50), growth (about 30), free list (about 20), minus the inline-children simplification |
| 3 | **Identity that survives sharing**: canonical form = kind, child identities, payload; equality inside a store by interned child slots; a canonical byte stream produced by a walk when a cell crosses a boundary | canonical = whole subtree inline; FNV-1a 64 computed and unused | 40-60 for the streaming writer; plus 0 if FNV stays an index, or 100-150 if a collision-resistant digest must be in C | writer is the size of `parse_canon` (28) plus bookkeeping for visited cells. The 100-150 for a SHA-256-class function is **assumed** (not measured here). Which of the two is needed is critic N2, not settled |
| 4 | **Seed image format for a shared graph**: records in dependency order, children by back-reference, identity checked on load, written as numbers | tree-only `FBOOT3`, 124 lines (`:1039-1162`) | 110-150 binary; +60-100 for a text form (decimal numbers, one cell per line) | present freeze + thaw + parse = 124 for the tree case and is the same job; a number-and-string text reader was 185 lines in the first rewrite including floats and symbols, which a numeric form does not need |
| 5 | **Walker** (only if the seed is not machine code): literal, local, let, if, do, call, closure, tail call, offer, one recover point | none, by design (`h:31`) | 250-350 | the first rewrite's six-kind frame machine is 300 lines without closures or recovery; `runtime-jit.md:433` reports about 550 lines for 53 hot arms in the old seed (not re-verified) |
| 6 | **Walker primitives**: about 25 (integer arithmetic and compare, bits, bytes length/at/slice/concat, cell compose/kind/child/payload, word tests) | none ("There is no arithmetic in this file", `h:36`) | 200-350 | 8 to 14 lines each, from `offer_add` (11 lines) and the 416 lines the first rewrite spent on 24 operations of mixed size |
| 7 | **Doors the first compiler run needs**: stdout/stderr write and stdin read, argv/env, exit, stat/list/mkdir/rename/remove, a mode on write (an executable must be 0755; today 0600 at `:733`), large files by mapping | 11 doors, about 25 lines each (`:657-872`); none of these | 150-200 | stdio 30, argv/env 20 (shared with 1), exit 5, five filesystem doors at about 15 = 75, mode 5, mapped read 40 |
| 8 | **The universal door**: `dlopen`/`dlsym` and one call trampoline, so sockets, Metal, audio and the rest are written in the grammar | one hard-coded `dlopen` (`:488`) | 80-120 | `runtime-jit.md:456` budgets 80; the ABI-1 entry that marshals 8 words is 20 lines (`:634-653`); integer and float argument classes roughly triple it |
| 9 | **Code arena**: one reserved `MAP_JIT` region, bump allocation, leaves larger than a page, direct leaf-to-leaf calls | one page and one `mmap` per leaf (`:558-584`) | +30-50 over the 27 kept | Apple's guidance is one region (`critic.md` W1; decision 5 in `critic.md:385-386`) |
| 10 | **Held work that can arrive**: a flight table and a resume | a dead struct | 0 (drop) to 120 | not needed to run a compiler once; needed for any door that waits. Critic C3 is open |
| 11 | **Frame that does not eat the store**: a bounded ring of fixed rows outside the cell store | 14 lines inside the store | 30 | measured limit 1 in 3.14 |
| 12 | **Writing its replacement**: an executable file format and, on arm64 macOS, a signature | `file.write` only | 0 in C if the emitter is in the grammar and `ld`/`codesign` are reached through the process door; unknown otherwise | the old body's own emitter "leans on `ld` for linking and the ad-hoc signature" (`web-bootstrap-language.md:124`, `:366`; `R/form/form-stdlib/form-macho.fk:6,19`, observed) |
| 13 | **A witness for the new pieces**: cold walker = emitted code on the same cells; x86-64 leaves that are right | 31 checks, arm64 only | 150-250, in the witness file, outside the budget | present witness is 353 lines for 31 checks |

Totals (inferred; arithmetic from the table and section 6):

- Kept nearly unchanged: about 460.
- Rewritten in place, replacing the 530 lines of 6.2 (row 2 about 220, row 3 writer about 50, row 4 binary about 130,
  row 11 about 30, plus a table-driven dispatch and offer of about 150): about 580.
- New, without a walker (rows 1, 7, 8, 9): 300 to 430.
- **Standalone with a binary seed per target, no walker: about 1,350 to 1,500 lines.**
- Walker and primitives (rows 5, 6): 450 to 700.
- **Standalone with a walker and a numeric seed: about 1,800 to 2,200 lines** (add 60 to 100 for a text form of the
  seed, 100 to 150 if a strong digest must be in C).

This is higher than `runtime-jit.md:462` (1,400 to 1,600) by roughly the doors of row 7, the image work of row 4 and
the dispatch rewrite, which that table budgets at 80, 150 and 0. It sits inside `web-bootstrap`'s "under about
3,000" (`critic.md:340`).

The rewrite's history is the caution on every number above (section 2.3): 1,574 lines became 10,302 in 56 hours
because the interpreter's job was the whole body. The estimate holds only under three conditions that the old
attempts did not set: the walker runs one program (the compiler) and is never asked to run anything else; the
compiler is written against the walker's 25 primitives rather than against a 124-name registry; and a line count is
a gate from the first commit (`critic.md` decision 4).

---

## 8. What to carry forward and what to let go

Carry forward (each from an observed item above):

- The word of 3.2 and the sentence "Nothing and state 0 are different words" (`h:16`).
- "Equality is the bytes; a digest only indexes" (`h:19-20`) as the rule, with a canonical form that names children
  by identity so that sharing costs nothing (3.14 limit 2).
- `offer` as the single entry with a two-field outcome, and publication distinct from entry (`h:33-34`, `:945-965`).
- The leaf ABI in which code receives its callback as an argument and so holds no absolute address (`:639-640`).
- Ports as `(direction, shape, name)` with three distinct answers: unknown name = nothing, wrong shape = membrane 0
  plus a row, present = the door's ack (`:883-895`), and per-carrier counts of offers and nothings.
- Growth by minting: offering a channel an argument yields a new channel and leaves the old one byte-identical
  (`:966-985`).
- Thaw that re-derives identity and refuses the whole image on any mismatch (`:1119-1124`).
- A fault that is recorded and not resumed (`:1165-1178`).
- From the rewrite: its receipts' habit of naming the first stop of every door with the command that read it
  (`rewrite-floor.md:31-38`); the candidate-floor idea of 2.4, built **before** the candidate.
- From the notes: the build key is "seed digest + target" (`M/project-prebuilt-design-2026-11-09.md:12`); the output's
  file name is part of a reproducible macOS build (`M/project-fkwu-version-store-2026-11-09.md:16-17`).

Let go:

- Judging a new floor by the old body's bands and primitive registry (2.3).
- A second reader of the surface grammar in C, even "for a day" (2.3 integration receipt `:131-132`).
- A value passed as a wide struct and frames copied per argument (commit `803ce20d8`).
- An interning journal replayed by every process (commit `757ae59ca`).
- Numbered multiplexed doors whose numbers can differ between two runtimes (`M/project-rewrite-form-on-main-runtime-2026-11-08.md:19-21`).
- Fixed tables sized for a demonstration (`:22-27`), and observations stored in the store they observe (3.14 limit 1).
- A commit title that claims production when the code shells out to the old path (2.2 item 3).
- The x86 leaves and the vacuous branch check as they stand (3.13).
- Landing a stack change as one 200-file commit (2.5).
- Fixed-size path buffers (512 bytes at `:677`, `:723`, `:833`; 256 at `:838`; the old seed's 128-byte one that
  created `ck-turn`).

---

## 9. Open questions I could not settle

1. Was a full candidate-floor sweep of the rewritten runtime ever recorded (pass/short/stop counts over all bands)?
   I found only hand-run doors.
2. Why exactly was the rewritten evaluator 40 times slower: the value representation, the frame copies, the absence
   of the old seed's tiers, or allocation? The commit names frame copies and re-resolution; nobody profiled the
   remaining factor, and I ran nothing.
3. Was the membrane carrier intended as the base of the 2026-11-06 rewrite and then set aside, or were they two
   independent starts by different lanes? The commits do not say; the owner would know.
4. Is held work (arrived = 0) meant to be visible to leaf code? The carrier hides it at the ABI boundary (`:598`).
5. Should a scalar 0 or 1 and a state 0 or 1 be the same thing to a leaf (`:643-645`)? This is critic N1.
6. Does the seed image hold a walker-run compiler (numeric cells) or machine-code leaves per target? The two totals
   in section 7 differ by the walker; `critic.md` N6 leaves it open.
7. Does a self-emitted executable on arm64 macOS need a signature written in the grammar, or is calling `ld` and
   `codesign` through the process door acceptable for the first replacement?
8. What is in codex worktree 3cee ("09-04, 61 conflicts/45 vanished of 113") and `coherence-kernel-walk`? Not in my
   list; not opened.

## 10. Surprise, and what stayed uncomfortable

Most surprising: the carrier whose commit says it "produces fkwu and form-cli" never routes that production through
itself. The witness binary links the membrane, runs its 31 checks, and then a plain `fork`/`execvp` helper in the
witness file calls `cc` on the 32,016-line seed. The floor and the body share an executable, not a mechanism. The next
morning the body was rewritten by a different route, and the floor was not mentioned again.

Uncomfortable:

1. The witness is green and I could still find, by disassembling 13 bytes, that its x86-64 leaf returns the wrong
   number, and by reading six lines, that its branch check cannot fail in the way its name suggests. A passing witness
   written by the same hand as the code repeated the "numb green" the old body is leaving.
2. My line estimates are the fourth set in this study, and the only comparable real experiment (the rewrite)
   overran its starting size 6.5 times in 56 hours.
3. The 40-times figure is one measurement of one door by the rewrite's own author, on a branch nobody ran again.
4. I verified the carrier on one machine and one architecture only.

## 11. What I did not do

- I did not run `fkwu`, `bboot --produce`, any band, or any build of the old or rewritten seed.
- I did not read the 10,302-line final rewrite beyond its header, its `Value` struct and counts of its functions, nor
  the 1,259-line continuation receipt, nor `docs/kernel-interface.md` on the rewrite branch beyond lines 1-140.
- I did not open codex worktrees other than to list 4523's receipts, and did not open `coherence-kernel-walk` or
  `coherence-seed`.
- I did not re-verify the memory notes' measurements (40x, 58 MB, 12.94 s); they are quoted as dated records.
- The SHA-256 size in section 7 row 3 is assumed.
