# Gap G5: the emitter written in the language, read whole

Written 2026-11-24. Closes gap G5 of `critic.md` ("The emitter written in the language was counted, not read").

## 0. Method, paths, status words

- `R` = `coherence-kernel` (HEAD `b8ff978c9`, 2026-11-24).
- `M` = `memory`.
- `S` = `scratch`.
- **observed** = I read the cited line or ran the cited command in this session. **inferred** = reasoning from observed
  things, basis named. **assumed** = not verified.
- I did not run `fkwu`. I modified nothing in `R`. I wrote one probe directory, `S/emitter-check/` (a C file that
  transcribes the encoder formulas and the Mach-O byte layout, then Apple's `cc`, `otool`, `ld`, `codesign` read the
  result). Results of that probe are marked "observed (transcription)": they check my transcription of the recipe's
  arithmetic against Apple's tools, not a run of the recipe itself.
- Read whole: `R/form/form-stdlib/bml/jit-page.bml` (396 lines), `form-asm.fk` (306), `form-macho.fk` (74),
  `form-lower.fk` (678), `bml/anf-lower.bml` (220), `bml/native-jit-route.bml` (20), `form-cli-jit.bml` (92),
  `form/native/metal/sha256-arm64-jit.fk` (138), `tests/jit-page-band.bml` (104), `tests/form-macho-band.fk` (40),
  `docs/bml-direct-nodes-jit.md` (27), `docs/native-jit-routing.md` (141), `CURRENT_FLOOR.md:1255-1390`, the five memory
  notes named in the task plus `M/reference-hot-recipe-inline-drops-arguments.md`, and in `runtime/fkwu-uni.c` lines
  736-835, 1655-1678, 4881-4930, 5242-5447, 10226-10800, 10855-10979, 11055-11085, 15380-15569, 31718-31870.
- Read in part only (headers, first 30 lines, or grep): `jit-once-born.fk`, `bml/anf-shadow.bml`,
  `bml/body-fingerprint.bml`, `jit-tensor-emit.fk` (to line 92), `tests/jit-leaf-inram-band.fk`,
  `receipts/2026-11-04-optimization-book.md:205-294`. Not opened: `bml/bml-demand-jit.bml` (799 lines),
  `form-lower-sim.fk`, the C loop lane `fk_f64_*` (grounded by `S/runtime-jit.md` 5.2; I grep-counted only).
- Builds on, and does not redo: `S/runtime-jit.md` section 3.1 (value word table), 5.1 (page birth), 5.2 (C lane),
  5.3 (the Form page, at the level of "what it is"), 7 (timings); `S/lessons.md` RC2; `S/critic.md` R5, R7, R14.

## 1. The finding in brief

1. There is not one in-language emitter. There are **three in-language AArch64 writers plus one object-file writer**,
   and they share no encoder code with each other or with the C lane (section 2). The one the critic calls the
   "working ancestor" (`jit-page.bml`) is the only one wired to heat; it carries its own private instruction table.
2. `jit-page.bml` is a **partial, per-node compiler that never refuses a defn for containing something it cannot
   say**. Each node is either written in machine words, sent to a shared C function ("one meaning"), split into
   children plus a walker "mirror", walked in place, or handed back to the walker in tail position. That fallback
   ladder is the design, and it is why a 396-line emitter was usable on real work the day it landed.
3. Its value model is the walker's own tagged word, unchanged. Its frame is the walker's own value-stack frame. No
   value lives in a register across a helper call. That is what makes it safe under a moving collector with no stack
   maps, and it is also why it is slow relative to the C loop lane (every temporary is a memory slot).
4. The host contract is a **numbered table of 38 words** (ABI 4), a bind door, a node-reading door and a "mirror"
   door. The versioned table is the best idea in the set; the mirror door is the part to drop.
5. `form-macho.fk` writes an **object file** (`MH_OBJECT`), not an executable, unsigned. Apple's `ld` makes it runnable
   and ad-hoc signs it. I reconstructed the 251-byte object from the recipe's layout and it linked, ran (exit 40) and
   carried an ad-hoc linker signature (section 10). The demo script the file cites is no longer in the tree.
6. No wrong answer from `jit-page.bml` itself is on record. Every recorded miscompile belongs to the C loop lane, to
   `form-lower.fk`, or to a mirrored second implementation (section 12). The fault of 2026-11-24 fires at call 1024,
   which is the C lane's heat mark, not the Form page's 4096 (inferred; root cause still unknown).

## 2. The map: who writes machine code in the old body

| writer | file, size, last commit | emits | value model | host door | wired to heat |
|---|---|---|---|---|---|
| A. the transparent page | `R/form/form-stdlib/bml/jit-page.bml`, 396 lines, 10 commits 2026-10-11..2026-11-12 | list of 32-bit instruction words (ints) | walker's tagged word, walker's frame | `jit_fn_bind` (leaf-door mode 56), `jit_node` (mode 57), `kernel_ast` (tag 32) | yes: `FK_JIT_HEAT 4096`, or 32 time samples |
| B. the leaf compiler | `form-asm.fk` 306 lines (13 commits, last 2026-10-12) + `form-lower.fk` 678 lines (last 2026-11-08) | list of little-endian bytes | untagged 32-bit ints in `w` registers, or raw pointers | `jit_leaf_inram(image, arg)` (tag 146) | no: only by explicit call (`form-cli-jit.bml:47`, `native-jit-route.bml:16-17`, `anf-shadow.bml`) |
| C. CPU organ images | `form/native/metal/sha256-arm64-jit.fk` 138 lines; `bml/body-fingerprint.bml` (17 instructions) | byte string built from `form-asm.fk` rows | raw buffers | `jit_cpu_run([image, buffers, consts])` (mode 58) | no: called as a library |
| D. the object wrapper | `form-macho.fk` 74 lines (6 commits, last 2026-09-30) | byte list of a Mach-O `.o` | n/a | none; `ld` outside | no |
| E. source-text emitters | `jit-tensor-emit.fk` 401 lines; `form-cli-jit.bml:39-42` | Rust, MSL, CUDA source **strings** | n/a | Metal / rustc carriers | no |
| (C lane, for contrast) | `runtime/fkwu-uni.c` `fk_f64_*` | words via `fk_f64_put(words, wn, 0x...)`, 380 call sites (observed: `grep -c`) | untagged ints x10..x17, floats d0..d7 | internal | yes: `FK_F64_HEAT 1024` |

Sources (observed): op table rows `R/runtime/fkwu-optable.h:119,157,297-299`; `jit-page.bml:30` imports only
`form-stdlib/core.fk`; `form-lower.fk:28-29` imports `form-asm.fk`; `sha256-arm64-jit.fk:14`; `git log` per file.

Three consequences (inferred from the table):

