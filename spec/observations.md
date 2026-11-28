# Predictions and observed events

A station names a local instrument file, an ordered alphabet of byte values
and competing transition tables. Each table maps every alphabet position to
its predicted successor. The instrument writes canonical `world.sample`
nodes containing its sequence number and measured bytes.

| Command | Result |
| --- | --- |
| `./ck text world/open.ck 3<station` | Read the instrument once and open an observation history. |
| `./ck text world/predict.ck 3<station <history` | Replay the history and return the preferred forecast alongside every competing forecast and its error count. |
| `./ck text world/observe.ck 3<station 4<forecast <history` | Recheck the forecast, read the instrument once and return the updated history. |

The station's content ID binds the instrument path, symbol meanings and
hypotheses. Each forecast names the preceding event. Each accepted observation
retains the exact forecast and the bytes that tested it. The hypothesis with
the fewest prior errors supplies the next prediction. The declared order
breaks ties. Readers can inspect the alternatives.

An identical sample at the same sequence returns the same history. Changed
bytes at that sequence produce `world.conflict`. A missing sequence produces
`world.gap`. An older sample, edited history or forecast for another history
produces `world.stale` when replay detects the mismatch. Sampling an
unavailable instrument produces `world.unobserved`. The caller owns the
resulting history and may store it. The hash chain detects disagreement with
a retained checkpoint. The source path and producer's sequence remain local
trust assumptions.

In `make check-replay`, two hypotheses initially predict that a binary source
will stay put or change. Sequence 11 contradicted the preferred prediction.
Fresh processes retained that wrong forecast and its reading, preferred the
other rule and correctly predicted sequence 12. The ground also compares all
32 binary histories of five observations with an independent error count.
These results cover an instrument with a declared alphabet and hypothesis
set. Open-ended perception remains a separate capability.
