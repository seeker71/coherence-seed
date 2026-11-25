---
name: verdict-fold-2-53-ceiling
description: Decimal-digit band verdicts break past 2^53 — the TS arm rounds while Go/Rust/fkwu agree; use power-of-two bit weights beyond ~15 readings
metadata: 
  node_type: memory
  type: reference
  originSessionId: 0f172d02-19f5-4224-9f29-1558861014f5
  modified: 2026-07-25T09:18:48.363Z
---

A band verdict folded as decimal digits (`1`, `10`, `100`, … one digit per reading)
silently stops being exact at the **16th reading**. `10^17` is past `2^53`, the
largest integer a JavaScript double carries exactly, so the TypeScript arm rounds
while Go, Rust and fkwu agree.

Witnessed 2026-07-25 building `core-lexicon.fk` with 18 readings:

    go / rust / fkwu   111111111111111111
    typescript         111111111111111120

Every underlying scalar was **identical on all four arms** — a folded diagnostic
of the same ten sub-values (`6404941108641210`, comfortably under 2^53) agreed
exactly everywhere. Only the encoding diverged.

**Why:** the failure looks like a logic bug and is not one, so the instinct is to
hunt the readings. Diff the *sub-values* first: if they all agree, suspect the
fold. And the direction of the error is benign-looking — `…120` reads as "two
readings off" rather than "the number was never representable", which is how it
would survive a glance.

**How to apply:** past ~15 readings, weight the fold in **powers of two**
(1, 2, 4, … 131072 → verdict `262143`), which is what
`homecoming-distillation-corpus-band.fk`'s `8191` and `offer-ack-core-band`'s
`1023` already do. Decimal digits stay fine below 10^15 and read better in a
header, so keep them for small bands. The general rule: a verdict is a
*representation*, and it needs to be exact on the weakest arm, not just the
strongest. Related: [[reference-form-comparisons-blur-beyond-2-53]] (the same
2^53 wall, reached from the comparison side rather than the encoding side),
[[reference-validate-sh-agreement-only]].

Second finding from the same build, unrelated to the ceiling: **fkwu's `--src`
reader accepted a source file missing one closing paren** and produced a
plausible verdict; the Rust arm correctly refused with `unclosed ( opened at
line 1 col 1`. A single-arm fkwu run cannot be trusted to have parsed what you
wrote — check balance, or run the four-way, before believing a right-looking
number ([[reference-axiom5-silent-lowering]], the numb-green family).