- **Three encoders for one ISA.** `jit-page.bml:42-78` (decimal word bases), `form-asm.fk:38-300` (byte rows through a
  generic packer), and the C lane's hex words. `jit-page.bml` does not import `form-asm.fk`; the instructions both
  need (`ldr`, `str`, `add`, `mul`, `sdiv`, `movz`, `movk`, `b`, `b.cond`, `eor`) are written twice with the same base
  numbers (for example 4181721088 at `jit-page.bml:42` and `form-asm.fk:183`).
- **Two value models.** A is exact with respect to the walker (63-bit tagged ints). B computes in 32-bit registers and
  sign-extends at the end (`form-lower.fk:243-254`), so B answers "the walker's modulo 2^32, read signed" outside
  [-2^31, 2^31-1] (`tests/jit-leaf-inram-band.fk:12-15`: `f(2000000000)` answers 1705032711 where the walker says
  6000000007).
- **The in-language page gets only what the C lane leaves.** The C lane fires at heat 1024 and "keeps its leaf, because
  the offer skips a body already wearing 194" (`fkwu-uni.c:31791-31792`, `CURRENT_FLOOR.md:1381-1384`).

Correction to `S/runtime-jit.md` 5.4 (observed): "It never accepts a byte image" (`fkwu-uni.c:4883-4886`) describes
tag 215 (`jit_arm64_u32_leaf`). The sibling door `jit_leaf_inram` (tag 146) does take a raw byte list:
`fk_jit_leaf_inram_image` checks only that the list holds ints 0..255 and that the length is a multiple of 4, then maps
and calls it (`fkwu-uni.c:5269-5289, 5385-5447`). `jit_cpu_run` takes a raw image string up to 1 MiB
(`fkwu-uni.c:10883`). So the seed has two raw machine-byte ingress doors with no content check.

## 3. `jit-page.bml`: what it compiles and what it declines

The emitter's dispatcher is `jp-emit` (`jit-page.bml:134-138`), falling through to `jp-op` (223-225). A code node is
four words `[tag, c1, c2, c3]` read with `kernel_ast(host_pid(), k)` (line 34). Every case below is observed at the
cited line; tag names are from `R/runtime/fkwu-optable.h` and the file's own header (lines 10-29).

### 3.1 In the page's own words (machine code, no helper on the int path)

| node | tag | emitted | line |
|---|---|---|---|
| int literal, magnitude below 2^61 | 1 | `movz`/`movn` + up to three `movk` of `2n` | 137, 85-90, 122 |
| `(empty)` (nil) | 18 | `movz x0, #1` | 137 |
| string literal | 24 | its pinned constant word as four 16-bit halves (`jit_node ["word", k]`) | 140-143 |
| parameter 0 | 2 | `ldr x0, [x21, #off]` | 137 |
| parameter or let read | 110 | `ldr x0, [x21, #(slot+off)*8]` | 173-176 |
| `let` | 109 | value, `str x0, [x21, #slot]`, body | 177-185 |
| `do` | 69 | first, then second | 186-190 |
| `if` | 6 | `cbz`; `cmp #2`; `b.eq`; else call `truth` | 193-206 |
| `add sub mul` | 3 4 42 | guard `orr x9,x1,x0; tbnz x9,#0`, then one or two instructions | 256-277 |
| `div mod` | 10 11 | guard plus `cbz x0` (zero divisor), `sdiv` (and `msub`) between untag and retag | 257, 259-262 |
| `band bor bxor` | 34 35 36 | one instruction on the tagged words | 257 |
| `le lt eq` | 5 103 102 | `cmp; cset; lsl #1` (answers 0 or 2); `eq` also takes an identity path when either side is nil | 257, 263 |
| `shl_u32 shr_u32 rotr_u32 add_u32` | 37 38 39 40 | untag both, one 32-bit instruction, retag | 257 |
| `bnot_u32` | 41 | `asr; mvn w; lsl` | 234-238 |
| `value_eq` / `node_eq` | 80 | same word is equal; two different even words are unequal; any other pair goes to the walker's arm over a mirror | 150-165 |
| self tail call | 12 240 241 in tail position naming the defn itself | arguments to temporaries, moved to slots 0..a-1, `tick`, branch to the loop top | 303-306, 365-370, 373 |
| call to a small callee | 12 240 241 | the callee's body written in place of the call, two deep, at most 768 words; the frame chain is pushed and popped by the page's own words (11 instructions plus a constant load and a store on entry, 5 plus the same on leaving) | 311-358 |

### 3.2 Always through a helper: "one meaning" (the same C function the walker's arm calls)

`head` 20, `tail` 21, `len` 22, `str_len` 25, `byte_to_str` 33 (unary); `str_eq` 26, `str_byte_at` 28, `cons` 19,
`nth` 23 (binary); `value_kind` (leaf door 201, mode 4). Lines 212-214, 226-232, 239-253. And every op of 3.1 when an
operand is not an int, or the divisor is zero: the slow path swaps the operands back and calls the op's table word
(line 272).

### 3.3 Calls

A call that is not a self tail call and not written in place evaluates its arguments left to right into temporaries
`t .. t+n-1`, then calls helper 6 (`call`, value position) or helper 7 (`tail`, tail position; the helper answers the
node the walker continues at, the page stores it in `cx.next` and returns). Lines 359-364, 108.

### 3.4 "The mirror": strict nodes the seed names

For any other tag the page asks the seed `jit_node ["kids", k]` (line 147). If the seed names the node's value
children, the page evaluates each into a temporary in its own words, asks `jit_node ["mint", k, t]` for a **new
walker node** whose children are slot reads, and has the walker run only that mirror (lines 146-172). The table is in C
beside the walker (`fkwu-uni.c:10535-10546`): `record_get` 65, `record_set` 66, `record_has` 67, `record_keys` 99,
`value_eq` 80, `node_category` 92, `str_find` 30, `intern_node` 47, `intern_trivial_string` 46, `intern_trivial_int`
43, `node_children` 48, `node_value` 49, `file_size` 60, `host_file_append_bytes` 61, `read_file` 63, `write_file` 104,
a `str_concat` chain 27, every leaf door 201 except `attempt` (mode 28), and a compare against `(len X)` with a literal
(so the walker keeps its bounded `len`, `jit-page.bml:216-224`).

### 3.5 Declined, per node (never per defn)

Anything else is **walked in place** in value position (helper 2, `walk(node, fp)`) or **given back** in tail position
(`cx.next = node`; the 194 arm continues there). Observed members of this set, by tracing `jp-op` (line 224) against
the seed's table: a float literal (tag 53), a lambda or capturing closure, an indirect call (244), the call forms 44
and 7 (they only block in-place inlining, line 336), `attempt`, `nothing()`, a once-held constant (190), `record_new`,
an int literal at or beyond 2^61 (line 137), a slot at or beyond 4096 (lines 123-125). Per `M/project-form-page-jit.md`
(2026-10-12): "Still walked: string literals (tag 24), floats, doors (201), closures"; the file at HEAD has since
taken string literals and strict doors (stale in the note, observed in the file).

