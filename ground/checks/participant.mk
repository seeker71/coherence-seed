define participant_recovery
(set -eu; \
  BROKER="$$work/recovering"; p establish >/dev/null; \
  for key in delegate replacement; do KEY_DIR="$$work/$$key" p key; done; \
  draft() { printf %s "$$1" >"$$work/recovery-mode"; ./ck text recovery/draft.ck 3<"$$work/recovery-mode" 4<"$${2:-/dev/null}"; }; \
  sign_statement() { ./ck text recovery/signing-bytes.ck <"$$work/$$2" >"$$work/signing-bytes"; openssl pkeyutl -sign -rawin -inkey "$$work/$$1/signing.pem" -in "$$work/signing-bytes"; }; \
  seal() { ./ck text recovery/seal.ck 4<"$$work/$$2" 5<"$${3:-/dev/null}" <"$$work/$$1"; }; \
  draft charter "$$work/delegate/public" <"$$ROSTER" >"$$work/charter"; \
  sign_statement owner charter >"$$work/appointment"; seal charter appointment >"$$work/enrollment"; \
  p recovery-enroll <"$$work/enrollment" | cmp - "$$work/enrollment"; \
  p owner-head >"$$work/old-revision"; \
  KEY_DIR="$$work/owner" ACTION=withdraw HEAD="$$work/old-revision" p sign </dev/null >"$$work/old-packet"; \
  mkdir "$$work/cold"; ln -s "$$PWD/bootstrap" "$$work/cold/bootstrap"; \
  $(MAKE) --no-print-directory -s -C "$$work/cold" -I "$$PWD" -f "$$PWD/Makefile" ck >"$$work/bootstrap-output"; test ! -s "$$work/bootstrap-output"; \
  (unset BROKER KEY_DIR; p verify <"$$work/old-packet") | cmp - "$$work/owner-role"; \
  p submit <"$$work/old-packet" >/dev/null; \
  OWNER_PUBLIC="$$work/replacement/public" p propose >"$$work/successor"; \
  draft transfer "$$work/successor" <"$$work/charter" >"$$work/transfer"; \
  sign_statement delegate transfer >"$$work/approval"; sign_statement replacement transfer >"$$work/acceptance"; \
  seal transfer approval "$$work/acceptance" >"$$work/proof"; \
  draft notice <"$$work/charter" >"$$work/notice"; sign_statement owner notice >"$$work/cancel-signature"; \
  seal notice cancel-signature >"$$work/cancellation"; \
  (BROKER="$$work/canceled"; p establish >/dev/null; p recovery-enroll <"$$work/enrollment" >/dev/null; \
    p recovery-cancel <"$$work/cancellation" | cmp - "$$work/cancellation"; refuse rekey.withdrawn p recovery-apply <"$$work/proof"); \
  (unset ROSTER KEY_DIR; p recovery-apply <"$$work/proof" >"$$work/recovered"; p recovery-apply <"$$work/proof" | cmp - "$$work/recovered"); \
  cmp "$$BROKER/private/peers" "$$work/successor"; \
  (ROSTER="$$work/successor"; refuse peer.wrong-control p verify <"$$work/old-packet"); \
  p verify <"$$work/old-packet" | cmp - "$$work/owner-role")
endef

