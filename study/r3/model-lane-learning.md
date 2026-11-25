# The model lane as built, and what a direct node interface would change

Study of 2026-11-24. Scope: how the old body (coherence-kernel) attaches new capability to its
model today, what that cost and where it failed, and which of those costs and failures a direct
node interface (node embeddings in, node choices out) would remove.

Conventions. `K/` stands for
`coherence-kernel/`,
`M/` for `memory/`,
`S/` for `coherence-seed/`. A citation is `path:line`.
Status words: **observed** (I read the file or ran the command on 2026-11-24), **relayed** (read
through a web fetch tool that summarises pages; fetched 2026-11-24), **inferred** (my reasoning,
with what it rests on), **assumed** (not verified). The old runtime was not run; every number
about the old body is a number its documents and ledgers hold, read today. A memory note is a
dated record, so "observed" on a note means "the note says this", not that the code still does.

---

## 0. The short answer

1. The body has three working ways to change what its model does without touching base weights,
   and all three are small: a role prompt (text), a rank-8 LoRA pair on one projection of the last
   block of the 27B (721,096 bytes, 0.3% decode cost), and a rank-one head adapter (41,152 bytes)
   that makes the frozen 27B emit a reserved vocabulary row as "a Form control opens here".
2. None of the 27B adapters is worn in serving. Three trained children were judged and all three
   decisions read `wear 0`; a placebo pair with its learning permuted out scored inside the same
   band as two of them.
3. Knowledge reaches the model only as text. Retrieval is lexical (word matches over file names
   and head comments, a hand-authored sparse dual-hash feature map). No node has an embedding the
   model consumes; no vector index of nodes exists in the model's path.
4. The model's output becomes nodes only by writing text that a cursor parses: nine control verbs
   plus a tool verb, each wrapped in a 15-token envelope on the 27B, each answer wrapped in
   another 15. A node is addressed by spelling `@p.l.t.i` in digits.
5. The text route's recorded failures are mostly reference failures and framing failures: names
   that do not exist, a name written where an address was meant, a forged observation, the wrong
   one of two neighbouring verbs, a dialect the reader does not read. In the turn ledger, 106
   model-driven options produced 8 green results while Form injected 14.4 ids for every id the
   model generated.
6. A direct node interface would remove the framing failures and the unresolvable-reference
   failures by construction and cut the per-reference cost from tens of positions to one. It
   would not remove wrong choices among real nodes, weak plans, dishonest witnesses, long-context
   prefill, GPU contention or the thin generalisation of small adapters.
7. The asymmetry that matters for building it: on this machine the output side (new rows the head
   can choose) is trainable today with kernels that exist, in closed form or at about 0.7 s a
   step. The input side (learned rows the model reads) needs a backward pass through all 64
   layers of the 27B, 48 of which are recurrent layers whose backward is not written.

---

## 1. The LoRA lane as built

### 1.1 Two models, two lanes

| | Llama-3.2-3B (4-bit, MLX snapshot) | Qwen3.8-27B (Q8_0 GGUF, 27.05 GiB mapped) |
|---|---|---|
| role | the session school and the Form-token voice | the voice that answers, plans and codes |
| trainer | full LoRA gradients through the whole model, native on Metal | last block only; rank-one head beside it |
| adapters in tree | 4 directories, 13.9 MB each | none worn; candidates under ignored `-next*` names |
| serving | adapter selected per lane | bare |

