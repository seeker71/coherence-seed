# Structured embeddings: giving a content-addressed node a vector a model can use, and finding nodes from a hidden state

Study report, written 2026-11-24 for the coherence-seed design graph.
Scope: how a NODE of a content-addressed numeric space gets an embedding a language model can consume, and how a model's hidden state selects a node, at about the cost of the output projection.

## 0. How to read this report

Status words, as in graph node 8.5 (`coherence-seed/graph/8.kg`):

- **observed**: I read the file at that line, read the paper's extracted text myself, or ran the command on this machine today.
- **relayed**: read through the web fetch or web search tool, which summarises pages. Every web claim below is relayed unless marked otherwise. Numbers should be read at the source before anything is built on them (graph limit 6.3.6, `coherence-seed/graph/6.kg:327`).
- **inferred**: my reasoning; the premises are named.
- **assumed**: from memory or not verified.

All web sources were fetched on 2026-11-24. Each source is listed once in section 8 with its publication date; the body cites it as [Sn].

Limits of this study:

1. Abstract pages often carried no numbers; where a number mattered I fetched the full HTML. Where I could not, the claim says so.
2. The local measurements were taken on one shared Apple M4 Max (128 GB) while two processes of the old runtime owned by other sessions were running (seen in `ps`, 13.7% and 12.9% CPU). I did not start or touch them. Figures are minimums over repeated runs and vary by up to 3x between repetitions on the GPU (graph limit 6.3.8, `graph/6.kg:345`).
3. Nothing here was tested with a language model in the loop. Every statement about what a frozen model can do with node embeddings rests on published work in neighbouring settings (tools, documents, images), not on nodes of this space.
4. I did not verify the vocabulary size or hidden width of any specific model; the "unembedding-shaped" scans use 151,936 rows by 4,096 or 5,120 columns as an assumed shape for a current open-weight model family.

## 1. What the design already fixes (ground)

| Graph node | What it says | Where (observed) |
|---|---|---|
| 5.1.1 one-node-shape | A node is a blueprint id and ordered child ids, or a blueprint id and a leaf of bytes. No name, no state. | `coherence-seed/graph/5.kg:10` |
| 5.2.2 content-id | SHA-256 over the canonical encoding: blueprint id, then children by content id, or leaf bytes. No name enters it. | `graph/5.kg:74` |
| 5.2.3 identity-at-freeze | The content id is computed at freeze or at a boundary crossing, not at creation. | `graph/5.kg:86` |
| 5.2.4 names-beside-nodes | Names are rows keyed by (node, lens); a lens is itself a node. | `graph/5.kg:97` |
| 5.2.5 values-inside-shape-beside | A recipe's shape with literals replaced by holes is a second, derived node used as a fingerprint. | `graph/5.kg:108` |
| 5.6.4 compiled-code-is-not-ice | Machine code is gas, or water in a cache keyed by (recipe id, generator id, target id). Losing it loses nothing. | `graph/5.kg:450` |
| 5.6.5 state-invariants | Ice never points at water or gas. | `graph/5.kg:460` |
| 5.7.3 handles-in-memory | 32-bit handles in memory; a cheap non-cryptographic hash may pick a table slot. | `graph/5.kg:513` |
| 5.10.2 recipe-is-the-unit | Caches are keyed by content ids; no key contains a path, a time or a spelling. | `graph/5.kg:712` |
| 5.10.7 matrix-and-gpu | GPU programs are generated in the grammar and reached through a door. | `graph/5.kg:764` |
| 3.15 hash cost | On this M4 Max a 40-byte digest costs 16 ns with the CPU SHA-256 instructions; interning costs 14 to 25 ns and 56 bytes per node. | `graph/3.15.kg:6`, `graph/6.kg:206` |

Two consequences follow directly (inferred from 5.2.2, 5.2.4, 5.6.4):

- A SHA-256 content id has no geometry by construction: two nodes that differ in one leaf have unrelated ids. Any notion of "near" must be a second, derived artifact.
- An embedding is exactly the same kind of thing as compiled code: a materialisation of a node by some generator for some consumer. It belongs in the same cache discipline, keyed by (node content id, encoder id), in water or gas, never in ice.

## 2. Measurements made for this study (observed)

Sources and outputs are in `scratch/study/emb-work/exp/` (`mips_cpu.c`, `mips_metal.m`, `mips_kernel.m`, `mips_kernel2.m`, `vsa_capacity.c` and their `.out` files). Random data, so the numbers measure cost, not recall.

### 2.1 Exact inner-product scan: one query against a table (the cost of "select one node")

"Unembedding-shaped" means the shape of a model's output projection: vocabulary rows by hidden width.

**CPU, Accelerate `cblas_sgemv`, float32, minimum of 3 to 7 runs** (`mips_cpu.out`):

| Rows | Width | Table size | Scan | Argmax |
|---|---|---|---|---|
| 151,936 | 4,096 | 2.49 GB | 16.98 ms | 0.012 ms |
| 151,936 | 5,120 | 3.11 GB | 21.19 ms | 0.011 ms |
| 1,000,000 | 64 | 0.26 GB | 1.82 ms | 0.069 ms |
| 1,000,000 | 256 | 1.02 GB | 6.42 ms | 0.071 ms |
| 1,000,000 | 512 | 2.05 GB | 11.97 ms | 0.064 ms |
| 1,000,000 | 1,024 | 4.10 GB | 20.75 ms | 0.061 ms |
| 10,000,000 | 64 | 2.56 GB | 15.24 ms | 0.560 ms |
| 10,000,000 | 256 | 10.24 GB | 53.32 ms | 0.518 ms |

Throughput was 70 to 99 GFLOP/s throughout, so the scan is bound by memory traffic, not arithmetic.

**CPU, Hamming scan over binary sketches, one thread, plain `popcount` loop** (`mips_cpu.out`):

| Rows | Bits | Table size | Scan |
|---|---|---|---|
| 1,000,000 | 256 | 0.03 GB | 1.22 ms |
| 10,000,000 | 256 | 0.32 GB | 12.11 ms |
| 100,000,000 | 256 | 3.20 GB | 124.3 ms |
| 100,000,000 | 64 | 0.80 GB | 36.6 ms |

**GPU, hand-written Metal compute kernel, one GPU thread per row, argmax read on the CPU from the shared buffer; second of two passes of 30 runs each** (`mips_kernel2.out`):

