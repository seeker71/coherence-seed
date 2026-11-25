---
name: project-per-recipe-jit-slice
description: JIT mechanism (crystallize-on-heat) wired byte-source-agnostic; form-lower is a fungible byte-source, not the JIT
metadata: 
  node_type: memory
  type: project
  originSessionId: c10c9afe-a018-47f2-ac31-562993dd61df
  modified: 2026-09-02T08:43:49.541Z
---

**Category correction (389206f8, row 728): form-lower is NOT the JIT.** host-kernel.form defines
the JIT as crystallize-on-heat → cache-by-NodeID → dispatch → melt, health-earned
(`native-replace CAN-not-MUST`); the reference jit.go crystallized via a Go plugin. form-lower's
axis is sovereignty ("drops clang"), a **fungible byte-source**, not the mechanism. The mechanism
is wired as a Form composite: `form/form-stdlib/jit-crystallize.fk` (heat + content-keyed native
cache + champion-challenger dispatch + melt; the ONE byte-source seam is `jc-crystallize`, today
form-lower, swappable). `jit-crystallize-band` = 31. Runtime still has no heat/cache — the
mechanism lives in Form.

**Purity RETIRED as a JIT gate (c4464cd5, row 730), then RE-HOMED (7b1d91f3, row 732).**
Crystallizing caches the CODE, not the RESULT — effects lower to real `svc` syscalls / `bl` into
an injected address, so any recipe is JIT-compatible (**exceptionless**). So purity is NOT a JIT
gate. But it returns as the precondition for ONE strategy — store-as-cell (memoizing a value needs
purity+const; an effectful const still direct-lowers so its effect fires). **The JIT has TWO real routes**
(`jc-strategy` in jit-crystallize.fk): 0 direct-lower · 1 store-as-cell (pure const → native = load
the value, const-folded shorter: 12 vs 16 bytes). Totality comes from form-lower COVERING the recipe
vocabulary (fk-to-prog/f2p-tag emits only covered tags; lo-node covers 1-6,8,9,10,11; calls lower via
the real lo-callconv), NOT a fallback ladder. "strat 2" is only a safety guard (decline a malformed
prog), unreachable for real recipes. NOTE: an earlier "inject-bl" 3rd route was warrantless — a term
I coined with no user (only a synthetic tag 99); removed ea0f2a27 (row 733). `jit-crystallize-band` = 63. Health is EXTENSIONAL, not byte-identity (row 731): byte-identity to
clang is a ceiling form-lower-sim already lifts (validates by execution); per-instruction keeps the
one true ISA encoding; nat_run gives hardware execution. clang → benchmark oracle (length/speed/
parallel), not template. jit-decision.fk carries a correction note (four-way 11111 untouched).
**Still purity/byte-identity-framed (reconcile): jit-tier-policy.fk `jpr-pure`, opencode-loop, the
jit-decision-band wording; and length/speed/parallel measurement infra does not exist yet.**

Remaining: TRANSPARENT runtime dispatch (auto-crystallize hot fns, not the explicit `jc-call`),
make_nodeid as the real cache key (content-fold today), the mechanism interpreting the recipe for
the champion (walk result is passed in now).

Architecture direction Urs set (2026-07-05): code flows into Form-native recipe objects; a byte
source (`form-lower`/`lo-compile-fn`, or clang/Go) converts ONE recipe → ASM at a time.
The `--src` whole-file→one-AST→walk path is the **bootstrap seed meant to RETIRE**
(runtime/fkwu-uni.c:6870, "carrier-last inversion" if grown) — not the runtime. Running real
programs (RAG, bands) through `--src` is what made the AST cap even matter.

**Proven end-to-end (afb83fb0):** the arm64 native door was a `return -1` stub — un-stubbed with
Apple Silicon MAP_JIT (`pthread_jit_write_protect_np` + `sys_icache_invalidate`). New op
`(nat_run BYTES ARG)` (tag 245) hands a lowered byte-list to `fk_native_call`.
`form/form-stdlib/tests/jit-native-run-band.fk` = 15: recipes lowered by form-lower run on
hardware and match the walker. `native_call_test` now 6 (was -1). Adding the op was the blessed
way — a `flt-ops` row in `flatten/form-flatten.fk`, regen `runtime/fkwu-optable.h` via two
pure-Form `gen-source-walker.fk` calls (no C edit to the header).