### 3.6 Declined, whole defn

`jp-page-at` answers `nothing()` when the frame would need more than 4095 slots or when **no node** was taken in the
page's own words (`r[2] == 0`, line 384); `jp-page` when the frame wrapper's slot count cannot be read (line 379);
`jp-offer` when the offer's ABI number is not 4 (lines 388-393). Then nothing is bound and the defn stays walked.

## 4. Register and frame convention of the page

All observed in `jit-page.bml`; instruction meanings confirmed by disassembly (section 8).

- **Entry**: `long long page(long long fp, void *cx)` (`fkwu-uni.c:10249`). `x0 = fp` (the frame's index into the value
  stack), `x1 = &fk_jit_cx`.
- **Prologue** (line 100): `stp x29,x30,[sp,#-48]!`; `mov x29,sp`; `stp x19,x20,[sp,#16]`; `stp x21,x22,[sp,#32]`;
  `x19 = cx`; `x20 = fp`; then `enter(fp, need)` (helper 5) reserves and zeroes the frame up to `fp + need`.
- **Pinned registers**: `x19` = helper table, `x20` = fp, `x21` = address of `fk_vs[fp]`. `x21` is **recomputed after
  every helper call** (`ldr x9,[x19]; ldr x9,[x9]; add x21,x9,x20,lsl #3`, line 97), "because a helper can grow the
  value stack" (lines 12-13). `x22` is saved and never used (observed: no `22` operand elsewhere in the file).
- **Values**: the current value is in `x0`. Binary ops put the left operand in `x1` and the right in `x0`. `x9..x13`
  are scratch between helpers. "nothing lives in a register across a helper" (line 13).
- **Frame**: slots `0 .. arity-1` parameters, then lets (the walker's own layout, read from the tag-111 wrapper),
  then the page's temporaries from `t0 = max(arity, k111 + 1)` upward (lines 374-383). All of them are value-stack
  cells, "so a melt sees and moves every value the page holds" (lines 10-11). Slot offsets are scaled 12-bit
  immediates, hence the 4096 limit.
- **A body written in place** gets its frame among the caller page's temporaries: its slot 0 stands at page slot `t`,
  and every read adds the offset `cx[2]` (lines 131-133, 343-348). Nodes it cannot say are walked at `fp + off`
  (`add x1, x20, #off`, line 107).
- **Return**: value in `x0`, `str xzr,[x19,#8]` (clears `cx.next`), restore, `ret` (lines 102-103). **Hand-off**: node
  id into `cx.next`, restore, `ret` (line 104); the walker's 194 arm then continues at that node in the same frame
  (`fkwu-uni.c:11069-11077`).
- **Loop**: a self tail call ends in `-1`, a placeholder that `jp-link` replaces with a backward `b` to the first word
  after the prologue (lines 110-111, 369, 373).
- **Stops**: a helper that stops (`fk_stop`, a `longjmp` to the nearest recover point per `S/runtime-jit.md` 6) unwinds
  over the page's native frame. Inferred from 4 and from `S/runtime-jit.md` 6: this is safe only because the page
  keeps nothing but callee-saved registers the `jmp_buf` restores and value-stack cells the recover point resets.

## 5. How values appear in emitted code

The page uses the walker's value word unchanged (`S/runtime-jit.md` 3.1 has the full band table; rows below are what
the emitter itself relies on, observed at the cited lines).

| value | word | where the page relies on it |
|---|---|---|
| int `n` | `2n` (even) | literal `n * 2` (137); int test is one `orr` + `tbnz #0` on both operands (262); add, sub, and, or, xor work on tagged words directly; mul untags one side; div, mod and the u32 ops untag both and retag |
| false / true from a compare | `0` / `2` (the ints 0 and 1) | `cset; lsl #1` (257) |
| nil, the empty list | `1` | `movz x0,#1` (137); `eq` identity path `cmp x,#1` (263) |
| pair | odd positive, heap index `<< 1 \| 1` | never built or opened in page words: `cons`, `head`, `tail`, `nth`, `len` are helpers (`fkwu-uni.c:15412-15482`) |
| string | odd negative, `fk_sbase - (index << 1) - 1` | a literal's word is loaded as four halves because the seed pins the index (`fkwu-uni.c:10792-10797`, 1659-1662) |
| float | odd, a box index | never in page words: an odd operand takes the slow path, where `fk_op_add` and friends compute in doubles and box (`fkwu-uni.c:15487-15509`); a float literal node is walked |
| nothing | the constant `-8999999999999999999` (odd) | never made by the page; as an `if` condition it reaches `truth`, which stops (`fkwu-uni.c:15388-15393`) |
| defn index, node index | plain immediates in the instruction stream | `jp-callee`, `jp-walk`, `jp-handoff`, `jp-enter-in-place` (104-108, 352-353) |

The `if` law the page writes in line (lines 191-206): word 0 is false, word 2 is true, any other word asks
`fk_truth` (0 and a float zero are false, nothing refuses, everything else is true).

Consequence (observed in `R/docs/native-jit-routing.md:50-53`): "emitted words can contain current function/node
indices and literal-pool words, so saving raw pages is insufficient". The page's code is position-independent but
**process-dependent**: it embeds node ids, defn ids and pinned string words as immediates.

## 6. The helper table (ABI 4), every word

`struct fk_jit_cx` at `fkwu-uni.c:10261-10279`; filled at `10498-10516`; mirrored by `jp-op-word` at
`jit-page.bml:92-96`. Words are 8 bytes; the page reads word `i` with `ldr x9,[x19,#i*8]`. Both sides observed and
cross-checked slot by slot.

