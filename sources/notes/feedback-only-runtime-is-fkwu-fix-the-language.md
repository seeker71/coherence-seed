---
name: feedback-only-runtime-is-fkwu-fix-the-language
description: "Urs 2026-09-15 — no CPython anywhere, no walker or other runtime detour; a language flaw is fixed in the language, never in its users"
metadata: 
  node_type: memory
  type: feedback
  originSessionId: d7d665d0-ee49-4194-b96b-f85d64a45006
  modified: 2026-09-15T01:11:22.737Z
---

Urs, 2026-09-15, correcting advice that proposed fixing the Python compiler, moving a walker onto fkwu, and patching compiler output around nested-let scoping: "we do NOT want CPython anywhere for sure, the only runtime we want is the form native C-bootstrapped kernel"; "we don't need a walker: we have a form native JIT … focus on the best form native JIT active path, not any other detour"; "hitting this means fixing the language design and implementation, not the use of it, that is completely backwards"; a Python compiler has no use unless it is 100% Form-native generating Form recipes.

**Why:** a reference runtime (CPython, node) or a second walker keeps the thing being left as the judge of the body; patching users leaves the language flaw in place for the next author.

**How to apply:** when a check needs CPython/node or a sibling as runtime, release it rather than heal it. When a language behavior surprises a caller, go to the language design (BML north star) and the kernels' implementation. Advice points at the Form-native JIT path first.

Related: [[feedback-heal-integrate-or-release]] [[feedback-core-recipes-bml-not-c]] [[reference-fkwu-do-keeps-let-siblings-leak]]