**Next increments** (each de-risked, none fundamental): wire `fk-to-prog` so a real *source*
function → recipe → `nat_run` (the recipe is hand-built in the band today); a **multi-arg native
door** (current door passes one arg in x0/rdi); then make flatten→per-recipe the runtime and
retire `--src`. See receipts/2026-07-05-per-recipe-jit-slice-arm64-door.md. Ties to
[[reference-fk-nodes-probe-cap-vs-bug]] (the monolith's cap pain) and [[project-consolidation-program]].
Frontier word row 727: "sundered" — emit-half and run-half were proven separately, never joined.

**2026-09-01 (Urs): JIT in RAM, not on disk.** Crystallization promotes
hot recipes to executable pages in the RUNNING process (the un-stubbed
MAP_JIT arm64 door) — no .dylib requirement, no native-emission disk
artifact required, ever. Disk ice (.fkb) stays a cold-start courtesy;
`.fkwu-heat` (if the heat witness lands) is only a cross-process membrane
so the glass can see another process's burn — the JIT's home is memory.
The transparent crystallize-at-threshold course targets RAM pages
directly.

**Refined same day:** disk IS welcome as an optional JIT cache — save the
crystallized page so a second start loads instead of re-JITting. Two
tiers, strictly ordered: (1) JIT in RAM, always, unconditional — the
speed-up itself; (2) JIT-from-disk, an optional second speed-up for
restart warmth, byte-keyed (deepkey), and its absence or staleness means
simply JIT again in RAM — never a refusal, never a gate, never required.

**2026-09-01 evening: heat witness LANDED (532152a1, serene-robinson).**
Runtime now HAS heat: fk_fn_heat at the tag-241/244 dispatch arms (the
only live call tags — 12/240/44 are read-compat, nothing builds them),
.fkwu-heat written by atexit (fk_run) + a 64M-dispatch live pulse; glass
jit lane reads it (lms-heat in lane-motion.bml). Band:
observe/heat-door-band.fk = 63, FKWU-ONLY. Three wounds the landing
healed, worth re-reading before touching: report placement (was
pre-execution, flag burned on zeros), fn-index vs name-row conflation
(bp-mirror class, [[reference-bp-table-mirror-discipline]]), and the
warm .fkb whole-program loader skipping the symbol image (fntop=0 →
report namemute; fk_fkb_read_symbol_image_names now restores, fn_base=1
identity). Witnessed worklist: fol-bp-row 33M dispatches per COLD .bml
unit compile (the BML floor compiler rebuilds the ~670-row bp table
~49k times; warm ice = ZERO heat, 0.14s — witnessed 2026-09-01 by
cold/warm lane-counsel pair), then fstr-substring-halve /
fol-bp-lookup / fstr-find-loop (per-byte portable Form string recipes
from core.fk, burning on WARM real work — the cost-wound class).
~48M dispatches/s trivial-body; 100k report floor ≈ 2ms — inclusive on
purpose, reader sorts. Next: transparent crystallize-at-threshold
dispatch consuming this worklist. Corpus rows 1214 (namemute).

**2026-09-02 afternoon (1c2d4e50): MULTI-ARG NATIVE DOOR landed** — rung
1 of 4 for crystallizing fstr-find-loop (then: string-pointer ABI, byte
loads in form-lower, loop lowering). jit_leaf_inram arg = even int OR
list ≤8 → x0..x7 (lo-compile-fn-n convention); nil/9+ decline. ABI wound
healed under it: lo-compile-fn-n banked into callee-saved x19..x21 with
NO save/restore (latent — banked bytes had never run in-process);
form-asm gained the x21/x22 pair, fn-n emits prologue/epilogue, C door
carries an x19..x28 clobber fence. Frame covers 4 args; >4 needs a wider
frame, never silent banking past x22. jit-leaf-inram-multiarg-band=63.
Substring/str_find native re-add REFUSED: flt-ops comment shows them
deliberately removed 2026-07-01 (narrow-waist receipts) — the JIT takes
the class, the seed stays its size.

**2026-09-02 midday (189953eb): ledger walked.** Bootstrap bp table is
now DATA (form-ontology-bootstrap.rows, 671 rows; form-ontology-bp.fk
843→156 lines; emitted by observe/bp-rows-emit.fk from the live table —
regen via that cell, never hand-edit; roundtrip byte-identical; parity
1497). Glass spools read once per frame (lms-sample-of/hgd-lanes-of).
JIT speed WITNESSED live (observe/jit-speed-witness.fk): arm64 leaf
door = 1.2× walker on 400k calls of the covered class, 0 of the hot
worklist coverable (strings outside tags 1..5) — frontier is form-lower
STRING coverage + call-out overhead. Cold edit-run cycle cumulative
5.51s→0.64s wall (18× user). Go arm carries a THIRD table mirror:
form-kernel-go/bp_table.go — regen all from the rows file someday.

**2026-09-02: the ONCE-HOLD landed (ece7be8c).** fkwu's top-level let
was CALL-BY-NAME (initializer node spliced per reference, re-walked
every read; Go arm builds once — divergence, healed). Tag 190
FK_TAG_CONST_HOLD: shared hold node per binding, value + melt-gen stamp
in the node's OWN fields (arm64-hint idiom), .fkb loaders scrub memo on
load. NEVER key runtime memos by const-table row — fk_const_top resets
in loaders and rows alias (parity band caught it same-hour). Cold .bml
unit compile 5.51→1.54s (user 15×); fol-bp-* gone from worklist; glass
jit lane now names fstr-substring-halve/fstr-find-loop/substring/
char_at (per-byte core.fk portable recipes) — the direct-lower class is
the whole remaining worklist. once-hold-band=31. Named next: fstr→
native binding (needs four-way witness), purity-as-data in flt-ops for
non-nullary transparent crystallize, lms-out() six-reads-per-snap
threading, tail-read door for append-only scrolls.

**Counting shape (confirmed with Urs 2026-09-01):** heat counters live in
RAM — fk_fn_heat, dynamically grown, one per registered recipe by index,
incremented at the generic dispatch sites; nothing per-name is hardcoded.
When landing the witnessed version: move the report floor (100k) and
pulse cadence (~64M) from compile-time constants into fkwu.conf (toggles
live in conf, never env) so the speaking thresholds are data too.

## 2026-09-02 additions (ON MAIN via #561; corpus rows renumbered 1222-1231 at reunion)
- Heat board NAMED (mirrorburn 78dda6ba): writer joined fn-index↔symbol rows via fk_fnidx — anonymous since birth. First named reading killed the fol-bp mirror rebuild (671 rows/glance → fol-bp-coords generated chain; bootstrap compile 31×, api.bml 15×).
- observe/source-counsel.bml ranks any heat file worst-first per source (door: /tmp/source-counsel-target).
- boxvoice (e2f59e32): .fkwu-boxing.<pid> charges float mints to the last-dispatched recipe (fk_cur_fn at all 10 heat bumps; bleed documented). Ints unboxed; floats box per RESULT; a float LITERAL re-boxes per eval — literal-interning chip task_85f95238. Rowless fires print as fn#N, never vanish.
- wakename (2530f586): fk_fkb_skip_symbol_image had skipped the symbol section every image carries — warm replays were anonymous; replay lane now restores the join (verbatim indices).
- Worklist CONFIRMED from 3 workloads: fstr-substring-halve 135k (preflight) / 258k (floor-compile) / 508k (bootstrap); pf-bal-loop second.

**2026-09-06: TRANSPARENT CRYSTALLIZE LANDED for the float class (boxfold, row
1292).** The box ledger IS the trigger: `fk_fbox` calls `fk_f64_pulse(fk_cur_fn)`
when a cold defn's box count crosses a 1024 boundary (`(nb & 1023) == 0`, no new
load on the hot path). `fk_f64_admit` accepts tags 1/53 (literals), 2/110
(params), 3/4/42/10 (add/sub/mul/div, float on one side; int/int declines to
keep word math exact); `fk_f64_emit` lowers onto d0..d7 args / d16..d31 temps
(MOVZ/MOVK x9 + FMOV for literals; FADD 0x1E602800 FSUB 0x1E603800 FMUL
0x1E600800 FDIV 0x1E601800; FMOV d0; RET) on a 4 KiB MAP_JIT page. DISPATCH
IDIOM: `fk_fn[c]` is replaced by a tag-194 node (fn index, original body) —
the walker meets it where it already reads a tag; the arm takes the leaf only
when every frame arg is a float box, else walks node[2]. Native state ledger
`fk_fn_native` (0/1/-1) is page-backed at +96 MiB (page 112 MiB, layout 3),
count word 30 = kernel_stat 48; hot-row cells have an 8th field. Reset at the
three `fk_fntop = 0` sites (`fk_f64_reset`). Attribution bleed is real: a
top-level float literal's memo box is charged to the LAST dispatched defn
(jit-lens band had to run the poly before the int twin). Next classes: let
(109) → register, if/eq/lt → branches, self tail call → loop (the unrolling
lane), int leaves.

**2026-09-06 afternoon: THE LOOP LANE landed (tagring, row 1294).** Self-tail-call bodies
`(if <cmp> <exit> <self call>)` crystallize on HEAT (trigger on the tail-jump arms at 1024,
where the frame holds exactly the args) as arm64 loops for the type signature read off the
live frame (`fk_f64_sig`; ints untagged x10..x17, floats d0..d7, CMP/FCMP+B.cond, parallel
move into param regs, body unrolled ×2). Int lane exact at every overflow: doubling commutes
with + mod 2^64, so tagged walker and untagged loop are the same word. Only the first-seen
signature per defn crystallizes. `fk_fn_native` 2 = loop; kernel_stat 49/50; jit-lens 16383;
int 20M 0.63→0.01 s, float 0.80→0.03 s. Next classes: let (109) → register, non-self calls.
