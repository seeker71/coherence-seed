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
	  if ./ck text ground/paths.ck ground/for-host.ck | grep -Fxq ground/host-refused.ck; then \
	    name=/ck.image.$$$$; trap 'rc=$$?; set +e; ./fb drop '$$name' 2>/dev/null; rm -rf "$$work"; exit $$rc' EXIT; idof() { printf '(#78 #%s)' $$1 | ./ck text /dev/stdin; }; \
	    ./ck text share/pack.ck < ground/nums-image.ck > "$$work/member"; \
	    { printf '\003'; idof 16; idof 77; idof 78; idof 79; } > "$$work/part"; ./seed-run "$$work/member" < "$$work/part" > "$$work/part.reply"; \
	    { printf '\001'; idof 16; dd if="$$work/part.reply" bs=1 skip=1 count=32 2>/dev/null; } > "$$work/holder"; \
	    cat "$$work/requests" "$$work/part" "$$work/holder" | ./seed-run "$$work/member" > "$$work/alone"; \
	    { $(call joined,$$name); cat "$$work/requests" "$$work/part" | CK_SHARE=$$name CK_SHARE_BYTES=67108864 ./seed-run "$$work/member" > "$$work/second"; cat "$$work/holder"; } | \
	      CK_SHARE=$$name CK_SHARE_BYTES=67108864 ./seed-run "$$work/member" > "$$work/first"; \
	    cat "$$work/second" "$$work/first" | cmp -s - "$$work/alone" || { echo "image: two members of one substrate did not reply what the port alone replies"; exit 1; }; \
	    ! $(call shown,$$name) > /dev/null 2>&1 || { echo "image: the substrate outlived its members"; exit 1; }; \
	    echo "image: two ports that are members of one substrate replied, between them, byte for byte what one alone replies, also where a node lies in a lower lane than its part"; \
	  fi; \
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

# A process is a member of the substrate CK_SHARE names when it runs after the prelude $(MEMBER), which makes or joins it through
# the host's rows. $(call shown,NAME) lists a substrate and its lanes by the same rows, and fails when none stands.
MEMBER = share/all.ck
shown = printf %s $(1) | ./ck text share/show.ck
# Wait until the substrate named $(1) has a member, for at most ten seconds.
joined = n=0; until $(call shown,$(1)) > /dev/null 2>&1; do n=$$((n+1)); [ $$n -gt 400 ] && break; sleep 0.025; done
G = .ck-ground

# Every ground program in its own process, all of them members of one substrate: sharing changes no outcome. One
# more member, the long recursion, is killed in mid-run beside them. The member that held the substrate open then
# makes every settled node again from its parts and must be answered its handle. A second substrate loses every
# member to kill -9, after ground/substrate-recycled.ck wrote its burying and a lane in the name of a process that
# runs but was started at another instant, as a process id the host gave again would leave them; the next to open it
# buries it and makes a new one. Both names are dropped at the end, and each substrate has a small allowance, so
# nothing here can hold the machine's memory.
ground-isolated: ck fb
	@set -e; paths=$$(./ck text ground/paths.ck ground/for-host.ck); name=/ck.ground.$$$$; dead=/ck.ground.$$$$.k; \
	  trap 'rc=$$?; set +e; kill -9 $$(cat $(G).victim $(G).victim1 $(G).victim2 2>/dev/null) 2>/dev/null; ./fb drop '$$name' 2>/dev/null; ./fb drop '$$dead' 2>/dev/null; rm -f $(G).differences $(G).held $(G).lanes $(G).killed $(G).new $(G).victim $(G).victim1 $(G).victim2; exit $$rc' EXIT; \
	  export CK_SHARE=$$name CK_SHARE_BYTES=1073741824; \
	  { $(call joined,$$name); (CK_MEMORY_BYTES=67108864 ./ck run $(LONG) $(MEMBER) > /dev/null 2>&1 & echo $$! > $(G).victim); \
	    (sleep 0.4; kill -9 $$(cat $(G).victim) && echo killed > $(G).killed) & \
	    printf '%s\n' "$$paths" | tr '\n' '\0' | xargs -0 -n 1 -P 6 sh -c 'got=$$(printf %s "$$0" | ./ck run ground/check-one.ck $(MEMBER) 2>&1); [ "$$got" = "node #33" ] || echo "  $$0: $$got"' > $(G).differences; \
	    wait; $(call shown,$$name) > $(G).lanes; cat region/one-handle.ck; } | ./ck text /dev/stdin $(MEMBER) > $(G).held; \
	  differences=$$(cat $(G).differences); held=$$(cat $(G).held); victim=$$(cat $(G).victim); \
	  if [ -n "$$differences" ]; then printf 'ground differences:\n%s\n' "$$differences"; exit 1; fi; \
	  [ -f $(G).killed ] && grep -q "gone  *pid $$victim " $(G).lanes || { echo "ground: the member to kill had ended, or its lane was not retired"; exit 1; }; \
	  case "$$held" in *" settled nodes, 0 not found as themselves, "*) ;; *) echo "ground: over what the members left: $$held"; exit 1;; esac; \
	  ! $(call shown,$$name) > /dev/null 2>&1 || { echo "ground: the substrate outlived its last member"; exit 1; }; \
	  export CK_SHARE=$$dead CK_SHARE_BYTES=67108864 CK_MEMORY_BYTES=67108864; \
	  for k in 1 2; do (./ck run $(LONG) $(MEMBER) > /dev/null 2>&1 & echo $$! > $(G).victim$$k); done; $(call joined,$$dead); sleep 0.3; \
	  again=$$(printf %s $$dead | ./ck run ground/substrate-recycled.ck 2>&1); [ "$$again" = 'node (#16)' ] || { echo "ground: a process id with another start: $$again"; exit 1; }; \
	  kill -9 $$(cat $(G).victim1 $(G).victim2); sleep 0.1; old=$$($(call shown,$$dead) | head -1); \
	  got=$$({ n=0; until [ "$$($(call shown,$$dead) 2> /dev/null | head -1)" != "$$old" ]; do n=$$((n+1)); [ $$n -gt 400 ] && break; sleep 0.025; done; \
	    $(call shown,$$dead) | head -1 > $(G).new; cat ground/add.ck; } | ./ck run /dev/stdin $(MEMBER)); new=$$(cat $(G).new); \
	  [ "$$got" = 'node [#4 2a]' ] && [ -n "$$old" ] && [ -n "$$new" ] && [ "$$new" != "$$old" ] && ! $(call shown,$$dead) > /dev/null 2>&1 || \
	    { echo "ground: after every member was killed the next answered '$$got'; the substrate before: '$$old', after: '$$new'"; exit 1; }; \
	  echo "ground: $$(printf '%s\n' "$$paths" | wc -l | tr -d ' ') applicable programs passed as members of one substrate while one more member was killed in mid-run; over what they left: $$held; after every member of a second substrate was killed, its burying and a lane left in the name of a process that runs but was started at another instant, the next to open it buried it and made a new one"

