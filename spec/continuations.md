# Resuming a local delivery job

A caller can ask an existing circle broker to deliver an ordered list of letters.
The job records its caller, owner, circle, recipient, directories, pinned episode
and the intended ticket and content identity for each effect. Each letter has its
own nonce and grant. Prepare the actions in order and put their canonical nodes in a
Form list.

| Command under `continuation/` | Standard input | Other descriptors | Result |
| --- | --- | --- | --- |
| `action.ck` | Exact share scope | 3 ticket | Action bound to the ticket, scope and expected letter |
| `plan.ck` | Ordered list of actions | 3 channel, 4 caller | Job |
| `begin.ck` | Job | 3 channel, 4 caller | New or previously stored checkpoint |
| `wake.ck` | Job | 3 channel, 4 caller | Event correlated with the current blocked action and changed local observation |
| `resume.ck` | Job | 3 channel, 4 caller, 5 event or canonical `no` | Current or advanced checkpoint |

The host supplies the trusted channel and caller. A local operator can keep the job
and submit it to a fresh process after changing the required permission or restoring
the recipient directory.

```sh
./ck text continuation/plan.ck 3<channel.ck 4<caller.ck <actions.ck >job.ck
./ck text continuation/begin.ck 3<channel.ck 4<caller.ck <job.ck >first-checkpoint.ck
./ck text continuation/wake.ck 3<channel.ck 4<caller.ck <job.ck >event.ck
./ck text continuation/resume.ck 3<channel.ck 4<caller.ck 5<event.ck <job.ck >next-checkpoint.ck
```

If a process exits without an answer, submit the original job to `resume.ck`
with canonical `no` on descriptor 5. An interrupted attempt can then be reconciled.
A recorded refusal retains the job, completed receipts, exact cause and the source
and listener revision identities. It stays pending until a matching event arrives.
The broker checks the event against both the saved checkpoint and its current
registers and recipient-directory availability. A new source grant must match the
ticket nonce, full scope identity and channel address. A new listener invitation
must name the exact ticket. An unchanged finite grant can wake at its start time.
Resume checks the time again before delivering. Directory restoration can wake
the exact failed publication. The broker can regenerate a dropped wake from its
current observations.

Before issuing a letter the worker persists the intended effect and the revisions
that approved it. On replacement, it verifies earlier receipts against the actual
destination files. If an in-flight letter already exists with the intended ticket
and complete content identity, the replacement records a separate observation of
that effect. A fresh publication still needs the live source grant, listener ticket
and clock checks. Checkpoints and letters use the same broker lock. Checkpoint
data carries register identities so unrelated grant payloads stay in their authority store.

The maintained comparison terminates a real worker after the first or second
successful destination link, before the corresponding acknowledgment. It checks the
actual files, their bytes, and the original file’s identity. It also exercises a
different owner or caller, a substituted target, a missing earlier letter, a late
signal and restoration of the recipient directory after the exact publish fault.
A separate reader replaced the second completed letter with the first and observed
`inbox.conflict` on resume. Restoring the original bytes and ticket restored the
confirmed state.

A second comparison opens two jobs from distinct callers and source owners in one
private store, with a shared recipient directory and identical ordinal nonces.
The second job finishes first. Crossed signals and a signal superseded by another
owner’s grant leave both checkpoints byte-for-byte unchanged. The four files
contain the four distinct intended payloads. With a grant valid from 100 through
109, changing only the clock from 99 to 100 also supplies a wake. Delivering
that wake at 110 leaves the checkpoint and recipient directory untouched.

The receipt covers an immutable local file publication. A payment or another
effect without an observable, stable identity needs its own confirmation protocol.
This flow assumes trusted local storage and ordinary process termination. Power
loss, whole-store rollback, delivery to a remote peer and authentication of the host
caller have separate boundaries.
