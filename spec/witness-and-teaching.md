# Checking claims, recording a pause and permitting local learning

## Claim and correction

A review keeps the proposed claim and its proposer, each citation and its matching entries,
the responsible reader, the edition digest, the decision and the answer. A citation names
an edition, language and exact wording. Each distinct citation must resolve to one entry
and all answers must agree. Agreement with the proposition yields `witness.supported`.
Agreement on a different answer yields `witness.corrected`. Missing, repeated or
conflicting evidence yields `witness.unresolved` with the findings available for inspection.
Independent citations can share an underlying source. The review establishes what the supplied
edition says.

| Run with `./ck text` | Standard input | Other input descriptors | Standard output |
| --- | --- | --- | --- |
| `witness/check.ck` | Claim | 3 edition entries, 4 reader | Review |
| `witness/retain.ck` | Review | 3 trusted reader | Receipt binding reader, claim and complete review |
| `witness/recall.ck` | Original claim | 3 edition entries, 4 reviews, 5 trusted receipts | List of verified reviews |

Keep receipts with the reader or their trusted local storage. Recall rechecks sources and
the complete report against the retained receipt. In a separate two-citation exercise the
reader corrected “rain” to “snow”. Removing an entry, adding an alternative at one citation
or changing one answer prevented agreement. Substituting a different report reader, answer or
edition on recall produced `witness.stale`. These local receipts rely on how the reader and
receipt storage are identified and protected.

## An invitation and its outcome

| Run with `./ck text` | Standard input | Other input descriptors | Standard output |
| --- | --- | --- | --- |
| `witness/observe.ck` | Broker receive result | 3 invitation with exact scope and ticket, 4 prior listening register, 5 subsequent register | Activity retaining all four inputs |
| `witness/account.ck` | Requested account or correction | 3 activity, 4 previous account or `no`, 5 trusted author | Attributed account with its predecessor |

For the matching invitation and an actual paused receive, unchanged empty listening
registers give `witness.unanswered`. Withdrawing the matching invitation from its
immediate preceding revision gives `witness.stopped`. Other combinations stay unresolved.
A fresh-store comparison checks the stopped delivery against an empty destination. An invited
participant can separately supply or correct their own words. The synthetic exercise records
an operation and a change of authority. A claim about a person’s wellbeing, reasons or
experience requires their voluntarily supplied account.

## Teaching, activation and use

Provision a private broker directory, its recipient directory and three authority registers:
`authority`, `listening` and `adapter-authority`. For a pinned episode, place the public roster
in `private/peers` and initialize each register with the output of `circle/pin.ck` for that
roster. A trusted local, unpinned deployment initializes them with canonical `no`.
Each command below reads canonical nodes. `use` emits raw bytes.

| Run with `./ck text` | Standard input | Other input descriptors | Standard output or effect |
| --- | --- | --- | --- |
| `learning/teach.ck` | Source ticket | 3 channel | Delivers the granted examples and returns a round |
| `learning/artifact-scope.ck` | Round | 3 design, 4 parent run, 5 fitted run, 6 shuffled run | Exact artifact scope for owner inspection |
| `learning/allow.ck` | Owner change | 3 channel | Changes local unpinned artifact permission |
| `learning/approve.ck` | Signed owner proof | 3 channel, 4 owner change | Changes pinned artifact permission |
| `learning/activate.ck` | Round | 3 channel, 4 design, 5 parent run, 6 fitted run, 7 shuffled run | Records and returns the selected artifact |
| `learning/use.ck` | Requested selection | 3 channel | Returns verified bytes to the explicit caller |

The source owner grants the exact bytes for `teach.examples` and `teach.gradient`, naming
the participant, recipient directory, training base, parent and protocol. The listener
accepts that ticket. After delivery or exact replay, the broker writes a private admission receipt that pins both revisions. Activation and use check the receipt. Reconstructing a round from current permissions alone cannot establish admission. The artifact grant
separately covers the round, complete design, three complete reports, destination and SHA-256
of the candidate. Stage the candidate at `<recipient directory>/<lowercase SHA-256>.safetensors`.

Activation recomputes the three-arm judgment, requires `trial.better`, verifies the
candidate bytes and publishes the selection by renaming its staged record. The broker lock
serializes each activation and use with both kinds of permission change. Both paths recheck
the original grants, the artifact permission and its time window at the effect boundary.
An unchanged selection survives a process restart. Power-loss durability and restoration of
a whole directory to an earlier state require separate evidence. Downstream copies, a caller
that bypasses this interface and the provenance of external trainers are outside its control.

Separate processes verified the exact 88-byte test tensor through the use interface. MLX loaded those selected bytes and computed 1 and 2 for two independent inputs. A recall-only
grant, a withdrawn source, a withdrawn listener invitation, an identical later grant, altered
model bytes and an equally eligible but different set of reports each refused the operation.
A signed exercise also refused the unsigned pinned override and the listener’s attempt to
change artifact permission. The first measured 3B candidate stayed inactive with its actual
weights, all three reports and a valid artifact grant. Successful fixture activation establishes
permission enforcement and byte delivery. The measured training results are in
[Local language learning](local-learning.md).

## Constructing a teaching scope before fitting

`./ck text learning/source.ck 3<channel 4<material <examples` builds the
scope for exact example bytes. The material names their provenance,
trainer location, base, parent and training protocol. The caller can pin
the protocol and register a held-out evaluation before any candidate
weights exist. The owner and listener use the resulting scope for
`learning/teach.ck`. Admission identifies the actual letter available
to the trainer. Assessment and derivative permission refer to the
subsequently fitted artifact.
