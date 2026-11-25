# Expanding a frozen model at run time: adapters, steering, features, edits, memories, branches, dreams

Study for the coherence-seed design graph. Written 2026-11-24. All web pages were fetched on 2026-11-24.

## 0. How to read this report

Status words, used on every claim:

- **observed**: I read the file or ran the command myself in this session.
- **relayed**: read through a web fetch (which summarises the page through a small model) or through a note
  written by an earlier session. The number is the source's, not mine; a relayed number can be wrong in a digit.
- **inferred**: my reasoning; the premises are named.
- **assumed**: from my prior knowledge, not checked today.

Sources are listed in section 12 as `[Sn]` with URL, the source's own date and the fetch date. Where an arXiv
abstract page carried no number, I fetched the paper's HTML and say so.

Scope: ways to add to or redirect a frozen language model without retraining it. The neighbouring question (which
encoder makes a node's embedding, and how a node id becomes a token) is touched only where an adapter-like unit
answers it (sections 5.5 and 9); I take it that other studies of this round cover encoders in depth (assumed).

Ground read first (observed): `coherence-seed/graph/0.kg` lines 1-61 and the output of
`./kg brief` for hub 5 (`coherence-seed/graph/5.kg`). The design nodes this report leans on:
5.1.1 one-node-shape, 5.2.2 content-id, 5.2.3 identity-at-freeze, 5.2.7 holder-identity, 5.4.3 choice, 5.4.9
choice-point-stack, 5.6.2 ice-water-gas, 5.6.4 compiled-code-is-not-ice, 5.7.2 regions-follow-choice, 5.10.2
recipe-is-the-unit, 5.10.7 matrix-and-gpu, 5.11.1 a-check-must-be-able-to-fail. A search of the brief for
"embedding", "LoRA", "adapter", "token" found no design node about a model executor (observed), so everything
below is new ground for the graph.

## 1. The one-page answer

1. **The unit of capability comes in seven shapes**, from heavy to light: a low-rank matrix pair (LoRA family), a
   per-channel scale vector ((IA)^3, VeRA), a residual-stream vector (steering, function vectors), a low-rank
   edit of the representation at chosen positions (ReFT), a dictionary feature (SAE or transcoder direction), a
   key-value row attached to a layer (GRACE, kNN-LM, memory tokens), and a cached attention state (prompt cache,
   Cartridges). A logit-space mix of two runs (classifier-free guidance, contrastive decoding) is an eighth that
   has no stored unit at all. Section 10 has the table.

2. **Almost every one of them costs under 3% of a decoded token when applied unmerged**, by arithmetic (inferred,
   section 2). The measured overhead of "dynamic adapters" in 2024-2026 systems, 2.5x slower decoding [S19][S20],
   comes from many small GPU kernel launches, not from the arithmetic. The cure both papers use is one routing
   decision per token and one fused kernel. That is a direct instruction for a Metal executor that already pays
   about 1,300 dispatches per token (relayed, [L2]).

3. **The units that can be created inside a decode step are the ones whose content is an activation the forward
   pass already computed**: a steering vector is a difference of two residuals, a codebook key is a cached
   activation, a node's output row can be an output-layer row or a hidden state, a KV block is the prefill's own
   product. Creating one is a copy plus a freeze. Matrix-pair units need either gradient steps (minutes) or a
   meta-trained hypernetwork (one pass, 0.2 s for a 2B model, after days of meta-training per base model) [S23].

4. **Composition is where the published record is weakest.** Adding LoRAs loses each one's identity [S15];
   in-place weight edits collapse after 100 to 4,000 sequential edits [S45]; steering has a narrow usable range
   and off-target effects [S36]; SAE features are not stable identities across trainings (30% shared across
   seeds) [S38]. What holds up is anything kept **beside** the weights and **routed**: side memories and
   codebooks [S43][S44], concatenated ranks [S23], additive accumulators [S24], cached KV blocks [S27].

5. **For a store of immutable content-addressed nodes the lesson is simple**: never mutate the base, keep every
   unit unmerged as a cell bound to the base model's id, treat merged weights and cached KV as derived
   materializations (the same class as compiled code in 5.6.4), make A/B a batch of rows over one shared prefix,
   and let consolidation ("dreaming") only mint candidate cells that are accepted by a witness on real held-out
   rows. Section 9 gives the lessons, section 11 nine constructions with a first experiment each.

## 2. A cost model to read everything against

Premise (relayed, [L3]): on the old body's Metal lane a decode step of the 27B model performs one fused
multiply-add per weight per token; the lane's width is d = 5120 and its feed-forward width 17,408. So the base
cost of one decoded token is about P = 2.7e10 multiply-adds. Assumed where needed: 64 layers, a vocabulary of
about 152,000.

Arithmetic (all inferred from those premises):

| Unit applied at one token | Multiply-adds | Share of a 27B token |
|---|---|---|
| Steering vector, one layer | 5,120 adds | 2e-7 |
| (IA)^3 scale vectors, all layers | about 1.2e6 | 5e-5 |
| LoRA rank 8 on query and value, all layers, unmerged | 2 x 2 x 8 x 5120 x 64 = 1.05e7 | 0.04% |
| LoRA rank 64 on all seven matrices, all layers, unmerged | about 4.4e8 | 1.6% |
| LoReFT rank 4, one layer, one position | about 4.1e4 | 2e-6 |
| The vocabulary head itself (152k x 5120) | 7.8e8 | 2.9% |
| A node head over N node rows (N x 5120) | N=152k: 7.8e8; N=1M: 5.1e9 | 2.9%; 19% |
| Codebook or kNN lookup over N keys at one layer | N=4,500: 2.3e7; N=1M: 5.1e9 | 0.09%; 19% |
| SAE encode, 131k latents, one layer | 6.7e8 | 2.5% |
| Attending to a cached KV block of K tokens (all layers) | about 5.2e5 x K; K=500: 2.6e8 | 1% |
| A second row in the same batch (guidance, A/B) | one more row | measured locally at about 18% of a token per extra row (relayed [L4]) |

Two readings follow. First, a node head the size of the vocabulary costs what the vocabulary head costs, about 3%
of the step: picking a node instead of a token is affordable by brute force up to a few hundred thousand nodes,
and needs an index beyond that. Second, the arithmetic is never the problem below rank 64; dispatch count and
memory traffic are.

Local measurements these shares should be multiplied into (relayed from notes of earlier sessions on this
machine, an M4 Max; I did not rerun anything):

- 27B, Q8_0: the old runtime decodes at 5.08 tokens/s (197 ms/token, 1,356 dispatches/token) quiet; llama.cpp on
  the same file 9.27 tokens/s [L2].
- 3B: 20.7 ms/token burst, 50.8 ms/token sustained over 400 tokens (the GPU governor throttles 2.6x under
  sustained load) [L5].
- An 8-row verification batch took 235 ms against 105 ms for one token: each extra row costs about 18% of a
  token [L4].
- A LoRA training forward on the 3B took 36 s for a 1.2k-token row on 2026-09-11; the voice adapters are rank 8
  over 8 layers, 600 iterations [L1].

## 3. The LoRA family: a matrix pair as the unit

| Method | Unit | Size (source's numbers) | Per-step cost | Made by |
|---|---|---|---|---|
| LoRA [S1] | pair B (d x r), A (r x d) per adapted matrix | GPT-3 175B, r=4 on q,v: 4.7M parameters, 35 MB against 350 GB; RoBERTa-base 0.3M | merged: zero added latency; unmerged: +2rd per matrix | gradient steps |
| QLoRA [S2] | LoRA over a frozen 4-bit base | fine-tunes a 65B model on one 48 GB GPU | as LoRA | gradient steps |
| DoRA [S3] | magnitude vector + low-rank direction | no figure in abstract | none when merged; "bigger overhead than pure LoRA" unmerged [S12] | gradient steps |
| rsLoRA [S4] | LoRA scaled by alpha/sqrt(r) [S12] | same | same | gradient steps |
| VeRA [S5] | two scale vectors per matrix over one frozen random pair shared by all layers, regenerable from a seed | Llama-7B 1.6M against LoRA 159.9M; GPT-3 rank 16: 2.8M (10.5 MB) against 75.5M | as LoRA at VeRA's (higher) rank | gradient steps |
| LoRA-XS [S6] | an r x r matrix between frozen SVD factors of the weight | r^2 per module, independent of width; RoBERTa-large 24.6K matches LoRA 800K (88.03 vs 87.82 GLUE); Mistral-7B 3.67M beats LoRA 168M on GSM8K (70.35 vs 67.70); 0.057M falls to 57.92 | as LoRA | gradient steps |
| (IA)^3 [S7] | three scale vectors per layer (keys, values, feed-forward) | 540K on T0-3B, 4.2 MB; training 30 min on one A100 | element-wise multiplies | gradient steps |
| Adapters [S8] | a bottleneck MLP inserted in each layer | 3.6% of parameters per task, within 0.4% of full fine-tuning on GLUE | cannot be merged: +20.7% to +30.3% latency at batch 1, length 128 on GPT-2 medium [S1] | gradient steps |
| BitFit [S9] | the bias terms | no figure in abstract | zero (a bias is a bias) | gradient steps |

All rows relayed. Points that matter for an executor:

- **Merged or unmerged is the first design choice, and the paper that introduced LoRA already names the
  trade**: merged weights give zero latency, but "it is not straightforward to batch inputs to different tasks";
  switching is "subtracting BA and then adding a different B'A'" [S1] (relayed). The official PEFT documentation
  adds a fact a content-addressed store must respect: "On a quantized model, unmerging is not exact: merging
  re-quantizes the base weight, and repeated merge and unmerge cycles can accumulate rounding error" [S12]
  (relayed). So merge-in-place is not reversible on the Q8_0 and 4-bit files this system runs.
- **A scale vector is batchable per row**: (IA)^3 notes that because each example's activations are multiplied by
  its own vector, one model serves mixed-task batches cheaply [S7] (relayed). That is the cheapest per-row variant
  switch in the literature.
- **The smallest useful matrix unit is far below LoRA's size**: LoRA-XS at 24.6K parameters equals LoRA at 800K
  on GLUE, and VeRA stores a seed plus vectors [S5][S6] (relayed). A unit of a few tens of kilobytes is realistic.
- **Bias-only and steering are the same operation** (inferred from [S9] and [S29]): adding a fixed vector to the
  residual stream at a layer is a bias on that layer's output. BitFit learns it by gradient; activation addition
  computes it from two forward passes.

## 4. Serving many adapters, switching per request and per token

- **S-LoRA** [S10] (relayed, HTML read): keeps every adapter unmerged in host memory, batches the base product
  xW across all requests in one matrix multiply and computes each request's xAB with a custom kernel over
  non-contiguous memory. 2,000 adapters on one A100-80GB at 7.61 requests/s against 8.05 at 5 adapters; up to 4x
  vLLM-packed and up to 30x PEFT. Its "unified paging" puts adapter weights and KV cache tensors in one pool. It
  gives no percentage for the unmerged overhead, only that xAB is "substantially lower" than xW.
- **Batched LoRA (FLoRA)** [S11] (relayed): each example in a minibatch carries its own low-rank weights.
- **PEFT mixed batches** [S12] (relayed): `adapter_names` per sample works for inference; the documented overhead
  is that "the batch size is effectively reduced to the number of samples per adapter"; it does not work with
  DoRA or with any merged adapter.
- **Dynamic (per-layer, per-token) adapter mixtures are slow as built**: LoRA-Switch [S19] and AdaFuse [S20]
  both report that dynamic adapters slow decoding by more than 2.5x, AdaFuse attributing it to "fragmented,
  sequential CUDA kernel launches". Both fix it the same way: decide routing once per token for all layers, merge
  the selected adapters into the backbone with one fused kernel, and decode; both report about 2.4x lower latency
  than other dynamic adapters (relayed).
- **Per-token routing without training**: token-level adaptation [S17] computes cosine similarity between the
  prompt embedding and each adapter's dataset centroid, softmaxes, and merges four adapters by those weights;
  re-routing every other token scored 48.3 against 40.0 for per-task specialists and 16.7 for the base, but on 60
  hand-graded questions per dataset, so the ordering of rows is weak evidence (relayed, HTML read).
- **MoLoRA** [S18] (relayed, abstract only): a learned per-token router; claims a 1.7B model with routed adapters
  exceeds an 8B model on four reasoning benchmarks; no latency measurement on the abstract page.
- **Composition and interference**: LoRAHub [S13] composes 20 sampled modules with coefficients found by a
  gradient-free optimizer in at most 40 steps from 5 examples: BBH 34.7 against 27.0 zero-shot, 37.3 in-context,
  37.7 LoRA tuning, at 111.6 tokens per example against 597.8 for in-context (relayed, HTML read). Mixture of
  LoRA Experts [S15] states that direct arithmetic merging "may result in the loss of the original pre-trained
  model's generative capabilities" or of each LoRA's identity (relayed). LoRA Soups [S16]: concatenating LoRAs
  with learned weights beats model-merging baselines by 43% on average on math-word skill composition (relayed).
  TIES [S47] names the two interference sources for added task vectors: redundant small values and sign
  disagreement (relayed). LoraRetriever [S14] retrieves adapters per input and composes them, with batched
  heterogeneous inference (relayed, no numbers on the abstract page).
- **Activated LoRA (aLoRA)** [S21] (relayed, HTML read) is the most important serving result for this design.
  The adapter changes weights only for tokens after an invocation point, so the base model's KV cache of
  everything before is reused as it is. For N adapters over a shared prefix of T_cache tokens generating T_new
  each, the paper gives O(N x T_cache x T_new) time against LoRA's O(N x (T_cache + T_new)^2); measured up to 35x
  faster adapter generation depending on model and prompt length, 2 to 7x even at 250-token prompts. It needs a
  higher rank (32 against 8) because it cannot rewrite earlier keys and values; accuracy difference against LoRA
  had median 0.0% over 4 models and 7 tasks. Caveat the paper states: KV produced under an aLoRA is not reusable
  by the base or by other adapters. PEFT ships it and notes it cannot be merged by definition [S12].
- **KV reuse across fine-tunes**: DroidSpeak [S22] recomputes a few layers of another same-architecture model's
  KV cache and reuses the rest: up to 4x throughput, about 3.1x faster prefill, negligible quality loss (relayed).

Unit summary: a matrix pair per adapted matrix; 10^4 to 10^8 parameters; unmerged cost well under 2% of the step
by arithmetic (section 2) but 2.5x in practice unless fused; created only by gradient steps or a hypernetwork;
composes by weighted sum (lossy), by concatenation of ranks (exact but grows), or by routing (exact, one at a
time); rolled back by not applying it.

## 5. Making a unit in one pass: hypernetworks and context-to-weights

- **Text-to-LoRA** [S25] (relayed, HTML read; ICML 2025): a hypernetwork of 5M, 34M or 55M parameters emits a
  rank-8 query/value LoRA (3.4M parameters) for Mistral-7B from an embedding of a task description. Trained on
  479 tasks. Zero-shot on 10 benchmarks: 67.7 against 55.8 for the base, 66.3 for one multi-task LoRA, 75.8 for
  per-task LoRAs (8-task average). The hypernetwork pass costs about 5e-6 TFLOPs against 0.827 for the base
  model's own pass; the text encoder costs 0.029. Limitation stated: zero-shot still trails task-specific LoRAs,
  and reconstruction-trained variants do not generalise to unseen tasks.
- **Generative Adapter** [S24] (relayed, HTML read): the generator maps the base model's own hidden states H of a
  context chunk to a weight delta W = A1 . S . B2 where S = S_prev + A2 . H^T H . B1 is a 1024 x 1024 accumulator
  per adapted layer. Updates are additive across chunks and need only forward passes. About 500M generator
  parameters, 32M adapter. StreamingQA F1 19.5 to 31.5 against supervised fine-tuning; 4x less computation and
  memory than prompting with the whole conversation; but it loses to plain short-context prompting (SQuAD at 512
  tokens: 48.8 against 64.8) and its accuracy falls as context grows.
- **Drag-and-Drop LLMs** [S26] (relayed, abstract): a few unlabeled prompts in, LoRA matrices out, "in seconds",
  claimed up to 12,000x lower overhead than fine-tuning and up to 30% better than the strongest training LoRAs on
  unseen benchmarks.
- **Doc-to-LoRA** [S23] (relayed, HTML read; ICML 2026): a 309M-parameter Perceiver-style hypernetwork turns a
  document into a rank-8 LoRA on the feed-forward down projection of gemma-2-2b in 0.209 s (batched, 11.5 GB
  extra memory) or 0.551 s (iterative, 3.8 GB), against about 40 s for oracle context distillation. Quality:
  0.857 of the in-context upper bound on 2WikiMultihopQA, 82.5% on SQuAD. Long documents are split into chunks
  and the per-chunk adapters are **concatenated along the rank dimension** (8 chunks give rank 64).
  Needle-in-a-haystack stays near-perfect to about 40K tokens on a model with an 8K window, using under 50 MB
  where the base needs over 12 GB at 128K. Costs stated: meta-training about 5 days on 8 H200 GPUs, a new
  hypernetwork per base model, an unexplained failure on Mistral-7B for long contexts.
- **Its failure mode is measured**: "The Override Gap" [S28] (relayed, abstract): when the document contradicts
  what the model already believes, accuracy falls from 65.3% (weakest-prior quartile) to 16.3% (strongest) on
  194 conflicts; scaling the adapter at the most active layers lifts deep-conflict accuracy from 46.4% to 71.0%
  with no training.
- **A hypernetwork-written adapter is nearly a linear function of an embedding** [S49] (relayed; NeurIPS 2026
  workshop): a Doc-to-LoRA update is "a near-linear image of the document embedding (held-out R^2 = 0.97),
  confined to a low-dimensional subspace"; a linear probe on the factors identifies the document at recall@5
  0.86 to 0.99, where SGD-trained adapters decode at chance. It is reported as a privacy leak. For this design it
  says something else too: the adapter carries little the embedding did not.
- **Cartridges** [S27] (relayed, abstract): a trained KV cache per corpus, made offline by "self-study"
  (synthetic conversations about the corpus, context-distillation loss). Matches in-context quality at 38.6x
  less memory and 26.4x higher throughput, extends effective context from 128k to 484k tokens on one benchmark,
  and Cartridges "can be composed at inference time without retraining". Plain next-token training of the
  cache did not work.
- **Why context can become weights at all**: "Learning without training" [S50] (relayed, abstract) argues that a
  self-attention layer followed by an MLP makes a forward pass with context equal to a pass without it whose MLP
  weights carry "a minimal low-rank update representing the context". "In-Context Learning Creates Task
  Vectors" [S32] finds demonstrations compressed into one vector that then modulates the model (relayed).

### 5.5 One vector as the inline form of a node

- **xRAG** [S51] (relayed): a retrieved document enters as **one token embedding** through a trained projector,
  retriever and model frozen; over 10% average gain on six tasks, 3.53x fewer FLOPs than text retrieval.
- **ConceptFormer** [S52] (relayed): knowledge-graph nodes mapped to concept vectors in the input embedding space
  of a frozen GPT-2; one vector per node beats text-serialised graph retrieval with 130x fewer input tokens.
- **TokMem** [S53] (relayed; ICLR 2026): each procedure compiled into one trainable memory token for a frozen
  backbone, 1,000 tasks, new procedures added "without interfering with existing ones".
- **ToolkenGPT** [S54] (relayed): each tool is an extra output token with a learned embedding; predicting it
  triggers the call; tools are added by adding rows.
- **Tool choice is one linear direction** [S55] (relayed; ICML 2026 workshop, 12 models from 270M to 27B): the
  causal effect of tool selection "concentrates along the output-layer row that produces the target tool's first
  token"; a unit vector along that row switches the chosen tool with 93-100% accuracy at 4B and above; the
  arguments generated afterwards follow the new tool's schema; even base models encode the right tool before
  they can emit it (cosine readout 69-82% where generation gets 2-10%); a small top-1/top-2 gap predicts 14-21x
  more wrong calls. Limit stated: single turn, fixed menu.
- **Prometheus Mind** [S56] (relayed, one author, small test sets): for injecting a remembered value into a
  frozen 4B model "the lm_head weight rows already provide the needed mapping", no training; end-to-end training
  of its adapters collapsed; retrieval 94.4% on clean inputs (n=54) and 19.4% on informal ones (n=36).

## 6. Intervening on representations: vectors, subspaces, features

### 6.1 Steering vectors

- **Activation addition** [S29] (relayed): the vector is the difference of intermediate activations for one pair
  of prompts; no optimisation.
- **Contrastive activation addition** [S30] (relayed): the mean difference of residual activations over many
  positive/negative pairs, added at every position after the prompt with a signed coefficient.
- **Function vectors** [S31] (relayed): a small number of attention heads carry a compact task representation;
  inserted elsewhere it triggers the task; vectors "can, to some extent, be summed".
- **In-context vectors** [S33] (relayed): one forward pass over demonstrations gives a vector that replaces the
  demonstrations; strength is the magnitude; instruction types combine by vector arithmetic.
- **Representation engineering** [S34], **persona vectors** [S35] (relayed): population-level directions for
  traits; used to monitor, steer, and flag training data.
- **Task arithmetic** [S46] (relayed) is the same idea in weight space: fine-tuned minus base weights; negate to
  remove, add to combine.

Reliability, measured:

- Steering has a bounded usable range. Anthropic's study on 29 SAE features of Claude 3 Sonnet found a "sweet
  spot" of steering factors from -5 to 5 for all of them, outside which capability "drops sharply", and found
  off-target effects (a gender-bias feature moved age-bias scores by about 13%) [S36] (relayed).
- "Steerability is highly variable across different inputs" and for several concepts vectors "are brittle to
  reasonable changes in the prompt" [S37] (relayed).
- AxBench [S39]: for steering, "prompting outperforms all existing methods, followed by finetuning"; SAEs "are not
  competitive"; difference-in-means is best for detection (relayed). A May 2026 reply [S40] reports SAE features
  chosen by a supervised pipeline come "close to on par" with the LoRA reference (relayed, abstract only).
- FaithSteer-BENCH [S41] (relayed, abstract, March 2026): at one fixed operating point, methods show "illusory
  controllability", a measurable "cognitive tax" on unrelated capability, and brittleness under mild
  perturbation.

Unit: one vector of width d at one layer (20 KB at d = 5120 in float32); cost d adds; created by forward passes
only; composes by addition within a narrow norm budget; rolled back by not adding it.

### 6.2 ReFT

LoReFT [S42] (relayed, HTML read) edits the hidden state at a few chosen positions: h + R^T(Wh + b - Rh), with R
of r orthonormal rows. 0.031% of parameters against LoRA's 0.826% on LLaMA-7B, with 80.2 against 74.7 on
commonsense tasks but 42.6 against 46.9 on arithmetic. Because the number of edited positions is fixed it adds
"a fixed additional inference cost that does not scale with prompt length". It is trained by gradient. Its
rank-1 form (ReFT-r1) is AxBench's competitive steering method [S39].

### 6.3 Sparse-autoencoder features and transcoders

- A dictionary learned on activations gives features that can be amplified or suppressed; "millions of features"
  from one middle layer of Claude 3 Sonnet; the Golden Gate Bridge demonstration; stated limits: the features
  found are "a small subset", a full set would cost more compute than training the model, and knowing a feature
  does not give the circuit [S57] (relayed).
- Gemma Scope [S58]: open JumpReLU SAEs on every layer and sub-layer of Gemma 2 2B and 9B (relayed).
- Transcoders [S59] approximate an MLP sublayer by a wider sparse one, on par with SAEs for sparsity,
  faithfulness and interpretability, and give input-invariant circuit terms (relayed).
- Cross-layer transcoders and attribution graphs [S60] (relayed): up to 30M features for Claude 3.5 Haiku; the
  largest 18-layer replacement model reproduces the original's next token on 50% of evaluated prompts; the rest
  is carried by error nodes; attention patterns are frozen, so how attention forms is not explained.
- Limits: only about 30% of features are shared between two SAEs trained with different seeds (Llama 3 8B, 131K
  latents) [S38]; features are neither complete nor atomic, and what is found depends on dictionary size
  [S61] (both relayed).

Unit: a decoder direction of width d plus an encoder row; reading every feature at a layer costs one N x d
product (2.5% of a token at N = 131K, section 2); writing one is a vector add; the dictionary costs a large
training run per model and layer; identity of a feature **index** does not survive retraining.

## 7. Editing, test-time training, attached memories, side networks

### 7.1 Model editing

| Method | Unit | Created by | Record |
|---|---|---|---|
| ROME [S62] | a rank-one update of one mid-layer feed-forward matrix | an optimised value vector plus a closed-form insert (assumed for the detail) | one edit can already disable a model: a "disabling edit" has a normalised weight distance of 3.3e-4 against 8.2e-7 for normal edits; collapse between about 100 and 1,000 sequential edits [S45] |
| MEMIT [S63] | a batch update across several layers | as ROME, for thousands of facts at once | gradual forgetting, then collapse near 1,400 edits (GPT-J) or 4,000 (GPT2-XL) when applied sequentially [S45] |
| MEND [S64] | a low-rank transform of the fine-tuning gradient by small editor networks | one forward and backward pass after training the editor (under a day on one GPU) | not assessed here |
| GRACE [S43] | a codebook row at one layer: key (cached activation), value (learned vector), radius | key is free; value takes 100 gradient steps | 1,000 edits in 137 keys; edits retained 0.96, unrelated retained 0.69 to 1.00; 1.32x slower at 4,500 entries; generalises poorly to rephrasings (0.08 in [S44]'s table) |
| WISE [S44] | a side copy of one feed-forward matrix, routed by an activation threshold | gradient steps into the side memory | at 1,000 sequential edits on LLaMA-2-7B: reliability 0.77, generalisation 0.72, locality 1.00, where ROME scores 0.01 and MEMIT 0.04; +3% latency, +0.64% parameters |
| AlphaEdit [S65] | an edit projected onto the null space of preserved knowledge | one extra projection | +36.7% average for locate-then-edit methods |

All rows relayed (HTML read for [S43], [S44], [S45]). "Even a single edit can trigger model collapse", and
perplexity of the edited model tracks its downstream performance and can stand in for a benchmark run [S66]
(relayed).

### 7.2 Test-time training and fast weights

- Test-time training [S67] (relayed, HTML read): a rank-128 LoRA trained per task on leave-one-out variants of
  the task's own examples, 2 epochs; ARC accuracy up to 6x over fine-tuned baselines, 53.0% with an 8B model;
  about 12 hours on one A100 for 100 tasks. So: minutes per task, not a token's budget.
- SEAL [S68] (relayed, HTML read): the model writes its own fine-tuning data ("self-edits"), a LoRA is trained on
  it, and reinforcement learning rewards edits that help: no-context SQuAD 33.5% to 47.0%; each self-edit
  evaluation takes 30 to 45 s; the paper states it "is still susceptible to catastrophic forgetting".
- Fast weights [S69]: a third kind of variable, slower than activity and faster than weights, holding the recent
  past (relayed). Titans [S70]: a neural long-term memory that learns at test time, scaling past 2M tokens of
  context (relayed, abstract).

### 7.3 Memories attached beside the model

- kNN-LM [S71] (relayed, HTML read): a datastore of (hidden state, next token) rows, 103M rows of width 1024,
  mixed into the output as lambda x p_kNN + (1 - lambda) x p_LM; perplexity 15.79 on Wikitext-103, 2.9 better,
  with no training; a model trained on 100M tokens with a 3B-token datastore (13.73) beats the model trained on
  all 3B (15.17): "retrieving nearest neighbors from the corpus outperforms training on it".
- Memorizing Transformers [S72]: approximate kNN into a non-differentiable memory of recent (key, value) pairs,
  improving up to 262K tokens of memory; uses functions and theorems defined only at test time (relayed).
- Larimar [S73]: an episodic memory giving one-shot updates without training, 8-10x faster than editing
  baselines, with selective forgetting (relayed).

### 7.4 Side networks

Ladder side-tuning [S74] (relayed): a small side network reads the backbone's intermediate activations through
shortcuts; training does not backpropagate through the backbone, saving 69% of training memory against 26% for
other methods. The backbone's forward pass is untouched, so its KV cache stays valid for every side network
(inferred).

## 8. Comparing variants cheaply, and dreaming

### 8.1 Low-cost A/B

- **A second run mixed at the logits.** Classifier-free guidance [S75] (relayed, HTML read): logits =
  unconditional + gamma x (conditional - unconditional), two forward passes per step; in 5 of 9 tasks it matched a
  model of twice the size; LLaMA-7B on LAMBADA 73.6% to 81.3% at gamma 1.5; gamma 2 hurts code; a negative prompt
  replaces the unconditional run. Contrastive decoding [S76]: maximise expert minus amateur log-likelihood under
  a plausibility constraint, no training (relayed). DoLa [S77]: contrast late-layer against early-layer logits
  of the **same** pass, +12-17 points on TruthfulQA, no second model (relayed).
- **Shared prefix, forked suffix.** llama.cpp exposes `llama_memory_seq_cp(mem, seq_src, seq_dst, p0, p1)`,
  "Copy all tokens that belong to the specified sequence to another sequence", alongside runtime LoRA
  (`llama_set_adapters_lora` with a scale per adapter) and control vectors (`llama_set_adapter_cvec` over an
  inclusive layer range) [S78] (relayed). Prompt Cache [S79]: precomputed attention states for reusable prompt
  modules, 8x (GPU) to 60x (CPU) lower time to first token (relayed). aLoRA gives the complexity result for N
  variants over one prefix (section 4).
- **Verify several continuations in one batch.** Speculative decoding [S80]: 2-3x with an unchanged output
  distribution (relayed). The old body measured its own version: 8 verification rows for 2.24 token-times [L4].
- **Weight interpolation.** Model soups [S81]: averaging fine-tuned weights improves accuracy at no inference
  cost (relayed). It is a merge, with the merge caveats of section 4.

### 8.2 Dreaming: self-generated data for consolidation

Evidence of collapse:

- Training on model-generated content causes "irreversible defects"; the tails of the distribution disappear
  [S82] (relayed).
- Collapse comes from **replacing** real data: accumulating each generation's synthetic data beside the original
  real data avoids it, with a proved bound only for linear models and empirical support for language models,
  diffusion and VAEs [S83] (relayed).
- SEAL forgets earlier passages as self-edits accumulate [S68]; in-place editing collapses (7.1).

Evidence of benefit:

- Deep generative replay [S84]: a generator replays earlier tasks' data beside new data (relayed).
- Self-synthesised rehearsal [S85]: the LLM generates its own rehearsal instances; "superior or comparable" to
  conventional rehearsal (relayed, no figures on the abstract page).
- Consolidation as a phase: up to 55% fewer replay samples for the same performance [S86] (relayed).
- Cartridges' self-study is dreaming with a witness: synthetic conversations about a corpus, distilled into a
  KV cache that matches in-context quality [S27].
- Nightly consolidation into a LoRA on one consumer GPU [S87] (relayed, abstract): 80.4 +/- 1.3% of knowledge
  retained against 36.8 +/- 3.0% for three cycles of cascading context compaction (floor 11.8%, ceiling 90.1%),
  n = 10 conversations, 1,146 questions. A measurement note worth keeping: **median** per-token validation
  cross-entropy tracks judged accuracy (r = +0.99) while the mean anti-correlates (r = -0.51).
- Cortex [S88] (relayed, abstract, October 2026): prospective queries anticipate how a context will be used; the
  context-conditioned outputs are distilled into mergeable low-rank or structural updates; after 50 sequential
  edits on a 14B model 88.9% of the context's gain is kept and 99.8% of non-target performance.
- "Language Models Need Sleep" [S89] (relayed, abstract, June 2026): consolidation by distillation into a larger
  network plus a "Dreaming" stage where reinforcement learning generates a synthetic curriculum. The abstract
  carries no numbers and no forgetting measurement.
- Sleep-replay consolidation in small networks (an offline unsupervised phase with local plasticity reducing
  forgetting) is known to me from memory only; the journal page was behind a login today (assumed).

Reading (inferred): dreaming helps when (a) the output of the dream is a **new side unit** rather than a change
to the base, (b) the objective is distillation of what the model does **with** the context rather than next-token
prediction on the raw text (Cartridges' and Doc-to-LoRA's shared finding), (c) real data stays in the mix, and
(d) acceptance is decided by a held-out witness of real rows.

## 9. Lessons for a system where every unit is an immutable content-addressed node

Each lesson is inferred; the evidence is named.

1. **The base model is ice and is never written.** In-place edits fail at scale ([S45], [S66]); unmerge is
   inexact on quantized weights [S12]. Every capability unit is its own cell. Rollback is then the design's
   existing rule, repointing a name (1.7.8), with no numeric residue.

2. **A unit's blueprint must name what it plugs into**: base model content id, hook point (layer, sublayer,
   position rule), dtype and shape. A Doc-to-LoRA hypernetwork works for one base model [S23]; an SAE feature for
   one model and layer; a KV block for one model and one position rule. The unit's content id then covers its
   compatibility, and offering it to another model is answered 0 with a reason rather than applied.

3. **Unmerged is the stored truth; merged is a materialization.** Merged weights, a fused "all adapters for this
   token" kernel [S19][S20], and a cached KV block are derived from (base id, unit ids, target) exactly as
   compiled code is derived from (recipe id, generator id, target id) in 5.10.2, and belong in the same cache
   class (5.6.4: gas, optionally water). They are never ice.

4. **One routing decision per token, one fused kernel.** The 2.5x penalty of dynamic adapters is dispatch
   overhead [S20]. On Metal with over a thousand dispatches per token already [L2], per-layer conditional
   adapters would be the wrong shape. The executor should resolve the token's active unit set once and run a
   pipeline specialised for that set.

5. **Prefer units that leave the prefix's KV valid.** aLoRA [S21], side memories and codebooks that fire only on
   their keys [S43][S44], side networks [S74], steering applied from a position onward [S30], and logit mixes
   [S75] all let N variants share one prefix. A classic all-token LoRA does not. This is the property that makes
   A/B cost N rows instead of N prefills.

6. **A/B is a batch.** Fork the sequence (KV copy or shared pages), give each row its own unit set, decode the
   rows together, compare. The choice-point frame and trail of 5.4.9 and the region rule of 5.7.2 are the same
   mechanism the KV fork needs: a rejected branch's rows are released by moving a pointer back. Per-row scale
   vectors [S7] and per-row low-rank terms [S10][S11] are the published kernels.

7. **Cheap creation comes from activations already computed.** A steering vector (two rows subtracted), a
   codebook key (one cached activation), a node's output row (section 5.5), a KV block (the prefill itself): each
   is minted by a copy and a freeze, at no extra forward pass. By 5.2.3 the content id is computed only at
   freeze, so a candidate that is tried and dropped costs no hash.

8. **Identity is the bytes of the direction, never a dictionary index.** Feature indices do not survive
   retraining [S38][S61]. A feature cell is (model id, layer, the vector); "feature 4,711 of dictionary D" is a
   name, and names are 0..many (1.7.2).

9. **Addition has a budget; concatenation and routing do not.** Summed LoRAs and summed steering vectors
   interfere ([S15], [S47], [S36]). Rank concatenation [S23], additive accumulators designed for it [S24], and
   keyed routing [S43][S44] compose without loss until memory runs out. The store should record, for each unit
   kind, which composition is exact.

10. **Dreams mint candidates; a witness accepts them.** Consolidation writes a new cell from distillation
    ([S27], [S87], [S88]); the candidate is offered, compared in an A/B batch on held-out **real** rows, and a
    name is repointed or not. Lineage is a chain of immutable cells, so collapse is visible as a failed witness
    and undone by repointing. The witness should include a planted-fault control (5.11.1): a random direction or
    a shuffled adapter of the same norm must fail to pass.

11. **Cheap health checks exist**: perplexity after a change tracks downstream damage [S66]; median per-token
    cross-entropy tracks judged accuracy where the mean misleads [S87]; the top-1/top-2 margin of a node choice
    predicts wrong calls [S55]. Each is a by-product of rows already being decoded.

12. **Honest ceiling**: for steering behaviour, a prompt still beats every representation method measured
    [S39]. A prompt whose KV is cached (a KV cell) is therefore the baseline every new unit must beat on quality
    per millisecond, and the first unit kind worth building.

## 10. Summary table of units

| Unit | Size | Step cost vs base | One pass? | Composes by | Evaluate / roll back |
|---|---|---|---|---|---|
| LoRA pair | 10^4-10^8 params | <2% arithmetic; 2.5x if unfused | no (gradient), or yes via hypernetwork | weighted sum (lossy), rank concat, routing | A/B rows; drop the cell |
| Scale vectors ((IA)^3, VeRA) | 10^5-10^6 | ~0 | no | per-row selection | same |
| Bias / steering vector | d floats per layer | ~0 | yes (2 forward rows) | addition within a norm budget | margin and capability check; stop adding |
| ReFT edit | ~10^4-10^6 | fixed, independent of length | no | by position sets | same |
| SAE / transcoder feature | d floats (+ dictionary) | read all: 2.5%/layer; write: ~0 | no (dictionary training) | addition, narrow range | not stable across trainings |
| Rank-one weight edit | one matrix delta | 0 if merged | partly | fails sequentially | perplexity; keep delta to subtract |
| Codebook row (GRACE) | 2d+1 floats | N x d lookup | key yes, value no | independent rows | delete the row |
| Side memory (WISE) | one FFN matrix copy | +3% | no | shards merged | discard the side memory |
| kNN datastore row | d floats + token | N x d or an index | yes | independent rows | delete rows |
| One-token node embedding | d floats | one more position | via trained projector | more positions | remove the position |
| KV block (prompt cache, Cartridge) | 2 x layers x heads x K x head-dim | ~1% per 500 tokens | yes (prefill); Cartridge no | concatenation | drop the block |
| Logit mix (CFG, contrast) | no stored unit | one more row (~18% here) | yes | weighted sum of rows | set gamma to 1 |

All sizes and costs: section 2 arithmetic (inferred) or the cited source (relayed).

## 11. Constructions that could be new, each with a first experiment

For each I state what is borrowed, what I did not find in the sources fetched today (which is weaker than "nobody
has done it"), and an experiment sized for the 3B model on this machine (20.7 ms/token burst [L5]).

**N1. A node head beside the vocabulary head.** The output step scores the final hidden state against a second
matrix whose rows are node cells, and the sampler picks a token or a node id; a picked node is offered (run,
read, attach). Borrowed: toolkens [S54], memory tokens [S53], and the finding that tool choice is one direction
along an output-layer row [S55]. New: rows are content-addressed cells created without training, by one of two
one-pass recipes: (a) the mean final hidden state at the position where the model, shown the node's canonical
print and a call site, is about to name it; (b) the mean of the output-layer rows of the node's name tokens.
Cost: 2.9% of a step per 152k nodes (section 2). *Experiment*: 200 recipe nodes with one-line descriptions; 20
held-out prompts each; measure top-1 node accuracy, the false-fire rate on plain prose, and added ms/token, for
(a), (b) and trained toolken rows as the reference. It fails if (a) and (b) are under half the trained rows'
accuracy or fire on more than 1% of prose tokens. Use the top-1/top-2 margin [S55] as the hold signal: a small
margin acknowledges nothing rather than guessing.

**N2. KV cells: a node's inline presence is its cached attention state.** Each knowledge node has a KV block
computed once in isolation, stored under (model id, node id, position rule); reading the node inline splices the
block. Borrowed: Prompt Cache [S79], Cartridges [S27]. New here: blocks keyed by content id, shared by reference
across branches, relocated by rotating keys when the model uses rotary positions (assumed exact because an
isolated block depends only on its internal offsets; this is the thing to test). *Experiment*: 50 nodes of about
500 tokens; answer 10 questions each under four conditions: text in context, spliced KV at three different
offsets, one-vector injection (N5), no context. Report accuracy and ms/token; the relocation claim fails if
spliced accuracy differs across offsets by more than run-to-run noise.

**N3. Every A/B run mints a steering candidate.** When two branches over one prefix are judged (one accepted, one
not), the difference of their residuals at a middle layer, averaged over judged pairs, is a contrastive vector
[S30] obtained at zero extra forward cost. The system's acknowledgments (1 against 0, 5.4.2) become the labels;
trust formed by observed consequence (1.7.4) becomes a direction. I did not find outcome-labelled harvesting from
live branching in today's sources. *Experiment*: 300 judged branch pairs from one task family; build the vector
from 200; on the other 100 measure the lift in acceptance when it is added at three strengths, the capability
cost (perplexity on a fixed text), and the same for a random vector of equal norm. It fails if the lift does not
exceed the random control or if perplexity rises more than the lift is worth.

**N4. Trigger rows: a codebook whose value is a node to offer.** GRACE's key is a cached activation and its value
a trained vector [S43]. Replace the value by a node id: when the residual at layer L falls within a radius of a
key, the executor offers that node (a native flow) at that position. Creation needs no gradient: the key is
copied from the forward pass in which a person or a flow marked "here". Cost N x d per token (0.09% at 4,500
keys). *Experiment*: 100 situations (for example "an arithmetic expression is about to be evaluated"), 5 marked
examples each; sweep the radius; report precision and recall of firing on held-out text against N1's output-side
selection. GRACE's known weakness, poor generalisation to rephrasings [S44], is the expected failure.

**N5. An adapter is a materialization of an embedding.** Since a hypernetwork-written adapter is a near-linear
image of the document embedding (R^2 0.97) [S49], store the embedding as the cell (a few kilobytes, ice) and
derive the low-rank factors on demand through a fixed linear generator per model and layer (gas, cached). Sums of
embeddings then give a composed adapter in one generator pass. *Experiment*: with the public Doc-to-LoRA code,
fit a linear map from document embeddings to its adapters on 2,000 documents; on 200 held-out documents compare
QA accuracy of linear-generated against hypernetwork-generated adapters; then test whether the adapter of a
summed embedding matches the rank-concatenated pair. It fails if the linear adapters lose more than a few points,
or if summed embeddings interfere where concatenation does not. Carry the override gap [S28] as a known limit:
this unit kind is weak where the node contradicts the base model.

**N6. Guidance between cells.** Apply the classifier-free guidance formula to two rows that differ by one cell:
logits = without + gamma x (with - without). Any unit kind (adapter, KV cell, steering vector) gets a continuous
strength and a negation without touching its content, and gamma = 0 is an exact off. Borrowed: [S75], [S76]. Cost:
one extra row. *Experiment*: one rank-8 adapter and one KV cell; sweep gamma over 0, 0.5, 1, 1.5, 2, 3; plot task
score and perplexity; compare with scaling the adapter's weights by the same factor. Expect the usable range to
be narrow (gamma 2 already hurt code in [S75]).

**N7. Invocation-scoped units as the default binding.** Generalise aLoRA [S21]: every unit kind takes effect only
from the position where it was accepted, so the KV of all earlier positions stays the base model's and stays
shared. Combined with N1, the accepted node id itself is the invocation point. *Experiment*: one adapter trained
both ways (all tokens, from-invocation at rank 32); measure the time to switch among 8 variants over a 2,000-token
prefix and the accuracy difference. The paper's own numbers predict up to 35x on switching and a median accuracy
difference of zero.

**N8. Dream to a candidate, accept by witness.** A nightly pass over the day's contexts generates prospective
queries [S88], answers them with the context in view, and distils the answers into a new unit (a KV cell by
self-study [S27], or a low-rank adapter [S87]); the unit is offered and accepted only if an A/B batch on held-out
real rows passes, including a planted-fault control. Units accumulate as a chain; a periodic re-dream distils the
chain into one unit, again as a candidate. *Experiment*: 7 simulated days of project conversation; after each
night measure retained knowledge (held-out questions), drift on a fixed capability set, and median per-token
cross-entropy [S87], for three policies: stack units, re-dream weekly, cascade-compact the context. It fails if
stacking beyond a few units degrades the fixed set (the interference of section 4) with no policy recovering it.

**N9. Speculative native execution of a picked node.** When the node head's margin is high, start the recipe on
the CPU while the GPU decodes the next token; if the model then confirms the call, the result is already there
and enters as one embedding or a short KV block; if not, the work is dropped (it was gas). Borrowed: the
accept/verify shape of speculative decoding [S80]. *Experiment*: an arithmetic and a lookup flow; compare
end-to-end tokens and wall time against text-form tool calling; count wasted speculative runs as the cost.

## 12. Sources

Local (observed today unless marked):

- [L0] `coherence-seed/graph/0.kg` lines 1-61; `coherence-seed/graph/5.kg`
  through `./kg brief` (hub 5 lines). Dated 2026-11-24 in the files.
- [L1] `memory/reference-native-lora-teaching-lane.md`
  (note modified 2026-09-15). I read the note; its measurements are relayed.
- [L2] `.../memory/project-qwen-speed-vs-llamacpp-2026-10-04.md` (modified 2026-10-07). Relayed measurements.
- [L3] `.../memory/project-dflash2-drafter-2026-10-07.md` (modified 2026-10-07): lane width 5120, gate 17408 x 5120,
  "1 fma per weight per column". Relayed.
- [L4] same note: "8-row check 235 ms vs 105 ms token", "Per ROW of the check span ~18% of a decode token". Relayed.
- [L5] `.../memory/reference-decode-submit-per-layer-and-reading-variance.md` (modified 2026-10-13). Relayed.

Web (all fetched 2026-11-24; status relayed; date is the source's first submission or publication):

- [S1] LoRA. https://arxiv.org/abs/2106.09685 and https://arxiv.org/html/2106.09685v2 (2021-06-17)
- [S2] QLoRA. https://arxiv.org/abs/2305.14314 (2023-05-23)
- [S3] DoRA. https://arxiv.org/abs/2402.09353 (2024-02-14)
- [S4] rsLoRA. https://arxiv.org/abs/2312.03732 (2023-11-28)
- [S5] VeRA. https://arxiv.org/abs/2310.11454 and https://arxiv.org/html/2310.11454v2 (2023-10-17)
- [S6] LoRA-XS. https://arxiv.org/abs/2405.17604 and https://arxiv.org/html/2405.17604v3 (2024-05-27)
- [S7] (IA)^3 / T-Few. https://arxiv.org/abs/2205.05638 and https://arxiv.org/html/2205.05638v2 (2022-05-11)
- [S8] Adapters. https://arxiv.org/abs/1902.00751 (2019-02-02)
- [S9] BitFit. https://arxiv.org/abs/2106.10199 (2021-06-18)
- [S10] S-LoRA. https://arxiv.org/abs/2311.03285 and https://arxiv.org/html/2311.03285v3 (2023-11-06)
- [S11] Batched LoRA (FLoRA). https://arxiv.org/abs/2312.05677 (2023-12-09)
- [S12] Hugging Face PEFT, LoRA developer guide. https://huggingface.co/docs/peft/developer_guides/lora (living page, read 2026-11-24)
- [S13] LoRAHub. https://arxiv.org/abs/2307.13269 and https://arxiv.org/html/2307.13269v3 (2023-07-25)
- [S14] LoraRetriever. https://arxiv.org/abs/2402.09997 (2024-02-15)
- [S15] Mixture of LoRA Experts. https://arxiv.org/abs/2404.13628 (2024-04-21)
- [S16] LoRA Soups. https://arxiv.org/abs/2410.13025 (2024-10-16)
- [S17] Token-level adaptation of LoRA adapters. https://arxiv.org/abs/2311.10847 and https://arxiv.org/html/2311.10847v2 (2023-11-17)
- [S18] MoLoRA. https://arxiv.org/abs/2603.15965 (2026-03-16)
- [S19] LoRA-Switch. https://arxiv.org/abs/2405.17741 (2024-05-28)
- [S20] AdaFuse. https://arxiv.org/abs/2603.11873 (2026-03-12; AAAI 2026)
- [S21] Activated LoRA. https://arxiv.org/abs/2504.12397 and https://arxiv.org/html/2504.12397v5 (2025-04-16)
- [S22] DroidSpeak. https://arxiv.org/abs/2411.02820 (2024-11-05)
- [S23] Doc-to-LoRA. https://arxiv.org/abs/2602.15902 and https://arxiv.org/html/2602.15902 (2026-02-13; ICML 2026 per search result)
- [S24] Generative Adapter. https://arxiv.org/abs/2411.05877 and https://arxiv.org/html/2411.05877 (2024-11-08)
- [S25] Text-to-LoRA. https://arxiv.org/abs/2506.06105 and https://arxiv.org/html/2506.06105v2 (2025-06-06; ICML 2025)
- [S26] Drag-and-Drop LLMs. https://arxiv.org/abs/2506.16406 (2025-06-19)
- [S27] Cartridges. https://arxiv.org/abs/2506.06266 (2025-06-06)
- [S28] The Override Gap. https://arxiv.org/abs/2604.23750 (2026-04-26)
- [S29] Activation addition. https://arxiv.org/abs/2308.10248 (2023-08-20)
- [S30] Contrastive activation addition. https://arxiv.org/abs/2312.06681 (2023-12-09)
- [S31] Function vectors. https://arxiv.org/abs/2310.15213 (2023-10-23)
- [S32] In-context learning creates task vectors. https://arxiv.org/abs/2310.15916 (2023-10-24)
- [S33] In-context vectors. https://arxiv.org/abs/2311.06668 (2023-11-11)
- [S34] Representation engineering. https://arxiv.org/abs/2310.01405 (2023-10-02)
- [S35] Persona vectors. https://arxiv.org/abs/2507.21509 (2025-07-29)
- [S36] Anthropic, Evaluating feature steering. https://www.anthropic.com/research/evaluating-feature-steering (2024-10-25)
- [S37] Generalization and reliability of steering vectors. https://arxiv.org/abs/2407.12404 (2024-07-17)
- [S38] SAEs trained on the same data learn different features. https://arxiv.org/abs/2501.16615 (2025-01-28)
- [S39] AxBench. https://arxiv.org/abs/2501.17148 (2025-01-28)
- [S40] SAEs can outperform simple baselines. https://arxiv.org/abs/2605.31183 (2026-05-29)
- [S41] FaithSteer-BENCH. https://arxiv.org/abs/2603.18329 (2026-03-18)
- [S42] ReFT. https://arxiv.org/abs/2404.03592 and https://arxiv.org/html/2404.03592v3 (2024-04-04)
- [S43] GRACE. https://arxiv.org/abs/2211.11031 and https://arxiv.org/html/2211.11031v5 (2022-11-20)
- [S44] WISE. https://arxiv.org/abs/2405.14768 and https://arxiv.org/html/2405.14768v3 (2024-05-23)
- [S45] Model editing at scale leads to gradual and catastrophic forgetting. https://arxiv.org/abs/2401.07453 and https://arxiv.org/html/2401.07453v4 (2024-01-15)
- [S46] Task arithmetic. https://arxiv.org/abs/2212.04089 (2022-12-08)
- [S47] TIES-Merging. https://arxiv.org/abs/2306.01708 (2023-06-02)
- [S49] Reading documents out of weight updates. https://neurips.cc/virtual/2026/160164 (NeurIPS 2026 workshop poster; no day given)
- [S50] Learning without training. https://arxiv.org/abs/2507.16003 (2025-07-21)
- [S51] xRAG. https://arxiv.org/abs/2405.13792 (2024-05-22)
- [S52] ConceptFormer. https://arxiv.org/abs/2504.07624 (2025-04-10)
- [S53] TokMem. https://arxiv.org/abs/2510.00444 (2025-10-01; ICLR 2026)
- [S54] ToolkenGPT. https://arxiv.org/abs/2305.11554 (2023-05-19)
- [S55] Tool calling is linearly readable and steerable. https://icml.cc/virtual/2026/77967 (ICML 2026 workshop oral; no day given)
- [S56] Prometheus Mind. https://arxiv.org/abs/2601.15324 (2026-01-18)
- [S57] Anthropic, Mapping the mind of a large language model. https://www.anthropic.com/research/mapping-mind-language-model (2024-05-21). The full paper page exceeded the fetch size limit.
- [S58] Gemma Scope. https://arxiv.org/abs/2408.05147 (2024-08-09)
- [S59] Transcoders. https://arxiv.org/abs/2406.11944 (2024-06-17)
- [S60] Circuit tracing methods. https://transformer-circuits.pub/2025/attribution-graphs/methods.html (2025-03-27; first 100,000 characters read)
- [S61] SAEs do not find canonical units. https://arxiv.org/abs/2502.04878 (2025-02-07)
- [S62] ROME. https://arxiv.org/abs/2202.05262 (2022-02-10)
- [S63] MEMIT. https://arxiv.org/abs/2210.07229 (2022-10-13)
- [S64] MEND. https://arxiv.org/abs/2110.11309 (2021-10-21)
- [S65] AlphaEdit. https://arxiv.org/abs/2410.02355 (2024-10-03)
- [S66] The butterfly effect of model editing. https://arxiv.org/abs/2402.09656 (2024-02-15)
- [S67] Test-time training for few-shot learning. https://arxiv.org/abs/2411.07279 and https://arxiv.org/html/2411.07279v2 (2024-11-11)
- [S68] SEAL. https://arxiv.org/abs/2506.10943 and https://arxiv.org/html/2506.10943v2 (2025-06-12)
- [S69] Fast weights. https://arxiv.org/abs/1610.06258 (2016-10-20)
- [S70] Titans. https://arxiv.org/abs/2501.00663 (2024-12-31)
- [S71] kNN-LM. https://arxiv.org/abs/1911.00172 and https://arxiv.org/html/1911.00172v2 (2019-11-01)
- [S72] Memorizing Transformers. https://arxiv.org/abs/2203.08913 (2022-03-16)
- [S73] Larimar. https://arxiv.org/abs/2403.11901 (2024-03-18)
- [S74] Ladder side-tuning. https://arxiv.org/abs/2206.06522 (2022-06-13)
- [S75] Classifier-free guidance for language models. https://arxiv.org/abs/2306.17806 and https://arxiv.org/html/2306.17806 (2023-06-30)
- [S76] Contrastive decoding. https://arxiv.org/abs/2210.15097 (2022-10-27)
- [S77] DoLa. https://arxiv.org/abs/2309.03883 (2023-09-07)
- [S78] llama.cpp public header. https://raw.githubusercontent.com/ggml-org/llama.cpp/master/include/llama.h (master as of 2026-11-24)
- [S79] Prompt Cache. https://arxiv.org/abs/2311.04934 (2023-11-07)
- [S80] Speculative decoding. https://arxiv.org/abs/2211.17192 (2022-11-30)
- [S81] Model soups. https://arxiv.org/abs/2203.05482 (2022-03-10)
- [S82] The curse of recursion. https://arxiv.org/abs/2305.17493 (2023-05-27)
- [S83] Is model collapse inevitable? https://arxiv.org/abs/2404.01413 (2024-04-01)
- [S84] Deep generative replay. https://arxiv.org/abs/1705.08690 (2017-05-24)
- [S85] Self-synthesized rehearsal. https://arxiv.org/abs/2403.01244 (2024-03-02)
- [S86] Scalable strategies for continual learning with replay. https://arxiv.org/abs/2505.12512 (2025-05-18)
- [S87] Weight-based consolidation against cascading compaction. https://arxiv.org/abs/2605.24657 (2026-05-23)
- [S88] Cortex. https://arxiv.org/abs/2610.20399 (2026-10-14)
- [S89] Language models need sleep. https://arxiv.org/abs/2606.03979 (2026-06-02)

Not reached today: the DeepMind note on negative results for SAEs (HTTP 403 and a DNS failure on the mirror); the
two Nature pages on model collapse and sleep replay (login redirect); the full "Scaling Monosemanticity" page
(over the size limit). Nothing in this report rests on them.

## 13. What stayed uncertain

- Every web number passed through a summarising fetch. The ones I would re-read in the PDF before building on
  them: aLoRA's speedups [S21], Doc-to-LoRA's latency and memory table [S23], the R^2 = 0.97 linearity [S49], and
  the 77-100% tool switching [S55] (both of the last two are workshop papers).
- The cost table in section 2 is arithmetic over three numbers from a session note and two assumptions (64
  layers, a 152k vocabulary). No kernel was run today.
- Novelty of N1 to N9 is "not found in what I fetched", nothing stronger.
- I found no measurement of any of these units on Apple Metal. The dispatch-overhead finding [S20] is from CUDA;
  whether Metal's per-dispatch cost makes it better or worse is open.
