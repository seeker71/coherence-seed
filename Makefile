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
	@set -e; root=$$PWD; work=$$(mktemp -d); trap 'rm -rf "$$work"' EXIT; mkdir "$$work/run"; \
	  ./ck text nums/pack.ck < ground/nums-image.ck > "$$work/run/image"; \
	  ./ck text ground/nums-requests.ck > "$$work/requests"; test -s "$$work/requests"; \
	  ./ck text ground/nums-native-requests.ck >> "$$work/requests"; \
	  (cd "$$work/run" && "$$root/seed-run" image) < "$$work/requests" > "$$work/replies"; \
	  got=$$(./ck run ground/nums-replies.ck < "$$work/replies"); \
	  [ "$$got" = 'node #33' ] || { echo "image replies differed: $$got"; exit 1; }; \
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
	  change authorize offered genesis sender; send receive ticket reply; expect reply inbox.paused; empty; \
	  change listen ticket genesis listener; gen expire-during-stage ticket reply channel; expect reply share.expired; empty; \
	  send receive wrong reply; expect reply share.different; empty; \
	  change listen none listener paused; change listen ticket genesis stale; \
	  [ "$$(cat "$$work/stale")" = "$$(printf inbox.stale | ./ck text lens/to-nodes.ck lens/all.ck)" ]; \
	  send receive ticket reply; expect reply inbox.paused; empty; \
	  change listen ticket paused listener; \
	  interrupt() { printf %s "$$1" > "$$work/phase"; rc=0; ./ck text circle/receive.ck ground/circle-interrupt.ck 3<"$$work/channel" 4<"$$work/phase" <"$$work/ticket" >"$$work/discarded" || rc=$$?; [ "$$rc" = "$$2" ] && [ ! -s "$$work/discarded" ]; }; \
	  interrupt before 75; empty; interrupt after 76; \
	  set -- "$$work/letters/"*; [ "$$#" = 1 ]; ln "$$1" "$$work/original"; gen bytes original seen; printf 'Kami boleh berbagi' | cmp - "$$work/seen"; \
	  send receive ticket reply; expect reply inbox.replayed; gen reference reply confirmed; cmp "$$work/confirmed" "$$work/ticket"; test "$$1" -ef "$$work/original"; \
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

check-bare: ck
	@n=$$(./ck text ground/reads-by-number.ck | head -1); \
	  if [ "$${n:-99999}" -le $(BARE) ]; then echo "parts by name: $$n reads by a bare number are left in the stored form, within the budget of $(BARE)"; \
	  else echo "parts by name: $$n reads by a bare number, over the budget of $(BARE); name the parts"; exit 1; fi

clean:
	rm -f ck fb kg seed-run .ck-header .ck-check.ice .ck-check.rec .ck-watch.rec .ck-hardened

.PHONY: all metal-image check test-inventory $(CHECK_FLOWS) $(FULL_ONLY) check-full ground-isolated ground-together watch clean header
