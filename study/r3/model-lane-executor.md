# The model lane as an executor of nodes: what the old body's executor does, what it costs, and what a node step may spend

Study of 2026-11-24. Read-only. Nothing was run on the old runtime; every number below about the old body is a number it recorded itself.

## 0. How to read this report

Path prefixes used in citations (each is an absolute path; the prefix is written once here):

- `OLD` = `coherence-kernel/.claude/worktrees/coherence-kernel-redesign-8fa6bc` (the old body at commit b8ff978c9 of 2026-11-24)
- `MEM` = `memory` (private working notes kept beside the old body)
- `SEED` = `coherence-seed` (the restart; node numbers such as 5.10.7 are nodes of `SEED/graph/5.kg` as printed by `./kg brief`)

Status words: **observed** (I read the file at that line today), **relayed** (read through the web fetch tool, which summarises pages and refused to quote abstracts whole), **inferred** (my arithmetic or reasoning, with what it stands on), **assumed** (not verified).

Limits of this study:

- The old runtime was not run. Every timing is a recorded reading, most from private notes dated 2026-10-01 to 2026-11-12. The notes themselves say one reading moves 2 to 4 times with machine state (`MEM/reference-decode-submit-per-layer-and-reading-variance.md:13,17`), so every millisecond below is a reading on one shared M4 Max, not a constant.
- No receipt holds a per-phase split of the final 63 ms decode token (attention against MLP against head). Section 4 says what is measured and what is only apportioned by weight bytes.
- Web sources were read through a summarising fetch on 2026-11-24. Where the tool gave a number I repeat it as relayed; I did not open any PDF.
- The model's context limit as declared in the GGUF was not read (the header was not parsed in this study). The lane takes `maxpos` as a caller's parameter.

## 1. One decode step, as the old body runs it

Model: Qwen3.8-27B, architecture string `qwen35`, from a GGUF file; the lane is `OLD/form/native/metal/qwen35-dense-token-handle.fk` (126 KB). All of the following is **observed** in that file unless marked.

| # | Stage | What runs | Where | Source |
|---|---|---|---|---|
| 0 | open a batch | `metal_batch_concurrent()` arms a concurrent compute encoder | host call into the carrier | `:683` |
| 1 | token id in, embedding row | one dispatch: dequantise row `id * 5120` of `token_embd.weight` into the residual buffer `bs[0]` (5,120 threads). The table stays a mapped file view; one row is read per token | GPU | `:145`, `OLD/form/form-stdlib/bml/qwen38-embedding.bml:6-9` |
| 2a | linear-attention block (48 of 64 layers; Gated DeltaNet) | 13 dispatches: RMSNorm; four Q8_0 matvecs (fused qkv 5120 to 10240, alpha, beta, gate 5120 to 6144); conv window update; two small norms; gates; the delta-rule state update; norm-and-gate; out-projection 6144 to 5120; residual add | GPU | `:235-248` |
| 2b | full-attention block (16 of 64 layers, every 4th) | 8 dispatches before the position (norm, q, k, v matvecs, q/gate split, per-head norms), 4 that carry the position (RoPE on q and k, K row and V row written at `pos * 1024`), 1 or 2 for attention over `pos + 1` rows (one threadgroup of 256 threads per query head, or the split-KV "flash-decoding" pair), 3 after (output gate, out-projection, residual add) | GPU | `:253-282`, `:87-96` |
| 2c | SwiGLU feed-forward, every layer | 6 dispatches: RMSNorm; gate and up matvecs 5120 to 17408; SwiGLU; down matvec 17408 to 5120; residual add | GPU | `:289-295` |
| 3 | hand-over | after every 2 layers the open batch is committed to the queue without waiting (`metal_submit`) and a new one armed, so the device runs early layers while the host encodes later ones | host | `:640-649` |
| 4 | final norm | RMSNorm of `bs[0]` with `output_norm.weight` into `bs[1]`: one threadgroup of 256 threads | GPU | `:657`, `:155-163` |
| 5 | output projection | one Q8_0 matvec `output.weight` (248,320 x 5,120) from `bs[1]` into the logits buffer `bs[28]` | GPU | `:658` |
| 6 | pick | greedy: a 1,024-partition argmax over the 248,320 logits, then a one-thread merge writing 4 bytes into `bs[31]` | GPU | `:659-660` |
| 7 | token id out | `metal_buf_read(bs[31], 0, 4)`: the read settles every submitted batch that named the buffer, then copies 4 bytes from the shared buffer | host, the one wait of the step | `:691`, `OLD/form/native/metal/fk-metal-carrier.m:1997-2015` |

Counts.

- Dispatches per token: 48 x (13 + 6) + 16 x (16 + 6) + 1 = **1,265** before the head, plus 4 for the head. **inferred** from the record lists above; it equals the count the old body printed ("1265 dispatches", `MEM/project-qwen-speed-vs-llamacpp-2026-10-04.md:19`; "A token is ~1265 dispatches", `qwen35-dense-token-handle.fk:165`).
- Submissions per token: one command buffer per 2 layers, so **32 command buffers and one wait** (`:644`; "32 command buffers a token", `MEM/project-spec-verify-matrix-unit-2026-10-07.md:21`). **observed** in source, **relayed** count from the note.
- Everything from step 1 to step 6 is on the GPU. The host builds bindings and enqueues; with a plan (`q38-plan`, `:723-749`) the position-independent records of a whole token are built once per state and a token only replays them.

