# Token or node from one step: what the literature shows

Study date: 2026-11-24. Every web source below was fetched on 2026-11-24 through a fetch tool that
summarises pages, so every web claim has status **relayed** unless marked otherwise. Numbers were
taken from the full HTML text of a paper where the tool could read it (marked "html"), and from the
abstract page where it could not (marked "abs"). Claims from my own memory are marked **assumed**.
Reasoning of mine is marked **inferred** with what it is inferred from. Local files I opened are
**observed**. Sources are listed in section 12 as [S1]..[S77] and [L1]..[L4], each with its
publication date.

The question: a language model runs on the GPU inside the seed system. Each step it picks a next
token. The aim is that the same step can pick, read or create a node of the numeric space, that
nodes have embeddings the model consumes inline, that capability attaches to a frozen model, that
variants are compared cheaply, and that this costs about one next-token step.

## 0. The answer in one page

1. **Appending rows to the output head is the proven cheapest mechanism, and it works on a frozen
   model.** ToolkenGPT appends a matrix of tool rows to the word head and takes one softmax over
   both; only the new rows train (2 minutes on one RTX 3090 for 13 tools against 40 minutes on
   8 A100 for LoRA) [S1]. TokMem (ICLR 2026) does the same for 1,000 procedures with 94.7 to 97.5 %
   routing accuracy and no forgetting when rows are added later [S5].
2. **A row that is trained per item cannot be chosen before it is trained.** On 837 unseen tools,
   trained-row selection scored 0.0 %; rows computed from the tool's description by the same frozen
   model scored 10.4 % top-1 and 33.7 % top-5 [S4]. A design that mints nodes at run time needs
   rows that are **computed from the node's content**, with learned rows as an optional later
   refinement.
3. **A random identifier is a poor thing to generate digit by digit.** Random ids lose to learned
   semantic ids (Recall@5 0.0296 against 0.0454 [S16]; SoPR 35.45 numeric against 55.00 one-token
   [S2]). A content hash is a random identifier. The hash stays the identity; what the model emits
   is a separate, derived address.
4. **Digit-by-digit identifiers cost one full forward pass per digit.** Google measured 400 ms of
   decode falling to 20 ms on Mistral-7B when a multi-token item id became one decoding step with a
   two-level softmax over about sqrt(N) clusters [S29]. If a multi-digit code is wanted, a small
   depth head can emit all digits inside one backbone step: Moshi emits 17 tokens per step with a
   6-layer head beside a 7 B backbone [S75].
5. **Choosing among millions in sub-linear work is solved three ways**: frequency clusters
   (adaptive softmax, 2x to 10x over the full softmax [S68]), a two-level softmax [S29], and
   product keys (64 M keys, each query compared with about sqrt(N) half-keys [S36]).
6. **A mask over a huge candidate set costs tens of microseconds on the CPU and overlaps the GPU
   step**: under 40 µs per token for JSON grammars over a 128k vocabulary [S63]; about 50 µs
   [S64]. With a trie over valid item tokens, invented tools fell from about 7 % to 0 [S2].
7. **A node can enter the input as one row**: a frozen document embedding passed through a 2-layer
   projector became one input token and kept most of the gain of the full text (42.46 against
   43.63 on Mistral-7B, input 175.1 tokens down to 1) [S46]. A node can also enter as a key-value
   pair in attention, at cost linear in the number of nodes (more than 10,000 triples beside an 8k
   context on one A100) [S44].
8. **The three things that failed at scale**: storing a corpus in weights and generating ids
   (MRR@10 24 to 27 against 34.8 for a dual encoder at 8.8 M passages [S17]; indexing new documents
   forgot old ones by about 25 points [S18]); full fine-tuning to add tokens (general ability 63.76
   down to 26.94 [S2]); and feeding hidden states back as inputs (gains shrink on larger models,
   42.2 to 43.6 % on Llama 3 8B [S58], and a 2026 causal analysis calls the latent tokens
   placeholders [S62]).

## 1. Ground: the design this study serves, and the machine

- A node is a blueprint id and ordered child ids, or a blueprint id and leaf bytes
  (coherence-seed/graph/5.kg:10, node 5.1.1). **observed**
- Three identities are kept apart: content id (SHA-256), a 32-bit in-memory handle, and a name that
  moves (5.kg:64 node 5.2.1; 5.kg:74 node 5.2.2; 5.kg:513 node 5.7.3). **observed**
- Names are rows in a table keyed by node and lens; a lens is itself a node (5.kg:97, node 5.2.4).
  **observed**
- The four-part reading (package, level, type, instance) is derived from content: package is the
  namespace of the blueprint, level its height, type its blueprint, instance its content id or
  leaf value (5.kg:119, node 5.2.6). **observed**
- A recipe's value-free shape is a second derived node used as a fingerprint (5.kg:108, node
  5.2.5). **observed**
- Compiled code is never ice; it is gas or cached water keyed by recipe id, generator id and
  target id (5.kg:450 node 5.6.4; 5.kg:712 node 5.10.2). **observed**
- Reading text through a lens answers with no node, one node or several candidates (5.kg:167, node
  5.3.3); every evaluation is an offer acknowledged by nothing, 0, 1 or a node (5.kg:232, node
  5.4.1). **observed**
- GPU programs are generated in the grammar and reached through a door (5.kg:764, node 5.10.7).
  **observed**
- The graph's node 6.1.4 leaves open whether a node's id is a digest, a coordinate or a handle
  (coherence-seed/graph/6.kg:35). **observed**
- The old body already had a grammar of eight control tokens a model stream can carry
  (coherence-seed/graph/2.7.kg:233). **observed** (the graph row; the file
  itself was not opened)

The per-step budget on the first machine, from the owner's private notes (I read the notes; the
measurements are theirs, so **relayed**):

- Qwen3.8-27B Q8_0 on the M4 Max: plain decode 63 ms per token (15.6 tokens/s) in a clean window;
  llama.cpp read 11.18 tokens/s quiet; the chip's memory bandwidth is given as 546 GB/s; the model
  width is 5120; a tiny dependent dispatch costs about 20 µs
  (memory/project-qwen-speed-vs-llamacpp-2026-10-04.md:3,
  :21, :31) [L2].
- Extra heads on the same hidden state already ran on this machine: a multi-token-prediction head
  and a 2 B block drafter conditioned on captured hidden states, 4.38x plain decode on code (same
  file, line 31) [L2].
