# A private account and a shared excerpt

Recording a first-person account is optional. With no account, the circle can
remain quiet. A journal entry keeps its author's exact words in their own key
directory. Sharing requires a separately chosen excerpt and a fresh, exact
decision by both the owner and listener. The listener sees the author’s label
and selected words. An account reports what its author said. Health outcomes,
reasons for a pause and anyone else's experience require their own evidence.

These commands run from a built seed checkout with Make and an Ed25519-capable
OpenSSL. Commands that accept wording read literal standard input, including
line breaks. The examples use fictional wording. Run the role commands in each
person's own environment. Keep private inputs and outputs in a directory that
only its owner can access. Set `umask 077` before creating redirected files.

## Establish the circle

1. Each person chooses a new local directory for `KEY_DIR` and runs
   `make -s circle-key`. Exchange the 32-byte `public` file. The private
   `signing.pem` stays with the person who created it. Protect it using local
   account permissions and device storage controls. The journal stores canonical
   records locally without encryption. Use separate operating-system accounts
   or devices when the participants need private custody from one another.
   A process with direct read access to a journal can inspect its files.
2. The organizer sets `OWNER`, `LISTENER`, `CIRCLE`, `OWNER_PUBLIC` and
   `LISTENER_PUBLIC`, then runs `make -s circle-propose > roster.ck`. Both
   participants get the same proposed public roster. Each sets `ROSTER` to
   that file and uses `make -s circle-inspect` to review the proposal.
3. The owner runs `ROLE=owner make -s circle-endorse > owner.ok` with their
   own `KEY_DIR`. The listener runs `ROLE=listener make -s circle-endorse >
   listener.ok` with theirs. Only endorsements and public material go to the
   broker. Its operator sets `BROKER`, `ROSTER`, `OWNER_ENDORSEMENT` and
   `LISTENER_ENDORSEMENT` and runs `make -s circle-establish`.
4. Either participant can independently opt into a private journal with
   `make -s circle-join`, using the original `ROSTER` and their own `KEY_DIR`.
   They may also leave it untouched. Joining opens an empty journal. Each
   author has a separate local history bearing their enrolled identity.

The broker commands below use its protected directory. A participant operating
elsewhere exchanges portable revision, ticket and signed packet files with the
broker. The broker alone needs its register and delivery directories. Both
roles use their respective trusted copy of the roster.

## Write, review or retract

In either participant’s own environment, make the first private entry and save its revision.

```sh
printf '%s\n' 'Private context for me.' 'I would like a quiet moment.' |
  make -s circle-record > edition.ck
make -s circle-show
```

`circle-edition` prints the current revision to a file. To correct, pass the
revision you reviewed with `EDITION=edition.ck make -s circle-correct` and
supply the complete replacement wording on standard input. Capture the new
revision. A stale edit is refused. To retract the statement, run
`EDITION=edition.ck make -s circle-retract > retracted.ck`. To resume,
supply new wording to `EDITION=retracted.ck make -s circle-resume`.
A private retraction and a broker withdrawal are independent decisions.
Retraction preserves private history. Already delivered words remain with
their recipient.

## Select and deliver

The owner selects the exact part of the current wording to share, reviews the
destination, signed window and exact wording locally, then creates a separate invitation. Set `UNTIL` to the exclusive
Unix expiry second of this offer. The local permit contains the selected words.
The invitation contains only a random nonce and a commitment to the exact scope.

```sh
printf %s 'I would like a quiet moment.' |
  EDITION=edition.ck UNTIL="$end_second" make -s circle-select > permit.ck
make -s circle-preview < permit.ck
make -s circle-ticket < permit.ck > invitation.ck
```

The broker returns the current revision of each role with `circle-owner-head`
and `circle-listener-head`. Each participant receives only the revision needed
for their decision. The owner signs the grant in their own environment. The
listener can sign the invitation in theirs or leave it unanswered.

```sh
# Owner
ACTION=share HEAD=owner-head.ck make -s circle-sign < permit.ck > grant.ck

# Listener
ACTION=listen HEAD=listener-head.ck make -s circle-sign < invitation.ck > listen.ck

# Broker
make -s circle-submit < grant.ck > next-owner.ck
make -s circle-submit < listen.ck > next-listener.ck
make -s circle-read < invitation.ck
```

Successful read publishes one letter through the regular broker and renders
the owner's attributed words. Repeating the read while consent remains valid
reuses the same letter. To pause, the listener signs with `ACTION=pause` and
their current `HEAD`. To withdraw a grant, the owner signs with `ACTION=withdraw`
and theirs. Both commands take empty standard input and produce a signed packet
that takes effect when submitted. A subsequent offer, including corrected
wording, needs its own current grant and matching invitation.

The owner can run `make -s circle-source < permit.ck` to recover the exact
private edition from which that particular excerpt was selected.
Two offers for the same words can lead to different private editions. The
private origin, surrounding wording and earlier editions stay in the journal.
A selected permit documents a proposal. The broker’s signed registers document
actual permission.

## Send an excerpt back to the original speaker

The original listener can record, review, correct, retract and resume privately
under the original roster. The original owner's revisions and offers give the
listener no access to the owner's private journal. If the listener chooses to
send an excerpt back, they use the revision from their own original journal.

```sh
# Original listener, still using the original ROSTER and their own KEY_DIR
printf %s 'I can listen for a while.' |
  EDITION=my-edition.ck UNTIL="$end_second" make -s circle-select-return > return-permit.ck
make -s circle-source < return-permit.ck
make -s circle-propose-return > return-roster.ck
```

The proposed return roster identifies the same circle and two keys, reverses
their roles and binds a distinct episode to the original roster. Each person
inspects and endorses that roster. The original listener endorses as its owner, and
the original speaker endorses as its listener. A broker operator establishes
the return roster in a separate `BROKER` directory. With `ROSTER` set to
`return-roster.ck`, use the ordinary preview, ticket, head, sign, submit and
read commands above. The return channel starts without a grant or invitation.
The original listener grants the excerpt and the original speaker chooses
whether to receive it. Returning to the original `ROSTER` lets each person
continue their private journal. The complete entry stays there.

## What was checked

Two synthetic role keys exercised these ordinary commands. They recorded
identical words in separate private journals and obtained distinct revisions.
Both authors corrected, retracted and resumed their own accounts. One listener
excerpt reached the original speaker through its separately endorsed return
channel and fresh decisions. Withdrawing the return grant stopped later reads. An unanswered offer
left the recipient directory empty. With both decisions, the recipient read
exactly the selected excerpt attributed to its owner. Pause and withdrawal
stopped subsequent reads. Fresh decisions delivered the corrected excerpt.
Even with two valid signatures, inconsistent source attribution and payload
were refused before a file was published. Another episode with the same names
and keys rejected the original selection and signed read.

The private journal retains a unique successor for every committed revision.
Restart can follow a complete stored successor if a cached head is behind.
Missing, malformed or unreadable heads fail closed. Complete rollback or loss
of the trusted journal, loss of private keys, and the identity of a human
holding a local credential need additional evidence. These fictional entries
provide no participant testimony.