define verify_participant_account
@set -eu; \
  work=$$(mktemp -d); \
  trap 'rm -rf "$$work"' EXIT; \
  p() { $(MAKE) --no-print-directory -s "circle-$$@"; }; \
  word() { printf %s "$$1" | $(CANONICAL_WORDS); }; \
  refuse() { reason=$$1; shift; rc=0; "$$@" >"$$work/failure" 2>"$$work/error" || rc=$$?; test "$$rc" != 0 && test "$$(cat "$$work/failure")" = "fault $$(word "$$reason")" || { echo "participant expected $$reason, received $$(cat "$$work/failure")" >&2; return 1; }; }; \
  inspect_roster() { (unset KEY_DIR BROKER; p inspect) >"$$work/inspected.words"; \
    ./ck text form/from-print.ck <"$$ROSTER" >"$$work/roster.records"; ./ck text lens/to-nodes.ck lens/all.ck <"$$work/inspected.words" | cmp - "$$work/roster.records"; \
    (ROSTER="$$work/roster.records"; p inspect) | cmp - "$$work/inspected.words"; }; \
  OWNER=Juniper; LISTENER=Lark; CIRCLE="Imagined rehearsal"; BROKER="$$work/broker"; ROSTER="$$work/roster"; export OWNER LISTENER CIRCLE BROKER ROSTER; \
  KEY_DIR="$$work/owner"; export KEY_DIR; p key; KEY_DIR="$$work/listener"; p key; \
  OWNER_PUBLIC="$$work/owner/public"; LISTENER_PUBLIC="$$work/listener/public"; export OWNER_PUBLIC LISTENER_PUBLIC; p propose >"$$ROSTER"; inspect_roster; ROLE=owner; KEY_DIR="$$work/owner"; export ROLE; p endorse >"$$work/owner.ok"; ROLE=listener; KEY_DIR="$$work/listener"; p endorse >"$$work/listener.ok"; \
  dd if="$$work/roster.records" of="$$work/partial-roster" bs=20 count=1 2>/dev/null; (ROSTER="$$work/partial-roster"; refuse '"short"' p inspect); \
  printf %s same-keys >"$$work/mode"; printf %s '#32' | ./ck text ground/peer-case.ck 3<"$$work/mode" >"$$work/invalid-roster"; (ROSTER="$$work/invalid-roster"; refuse peer.invalid-roster p inspect); \
  OWNER_ENDORSEMENT="$$work/owner.ok"; LISTENER_ENDORSEMENT="$$work/listener.ok"; export OWNER_ENDORSEMENT LISTENER_ENDORSEMENT; \
  rc=0; (OWNER_ENDORSEMENT="$$work/listener.ok"; p establish) >"$$work/refusal" 2>"$$work/failure-reason" || rc=$$?; test "$$rc" != 0; grep -F "fault $$(word ed25519.invalid-signature)" "$$work/failure-reason" >/dev/null; test ! -e "$$BROKER"; \
  mkdir -p "$$BROKER/.seed-enrollment"; printf interrupted >"$$BROKER/.seed-enrollment/authority"; \
  p establish >"$$work/agreement" & first=$$!; p establish >"$$work/other-result"; wait "$$first"; cmp "$$ROSTER" "$$work/agreement"; cmp "$$ROSTER" "$$work/other-result"; \
  test ! -e "$$BROKER/.seed-enrollment"; test -s "$$BROKER/private/first-roster"; \
  printf %s "$$(word peer.owner)" >"$$work/owner-role"; $(participant_recovery); \
  KEY_DIR="$$work/owner"; p join; p owner-head >"$$work/owner0"; p listener-head >"$$work/listener0"; \
  refuse journal.unrecorded p show; test -z "$$(find "$$work/owner/journal" -name "account-edition-*")"; test -z "$$(ls -A "$$BROKER/letters")"; \
  printf 'Private line about a fictional tree.\nI would like a quiet moment.\n' >"$$work/first"; \
  p record <"$$work/first" >"$$work/edition1"; p show | cmp - "$$work/first"; p edition | cmp - "$$work/edition1"; \
  KEY_DIR="$$work/listener"; test -z "$$(ls -A "$$work/listener/journal")"; p join; refuse journal.unrecorded p show; \
  p record <"$$work/first" >"$$work/listener-edition1"; test "$$(cat "$$work/listener-edition1")" != "$$(cat "$$work/edition1")"; p show | cmp - "$$work/first"; \
  KEY_DIR="$$work/owner"; EDITION="$$work/edition1"; UNTIL=$$(($$(date +%s) + 3600)); export EDITION UNTIL; \
  printf 'I would like a quiet moment.' >"$$work/copy"; printf 'I am thrilled.' >"$$work/never"; refuse journal.unselected p select <"$$work/never"; \
  p select <"$$work/copy" >"$$work/permit1"; p preview <"$$work/permit1" >"$$work/preview"; p ticket <"$$work/permit1" >"$$work/ticket1"; \
  printf 'Juniper to Lark (first-person account)\nPermission to read from Unix second 0 until %s (exclusive).\nI would like a quiet moment.' "$$UNTIL" | cmp - "$$work/preview"; \
  private_hex=$$(printf %s 'Private line about a fictional tree.' | od -An -tx1 -v | tr -d ' \n'); excerpt_hex=$$(od -An -tx1 -v "$$work/copy" | tr -d ' \n'); if grep -q "$$private_hex" "$$work/permit1" || grep -q "$$excerpt_hex" "$$work/ticket1"; then exit 1; fi; \
  HEAD="$$work/owner0"; ACTION=share; export HEAD ACTION; KEY_DIR="$$work/listener"; refuse ed25519.invalid-signature p sign <"$$work/permit1"; KEY_DIR="$$work/owner"; p sign <"$$work/permit1" >"$$work/grant1"; p submit <"$$work/grant1" >"$$work/owner1"; \
  refuse inbox.paused p read <"$$work/ticket1"; test -z "$$(ls -A "$$BROKER/letters")"; \
  KEY_DIR="$$work/listener"; HEAD="$$work/listener0"; ACTION=listen; p sign <"$$work/ticket1" >"$$work/listen1"; p submit <"$$work/listen1" >"$$work/listener1"; \
  p read <"$$work/ticket1" >"$$work/heard1"; printf 'Juniper (first-person account)\nI would like a quiet moment.' | cmp - "$$work/heard1"; \
  KEY_DIR="$$work/listener"; HEAD="$$work/listener1"; ACTION=pause; p sign </dev/null >"$$work/pause"; p submit <"$$work/pause" >"$$work/listener2"; refuse inbox.paused p read <"$$work/ticket1"; \
  KEY_DIR="$$work/owner"; HEAD="$$work/owner1"; ACTION=withdraw; p sign </dev/null >"$$work/withdraw"; p submit <"$$work/withdraw" >"$$work/owner2"; refuse share.ungranted p read <"$$work/ticket1"; \
  printf 'Private line about a fictional lake.\nI am ready to listen.\n' >"$$work/second"; p correct <"$$work/second" >"$$work/edition2"; p show | cmp - "$$work/second"; refuse journal.stale p correct <"$$work/first"; refuse journal.stale p select <"$$work/copy"; \
  EDITION="$$work/edition2"; refuse journal.unselected p select <"$$work/copy"; printf 'I am ready to listen.' >"$$work/current"; p select <"$$work/current" >"$$work/permit2"; p ticket <"$$work/permit2" >"$$work/ticket2"; HEAD="$$work/owner2"; ACTION=share; p sign <"$$work/permit2" >"$$work/grant2"; p submit <"$$work/grant2" >"$$work/owner3"; \
  refuse inbox.paused p read <"$$work/ticket2"; KEY_DIR="$$work/listener"; HEAD="$$work/listener2"; ACTION=listen; p sign <"$$work/ticket2" >"$$work/listen2"; p submit <"$$work/listen2" >"$$work/listener3"; p read <"$$work/ticket2" >"$$work/heard2"; \
  printf 'Juniper (first-person account)\nI am ready to listen.' | cmp - "$$work/heard2"; \
  set -- "$$BROKER/letters/"*; test "$$#" = 2; test "$$(find "$$work/owner/journal" -name "account-edition-*" | wc -l)" -eq 2; \
  ./ck text lens/show-canonical.ck lens/all.ck <"$$work/permit2" | sed 's/I am ready to listen[.]/Incorrect attribution./' | $(CANONICAL_WORDS) >"$$work/inconsistent"; p ticket <"$$work/inconsistent" >"$$work/inconsistent-ticket"; \
  KEY_DIR="$$work/owner"; HEAD="$$work/owner3"; ACTION=share; p sign <"$$work/inconsistent" >"$$work/inconsistent-grant"; p submit <"$$work/inconsistent-grant" >"$$work/owner4"; \
  KEY_DIR="$$work/listener"; HEAD="$$work/listener3"; ACTION=listen; p sign <"$$work/inconsistent-ticket" >"$$work/inconsistent-listen"; p submit <"$$work/inconsistent-listen" >"$$work/listener4"; refuse journal.invalid p read <"$$work/inconsistent-ticket"; \
  set -- "$$BROKER/letters/"*; test "$$#" = 2; \
  KEY_DIR="$$work/owner"; EDITION="$$work/edition2"; p retract >"$$work/retracted"; refuse journal.retracted p show; EDITION="$$work/retracted"; refuse journal.retracted p select <"$$work/current"; refuse journal.stale p correct <"$$work/second"; \
  printf 'A different private context.\nI would like a quiet moment.\n' >"$$work/third"; p resume <"$$work/third" >"$$work/edition3"; EDITION="$$work/edition3"; p select <"$$work/copy" >"$$work/permit3"; \
  p source <"$$work/permit1" | cmp - "$$work/first"; p source <"$$work/permit3" | cmp - "$$work/third"; KEY_DIR="$$work/listener"; refuse witness.wrong-owner p source <"$$work/permit3"; \
  KEY_DIR="$$work/owner"; headfile=$$(find "$$work/owner/journal" -name head); mv "$$headfile" "$$work/saved"; rc=0; p record <"$$work/first" >"$$work/failure" 2>"$$work/error" || rc=$$?; test "$$rc" != 0; test ! -e "$$headfile"; printf 'broken record' >"$$headfile"; rc=0; p record <"$$work/first" >"$$work/failure" 2>"$$work/error" || rc=$$?; test "$$rc" != 0; test "$$(cat "$$headfile")" = 'broken record'; mv "$$work/saved" "$$headfile"; p show | cmp - "$$work/third"; \
  word no >"$$headfile"; refuse journal.stale p record <"$$work/first"; p show | cmp - "$$work/third"; p edition | cmp - "$$work/edition3"; \
  cp "$$BROKER/private/authority" "$$work/advanced-owner"; cp "$$BROKER/private/listening" "$$work/advanced-listener"; p establish >"$$work/original"; cmp "$$ROSTER" "$$work/original"; \
  (ROSTER="$$work/another"; p propose >"$$ROSTER"; KEY_DIR="$$work/owner"; ROLE=owner; OWNER_ENDORSEMENT="$$work/a2"; p endorse >"$$OWNER_ENDORSEMENT"; KEY_DIR="$$work/listener"; ROLE=listener; LISTENER_ENDORSEMENT="$$work/b2"; p endorse >"$$LISTENER_ENDORSEMENT"; refuse pact.different p establish); \
  cmp "$$BROKER/private/authority" "$$work/advanced-owner"; cmp "$$BROKER/private/listening" "$$work/advanced-listener"; \
  original_roster=$$ROSTER; original_broker=$$BROKER; KEY_DIR="$$work/listener"; EDITION="$$work/edition3"; refuse journal.stale p correct <"$$work/second"; \
  EDITION="$$work/listener-edition1"; printf %s 'I have not decided whether to release my copy.' >"$$work/back-excerpt"; { printf '%s\n' 'Private words from the second participant.'; cat "$$work/back-excerpt"; } >"$$work/listener-words"; p correct <"$$work/listener-words" >"$$work/listener-edition2"; p show | cmp - "$$work/listener-words"; \
  EDITION="$$work/listener-edition2"; p retract >"$$work/listener-retracted"; refuse journal.retracted p show; EDITION="$$work/listener-retracted"; p resume <"$$work/listener-words" >"$$work/listener-edition3"; \
  EDITION="$$work/listener-edition3"; refuse witness.wrong-owner p select <"$$work/back-excerpt"; p select-return <"$$work/back-excerpt" >"$$work/back-permit"; \
  p propose-return >"$$work/back-roster"; ./ck text ground/intent-excerpt.ck 3<"$$work/back-roster" <"$$work/back-permit" >"$$work/input"; \
  ./ck text learning/intent-fit.ck <ground/intent-corpus.ck >"$$work/specialist"; ./ck text learning/intent-read.ck 3<"$$work/specialist" <"$$work/input" >"$$work/prediction"; \
  labels=$$(./ck text ground/intent-labels.ck <"$$work/prediction"); test "$$labels" = "$$(word '(list (list "K" "V"))')"; \
  p source <"$$work/back-permit" | cmp - "$$work/listener-words"; KEY_DIR="$$work/owner"; refuse witness.wrong-owner p source <"$$work/back-permit"; refuse witness.wrong-owner p select-return <"$$work/back-excerpt"; p show | cmp - "$$work/third"; \
  ACTION=share; HEAD="$$work/owner4"; refuse inbox.misdirected p sign <"$$work/back-permit"; ROSTER="$$work/back-roster"; inspect_roster; BROKER="$$work/back-broker"; \
  KEY_DIR="$$work/listener"; ROLE=owner; p endorse >"$$work/back-owner.ok"; KEY_DIR="$$work/owner"; ROLE=listener; p endorse >"$$work/back-listener.ok"; OWNER_ENDORSEMENT="$$work/back-owner.ok"; LISTENER_ENDORSEMENT="$$work/back-listener.ok"; p establish >"$$work/back-agreement"; \
  p owner-head >"$$work/back-owner0"; p listener-head >"$$work/back-listener0"; p ticket <"$$work/back-permit" >"$$work/back-ticket"; refuse share.ungranted p read <"$$work/back-ticket"; \
  KEY_DIR="$$work/owner"; ACTION=listen; HEAD="$$work/back-listener0"; p sign <"$$work/back-ticket" >"$$work/back-listen"; p submit <"$$work/back-listen" >"$$work/back-listener1"; \
  KEY_DIR="$$work/listener"; ACTION=share; HEAD="$$work/back-owner0"; refuse inbox.invalid p sign <"$$work/prediction"; \
  refuse inbox.invalid p submit <"$$work/prediction"; p owner-head | cmp - "$$work/back-owner0"; p listener-head | cmp - "$$work/back-listener1"; \
  refuse share.ungranted p read <"$$work/back-ticket"; test -z "$$(ls -A "$$BROKER/letters")"; \
  p sign <"$$work/back-permit" >"$$work/back-grant"; p submit <"$$work/back-grant" >"$$work/back-owner1"; p read <"$$work/back-ticket" >"$$work/return-heard"; \
  { printf '%s\n' 'Lark (first-person account)'; cat "$$work/back-excerpt"; } | cmp - "$$work/return-heard"; set -- "$$BROKER/letters/"*; test "$$#" = 1; \
  KEY_DIR="$$work/listener"; ACTION=withdraw; HEAD="$$work/back-owner1"; p sign </dev/null >"$$work/back-withdraw"; p submit <"$$work/back-withdraw" >"$$work/back-owner2"; refuse share.ungranted p read <"$$work/back-ticket"; \
  ROSTER=$$original_roster; BROKER=$$original_broker; KEY_DIR="$$work/listener"; p show | cmp - "$$work/listener-words"; set -- "$$BROKER/letters/"*; test "$$#" = 2; \
  echo "participant: private records survived correction; two private authors shared two forward excerpts and one separately authorized return excerpt across pause and withdrawal"
endef
