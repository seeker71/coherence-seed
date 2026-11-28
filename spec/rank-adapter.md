# Training an output adapter for the owned language model

The seed can fit and serialize two integer factor matrices against a frozen `lm.parameters`. For hidden vector *h*, native inference computes *Wh + A(Bh)*. W and the embeddings come from the frozen parent. A has one row per vocabulary token and B has one column per hidden coordinate. The declared adapter rank must be positive and smaller than the parent’s hidden width. The config and parent bounds must leave enough signed-integer headroom for the scores and their difference. The serialized `rank.adapter` binds the exact parent content ID, including vocabulary order.

A starts at zero. B starts with an identity block in its leading columns and seeded small integers in the remaining columns. The initial score vector therefore reproduces the parent exactly. On each supplied `rank.example(history, nextToken)`, the trainer compares the target with the strongest competing token. If the score margin is insufficient, it changes A and B by clipped sign-gradient steps. Both updates use the same previous factors. The trainer neither copies a desired output matrix nor rewrites the parent.

| Entry | Inputs | Result |
| --- | --- | --- |
| `learning/rank-fit.ck` | Checked parent on fd 3, `rank.training` on stdin | Complete `rank.adapter` |
| `learning/rank-scores.ck` | Parent on fd 3, adapter on fd 4, token history on stdin | All vocabulary scores |
| `learning/rank-generate.ck` | Parent on fd 3, adapter on fd 4, `lm.request` on stdin | Direct, unrestricted factor-model completion |
| `learning/rank-compose.ck` | Parent on fd 3, adapter on fd 4, `rank.scope` on stdin | `rank.child` binding both factors and scope |
| `learning/rank-use.ck` | Channel on fd 3, active derivative selection on fd 4, original parent bytes on fd 5, `lm.request` on stdin | The currently permitted scoped completion |

A scope declares allowed vocabulary IDs for each position of an exact-length history. The call checks the caller’s original history once. A match authorizes the factors for the first next-token prediction. All later predictions use the parent. An initially out-of-scope request stays on the parent throughout generation, even if its generated text reaches an otherwise eligible context. The approved artifact includes this routing rule. This direct adapter requires the same base and immediate parent. At use, the owner’s broker checks that lineage, the parent’s SHA-256, the active selection and derivative permission, then invokes the checked child while holding its lock. Direct factor-model experiments and the protected derivative endpoint serve different callers.

## The finite grammar study

The frozen parent had 15 vocabulary entries and eight hidden coordinates. A sibling reserved one adjective/noun combination per noun before the adapter ran. The other 18 combinations provided 36 labels for a rotated verb followed by EOS. The correct mapping was runs→flies, flies→swims and swims→runs. The matched control used the opposite rotation with the same balanced label counts and teacher-forced EOS examples. All arms used the same frozen parent; only the two fitted arms trained. They shared rank 3, factor limit 12, margin 8, initialization seed, row order and 12 passes, or 432 observations each.

| Held-out result on the raw model | Parent / zero-effect adapter | Fitted factors | Reverse-rotation factors |
| --- | ---: | ---: | ---: |
| Correct rotated verb and EOS | 0/6 | 6/6 | 0/6 |
| Correct EOS following the supplied gold verb | 6/6 | 6/6 | 6/6 |

The six combinations were excluded from adapter teaching. Five had appeared in the frozen parent’s original grammar with different verb labels. These results establish recombination within the declared synthetic grammar.

The raw adapter changed four of five previously correct one-token starts (the empty prompt and three single adjectives). An initial context rule looked at the growing history. It preserved all five first tokens but changed all five longer protected continuations. For example, from “quiet” with a budget of three, the parent produced “trout swims” and EOS. The adapter intervened after “trout” and returned “trout runs” instead. Independent sibling replay found this through the live derivative endpoint.

The repaired child fixes eligibility at the original caller-supplied history. Its evaluation contains six rotated completions, six EOS prompts, five one-token starts and those five starts rolled through EOS with budgets of three or four. Loss counts teacher-forced top-1 token errors. These scope rules were developed after observing the raw and continuation regressions.

| 22 protected cases | Exact | Token errors | Previously correct parent cases lost |
| --- | ---: | ---: | ---: |
| Frozen parent | 16 | 6 | 0 |
| Raw fitted factors | 13 | 14 | 9 |
| Raw control | 9 | 15 | 7 |
| Rule that recomputed eligibility each step | 17 | 5 | 5 |
| Caller-bound fitted child | 22 | 0 | 0 |
| Caller-bound control | 16 | 6 | 0 |

The fixture used the owner’s local protected registers to admit the actual teaching bytes, assess the measured child and control, and separately permit the eligible derivative. Detached peer signatures were exercised in the separate circle and mesh grounds. On “young fox” the checked child produced “flies” and EOS. It preserved the parent’s empty-history choice and its full “quiet” continuation. Supplying different parent bytes or withdrawing derivative permission refused the call. Reusing these cases to check the repair limits claims about a freshly sealed policy evaluation. The 3B trials in [local language learning](local-learning.md) used different models and evidence.

## Reproduction

The maintained comparison independently constructs dense coefficients for all 15 logits across 24 contexts, their teacher-forced continuations and three factor sets. A separate coordinate-perturbation check compares the actual native update at every factor coordinate of three adapter states. Invalid rank, unsafe arithmetic bounds, malformed factors, invalid labels, an invalid scope and a same-weights parent with swapped vocabulary entries are refused. The cold parent bytes compare equal before training and after authorized inference.

The local witness bundle `~/Library/Caches/coherence-seed/owned-rank-v1` contains canonical model and training bytes, their readable renderings and a standard-library scalar reference. The reference independently recomputes every active training update from dense score differences. It reproduced the final coefficients after 34 fitted updates (2,346 coordinate perturbations) and 22 control updates (1,518 perturbations). From the bundle run `python3 oracle.py` and `shasum -a 256 -c SHA256SUMS`.
