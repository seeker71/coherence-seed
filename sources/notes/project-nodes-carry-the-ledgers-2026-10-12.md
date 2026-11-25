---
name: project-nodes-carry-the-ledgers-2026-10-12
description: "Working ledgers are Form nodes at rest (FORMBIN2) with form-rows text only as the view — fvn-* codec, atomic write_form_binary, what moved and what stays text"
metadata:
  node_type: memory
  type: project
  originSessionId: 39d16a37-d4e8-4631-9ac0-105916d511fd
  modified: 2026-10-12T07:16:04.078Z
---

Urs 2026-10-12: "how is having rows the most alive and dynamic and expansible way to carry knowledge; that seems very odd and mis aligned" then "nodes should be faster". Answer built the same day: `form/form-stdlib/bml/form-value-nodes.bml` (`fvn-of/fvn-to/fvn-field/fvn-ledger-append/fvn-rows-of/fvn-text`, band `form-value-nodes-band` 127), door `observe/ledger-run.bml` (show / from-to).

**Measured (3000 rows, loop's shape):** nodes built 25-34 ms, `write_form_binary` 2 ms, `read_form_binary` 3 ms; text rows 130 ms write + 55 ms read; Form-level FORMBIN2 encode 654 ms (the native door does it in 2).

**Traps found:**
- A row built with raw-word kids (strings/ints straight in `intern_node(cat, ["k", 1])`) is read back from disk with node kids (`@1.1.2.x`) — a DIFFERENT node. Build rows with `intern_trivial_*` / `make_float64` kids (that is what `fvn-of` does) and the id survives the disk. `node_eq` of built vs read said 0 until then.
- `write_form_binary` used O_TRUNC in place (a death mid-write wipes the ledger); runtime mode 8 now writes `path.tmp` and renames over. runtime/fkwu-uni.c has no stdio: build names by hand, no snprintf.
- A node ledger is rewritten whole per append: single writer only. Multi-writer ledgers (`.hearth/form-tokens/crossings.rows`, appended by any process running a voice) stay text until FORMBIN2 has a shareable append.
- The time door summed BOTH `validation` events (baseline before + held-out after) under "baseline validation"; the 148/69/93 s figures in receipts/2026-10-11-form-token-time.md were the after-training check once the baseline is remembered. Door now splits them (optimizer_step 0 vs not). Open: why the after-check costs ~0.66 s a row vs 0.2 s training forward (read on a quiet GPU).
- My trainer proof runs append rows to tracked `receipts/form-token-lora.rows`; `git checkout --` it before committing.
- Principle used: working state (.hearth) = nodes; receipts and the committed trace corpus = text documents read in git.

Moved: `training-events.fkb`, cycle `ledger.fkb`, `baseline-<key>.fkb`. Not moved: dataset/assessment `.rows` (JSON-dialect readers), receipts, corpus, crossings.

See [[project-json-dissolved-inside-form-2026-10-12]], [[feedback-tools-speak-node-ids]], [[project-form-token-time-analysis-2026-10-11]].
