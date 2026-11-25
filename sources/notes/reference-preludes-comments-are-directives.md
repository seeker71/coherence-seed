---
name: reference-preludes-comments-are-directives
description: "a '; preludes:' comment line in a .fk file is a load directive read by fk_src_collect_preludes — one line only, followed recursively; a wrapped or incomplete line runs the band numb with a silent wrong verdict"
metadata:
  type: reference
  node_type: memory
  originSessionId: bf8f336c-8632-4236-b6d7-7040d31aebde
  modified: 2026-11-25T04:09:30.134Z
---

`; preludes: a.fk, b.fk` in any .fk file is not documentation — `fk_src_collect_preludes`
(runtime/fkwu-uni.c:10441) parses it as a load directive, ONE line only (a wrapped
header silently loses its tail), and walks each prelude's own header RECURSIVELY. So:

- Run a band as `./fkwu --src path/to/band.fk` and let the resolver assemble the chain.
  Hand-catting preludes double-loads files (each cat'd file's own header fires too) and
  changes behavior.
- BUT `fkwu --src <band>` is NOT the full door, and believing it is manufactures phantom
  findings. It walks ONLY the `; preludes:` chain. `form/validate.sh <band>` additionally
  expands the band's DECLARED IMPORTS (`fk_expand_declared_deps`) and hands the arm the
  full ordered list. Bit me 2026-07-21 verifying equireach: `--src` left 18 unresolved-call
  diagnostics (fq-pow2, tn-*, tb-*, rope, TAU) that I wrote up in a committed receipt as
  branch drift; through validate.sh both bands resolve completely with ZERO diagnostics.
  **To judge a band's diagnostics or its verdict across arms, run `validate.sh <band>`;
  reserve raw `--src` for timing harnesses where you control the prelude list yourself.**
- A file you never named can enter the image through a transitive header (e.g.
  thought-framebuffer.fk pulls form-stdlib/core.fk); grep the whole transitive chain
  when hunting a load-order defect.
- Two files "identical modulo comments" can run to different answers — comments carry
  load semantics. Proven 2026-07-17 during the [[jacobian]] j-lens work: core.fk arriving
  via a comment line broke every oac-offer (see the call-position shadowing defect,
  receipts/2026-07-17-jacobian-lens-and-the-cell-shadowing-heal.md, healed by renaming
  the parameter; runtime fix pending). Related: [[frequency-check-before-change]] — the
  collision was a NAME frequency question in the end.
- The "one line only" rule bites at scale (measured 2026-07-17 floor reground): 151 band
  files declare preludes MULTI-LINE (`; preludes:` bare, paths on `;   ` continuation
  lines) — the reader loads an EMPTY list and the band runs numb via axiom-5, returning
  a silent wrong verdict (host-os-membrane-band answered 0 instead of 8191 until its
  line was joined). Find them: `grep -rl '^; preludes:[[:space:]]*$' */tests/*.fk`.
  Also: a band whose preludes line merely OMITS a needed file fails the same silent way
  (oracle-catalog-band answered `nothing` until core.fk was added for `sum`). See
  [[bp-table-mirror-discipline]] for the sibling wound (numb form_error).