| Rows | Width | Type | Table size | Scan (wall, min) | Rate |
|---|---|---|---|---|---|
| 151,936 | 4,096 | float16 | 1.24 GB | 3.43 ms | 363 GB/s |
| 151,936 | 4,096 | float32 | 2.49 GB | 7.38 ms | 337 GB/s |
| 1,000,000 | 256 | float16 | 0.51 GB | 2.86 ms | 179 GB/s |
| 1,000,000 | 256 | float32 | 1.02 GB | 2.47 ms | 415 GB/s |
| 1,000,000 | 512 | float16 | 1.02 GB | 2.51 ms | 409 GB/s |
| 10,000,000 | 64 | float16 | 1.28 GB | 2.9 to 4.0 ms | 319 to 379 GB/s |
| 10,000,000 | 256 | float16 | 5.12 GB | 11.62 ms | 441 GB/s |
| 100,000,000 | 64 | float16 | 12.80 GB | 28.2 ms | 453 GB/s |

The first pass of the same program was slower on several rows (for example 41.2 ms against 11.6 ms for 10 million by 256), so these are floors on a busy machine, not typical values. No scan came in under about 2.5 ms regardless of size, which reads as a fixed launch-and-wake cost of a separate command buffer (inferred from the three 1-million-row lines being equal within noise).

**GPU, the generic library path (`MPSMatrixMultiplication` plus `MPSMatrixFindTopK`)** (`mips_metal.out`): 1,000,000 by 256 float32, one query: 47.1 ms; the same with 32 queries in one batch: 17.5 ms total, 0.55 ms per query; 1,000,000 by 64, one query: 23.8 ms (13 times slower than the CPU); 1,000,000 by 1,024, one query: 8.6 ms (2.4 times faster than the CPU); top-16 over 10 million scores added 57 to 73 ms. The library path is erratic for a single tall-thin query and should not be the basis of a design figure.

What these numbers support (inferred from the tables above):

- On the same kernel, an exact scan of 10^6 node vectors at width 256 to 512 costs **0.7 to 0.85 of one unembedding-shaped scan** (2.5 to 2.9 ms against 3.4 ms). By arithmetic count it is 256 to 512 million multiply-adds against 622 million.
- 10^7 nodes fit the same budget at width 64 (about 3 ms) and cost about 3.4 unembeddings at width 256 (11.6 ms).
- 10^8 nodes cost 28 ms at width 64 in float16, about 8 unembeddings, and need 12.8 GB.
- 10^9 nodes cannot be scanned inside a token step on this machine: at the measured ceiling of about 450 GB/s, 128 GB of width-64 float16 would take about 280 ms, and it would not leave room for a model. Only the 256-bit sketches (32 GB) fit, and a scan of them is about 1.2 s on one thread.

### 2.2 How many role-filler pairs one fixed-width vector holds

Probe (`vsa_capacity.c`, `vsa_capacity.out`): bipolar vectors, bind by element-wise product, bundle by sum, unbind by multiplying with the role again, clean up by nearest of 100,000 random item vectors. Exact recovery of the filler, 80 to 320 probes per cell:

| Width D | K=4 | K=8 | K=16 | K=32 | K=64 | K=128 | K=256 |
|---|---|---|---|---|---|---|---|
| 256 | 1.000 | 0.931 | 0.388 | 0.066 | 0.009 | 0.000 | 0.003 |
| 1,024 | 1.000 | 1.000 | 1.000 | 0.887 | 0.322 | 0.066 | 0.013 |
| 4,096 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 0.881 | 0.294 |

Reading: recovery is exact up to about K = D/64 pairs and has collapsed by K = D/16, against an item memory of 10^5. The transition tracks the signal-to-noise ratio sqrt(D/(K-1)) crossing about 8. This agrees in shape with the published comparison in section 3.2.

## 3. What the literature shows

### 3.1 Encoders over trees and graphs

| Claim | Source | Status |
|---|---|---|
| Tree-LSTM generalises the LSTM to tree-structured topologies and beat sequence LSTM baselines on sentence relatedness (SemEval 2014 task 1) and sentiment (Stanford Sentiment Treebank). The page did not spell out the child-composition rule. | [S1], 2015-02-28 | relayed |
| code2vec represents a code snippet as one fixed-length vector by decomposing it into paths through its syntax tree and learning to aggregate them; trained on 14 million methods; over 75% relative improvement on method-name prediction. | [S2], 2018-03-26 | relayed |
| GraphCodeBERT uses the data-flow graph ("where-the-value-comes-from") rather than the syntax tree, arguing the tree's deep hierarchy is less efficient; pre-training adds edge prediction and code-to-structure alignment; state of the art on code search, clone detection, translation, refinement. | [S3], 2020-09-17 | relayed |
| UniXcoder maps a syntax tree to a sequence one-to-one, keeping all structure, and uses comments and the tree together with contrastive learning for fragment embeddings; five tasks, nine datasets; both comments and tree help. | [S4], 2022-03-08 | relayed |
| Tree positional encodings replace sinusoidal positions with an encoding of a node's place in a tree, enabling tree-to-tree and sequence-to-tree transformers; 22% absolute gain on a JavaScript-to-CoffeeScript translation task. | [S5], NeurIPS 2019 | relayed (search summary of the publisher page) |
| Graphormer adds structural encodings to a standard transformer and won the OGB Large-Scale Challenge track it entered. The abstract page did not list the encodings. | [S6], 2021-06-09 | relayed |
| GraphToken learns an encoder that represents graph-structured data explicitly for a language model and reports up to 73 percentage points improvement on node, edge and graph tasks of GraphQA. | [S7], 2024-02-08 | relayed |
| No published system found that caches embeddings per syntax-subtree content hash. What exists: caches keyed by file or chunk hash, and Merkle trees over files and folders used to find what changed. | [S8], search of 2026-11-24 | relayed; the absence is a search result, not a proof |

What this gives the design (inferred from the rows above and graph 5.1.1):

- A node here is already a tree node of (blueprint, ordered children). A recursive encoder, embedding(node) = f(embedding(blueprint), embeddings of children in order), is a pure function of the subtree, so its result is cacheable under the content id and an edit recomputes only the path to the root. That is the Tree-LSTM shape with a cache; I found nobody doing the cache, which makes it cheap to try and unproven.
- The same property is its weakness: a subtree-only embedding is context-free. GraphCodeBERT's choice of data flow and UniXcoder's gain from comments both say that what a piece of code means to a model depends on things outside the subtree and on words attached to it. In this design both of those live outside the node: uses are other nodes, and names and comments are lens rows (5.2.4).

### 3.2 Vector-symbolic and hyperdimensional representations

