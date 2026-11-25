---
name: project-form-page-jit
description: "2026-10-11: the walker's transparent JIT is Form (jit-page.bml): heat 4096 offers a defn to a resident cell that emits AArch64 and binds it; one call meaning, one op meaning, one page birth; how to witness"
metadata:
  node_type: memory
  type: project
  originSessionId: 533ece4b-c058-4748-885c-284ca4160f07
  modified: 2026-10-12T02:29:11.048Z
---

Built 2026-10-11 on Urs's "please build it right now" (transparent on-demand JIT; "JIT on demand is there to resolve anything cold"). The same day he asked for a close review of the ~500 lines it added to the seed, and those lines were healed down to one meaning per thing.

- fk_jit_state (2026-10-11): 0 never offered, 1 offered, 2 waiting in the line (pushed once; a full line leaves it at 0 to ask again), 3 written in place (jit_node ["inline", c]; offered by heat only). The sampler posts a never-offered defn whose own samples reach a multiple of 32 to `fk_jit_time_want`, and the run thread offers it at the next pulse (offer by time; a probe recipe called seven times was bound at heat 7). The board's how-words are page, leaf, declined, waiting, inplace and walk.
- Trigger: `fk_heat_pulse` → `fk_jit_pulse` → `fk_jit_offer` at heat multiples of FK_JIT_HEAT (4096). It runs `form/form-stdlib/bml/jit-page.bml` in-process via `fk_cell_run_door` with `[fn, body, arity, abi]`. Offers wait in a 64-entry line while the JIT runs, or while a unit is being collected and the JIT unit is not yet resident (`fk_jit_quiet`). The first BML floor call loads the JIT unit (`fk_jit_warm`), at the same point where the floor compiler's own first load runs. A resident JIT is then offered inside loads. Before 2026-10-11 (85acbd6cf), a process that lowers inside its loads (form-cli --compile-source, any cold BML load) walked everything. In a sealed body, a cold `--check` of form-cli-repl.fk lowered 118 units in 90 s with the JIT and 0 without it.
- **Found from anywhere (Urs 2026-10-11: "the JIT shall be available for any part"):**
  - The JIT unit, its floor compiler, the home index and the call-head table are all found from the named unit, else the working directory, else beside the binary (`fk_self_path`), as the Metal carrier is.
  - The floor names itself by `cell_path()` (`fsc-floor-self`). `bml-source-candidates` ends with the running unit's checkout.
  - A home index that isn't found is asked for again. Before this, a first unit from outside any checkout left the process unlinked, and the emitter declined every offer with value 0.
  - The form-cli seal must carry jit-page.bml (form-cli-source-roots.txt).
  - A JIT offer's cell runs in the time board's `jit-offer` phase.
- **The mirror lane:**
  - `jit_node(spec)` is leaf mode 57, a native with arity 1. `["kids", k]` answers k's value children in the order its arm walks them. `["mint", k, t]` makes a mirror of k whose value children read slots t+i. `["word", k]` gives a string literal's pinned word as four 16-bit halves.
  - The strict table lives in the seed beside the arms (`fk_jit_plain_kids`, `fk_jit_kids_of`):
    - the `str_concat` chain and the len-compare shape;
    - record get/set/has/keys, value_eq, str_find and the node natives;
    - file_size, read_file, write_file, host_file_append_bytes;
    - the leaf doors, except attempt (28). Modes 9/33/49/54 read a cons as their third child, and 50 a nested cons.
  - The emitter's `jp-strict` asks, then evaluates the children into temps and has the walker run only the mirror.
  - Add a tag only after reading its arm. Each value child must be walked once, in order; an arm that inspects a child structurally is shape.
