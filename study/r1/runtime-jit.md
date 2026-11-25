# Study: the fkwu runtime and JIT (coherence-kernel, as of 2026-11-24)

Study date: 2026-11-24. Tree read: `coherence-kernel`
at commit `b8ff978c9` (2026-11-24 18:11 +0800). Short path prefix used below: `R/` = that worktree root;
`M/` = `memory/`.

Status words: **observed-run** (I ran it in this session), **observed-read** (I read the cited line; a number in it is
the source's own measurement, not mine), **inferred** (my reasoning, basis named), **assumed** (not verified).

Nothing in the repositories was modified. One scratch build of the seed and five tiny probe cells live under
`scratch/run/`.

---

## 1. What the runtime is, in one paragraph

`fkwu` is one C translation unit, `R/runtime/fkwu-uni.c` (34,149 lines, 1,634,539 bytes; observed-run `wc`), plus two
generated headers (`fkwu-node-word.h` 37 lines, `fkwu-optable.h` 302 lines). It is built with one command,
`cc -O2 -o fkwu runtime/fkwu-uni.c` (`R/BOOTSTRAP.md:14-16`). It is a tree-walking evaluator over 4-word code-node
rows with an 8-bit tag space, a tagged 64-bit value word, a compacting collector for pairs, three machine-code lanes
that all emit AArch64 into `MAP_JIT` pages, a file-image cache (`.fkb`), a host-wide shared-memory store, and roughly
six thousand lines of host doors (files, processes, sockets, HTTP/TLS, audio, camera, mesh, Windows shims). The
higher grammar (BML) is not parsed by C: it is lowered to Form text by a Form program run inside the same process, and
the C parser reads the result.

The seed's own header comment says it should not have become this: "BOUNDED -- do not grow this into a full C
flattener (that is the carrier-last inversion)" (`R/runtime/fkwu-uni.c:23281-23282`, observed-read).

---

## 2. Boot path: C `main` to running Form

All line numbers are `R/runtime/fkwu-uni.c` unless stated. Status: observed-read.

1. `main` (34114) arms the fatal-signal handlers (`fk_fatal_arm`), reads `FORM_KERNEL_STACK_MB` from `fkwu.conf`
   (default 256), creates one pthread with a 256 MB stack and runs `fk_run` on it (34118-34135). The main thread
   becomes a 1 ms sampler: `nanosleep` 1 ms, `fk_prof_sample()` (34138-34145), which feeds the time board
   (`.fkwu-time.<pid>`) and the offer-by-time JIT trigger.
2. `fk_run` (33970): `fk_seed_alias(argv)` checks whether the running binary is the version its checkout's seed
   names; if not it builds or clones the right one from `$HOME/.cache/fkwu/bin/<digest>/fkwu` and `execv`s it
   (33691-33710, 33960-33967). Then `fk_cache_arm`, `fk_fkb_builder_admit`, `fk_nodes_init` (33982-33985).
3. Dispatch on argv (34009-34050): `--check`, `--closure`, a `.fkb` image (`fk_src_load_fkb` then
   `fk_run_loaded_program_image`), a `.dylib`, a unit that needs lowering (`fk_run_bml`), else `fk_run_src`.
4. `fk_run_src` (31871): collect the unit's closure (`; preludes:`, `import`, `home-index.txt` names) into one text
   buffer `fk_srctext` (collector `fk_src_collect_*`, 27500-27800); try the content-bound `.fkb` image; otherwise
   parse with the recursive-descent s-expression parser `fk_sparse` (25018-25759) into code-node rows via
   `fk_smknode(tag, c1, c2, c3)` (24488); write the image; walk the root with `fk_walk`.
5. `fk_run_bml` / `fk_bml_bootstrap` (33187-33690): for `.bml` and section-bearing `.fk`, look up the `.lowfk` memo;
   for misses run the Form floor compiler (`form/form-stdlib/bml-floor-compile.fk`, "a plain-.fk closure of 23 units")
   as a resident cell of this process through `fk_cell_run_door` (33187-33194); write the memo; then "begin again"
   (re-exec with `FK_BML_ROUND` / `FK_BML_HELD` in the environment, 33988-34003).
6. While the walker runs, every call bumps `fk_fn_heat[c]` and calls `fk_heat_pulse` (10374-10378, 24343-24354);
   heat thresholds trigger the machine-code lanes (section 5).

Observed-run: a cold `(add 40 2)` cell answers `42` in 10-11 ms wall, steady state (first launch of a new binary
0.46 s wall with 0.01 s user, which I read as the OS's first-exec cost, inferred).

---

## 3. Value representation

### 3.1 The value word (one signed 64-bit word, banded by magnitude and parity)

Observed-read at the cited lines; the consolidated list is also in
`M/reference-fkwu-tag-space-and-closure-scratch-ordering.md` (note dated 2026-09-05).

| kind | encoding | source |
|---|---|---|
| int | any even word, value `v >> 1` (63-bit two's complement) | 941-949; `R/docs/kernel-interface.md:50-51` |
| nil / empty list | `1` | 741-742 |
| pair (list cell) | odd positive, heap index `v >> 1` into `fk_hh/fk_ht`; shared-field pairs at index >= 2^40 | 6438-6440, 499 |
| nothing | the constant `-8999999999999999999` (odd) | 743 |
| float box | odd, `<= fk_fbase - 3` with `fk_fbase = -9e18`; index into the pool `fk_fv[]` | 735, 941-959 |
| string | odd, `fk_sbase - (si<<1) - 1`, `fk_sbase = -8.5e18`; index into an intern pool | 1646-1662 |
| function value / closure | odd band at `fk_fnbase = -8e18`; indices >= 1e9 name a closure row | 745-770 |
| record | odd band just below `fk_rbase = -7e18` | 739-740, 750 |
| value-node box | "remaining negatives", index into the node columns | M note above |

Consequences observed in the notes: ints are unboxed; every float *result* allocates a pool slot and "the pool never
reclaims -- fk_fp only ever rises" (22953-22956); the band scheme was repaired at least twice after aliasing bugs
(float/int alias 2026-07-17 at 941-946; string band at 1640-1645). A function value "cannot cross a process"
(M note above).

### 3.2 Two different node stores, two more word encodings

- **Code nodes**: `static long long (*fk_node)[4]` (561): rows of 4 words `[tag, c1, c2, c3]`, tag in 0..255
  (`FK_OPCODE_ARM_CAP 256`, 665). Function table `fk_fn[c]` holds the body node index. A tag is the walker arm.
- **Value nodes** (the content-addressed cells of axiom 3): column arrays `fk_nkind, fk_ncat, fk_nkids, fk_nval,
  fk_nid` plus source attribution columns (5758-5778); interned through a hash index (`fk_intern_composite`, 16944:
  "re-interning the same cat+kids yields the SAME cell", M note). Handles are indices, so columns double without
  relocation (5649-5662).
- **NodeID word** (`R/runtime/fkwu-node-word.h:6-14`, generated by `observe/native-node-word-generate.bml`): bit 63
  mode; mode 0 = a 63-bit immediate int (coordinate 1.1.1.i); mode 1 = package 6 bits | level 13 | type 12 |
  instance 32.
- **Membrane word** (`R/runtime/membrane-boot.h:10-15`): 0 = state 0, 1 = state 1, 2 = nothing, `(slot<<2)|3` = a
  node.

Inferred (from the three citations): the runtime directory carries three incompatible 64-bit word encodings (walker
value word, NodeID word, membrane word). Only the membrane word maps one-to-one onto axiom 1's three states.

---

## 4. Node states: how ice / water / gas are actually implemented

The words are used in three different senses in the old tree (observed-read; the divergence is my inference).

1. **Where the value-node population lives** (`fkwu-uni.c:22929-22943`): "ice -- the shared field `/fg-field2-*`:
   content-addressed, shared by every kernel on the host, outlives all of them; water -- the per-pid shared store
   `/fg-c<pid>-*`: mapped and visible to siblings, gone when this pid goes; gas -- the private heap: RAM only".
   Exactly one home carries the whole population (kernel_stat 58/59/60). The stores are sparse reservations
   committed page by page: 2^26 node cells, 2^27 pairs, 2^31 string bytes, 2^26 floats (464-468); a per-pid program
   surface `/fg-c<pid>-A|S|D` exposes AST rows, source text and the defn table to other processes (470-479).
2. **Source vs cache vs result** (`R/form/form-stdlib/cache.fk:10-17`): the source file is "canonical ice", the
   in-memory recipe "water", lookup results "gas"; and in the notes ".fkb ice" means the on-disk image
   (`M/reference-fkb-ice-is-path-bound.md`).
3. **A state computed from counts**, orthogonal to substance (`R/form/form-stdlib/substrate-phase.fk:1-14`): gas 0,
   water 1, ice 2, plus a fourth "plasma" and the moves condense / freeze / melt / sublimate.

Code has its own, unnamed, temperature ladder (observed-read):

| state of a defn | mechanism | source |
|---|---|---|
| authored source | text; "Source is authoritative; the page is local cache" | `R/docs/native-jit-routing.md:5-6` |
| lowered text | `<unit>.lowfk` memo, keyed with the kernel build | 32153-32160 |
| image | `.fkb` v9, magic `FKPIFB1`: "stores an evaluator graph" | 27808, 28334; `native-jit-routing.md:48` |
| asleep | "A cell image keeps its node bytes. fk_fn[c] stays asleep until a call reaches it" (`fk_fn_asleep`, `fk_demand_*`) | 10362-10367 |
| walked (cold) | `fk_walk` / `fk_walk_body` / `fk_walk_cold` | 15597, 10981, 20786 |
| once-held constant | tag 190: first read walks, value kept in the node's own fields | 15614-15628 |
| hot, crystallized | `fk_fn[c]` replaced by a tag-194 node `(fn index, original body)`; the 194 arm runs the page, or walks the original body when argument types differ | 12322-12326 |
| dropped | `fk_f64_reset` unmaps every page at a reload | 12498-12511 |

Note the word collision: `fk_melt` is the pair-heap compacting collector (9596), not a phase change of a node.

---

## 5. The JIT: three machine-code lanes plus one image door, all through one page birth

### 5.1 How code becomes executable on macOS arm64 (observed-read, 4905-4929)

```
mmap(0, span, PROT_READ|PROT_WRITE|PROT_EXEC (0x7), MAP_PRIVATE|MAP_ANON|MAP_JIT (0x1802 on Apple), -1, 0)
pthread_jit_write_protect_np(0); memcpy(mem, words, n); pthread_jit_write_protect_np(1);
__builtin___clear_cache(mem, mem + n);
```

"THE PAGE, one birth for every lane that runs machine code ... A host door: the shrink path is none" (4905-4909).
Page size per leaf is 16 KB (`FK_F64_WORD_CAP 4096` words, 12330-12331). The membrane carrier has the same 27 lines
with `sys_icache_invalidate` (`R/runtime/membrane-boot.c:558-584`). Observed-run: a plain `cc -O2` build (linker
ad-hoc signature, no entitlements file) crystallized and ran pages on this Mac (section 7), so `MAP_JIT` needed no
extra entitlement in that configuration. Whether a hardened-runtime or notarized build needs
`com.apple.security.cs.allow-jit` is **assumed**, not verified here.

ISA coverage (observed-read): `FK_HAVE_DARWIN_ARM64_JIT_WITNESS` is defined only for Apple arm64 (2040-2041);
`FK_HAVE_CPU_IMAGE_JIT` adds Linux x86_64 for the Form-emitted CPU image door only (2043-2044, 10868-10880). The C
lanes emit AArch64 only and install only on Darwin arm64 (14434-14463). No x86_64 emitter exists in C.

### 5.2 Lane A: the C "f64 leaf" and "loop lane" (direct AArch64 emission in C)

Observed-read, 12313-15330 (about 3,000 lines, the largest single subsystem after the cold walker).

- Triggers: the float-box ledger crossing 1024 boxes for a cold defn (`fk_f64_pulse`, 12313-12326), and heat crossing
  a multiple of `FK_F64_HEAT 1024` on call and tail-call arms (`fk_f64_loop_pulse`, 10379, 10403).
- Admission (`fk_f64_admit`, 12606) builds a small typed program (`fk_f64_node` kinds 1..63: float/int literals and
  args, arithmetic, int-to-float, string byte and length, if-over-compare, call to a crystallized leaf, later
  str_find, substring, bit doors, nth, nested let, opaque words; 12480 and
  `M/project-jit-refusals-are-arms-2026-11-14.md`).
- Emission (`fk_f64_emit`, 13529-14413) writes instruction words directly: ints untagged in x10..x17, floats in
  d0..d7 with temps d16..d31, `CMP/FCMP + B.cond`, the self tail call as a parallel move plus branch, the body
  unrolled twice (12458-12479). "Only the first-seen signature crystallizes" (12475-12476).
- Limits written into the code: arity <= 6 or 8 register slots, 24 let slots, 32-step chains, 64 patches, 8 string
  literals (12329-12342, 12367). Refusals are counted by code (`-(1000+tag)`, `-(3000+tag)`; M note 2026-11-14).

### 5.3 Lane B: the Form page (the emitter is written in the language)

Observed-read, 10227-10980 and 31724-31870, plus `M/project-form-page-jit.md` (2026-10-11/12).

- Trigger: heat at a multiple of `FK_JIT_HEAT 4096` (10281), or 32 sampler hits for a seldom-called slow defn
  (`FK_JIT_TIME_SAMPLES`, 10296-10300).
- The offer runs `form-stdlib/bml/jit-page.bml` as a resident cell in the same process with `[fn, body, arity, abi]`
  (31793-31801). The cell reads the defn's nodes (`kernel_ast`, `jit_node` mirror door), writes AArch64 words as a
  list of 32-bit ints and binds them with `jit_fn_bind` (leaf-door mode 56). The defn then wears tag 194.
- Page ABI 4: `page(fp, &fk_jit_cx)`; the context struct is a table of 38 words: the value stack, `next`, and C
  helper pointers `walk, truth, tick, enter, call, tail`, then 26 one-meaning ops, then the frame chain
  (10254-10279). The page keeps values in the walker's frame so the collector sees them (10233-10235); anything it
  has no words for it hands back to the walker (`cx.next`, 10244-10245).
- Stated shrink path: "what stays in the seed is the page birth (fk_page_alloc / fk_page_write, a host door: mmap,
  write protection, icache)" (10246-10247).
- Size of the in-language emitter: `jit-page.bml` 396 lines / 33,057 bytes; `form-asm.fk` 306 lines; `form-lower.fk`
  678 lines (observed-run `wc`).
- Still walked under a page as of the note: string literals (tag 24), floats, doors (201), closures (M note).
- The C lane fires first (heat 1024 < 4096) and "keeps its leaf, because the offer skips a body already wearing 194"
  (31791-31792; M note). So the lane written in the language only gets what the C lane declined.

### 5.4 Lane C: recipe twins, and the narrow doors

- `fk_twin_pulse` binds four named stdlib recipes (`nil? append int_to_str reverse-onto`) to hand-written C
  twins "by NAME + ARITY + DEFINING UNIT" (`M/reference-len-walks-and-page-profiling.md`, 2026-09-06).
- `jit_leaf_inram` / tag 215: a structural request `(program, root, u32 argument)` that C re-admits and emits itself:
  "It never accepts a byte image" (4881-4888). `jit_cpu_run` runs a Form-emitted CPU image over the body's buffers
  (10868-10900, phase `cpu-image`).
- `register_jit` / `.dylib` execution remain as direct native operations (`R/docs/native-jit-routing.md:29-31, 68-71`).

### 5.5 What is not persisted

"transparent native pages and pure constant-hold results remain process-local ... Persisted native specialization
must carry relocatable references and any added graph nodes: emitted words can contain current function/node indices
and literal-pool words, so saving raw pages is insufficient" (`R/docs/native-jit-routing.md:48-53`). So every process
re-heats and re-emits; only the scalar demand JIT (`form-cli-jit.bml`, a sealed v5 artifact) and Metal archives
(`.metalbin`) reach disk (same file, 73-87, 109-128).

---

## 6. Memory management, stops, and choice

Observed-read unless marked.

- **Value stack** `fk_vs` (1321) holds every frame: parameters and lets at `fk_vs[fp + slot]`. Calls are
  `fk_call_value` / `fk_call_tail` (10368-10408), "one meaning for every caller", pages included.
- **Pairs**: index arrays `fk_hh/fk_ht`; `fk_melt` (9596) is a copying compaction at 90% fill with roots = value
  stack, `fk_mem`, records, nodes, closures, the framebuffer ring; growth by doubling where the collector cannot
  run (6438-6446). Because pairs move, any C arm that holds a list or string index across a child walk must push it
  first: "eleven arms ... now fk_vp the held word around the walk; any NEW arm that walks a child while holding a
  string or list must push it" (`M/reference-len-walks-and-page-profiling.md`). `fk_melt_gen` is a generation stamp
  every cached pointer must check (4893-4896).
- **Strings**: intern pool with a mark-and-free pass `fk_smelt` (9541); "Live strings NEVER move (shared-store
  readers)" (same M note).
- **Floats**: pool that only grows (22953-22956).
- **Stops and recovery**: `fk_stop` (23613) unwinds with `longjmp` to the nearest `fk_attempt` recover point, which
  restores `fk_vsp`, the frame chain and phase (23623-23640). A door "answers its value or nothing"
  (`R/docs/kernel-interface.md:122-126`).
- **Choice / backtrack**: inferred from `grep` (`oac-backtrack-walk` in `R/form/form-stdlib/bml/native-events-darwin.bml:242-262`)
  and from `M/reference-body-awk-and-path.md` ("A journal undoes a failed option's writes"): choice, fail, cut,
  store/undo are implemented in Form/BML over `attempt` and `nothing`. The C kernel has no native choice point or
  trail.
- **Deep recursion**: the walker is recursive C on a 256 MB thread stack with a depth wall
  (`fk_depth_wall`, 15598-15601; 34053-34057 records exit 127 at ~120 levels on a 1 MB Windows stack before the fix).

---

## 7. Measured performance facts

### 7.1 Mine, this session (observed-run, 2026-11-24, Apple M4 Max, 128 GB, load average 3.6, Apple clang 17.0.0)

Scratch build of the committed seed at `b8ff978c9`; cells run from a directory outside any checkout with
`FKWU_CACHE=off`, so the Form page lane (which needs `jit-page.bml` beside the binary, 31726-31753) was **not**
reachable: these numbers are the C lanes only.

| reading | result |
|---|---|
| `cc -O2 -o fkwu runtime/fkwu-uni.c` | 8.5 s wall (8.2 s user), binary 745,624 bytes, `__TEXT` 638,976, `__DATA` 33.1 MB (static tables) |
| `(add 40 2)` whole process | 10-11 ms wall, three runs |
| `fib(32)` (7,049,155 calls), walker only (`FK_NO_JIT 1`, `FK_NO_F64 1`) | 0.54 s, about 13 M calls/s |
| `fib(32)`, default lanes | 0.046-0.056 s, about 10-12x |
| 50 M-iteration self tail loop `(loop i acc)`, walker only | 2.2 s, 3.1 s, 3.6 s, 3.8 s, 4.0 s across five runs (12-23 M iterations/s) |
| same loop, default lanes (loop lane) | 0.08-0.09 s, about 550-600 M iterations/s |

The walker's own spread (2.2-4.0 s for one program) is larger than some speedups the notes report; the body's docs say
the same ("Readings carry the machine", `R/docs/resident-server.md:72-75`).

Side observation (observed-run): with `FKWU_CACHE=off` a 78-byte source still left `fib.fkb` (1,215 bytes),
`fib.sym` (348 bytes), `fib.fkb.make_ms`, and `.fkwu-time.<pid>` / `.fkwu-heat.<pid>` boards in the working directory.

### 7.2 The body's own receipts (observed-read; numbers are the sources')

| fact | source, date |
|---|---|
| ~48 M dispatches/s on a trivial body | `M/project-per-recipe-jit-slice.md`, 2026-09-01 |
| loop lane: int 20 M iterations 0.63 s -> 0.01 s; float 0.80 s -> 0.03 s | same, 2026-09-06 |
| arm64 leaf door = 1.2x walker on 400k calls; "0 of the hot worklist coverable" | same, 2026-09-02 |
| once-hold for top-level lets: cold `.bml` unit compile 5.51 s -> 1.54 s | same, 2026-09-02 |
| `nil?` peephole: x100K on a 10K list 300 -> 6 ms; `append` x200 4677 -> 161 ms | `M/reference-len-walks-and-page-profiling.md`, 2026-09-06 |
| a 59,464-byte door -> 201,998 bytes lowered -> 38,391,347-byte `.fkb`, "645 times its source", closure of 351 units | `R/docs/bml-direct-nodes-jit.md:5`, 2026-11-13 |
| cold build 0.37 s per MiB of image (12.08 s for 32 MiB); warm read of the same image 0.18 s; a trivial door importing 12 planner units 7.63 s cold | same, line 6 |
| image written "at 9 bytes a value"; C parse about 17% of beats | same, line 7; 27866-27879 |
| per-recipe emit: text printer 15 us, per-node `jit_node` door 116 us (7.7x slower) | same, line 26 |
| cold floor compile of the planner closure 25.9 s -> 8-10 s after the opaque-word lane | `M/project-jit-refusals-are-arms-2026-11-14.md` |
| `bml-source-fingerprint` over 1.9 MB: 2416 ms -> 5 ms once it crystallized | same |
| `-O2` chosen: O3 +7% cold, Os +16%; kernel 711 KB, 3.6 s cc; builds byte-reproducible | `M/project-prebuilt-design-2026-11-09.md` |
| fresh host with a warm pack: 54 s -> 4 s | same |
| resident dispatch 2.5 ms -> 32-50 us; cold admission 495 ms vs 30 ms warm | `R/receipts/2026-11-04-optimization-book.md:266-267, 508` |
| GPU serving: 15 tok/s plain, 20-21 tok/s with self-drafting on code (27B model, exact association) | `R/docs/resident-server.md:98-103` |

---

## 8. Size breakdown of the C

Method (inferred from observed line ranges): I listed 1,025 function starts with `grep -n`, bucketed them into
500-line windows, and assigned contiguous line ranges to subsystems by the functions and banner comments in each.
Boundaries are approximate to about +-150 lines per class; globals and comments count with the code they precede.

| class | approx lines | share | main ranges |
|---|---|---|---|
| Host doors (files, dirs, processes, sockets, HTTP/TLS, mesh, sensors, mic/camera/speaker, tty, Windows shim) | 5,850 | 17% | 1-250, 3179-4880, 5851-7384, 7588-9000, 11372-12312 |
| JIT in C (page birth, u32/in-RAM leaves, Form-page host and mirror door, f64 leaf, loop lane, offers) | 4,950 | 14.5% | 4881-5648, 10227-10980, 12313-15330, 31724-31870 |
| Images, lowering memo, stores, cache ledger, seed version store, SHA-256 | 4,800 | 14% | 27798-30574, 32153-34149 |
| Shared-memory field, gift, owned values, live page, roster, stage bus, framebuffer, admission | 4,150 | 12% | 16149-20785 (minus the fatal organ) |
| Evaluator (three walkers, ops, call/tail, len-compare) | 3,900 | 11.5% | 10981-11371, 15333-16148, 20786-23297 |
| Reader side (op tables, node builder, parser, prescan, closure collector, preludes, home index) | 3,400 | 10% | 24407-27797 |
| cell_run, diagnostics, organ signals, profiler and boards, fatal-signal organ | 3,350 | 10% | 251-900, 18018-18400, 23298-24406, 30575-31723 |
| Values and memory (bands, float pool, strings, intern index, records, methods, node table, melt, printing) | 3,100 | 9% | 900-2550, 5649-5850, 9000-10226 |
| GPU doors (Metal dylib resolver and wrappers, CUDA driver API) | 820 | 2.4% | 2555-3178, 7385-7587 |

Arms (observed-run `grep -c`): `fk_walk` 53 top-level `if (t == N)` arms, `fk_walk_cold` 183, `fk_walk_body` 11;
55 `mode == N` tests for the multiplexed leaf door; the op table has 287 rows (223 ops, 64 rewrite rows by position,
`R/runtime/fkwu-optable.h:7-302`).

Outside the seed: the Metal carrier `R/form/native/metal/fk-metal-carrier.m` is 2,326 lines of Objective-C, loaded
with `dlopen` (`fk_metal_dyn_*`, 2555-2740); the `gpu/` directory holds one file, a 1,515-byte PTX matvec
(`R/gpu/fptx-matvec.ptx`), and no README. GPU kernels are MSL source strings written in Form and compiled by the
driver at run time (`R/docs/native-jit-routing.md:109-128`).

Growth (observed-run `git cat-file | wc -l` at the last commit before each date): 160 lines at birth
(`c02dae6b4`, 2026-06-28); 1,954 on 2026-07-01; 13,647 on 2026-08-01; 13,811 on 2026-09-01; 25,570 on 2026-10-01;
31,066 on 2026-11-01; 34,149 on 2026-11-24. 400 commits touch the file.

---

## 9. Caches and their identity

Observed-read unless marked.

| cache | identity | where | notes |
|---|---|---|---|
| kernel binary | FNV digest of the three seed files + flags; SHA-256 seal beside it | `$HOME/.cache/fkwu/bin/<digest>/fkwu`; `./fkwu` is a copy-on-write clone | 33691-33710; build is byte-reproducible only with a fixed file name and layout because "macOS signs a binary with its file name as the identifier" |
| lowering memo `.lowfk` | source digest + kernel build | beside the unit and in a shared store | 32153-32160; `M/project-fkwu-version-store-2026-11-09.md` |
| image `.fkb` v9 | builder id (this binary's bytes; on macOS the loaded image UUID + text fingerprint) x repo-anchored path id x v4 source identity (ordered paths, sizes, content digests of the closure) | beside the unit and `$HOME/.cache/fkwu/fkb/<key>` | 28246-28262; `R/docs/native-jit-routing.md:58-66`; `R/docs/native-core-care.md:33-37` |
| `.sym`, `.make_ms` | sidecars of the image | beside the unit | observed-run |
| JIT pages | none: process-local RAM | per process | `native-jit-routing.md:48-53` |
| scalar demand JIT artifact | SHA-256 sealed admission binding builder, source closure, emitter ABI, one checked argument/result | disk | same, 73-87 |
| Metal archive `.metalbin` | source / entry / target identity + seal | disk | same, 109-128 |
| warm packs | seed digest + OS-arch; manifest SHA-256 pinned in git | a mirror outside git | `R/prebuilt/index.txt:1-7` |
| value nodes | content (category + children) | shared memory field | 16944; 22934 |

Known identity defects recorded in the notes: every rebuild changes the builder id so images are refused and
recompiled (`M/reference-fkwu-import-lane-vs-flat.md`); lowering bytes are "NOT deterministic even on one host"
because a lambda name embeds an allocation-dependent id (`M/project-prebuilt-design-2026-11-09.md`); a stale binary
"writes ices its own reader refuses ... on EVERY run" (`M/reference-fkb-cache-cross-binary-pollution.md`); image
keys are 64-bit FNV-1a, not a cryptographic hash (28257-28259).

---

## 10. Hard-nosed list

### 10.1 What made it large and fragile

1. **The unit of caching is the closure, not the recipe.** One edit rebuilds a 38 MB image at about 3 MB/s
   (`R/docs/bml-direct-nodes-jit.md:5-8`). "Strings are the seams between stages and also the cache keys, so one edit
   invalidates a whole closure while the unit of meaning is the recipe."
2. **Two grammars, joined by text.** BML -> grammar templates -> Form text -> C parse -> nodes -> image
   (same doc, line 7), with a re-exec between lowering and running (33988-34003).
3. **Every primitive exists several times.** Walker arm, `fk_op_*` shared op, C-lane emitter kind, Form-page op
   slot, mirror table (`fk_jit_plain_kids`), image arity table (`fk_fkb_node_arity_for_tag`), flattener row, manifest
   row. Recorded drift: a node field past the arity table "keeps pointing into the FIRST image's node range ...
   silent" (`M/reference-fkwu-import-lane-vs-flat.md`); the Go kernel's JIT mirror reverting a healed `len`
   (`M/reference-jit-abi-mirrors-natives.md`).
4. **Hot and cold semantics diverged.** Call-by-name inlining dropped the effects of ignored arguments past 1,024
   calls: "3,936 of 780,000 effects" landed (`M/reference-hot-recipe-inline-drops-arguments.md`, 2026-10-03). On
   2026-11-24 a closure over locals "goes wrong at its 1024th call" and "The kernel fault itself is not repaired"
   (`M/reference-hot-closure-over-locals-stops-at-heat-mark.md`).
5. **An 8-bit tag space that filled.** New operations are multiplexed through modes of one tag (`float_leaf` 201,
   modes into the 60s) and integer keys of `kernel_stat`; a mistaken tag "silently hijacks that op's real walker arm"
   (`M/reference-fkwu-tag-space-and-closure-scratch-ordering.md`).
6. **One translation unit with hand-declared libc and global state.** Two tentative definitions of one name merged
   silently and every run died with rc 139 (`M/reference-fkwu-static-name-merge.md`); a shared scratch channel for
   closure captures was clobbered by nested calls (tag-space note, 2026-09-03).
7. **A moving collector under C arms that hold indices.** Each new arm must remember to root what it holds (section 6).
8. **Type-banded value words chosen by magnitude.** Two aliasing repairs; floats boxed and never reclaimed.
9. **The seed absorbed the operating system.** 17% host doors, including Windows Media Foundation camera capture and
   a CUDA driver path, inside the file whose header calls it a one-time bootstrap.
10. **Fixed caps inside the C JIT** (8 register slots, 6 parameters, 24 lets, first-seen signature only) turned into
    a long tail of refusal codes, each needing another C arm (`M/project-jit-refusals-are-arms-2026-11-14.md`).
11. **JIT output is not relocatable**, so nothing native survives the process (section 5.5).
12. **Cache litter and identity churn**: sidecars beside every source, per-pid boards (7,329 of 7,403 entries in a
    worktree root on 2026-09-12, `M/reference-fkwu-heat-boards-burial.md`), 5.1 GiB of dead images per checkout
    (`M/project-fkwu-version-store-2026-11-09.md`).
13. **One reader left.** With the Go, Rust and TS kernels retired, "a value fkwu gets wrong ... has no second reader"
    (`R/CURRENT_FLOOR.md:64-68`).

### 10.2 What proved excellent

1. **One-command, dependency-free build**: 8.5 s, 745 KB, byte-reproducible (observed-run; M prebuilt note).
2. **Tag 194 as the dispatch idiom**: a crystallized defn is just another node the walker already reads, with the
   original body kept as the fallback (12322-12326). Zero dispatch cost, graceful decline.
3. **One page birth for every lane** (4905-4929): about 20 lines is the whole W^X story.
4. **The page ABI as a numbered helper table** (`FK_JIT_ABI`, 10250-10254): an emitter written for another layout
   declines instead of miscompiling.
5. **The emitter in the language works**: 396 lines of BML emit AArch64 for hot recipes, in-process, on demand.
6. **Heat and time as triggers, with a live profiler**: per-defn heat, a 1 ms sampler, the time board, the live page
   another process can read (`kernel_page_hot`), refusal codes that name the missing arm.
7. **Content-bound image identity** (paths, sizes, digests, builder) replacing mtimes, with embedded compile errors
   and a checksum (`native-jit-routing.md:58-66`).
8. **Sealed, self-healing kernel store** (33691-33710).
9. **Cells in process**: `cell_run` answers a record `{value, out, diag, stopped, errors, resident, ms, cpu_us}`
   with a deadline and a name window, replacing process spawns (30575-30597).
10. **Stops as backtrack points; doors answer value-or-nothing with a reason word** (`kernel-interface.md:122-126`).
11. **Handles as index + generation** (same doc, 131-135).
12. **Kernel laws pinned hot and cold by one band** (same doc, 50-82).
13. **The fatal-signal organ**: a dying process writes one structured row with the Form frame chain (18018-18037).
14. **The op table generated from one source** (`fkwu-optable.h:1-5`).
15. **GPU as a dynamically admitted carrier**: programs are source generated in the language, compiled in RAM,
    archived by identity; paged and contiguous kernels equal byte for byte (`resident-server.md:39-46`).

---

## 11. The greenfield carrier already in the tree: `membrane-boot.c`

Observed-read. `R/runtime/membrane-boot.c` (1,199 lines) and `membrane-boot.h` (169 lines), landed 2026-11-05
(`16b3d3eb6`), with a 510-line witness. Its header states the design the restart is asking for:

- "The carrier exists before a parser, a language, or a model. It implements the five axioms on this hardware and
  nothing else" (`membrane-boot.h:5-8`).
- Word: 0, 1, nothing, node (10-15). Cells immutable, identity = canonical bytes of kind + children + payload; "A
  digest of those bytes only indexes the store. Equality is the bytes" (17-23).
- "offer is the only act" (25-29). "C does not evaluate a recipe ... A leaf is bytes the image supplied ... There is
  no arithmetic in this file" (31-37).
- Ports are `(direction, shape, name)`; carriers present: file read/write, clock, entropy, identity-eq, event
  wait/signal, process, GPU admit, cpu-leaf (`membrane-boot.c:657-912`).
- `freeze` / `thaw` of the channel graph with identity check (`membrane-boot.h:46-49, 158-159`).
- Leaf ABI 0: `int64 f(int64 x4)`; ABI 1: `uint64 f(const uint64 *words, uint64 n, OfferFn)` so a leaf can call back
  through `offer` (`membrane-boot.c:612-655`).

Its limits, also observed-read: fixed `MAX_CELLS 8192`, `MAX_CHILDREN 32`, leaf <= one page (`membrane-boot.c:22-26,
562`); and its "produce" proof compiles `fkwu-uni.c` by spawning `cc` through the process port
(`R/observe/greenfield-produce-run.bml:11-12`, commit message). It carries machine-code leaves but nothing that turns
the language into those leaves without the old 34k-line seed. That gap is the bootstrap problem.

---

## 12. The smallest bootstrap: a grounded estimate

Status of this whole section: **inferred**, from the sizes observed above (fkwu at day three ran Form source in
1,954 lines; the membrane carrier is 1,199 lines with no evaluator; the page birth is 20-27 lines; `fk_walk`'s 53 hot
arms are about 550 lines).

The one unavoidable question: something must run the compiler the first time. There are only three ways, and they
set the size.

- **(a) Seed interpreter.** The repo stores the compiler as source in the one grammar; the bootstrap contains a
  reader and a slow walker for a core subset, runs the compiler once to emit native code for itself, then never
  walks again on that host.
- **(b) Seed image.** The repo stores the compiler already compiled (machine-code leaves per ISA) and the bootstrap
  only thaws and enters it (the membrane design). Smallest C, but a binary blob per target lives in git and cannot be
  regenerated without a previous build.
- **(c) Seed as numeric cells.** The repo stores the compiler as a canonical, diffable serialization of cells in the
  numeric space (no surface syntax), and the bootstrap contains a thaw plus a walker over the handful of core node
  kinds. No parser in C at all; the one grammar's parser is itself a cell program in the seed. This fits "symbols are
  a replaceable lens" and keeps the seed regenerable and reviewable.

Irreducible pieces under (c), with a line budget in C:

| piece | why it cannot be in the language | budget | evidence |
|---|---|---|---|
| entry, big-stack thread, argv | the OS enters C | 40 | 34114-34148 |
| page birth: `mmap(MAP_JIT)`, write-protect toggle, icache flush, munmap | host fact; per-thread on Apple arm64 | 30 | 4910-4929; `membrane-boot.c:558-584` |
| minimal host doors: open/read/write/close, clock, exit, stderr | needed before any FFI exists | 80 | `membrane-boot.c:674-786` |
| the universal door: `dlopen`/`dlsym` + one call trampoline (N integer and M float args) | lets every other door (sockets, processes, Metal via the Objective-C runtime, audio) be written in the language | 80 | the Metal carrier's own comment: "Form-emitted host-ABI calls can replace this adapter" (`fk-metal-carrier.m:21-24`) |
| word + cell store: three states and node, content addressing by canonical bytes, growable columns | axioms 1-3; must exist before any code runs | 300 | `membrane-boot.c` cells, hash, compose, release |
| thaw / freeze of a canonical cell image with identity check | loads the seed | 150 | `membrane-boot.c:1008-1160` |
| core walker: literal, slot, let, if, do, call, tail call, closure, offer, a recover point; ~25 primitive ops (int arithmetic, compare, bits, bytes, cell compose/child/kind) | runs the compiler once | 600-800 | `fk_walk` 550 lines for 53 arms; fkwu ran source at 1,954 lines total |
| bump allocator with growth; no collector in C | the collector is language code once native | 60 | 6438-6446 |
| fault handler writing one row on an alternate stack | must be async-signal-safe C | 60 | 18018-18037: "what stays in C is the handler and the one row it writes" |
| **total** | | **about 1,400-1,600 lines** | |

Under (a) add a reader for the one grammar: 250 lines if the seed is kept in a deliberately tiny kernel notation,
far more (fkwu's reader side is about 3,400 lines) if C must read the full surface. Under (b) drop the walker and
land near 800-900 lines, at the cost of binary seeds in git.

Everything else observed in fkwu can leave C: the parser, lowering, both JIT lanes, the image format and cache
identity, the collector, strings and floats, records and methods, cell_run, diagnostics, profiler, every host door
beyond the minimal six, the GPU carrier, the scheduler. The Form page's stated shrink path already says so
(10246-10247).

Two design consequences that follow from the fragility list:

- The walker in the bootstrap must be the **only** second implementation of semantics, and it should be checked
  against the native compiler by the same law band hot and cold (the pattern at `kernel-interface.md:67-82`), or
  better, retired from the running process once the compiler is native so there is no cold/hot pair at all.
- Native code must be **content-addressed per recipe and relocatable** from day one (the destination written at
  `bml-direct-nodes-jit.md:10`), so cold differs from warm only by reading pages by id.

---

## 13. Bootstrap language: what this study contributes (C vs Go vs Rust)

- **Observed**: the C seed builds in 8.5 s with no dependency beyond `cc`, on macOS and Linux (arm64 and x86_64)
  byte-reproducibly, and the page birth is a direct libc call.
- **Observed-read**: the retired Go kernel crystallized "via a Go plugin" and carried every hot primitive twice
  (`jit.go` + `jitabi`), which drifted (`M/project-per-recipe-jit-slice.md`, `M/reference-jit-abi-mirrors-natives.md`).
- **Assumed** (general knowledge, not verified in this session): `pthread_jit_write_protect_np` is per-thread, so a
  Go bootstrap must pin the OS thread and reach it through cgo or assembly; JIT code cannot safely run on a movable
  goroutine stack; Go's runtime owns signals and its own collector, both of which a self-hosting runtime wants to own.
  Rust can do everything C does here with `unsafe` and `libc`, at the cost of a toolchain dependency and slower
  builds, and its safety guarantees stop at the JIT boundary, which is where this system lives.
- **Inferred**: for a bootstrap whose job is to be about 1,500 lines, to call five libc functions, and then to be
  replaced by code the language emits, C is the smallest and most portable choice; the fragility recorded in section
  10 came from growing the C, not from C.

---

## 14. Open questions

1. Does a hardened-runtime or notarized build need the allow-jit entitlement for `MAP_JIT` on current macOS?
2. What is the Form page lane's steady-state speed against the C lane on the same recipe? I could not run it without
   writing caches into a checkout.
3. Where exactly is the closure-over-locals fault at the 1,024th call (2026-11-24 note)? Unrepaired, and it is in the
   lane the new design would not carry.
4. Can the collector, written in the language and JIT-compiled, find roots in JIT frames without the walker's frame
   convention (10233-10235)? The old design never tried: pages keep values in the walker's stack.
5. How are choice points, undo and store meant to interact with native frames and with content-addressed cells?
   The old kernel has only `setjmp` recovery; journals live in the language.
6. Which of the three ice/water/gas meanings is the one the new graph should fix as the definition?
7. Is a seed stored as canonical numeric cells acceptable as "bootstrap code" in git, and who regenerates and
   reviews it?
8. Relocation model for persisted native code: position-independent leaves with a literal pool by node id, or
   re-link at load?

## 15. Surprise

The tree already contains its own answer and its own contradiction side by side. `membrane-boot.c` states "C does
not evaluate a recipe ... There is no arithmetic in this file" in 1,199 lines, while its only end-to-end proof is to
shell out to `cc` on the 34,149-line seed whose header says "BOUNDED -- do not grow this". And the lane written in
the language (396 lines of BML) is shadowed in practice by the 3,000-line C lane that fires first at heat 1,024 and
keeps the defn. What stayed uncomfortable: the two hot-path miscompiles I read about are both on the C lane, one of
them dated today and unrepaired, and my own timing spread on the plain walker (2.2-4.0 s for one program) is wide
enough that several small speedups in the receipts cannot be told from weather.
