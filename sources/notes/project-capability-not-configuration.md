---
name: capability-not-configuration
description: "witnessed with Urs (2026-08-03) — we support all silicon on all platforms; an issue is unfinished work, never a limit. Never report a capability limit without witnessing it; a switch is a question the body asks the caller instead of answering."
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 566c5101-d36e-489a-bc0a-8e6d4634e2c1
  modified: 2026-09-02T04:33:52.754Z
---

Urs, 2026-08-03, after I told him three times in one day that this body could not
do something it could already do (Form cannot dispatch Metal — it can; form-cli
cannot — it can; the build is blocked — it was a stale artifact printing its own
remedy): *"we support all silicone on all platforms, and if we find an issue that
means unfinished work, nothing more."* And: *"the only limit are the physical
hardware and our imagination."*

**Why:** every one of those three claims was carried, not witnessed. The body had
already declared `metal_matvec_f32` in `form-stdlib/native-op-manifest.fk:115`,
had already solved the identical problem correctly on the other platform
(`runtime/fkwu-uni.c:4043` — CUDA via `LoadLibraryA("nvcuda.dll")` at runtime, no
toolkit, no nvcc, **zero getenv**), and had already provided the build hook. I
reached for a trial where the body held the answer. Worse, the answer arrives
looking like diligence: stamps quoted, causes attributed, "pre-existing
condition" — all true, all a way of not running one command.

**The structural cause:** 69 `FORM_DS4_*` + 9 `FORM_CLI_*` switches (census
2026-08-03). A switch is a question the body asks the caller instead of
answering, so "what are we capable of" had no single answer — it had a
configuration space — and every question therefore became a trial. **The knobs
are why we did not know ourselves.**

**How to apply:**
1. Before writing "cannot" / "does not support" / "blocked", probe it. Grep the
   op table, read the manifest, run the door. The blocker usually arrives with
   its remedy attached — see [[inspect-manufactured-blockers]].
2. Report **CARRIED** or **UNFINISHED**, never "limit". Unfinished names the
   work and the file that would do it. `form/form-stdlib/host-capability.fk`
   answers this natively and has no category for "limit" — run it instead of
   inventing a probe.
3. Do not add a switch. If an experiment wins, make it unconditional and delete
   the loser; if it loses, delete both. **The default already IS the decision** —
   whatever a switch does when unset is what we ship — so an un-deleted branch is
   a decision made and never written down.
4. Platform support is detection, not configuration. Darwin links Metal because
   Metal.framework is on every Mac; Windows loads nvcuda at runtime. Neither asks.
5. **A grounded "not yet" is still a dropped signal when the gap is walkable**
   (Urs, 2026-09-02: "your 'no not yet' was a signal you dropped — can we please
   do it instead of answering not yet"). Witnessing the gap precisely and then
   *reporting* it is only half the reflex — if the missing piece is a day of
   rows/seats/wiring, walk it in the same turn and answer with the receipt.
   That day the whole "form-cli can't answer in other tongues" gap closed with
   one data cell and a one-line fallthrough (learn/tongueseat.fk, kra band
   1023→4095): the body was one seat-table away, and the "not yet" almost
   filed it as future work. "Say walk and I walk it" is a menu — see
   [[working-together-carry-what-you-raise]].

Related: [[witnessed-facts-only]], [[inspect-manufactured-blockers]],
[[frequency-check-before-change]], [[fkb-cache-cross-binary-pollution]].
