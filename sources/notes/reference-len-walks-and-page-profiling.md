---
name: reference-len-walks-and-page-profiling
description: "fkwu len is O(n) (nil? = (eq (len xs) 0) made every list recursion quadratic) — healed by a compare-against-len peephole 2026-09-06; the live page is the profiler: kernel_page_hot pid n on a running process"
metadata:
  type: reference
---

`len` (tag 22) walks the cons chain; `nil?` in core.fk (and sha256.fk, line-grammar.fk, form-asm.fk,
fourth-shim.fk, core-native.fk — six copies) is `(eq (len xs) 0)`, so `append`/`map`/`take` and every
BML fold were O(n^2). Since 333c2423 the eq/lt/le arms (tags 102/103/5, both operand orders; gt/ge lower
onto le with the literal on the LEFT) call `fk_len_cmp` → `fk_len_upto(v, K+1)`: walk at most K+1
cells, child evaluated once, identical answers (probe 8 kinds: empty/one/""/"abc"/0/5/1.5/nested).
`(eq xs (list))` would be O(1) but CHANGES nil? for strings/ints/floats (1 → 0) across four kernels —
do not "fix" nil? in core.fk. Numbers: nil? ×100K on 10K list 300→6 ms; append ×200 4677→161 ms;
corpus band cold 2.30→1.87 s. `len` used as a value is still O(n).

**Profiling a running kernel:** run it (`</dev/null` for no tty), then from another fkwu
`(kernel_page_hot <pid> 24)` → (heat name unit line col boxes unboxes native) and `(kernel_live pid)`
words (4 dispatches, 18 cpu-us). Dispatches per CPU-second rising while the count rises = a cost that
lived under the counter (C time inside one arm), not in it. Corpus rows 1303 mixblind (sibling), 1304 lencap.

**Why:** the hottest defn on the glass was `nil?`, not a glass recipe; the whole body paid.
**How to apply:** before optimizing a recipe, read the page; check whether a native under it walks.

**String melt (2026-09-06 evening, deadslot row 1306):** `fk_smelt` runs at the end of `fk_melt`: marks local
string indices reachable from the melt roots (fk_vs, fk_mem, records rval/rbp, nodes when !field) + raw-index
holders (fk_rkey, tag-24 literal nodes), frees unmarked slots to `fk_sfree` (`fk_sdead` bitmap); `fk_sintern`
reuses a freed slot (and its bytes when the new string fits and the scratch is at the top). Live strings
NEVER move (shared-store readers). Hazard class: an arm holding a string INDEX in a C local across a later
child walk — eleven arms (26/27/28/30/66/98/197/198 …) now `fk_vp` the held word around the walk; any NEW
arm that walks a child while holding a string or list must push it (tag 23's idiom). A top-only rollback
reclaims nothing (the newest string is live mid-computation). `kernel_stat 51` = reclaimed slots. The glass's
FGLStringMoltGrowth selfmolt (262144) no longer fires: strings flat at ~33K. `fk_clo_capvals` are not in the melt loop, but a closure holding a 50-list survived 21 melts intact (probe 2026-09-06, captures are delivered into frames) — not a wound. Old note: `fk_clo_capvals` are NOT melt
roots (pre-existing; closures holding lists across melts — unwitnessed, worth a probe).

**Recipe twins (2026-09-06 evening, recipetwin row 1307):** THE TAG SPACE IS FULL AND USED — tags 12/240/44
are live call arms (12 = single-arg call by name; the "read-compat, nothing builds them" note was wrong),
150 is the native-surface probe. The door for a native recipe is the defn's own body node: `fk_twin_pulse`
(heat 1024, on the non-tail call arms too) binds nil?/append/int_to_str/reverse-onto by NAME + ARITY +
DEFINING UNIT (core.fk, line-grammar.fk, sha256.fk, fourth-shim.fk, core-native.fk, form-asm.fk) →
`fk_fn[c]` = tag-194 node, `fk_fn_native[c]` = 3, twin id in `fk_f64_sig[c]`; the 194 arm calls
`fk_twin_call` and walks the body when it DECLINES (int_to_str of float/string/list, reverse-onto of a
non-empty string). Witness the recipe's edge answers on the old binary BEFORE writing a twin (append of a
non-empty string is (cons nil ys); int_to_str of a float is "" / "-"). `fk_cons_val` never melts (it
grows), so native list building needs no pushes. `kernel_stat 52` = twin calls; glass label " twin".
Adding a twin: extend fk_twin_id_of + fk_twin_call; keep the six-unit guard.

**Framebuffer ring (2026-09-06, rootring row 1309):** `fk_fbroots` is a ring of FK_FB_RING = 2048 (tag 128
records `fbn % ring`, tag 129 `framebuffer-events` conses the newest ≤2048 oldest-first). Before, the history
only grew and the glass consed it every tick (the 69→207 ms climb, and the 262144 node-selfmolt clock). Still
named: some glass site mints ~3 roots/tick (nodes into the SHARED FIELD ~8/tick field-wide; the field never
melts, 2^26 ceiling ≈ 90 h) — find it by counting `intern_node_at` callers per tick; the cell doors take REFS
not indices (`cell_ref`), so "read the newest nodes off the store by index" guessed the encoding and read
noise. Strings under the string melt climb between melts (84K after 60 s of the glass) because melts fire
only when the heap fills and the heap cap doubles as live grows — bounded by heap cadence, not zero.

**Mint lane (2026-09-06, mintlane row 1311):** `fk_mint_total` counts a fresh permanent cell where it is
minted (the seven `fk_np = fk_np + 1` sites and `fk_field_fill`); live page word 33, `kernel_stat 53`, glass
rows `nodes-minted` (own) and `value nodes minted` (per kernel from its page). Class witnessed: a row built
with the SAME id mints 0; a fresh id mints 2 (the interned id + its projection node) — the arena grows where
identity is new, not where writes are frequent. Rates: machine sensor 0 / 15 s, organs 0, host sensors 48,
live glass loop 1038 / 20 s (~2 a tick, inside the RENDER — all 215 published row ids are stable second to
second, checked by diffing frame ids). Do NOT try to attribute arena growth by differencing a page's node
word: it is only current at note moments (open, field open, melt, exit, self-read), and the field counter is
host-wide across every kernel.
