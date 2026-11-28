# Recovering an enrolled owner key

A trusted local broker can recognize a replacement owner key after the original
key is lost. At initial enrollment, the original owner signs a charter naming
one independent recovery key. The charter includes the complete original roster.
The normal owner, the listener and the recovery delegate have distinct keys. An
operator installs this charter while the protected channel's signed source and
listener registers are still at their original empty pin.

| Command | Standard input | Other input | Result |
| --- | --- | --- | --- |
| `recovery/signing-bytes.ck` | A charter, cancellation notice or transfer | None | The 32 raw bytes to sign with detached Ed25519 |
| `recovery/enroll.ck` | `rekey.enrollment(charter, owner signature)` | Channel on fd 3 | The protected original charter and signature |
| `recovery/cancel.ck` | `rekey.cancellation(notice, owner signature)` | Channel on fd 3 | A retained withdrawal of this charter |
| `recovery/apply.ck` | `rekey.proof(transfer, delegate signature, replacement signature)` | Channel on fd 3 | The successor roster's content identity |

The notice names the exact original roster and charter by content identity.
The transfer additionally contains the complete successor roster. The successor
keeps the owner label, circle, recipient and listener key. It supplies a fresh
32-byte episode and a new owner public key distinct from the delegate. Both
signers review and sign the same transfer. The broker checks their signatures
against its stored charter and the proposed successor. The original owner can
withdraw the charter until the broker durably accepts a transfer. Applying a
withdrawn charter or supplying a charter for the first time during recovery
fails. A recovered owner can enroll a new charter while the new episode is
pristine.

## Initial enrollment

For a new circle, `make circle-propose` produces a public roster with a fresh
episode. Each role can inspect the names, circle and public keys locally with
`make circle-inspect`. On their own machine each runs `make circle-endorse`
with their role, the same roster file and their local key directory.
The signed message is the 32-byte content ID of a typed `pact.acceptance`
containing the roster ID and the signing role. A signature over a grant,
invitation, recovery charter or another episode cannot stand in for this
endorsement.

`make circle-establish` verifies the two signatures before creating a broker.
Under the broker directory lock it prepares the original roster, the two empty
consent registers and a `pact.record` with both endorsements. A single
directory rename makes them available together. Repeating an interrupted
setup reconstructs the unpublished files. Once published, an identical
request returns the original roster and preserves the current registers.
This makes the original `private/first-roster` evidence available after
later operations. Recovery subsequently uses the currently pinned roster.

A rehearsal exchanged the role signatures and failed before a broker existed.
Two worker processes then finished an incomplete staged directory and returned
the same verified roster. Repeating establishment after delivery preserved
the advanced registers. Another episode endorsed by the same two keys
established a fresh broker and was refused by the original.

The broker directory is private to its operator. The filesystem must support
directory locks and same-filesystem directory rename. The tests cover process
restart. Power-loss guarantees depend on the underlying storage. Public key
ownership by a particular person requires their own corroboration.
[Participant commands](participant-circle.md#establish-the-circle)

## Preparing and reviewing a recovery

The participants exchange public statements and detached signatures. The
delegate keeps its own signing key. Use `recovery/draft.ck` with a mode on
file descriptor 3 to prepare the exact statement each signer will review.

| Mode | Standard input | File descriptor 4 | Statement |
| --- | --- | --- | --- |
| `charter` | Original roster | Delegate's raw 32-byte public key | Original roster and appointed delegate |
| `notice` | Charter | Unused | Withdrawal naming the original roster and charter |
| `transfer` | Charter | Proposed successor roster | Both predecessor identities and the complete successor |

For example, prepare a transfer and render its names and keys locally.

```sh
printf transfer > mode
./ck text recovery/draft.ck 3<mode 4<successor.roster <charter >transfer
./ck text lens/show-canonical.ck lens/all.ck <transfer
./ck text recovery/signing-bytes.ck <transfer >transfer.id
```

Each signer independently compares the proposal with their own retained
roster, charter and expected replacement public key, then signs the 32-byte
statement identity. `recovery/seal.ck` takes the reviewed statement on
standard input and the detached signature on file descriptor 4. For a
transfer, give it the delegate's signature on 4 and the replacement owner's
signature on 5. Sealing assembles the packet. The protected broker verifies
the signatures, episode, charter and current state when it installs it.

```sh
./ck text recovery/seal.ck 4<delegate.sig 5<replacement.sig <transfer >proof
BROKER="$broker" make -s circle-recovery-apply <proof
```

The same broker interface accepts a sealed original charter through
`circle-recovery-enroll` and a sealed notice through `circle-recovery-cancel`.
It derives the route from its installed roster. Either cell can independently
authenticate a complete historical participant packet offline. With only
its retained public roster, `ROSTER=original.roster make -s circle-verify
<signed.packet` returns the role that signed it. The successor roster
rejects a predecessor packet even when the listener's key stayed the same.
This comparison authenticates a past decision. Current permission lives in
the broker's registers.

## What crosses the restart

Under the existing directory lock the broker retains the signed decision and
retirement markers, then atomically replaces its public roster. This change
fences operations using either stale register. It archives the complete old
source and listener registers, including their signed controls, before replacing
each with an empty pin for the successor. Restart with the same signed transfer.
Any already advanced successor register stays intact. A different transfer for
the same predecessor cannot replace the retained decision. Retired owner keys
and episode identifiers remain ineligible for reuse at the original store.

The successor must issue its own source grant, and the listener must sign an
invitation under the successor roster. Existing episode-bound jobs, permissions
and receipts keep their original identity. A verifier retaining the original
roster can verify its historical signatures. A broker on the successor roster
rejects those signatures for new authority.

## What was observed

Two independently operated cells, Ash and Elm, established a local broker.
Ash appointed a separately operated delegate before either cell advanced its
register. Ash withdrew and Elm paused. Each kept its own public evidence.
Ash removed the disposable original signing file and could no longer sign
a fresh challenge with it. The delegate and replacement owner independently
reconstructed and signed the same transfer.

The broker archived both original registers byte for byte. Elm verified all
four controls, two from each episode. Each authenticated with its original
roster and failed with the other one. Submitting the retired controls at the
updated broker returned `peer.wrong-control`. Fresh listening consent alone
produced no letter. After Ash granted the excerpt, Elm received 60 attributed
bytes. Reapplying the transfer preserved both advanced successor registers
and the same delivered letter. All roles ran on one host with distinct
private directories and process profiles.

`make check-held-back` uses public-only fixtures from independent ephemeral
Ed25519 keys. The original private key was discarded before exercising recovery.
A worker terminated immediately after publishing the successor roster and before
updating either register. Neither the stale owner proof nor the fresh proof
could advance the old-pinned register during that gap. A fresh worker resumed,
archived the two original signed heads, and preserved the successor’s later
grant and invitation on another replay. The old key signing a successor control
failed verification. The valid old mesh receipt failed on the successor.
A fresh signed grant, listener invitation and mesh receipt succeeded under
the new episode, and the normal receive command delivered a letter.

A separately retained, uninformed copy of the old broker still accepted its old
authorization. Recovery reaches the protected store that applied it. Independent
brokers and mesh time witnesses require their own authenticated enrollment or
transition and contact. Whole-store rollback, equivalent copies of secret keys,
power-loss durability of directory renames, threshold or social recovery, and a
natural person's identity or experience remain outside this evidence.