- The GPU slows 2.6x under about 30 s of sustained load, so burst and sustained numbers differ; an
  argmax over 128,256 logits is one kernel
  (memory/reference-decode-submit-per-layer-and-reading-variance.md:13,
  :15) [L3].

**Inferred cost of a node head on this machine** (from the bandwidth and width above; not
measured): a decode step is bound by the bytes it reads. Scoring N node rows of r bytes each reads
N*r bytes. One million rows of 256 one-byte dimensions is 256 MB; at about 300 GB/s (the rate the
notes report for decode) that is about 0.85 ms, 1.4 % of a 63 ms step. One hundred thousand rows
at the model's full width of 5120 bytes is 512 MB, about 1.7 ms. A two-level choice over one
thousand centroids and one thousand members reads 512 KB and is bounded by the dispatch floor of
about 20 µs each. So a brute-force node head up to about a million low-dimension rows sits inside
"roughly one token step" by a few percent; beyond that it needs the two-level or product-key form.

## 2. Tool and function tokens on a frozen model

| System | Added | Trained | Per-step cost | New item | Measured |
|---|---|---|---|---|---|
| ToolkenGPT [S1] html | matrix W_tau of (tools x d) appended to the word head; softmax over [W_v; W_tau] h | only W_tau; LLM frozen, no gradient through its body | one extra row per tool in the head product | add a row, train it on demonstrations (in-domain or synthesised) | FuncQA one-hop 0.73 against ReAct 0.57; VirtualHome success 0.68 with 58 toolkens against 0.38; KAMEL up to 234 relations; training 2 min on one RTX 3090 against LoRA 40 min on 8 A100 |
| Toolken+ [S3] abs | a "Reject" option and a rerank of the top-k tools | small | rerank only when a tool ranks first | same | names the two faults of ToolkenGPT: no use of documentation, and wrong decisions on whether to call at all |
| CoTools [S4] html | a Tool Judge (gated bottleneck over the last hidden state, threshold 0.5) and a Tool Retriever (query encoder, tool encoder, a per-dimension weight vector of length d) | Judge and Retriever only | one small head per step; retrieval only when the judge fires | **no training**: the tool's vector is the frozen model's hidden state over its name and description | 1,836 tools, 837 unseen: top-1 10.4 %, top-5 33.68 % on unseen (ToolkenGPT 0.0 %); seen 35.1 % against 23.8 %; KAMEL 93.8 % against 93.4 %; 1,561 of 4,096 hidden dimensions carry the selection |
| TokMem [S5] html | one row per procedure in a bank of (l x d); the row is both index and steering signal | only the new rows (0.10 M to 0.20 M parameters in the compositional setting against 0.85 M to 3.41 M for LoRA); an optional one-time LoRA adaptation | one extra row per procedure | add a row; renormalise it to the mean norm of existing rows, or it crowds older rows out | 1,000 tasks: routing 94.7 to 97.5 % against 79.7 % for a sentence-encoder retriever; ROUGE-L 64.8 against 61.6 (LoRA) and 42.6 (RAG) on Llama 3.1 8B; no forgetting on sequential addition, where replay fine-tuning dropped sharply |
| ToolGen [S2] html | 46,985 tokens added to a 128,256 vocabulary, one per API | **full fine-tuning** in three stages (memorise, retrieve, act) on 4 A100 | none beyond a larger head | weak: the paper says entirely new tools are not handled | NDCG@1 89.17 against 80.50 for a retriever; one token per tool beat numeric ids (SoPR 55.00 against 35.45) and hierarchical ids (45.50); a trie mask took invented tools from about 7 % to 0; general ability fell from 63.76 to 26.94 without mixed data |
| Octopus v2 [S6] html | N functional tokens plus an end token in the head | full or LoRA (rank 16) fine-tune, 100 to 1,000 examples per function | function choice is one token | retrain with data for the new function | 20 functions: 99.5 % accuracy at 0.38 s against 13.46 s for a 7 B model with retrieval; prompt shorter by more than 95 % |

Lessons, **inferred** from the table:

- The row-appended head with a frozen body is the mechanism to carry forward. Its three known
  faults are each addressed by a later paper: no entry for the unseen (computed rows, CoTools),
  miscalibrated "call or not" (a reject entry or a judge head), and norm growth of new rows
  (renormalisation, TokMem).
- Full fine-tuning to make room for items damages the model and closes the door on new items.
- The row that selects a node on the output side can be the same vector that steers generation on
  the input side (TokMem). A node then needs one row, used in both places.

## 3. New rows: how to initialise them

- A new row with small random or zero values can take almost all probability: adding three words
  to GPT-2 small made it emit only those words. Setting the row to the mean of existing rows bounds
  the divergence from the old distribution by log(1 + 1/n) for vocabulary size n [S8] (2021-12-06,
  html).
- If each new row lies inside the convex hull of the existing rows, greedy outputs on old prefixes
  do not change; mean and hull-preserving initialisations performed close to the best method, and
  random initialisation collapsed on several tasks (LLaMA2-7B, 25k new tokens) [S7] (2024, html).
- Mean initialisation puts all new rows in one place, and fine-tuning struggles to separate them;
  a 2026 paper proposes placing each new row at a distinct, meaningful location before fine-tuning
  [S10] (2026-04-02, abs; no numbers on the page).
- A hypernetwork can predict the embedding rows of a whole new tokenizer with no training of the
  model, closing the remaining gap with under 1 B tokens of further training [S9] (2024, abs).

Lesson, **inferred**: a new node's row should start inside the hull of existing rows and at a
distinct point. A row computed by an encoder from the node's content has both properties if the
encoder's output is projected into the hull; a mean row has only the first.

## 4. Pointers and copying: choosing among items present in the context

- A pointer network uses attention weights as the output distribution over input positions, so
  the output set can change size with every input [S11] (2015-06-09, abs).
- The pointer sentinel appends one sentinel vector to the pointer's attention; the mass on the
  sentinel is the gate g, and p = g * p_vocab + (1 - g) * p_pointer. It adds H^2 + 2H parameters
  and reached perplexity 70.9 on Penn Treebank with 21 M parameters against 78.4 for a 66 M LSTM;
  the gain grows as words get rarer [S12] (2016-09-26, html through ar5iv).
- A pointer-generator copies from the source or generates from the vocabulary, at least 2 ROUGE
  points over the earlier abstractive result [S13] (2017-04-14, abs).