| Claim | Source | Status |
|---|---|---|
| Bundling capacity across eleven architectures: the number of dimensions needed to recover k bundled vectors at 99% accuracy grows linearly in k (straight-line fits), with an item memory of 1,000; dense binary needs the most dimensions, real-valued a little fewer, complex-valued (FHRR) and sparse binary the fewest. | [S9] text lines 860 to 935 of my extraction | observed (read in the paper's text) |
| With k fixed at 10, the dimensions needed grow with the logarithm of the item-memory size, in accordance with Plate's analysis; the plotted range stays under 700 dimensions up to 10^6 items. | [S9] lines 951 to 1010 and figure 5 axis | observed (text and axis labels; curve values not read) |
| Some bindings invert exactly (element-wise product on bipolar vectors, XOR on binary, FHRR), others only approximately (circular convolution in HRR). | [S9] lines 425 to 486 | observed |
| A superposition memory reaches a channel capacity of about half a bit per neuron. | [S10], 2017-07-05 | relayed |
| Resonator networks factor a product of several code vectors by alternating unbinding with pattern completion, "searching in superposition"; not guaranteed to converge; demonstrated on parsing a tree-like structure. | [S11], 2020-07-07; [S12], 2019-06-19 | relayed |
| The operational capacity of resonator networks scales as a quadratic function of vector dimensionality. | [S12] | relayed through a search summary; coefficients not read |
| A projection step fixes the numerical instability of HRR in a differentiable setting and improves concept retrieval by over 100 times. | [S13], 2021-09-05 | relayed |
| HRR-based self-attention runs in O(T H log H) time and O(T H) space, up to 280 times faster to train on Long Range Arena, near state of the art with one layer. | [S14], 2023-05-31 | relayed |
| GPT-2's residual stream can be read as bundling and binding of nearly orthogonal vectors; the reading accounts for a significant share of actual MLP weights. | [S15], 2024-12-10, workshop paper | relayed |
| A probe that maps residual-stream states into 4,096-dimensional bipolar hypervectors (codebook of 2,996 concepts) uses a 55 to 71 million parameter encoder and reaches 83% average probing@1 on synthetic analogies across six models; on SQuAD the encoder's cosine similarity falls to 0.44. The mapping is one-way. | [S16], 2025-09-29, revised 2025-12-02 | relayed |
| A 2025 position paper proposes explicit binding heads and hyperdimensional memory layers; it reports no experiments. | [S17], 2025-12-08 | relayed |

What this gives the design (inferred from [S9], [S10] and measurement 2.2):

- One fixed-width vector can literally hold role-filler structure, and the price is known: about D/64 exactly recoverable pairs against 10^5 candidates in my probe, linear in D, logarithmic in the number of candidates. A width-256 vector holds about four pairs exactly; a width-4,096 vector about sixty-four.
- Nesting does not escape this: a tree bundled bottom-up into one vector carries noise from every leaf, so the budget is total bound items in the subtree, not fan-out at one level. A node's vector therefore cannot be a decodable copy of a deep node. It can be a similarity fingerprint, and it does not need to be more, because the exact structure is always one dereference away under the content id.
- The strong use of binding in this design is different: it is a deterministic, training-free way to turn (blueprint, position, child) into a vector with integer arithmetic, so every runtime computes the same sketch from the same content. No learned encoder has that property.

### 3.3 Quantisation and semantic ids

| Claim | Source | Status |
|---|---|---|
| Product quantisation splits the space into low-dimensional subspaces, quantises each, and estimates distances from the short codes. Validation on two billion vectors is stated on a third-party summary only. | [S18], TPAMI 2011 | relayed; the 2-billion figure not confirmed at the source |
| Optimised product quantisation minimises distortion over both the space decomposition and the codebooks. | [S19], CVPR 2013 | relayed |
| RQ-VAE represents each position as a stack of residual codes; a 256x256 image becomes an 8x8 map of code stacks. | [S20], 2022-03-03 | relayed |
| TIGER builds a semantic id per item from a 768-dimensional Sentence-T5 content embedding with a 3-level residual quantiser, 256 codes per level, 32-dimensional codewords, plus a fourth code to break collisions; a sequence model generates the id. Recall@5 gains over the best baseline: 5.22%, 17.31%, 12.53% on three Amazon sets. | [S21] HTML, 2023-05-08 | relayed |
| Same paper, table 2, Recall@5 on three sets: random ids 0.007 / 0.0296 / 0.0362; ids from locality-sensitive hashing 0.0215 / 0.0379 / 0.0412; RQ-VAE ids 0.0264 / 0.0454 / 0.0521. | [S21] HTML | relayed |
| The paper states 256^4 as "approximately 4 trillion"; it is 4,294,967,296. | [S21] HTML; arithmetic mine | relayed, with the correction inferred |
| DSI trains one transformer to map query strings directly to document ids, storing the corpus in its parameters. | [S22], 2022-02-14 | relayed |
| 2026 reports from industry describe severe codebook collapse in RQ-VAE tokenisers and use non-differentiable residual k-means as an alternative; an "hourglass" concentration of middle-level codes is reported for long-tail data. | [S23], search of 2026-11-24 | relayed through a search summary; individual papers not read |
| Evolving the construction of semantic ids and collision handling by program search raised Recall@1 on NQ320K from 55.8% to 59.3%. | [S24], 2026-10-20 | relayed |

What this gives the design (inferred): a semantic id is a short numeric code with geometry, which is what a content hash lacks. But it is a function of trained codebooks, it collides, and its training is fragile. It is an index over embeddings, valid for one encoder and one codebook, and it can never stand in for identity. From the three rows of table 2, hashing recovers (0.0215-0.007)/(0.0264-0.007) = 75%, 53% and 31% of the gain that learned codes give over random ids: a training-free locality code is worth having first, and a learned one is worth measuring second.

### 3.4 One vector at several widths; binary codes

| Claim | Source | Status |
|---|---|---|
| Matryoshka training nests widths {8, 16, ..., 2048} (ResNet50) or {12, ..., 768} (ViT, BERT) in one vector. ImageNet-1K top-1 at width 8 / 64 / 256 / 2048: 66.63 / 75.82 / 76.47 / 76.80. | [S25] HTML, 2022-05-26 | relayed |
| Shortlisting 200 candidates at width 16 and re-ranking at width 2048 matches single-shot 2048 accuracy, about 128 times cheaper in theory and about 14 times faster in practice with HNSW; on ImageNet-4K the shortlist needs width 64 (32 times and 6 times). | [S25] HTML | relayed |
| Binary quantisation of a 1,024-dimensional text embedding gives 32 times less storage; retrieval quality retained is 92.53% without and 96.45% with float re-scoring; mean 24.76 times faster retrieval. Int8 retains about 97% to 99.3% at 4 times less storage. | [S26], 2024-03-22 | relayed |

### 3.5 Late interaction and multi-vector retrieval

| Claim | Source | Status |
|---|---|---|
| ColBERT encodes query and document independently and applies a cheap fine-grained interaction; two orders of magnitude faster and four orders fewer FLOPs per query than BERT rankers. | [S27], 2020-04-27 | relayed |
| ColBERTv2's residual compression cuts the storage of late-interaction models six- to tenfold. | [S28], 2021-12-02 | relayed |
| MUVERA turns multi-vector similarity into single-vector inner-product search with fixed-dimensional encodings: about 10% better recall at about 90% lower latency, retrieving 2 to 5 times fewer candidates. | [S29], 2024-05-29, revised 2026-06-08 | relayed |

What this gives the design (inferred): a node has a natural multi-vector form without any extra machinery, its own vector plus its children's. Multi-vector scoring is a re-ranking step over a shortlist, never the first scan.

### 3.6 Aligning an outside embedding space to a frozen model's input

| Claim | Source | Status |
|---|---|---|
| LLaVA: one trainable linear matrix maps frozen CLIP ViT-L/14 features to tokens in the language model's embedding space. Stage 1 trains only that matrix on about 595,000 image-text pairs, one epoch, under 4 hours on 8 A100 GPUs, with encoder and language model frozen. Stage 2 also trains the language model (158,000 samples, under 10 hours). Skipping stage 1 dropped ScienceQA accuracy from 90.92% to 85.81%. | [S30] HTML, 2023-04-17 | relayed |
| LLaVA-1.5: an MLP connector, 1.2 million data points, about one day on one 8-A100 node, state of the art on 11 benchmarks. | [S31], 2023-10-05 | relayed |
| LiMBeR: a single trained linear projection from a frozen image encoder into a frozen language model's input space is competitive on captioning and visual question answering; encoders trained with language supervision transfer category distinctions better. | [S32], 2022-09-30 | relayed |
| BLIP-2: a lightweight querying transformer between a frozen image encoder and a frozen language model; 54 times fewer trainable parameters than Flamingo80B and 8.7% better zero-shot VQAv2. | [S33], 2023-01-30 | relayed |
| Flamingo bridges frozen vision and language models and handles interleaved inputs; the abstract page gave no architecture detail or numbers. | [S34], 2022-04-29 | relayed |
| **xRAG**: a frozen retriever's 4,096-dimensional document embedding is projected by a two-layer MLP (0.46% of Mistral-7B's parameters, 0.07% of Mixtral's) into ONE input token of a frozen language model. Average over six tasks, Mistral-7B: no retrieval 36.54, full-text retrieval (175.1 tokens on average) 43.63, xRAG with one token 42.46. Mixtral: 42.76 / 46.22 / 46.91. FLOPs fall 3.53 times, wall time 1.64 times. | [S35] HTML, 2024-05; v2 2024-12-09 | relayed |
| xRAG's projector was trained in two stages on 8 A100: up to 2,000,000 paraphrase samples, then 955,338 instruction samples. | [S35] HTML | relayed |
| xRAG loses on reasoning-heavy tasks: HotpotQA 34.05 against 38.79 with full text, FactKG 63.08 against 67.76. Multi-document and multi-vector use were untested. | [S35] HTML | relayed |
| Gist tokens: a model fine-tuned with a modified attention mask compresses a prompt into cacheable tokens, up to 26 times compression, up to 40% fewer FLOPs. | [S36], 2023-04-17 | relayed |
| ICAE compresses context into memory slots, 4 times compression with about 1% additional parameters. | [S37], 2023-07-13 | relayed |
| Cartridges: a key-value cache trained offline per corpus by self-study matches in-context performance with 38.6 times less memory and 26.4 times higher throughput, and cartridges compose at inference without retraining. | [S38], 2025-06-06 | relayed through a search summary |
| Function vectors: a small number of attention heads carry a compact vector for a demonstrated task; it triggers the task in zero-shot settings and can to some extent be summed into vectors for new composite tasks. | [S39], 2023-10-23 | relayed |
| A 2026 study of hashed external memory trained on one model and attached frozen to another finds that the usable signal depends on a target-side reader; with a compatible reader interface the frozen table transfers with no target training. | [S40], 2026-08-17, revised 2026-11-01 | relayed |

What this gives the design (inferred): the input direction is the best-supported part of the whole question. A frozen encoder's vector plus a projector of well under 1% of the model gives one input token that carries most of the value of a 175-token passage, and it fails exactly where exact content is needed. The pattern "one vector for the gist, dereference for the content" matches a store where the content is always available by id.

### 3.7 Reading a hidden state as a query

| Claim | Source | Status |
|---|---|---|
| ToolkenGPT: a matrix of tool embeddings is concatenated with the frozen model's output head, so the next-token distribution is softmax over [vocabulary; tools] applied to the same hidden state. Only the tool embeddings are trained. After a tool token wins, generation switches to a mode that fills arguments from in-context demonstrations. | [S41] HTML, 2023-05-19 | relayed |
| ToolkenGPT scale and results: 4, 13, 58 and up to 234 tools; FuncQA one-hop 0.73 against 0.57 for ReAct; VirtualHome success 0.68 against 0.38; in-context learning degrades sharply beyond 30 tools. Training took 2 minutes on one RTX 3090, against 40 minutes on 8 A100 for LoRA, which scored slightly higher on that comparison (0.62 against 0.55 one-hop). | [S41] HTML | relayed |
| ToolGen gives each of over 47,000 tools its own vocabulary token and integrates tool knowledge into the model's parameters, so retrieval and calling are generation. | [S42], 2024-10-04 | relayed |
| kNN-LM: with no training, the frozen model's hidden state (the 1,024-dimensional input to the last layer's feed-forward block) is the key for a nearest-neighbour search over 103 million stored states (4,096 coarse centroids, 64-byte codes, 32 lists probed, 1,024 neighbours). Wikitext-103 test perplexity 18.65 to 16.12, 15.79 with a cache. | [S43] HTML, 2019-11-01 | relayed |
| kNN-LM: a model trained on 100 million tokens with a 3-billion-token datastore reaches 13.73 perplexity, better than the same model trained on all 3 billion (15.17). | [S43] HTML | relayed |
| Copy-is-all-you-need: generation as a series of copy operations, each selecting a phrase from an indexed collection by vector search; better quality on WikiText-103, inference efficiency comparable to token-level models, and swapping the collection adapts the domain without training. | [S44], 2023-07-13 | relayed |
| The output projection plus top-k is a maximum inner product search over token embeddings; a small-world graph search finds the top-K words with an order of magnitude less decoding time at near-full-softmax accuracy. | [S45], 2018-06-11 | relayed |
| Replacing the dense vocabulary projection with an HNSW index improves end-to-end single-stream CPU decoding throughput by up to 82% (Gemma 3 270M), quality preserved on AlpacaEval; tested on Gemma 3, Llama 3.2, Qwen 3. | [S46], 2026-07-01, workshop paper | relayed |
| The logit lens is often brittle; an affine probe per layer (the tuned lens) decodes hidden states into the vocabulary more reliably, on models up to 20 billion parameters. | [S47], 2023-03-14, revised 2025-11-11 | relayed |
| Patchscopes uses the model itself to explain its hidden states and fixes the failure of vocabulary projection on early layers. | [S48], 2024-01-11 | relayed |
| With a hidden width far below the number of classes, some classes can be impossible to select by argmax whatever the input; 13 of 150 public models had such tokens, all rare. | [S49], 2022-03-12 | relayed |
| A text embedding can be inverted: an iterative correct-and-re-embed method recovers 92% of 32-token inputs exactly. | [S50], 2023-10-10 | relayed |
| A decoder-only model becomes a strong text encoder with bidirectional attention, masked next-token prediction and unsupervised contrastive learning. | [S51], 2024-04-09 | relayed |

