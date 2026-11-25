---
name: feedback-core-recipes-bml-not-c
description: "Urs 2026-09-12 (relayed by epic-edison): core Form recipes (int_to_str, value_str, eq, value_eq, string ops) must not reside in the C kernel — BML compiled per hardware to O(1) machine code; add no C natives for them"
metadata: 
  node_type: memory
  type: feedback
  originSessionId: de70343c-34cf-4cc9-92b9-5d1df43ed9e0
  modified: 2026-09-12T07:03:45.439Z
---

Urs's standing goal from 2026-09-12, relayed in part verbatim by the epic-edison session:

> "int_to_str, value_str, eq, value_eq etc, those and all other core form recipes shall be designed and implemented in the most generic ... hardware aware ... way for all hardware resources ... and should not reside in the kernel, and generated once per hardware platform to the core hardware optimal shape and then reused as O(1) assembly block including boxing, unboxing, recursion unrolling, loop unrolling, and all the other JIT tricks ... written in BML and compiled down to the most efficient machine code ... the C bootstrap kernel shall be minimal, and high BML grammar usage shall be maximal, and any 'cold' vs 'warm' differences are signals for optimization."

**Why:** this is the body's own 2026-07-01 direction ("the JIT takes the class, the seed stays its size"), now made the rule for every core recipe.

The same day, before I knew of the goal, I landed two C arms:
- value_str as fkwu leaf mode 25 (35bfb1583);
- int_to_str as mode 26 (43ebadfa7).

Where they stand after the rung program (main 7cbac66b1, 2026-09-12):
- `int_to_str` left the seed with rung 6 (597bb6e0c). Mode 26, its twin and its optable row are gone. core.fk's recipe asks `value_kind` (f42573e65) and never arithmetic, and it matches the old C arm on numbers, strings, lists and nothing.
- `value_str` is still leaf mode 25 in C. The rung that lets it leave is a leaf that stores the bytes of any value.

**How to apply:**
- Never add a C native (a fkwu leaf-door mode or tag arm) for a core value recipe: value_*, int_to_str / str_to_int, the eq / value_eq family, string ops.
- Write the recipe in BML/Form and let the JIT lane specialise it. A cold/warm gap is a signal to optimise, not a reason to write C.
- The goal names the C bootstrap kernel. Whether it also covers Go/Rust/TS sibling natives was not stated, so ask before adding a core recipe as a sibling native.
- Host doors such as pg and sockets are not core recipes.

Related: [[project-per-recipe-jit-slice]], [[reference-bootstrap-stamp-cascade]], [[feedback-authoring-altitude-bml-or-higher]].
