# Reading an attributed English decision

The input is one selected speaker’s synthetic English statement. The four study labels mean permission to release a copy (K), withholding or stopping (M), correcting wording (P), and undecided (V). An interpretation can be wrong even when it quotes the speaker accurately. Authority over delivery still comes from the owner’s current explicit grant and the recipient’s current listening state.

## Two local readers

`learning/intent-fit.ck` reads canonical `intent.case(statement, label)` rows from standard input and writes a reusable checkpoint carrying the native identity of its training rows, the ordered codebook, and four rows of 256 signed coefficients. A fit requires examples for every class. The learner and the rule reader both select the last nonempty clause delimited by a full stop, question mark, exclamation mark, semicolon or colon. They lowercase ASCII letters and preserve their original byte offsets. A statement may contain up to 4,096 bytes. Quotation, scope of negation, indirect speech and non-ASCII words need more language knowledge than this tokenizer provides.

For each document, the learner counts each occupied bin once. Unigrams use initial hash 1. Adjacent words joined by one space use initial hash 2. Each byte updates the hash to `(131h + byte) mod 256`. The coefficient is the class prevalence minus the prevalence among the other classes, each scaled by 4,096 and rounded down before subtraction. A unique positive sum selects a class. A tie or a nonpositive maximum yields `?`. Distinct words can share a bin.

The deterministic reader matches the published cue phrases in `learning/intent-cues.ck`. Each hit reports its class, start byte, exclusive end byte, and matched words. A single supported class selects an interpretation. Conflicting classes and absent support yield `?`. An explicit negative cue also blocks a release reading. Every matching span remains available on abstention.

```sh
./ck text learning/intent-fit.ck <ground/intent-corpus.ck >model.ck
printf '%s' '(list "I said please send. Now I withdraw.")' |
  ./ck text lens/to-nodes.ck lens/all.ck >requests.ck
./ck text learning/intent-read.ck 3<model.ck <requests.ck |
  ./ck text lens/to-words.ck lens/all.ck
./ck text ground/intent-study.ck
```

For a rule reading directly from a person’s local text, pipe the raw UTF-8 bytes to the standalone reader. It reports “withdraw” at byte offsets 26 through 33 in this example.

```sh
printf '%s' 'I said please send. Now I withdraw.' |
  ./ck text learning/intent-explain.ck |
  ./ck text lens/to-words.ck lens/all.ck
```

The last ground command above fits both the original 192 synthetic teaching cases and a control that rotates all four training labels. It compares all four learned rows after rotation, replays both native readers on the 32 fresh statements, and checks the original byte span of a retraction, a guarded affirmative cue, an empty statement and missing-class training. An independent implementation also reproduced all 1,024 original coefficients, 32 score vectors and all matched spans.

## Training and evaluation authority

The original published teaching letter, teaching round, ticket and control revisions identify the historical release of the 192 synthetic JSONL rows to the 3B training protocol. The specialist study separately converted just their statements and labels into `ground/intent-corpus.ck` and recorded both source hashes. The synthetic fixtures exercise a local consenting owner and listener. The research comparison makes no claim to authority over someone else’s words.

A fresh grant binds the canonical training rows, native learner basis, recipient, and the study agreement in `ground/intent-contract.ck`. Both peers admitted the rows through `learning/teach.ck`. Under the broker lock, `learning/intent-admitted-fit.ck` checks the live teaching round, training purpose, received payload, basis, and protocol. It fits from the granted letter’s own bytes and rechecks the current grant before returning the checkpoint. Withdrawing the source grant refuses another fit. The newly admitted checkpoint matched the pre-unseal checkpoint byte for byte.

A separate owner grant covers observation of the sealed speakers’ statements with that exact checkpoint. The evaluation payload contains only the selected statements. Gold labels stay with the evaluator. `learning/intent-evaluation-source.ck` creates the observations scope. `learning/intent-evaluate.ck` consumes it under the same checked callback. A substituted model, a paused listener and an attempt to use that evaluation ticket for training all refuse. All 32 purpose-bound score vectors and cue-span lists matched the independent predeclared reconstruction. The native ground uses local synthetic authority. Deployment with real speakers requires their own explicit, purpose-specific participation.

