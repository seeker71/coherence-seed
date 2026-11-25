---
name: reference-make-nodeid-mechanism
description: "The complete make_nodeid / NodeID mechanism — read this before touching it, do not re-derive"
metadata: 
  node_type: memory
  type: reference
  originSessionId: faf3891e-9ee2-4b5f-b8e7-1f46da6262f1
  modified: 2026-09-14T07:43:52.151Z
---

**make_nodeid is the body's content-address constructor.** `(make_nodeid p l t i)` builds a **NodeID**
— the body's identity-by-content — from four integer coordinates: `p` = package/category, `l` = level,
`t` = type, `i` = instance. A NodeID content-addresses a cell; `bp "NAME"` (tag 45) is the by-name
lookup that returns a NodeID, `make_nodeid` is the explicit by-coordinates constructor
(flatten/form-flatten.fk header ~line 37: "the explicit NodeID door carries make_nodeid/category/pkg/
level/type/inst; literal bp calls … fold onto make_nodeid packets").

**Three layers — hold all three or you will re-dance it:**
1. **Op:** `make_nodeid` is optable tag **91, arity 4** (the ONLY arity-4 op in `runtime/fkwu-optable.h`
   / `flt-ops`).
2. **Construction** (`flt-nodeid4`, form-flatten.fk:448): lowers to
   `(fk-node 91 (fk-cons (fk-lit p) (fk-cons (fk-lit l) (fk-cons (fk-lit t) (fk-cons (fk-lit i) (fk-empty))))))`
   — a tag-91 node whose **single child [1] is a cons-LIST of the four literals**, NOT four separate
   children.
3. **Evaluation** (fk_walk `if (t == 91)`, runtime/fkwu-uni.c): `fk_walk(fk_node[i][1])` yields the
   cons-list; it walks head/tail four times to unpack p/l/t/i, then mints a VALUE node
   (`fk_np++; fk_nkind=3; fk_nid[fk_np] = [p,l,t,i]`) and returns it.

**Direct-source lane NOW MINTS (re-witnessed 2026-08-30):** the flatten-only era is over — the
source parser builds the LIST-shaped node (child [1] = cons list of the four args, see
runtime/fkwu-uni.c ~9501 "arity-4+ ops … build a LIST-shaped node") and tag-91 eval unpacks it
(~7616). `./fkwu` on a bare `(make_nodeid 1 2 3 4)` returns a boxed kind-3 value node (prints as a
negative handle — that is a healthy mint, not an error). The 2026-07-03 decline-and-drain history
(677k-diagnostic CPU-spin root) is history only.

**eq carries the coordinate law since 2026-08-30:** tag-102 `eq` used to compare node-boxed values
by raw handle, so two mints of EQUAL coordinates answered 0 (`fk_neq` knew the kind-3 coordinate
law all along; eq never asked it). Healed: nodeid/nodeid pairs now compare the four coordinates.
Witness: `form/form-stdlib/tests/nodeid-interning-band.fk` (127). The heal lit the turnwheel
band's held-out equivalence gate c22 from a distance (4194303 -> its declared 8388607).

**Since 8afd3688a (2026-09-14) make_nodeid DEDUPS.** The same four coordinates answer the same cell, in
the host field (fk_field_intern_node now hashes kind 3 like kinds 1 and 2) and in private tables
(fk_intern_tab). The key is fk_nodeid_key, which is exactly fk_deep_hash_node's kind-3 hash, so no hash
value moved. Before this, every call minted a fresh PERMANENT field cell. The glass asked for
`fgo-metric-field-category` = make_nodeid(1,2,99,1759) once per sample, about 10,000 a second, and 99.7%
of the 2^26-cell field became copies of that one NodeID. That filled the field, not its size.
nodeid-one-cell-band 7 pins it (a defn's hot-row mints column, nth 8).

**Consequence for the .tbl serializer gate:** identity is the NodeID (content-address), not bytes — so
verify a produced .tbl by **verdict / four-way equivalence** (and content-address equivalence of the
interned tree), never `diff`. Byte-identity is easy but NOT axiom-grounded.
