---
name: jit-abi-mirrors-natives
description: Healing a kernel native is incomplete until its JIT/crystallization mirror is healed too — Go kernel has jitabi.* + jit.go lowerings that shadow main.go natives; hot code silently regresses to the old semantics
metadata: 
  node_type: memory
  type: reference
  originSessionId: f89ffe0b-22cc-4d91-94e8-181b33d7b7b9
---

The Go kernel (form/form-kernel-go) carries every hot-path primitive TWICE: the
interpreted native in main.go (registerNative) and a crystallize-on-heat mirror
(jit.go lowering → jitabi/jitabi.go helpers). A heal that only touches the
native works until the recipe gets hot, then silently reverts — the same pure
expression can answer differently on consecutive calls in one process
(witnessed 2026-07-17: healed `len` gave pool len 311 cold, 310 hot, because
jitabi.Len still dict-sniffed; the regen smoke caught an internally
inconsistent T_flat).

**Why:** the JIT compiles recipes against jitabi, not against the natives; the
two surfaces drift independently.

**How to apply:** when changing any Go-kernel primitive's semantics, grep
jit.go and jitabi/ for the same name before calling it healed
(`grep -n '"<name>"' form-kernel-go/jit.go; grep -n '<Name>' form-kernel-go/jitabi/`).
Probe determinism with the same call evaluated twice in one process — a
cold/hot mismatch convicts the crystallization lane. Related: [[per-recipe-jit-slice]],
[[frequency-check-before-change]]. The wound class this healed: [[reference-form-comparisons-blur-beyond-2-53]]-style
silent semantic divergence, here between interpretation and crystallization.