- Bind: `jit_fn_bind(fn, words)`, leaf-door mode 56. A list of 32-bit words makes a page. `[]` asks whether a page stands (2 or 0). fn may be an index or a fn value. The defn then wears tag 194, and the 194 arm in `fk_walk_body` calls the page first.
- Page ABI 3 (2026-10-11): `page(fp, &fk_jit_cx)`. x19 holds cx, x20 fp, x21 &fk_vs[fp], reloaded after every helper.
  - cx words: vs 0, next 1, walk 2, truth 3, tick 4, enter 5, call 6, tail 7.
  - Then 26 ops from word 8: head tail len str_len str_eq str_byte_at cons, add sub mul div mod le lt eq, band bor bxor shl shr, nth byte_to_str, value_kind (30), rotr add_u32 bnot (31-33).
  - Then, since ABI 4 (2026-10-12), the frame chain as four words the page keeps itself: cur_fn 34, fr_depth 35, fr_fn 36, fr_cap 37. The inl_enter/inl_leave helpers are gone.
  - `jp-op-word` in jit-page.bml mirrors `struct fk_jit_cx`; change FK_JIT_ABI with the layout.
  - `cx.next != 0` means the walker continues at that node.
- **Written in place** (2026-10-11): `kernel_ast(pid, ["fn", c])` answers [body, arity], or nothing for a capturing closure (it was `jit_node ["fn", c]` until 2026-10-12).
  - `jp-call-in` writes a callee in place when its tail spine (111, a let's body, a do's last, if arms) holds no call, or only a call to a defn whose own spine has none (`jp-spine-ok?`). It goes two deep.
  - The emitter's cx is [self (-1 when inlined), limit, off, depth, name]. Slots are read at s+off; walked nodes and mirrors are walked at fp+off (mint with t-off).
  - The page's own words push the frame chain and set fk_cur_fn (`jp-enter-in-place`, `jp-leave-in-place`), so the board and stops still name the callee.
  - A callee written in place never heats to a page of its own; jit-page-band claim 32 witnesses this.
  - The u32 doors have int paths in own words (32-bit add/ror/lsl/lsr/mvn between untag and retag).
  - An untyped `==` in a hot leaf is a call to bml-value-equal and blocks inlining; give it `(Comparable)`.
- One meanings shared by walker and page: `fk_call_value` / `fk_call_tail` (every call arm of both walkers), `fk_op_*` (every strict op arm), `fk_page_alloc` / `fk_page_write` (all four machine-code lanes). A closure's captures are written by `fk_call_caps` after the heat pulse, because the pulse can run the JIT's cell. `if` reads `fk_truth` in both walkers since 2026-10-11 (the body loop used to read `== 0`).
- Witness: `form/form-stdlib/tests/jit-page-band.bml` must read 63 (31 before 2026-10-11's in-place claim), with the default lanes and with `FK_NO_F64`. Lowering a unit must give the same bytes with `FK_NO_JIT` as with the default. Toggles in fkwu.conf: `FK_NO_JIT`, `FK_NO_F64` (holds the C loop lane), `FK_JIT_SAY` (names each offer and its stop on stderr). kernel_stat 74-76: offers, bound, offer µs.
- The C f64 lane fires first at heat 1024 and keeps its leaf, because the offer skips a body already wearing 194. The R8 flip waits on the page carrying unboxed floats.
- `kernel_ast(own pid, ...)` reads nodes, defn rows and source spans straight from the kernel's tables.
- `observe/seed-census-run.bml` measures the seed: families, references in code only, arms per walker, arms no producer names, toggles. `printf 'table\n' |` prints every function.

**Why:** Urs: a compiler gap is resolved, not worked around; cold relowering was slow because the compiler's hot loops walked; seed code is reviewed closely and carries one meaning per thing.
**How to apply:** widen the page's own words before adding a walker workaround. Still walked: string literals (tag 24), floats, doors (201), closures. A miscompile shows as jit-page-band < 31, or as lowering bytes that differ between FK_NO_JIT and the default. Compare those first.

Related: [[feedback-bml-plus-for-strings-jit-appends]], [[reference-hot-recipe-inline-drops-arguments]], [[feedback-no-child-processes-cells-run-in-process]].