What this gives the design (inferred):

- "The unembedding is a retrieval" is literal and has been built twice ([S45], [S46]). Adding rows to that retrieval is what ToolkenGPT does. So "pick a token or pick a node in the same step" is an extension of an existing operation, not a new one.
- The two published ways to add rows both fail the requirement "any new node, no retraining": ToolkenGPT trains one vector per tool in the model's own output space (tested to 234), and ToolGen changes the model's parameters (47,000). Neither makes a node created a millisecond ago selectable.
- kNN-LM shows a frozen model's hidden state is a usable query with no training, but only against keys that are that same model's hidden states. Node embeddings come from a different encoder, so a trained map is needed, and the tuned lens says an affine map from a chosen layer is the right first form.
- Unit-normalising node vectors removes the unselectable-class hazard: every unit vector is the strict maximiser of its own direction. (Inferred from [S49] and elementary geometry; not tested.)
- An embedding leaks its content ([S50]). A node behind an interface its cell has not offered must not have its embedding exposed either; the vector is a crossing in the sense of axiom 4.

### 3.8 Nearest-neighbour search, on GPU and on Apple silicon

| Claim | Source | Status |
|---|---|---|
| The FAISS GPU work: k-selection at up to 55% of theoretical peak, 8.5 times faster than the prior GPU state of the art; a k-NN graph on 95 million images in 35 minutes; a graph over 1 billion vectors in under 12 hours on 4 Titan X GPUs. | [S52], 2017-02-28 | relayed |
| FAISS GPU support is CUDA only; no Metal or Apple support is mentioned; 5 to 10 times faster than its CPU counterpart on one GPU; k and nprobe at most 2,048. | [S53], wiki page read 2026-11-24, undated | relayed |
| CAGRA (GPU graph index): 33 to 77 times the throughput of HNSW at 90 to 95% recall in large batches; 3.4 to 53 times faster for a single query at 95% recall; build 2.2 to 27 times faster. | [S54], 2023-08-29 | relayed |
| HNSW: layered proximity graphs with logarithmic search scaling. | [S55], 2016-03-30 | relayed |
| SOAR adds redundant partition assignments with an orthogonality-amplified residual loss to ScaNN-style indexes. No numbers on the abstract page. | [S56], 2024-03-31 | relayed |
| DiskANN indexes a billion points on one workstation with 64 GB RAM and an SSD: over 5,000 queries per second, under 3 ms mean latency, over 95% 1-recall@1 on SIFT1B, 16 cores. | [S57], NeurIPS 2019 | relayed through a search summary |
| mlxVS, an open-source library on MLX for Apple GPUs (exact k-NN as "matrix multiply and a fused top-k kernel", IVF variants, a CAGRA-style graph). On an M3 Pro, SIFT1M (985,462 by 128): graph built in 9.6 s; 143,300 queries/s at 0.980 recall@10 in batches of 10,000, against hnswlib on 12 cores at 36,800 queries/s at 0.964. Single query: 0.45 to 0.47 ms median at 0.987 to 0.999 recall, of which about 0.15 ms is launch and synchronisation overhead; hnswlib on one thread 0.35 ms at 0.989. On 982,790 by 1,536: 14,400 queries/s at 0.979. | [S58], repository README read 2026-11-24, 2026 copyright, 43 commits, 4 stars | relayed; a small project's own figures, not independently reproduced |
| Apple's own MLX post gives memory bandwidth of 120 GB/s for the base M4 and 153 GB/s for the M5, and generation-speed ratios only, no absolute tokens per second. | [S59], 2025-11-19 | relayed |