Sources: `K/docs/native-session-learning.md:334-339` ("This integration trains the native
**Llama 3B** adapter. It does not train the Qwen base") **observed**;
`K/form/form-stdlib/adapters/` listing (four `llama-3.2-3b-*` directories of 13,905,626 or
13,905,808 bytes and `qwen-teach-overlay.lora`, 529 bytes) **observed**;
`K/docs/qwen-last-block-lora-design.md:16` (27.05 GiB mmap) **observed**.

### 1.2 The 3B lane

- **Shape.** Rank 8, scale 20, 8 layers, Adam, learning rate 1e-4, batch 4; the first voice
  adapter was 600 iterations from `mlx-lm`
  (`K/form/form-stdlib/adapters/llama-3.2-3b-voice/adapter_config.json:10-22`) **observed**; the
  Form-token adapter is 100 native steps from parent `llama-3.2-3b-voice-native`, owner
  "Form-native-Metal" (`K/form/form-stdlib/adapters/llama-3.2-3b-form-tokens/adapter_config.json:1`)
  **observed**.
- **Trained on this machine, natively.** Full backward kernels (`nlb_ce`, `nlb_rms`, `nlb_swiglu`,
  `nlb_dp`, `nlb_softmax`, `nlb_dq`, `nlb_dkv`, `nlb_adam`) admit only `model_type == "llama"`,
  4-bit, tied embeddings (`K/docs/qwen-last-block-lora-design.md:15`) **observed**.
- **Time.** A 100-step, batch-4 run: 3,871 s on 2026-10-02, then 765 s, 306 s and 606 s after the
  kernel work of 10-11 (`K/receipts/form-token-lora.rows` rows with `lane "llama"`: `wall_ms
  3871119`, `765302`, `306310`, `606221`) **observed**. The train stage alone went from about
  4,700 s to 562.6 s and then 315 s on a quiet machine; per input token 22/40 ms forward/backward
  became 3.5/2.0 ms and then 1.7/1.0 ms (`K/receipts/2026-10-11-form-token-time.md:8-16, 49-54`)
  **observed**. Before that work a 1.2k-token row's forward took 36 s and one round of 1.8k
  tokens about 6 minutes, with a 23 GB Metal reservation
  (`M/reference-native-lora-teaching-lane.md:25-27`) **observed**.
- **Effect, with receipts.** Held-out loss 2.935 to 0.042 nats and 2.700 to 0.029
  (`K/receipts/form-token-lora.rows`, `valid_loss_before_milli 2935 valid_loss_after_milli 42`;
  later rows `2700`/`29`) **observed**. Decoded: without controls the adapter answered 25 of 64
  questions exactly; writing controls in stream it answered 53, then 60, then 66 and 67 of 81
  (`K/receipts/form-token-eval.rows` rows 4-13) **observed**.
- **Corpus.** `K/learn/form-token-traces.rows`, 728 lines (wc) **observed**; a second corpus of
  repository knowledge, 2,191 examples from 1,278 source units, sealed by sha256
  (`M/reference-native-lora-teaching-lane.md:19-20`) **observed**.

### 1.3 The 27B lane: last block, one projection

Design read on 2026-10-28; five rungs observed 2026-10-28 to 2026-11-01.

- **Which layers.** Block 63, the last full-attention block (`kth-full?(l) = (l+1)%4 == 0`, so
  16 of 64 layers are attention and 48 are Gated DeltaNet), projection `ffn_down` only, rank 8,
  alpha 16, scale 2: `y = W act + scale (act A) B`, A `[17408, 8]`, B `[8, 5120]`
  (`K/docs/qwen-last-block-lora-design.md:16, 127-133, 259, 273-277`) **observed**.
  `attn_output` was not added because its backward crosses rms, swiglu and two frozen Q8
  transposes (`:220-223`) **observed**. Earlier layers were not attempted because the Gated
  DeltaNet backward (conv window, delta recurrence, L2-norm, gates) needs four new kernels plus
  recurrent-state checkpointing (`:74`) **observed**.
- **Size.** 180,224 floats; `adapters.safetensors` is 721,096 bytes (`:273-275`) **observed**.
- **How trained here.** One serving prefill per row through all 64 layers with a tap at layer 62
  captures the block's input (`[159 x 5120]` f32, 3.26 MB a row); the trainer replays block 63
  from the capture on its own scratch, runs the frozen head (`output_norm`, `output.weight`
  248,320 x 5120, cross-entropy), and backs the gradient through the head and the norm to the
  pair. One new kernel was needed, the transposed Q8_0 matmul `form_q8_tmm_f32` (`:224-231,
  237-241`) **observed**. The block replay is bit-identical to serving: 0 of 117,760 words differ
  (`:103-107`) **observed**. Analytic gradients match central finite differences to 5.6e-6 (B)
  and 5.2e-5 (A) on the mean-square rung and 7.2e-4 through the head (`:160, 253-254`)
  **observed**.
- **How long.** Capture: 6 to 13.6 s per 159-position row on a shared GPU, 5 to 10 minutes per
  row under heavy contention (`:239-241, 356`) **observed**. Steps: 1.9 to 5.2 s each at 159
  positions with the head on device (`:267-268`); 0.7 s each once captures are cached (400 steps
  in 281,556 ms, `:450`) **observed**. Row preparation through the tokenizer index was the
  largest cost of the judging door: 504,322 ms for 40 rows, 70% of an 11 m 58 s wall (`:354-355`)
  **observed**.
- **Memory.** 418,404,912 bytes (399 MiB) at T 160, of which logits and their gradient are
  303 MiB; design budget 480 MiB; rank changes nothing visible (`:280-283, 49-56`) **observed**.
  The lease was granted at 27.8 GiB beside the shared 27 GiB file (`:360`) **observed**.
- **How applied at decode.** Hook `q38-wear-recs`: on an adapted layer, right after the
  projection's residual add, `u = A^T x` then `y += scale B^T u`, two Metal kernels per record,
  for decode, plan and span prefill alike; three residual writers carry it (`ffn_down`,
  `attn_output`, `ssm_out`). The base KV rows and recurrent state are computed before the add, so
  state mark/restore and KV rewind stay valid (`K/docs/qwen-last-block-lora-design.md:381-391`;
  `K/form/form-stdlib/qwen-lora-head.fk:41-64, 68-69`;
  `K/form/native/metal/qwen35-dense-token-handle.fk:192, 248, 282, 295`) **observed**. A wear
  costs 723,144 bytes and 3 device handles (`K/docs/qwen-last-block-lora-design.md:403`)
  **observed**. Scale 0 is the bare lane bit for bit (`:393-394`) **observed**.
- **Decode cost of wearing.** Bare best 105.19 ms an id, worn best 105.5 ms, ratio 1.0030, on a
  GPU at 97-98% utilisation; a second reader got 1.018 and 0.997 (`:401-402, 501`) **observed**.
- **How many adapters at once.** The record table is a list per layer, so several records can
  sit on one layer and on several layers; the band exercised three pairs at once (the child on
  `ffn_down`, rank-2 pairs on `attn_output` of block 63 and `ssm_out` of block 62) (`:393-394`)
  **observed**. A worn session is one adapter set for the whole context. The speculative span
  blocks and the drafter carry no hook (`:388, 525`) **observed**, and the multi-stream lane
  `K/form/form-stdlib/bml/q38-multi.bml` has no mention of wear, `qlh-rr` or LoRA (grep, 0 hits)
  **observed**. So two adapters cannot today be compared as two rows of one batched pass.
- **Measured effect.**
  - In-sample (8 rows, 20 steps): mean token loss 1.7920 to 1.7322 nats, every row lower
    (`:259-263`) **observed**.
  - Held-out accepted rows (5 rows, 560 supervised tokens): 2.7795 to 2.7697, a fall of 0.0098
    nats (0.35%), about a sixth of the in-sample fall; rejected rows the gradient never saw fell
    further, 0.0140 nats (`:344-353`) **observed**.
  - Decoded A/B on 132 predicates the gradient never saw: with the role prompt, bare 116, the
    three children 114, 115, 112; without the role prompt, bare 61, children 63, 65, 74
    (`:463-476`) **observed**.
  - Decisions: hold, all three (`:480-485`; `K/receipts/voice-wear-decisions.jsonl`, 3 rows, all
    `"wear":0`) **observed**.
  - Control: the same bytes with A's rows and B's columns permuted (same norms, no learned
    direction) at a matched dose read 116/132 with the prompt and 66/132 without, a gain of 5
    where the rule asked for 3; two of the three children are not told apart from it
    (`K/docs/qwen-last-block-lora-design.md:514-521`) **observed**.
- **The role prompt is the larger adapter.** Bare voice, 225 predicates: 209 with the role
  prompt, 116 without (`:435-446`) **observed**. The text prefix carries 93 predicates; the best
  trained pair moved 13 on a smaller set.

### 1.4 The rank-one head adapter and the reserved rows

- The Qwen GGUF declares 248,320 rows in both `token_embd` and `output.weight` while its
  tokenizer names the last 243 ids `[PAD...]`: rows the head already holds and no text selects.
  Form takes the first as the control-open id (248077) and the next nine as one id per verb
  (248078-248086) (`K/form/form-stdlib/qwen-lora-train.fk:54-62`;
  `K/docs/form-token-expansion.md:5-8, 25-29`) **observed**.
- A rank-one residual on the output-normed hidden state, `h + (a.h) b`, two 5120-vectors, is
  fitted by ridge regression on captured activations at the sites where a traced reply chose what
  came next. On 24 sites (17 kept, 7 held out): control openings before the fit 0 of 12, after
  11 of 12, other sites opening wrongly 0; held out 3 of 4 opened, 0 wrong; held-out separation
  10.0 against 2.2 for 63 scrambled labellings; artifact 41,152 bytes; wall 327 s
  (`K/receipts/form-token-lora.rows` row with `lane "qwen" ... target "control-id"`;
  `K/observe/form-token-lora-run.bml` head, lane `qwen`) **observed**.
- The same adapter trained on verified control sequences shows finite loss descent and no decoded
  quality gain, and is not promoted (`K/docs/native-session-learning.md:394-398`) **observed**.
- The serving lane does not wear it; "wearing it in the code lane is the step that makes the
  engineering voice a Form-token voice", and the crossings ledger that would record it holds 0
  rows in every checkout (`K/docs/form-token-expansion.md:25-29`;
  `M/project-form-native-token-usage-2026-10-08.md:10`) **observed**.

### 1.5 Other overlays in the tree

- **Teach layer (text).** A system-prefix any chat model can wear: eleven substrate nouns and
  eight control tags (`K/form/form-stdlib/form-teach-layer.fk:6-20`) **observed**.
- **Control-logit slot for another model.** DeepSeek's reserved control bank at id 128000 takes a
  five-float file; in the tree it is five zeros
  (`K/form/native/metal/artifacts/dsv4-control-logit-adapter.f32`, 20 bytes `0.0` x 5;
  `K/form/form-stdlib/form-cli-qwen-teach-layer.fk:8-20`: "write a small slot, leave base tensors
  bit-identical. That is not LoRA") **observed**. For Qwen "a logit-latch at q38-head is named,
  not claimed" (`K/form/form-stdlib/form-teach-layer.fk:19-20`) **observed**.
- **Per-lane rank-one adapters** for every registry model, all unpromoted
  (`K/docs/native-session-learning.md:361-369`) **observed**.

---

## 2. How the body's knowledge reaches the model today

### 2.1 Only as text, selected by words

- **The grounded door.** Up to six terms of five letters or more from the person's words; files
  under `teachings/` and `receipts/` ranked by whether their names carry those terms; at most two
  rows; each shown as its path plus a 180-byte opening, plus an exact-entry enrich reading
  (`K/observe/companion-ground.bml:14-31`) **observed**. For asks about the body, source doors
  are matched on name and head comment (`K/docs/grounded-voice-door.md:3-7`) **observed**.
- **Session experience and rehearsal.** "This is a lexical retrieval heuristic": shared-feature /
  union overlap over retained examples (`K/docs/native-session-learning.md:192-198, 283-288`)
  **observed**.
- **The planner and the code lane.** A packet of whole files or a catalog of names
  (`M/project-feeding-the-voice-2026-10-13.md:20-21`;
  `M/project-local-loop-goal-2026-11-09.md:81, 115`) **observed**.

### 2.2 Is anything embedded?

- `rag-embed.fk` is a lexical embedding: words hashed into a histogram, or a sparse list of
  dual-hash codes over a hand-authored concept lexicon; "not open-world language understanding;
  unknown synonyms remain apart" (`K/form/form-stdlib/rag-embed.fk:6-23`) **observed**.
- `rag-retrieve.fk` can rank quantised neural vectors by L1 distance, with the vectors produced
  by an outside embedder (nomic-embed-text through ollama), "a bootstrap carrier"
  (`K/form/form-stdlib/rag-retrieve.fk:5-23`) **observed**. The live ask lane uses the sparse
  semantic codes, not those vectors (`K/form/form-stdlib/rag-ask.fk:13-21`) **observed**.
- The retrieval index is typed by node id: a row carries `node_id "@8.3.4.9"`, a content node id,
  a source path as a lens and a vector; a path can never stand in for the id
  (`K/form/form-stdlib/rag-index-codec.fk:7-20`) **observed**. So retrieval already answers a
  node, and then the node's text is pasted.
- Dense vectors exist only for perception: 256-d voiceprints (6,018 samples), 768-d face prints
  (3,330), matched by cosine, with quantised packs 28 to 31 times smaller
  (`M/project-recognition-training-system.md:17-20, 45-51`;
  `K/form/form-stdlib/world-model.fk:6-24`) **observed**. Identical exemplars intern to one node
  id, so a union of two devices' sets dedups for free
  (`K/form/form-stdlib/co-learning.fk:9-14`) **observed**.
- The model's own embedding table is read as rows by a small kernel (`form_qwen_f32_row`,
  `d[i] = s[off+i]`, beside Q4_K and Q6_K row decoders;
  `K/form/form-stdlib/bml/qwen38-embedding.bml:6-9`) **observed**. Nothing writes a non-vocabulary
  vector into a position (**inferred** from the readings above and from no file found by a search
  for `*embed*` under `K/form` doing so).

### 2.3 What a grounding costs

- Retrieval itself: under 150 ms (`K/docs/grounded-voice-door.md:52-53`); an earlier walk grounded
  8 receipts into a 6,330-byte packet in 312 ms (`K/docs/rent-to-zero-goal.form:78`) **observed**.
- Tokens: the training rows measure 2.6 to 3.7 bytes per id on this tokenizer
  (`K/docs/qwen-last-block-lora-design.md:236`) **observed**; so two 180-byte excerpts with their
  paths and the framing sentences are on the order of 150 to 250 ids, and a 6,330-byte packet
  about 1,700 to 2,400 ids (**inferred** by division).
- Time to read it: batched prefill is 190 positions a second on a quiet GPU and 166 a second in
  the lane's sliced path at 188 ids (`M/project-local-loop-goal-2026-11-09.md:71, 107`); the
  rate falls with context, to about 41 ids a second at 12k and about 6 a second at 26k under
  contention, 70 minutes for one 26k-position packet (`:117, 123`) **observed**.
- Effect: on 55 predicates, bare 34, grounded 43, placebo ground 34; interfaces 10 to 18 of 18.
  A list of the offered paths with no model at all scores 24 of 30 on the two graded axes, so
  the gain is retrieval putting the right door in reach, and "it does not show alignment"
  (`K/docs/grounded-voice-door.md:35-59, 63-78`) **observed**.
- A prefix seen before is nearly free: a 2,614-id prompt prefilled again in 184 ms instead of
  19,462 ms from a state snapshot (`M/project-local-loop-goal-2026-11-09.md:128`) **observed**.
- Ceiling met: a planner packet of 1.6 MB against a 48,000-byte budget was refused unread; 26 of
  the planner's refusals in one night were this kind (`:81, 115`) **observed**.

---

## 3. How the model's output becomes nodes

### 3.1 The grammar

- A model writes `<|form:VERB|>PAYLOAD<|/form:VERB|>`; when the stream closes the control, Form
  runs the verb and prefills `<|form:observation|>RESULT<|/form:observation|>` into the same KV
  stream. RESULT is `value=<v>;`, `recipe=@p.l.t.i;`, `node=@p.l.t.i;` or `nothing=<reason>;`
  (`K/form/form-stdlib/bml/form-token-grammar.bml:3-19`) **observed**.
- Nine verbs in one list: recipe-birth, recipe-exec, choice, eval, knowledge-query, node-get,
  node-make, node-select, recipe-import; the stream also carries nodeid-knowledge-query and tool
  (`K/form/form-stdlib/bml/form-token-kinds.bml:15`;
  `K/form/form-stdlib/bml/form-token-grammar.bml:42`) **observed**.
- Node verbs work on immutable field content: `node-make|pair;3,4` returns `node=@address;`,
  "copy that actual coordinate in the next control"; `node-get|@address` returns
  `value=[kind, value, [children]];`; `node-select` takes a BML predicate and candidate addresses
  (`K/docs/local-reasoning-homecoming-lanes.md:22-40`) **observed**.
- A value too large for the observation is answered as `node=@<address> <summary>;`, and
  `node-get @address;from..to` reaches its parts
  (`K/form/form-stdlib/bml/form-token-verbs.bml:46-50`) **observed**.
- The tool verb: name, words, optional verbatim block. A word may be `@p.l.t.i`, "a text cell
  stands for its text, so a document or an earlier result is named by its node and never sent
  again". Tools are interned cells; the call is read by one cursor into the same
  `form-code-action` cell the lane steps, so the same call is the same node id; nine named
  refusals (`K/form/form-stdlib/bml/form-token-tool.bml:1-35, 41-52, 64-65`) **observed**.
- There is no mask over the model's logits: "a malformed call is refused in the stream, by name,
  after it is written. The first-try validity measured above is prompt only"
  (`K/receipts/2026-10-08-tool-call-as-control.md:81-82`) **observed**. A search of
  `K/form/form-stdlib`, `K/form/native/metal` and `K/observe` for a logit mask, bias or
  constraint found only the sentence that names the latch as unclaimed (grep) **observed**.

### 3.2 What a control costs in tokens

- On the 27B tokenizer `<|form:tool|>` is 7 tokens and its close 8; the observation envelope is
  7 and 8: 15 per call and 15 per result. Over seven real calls: JSON 46 tokens a call, an eval
  constructor 60, the tool control 54, the words alone 39
  (`K/receipts/2026-10-08-tool-call-as-control.md:24-26`;
  `M/project-form-native-token-usage-2026-10-08.md:11`) **observed**.
- The vocabulary already holds single ids for `<tool_call>` (248058), `</tool_call>` (248059),
  `<tool_response>` (248066), `</tool_response>` (248067); the lane spells its own markers as
  text (`K/docs/form-token-expansion.md:30-33`) **observed**.
- On the 3B voice the last evaluation reads 2,512 generated ids for 70 controls and 29 correct
  answers, 86 ids per correct answer; node-get 118 ids for 4 controls, node-select 282 ids for 8
  (`K/receipts/form-token-eval.rows` row 15) **observed**. About 30 to 35 generated ids per node
  operation (**inferred** by division).
- At 66 ms an id (quiet GPU, `M/project-local-loop-goal-2026-11-09.md:129`) a 54-token call is
  3.6 s of decode before anything runs (**inferred**).
- Encoding the observation was once the larger cost: 9,000 to 17,000 ms per crossing through the
  reference encoder (16 s for a 60-byte observation), 36 to 43 ms through the indexed cursor
  (`K/receipts/2026-10-08-tool-call-as-control.md:66-70`) **observed**. The tokenizer's own
  tables took 41 s to open before records gained a hash index
  (`K/receipts/2026-10-11-form-token-time.md:24-26`) **observed**.

### 3.3 Parse failures, repairs, rates

| reading | number | source | status |
|---|---|---|---|
| JSON era, 340 replies: a call valid first try | 148 of the 232 that tried (64%); 24 more only through the tolerant reader; 5 never read; 40 echoed the observation | `K/receipts/2026-10-08-tool-call-as-control.md:16-20` | observed |
| packet era, 175 replies: eval action valid first try | 131 of 160 (82%) | same | observed |
| 27B, tool control arm | 50 controls, 7 refused (14%): 5 read an `@address` as a path, 2 wrote a context read as a tool | same `:55-58` | observed |
| 27B, eval arm | 94 controls, 33 refused (35%): `unknown-name:code_context` 19, `task_context` 10, forged observation 6, parse and arity 4 | same | observed |
| one task both arms completed | tool arm 5 turns, 188 generated ids, 105 s; eval arm 6-7 turns, 450-526 ids, 177-228 s | same `:59-60` | observed |
| census of 388 streams | 252 controls in 18,414 generated ids (13.7 per 1,000); 231 observations; 181 forged observations written by the voice itself | `M/project-form-native-token-usage-2026-10-08.md:10` | observed |
| 3B voice, generation 50 | 127 controls, 26 refusals, 5 silent, 2 forged; 39 of 72 exact | `K/receipts/2026-10-09-form-token-endtoend.md:18-26` | observed |
| 3B voice, trained | 141-156 controls, 11-22 refusals, 0-4 forged; 66-67 of 81 exact | `K/receipts/form-token-eval.rows` rows 9-13 | observed |
| trainer contract | 49 of 664 corpus rows refused: 36 `unknown-verb` (the trainer's verb list lacked the ninth), 13 `observation-mismatch` | `K/receipts/2026-10-09-form-token-endtoend.md:8-9` | observed |
| parse cost | control scan 2 us, tool reader 123 us, eval cursor 763 us, tolerant JSON reading 14,267 us | `K/receipts/2026-10-08-tool-call-as-control.md:22-23` | observed |

The repair machinery (a tolerant JSON reader, a call repair step) is still in the sources, pinned
by a band, and unreachable from the model (`:8-12, 83-84`) **observed**.

---

## 4. What "form token expansion" and "form share" measure

- **Form token expansion** (`K/docs/form-token-expansion.md`) is a plan and a meter for the nine
  control verbs. The meter (`observe/form-token-usage-run.bml`) reads an evaluation row kind by
  kind: taught, expected, written, substituted, kinds used, each family's answers. "Effective
  kinds" is the diversity of verbs written (3.84 to 4.40 on one run). The expansion's order is:
  contrast rows before volume; one closed loop per feature (a family in the corpus, a line in the
  meter, a claim in a band); then the Qwen head taking one id per Form token; then the model's own
  tool envelope ids; then features the voice cannot reach yet (`:5-37`) **observed**.
- **Form share** (`K/docs/form-share-measurement.md`) counts events in a completed turn of a
  rented coding session: native (completed `fkwu` command receipts), local (other command
  receipts), remote (provider call-usage events), normalised to 100%. One observed turn read
  native 56, local 127, remote 58, or 23/53/24. "They are not token-volume percentages, semantic
  contribution, autonomy, or proof that the voice is home" (`:37-51`) **observed**.
- The related cost reading: native preparation plus one provider synthesis took 92,060 ms and
  26,946 tokens where the context-equipped baseline took 258,074 ms and 459,735; on a second
  comparison the advantage "did not survive the correction", and an answer review still found a
  causal error (`K/docs/form-response-synthesis.md:94-112`) **observed**.

---

## 5. Every place the documents wish the model could address nodes directly

1. "Dedicated learned Form IDs require compatible embedding and output rows as well as their
   native bindings. An adapter alone does not add those rows or make a request execute." and, as
   remaining work: "dedicated learned Form IDs ... Fixed pretrained Qwen weights still require
   their model-specific ID mapping. Native training can move that boundary toward Form's own
   cells; renaming the existing codec would not accomplish it"
   (`K/docs/local-reasoning-homecoming-lanes.md:197-200, 215-219`) **observed**.
2. "Model, parser, compiler and hardware should share native Form recipe identities" (`:206`)
   **observed**.
3. "The reserved PAD rows give each Form token one id ... wearing it in the code lane is the step
   that makes the engineering voice a Form-token voice"
   (`K/docs/form-token-expansion.md:25-29`) **observed**.
4. "Feeding the model its own ids is cheaper and what it was trained on" (`:30-33`) **observed**.
5. The owner, 2026-10-08: "to call tools we shall use form native substrate node ids or node
   serialization, JSON means it is not form native and is adding way too much overhead"
   (`K/receipts/2026-10-08-tool-call-as-control.md:3`) **observed**.
6. The owner, 2026-10-04: "we don't have to serialize, we can send IDs, so many tasks can become
   much, much simpler"; and 10-05: "we can adapt any tool to return exactly what the caller
   needs, with no parsing required at all" (`M/feedback-tools-speak-node-ids.md:13, 19`)
   **observed**.
7. Not done as of 10-08: "node-address edits (span by address or line range instead of the old
   text)" (`M/project-form-native-token-usage-2026-10-08.md:13`) **observed**.
8. "Decode under the grammar. The session lane has no logit mask"
   (`K/receipts/2026-10-08-tool-call-as-control.md:81`) **observed**.
9. "A logit-latch at q38-head is named, not claimed"
   (`K/form/form-stdlib/form-teach-layer.fk:20`) **observed**.
10. "It still reads what we say as strings; one meaning with many codes, the same cell in every
    tongue, is a seed of 29 rows" (`K/docs/local-agent-goal.form:13-15`) **observed**.
11. The `@p.l.t.i` word in the tool control and the `node=@address` result are the partial form
    of the wish: the node is addressed, but by digits the model must copy
    (`K/form/form-stdlib/bml/form-token-tool.bml:21-23`;
    `K/docs/local-reasoning-homecoming-lanes.md:30-31`) **observed**.

---

## 6. Recorded failures of the text route, with numbers

### 6.1 References that do not resolve, or resolve to the wrong thing

- `unknown-name:code_context` 19 and `task_context` 10 of 94 eval controls
  (`K/receipts/2026-10-08-tool-call-as-control.md:58`) **observed**.
- An `@address` read as a path, 5 of 50 tool controls (`:57`) **observed**.
- Planner, one night of 9 h 47 m: 24 requests, 0 queued. Refusals: packet unread 26, reference
  did not compile 15 ("model writes lisp-ish dialect, not BML"), gap already closed 6, case call
  moved 4, unreachable 3 (`M/project-local-loop-goal-2026-11-09.md:115`) **observed**.
- `reference-missing:organ_steady`: the true subject was not among the 12 names of the request's
  catalog, so a correct plan was refused five times (`:95-97`) **observed**.
- `request-definition:wfp-admit`: an 18,790-byte, 5,966-id plan refused because it changed
  `wfp-known?` where the request named `wfp-admit` (`:124`) **observed**.
- `reference-missing:organ-health-identity-band`: the model wrote a band file where a definition
  was asked, three options, 1,598/1,582/1,535 ids (`:130`) **observed**.
- A packet's wording `x.str()` was copied literally as `x.str(ms)`
  (`M/reference-live-planner-walk-to-landing-2026-11-20.md:25`) **observed**.
- The three newest commits on the branch are repairs of this class: near-name ranking, loading
  an unloaded file that holds a called name, and telling the next try "how each called name
  stands" (git log in the session's starting status, b8ff978c9, 827392e93, 52f96b8f5)
  **observed**.
- Paths cited by the voice: 9 of 9, 14 of 14 and 5 of 5 stood; invented paths 0. But a standing
  path was the wrong one for one ask (`K/docs/grounded-voice-door.md:45, 50-51`) **observed**.
  The 27B does not invent paths when the right ones are in the packet.

### 6.2 Framing failures

- 181 forged observations in 388 streams before the fix; 6 of 94 eval controls; 0 to 4 per 81
  questions on the trained 3B (section 3.3) **observed**.
- 40 of 340 JSON-era replies echoed the observation (section 3.3) **observed**.
- After an in-stream control ran, decoding stopped and the cut read as `generation-incomplete`:
  three days with no green, every option ending after about 29 ids
  (`M/project-feeding-the-voice-2026-10-13.md:50-53`) **observed**.
- Two ceilings (96 ids, 3 crossings) read the longest control chains as model failures:
  node-select 0 of 5, recipe-both 0 of 4 with 4 forged, though "the voice had written the
  controls correctly" (`K/receipts/2026-10-11-form-token-time.md:33-34`) **observed**.

### 6.3 The wrong one of two real things

- Verb substitution after rebalancing the corpus: recipe-exec 9 expected, 0 written (the voice
  wrote `choice`); node-select 5 expected, 15 written; exact answers 55 of 64 to 39 of 72;
  observations carried into the answer 50% to 27%
  (`K/receipts/2026-10-09-form-token-endtoend.md:18-29, 36-40`) **observed**.
- A greedy voice returns byte-identical replies to near-identical packets: three options of
  2,393 ids each, all failing the same way; about 15 to 30 minutes each
  (`M/project-local-loop-goal-2026-11-09.md:95, 99`) **observed**. Sampling was added later and
  the replies then differ and "still fail the same way" (`:131`) **observed**.

### 6.4 Cost in tokens and time

- The native turn ledger, all 707 rows from 2026-09-25 to 2026-11-21, tallied today (awk over
  `K/receipts/native-turn-ledger.jsonl`): 457 rows `already-full`; 250 real options; 106 options
  in which the model generated at least one id (78 on the Q4 file, 22 on Q8); 8 of those 106
  green (7.5%); 124,196 generated ids against 1,786,468 injected ids, 14.4 injected per
  generated; 210,151 s of wall (58.4 h), 1.69 s of wall per generated id. The 8 green options:
  7,915 generated, 54,951 injected, 9,297 s. Of the 144 real options with no model, 32 were
  green **observed** (the tally is mine; the ratios are **inferred** by division).
- A lane option's median is 38 minutes over 58 real options
  (`M/project-local-loop-goal-2026-11-09.md:57`) **observed**.
- Two landed voice edits: 359 generated tokens in about 10 minutes; 4,264 tokens in 3.5 hours;
  "both edits were exactly right" (`M/project-feeding-the-voice-2026-10-13.md:30-32`)
  **observed**.
- A zero-guidance round: 55 model turns in 1 h 54 m (14 rg, 14 read, 2 edit, the rest text), two
  correct edits, no check run, diff empty (`M/project-zero-guidance-rounds-2026-10-04.md:15`)
  **observed**.
- The first full zero-rented turn: 2 options, 7,211 generated ids, about 35 minutes an option,
  then executed in 2.9 s; the change passed its band and failed the real gate, and was reverted
  (`M/project-local-loop-goal-2026-11-09.md:83`) **observed**.

### 6.5 Failures that are not about text

- Witnesses satisfied by constants and by deleting conditions; three landings reverted
  (`M/project-flow-honest-refusals-2026-11-18.md:13-15`) **observed**.
- Contention: one Q8 session decodes at 105 to 218 ms an id, two at 0.8 to 1.3 s an id each
  (`K/docs/qwen-last-block-lora-design.md:487, 527`); the always-on ear cost 3.9 times decode
  (`M/project-local-loop-goal-2026-11-09.md:87`) **observed**.
- Adapter generalisation: sections 1.3 above.

---

## 7. What the machine's numbers say a node step may cost

All from the old body's own readings on the M4 Max, Q8_0 27B.

- A decode step reads about 29.05 GB of weights; 66.1 ms a token on a quiet GPU, 439 GB/s, 80%
  of a 546 GB/s floor (`M/project-local-loop-goal-2026-11-09.md:129`) **observed**. Apple's own
  note says the same of MLX: "Generating subsequent tokens is bounded by memory bandwidth, rather
  than by compute ability" (machinelearning.apple.com/research/exploring-llms-mlx-m5, published
  2025-11-19) **relayed**.