How the kernels are written. **observed.**

- Each kernel is Metal Shading Language text held as a string inside a function of the old language and joined by string concatenation, for example `q38-rmspar-msl` (`:163`), `q38-gqa-tg-msl` (`:96`), `q38-delta-off-msl` (`:86`). The file's head says "MSL is assembled by Form and passed to metal_pipeline, so every kernel is JIT-compiled for the device" (`:7-8`).
- This is hand-written MSL carried in the language, parameterised by function name. It is not lowered from the language's own nodes. A lowering from nodes to MSL exists only for reduce, map and filter shapes (`MEM/project-gpu-lane-program.md:39-48,67-77`), and that note records that the tensor emitters "take kernel NAMES, never a body" (`:28-30`).
- A kernel is identified in the lane by its **index in a pipeline list** (0 to 75: `:136-144`, `OLD/form/native/metal/tests/qwen35-dense-token-handle-band.fk:44-51`). The file records one fault this caused: the decode block took "the last pipeline" for the norm-gate kernel, a merge appended four more, the gate never ran, and the head read back the token it was fed (`:124-127`).
- Pipeline compile: 54 ms once, re-admission of the same source 0 ms (cache keyed by function name and source), `MEM/project-gpu-lane-program.md:15-16`. **relayed from a note of 2026-09-15.**
- The carrier is one Objective-C file, `OLD/form/native/metal/fk-metal-carrier.m` (123 KB), loaded as a dynamic library built on demand (`OLD/form/form-stdlib/metal-carrier.bml:11-23`). Its doors: pipeline, buffer alloc, buffer from file (no-copy map), write, enqueue, submit (commit, no wait, returns a fence), fence wait, sync, read, free, status (`fk-metal-carrier.m:1365,1593,1612,1685,1772,1949,1997,2034,2072,2099,2126`).

## 2. The model's real dimensions

**observed** in `OLD/receipts/native-training-plan.md:66-70` and pinned by `OLD/form/native/metal/tests/qwen35-dense-token-handle-band.fk:29-43` (the test reads them from the GGUF header):

| Quantity | Value |
|---|---|
| hidden width | 5,120 |
| layers | 64; every 4th is full attention (16), the other 48 are Gated DeltaNet linear attention |
| feed-forward width | 17,408 |
| full attention | 24 query heads x 256, 4 key/value heads, RoPE over 64 dimensions |
| linear attention | 16 key heads x 128, 48 value heads x 128, conv kernel 4; fused in-projection 5,120 to 16,384; out-projection 6,144 to 5,120 |
| vocabulary | 248,320 rows; `output.weight` is untied from `token_embd.weight` |
| quantisation, file A | Q8_0: every large matrix GGUF type 8, every norm f32 (`qwen35-dense-token-handle.fk:149-150`); 27.0 GiB on disk (`native-training-plan.md:77-78`) |
| quantisation, file B | Q4_K_M: a mix of Q4_K and Q6_K; `attn_qkv`, the early `ffn_down`, `attn_v` and the head are Q6_K (`native-training-plan.md:240-241`); about 15.6 GiB (`:75`) |
| context | `maxpos` is a parameter of the open (`qwen35-dense-token-handle.fk:980`); the resident server's attention scratch defaults to 65,536 positions and its pool reaches about 130,000 (`OLD/docs/resident-server.md:113-115`). The GGUF's declared context length was not read. |

Weights touched by one decode token. **inferred** from the tensor shapes in `native-training-plan.md:80-83` and the records in section 1:

- one linear layer: 5120 x (10240 + 48 + 48 + 6144) + 5120 x 6144 + 3 x 17408 x 5120 = 383,221,760 weights
- one full layer: 5120 x (12288 + 1024 + 1024 + 6144) + 3 x 17408 x 5120 = 372,244,480 weights
- all layers: 48 x 383,221,760 + 16 x 372,244,480 = **24,350,556,160**
- output projection: 248,320 x 5,120 = **1,271,398,400**
- one token: **25,621,954,560** multiply-adds; at Q8_0 (34 bytes per 32 weights) **27.22 GB** of weight bytes, of which the head is 1.351 GB

## 3. Measured time and memory, with their receipts

All on one Apple M4 Max (40 GPU cores, 128 GB, stated bandwidth 546 GB/s). All **relayed from the old body's notes and receipts**; none re-measured today.

Decode, Q8_0 file, one stream:

| Reading | Value | Date | Source |
|---|---|---|---|
| first measured | about 2 tok/s; lane process at 2 to 7 % CPU | 2026-10-01 | `MEM/project-local-lane-speed-is-gpu-bound.md:13` |
| quiet, before repairs | 197 ms/token (5.08 tok/s, 147 GB/s, 1,356 dispatches) | 2026-10-04 | `MEM/project-qwen-speed-vs-llamacpp-2026-10-04.md:15` |
| split of that state | host encode 44 ms + device wait 405 ms per position (contended) | 2026-10-04 | same `:19` |
| after the parallel norm and the 2-layer hand-over | 86 to 90 ms/token (11.16 to 11.57 tok/s, 324 to 336 GB/s); llama.cpp on the same file in the same minute 11.18 +/- 0.94 tok/s | 2026-10-05 | same `:21` |
| clean window, same process | **63 ms/token = 15.6 tok/s** | 2026-10-07 | same `:31`; `MEM/project-spec-verify-matrix-unit-2026-10-07.md:17` |
| resident server, one row, 200 ids | 13,062 ms = 65.3 ms/token (15.3 tok/s); 13,266 ms (15.0) | 2026-11-12 | `OLD/receipts/serve-spec-pairs-2026-11-12.jsonl:2,4` |
| degraded windows | 103 to 107 ms/token; 130 ms/token "mid weather" | 2026-10-07 | `MEM/project-spec-verify-matrix-unit-2026-10-07.md:17`; `MEM/project-spec-lane-session-speculative-2026-10-07.md:17` |

At 63 ms the lane reads 27.22 GB / 0.063 s = 432 GB/s, 79 % of the stated bandwidth (**inferred**). The step is bound by reading weights, not by host code.

Sustained load. The GPU clocks down under continuous work: a fixed product read 27.4 to 29.1 ms at rest and 71 to 76 ms after 30 s (`MEM/reference-decode-submit-per-layer-and-reading-variance.md:13`); a fixed FMA loop fell from 12.7 to 4-5 TFLOP/s in two seconds (`MEM/project-qwen-speed-vs-llamacpp-2026-10-04.md:29`). Every figure of a few seconds is a burst figure.

Prefill: 64-position spans at 49 to 54 positions/s quiet on 2026-10-04 (same note `:15`); a 64-id chunk 0.34 s near position 0 and about 1.1 s near position 8,000 on the server (`OLD/docs/resident-server.md:158`); best voice-door reading 32 ms per prompt token (`OLD/receipts/native-training-plan.md:72`). Per layer at 64 tokens: feed-forward 11.7 ms, linear attention 10.7 ms, full attention 6.0 ms (`MEM/project-qwen-speed-vs-llamacpp-2026-10-04.md:23`).

Batched and speculative decode:

- Eight rows in one weight pass on the server: four rows 36.4 tok/s aggregate, six 37.5, eight 34.1 (`OLD/CURRENT_FLOOR.md:127`).
- An 8-row verify pass on the matrix unit costs 92 to 94 ms against 63 to 64 ms for one token: 1.46 times a token for eight columns (`MEM/project-spec-verify-matrix-unit-2026-10-07.md:17`). In exact arithmetic the same check cost 235 ms against 105 ms (`MEM/project-dflash2-drafter-2026-10-07.md:15`).
- With the DFlash2 block drafter: 68.5 tok/s on code (4.38 times plain, 6.4 tokens a step), 1.5 times on prose (same note `:17`). The server's own n-gram drafter: code 21.0 and 20.5 against 15.3 and 15.0 tok/s (`serve-spec-pairs-2026-11-12.jsonl:1-4`; `resident-server.md:98-103`).

Memory:

- Weights are mapped from the file without a copy and shared by every process that maps the file: 6.2 GiB of file-backed pages for the first of two kernels, 0.0 for the second (`OLD/docs/native-model-control-plane.md:181-185`).
- First forward after open costs seconds more than a steady one (5.9 s against 1.9 s for a 21-id prompt): the device's first use of the mapped buffers, not the page cache (`resident-server.md:25-34`).
- Mapped tensors start at any byte offset inside a page-aligned map; a 32-bit load through an unaligned device pointer returned other bytes and a real model decoded to noise while every synthetic test passed (`MEM/reference-mapped-tensors-start-at-any-byte-offset.md:11`).

## 4. Where the time goes

Measured pieces (all **relayed**):

- Before repair, 129 RMSNorms a token at 1.1 to 2.2 ms each on one GPU thread were most of a 197 ms token; the 256-thread form costs about 41 microseconds (`MEM/project-qwen-speed-vs-llamacpp-2026-10-04.md:21`; `qwen35-dense-token-handle.fk:155-161`). Today: 129 x 41 us = 5.3 ms a token (**inferred**).
- Full attention: 2,113 us of GPU per block with one thread per head, 553 us with a threadgroup per head (`qwen35-dense-token-handle.fk:87-91,262-265`). 16 blocks x 0.55 ms = 8.8 ms a token (**inferred**, at the context length of that probe, which the comment does not state).
- One feed-forward gate matvec (17,408 x 5,120, 94.7 MB at Q8_0): 187 to 312 us cold in a clean window (`MEM/project-spec-verify-matrix-unit-2026-10-07.md:15`), that is 303 to 506 GB/s (**inferred**).
- Host encode: 85 ms then 24 ms a token once dispatches became records built once (`MEM/project-qwen-speed-vs-llamacpp-2026-10-04.md:29`), overlapped with the device by the hand-over. A tiny dependent dispatch costs about 20 us (`:21`).
- Head: no direct Q8_0 reading. The Q6_K head shape (248,320 x 5,120, 1.04 GB) read 436.9 to 437.6 GB/s in isolation (`OLD/form/form-stdlib/bml/qwen38-decode-quant.bml:53-54`), about 2.4 ms.