What this gives the design (inferred from this table and section 2.1): below about 10^7 nodes an index is unnecessary on this machine; an exact scan fits the budget and has no recall loss, no build step and nothing to invalidate when nodes are added. An index earns its place from 10^8 upward. For a single query the GPU has no large advantage over one CPU thread on a graph index ([S58]: 0.45 ms against 0.35 ms), so the reason to keep selection on the GPU is to stay inside the model's own pass, not raw speed.

### 3.9 Hashing with locality

| Claim | Source | Status |
|---|---|---|
| Random-hyperplane hashing (SimHash): the hash is the sign of an inner product with a random Gaussian vector; it approximates cosine similarity. | [S60], STOC 2002 | relayed through a search summary |
| Two vectors at angle theta agree on one such bit with probability 1 - theta/pi. | standard result; not confirmed at the source today | assumed |
| DECKARD approximates syntax subtrees as characteristic vectors and clusters them with locality-sensitive hashing to find clones; a 2025 survey covers hierarchical locality-sensitive hashing for trees and graphs; no 2024 to 2025 paper was found that hashes Weisfeiler-Lehman relabelings of syntax trees for search. | [S61], search of 2026-11-24 | relayed through a search summary |

## 4. Answers for this design

Everything in this section is **inferred** from sections 1 to 3. Each answer names what it stands on.

### 4.1 What a node's embedding is computed from: three derived artifacts, not one

