# The bootstrap's build. Its lines are counted at each build (graph node 5.9.2): the count is to fall
# as parts leave C for Form, and no fixed number holds work back.
# Nothing here needs more than a C compiler: no part of the old kernel builds or runs this repository.
# How many times the stored nodes may still read a part of a node by a bare number (graph node
# 7.22). The count is of the nodes, whatever names lie beside them. It is lowered, never raised.
BARE = 474
CC ?= cc
CFLAGS = -std=c99 -O2 -Wall -Wextra -Werror
ifeq ($(shell uname -s),Linux)
LDLIBS ?= -pthread -ldl -lrt
endif
ifeq ($(shell uname -s),Darwin)
METAL_LIBS = -framework Metal -framework Foundation -lobjc
endif
BOOT = bootstrap/ck.c bootstrap/fb.h bootstrap/metal.h
LONG = ground/long-recursion.ck

all: ck fb kg seed-run

ck: $(BOOT)
	@echo "bootstrap: $$(cat $(BOOT) | wc -l | tr -d ' ') lines of C"
	$(CC) $(CFLAGS) -o ck bootstrap/ck.c $(LDLIBS) $(METAL_LIBS)

seed-run: ck $(BOOT)
	$(CC) $(CFLAGS) -DNUMERIC_PORT -o $@ bootstrap/ck.c $(LDLIBS) $(METAL_LIBS)

# The observer: a second process that shows, records and replays a framebuffer.
fb: tools/fb.c bootstrap/fb.h
	$(CC) $(CFLAGS) -o fb tools/fb.c $(LDLIBS)

# The claim graph's checker.
kg: tools/kg.c
	$(CC) $(CFLAGS) -o kg tools/kg.c

# bootstrap/fb.h is made from nodes: the catalogue's words and the layout of the region's records.
header: ck
	./ck text region/header.ck > .ck-header && mv .ck-header bootstrap/fb.h

# Wait until the region named $name stands, for at most ten seconds.
up = n=0; until ./fb show $$name > /dev/null 2>&1; do n=$$((n+1)); [ $$n -gt 400 ] && break; sleep 0.025; done

# One real run, watched live: the long recursion runs while a second process records its
# memory every 5 ms; then the recording is replayed and the answer is asked who wrote it.
watch: ck fb
	@name=/ck.watch.$$$$; (CK_FB=$$name ./ck run $(LONG) &); $(up); ./fb record $$name .ck-watch.rec 5; \
	  ./fb play .ck-watch.rec | head -3; ./fb play .ck-watch.rec | tail -1; ./fb play .ck-watch.rec -1 | grep '^outcome'; \
	  ./fb drop $$name; rm -f .ck-watch.rec

# Three end-to-end flows at most. The ground catalogue is exercised by check-full.
CHECK_FLOWS := check-bml check-image check-live
FULL_ONLY := check-header check-ice check-native check-replay check-held-back check-bare
TEST_LIMIT := 86
ifneq ($(word 4,$(CHECK_FLOWS)),)
$(error make check has room for three flows)
endif
check: test-inventory $(CHECK_FLOWS)

# One slot per ground expectation or named Makefile check, including host-specific checks.
test-inventory: ck
	@set -e; paths=$$(./ck text ground/paths.ck); ground=$$(printf '%s' "$$paths" | awk 'END {print NR}'); \
	  flows=$(words $(sort $(CHECK_FLOWS) $(FULL_ONLY))); total=$$((ground + flows)); \
	  echo "maintained tests: $$total/$(TEST_LIMIT) ($$ground ground, $$flows Makefile)"; \
	  [ "$$total" -le "$(TEST_LIMIT)" ] || { echo "replace or consolidate an existing test"; exit 1; }

check-bml: ck
	@got=$$(printf 'nums/bml.ck\n10 - 3 - 2\n' | ./ck text nums/read.ck | ./ck run /dev/stdin nums/run.ck) && \
	  [ "$$got" = 'node [#4 09]' ] || { echo "BML expression returned '$$got', expected 9"; exit 1; }; \
	  echo "BML: the source expression 10 - 3 - 2 ran as numeric nodes and returned 9"

