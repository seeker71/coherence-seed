# Reconnecting a broker to its sender

A sender can retain what it expects to reach a recipient and later verify what the enrolled recipient broker observed. `mesh/expect.ck` reads a ticket on stdin and the sender’s protected channel on fd 3. It records the roster ID, exact ticket, expected canonical letter ID and both signed control revision IDs. The sender can then disconnect. A pending expectation makes no claim that anything has arrived.

The receiving broker runs `mesh/deliver.ck` with its own protected channel on fd 3, the expectation on fd 4, and a live time-witness connection on stdin and fd 5. It compares its controls, asks the witness for time, and uses the circle broker’s normal grant, invitation, expiry and publication flow. Under the broker lock it reads the linked letter and compares its content ID to the commitment. It retains a signable observation of the expectation and the witnessed Unix second. After loss of an observation record it can attest when the letter was observed again.

`mesh/signing-bytes.ck` emits the 32 raw content-ID bytes for an observation on stdin. The recipient signs those bytes with its enrolled Ed25519 listener key. `mesh/seal.ck` takes the observation on stdin and its 64-byte signature on fd 4. The sender later passes this sealed receipt to `mesh/confirm.ck` with its protected channel on fd 3. The sender checks its retained expectation and enrolled listener key before saving the confirmation. Receipts for distinct tickets can reconnect in either order. The first valid receipt for an expectation stays available across restart. An identical duplicate is idempotent, and conflicting observations return `inbox.conflict`. A signature identifies the enrolled key that vouched for arrival. Human attention, consumption and continuing consent require their own evidence.

## Independent time and control revisions

The witness owner provisions a separate protected directory. `mesh/pin.ck` takes the public roster and emits the initial `mesh.watermark` for the witness’s `private/mesh-time` file. Provision that file once. A missing or corrupt head prevents service. The receiver only needs the private duplex connection that its supervisor established with this actor. It does not need access to the witness directory. Each delivery opens one session. On the witness side, `mesh/clock.ck` takes the witness channel on fd 3, reads requests on stdin and replies on fd 5.

The wire uses NUMS request and reply frames. A query names the episode and includes 32 fresh bytes from the receiver’s OS random source. The witness echoes both in its response. The receiver rejects a wrong episode, a prior challenge and a short reply. The connection’s identity and integrity come from its provisioned endpoints. A transport accepting arbitrary third-party writers needs additional authentication.

The witness retains the maximum valid Unix second and the last owner and listener revision IDs it has observed. A broker reports only those opaque IDs and their parents over the connection. The witness accepts the same head or a direct successor independently for each role. It samples its own clock and keeps the existing heads if the incoming ancestry conflicts. It holds its directory lock until the client closes the session. A competing checkpoint therefore waits through final publication. An interrupted connection releases the lock. An unclosed connection can delay other clients of that witness.

An owner can report newer controls before delivery using `mesh/checkpoint.ck` with its channel on fd 3 and the same private connection. `mesh/now.ck` takes the enrolled episode ID on fd 3 and samples time independently of a delivery. A missing intermediate control requires the holders to supply its signed history in order. A revocation still held only by a disconnected owner cannot inform either receiver or witness. Restoring both the receiver and the independent witness, or compromising their endpoint custody, exceeds this recovery model.

## What has run

`make check-held-back` provisions independent sender, witness and recipient roots. Two signed turns in the same enrollment use distinct tickets with identical payload bytes. The recipient’s observations match separately constructed commitments; the sender confirms B, then A, then B again across fresh processes. Altered time, an owner-role signature, a different enrolled episode, an unissued expectation and a conflicting valid listener observation all fail. The sender’s original confirmation remains retrievable.

With time as the sole witness state, restoring the entire receiver from before A and presenting that still-valid A after B wrote 137 stale bytes at second 125. With independently retained control heads, the same request returns `inbox.stale` and writes zero bytes. Moving the witness clock backward returns `share.clock-regressed`. A later owner checkpoint affects a receiver that has retained its older, genuinely signed control. The test also submits a real wall-clock sample and checks that the witness file stays locked for the open session.

In the independent VM replay, a Linux recipient with loopback alone contacted the macOS witness over a private second serial device. It started with zero letters, linked one 137-byte letter, and signed the actual observation inside the guest. Its 591-byte receipt matched an independently generated OpenSSL fixture. The host sender accepted those bytes after confirming B. The public local witness bundle at `~/Library/Caches/coherence-seed/mesh-receipt-v1` includes the guest image, kernel, serial transcript, expected bytes and replay driver. The fixture’s synthetic Ed25519 seeds derive from SHA-256 of `public synthetic mesh receipt v1 owner` and `public synthetic mesh receipt v1 listener`.

```sh
probe="$HOME/Library/Caches/coherence-seed/mesh-receipt-v1"
python3 "$probe/guest-run.py" "$PWD"
MESH_WITNESS="$probe/guest-delivery" make check-held-back
```