Lesson, **inferred**: a node already present in the context needs no id to be generated. The
step's attention over the context positions that hold nodes is the choice, at zero extra rows, and
the sentinel is the cheapest known gate between "token" and "item". This is the natural first
mechanism for arguments: most children of a new node are nodes already in view.

## 5. Generative retrieval and semantic ids

How an item gets a short hierarchical code:

- DSI compared three ids: one new output row per document, a number generated digit by digit, and
  a code from hierarchical k-means (k = 10, clusters split until 100 remain) so that similar
  documents share prefixes. With T5-XXL on 320k documents the semantic code reached Hits@1 40.4
  against 24.3 for a dual encoder; atomic rows were unstable to train; the model forgot earlier
  batches while indexing new ones [S14] (2022-02, html).
- TIGER embeds an item's text with a frozen sentence encoder (768 dimensions), then a residual
  quantiser (3 levels of 256 codes, latent width 32) gives 3 digits, and a 4th digit breaks
  collisions. A 13 M-parameter model generates the 4 digits. Recall@5 on Beauty 0.0454 against
  0.0387 for the baseline; random ids 0.0296, hashed ids 0.0379; invalid ids in the top 10 at 0.1
  to 1.6 %; a new item is reached by matching the first three digits; beam search makes inference
  dearer than a nearest-neighbour index [S16] (2023-05, html).
- Residual quantisation represents a vector as a stack of codes from a fixed codebook, and a
  second small transformer predicts the stack at each position [S19] (2022-03-03, abs).
- NCI: semantic ids with a prefix-aware decoder, +21.4 % relative Recall@1 on NQ320k [S15]
  (2022-06-06, abs).

What happened at scale:

- At 8.8 M passages the best generative retriever (T5-XL, plain numeric ids) reached MRR@10 26.7;
  a dual encoder reached 34.8. One row per document cost about 7 B extra parameters. Semantic ids
  did not beat plain ids there, and the special decoder, constrained decoding and consistency loss
  were not worth their cost. The paper calls scaling to millions "unsolved" [S17] (2023-05-19,
  html).
- Indexing new documents into the weights cut indexing accuracy on the first corpus by about 25
  points, and about 75 after five increments; flatter minima and replay of generated queries
  recovered part of it [S18] (2022-12, html).
- Industry moved the idea to recommendation with a fixed quantiser and a large model. YouTube's
  ranking model uses 8 levels of 2,048 codes from a frozen quantiser over 2,048-dimension content
  embeddings; quantisers trained six months apart performed comparably [S22] (2023-06, html).
  PLUM uses codebooks that shrink by level (2048 / 2^(level-1)), reaches 96.7 % unique codes,
  continues pretraining on about 260 B tokens, and reports under 5 % invented codes [S21]
  (2025-10, html). OneRec uses 3 levels of 8,192 codes from balanced k-means, chosen to avoid the
  uneven code use of a learned quantiser, and reports +1.68 % watch time online [S20] (2025-02-26,
  html). Spotify reports a deployment with up to +14.3 % new-show discovery [S27] (2026-03-18,
  abs).
- Two 2026 studies say no semantic-id design is best everywhere: effects are non-monotonic, a
  balanced codebook is not sufficient, and longer codes do not always help [S26] (2026-09-21,
  abs); a unified space of product, residual and hybrid quantisation with training-free quality
  measures is proposed [S25] (2026-10-06, abs).
- A flat token per code digit gives the same embedding whatever the prefix, so one digit means
  different things under different prefixes; fixing that by pretraining "requires hundreds of
  billions of tokens" and ties the codes to one checkpoint. A separate encoder of hashed prefix
  tables (2 M rows, width 256) raised deepest-level accuracy from 37.6 % to 54.8 % at under 0.02 %
  extra compute, and a 0.6 B model with it beat a 4 B model without [S28] (2026-05-29, html).
- Generating a multi-token id needs one full forward pass per token. One embedding per item and a
  two-level softmax (pick one of about sqrt(N) clusters, then an item in it) cut Mistral-7B
  decode from 400 ms to 20 ms and the whole call from 475 ms to 55 ms; tested to 37,712 items
  [S29] (2025-09-03, html).
- Names instead of codes: GENRE retrieves an entity by generating its unique name, and a new
  entity is added by giving its name [S23] (2020-10-02, abs); SEAL lets any n-gram of a passage
  stand as its identifier [S24] (2022-04-22, abs). That both use a prefix structure to keep
  generation inside the valid set is **assumed** from memory; the abstract pages do not say it.

Lessons, **inferred**:

- The hash is not the code. A content id is uniform noise by construction, and every result above
  says a model generates meaningful prefixes far better than noise. The seed already separates
  identity from name (node 5.2.4); a semantic address is one more name, in a lens whose identity is
  (encoder id, codebook id).
- A semantic address is derived and can be recomputed, so it is water or gas, never ice. A new
  codebook changes every address and no node.
- The four-part reading is already a hierarchy whose first three parts have low cardinality and
  come from content. Package, level and type can be the leading digits of a node address for free;
  only the instance part needs a learned or computed code. This is a hierarchical softmax whose
  tree is the coordinate.
- New items are the standing weakness of every scheme that stores items in weights. The two
  schemes that admit a new item with no training are a computed embedding (dual-encoder style) and
  a name or address checked against a trie.
- Collisions are normal (94 to 97 % unique at best). The design needs a last digit or a pointer
  that separates nodes sharing a prefix.

## 6. Retrieval inside the forward pass

- kNN-LM stores one (1024-dimension hidden state, next token) pair per training token, 103 M for
  WikiText-103, searches 1,024 neighbours and interpolates with weight 0.25: perplexity 18.65 to
  16.12, and a new domain is adopted with no training (34.84 to 20.47) [S31] (2019-11, html). It
  runs 14x to 26x slower than the base model (3,847 against 277 tokens/s); skipping half the
  lookups with a small gate, pruning and 512-dimension keys bring back 6.6x at equal perplexity
  [S32] (2021-09, html).
- RETRO retrieves 64-token chunks from up to 2 trillion tokens with a frozen encoder and reads
  them through chunked cross-attention; a lookup takes about 10 ms amortised per chunk; a frozen
  pretrained model can be fitted by training only the new cross-attention and encoder weights,
  under 10 % of a 7 B model, on about 3 % of the pretraining sequences, with behaviour unchanged
  when retrieval is off [S33] (2021-12, html).