ground-together: ck
	@got=$$(./ck run ground/check.ck 2>&1); \
	  if [ "$$got" = "node (#16)" ]; then echo "one run: all of them in one process end as expected, since a fail gives back what it returns past"; \
	  else echo "one run: path and outcome of each program that ends otherwise in one process: $$got"; exit 1; fi

# A node is frozen by one process and thawed by a member of a substrate, which answers the node's handle; a second
# member, the one that made the substrate, names the node by that handle alone and prints it.
check-ice: ck fb
	@name=/ck.ice.$$$$; export CK_SHARE_BYTES=67108864 CK_MEMORY_BYTES=268435456; trap './fb drop '$$name' 2>/dev/null; rm -f .ck-check.ice .ck-check.named' EXIT; \
	  ./ck text ice/freeze.ck lens/all.ck ice/ice.ck < ground/recursion.ck > .ck-check.ice && \
	  b=$$({ $(call joined,$$name); CK_SHARE=$$name ./ck text ice/named.ck $(MEMBER) lens/all.ck ice/ice.ck < .ck-check.ice > .ck-check.named; cat .ck-check.named; } | CK_SHARE=$$name ./ck run /dev/stdin $(MEMBER)); \
	  if [ "$$b" = "node $$(head -1 ground/recursion.ck)" ]; then echo "ice: frozen by a program in form, thawed by a member of a substrate, and named by its handle alone, $$(cat .ck-check.named), by a second member: the same node"; \
	  else echo "ice: named '$$(cat .ck-check.named)', which is '$$b'"; exit 1; fi

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
	  then echo "held back: of five tries in a search only the one that held wrote its line; a faulted path wrote nothing; a hundred held calls ran in the order held, and a fault past a closed choice kept what was held outside it"; \
	  else echo "held back: '$$got'"; exit 1; fi

check-bare: ck
	@n=$$(./ck text ground/reads-by-number.ck | head -1); \
	  if [ "$${n:-99999}" -le $(BARE) ]; then echo "parts by name: $$n reads by a bare number are left in the stored form, within the budget of $(BARE)"; \
	  else echo "parts by name: $$n reads by a bare number, over the budget of $(BARE); name the parts"; exit 1; fi

clean:
	rm -f ck fb kg seed-run .ck-header .ck-check.ice .ck-check.rec .ck-watch.rec .ck-hardened

.PHONY: all metal-image check test-inventory $(CHECK_FLOWS) $(FULL_ONLY) check-full ground-isolated ground-together watch clean header
