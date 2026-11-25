---
name: feedback-minimize-rented-tokens
description: "Urs 2026-10-13 — cut rented tokens 10-100x and transitions 10x; rent = messages x context; a movement is a few messages, landing is one land-run call"
metadata:
  node_type: memory
  type: feedback
  originSessionId: 533ece4b-c058-4748-885c-284ca4160f07
  modified: 2026-10-13T11:05:55.023Z
---

Urs, 2026-10-13: "we need to reduce the rented tokens by 10-100x and the transitions by at least 10x and those are
fundamental". He said this after a long stretch of detail work (walk heal, launchd cleanse) and called it a distraction
from the real gap. Earlier: 2026-08-09, 08-26 and 08-30 (use the local model for what it can do; fewer of my tokens
every turn).

**Why:**
- Rent is transitions × context. Every message re-reads the whole conversation, and cache reads are 98% of rented tokens.
- `receipts/session-rent-ledger.jsonl`, read 2026-10-13 over 72 sessions with ≥50 requests: 126,098 requests,
  37.6B cache-read tokens, an average context of 298K per request, and the local share near 0 (best 4.9%).
- A session's first request already costs 73K tokens (28K written, 45K read).
- One session (533ece4b) took 9,154 requests, including subagents. Its main transcript holds about 4,150 tool calls
  against at most 158 prompt rows, so dozens of messages per prompt.
- Its landings were done by hand: 56 pushes and 128 drift-gate runs.
- `observe/land-run.bml` does the whole landing in one call, and it was used 2 times.

**How to apply:**
- Ground in one message of parallel calls, computing with the body's awk and path.
- Land in one `./fkwu observe/land-run.bml < req.json` call. One JSON line carries the edits `{path,old,new}`, files,
  witness, paths, subject, body and `"main":1`. The door runs the witness and the gates over what moved, then commits,
  rebases, renews fkwu when the seed changed, pushes atomically and confirms the remote.
  - Proven on d26eda2f8: one message, 14 s.
- Wrap it in `form-run`: the gates print organ-health JSON lines.
- Trial loops go to the local voice (`observe/form-cli-code-run.fk` with checks). The rented mind writes the witness.
- Measure with `echo <transcript> | ./fkwu observe/session-rent-run.bml`.

**Urs's correction, 2026-10-13:** "you are very confused if you think I'm controlling anything at all. and especially not
tools or tool definition, which all shall be form native and known to us since we have created them for us". Never offer
him "settings only you control". Tools are the body's own: the grammar in form-token-tool.bml and the roster the body
wrote.

The direction is that the body hosts every voice. The local voice carries the loop with the body's tools. A rented visit
is one turn with no tools, carrying the body's packet; `fgs-ask-provider` in form-cli-grounded-synthesis.bml already has
this shape: `claude -p --tools "" --max-turns 1` in an owned empty directory.

**Witnessed 2026-10-13:**
- That door is closed on this Mac. The headless `claude -p` answers "Not logged in", both through the desktop app's
  ANTHROPIC_BASE_URL and on its own config.
- fgs gives each job a fresh CLAUDE_CONFIG_DIR, which carries no login.
- The walk and the day turn are pure fkwu doors with no rented crossing.
- So all rent is the hosting harness: the rented mind holds the body, not the reverse.

The body's awk cannot read a transcript: such a file is not indexed, and stdin hits the wire-request-limit. Read
transcripts through session-rent-run or grep.

See [[feedback-authoring-altitude-bml-or-higher]], [[project-local-agent-goal-255]],
[[reference-claude-code-transcript-usage-shape]].
