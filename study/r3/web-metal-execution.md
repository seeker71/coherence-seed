# Metal execution: what "about the budget of a next-token step" means on the first machine

Study round of 2026-11-24. Question: a language model runs on the GPU (Apple Metal first). Each step picks a token today. What can share that step (score extra rows that stand for nodes, take node embeddings as input, apply a low-rank update, read a hidden vector back, query a table of vectors, run several branches), and what does each cost against the step itself?

Statuses used: **observed** (I read the file or ran the command today), **relayed** (read through a web fetch, which summarises pages, or read from a dated note of an earlier session), **inferred** (my reasoning, with what it rests on), **assumed** (not verified). All web pages were fetched on 2026-11-24; the source's own date is given beside each.

Limits that apply to the whole report:

- Web fetches return a summary of the page, not the page. Numbers quoted from them are relayed.
- Every measurement I made on the first machine ran while other work held the CPU and the GPU (one process at 100% CPU, several old-runtime processes resident, one of 20 GB). They are contended readings; the minimum over rounds is the usable figure and even that is a lower bound on what a quiet machine does. One machine, two runs.
- The old runtime was not run. Its figures come from dated notes of earlier sessions and are relayed.
- Encoders, embedding training and the question of how a node gets a useful vector are outside this report; it covers execution cost only.

---

## 1. The first machine

| Fact | Value | Source | Status |
|---|---|---|---|
| Chip | Apple M4 Max, 16 CPU cores (12 performance, 4 efficiency), 40 GPU cores | `sysctl -n machdep.cpu.brand_string hw.ncpu hw.perflevel0.physicalcpu hw.perflevel1.physicalcpu`; `system_profiler SPDisplaysDataType` run 2026-11-24 | observed |
| Memory | 137,438,953,472 bytes (128 GiB), unified | `sysctl -n hw.memsize`; probe line `unified=1` in `scratch/study/metal-probe/run-1.log:1` | observed |
| Metal | "Metal Support: Metal 4"; `maxBufferLength` 80.6 GiB; `recommendedMaxWorkingSetSize` 107.5 GiB; `threadExecutionWidth` 32 | `system_profiler`; `run-1.log:1-2` | observed |
| OS | macOS 26.3.1 (a), build 25D771280a | `sw_vers` | observed |
| Matrix instructions | `hw.optional.arm.FEAT_SME: 1`, `FEAT_SME2: 1` | `sysctl` | observed |
| Stated memory bandwidth | "up to 546GB/s" for M4 Max | Apple newsroom, "Apple introduces M4 Pro and M4 Max", https://www.apple.com/newsroom/2024/10/apple-introduces-m4-pro-and-m4-max/ (2024-10-30), read through a search summary | relayed |
| What the graph already says | M4 Max, 16 KiB pages, SME and SME2; Metal compiles shader source at run time; Metal reached from about 20 lines of C at 5.9 ns a message; "nothing was run on a GPU" | `coherence-seed/graph/4.2.kg:238-282`, `coherence-seed/graph/4.1.kg:216-224`, `coherence-seed/graph/5.kg:764-772` | observed |

The design graph's GPU node (5.10.7) has confidence low and the limit "nothing here was measured" (`coherence-seed/graph/5.kg:770-771`). Sections 5, 6 and 9 below are the first GPU measurements made for the new repository.

One consequence of the OS version: Apple's tech talk lists cooperative tensors as matmul inputs in 26.3 and 4-bit and 8-bit integer tensors in 26.4 (section 4). The first machine is on 26.3.1, so the quantised tensor types are one update away. Relayed plus observed; inferred conclusion.

---

## 2. What a decode step costs, and why

### 2.1 The limit is bytes of weights read per token

- Apple, MLX on M5 (2025-11-19): "Generating subsequent tokens is bounded by memory bandwidth, rather than by compute ability." M4 at 120 GB/s against M5 at 153 GB/s gives a 19 to 27% generation gain, while time to first token gains 3.3 to 4.1 times from the GPU's neural accelerators. Prompt 4096 tokens, 128 generated, `mlx_lm.generate`, 24 GB MacBook Pro M5. https://machinelearning.apple.com/research/exploring-llms-mlx-m5. Relayed.
- Apple tech talk 111432 (undated, 2026 copyright): decode "generates one token at a time using tall, skinny matrices. It is memory bound because the full weights must be read for each token"; prefill is compute bound. https://developer.apple.com/videos/play/tech-talks/111432/. Relayed.
- Apple, Core ML Llama 3.1 (2024-11-01): such models are "usually constrained by memory bandwidth"; the GPU has the best mix of compute and bandwidth. https://machinelearning.apple.com/research/core-ml-on-device-llama. Relayed.
- WWDC26 session 330: "The inference stage is typically memory bandwidth bound, so compressing the weights becomes necessary." https://developer.apple.com/videos/play/wwdc2026/330/. Relayed.

### 2.2 Measured generation speed against bandwidth (llama.cpp)

Source: llama.cpp discussion 4167, "Performance of llama.cpp on Apple Silicon M-series", https://github.com/ggml-org/llama.cpp/discussions/4167, instructions "updated as of 2026 Aug 25". Model Llama-2-7B (6.74 B parameters): F16 12.55 GiB, Q8_0 6.67 GiB, Q4_0 3.56 GiB. `llama-bench -p 512 -n 128 -ngl 99`. Relayed. The last two columns are derived by me: file bytes times tokens per second, and that over the stated bandwidth.

| Chip (GPU cores) | Stated GB/s | Format | PP t/s | TG t/s | ms per token | Bytes streamed GB/s | Share of bandwidth |
|---|---|---|---|---|---|---|---|
| M4 (10) | 120 | Q4_0 | 221.29 | 24.11 | 41.5 | 92 | 77% |
| M4 Pro (20) | 273 | Q4_0 | 439.78 | 50.74 | 19.7 | 194 | 71% |
| **M4 Max (40)** | **546** | **F16** | 922.83 | **31.64** | 31.61 | 426 | 78% |
| **M4 Max (40)** | **546** | **Q8_0** | 891.94 | **54.05** | 18.50 | 387 | 71% |
| **M4 Max (40)** | **546** | **Q4_0** | 885.68 | **83.06** | 12.04 | 318 | 58% |
| M2 Ultra (76) | 800 | Q4_0 | 1238.48 | 94.27 | 10.6 | 360 | 45% |
| M5 Max (40) | 614 | Q4_0 | 3219.99 | 119.92 | 8.3 | 458 | 75% |
| M5 Max (40) | 614 | F16 | 3158.49 | 37.11 | 26.9 | 500 | 81% |
| M5 Ultra (80) | 1228 | Q4_0 | 4944.66 | 179.10 | 5.6 | 685 | 56% |

Derived from the three M4 Max rows: a straight line through F16 and Q4_0 gives **T = 4.3 ms + 2.03 ms per GB of weights** (a marginal 493 GB/s, 90% of the stated 546) and predicts the Q8_0 row at 18.8 ms against the 18.5 ms measured. Status: derived. Limit: three points, three formats whose per-byte arithmetic differs, one model.

Reading (inferred from the table): the smaller the model, the more a fixed cost of a few milliseconds a token shows. At 12 ms a token the fixed part is about a third.

