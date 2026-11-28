define verify_exchange
@set -eu; \
  work=$$(mktemp -d); trap 'rm -rf "$$work"' EXIT; mkdir "$$work/private" "$$work/letters"; \
  gen() { printf %s "$$1" > "$$work/mode"; ./ck text ground/circle-exchange.ck 3<"$$work/mode" 4<"$$work/$${4:-none}" <"$$work/$$2" >"$$work/$$3"; }; \
  send() { ./ck text "circle/$$1.ck" 3<"$$work/channel" <"$$work/$$2" >"$$work/$$3"; }; \
  expect() { gen status "$$1" status; actual=$$(cat "$$work/status"); wanted=$$(printf %s "$$2" | $(CANONICAL_WORDS)); [ "$$actual" = "$$wanted" ] || { echo "circle $$1: $$actual, expected $$2" >&2; exit 1; }; }; \
  empty() { [ -z "$$(ls -A "$$work/letters")" ] || { echo "circle: premature publication" >&2; exit 1; }; }; \
  change() { gen change "$$2" update "$$3"; send "$$1" update "$$4"; }; \
  printf '#32' > "$$work/none"; cp "$$work/none" "$$work/private/authority"; cp "$$work/none" "$$work/private/listening"; \
  printf %s "$$work" > "$$work/path"; printf '[#2 31]' > "$$work/first"; printf '[#2 32]' > "$$work/second"; \
  gen channel path channel; gen identity none genesis; \
  gen open none opening; gen propose opening proposal; gen correct proposal correction; \
  gen decline correction refusal; gen silence refusal quiet; gen scope quiet scope; gen other-scope proposal alternative; \
  gen permit scope offered first; gen ticket scope ticket first; gen ticket alternative wrong first; \
  witness_case() { printf %s "$$1" > "$$work/witness-mode"; ./ck text ground/witness-case.ck 3<"$$work/witness-mode" 4<"$$work/$${4:-none}" <"$$work/$$2" >"$$work/$$3"; }; \
  witness_word() { printf %s "$$1" | $(CANONICAL_WORDS); }; \
  observe_listener() { ./ck text witness/observe.ck 3<"$$work/invitation" 4<"$$work/$$1" 5<"$$work/private/listening" <"$$work/reply" >"$$work/$$2"; witness_case status "$$2" observed-status; test "$$(cat "$$work/observed-status")" = "$$(witness_word "$$3")"; }; \
  witness_case claim none false-claim; witness_word '"source reader"' >"$$work/reader"; \
  ./ck text witness/check.ck 3<ground/witness-sources.ck 4<"$$work/reader" <"$$work/false-claim" >"$$work/review"; \
  witness_case check-review review checked; test "$$(cat "$$work/checked")" = "$$(witness_word yes)"; \
  ./ck text witness/retain.ck 3<"$$work/reader" <"$$work/review" >"$$work/review-receipt"; \
  witness_case list review reviews; witness_case list review-receipt review-receipts; \
  ./ck text witness/recall.ck 3<ground/witness-sources.ck 4<"$$work/reviews" 5<"$$work/review-receipts" <"$$work/false-claim" | cmp - "$$work/reviews"; \
  witness_case invitation scope invitation ticket; \
  change authorize offered genesis sender; send receive ticket reply; expect reply inbox.paused; empty; \
  observe_listener none unanswered witness.unanswered; \
  change listen ticket genesis listener; mkdir -p "$$work/expiry/private" "$$work/expiry/letters"; cp "$$work/private/authority" "$$work/private/listening" "$$work/expiry/private"; \
  printf %s "$$work/expiry" > "$$work/expiry-path"; gen channel expiry-path expiry-channel; gen expire-during-stage ticket reply expiry-channel; expect reply share.expired; test -z "$$(ls -A "$$work/expiry/letters")"; \
  send receive wrong reply; expect reply share.different; empty; \
  cp "$$work/private/listening" "$$work/before-stop"; \
  change listen none listener paused; change listen ticket genesis stale; \
  [ "$$(cat "$$work/stale")" = "$$(printf inbox.stale | $(CANONICAL_WORDS))" ]; \
  send receive ticket reply; expect reply inbox.paused; empty; \
  observe_listener before-stop stopped witness.stopped; \
  witness_word '"B"' >"$$work/participant"; witness_word '"synthetic account v1"' >"$$work/first-text"; witness_word '"synthetic account v2"' >"$$work/second-text"; \
  witness_case request stopped account-request first-text; \
  ./ck text witness/account.ck 3<"$$work/stopped" 4<"$$work/none" 5<"$$work/participant" <"$$work/account-request" >"$$work/account"; \
  witness_case request account account-correction second-text; \
  ./ck text witness/account.ck 3<"$$work/stopped" 4<"$$work/account" 5<"$$work/participant" <"$$work/account-correction" >"$$work/corrected-account"; \
  witness_case check-account corrected-account checked account; test "$$(cat "$$work/checked")" = "$$(witness_word yes)"; \
  change listen ticket paused listener; \
  interrupt() { printf %s "$$1" > "$$work/phase"; rc=0; ./ck text circle/receive.ck ground/circle-interrupt.ck 3<"$$work/channel" 4<"$$work/phase" <"$$work/ticket" >"$$work/discarded" || rc=$$?; [ "$$rc" = "$$2" ] && [ ! -s "$$work/discarded" ]; }; \
  interrupt before 75; empty; interrupt after 76; \
  set -- "$$work/letters/"*; [ "$$#" = 1 ]; ln "$$1" "$$work/original"; gen bytes original seen; printf 'Kami boleh berbagi' | cmp - "$$work/seen"; \
  send receive ticket reply; expect reply inbox.replayed; gen reference reply confirmed; cmp "$$work/confirmed" "$$work/ticket"; test "$$1" -ef "$$work/original"; \
  mv "$$work/letters" "$$work/asleep"; rc=0; send receive ticket failed || rc=$$?; [ "$$rc" = 1 ]; \
  reason=$$(printf '(list inbox.io "publish" "%s")' "$$1" | $(CANONICAL_WORDS)); \
  [ "$$(cat "$$work/failed")" = "fault $$reason" ]; [ ! -e "$$work/private/pending" ]; mv "$$work/asleep" "$$work/letters"; \
  gen permit alternative other-permit first; change authorize other-permit sender sender; change listen wrong listener listener; \
  send receive wrong reply; expect reply inbox.conflict; gen bytes original intact; cmp "$$work/intact" "$$work/seen"; test "$$1" -ef "$$work/original"; \
  change authorize none sender stopped; send receive wrong reply; expect reply share.ungranted; \
  gen permit scope another second; gen ticket scope ticket-two second; change authorize another stopped sender; change listen ticket-two listener listener; \
  send receive ticket-two race-a & first_pid=$$!; send receive ticket-two race-b & second_pid=$$!; wait "$$first_pid"; wait "$$second_pid"; \
  gen status race-a a; gen status race-b b; accepted=$$(printf inbox.accepted | $(CANONICAL_WORDS)); \
  if [ "$$(cat "$$work/a")" = "$$accepted" ]; then expect race-b inbox.replayed; else expect race-a inbox.replayed; expect race-b inbox.accepted; fi; \
  set -- "$$work/letters/"*; [ "$$#" = 2 ]; for entry in "$$@"; do gen bytes "letters/$${entry##*/}" held; cmp "$$work/held" "$$work/seen"; done; \
  ./ck text ground/turn-projection.ck > "$$work/turn_projection.py"; \
  if command -v python3 >/dev/null 2>&1; then python3 -m doctest "$$work/turn_projection.py"; fi; \
  echo "circle: two roles retained their readings; consent, expiry and replay matched the letters in the receiving directory"
