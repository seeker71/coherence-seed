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
	@set -eu; work=$$(mktemp -d); trap 'rm -rf "$$work"' EXIT; \
	  word() { printf %s "$$1" | ./ck text lens/to-nodes.ck lens/all.ck; }; \
	  word '(trial.template "Read: " ".")' >"$$work/template"; word '"B"' >"$$work/other"; \
	  word '(trial.scene "room" "A" (list (trial.statement "A" "Both statements") (trial.statement "B" "Both statements")))' >"$$work/scene"; \
	  rc=0; ./ck text learning/pair.ck 3<"$$work/template" 4<"$$work/other" <"$$work/scene" >"$$work/out" || rc=$$?; test "$$rc" = 1; test "$$(cat "$$work/out")" = "fault $$(word trial.indistinguishable)"
	@set -eu; \
	  work=$$(mktemp -d); \
	  trap 'rm -rf "$$work"' EXIT; \
	  mkdir "$$work/private" "$$work/letters"; \
	  word() { printf %s "$$1" | ./ck text lens/to-nodes.ck lens/all.ck; }; \
	  digest() { openssl dgst -sha256 -binary "$$1" | od -An -tx1 -v | tr -d ' \n'; }; \
	  run() { printf %s "$$1" >"$$work/mode"; ./ck text ground/owned-study.ck 3<"$$work/mode" 4<"$$work/$${3:-none}" 5<"$$work/$${4:-none}" <"$$work/$$2"; }; \
	  g() { printf %s "$$1" >"$$work/mode"; ./ck text ground/circle-exchange.ck 3<"$$work/mode" 4<"$$work/$${4:-none}" <"$$work/$$2" >"$$work/$$3"; }; \
	  change() { g change "$$2" update "$$3"; ./ck text "$$1.ck" 3<"$$work/channel" <"$$work/update" >"$$work/$$4"; }; \
	  printf '#32' >"$$work/none"; for a in authority listening adapter-authority; do cp "$$work/none" "$$work/private/$$a"; done; \
	  printf %s "$$work" >"$$work/location"; word '"owned grammar round"' >"$$work/nonce"; g channel location channel; g identity none genesis; \
	  cp ground/language-corpus.ck "$$work/data"; run parent data >"$$work/parent-model"; parent=$$(digest "$$work/parent-model"); cases=$$(digest ground/language-cases.ck); \
	  printf '%s\n' "Owned grammar v1; six finite completions in canonical request order; score every next gold token and EOS using top-1 mistakes; control rotates all final verbs by one; assess all cases as protected." "$$cases" "$$(digest ground/owned-study.ck)" "$$(digest learning/network.ck)" >"$$work/protocol"; \
	  protocol=$$(digest "$$work/protocol"); word "(teach.material [bytes $$(digest "$$work/data")] \"$$work/letters\" [bytes $$parent] [bytes $$parent] [bytes $$protocol])" >"$$work/material"; \
	  ./ck text learning/source.ck 3<"$$work/channel" 4<"$$work/material" <"$$work/data" >"$$work/source"; g permit source granted nonce; g ticket source ticket nonce; \
	  change circle/authorize granted genesis issued; change circle/listen ticket genesis listener; \
	  ./ck text learning/teach.ck 3<"$$work/channel" <"$$work/ticket" >"$$work/round"; \
	  set -- "$$work/letters/"*; test "$$#" = 1; cp "$$1" "$$work/received"; g bytes received admitted; cmp "$$work/data" "$$work/admitted"; \
	  ./ck text learning/train.ck <"$$work/admitted" >"$$work/fit-model"; run control admitted >"$$work/perturbed"; ./ck text learning/train.ck <"$$work/perturbed" >"$$work/shuffled-model"; \
	  fit=$$(digest "$$work/fit-model"); shuffled=$$(digest "$$work/shuffled-model"); word "(list [bytes $$parent] [bytes $$fit] [bytes $$shuffled] [bytes $$protocol])" >"$$work/identities"; \
	  run design identities >"$$work/plan"; for arm in parent fit shuffled; do word "[bytes $$(digest "$$work/$$arm-model")]" >"$$work/model-id"; run run plan "$$arm-model" model-id >"$$work/$$arm"; done; \
	  ./ck text learning/assess.ck 3<"$$work/parent" 4<"$$work/fit" 5<"$$work/shuffled" <"$$work/plan" >"$$work/decision"; \
	  ./ck text learning/artifact-scope.ck 3<"$$work/plan" 4<"$$work/parent" 5<"$$work/fit" 6<"$$work/shuffled" <"$$work/round" >"$$work/derivative"; g permit derivative allowed nonce; \
	  cp "$$work/fit-model" "$$work/letters/$$fit.artifact"; change learning/allow allowed genesis approved; \
	  ./ck text learning/activate.ck 3<"$$work/channel" 4<"$$work/plan" 5<"$$work/parent" 6<"$$work/fit" 7<"$$work/shuffled" <"$$work/round" >"$$work/active"; \
	  ./ck text learning/use.ck 3<"$$work/channel" <"$$work/active" >"$$work/received-model"; cmp "$$work/received-model" "$$work/fit-model"; \
	  word '(lm.request (list 2 6) 3)' >"$$work/question"; ./ck text learning/generate.ck 3<"$$work/received-model" <"$$work/question" >"$$work/reply"; test "$$(cat "$$work/reply")" = "$$(word '(lm.completion (list 12) lm.eos)')"; \
	  mv "$$work/letters/$$fit.artifact" "$$work/letters/$$fit.safetensors"; ./ck text learning/use.ck 3<"$$work/channel" <"$$work/active" | cmp - "$$work/fit-model"; \
	  change learning/allow none approved stopped; rc=0; ./ck text learning/use.ck 3<"$$work/channel" <"$$work/active" >"$$work/refusal" || rc=$$?; test "$$rc" = 1; test "$$(cat "$$work/refusal")" = "fault $$(word share.ungranted)"; \
	  ./ck text lens/to-words.ck lens/all.ck <"$$work/decision"
	@set -eu; work=$$(mktemp -d); trap 'rm -rf "$$work"' EXIT; \
	  [ "$$(./ck run ground/supervision-reference.ck)" = 'node #33' ]; \
	  [ "$$(./ck run ground/language-reference.ck)" = 'node #33' ]; \
	  ./ck text learning/train.ck <ground/language-corpus.ck >"$$work/model"; \
	  ./ck text learning/train.ck <ground/language-corpus.ck | cmp - "$$work/model"; \
	  [ "$$(./ck run ground/language-replies.ck <"$$work/model")" = 'node #33' ]; \
	  echo "language: fresh training reproduced the weights and a new process generated each held-out completion"
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
	@set -eu; \
	  work=$$(mktemp -d); \
	  trap 'rm -rf "$$work"' EXIT; \
	  word() { printf %s "$$1" | ./ck text lens/to-nodes.ck lens/all.ck; }; \
	  station() { word "(meter.station (world.station \"$$work/sensor\" (list [bytes 4c] [bytes 48] [bytes 3f]) (list (world.rule \"stay\" (list 0 1 2)))) (meter.profile \"io-kit\" \"registry-tool\" \"mV\" 12090 $$1 [bytes 4c] [bytes 48] [bytes 3f]))"; }; \
	  packet() { word "(meter.packet $$1 (meter.reading \"io-kit\" \"mV\" $$2 $$3 $$4) (meter.reading \"$$5\" \"mV\" $$6 $$7 $$8))" >"$$work/sensor"; }; \
	  sample() { printf '%s' "$$1" | ./ck text lens/to-nodes.ck lens/all.ck >"$$work/index"; ./ck text ground/instrument-input.ck <"$$work/index" >"$$work/sensor"; }; \
	  refuses() { code=$$(word "$$1"); shift; rc=0; "$$@" >"$$work/cause" || rc=$$?; test "$$rc" = 1; test "$$(cat "$$work/cause")" = "fault $$code"; }; \
	  open_station() { ./ck text world/open.ck 3<"$$work/station"; }; \
	  predict() { ./ck text world/predict.ck 3<"$$work/station" <"$$work/history"; }; \
	  observe() { ./ck text world/observe.ck 3<"$$work/station" 4<"$$work/prediction" <"$$work/history"; }; \
	  for margin in 0 1; do \
	   station "$$margin" >"$$work/station"; sample 0; open_station >"$$work/history"; \
	   for i in 1 2 3 4 5 6 7; do predict >"$$work/prediction"; sample "$$i"; observe >"$$work/updated"; cp "$$work/updated" "$$work/history"; done; \
	   predict >"$$work/prediction"; observe | cmp - "$$work/history"; printf '%s' "$$margin" >"$$work/mode"; \
	   ./ck text ground/instrument-result.ck 3<"$$work/mode" 4<"$$work/station" <"$$work/history" >"$$work/checked"; test "$$(cat "$$work/checked")" = '#33'; \
	  done; \
	  packet 7 200 205 12090 registry-tool 201 204 12090; refuses world.conflict observe; \
	  packet 8 200 205 12090 registry-tool 206 209 12090; refuses meter.outside-window observe; \
	  packet 8 200 208 12090 io-kit 201 202 12090; refuses meter.wrong-reader observe; \
	  packet 8 200 205 12088 registry-tool 201 204 12092; refuses world.stale observe; \
	  packet 8 200000000000000 200000000000005 12088 registry-tool 200000000000001 200000000000004 12092; observe >"$$work/updated"; cp "$$work/updated" "$$work/history"; \
	  predict >"$$work/prediction"; packet 9 210 215 9223372036854775807 registry-tool 211 214 9223372036854775807; refuses world.invalid observe; \
	  echo "instrument: eight overlapping reader pairs replayed with the exact time brackets; a changed calibration preserved ambiguity and a changed duplicate was refused"
	@set -eu; \
	  work=$$(mktemp -d); \
	  trap 'rm -rf "$$work"' EXIT; \
	  word() { printf %s "$$1" | ./ck text lens/to-nodes.ck lens/all.ck; }; \
	  word "(world.station \"$$work/source\" (list [bytes 30] [bytes 31]) (list (world.rule \"hold\" (list 0 1)) (world.rule \"toggle\" (list 1 0))))" >"$$work/station"; \
	  predict() { ./ck text world/predict.ck 3<"$$work/station" <"$$work/$$1"; }; \
	  observe() { ./ck text world/observe.ck 3<"$$work/station" 4<"$$work/$$2" <"$$work/$$1"; }; \
	  refuses() { expected=$$(word "$$1"); shift; rc=0; "$$@" >"$$work/reason" || rc=$$?; [ "$$rc" = 1 ] && [ "$$(cat "$$work/reason")" = "fault $$expected" ]; }; \
	  [ "$$(./ck run ground/world-reference.ck)" = 'node #33' ]; \
	  word '(world.sample 10 [bytes 30])' >"$$work/source"; \
	  ./ck text world/open.ck 3<"$$work/station" >"$$work/initial"; \
	  predict initial >"$$work/first"; \
	  observe initial first | cmp - "$$work/initial"; \
	  word '(world.sample 10 [bytes 31])' >"$$work/source"; refuses world.conflict observe initial first; \
	  word '(world.sample 12 [bytes 31])' >"$$work/source"; refuses world.gap observe initial first; \
	  word '(world.sample 11 [bytes 31])' >"$$work/source"; \
	  observe initial first >"$$work/next"; observe initial first | cmp - "$$work/next"; \
	  predict next >"$$work/second"; refuses world.stale observe next first; \
	  word '(world.sample 12 [bytes 30])' >"$$work/source"; \
	  observe next second >"$$work/last"; \
	  ./ck text ground/world-result.ck 3<"$$work/first" 4<"$$work/second" <"$$work/last" >"$$work/result"; \
	  test "$$(cat "$$work/result")" = '#33'; \
	  rm "$$work/source"; refuses world.unobserved observe next second; \
	  echo "world: two hypotheses followed a sequenced physical source; the rejected forecast, changed ranking and exact readings survived a restart"
	@name=/ck.check.$$$$; (CK_FB=$$name ./ck run $(LONG) > /dev/null &) ; $(up); ./fb record $$name .ck-check.rec 5 > /dev/null; \
	  frames=$$(./fb play .ck-check.rec | grep -c moving); last=$$(./fb play .ck-check.rec -1 | grep '^outcome'); \
	  printf '\000\000\000\000' | dd of=.ck-check.rec bs=1 seek=60 conv=notrunc 2>/dev/null || exit 1; \
	  refused=$$(./fb play .ck-check.rec 2>&1); status=$$?; ./fb drop $$name; rm -f .ck-check.rec; \
	  [ $$status = 2 ] && [ "$$refused" = 'fb: incompatible framebuffer layout' ] || { echo "framebuffer replay: $$refused"; exit 1; }; \
	  case "$$last" in "outcome  node 2178309 "*"written by (add (call (self)"*) ok=1;; *) ok=0;; esac; \
	  if [ $$ok = 1 ] && [ $$frames -ge 2 ]; then echo "framebuffer: a second process recorded $$frames frames of a run in motion; the last holds the answer and its writer"; \
	  else echo "framebuffer: $$frames frames in motion; last frame says '$$last'"; exit 1; fi