- The vocabulary head is `output.weight`, 248,320 x 5120 Q8_0, 1.35 GB
  (`K/docs/qwen-last-block-lora-design.md:16`) **observed**: 4.6% of a step's bytes
  (**inferred**).
- So a second head over node rows of width 5120 costs, per step (**inferred**, bytes over 29.05
  GB): 4,096 nodes in f32, 84 MB, 0.3%; 65,536 nodes in Q8, 336 MB, 1.2%; 248,320 in Q8, the
  vocabulary head again, 4.6%; 1,000,000 in Q8, 5.1 GB, 18%. Through a 256-wide projection of
  the hidden state, 1,000,000 nodes are 256 MB, 0.9%. A flat node head stays inside "about a
  token step" up to roughly 10^5 nodes; past that it needs a projection or a two-step code.
- Reading a node as one position costs one position: 5.3 ms in batched prefill at 190 positions a
  second, 66 ms as a single step (**inferred** from the two rates).
- A second stream in the same weight pass is nearly free at N = 2: 68 ms a step for two streams
  against 67 ms for one; N = 4, 82 ms; N = 8, 88 ms in the fast mode on the best reading, 157 to
  194 ms on others (`M/project-local-loop-goal-2026-11-09.md:67, 107-111, 115`) **observed**.
  This is the measured basis for cheap A/B: variants as rows of one pass. It was measured without
  adapters; per-row adapter records are not built (section 1.3).
