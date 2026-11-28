# Local translation and owned corrections

The grammar in `lens/translate.ck` gives exact readings for a small English and
Indonesian construction. An explicit local model request can propose translations of
free text. Both paths keep the original. Only an explicit owner decision records
which reading they accept. Sharing, rendering audio and teaching a model each require
their own permission.

## Run locally

Create an utterance from raw UTF-8. Descriptors 3 and 4 carry source and target
language tags, 5 the owner, 6 a caller-selected occurrence and optionally 7 a source
edition or attribution. These text files contain the value exactly, including any
line break. Standard input is the complete passage.

```sh
printf en >from; printf id >into; printf 'local speaker' >owner
printf 'first turn' >occurrence
printf 'We may listen' |
  ./ck text translation/utterance.ck 3<from 4<into 5<owner 6<occurrence >utterance.ck
TRANSLATION_MODEL=/absolute/local/MLX/checkpoint TRANSLATION_UTTERANCE=utterance.ck \
  make -s translate-local >study.ck
./ck text lens/to-words.ck lens/all.ck <study.ck
```

The optional target invokes an installed `mlx_lm.generate` with a local single-file
checkpoint and tokenizer, in Hugging Face offline mode. The seed supplies the exact
prompt and decoding options. The operator needs MLX-LM, OpenSSL and shasum for this
path. Before and after generation the driver compares the raw SHA-256 identities of
the weights, tokenizer and configuration assets. The model record also carries the
launcher identity and available package versions. Protect the local model installation
and worker. These identities allow replay and comparison of inputs.

## Reading the result

The legacy three-line grammar input still returns its readings. To prepare a model
request, prefix a canonical utterance with the line `@review` and supply a canonical
model record on descriptor 3 of `lens/translate.ck`. The resulting `resident.request`
binds the occurrence, exact source and target, edition, model, grammar version, all
grammar readings and the full prompt. A worker's reply must name that request.
`translation/review.ck` compares it to the expected request on descriptor 3. It keeps
the raw reply and every offered alternative with its origin.

The single-line protocol expects one complete `T: ` line per proposed reading and a
separate `END` or `END.` line. A multiline passage uses its physical line numbers.
Every numbered source line must appear in order in each proposed reading, followed
by its end marker. The renderer supplies the copyable marker `[END] END`. The seed
restores blank lines and the final newline from the source. An explicit `UNKNOWN`, malformed framing, an
unterminated response, a provider error or an unconfigured language leaves the model
status unknown with a reason. A study can still contain grammar readings in those
cases. A complete frame establishes what the provider proposed. A well-framed
translation can still omit a condition within a line, change a pronoun or add a claim.
Provider failures keep the exit code, partial standard output and diagnostic output.

A fresh reader can supply an independently retained utterance on descriptor 3 and
model record on 4 to `translation/recheck.ck`, with the study on standard input.
Another edition, another target, a substituted report or a different model prevents
replay from succeeding. Replay uses the installed grammar and prompt policy. Archived
versions can be used to recheck studies created with an earlier policy.

## A participant’s decision

`translation/choice.ck` takes the current study or previous position on descriptor 3
and the raw text `attest`, `correct` or `decline` on 4. To attest, supply the zero-based
option index in canonical Form on standard input. To correct, supply the participant’s
own UTF-8 wording. A decline needs no wording. This produces a proposed change bound
to that specific version.

`translation/amend.ck` takes that change on standard input, the original study on 3,
the previous position or canonical `no` on 4 and the trusted owner's canonical name
on 5. A correction preserves the earlier position, all alternatives and the proposed
model output. Other owners, stale proposed changes and invented option references
are refused. Host authentication and durable ownership of the current position belong
to the caller.

## What was observed

A local MLX-LM run used the 4-bit Llama 3.2 3B Instruct checkpoint, greedy decoding
and a 128-token limit. Eleven recorded passages covered Indonesian to English, French
to English, German to English, English to Indonesian and English to Spanish. Fresh
native replay checks the actual resulting bytes in `ground/translation-samples.ck`.
The model card lists eight supported languages. Indonesian is an experimental route
here. [Meta model card](https://huggingface.co/meta-llama/Llama-3.2-3B-Instruct)

The Indonesian source containing *kita* produced the public English reference exactly.
For the *kami* source the model wrote “We will contact you if we have further questions.”
The linked English uses a contraction and includes “any”. English *we* alone does not
recover Indonesian addressee inclusion. The pairs are public Tatoeba data, CC BY 2.0
FR. Bilmanda contributed the links. English #68806 comes from the Tanaka Corpus and
was edited by CK, who contributed #3142557. [Inclusive pair](https://tatoeba.org/en/sentences/show/13043619) · [English #68806](https://tatoeba.org/en/sentences/show/68806) · [Exclusive pair](https://tatoeba.org/en/sentences/show/3991849) · [English #3142557](https://tatoeba.org/en/sentences/show/3142557) · [License](https://creativecommons.org/licenses/by/2.0/fr/)

Changing three to four in German changed the English number. Changing French polarity
changed the English polarity, and the negative version generalized one Tuesday to
“Tuesdays”. For “We may listen.” the model chose *kami* and *mungkin*, introducing an
exclusive group and a reading of possibility. A German two-line source initially produced only “Please stay.”, omitting a direction
to keep the door closed. With the indexed format, the live provider translated both
lines and the seed restored the final newline. A separate German example used ambiguous
*Sie* and the model supplied only third-person *they*. The bounded grammar preserves both
pronoun readings for its exact unpunctuated permission construction. A fresh-reader
comparison separately changed just the edition and just the target of the public
*kita* study. Both returned `lingua.stale`. A speaker’s
endorsement is recorded through a separate attributed decision.