Apportioned by weight bytes (**inferred**, valid only as far as the step is bandwidth-bound):

| Part | Multiply-adds | Share | At a 63 ms token |
|---|---|---|---|
| feed-forward (3 matvecs x 64 layers) | 17.11 G | 66.8 % | 42.1 ms |
| attention projections (all layers) | 7.24 G | 28.3 % | 17.8 ms |
| output projection | 1.27 G | 4.96 % | 3.1 ms |

The measured non-matvec pieces above (norms 5.3 ms, full attention 8.8 ms) do not fit inside this table at 63 ms, so either the matvecs run faster than the 432 GB/s average or those readings were taken in slower windows. **No receipt resolves it. A per-kind split of one decode token at its present speed does not exist in the old body.**

## 5. What persists between steps

**observed** in `qwen35-dense-token-handle.fk:958-973, 1027-1044`:

- **Full-attention layers (16):** one K buffer and one V buffer per layer, indexed by position; a row is 4 x 256 = 1,024 floats. Per position: 16 x 2 x 1,024 x 4 B = **131,072 B (128 KiB)** in f32. Rewinding is by position alone; later writes overwrite.
- **Linear layers (48):** recurrent state that knows no position: a conv window (10,240 channels x 4 taps) and a delta state `S` of 48 x 128 x 128 floats per layer. 163,840 + 3,145,728 B per layer, **158.9 MB** for the stack (**inferred** from `:959`). Returning to a mark means copying that state aside and back, device-side (`:1029-1037`).
- The delta state is an associative memory written by the delta rule: per token `S = alpha * S; err = v - S k; S += beta * err * k^T; out = S (qscale * q)` (`:86`).
- **Resident server:** K and V live in one pool of blocks (512 blocks of 256 positions by default), a table row per sequence maps position to block; paged and contiguous outputs are byte-equal (`resident-server.md:37-46`). 131,072 positions x 128 KiB = 16 GiB at f32 (**inferred**).
- **Prefix cache:** a snapshot of the full layers' K/V to a position plus the linear layers' state, taken at a question's end and at 1,024, 2,048, 4,096 ... ids; a request whose ids begin with a snapshot's ids (compared by packed bytes) starts from a copy. A 2,614-id prompt again: prefill 19.5 s to 0.18 s, same text (`resident-server.md:121-128`).
- Scratch: 34 working buffers reused by every position (`:776-782`); the logits buffer is 248,320 x 4 = 993 KB per row.

## 6. Every existing place where something other than the model touches hidden states, logits, embeddings or the cache

All **observed** in source unless marked.

| Hook | What it touches | Where |
|---|---|---|
| rank-one head adapter | `h' = h + B * (alpha * (A . h))` after the final norm, before the output projection; 2 dispatches; `output.weight` is never writable | `qwen35-dense-token-handle.fk:50-59, 665-676, 195-217` |
| "wear": rank-r LoRA records on a layer | per record `u = A^T x` then `y += scale * B^T u` on the residual, at slot 4 (`ffn_down`), 10 (`attn_output`) or 13 (`ssm_out`); 2 dispatches per record | `:181-192, 248, 282, 295` |
| tap | a function `tap(phase, l, n, pos0)` the prefill walk calls inside its own batch; used to capture the residual after 5 layers for the drafter and at layer 62 for the trainer | `:579-595`; `MEM/project-dflash2-drafter-2026-10-07.md:13`; `OLD/docs/qwen-last-block-lora-design.md:22` |
| attention scope | a buffer of position ranges the attention kernel reads through, set from control text in the generated bytes; no KV row is evicted or renumbered | `OLD/form/form-stdlib/declarative-attention-metal.bml:16-42`, `declarative-attention.bml:1-3`; `qwen35-dense-token-handle.fk:278, 695-699` |
| sampling, session | device selects exact top-20 over 256 partitions after a presence penalty read from a `seen` buffer; the host applies temperature 0.7, nucleus 0.8 and the draw | `OLD/form/form-stdlib/bml/form-cli-model-sampling.bml:9-36` |
| sampling, server | Gumbel-max among tokens within min-p 0.05, on the device, per row | `OLD/docs/resident-server.md:132-136`; `OLD/form/form-stdlib/bml/q38-multi.bml:173-177` |
| speculative columns | a row carries draft ids as extra columns of one pass; a rejected tail restores linear state from a snapshot | `resident-server.md:79-88` |
| drafters | the model's own MTP head (GGUF `blk.64`), a 5-layer block-diffusion drafter conditioned on tapped residuals and sharing the target's embedding and head, and an n-gram of the row's own text | `MEM/project-qwen-speed-vs-llamacpp-2026-10-04.md:29-31`; `MEM/project-dflash2-drafter-2026-10-07.md:13`; `resident-server.md:90` |
| observation injection | Form's answer to a control is tokenised and prefilled into the same KV stream; the pending id enters once | `OLD/form/form-stdlib/form-cli-model-session.fk:797-836`; `OLD/docs/local-reasoning-homecoming-lanes.md:69-72` |
| reserved rows | PAD rows 248,077 to 248,086 of the vocabulary are named as one id per Form control; the head adapter is fitted toward the opening id | `OLD/docs/form-token-expansion.md:7, 25-29`; `OLD/form/form-stdlib/bml/form-token-kinds.bml:9-10` |