## What the five readers did

The 192 consent-released synthetic statements contain 48 distinct final wordings in four framing conditions. A sibling sealed 16 pairs of fresh, distinct statements, with two named speakers in each pair and eight cases per class. Each reader received the same selected speaker’s words. The neural readers also received the original fixed codebook instruction. Their score requires exactly one predicted label token followed by EOS. The existing fourth LoRA trained using a corrected completion mask. The third used the earlier mask. These results measure the previously frozen checkpoints.

| Reader | Exact /32 | Both speakers correct /16 | Answered /32 | Identified withholding /8 | M or V read as K | P read as K |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Deterministic cue reader | 6 | 0 | 7 | 1 | 0 | 0 |
| Native fitted specialist | 20 | 7 | 32 | 1 | 0 | 1 |
| Untouched local Llama 3.2 3B parent | 22 | 7 | 32 | 6 | 0 | 0 |
| Third raw LoRA | 28 | 12 | 32 | 7 | 2 | 1 |
| Fourth raw LoRA | 28 | 12 | 32 | 6 | 2 | 1 |

One pair shows the cost of a literal cue. In “They are correct now, so pass the piece on,” the rule labeled the adjective “correct” as an amendment. The fitted reader made the same mistake. The other speaker asked for the quote to be reattributed. The fitted reader interpreted that request as permission to release. The fourth LoRA read two requests to withhold as permission. The third did so once on a withholding request and once on an undecided statement. First-symbol mean gold negative log-likelihood was 0.855 for the parent, 0.322 for the third LoRA, and 0.386 for the fourth. These results support retaining the previous promotion hold. A deployment threshold would need a separately approved error budget and owner-reviewed cases drawn from the intended population, including ambiguous speech, retractions and corrections. None was established here. This cohort measures synthetic attributed English decisions. A person’s intended meaning and willingness to share require their own participation.

A sibling then supplied “I have not decided whether to release my copy.” The fitted specialist gave K 3,417 and V 3,415. The cue reader selected V and retained “have not decided” at bytes 2–17, “not decided” at 7–17, and the negation at 7–9. The raw inference and independent reference both reproduced the incorrect permission prediction. This later challenge is recorded separately from the sealed cohort. No fit or cue was tuned after either result.

The participant comparison in `make check-held-back` extracts this same sentence from the synthetic owner’s selected excerpt and runs both readers on it. The listener signed an invitation for those exact words. The owner’s signing command and the broker’s submission command each refused the readers’ output with `inbox.invalid`. Both consent revisions stayed unchanged. Reading the ticket returned `share.ungranted` and the delivery directory stayed empty. When the owner separately signed and submitted the grant, the broker returned one attributed excerpt for the listener. Withdrawing that grant stopped subsequent reads.

The preregistration SHA-256 is `7fd8d60aa361ec4cc10cd8317530c1e97ea8e74045ae4cb214ed442b4e6e26b6`. The admitted training JSONL is `2fa9b72adccc16943070b8370cc012f5465229790e86932a27fd38929278a32f`. The independently sealed cohort is `0c5f41e8d98b5aa72f50f3f5d6c6bd9b34d088c4e35ec2fc7e821d25cd49f8d2`. The serialized native checkpoint is `b33f368a02160ccf0f69dcd387931c9aeabb639cb93134dac189adaa65959c1e`. On the study host the synthetic corpus, full predictions, independent scorer, two raw LoRAs, manifests, and offline replay instructions are in `~/Library/Caches/coherence-seed/intent-specialist-v1`. The neural replay also requires the locally cached public 4-bit parent at revision `7f0dc925e0d0afb0322d96f9255cfddf2ba5636e` and MLX. The checked-in native replay needs a built seed.