- Memorizing Transformers add a non-differentiable key-value memory to one attention layer, top
  32 neighbours, a learned per-head gate; step time for a 200 M model went from 0.20 s to 0.25 s
  with 8k memory and 0.6 s with 65k; an 8k memory matched a model with 5x the parameters; a
  pretrained 1 B model learned to use the memory in 20k to 100k steps [S34] (2022-03-16, html).
- Unlimiformer serves every cross-attention head of every decoder layer from one index over the
  encoder's states, with no added weights and no training, at about 2 GB per million tokens;
  inference cost 4.48x on 143k-token inputs [S35] (2023-05, html).
- Memory layers with product keys replace a feed-forward layer by a trainable key-value lookup;
  each query half is compared with about sqrt(N) half-keys. With 64 M keys (128 B memory
  parameters) on a 1.3 B model, NQ rose from 7.76 to 20.78 and TriviaQA from 32.64 to 62.14,
  near a 7 B model at about a tenth of the compute. The operation is bound by memory bandwidth;
  custom kernels reached about 3 TB/s on an H100 against under 400 GB/s for the framework's own.
  It was trained with the model, not added to a frozen one [S36] (2024-12, html). The earlier
  form: a 12-layer model with a product-key memory beat a 24-layer model at twice the speed [S37]
  (2019-07-10, abs).

Lessons, **inferred**: the hidden state is a usable query into an external store without training
(kNN-LM, Unlimiformer). Exact nearest-neighbour search per token on a CPU index is what made
kNN-LM slow; a product-key or two-level lookup on the GPU is bound by bandwidth, which is the same
resource a decode step is bound by on the M4 Max [L2]. A gate that skips the lookup on most steps
halves its cost at no loss [S32].

## 7. Entity memories and knowledge-graph models

- Entities as Experts: 1 M entity rows of 256 dimensions (about 256 M of 367 M parameters) placed
  after layer 4 of 12. At a mention the model forms a query from the span's start and end states,
  scores all entities, and adds the weighted sum of the top 100 back into the hidden state.
  TriviaQA 43.2 against 42.3 for T5-11B with about 30x fewer parameters. Restricting to the top 10
  changed TriviaQA by 0.1 (43.1 against 43.2); the full million gave 43.4 [S38] (2020-04-15, html
  through ar5iv).
- LUKE: about 500k entity rows of 256 dimensions projected to 1024 (128 M of 483 M parameters),
  entities as tokens of their own beside words, with separate attention query matrices by token
  type; an entity outside the vocabulary becomes one "unknown" entity [S39] (2020-10-02, html
  through ar5iv).
- K-BERT injects triples into the sentence with soft positions and a visibility matrix so that
  injected knowledge does not disturb the rest; it loads a pretrained model with no knowledge
  pretraining of its own [S40] (2019-09-17, abs).
- KEPLER computes an entity's embedding by encoding its description with the language model, so
  it works inductively on link prediction [S41] (2019-11-13, abs). That this covers unseen entities
  with a description at no table cost is **inferred** from the mechanism.
- GreaseLM fuses language-model and graph-network states over several interaction layers and can
  beat models 8x larger on question answering [S42] (2022-01-21, abs).
- GraphToken: a graph network of 17k to 199k parameters plus a projection of about 11 M emits a
  few soft tokens placed before the prompt of a frozen model; node-count accuracy 0.996 against
  0.276 for the best text prompt, node degree 0.962 against 0.292 [S43] (2024-02, html).
- KBLaM: each triple becomes one key-value pair per attention layer through a frozen sentence
  encoder and trained linear adapters. Prompt tokens attend to knowledge tokens; knowledge tokens
  attend to nothing, so cost is linear in the number of triples. More than 10,000 triples (about
  200,000 text tokens) beside an 8k context on one A100 80 GB; time to first token with 4,096
  triples is below retrieval-augmented text with 5; a triple is added, changed or removed by
  editing its own pair, with no retraining; the adapters are trained once on synthetic data [S44]
  (2024-10, html; blog 2025-03-18).
- Knowledge Capsules (2026): a frozen model compiles triples and their source sentences into
  per-layer key-value tensors with no training, and selected tensors are placed before the
  attention cache; exact match on HotpotQA 33.3 against 22.5 for text retrieval on Qwen2.5-7B. No
  latency, memory or scale is reported, and selection depends on detecting entities in the query
  [S45] (2026-04-23, html).