# Build-time lens and Apple toolchain. The resulting image runs on its own.
export METAL_RECIPE METAL_INPUTS METAL_APPLICATION METAL_IMAGE
metal-image: ck
	@set -eu; : "$${METAL_RECIPE:?set METAL_RECIPE to a NUMS declaration}" "$${METAL_INPUTS:?set METAL_INPUTS to a list of byte leaves}" "$${METAL_IMAGE:?set METAL_IMAGE to the output path}"; \
	  work=$$(mktemp -d); out=; trap 'rm -rf "$$work"; if [ -n "$$out" ]; then rm -f "$$out"; fi' EXIT; \
	  ./ck text nums/metal-source.ck < "$$METAL_RECIPE" > "$$work/kernel.metal" || { cat "$$work/kernel.metal" >&2; exit 1; }; \
	  xcrun -sdk macosx metal -std=macos-metal2.4 -c "$$work/kernel.metal" -o "$$work/kernel.air"; \
	  xcrun -sdk macosx metallib "$$work/kernel.air" -o "$$work/kernel.metallib"; \
	  out=$$(mktemp "$$METAL_IMAGE.XXXXXX"); \
	  set -- "$$METAL_RECIPE" "$$METAL_INPUTS" "$$work/kernel.metallib"; \
	  if [ -n "$$METAL_APPLICATION" ]; then set -- "$$@" "$$METAL_APPLICATION"; fi; \
	  printf '%s\n' "$$@" | ./ck text nums/metal-pack.ck > "$$out" || { cat "$$out" >&2; exit 1; }; \
	  mv -f "$$out" "$$METAL_IMAGE"

check-image: seed-run
	@set -eu; work=$$(mktemp -d); trap 'rm -rf "$$work"' EXIT; \
	  [ "$$(./ck run ground/language-reference.ck)" = 'node #33' ]; \
	  ./ck text learning/train.ck <ground/language-corpus.ck >"$$work/model"; \
	  ./ck text learning/train.ck <ground/language-corpus.ck | cmp - "$$work/model"; \
	  [ "$$(./ck run ground/language-replies.ck <"$$work/model")" = 'node #33' ]; \
	  echo "language: fresh training reproduced the weights and a new process generated each held-out completion"
	@set -e; root=$$PWD; work=$$(mktemp -d); trap 'rm -rf "$$work"' EXIT; mkdir "$$work/run"; \
	  ./ck text nums/pack.ck < ground/nums-image.ck > "$$work/run/image"; \
	  ./ck text ground/nums-requests.ck > "$$work/requests"; test -s "$$work/requests"; \
	  ./ck text ground/nums-native-requests.ck >> "$$work/requests"; \
	  (cd "$$work/run" && "$$root/seed-run" image) < "$$work/requests" > "$$work/replies"; \
	  got=$$(./ck run ground/nums-replies.ck < "$$work/replies"); \
	  [ "$$got" = 'node #33' ] || { echo "image replies differed: $$got"; exit 1; }; \
	  [ "$$(./ck run ground/resident-owners.ck)" = 'node #33' ]; \
	  for part in policy first second; do printf %s "$$part" | ./ck text ground/resident-case.ck > "$$work/$$part"; done; \
	  mkfifo "$$work/turns" "$$work/returns"; \
	  (cd "$$work/run" && exec "$$root/seed-run" image) <"$$work/turns" >"$$work/returns" & worker=$$!; \
	  exec 5>"$$work/turns" 6<"$$work/returns"; \
	  for part in first second; do ./ck text resident/call.ck 3<"$$work/policy" 4<"$$work/$$part" <&6 >"$$work/$$part.out"; done; \
	  exec 5>&- 6<&-; wait "$$worker"; \
	  [ "$$(printf verify | ./ck text ground/resident-case.ck 3<"$$work/first.out" 4<"$$work/second.out")" = '#33' ]; \
	  echo "image: the standalone runtime answered by content id from a directory containing only the image"; \
	  if [ "$$(uname -s)" = Darwin ] && xcrun -sdk macosx metal -v >/dev/null 2>&1; then \
	    printf '(has-target metal-map64)' | ./ck text lens/to-nodes.ck lens/all.ck > "$$work/target"; \
	    gpu=$$(./ck run "$$work/target"); case "$$gpu" in 'node #32') exit 0;; 'node #33') ;; *) echo "$$gpu"; exit 1;; esac; \
	    $(MAKE) --no-print-directory metal-image METAL_RECIPE=ground/metal-polynomial.ck METAL_INPUTS=ground/metal-vectors.ck METAL_APPLICATION=ground/metal-application.ck METAL_IMAGE="$$work/run/image"; \
	    ./ck text ground/metal-requests.ck > "$$work/requests"; \
	    (cd "$$work/run" && "$$root/seed-run" image) < "$$work/requests" > "$$work/replies"; \
	    got=$$(./ck run ground/metal-replies.ck < "$$work/replies"); test "$$got" = 'node #33' || { echo "Metal replies differed: $$got"; exit 1; }; \
	    echo "Metal: direct and backtracking calls by ID agree with NUMS frames"; \
	  fi