What does **not** exist: a logit mask or grammar constraint on the device ("The server decodes greedily with no grammar hook", `resident-server.md:18`; listed as future work at `:159`); any path that feeds a vector that is not a vocabulary row into `bs[0]`; any extra row scored beside the vocabulary; any read of a hidden vector by the host during decode.

## 7. How the model reaches the body today

**observed** in `OLD/form/form-stdlib/bml/form-token-grammar.bml:1-19`, `form-token-kinds.bml:15`, `form-token-tool.bml:1-35`, `OLD/docs/local-reasoning-homecoming-lanes.md:22-56`.

- The model writes text: `<|form:VERB|>PAYLOAD<|/form:VERB|>`. A byte cursor over the decoded text recognises it. Nine verbs: recipe-birth, recipe-exec, choice, eval, knowledge-query, node-get, node-make, node-select, recipe-import; plus `tool` for the coding lane.
- Form runs the verb and answers in the same stream with `<|form:observation|>RESULT<|/form:observation|>`, where RESULT is `value=...;`, `recipe=@p.l.t.i;`, `node=@p.l.t.i;` or `nothing=<reason>;`. The observation is tokenised and prefilled.
- Node verbs already work on immutable content: `node-make` returns `node=@address;`, `node-get` returns kind, value and children, `node-select` returns the first candidate a predicate holds for (`local-reasoning-homecoming-lanes.md:22-40`). A tool call may name an earlier result by address so it "is named by its node and never sent again" (`form-token-tool.bml:22-23`).
- Cost in tokens: on the 27B, `<|form:` alone is seven tokens and a verb more; `<|form:tool|>` is fifteen tokens a call; the model's own `<tool_call>` markers are single ids (248,058, 248,059, 248,066, 248,067) that the lane spells as text instead (`OLD/docs/form-token-expansion.md:25-33`). At 63 to 90 ms a token the envelope of one call is about 1 to 1.4 s before any payload (**inferred**).
- Measured on the 3B voice trained by LoRA to write the controls: 55 of 64 held-out answers exact against 25 plain and 5 taught by prompt (`OLD/docs/native-model-control-plane.md:140-144`); after a corpus rebalance 39 of 72, with taught mass moving into neighbouring verbs and 96 ids per correct answer against 47 (`OLD/receipts/2026-10-09-form-token-endtoend.md:18-29`).
- One coding option on the 27B: 24 turns, about 80 minutes, 5,343 generated and 13,513 injected ids (`MEM/project-local-lane-speed-is-gpu-bound.md:16`). Injected text is 2.5 times the generated text.
- A learned control layer was the stated direction: "Dedicated learned Form IDs require compatible embedding and output rows as well as their native bindings. An adapter alone does not add those rows" (`local-reasoning-homecoming-lanes.md:197-200`).

## 8. The budget of a next-token step, in numbers

All **inferred** from sections 2 and 3. Reference step: 63 ms, 25.62 G multiply-adds, 27.22 GB of Q8_0 weight bytes.

**One output projection against one full step.** 1.271 G of 25.62 G multiply-adds = **4.96 %**. By bandwidth that is 3.1 ms of a 63 ms step; from isolated matvec rates (300 to 500 GB/s) 2.7 to 4.5 ms. The head is one twentieth of a step.

**Extra rows that can be scored.** A row is 5,120 multiply-adds.

| Budget | Multiply-adds | Rows at width 5,120 | Bytes read if Q8_0 | Rows if stored f16 | Rows if stored f32 |
|---|---|---|---|---|---|
| 1 % of a step (0.63 ms) | 256 M | **50,000** | 272 MB | 26,600 | 13,300 |
| 10 % of a step (6.3 ms) | 2.56 G | **500,000** | 2.72 GB | 266,000 | 133,000 |

The f16 and f32 columns assume the bound is bytes read at 432 GB/s, so a wider storage format buys fewer rows. Adding 10,000 rows to the head costs 0.2 % of a step.

With a learned projection of the hidden state to 256 dimensions (one 256 x 5,120 matvec, 0.005 % of a step), 1 % of a step scores **one million** rows of width 256.

**Readback of one hidden vector.** 5,120 x 4 = 20,480 bytes. The step already ends with one settle-and-read of 4 bytes (section 1, step 7). A buffer read is a `memcpy` from the shared buffer's contents after the settle (`fk-metal-carrier.m:1997-2015`); the old body measured 6 us for a 4 KB read and 108 us for an enqueue-plus-sync round trip (`MEM/project-gpu-lane-program.md:17`). So reading the hidden vector at the point where the token id is already read adds on the order of 10 to 30 us, under 0.05 % of a step; reading it at any other point adds a wait, and "the membrane's price is the wait, not the crossing" (same line).

**Writing one row.** 20,480 bytes f32; a 400 KB buffer write took under 1 ms (`MEM/project-gpu-lane-program.md:18`), so a row write is tens of microseconds (**inferred**). No pipeline is compiled: rows are data.

**Feeding one vector in.** One forward without the head is 95 % of a step (60 ms). Today's alternative, an observation as text, costs one such forward per token of the envelope and the result.

