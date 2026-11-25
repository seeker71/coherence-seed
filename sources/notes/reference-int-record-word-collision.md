---
name: int-record-word-collision
description: "HEALED b2ebcf743 (2026-09-13): fkwu records box to fk_rbase - (r<<1) - 1 (fk_rbase -7e18), their own odd band, so no int wears a record's word; value_kind(-1) = int on all four kernels (record-word-band 15); Go/Rust/TS DO have records (typed values)"
metadata:
  node_type: memory
  type: reference
  originSessionId: f784a9b1-bfab-4fa2-a520-95b2ae087e9c
  modified: 2026-09-13T10:22:19.001Z
---

**What is (since b2ebcf743, 2026-09-13):** `fk_rbox(r) = fk_rbase - (r<<1) - 1` with `fk_rbase = -7e18`. The word is
odd and lies between the fn-value band (at or below -8e18) and every node box. `fk_ridx` reads only that band, and
`fk_isrec` bounds r by fk_rp. An int is `n<<1` (even), so no integer can be a record's word, and value_kind and
record? read a kind from the word alone.

Consequences:
- (eq rec -1) is 0; a record still equals itself.
- add/lt on a record stop (FK_ARITH_REFUSAL / FK_ORDER_REFUSAL), as on any non-number.
- The loop lane types a record 0 (declines).
- The kind fold (53a58d319, row 1526 knownkind) folds every name for an int operand. Its interim int/record
  exception is gone.

**The wound it healed (row 1525 kindshadow, 740ea5e1c):** records were `0-(r<<1)`, the word of integer -r. So once r
records existed, value_kind(-r) said "record" and record?(-r) said 1, a kind read off a population count.

**Correction to the original note:** Go, Rust and TypeScript DO implement records: `record_new` is registered in all
three, and they hold `core.Record` / `Value::Record`. There value_kind(-1) was always "int", so kindshadow was a
four-way DIVERGENCE a band could reach, not a wound invisible to the gate. `form/form-stdlib/tests/record-word-band.fk`
(four-way, registered at 15) reads 15 on all four kernels; fkwu read 4 before the heal. The repair was two
functions and three comments in fkwu, not "a value-encoding change across every kernel".

**Lessons:**
- Count where a representation is built before calling its repair large.
- Verify a claim about sibling kernels by grepping their registerNative before repeating it.

The form-cli carrier kernel (form/form-stdlib/bootstrap) keeps the old fk_rbox but has no record_new, so it never
mints a record. Receipt: `receipts/2026-09-13-a-record-gets-its-own-band.md`, row 1528 ownband.

Consequence seen 2026-09-28: code that turned a record into text with int_to_str(owner) (for a flow id) died with
"order: only numbers have an order" once records became their own kind — metal-ask.bml (every refusal since 09-13),
native-pipe-darwin.bml and dsv4-oracle-input.bml. Healed by minting an integer "id" field (host_monotonic_ms()) in
the owner record and reading record_get(owner,"id"). Grep `int_to_str(owner)` before trusting a door with a record.