check-live: ck fb
	@name=/ck.ask.$$$$; budget=33554432; (CK_MEMORY_BYTES=$$budget CK_FB=$$name ./ck run $(LONG) > /dev/null &) ; $(up); \
	  ask() { printf '%s\n%s' "$$name" "$$1" | ./ck text region/ask.ck | tr -d '()\n'; }; \
	  on=$$(ask '(seq (need ((recover (seq (take u8 '"$$((budget - 4096))"') no) {reason | reason}) = (list "full" "rooms"))) (bind add (lazy (seq (note #1002 (add 1 (if (held #1002) (held #1002) 0))) (base $$0)))))'); \
	  one=$$(ask '(held #1002)'); two=$$(ask '(held #1002)'); off=$$(ask '(unbind add)'); three=$$(ask '(held #1002)'); four=$$(ask '(held #1002)'); \
	  n=0; until ./fb show $$name | grep -q 'mode done'; do n=$$((n+1)); [ $$n -gt 1200 ] && break; sleep 0.05; done; \
	  last=$$(./fb show $$name | grep '^outcome'); ./fb drop $$name; n1=$${one##* }; n2=$${two##* }; \
	  case "$$last" in "outcome  node 2178309 "*) ok=1;; *) ok=0;; esac; \
	  if [ $$ok = 1 ] && [ "$$on" = 'list "node" yes' ] && [ "$$off" = "$$on" ] && [ "$${n1:-0}" -gt 0 ] && [ "$${n2:-0}" -gt "$$n1" ] && [ "$$three" = "$$four" ]; \
	  then echo "live: a room beyond the shared allowance recovered; a second process installed an addition counter, read $$n1 and $$n2, removed it, and observed the final answer 2178309"; \
	  else echo "live: on '$$on', counts '$$one' '$$two', off '$$off', after '$$three' '$$four', final '$$last'"; exit 1; fi

check-full: check
	@$(MAKE) -k ground-isolated ground-together $(FULL_ONLY)

check-header: ck
	@./ck text region/header.ck | cmp -s - bootstrap/fb.h && echo "header: bootstrap/fb.h is what the nodes generate" || \
	  { echo "header: bootstrap/fb.h is not what region/header.ck writes; run make header"; exit 1; }

ground-isolated: ck
	@set -e; paths=$$(./ck text ground/paths.ck ground/for-host.ck); \
	  differences=$$(printf '%s\n' "$$paths" | tr '\n' '\0' | xargs -0 -n 1 -P 8 sh -c 'got=$$(printf %s "$$0" | ./ck run ground/check-one.ck 2>&1); [ "$$got" = "node #33" ] || echo "  $$0: $$got"'); \
	  if [ -n "$$differences" ]; then printf 'ground differences:\n%s\n' "$$differences"; exit 1; fi; \
	  echo "ground: $$(printf '%s\n' "$$paths" | wc -l | tr -d ' ') applicable programs passed in their own processes"