- A rank-8 pair adds 180,224 multiply-adds per token on a 17408-to-5120 projection and reads
  1.003 times bare (section 1.3).
- A known prefix restored from a snapshot costs 184 ms instead of 19.5 s (section 2.3). With 48 of
  64 layers recurrent, a snapshot is a state after a prefix, not a block that can be spliced at
  an arbitrary point (**inferred** from the layer pattern at
  `K/docs/qwen-last-block-lora-design.md:16, 74` and from the Gated DeltaNet recurrence
  `S_t = S_{t-1}(a_t(I - b_t k_t k_t^T)) + b_t v_t k_t^T`, arXiv 2412.06464 v3 of 2025-03-06,
  **relayed**). That paper also states that a fixed state size "makes it hard for retrieval
  tasks" (**relayed**), which is the reason the hybrid keeps attention every fourth layer.

---

## 8. What outside work shows (fetched 2026-11-24; all relayed)

Every row here came through a fetch tool that summarises pages; numbers should be read at the
source before anything is built on them.

### 8.1 Choices out: units of the system as rows of the output head

| work | date | what it shows | numbers |
|---|---|---|---|
| ToolkenGPT, arXiv 2305.11554 v4 | 2023-05-19, rev. 2024-01-15 | Each tool is one extra row `W_tau` appended to the output matrix of a frozen LLM; "this embedding matrix contains the only parameters to optimize"; arguments are then written in a separate tool mode with demonstrations | 13, 58 and 234 tools; about 40 to 200 examples a tool; training about 2 minutes on one 24 GB GPU against about 40 minutes on eight 80 GB GPUs for LoRA; FuncQA one-hop 0.73 against ReAct 0.57; VirtualHome success 0.68 against 0.20 in-context |
| ToolGen, arXiv 2410.03439 v3 | 2024-10-04, rev. 2025-03-29 | One vocabulary token per tool, 46,985 new tokens on Llama-3-8B (128,256 to 175,241), full fine-tuning in three stages; constrained decoding to the tool set | with the constraint it "does not hallucinate at all"; without it 7% non-tool tokens; a baseline with no tools in the prompt wrote over 50% nonexistent tool names; weaker than a baseline on unseen tools (its appendix J) |
| TokMem, arXiv 2510.00444 v2, ICLR 2026 | 2025-10-01, rev. 2026-03-08 | One trainable vector per procedure, shared between input embedding and head, backbone frozen; the model routes by logits over memory tokens; new vectors renormalised to the mean norm so later ones do not crowd out earlier ones | 1,000 procedures; routing accuracy 94.7 to 97.5% against 79.7% for a sentence-embedding retriever; better than LoRA at 10 to 500 samples a task; 0.10 to 0.20 M trainable parameters against 0.85 to 3.41 M; unfreezing the backbone causes severe forgetting |
| XGrammar, arXiv 2411.15100 v3, MLSys 2025 | 2024-11-22, rev. 2025-05-12 | Grammar masks with most tokens prechecked per grammar state | "up to 100x" over earlier engines, "near-zero overhead" (no absolute figure in the abstract) |
| TIGER, arXiv 2305.05065, NeurIPS 2023 | 2023-05-08 | An item is a short tuple of codewords (a semantic ID) decoded autoregressively, in place of nearest-neighbour search | no numbers in the abstract |

