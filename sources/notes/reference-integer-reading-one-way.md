---
name: reference-integer-reading-one-way
description: "TS folds integers in int64 past 2^53 (foldInt64), band/bor/bxor take the int64 word everywhere, str_to_int reads core.fk's way on all four; fkwu wraps at 2^63 and has no max/min/pow"
metadata: 
  node_type: memory
  type: reference
  originSessionId: de70343c-34cf-4cc9-92b9-5d1df43ed9e0
  modified: 2026-09-12T02:23:19.991Z
---

Integers across the four kernels, measured 2026-09-12 (receipt "the kernel that did what the comment said", row 1478 letterkept):
- TS `walkMath` folds bare-width integers in JS numbers while every step stays within ±(2^53−1). A bigint operand, or a step that leaves that range, refolds in BigInt wrapped at 64 bits (`foldInt64`), and `intOrWide` answers a plain int when it's safe and i64 otherwise. All four agree on `(mul 100000000000 100000000000)` = 1864712049423024128.
- `band`/`bor`/`bxor` combine the whole int64 word on Go, Rust and TS; TS's `bitwiseInt` uses hi/lo halves, and BigInt past 2^53. Only the `_u32` natives narrow to 32 bits. The "32-bit-unsigned semantics" comments in Go and Rust described no code.
- `str_to_int` has one reading, core.fk's:
  - space, tab, LF and CR are skipped;
  - one leading "-" is allowed;
  - digits run to the first non-digit;
  - no digits, or a leading "+5", reads 0.
  Go, Rust and TS carry it as `leadingInt`/`leading_int`. Band: form-stdlib/tests/str-to-int-reading-band.bml, 127, rowed.
- TS `value_eq` keeps the integer and float families apart (1.0 ≠ 1), matching the Go and Rust same-kind rule; integers compare across widths.
- fkwu integers wrap at 2^63:
  - `(mul 3037000499 3037000499)` is -5928526807 on fkwu and 9223372030926249001 on Go, Rust and TS;
  - `(add 9223372036854775807 1)` is 0 on fkwu.
- fkwu has no `max`, `min` or `pow`: they aren't in core.fk and there's no native, so they read nothing there.

**How to apply:**
- Compare integer results four-way with a probe of one value per line.
- In TS natives, read possible i64 values through `expectBigInt`/`listElemInt`, not `argInt`: `argInt` converts to a Number, which loses precision past 2^53.

Related: [[reference-float-rendering-one-rule]], [[reference-verdict-fold-2-53-ceiling]], [[reference-form-comparisons-blur-beyond-2-53]].