ground-together: ck
	@got=$$(./ck run ground/check.ck 2>&1); \
	  if [ "$$got" = "node (#16)" ]; then echo "one run: all of them in one process end as expected, since a fail gives back what it returns past"; \
	  else echo "one run: path and outcome of each program that ends otherwise in one process: $$got"; exit 1; fi

check-ice: ck
	@./ck text ice/freeze.ck lens/all.ck ice/ice.ck < ground/recursion.ck > .ck-check.ice && \
	  b=$$(./ck run ice/thaw.ck lens/all.ck ice/ice.ck < .ck-check.ice); rm -f .ck-check.ice; \
	  if [ "$$b" = "node $$(head -1 ground/recursion.ck)" ]; then echo "ice: frozen by a program in form, thawed by another in a second process to the same node"; \
	  else echo "ice: thawed '$$b'"; exit 1; fi

check-native: ck
	@set -e; if [ "$$(uname -s)" = Darwin ] && ./ck text ground/paths.ck ground/for-host.ck | grep -Fxq ground/native-add.ck; then \
	  binary=$$(mktemp); trap 'rm -f "$$binary"' EXIT; cp ck "$$binary"; chmod +x "$$binary"; \
	  codesign -f -s - --options runtime --entitlements bootstrap/jit.entitlements "$$binary" 2>/dev/null; \
	  test "$$("$$binary" run ground/native-add.ck)" = 'node [#4 2a]'; \
	  echo "native: generated code and frames agree under the hardened runtime"; \
	  if ./ck text ground/paths.ck ground/for-host.ck | grep -Fxq ground/host-refused.ck; then \
	    for run in ./ck "$$binary"; do got=$$("$$run" run ground/host-refused.ck 2>&1); \
	      [ "$$got" = 'node (#16)' ] || { echo "host: cases that differ under $$run: $$got"; exit 1; }; done; \
	    echo "host: a file written and read back, a host callback, float and varying values answer the same by name, by address and through the berth, also under the hardened runtime"; fi; \
	else echo "native: hardened signing applies to native Darwin targets"; fi

# The first recorded page starts at byte 44. Its layout version follows the 16-byte identity prefix.
check-replay: ck fb
	@name=/ck.check.$$$$; (CK_FB=$$name ./ck run $(LONG) > /dev/null &) ; $(up); ./fb record $$name .ck-check.rec 5 > /dev/null; \
	  frames=$$(./fb play .ck-check.rec | grep -c moving); last=$$(./fb play .ck-check.rec -1 | grep '^outcome'); \
	  printf '\000\000\000\000' | dd of=.ck-check.rec bs=1 seek=60 conv=notrunc 2>/dev/null || exit 1; \
	  refused=$$(./fb play .ck-check.rec 2>&1); status=$$?; ./fb drop $$name; rm -f .ck-check.rec; \
	  [ $$status = 2 ] && [ "$$refused" = 'fb: incompatible framebuffer layout' ] || { echo "framebuffer replay: $$refused"; exit 1; }; \
	  case "$$last" in "outcome  node 2178309 "*"written by (add (call (self)"*) ok=1;; *) ok=0;; esac; \
	  if [ $$ok = 1 ] && [ $$frames -ge 2 ]; then echo "framebuffer: a second process recorded $$frames frames of a run in motion; the last holds the answer and its writer"; \
	  else echo "framebuffer: $$frames frames in motion; last frame says '$$last'"; exit 1; fi

