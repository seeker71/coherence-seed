---
name: reference-fkwu-add-passes-non-numbers
description: "since 03a03f6f4 add/sub/mul/div/mod stop on a non-number on all four kernels (before it fkwu's add passed a string through and mul gave a garbage int); tell a number by value_kind, which the form-cli walker lacks"
metadata: 
  node_type: memory
  type: reference
  originSessionId: de70343c-34cf-4cc9-92b9-5d1df43ed9e0
  modified: 2026-09-13T10:49:54.487Z
---

Witnessed 2026-09-12 on rung 6's fkwu, with the value passed as a defn parameter:
- `(add "abc" 0)` → `"abc"`, and `(add (list 1 2) 0)` → the list. So `(eq (add n 0) n)` is 1 for strings and lists too.
- `(eq (sub (add n 1) 1) n)` is also 1 for strings and lists.
- `(mul "abc" 1)` → `-4250000000000000002` and `(mul (list 1 2) 1)` → `16`: garbage ints from the value's tagged word.
- `(eq (mul n 1) n)` is 1 for ints, both tagged limits and floats, and 0 for strings, the empty string and lists.
- `fk_num` (runtime/fkwu-uni.c) reads a non-float word as `v >> 1`. Arithmetic on a string never fails; it answers a number.

Why it mattered: rung 6 moved `int_to_str` out of C into core.fk. The recipe tested floats arithmetically (`fstr-float?`) and sent everything else to the digit loops. So `int_to_str "abc"` answered a garbage number where Go, Rust, TS and the old C arm pass the string through; TS notes that emit-engine.fk relies on that. `(eq (add n 0) n)` did not fix it. `(eq (mul n 1) n)` did.

**Since 03a03f6f4 (2026-09-12, landed on main) this no longer holds:** fkwu's add, sub, mul, div and
mod over a non-number stop with "arith: only numbers add, subtract, multiply and divide -- ask
value_kind first", as the Go, Rust and TS siblings always stopped. `(mul "abc" 1)` stops; it is no
longer a number test anywhere.

**The string sibling, since 4989307ac (2026-09-13):** str_len, str_byte_at, str_concat, str_eq,
str_to_float and byte_to_str stop on a wrong-kind argument, as on Go, Rust and TS. Before, they answered
0, -1 or ""; `(str_eq 5 6)` was 1. str_eq still admits `nothing` (str-eq-absence-band, #577: an absence
equals only an absence). str_byte_at past the end still answers -1, and byte_to_str past 255 answers "".
string-op-stops-band 127.

**How to apply:**
- Tell a number from another value by asking its kind: `value_kind` answers int/float/string/list/null
  the same on all four kernels.
- Before landing a stop on a primitive, sweep every registered band with a trace build that keeps the
  old answer and logs each hit. That is how 03a03f6f4 and 4989307ac found their only dependents.
- A runner that reads `(read_line)` needs its input on stdin. Run with `</dev/null` and it measures
  nothing and stops. That is not a red band.
- A walker that has no `value_kind` (the form-cli walker, per rung 6) needs the door, not an
  arithmetic identity.
- Probe a type test with the value passed through a parameter, since a literal may be folded differently.

Related: [[reference-integer-reading-one-way]], [[reference-bootstrap-stamp-cascade]], [[feedback-core-recipes-bml-not-c]].
