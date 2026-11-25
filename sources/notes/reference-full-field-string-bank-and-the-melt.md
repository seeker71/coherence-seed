---
name: reference-full-field-string-bank-and-the-melt
description: "The shared field has no fallback since 2026-10-08 (field version 5): strings and pairs grow by segments, floats and indexes stop by name; a field node never holds a local word, so neither melt walks the field"
metadata:
  node_type: memory
  type: reference
  originSessionId: 533ece4b-c058-4748-885c-284ca4160f07
  modified: 2026-10-08T13:24:23.885Z
---

How the field and the melts fit, end to end (runtime/fkwu-uni.c, 2026-10-08, landed 625326909):

- **Two stores.** A kernel's LOCAL store (string pool, cons heap, float pool) is reclaimed by its melts: `fk_melt` copies live pairs from roots, `fk_smelt` frees unmarked strings (slot zeroed, bytes become holes). The SHARED field (shm `/fg-field<version>-<letter>`) is content-addressed and append-only: nodes, pairs, strings, floats and their indexes. Nothing in it is ever freed; `observe/field-reset-run.fk` (only with no other kernel alive) is its only reclamation.
- **The invariant:** every word a field node or field pair holds is a shared word. `fk_field_share_value` copies strings (`fk_field_share_string`), floats and lists (into field pairs) when a node is filled. Because of that, the melts never need to look at the field.
- **What broke it:** when a bank was full, sharing answered the LOCAL word (the fallback). The field node then depended on a local string or pair that no melt root named; the string melt freed it, and new .bml definition names read back as "".
- **Now, no fallback:** string bytes grow by segments (a sibling, field version 4); pairs grow by segments the same way (`fk_field_pair_span`, readers map past their extent in `FK_HH`/`FK_HT`, version 5); floats (a fixed 2^25 index), string metadata, node columns and the intern indexes `fk_die` by name, pointing at the reset. `fk_ndep` (a per-node dependency list I added first) was removed: with no local words in the field there is nothing for a node to carry.
- **Readings:** `kernel_stat` 69 melts, 70/71 shared string bytes/slots, 72 shared pairs claimed, 73 pair segments mapped by this kernel, 57 shared floats.
- **Proof recipe:** a scratch build with `FK_FIELD_PAIRS (1LL << 12)` and `FK_FIELD_VERSION 905` (its own namespace) crosses segments in every band. That test field still sits in shm (the reset door refuses while any kernel lives). A version bump starts a fresh field for new binaries; old binaries stay on theirs.
- Debug path that found it: diff floor output fresh vs cached, count the name's bytes in `.discovery.fkb`, toggle `FK_FIELD_OFF 1` in `fkwu.conf` (the env var does not reach `fk_conf`).

Related: [[reference-shared-field-ceiling]].