| word | name | signature / content | walker tag served | used by the emitter at |
|---|---|---|---|---|
| 0 | `vs` | `long long **`: address of the value-stack base pointer | | `jp-reload` 97 |
| 1 | `next` | node the walker continues at, or 0 | | `jp-return` 103, `jp-handoff` 104, tail call 362 |
| 2 | `walk` | `(node, fp) -> word` | any | `jp-walk` 107 |
| 3 | `truth` | `(word) -> 0/1`, stops on nothing | 6 | `jp-if` 200 |
| 4 | `tick` | `()`: the cell deadline tick | | self loop 369 |
| 5 | `enter` | `(fp, need)`: grow the stack, zero the frame | 111 | prologue 100 |
| 6 | `call` | `(c, fp, t, n) -> word` | 12 240 241 | 362 |
| 7 | `tail` | `(c, fp, t, n) -> node` | 12 240 241 tail | 362 |
| 8 | `head` | `(a, _) -> word` | 20 | |
| 9 | `tail` (list) | `(a, _)` | 21 | |
| 10 | `len` | `(a, _)` | 22 | |
| 11 | `str_len` | `(a, _)` | 25 | |
| 12 | `str_eq` | `(a, b)` | 26 | |
| 13 | `str_byte_at` | `(a, b)` | 28 | |
| 14 | `cons` | `(a, b)` | 19 | |
| 15 | `add` | `(a, b)` | 3 | |
| 16 | `sub` | `(a, b)` | 4 | |
| 17 | `mul` | `(a, b)` | 42 | |
| 18 | `div` | `(a, b)`, stops on zero | 10 | |
| 19 | `mod` | `(a, b)`, stops on zero | 11 | |
| 20 | `le` | `(a, b)` | 5 | |
| 21 | `lt` | `(a, b)` | 103 | |
| 22 | `eq` | `(a, b)` | 102 | |
| 23 | `band` | `(a, b)` | 34 | |
| 24 | `bor` | `(a, b)` | 35 | |
| 25 | `bxor` | `(a, b)` | 36 | |
| 26 | `shl_u32` | `(a, b)` | 37 | |
| 27 | `shr_u32` | `(a, b)` | 38 | |
| 28 | `nth` | `(a, b)` | 23 | |
| 29 | `byte_to_str` | `(a, _)` | 33 | |
| 30 | `value_kind` | `(a, _)` | 201 mode 4 | `jp-kind` 231 |
| 31 | `rotr_u32` | `(a, b)` | 39 | |
| 32 | `add_u32` | `(a, b)` | 40 | |
| 33 | `bnot_u32` | `(a, _)` | 41 | `jp-bnot` 236 |
| 34 | `cur_fn` | `long long *`: address of the running defn | | in-place enter/leave 352-353 |
| 35 | `fr_depth` | `long long *`: address of the frame chain's depth | | same |
| 36 | `fr_fn` | `int *`: the frame chain array | | same |
| 37 | `fr_cap` | the chain's capacity (1048576) | | same |

Facts about the table (observed):

- It is **one static struct for the process** (`static struct fk_jit_cx fk_jit_cx`, 10280). `next` is therefore one
  global cell: the 194 arm clears it before the call (11070) and every value return clears it again (103), because a
  nested page may have left it set (inferred from those two writes).
- The number travels both ways: the offer hands `FK_JIT_ABI` as the fourth argument (31793), the emitter declines
  another number (`jit-page.bml:390`), and the bind door refuses a word list not headed by it (10470). Reason given:
  "a checkout's ./fkwu is not rebuilt by a merge" (10250-10253). The history shows ABI 3 on 2026-10-11 and ABI 4 on
  2026-10-12 (`M/project-form-page-jit.md`).
- Words 34-37 expose raw C data addresses so the page can push the frame chain itself; they replaced two helper
  functions (same note).
- The 26 ops are the functions the walker's own arms call (`fkwu-uni.c:15410-15411, 15483-15485`), so there is one
  definition per op for cold and hot.

## 7. Offer, install, fall back; and every refusal code

Observed unless marked.

**Offer** (`fkwu-uni.c:31718-31870`, `24343-24351`).
1. Trigger: `fk_heat_pulse` -> `fk_jit_pulse(c)` when `fk_fn_heat[c]` is a nonzero multiple of 4096 and the defn's
   state is offerable (0 or 3); or the sampler posted a never-offered defn whose own samples reached a multiple of 32
   (`fk_jit_time_want`, 10296-10300, 24094).
2. If the JIT is busy, or a load is in progress and the JIT unit is not yet resident, the defn waits once in a line of
   64 (state 2) and is offered at the next quiet pulse (31838-31854). A full line leaves it at 0 to ask again.
