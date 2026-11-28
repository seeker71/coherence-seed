# Local language learning

## Train and generate

`learning/train.ck` reads an `lm.training` node from standard input and writes
the fitted `lm.parameters`. An independent process can load those parameters
and generate a continuation.

```sh
./ck text learning/train.ck <ground/language-corpus.ck > model.ck
printf '%s' '(lm.request (list 2 6) 3)' | ./ck text lens/to-nodes.ck lens/all.ck > request.ck
./ck text learning/generate.ck 3<model.ck <request.ck | ./ck text lens/to-words.ck lens/all.ck
```

For this vocabulary, `2 6` means “small fox”. The result `(lm.completion
(list 12) lm.eos)` means the model generated “runs” and then chose the end
token. A caller can also ask for one prediction with `learning/predict.ck`.
Its integer score is a ranking score. Reaching a caller's generation limit
produces `lm.limited`.

The layout holds the ordered vocabulary, context width, rank, integer weight
limit, training margin and initialization seed. Positions 0 and 1 in the
vocabulary denote left padding and end. Each context position selects an
embedding row. Their sum is the hidden vector. Its dot product with each
output row gives the next-token scores. Training compares the observed token
to its highest-scoring competitor. When their score gap falls below the
margin, both sets of weights move one bounded step in the sign of their
respective gradients. Both updates read the same previous weights. Ties
resolve by vocabulary order. A serialized model contains its layout and the
two learned matrices.

The public fixture has 15 tokens and 18 synthetic training sentences. Six
adjective and noun combinations were held out. Fresh initialization completed
none of the six correctly. Six training passes produced all six expected
verbs and the end token. Two cold runs produced identical model bytes.
An independent scalar calculation matched all 360 weights after 432
next-token observations. The maintained comparison perturbs each weight
and checks the update against its effect on the score difference. This
establishes a locally trained causal model on a finite grammar.

## Assess an adapter against its parent

An evaluation design binds the base, immediate parent, fitted candidate,
control, protocol and ordered cases. Each case names its digest, expected
emission and whether losing a previously correct answer disqualifies the
candidate. A case can require both answers to a change of owner. Each run
names its design and artifact. Rows arrive in ascending digest order and
carry the actual emission and nonnegative loss in the protocol's integer
units. A truncated answer can be recorded as `no`. A study using token loss
records micro-nats and compares totals on the identical cases.

```sh
./ck text learning/assess.ck 3<parent.run 4<fitted.run 5<control.run <design.ck
```

The result retains each arm's exact matches, loss and protected regressions.
`trial.better` requires more exact matches and lower total loss than each
comparator, with every protected parent success preserved. Aliased arms,
missing, duplicated or reordered cases, negative loss and overflow are
refused. The result establishes what follows from the submitted measurements.
The evaluator and custody of the underlying inputs remain part of the evidence.

`learning/focus.ck 3<template.ck <scene.ck` selects a named owner's
attributed statement. Its result carries the original scene ID, template ID,
selected statement and rendered prompt. Missing or ambiguous ownership
prevents projection. With a public register codebook, the rendered prompts
matched all 408 local provider inputs byte for byte.

## Measured 3B adapter trials

An installed MLX-LM trainer fitted rank-eight adapters on the last four query
and value projections of a frozen local Llama 3.2 3B Instruct 4-bit model.
Each trial used 288 synthetic teaching examples, seed 391 and 96 updates.
The control used the same compute and a balanced reassignment that changed
every training label. Independent test prompts swapped only the named owner.
Both owners had to receive their exact one-symbol answer and terminate.

| Independent evaluation | Unchanged parent | Fitted adapter | Shuffled-label adapter |
| --- | ---: | ---: | ---: |
| Full dialogue, individual answers | 0/24 | 5/24 | 6/24 |
| Full dialogue, complete pairs | 0/12 | 0/12 | 0/12 |
| Owner projection and public codebook, individual answers | 34/36 | 9/36 | 9/36 |
| Owner projection and public codebook, complete pairs | 16/18 | 0/18 | 0/18 |

Each row compares three arms on identical inputs. The two designs used
different sealed test sets. The first fitted adapter lowered mean first-token
loss from 6.0455 to 1.4541 nats and still failed every owner pair. In the
second design the fitted adapter raised loss from 0.4180 to 1.5435. Both
candidates stayed inactive. These decisions concern a synthetic register
task. Human trust, vitality and consent to learn from personal utterances
require their own evidence.