### 8.2 Embeddings in: a unit of knowledge as one or a few input vectors

| work | date | what it shows | numbers |
|---|---|---|---|
| xRAG, arXiv 2405.13792 v2 | 2024-05-22, rev. 2024-12-09 | A document's retrieval embedding (4096-d) goes through a two-layer MLP into one input position of a frozen 7B; only the MLP is trained (0.46% of the 7B's parameters) | 175.1 tokens become 1; average score 42.46 against 43.63 for full-text retrieval and 36.54 with none; FLOPs down 3.53 times; "lags behind by a considerable gap" on tasks that reason over the document |
| KBLaM, arXiv 2410.10450 v2 | 2024-10-14, rev. 2025-02-09 | Each triple becomes a key vector and a value vector through a sentence encoder and linear adapters; prompt tokens attend to them, they attend to nothing, so one triple can be replaced alone; adapters trained on synthetic data | more than 10,000 triples into an 8B model with an 8K window on one 80 GB GPU; cost linear in the number of triples |
| ConceptFormer, arXiv 2504.07624 | 2025-04-10 | An entity's neighbourhood in a graph becomes 1 to 20 vectors in the input space of a frozen LLM, precomputed into a lookup table | GPT-2 only; Hit@1 46.7% against 6.6% for the graph as text on one set; about 100 to 130 times fewer input tokens |
| KoRe, arXiv 2605.20170 v2 | rev. 2026-07-22 | A 1-hop subgraph becomes 20 discrete codes (codebook of 128) through a graph network and residual quantisation, injected at a placeholder; Qwen3-8B with a rank-4 LoRA | 62.9 tokens against 676.5 as text; Hit@1 51.4 against 1.8 on one zero-shot set; single hop only; on one set plain text scored higher |
| GraphToken, arXiv 2402.05862 | 2024-02-08 | A learned graph encoder writes soft tokens for an LLM | "up to 73% points" on graph tasks |
| Token Distillation, arXiv 2505.20133 v3 | rev. 2026-03-13 | One new input embedding is optimised so the frozen model's hidden states match those it has for the multi-token spelling | 2,500 new tokens in under 10 minutes on one GPU, at most 25 snippets of 50 tokens each; definition correctness 74.4 against 18.6 for the mean of sub-token embeddings; does not learn output rows; a degenerate large-norm row on a tied-embedding 3B model |
| Concept Tokens, arXiv 2601.04465 | 2026-01-08 | One new token whose embedding alone is trained from definitions steers a frozen model | no numbers in the abstract |