| Artifact | Computed from | Key | Trained? | Purpose |
|---|---|---|---|---|
| **Structural sketch** | Blueprint id and the children's sketches, bottom-up, with role vectors derived deterministically from content ids; integer or bipolar arithmetic; plus its sign bits as a 256-bit code | node content id (and the sketch recipe's id) | no | A locality code beside the content hash. Same in every runtime. Near-duplicate and same-shape detection, first-stage filter at 10^8 and above, cold-start neighbour for nodes no encoder has seen. |
| **Node embedding** | The name-free structure through a learned compositional encoder: f(blueprint embedding, child embeddings in order) | (node content id, encoder id) | yes, once, model-independent | The geometry a model selects in. One shared space for all models. |
| **Lensed embedding** | The node embedding plus the lens rows attached to the node (names, documentation, a claim's sentence) | (node content id, lens id, encoder id) | same encoder family | What the node means to a reader of that lens. Needed because most published gains in code models come from words and context ([S3], [S4]). |

Stands on: 5.2.2 and 5.2.4 (names are not in identity, so a name-informed vector cannot be keyed by content id alone); 3.1 (structure-only is cacheable but context-free); 3.2 and measurement 2.2 (a sketch is a fingerprint, not a copy); table 2 of [S21] (a hashed locality code recovers 31 to 75% of what a learned code gives over random).

Per model there is then a pair of small adapters, which is where the "like LoRA" property lives:

- an **in-projector** from the node space into that model's input embedding space, giving one or a few tokens per node (xRAG's form: two-layer MLP, under 0.5% of the model);
- an **out-projector** from a chosen hidden layer into the node space, giving a query (the tuned lens's form: affine).

A new node then needs no training at all: its embedding is one encoder call, and every model that has the two projectors can read it and select it. A new model needs two small projectors and no change to any node. Variants for comparison are different encoder ids or projector ids over the same node table, so two variants cost two caches, not two stores.

### 4.2 When it is computed

- The sketch at freeze, in the same pass that computes the content id (5.2.3). A sketch of width 256 over a node with a handful of children is a few thousand integer operations; the digest alone is 16 ns (`graph/3.15.kg:6`). Transient nodes get none unless asked.
- The node embedding lazily on first use, or eagerly at freeze when an encoder is bound. Because it is bottom-up, its cost per new node is one small composition over already-cached child vectors. For example a two-layer map from three width-256 inputs to one width-256 output is about 0.26 million multiply-adds, against 622 million for one unembedding-shaped projection: a node can be created and embedded for well under 1% of a token step (arithmetic from the shapes; not measured).
- Both are water or gas (5.6.4, 5.6.5): derived, droppable, never referenced from ice. The repository holds none of them.
- One hazard to settle before relying on the cache: floating-point results differ between CPU and GPU and between kernels, so either the key includes the generator and target (as compiled code does, 5.10.2), or embeddings are compared within a tolerance and never by bytes. The integer sketch does not have this problem.

### 4.3 How wide

Proposal: **width 256 in float16 as the node space, trained so that the first 64 dimensions stand alone, plus the 256 sign bits; an optional wider vector (1,024) only for re-ranking.**

- Cost: 10^6 nodes at width 256 float16 are 0.51 GB and scan in 2.5 to 2.9 ms on this GPU, under one unembedding-shaped scan at 3.4 ms (section 2.1). Width 64 carries the same budget to 10^7.
- Quality: nested widths lose little until they get very small ([S25]: 75.82 at width 64 against 76.80 at 2,048 on ImageNet top-1), and shortlist-then-re-rank recovers full-width accuracy. Sign bits keep about 92.5% of retrieval quality alone and 96.5% with re-scoring at 32 times less storage ([S26]). These are image and text results; whether node embeddings behave the same is untested.
- What width 256 cannot do: hold structure decodably (about four pairs, section 2.2). That is accepted; exact structure comes from the store.

### 4.4 Hidden state to node: selecting one among 10^6 to 10^9

One step, in order:

1. The model's ordinary output projection runs unchanged, with one or a few extra rows that mean "a node follows" (one per kind of pick if useful: run a recipe, read knowledge, apply a feature). This is ToolkenGPT's concatenated head reduced to a gate; its cost is a few rows on top of about 150,000.
2. Only when a gate row wins or passes a threshold: the out-projector maps the hidden state to a width-256 query (about 1 million multiply-adds for a 4,096-wide model).
3. The query is scored against the node table and the best node, or the top few for a choice, is returned. Unit-normalised vectors, so no node is unselectable.
4. If the pick is a recipe that takes arguments, the following steps pick or create argument nodes the same way, constrained to what the recipe's blueprint accepts. Constraint by blueprint is a mask on the scan and removes most of the table.

Step 3 by scale, on this machine:

| Nodes | Method | Cost | Basis |
|---|---|---|---|
| up to 10^6 | exact scan, width 256 float16 | 2.5 to 2.9 ms, under one unembedding | observed, 2.1 |
| 10^7 | exact scan at width 64, re-rank the top few hundred at width 256 | about 3 ms plus a negligible re-rank | observed for the scan; re-rank inferred |
| 10^8 | exact scan at width 64 (28 ms, 12.8 GB), or sign-bit scan plus re-rank (124 ms on one thread, 3.2 GB) | several unembeddings | observed |
| 10^9 | not a scan. A partitioned or graph index over compressed codes: coarse cells first, then a few cells exactly; or a disk-backed graph | published: 1 billion points, 64 GB RAM, under 3 ms mean, 95% recall@1 on 16 cores ([S57]); on Apple GPUs only million-scale figures exist ([S58]) | relayed; nothing measured here at this scale |

Generating a semantic id code by code ([S21], [S22]) is the wrong tool for this step: each code costs a full model step, so a 4-code id costs four tokens where a scan costs a fraction of one. Semantic ids are useful as the coarse cells of the 10^9 index, not as something the model spells.

What is not established: that one shared out-projector, trained once, lets a frozen model select correct nodes it never saw during projector training, among 10^6. Published results stop at 234 per-tool trained vectors (frozen model) and 47,000 tools (model changed). This is the central open risk.

### 4.5 Node to input: one or a few embeddings

Three resolutions, chosen per use:

| Resolution | Form | Cost in context | Evidence |
|---|---|---|---|
| Pointer with gist | in-projector applied to the (lensed) node embedding: 1 token | 1 position | [S35]: one token recovers 42.46 of 43.63 on Mistral-7B against 36.54 with nothing; weak on multi-hop |
| Multi-vector | the node's token plus tokens for its children, or a small resampler output | a handful of positions | [S33], [S27] to [S29]; untested for nodes |
| Exact | print the node through a lens into ordinary tokens | as long as the print | by construction (5.3.3) |

A fourth, heavier form exists for hot knowledge nodes: a trained per-(node set, model) key-value cache ([S38]: 38.6 times less memory than putting the text in context). It costs training per corpus, like an adapter, and belongs with the LoRA-style artifacts, not with embeddings.

A node placed in the input as one token is also immediately selectable again at the output, because both directions go through the same node vector. That closes the loop the request asks for: a node the model just created can be read and picked on the next step with no training.

### 4.6 The smallest experiment for each answer

Each is sized for one machine and one small frozen open-weight model, and each has a control that must fail.

| # | Question | Smallest experiment | Passes when | Control that must turn it red |
|---|---|---|---|---|
| E1 | Does the scan fit inside a real decode step? | Run the section 2.1 kernel in the same command buffer as a model's forward pass, 10^6 by 256 float16, every step. | Added time per token is at most the time of the model's own output projection. | Run it in a separate command buffer and confirm the roughly 2.5 ms floor reappears. |
| E2 | Is the structural sketch a useful locality code? | Parse a few thousand real definitions into nodes; sketch every subtree; for 200 queries compare sketch neighbours with neighbours by tree edit distance. | Top-10 overlap well above a bag-of-blueprints baseline. | Re-draw the role vectors per node (destroying consistency): overlap must fall to chance. |
| E3 | Does bottom-up caching hold? | Change one leaf in a tree of 10^5 nodes; count recomputed vectors. | Recomputed count equals the depth of the leaf; result equals a full recompute (bit-equal for the sketch, within tolerance for the float embedding). | Corrupt one cached child vector: the comparison must catch it. |
| E4 | Node to input with one token | The design graph itself is a ready corpus (1,796 nodes under 207 hubs, counted with `./kg brief` on 2026-11-24). Frozen text encoder, frozen small model, train only a two-layer projector on paraphrase ("this node says ..."), then ask which hub a held-out claim belongs to, given one token, the full text, or nothing. | One token recovers most of the gap between nothing and full text, as in [S35]. | Shuffle node-to-vector assignment at test time: accuracy must fall to the nothing baseline. |
| E5 | Hidden state to node with a shared projector | Same corpus and model. Train a gate row and an affine out-projector on (context, next node) pairs from 80% of nodes. Test on the held-out 20%, with 10^3 to 10^6 distractor vectors. | Recall@1 and recall@10 on never-seen nodes are far above chance and degrade gracefully with distractors. | Compare with per-node trained vectors (the ToolkenGPT form) on seen nodes, and with a random projector. |
| E6 | Width | Truncate the E5 node vectors to 64, 128 and sign bits; measure recall@10 of the true node against the width-256 exact result, with and without re-ranking. | Width 64 plus re-rank is within a point or two of width 256. | Random projection to the same width as the comparison floor. |
| E7 | Creation inline | In E5's setup let the model pick a blueprint and two children, mint the node, embed it by composition, and select it on the next step. | The freshly minted node is retrievable by a context that describes it. | Skip the embedding step: it must not be retrievable. |

E5 is the decisive one. If a shared projector does not generalise to unseen nodes, the design falls back to per-node trained vectors, and "create a node inline and use it at once" is no longer true.

## 5. Carry forward

1. Treat every vector as a derived artifact keyed by (node content id, generator id), in water or gas, exactly like compiled code. Identity stays the SHA-256 content id.
2. Keep three artifacts apart: a deterministic integer structural sketch with its sign bits; a learned name-free node embedding; a lens-keyed embedding that includes attached words.
3. Compute the sketch in the freeze pass. Compute learned embeddings bottom-up by composition so a new node costs one small map over cached child vectors.
4. One shared node space of width 256 float16 with a standalone 64-wide prefix and 256 sign bits.
5. Attach capability per model as two small projectors (in and out), not as per-node trained rows and not as changed model weights.
6. Select by a gate row in the ordinary output projection followed by an exact scan on the GPU, in the model's own pass; no index below about 10^7 nodes.
7. Unit-normalise node vectors.
8. Read a node at three resolutions: one projected token, a few tokens, or the exact print through a lens.
9. Use blueprint constraints as a mask on the scan.
10. Treat a node's embedding as a crossing of its interface: no embedding leaves for a node whose cell has not offered it.
11. Write the scan as a kernel generated in the grammar and reached through the GPU door (5.10.7); the library matrix path is not a substitute.

## 6. Let go

1. A content id with geometry, or any attempt to make the hash itself locality-sensitive. Keep the two codes side by side.
2. A single vector meant to be decoded back into a deep node. The capacity is about D/64 bound pairs.
3. Semantic ids as identity, or as something the model spells token by token to name a node.
4. Per-node trained output rows as the main mechanism (tested to 234 tools; needs training per node), and adding nodes to the model's vocabulary by fine-tuning (47,000 tools; changes the model).
5. The raw logit-lens reading (the model's own unembedding applied to the hidden state) as the query; use a trained affine map from a chosen layer.
6. An approximate index as the default. It adds build time, recall loss and invalidation to a problem a scan solves below 10^7.
7. The generic GPU matrix-library call for a single tall-thin query.
8. Embeddings in ice or in the repository.
9. A learned encoder whose output must be bit-identical across targets.
10. Name-informed vectors keyed by content id alone.

## 7. Open questions

1. Does a shared out-projector generalise to nodes unseen in its training, at 10^6 distractors? (E5; nothing published settles it.)
2. How are a token logit and a node score put on one scale? ToolkenGPT concatenates logits; kNN-LM interpolates probabilities with a fixed weight. A node pick that is wrong costs more than a token that is wrong.
3. What trains the compositional encoder, and on what? No corpus of nodes of this space exists yet. Candidates: contrastive pairs from one program read through two host lenses (stage 7.8), from the shape-with-holes fingerprint (5.2.5), and from recipes with equal observed behaviour.
4. Is a subtree-only embedding good enough for recipes, or must use-sites and data flow enter, which would break caching by content id?
5. Which hidden layer is the best query, and does it differ by pick kind?
6. Which integer sketch: bipolar element-wise binding with permutation for child order, or sparse binary (the most efficient per [S9]), and at what width?
7. When an encoder is replaced, all embeddings under the old encoder id are stale and both projectors per model are too. What is the migration cost at 10^8 nodes, and can a linear map between old and new spaces bridge it?
8. Does placing many node tokens in one context degrade a frozen model (xRAG tested one document per query)?
9. At 10^9: is the index a recipe-compiled kernel on the GPU, or a CPU-side graph on memory-mapped ice? No Apple-GPU figure above about 10^6 was found.
10. Does the roughly 2.5 ms floor of a separate GPU launch vanish when the scan is encoded in the model's own pass? (E1.)
11. How is breach made observable for embeddings: can a cell prove which of its nodes have had vectors computed and by whom?

## 8. Sources (all fetched 2026-11-24)

| # | Source | Published |
|---|---|---|
| S1 | https://arxiv.org/abs/1503.00075 (Tree-LSTM) | 2015-02-28 |
| S2 | https://arxiv.org/abs/1803.09473 (code2vec) | 2018-03-26 |
| S3 | https://arxiv.org/abs/2009.08366 (GraphCodeBERT) | 2020-09-17 |
| S4 | https://arxiv.org/abs/2203.03850 (UniXcoder) | 2022-03-08 |
| S5 | https://papers.nips.cc/paper/2019/hash/6e0917469214d8fbd8c517dcdc6b8dcf-Abstract.html (tree positional encodings), via search | NeurIPS 2019 |
| S6 | https://arxiv.org/abs/2106.05234 (Graphormer) | 2021-06-09 |
| S7 | https://arxiv.org/abs/2402.05862 (GraphToken) | 2024-02-08 |
| S8 | Web search "embedding cache keyed by content hash Merkle tree AST subtree"; hits included https://cursor.com/blog/secure-codebase-indexing and docs.rs pages of three indexers | undated pages |
| S9 | https://arxiv.org/pdf/2001.11797 (A Comparison of Vector Symbolic Architectures; accepted at Artificial Intelligence Review, DOI 10.1007/s10462-021-10110-3). Text extracted to `.../scratchpad/study/emb-work/pdf/vsa-comparison.txt` | arXiv 2020-01; journal 2021 |
| S10 | https://arxiv.org/abs/1707.01429 (superposition theory) | 2017-07-05 |
| S11 | https://arxiv.org/abs/2007.03748 (resonator networks, part 1) | 2020-07-07 |
| S12 | https://arxiv.org/abs/1906.11684 (resonator networks, part 2) | 2019-06-19 |
| S13 | https://arxiv.org/abs/2109.02157 (Learning with HRR) | 2021-09-05 |
| S14 | https://arxiv.org/abs/2305.19534 (HRR self-attention) | 2023-05-31 |
| S15 | https://arxiv.org/abs/2412.07947 (GPT-2 through the lens of VSA), via search | 2024-12-10 |
| S16 | https://arxiv.org/html/2509.25045v2 (Hyperdimensional Probe) | 2025-09-29; v2 2025-12-02 |
| S17 | https://arxiv.org/abs/2512.14709 (Attention as Binding) | 2025-12-08 |
| S18 | https://hal.archives-ouvertes.fr/file/index/docid/825085/filename/jegou_pq_postprint.pdf (product quantisation), via search | TPAMI 2011 |
| S19 | https://openaccess.thecvf.com/content_cvpr_2013/html/Ge_Optimized_Product_Quantization_2013_CVPR_paper.html, via search | CVPR 2013 |
| S20 | https://arxiv.org/abs/2203.01941 (RQ-VAE) | 2022-03-03 |
| S21 | https://arxiv.org/html/2305.05065 (TIGER) | 2023-05-08 |
| S22 | https://arxiv.org/abs/2202.06991 (DSI) | 2022-02-14 |
| S23 | Web search on semantic-id codebook collapse; hits https://arxiv.org/html/2606.01844v1, https://arxiv.org/html/2604.03949v1, https://arxiv.org/pdf/2407.21488 | 2024 to 2026 |
| S24 | https://arxiv.org/abs/2610.28568 (Evolving Semantic Indexes) | 2026-10-20 |
| S25 | https://arxiv.org/html/2205.13147 (Matryoshka) | 2022-05-26 |
| S26 | https://huggingface.co/blog/embedding-quantization | 2024-03-22 |
| S27 | https://arxiv.org/abs/2004.12832 (ColBERT) | 2020-04-27 |
| S28 | https://arxiv.org/abs/2112.01488 (ColBERTv2) | 2021-12-02 |
| S29 | https://arxiv.org/abs/2405.19504 (MUVERA) | 2024-05-29; v2 2026-06-08 |
| S30 | https://arxiv.org/html/2304.08485 (LLaVA) | 2023-04-17 |
| S31 | https://arxiv.org/abs/2310.03744 (LLaVA-1.5) | 2023-10-05 |
| S32 | https://arxiv.org/abs/2209.15162 (LiMBeR) | 2022-09-30 |
| S33 | https://arxiv.org/abs/2301.12597 (BLIP-2) | 2023-01-30 |
| S34 | https://arxiv.org/abs/2204.14198 (Flamingo) | 2022-04-29 |
| S35 | https://arxiv.org/html/2405.13792v2 (xRAG) | 2024-05; v2 2024-12-09 |
| S36 | https://arxiv.org/abs/2304.08467 (gist tokens) | 2023-04-17 |
| S37 | https://arxiv.org/abs/2307.06945 (ICAE) | 2023-07-13 |
| S38 | https://arxiv.org/abs/2506.06266 (Cartridges), via search | 2025-06-06 |
| S39 | https://arxiv.org/abs/2310.15213 (function vectors) | 2023-10-23 |
| S40 | https://arxiv.org/abs/2608.17050 (Frozen Memory Is Not Enough) | 2026-08-17; v6 2026-11-01 |
| S41 | https://ar5iv.labs.arxiv.org/html/2305.11554 and https://arxiv.org/abs/2305.11554 (ToolkenGPT) | 2023-05-19 |
| S42 | https://arxiv.org/abs/2410.03439 (ToolGen) | 2024-10-04 |
| S43 | https://ar5iv.labs.arxiv.org/html/1911.00172 and https://arxiv.org/abs/1911.00172 (kNN-LM) | 2019-11-01 |
| S44 | https://arxiv.org/abs/2307.06962 (Copy is All You Need) | 2023-07-13 |
| S45 | https://arxiv.org/abs/1806.04189 (FGD) | 2018-06-11 |
| S46 | https://arxiv.org/abs/2608.27460 (vector-index output embeddings) | 2026-07-01 |
| S47 | https://arxiv.org/abs/2303.08112 (tuned lens) | 2023-03-14; v6 2025-11-11 |
| S48 | https://arxiv.org/abs/2401.06102 (Patchscopes) | 2024-01-11 |
| S49 | https://arxiv.org/abs/2203.06462 (unargmaxable classes) | 2022-03-12 |
| S50 | https://arxiv.org/abs/2310.06816 (vec2text) | 2023-10-10 |
| S51 | https://arxiv.org/abs/2404.05961 (LLM2Vec) | 2024-04-09 |
| S52 | https://arxiv.org/abs/1702.08734 (FAISS GPU) | 2017-02-28 |
| S53 | https://github.com/facebookresearch/faiss/wiki/Faiss-on-the-GPU | undated wiki |
| S54 | https://arxiv.org/abs/2308.15136 (CAGRA) | 2023-08-29 |
| S55 | https://arxiv.org/abs/1603.09320 (HNSW) | 2016-03-30 |
| S56 | https://arxiv.org/abs/2404.00774 (SOAR) | 2024-03-31 |
| S57 | https://papers.nips.cc/paper/2019/hash/09853c7fb1d3f8ee67a61b6bf4a7f8e6-Abstract.html (DiskANN), via search | NeurIPS 2019 |
| S58 | https://github.com/mikepapadim/mlxvs (mlxVS README) and https://pypi.org/project/mlxvs/ | 2026 |
| S59 | https://machinelearning.apple.com/research/exploring-llms-mlx-m5 | 2025-11-19 |
| S60 | https://www.cs.princeton.edu/courses/archive/spr04/cos598B/bib/CharikarEstim.pdf (SimHash), via search | STOC 2002 |
| S61 | Web search on tree and graph locality-sensitive hashing; hits https://www.cs.ucdavis.edu/~su/publications/icse07.pdf (DECKARD, ICSE 2007) and https://arxiv.org/pdf/2204.11209v4 (hierarchical LSH survey) | 2007; 2022 to 2025 |

Not found or not confirmed: the PyPI page for mlxVS did not load; a FAISS Metal backend (a search returned nothing on it; "no Metal backend exists" is assumed, the wiki only says "GPU support is via CUDA"); Plate's original capacity formula (cited only through [S9]); absolute tokens-per-second for any model on this machine.
