# Local stewardship and voluntary contributions

The receiver measures the payload bytes of a committed letter. Its account
tracks these bytes separately from the voluntary units a person offered or
received. An unanswered invitation produces no letter. A delivered record of
silence, refusal, consent or care contributes only its payload byte count.

| Command | Result |
| --- | --- |
| `./ck text value/output.ck <component` | Read the named public component and return its content identity as a byte node. |
| `./ck text value/post.ck 3<channel <ticket` | Deliver under the current owner grant and recipient invitation, then return the scoped `value.receipt`. |
| `./ck text value/statement.ck 3<channel <person` | Reconcile a pending effect and return that person's receiver bytes, units offered and units received on this channel. |
| `./ck text value/unconfirmed.ck 3<channel` | Reconcile visible letters and return the still-unresolved intents in the channel custodian's private store. |

The inputs are canonical Form. A backend output scope names `value.material`
as its source, `value.raw` as its fidelity and `value.output` as its purpose.
The material names either `value.interface` for the stewardship component or
`value.operations` for the installed operation declarations. The receiver
recomputes the identity from that public file and checks the exact payload.
An application can add public providers to this catalogue. The request carries
a component name; the installed catalogue supplies its file path.

A voluntary offer has its own occurrence, grant and recipient invitation. Its
scope names `value.offer` with the prior service receipt's identity and a
positive integer amount. Its fidelity is `value.voluntary`, its purpose is
`value.contribute` and its payload is empty. The receipt must belong to the
same owner, recipient, circle, key roster and storage locations. A receipt of
a social activity cannot qualify as a service, even if a later request
relabels its identity. A second offer against the same service or a conflicting
use of an occurrence returns a `value.disagreement` with the occurrence key,
identity of the previous evidence and proposed scope identity. A matching
retry returns the original receipt.

Under the inbox directory lock, the receiver saves the intended receipt,
letter path and expected letter identity before delivery. On restart,
accounting checks the immutable letter and reconciles the saved intent.
A withdrawal prevents a new delivery. Accounting can still recognize a
previously published letter. The private ledger retains the two control
identities, scope and source identities, roles and amounts. Anyone who can
read the private directory has access to these records.

If a reader collects a previously accounted letter, retrying its ticket
reports the absent path and retained receipt identity. A different letter
at its path produces a disagreement. If the receiver stopped after staging
an intent and the letter is absent, the two possible outcomes are
indistinguishable. The receiver retains the original intent for review.
Retry returns `value.unconfirmed` and its intent ID. A second voluntary
offer on the same underlying service is also held while its first offer is
unresolved. Other occurrences can proceed. The statement contains confirmed
amounts, and the unresolved intents remain separately inspectable.

The maintained run moved a published letter before the private
acknowledgment. Retry left the destination empty. Restoring the exact
letter reconciled the original receipt and increased the recipient total
once. The same check interrupts before publication and verifies that
unrelated work can proceed. A permanently missing original needs independent
evidence to resolve what happened.

`make check-held-back` runs fresh owner, listener, receiver and statement
processes. Four protected transfers add four receiver bytes. A service adds
32 bytes. The first personal units arise from a separately invited offer of
seven. Withdrawing after the successful link and before the acknowledgment
still yields one contribution on restart. The ground also checks changed
terms, another owner, missing invitation, integer limits and duplicate
receipts. The units are local records of voluntary support. Actual money
settlement, machine energy costs, independent accounting stores and
power-loss recovery require separate evidence.
