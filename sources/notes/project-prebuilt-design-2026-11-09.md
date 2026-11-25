---
name: project-prebuilt-design-2026-11-09
description: "Prebuilt kernel + warm packs LANDED 2026-11-09 (afcb161af, 467cc9b6a): identity = seed digest + OS-arch (flags don't matter), Linux arm64/x86_64 build and run, float_leaf 65 sha256 / 66 identity, observe/prebuilt-run.bml publish+fetch, pinned in prebuilt/index.txt; fresh host 54 s -> 4 s"
metadata:
  type: project
---

Urs 2026-11-09: prebuild per OS+arch, regenerate on source change, weigh size vs speed freely, HIGH-ORDER design over exact numbers,
bytes outside git (public disposable hosting is fine; archive.org is permanent, pastebin is text-only, GitHub Releases can expire).

**Witnessed:** -O2 is the flag (O3 +7% cold, Os +16%, O1 slower warm; kernel 711 KB, 3.6 s cc). Lowering AND image bodies are byte-identical
across O1/O2/O3/Os (only the stamp differs), so the key is seed digest + target, not exe bytes (a plain `cc` kernel still follows exe bytes).
Builds reproduce byte-for-byte from any dir, on macOS and Linux. A seed-version ./fkwu in a fresh HOME never populates the store (only a
hop-build does). Cross-platform: 9 of 14 lowering units identical darwin vs linux; the 5 that differ differ only in `$lambda-@..<n>` names
(source-compiler.fk:671 embeds value_str of a recipe node = allocation-dependent id): lowering bytes are NOT deterministic even on one host.
OPEN: derive that name from content. Packs stay per target until fixed.

**Landed:** Linux compiled+ran on arm64 and amd64 (own struct timeval collided with glibc; getuid undeclared; `/dev/shm` 64 MB default
raised SIGBUS because the field maps sparse shm: kernel now keeps tables private below 2 GiB free shm). float_leaf 65 `[path limit]`
= sha256 hex, 66 = kernel identity string "fkwu-image-v1 seed <key> <os>-<arch>". observe/prebuilt-run.bml: `publish <dir>` (what a warm run
touched since T, `compress 1` = zstd -19: 81 MiB -> 11), `fetch <mirror>` (dir or http(s) via curl; manifest hash pinned in
prebuilt/index.txt, objects checked vs manifest; tamper and wrong pin refused). Fresh home+checkout: fetch 7 s, band 4 s, built 0 (cold 54 s).
Store-root home limit now leaves 600 bytes (gcc format-overflow warnings).

**Open:** nothing published (publishing is Urs's call: GitHub Releases / archive.org need his account); no CI workflow; fetch is ~2 spawns per
object (batch it); kernel bootstrap on a bare machine is one curl + sha256 check outside the body (documented in BOOTSTRAP.md/AGENTS.md);
Windows/Android/x86_64-darwin rows unbuilt; manifest-per-release churn in git is a line per pack.
`door-links` still red on main from #415 (nl-tongues.fk, text-frequency.fk) so `observe/land-run.bml` stops at "verification needs local care"
after committing; I rebase and `git push origin HEAD:main` (no force) after reading the same gates. Another cell's work, left.

**Process lessons:** Bash is zsh: `$cmd` does not word-split (call compressors explicitly). macOS has no setsid; use run_in_background. alpine:3.20 has no httpd
applet (apk add busybox-extras). `docker run ... httpd` failing is the container, not the code. A vacuous check (two empty hashes equal) said "yes": test
that the compared value is non-empty.