### 2.3 Models of 20 to 30 billion parameters

- M2 Ultra, llama.cpp build b828e18c7 (7948), Darwin 25.2.0 (kernel dated 2025-11-18), `llama-bench` tg32: gpt-oss-20b MXFP4 (11.27 GiB, mixture of experts) 129.97 t/s; Qwen3-Coder-30B-A3B Q8_0 (30.25 GiB, mixture of experts, about 3 B active) 78.97 t/s; GLM-4.7-Flash Q8_0 (29.65 GiB) 59.58 t/s; Qwen2.5-Coder-7B Q8_0 (7.54 GiB, dense) 79.68 t/s; gemma-3-4b Q4_0 (2.35 GiB) 134.28 t/s. https://raw.githubusercontent.com/ggml-org/llama.cpp/master/benches/mac-m2-ultra/mac-m2-ultra.md. Relayed. A mixture-of-experts model reads only its active experts per token, so its file size overstates the bytes streamed (inferred).
- **First machine, dense 27 B at Q8_0 (about 29 GB), relayed from the earlier sessions' notes** (`memory/project-qwen-speed-vs-llamacpp-2026-10-04.md`, lines 13, 21 and 31, written 2026-10-04 to 2026-10-07):
  - llama.cpp `llama-bench -ngl 99 -p 64,512 -n 32`, quiet: tg32 9.27 t/s, later 11.18 +/- 0.94 t/s; pp512 115.6 to 131.7 t/s. With another model generating on the GPU: tg32 2.15 t/s.
  - The old body's own decode: 5.08 t/s at 1,356 dispatches a token, then 11.16 to 11.57 t/s (324 to 336 GB/s) after two changes, then "plain decode 63 ms/token = 15.6 tok/s" in a clean window.
  - 63 ms for about 29 GB is 460 GB/s, 84% of the stated 546 (derived). The line fitted in 2.2 predicts 4.3 + 29 x 2.03 = 63.1 ms for the same model (derived). Two independent readings agree; I did not expect that, and one clean window is thin evidence.