### 8.3 Capability attached without retraining the model

| work | date | what it shows | numbers |
|---|---|---|---|
| LoRA, arXiv 2106.09685 | 2021-06-17 | low-rank pair on frozen weights | "no additional inference latency" when merged |
| ReFT, arXiv 2404.03592 v3 | 2024-04-04 | learned edits of hidden states on a frozen model | 15 to 65 times fewer parameters than LoRA |
| Function vectors, arXiv 2310.15213, ICLR 2024 | 2023-10-23 | a task demonstrated in context is carried by a compact vector that triggers the task elsewhere; vectors "can be summed" to some extent | none in the abstract |
| Text-to-LoRA, arXiv 2506.06105 | 2025-06-06 | a hypernetwork writes a LoRA from a task description in one forward pass | trained on 9 adapters; compresses hundreds |
| Doc-to-LoRA, arXiv 2602.15902, ICML 2026 | 2026-02-13 | a hypernetwork writes a LoRA that holds a document, so later queries do not re-read it | near-perfect needle recall at more than 4 times the context window; sub-second (vendor post of 2026-02-27) |
| The Override Gap, arXiv 2604.23750 v3 | 2026-04-26, rev. 2026-11-22 | such written adapters fail when the document contradicts pretraining | 46.4% on deep conflicts, 71.0% after scaling the adapter at its most active layers; 16.3% in the strongest-prior quartile |
| Memory Layers at Scale, arXiv 2412.09764 | 2024-12-12 | a trainable sparse key-value lookup adds parameters "without increasing FLOPs" | up to 128 B memory parameters; beats dense models of more than twice the compute; needs pretraining (1 trillion tokens) |
| kNN-LM, arXiv 1911.00172, ICLR 2020 | 2019-11-01 | next-token distribution interpolated with nearest neighbours of the hidden state in a datastore, "no additional training" | perplexity 15.79, 2.9 better |
| Apple foundation models report, arXiv 2407.21075 | 2024-07-29 | task adapters on a frozen on-device base, "dynamically loaded, temporarily cached in memory, and swapped" | rank 16 on a 3 B model, "10s of megabytes", 16-bit adapter values, base at 3.7 bits a weight |

