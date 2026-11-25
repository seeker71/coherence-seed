---
name: reference-form-comparisons-blur-beyond-2-53
description: "eq/value_eq lied on integers <= -2 until 2026-09-10 (negative ints aliased node indices); -1 was always safe, which is why it hid"
metadata: 
  node_type: memory
  type: reference
  originSessionId: 3559fc8e-37e4-4870-9974-5a25c655b215
  modified: 2026-09-10T02:47:23.521Z
---

`eq` and `value_eq` answered **1 for any two integers at or below −2** until
2026-09-10 (fixed on main, `e911d1c2`). `eq(-3,-2)=1`, `eq(-2,-4)=1`, and
`value_eq(list(1,-2), list(1,-3))=1` — two different lists, equal.

Cause: the NodeID-identity case in the tag-102 arm, and the same shape in
`fk_neq`, tested only that both words were **negative**, then read them as node
indices. An integer is `v << 1` (EVEN); a node box is `0 - ((i << 1) | 1)`
(ODD). Int −2 is the word −4, which `fk_nidx` maps to node 1; −3 maps to 2.
Both guards now also require odd words.

**Why it hid:** −1 is the word −2, which maps to index 0 and fell out at a
`>= 1` guard. Every `-1` sentinel was safe, and `-1` is the sentinel everyone
reaches for. A defect that spares the common case is a patient one.

Consequences for reading old work:
- Any verdict from before 2026-09-10 that compared distinct negative sentinels
  below −1 with `eq`/`value_eq` is untrustworthy — including green ones.
- `lt`, `le`, `add`, `sub` were never affected.
- Earlier note here (PR #273 made int/int compares exact, nil ≠ int 0) is still
  true and was **not** the whole story; #273 fixed the positive/float side.

Also pre-#273: `fkwu --src` accepted a source missing a `)` and answered anyway.
