---
name: project-bml-surface-directions-2026-09-29
description: "Urs 09-29 decisions for BML: cursor is the one reader, compiler = transmuter over its nodes; ++ ok; unspaced a+b works; destructure + destructor/Disposable; >=10 sites per construct; 10x was a guide"
metadata:
  node_type: memory
  type: project
  originSessionId: 39d16a37-d4e8-4631-9ac0-105916d511fd
  modified: 2026-09-29T14:42:20.687Z
---

Urs, 2026-09-29 (after the 10x cut landed as 853f0a8d8):

- The compiler must READ THROUGH THE CURSOR. The cursor (form-bml.fk + form-bml-lower.fk over bmf-grammar/bmf-core)
  stays generic; the compiler is the transmuter that receives the program context and every named identifier as
  nodes. "Two lanes" (cursor grammar + the fsc-* statement walker in source-compiler.fk) is not the shape he wants:
  new syntax lands once, in the cursor, and R144 (walker released) is the prerequisite.
- Accepted: `++` as string join; unspaced `a+b` must work (hyphenated names like `a-b` stay whole).
- "Destructive and Disposable are important for resource management": read as destructuring plus a
  destructor/Disposable protocol (scope-exit cleanup that also runs when the body stops). I chose both readings.
- Add the new keywords AND remove the older longer spellings; find at least 10 real use sites for each keyword and
  operator added, so each earns its place.
- The 10x reduction was a guide, not a hard rule (no pressure to cut further; restoring a survivor is fine).

**Why:** the walker hid a wrong recipe once (`1e-12` as an ident); one reader leaves no second reader to make two
wrongs agree ([[project-cursor-reads-every-section]]).

**How to apply:** before adding syntax, check it reads through the cursor; do not add a fsc-* walker arm. Count the
sites a construct replaces and show the count. Keep the LoRA curriculum ([[project-local-agent-goal-255]]) pointed at
the final surface, after the grammar settles.