3. `fk_jit_offer_one`: state := 1; skip if the body already wears tag 194 (the C lane's leaf); build
   `[c, body, arity, 4]`; run `form-stdlib/bml/jit-page.bml` as a cell **in this process** through `fk_cell_run_door`
   in phase `jit-offer`; count offers and microseconds (kernel_stat 74, 76).
4. The unit is found from the boot root, else beside the binary (31729-31753). `FK_NO_JIT` in `fkwu.conf` turns the
   lane off; `FK_JIT_SAY` prints each offer.

**Install** (`fk_jit_bind`, 10453-10522). `jit_fn_bind(fn, [abi, w0, w1, ...])`: checks the defn, that no page stands,
the ABI head, that every word is an int in 0..2^32-1, at least one word; then `fk_page_alloc(span)` (span rounded up to
whole pages; `getconf PAGESIZE` is 16384 on this Mac, observed-run), `fk_page_write`, stores `fk_jit_page[c]`, and
replaces `fk_fn[c]` by a new node `(194, c, old body)`. Answers 1 (tagged 2), or 0. `jit_fn_bind(fn, [])` asks whether
a page stands. Only on Darwin arm64 (`FK_HAVE_DARWIN_ARM64_JIT_WITNESS`, 10481-10483): elsewhere it answers 0.

**Run** (11065-11078). The walker's tag-194 arm: `next = 0`; call `page(fp, &cx)`; if `next == 0` the value is the
answer; else continue the walker's loop at node `next` in the same frame.

**Fall back**. Per node as in 3.5. Per defn: no page, body walked as before. **There is no uninstall**: nothing in the
file assigns `fk_jit_page[c] = 0` or unmaps a Form page (observed: `grep` finds only the bind and a fault-test plant at
18390-18404). Inferred: each bound defn holds at least one 16 KiB mapping until the process ends (160 pages for one
lowering, per `CURRENT_FLOOR.md:1346`, is about 2.6 MiB).

**A failing offer is not silent, and it changes the program's outcome**: if the JIT's cell stops, the state returns
to 0, a `native-failure` diagnostic is written, and `fk_stop(detail)` is raised in the program that happened to cross
the heat mark (31807-31818).

**Refusal vocabularies** (three, observed):

| where | values |
|---|---|
| `fk_jit_state[c]` (10284-10286), shown on the time board as page / leaf / declined / waiting / inplace / walk (24132-24134) | 0 offerable, 1 offered (page bound or declined), 2 waiting in the line, 3 written in place by a caller's page |
| `jit_fn_bind` answer | 1 bound, 0 refused (not a defn, already bound, other ABI, a non-instruction word, no words, no executable memory, not this platform) |
| `jit_node ["why"]` (10644-10649) | 1 unknown word or bad spec; 2 tag outside 1..255; 3 extra element; 4 short spec; 5 a field not the int or name expected; 6 a node the page may not take apart, or no node; 7 more slots than a frame holds; 8 empty name or arity outside 0..255; 9 an index that is no function, or a body already bound; 10 a name no function was registered under |
| emitter (`jp-offer`) | value 0 with nothing bound; the reason is not reported |
| C lane, for contrast (`M/project-jit-refusals-are-arms-2026-11-14.md`) | -1 declined, -2 frame slot kind, -4 no operation named, -5 emitter ran out, -6 arity above 6, -7 loop step named nothing, -(1000+tag) no arm, -(3000+tag) innermost node unnamed |

The Form page has **no reason code of its own**: a declined defn reads "declined" and the node kinds a page left to
the walker are measured afterwards by the sampler (`pagewalk` and `walktag` rows, `CURRENT_FLOOR.md:1328-1333`).

**The other doors the emitter needs** (`jit_node`, 10523-10800): `kids`, `mint`, `word`, `inline` (page use);
`node`, `defn`, `body`, `lookup`, `arity`, `run`, `why` (a Form producer building code nodes without text,
`R/docs/bml-direct-nodes-jit.md:21-27`). And `kernel_ast(pid, k)` for a node's four words, `kernel_ast(pid, ["fn", c])`
for `[body, arity]` or nothing for a capturing closure (`jit-page.bml:34, 316`; `fkwu-uni.c:20672-20674`).

## 8. The AArch64 encoding helpers

### 8.1 In `jit-page.bml` (lines 41-102)

- 33 parametrized instruction forms: `ldr str mov add sub and orr eor mul sdiv msub` (64-bit), `add ror lsl lsr mvn`
  (32-bit), `asr #1`, `add #imm`, `sub #imm`, `str w,[x,x,lsl #2]`, `lsl #1`, `cmp`, `cmp #imm`, `cset`, `tbnz #0`,
  `cbz`, `b.cond`, `b`, `blr`, `movz`, `movk`, `movn`, `add ..,lsl #3`. Plus 8 fixed words (the three `stp`, three
  `ldp`, `mov x29,sp`, `ret`). Four condition codes (eq 0, ge 10, lt 11, le 13).
- **How an encoding is expressed**: a function returning one integer, `base + field * 2^position` with the base as a
  **decimal** literal and the hex only in a trailing comment, e.g.
  `def jp-ldr(rt, rn, slot) = 4181721088 + slot * 1024 + rn * 32 + rt;   // ... f9400000` (line 42). The header says
  "the base words were converted from their hex by fkwu itself" (line 41). Signed branch offsets are folded by hand
  (`jp-s19`, line 69; 26-bit at line 73). A 64-bit constant is `movz`/`movn` plus a `movk` per non-trivial halfword
  (85-90).
- **Checked** (observed, transcription): I transcribed 39 sample encodings and the 8 fixed words into
  `S/emitter-check/gen.c`, assembled the resulting `.long` words with Apple clang 17 and disassembled with
  `otool -tv`. Every word decodes to the instruction its comment names (for example `jp-msub(0,3,2,1)` ->
  `msub x0, x3, x2, x1`; `jp-cset(0, le)` -> `cset x0, le`; `jp-add-lsl3(21,9,20)` -> `add x21, x9, x20, lsl #3`; the
  prologue and epilogue as in section 4). No mismatch found.

### 8.2 In `form-asm.fk`

- 81 definitions (observed: `grep -c`): 71 instruction encoders (63 parametrized, 8 fixed byte rows) and 10 helpers.
  Families: 32-bit integer (`movz add sub mov cmp mul udiv sdiv csel` and register forms), 64-bit integer
  (`movz movk lsl-imm add sub cmp mul eor`), loads and stores (`ldrb strb ldr str` in w, x, d, q widths, a PC-relative
  literal load), control (`b b.cond bl ret svc nop`), frame pairs, scalar double (`fadd fsub fmul fdiv fmax fmin fmadd
  fsqrt fmov` four ways, `fcmp scvtf fcvtzs frintn`), NEON and SHA-256 (`rev32 add orr sha256h sha256h2 sha256su0
  sha256su1`). A few are aliases of one instruction (`fa-b`/`fa-b-off`, `fa-bgt`/`fa-bcond`, `fa-movk-x-16`/`fa-movk-x`).
- **How an encoding is expressed**: as **data**. `fa-asm(base, mults, args)` packs `base + sum(arg_i * mult_i)` and
  splits it into four little-endian bytes (`form-asm.fk:26-35`): "a new instruction is a DATA row, not new code".
  Example: `def fa-add(rd, rn, imm) = fa-asm(285212672, [1, 32, 1024], [rd, rn, imm]);` (line 40).
- The file's stated reason for bytes, "the kernels wrap a literal > 2^31 to signed int32" (lines 13-17), is historical:
  later rows in the same file use bases above 2^31 through the same packer (for example 4181721088 at line 183). A few
  early words are still hand-split bytes for that old reason (`fa-ret`, `fa-bl`, `fa-sxtw`, the frame pairs).
- **Its verification discipline**: each row carries the oracle word it was checked against ("VERIFIED byte-for-byte
  against the clang/LLVM oracle (otool -tvVj of clang -c)", lines 215-216), and `fa-conviction` / `fa-may-drop`
  (301-304) state the rule that an external assembler may be dropped only after byte identity over the whole corpus.

### 8.3 What neither has

No label or fixup mechanism (branch distances are computed from list lengths at every site), no register allocator
(A: everything in frame slots; B: a single accumulator plus `w9..w15` by depth, 7 levels, `form-lower.fk:97-110`), no
relocation records, no literal pool for the page (B has a PC-relative f64 load row but `lo-flit` uses `movz`+`movk`).

## 9. `form-lower.fk`, the leaf compiler (writer B), in brief

Input: a small op-tagged tree, its **own** tag space (1 LIT, 2 ARG, 3 ADD, 4 SUB, 5 MUL, 8 DIV, 6 COND, 7 CALL, 9
STR_EQ, 10 READ_FILE, 11 WRITE_FILE, 12 I2F, 13 F2I, compares 5/14..18), not the walker's tags (`form-lower.fk:71,
137-141, 296-301`). `anf-shadow.bml` translates walker nodes (via `anf-lower.bml`) into it.

- Covers: integer arithmetic over arguments and literals with operand-aware instruction selection; `COND` over
  "compare of an argument with a literal" only; a fib-shaped self recursion (`lo-rec-fn`, 392-405); one-argument float
  trees (`lo-fnode`, 296-330) and an abstract float program checked by a simulator (331-349); `str_eq` and `str_find`
  as inline byte loops over raw pointers (173-184, 406-676); `read_file`/`write_file` as raw `svc #0x80` syscall
  sequences (186-216); a source map from each instruction to its node (350-378).
- ABI: AAPCS64, up to 8 integer slots; a string argument spends two slots, base pointer and byte length
  (`fkwu-uni.c:5317-5368`); result is `x0` as a 64-bit int, sign-extended from `w0`.
- Declines by answering `nothing()` and **carrying it upward** (`lo-app`, 32-35): an uncovered tag, a division whose
  divisor is not a nonzero literal ("a native page has no way to stop", 90-94), more than four banked arguments, an
  effect row the root does not reach (229-242).
- It cannot call back into the runtime: "there is no relocation/trampoline table for compiled bytes to `bl` back into a
  C native by address" (414-423). That is the structural difference from the page, whose whole design is the helper
  table.
- `anf-lower.bml` is the piece of this lane worth most: rows in which "every call's arguments are bound to
  temporaries IN ORDER BEFORE the call row" (lines 1-16), with an interpreter over the rows as the reference
  (`anf-eval`, 191-212) and a reason for every refusal (`anf-why`).

## 10. `form-macho.fk`: what it can write

Observed in the file:

- One function family, `mo-object-sym(code, name)` (line 69): a **Mach-O 64-bit arm64 relocatable object**
  (`filetype 1`, `MH_OBJECT`), with exactly two load commands (`LC_SEGMENT_64` holding one `__TEXT,__text` section, and
  `LC_SYMTAB`), the code at file offset 208, one exported symbol at section offset 0, a string table. `mo-object(code)`
  exports `_main`.
- It writes **no** executable (`MH_EXECUTE`), no dylib, no relocations (`nreloc 0`), no data or constant section, no
  `LC_BUILD_VERSION`, no `LC_DYSYMTAB`, no code signature. Sizes are 32-bit (`mo-u64` writes the low word and four
  zero bytes, line 30).
- The file says so itself: "`ld` links the .o, the binary runs ... `ld`'s ad-hoc signature meets the macOS arm64
  signing requirement" (lines 3-6). The linker is an outside program; nothing in the old body writes a signed
  executable. The script it cites, `scripts/form_macho_demo.sh`, is not in the repository (observed:
  `git ls-files | grep form_macho_demo` is empty). Its band checks structure only: five byte-level claims, verdict 31
  (`tests/form-macho-band.fk:13-19`).
- A comment miscounts: "the four load commands sum to 208 bytes" (line 31); there are two (`ncmds 2`, line 37) and 208
  is header 32 + 152 + 24.

Observed (transcription, `S/emitter-check/`): I rebuilt the band's object in C from the recipe's layout with the
band's 20 code bytes (`movz w0,#40; add w0,w0,#1; sub w0,w0,#1; sxtw x0,w0; ret`). Result: 251 bytes, as the band
states; `otool -hv` reads `MH_MAGIC_64 ARM64 OBJECT 2 176`; `nm` reads `T _main`; `ld -o fm formmacho.o -lSystem
-syslibroot <sdk> -e _main -arch arm64 -platform_version macos 13.0 13.0` links it with one warning ("no platform load
command found ... assuming: macOS"); `./fm` exits 40; `codesign -dv fm` reads `flags=0x20002(adhoc,linker-signed)`;
`ld -dylib` also produces a signed dylib. So the layout is a valid object today, and the answer to "runnable? signed?"
is: **runnable and signed only after Apple's linker**; the recipe supplies about 230 bytes of container around the
code.

## 11. Measured speeds on record

None measured by me: I did not run `fkwu`. All rows are the sources' numbers (observed-read), with the source's date.

"Before" is the walker alone in the first five rows and the last three; in the two rows marked (*) it is the page
lane before the named commit.

| what | before | after | source, date |
|---|---|---|---|
| a 3M-step self tail loop (Form page) | 156 ms | 7 ms, plus 7 ms for the first offer | `R/CURRENT_FLOOR.md:1341`, 2026-10-11 |
| fingerprinting 232 KB of source (Form page) | 36 ms | 2 ms, plus 9 ms for the first offer | `:1342` |
| lowering `form-find.bml` (same bytes out) | 2.1 s | 1.26 s; 171 offers, 160 pages bound, 0.15 s offering | `:1343, 1346` |
| lowering `form-path.bml` | 1.65 s | 1.0 s | `:1344` |
| cold `--check` of `form-cli-repl.fk` in a sealed body | 0 units lowered in the time | 118 units in 90 s | `:1261-1262` |
| (*) SHA-256 in Form over 256 KB, once pages wrote leaf callees in place and the u32 ops in own words (the rounds were also restructured in that commit) | 534 ms | 54 ms | commit `d03d6041f`, 2026-10-11; `:1368` |
| (*) a header parse, once the generic compare was written in page words | 8.8 s cpu, page-walk `node_eq` 22.6% | 7.9 s, 0.0% | commit `8503632b4`, 2026-11-12 |
| emitter cost per offer | | about 0.9 ms (0.15 s / 171) | inferred from `:1346` |
| emitting a 256-argument call | 189 ms (old `jp-args`) | 55 ms (reversed accumulator), identical words | `R/receipts/2026-11-04-optimization-book.md:241-249` |
| building one 80-byte recipe | 15 us by the text printer | 116 us through per-node `jit_node` calls (slower) | `R/docs/bml-direct-nodes-jit.md:26`, 2026-11-13 |
| leaf door (writer B) against the walker on 400k calls | 1x | 1.2x; "0 of the hot worklist coverable" | `M/project-per-recipe-jit-slice.md`, 2026-09-02 |
| FNV fingerprint of 1.39 MB | 18 ms as a Form recipe | 2 ms as a 17-instruction CPU image | `R/CURRENT_FLOOR.md:1352-1354` |
| SHA-256 CPU image over a 2.02 GB file | | 659 ms (3.1 GB/s) | `:1361-1363` |
| C loop lane, for contrast: 20M-iteration int loop | 0.63 s | 0.01 s | `M/project-per-recipe-jit-slice.md`, 2026-09-06 |

Reading (inferred): the Form page's gain is about 20x on a tight int loop and about 1.6x on real compiler work, where
time moves into the helpers (record_get, value_kind, str_concat per `CURRENT_FLOOR.md:1330-1332`). The C lane's
untagged registers give about 60x on the same kind of loop. The page lane has no figure from this study round
(`S/runtime-jit.md` 7.1 could not reach it).

## 12. Recorded miscompiles and wrong answers, with root cause where known

| # | what went wrong | lane | root cause | status | source, date |
|---|---|---|---|---|---|
| 1 | Past 1024 calls, arguments a callee never reads lost their effects: 3,936 of 780,000 effects landed | C loop lane | `fk_f64_inline_try` inlined by substituting argument **expressions** at parameter reads (call by name) | guarded in C (`dd397a892`); the contract moved to `anf-lower.bml` | `M/reference-hot-recipe-inline-drops-arguments.md`, 2026-10-03; `fkwu-uni.c:14728-14732` |
| 2 | A closure over three locals, mapped over 317 names, stops at its roughly 1024th call with "order: only numbers have an order" | fires at `FK_F64_HEAT` = 1024, so a C lane or its pulse (inferred) | **not known**; "The kernel fault itself is not repaired" | open; the caller was rewritten without the closure | `M/reference-hot-closure-over-locals-stops-at-heat-mark.md`, 2026-11-24; commit `b8ff978c9` |
| 3 | `-7/2` answered 2147483644 | `form-lower.fk` | unsigned divide instruction | fixed to `sdiv` | `form-lower.fk:88-89`; 2026-10-04 |
| 4 | An uncovered tag lowered to an empty list, so the image silently missed a node: "native answers 10 for arg 5" | `form-lower.fk` | absence represented as `[]` and concatenated | now `nothing()`, carried by `lo-app` | `form-lower.fk:32-33` |
| 5 | `-3` arrived as 4294967293 | `form-lower.fk` | result in `w0`, zero-extended into `x0`; each caller had to know | `sxtw` in the epilogue (`lo-sign`) | `form-lower.fk:243-253`, 2026-10-04 |
| 6 | `f(2000000000)` answers 1705032711, walker says 6000000007 | `form-lower.fk` | 32-bit registers | standing, named | `tests/jit-leaf-inram-band.fk:12-15` |
| 7 | A `COND` whose condition was not a compare "lowered to a compare of garbage registers and answered with full confidence" | `form-lower.fk` | a default arm read any tag as LE | default is now `nothing()` | `form-lower.fk:139-140` |
| 8 | An effect row not under the root was never lowered and nothing said so | `form-lower.fk` | lowering by reachability with no check | `lo-effects-bound?` | `form-lower.fk:229-233` |
| 9 | Callee-saved `x19..x22` used without save and restore | `form-lower.fk` | missing prologue; latent until the bytes ran in-process | fixed | `form-lower.fk:268-276`; 2026-09-02 |
| 10 | `str_find` from a nonzero start read bytes offset from its own index | `form-lower.fk` | index and pointer not advanced together | caught by its own band before landing | `form-lower.fk:639-651`, 2026-09-03 |
| 11 | `len` answered 311 cold and 310 hot | Go sibling kernel | every primitive existed twice (native and JIT helper) and only one was healed | sibling kernels are gone | `M/reference-jit-abi-mirrors-natives.md`, 2026-07-17 |
| 12 | Call arms 44 and 199 "had drifted (no frame chain, no caller restored)"; the body loop's `if` read `== 0` where the value walker read `fk_truth` | the two C walkers | duplicated call and branch code | one `fk_call_value` / `fk_call_tail`, one `fk_truth` | commit `12d616270`, 2026-10-11 |
| 13 | Every offer declined with value 0 when the first unit ran from outside a checkout | Form page | the emitter's own imports were not found; a decline carries no reason | found "from anywhere" | `M/project-form-page-jit.md`, 2026-10-11 |
| 14 | A `jit_node` word longer than any it knows ended the process; `kids` and `mint` refused past 16 leaves | seed door | fixed buffers | fixed | `R/docs/bml-direct-nodes-jit.md:23`, 2026-11-13 |

For `jit-page.bml` itself I found no recorded wrong answer in its ten commits or the notes (observed: commit subjects
and bodies scanned). Its stated detector is: "`jit-page-band` < 31, or ... lowering bytes that differ between
`FK_NO_JIT` and the default" (`M/project-form-page-jit.md`).

On fault 2 (inferred, not verified): the page lane's mark is 4096, the fault is at 1024; a closure is excluded from the
loop lane (`lane` at `fkwu-uni.c:10373, 10379`) but not from the heat pulse or the float-box pulse; and the seed
documents a shared scratch for captured values that "nothing may run between this write and the callee's prologue
read" because "the heat pulse can run the JIT's offer, a whole Form cell" (`fkwu-uni.c:10352-10354`). That scratch is
the first place I would look. I did not reproduce the fault.

The pattern across 1, 11, 12 and the open 2 (inferred): each is a second copy of a meaning drifting from the first,
visible only after a heat threshold. The three encoders of section 2 are the same shape, not yet failed.

## 13. What a clean-slate emitter in the one grammar should inherit

Inherit (each observed working in A unless marked):

1. **Per-node fallback instead of per-defn refusal.** `walk(node, frame)` and the `next` hand-off let the emitter
   start with ten node kinds and grow by one arm at a time, with the uncovered share measured by kind.
2. **The interpreter's frame is the compiled code's frame.** All live values in frame cells across any call out; the
   base pointer reloaded after every helper. No stack maps, no register roots, collector unchanged.
3. **One definition per operation**, called by the interpreter's arm and by the slow path of compiled code; compiled
   code adds only an inline fast path guarded by a tag test.
4. **A numbered helper table whose number travels in the offer and comes back in the bind.**
5. **Arguments evaluated once, in order, into temporaries before the call** (`anf-lower.bml`, `jp-args`), as a property
   of the intermediate form.
6. **A decline is `nothing`, carried upward; never an empty or partial image** (`lo-app`).
7. **Encodings as data rows** (`fa-asm`: base and field multipliers), one table for every writer, each row checked once
   against an outside assembler. Section 8.1 shows the check costs a 100-line probe.
8. **The cold-equals-hot band over the whole vocabulary**, comparing through the printed value so ints, floats, lists
   and nothing are all held (`tests/jit-page-band.bml:1-20`), plus "same output bytes with the JIT off".
9. **Offer by time as well as by count**, and a board that names where compiled code is not.
10. **A self tail call is a loop that still ticks the deadline.**
11. **Raw CPU images over buffers** (`jit_cpu_run`): `fn(buffers, consts)` with strings where they lie and file spans
    mapped for the call is a small, clean door for hashing and similar kernels.

Let go:

1. Three encoders for one ISA and two JIT lanes with different thresholds where C pre-empts the grammar.
2. The mirror door (`kids` / `mint`): it mints walker nodes at emit time, keeps a strictness table in C beside the
   arms, and is one reason pages cannot be saved. In one grammar, strictness is a column of the operation's own row.
3. Instruction words as cons lists of boxed ints, and decimal opcode bases with the hex in a comment.
4. Absolute node ids, defn ids and pinned string words as immediates in the code.
5. One mapping per defn with no release.
6. A second value model (untagged 32-bit) for the same integers (writer B).
7. A helper table that is one process-wide static with one `next` cell (it forbids a second thread of compiled code).
8. An emitter failure surfacing as a stop in the user's program at the heat mark.
9. A decline with no reason (value 0).
10. Writing object files for an outside linker as the route to an executable. If the bootstrap is to be replaced by
    emitted code, the body needs its own `MH_EXECUTE` writer and its own ad-hoc signature, or a stated dependence on
    `ld` and `codesign`.

## 14. The cleanest minimal emitter interface to a C bootstrap

Inferred design, derived from the observed set in sections 6 and 7 by removing what exists only because the old
walker's arms are opaque C. Names are proposals. "Old" names the door it replaces.

### 14.1 Functions the bootstrap must expose to the grammar

Host doors (C for as long as the host is an OS; they cannot be emitted before code can be emitted):

| # | function | contract | old |
|---|---|---|---|
| 1 | `code_reserve(bytes) -> region` | one `MAP_JIT` read-write-execute reservation at boot; nothing if the host forbids it | `fk_page_alloc` per page |
| 2 | `code_commit(region, offset, bytes, n) -> entry` | lift this thread's write protection, copy, restore, flush the instruction cache; answers the entry address, or nothing | `fk_page_write` |
| 3 | `code_release(entry, n)` | return a span to the region's free list; may be a no-op in the first seed but must exist | none (observed gap) |

Dispatch doors (C while the bootstrap walker is C; the first things the emitted compiler replaces):

| # | function | contract | old |
|---|---|---|---|
| 4 | `recipe_bind(recipe_id, entry, abi) -> 1 / 0` | refuses another ABI number, an entry outside the region, a recipe already bound | `jit_fn_bind` + tag 194 |
| 5 | `recipe_unbind(recipe_id) -> 1 / 0` | the walker walks it again; required for "ice / water / gas" to be reversible | none |
| 6 | `recipe_entry(recipe_id) -> entry / nothing` | asks whether code stands | `jit_fn_bind(fn, [])` |
| 7 | `helper_table() -> (abi, address)` | the table of 14.2; the emitter reads the number and declines one it was not written for | `FK_JIT_ABI` in the offer |

Read doors (not JIT-specific: the interpreter and every other tool in the grammar use the same three):

| # | function | contract | old |
|---|---|---|---|
| 8 | `cell_read(id) -> (kind, a, b, c)` | one numeric cell of the store | `kernel_ast(pid, k)` |
| 9 | `recipe_shape(id) -> (body, arity, frame_slots, captures)` | what the emitter needs to lay out a frame; states captures instead of answering nothing for a closure | `kernel_ast(pid, ["fn", c])`, tag 111 |
| 10 | `op_row(kind) -> (arity, strict_mask, helper_slot, may_stop, may_allocate)` | **the one table** the interpreter's dispatch, the emitter and the helper table are all generated from | `flt-ops` + `fk_jit_plain_kids` + `jp-op-word` (three mirrors) |

One call in the other direction, which the bootstrap makes and the grammar answers:

| # | call | contract | old |
|---|---|---|---|
| 11 | `emit(recipe_id, abi) -> entry / nothing(reason)` | invoked by the walker at a heat or time threshold, never re-entered; a failure is a counted decline with its reason, never a stop in the caller | `fk_jit_offer` + `jp-offer` |

That is ten functions and one callback. `jit_node`'s eleven words reduce to none: `kids`/`mint` are replaced by
`op_row`'s strict mask (the emitter evaluates strict children itself and calls the op's helper with them in frame
slots), `word` by a per-entry literal table, `inline` by `recipe_bind`'s caller, and `node`/`defn`/`body`/`lookup`/
`arity`/`run` are ordinary store writes and reads in a design whose programs are cells already.

### 14.2 The helper table the emitted code calls through

Per interpreter activation (a pointer passed to every entry, not a process static), numbered, with its ABI number in
word 0 so a saved entry can be checked when it is loaded:

| word | helper | purpose | old word |
|---|---|---|---|
| 0 | `abi` | the layout's number | (in the offer only) |
| 1 | `stack_base` | address of the frame store's base pointer | 0 |
| 2 | `resume` | cell to continue interpreting at, or 0 | 1 |
| 3 | `interpret(cell, frame) -> word` | evaluate one cell the emitter did not cover | 2 |
| 4 | `enter(frame, need)` | reserve and clear the frame | 5 |
| 5 | `call(recipe, frame, first, n) -> word` | the one call | 6 |
| 6 | `tail(recipe, frame, first, n) -> cell` | the one tail call | 7 |
| 7 | `safepoint()` | deadline, collection and tier events | 4 |
| 8 | `literal(entry, i) -> word` | constants and cell ids by index, so code holds no process-local immediates | (immediates) |
| 9.. | one slot per `op_row` with a helper | the operation's whole law, the interpreter's own function | 8..33 |

Dropped relative to ABI 4: `truth` as a special word (it becomes an op row like any other), and the four raw frame-
chain addresses 34-37 (in-place inlining is an optimisation the first emitter does not need; if kept it should be two
helpers, not exposed C data).

Entry convention (kept from A, observed to work): `word entry(frame_index, table)`; three pinned callee-saved
registers (table, frame index, frame address); result register holds the current value; frame address recomputed after
every helper.

### 14.3 What this assumes

- The first emitter targets the interpreter's frame and tagged word, as A does. Unboxed registers (the C lane's
  60x) are a second tier, not part of the seed interface.
- Persisting native code needs word 8 (`literal`) plus a per-entry relocation list; neither old lane had it
  (`R/docs/native-jit-routing.md:50-53`).
- `emit` running as code in the same store means the emitter's own recipes get hot and are offered to themselves; the
  old body handled this with a busy flag and a line of 64 (`fkwu-uni.c:10290-10295`). The seed needs the same two
  things and nothing more.
- Replacing the bootstrap itself (not only hot recipes) with emitted code additionally needs an executable-file
  writer and a signature (section 10); that is outside this interface.

## 15. Open questions

1. Root cause of the hot-closure fault of 2026-11-24 (table row 2). Not found in any record; I did not reproduce it.
2. Does the `longjmp` unwind over page frames have a failure mode (for example a stop raised while a body written in
   place has pushed the frame chain)? The recover point restores the chain per `S/runtime-jit.md` 6; I did not read
   that restore against `jp-enter-in-place`.
3. What fraction of real work the Form page covers when the C lane is held (`FK_NO_F64`)? The note says the band
   reads 63 either way and "63 pages bound"; there is no throughput figure for that configuration.
4. Is the tagged-word, frame-slot design fast enough as the only tier? On record: about 20x on a tight loop, 1.6x on
   compiler work. No measurement separates "helper cost" from "slot traffic".
5. `bml-demand-jit.bml` (799 lines, unread): it holds the sealed, disk-persisted scalar artifact (v5) for writer B.
   Whether its admission and identity design is worth inheriting is unsettled here.
6. An x86-64 path: `body-fingerprint.bml:20-27` carries a hand-written `linux-x86_64` byte image; there is no x86
   encoder table. The one-table design of 14.1 row 10 has not been tested against a second ISA.
7. Whether the new seed should write its own signed executable or depend on `ld`.

## 16. Surprise, and what stayed uncomfortable

Most surprising: the "JIT written in the language" is, in machine-word terms, mostly **not** generating code for the
language's operations. Of the roughly 55 node kinds it handles (my count over sections 3.1 to 3.4), 28 become
instructions on their int path; the rest become "put the children in frame slots, then call the C function the
interpreter already had". Its real content is the calling
discipline (frame, reload, hand-off, ABI number), and that part is good. The second surprise: it shares nothing with
`form-asm.fk`, the careful, oracle-checked assembler that sits two directories away and is used only by lanes no heat
ever reaches.

Uncomfortable:

1. The only miscompile detector for the page is a band that needs heat 4096 inside one process. The fault recorded
   today appeared at the other lane's threshold in a multi-request process and was worked around in the caller.
2. Two doors accept raw machine bytes from any Form program with no check beyond "a multiple of four".
3. The page can never be unbound, and a failing emitter stops the user's program. Both contradict "heat changes
   speed, never meaning" at the edges.
4. My encoding and Mach-O checks verify a transcription. The recipes themselves were not run, by rule.
