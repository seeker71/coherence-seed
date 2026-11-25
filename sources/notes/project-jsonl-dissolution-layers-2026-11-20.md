---
name: project-jsonl-dissolution-layers-2026-11-20
description: "Urs 2026-11-20 — dissolve any use of jsonl; layer 1 landed (receipt/tenant/closing ledgers are .rows), layers 2-4 sized (run+plan ledgers, requests+queue, flow verdict line + fixtures)"
metadata:
  node_type: memory
  type: project
  originSessionId: 39d16a37-d4e8-4631-9ac0-105916d511fd
  modified: 2026-11-20T13:59:47.024Z
---

Urs 2026-11-20: "dissolve any use of jsonl" (his earlier 2026-10-12 direction: dissolve JSON inside Form; see [[project-json-dissolved-inside-form-2026-10-12]] and [[project-nodes-carry-the-ledgers-2026-10-12]]).

**Layer 1 landed (355916f3c):** the ledgers and doors this loop added are Form rows now: `receipts/request-receipts.rows`, `ask-tenants.rows`, `request-set-receipts.rows`, and the tracked closing ledger `receipts/goal-closing-ledger.rows` (34 rows carried by `observe/rows-from-json-run.bml`, `from X.jsonl` / `to X.rows`, whole or not at all). `form-stdlib/bml/request-receipt.bml` builds records and writes them with `frw-emit`; doors `request-receipt-run.bml` and `gpu-tenants-run.bml` take frw-options input (`request ID`, `set STATUS`, `seconds N`) and print a form-rows line. A multi-writer ledger stays TEXT rows (nodes are single-writer, rewritten whole per append).

**Layer 2a landed (3ff1d55ec):** `ledger.bml` chooses the format by the PATH: a path ending `.rows` is written as a form-rows line (`row-line`) and read back as the typed cell (`row-cell-rows`); `ledger-append`/`ledger-rows` need no other change, so any of the 17 blueprint ledgers moves by renaming its path + carrying history (`rows-from-json-run`). `receipts/local-run-ledger.jsonl` -> `.rows` (3393 rows, all typed cells the same NodeID before and after); consumers moved: `lrun-ledger()`, planner rate reader `flp-rate-path` + its two field patterns (`prompt_ids ` / `prefill_tok_s `, the old `"prompt_ids":` spelling is gone), host-walk writer/carry lists, request-receipt.bml. `observe/ledger-band.bml` reads 23 of 23. A string-scanning reader of a ledger must change its key spelling when the ledger moves (flp-rates did).

**Still JSON, sized:** layer 2b = `receipts/local-plan-ledger.jsonl` (1171 rows) is a DEDICATED movement, not a rename: it holds 11 rung kinds (triage 365, flow 322, recheck 180, resume 99, lens 75, first 55, focus 29, reference-repair 19, reference-attention 19, voice 7, reference-reentry 2). Flow rows: `fsg-end-row` (flow-signal.bml) builds JSON TEXT and `fl-door-stamp` / `fl-crashed` edit that text (strip `}` and append) at flow-direct-run.bml 880 and 999. Planner rows: written by local-plan-run.bml (`lpr-tg-write` json-emit; option rows) and read back with the fcgp-* JSON-object readers (`lpr-plan-row`, `fcgp-str/fcgp-num`) and exact key-text scans (`"request":"id"`, `"rung":"triage"`, `"kind":"gap-closed"`, `"kind":"X","got":N,` in lpr-tg-mine / lpr-tg-write / lpr-tg-ledger-pts); more readers in local-plan-repair-run.bml and native-turn-run.bml (`nt-rungs`). Plan: one record-row writer for the whole family, readers on frw, history by rows-from-json-run, witness = per-request-id match counts old-pattern-on-JSON equal new-pattern-on-rows over all ids, then one real request live. Also request-receipt.bml's one parse-json edge; 16 more blueprint ledgers move by one rename each; layer 3 = `learn/local-requests.jsonl` (11 files, tracked, rows with embedded line breaks) and `learn/native-turn-queue.jsonl` (26 files); layer 4 = the flow's verdict stdout line (printed as JSON text, read by watchers) and the 74 tracked .jsonl files (fixtures, alignment scenarios, corpora).

**Traps:** `observe/ledger-run.bml show` reads NODE ledgers, not `.rows` text (it printed 0 rows for a .rows path); read text rows with `frw-rows`. In a request row (still JSON) a raw `"` typed into a string truncates the request the flow reads: escape as `\"`. `frw-parse` answers `["ok", v]`, never the bare value.

**How to apply:** continue layer by layer, each landed with one real request through the flow as the witness; carry history with rows-from-json-run before deleting a `.jsonl`; never `sed -i` (slipped twice this session).