endef

define verify_signed_controls
@set -eu; \
  work=$$(mktemp -d); trap 'rm -rf "$$work"' EXIT; mkdir "$$work/private" "$$work/letters"; printf '#32' > "$$work/none"; \
  sample() { printf %s "$$1" > "$$work/mode"; ./ck text ground/peer-case.ck 3<"$$work/mode" <"$$work/$${2:-none}"; }; \
  word() { printf %s "$$1" | $(CANONICAL_WORDS); }; \
  apply() { ./ck text circle/apply.ck 3<"$$work/channel" 4<"$$work/$$1" <"$$work/$$2"; }; \
  verify() { ./ck text circle/check-proof.ck 3<"$$work/$$1" 4<"$$work/$$2" <"$$work/$$3"; }; \
  refused() { expected=$$1; shift; rc=0; "$$@" >"$$work/reason" || rc=$$?; actual=$$(cat "$$work/reason"); [ "$$rc" = 1 ] && [ "$$actual" = "fault $$(word "$$expected")" ] || { echo "signed control: $$actual, expected $$expected" >&2; return 1; }; }; \
  for item in roster owner-change listener-change withdraw-change owner-proof listener-proof withdraw-proof listener-ticket wrong-episode same-keys owner-other-proof listener-other-proof; do sample "$$item" > "$$work/$$item"; done; \
  printf %s channel > "$$work/mode"; printf %s "$$work" | ./ck text ground/circle-exchange.ck 3<"$$work/mode" >"$$work/channel"; \
  cp "$$work/roster" "$$work/private/peers"; ./ck text circle/pin.ck < "$$work/roster" > "$$work/pin"; \
  ./ck text form/from-print.ck <"$$work/roster" >"$$work/roster.records"; ./ck text circle/pin.ck <"$$work/roster.records" | cmp - "$$work/pin"; \
  cat "$$work/roster.records" "$$work/roster.records" >"$$work/two-rosters"; refused '(list inbox.invalid "/dev/stdin")' ./ck text circle/pin.ck <"$$work/two-rosters"; \
  cp "$$work/pin" "$$work/private/authority"; cp "$$work/pin" "$$work/private/listening"; \
  for role in owner listener; do sample "$$role-intent" "$$role-change" >"$$work/intent"; ./ck text circle/statement.ck 3<"$$work/roster" <"$$work/intent" >"$$work/control"; ./ck text circle/signing-bytes.ck <"$$work/control" >"$$work/digest"; sample "$$role-digest" | cmp - "$$work/digest"; sample signature "$$role-proof" >"$$work/sig"; ./ck text circle/seal.ck 4<"$$work/sig" <"$$work/control" >"$$work/built"; cmp "$$work/$$role-proof" "$$work/built"; done; \
  refused peer.invalid-roster ./ck text circle/pin.ck < "$$work/same-keys"; \
  refused inbox.pinned ./ck text circle/authorize.ck 3<"$$work/channel" <"$$work/owner-change"; \
  refused peer.wrong-control verify wrong-episode listener-change listener-proof; \
  refused peer.wrong-control apply withdraw-change owner-proof; \
  cmp "$$work/pin" "$$work/private/authority"; cmp "$$work/pin" "$$work/private/listening"; \
  adapter=$$(./ck run ground/ed25519.ck); \
  if [ "$$adapter" = "node $$(word ed25519.unavailable)" ]; then \
    refused ed25519.unavailable apply listener-change listener-proof; cmp "$$work/pin" "$$work/private/listening"; echo "signed control: provider unavailable, pinned register unchanged"; exit 0; \
  fi; \
  test "$$adapter" = "node $$(word ed25519.verified)"; \
  cp "$$work/none" "$$work/private/authority"; refused inbox.pinned apply owner-change owner-proof; cmp "$$work/none" "$$work/private/authority"; cp "$$work/pin" "$$work/private/authority"; \
  sample control owner-proof >"$$work/control"; sample signature listener-proof >"$$work/sig"; \
  ./ck text circle/seal.ck 4<"$$work/sig" <"$$work/control" >"$$work/swapped"; \
  refused ed25519.invalid-signature apply owner-change swapped; cmp "$$work/pin" "$$work/private/authority"; \
  apply listener-change listener-proof > "$$work/listener-revision"; test "$$(verify roster listener-change listener-proof)" = "$$(word peer.listener)"; \
  receive() { ./ck text circle/receive.ck 3<"$$work/channel" <"$$work/listener-ticket" >"$$work/result"; printf status >"$$work/mode"; ./ck text ground/circle-exchange.ck 3<"$$work/mode" <"$$work/result"; }; \
  test "$$(receive)" = "$$(word share.ungranted)"; test -z "$$(ls -A "$$work/letters")"; \
  apply owner-change owner-proof >"$$work/revision"; apply owner-change owner-proof | cmp - "$$work/revision"; \
  sample witness private/authority | cmp - "$$work/owner-proof"; sample witness private/listening | cmp - "$$work/listener-proof"; \
  test "$$(receive)" = "$$(word inbox.accepted)"; set -- "$$work/letters/"*; test "$$#" = 1; printf bytes > "$$work/mode"; ./ck text ground/circle-exchange.ck 3<"$$work/mode" <"$$1" > "$$work/observed"; printf "Kami boleh berbagi" | cmp - "$$work/observed"; \
  apply withdraw-change withdraw-proof >"$$work/withdrawn"; test "$$(apply owner-change owner-proof)" = "$$(word inbox.stale)"; test "$$(receive)" = "$$(word share.ungranted)"; sample witness private/authority | cmp - "$$work/withdraw-proof"; \
  mv "$$work/private" "$$work/previous"; mkdir "$$work/private"; cp "$$work/wrong-episode" "$$work/private/peers"; \
  ./ck text circle/pin.ck <"$$work/wrong-episode" >"$$work/private/authority"; cp "$$work/private/authority" "$$work/private/listening"; \
  refused peer.wrong-control apply listener-change listener-proof; apply listener-change listener-other-proof >"$$work/second-listener"; apply owner-change owner-other-proof >"$$work/second-owner"; \
  test "$$(receive)" = "$$(word inbox.accepted)"; set -- "$$work/letters/"*; test "$$#" = 2; \
  echo "signed control: two enrolled keys, scoped registers, retained signatures, exact recipient bytes and stale withdrawal replay agreed"; \
  for item in roster grant listener withdraw ticket grant-proof listener-proof withdraw-proof; do sample "finite-$$item" > "$$work/finite-$$item"; done; \
  ./ck text circle/pin.ck < "$$work/finite-roster" > "$$work/finite-pin"; \
  lease_apply() { \
    ./ck text circle/apply.ck 3<"$$work/$$1/channel" 4<"$$work/finite-$$2" <"$$work/finite-$$2-proof" >"$$work/$$1/revision"; \
  }; \
  lease_fresh() { \
    mkdir -p "$$work/$$1/private" "$$work/$$1/letters"; \
    printf %s channel > "$$work/mode"; \
    printf %s "$$work/$$1" | ./ck text ground/circle-exchange.ck 3<"$$work/mode" >"$$work/$$1/channel"; \
    cp "$$work/finite-roster" "$$work/$$1/private/peers"; cp "$$work/finite-pin" "$$work/$$1/private/authority"; cp "$$work/finite-pin" "$$work/$$1/private/listening"; \
    lease_apply "$$1" grant; lease_apply "$$1" listener; \
    sample witness "$$1/private/listening" | cmp - "$$work/finite-listener-proof"; \
  }; \
  lease_receive() { \
    word "$$2" > "$$work/$$1/clock"; \
    ./ck text circle/receive.ck ground/broker-clock.ck 3<"$$work/$$1/channel" 4<"$$work/$$1/clock" <"$$work/finite-ticket" >"$$work/$$1/result"; \
    printf %s reference > "$$work/mode"; ./ck text ground/circle-exchange.ck 3<"$$work/mode" <"$$work/$$1/result" | cmp - "$$work/finite-ticket"; \
    printf %s status > "$$work/mode"; observed=$$(./ck text ground/circle-exchange.ck 3<"$$work/mode" <"$$work/$$1/result"); expected=$$(word "$$3"); \
    [ "$$observed" = "$$expected" ] || { echo "finite grant at $$1, time $$2: expected $$3, observed $$observed" >&2; return 1; }; \
  }; \
  lease_evidence() { \
    sample witness "$$1/private/authority" | cmp - "$$work/finite-$$2-proof"; \
    if [ "$$3" = empty ]; then test -z "$$(ls -A "$$work/$$1/letters")"; else \
      broker=$$1; set -- "$$work/$$broker/letters/"*; test "$$#" = 1; test -f "$$1"; \
      printf %s bytes > "$$work/mode"; ./ck text ground/circle-exchange.ck 3<"$$work/mode" <"$$1" > "$$work/$$broker/bytes"; \
      printf 'Kami boleh berbagi' | cmp - "$$work/$$broker/bytes"; \
      if [ -f "$$work/$$broker/delivered" ]; then test "$$1" -ef "$$work/$$broker/delivered"; else ln "$$1" "$$work/$$broker/delivered"; fi; \
    fi; \
  }; \
  validity() { ./ck text circle/validity.ck 3<"$$work/finite-roster" 4<"$$work/finite-$$1" <"$$work/finite-$$2-proof"; }; \
  expect_validity() { sample control "finite-$$1-proof" > "$$work/control"; control=$$(./ck text lens/show-canonical.ck lens/all.ck <"$$work/control"); terms=$$(word "($$2 $$control $$3)"); actual=$$(validity "$$1" "$$1"); test "$$actual" = "$$terms"; }; \
  ticket=$$(./ck text lens/show-canonical.ck lens/all.ck <"$$work/finite-ticket"); expect_validity grant peer.window "$$ticket 100 110"; expect_validity withdraw peer.withdrawal ""; \
  refused peer.not-owner validity listener listener; refused peer.wrong-control validity withdraw grant; \
  lease_fresh first/owner; lease_fresh first/isolated; \
  lease_receive first/owner 99 share.early; lease_receive first/isolated 99 share.early; \
  lease_apply first/owner withdraw; lease_receive first/owner 100 share.ungranted; lease_evidence first/owner withdraw empty; \
  lease_evidence first/isolated grant empty; lease_receive first/isolated 100 inbox.accepted; lease_evidence first/isolated grant published; \
  lease_receive first/isolated 109 inbox.replayed; lease_receive first/isolated 110 share.expired; lease_evidence first/isolated grant published; \
  lease_apply first/isolated withdraw; lease_receive first/isolated 111 share.ungranted; lease_evidence first/isolated withdraw published; \
  lease_fresh second/owner; lease_fresh second/isolated; \
  lease_receive second/isolated "(list 109 110)" share.expired; lease_evidence second/isolated grant empty; \
  lease_receive second/owner 109 inbox.accepted; lease_evidence second/owner grant published; \
  lease_receive second/isolated 109 share.clock-regressed; lease_evidence second/isolated grant empty; \
  lease_apply second/isolated withdraw; lease_receive second/isolated 109 share.ungranted; lease_evidence second/isolated withdraw empty; \
  lease_fresh moving/isolated; lease_receive moving/isolated "(list 104 105 106)" share.clock-unsettled; lease_evidence moving/isolated grant empty; \
  lease_receive moving/isolated 105 share.clock-regressed; lease_receive moving/isolated 106 inbox.accepted; lease_evidence moving/isolated grant published; \
  echo "finite grant: two signed broker schedules agreed with the grant window, independent withdrawal witnesses, rollback and recipient files"
endef