**What small extra dispatches cost.** About 20 us each. A rank-r adapter on all seven projections of all 64 layers, applied as the present wear does (2 dispatches a record), is 896 dispatches, about 18 ms, 28 % of a step, although its weights (54.5 M at rank 8, `native-training-plan.md:117`) are 0.8 % of a step's bytes. An adapter must be folded into the kernel that already runs or be confined to a few sites. Two dispatches at one layer are 0.06 % of a step.

**A second variant in the same pass.** Eight columns cost 1.46 times one: each extra row is about **6.6 %** of a step on the matrix-unit path ((1.46 - 1) / 7). In exact arithmetic it was about 18 % per row (`MEM/project-dflash2-drafter-2026-10-07.md:15`).

## 9. What work outside this body shows

All **relayed**: read through a summarising fetch on 2026-11-24. Dates are the source's own.

| Work | What it shows | Numbers the fetch returned |
|---|---|---|
| ToolkenGPT, arXiv 2305.11554 (v1 2023-05-19, v4 2024-01-15) | Each tool is a token with a learned embedding appended to a frozen model's head; the model calls a tool by emitting it, then completes arguments | none in the abstract |
| ToolGen, arXiv 2410.03439 (v1 2024-10-04, v3 2025-03-29) | Each of a large tool set is one token; retrieval and calling become next-token generation, with the model trained | over 47,000 tools |
| Chain-of-Tools, arXiv 2503.16779 (2025-03-21) | Frozen model; tool chosen from hidden states against vectors computed from tool descriptions, so unseen tools can be added | no numbers in the abstract |
| "Reading the Plan", NeurIPS 2026 workshop poster (neurips.cc/virtual/2026/168983) | A probe on one frozen residual state at about three-quarters depth predicts the agent's next tool call | probe 1.01 M parameters, 0.09 ms per decision; beats the model's own verbalised prediction by 12 to 30 points; 110-tool benchmark |
| NTILC, arXiv 2606.06566 (2026-06-04) | Intent and tool specifications in one embedding space; only the chosen tool's schema enters the context | context use down "over 95 %", latency down up to 74 % against in-context tool lists |
| CoLT, arXiv 2602.04246 (2026-02-04) | The model emits seed tokens whose hidden states an external decoder expands; latent work treated as a tool call | four datasets; no speed figures in the abstract |
| xRAG, arXiv 2405.13792 (v1 2024-05-22, v2 2024-12-09) | A retrieved document enters as one embedding through a trained bridge; retriever and model frozen | FLOPs down 3.53 times; over 10 % average gain on six tasks |
| KBLaM, arXiv 2410.10450 (v1 2024-10-14, v2 2025-02-09) | Knowledge triples become key-value vector pairs from a sentence encoder with linear adapters, attended through a rectangular attention; cost linear in the size of the base | more than 10 K triples into an 8 B model with an 8 K window on one 80 GB A100 |
| Cartridges, arXiv 2506.06266 (2025-06-06 to 06-13) | A KV cache trained offline per corpus by "self-study" replaces the corpus in context; composable at inference | 38.6 times less memory, 26.4 times higher throughput, effective context 128 k to 484 k tokens |
| kNN-LM, arXiv 1911.00172 (2019-11-01, ICLR 2020) | The next-token distribution is interpolated with nearest neighbours over stored hidden states; no training | perplexity 15.79 on Wikitext-103, 2.9 better |
| Memory Layers at Scale, arXiv 2412.09764 (2024-12-12/20) | A trainable key-value lookup adds parameters without adding FLOPs | up to 128 B memory parameters; beats dense models with more than twice the compute |
| Limited Memory Language Models, arXiv 2505.15962 (v1 2025-05-21, v3 2025-10-02) | Facts are kept in an external database during pre-training; the model learns to look them up | none in the abstract |
| Self-RAG, arXiv 2310.11511 (2023-10-17) | Special tokens the model emits decide when to retrieve and grade what came back | 7 B and 13 B models |
| ReFT, arXiv 2404.03592 (v1 2024-04-04, v3 2024-05-22) | Learned interventions on hidden representations of a frozen model | 15 to 65 times more parameter-efficient than LoRA |
| Text-to-LoRA, arXiv 2506.06105 (2025-06-06, ICML 2025) | A hypernetwork writes a LoRA from a task description in one forward pass | trained on 9 adapters; compresses hundreds |
| Doc-to-LoRA, arXiv 2602.15902 (2026-02-13, ICML 2026); SHINE, arXiv 2602.06358 | A hypernetwork writes a LoRA from a document in one pass | near-perfect recall at more than 4 times the model's context window |
| "The Override Gap", arXiv 2604.23750 (v1 2026-04-26, v3 2026-11-22, NeurIPS 2026) | Those generated adapters fail when the document contradicts what the model already believes | accuracy falls from 65.3 % to 16.3 % from the weakest to the strongest prior; scaling the adapter at its most active layers lifts deep-conflict accuracy 46.4 % to 71.0 % |
| S-LoRA, arXiv 2311.03285 (v1 2023-11-06, v3 2024-06-05) | Thousands of adapters over one base; base computation batched, per-adapter term in custom kernels; adapters and KV in one paged pool | up to 4 times throughput |
| Zero-Shot Tokenizer Transfer, arXiv 2405.07883 (v1 2024-05-13, v2 2025-10-28) | A hypernetwork predicts embedding rows for tokens the model never saw | gap closed with under 1 B tokens of further training |
| Coconut, arXiv 2412.06769 (v1 2024-12-09, v4 2026-08-23) | The last hidden state is fed back as the next input embedding; needs training | none in the abstract |
| Semantic IDs (TIGER), arXiv 2305.05065 (2023-05-08 to 11-03) | An item is named by a short tuple of codewords the model generates one by one; helps items never seen | none in the abstract |
| XGrammar, arXiv 2411.15100 (v1 2024-11-22, v3 2025-05-12) | Grammar masks split into tokens checkable ahead of time and tokens checked at run time, computed while the GPU runs | up to 100 times faster than earlier engines |
| DFlash, arXiv 2602.06036 (v1 2026-02-05, v2 2026-05-28, ICML 2026) | A block-diffusion drafter conditioned on the target's hidden features drafts a block in one pass | over 6 times lossless speedup; up to 2.5 times EAGLE-3 |
| Splash README (github.com/incoai/splash, undated page) | A Metal engine for the same model family | Qwen3.8-27B: 74 tok/s on an M5 Pro against 16 for llama.cpp on the same Q4_K_M weights; prefill 363 tok/s at 32 K; 32 K cached replay 282 ms; 99.30 to 99.45 % next-token agreement with llama.cpp; KV stored as INT8 by default |
| arXiv 2610.31479 (2026-10-22) | Server-side inference optimisations re-measured on four Apple machines | every speculative-decoding method tested lowered throughput on Apple under the common benchmark; KV pruning up to 1.75 times; ported CUDA kernel ideas up to 1.20 times |
| Apple, `MTL4MachineLearningCommandEncoder` (developer.apple.com, undated; OS 26.0) | Metal 4 can dispatch a whole network inside a command buffer; a tensor resource and ML operations inside shaders exist | no performance figures |

