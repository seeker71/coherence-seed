define verify_key_recovery
@set -eu; \
  work=$$(mktemp -d); \
  trap 'rm -rf "$$work"' EXIT; \
  word() { printf %s "$$1" | ./ck text lens/to-nodes.ck lens/all.ck; }; \
  row() { printf %s "$$2" > "$$work/label"; ./ck text "ground/$$1.ck" 3<"$$work/label" <"$$work/none"; }; \
  entry() { ./ck text "$$1.ck" 3<"$$work/channel" <"$$work/$$2"; }; \
  decision() { ./ck text circle/apply.ck 3<"$$work/channel" 4<"$$work/$$1-change" <"$$work/$$2"; }; \
  refused() { why=$$1; shift; rc=0; "$$@" >"$$work/fault" || rc=$$?; test "$$rc" = 1 && test "$$(cat "$$work/fault")" = "fault $$(word "$$why")" || { echo "recovery expected $$why, received $$(cat "$$work/fault")" >&2; return 1; }; }; \
  word no >"$$work/none"; printf %s "$$(word peer.owner)" >"$$work/owner-role"; printf %s "$$(word share.ungranted)" >"$$work/ungranted"; printf %s "$$(word inbox.accepted)" >"$$work/accepted"; \
  for label in cancel false-cancel old new other old-pin enrolled alternative overlap recovery impostor no-possession new-as-old-episode stale-owner-signature old-owner old-listener new-owner new-listener old-owner-head old-listener-head old-expected new-expected old-observation new-observation; do row recovery-case "$$label" >"$$work/$$label"; done; \
  for label in owner-change listener-change listener-ticket; do row peer-case "$$label" >"$$work/$$label"; done; \
  mkdir "$$work/private" "$$work/letters"; printf %s "$$work" >"$$work/path"; printf channel >"$$work/label"; ./ck text ground/circle-exchange.ck 3<"$$work/label" <"$$work/path" >"$$work/channel"; \
  cp "$$work/old" "$$work/private/peers"; cp "$$work/old-pin" "$$work/private/authority"; cp "$$work/old-pin" "$$work/private/listening"; \
  refused rekey.not-enrolled entry recovery/apply recovery; refused rekey.invalid entry recovery/enroll overlap; \
  entry recovery/enroll enrolled | cmp - "$$work/enrolled"; refused ed25519.invalid-signature entry recovery/cancel false-cancel; \
  cp -R "$$work/private" "$$work/enrolled-copy"; entry recovery/cancel cancel | cmp - "$$work/cancel"; entry recovery/cancel cancel | cmp - "$$work/cancel"; refused rekey.withdrawn entry recovery/apply recovery; \
  mv "$$work/private" "$$work/canceled-scene"; mv "$$work/enrolled-copy" "$$work/private"; refused rekey.occupied entry recovery/enroll alternative; \
  refused ed25519.invalid-signature entry recovery/apply impostor; refused ed25519.invalid-signature entry recovery/apply no-possession; refused rekey.invalid entry recovery/apply new-as-old-episode; \
  cmp "$$work/old" "$$work/private/peers"; cmp "$$work/old-pin" "$$work/private/authority"; \
  decision owner old-owner >"$$work/owner-result"; decision listener old-listener >"$$work/listener-result"; entry mesh/expect listener-ticket | cmp - "$$work/old-expected"; entry mesh/confirm old-observation | cmp - "$$work/old-observation"; \
  cp -R "$$work/private" "$$work/uninformed-private"; \
  printf rename >"$$work/operation"; printf %s "$$work/private/peers" >"$$work/target"; rc=0; \
  ./ck text recovery/apply.ck ground/continuation-interrupt.ck 3<"$$work/channel" 6<"$$work/target" 7<"$$work/operation" <"$$work/recovery" >"$$work/interrupted" || rc=$$?; \
  test "$$rc" = 76; test ! -s "$$work/interrupted"; cmp "$$work/new" "$$work/private/peers"; cmp "$$work/old-owner-head" "$$work/private/authority"; \
  refused inbox.pinned entry circle/receive listener-ticket; refused peer.wrong-control decision owner old-owner; refused inbox.pinned decision owner new-owner; \
  entry recovery/apply recovery >"$$work/new-id"; ./ck text circle/pin.ck <"$$work/new" >"$$work/new-pin"; cmp "$$work/new-pin" "$$work/private/authority"; cmp "$$work/new-pin" "$$work/private/listening"; \
  set -- "$$work/private/retired-register-"*; test "$$#" = 2; if cmp -s "$$1" "$$work/old-owner-head"; then cmp "$$2" "$$work/old-listener-head"; else cmp "$$2" "$$work/old-owner-head"; cmp "$$1" "$$work/old-listener-head"; fi; \
  refused rekey.wrong-episode entry recovery/cancel cancel; refused e.id entry mesh/confirm old-observation; refused peer.wrong-control decision owner old-owner; refused ed25519.invalid-signature decision owner stale-owner-signature; \
  entry circle/receive listener-ticket >"$$work/empty-result"; printf status >"$$work/label"; ./ck text ground/circle-exchange.ck 3<"$$work/label" <"$$work/empty-result" | cmp - "$$work/ungranted"; test -z "$$(ls -A "$$work/letters")"; \
  decision owner new-owner >"$$work/new-owner-result"; decision listener new-listener >"$$work/new-listener-result"; cp "$$work/private/authority" "$$work/new-owner-head"; cp "$$work/private/listening" "$$work/new-listener-head"; \
  entry recovery/apply recovery | cmp - "$$work/new-id"; cmp "$$work/new-owner-head" "$$work/private/authority"; cmp "$$work/new-listener-head" "$$work/private/listening"; \
  entry mesh/expect listener-ticket | cmp - "$$work/new-expected"; entry mesh/confirm new-observation | cmp - "$$work/new-observation"; refused e.id entry mesh/confirm old-observation; \
  entry circle/receive listener-ticket >"$$work/live-result"; printf status >"$$work/label"; ./ck text ground/circle-exchange.ck 3<"$$work/label" <"$$work/live-result" | cmp - "$$work/accepted"; set -- "$$work/letters/"*; test "$$#" = 1; \
  ./ck text circle/check-proof.ck 3<"$$work/old" 4<"$$work/owner-change" <"$$work/old-owner" | cmp - "$$work/owner-role"; \
  refused peer.wrong-control ./ck text circle/check-proof.ck 3<"$$work/new" 4<"$$work/owner-change" <"$$work/old-owner"; \
  mkdir "$$work/offline" "$$work/offline/letters"; mv "$$work/uninformed-private" "$$work/offline/private"; printf %s "$$work/offline" >"$$work/path"; printf channel >"$$work/label"; ./ck text ground/circle-exchange.ck 3<"$$work/label" <"$$work/path" >"$$work/offline-channel"; \
  ./ck text circle/receive.ck 3<"$$work/offline-channel" <"$$work/listener-ticket" >"$$work/offline-result"; printf status >"$$work/label"; ./ck text ground/circle-exchange.ck 3<"$$work/label" <"$$work/offline-result" | cmp - "$$work/accepted"; \
  mkdir "$$work/unchartered" "$$work/unchartered/private" "$$work/unchartered/letters"; printf %s "$$work/unchartered" >"$$work/path"; printf channel >"$$work/label"; ./ck text ground/circle-exchange.ck 3<"$$work/label" <"$$work/path" >"$$work/late-channel"; \
  cp "$$work/old" "$$work/unchartered/private/peers"; cp "$$work/old-owner-head" "$$work/unchartered/private/authority"; cp "$$work/old-listener-head" "$$work/unchartered/private/listening"; \
  refused rekey.occupied ./ck text recovery/enroll.ck 3<"$$work/late-channel" <"$$work/enrolled"; refused rekey.not-enrolled ./ck text recovery/apply.ck 3<"$$work/late-channel" <"$$work/recovery"; \
  cp "$$work/other" "$$work/unchartered/private/peers"; ./ck text circle/pin.ck <"$$work/other" >"$$work/unchartered/private/authority"; cp "$$work/unchartered/private/authority" "$$work/unchartered/private/listening"; \
  refused rekey.wrong-episode ./ck text recovery/enroll.ck 3<"$$work/late-channel" <"$$work/enrolled"; \
  echo "recovery: pre-enrolled independent and replacement signatures restored control after owner key loss and interruption; stale owner and receipt refused, fresh control and delivery verified"
endef