check-held-back: ck seed-run
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

	@set -eu; \
	  stewardship_root=$$(mktemp -d); \
	  trap 'rm -rf "$$stewardship_root"' EXIT; \
	  for scene in normal before collected; do work="$$stewardship_root/$$scene"; mkdir "$$work"; \
	  mkdir "$$work/private" "$$work/letters"; \
	  printf '#32' > "$$work/none"; \
	  cp "$$work/none" "$$work/private/authority"; \
	  cp "$$work/none" "$$work/private/listening"; \
	  printf %s "$$work" > "$$work/location"; \
	  word() { printf %s "$$1" | ./ck text lens/to-nodes.ck lens/all.ck; }; \
	  case_value() { \
	  printf %s "$$2" > "$$work/mode"; \
	  ./ck text "$$1.ck" 3<"$$work/mode" 4<"$$work/$${5:-none}" <"$$work/$$3" > "$$work/$$4"; \
	  }; \
	  g() { case_value ground/circle-exchange "$$@"; }; \
	  v() { case_value value/cases "$$@"; }; \
	  change() { \
	  g change "$$2" update "$$3"; \
	  ./ck text "$$1.ck" 3<"$$work/channel" <"$$work/update" > "$$work/$$4"; \
	  }; \
	  allow_scope() { \
	  word "\"$$2\"" > "$$work/nonce"; \
	  g permit "$$1" permit nonce; \
	  g ticket "$$1" ticket nonce; \
	  change circle/authorize permit issued issued; \
	  change circle/listen ticket listened listened; \
	  }; \
	  post() { ./ck text value/post.ck 3<"$$work/channel" <"$$work/ticket"; }; \
	  account() { \
	  word "\"$$1\"" > "$$work/actor"; \
	  ./ck text value/statement.ck 3<"$$work/channel" <"$$work/actor" > "$$work/actual"; \
	  word "(value.account \"$$1\" $$2 $$3 $$4)" > "$$work/expected"; \
	  [ "$$(cat "$$work/expected")" = "$$(cat "$$work/actual")" ]; \
	  }; \
	  denied() { \
	  reason=$$1; \
	  shift; \
	  rc=0; \
	  "$$@" > "$$work/denied" || rc=$$?; \
	  expected=$$(word "$$reason"); \
	  [ "$$rc" = 1 ] && [ "$$(cat "$$work/denied")" = "fault $$expected" ] || { echo "expected $$reason, obtained $$(cat "$$work/denied")" >&2; return 1; }; \
	  }; \
	  interrupt() { \
	  printf %s "$$1" > "$$work/phase"; \
	  rc=0; \
	  ./ck text value/post.ck ground/circle-interrupt.ck 3<"$$work/channel" 4<"$$work/phase" <"$$work/ticket" > "$$work/lost-ack" || rc=$$?; \
	  [ "$$rc" = "$$2" ] && [ ! -s "$$work/lost-ack" ]; \
	  }; \
	  disagreement() { \
	  rc=0; post > "$$work/denial" || rc=$$?; \
	  [ "$$rc" = 1 ] && sed -n 's/^fault //p' "$$work/denial" > "$$work/disagreement"; \
	  v key disagreement got-key; v independent-key channel want-key nonce; cmp "$$work/got-key" "$$work/want-key"; \
	  v prior disagreement remembered; g identity "$$1" earlier; cmp "$$work/remembered" "$$work/earlier"; \
	  v dispute-for disagreement expected "$$2"; [ "$$(cat "$$work/disagreement")" = "$$(cat "$$work/expected")" ]; \
	  }; \
	  kind() { word value.kind > "$$work/field"; v field "$$1" kind field; word "$$2" > "$$work/expected"; [ "$$(cat "$$work/kind")" = "$$(cat "$$work/expected")" ]; }; \
	  g channel location channel; \
	  g identity none issued; \
	  cp "$$work/issued" "$$work/listened"; \
	  word 7 > "$$work/units"; \
	  word 9223372036854775807 > "$$work/max"; word 9223372036854775806 > "$$work/below"; word 1 > "$$work/one"; word 0 > "$$work/zero"; \
	  v sum below near one; test "$$(cat "$$work/max")" = "$$(cat "$$work/near")"; \
	  v sum max edge zero; test "$$(cat "$$work/max")" = "$$(cat "$$work/edge")"; \
	  printf sum > "$$work/mode"; \
	  denied value.overflow ./ck text value/cases.ck 3<"$$work/mode" 4<"$$work/one" <"$$work/max"; \
	  for component in interface operations; do \
	  word "value.$$component" > "$$work/component"; \
	  ./ck text value/output.ck < "$$work/component" > "$$work/$$component-output"; \
	  v "$$component-reference" none reference; \
	  cmp "$$work/$$component-output" "$$work/reference"; \
	  done; \
	  v work none work-scope interface-output; \
	  allow_scope work-scope ordinary; \
	  post > "$$work/service"; \
	  kind service value.served; \
	  post > "$$work/repeated"; \
	  cmp "$$work/service" "$$work/repeated"; \
	  account A 0 0 0; \
	  account B 32 0 0; \
	  set -- "$$work"/letters/*; first=$$1; \
	  if test "$$scene" = before; then \
	    pending() { ./ck text value/unconfirmed.ck 3<"$$work/channel" >"$$work/unconfirmed"; }; \
	    reviewed() { pending; test -s "$$work/unconfirmed"; test "$$(cat "$$work/unconfirmed")" != "$$(word '(list)')"; cp "$$work/unconfirmed" "$$work/previous-review"; rc=0; post >"$$work/denial" || rc=$$?; test "$$rc" = 1; pending; cmp "$$work/previous-review" "$$work/unconfirmed"; }; \
	    v other none other-scope operations-output; allow_scope other-scope instrument; \
	    interrupt before 75; reviewed; account B 32 0 0; reviewed; set -- "$$work/letters/"*; test "$$#" = 1; \
	    v gift service small-gift units; allow_scope small-gift distinct-gift; post >"$$work/independent-post"; account B 32 0 7; pending; cmp "$$work/previous-review" "$$work/unconfirmed"; \
	    echo "stewardship: an uncertain occurrence withheld its own retry while unrelated work completed"; \
	    continue; fi; \
	  if test "$$scene" = collected; then \
	    v other none other-scope operations-output; allow_scope other-scope instrument; \
	    interrupt after 76; \
	    for candidate in "$$work"/letters/*; do if [ ! "$$candidate" -ef "$$first" ]; then moved=$$candidate; fi; done; \
	    mv "$$moved" "$$work/collected"; \
	    pending() { ./ck text value/unconfirmed.ck 3<"$$work/channel"; }; \
	    pending > "$$work/first-review"; test "$$(cat "$$work/first-review")" != "$$(word '(list)')"; \
	    rc=0; post >"$$work/retry" || rc=$$?; test "$$rc" = 1; test ! -e "$$moved"; \
	    pending | cmp - "$$work/first-review"; account B 32 0 0; \
	    mv "$$work/collected" "$$moved"; ln "$$moved" "$$work/proof"; \
	    pending >"$$work/settled"; test "$$(cat "$$work/settled")" = "$$(word '(list)')"; \
	    post >"$$work/retry"; test "$$work/proof" -ef "$$moved"; account B 64 0 0; \
	    echo "stewardship: the collected letter withheld replay; returning the original reconciled accounting once"; \
	    continue; fi; \
	  set -- "$$work"/letters/*; original=$$1; mv "$$original" "$$work/collected"; \
	  word "\"$$original\"" > "$$work/original-path"; v missing service expected original-path; \
	  rc=0; post > "$$work/denial" || rc=$$?; \
	  test "$$rc" = 1 && test "$$(cat "$$work/denial")" = "fault $$(cat "$$work/expected")"; \
	  test ! -e "$$original"; account B 32 0 0; \
	  cp "$$work/none" "$$original"; disagreement none work-scope; \
	  mv "$$work/collected" "$$original"; post | cmp - "$$work/service"; \
	  g open none opening; g silence opening silence; g decline opening refusal; \
	  v grant permit consent; v invitation work-scope invitation ticket; \
	  cp "$$work/private/listening" "$$work/before-stop"; change circle/listen none listened listened; \
	  ./ck text circle/receive.ck 3<"$$work/channel" <"$$work/ticket" > "$$work/paused"; \
	  ./ck text witness/observe.ck 3<"$$work/invitation" 4<"$$work/before-stop" 5<"$$work/private/listening" <"$$work/paused" > "$$work/stopped"; \
	  for source in silence refusal consent stopped; do \
	  v social "$$source" social-scope; \
	  allow_scope social-scope "$$source"; \
	  post > "$$work/social"; \
	  kind social value.unpriced; \
	  v gift social social-gift units; \
	  allow_scope social-gift "gift-$$source"; \
	  denied value.protected post; \
	  v disguised social wrong-material; \
	  allow_scope wrong-material "name-$$source"; \
	  denied value.protected post; \
	  done; \
	  account A 0 0 0; \
	  account B 36 0 0; \
	  v unknown none bad-gift units; \
	  allow_scope bad-gift ghost; \
	  denied value.protected post; \
	  v wrong-content none wrong-scope; \
	  allow_scope wrong-scope changed-content; \
	  denied value.invalid post; \
	  for amount in 0 -1; do \
	  word "$$amount" > "$$work/bad-amount"; v gift service bad-amount-scope bad-amount; \
	  allow_scope bad-amount-scope "invalid-$$amount"; \
	  denied value.invalid post; \
	  done; \
	  v gift service donation units; \
	  allow_scope donation gift; \
	  change circle/listen none listened listened; \
	  denied inbox.paused post; \
	  account A 0 0 0; \
	  change circle/listen ticket listened listened; \
	  interrupt after 76; \
	  change circle/authorize none issued issued; \
	  account A 0 7 0; \
	  account B 36 0 7; \
	  denied share.ungranted post; \
	  change circle/authorize permit issued issued; \
	  post > "$$work/gift-replay"; \
	  kind gift-replay value.given; \
	  account B 36 0 7; \
	  allow_scope donation second-gift; \
	  disagreement gift-replay donation; \
	  v other none other operations-output; \
	  allow_scope other ordinary; \
	  disagreement service other; \
	  account B 36 0 7; \
	  v other-channel channel alternate; \
	  v other-owner donation another-gift; \
	  mv "$$work/channel" "$$work/original-channel"; cp "$$work/alternate" "$$work/channel"; \
	  allow_scope another-gift foreign-account; \
	  denied value.foreign post; \
	  account C 0 0 0; \
	  account B 0 0 0; \
	  mv "$$work/original-channel" "$$work/channel"; \
	  account A 0 7 0; \
	  account B 36 0 7; \
	  echo "stewardship: metered resource, four protected sources, explicit contribution and two process-interruption boundaries verified"; \
	  done

	@set -eu; \
	  work=$$(mktemp -d); \
	  trap 'rm -rf "$$work"' EXIT; \
	  printf '#32' >"$$work/none"; printf '[#2 31]' >"$$work/one"; printf '[#2 32]' >"$$work/two"; printf '[#2 63616c6c6572]' >"$$work/caller"; printf '[#2 6775657374]' >"$$work/other"; printf %s "$$work" >"$$work/location"; \
	  word() { printf %s "$$1" | ./ck text lens/to-nodes.ck lens/all.ck; }; \
	  value() { printf %s "$$2" >"$$work/mode"; ./ck text "ground/$$1.ck" 3<"$$work/mode" 4<"$$work/$${5:-none}" <"$$work/$$3" >"$$work/$$4"; }; \
	  g() { value circle-exchange "$$@"; }; \
	  c() { value continuation-case "$$@"; }; \
	  run() { ./ck text "continuation/$$1.ck" 3<"$$work/channel" 4<"$$work/$${3:-caller}" 5<"$$work/$${4:-none}" <"$$work/$${2:-job}"; }; \
	  control() { g change "$$2" change "$$3"; ./ck text "circle/$$1.ck" 3<"$$work/channel" <"$$work/change" >"$$work/$$4"; }; \
	  refused() { why=$$1; shift; rc=0; "$$@" >"$$work/failed" || rc=$$?; test "$$rc" = 1 && test "$$(cat "$$work/failed")" = "fault $$(word "$$why")" || { echo "care expected $$why, received $$(cat "$$work/failed")" >&2; return 1; }; }; \
	  reference() { printf %s "$$1" >"$$work/mode"; actual=$$(./ck text ground/continuation-reference.ck 3<"$$work/mode" 4<"$$work/job" 5<"$$work/a-owner" 6<"$$work/a-listener" 7<"$$work/b-owner" 8<"$$work/b-listener" <"$$work/$$2"); test "$$actual" = "$$(word yes)" || { echo "care expected $$1 reference, received $$actual" >&2; return 1; }; }; \
	  for scene in normal orphan repaired; do \
	    mkdir "$$work/$$scene"; destination="$$work/$$scene"; mkdir "$$destination/private" "$$destination/letters"; printf %s "$$destination" >"$$work/location"; g channel location channel; \
	    cp "$$work/none" "$$destination/private/authority"; cp "$$work/none" "$$destination/private/listening"; \
	    g identity none genesis; c first channel first-scope; c second channel second-scope; \
	    g permit first-scope first-permit one; g ticket first-scope first-ticket one; g permit second-scope second-permit two; g ticket second-scope second-ticket two; \
	    refused share.different ./ck text continuation/action.ck 3<"$$work/second-ticket" <"$$work/first-scope"; \
	    ./ck text continuation/action.ck 3<"$$work/first-ticket" <"$$work/first-scope" >"$$work/first-action"; ./ck text continuation/action.ck 3<"$$work/second-ticket" <"$$work/second-scope" >"$$work/second-action"; c pair first-action actions second-action; \
	    control authorize first-permit genesis owner-a; control listen first-ticket genesis listener-a; \
	    cp "$$destination/private/authority" "$$work/a-owner"; cp "$$destination/private/listening" "$$work/a-listener"; cp "$$work/none" "$$work/b-owner"; cp "$$work/none" "$$work/b-listener"; \
	    run plan actions >"$$work/job"; c location job head-label; head_file="$$destination/private/$$(cat "$$work/head-label")"; \
	    if test "$$scene" = repaired; then \
	      c first-path job expected-path; mv "$$destination/letters" "$$destination/offline"; \
	      run begin >"$$work/io-blocked"; c cause io-blocked io-cause; expected_path=$$(cat "$$work/expected-path"); \
	      test "$$(cat "$$work/io-cause")" = "$$(word "(list inbox.io \"publish\" \"$$expected_path\")")"; refused care.unchanged run wake; \
	      mv "$$destination/offline" "$$destination/letters"; run wake >"$$work/restored"; run resume job caller restored >"$$work/after-io"; \
	      c cause after-io next-cause; test "$$(cat "$$work/next-cause")" = "$$(word share.different)"; set -- "$$destination/letters/"*; test "$$#" = 1; \
	      g bytes "$$scene/letters/$${1##*/}" observed; printf 'first\0' | cmp - "$$work/observed"; continue; \
	    fi; \
	    if test "$$scene" = normal; then \
	      c wrong-id first-action lied; c pair lied wrong-actions second-action; run plan wrong-actions >"$$work/wrong-job"; run begin wrong-job >"$$work/wrong-result"; c cause wrong-result wrong-cause; \
	      test "$$(cat "$$work/wrong-cause")" = "$$(word care.mismatch)"; test -z "$$(ls -A "$$destination/letters")"; \
	      run begin >"$$work/blocked"; test -f "$$head_file"; cmp "$$work/blocked" "$$head_file"; reference blocked blocked; \
	      run resume | cmp - "$$work/blocked"; refused care.unchanged run wake; \
	    else \
	      c first-path job target; rc=0; ./ck text continuation/begin.ck ground/continuation-interrupt.ck 6<"$$work/target" 3<"$$work/channel" 4<"$$work/caller" <"$$work/job" >"$$work/lost" || rc=$$?; \
	      test "$$rc" = 76; test ! -s "$$work/lost"; test -s "$$head_file"; \
	    fi; \
	    set -- "$$destination/letters/"*; test "$$#" = 1; first_path=$$1; ln "$$first_path" "$$destination/first-proof"; g bytes "$$scene/first-proof" observed; printf 'first\0' | cmp - "$$work/observed"; \
	    control authorize second-permit owner-a owner-b; cp "$$destination/private/authority" "$$work/b-owner"; \
	    if test "$$scene" = normal; then \
	      run wake >"$$work/early"; refused care.wrong-owner run resume job other early; \
	    fi; \
	    control listen second-ticket listener-a listener-b; cp "$$destination/private/listening" "$$work/b-listener"; \
	    if test "$$scene" = normal; then \
	      run resume | cmp - "$$work/blocked"; refused care.stale run resume job caller early; \
	      run wake >"$$work/event"; c other-owner channel other-channel; \
	      refused care.wrong-owner ./ck text continuation/resume.ck 3<"$$work/other-channel" 4<"$$work/caller" 5<"$$work/event" <"$$work/job"; \
	      mv "$$first_path" "$$destination/temporarily-absent"; refused care.absent run resume job caller event; cmp "$$work/blocked" "$$head_file"; test -z "$$(ls -A "$$destination/letters")"; mv "$$destination/temporarily-absent" "$$first_path"; \
	      c second-path job target; rc=0; ./ck text continuation/resume.ck ground/continuation-interrupt.ck 3<"$$work/channel" 4<"$$work/caller" 5<"$$work/event" 6<"$$work/target" <"$$work/job" >"$$work/lost" || rc=$$?; \
	      test "$$rc" = 76; test ! -s "$$work/lost"; set -- "$$destination/letters/"*; test "$$#" = 2; \
	      refused care.stale run resume job caller event; run resume >"$$work/finished"; reference finished finished; \
	      refused care.stale run resume job caller event; \
	    else \
	      refused care.wrong-owner run resume job other; run resume >"$$work/finished"; reference orphan finished; \
	    fi; \
	    run resume | cmp - "$$work/finished"; run begin | cmp - "$$work/finished"; \
	    set -- "$$destination/letters/"*; test "$$#" = 2; if test "$$1" -ef "$$destination/first-proof"; then second=$$2; else test "$$2" -ef "$$destination/first-proof"; second=$$1; fi; \
	    printf bytes >"$$work/mode"; ./ck text ground/circle-exchange.ck 3<"$$work/mode" <"$$second" >"$$work/observed"; printf 'second\n' | cmp - "$$work/observed"; \
	  done; \
	  mkdir "$$work/crossed" "$$work/crossed/private" "$$work/crossed/letters"; printf %s "$$work/crossed" >"$$work/location"; g channel location alpha-channel; c other-owner alpha-channel beta-channel; \
	  cp "$$work/none" "$$work/crossed/private/authority"; cp "$$work/none" "$$work/crossed/private/listening"; cp "$$work/caller" "$$work/alpha-caller"; cp "$$work/other" "$$work/beta-caller"; \
	  g identity none source-head; cp "$$work/source-head" "$$work/listen-head"; \
	  pair_call() { ./ck text "continuation/$$2.ck" 3<"$$work/$$1-channel" 4<"$$work/$$1-caller" 5<"$$work/$${3:-none}" <"$$work/$$1-job"; }; \
	  pair_rights() { cp "$$work/$$1-channel" "$$work/channel"; control authorize "$$1-$$2-permit" source-head source-head; control listen "$$1-$$2-ticket" listen-head listen-head; }; \
	  pair_delivered() { c "$$2-path" "$$1-job" pair-path; printf bytes >"$$work/mode"; ./ck text ground/circle-exchange.ck 3<"$$work/mode" <"$$(cat "$$work/pair-path")" >"$$work/actual-bytes"; printf '%s' "$$1/$$2" | cmp - "$$work/actual-bytes"; }; \
	  for who in alpha beta; do \
	    for slot in first second; do \
	      marker=$$(printf '%s' "$$who/$$slot" | od -An -v -tx1 | tr -d ' \n'); printf '[#1 %s]' "$$marker" >"$$work/marker"; c "$$slot" "$$who-channel" "$$who-$$slot-scope" marker; \
	      nonce=one; if test "$$slot" = second; then nonce=two; fi; g permit "$$who-$$slot-scope" "$$who-$$slot-permit" "$$nonce"; g ticket "$$who-$$slot-scope" "$$who-$$slot-ticket" "$$nonce"; \
	      ./ck text continuation/action.ck 3<"$$work/$$who-$$slot-ticket" <"$$work/$$who-$$slot-scope" >"$$work/$$who-$$slot-action"; \
	    done; \
	    c pair "$$who-first-action" "$$who-actions" "$$who-second-action"; ./ck text continuation/plan.ck 3<"$$work/$$who-channel" 4<"$$work/$$who-caller" <"$$work/$$who-actions" >"$$work/$$who-job"; \
	    pair_call "$$who" begin >"$$work/$$who-wait"; c cause "$$who-wait" pair-cause; test "$$(cat "$$work/pair-cause")" = "$$(word share.ungranted)"; \
	  done; \
	  pair_rights alpha first; pair_call alpha wake >"$$work/wake-alpha"; refused care.unchanged pair_call beta wake; \
	  pair_rights beta first; pair_call beta wake >"$$work/wake-beta"; refused care.unchanged pair_call alpha wake; \
	  refused care.stale pair_call alpha resume wake-beta; refused care.stale pair_call beta resume wake-alpha; refused care.stale pair_call alpha resume wake-alpha; \
	  for who in alpha beta; do pair_call "$$who" resume | cmp - "$$work/$$who-wait"; done; test -z "$$(ls -A "$$work/crossed/letters")"; \
	  pair_call beta resume wake-beta >"$$work/beta-middle"; pair_delivered beta first; pair_call alpha resume | cmp - "$$work/alpha-wait"; \
	  pair_rights beta second; pair_call beta wake >"$$work/updated"; pair_call beta resume updated >"$$work/beta-finished"; pair_delivered beta second; refused care.unchanged pair_call alpha wake; \
	  for slot in first second; do pair_rights alpha "$$slot"; pair_call alpha wake >"$$work/updated"; pair_call alpha resume updated >"$$work/alpha-last"; pair_delivered alpha "$$slot"; done; \
	  pair_call beta resume | cmp - "$$work/beta-finished"; pair_call alpha resume | cmp - "$$work/alpha-last"; set -- "$$work/crossed/letters/"*; test "$$#" = 4; \
	  echo "continuation: beta finished before alpha in one store; crossed wakes left both checkpoints unchanged and all four owner-specific letters retained their exact bytes"; \
	  clock_call() { word "$$2" >"$$work/now"; ./ck text "continuation/$$1.ck" ground/broker-clock.ck 3<"$$work/channel" 4<"$$work/caller" 5<"$$work/$${3:-none}" 7<"$$work/now" <"$$work/job"; }; \
	  for scene in allowed expired; do \
	    mkdir "$$work/$$scene" "$$work/$$scene/private" "$$work/$$scene/letters"; printf %s "$$work/$$scene" >"$$work/location"; g channel location channel; \
	    cp "$$work/none" "$$work/$$scene/private/authority"; cp "$$work/none" "$$work/$$scene/private/listening"; g identity none genesis; \
	    c first channel first-scope; c second channel second-scope; g permit first-scope first-permit one; g ticket first-scope first-ticket one; g ticket second-scope second-ticket two; \
	    word '(list 100 110)' >"$$work/window"; c finite first-permit timed-permit window; control authorize timed-permit genesis owner-a; control listen first-ticket genesis listener-a; \
	    for slot in first second; do ./ck text continuation/action.ck 3<"$$work/$$slot-ticket" <"$$work/$$slot-scope" >"$$work/$$slot-action"; done; \
	    c pair first-action actions second-action; run plan actions >"$$work/job"; clock_call begin 99 >"$$work/early"; c cause early cause; test "$$(cat "$$work/cause")" = "$$(word share.early)"; \
	    refused care.unchanged clock_call wake 99; clock_call wake 100 >"$$work/edge"; \
	    if test "$$scene" = expired; then \
	      refused care.stale clock_call resume 110 edge; run resume | cmp - "$$work/early"; test -z "$$(ls -A "$$work/$$scene/letters")"; \
	    else \
	      clock_call resume 100 edge >"$$work/middle"; c first-path job path; printf bytes >"$$work/mode"; ./ck text ground/circle-exchange.ck 3<"$$work/mode" <"$$(cat "$$work/path")" >"$$work/actual"; printf 'first\0' | cmp - "$$work/actual"; \
	    fi; \
	  done; \
	  echo "continuation: changing only the time woke the exact finite grant at 100; the same signal at the exclusive cutoff 110 left the original checkpoint and destination untouched"; \
	  echo "continuation: two intended letters, original consent and caller retained; dropped and stale wakes kept the checkpoint; another receiver reconciled an unacknowledged link"

	@set -eu; \
	  area=$$(mktemp -d); \
	  trap 'rm -rf "$$area"' EXIT; \
	  printf '#32' > "$$area/none"; \
	  pc() { printf %s "$$1" > "$$area/mode"; ./ck text ground/provider-case.ck 3<"$$area/mode" 4<"$${4:-$$area/none}" <"$$2" >"$$3"; }; \
	  word() { printf %s "$$1" | ./ck text lens/to-nodes.ck lens/all.ck; }; \
	  testword() { word "$$2" >"$$area/want"; [ "$$(cat "$$1")" = "$$(cat "$$area/want")" ]; }; \
	  ./ck text learning/train.ck <ground/language-corpus.ck > "$$area/model"; \
	  pc job "$$area/model" "$$area/job"; pc policy "$$area/none" "$$area/local-policy"; \
	  pc changed-source "$$area/job" "$$area/other"; \
	  pc roster "$$area/none" "$$area/roster"; pc alt-roster "$$area/none" "$$area/alt-roster"; \
	  ./ck text circle/pin.ck <"$$area/roster" >"$$area/pin"; \
	  ./ck text circle/pin.ck <"$$area/alt-roster" >"$$area/alt-pin"; \
	  for role in owner listener withdraw alt-owner alt-listener; do pc "$$role-proof" "$$area/none" "$$area/$$role-proof"; done; \
	  setup() { \
	   dir="$$area/$$1"; mkdir -p "$$dir/private" "$$dir/letters"; \
	   prefix=$${2:-}; cp "$$area/$${prefix}roster" "$$dir/private/peers"; \
	   cp "$$area/$${prefix}pin" "$$dir/private/listening"; cp "$$area/$${prefix}pin" "$$dir/private/authority"; \
	   word "\"$$dir\"" > "$$dir/path"; pc channel "$$dir/path" "$$dir/channel"; \
	  }; \
	  apply_role() { ./ck text circle/apply.ck 3<"$$dir/channel" 4<"$$area/$${prefix}$$1-change" <"$$area/$${prefix}$$1-proof" >"$$area/revision"; }; \
	  job_run() { ./ck text provider/dispatch.ck 3<"$$dir/channel" 4<"$$1" 7<"$$2" < /dev/null > "$$3"; }; \
	  check_route() { pc decision "$$1" "$$area/decision"; pc selected "$$area/decision" "$$area/kind"; testword "$$area/kind" "$$2"; pc cause "$$area/decision" "$$area/cause"; testword "$$area/cause" "$$3"; verify_trace; }; \
	  verify_trace() { for record in "$$dir/private"/route-*; do if cmp -s "$$record" "$$area/decision"; then return 0; fi; done; return 1; }; \
	  empty() { test -z "$$(ls -A "$$dir/letters")"; }; \
	  setup refused; \
	  pc revision "$$area/none" "$$area/genesis"; pc permit "$$area/job" "$$area/permit" "$$dir/channel"; \
	  pc ticket "$$area/job" "$$area/ticket" "$$dir/channel"; \
	  pc change "$$area/permit" "$$area/owner-change" "$$area/genesis"; \
	  pc change "$$area/ticket" "$$area/listener-change" "$$area/genesis"; \
	  apply_role owner; \
	  pc change "$$area/none" "$$area/withdraw-change" "$$area/revision"; \
	  job_run "$$area/none" "$$area/job" "$$area/no-listener"; check_route "$$area/no-listener" provider.unserved inbox.paused; empty; \
	  apply_role listener; \
	  ./ck text provider/offer.ck 3<"$$dir/channel" <"$$area/job" >"$$area/prepared"; \
	  apply_role withdraw; \
	  job_run "$$area/none" "$$area/job" "$$area/cancelled"; check_route "$$area/cancelled" provider.unserved share.ungranted; empty; \
	  setup active; \
	  apply_role owner; apply_role listener; \
	  job_run "$$area/none" "$$area/other" "$$area/wrong"; check_route "$$area/wrong" provider.unserved share.different; empty; \
	  job_run "$$area/none" "$$area/job" "$$area/sent"; check_route "$$area/sent" provider.external inbox.accepted; \
	  cp "$$area/decision" "$$area/dispatch"; set -- "$$dir/letters/"*; test "$$#" = 1; cp "$$1" "$$area/letter"; \
	  pc bytes-exact "$$area/letter" "$$area/exact" "$$area/job"; testword "$$area/exact" yes; \
	  ./ck text provider/answer.ck 3<"$$area/model" 4<"$$area/roster" <"$$area/letter" >"$$area/answer"; \
	  if ./ck text provider/answer.ck 3<"$$area/model" 4<"$$area/alt-roster" <"$$area/letter" >"$$area/wrong-episode" 2>"$$area/recipient-error"; then exit 1; fi; \
	  test "$$(cat "$$area/wrong-episode")" = "fault $$(word e.id)"; \
	  pc generated "$$area/answer" "$$area/match"; testword "$$area/match" yes; \
	  seal() { pc "$$1" "$$area/none" "$$area/signature-node"; ./ck text "$$area/signature-node" >"$$area/raw-sig"; ./ck text provider/seal.ck 4<"$$area/raw-sig" <"$$2" >"$$3"; }; \
	  seal answer-signature "$$area/answer" "$$area/attested"; \
	  seal wrong-signature "$$area/answer" "$$area/wrong-role"; \
	  pc changed-answer "$$area/answer" "$$area/changed-answer"; \
	  seal answer-signature "$$area/changed-answer" "$$area/altered"; \
	  pc ledger "$$area/job" "$$area/ledger"; \
	  unchanged() { pc was-untouched "$$area/settlement" "$$area/unchanged" "$$area/ledger"; testword "$$area/unchanged" yes; }; \
	  consume() { ./ck text provider/resume.ck 3<"$$dir/channel" 4<"$$1" 7<"$$area/dispatch" 8<"$$area/ledger" <"$${2:-$$area/attested}" >"$$area/settlement"; pc settled "$$area/settlement" "$$area/status"; }; \
	  consume "$$area/job" "$${PROVIDER_WITNESS:-$$area/attested}"; testword "$$area/status" share.allowed; pc completed "$$area/settlement" "$$area/completed" "$$area/answer"; testword "$$area/completed" yes; \
	  for envelope in altered wrong-role; do consume "$$area/job" "$$area/$$envelope"; testword "$$area/status" ed25519.invalid-signature; unchanged; done; \
	  consume "$$area/other"; testword "$$area/status" e.id; unchanged; \
	  job_run "$$area/none" "$$area/job" "$$area/repeated"; check_route "$$area/repeated" provider.external inbox.replayed; cmp "$$1" "$$area/letter"; \
	  apply_role withdraw; \
	  consume "$$area/job"; testword "$$area/status" share.ungranted; unchanged; \
	  setup renewed alt-; \
	  pc permit "$$area/job" "$$area/alt-permit" "$$dir/channel"; pc ticket "$$area/job" "$$area/alt-ticket" "$$dir/channel"; \
	  pc change "$$area/alt-permit" "$$area/alt-owner-change" "$$area/genesis"; \
	  pc change "$$area/alt-ticket" "$$area/alt-listener-change" "$$area/genesis"; \
	  apply_role owner; apply_role listener; \
	  job_run "$$area/none" "$$area/job" "$$area/renewed-turn"; check_route "$$area/renewed-turn" provider.external inbox.accepted; \
	  cp "$$area/dispatch" "$$area/first-decision"; cp "$$area/decision" "$$area/dispatch"; \
	  consume "$$area/job"; testword "$$area/status" e.id; unchanged; \
	  ./ck text provider/answer.ck 3<"$$area/model" 4<"$$area/alt-roster" <"$$(find "$$dir/letters" -type f)" >"$$area/renewed-answer"; \
	  if cmp -s "$$area/answer" "$$area/renewed-answer"; then exit 1; fi; \
	  for record in "$$area/active/private"/route-*; do if cmp -s "$$record" "$$area/first-decision"; then cp "$$record" "$$dir/private/"; fi; done; \
	  cp "$$area/first-decision" "$$area/dispatch"; consume "$$area/job"; testword "$$area/status" e.id; unchanged; \
	  setup local; \
	  apply_role owner; apply_role listener; \
	  ./ck text nums/pack.ck <ground/nums-image.ck >"$$area/image"; \
	  mkfifo "$$area/requests" "$$area/returns"; ./seed-run "$$area/image" <"$$area/requests" >"$$area/returns" & pid=$$!; \
	  ./ck text provider/dispatch.ck 5>"$$area/requests" <"$$area/returns" 3<"$$dir/channel" 4<"$$area/local-policy" 7<"$$area/job" >"$$area/served"; \
	  wait "$$pid"; pc decision "$$area/served" "$$area/decision"; pc selected "$$area/decision" "$$area/kind"; testword "$$area/kind" provider.local; verify_trace; empty; \
	  pc local-answer "$$area/served" "$$area/local-answer"; pc local-value "$$area/local-answer" "$$area/value"; testword "$$area/value" 42; \
	  ./ck text provider/dispatch.ck 5>/dev/null </dev/null 3<"$$dir/channel" 4<"$$area/local-policy" 7<"$$area/job" >"$$area/uncertain-local"; \
	  check_route "$$area/uncertain-local" provider.unserved '(list provider.local e.short)'; empty; \
	  printf 'received canonical letter bytes: '; wc -c <"$$area/letter"; \
	  printf 'refused provider inbox bytes: '; find "$$area/refused/letters" -type f -exec cat {} + | wc -c; \
	  echo "provider: exact dispatch, signed answer, epoch change, late refusal and local preference agreed"

	@set -eu; \
	  work=$$(mktemp -d); \
	  trap 'rm -rf "$$work"' EXIT; \
	  ./ck text ground/search-trace.ck > "$$work/reference"; \
	  module() { printf '%s\n' ground/search-package.ck "nums/lang-$$1.ck" | ./ck text nums/write-text.ck > "$$2"; }; \
	  adapter() { printf %s "$$1" | ./ck text ground/search-targets.ck; }; \
	  compare() { test "$$(cat "$$work/actual")" = "$$(cat "$$work/reference")" || { diff -u "$$work/reference" "$$work/actual"; return 1; }; }; \
	  if command -v python3 >/dev/null 2>&1; then \
	  module python "$$work/search.py"; adapter python-runtime > "$$work/form.py"; adapter python-reader >> "$$work/search.py"; \
	  python3 "$$work/search.py" > "$$work/actual"; compare; echo "projection: Python observed all five native answer and effect traces"; \
	  fi; \
	  if command -v deno >/dev/null 2>&1; then \
	  module typescript "$$work/search.ts"; adapter typescript-runtime > "$$work/form.ts"; adapter typescript-reader >> "$$work/search.ts"; \
	  deno run "$$work/search.ts" > "$$work/actual"; compare; echo "projection: TypeScript observed all five native answer and effect traces"; \
	  fi; \
	  jhome=$${JAVA_HOME:-}; javac=$${jhome:+$$jhome/bin/}javac; java=$${jhome:+$$jhome/bin/}java; \
	  if "$$javac" -version >/dev/null 2>&1; then \
	  mkdir -p "$$work/j/form/write"; module java "$$work/j/form/write/Writer.java"; \
	  for type in Node Entry Blueprinted Fault Runtime; do adapter "java-$$type" > "$$work/j/form/$$type.java"; done; \
	  adapter java-reader > "$$work/j/Check.java"; "$$javac" "$$work/j"/form/*.java "$$work/j/form/write/Writer.java" "$$work/j/Check.java"; \
	  "$$java" -cp "$$work/j" Check > "$$work/actual"; compare; echo "projection: Java observed all five native answer and effect traces"; \
	  fi; \
	  if command -v dotnet >/dev/null 2>&1 && version=$$(dotnet --version 2>/dev/null); then \
	  mkdir "$$work/cs"; module csharp "$$work/cs/Writer.cs"; adapter csharp-runtime > "$$work/cs/Program.cs"; \
	  printf '<Project Sdk="Microsoft.NET.Sdk"><PropertyGroup><OutputType>Exe</OutputType><TargetFramework>net%s.0</TargetFramework></PropertyGroup></Project>\n' "$${version%%.*}" > "$$work/cs/Fixture.csproj"; \
	  dotnet build "$$work/cs/Fixture.csproj" -o "$$work/net" --nologo --verbosity quiet; \
	  dotnet "$$work/net/Fixture.dll" > "$$work/actual"; compare; echo "projection: C# observed all five native answer and effect traces"; \
	  fi;

	@set -eu; \
	  work=$$(mktemp -d); \
	  trap 'rm -rf "$$work"' EXIT; \
	  word() { printf %s "$$1" | ./ck text lens/to-nodes.ck lens/all.ck; }; \
	  pc() { printf %s "$$1" >"$$work/mode"; ./ck text ground/mesh-case.ck 3<"$$work/mode" <"$${2:-$$work/no}" ; }; \
	  word no >"$$work/no"; \
	  for name in roster foreign-roster episode withdraw-change withdraw-proof wrong-role altered-time competing-receipt; do pc "$$name" >"$$work/$$name"; done; \
	  for tag in A B; do for kind in ticket expectation arrival receipt letter owner-change owner-proof listener-change listener-proof; do pc "$$tag-$$kind" >"$$work/$$tag-$$kind"; done; done; \
	  setup() { area="$$work/$$1"; mkdir -p "$$area/private" "$$area/letters"; word "\"$$area\"" >"$$area/path"; pc channel "$$area/path" >"$$area/channel"; }; \
	  for place in sender receiver; do setup "$$place"; cp "$$work/roster" "$$area/private/peers"; ./ck text circle/pin.ck <"$$work/roster" >"$$area/private/authority"; cp "$$area/private/authority" "$$area/private/listening"; done; \
	  setup witness; \
	  ./ck text mesh/pin.ck <"$$work/roster" >"$$area/private/mesh-time"; \
	  apply() { for actor in owner listener; do ./ck text circle/apply.ck 3<"$$work/$$1/channel" 4<"$$work/$$2-$$actor-change" <"$$work/$$2-$$actor-proof" >"$$work/last-change"; done; }; \
	  expecting() { ./ck text mesh/expect.ck 3<"$$work/sender/channel" <"$$work/$$1-ticket" >"$$work/expectation"; cmp "$$work/expectation" "$$work/$$1-expectation"; }; \
	  apply sender A; apply receiver A; expecting A; \
	  cp -R "$$work/receiver" "$$work/early-receiver"; \
	  mkfifo "$$work/to-clock" "$$work/from-clock"; \
	  cross() { \
	   sample=$$1; shift; \
	   word "$$sample" >"$$work/tick"; \
	   ./ck text "$${CLOCK_SERVICE:-ground/mesh-clock.ck}" 3<"$$work/witness/channel" 4<"$$work/tick" <"$$work/to-clock" 5>"$$work/from-clock" >"$$work/service-log" & clock_pid=$$!; \
	   code=0; "$$@" 5>"$$work/to-clock" <"$$work/from-clock" >"$$work/delivered" || code=$$?; \
	   wait "$$clock_pid" || return 1; \
	   return "$$code"; \
	  }; \
	  deliver() { cross "$$2" ./ck text mesh/deliver.ck 3<"$$work/receiver/channel" 4<"$$work/$$1-expectation"; }; \
	  check_error() { test "$$(cat "$$work/$$1")" = "fault $$(word "$$2")"; }; \
	  for sample in mesh.episode mesh.challenge no; do \
	   if deliver A "$$sample"; then exit 1; fi; \
	   reason=e.id; if test "$$sample" = no; then reason=e.short; fi; check_error delivered "$$reason"; \
	   test -z "$$(ls -A "$$work/receiver/letters")"; \
	  done; \
	  deliver A 100; \
	  cmp "$$work/delivered" "$$work/A-arrival"; set -- "$$work/receiver/letters/"*; test "$$#" = 1; cmp "$$1" "$$work/A-letter"; \
	  test -z "$$(ls -A "$$work/sender/letters")"; \
	  if ./ck text mesh/confirm.ck 3<"$$work/sender/channel" <"$$work/B-receipt" >"$$work/confirmed"; then exit 1; fi; check_error confirmed e.unbound; \
	  for p in sender receiver; do apply "$$p" B; done; expecting B; \
	  deliver B 120; cmp "$$work/delivered" "$$work/B-arrival"; \
	  confirm() { ./ck text mesh/confirm.ck 3<"$$work/sender/channel" <"$$work/$$1" >"$$work/confirmed"; }; \
	  confirm B-receipt; cmp "$$work/confirmed" "$$work/B-receipt"; \
	  if test -n "$${MESH_WITNESS:-}"; then cp "$$MESH_WITNESS" "$$work/guest-delivery"; confirm guest-delivery; cmp "$$work/confirmed" "$$work/A-receipt"; fi; \
	  confirm A-receipt; cmp "$$work/confirmed" "$$work/A-receipt"; \
	  confirm B-receipt; cmp "$$work/confirmed" "$$work/B-receipt"; \
	  for event in altered-time wrong-role competing-receipt; do \
	   if confirm "$$event"; then exit 1; fi; \
	   reason=ed25519.invalid-signature; if test "$$event" = competing-receipt; then reason=inbox.conflict; fi; \
	   test "$$(cat "$$work/confirmed")" = "fault $$(word "$$reason")"; \
	  done; \
	  confirm A-receipt; cmp "$$work/confirmed" "$$work/A-receipt"; \
	  cp "$$work/foreign-roster" "$$work/sender/private/peers"; \
	  if confirm A-receipt; then exit 1; fi; check_error confirmed e.id; \
	  cp "$$work/roster" "$$work/sender/private/peers"; \
	  mv "$$work/receiver" "$$work/later-receiver"; cp -R "$$work/early-receiver" "$$work/receiver"; \
	  if deliver A 110; then exit 1; fi; \
	  test "$$(cat "$$work/delivered")" = "fault $$(word share.clock-regressed)"; \
	  test -z "$$(ls -A "$$work/receiver/letters")"; \
	  pc tick "$$work/witness/private/mesh-time" >"$$work/watermark"; word 120 >"$$work/want"; test "$$(cat "$$work/watermark")" = "$$(cat "$$work/want")"; \
	  if deliver A 125; then exit 1; fi; \
	  test "$$(cat "$$work/delivered")" = "fault $$(word inbox.stale)"; test -z "$$(ls -A "$$work/receiver/letters")"; \
	  if deliver B 125; then exit 1; fi; \
	  test "$$(cat "$$work/delivered")" = "fault $$(word inbox.stale)" || test "$$(cat "$$work/delivered")" = "fault $$(word inbox.paused)"; \
	  test -z "$$(ls -A "$$work/receiver/letters")"; \
	  cp "$$work/later-receiver/private/authority" "$$work/later-receiver/private/listening" "$$work/receiver/private"; \
	  ./ck text circle/apply.ck 3<"$$work/sender/channel" 4<"$$work/withdraw-change" <"$$work/withdraw-proof" >"$$work/stopped"; \
	  cross 130 ./ck text mesh/checkpoint.ck 3<"$$work/sender/channel"; \
	  if deliver B 135; then exit 1; fi; check_error delivered inbox.stale; \
	  test -z "$$(ls -A "$$work/receiver/letters")"; \
	  cross 201 ./ck text mesh/now.ck 3<"$$work/episode"; \
	  pc tick "$$work/witness/private/mesh-time" >"$$work/watermark"; test "$$(cat "$$work/watermark")" = "$$(word 201)"; \
	  word 205 >"$$work/tick"; \
	  ./ck text ground/mesh-clock.ck 3<"$$work/witness/channel" 4<"$$work/tick" <"$$work/to-clock" 5>"$$work/from-clock" >"$$work/service-log" & session_pid=$$!; \
	  exec 7>"$$work/to-clock" 8<"$$work/from-clock"; \
	  ./ck text mesh/now.ck 3<"$$work/episode" 5>&7 <&8 >"$$work/clock-observation"; \
	  pc witness-busy "$$work/witness/channel" >"$$work/busy"; test "$$(cat "$$work/busy")" = "$$(word yes)"; \
	  exec 7>&- 8<&-; wait "$$session_pid"; \
	  pc witness-busy "$$work/witness/channel" >"$$work/busy"; test "$$(cat "$$work/busy")" = "$$(word no)"; \
	  CLOCK_SERVICE=mesh/clock.ck cross 0 ./ck text mesh/now.ck 3<"$$work/episode"; \
	  pc tick "$$work/witness/private/mesh-time" >"$$work/watermark"; cmp "$$work/watermark" "$$work/delivered"; \
	  echo "mesh: signed arrival, reordered receipts, changed attestations and rollback agreed"

	@set -eu; \
	  work=$$(mktemp -d); \
	  trap 'rm -rf "$$work"' EXIT; \
	  mkdir "$$work/private" "$$work/letters"; \
	  printf '#32' >"$$work/none"; printf %s "$$work" >"$$work/location"; \
	  cp "$$work/none" "$$work/private/authority"; cp "$$work/none" "$$work/private/listening"; \
	  word() { printf %s "$$1" | ./ck text lens/to-nodes.ck lens/all.ck; }; \
	  g() { printf %s "$$1" >"$$work/mode"; ./ck text ground/circle-exchange.ck 3<"$$work/mode" 4<"$$work/$${4:-none}" <"$$work/$$2" >"$$work/$$3"; }; \
	  g channel location channel; g identity none genesis; \
	  fmt='(voice.format 16000 1 voice.s16le)'; \
	  turn='[bytes 000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f]'; \
	  printf '\001\002\003\004' >"$$work/en.pcm"; printf '\212\214' >"$$work/old.pcm"; printf '\005\006' >"$$work/id.pcm"; \
	  if [ -n "$${VOICE_SAMPLES:-}" ]; then cp "$$VOICE_SAMPLES/english.pcm" "$$work/en.pcm"; cp "$$VOICE_SAMPLES/indonesian.pcm" "$$work/id.pcm"; cp "$$VOICE_SAMPLES/original.pcm" "$$work/old.pcm"; fi; \
	  offer() { word "(voice.request (voice.line (voice.segment $$turn $$2 $$fmt) \"$$3\" \"$$4\" \"$$5\") 0 0x7fffffffffffffff)" >"$$work/request"; ./ck text voice/prepare.ck 3<"$$work/channel" 4<"$$work/$$1.pcm" <"$$work/request" >"$$work/$$1-permit"; ./ck text voice/ticket.ck <"$$work/$$1-permit" >"$$work/$$1-ticket"; ./ck text voice/expect.ck <"$$work/$$1-permit" >"$$work/$$1-id"; }; \
	  control() { g change "$$2" change "$$3"; ./ck text "circle/$$1.ck" 3<"$$work/channel" <"$$work/change" >"$$work/$$4"; }; \
	  receive() { ./ck text circle/receive.ck 3<"$$work/channel" <"$$work/$$1-ticket" >"$$work/reply"; g status reply status; test "$$(cat "$$work/status")" = "$$(word "$$2")"; }; \
	  clip() { proof=$$(./ck text lens/to-words.ck lens/all.ck <"$$work/$$2-id"); printf '(voice.clip "%s" %s)' "$$1" "$$proof"; }; \
	  queue() { word "(voice.queue $$turn $$fmt (list $$1))" >"$$work/queue"; }; \
	  play() { ./ck text voice/render.ck 5>"$$work/sound" <"$$work/queue" >"$$work/receipt"; }; \
	  offer en 0 en "We may listen before we speak." Samantha; \
	  offer old 1 id "Kita boleh berbagi cerita." Damayanti; \
	  offer id 1 id "Kami boleh berbagi cerita." Damayanti; \
	  control authorize en-permit genesis a; control listen en-ticket genesis b; receive en inbox.accepted; \
	  set -- "$$work/letters/"*; test "$$#" = 1; first=$$1; queue "$$(clip "$$first" en) $$(clip "$$first" en)"; play; cmp "$$work/sound" "$$work/en.pcm"; \
	  kind=$$(word inbox.letter); printf '(%s %s [#1 aabbccdd])' "$$kind" "$$(cat "$$work/en-ticket")" >"$$work/substituted"; \
	  queue "$$(clip "$$work/substituted" en)"; rc=0; play || rc=$$?; test "$$rc" = 1; test "$$(cat "$$work/receipt")" = "fault $$(word voice.different)"; test ! -s "$$work/sound"; queue "$$(clip "$$first" en)"; \
	  control authorize old-permit a a; control listen old-ticket b b; control authorize none a a; receive old share.ungranted; \
	  set -- "$$work/letters/"*; test "$$#" = 1; play; cmp "$$work/sound" "$$work/en.pcm"; \
	  control authorize id-permit a a; receive id inbox.paused; \
	  control listen id-ticket b b; receive id inbox.accepted; receive id inbox.replayed; \
	  set -- "$$work/letters/"*; test "$$#" = 2; if test "$$first" -ef "$$1"; then second=$$2; else second=$$1; fi; \
	  queue "$$(clip "$$second" id)"; rc=0; play || rc=$$?; test "$$rc" = 1; test "$$(cat "$$work/receipt")" = "fault $$(word voice.gap)"; test ! -s "$$work/sound"; \
	  queue "$$(clip "$$first" en) $$(clip "$$second" id) $$(clip "$$first" en)"; play; cat "$$work/en.pcm" "$$work/id.pcm" >"$$work/expected"; cmp "$$work/expected" "$$work/sound"; \
	  length=$$(wc -c <"$$work/expected"); test "$$(cat "$$work/receipt")" = "$$(word "(voice.result 2 $$length $$((length/2)) 0 voice.accepted)")"; \
	  if [ -n "$${VOICE_RENDERED:-}" ]; then cp "$$work/sound" "$$VOICE_RENDERED"; fi; \
	  ./ck text ground/voice-short.ck 5>"$$work/short-sound" <"$$work/queue" >"$$work/short"; head -c 1 "$$work/expected" | cmp - "$$work/short-sound"; \
	  test "$$(cat "$$work/short")" = "$$(word '(voice.result 0 1 0 1 voice.short)')"; \
	  control authorize old-permit a a; control listen old-ticket b b; receive old inbox.conflict; play; cmp "$$work/expected" "$$work/sound"; \
	  echo "voice: both owners controlled the two ordered slots; withdrawal emitted no second clip, replay produced identical PCM, a short write retained its exact frame remainder"

	@test "$$(./ck run ground/translation-reference.ck)" = 'node #33'

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

.PHONY: translate-local
translate-local: ck
	@set -eu; work=$$(mktemp -d); trap 'rm -rf "$$work"' EXIT; \
	  model=$${TRANSLATION_MODEL:?set TRANSLATION_MODEL to a local MLX checkpoint directory}; input=$${TRANSLATION_UTTERANCE:?set TRANSLATION_UTTERANCE to the owned utterance file}; \
	  model=$$(cd "$$model" && pwd -P); test -r "$$input"; engine=$$(command -v mlx_lm.generate); \
	  fingerprint() { root=$$1; mkdir "$$root"; (cd "$$model"; test -f model.safetensors; set -- *.safetensors; test "$$#" -eq 1; test -f tokenizer.json; openssl dgst -sha256 -binary model.safetensors >"$$root/weights"; openssl dgst -sha256 -binary tokenizer.json >"$$root/vocabulary"; for file in *.json *.model *.tiktoken *.jinja; do if test -f "$$file"; then shasum -a 256 "$$file"; fi; done) >"$$root/assets"; }; \
	  fingerprint "$$work/before"; printf '%s' "$${TRANSLATION_MODEL_NAME:-local MLX checkpoint}" >"$$work/name"; \
	  { (cd "$$(dirname "$$engine")" && shasum -a 256 "$$(basename "$$engine")"); interpreter=$$(sed -n '1s/^#!//p' "$$engine"); prefix=$$(dirname "$$(dirname "$$interpreter")"); for meta in "$$prefix"/lib/python*/site-packages/mlx_lm-*.dist-info/METADATA "$$prefix"/lib/python*/site-packages/mlx-*.dist-info/METADATA; do if test -f "$$meta"; then sed -n -e '/^Name: /p' -e '/^Version: /p' "$$meta"; fi; done; } >"$$work/engine"; \
	  ./ck text translation/model.ck 3<"$$work/name" 4<"$$work/before/weights" 5<"$$work/before/vocabulary" 6<"$$work/engine" <"$$work/before/assets" >"$$work/model"; \
	  { printf '@review\n'; cat "$$input"; } >"$$work/open"; ./ck text lens/translate.ck 3<"$$work/model" <"$$work/open" >"$$work/request"; \
	  ./ck text translation/prompt.ck <"$$work/request" >"$$work/prompt"; ./ck text translation/settings.ck <"$$work/request" >"$$work/settings"; \
	  if test -s "$$work/prompt"; then set -- --model "$$model" --prompt -; while IFS= read -r arg; do set -- "$$@" "$$arg"; done <"$$work/settings"; \
	    if HF_HUB_OFFLINE=1 TRANSFORMERS_OFFLINE=1 "$$engine" "$$@" <"$$work/prompt" >"$$work/answer" 2>"$$work/diagnostic"; then status=1; code=0; else code=$$?; status=2; fi; \
	    fingerprint "$$work/after"; for part in weights vocabulary assets; do cmp "$$work/before/$$part" "$$work/after/$$part" || { echo 'Model files changed during translation.' >&2; exit 1; }; done; \
	  else status=0; code=0; : >"$$work/answer"; : >"$$work/diagnostic"; fi; \
	  printf '%s' "$$code" | ./ck text lens/to-nodes.ck lens/all.ck >"$$work/code"; printf '%s' "$$status" | ./ck text lens/to-nodes.ck lens/all.ck >"$$work/status"; ./ck text translation/respond.ck 3<"$$work/request" 4<"$$work/status" 5<"$$work/code" 6<"$$work/diagnostic" <"$$work/answer" >"$$work/reply"; \
	  ./ck text translation/review.ck 3<"$$work/request" <"$$work/reply"