### 8.4 Many variants side by side

| work | date | what it shows | numbers |
|---|---|---|---|
| S-LoRA, arXiv 2311.03285 v3 | 2023-11-06 | thousands of adapters on one base; heterogeneous adapters in one batch | up to 4 times the throughput of two libraries |
| MoLoRA, arXiv 2603.15965 | 2026-03-16 | a router (2-layer MLP, hidden 64) picks an adapter per token | a learned router costs 12 to 49 times a single adapter's kernel time for k = 1 to 4; its headline accuracy claim lacks absolute scores |
| Scute, arXiv 2611.01089 | 2026-10-31 | DoRA, IA3, VeRA and OFT written as one LoRA-shaped normal form and served in one batch | 5 to 17% more decode time than the same number of LoRA adapters; 64 to 1,024 small adapters a GPU |
| Coconut, arXiv 2412.06769 | 2024-12-09 | the last hidden state fed back as the next input in place of a decoded token | none in the abstract; the model is trained for it |

### 8.5 The model itself

Qwen3.5-family models mix Gated DeltaNet layers with full attention every fourth layer and use a
vocabulary of 248,320 (search summary of vendor and framework documentation) **relayed**. The
paper the old body borrows its turn rewards from names "Qwen3.8-27B" as its base (arXiv
2610.22192, 2026-10-15) **relayed**.

---

## 9. Which costs and failures a direct node interface would remove

A direct node interface here means: (in) a node occupies one input position as a vector the
model reads; (out) the head scores nodes beside tokens, masked by what the current blueprint
admits, and a chosen node is taken by handle with no text between.

### 9.1 Removed by construction

| failure or cost | recorded size | why it goes |
|---|---|---|
| names that do not exist (`unknown-name`, `reference-missing`, `tool-unknown`, `tool-address-unknown`) | 29 of 94 eval controls; 15+ planner refusals in one night | the head has rows only for nodes that exist; the seed design already says an unresolved name is a fault, never a value (`S/` node 5.3.8) |
| an address read as a path; a name written where a definition was asked | 5 of 50; three planner options | a kind mask from the expected blueprint admits only rows of that kind |
| forged observations and echoed observations | 181 in 388 streams; 40 of 340 replies | observation rows are never in the output mask; only the system writes them |
| malformed calls, unclosed quotes and blocks, JSON repair, dialect errors | 36% of JSON-era calls not valid first try; 15 `reference-compile` refusals | there is no surface syntax to get wrong; a node is a blueprint id and child ids (`S/` node 5.1.1) |
| the envelope | 15 of 54 tokens a call, 15 more an answer | one step selects; one position answers |
| spelling an address in digits and copying it back | every `node=@p.l.t.i` round trip | the handle is the row |
| the tokenizer on the system's side of the loop | 9 to 17 s a crossing before the index; 41 s open; 12.6 s a row in the judge | an observation that is a node is one row lookup |
| re-sending text the model has already been given | packets of whole files; 14.4 injected ids per generated id | a node already read is one position, or zero if its state is held |
| the tolerant reader, the repair step and their bands | still in the tree, unreachable | nothing to repair |

