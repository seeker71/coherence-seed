# Resident inference and request ownership

A worker can retain model weights and key/value history between predictions. A caller can hand its result to another process without relying on the order in which callbacks arrive. The seed uses separate content identities for the weights, vocabulary, token history, and each occurrence of a request. The occurrence distinguishes two callers who submit identical model input.

`resident/route.ck` defines the request, outcome, reply and assignment records. A request contains its three-part context, occurrence ID and input. A reply contains the ID of the entire request and a typed outcome. Status 1 carries an answer, 0 carries the reason for no answer, and 2 carries a fault. Zero and false are both ordinary answers.

| Operation | Behavior |
| --- | --- |
| `resident.issue(entries, request)` | Adds a waiting assignment. Reissuing identical content preserves the entries. Reusing its occurrence for a changed request faults. |
| `resident.execute(context, request, evaluate)` | Checks the expected context before calling the supplied evaluator. Wraps its typed outcome with the request ID. |
| `resident.accept(entries, reply)` | Completes the assignment with the matching request ID. Exact duplicate receipts preserve the result. Unknown owners, conflicting results and duplicate matching assignments fault. The caller retains its previous immutable entries. |

An entry list can be saved as a node between calls. The owner chooses how long to keep completed entries for replay detection. The request must identify the actual history selected by the local worker. Context ownership and association give the caller no independent proof of a worker's identity or the contents of a remote process.

## Calling a local worker

`resident/call.ck` accepts a canonical `resident.port(context, target, byte-limit)` through descriptor 3 and a canonical `resident.request(context, occurrence, resident.invocation(target, argument-IDs))` through descriptor 4. Descriptor 5 writes to the private request pipe of a mounted NUMS worker. Standard input reads its private reply pipe. Standard output returns a canonical `resident.reply`. The byte limit bounds each reply body before allocation. A short, malformed or content-ID-mismatched reply faults.

```sh
./ck text resident/call.ck 5>worker.requests <worker.replies 3<local-policy.ck 4<request.ck
```

Give a connection one in-flight invocation. The numeric worker replies in stream order and its raw frames carry a result hash. Bind each response to its outstanding call before combining asynchronous results from separate connections. The owner supplies trusted descriptors and preserves the model and its history on the selected worker. A lost reply can leave the worker's history advanced. Recovery requires the worker's checkpoint or a new history.

`make check-image` exercises one live NUMS worker through local pipes. Two distinct requests return the same integer, 42. The caller receives the second assignment first, applies it twice and then applies the first. The comparison checks the owners as well as the values. `ground/resident-owners.ck` also changes each context identity independently and supplies malformed and conflicting assignments.

## Replaying the neural turn

The optional replay takes a local 28-layer Llama 3.2 3B Q4_K_M GGUF, its resolved eight-stream BOS Metal manifest, and the native runtime matching that manifest. The recorded input uses the `form-llama-vital-ground-q4_k_m.gguf` checkpoint. All prompts in the replay are synthetic token IDs. The weight SHA-256 is `dba8d50e3c63bde16cd918bb4de8e7cde6c5a2dae9a8360bd14480e71d8d7fa4`. The vocabulary SHA-256 is `d6ee1fd04027c02870a12fa793d8e7506eeeee705f48bc4569d42b5b5eab489e`, over the complete GGUF metadata records with keys beginning `tokenizer.`, concatenated in key order. Changing a tokenizer, normalization rule, template or special token record changes that pin. For another model, identify its weights and embedded tokenizer from the actual local artifact before creating its context.

Build a separate native checkout of seed revision `55b5b95` using `make ck seed-run`. Its model checkpoint stores the source manifest under `55b5b95-resident-metal/single-bos/manifest.ck` in the local coherence-seed model cache. Supply absolute paths to the actual files. The replay checks the GGUF hash and verifies the matrix-file paths recorded in the manifest. The model's Metal library, mapped GGUF and matching runtime are deployment inputs. The default portable test needs only this checkout.

```sh
RESIDENT_NATIVE=/absolute/path/to/built-native-checkout \
RESIDENT_MANIFEST="$HOME/Library/Caches/coherence-seed/55b5b95-resident-metal/single-bos/manifest.ck" \
RESIDENT_GGUF="$HOME/models/form-llama-vital-ground-q4_k_m.gguf" \
make resident-replay
```

The build reads the existing native graph through its own lens. It preserves the compiled decoder and the second-position input frame. The two extra frames supply the public result values through the model's ordinary token input. At runtime all eight streams start with BOS token 128000. The actual first prediction, 128006, selects one registered Form subtraction recipe. The result selects the fitting input frame on the still-mounted worker. Each counterfactual starts with a fresh KV history.

| Public arguments | Observed first token | Actual recipe result | Observed next token in every lane |
| --- | ---: | ---: | ---: |
| 25, 5 | 128006 | 20 | 14 |
| 25, 3 | 128006 | 22 | 56012 |

An independent greedy llama.cpp decoder agreed at both positions. A separate control using feedback tokens 128006 and 128007 gave the same next token, 9125, on both histories. The replay establishes a causal difference for the two table rows. The model's vocabulary has one reviewed action in this fixture. General tool selection, open-ended text input and shared scheduling across clients require additional evidence.

To repeat the reference locally, launch `llama-server --offline -m "$RESIDENT_GGUF" --host "$socket" --no-webui -np 1 -c 512` on a private path ending in `.sock`. Wait for its local `/health` response. Posting these three token histories to the completion endpoint returns the first token and each continuation.

```sh
for prompt_ids in 128000 128000,20 128000,22; do
  printf '{"prompt":[%s],"n_predict":1,"temperature":0,"return_tokens":true}' "$prompt_ids" |
    curl --fail --silent --show-error --unix-socket "$socket" \
      -H 'Content-Type: application/json' --data-binary @- http://localhost/completion |
    jq -ec '.tokens | select(length == 1)'
done
```

The expected ordered outputs are `[128006]`, `[14]` and `[56012]`.

## Fractional Q8_0 projection

The same native revision ran 1 × 32, 3 × 96 and 65 × 3,072 GGUF Q8_0 matrices with mixed binary16 scales, signed coefficients, and Float32 activations. An independent CPU decoder fed the dequantized matrix to Apple Accelerate. Exact dyadic arithmetic supplied a third result for every row. All 69 physical Metal and CPU results met a forward-error bound based on the sum of absolute products. Their values differed on 60 rows. Nineteen rows had enough cancellation to make their exact magnitude less than one millionth of that sum. On the 1 × 32 row, the exact dot was −3.279456 × 10⁻⁸, Metal returned zero, and Accelerate returned +1.281500 × 10⁻⁶.

Recomputing the kernel's lane-level fused multiply-add operations gave a tighter absolute bound that held for all 69 GPU rows. The last row of the largest matrix differed from each of nine tested CPU reduction orders. Reversing a coefficient failed exactly its physical output row. Swapping two rows failed exactly their two positional comparisons. The measured boundary covers finite inputs at these extents on the observed Apple backend. Subnormal handling, additional quantizations and other devices need separate comparisons.

The local witness directory `~/Library/Caches/coherence-seed/55b5b95-q8-fraction` contains the public binary fixtures, native Form readback, independent CPU and exact-arithmetic references, row-level observations, Metal library and checksums. From a local checkout, replay it with the native checkout built above.

```sh
probe="$HOME/Library/Caches/coherence-seed/55b5b95-q8-fraction"
python3 "$probe/compare.py" "$RESIDENT_NATIVE" "$probe/qwen.lib"
```
