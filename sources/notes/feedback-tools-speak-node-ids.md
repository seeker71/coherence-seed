---
name: feedback-tools-speak-node-ids
description: "Urs 2026-10-04 — body tools should be aware of Form nodes, cells and blueprints and pass IDs between stages instead of serializing text; text is only the terminal edge"
metadata:
  node_type: memory
  type: feedback
  originSessionId: 533ece4b-c058-4748-885c-284ca4160f07
  modified: 2026-10-05T03:09:43.146Z
---

Urs, 2026-10-04, after the grep/git/cell_run migrations:

> "we can more deeply integrate all tools by making them aware of form, form nodes, form cells, form blueprints, we don't have to serialize, we can send IDs, so many tasks can become much, much simpler"

**Why:** the migrations removed child processes, but tools still take and return text (grep-text(args, text), jsonl rows, `.out`) and re-parse between stages. A node ID is already a content-addressed reference; a result set can be a node whose children keep source ID + offset, so provenance is free and no stage re-parses.

**How to apply:** new or migrated tools accept text OR a node ID and return a node ID with on-demand text rendering; rows (membrane site, band verdict, git commit) are blueprint-shaped so fields are asked by name; render text only at the edge shown to Urs. Check what is same-process vs shared-field stable before passing IDs across `cell_run` boundaries. Slice in progress: `form/form-stdlib/bml/form-agent-nodes.bml` and a section in `docs/form-native-agent-tools.md`.

Reinforced 2026-10-05: "we also cross membrane boundaries too often, not relying on native tools, not adapting the native tools to what form already knows natively, we don't need to pass strings and serialize and then parse again ... we can adapt any tool to return exactly what the caller needs, with no parsing required at all". Apply: when a body door renders text that another body door parses, change the producer to return the value; keep text only at the terminal edge or a persisted artefact. Census + guard: `observe/roundtrip-census-run.bml`, `docs/typed-returns.md`. Built so far: form-find (index), form-path (node ids with spans).

Related: [[feedback-no-child-processes-cells-run-in-process]], [[feedback-form-native-tools]].