Outside evidence that the mechanism works on a frozen model: ToolkenGPT, TokMem and ToolGen in
8.1, and the body's own rank-one fit that opened 11 of 12 control sites with 0 wrong (section
1.4).

### 9.2 Reduced, not removed

- **Tokens per reference.** One position instead of 50 to 250 for an excerpt (xRAG: 175 to 1;
  KoRe: 677 to 63). The saving is in prefill, which is the cheaper phase here (5.3 ms a position
  batched against 66 ms a decoded token), so the larger gain is on the output side: one 66 ms
  step in place of 30 to 54 decoded tokens.
- **Long-context cost.** Fewer positions, but attention cost per position still grows with
  context, and the 48 recurrent layers must still be walked in order.
- **Retrieval misses.** A learned node row can route better than word overlap (TokMem 94.7 to
  97.5% against 79.7%), but a node with no row, or a row computed without training, routes no
  better than its initialisation (Token Distillation: 18.6 against 74.4 for the untrained mean).

### 9.3 Not removed

- **The wrong one of two real nodes.** Verb substitution (recipe-exec 9 expected, 0 written) is a
  choice between two rows that both exist and both pass any mask. Nearby rows may make it worse;
  TokMem's norm inflation is the same hazard on the row side.
- **Plans that change the wrong definition**, and plans whose witness is a constant. These are
  errors of judgement over well-formed nodes (sections 6.1, 6.5).
- **New content.** Creating a node whose leaf is new bytes (a number, a string, a body no
  existing node holds) still takes one step per child or per token. A direct interface makes the
  result well-formed; it does not make it short.
- **Exact recall from a compressed node.** One vector cannot carry a document's exact words: xRAG
  trails on tasks that reason over the text; KoRe handles one hop. A node the model must quote
  still has to be expanded, on demand, into its print.
- **Conflict with what the model already believes.** Written adapters fail at 16 to 46% when the
  knowledge contradicts pretraining (8.3).
- **Thin generalisation of small adapters.** The body's own placebo reading (section 1.3) is not
  about text.
- **Contention and wall time per step.** One model session at a time is about twelve times faster
  than two (`K/docs/qwen-last-block-lora-design.md:487`) **observed**.
- **Per-model rows.** A node's row is fitted to one model's hidden space. Every model, and every
  adapter that moves the hidden space, needs its own rows; a row is a derived thing keyed by
  (node, model, encoder), like compiled code in the seed design (`S/` nodes 5.6.4, 5.10.2)
  (**inferred**).

### 9.4 New costs it adds

- A row table: 20,480 bytes a node in f32 at width 5120, 5,120 in Q8; a million nodes are 5 to
  20 GB unless projected (section 7) (**inferred**).
- Fitting rows. Output rows: closed-form ridge on captured activations (built, 327 s for 24
  sites) or head-only gradient steps (built, about 0.7 s a step). Input rows by gradient need a
  backward through the whole model, which exists for the 3B and not for the 27B's recurrent
  layers (section 1.3). Input rows without gradient need a bridge trained once per model (xRAG's
  MLP), which is again a full-depth backward (**inferred**).
- A second place for drift: rows trained against a node's old content. Content addressing settles
  it if the row's key holds the content id; a changed node is a new node with no row yet
  (**inferred**).
- A mask per step. The seed design's blueprint supplies the set; outside work reports the masking
  itself at near-zero cost (8.1, relayed, no absolute figure).

---

## 10. What the design could take, stated as proposals (all inferred)

These follow from sections 1 to 9; none is built or measured.

1. **Reserved rows first.** The 243 unused rows are in both tables of the frozen file. The core
   operations and control words of the seed (about 25 plus control, `S/` node 5.1.5) fit in them
   with a rank-K head overlay, no tensor reshaped. The first measurable step is the one the old
   body stopped before: wear the control-id overlay in a live lane and count crossings.
2. **A node head beside the vocabulary head**, over the nodes in scope (workspace plus
   retrieved), not over the whole space. At 4,096 to 65,536 rows it costs 0.3 to 1.2% of a step.
   Reading through it is the seed's "reading is a choice that may find no node, one or several"
   (`S/` node 5.3.3): the top rows are the candidates.
3. **A mask from the blueprint.** The expected child's blueprint gives the admissible kinds, so
   the framing failures of section 6 cannot be written.
4. **Creation as a walk.** A new node is emitted as its blueprint, then its children in order,
   each an existing node, a nested new node or a leaf in token mode; it is a transient node with
   no id until it freezes (`S/` nodes 5.1.1, 5.2.3). Cost: one step per child.
5. **Rows as derived data.** Key (content id, model id, encoder id); droppable and remade.
6. **Adapters as nodes; variants as rows of one pass.** An adapter is a leaf of bytes under a
   blueprint that names layer, slot, rank and scale. A/B is two rows of a multi-stream pass
   wearing different records; the base pass was measured at 68 ms for two rows against 67 for
   one. Every comparison carries the permuted-pair arm the old body found it needed.
7. **Prefix state as a held node.** A role prompt or a standing packet is restored, not re-read
   (19.5 s to 184 ms). It composes only as a prefix on this architecture.
8. **Prototype the input side on the 3B**, where the full backward exists, before writing the
   recurrent backward for the 27B.

---

## 11. What could not be reached

- The old runtime was not run. Every figure about it is from its documents, receipts and notes;
  several notes are 12 to 57 days old.
- The planner's refusal tally and the decode and prefill rates stand on one memory note written
  during the work it describes; the ledgers behind the planner tally are in local `.hearth`
  directories that were not read.
- "Injected ids" in the turn ledger was read by its name and by the lane's own reader
  (`K/form/form-stdlib/bml/local-flow-reading.bml:200`), not by its definition.
- How many tokens an `@p.l.t.i` address costs on the 27B tokenizer was not measured.
- Whether all 243 trailing rows are free of any trained use beyond "nonzero" was not checked.
- Every web claim is relayed. Abstract pages gave no numbers for several papers (ToolkenGPT's
  abstract, Coconut, Gated DeltaNet, TIGER); full-text fetches were cut at 100,000 characters.
  The Qwen3.8 model card was not found; the architecture claim rests on the old body's own
  reading of the file plus search summaries about the 3.5 family.
- No source found measures the specific combination asked for: a frozen model on Apple GPU
  choosing among content-addressed nodes through an added head with a kind mask. The nearest are
  ToolkenGPT and TokMem (rows out, frozen model) and xRAG and KBLaM (vectors in, frozen model);
  none combines both sides, none reports Metal figures.