- xRAG: one frozen 4096-dimension document embedding through a trained 2-layer projector (0.46 %
  of Mistral-7B's parameters) becomes **one** input token. Average over six tasks 42.46 against
  43.63 for the full text (175.1 tokens), which is +16.2 % and +19.4 % over no retrieval; total
  FLOPs down 3.53x and GPU time 1.64x better; weaker
  on multi-hop questions (HotpotQA 34.05 against 38.79) [S46] (2024-05, html).

Lessons, **inferred**:

- A 256-dimension row projected up to the model width was enough for a million entities in two
  independent systems. Node rows need not be model-width, which is what makes a million of them
  fit a token step's bandwidth (section 1).
- The top 10 to 100 of a million carried the whole benefit, so the read-back can be sparse.
- KBLaM's rectangular attention is the cleanest precedent for "nodes readable by the model without
  entering the token sequence": nodes are independent rows, each edited alone, cost linear in
  their number. Its adapters are trained per model; the 2026 capsule work claims the same shape
  with nothing trained, by letting the model itself produce the pairs, but reports no cost.
- A graph's structure reaches a frozen model better through a small trained encoder than as text.

## 8. A concept as an embedding row; a context as a few rows

- Prompt tuning trains only soft prompt vectors on a frozen model and matches full tuning as
  models pass billions of parameters [S47] (2021-04-18, abs). Prefix tuning trains about 0.1 % of
  the parameters [S48] (2021-01-01, abs). LoRA trains low-rank updates, 10,000x fewer trainable
  parameters than full tuning of a 175 B model, with no added inference latency [S50] (2021-06-17,
  abs). That the update is merged into the weights is **assumed**; the page does not say how.
- Textual inversion learns one new word's embedding in a frozen text encoder from 3 to 5 images in
  5,000 steps (about two hours on 2 V100), started from a coarse descriptor word; the word then
  composes in sentences like any other [S49] (2022-08-02, html).
- A hypernetwork writes a LoRA from a task description in one forward pass, trained from 9
  adapters, and generalises to unseen tasks [S51] (2025-06-06, abs).
- Gist tokens: with an attention mask that hides the prompt behind 1 to 10 gist tokens, a prompt
  compresses up to 26x for 40 % fewer FLOPs; one cached key-value token of LLaMA-7B is 1.05 MB;
  agreement with the uncompressed model is near 50 % win rate, with visible failures where exact
  wording mattered [S52] (2023-04, html).
- AutoCompressors turn up to 30,720 tokens into summary vectors that can be precomputed for a
  corpus [S53] (2023-05-24, abs). ICAE compresses 512 tokens into 128 slots with about 1 % added
  parameters, reconstruction BLEU 99.1 %, falling apart on random text (BLEU 0.2) [S54] (2023-07,
  html). 500xCompressor keeps the key-value state of 1 to 16 tokens at every layer for about 500
  tokens of text, retaining 62 to 73 % of capability [S56] (2024-08-06, html). CEPE adds a 435 M
  encoder and cross-attention (1.8 B new parameters) to a frozen 7 B decoder and reads 128k tokens
  at 9.9x the throughput and one sixth the memory [S55] (2024-02, html).
- Cartridges: a trained key-value prefix per corpus, model frozen, trained by self-generated
  questions and distillation from the model that has the corpus in context. It matches in-context
  quality at 38.6x less memory and up to 26.4x throughput; a 484k-token book fits a 128k model;
  **independently trained cartridges concatenate and are queried together with no joint
  training**; about 30 minutes on 8 H100 each; initialising from the first tokens' real key-value
  state reached 55.3 % against 29.9 % from random vectors [S57] (2025-06, html).

Lessons, **inferred**:

- "A new concept is a new row" holds from one row (textual inversion, TokMem) through a prefix of
  rows (prompt tuning) to per-layer pairs (prefix tuning, cartridges). All leave the model
  untouched, so all are removable and composable, which is the property A/B comparison needs.
- A cached key-value token costs about 1 MB on a 7 B model [S52]. A million nodes as key-value
  state would be about a terabyte. Key-value form is for the hot few; one low-dimension row per
  node is for all.
- Compression is lossy where exact bytes matter. A node's embedding can route to the node and
  prime the model; the node's exact content must still be read from the store.

## 9. Hidden states fed back as inputs

- Coconut feeds the last hidden state back as the next input between two marker tokens. On GPT-2:
  ProsQA 97.0 % against 77.5 % for written reasoning with 14.2 against 49.4 tokens; GSM8k 34.1 %
  against 42.9 %. Training with n latent steps needs n + 1 passes and a staged curriculum; without
  the curriculum it is no better than no reasoning; on Llama 3 8B the gain was 42.2 to 43.6 %
  [S58] (2024-12, html).
- CODI distils written reasoning into continuous steps and reports matching written reasoning on
  GSM8k at GPT-2 scale with 3.1x compression [S59] (2025-02-28, abs).
- Soft Thinking needs no training: the next input is the probability-weighted mixture of token
  embeddings; up to +2.48 points and up to 22.4 % fewer tokens [S60] (2025-05-21, abs).
- Token Assorted adds discrete latent codes from a quantiser to the vocabulary and mixes them with
  text [S61] (2025-02-05, abs).
- A causal analysis accepted at EMNLP 2026 finds Coconut's latent tokens barely sensitive to
  perturbation and reliant on dataset shortcuts [S62] (2025-12-25, abs).

Lesson, **inferred**: the one training-free result here is the mixture input [S60], and it stays
inside the hull of existing rows, which section 3 says is the safe region. A frozen model does not
read its own raw hidden state as input without training. For this design, the input for a chosen
node should be the node's own row, not the raw state that chose it.

## 10. Masks, grammars, heads and logarithmic choice

Constrained decoding:

- Outlines recasts generation as moves between states of a finite-state machine and builds an
  index over the vocabulary [S65] (2023-07-19, abs). That the index maps each state to its allowed
  tokens for O(1) average cost per step is **assumed** from memory.
- XGrammar runs a byte-level pushdown automaton. Over 99 % of tokens can be judged without the
  stack (1,134 of about 128k need it for JSON, 120 after context expansion) and are precomputed
  per automaton node, stored as whichever of the accepted or rejected sets is smaller: 160 MB
  down to 0.46 MB. Per-token mask time fell from 65.776 ms to 0.018 ms; under 40 µs for JSON and
  under 200 µs for XML and a Python DSL. The mask is computed on the CPU while the GPU runs, and
  time per output token moved from 6.2 ms to 6.3 ms [S63] (2024-11, html).
- llguidance computes masks at about 50 µs per token for a 128k tokenizer with negligible start-up,
  using an Earley parser, a lexer of regular-expression derivatives and a walk over a token trie;
  under 1 % of masks take over 1 ms. OpenAI shipped it for structured outputs on 2025-05-20 [S64]
  (README, read 2026-11-24).
- llama.cpp's GBNF can match a token by id; repeated optional elements "may result in extremely
  slow sampling" [S66] (README on master, read 2026-11-24).
- On about 10k real schemas, grammar compile time ran from 0.00 to 0.01 s (Guidance) to 3.48 to
  8.05 s (Outlines); constrained output was 30 to 36 % faster end to end where forced tokens were
  skipped, and task accuracy rose 3 to 4 points [S67] (2025-01, html).

Several outputs from one step:

- Moshi emits 17 tokens per 80 ms frame: a 7 B backbone runs once, and a depth transformer of 6
  layers, width 1024, predicts the tokens of the frame in order. Flattening would multiply the
  backbone passes by 17 [S75] (2024-10-02, html).
- A separate small head on the hidden state decides "tool or text" in CoTools [S4]; a reject entry
  does it in Toolken+ [S3]; a sentinel's attention mass does it in the pointer sentinel [S12];
  special reflection tokens decide when to retrieve in Self-RAG [S76] (2023-10-17, abs).

Logarithmic choice:

- Adaptive softmax: GPU time for a matrix product is flat below about 50 columns and linear above,
  with a fixed cost of 0.22 to 0.40 ms per product on the GPUs of 2016. A head holds the frequent
  words plus one entry per tail cluster; tail clusters use a projection to d/4. 2x to 10x faster
  than the full softmax at equal perplexity on vocabularies of 44k to 800k; 2 to 5 clusters in
  practice [S68] (2016-09-14, html through ar5iv).
- A hierarchical model over a word tree built from the data can outperform flat models [S69]
  (NIPS 2008, abs). That a binary tree gives O(log V) decisions per word, from Morin and Bengio
  2005, is **assumed** from memory.

Items as output units with computed rows:

- Dynamic vocabulary: arbitrary phrases act as output units beside tokens; a 124 M phrase encoder
  computes their input and output rows; the phrase set is swapped with no retraining. With 65,536
  phrases memory rose 1.30x; a new domain was adopted with no training (MAUVE 26.35 against 23.06
  for a model fine-tuned on that domain); the base model was fine-tuned, not frozen; prefixes of a
  phrase had to be added as hard negatives [S70] (2024-10-11, html).
- Copy-is-all-you-need frames generation as choosing phrases from an indexed collection; a new
  collection needs no training [S71] (2023-07-13, abs). NPM replaces the softmax by a distribution
  over every phrase of a corpus [S72] (2022-12-02, abs).
- An embedding can be computed from an id with no table: several hash functions give an identifier
  vector and a network turns it into the embedding [S73] (2020-10-21, abs); or small tables chosen
  by complementary partitions of the id are combined so each id still gets a unique row [S74]
  (2019-09-04, abs).

Lessons, **inferred**:

- A mask over the valid set is cheap, exact and already runs beside the GPU. The valid node set at
  a position is exactly what a blueprint's shape defines, so the seed's own lens rows can serve as
  the grammar; no separate schema language is needed. This is how "create a node inline" becomes
  ordinary decoding: emit a blueprint choice, then children under the mask that blueprint implies.
- The depth head is the right shape for an address of several digits, and the two-level softmax is
  the right shape for a flat choice. Both keep the backbone at one pass per node.
- The dispatch floor matters more than arithmetic for small heads: in 2016 the fixed cost per
  product was 0.2 to 0.4 ms [S68]; the notes for this Mac give about 20 µs [L2]. A head of a few
  products costs well under 1 % of a 63 ms step.

## 11. Constructions for "token or node from one step"

All three share one frame, **inferred** from sections 2 to 10. The backbone runs once and yields
the last hidden state h. Everything below reads h. Nothing in the backbone changes, so the base
model behaves exactly as before when the additions are absent (the RETRO-fit property [S33]).

### A. One softmax, computed rows, a sentinel

The head is [W_vocab; W_gate; P·E_nodes]·h. E_nodes holds one low-dimension row per node (256
dimensions, as in [S38][S39]); P is one trained projection to the model width (as in [S46]);
W_gate is a single reject or sentinel row ([S3][S12]). A node's row is **computed** from its
content at freeze by an encoder (the frozen model's own hidden state over the node's canonical
print, as [S4] does for tools, or a compositional function of its blueprint row and children's
rows) and cached as water keyed by (node id, encoder id). A learned correction can later be added
per node, renormalised ([S5]). On the input side the chosen node's row, projected by the same P,
is the next input embedding ([S5][S46]). Beyond about a million rows the node part becomes a
two-level choice ([S29]) whose first level is the four-part reading's package, level and type.
The lens's shape mask removes rows that cannot stand at this position ([S63][S2]).

- Cost, **inferred**: one product over N*256 bytes plus one projection; about 1.4 % of a step at a
  million rows on this Mac (section 1).
- What must be trained: P and the gate, once per model. Per node: nothing.
- Unknowns: (1) whether a frozen 27 B model's last state separates "I want node X" from "I want
  the word for X" well enough for a linear P; the only measurement is 10.4 % top-1 and 33.7 %
  top-5 on 837 unseen tools with a 7 B model [S4]. (2) Whether a compositional row built from
  children carries enough meaning; no source tested rows built from a Merkle structure.
  (3) Calibration of the gate against 150k word rows whose logit scale the new rows do not share
  ([S8][S5] show both failure directions). (4) Where the training pairs (context, node) come
  from; the system's own offer log is the candidate, and it does not exist yet.

### B. A depth head that writes a short address, checked against the store

A small transformer (the shape of [S75]: 6 layers, width 1024) reads h and emits a node address in
3 to 5 digits within the same step: leading digits are package, level and type from the four-part
reading; trailing digits are a residual-quantised code of the node's computed row ([S16][S20]); a
last digit or a pointer separates collisions. A trie over existing addresses masks each digit
([S2]: invented items 7 % to 0). One reserved first digit means "new node" and switches the mask
to the chosen blueprint's shape, so creation is the same loop: blueprint, then children, each a
pointer into context (section 4), an address, or leaf bytes. The kernel, not the model, computes
the content id at freeze.

- Cost, **inferred**: a few small-transformer steps per node, each well under the backbone step
  ([S75] does 17 per 80 ms frame beside a 7 B backbone), plus tens of microseconds of mask.
- What must be trained: the depth head and the codebooks, once per (model, encoder).
- Unknowns: (1) Addresses change when codebooks are refitted ([S22] found quality stable across
  refits, not the codes); the store must treat an address as a name under a codebook id.
  (2) Code collisions and uneven code use ([S20][S21]: 94 to 97 % unique). (3) Whether a digit's
  meaning must depend on its prefix; [S28] says yes and that a flat row per digit loses 17 points.
  (4) The 2026 studies find no semantic-id design that wins everywhere [S25][S26]. (5) Generative
  retrieval lost to dual encoders at 8.8 M items [S17]; B keeps the items outside the weights,
  which that result did not test.

### C. Nodes readable through attention, capability as attachable nodes

For reading rather than choosing: a node enters as one projected input row ([S46]) when it is in
the sequence, or as a key-value pair per layer that prompt positions attend to and that attends to
nothing ([S44]) when it should be available without occupying the sequence. Hot nodes carry a
precomputed key-value state made by the model itself ([S45][S57]); the rest carry only the row.
Capability attaches the same way: a row (a procedure, [S5]), a key-value prefix (a corpus,
[S57]), or a low-rank weight update ([S50]) is a leaf node whose key includes the base model's id,
exactly as compiled code is keyed by generator and target (5.kg:712). Since [S57] shows
independently made prefixes concatenate, and rows are independent by construction, two variants
share the backbone's cached state up to the point where they differ. An A/B of two heads or two
row sets on one h is one extra head product, not one extra forward pass. An A/B of two prefixes or
two weight updates is a second pass from the divergence point.

- What must be trained: the projector, once per model ([S46]: 0.46 % of a 7 B model); adapters for
  the key-value form unless the training-free route of [S45] holds up.
- Unknowns: (1) [S45] reports no cost, scale or latency, and [S44] needs adapters trained per
  model. (2) A key-value token is about 1 MB on a 7 B model [S52] and more on a 27 B one, so the
  hot set is small. (3) Compression loses exact content ([S54][S56]); the row routes, the store
  answers. (4) Whether weight updates that are not merged can run at token-step cost on Metal was
  not in this study's scope.

**Order to try, inferred**: A first, because it adds two trained matrices and nothing per node,
and its failure would be visible in one measurement (top-k accuracy of computed rows on held-out
nodes). C's input half comes with A at no extra training. B only if a flat choice over the live
node set proves too wide or too slow.

## 12. Sources

Web sources, all fetched 2026-11-24 through a summarising fetch tool. "html" means the paper's full
HTML text was read (first 100,000 characters where longer); "abs" means the abstract page only.

| Id | Work | URL | Published | Read |
|---|---|---|---|---|
| S1 | ToolkenGPT (NeurIPS 2023 oral) | https://arxiv.org/html/2305.11554v4 | 2023-05-19, v4 2024-01-15 | html |
| S2 | ToolGen (ICLR 2025) | https://arxiv.org/html/2410.03439v3 | 2024-10-04, v3 2025-03-29 | html |
| S3 | Toolken+ | https://arxiv.org/abs/2410.12004 | 2024-10-15 | abs |
| S4 | Chain-of-Tools | https://arxiv.org/html/2503.16779v1 | 2025-03-21 | html |
| S5 | TokMem (ICLR 2026 poster per the conference listing) | https://arxiv.org/html/2510.00444v2 | 2025-10, v2 2026-03-08 | html |
| S6 | Octopus v2 | https://arxiv.org/html/2404.01744 | 2024-04-02 | html |
| S7 | Vocabulary expansion and initialisation comparison | https://arxiv.org/html/2407.05841 | 2024-07, v2 2024-10-22 | html |
| S8 | Hewitt, Initializing new word embeddings | https://www.cs.columbia.edu/~johnhew/vocab-expansion.html | 2021-12-06 | html |
| S9 | Zero-Shot Tokenizer Transfer (NeurIPS 2024) | https://arxiv.org/abs/2405.07883 | 2024-05-13 | abs |
| S10 | Grounded Token Initialization | https://arxiv.org/abs/2604.02324 | 2026-04-02, v2 2026-08-27 | abs |
| S11 | Pointer Networks | https://arxiv.org/abs/1506.03134 | 2015-06-09 | abs |
| S12 | Pointer Sentinel Mixture Models | https://ar5iv.labs.arxiv.org/html/1609.07843 | 2016-09-26 | html |
| S13 | Pointer-generator networks | https://arxiv.org/abs/1704.04368 | 2017-04-14 | abs |
| S14 | Differentiable Search Index | https://arxiv.org/html/2202.06991 | 2022-02, v3 2022-10-21 | html |
| S15 | Neural Corpus Indexer (NeurIPS 2022) | https://arxiv.org/abs/2206.02743 | 2022-06-06 | abs |
| S16 | TIGER | https://arxiv.org/html/2305.05065 | 2023-05, v3 2023-11-03 | html |
| S17 | How does generative retrieval scale to millions of passages | https://arxiv.org/html/2305.11841 | 2023-05-19 | html |
| S18 | DSI++ | https://arxiv.org/html/2212.09744 | 2022-12, v3 2023-12-08 | html |
| S19 | RQ-VAE and RQ-Transformer (CVPR 2022) | https://arxiv.org/abs/2203.01941 | 2022-03-03 | abs |
| S20 | OneRec | https://arxiv.org/html/2502.18965 | 2025-02-26 | html |
| S21 | PLUM | https://arxiv.org/html/2510.07784 | 2025-10 | html |
| S22 | Better generalization with semantic ids | https://arxiv.org/html/2306.08121 | 2023-06, v2 2024-05-30 | html |
| S23 | GENRE (ICLR 2021) | https://arxiv.org/abs/2010.00904 | 2020-10-02 | abs |
| S24 | SEAL | https://arxiv.org/abs/2204.10628 | 2022-04-22 | abs |
| S25 | A systematic study of semantic id spaces | https://arxiv.org/abs/2610.08732 | 2026-10-06 | abs |
| S26 | What makes a good semantic id (SIGIR-AP 2026) | https://arxiv.org/abs/2609.24430 | 2026-09-21 | abs |
| S27 | Semantic-id retrieval for podcast discovery | https://arxiv.org/abs/2603.17540 | 2026-03-18 | abs |
| S28 | LLMs need encoders for semantic ids too | https://arxiv.org/html/2606.00324v1 | 2026-05-29 | html |
| S29 | Efficient item id generation | https://arxiv.org/html/2509.03746v1 | 2025-09-03 | html |
| S30 | LIGER | https://arxiv.org/abs/2411.18814 | 2024-11-27 | abs |
| S31 | kNN-LM | https://arxiv.org/html/1911.00172 | 2019-11, v2 2020-02-15 | html |
| S32 | Efficient nearest neighbor language models | https://arxiv.org/html/2109.04212 | 2021-09, v3 2021-11-15 | html |
| S33 | RETRO | https://arxiv.org/html/2112.04426 | 2021-12, v3 2022-02-07 | html |
| S34 | Memorizing Transformers | https://arxiv.org/html/2203.08913 | 2022-03-16 | html |
| S35 | Unlimiformer | https://arxiv.org/html/2305.01625 | 2023-05, v3 2023-10-30 | html |
| S36 | Memory layers at scale | https://arxiv.org/html/2412.09764 | 2024-12, v2 2024-12-20 | html |
| S37 | Large memory layers with product keys (NeurIPS 2019) | https://arxiv.org/abs/1907.05242 | 2019-07-10 | abs |
| S38 | Entities as Experts | https://ar5iv.labs.arxiv.org/html/2004.07202 | 2020-04-15 | html |
| S39 | LUKE (EMNLP 2020) | https://ar5iv.labs.arxiv.org/html/2010.01057 | 2020-10-02 | html |
| S40 | K-BERT | https://arxiv.org/abs/1909.07606 | 2019-09-17 | abs |
| S41 | KEPLER (TACL) | https://arxiv.org/abs/1911.06136 | 2019-11-13 | abs |
| S42 | GreaseLM (ICLR 2022) | https://arxiv.org/abs/2201.08860 | 2022-01-21 | abs |
| S43 | GraphToken | https://arxiv.org/html/2402.05862 | 2024-02 | html |
| S44 | KBLaM; and its research blog | https://arxiv.org/html/2410.10450 ; https://www.microsoft.com/en-us/research/blog/introducing-kblam-bringing-plug-and-play-external-knowledge-to-llms/ | 2024-10, v2 2025-02-09; blog 2025-03-18 | html (method only), blog |
| S45 | Knowledge Capsules | https://arxiv.org/html/2604.20487 | 2026-04, v2 2026-04-23 | html |
| S46 | xRAG | https://arxiv.org/html/2405.13792 | 2024-05, v2 2024-12-09 | html |
| S47 | Prompt tuning (EMNLP 2021) | https://arxiv.org/abs/2104.08691 | 2021-04-18 | abs |
| S48 | Prefix tuning | https://arxiv.org/abs/2101.00190 | 2021-01-01 | abs |
| S49 | Textual inversion | https://arxiv.org/html/2208.01618 | 2022-08-02 | html |
| S50 | LoRA | https://arxiv.org/abs/2106.09685 | 2021-06-17 | abs |
| S51 | Text-to-LoRA (ICML 2025) | https://arxiv.org/abs/2506.06105 | 2025-06-06 | abs |
| S52 | Gist tokens | https://arxiv.org/html/2304.08467 | 2023-04, v3 2024-02-12 | html |
| S53 | AutoCompressors (EMNLP 2023) | https://arxiv.org/abs/2305.14788 | 2023-05-24 | abs |
| S54 | In-context autoencoder | https://arxiv.org/html/2307.06945 | 2023-07, v4 2024-05-08 | html |
| S55 | CEPE | https://arxiv.org/html/2402.16617 | 2024-02, v2 2024-06-11 | html |
| S56 | 500xCompressor | https://arxiv.org/html/2408.03094 | 2024-08-06 | html |
| S57 | Cartridges | https://arxiv.org/html/2506.06266 | 2025-06, v3 2025-06-13 | html |
| S58 | Coconut | https://arxiv.org/html/2412.06769 | 2024-12 | html |
| S59 | CODI | https://arxiv.org/abs/2502.21074 | 2025-02-28 | abs |
| S60 | Soft Thinking | https://arxiv.org/abs/2505.15778 | 2025-05-21 | abs |
| S61 | Token Assorted | https://arxiv.org/abs/2502.03275 | 2025-02-05 | abs |
| S62 | Do latent tokens think? (EMNLP 2026) | https://arxiv.org/abs/2512.21711 | 2025-12-25, v2 2026-10-26 | abs |
| S63 | XGrammar (MLSys 2025) | https://arxiv.org/html/2411.15100 | 2024-11, v3 2025-05-12 | html |
| S64 | llguidance README | https://github.com/guidance-ai/llguidance | undated; v1.0.0 on 2025-06-23 | page |
| S65 | Efficient guided generation (Outlines) | https://arxiv.org/abs/2307.09702 | 2023-07-19 | abs |
| S66 | llama.cpp grammars README | https://raw.githubusercontent.com/ggml-org/llama.cpp/master/grammars/README.md | undated; master on 2026-11-24 | page |
| S67 | JSONSchemaBench | https://arxiv.org/html/2501.10868 | 2025-01, v3 2025-02-27 | html |
| S68 | Adaptive softmax (ICML 2017) | https://ar5iv.labs.arxiv.org/html/1609.04309 | 2016-09-14 | html |
| S69 | A scalable hierarchical distributed language model | https://papers.nips.cc/paper_files/paper/2008/hash/1e056d2b0ebd5c878c550da6ac5d3724-Abstract.html | NIPS 2008 | abs |
| S70 | Generation with dynamic vocabulary | https://arxiv.org/html/2410.08481 | 2024-10-11 | html |
| S71 | Copy is all you need (ICLR 2023) | https://arxiv.org/abs/2307.06962 | 2023-07-13 | abs |
| S72 | NPM (ACL 2023 Findings) | https://arxiv.org/abs/2212.01349 | 2022-12-02 | abs |
| S73 | Deep hash embedding (KDD 2021) | https://arxiv.org/abs/2010.10784 | 2020-10-21 | abs |
| S74 | Compositional embeddings by complementary partitions | https://arxiv.org/abs/1909.02107 | 2019-09-04 | abs |
| S75 | Moshi | https://arxiv.org/html/2410.00037 | 2024-10-02 (v2) | html |
| S76 | Self-RAG | https://arxiv.org/abs/2310.11511 | 2023-10-17 | abs |
| S77 | LLaVA (NeurIPS 2023) | https://arxiv.org/abs/2304.08485 | 2023-04-17 | abs |

Local sources:

| Id | File | Dated | How read |
|---|---|---|---|
| L1 | coherence-seed/graph/5.kg (lines cited inline), coherence-seed/graph/6.kg:35, coherence-seed/graph/0.kg | 2026-11-24 | opened; `./kg brief` run |
| L2 | memory/project-qwen-speed-vs-llamacpp-2026-10-04.md | 2026-10-04 to 2026-10-07 | opened; measurements are the note's, not re-run |
| L3 | memory/reference-decode-submit-per-layer-and-reading-variance.md | 2026-10-12 to 2026-10-13 | opened; measurements are the note's, not re-run |
| L4 | coherence-seed/graph/2.7.kg:233 | 2026-11-24 | opened (the graph row only) |

## 13. Limits of this study

- Every web number passed through a summarising tool. Three summaries disagreed with their own
  page on a detail (the xRAG parameter share, 0.46 % in a table against "less than 0.1 %" in the
  introduction; the ToolkenGPT training count, 5,054 against 5,063; DSI's dual-encoder figure).
  Numbers that a build will rest on should be read at the source.
- Two pages showed dates I could not reconcile: the Coconut HTML carried a stamp of 2026-08-23 on
  a paper first posted in 2024-12, and the memory-layers HTML body carried 2026-08-24 beside a v2
  stamp of 2024-12-20. I cite the arXiv id's month.
- KBLaM's experiment tables could not be read (the HTML fetch returned the method section only);
  its base model, adapter size and measured memory are not in this report.
- LLaVA [S77] and LIGER [S30] were read at the abstract only and gave nothing this report uses
  beyond their existence; that a vision projector maps encoder features into the word-embedding
  space is **assumed** from memory.
- Nothing here was run. The cost figures for this Mac in section 1 are arithmetic over numbers
  from private notes, and the notes themselves warn that single readings on this machine vary 2x
  to 4x with load.
- No source tested a node head on a model of 27 B parameters, a row computed from a Merkle
  structure, or a head choosing among live program nodes. The constructions in section 11 are
  compositions of measured parts; none is measured whole.