Reading across them (**inferred**): the pieces the request asks for each exist separately and each has been shown on a frozen model. Tools as rows of the head; documents as single input vectors; knowledge as key-value pairs or as a trained cache; adapters written in one pass by a hypernetwork; the next tool call readable from the residual stream by a probe of one million parameters. None of the sources read puts them on one substrate with one identity for the thing picked, read and created, and none reports the cost as a fraction of a decode step.

## 10. What follows for the design (all inferred; none built)

**10.1 A step answers with a node.** The last three dispatches of a step are "score rows against the hidden state, pick one". A vocabulary id is one kind of row. A second table of rows beside `output.weight`, scored by the same matvec and folded by the same argmax, makes the step's answer either a token or a handle into the node store. At 50,000 rows per 1 % of a step (section 8) the table can be large before it is noticed. This is ToolkenGPT's head extension with the row store being the node space. It fits `SEED` 5.4.1 (every evaluation is acknowledged by nothing, 0, 1 or a node) and 5.2.1: the row index is the 32-bit in-memory handle and never the content id.

**10.2 Rows are a materialisation, like compiled code.** A node's output row and input row for one model are derived from the node for one target. `SEED` 5.6.4 already says this of machine code: kept as water under the key (recipe id, generator id, target id), losing it loses nothing. The same key shape holds: (node content id, encoder id, model id). A row that was trained, not derived, cannot be remade and is ice with its lineage, as an adapter is.

**10.3 The table offered to a run is its interface.** A model run is a cell. The rows it can pick are exactly the views it holds (`SEED` 5.8.4, references are capabilities). A node not in the table cannot be picked; this is interface-only crossing without a mask.

**10.4 Three ways to read a node inline, by rising cost and rising evidence.**
1. As one input vector written into the residual buffer where the embedding row goes (`qwen35-dense-token-handle.fk:145` is a row copy into `bs[0]`): one forward without the head, 95 % of a step, in place of a text observation of dozens of tokens. xRAG shows one vector per document works when a bridge is trained.
2. As a frozen state snapshot. The prefix cache is already this: full-layer K/V blocks plus linear-layer state, found by the bytes of the ids, 19.5 s to 0.18 s. Made content-addressed and shared read-only between sequences through the block table, a snapshot is a node of knowledge that costs no decode step to read. It composes only as a prefix chain.
3. As rows of knowledge scored against the hidden state at one or a few layers (kNN-LM, KBLaM, the probe paper): 1 % of a step for 50,000 full-width rows.

**10.5 This architecture has only 16 key-value layers.** 48 of 64 layers keep a delta-rule associative state instead of a cache. KV-injection methods (KBLaM, Cartridges) reach a quarter of the stack. The linear layers' state is a fast-weight matrix written by `S += beta (v - S k) k^T`; a snapshot of it is the only tested way to carry knowledge through those layers. Whether state deltas from two snapshots can be added is not known.

