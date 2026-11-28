# Spoken turns

A speaker offers a single slot in an ordered turn. The offer binds the
turn, slot, sample rate, channel count, source language, wording, synthesizer
and exact PCM bytes. The recipient invites that offer through the circle
broker. Changing the wording or audio requires a fresh invitation. The slot
identity stays stable across revisions so two different deliveries cannot
occupy the same position.

| Entry point | Inputs | Result |
| --- | --- | --- |
| `voice/prepare.ck` | Request on stdin, channel on fd 3, signed 16-bit little-endian interleaved PCM on fd 4 | Owner permit covering the exact clip |
| `voice/ticket.ck` | Permit on stdin | Ticket for the listener and receiver |
| `voice/expect.ck` | The same permit on stdin | Expected full-letter ID, which the recipient can retain independently of the delivery location |
| `voice/render.ck` | Queue of `voice.clip` entries pairing each recipient path with its retained ID on stdin, byte sink on fd 5 | PCM in order on fd 5 and a measurement on stdout |

The recipient selects letters from its trusted delivery directory. Before
writing, the renderer checks the whole selection for missing slots, mixed
turns or formats, incomplete frames and conflicting duplicates. It checks
each complete letter against its independently supplied ID. Replacing a
single letter's audio while preserving its genuine ticket stops the whole
render before the first byte. The source of the expected IDs is a trust
boundary. Repeating an
identical letter counts its slot once. A short sink write ends the render and
reports the completed slots, accepted bytes, whole frames and remaining
bytes. A consumer can use the measurement to reconcile its own sink.
Replaying to an empty sink reproduces the stream. Playing a second time to
the same sink would write again. Withdrawal governs future delivery. A
recipient controls audio already in its custody.

`make check-held-back` exercises a withdrawal between offering and
receiving, a correction with a new invitation, replay, a conflicting
late edit, a gap and a physical one-byte write. Set `VOICE_SAMPLES` to a
directory containing `english.pcm`, `original.pcm` and `indonesian.pcm`
to run the same comparison with recordings at 16 kHz mono. Set
`VOICE_RENDERED` to a new output path to retain the assembled stream.

On the M4 Max, the installed Samantha and Damayanti voices produced the
English and Indonesian samples. The recipient delivered 53,946 bytes of
“We may listen before we speak.” followed by 59,194 bytes of “Kami boleh
berbagi cerita.” The withdrawn version started with “Kita”. Splitting the
assembled output at the accepted frame boundary reproduced both source
files byte for byte. Local Whisper small transcribed the two segments with
the intended words. This run establishes machine-recognized speech and
the receiving process's byte counts. Listener experience remains open.