check-held-back: ck
	@test "$$(./ck run ground/learning-trial.ck)" = 'node #33'
	@test "$$(./ck run ground/witness-review.ck)" = 'node #33'
	@got=$$(./ck run ground/held-back.ck | tr '\n' ' '); \
	  if [ "$$got" = "held node (#16 [#4 04] [#2 626f6f6d] [#2 697272657665727369626c65] [#1 340a] (#16 [#4 64] [#4]) (#16 [#2 67] [#4 08] (#16 [#2 66] [#4 01] [#4 02] [#4 03] ^7))) " ]; \
	  then echo "held back: branch, fault and ordering checks passed. Scoped delivery matched the current grant, selected reading, receipt and bytes in the receiving stream."; \
	  else echo "held back: '$$got'"; exit 1; fi

	@test "$$(./ck run ground/circle-turns.ck)" = 'node #33'
	@set -eu; \
	  work=$$(mktemp -d); trap 'rm -rf "$$work"' EXIT; mkdir "$$work/private" "$$work/letters"; \
	  gen() { printf %s "$$1" > "$$work/mode"; ./ck text ground/circle-exchange.ck 3<"$$work/mode" 4<"$$work/$${4:-none}" <"$$work/$$2" >"$$work/$$3"; }; \
	  send() { ./ck text "circle/$$1.ck" 3<"$$work/channel" <"$$work/$$2" >"$$work/$$3"; }; \
	  expect() { gen status "$$1" status; actual=$$(cat "$$work/status"); wanted=$$(printf %s "$$2" | ./ck text lens/to-nodes.ck lens/all.ck); [ "$$actual" = "$$wanted" ] || { echo "circle $$1: $$actual, expected $$2" >&2; exit 1; }; }; \
	  empty() { [ -z "$$(ls -A "$$work/letters")" ] || { echo "circle: premature publication" >&2; exit 1; }; }; \
	  change() { gen change "$$2" update "$$3"; send "$$1" update "$$4"; }; \
	  printf '#32' > "$$work/none"; cp "$$work/none" "$$work/private/authority"; cp "$$work/none" "$$work/private/listening"; \
	  printf %s "$$work" > "$$work/path"; printf '[#2 31]' > "$$work/first"; printf '[#2 32]' > "$$work/second"; \
	  gen channel path channel; gen identity none genesis; \
	  gen open none opening; gen propose opening proposal; gen correct proposal correction; \
	  gen decline correction refusal; gen silence refusal quiet; gen scope quiet scope; gen other-scope proposal alternative; \
	  gen permit scope offered first; gen ticket scope ticket first; gen ticket alternative wrong first; \
	  witness_case() { printf %s "$$1" > "$$work/witness-mode"; ./ck text ground/witness-case.ck 3<"$$work/witness-mode" 4<"$$work/$${4:-none}" <"$$work/$$2" >"$$work/$$3"; }; \
	  witness_word() { printf %s "$$1" | ./ck text lens/to-nodes.ck lens/all.ck; }; \
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
	  [ "$$(cat "$$work/stale")" = "$$(printf inbox.stale | ./ck text lens/to-nodes.ck lens/all.ck)" ]; \
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
	  reason=$$(printf '(list inbox.io "publish" "%s")' "$$1" | ./ck text lens/to-nodes.ck lens/all.ck); \
	  [ "$$(cat "$$work/failed")" = "fault $$reason" ]; [ ! -e "$$work/private/pending" ]; mv "$$work/asleep" "$$work/letters"; \
	  gen permit alternative other-permit first; change authorize other-permit sender sender; change listen wrong listener listener; \
	  send receive wrong reply; expect reply inbox.conflict; gen bytes original intact; cmp "$$work/intact" "$$work/seen"; test "$$1" -ef "$$work/original"; \
	  change authorize none sender stopped; send receive wrong reply; expect reply share.ungranted; \
	  gen permit scope another second; gen ticket scope ticket-two second; change authorize another stopped sender; change listen ticket-two listener listener; \
	  send receive ticket-two race-a & first_pid=$$!; send receive ticket-two race-b & second_pid=$$!; wait "$$first_pid"; wait "$$second_pid"; \
	  gen status race-a a; gen status race-b b; accepted=$$(printf inbox.accepted | ./ck text lens/to-nodes.ck lens/all.ck); \
	  if [ "$$(cat "$$work/a")" = "$$accepted" ]; then expect race-b inbox.replayed; else expect race-a inbox.replayed; expect race-b inbox.accepted; fi; \
	  set -- "$$work/letters/"*; [ "$$#" = 2 ]; for entry in "$$@"; do gen bytes "letters/$${entry##*/}" held; cmp "$$work/held" "$$work/seen"; done; \
	  ./ck text ground/turn-projection.ck > "$$work/turn_projection.py"; \
	  if command -v python3 >/dev/null 2>&1; then python3 -m doctest "$$work/turn_projection.py"; fi; \
	  echo "circle: two roles retained their readings; consent, expiry and replay matched the letters in the receiving directory"

	@set -eu; \
	  work=$$(mktemp -d); trap 'rm -rf "$$work"' EXIT; mkdir "$$work/private" "$$work/letters"; printf '#32' > "$$work/none"; \
	  sample() { printf %s "$$1" > "$$work/mode"; ./ck text ground/peer-case.ck 3<"$$work/mode" <"$$work/$${2:-none}"; }; \
	  word() { printf %s "$$1" | ./ck text lens/to-nodes.ck lens/all.ck; }; \
	  apply() { ./ck text circle/apply.ck 3<"$$work/channel" 4<"$$work/$$1" <"$$work/$$2"; }; \
	  verify() { ./ck text circle/check-proof.ck 3<"$$work/$$1" 4<"$$work/$$2" <"$$work/$$3"; }; \
	  refused() { expected=$$1; shift; rc=0; "$$@" >"$$work/reason" || rc=$$?; actual=$$(cat "$$work/reason"); [ "$$rc" = 1 ] && [ "$$actual" = "fault $$(word "$$expected")" ] || { echo "signed control: $$actual, expected $$expected" >&2; return 1; }; }; \
	  for item in roster owner-change listener-change withdraw-change owner-proof listener-proof withdraw-proof listener-ticket wrong-episode same-keys owner-other-proof listener-other-proof; do sample "$$item" > "$$work/$$item"; done; \
	  printf %s channel > "$$work/mode"; printf %s "$$work" | ./ck text ground/circle-exchange.ck 3<"$$work/mode" >"$$work/channel"; \
	  cp "$$work/roster" "$$work/private/peers"; ./ck text circle/pin.ck < "$$work/roster" > "$$work/pin"; \
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
	  expect_validity() { sample control "finite-$$1-proof" > "$$work/control"; control=$$(./ck text lens/to-words.ck lens/all.ck < "$$work/control"); terms=$$(word "($$2 $$control $$3)"); actual=$$(validity "$$1" "$$1"); test "$$actual" = "$$terms"; }; \
	  ticket=$$(./ck text lens/to-words.ck lens/all.ck < "$$work/finite-ticket"); expect_validity grant peer.window "$$ticket 100 110"; expect_validity withdraw peer.withdrawal ""; \
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

	@set -eu; \
	  work=$$(mktemp -d); \
	  trap 'rm -rf "$$work"' EXIT; \
	  mkdir "$$work/private" "$$work/letters" "$$work/elsewhere"; \
	  printf '#32' > "$$work/none"; for register in authority listening adapter-authority; do cp "$$work/none" "$$work/private/$$register"; done; \
	  printf %s "$$work" > "$$work/location"; printf '[#2 31]' > "$$work/nonce"; \
	  case_value() { printf %s "$$2" > "$$work/mode"; ./ck text "ground/$$1.ck" 3<"$$work/mode" 4<"$$work/$${5:-none}" <"$$work/$$3" >"$$work/$$4"; }; \
	  g() { case_value circle-exchange "$$@"; }; \
	  t() { case_value teaching-case "$$@"; }; \
	  change() { g change "$$2" update "$$3"; ./ck text "$$1.ck" 3<"$$work/channel" <"$$work/update" >"$$work/$$4"; }; \
	  begin() { ./ck text learning/teach.ck 3<"$$work/$${2:-channel}" <"$$work/$$1"; }; \
	  offer() { ./ck text learning/artifact-scope.ck 3<"$$work/plan" 4<"$$work/parent" 5<"$$work/$$2" 6<"$$work/control" <"$$work/$$1"; }; \
	  activate() { ./ck text learning/activate.ck 3<"$$work/channel" 4<"$$work/plan" 5<"$$work/parent" 6<"$$work/$$2" 7<"$$work/control" <"$$work/$$1"; }; \
	  use() { ./ck text learning/use.ck 3<"$$work/$${2:-channel}" <"$$work/$$1"; }; \
	  denied() { reason=$$1; shift; rc=0; "$$@" >"$$work/denied" || rc=$$?; expected=$$(printf %s "$$reason" | ./ck text lens/to-nodes.ck lens/all.ck); [ "$$rc" = 1 ] && [ "$$(cat "$$work/denied")" = "fault $$expected" ] || { echo "learning: expected $$reason, received $$(cat "$$work/denied")" >&2; return 1; }; }; \
	  g channel location channel; g identity none genesis; t wrong-location channel elsewhere-channel; \
	  for item in plan parent fit hold-fit other-fit control weights dataset; do t "$$item" none "$$item"; done; \
	  t source channel source plan; t recall source recall-only; \
	  g permit recall-only recall-permit nonce; g ticket recall-only recall-ticket nonce; \
	  g permit source source-permit nonce; g ticket source source-ticket nonce; \
	  change circle/authorize recall-permit genesis issued; change circle/listen recall-ticket genesis listened; \
	  denied teach.wrong-purpose begin recall-ticket; test -z "$$(ls -A "$$work/letters")"; \
	  change circle/authorize source-permit issued issued; change circle/listen source-ticket listened listened; \
	  denied teach.wrong-location begin source-ticket elsewhere-channel; \
	  t capture channel reconstructed source-ticket; offer reconstructed fit > "$$work/unadmitted-scope"; g permit unadmitted-scope unadmitted-permit nonce; change learning/allow unadmitted-permit genesis unadmitted-revision; \
	  denied teach.not-admitted activate reconstructed fit; test ! -e "$$work/private/active-adapter"; test -z "$$(ls -A "$$work/letters")"; \
	  begin source-ticket > "$$work/round"; set -- "$$work/letters/"*; test "$$#" = 1; cat "$$1" > "$$work/letter"; g bytes letter observed; cmp "$$work/dataset" "$$work/observed"; \
	  offer round fit > "$$work/scope"; t bad-sha scope bad-scope; g permit bad-scope bad-permit nonce; g change bad-permit update unadmitted-revision; denied teach.mismatch ./ck text learning/allow.ck 3<"$$work/channel" <"$$work/update"; g permit scope artifact-permit nonce; change learning/allow artifact-permit unadmitted-revision approved; \
	  t path round artifact-path plan; model_path=$$(cat "$$work/artifact-path"); cp "$$work/weights" "$$model_path"; \
	  denied teach.not-eligible activate round hold-fit; test ! -e "$$work/private/active-adapter"; \
	  denied share.different activate round other-fit; test ! -e "$$work/private/active-adapter"; \
	  change circle/authorize none issued stopped; denied share.ungranted activate round fit; test ! -e "$$work/private/active-adapter"; \
	  change circle/authorize source-permit stopped issued; denied teach.stale activate round fit; test ! -e "$$work/private/active-adapter"; \
	  begin source-ticket > "$$work/listener-round"; offer listener-round fit > "$$work/scope"; g permit scope artifact-permit nonce; change learning/allow artifact-permit approved approved; \
	  change circle/listen none listened silence; denied inbox.paused activate listener-round fit; test ! -e "$$work/private/active-adapter"; \
	  change circle/listen source-ticket silence listened; denied teach.stale activate listener-round fit; test ! -e "$$work/private/active-adapter"; \
	  begin source-ticket > "$$work/new-round"; offer new-round fit > "$$work/scope"; g permit scope artifact-permit nonce; change learning/allow artifact-permit approved approved; \
	  rm "$$model_path"; denied teach.inactive activate new-round fit; test ! -e "$$work/private/active-adapter"; \
	  provider=$$(./ck run ground/ed25519.ck); unavailable=$$(printf ed25519.unavailable | ./ck text lens/to-nodes.ck lens/all.ck); \
	  if [ "$$provider" = "node $$unavailable" ]; then cp "$$work/weights" "$$model_path"; denied teach.unavailable activate new-round fit; test ! -e "$$work/private/active-adapter"; echo "learning: missing verification provider kept the adapter inactive"; exit 0; fi; \
	  printf 'changed' > "$$model_path"; denied teach.mismatch activate new-round fit; test ! -e "$$work/private/active-adapter"; \
	  cp "$$work/weights" "$$model_path"; activate new-round fit > "$$work/selection"; use selection > "$$work/verified"; cmp "$$work/weights" "$$work/verified"; \
	  denied teach.wrong-location use selection elsewhere-channel; \
	  printf 'changed' > "$$model_path"; denied teach.mismatch use selection; cp "$$work/weights" "$$model_path"; \
	  change learning/allow none approved held; denied share.ungranted use selection; \
	  change learning/allow artifact-permit held approved; denied teach.stale use selection; \
	  activate new-round fit > "$$work/selection"; change circle/authorize none issued stopped; denied share.ungranted use selection; \
	  echo "learning: recall-only, held evidence, withdrawal, later grants and changed model bytes denied; selected fixture bytes reached only the explicit local consumer"