- **Dense 27 B at 4 bits on the first machine: no measured figure found.** The fitted line gives about 4.3 + 16.5 x 2.03 = 38 ms, 26 t/s (derived, assuming about 16.5 GB of weights). A forum table cited through a search summary gives 33.9 t/s for Gemma-3 27B on an M3 Ultra with the metric undefined; I do not rely on it.
- A press test reports the M4 Max (546 GB/s) generating 27 to 82% more tokens a second than a 273 GB/s machine with llama.cpp, under the heading "memory bandwidth isn't everything" (Tom's Hardware; article body not reachable, read through a search summary only). Relayed, weak.

### 2.4 Two properties of this GPU that change every reading

Both from the earlier sessions' notes on the first machine, relayed:

- **Sustained load slows the GPU about 2.6 times.** A fixed 128x3072x3072 product reads 27.4 to 29.1 ms at rest, 71 to 76 ms after 30 s of continuous work, 28.7 ms after 3 s of rest; decode at 20.7 ms a token in a burst averages 50.8 ms over 400 tokens (`memory/reference-decode-submit-per-layer-and-reading-variance.md:13`, 2026-10-13). A fixed multiply-add loop falls from 12.7 to 4 to 5 TFLOP/s in two seconds (`.../project-qwen-speed-vs-llamacpp-2026-10-04.md:29`).
- **Another GPU client makes a reading wrong by up to 4 times** (`.../project-qwen-speed-vs-llamacpp-2026-10-04.md:13`), and windows read 2 to 12 times slow (`.../project-spec-verify-matrix-unit-2026-10-07.md:21`). My probe saw the same thing: completion times of a small command buffer cluster at about 1, 2, 3, 4 and 6 ms when another client is active (`metal-probe/run-1.log:10-18`, `run-2.log:10-18`). Observed; that this is time slicing in roughly 1 ms quanta is inferred.

So a budget promised in milliseconds is a burst figure on a quiet machine. A budget promised as a share of the step (bytes read, dispatches, synchronisations) survives both effects better.

---

## 3. How a step is submitted today

### 3.1 llama.cpp Metal backend

Source: `ggml/src/ggml-metal/ggml-metal-context.m` on master, https://raw.githubusercontent.com/ggml-org/llama.cpp/master/ggml/src/ggml-metal/ggml-metal-context.m. Relayed (read through a fetch).

- `GGML_METAL_MAX_COMMAND_BUFFERS` is 8; a request above 2 logs that it "is not recommended and can degrade the performance".
- The main thread encodes the first `MAX(64, 0.1 * n_nodes)` nodes ("empirically determined") into its own command buffer; the rest are split across `n_cb` buffers encoded in parallel with `dispatch_apply` on a concurrent queue. Buffers are created with `commandBufferWithUnretainedReferences`, enqueued, and committed as soon as encoded. A fused group of nodes never spans two buffers.
- The graph call returns without waiting; `ggml_metal_synchronize` waits on the last buffer. So a token is 1 to 3 command buffers and one wait.
- Only `MTLResourceStorageModeShared` appears. Reading a tensor back wraps the caller's pointer with `newBufferWithBytesNoCopy` and issues a blit, queued until the next synchronise.
- Flags: concurrency (on unless `GGML_METAL_CONCURRENCY_DISABLE`), graph optimisation and op fusion (on by default), residency sets kept alive per graph.

Metal 4 in llama.cpp: PR 16634 "metal : initial Metal4 tensor API support", opened 2025-10-17, merged 2025-11-06. The tensor path is enabled on M5 and later only; "on an M2 Ultra, the tensor path was slightly slower". On M5: Mistral 7B Q4_0 pp512 252.82 to 608.05 t/s; Llama 3.1 8B Q4_0 pp512 257.25 to 609.26 and tg128 22.56 to 22.88. Pipeline creation for tensor ops "can take one to two seconds each". https://github.com/ggml-org/llama.cpp/pull/16634. Relayed. Reading: the new API speeds the compute-bound stage and leaves generation where bandwidth put it.

### 3.2 The old body on the first machine (relayed from notes)

- 621 dispatches a token (a 3 B model) in one batch: host encode about 12 ms plus device 16.5 ms added to 28.3 ms. Committing after each layer without waiting let host and device overlap: 20.7 ms (`.../reference-decode-submit-per-layer-and-reading-variance.md:11`, 2026-10-12).
- 27 B model: 1,265 to 1,356 dispatches a token; host encode 44 ms and device wait 405 ms at first ("the GPU kernels are the cost"); a hand-over every 2 layers; host encode later cut from 85 to 24 ms by building dispatch records once (`.../project-qwen-speed-vs-llamacpp-2026-10-04.md:19,21,29`).
- "A tiny dependent dispatch costs only ~20 us" (same file, line 21). 32 command buffers a token (`.../project-spec-verify-matrix-unit-2026-10-07.md:21`).
- A root cause worth carrying: at width 5120 a normalisation ran on **one GPU thread**, 1.1 to 2.2 ms each, 129 a token; a threadgroup version took 41 us and decode went from 5.08 to 9.09 t/s (`.../project-qwen-speed-vs-llamacpp-2026-10-04.md:21`).

### 3.3 MLX

- Lazy evaluation: operations record a graph; `eval()` runs it; printing, `item()`, conversion to NumPy and use in control flow evaluate implicitly. "Anything from a few tens of operations to many thousands of operations per evaluation should be okay." MLX 0.32.4 docs, https://ml-explore.github.io/mlx/build/html/usage/lazy_evaluation.html. Relayed.
- Unified memory: "The CPU and GPU have direct access to the same memory pool"; the device is chosen per operation (`stream=mx.cpu` or `mx.gpu`) with no copy; a matmul on the GPU with 500 small ops on the CPU ran in about 1.4 ms against 2.8 ms all on the GPU (M1 Max). https://ml-explore.github.io/mlx/build/html/usage/unified_memory.html. Relayed. This is published evidence that small sequential work belongs on the CPU.
- Custom kernels: `mx.fast.metal_kernel` takes the body of a Metal kernel as a string, generates the signature, builds a library (JIT compiled; "build the kernel once and reuse"); an example went from 55.7 to 6.7 ms on an M1 Max. https://ml-explore.github.io/mlx/build/html/dev/custom_metal_kernels.html. Relayed.
- Adapters and caches in mlx-lm: fine-tune types `lora`, `dora`, `full`; adapters are applied at generation with `--adapter-path` without fusing, or fused with `mlx_lm.fuse`; training about 250 tokens a second on an M1 Max 32 GB (Mistral 7B, batch 1, 4 layers). Prompt caches are saved to `.safetensors` and reused as a prefix; a rotating fixed-size KV cache with `--max-kv-size`; samplers and logits processors are Python callables over the logits. https://raw.githubusercontent.com/ml-explore/mlx-lm/main/mlx_lm/LORA.md and https://raw.githubusercontent.com/ml-explore/mlx-lm/main/README.md (undated). Relayed.
- A server on MLX (arXiv 2601.19139, 2026-01-27, M4 Max): 21 to 87% higher throughput than llama.cpp from Qwen3-0.6B to a 30 B model, up to 525 t/s, and "up to 4.3x aggregate throughput at 16 concurrent requests" from continuous batching. https://arxiv.org/abs/2601.19139. Relayed (abstract).
- A third-party runtime with hand-written Metal 4 kernels (arXiv 2607.19438, 2026-07-21, M5 Pro): prefill up to 6.4 times llama.cpp and 3.9 times MLX; decode up to 1.75 times llama.cpp and 1.33 times MLX on a "memory-bound decode path". https://arxiv.org/abs/2607.19438. Relayed (abstract).

### 3.4 Core ML, MPSGraph and the Neural Engine

- Llama-3.1-8B on an M1 Max GPU through Core ML (2024-11-01): 0.19 t/s with no KV cache; 1.25 t/s with the cache passed as model input and output; **16.26 t/s with the cache as in-place state**; 33.67 t/s with block-wise 4-bit weights (4.2 GB). Block-wise quantisation suits the GPU; palettisation suits the Neural Engine. https://machinelearning.apple.com/research/core-ml-on-device-llama. Relayed. The 13-fold step from "state crosses the boundary each token" to "state stays in place" is the lesson: nothing that persists across steps may be copied per step.
- MPSGraph (WWDC24 session 10218): a fused scaled-dot-product attention op, in-place slice update for the KV cache, 4-bit and 8-bit affine and lookup-table weights dequantised inside the matmul, and adapters as separately compiled graphs called from the main graph and bound when the main graph is compiled. https://developer.apple.com/videos/play/wwdc2024/10218/. Relayed. Whether adapter weights can change without recompiling is not stated.

---

## 4. Metal 4 and the machine-learning additions (2025 to 2026)

- **MTLTensor** (WWDC25 session 262): a resource with rank, extents, data type and usage (`MTLTensorUsageMachineLearning`, `Compute`, `Render`). Made by the device (opaque layout, "best performance") or over an `MTLBuffer` with explicit strides. https://developer.apple.com/videos/play/wwdc2025/262/. Relayed.
- **MTL4MachineLearningCommandEncoder**: "Encodes machine learning model inference commands for a single pass"; methods `setPipelineState`, `setArgumentTable`, `dispatchNetwork(intermediatesHeap:)`; macOS 26.0+, iOS 26.0+. https://developer.apple.com/tutorials/data/documentation/metal/mtl4machinelearningcommandencoder.json. The network is a Core ML ML-program exported offline and converted by `metal-package-builder` to an `.mtlpackage`; it runs on the GPU timeline and is ordered against other passes with barriers on `MTLStageMachineLearning` (session 262). Relayed.
- **Shader ML and Metal Performance Primitives**: `#include <MetalPerformancePrimitives/MetalPerformancePrimitives.h>`; `mpp::tensor_ops::matmul2d<descriptor, scope>` runs inside any shader stage at thread, simdgroup or threadgroup scope; tensors bound as buffers or built inline. (Session 262; the MPP Programming Guide, Version 1, dated 2026-03-16, https://developer.apple.com/download/files/Metal-Performance-Primitives-Programming-Guide.pdf, 13 pages, which I read as a PDF: observed.)
  - The guide's own advice: threadgroup staging "isn't necessary for the most optimized GEMM kernels"; start at a 2x2 tile of simdgroups, simdgroup tile 32x32 for 16-bit operands, a barrier every 128 along K, Morton order over threadgroups; **cooperative tensors** keep a product in thread-local memory so an activation or `alpha*(A@B)+beta*C` is fused before one store. Arithmetic intensity is `M*N/(M+N)` operations per input element: tall, thin products are memory bound, square ones compute bound (guide pages 6 to 12). Observed.
- **Neural accelerators**: "a new hardware block in M5, located directly in each shader core ... designed to accelerate dense compute-bound work such as the prefill stage of an LLM" (WWDC26 session 330). Tech talk 111432: matrix multiply "up to 4-8x faster, depending on precision"; time to first token up to 4 times; generation up to 25%. TensorOps is portable from M1 to M5 and "falls back to optimized shader code" without the accelerators. Relayed. **The first machine is an M4 Max and has none.**
- **Quantised tensors**: bfloat in 26.1, cooperative tensors as matmul inputs in 26.3, 4-bit and 8-bit integer tensors in 26.4 (tech talk 111432); in macOS 27, 4-bit and 8-bit floating types, 2-bit integers, and a scale plane on the same tensor with a block factor (every 32 elements share one scale; FP8 E8M0) (WWDC26 session 330; "What's new in Metal", https://developer.apple.com/metal/whats-new/). Relayed.

Reading for this design (inferred from the above): the ML command encoder takes a network converted offline by Apple's tools, which a system that generates its own kernels at run time cannot use as its main road. The fit is the shader-level API: kernels written as source, compiled at run time through the door the graph already names (`coherence-seed/graph/4.2.kg:274-282`), calling `matmul2d` where a product is large. That path costs nothing extra on M4 and gains 4 to 8 times on the compute-bound parts on M5.

---

## 5. Unified memory, storage modes, submission and readback

Documentation (relayed):

- `MTLStorageMode.shared`: "The CPU and GPU share access to the resource, allocated in system memory." Default for buffers and textures on Apple silicon. "You're responsible for synchronizing access": all scheduled changes on one side must complete before the other reads. https://developer.apple.com/tutorials/data/documentation/metal/mtlstoragemode/shared.json.
- `makeBuffer(bytesNoCopy:length:options:deallocator:)`: "Creates a buffer that wraps an existing contiguous memory allocation"; page-aligned pointer, length giving a page-aligned region, one VM region. https://developer.apple.com/tutorials/data/documentation/metal/mtldevice/makebuffer(bytesnocopy:length:options:deallocator:).json. So a mapped file of frozen node bytes (the design's ice store, `coherence-seed/graph/5.kg:494-502`) can be the GPU's buffer with no copy, provided it is page-aligned at 16 KiB (inferred).
- MPS tuning hints: "There can be a significant delay (up to 2.5 ms) just to get an empty command buffer through the pipeline to where the waitUntilCompleted() method returns"; overlap encoding with execution for "up to a factor of ten"; allocate resources once; "do more work with fewer command buffers". https://developer.apple.com/documentation/metalperformanceshaders/tuning-hints.md (2026 copyright). A forum post of November 2024 reports 0.12 ms for an empty kernel with `waitUntilCompleted` on an M2 Max, unanswered (https://developer.apple.com/forums/thread/767853).

Measured today on the first machine, contended (probe source `scratch/study/metal-probe/probe.m`; logs `run-1.log` and `run-2.log` beside it; classic `MTLCommandQueue`, not the Metal 4 queue). Observed:

| What | Run 1 | Run 2 |
|---|---|---|
| Compile 2 kernels from source (`newLibraryWithSource`) | 31.6 ms | 0.76 ms |
| Two pipeline states | 19.3 + 5.7 ms | 0.98 + 0.04 ms |
| Round trip, 1 tiny dispatch, back to back, n=300: encode | min 2.4, median 3.1 us | min 2.3, median 2.8 us |
| ... commit + `waitUntilCompleted` | min 76.5, median 105.7, p90 995, max 3,888 us | min 60.7, median 115.0, p90 425, max 51,916 us |
| ... `GPUEndTime - GPUStartTime` | min 3.3, median 4.4 us | min 2.4, median 2.9 us |
| Round trip after 20 ms idle, n=40: commit + wait | min 136, median 654, p90 1,931 us | min 118, median 557, p90 3,118 us |
| 1,024 dispatches (same pipeline and bindings, grid 5,120) in one buffer: encode | min 90 us (0.09 us each) | min 58 us (0.06 us each) |
| ... GPU time | min 2.01 ms (2.0 us each); median 112 ms | min 1.78 ms (1.7 us each); median 60 ms |
| `memcpy` of 5,120 floats out of `buffer.contents` | below 0.05 us | below 0.05 us |

Readings:

- A round trip to the CPU costs 60 to 115 us at best on this machine and 0.5 to 3 ms when the GPU has gone idle or another client holds it. Apple's 2.5 ms and the forum's 0.12 ms are both reproduced. Observed.
- The second run's compile at 0.76 ms against 31.6 ms is, I infer, a system shader cache hit; the cold figure is the one a newly generated kernel pays.
- The copy of a hidden vector is free. The synchronisation is the whole cost. Observed.
- Encoding in C is 0.06 to 0.09 us a dispatch when nothing is rebound. The old body's 24 to 85 ms of host encode for about 1,300 dispatches was 19 to 65 us each (derived from the notes): its cost was the interpreter, not Metal.

---

## 6. The CPU's matrix instructions as a co-processor

Sources (relayed):

- "Hello SME!" (arXiv 2409.18779, 2024-09-27): M4 is the first chip with SME; "over 2.3 FP32 TFLOPS"; JIT-generated small matrix kernels beat the vendor BLAS "in almost all tested configurations". https://arxiv.org/abs/2409.18779. Already in the graph (`coherence-seed/graph/4.2.kg:265-272`).
- "Demystifying ARM SME to Optimize General Matrix Multiplications" (arXiv 2512.21473, 2025-12-25, M4 Pro): average 1.23 times Apple Accelerate. https://arxiv.org/abs/2512.21473.
- Arm's learning path states llama.cpp dispatches SME2 kernels through KleidiAI when available, and one Arm benchmark reports +53.6% one-thread decode on an M4 Pro for a different framework (search summary only; weak).

Measured today, contended, Accelerate `cblas_sgemv` in fp32 and a plain summing loop (`metal-probe/run-1.log`, `run-2.log`). Observed:

| What | Run 1 | Run 2 |
|---|---|---|
| CPU read, 2.048 GB, 1 thread | 56.4 GB/s | 77.7 GB/s |
| CPU read, 4 / 8 / 16 chunks | 198 / 212 / 213 GB/s | 246 / 261 / 276 GB/s |
| `sgemv` 16 x 5,120 (328 KB) | 1.7 us | 0.8 us |
| `sgemv` 256 x 5,120 (5.2 MB) | 28.3 us | 14.2 us |
| `sgemv` 4,096 x 5,120 (84 MB) | 639 us | 438 us |
| `sgemv` 65,536 x 5,120 (1.34 GB) | 10.7 ms | 7.6 ms |
| `sgemv` 500,000 x 1,024 (2.05 GB) | 14.0 ms | 9.6 ms |
| Rank-16 pair at width 5,120 (down then up), hot | 19.9 us | 11.6 us |
| 128 distinct rank-16 pairs (84 MB), 1 thread | 3.03 ms | 1.88 ms |

Whether Accelerate used SME for these calls was not determined. The up-projection (5,120 rows of 16) is the slow half of the pair in this form; a column-oriented kernel would be faster (inferred, not measured).

The comparison that matters is in section 9: the same 16 x 5,120 product costs 129 to 136 us on the GPU in a one-thread-a-row kernel and 0.8 to 1.7 us on the CPU.

---

## 7. Sharing a step: branches, adapters, the KV cache

### 7.1 Several sequences in one pass (measured by a source)

`llama-batched-bench` on an M2 Ultra, prompt 512, 32 generated, B parallel sequences (same file as 2.3). Relayed; the per-step time is T_TG / 32 and the ratio is derived.

| Model | B=1 | B=2 | B=4 | B=8 | B=16 | B=32 |
|---|---|---|---|---|---|---|
| Qwen2.5-Coder-7B Q8_0 (dense), ms per step | 12.8 | 15.1 | 18.3 | 31.4 | 49.8 | 52.4 |
| ... ratio to B=1 | 1 | 1.18 | 1.43 | 2.45 | 3.90 | 4.09 |
| gemma-3-4b Q4_0 (dense), ms per step | 7.3 | 9.8 | 12.6 | 18.1 | 37.7 | 40.6 |
| ... ratio | 1 | 1.33 | 1.72 | 2.47 | 5.14 | 5.53 |
| gpt-oss-20b MXFP4 (experts), ms per step | 7.7 | 11.9 | 18.9 | 32.3 | 49.1 | 59.7 |
| ... ratio | 1 | 1.56 | 2.46 | 4.21 | 6.40 | 7.78 |
| Qwen3-Coder-30B-A3B Q8_0 (experts), ms per step | 13.1 | 18.0 | 26.3 | 41.6 | 80.4 | 81.3 |
| ... ratio | 1 | 1.37 | 2.01 | 3.17 | 6.14 | 6.21 |

Readings (inferred from the table):

- On a dense model a second branch costs 18 to 33% of a step and four branches cost 43 to 72% more than one. The weights are read once; each extra branch adds arithmetic.
- A mixture-of-experts model shares less, because each branch may pick different experts and so reads different weights.
- The curve is not smooth (B=8 against 16 against 32): kernels switch form with batch width. A design must measure its own curve.
- The marginal row on the dense 7 B (7.62 B parameters, 15.2 GFLOP a row) costs 2.3 ms at B=2 and 1.3 ms at B=32: an effective 6.6 to 11.9 TFLOP/s (derived).

First machine, relayed from notes: an 8-row step of the 27 B model (seven drafted positions verified with the next token) takes 92 to 94 ms against 63 ms for one token, 1.46 times (`.../project-spec-verify-matrix-unit-2026-10-07.md:17`). That is 4.3 ms an extra row, 6.6% of a step each; at 54 GFLOP a row, 12.6 TFLOP/s (derived). The same note records what made it cheap: at 8 columns "each weight meets exactly one" matrix operation, so staging through threadgroup memory was pure overhead (line 15), which is also what Apple's guide says (section 4).

KV sharing: llama.cpp's `llama_memory_seq_cp` "Copy all tokens that belong to the specified sequence to another sequence"; "Each token can be assigned up to n_seq_max sequence ids"; `kv_unified`: "use a unified buffer across the input sequences when computing the attention ... try to disable when n_seq_max > 1 for improved performance when the sequences do not share a large prefix". https://raw.githubusercontent.com/ggml-org/llama.cpp/master/include/llama.h. Relayed. So a shared prefix stored once with per-branch tails is an existing, shipped mechanism; its cost on Metal was not found in a source.

Speculative steps as shared steps: llama.cpp's multi-token prediction on an M2 Ultra (Qwen3.6-35B-A3B Q8): 68.07 t/s without, 76.00 at n=3 (1.12 times, 78% acceptance), 77.68 at n=4; prompt processing fell about 17% (dev.to, dated May 18, 2026 by its footer, https://dev.to/someoddcodeguy/llamacpps-new-mtp-on-macos-4ea0). Relayed. First machine, old body, relayed: a 2 B block drafter verified in one 8-row pass gave 68.5 t/s against 15.6 plain on code (4.38 times), 1.5 times on prose (`.../project-qwen-speed-vs-llamacpp-2026-10-04.md:31`).

### 7.2 Adapters

- LoRA (arXiv 2106.09685, 2021-06-17): frozen weights plus trainable rank-decomposition matrices; 10,000 times fewer trainable parameters; "no additional inference latency" because the product can be merged into the weights. https://arxiv.org/abs/2106.09685. Relayed.
- S-LoRA (arXiv 2311.03285, 2023-11-06): thousands of adapters over one base model; adapters in main memory, fetched for running queries; one memory pool for adapters of different ranks and KV tensors; custom kernels batching heterogeneous adapters; up to 4 times the throughput of PEFT and naive vLLM. https://arxiv.org/abs/2311.03285. Relayed. This is the published form of "A/B in one pass": unmerged adapters, one read of the base weights.
- llama.cpp: `llama_set_adapters_lora(ctx, adapters, n_adapters, scales)` sets adapters on a context; `llama_set_adapter_cvec(ctx, data, len, n_embd, il_start, il_end)` adds a control vector of width n_embd to a range of layers. Relayed (header above).
- Tool tokens (arXiv 2305.11554, 2023-05-19): each tool is a token with a learned embedding, called "in the same way as generating a regular word token", on a frozen model, and tools are added "by expanding the set of toolkens on the fly". https://arxiv.org/abs/2305.11554. Relayed. This is extra rows in the output projection as the mechanism of new capability; its execution cost is term (a) below.
- Nearest-neighbour language models (arXiv 1911.00172, 2019-11-01): interpolate the next-token distribution with neighbours found in the model's own embedding space, "no additional training"; perplexity 15.79 on Wikitext-103, 2.9 better. https://arxiv.org/abs/1911.00172. Relayed. Its execution cost is term (e).

---

## 8. Sampling and grammar masks: GPU against CPU

- llama.cpp PR 17004 "sampling : add support for backend sampling", opened 2025-11-04, merged 2026-01-04, flag `--backend-sampling`: samplers run inside the compute graph so "only the sampled token needs to be transferred". Implemented on the backend: temp, logit_bias, top_k, greedy, dist, min_p, top_p. **Not compatible with grammar sampling**; one output per sequence per batch; marked experimental in the header (`llama_set_sampler`). One timing on gpt-oss-20b: 278.40 to 373.50 t/s after an argsort change. https://github.com/ggml-org/llama.cpp/pull/17004. Relayed (first 100,000 characters of the page).
- llguidance (README, undated): about 50 us of CPU time a token for a mask over a 128,000-token vocabulary; under 1% of masks exceed 1 ms; no significant start-up cost; an Earley parser over a token trie. https://raw.githubusercontent.com/guidance-ai/llguidance/main/README.md. Relayed.
- XGrammar (arXiv 2411.15100, 2024-11-22): "up to 100x speedup"; "near-zero overhead" by precomputing context-independent tokens and by overlapping grammar work with GPU execution. https://arxiv.org/abs/2411.15100. Relayed.

Reading (inferred): the mask over choices is CPU work of tens of microseconds, and its natural place is in parallel with the layers of the same step, since it depends only on what was already chosen. With shared storage the CPU writes the mask into a buffer the choosing kernel reads; no upload exists. The published state of the art has the two halves (sampling on the device, grammar on the CPU) but not yet joined: llama.cpp's backend sampler refuses grammars. A design that puts the bias vector in a shared buffer joins them.

---

## 9. Probe: scoring rows on the first machine's GPU

Same probe as section 5. A deliberately naive kernel: fp16 weights, one GPU thread a row, four `float4` accumulators, no tiling. GPU time is `GPUEndTime - GPUStartTime`, minimum of 15 rounds. Observed, contended.

| Rows x width | Weight bytes | GPU time run 1 | GPU time run 2 | Bytes per second |
|---|---|---|---|---|
| 16 x 5,120 | 0.16 MB | 129 us | 136 us | latency bound |
| 256 x 5,120 | 2.6 MB | 135 us | 137 us | latency bound |
| 4,096 x 5,120 | 41.9 MB | 199 us | 187 us | 211 to 224 GB/s |
| 65,536 x 5,120 | 671 MB | 2.60 ms | 2.70 ms | 249 to 258 GB/s |
| 151,936 x 5,120 (the size of an output head) | 1.556 GB | 6.26 ms | 4.97 ms | 249 to 313 GB/s |
| 100,000 x 1,024 | 205 MB | 0.494 ms | 0.486 ms | 415 to 421 GB/s |
| 1,000,000 x 1,024 | 2.048 GB | 4.72 ms | 4.78 ms | 428 to 434 GB/s |

Top-1 over the scores on the CPU, reading the shared buffer in place (`vDSP_maxvi`): 3.8 to 10 us for 100,000 scores, 40 to 102 us for 1,000,000 (`run-1.log:44,48`, `run-2.log:44,48`).

Readings:

- **A lone GPU thread walking 5,120 elements takes about 130 us however few rows there are.** 16 rows and 256 rows cost the same. The earlier sessions met the same floor from the other side (a one-thread normalisation at 1.1 to 2.2 ms, 41 us once spread over a threadgroup). On this GPU the cost of small work is latency, not bytes. Observed plus relayed.
- Wide work streams at 250 to 430 GB/s even in a naive kernel on a busy machine. Shorter rows stream faster (more parallel streams per byte). Observed.
- One million vectors of width 1,024 in fp16 are scored exhaustively in 4.7 ms; one hundred thousand in 0.5 ms. Observed.

---

## 10. A cost model a design can use

Symbols: d hidden width; L layers; V vocabulary; W bytes of weights the step reads (file size less the input embedding table; for a mixture of experts, the active experts only); P parameters read; b bytes per stored weight (2 for fp16, about 1.06 for Q8_0, about 0.56 for Q4_0); n context length; **S** the rate the GPU streams weights in a tall-thin product; **F** the effective multiply-add rate in a wide product; **t_sync** one CPU round trip; **t_lat** the latency floor of one small dependent dispatch.

### 10.1 One step

```
T_step  =  t_fixed  +  W / S  +  n * c_attn  +  t_choose
```

| Term | Value on M4 Max class hardware | Status |
|---|---|---|
| W / S | S = 318 to 426 GB/s for a 7 B model in llama.cpp (58 to 78% of 546), section 2.2 | measured by a source |
| | S = 324 to 336 GB/s, and 460 GB/s in one clean window, 27 B Q8_0 on the first machine | relayed from notes |
| | S = 249 to 434 GB/s, naive fp16 kernel, contended | observed today |
| t_fixed | about 4.3 ms for a 7 B model in llama.cpp (the fit of 2.2) | derived |
| | dispatch count times cost: 1,265 to 1,356 dispatches a token in the old body; about 20 us for a tiny dependent dispatch; 1.7 to 2.0 us for a trivial independent one; encode 0.06 to 0.09 us each in C | relayed; relayed; observed; observed |
| n * c_attn | c_attn = 0.04 to 0.9 us per context position per token (M2 Ultra tg32 at depth 0 against 32,768: 7 B Q8 12.55 to 17.60 ms; 4 B 7.45 to 8.84 ms; 30 B experts 12.66 to 22.09 ms) | derived from a source's measurements |
| t_choose | head share of the step = V*d*b / W: about 2.7% for V=151,936, d=5,120 at Q8_0 in a 29 GB model; an fp16 head of that size streams in 5.0 to 6.3 ms (naive, contended) | derived (V assumed); observed |
| | sampling on the CPU: one t_sync plus tens of microseconds; on the device: no transfer but no grammar (section 8) | relayed |
| Whole step | 12.0 ms (7 B Q4_0), 18.5 ms (7 B Q8_0), 31.6 ms (7 B F16) | measured by a source |
| | 63 ms (clean window) to 89 ms (llama.cpp quiet) for 27 B Q8_0 on the first machine | relayed from notes |
| | about 38 ms for 27 B at 4 bits | derived, not measured |
| Sustained | multiply every time by up to 2.6 after tens of seconds of continuous load; by 2 to 4 with another GPU client | relayed from notes |

### 10.2 The six marginal costs

**(a) Score N extra rows in the output projection.**

```
dT_a  =  N * d * b / S            if the rows are appended to the head's own matrix
dT_a  =  max(t_lat, N * d * b / S)  if they are a separate dispatch
```

- Appended, fp16, d=5,120: 4,096 rows are 41.9 MB, 0.13 to 0.17 ms at 249 to 313 GB/s; 1,024 rows 34 to 42 us. Derived from observed S.
- Separate dispatch: 187 to 199 us for 4,096 rows; 2.6 to 2.7 ms for 65,536 rows; 129 to 136 us for 16 rows (the floor). Observed.
- As a share of the step it is N*d*b / W: 4,096 fp16 rows are 0.14% of a 29 GB model's bytes, 1.1% of a 3.8 GB model's. Derived.
- On the CPU after the hidden vector is read (fp32, Accelerate): 256 rows 14 to 28 us; 4,096 rows 0.44 to 0.64 ms. Observed.

**(b) Add k extra input embeddings to the context.**

```
dT_b(once)       =  0                     if the embedding is the next input in place of a token's
dT_b(once)       =  k * 2P / F            if k positions ride in one widened pass
dT_b(once)       =  k / S_pp              if they are prefilled as a span
dT_b(each later step)  =  k * c_attn
memory           =  k * (KV bytes per position)
```

- Writing the vectors: k*d*4 bytes into a shared buffer by the CPU, no upload (Apple documentation, relayed). Reading one row of a node-embedding table on the GPU is the same operation as the token-embedding lookup (inferred).
- Widened pass: 4.3 ms an extra row for the 27 B on the first machine (6.6% of a step), relayed and derived; 2.3 ms for a 7 B on an M2 Ultra (18%), derived from a source.
- Span prefill: 886 t/s for a 7 B Q4_0 on M4 Max (1.1 ms a position), measured by a source; 115.6 to 131.7 t/s for the 27 B Q8_0 on the first machine (7.6 to 8.7 ms a position), relayed.
- Later steps: k times 0.04 to 0.9 us, negligible for k in the tens. Derived.
- KV bytes per position depend on layer count and key/value width; not derived here. Assumed to be of the order of 1 MB a position for a 27 B model at fp16.

**(c) One rank-r update on m matrices, unmerged.**

```
dT_c  =  m * 2 * r * d * b / S                  if fused into the dispatches that already read those matrices
dT_c  =  2 * m * t_lat                           if each product is its own small dispatch
dT_c  =  0 per step, but one weight copy per variant   if merged
```

- r=16, m=128, d=5,120, fp16: 41.9 MB, 0.14% of a 29 GB model; 0.09 to 0.17 ms at 250 to 460 GB/s. Derived.
- As separate one-thread-a-row dispatches: 2 * 128 * 130 us = 33 ms, half a step. Derived from the observed floor. With threadgroup-wide kernels at 20 to 41 us (relayed): 5 to 10 ms. This is the term that decides whether an adapter is cheap.
- On the CPU: 11.6 to 19.9 us a pair hot; 1.9 to 3.0 ms for 128 distinct pairs on one thread. Observed. It would need the hidden vector on the CPU at each of the m points, which costs m synchronisations; not usable inside a step.
- Merged: LoRA's "no additional inference latency" (relayed), but two variants then need two copies of each touched matrix and cannot share one read of the weights, so a side-by-side comparison costs a second full pass (inferred).

**(d) One hidden-vector readback to the CPU.**

```
dT_d  =  t_sync  (+ the GPU's idle time until the CPU answers)
```

- The copy: d*4 bytes, below 0.05 us. Observed.
- t_sync: minimum 61 to 77 us, median 106 to 115 us back to back; median 0.56 to 0.65 ms and p90 1.9 to 3.1 ms after 20 ms idle or under contention. Observed. "Up to 2.5 ms" (Apple), 0.12 ms (forum, M2 Max). Relayed.
- One readback a token already exists wherever the CPU chooses the token. A second one in the middle of a step is 0.1 to 5% of a 63 ms step and 0.5 to 25% of a 12 ms step. Derived.

**(e) One nearest-neighbour query over M vectors of width w (exhaustive).**

```
dT_e  =  M * w * b / S  +  t_topk  (+ t_sync if the CPU needs the answer)
```

- GPU, fp16, naive: 0.49 ms for 100,000 x 1,024; 4.7 to 4.8 ms for 1,000,000 x 1,024. Observed.
- Top-1 on the CPU over the shared scores: 4 to 10 us per 100,000. Observed.
- CPU, fp32, Accelerate: 9.6 to 14.0 ms for 500,000 x 1,024. Observed.
- When w = d and the query is the final hidden vector, (e) is (a): the table is extra rows of the head. Inferred.
- Budget rule: to stay under a share e of the step, M*w*b <= e*W. At 5% of a 29 GB model, 1.45 GB: about 700,000 fp16 vectors of width 1,024, or 1.4 million at one byte a dimension. Derived. Beyond that an index is needed; graph-walking indexes are chains of dependent loads and belong on the CPU (inferred from the latency floor).

**(f) B branches that share a prefix.**

```
T(B)  ~  T_step(1)  +  (B - 1) * 2P / F  +  (per-branch adapter and tail-attention bytes) / S
```

- Dense 7 B, M2 Ultra: 1.18, 1.43, 2.45 times at B = 2, 4, 8; dense 4 B: 1.33, 1.72, 2.47; experts 20 B: 1.56, 2.46, 4.21. Measured by a source.
- 27 B on the first machine: 1.46 times at 8 rows. Relayed.
- F = 6.6 to 12.6 TFLOP/s effective on M2 Ultra and M4 Max. Derived. Apple claims 4 to 8 times for matrix multiply on M5 (relayed); on that hardware a branch should be several times cheaper, not measured by anyone I found for this case.
- Sixteen concurrent requests give 4.3 times the aggregate throughput on an M4 Max in one MLX server. Relayed.
- Memory: the prefix once plus each branch's tail, by `seq_cp`-style sharing. Relayed mechanism; cost on Metal not found.

### 10.3 The model in one table, for the first machine

Step taken as 63 ms (27 B at Q8_0, clean window, relayed) and 12 ms (7 B at Q4_0, measured by a source on the same chip). Every row inherits the statuses above.

| Addition | Cost | Share of 63 ms | Share of 12 ms |
|---|---|---|---|
| 4,096 node rows appended to the head (fp16) | 0.13 to 0.17 ms | 0.2 to 0.3% | 1.1 to 1.4% |
| 65,536 node rows (fp16) | 2.6 to 2.7 ms | 4% | 22% |
| A node's embedding as the next input in place of a token's | 0 | 0 | 0 |
| One extra position riding in the same pass | 4.3 ms (27 B); about 2 ms (7 B) | 7% | about 18% |
| Rank 16 on 128 matrices, fused | 0.09 to 0.17 ms | 0.1 to 0.3% | 0.8 to 1.4% |
| Rank 16 on 128 matrices, 256 small dispatches | 5 to 33 ms | 8 to 52% | 42 to 275% |
| One extra CPU round trip | 0.06 to 0.12 ms quiet; 0.5 to 3 ms contended | 0.1 to 5% | 0.5 to 25% |
| Exhaustive query, 100,000 x 1,024 fp16 | 0.49 ms | 0.8% | 4% |
| Exhaustive query, 1,000,000 x 1,024 fp16 | 4.7 ms | 7.5% | 39% |
| A second branch | +7% (27 B, relayed) to +33% (measured by a source) | 7 to 33% | 18 to 33% |
| Four branches | +20% to +72% | 20 to 72% | 43 to 72% |

What "roughly the same budget as the next token" means, stated so it can be checked (inferred from all of the above):

1. **Bytes.** An addition that streams less than about 1% of W (290 MB for the 27 B at Q8_0; 38 MB for a 7 B at Q4_0) and is fused into dispatches that already run is inside the step's noise.
2. **Synchronisations.** One CPU round trip a step is the baseline. Each further one costs 0.06 to 3 ms depending on the machine's state.
3. **Dispatches.** Each new small dispatch costs its latency floor (2 to 136 us by kernel shape), not its bytes. A hundred of them is a quarter of a small model's step.
4. **Rows.** Each extra row through the whole network (a branch, an inserted position, a drafted token) costs 7 to 33% of a step on M2 and M4 class GPUs.

The budget is therefore not one number. The larger the model, the more headroom each fixed-size addition has, because the step is dominated by W.

### 10.4 What must be measured on the first machine before the design promises the budget

None of these has a figure I can stand behind today.

1. T_step for the target model at the target quantisation on a quiet machine, as a burst and sustained over 400 tokens (the 2.6 times effect). Nothing was measured quiet in this round.
2. S for a tuned head kernel at the target weight format, and the true marginal time of N rows appended to it for N = 256 to 65,536. My figures are for a naive fp16 kernel.
3. t_lat for the kernels the design will actually generate (a rank-r down-projection and up-projection, a normalisation), and the cost of fusing a rank-r update into the matvec that reads the base matrix.
4. The distribution of t_sync on a quiet machine, on the Metal 4 command queue as well as the classic one, and with the GPU kept warm.
5. The curve T(B) for B = 1 to 8 with a shared prefix on this machine, dense model, with and without per-branch adapters. Only one point (8 rows, 1.46 times) exists, from notes.
6. The per-dispatch cost with real kernels and real rebinding, and how few dispatches a token can be (llama.cpp's fused graph against the old body's 1,300).
7. Whether a CPU scan (200 to 276 GB/s observed with 4 or more threads) running beside the GPU takes bandwidth away from it. Both share one memory system; nothing measured.
8. An SME2 kernel generated by the system against Accelerate for the small products of section 6, and whether Accelerate already uses SME.
9. `matmul2d` from Metal Performance Primitives on M4 (the fallback path) against hand-written simdgroup kernels, and the 4-bit and 8-bit tensor types once the machine is on macOS 26.4.
10. The cold cost of generating a kernel: 31.6 ms to compile two small kernels from source and 19 ms for the first pipeline state were observed once; tensor-op pipelines are reported at one to two seconds each (relayed).
11. The cost of the choosing kernel itself (mask, bias, top-k or sample over V + N on the GPU) and of handing only the chosen id to the CPU.

---

## 11. What follows for the design (inferred; each line names what it rests on)

- **Three places to put inline work, by cost.**
  - *In the kernel, no synchronisation:* extra head rows for nodes, a mask or bias buffer the CPU writes in shared memory, a fused low-rank side path, a gather from a node-embedding table for the next input. Rests on 10.2 (a), (b), (c) and section 5.
  - *On the CPU at the step's one readback:* the grammar or admissibility mask (about 50 us), small scoring, index walks, running the chosen recipe. Rests on sections 6 and 8.
  - *As rows:* branches, drafted positions, inserted embeddings, at 7 to 33% of a step each. Rests on 7.1.
- **Choosing a node and choosing a token can be one kernel.** If node rows live in the head's matrix and a chosen node's vector is the next input, a step that picks a node costs what a step that picks a token costs. Rests on (a) and (b); the usefulness of such vectors is not this report's subject.
- **Keep adapters unmerged.** Unmerged rank 16 on every matrix is 0.14% of the bytes; merging saves that and forfeits comparing two variants in one read of the weights. Rests on (c), (f), S-LoRA.
- **Never give small work its own GPU dispatch.** Sixteen rows cost what four thousand cost. Small work is fused into a wide kernel or done on the CPU. Rests on section 9 and the notes of 3.2.
- **Generate shader source and call the tensor primitives inside it.** The offline-converted network path does not fit a system that writes its own kernels; the shader path costs nothing on M4 and inherits the accelerators on M5. Rests on section 4.
- **State stays in place.** KV cache, node tables and adapter matrices are shared-mode buffers written where they live; a mapped ice store can be wrapped without a copy. Rests on 3.4 and section 5.
- **Budgets are shares, not milliseconds.** Rests on 2.4.

## 12. What to avoid

- A CPU round trip in the middle of a step (hooks that read a hidden vector and write it back per layer).
- One dispatch per small operation; one command buffer per dispatch.
- Merging adapters into weights as the default.
- Treating the Core ML and `mtlpackage` network encoder as the execution path for generated kernels.
- Trusting any single timing on this machine; trusting any timing taken while another GPU client runs; quoting a burst figure as a sustained one.
- Assuming M5's 4 to 8 times matrix gain on the first machine. It has no neural accelerators.
- Exhaustive vector scans whose bytes approach W.

## 13. Open questions

1. What does a quiet first machine read for the target model at 4 bits? The 38 ms is a fit, not a measurement.
2. Can a rank-r side path be fused into the quantised matvec kernel with no added dispatch, and what does it then cost?
3. How does T(B) behave for B = 2 to 8 with per-branch adapters and a shared KV prefix on M4 Max?
4. Does the Metal 4 command queue shorten or tighten t_sync?
5. Can the GPU free-run from choice to next input (choice kernel writes the id, the next step's gather reads it) while the CPU follows asynchronously, and what does a late CPU veto cost?
6. How large can a node table be before an index beats the exhaustive scan on this machine, given the latency floor of dependent loads on the GPU and 200 to 276 GB/s on the CPU?
7. Does CPU streaming beside the GPU reduce S?
8. What is the per-position KV cost for the target architecture, and what does a branch's tail cost in memory?
9. Does the hardened-runtime JIT region the bootstrap reserves (`coherence-seed/graph/5.kg:645-654`) coexist with run-time shader compilation without further entitlement? Not examined.
10. Is 1 ms time slicing between GPU clients real, and can a client ask for a longer slice?

## 14. Sources

Local (observed unless noted):

- `coherence-seed/graph/0.kg`, `coherence-seed/graph/5.kg`, `coherence-seed/graph/4.2.kg` (hub 4.2.4 at lines 238-292; 4.2.8.7 at 365-374), `coherence-seed/graph/4.1.kg` (4.1.4.2 at 216-224), read 2026-11-24.
- Probe written and run for this report: `scratch/study/metal-probe/probe.m`, logs `run-1.log` and `run-2.log` in the same directory, 2026-11-24.
- Notes of earlier sessions on the first machine (their contents are relayed): `memory/project-qwen-speed-vs-llamacpp-2026-10-04.md` (2026-10-04 to 10-07), `.../reference-decode-submit-per-layer-and-reading-variance.md` (2026-10-13), `.../project-spec-verify-matrix-unit-2026-10-07.md` (2026-10-07), `.../project-local-lane-speed-is-gpu-bound.md` (2026-10-01), `.../reference-native-lora-teaching-lane.md` (2026-09-15).
- Apple, Metal Performance Primitives (MPP) Programming Guide, Version 1, 2026-03-16, read as a PDF (pages 1 to 13).

Web, all fetched 2026-11-24 (relayed):

- https://github.com/ggml-org/llama.cpp/discussions/4167 (instructions updated 2026-08-25)
- https://raw.githubusercontent.com/ggml-org/llama.cpp/master/benches/mac-m2-ultra/mac-m2-ultra.md (kernel dated 2025-11-18, build 7948)
- https://raw.githubusercontent.com/ggml-org/llama.cpp/master/ggml/src/ggml-metal/ggml-metal-context.m (master, undated)
- https://raw.githubusercontent.com/ggml-org/llama.cpp/master/include/llama.h (master, undated)
- https://github.com/ggml-org/llama.cpp/pull/16634 (2025-10-17, merged 2025-11-06)
- https://github.com/ggml-org/llama.cpp/pull/17004 (2025-11-04, merged 2026-01-04)
- https://machinelearning.apple.com/research/exploring-llms-mlx-m5 (2025-11-19)
- https://machinelearning.apple.com/research/core-ml-on-device-llama (2024-11-01)
- https://developer.apple.com/videos/play/wwdc2025/262/ (WWDC25)
- https://developer.apple.com/videos/play/wwdc2026/330/ (WWDC26)
- https://developer.apple.com/videos/play/tech-talks/111432/ (undated, 2026 copyright)
- https://developer.apple.com/videos/play/wwdc2024/10218/ (WWDC24)
- https://developer.apple.com/metal/whats-new/ (undated, 2026 content)
- https://developer.apple.com/tutorials/data/documentation/metal/mtl4machinelearningcommandencoder.json (undated)
- https://developer.apple.com/tutorials/data/documentation/metal/mtlstoragemode/shared.json (undated)
- https://developer.apple.com/tutorials/data/documentation/metal/mtldevice/makebuffer(bytesnocopy:length:options:deallocator:).json (undated)
- https://developer.apple.com/documentation/metalperformanceshaders/tuning-hints.md (2026 copyright)
- https://developer.apple.com/forums/thread/767853 (November 2024)
- https://ml-explore.github.io/mlx/build/html/usage/lazy_evaluation.html, .../usage/unified_memory.html, .../dev/custom_metal_kernels.html (MLX 0.32.4)
- https://raw.githubusercontent.com/ml-explore/mlx-lm/main/mlx_lm/LORA.md and .../main/README.md (undated)
- https://arxiv.org/abs/2601.19139 (2026-01-27), https://arxiv.org/abs/2607.19438 (2026-07-21), https://arxiv.org/abs/2409.18779 (2024-09-27), https://arxiv.org/abs/2512.21473 (2025-12-25), https://arxiv.org/abs/2106.09685 (2021-06-17), https://arxiv.org/abs/2311.03285 (2023-11-06), https://arxiv.org/abs/2305.11554 (2023-05-19), https://arxiv.org/abs/1911.00172 (2019-11-01), https://arxiv.org/abs/2411.15100 (2024-11-22)
- https://raw.githubusercontent.com/guidance-ai/llguidance/main/README.md (undated)
- https://dev.to/someoddcodeguy/llamacpps-new-mtp-on-macos-4ea0 (May 18; year by footer 2026)
- Search summaries only, not fetched in full: Apple newsroom 2024-10-30 (546 GB/s); Tom's Hardware M4 Max article (body not reachable); Arm learning paths on SME2 and KleidiAI.

Not reached: the body of the Tom's Hardware article; the second half of PR 17004's page; any published Metal timing for KV-prefix sharing; any published measurement of several adapters in one pass on Apple hardware; a measured figure for a dense 27 B model at 4 bits on an M4 Max.