**10.6 Creating a node inline.** Minting the node is a store operation (the seed measured 14 to 25 ns to intern, `SEED` 6.1.25). Making it reachable by the model is two row writes of 20 KB, tens of microseconds, no compile. The open part is where the row comes from at no training cost: the normed hidden state at the step that creates it (a device-side copy, kNN-LM's key), or a small encoder or hypernetwork over the node's content (Zero-Shot Tokenizer Transfer for rows, Doc-to-LoRA for adapters). A hidden-state row is not on the same scale as trained vocabulary rows, so node scores and token logits need a learned gate or a separate normalisation.

**10.7 Attaching capability without retraining, four grains.**
- rows (10.1): leave every base row untouched, so the base model's behaviour on ordinary text is unchanged by construction;
- a residual intervention at one tap layer (ReFT): two small dispatches, 0.06 % of a step;
- low-rank deltas on projections (the existing wear): affordable only when folded into the matvec's own kernel;
- a snapshot (10.4.2).
The old body has the second half of each already: the tap, the wear, the rank-one head, the prefix cache.

**10.8 Comparing variants.** A second variant is a second row of the same weight pass with its own row table, its own intervention and its own block-table row sharing the prefix blocks: about 6.6 % of a step per variant on the bounded-numerics path. Speculation in the old server is already a choice with a trail: draft columns run, the model's own answer says how many stand, a rejected tail restores state from a snapshot and its K/V rows are overwritten (`resident-server.md:79-88`). That is `SEED` 5.4.9's choice-point frame, and "dream, envision, play" is a row whose effects are buffered until commit (`SEED` 5.4.8).

**10.9 Kernels.** `SEED` 5.10.7 has GPU programs generated in the grammar and reached through a door; the old lane's kernels are hand-written text addressed by list position. A kernel's identity should be its content id, and its binding a record, which the old body reached late (`metal-jit.bml` keys pipelines by SHA-256; `q38-plan` builds records once).

## 11. Carry forward

1. The step's shape: one residual buffer, records built once per state, a hand-over every few layers, one wait at the read. Measured: host encode 85 to 24 ms, token 197 to 63 ms.
2. Weights as mapped, shared, read-only file views; byte loads only.
3. Paged KV with a block table per sequence, and state snapshots as the unit of reuse.
4. Every hook that already exists as a named interface: tap, wear, head adapter, attention scope, per-row sampling.
5. Rows as data: nothing in adding a row compiles a pipeline.
6. The budget as a law: a node operation states its cost as a share of a step, in bytes read and in dispatches.
7. Losslessness defined against a verify path with stated bounds, not bit equality with one kernel.
8. Paired back-to-back readings in one process as the only timing that counts.

## 12. Let go

1. Tool calls spelled as text: 7 to 15 tokens of envelope per call, 2.5 times as much injected text as generated text.
2. Kernels identified by their index in a list; a 128-slot pipeline table.
3. Adapters as two extra dispatches per projection across the whole stack.
4. Bit-exact agreement as the gate for a faster kernel.
5. A single reading as a measurement; a whole-GPU busy gauge as a phase timer.
6. Teaching new verbs by LoRA over the existing vocabulary alone: taught mass moved into neighbouring verbs.
7. The assumption that KV-cache methods cover this model: three quarters of its layers have no cache.

## 13. Open questions

1. What does one decode token cost per kind (feed-forward, linear attention, full attention, head, norms) at its present 63 ms? The old body never recorded it.
2. Can a row copied from a hidden state compete with trained vocabulary rows, or does a node table need its own gate and normalisation? Nobody read measured it on this model.
3. Is the linear layers' delta state additive across snapshots, and can a pair be written into it directly?
4. Does one injected vector carry a node's content into a frozen Qwen3.8 without a trained bridge? xRAG trained one.
5. How large can the scored table be before the model's picks degrade, as distinct from before the time is noticed? ToolGen reports 47,000 with training.
6. Which layer's residual is the right one to score nodes against? The probe paper found about three-quarters depth; the drafter taps 5 layers.
7. Is speculative verification worth carrying at all on this hardware? The old body measured 1.3 to 4.4 times on code and nothing on prose; a 2026-10 paper found every tested method slower on Apple.
8. What is a node's embedding when the node is a recipe: an encoding of its text under a lens, of its shape (`SEED` 5.2.5), or of its observed behaviour?
9. Does a generated adapter survive conflict with what the model already holds? The 2026 override-gap result says not by default.
10. What does the GGUF declare as context length, and what does the paged pool cost when K/V are stored as 8-bit, as Splash does by default?

## 14. Most surprising, and what stayed uncomfortable

Surprising: the output projection, the thing a "next token" is, is 5 % of a step. Scoring fifty thousand more rows is 1 %. The old body has the model spell `<|form:tool|>` in fifteen tokens. It named ten spare vocabulary rows for single-id controls, and its page of 2026-10-10 records the ledger of such crossings as empty on every checkout (`OLD/docs/form-token-expansion.md:25-29`). The cheap door was the head, and its own page says what is missing: embedding and output rows with native bindings (`local-reasoning-homecoming-lanes.md:197-200`).

Uncomfortable:

- The 63 ms token has no per-phase receipt, so "about the budget of a next-token step" is exact for the head and only apportioned for everything else.
- Every timing is one machine under other load; the notes record 2.7 times between two readings an hour apart.
- The strongest outside results (xRAG, KBLaM, ToolkenGPT, Doc-to-LoRA) all train something, and none was read beyond a summarised abstract. Whether any of it holds on a 64-layer hybrid with 48 recurrent layers is untested.
- The old body's speculative gains and the 2026-10 Apple study disagree, and I could not read the study's method.