# The resident model image and its matching native runtime are deployment inputs.
.PHONY: resident-replay
resident-replay: ck
	@set -eu; : "$${RESIDENT_NATIVE:?Set RESIDENT_NATIVE to the built 55b5b95 checkout}" "$${RESIDENT_MANIFEST:?Set RESIDENT_MANIFEST to its single-bos manifest}" "$${RESIDENT_GGUF:?Set RESIDENT_GGUF to the mounted 3B GGUF}"; \
	  root=$$PWD; native=$$(cd "$$RESIDENT_NATIVE" && pwd); work=$$(mktemp -d); trap 'rm -rf "$$work"' EXIT; \
	  test "$$(shasum -a 256 "$$RESIDENT_GGUF" | cut -d' ' -f1)" = dba8d50e3c63bde16cd918bb4de8e7cde6c5a2dae9a8360bd14480e71d8d7fa4; \
	  printf %s "$$RESIDENT_GGUF" > "$$work/path"; cp "$$RESIDENT_MANIFEST" "$$work/source"; \
	  ./ck text lens/to-words.ck lens/all.ck < ground/resident-inputs.ck > "$$work/inputs.words"; \
	  (cd "$$native"; ./ck text lens/to-nodes.ck lens/all.ck < "$$work/inputs.words" > "$$work/inputs"; \
	    printf '%s' '(seq (step (read (file "lens/all.ck"))) (lens.to-store (step (read (file "/dev/fd/3")))))' | ./ck text lens/to-nodes.ck lens/all.ck > "$$work/build"; \
	    ./ck text "$$work/build" 3<"$$work/inputs" 4<"$$work/path" < "$$work/source" > "$$work/manifest"; \
	    ./ck text nums/pack.ck < "$$work/manifest" > "$$work/image"; \
	    printf '%s' '(let keys (child (read (file "/dev/stdin")) 1) (pack bytes (all (let digest (id (child keys (range 0 (count keys)))) (byte digest (range 0 32))))))' | ./ck text lens/to-nodes.ck lens/all.ck > "$$work/exports"; \
	    ./ck text "$$work/exports" < "$$work/manifest" > "$$work/ids"); \
	  mkfifo "$$work/turns" "$$work/returns"; \
	  CK_IMAGES= CK_MEMORY_BYTES=6442450944 "$$native/seed-run" "$$work/image" <"$$work/turns" >"$$work/returns" & worker=$$!; \
	  "$$root/ck" text ground/resident-neural.ck 5>"$$work/turns" <"$$work/returns" 3<"$$work/ids" >"$$work/observed"; wait "$$worker"; \
	  ./ck text lens/to-words.ck lens/all.ck < "$$work/observed"

check-bare: ck
	@n=$$(./ck text ground/reads-by-number.ck | head -1); \
	  if [ "$${n:-99999}" -le $(BARE) ]; then echo "parts by name: $$n reads by a bare number are left in the stored form, within the budget of $(BARE)"; \
	  else echo "parts by name: $$n reads by a bare number, over the budget of $(BARE); name the parts"; exit 1; fi

clean:
	rm -f ck fb kg seed-run .ck-header .ck-check.ice .ck-check.rec .ck-watch.rec .ck-hardened

.PHONY: all metal-image check test-inventory $(CHECK_FLOWS) $(FULL_ONLY) check-full ground-isolated ground-together watch clean header
