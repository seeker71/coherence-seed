# The bootstrap's build. Its lines are counted at each build (graph node 5.9.2): the count is to fall
# as parts leave C for Form, and no fixed number holds work back.
# Nothing here needs more than a C compiler: no part of the old kernel builds or runs this repository.
# How many times the stored nodes may still read a part of a node by a bare number (graph node
# 7.22). The count is of the nodes, whatever names lie beside them. It is lowered, never raised.
BARE = 454
CC ?= cc
CFLAGS = -std=c99 -O2 -Wall -Wextra -Werror
ifeq ($(shell uname -s),Linux)
LDLIBS ?= -pthread -ldl -lrt
endif
BOOT = bootstrap/ck.c bootstrap/fb.h
LONG = ground/long-recursion.ck
STORE = ground/region-describes-itself.ck
# Images of starts at rest that a check makes lie in a folder of its own in the tree, ignored by git, which it removes.
IMAGES = "$$(mktemp -d "$$PWD/.ck-images.XXXXXX")"
# A start saves its images in the user's cache unless CK_IMAGES says where; what make runs keeps them in the tree.
export CK_IMAGES ?= $(CURDIR)/.ck-images

all: ck fb kg seed-run

ck: $(BOOT)
	@echo "bootstrap: $$(cat $(BOOT) | wc -l | tr -d ' ') lines of C"
	$(CC) $(CFLAGS) -o ck bootstrap/ck.c $(LDLIBS)

seed-run: ck $(BOOT)
	$(CC) $(CFLAGS) -DNUMERIC_PORT -o $@ bootstrap/ck.c $(LDLIBS)

# The observer: a second process that shows, records and replays a framebuffer.
fb: tools/fb.c bootstrap/fb.h
	$(CC) $(CFLAGS) -o fb tools/fb.c $(LDLIBS)

# The claim graph's checker.
kg: tools/kg.c
	$(CC) $(CFLAGS) -o kg tools/kg.c

# Node files are stored as records (spec/node-encoding.md, section 8); git shows them through the lens. Run once per clone.
views: ck
	git config diff.ck.textconv "sh -c './ck text lens/to-words.ck lens/all.ck < \"\$$0\"'"

# bootstrap/fb.h is made from nodes: the catalogue's words and the layout of the region's records. So is
# region/places.ck, the places of the head that machine code reads, as numbers, so code made from it holds no words.
PLACES = printf '(seq (step (read (file "region/layout.ck"))) (fb.places))' | ./ck text lens/to-nodes.ck lens/all.ck > .ck-places.run && ./ck run .ck-places.run | sed 's/^node //' | $(NODES)
# form/print.ck, the printer of the answer line, is made by form/printer.ck: the walked printer and, beside it, the leaf
# the emitter compiles of the printer for AArch64, run where the layout version is the one it was made for.
PRINTER = ./ck run form/printer.ck | sed 's/^node //' | $(NODES)
header: ck
	./ck text region/header.ck > .ck-header && mv .ck-header bootstrap/fb.h
	$(PLACES) > .ck-places && mv .ck-places region/places.ck; rm -f .ck-places.run
	$(PRINTER) > .ck-print && mv .ck-print form/print.ck
	./ck text form/seed.ck > .ck-seed && mv .ck-seed ice/seed.ck

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
export METAL_RECIPE METAL_INPUTS METAL_APPLICATION METAL_IMAGE METAL_LIBRARY
metal-image: ck
	@set -eu; : "$${METAL_RECIPE:?set METAL_RECIPE to a NUMS declaration}" "$${METAL_INPUTS:?set METAL_INPUTS to a list of byte leaves}" "$${METAL_IMAGE:?set METAL_IMAGE to the output path}"; \
	  work=$$(mktemp -d); out=; trap 'rm -rf "$$work"; if [ -n "$$out" ]; then rm -f "$$out"; fi' EXIT; \
	  library=$${METAL_LIBRARY:-}; \
	  if [ -z "$$library" ]; then \
	    ./ck text nums/metal-source.ck < "$$METAL_RECIPE" > "$$work/kernel.metal" || { cat "$$work/kernel.metal" >&2; exit 1; }; \
	    xcrun -sdk macosx metal -std=macos-metal2.4 -c "$$work/kernel.metal" -o "$$work/kernel.air"; \
	    library="$$work/kernel.metallib"; xcrun -sdk macosx metallib "$$work/kernel.air" -o "$$library"; \
	  fi; \
	  out=$$(mktemp "$$METAL_IMAGE.XXXXXX"); \
	  set -- "$$METAL_RECIPE" "$$METAL_INPUTS" "$$library"; \
	  if [ -n "$$METAL_APPLICATION" ]; then set -- "$$@" "$$METAL_APPLICATION"; fi; \
	  printf '%s\n' "$$@" | ./ck text nums/metal-pack.ck > "$$out" || { cat "$$out" >&2; exit 1; }; \
	  mv -f "$$out" "$$METAL_IMAGE"

# The image flow runs cold, as the reference, then saves its start at rest and is entered from that image: the replies are the same bytes.
check-image: seed-run
	@set -e; root=$$PWD; work=$$(mktemp -d); images=$(IMAGES); trap 'rm -rf "$$work" "$$images"' EXIT; mkdir "$$work/run"; \
	  ./ck text nums/pack.ck < ground/nums-image.ck > "$$work/run/image"; \
	  ./ck text ground/nums-requests.ck > "$$work/requests"; test -s "$$work/requests"; \
	  ./ck text ground/nums-native-requests.ck >> "$$work/requests"; export CK_IMAGES="$$images"; mkdir -p "$$images"; \
	  (cd "$$work/run" && CK_IMAGES= "$$root/seed-run" image) < "$$work/requests" > "$$work/cold"; \
	  (cd "$$work/run" && "$$root/seed-run" image) < "$$work/requests" > "$$work/replies"; saved=$$(ls -i "$$images"); test -n "$$saved"; \
	  (cd "$$work/run" && "$$root/seed-run" image) < "$$work/requests" > "$$work/warm"; \
	  cmp -s "$$work/cold" "$$work/replies" && cmp -s "$$work/cold" "$$work/warm" && [ "$$saved" = "$$(ls -i "$$images")" ] || { echo "image: the replies of a start entered from its image differ from a cold start's, or it was not entered"; exit 1; }; \
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
	  echo "image: the standalone runtime answered by content id from a directory containing only the image, cold and entered from its start saved at rest, the same bytes"; \
	  if ./ck text ground/paths.ck ground/for-host.ck | grep -Fxq ground/host-refused.ck; then \
	    name=/ck.image.$$$$; trap 'rc=$$?; set +e; ./fb drop '$$name' 2>/dev/null; rm -rf "$$work" "$$images"; exit $$rc' EXIT; idof() { printf '(#78 #%s)' $$1 | $(NODES) | ./ck text /dev/stdin; }; \
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
	    gpu=$$(./ck run "$$work/target" host/metal-all.ck); case "$$gpu" in 'node #32') exit 0;; 'node #33') ;; *) echo "$$gpu"; exit 1;; esac; \
	    $(MAKE) --no-print-directory metal-image METAL_RECIPE=ground/metal-polynomial.ck METAL_INPUTS=ground/metal-vectors.ck METAL_APPLICATION=ground/metal-application.ck METAL_IMAGE="$$work/run/image"; \
	    ./ck text ground/metal-requests.ck > "$$work/requests"; \
	    rm -f "$$images"/*.image; (cd "$$work/run" && CK_IMAGES= "$$root/seed-run" image) < "$$work/requests" > "$$work/cold"; \
	    (cd "$$work/run" && "$$root/seed-run" image) < "$$work/requests" > "$$work/replies"; saved=$$(ls -i "$$images"); test -n "$$saved"; \
	    (cd "$$work/run" && "$$root/seed-run" image) < "$$work/requests" > "$$work/warm"; \
	    cmp -s "$$work/cold" "$$work/replies" && cmp -s "$$work/cold" "$$work/warm" && [ "$$saved" = "$$(ls -i "$$images")" ] || { echo "Metal: the replies of a start entered from its image differ from a cold start's, or it was not entered"; exit 1; }; \
	    got=$$(./ck run ground/metal-replies.ck < "$$work/replies"); test "$$got" = 'node #33' || { echo "Metal replies differed: $$got"; exit 1; }; \
	    echo "Metal: direct, attached and batched calls by ID agree with NUMS frames, cold and entered from the image of the start that loads them, the device opened after"; \
	    printf '(note "rounds" %s)' "$(METAL_ROUNDS)" | ./ck text lens/to-nodes.ck lens/all.ck > "$$work/rounds"; \
	    got=$$(./ck run ground/metal-holds.ck "$$work/rounds" < "$$work/run/image"); test "$$got" = 'node (#16)' || { echo "Metal in one run: $$got"; exit 1; }; \
	    echo "Metal: $$((2 * $(METAL_ROUNDS))) maps in one run leave in the region what $(METAL_ROUNDS) leave, and numbers only; has-target runs its value once, and a fault or a fail midway lets every object go"; \
	  fi

check-live: ck fb
	@name=/ck.ask.$$$$; work=$$(mktemp -d); trap 'rm -rf "$$work"' EXIT; budget=33554432; (CK_MEMORY_BYTES=$$budget CK_FB=$$name ./ck run $(LONG) > /dev/null &) ; $(up); \
	  ask() { printf '%s\n%s' "$$name" "$$1" | ./ck text region/ask.ck | tr -d '()\n'; }; \
	  on=$$(ask '(seq (need ((recover (seq (take u8 '"$$((budget - 4096))"') no) {reason | reason}) = (list "full" "rooms"))) (bind add (lazy (seq (note #1002 (add 1 (if (held #1002) (held #1002) 0))) (base $$0)))))'); \
	  one=$$(ask '(held #1002)'); two=$$(ask '(held #1002)'); off=$$(ask '(unbind add)'); three=$$(ask '(held #1002)'); four=$$(ask '(held #1002)'); \
	  printf '(seq (step (read (file "region/all.ck"))) (let named (file "/dev/stdin") watched (fb.watch (pack text (all (let c (lens.bytes-of named) (when (32 < c) c))))) m (held fb.mapped) size (far.size m) (tuple ((far.bytes m 0 8) = [bytes 434b4642310a0000]) ((count (far.bytes m (size - 8) 8)) = 8) ((fb.num (fb.offset (fb.kind "head") "bytes") 8) = size) (count (far.bytes m 0 0)) (outcome (far.bytes m (size - 7) 8)) (outcome (far.bytes m -1 1)) (outcome (far.bytes m 0 -1)))))' | ./ck text lens/to-nodes.ck lens/all.ck > "$$work/edges"; \
	  edges=$$(printf %s $$name | ./ck run "$$work/edges"); none=$$(printf %s $$name.none | ./ck run "$$work/edges"); \
	  n=0; until ./fb show $$name | grep -q 'mode done'; do n=$$((n+1)); [ $$n -gt 1200 ] && break; sleep 0.05; done; \
	  last=$$(./fb show $$name | grep '^outcome'); ./fb drop $$name; n1=$${one##* }; n2=$${two##* }; \
	  case "$$last" in "outcome  node 2178309 "*) ok=1;; *) ok=0;; esac; \
	  if [ $$ok = 1 ] && [ "$$on" = 'list "node" yes' ] && [ "$$off" = "$$on" ] && [ "$${n1:-0}" -gt 0 ] && [ "$${n2:-0}" -gt "$$n1" ] && [ "$$three" = "$$four" ] && [ "$$edges" = 'node (#16 #33 #33 #33 [#4] (#16 [#2 6e6f7468696e67] [#2 72616e6765]) ^2 ^2)' ] && case "$$none" in 'fault (#16 [#2 6e6f2d726567696f6e] '*) true;; *) false;; esac; \
	  then echo "live: a room beyond the shared allowance recovered; a second process installed an addition counter, read $$n1 and $$n2, removed it, and observed the final answer 2178309; read from outside through host rows, its bytes end where its head says, and a name that is no region is refused"; \
	  else echo "live: on '$$on', counts '$$one' '$$two', off '$$off', after '$$three' '$$four', final '$$last', edges '$$edges', none '$$none'"; exit 1; fi

# How many maps ground/metal-holds.ck compares with twice as many: few under make check, more here.
METAL_ROUNDS ?= 2
check-full: METAL_ROUNDS = 25
check-full: check
	@$(MAKE) -k ground-isolated ground-together $(FULL_ONLY)

check-header: ck
	@set -eu; work=$$(mktemp -d); trap 'rm -rf "$$work"' EXIT; \
	  for words in yes no '0' '(list yes no 0)' '(let seed-value 4 seed-value)'; do \
	    printf %s "$$words" | ./ck text lens/to-canonical.ck lens/all.ck >"$$work/protocol"; \
	    printf '%s\n' "$$words" >"$$work/want"; ./ck text lens/show-canonical.ck lens/all.ck <"$$work/protocol" | cmp - "$$work/want"; \
	  done; \
	  printf 'junk' >"$$work/invalid"; printf '#33\n(#16)\nextra' >"$$work/trailing"; \
	  for input in invalid trailing; do rc=0; ./ck text lens/show-canonical.ck lens/all.ck <"$$work/$$input" >"$$work/refusal" || rc=$$?; \
	    test "$$rc" = 1; case "$$input" in invalid) expected='fault [#2 73796e746178]';; trailing) expected='fault [#2 747261696c696e67]';; esac; \
	    test "$$(cat "$$work/refusal")" = "$$expected"; done
	@./ck text region/header.ck | cmp -s - bootstrap/fb.h && $(PLACES) | cmp -s - region/places.ck && $(PRINTER) | cmp -s - form/print.ck && ./ck text form/seed.ck | cmp -s - ice/seed.ck && rm -f .ck-places.run && echo "header: bootstrap/fb.h, region/places.ck, form/print.ck and the seed ice/seed.ck are what the nodes generate, the seed built again on itself the same bytes" || \
	  { rm -f .ck-places.run; echo "header: bootstrap/fb.h, region/places.ck, form/print.ck or ice/seed.ck is not what the nodes write; run make header"; exit 1; }


# A process is a member of the substrate CK_SHARE names when it runs after the prelude $(MEMBER), which loads the host's and
# the substrate's packages, a start that is saved at rest, and leaves the opening of the door and the joining to run after
# it. $(call shown,NAME) lists a substrate and its lanes by the same rows, after the start share/load.ck, and fails when none stands.
MEMBER = share/all.ck
# Every process restores a fail by the leaf the seed, ice/seed.ck, holds in the cell of choice-point. $(FLOOR) compiles
# that leaf again in the run that reads it, and the leaf it makes must restore as the seed's does.
FLOOR = form/walk.ck
# A canonical print written here is read by the reader in form and stored as the records ck reads.
NODES = ./ck text form/from-print.ck
# Signed feature protocols retain the canonical print; stored Form programs use records.
CANONICAL_WORDS = ./ck text lens/to-canonical.ck lens/all.ck
shown = printf %s $(1) | ./ck text share/show.ck share/load.ck
# Wait until the substrate named $(1) has a member, for at most ten seconds.
joined = n=0; until $(call shown,$(1)) > /dev/null 2>&1; do n=$$((n+1)); [ $$n -gt 400 ] && break; sleep 0.025; done
G = .ck-ground

# Every ground program in its own process, all of them members of one substrate: sharing changes no outcome. One
# more member, the long recursion, is killed in mid-run beside them. The member that held the substrate open then
# makes every settled node again from its parts and must be answered its handle. A second substrate loses every
# member to kill -9, after ground/substrate-recycled.ck wrote its burying and a lane in the name of a process that
# runs but was started at another instant, as a process id the host gave again would leave them; the next to open it
# buries it and makes a new one. Then ground/substrate-stale.ck keeps a third substrate mapped while its last member
# leaves and another process makes one of the same name, and buries through that stale mapping: the live one's name
# must stand. Every name is dropped at the end, and each substrate has a small allowance, so nothing here can hold
# the machine's memory.
ground-isolated: ck fb
	@set -e; paths=$$(./ck text ground/paths.ck ground/for-host.ck); name=/ck.ground.$$$$; dead=/ck.ground.$$$$.k; stale=/ck.ground.$$$$.s; images=$(IMAGES); export CK_IMAGES="$$images"; \
	  trap 'rc=$$?; set +e; rm -rf "$$images"; kill -9 $$(cat $(G).victim $(G).victim1 $(G).victim2 2>/dev/null) 2>/dev/null; ./fb drop '$$name' 2>/dev/null; ./fb drop '$$dead' 2>/dev/null; ./fb drop '$$stale' 2>/dev/null; ./fb drop '$$name'.d 2>/dev/null; rm -f $(G).stale $(G).differences $(G).held $(G).lanes $(G).killed $(G).new $(G).victim $(G).victim1 $(G).victim2; exit $$rc' EXIT; \
	  export CK_SHARE=$$name CK_SHARE_BYTES=536870912; \
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
	  export CK_SHARE=$$stale; ( (sleep 1; cat ground/add.ck) | ./ck run /dev/stdin $(MEMBER) > /dev/null 2>&1 &); $(call joined,$$stale); \
	  (printf %s $$stale | env -u CK_SHARE ./ck run ground/substrate-stale.ck > $(G).stale 2>&1 &); sleep 1.3; \
	  ( (sleep 2; cat ground/add.ck) | ./ck run /dev/stdin $(MEMBER) > /dev/null 2>&1 &); sleep 3.2; \
	  [ "$$(cat $(G).stale)" = 'node (#16)' ] && ! $(call shown,$$stale) > /dev/null 2>&1 || { echo "ground: burying through a stale mapping: $$(cat $(G).stale)"; exit 1; }; \
	  direct=$$(CK_SHARE=$$name.d CK_SHARE_BYTES=67108864 CK_MEMORY_BYTES=268435456 ./ck run $(STORE) $(MEMBER) 2>&1); alone=$$(env -u CK_SHARE CK_MEMORY_BYTES=268435456 ./ck run $(STORE) 2>&1); \
	  [ "$$direct" = "$$alone" ] || { echo "ground: a member making with no choice open answered: $$(printf %s "$$direct" | cut -c1-300)"; exit 1; }; \
	  echo "ground: $$(printf '%s\n' "$$paths" | wc -l | tr -d ' ') applicable programs passed as members of one substrate, each fail restored by the leaf of the seed, which a member holds before it joins, while one more member was killed in mid-run; over what they left: $$held; after every member of a second substrate was killed, its burying and a lane left in the name of a process that runs but was started at another instant, the next to open it buried it and made a new one; a process that buried through its mapping of a substrate already ended left the live one of that name alone; and a member running $(STORE) with no choice open, whose leaf publishes what it makes, answered what a process alone answers"

# All of them in one process, which is entered from the image of its start region/all.ck, saved at rest by the process
# before it; the image must be the one that process saved, not one made again: so each expectation is held cold, as a
# member, above, and warm, here.
ground-together: ck
	@d=$(IMAGES); mkdir -p "$$d"; export CK_IMAGES="$$d"; trap 'rm -rf "$$d"' EXIT; \
	  ./ck run ground/add.ck region/all.ck > /dev/null; saved=$$(ls -i "$$d"); \
	  got=$$(./ck run ground/check.ck region/all.ck 2>&1); \
	  if [ "$$got" = "node (#16)" ] && [ -n "$$saved" ] && [ "$$saved" = "$$(ls -i "$$d")" ]; then echo "one run: all of them in one process, entered from the image of its start, end as expected, since a fail gives back what it returns past"; \
	  else echo "one run: path and outcome of each program that ends otherwise in one process: $$got; the image before '$$saved', after '$$(ls -i "$$d")'"; exit 1; fi

include participant/commands.mk participant/enrollment.mk
include ground/checks/circle.mk ground/checks/components.mk ground/checks/continuation.mk \
        ground/checks/learning.mk ground/checks/mesh.mk ground/checks/participant.mk \
        ground/checks/provider.mk ground/checks/recovery.mk ground/checks/translation.mk \
        ground/checks/value.mk ground/checks/voice.mk ground/checks/world.mk

# A region saved at rest by one process is entered by the next, and each case where it must not be is not, pinned by
# what a cold start answers there: a start file binds entry 4096 to a recipe and the program answers it. Two builds of
# one start are the same bytes, and an image replaced after a case is the one a build makes.
# A node is frozen by one process and thawed by a member of a substrate, which answers the node's handle; a second
# member, the one that made the substrate, names the node by that handle alone and prints it.
check-ice: ck fb
	@set -eu; work=$$(mktemp -d); trap 'rm -rf "$$work"' EXIT; \
	  word() { printf %s "$$1" | $(CANONICAL_WORDS); }; \
	  word '(trial.template "Read: " ".")' >"$$work/template"; word '"B"' >"$$work/other"; \
	  word '(trial.scene "room" "A" (list (trial.statement "A" "Both statements") (trial.statement "B" "Both statements")))' >"$$work/scene"; \
	  rc=0; ./ck text learning/pair.ck 3<"$$work/template" 4<"$$work/other" <"$$work/scene" >"$$work/out" || rc=$$?; test "$$rc" = 1; test "$$(cat "$$work/out")" = "fault $$(word trial.indistinguishable)"
	$(verify_owned_model)
	$(verify_fit_replay)
	@name=/ck.ice.$$$$; export CK_SHARE_BYTES=67108864 CK_MEMORY_BYTES=268435456; trap './fb drop '$$name' 2>/dev/null; rm -f .ck-check.ice .ck-check.named' EXIT; \
	  ./ck text ice/freeze.ck lens/all.ck ice/ice.ck < ground/recursion.ck > .ck-check.ice && \
	  b=$$({ $(call joined,$$name); CK_SHARE=$$name ./ck text ice/named.ck $(MEMBER) lens/all.ck ice/ice.ck < .ck-check.ice > .ck-check.named; cat .ck-check.named; } | CK_SHARE=$$name ./ck run /dev/stdin $(MEMBER)); \
	  if [ "$$b" = "node $$(./ck text lens/to-print.ck < ground/recursion.ck)" ]; then echo "ice: frozen by a program in form, thawed by a member of a substrate, and named by its handle alone, $$(./ck text lens/to-print.ck < .ck-check.named), by a second member: the same node"; \
	  else echo "ice: named '$$(./ck text lens/to-print.ck < .ck-check.named)', which is '$$b'"; exit 1; fi
	@d=$(IMAGES); mkdir -p "$$d/images"; export CK_IMAGES="$$d/images"; I="$$CK_IMAGES/coherence-seed-images"; trap 'rm -rf "$$d"' EXIT; \
	  bind() { printf '(#71 #4096 (#36 [#4 0%s]))' "$$1" | $(NODES) > "$$d/start.ck"; }; \
	  printf '(#4096)' | $(NODES) > "$$d/program.ck"; \
	  ask() { ./ck run "$$d/program.ck" "$$d/start.ck" 2>&1; }; \
	  image() { ls "$$I"/*.image 2>/dev/null; }; \
	  fail=0; is() { [ "$$1" = "$$2" ] || { echo "  $$3: '$$1', not '$$2'"; fail=1; }; }; \
	  bind 1; is "$$(ask)" 'node [#4 01]' 'built'; one=$$(image); [ -n "$$one" ] || { echo "  no image was saved"; fail=1; }; \
	  cp "$$one" "$$d/first"; is "$$(ask)" 'node [#4 01]' 'entered'; cmp -s "$$one" "$$d/first" || { echo "  an entered image changed"; fail=1; }; \
	  rm -f "$$one"; is "$$(ask)" 'node [#4 01]' 'built again'; cmp -s "$$one" "$$d/first" || { echo "  two builds of one start differ"; fail=1; }; \
	  pages() { low=$$(od -A n -t u8 -j 104 -N 8 "$$1" | tr -d ' '); spot=$$(od -A n -t u8 -j 72 -N 8 "$$1" | tr -d ' '); n=$$(od -A n -t u4 -j 12 -N 4 "$$1" | tr -d ' '); \
	    od -A n -t u8 -j 88 -N $$((n * 24)) "$$1"; dd if="$$1" bs=16384 skip=$$((low / 16384)) count=$$(((spot - low + 16383) / 16384)) 2>/dev/null | head -c $$((spot - low)); }; \
	  pages "$$one" > "$$d/pages"; rm -f "$$one"; CK_MEMORY_BYTES=67108864 ./ck run "$$d/program.ck" "$$d/start.ck" > /dev/null; \
	  pages "$$one" | cmp -s - "$$d/pages" || { echo "  a start saved at another allowance saved other pages"; fail=1; }; \
	  if [ "$$(uname -m)" = arm64 ] && arch -x86_64 /usr/bin/true 2>/dev/null && mkdir -p "$$d/x86/region" "$$d/x86/form" && cp region/rest.ck "$$d/x86/region/" && cp form/nodes.ck form/print.ck "$$d/x86/form/" && $(CC) -arch x86_64 $(CFLAGS) -o "$$d/x86/ck" bootstrap/ck.c 2>/dev/null; then \
	    mkdir -p "$$d/x86/ice" && cp ice/seed.ck "$$d/x86/ice/"; rm -f "$$one"; is "$$({ arch -x86_64 "$$d/x86/ck" run "$$d/program.ck" "$$d/start.ck" 2>&1; echo "exit $$?"; } | tr '\n' ' ')" 'ck: the seed, ice/seed.ck, carries code for AArch64 alone, and this processor is another exit 2 ' 'built for x86-64'; \
	    is "$$(image)" '' 'a start refused for its processor'; fi; \
	  rm -f "$$one"; ask > /dev/null; cmp -s "$$one" "$$d/first" || { echo "  a build after the others differs from the first"; fail=1; }; \
	  bind 2; is "$$(ask)" 'node [#4 02]' 'a start file changed'; \
	  cp -p "$$d/start.ck" "$$d/before"; bind 3; touch -r "$$d/before" "$$d/start.ck"; is "$$(ask)" 'node [#4 03]' 'changed with its size and times put back'; \
	  bind 1; ask > /dev/null; cp "$$one" "$$d/first"; \
	  printf '\377\377\377\377' | dd of="$$one" bs=1 seek=16 conv=notrunc 2>/dev/null; is "$$(ask)" 'node [#4 01]' 'another executable'; cmp -s "$$one" "$$d/first" || { echo "  the image of another executable was not replaced"; fail=1; }; \
	  printf '\000\000\000\000' | dd of="$$one" bs=1 seek=8 conv=notrunc 2>/dev/null; is "$$(ask)" 'node [#4 01]' 'another layout'; cmp -s "$$one" "$$d/first" || { echo "  the image of another layout was not replaced"; fail=1; }; \
	  cold=$$(CK_IMAGES= CK_MEMORY_BYTES=262144 ./ck run "$$d/program.ck" "$$d/start.ck" 2>&1); is "$$(CK_MEMORY_BYTES=262144 ./ck run "$$d/program.ck" "$$d/start.ck" 2>&1)" "$$cold" 'an allowance smaller than the image charges'; \
	  is "$$(CK_SHARE=/ck.stale.$$$$ ask)" 'node [#4 01]' 'another layout by the setting CK_SHARE'; \
	  dd if="$$d/first" of="$$one" bs=16384 count=2 2>/dev/null; is "$$(ask)" 'node [#4 01]' 'an image cut short'; cmp -s "$$one" "$$d/first" || { echo "  an image cut short was not replaced"; fail=1; }; \
	  rm -f "$$one"; dd if="$$d/first" of="$$one.ab12cd" bs=16384 count=2 2>/dev/null; is "$$(ask)" 'node [#4 01]' 'a builder killed mid-save'; cmp -s "$$one" "$$d/first" && [ -e "$$one.ab12cd" ] || { echo "  a young builder's file was taken for an image, or removed"; fail=1; }; \
	  rm -f "$$one"; touch -t 202001010000 "$$one.ab12cd"; is "$$(ask)" 'node [#4 01]' 'a builder killed long ago'; cmp -s "$$one" "$$d/first" && [ ! -e "$$one.ab12cd" ] || { echo "  an old builder's file was not removed"; fail=1; }; \
	  printf '(#39 (#80) (#71 #4096 (#36 [#4 05])))' | $(NODES) > "$$d/impure.ck"; ./ck run "$$d/program.ck" "$$d/impure.ck" > /dev/null; is "$$(ls "$$I" | wc -l | tr -d ' ')" 1 'a start that read the clock'; \
	  printf '(#100 (#35 [#3]) (#36 (#40 (#39 (#42 (#44 (#49 (#35 [#3 01]) (#65 ^1)))) [#4]) (#108 (#67 ^1 ^9) (#110 (#37 (#38) ^1 (#46 ^9 [#4 01])) [#4 08])))) (#100 ^9 (#37 ^1 (#94 [#4 c004] ^20) ^12) (#100 (#35 [#3 02]) (#37 ^1 (#94 [#4 c804] ^20) ^12) (#70 #89 (#70 #16 (#43 (#40 [#2 73] (#70 #94 (#70 #16 (#43 (#40 (#47 (#46 ^9 ^26) [#4 0010]) ^42)))))))))))' | $(NODES) > "$$d/top.ck"; ./ck run "$$d/top.ck" | sed 's/^node //' | $(NODES) > "$$d/stack.ck"; printf '(#90 [#2 73])' | $(NODES) > "$$d/held.ck"; one1=$$(./ck run "$$d/held.ck" "$$d/stack.ck"); \
	  is "$$(ls "$$I" | wc -l | tr -d ' ')" 1 'a start that read the stack'; [ "$$one1" != "$$(./ck run "$$d/held.ck" "$$d/stack.ck")" ] || { echo "  two starts read the same bytes of the stack: one was entered"; fail=1; }; \
	  if [ "$$(uname -m)" = arm64 ]; then printf '(#89 [#2 74] (#51 [#52 40e03bd5010080d2c0035fd6]))' | $(NODES) > "$$d/code.ck"; printf '(#90 [#2 74])' | $(NODES) > "$$d/counter.ck"; one1=$$(./ck run "$$d/counter.ck" "$$d/code.ck"); \
	    is "$$(ls "$$I" | wc -l | tr -d ' ')" 1 'a start that ran code'; [ "$$one1" != "$$(./ck run "$$d/counter.ck" "$$d/code.ck")" ] || { echo "  two starts read one count of the clock in code: one was entered"; fail=1; }; fi; \
	  rm -f "$$one"; is "$$(./ck run "$$d/program.ck" "$$d/start.ck" "$$d/impure.ck" 2>&1)" 'node [#4 05]' 'a start whose last file read the clock'; \
	  is "$$(ls "$$I" | wc -l | tr -d ' ')" 1 'the part of a start before a file that read the clock'; cmp -s "$$one" "$$d/first" || { echo "  the part of a start at rest was not saved as that part alone is"; fail=1; }; \
	  saved=$$(ls -i "$$one"); is "$$(./ck run "$$d/program.ck" "$$d/start.ck" "$$d/impure.ck" 2>&1)" 'node [#4 05]' 'a start entered at its part at rest'; is "$$(ls -i "$$one")" "$$saved" 'the image of the part entered'; \
	  printf '(#71 #4096 (#36 [#4 06]))' | $(NODES) > "$$d/impure.ck"; is "$$(./ck run "$$d/program.ck" "$$d/start.ck" "$$d/impure.ck" 2>&1)" 'node [#4 06]' 'the rest of a start made pure'; \
	  is "$$(ls "$$I" | wc -l | tr -d ' ')" 2 'the whole start saved after its part was entered'; whole=$$(ls -i "$$I"); \
	  printf '(#71 #4096 (#36 [#4 07]))' | $(NODES) > "$$d/start.ck"; is "$$(./ck run "$$d/program.ck" "$$d/start.ck" "$$d/impure.ck" 2>&1)" 'node [#4 06]' 'the first file of a whole start changed'; \
	  is "$$(./ck run "$$d/program.ck" "$$d/start.ck" "$$d/impure.ck" 2>&1)" 'node [#4 06]' 'the whole start entered'; [ "$$(ls -i "$$I")" != "$$whole" ] || { echo "  a whole start saved from an entered part does not check that part's files"; fail=1; }; \
	  rm -f "$$I"/*.image; printf '(#89 [#2 76] (#45 (#39 (#115 #5 [#4 80f0fa02]) [#2 626967]) (#36 [#2 736d616c6c])))' | $(NODES) > "$$d/big.ck"; printf '(#90 [#2 76])' | $(NODES) > "$$d/v.ck"; \
	  is "$$(CK_MEMORY_BYTES=41943040 ./ck run "$$d/v.ck" "$$d/big.ck")" 'node [#2 736d616c6c]' 'a start that caught full at 40 MiB'; is "$$(ls "$$I" | wc -l | tr -d ' ')" 0 'a start that met the end of its allowance'; \
	  is "$$(CK_MEMORY_BYTES=62914560 ./ck run "$$d/v.ck" "$$d/big.ck")" "$$(CK_IMAGES= CK_MEMORY_BYTES=62914560 ./ck run "$$d/v.ck" "$$d/big.ck")" 'the same start at 60 MiB, as a cold start answers there'; is "$$(CK_MEMORY_BYTES=41943040 ./ck run "$$d/v.ck" "$$d/big.ck")" 'node [#2 736d616c6c]' 'its image offered at 40 MiB'; \
	  if [ "$$(uname -m)" = arm64 ]; then rm -f "$$I"/*.image; printf '(#89 [#2 64] (#45 (#51 [#52 fd7bbfa9ffffff97]) (#36 (#35 [#3]))))' | $(NODES) > "$$d/deep.ck"; printf '(#90 [#2 64])' | $(NODES) > "$$d/d.ck"; \
	    is "$$(./ck run "$$d/d.ck" "$$d/deep.ck")" 'node [#2 6372617368]' 'a start that caught code falling off the stack'; is "$$(ls "$$I" | wc -l | tr -d ' ')" 0 'a start that met the depth of its stack'; fi; \
	  rm -f "$$I"/*.image; for k in 1 2 3; do printf '(#71 #4096 (#36 [#4 0%s]))' $$k | $(NODES) > "$$d/e$$k.ck"; CK_IMAGES_BYTES=700000 ./ck run "$$d/program.ck" "$$d/e$$k.ck" > /dev/null; done; \
	  last=$$(ls -t "$$I"/*.image | head -1); kept=$$(ls -i "$$last"); is "$$(cat "$$I"/*.image | wc -c | awk '{print ($$1 <= 700000)}')" 1 'a directory of images over its cap after a save'; \
	  is "$$(CK_IMAGES_BYTES=700000 ./ck run "$$d/program.ck" "$$d/e3.ck")" 'node [#4 03]' 'the start whose image was saved last'; is "$$(ls -i "$$last")" "$$kept" 'the image saved last, kept and entered'; \
	  s="$$d/shared"; mkdir -p "$$s/sub.image"; dd if=/dev/zero of="$$s/Pharo.image" bs=16384 count=128 2>/dev/null; printf 'tiny!' > "$$s/tiny.image"; printf 'notes' > "$$s/notes.txt"; ln -s "$$s/notes.txt" "$$s/link.image"; \
	  mkdir -p "$$s/coherence-seed-images"; printf 'not an image' > "$$s/coherence-seed-images/0123456789abcdef0123456789abcdef.image"; ln -s "$$s/notes.txt" "$$s/coherence-seed-images/fedcba9876543210fedcba9876543210.image"; \
	  for k in 4 5; do printf '(#71 #4096 (#36 [#4 0%s]))' $$k | $(NODES) > "$$d/e$$k.ck"; CK_IMAGES="$$s" CK_IMAGES_BYTES=1 ./ck run "$$d/program.ck" "$$d/e$$k.ck" > /dev/null; done; \
	  for f in Pharo.image tiny.image notes.txt link.image sub.image coherence-seed-images/0123456789abcdef0123456789abcdef.image coherence-seed-images/fedcba9876543210fedcba9876543210.image; do [ -e "$$s/$$f" ] || [ -L "$$s/$$f" ] || { echo "  eviction removed $$f, which is not its image"; fail=1; }; done; \
	  [ "$$(cat "$$s/notes.txt")" = notes ] && [ "$$(ls "$$s/coherence-seed-images" | wc -l | tr -d ' ')" = 3 ] || { echo "  eviction under a shared folder: $$(ls "$$s/coherence-seed-images" | tr '\n' ' ')"; fail=1; }; \
	  mkdir -m 777 "$$d/open" "$$d/open/coherence-seed-images"; chmod 777 "$$d/open/coherence-seed-images"; is "$$(CK_IMAGES="$$d/open" ask)" "$$(CK_IMAGES= ask)" 'a folder others may write'; is "$$(ls "$$d/open/coherence-seed-images" | wc -l | tr -d ' ')" 0 'images kept in a folder others may write'; \
	  mkdir -m 777 "$$d/pub"; chmod 777 "$$d/pub"; is "$$(CK_IMAGES="$$d/pub" ask)" "$$(CK_IMAGES= ask)" 'a folder of images under one others may write'; is "$$(ls -A "$$d/pub" | wc -l | tr -d ' ')" 0 'images kept under a folder others may write'; \
	  shared=$$(ls -R /Library/Caches/coherence-seed 2>/dev/null | wc -l); is "$$(env -u CK_IMAGES HOME= ./ck run "$$d/program.ck" "$$d/start.ck")" "$$(CK_IMAGES= ask)" 'a start with no HOME'; is "$$(ls -R /Library/Caches/coherence-seed 2>/dev/null | wc -l)" "$$shared" 'images kept without a HOME'; \
	  printf '(#89 #103 (#34 (#89 [#2 61] [#4 01])))' | $(NODES) > "$$d/a.ck"; printf '(#89 #103 (#34 (#89 [#2 62] [#4 02])))' | $(NODES) > "$$d/b.ck"; printf '(#70 #16 (#43 (#40 (#90 [#2 61]) (#90 [#2 62]))))' | $(NODES) > "$$d/ab.ck"; \
	  is "$$(./ck run "$$d/ab.ck" "$$d/a.ck" "$$d/b.ck")" 'node (#16 [#4 01] [#4 02])' 'two files each leaving what runs after'; is "$$(./ck run "$$d/ab.ck" "$$d/a.ck" "$$d/b.ck")" 'node (#16 [#4 01] [#4 02])' 'the same start entered'; \
	  [ $$fail = 0 ] && echo "stale: every case answered as a cold start, and no image it must not enter was entered; a start saved at another allowance saved the same pages, and the executable built for x86-64, where it runs, was refused at start and saved none; a start entered the image of its part at rest and ran the rest; a directory over its cap after a save was under it, the image just saved kept, and nothing not its own touched"; \
	  [ $$fail = 0 ]

check-native: ck
	@set -e; if [ "$$(uname -s)" = Darwin ] && ./ck text ground/paths.ck ground/for-host.ck | grep -Fxq ground/native-add.ck; then \
	  home=$$(mktemp -d); binary="$$home/ck"; trap 'rm -rf "$$home"' EXIT; mkdir "$$home/form"; cp form/print.ck "$$home/form/"; mkdir "$$home/ice"; cp ice/seed.ck "$$home/ice/"; cp ck "$$binary"; chmod +x "$$binary"; \
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
	$(verify_instrument)
	$(verify_forecast)
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
	@got=$$(./ck run ground/held-back.ck | tr '\n' ' '); floor=$$(./ck run ground/held-back.ck $(FLOOR) | tr '\n' ' '); \
	  if [ "$$floor" = "$$got" ] && [ "$$got" = "held node (#16 [#4 04] [#2 626f6f6d] [#2 697272657665727369626c65] [#1 340a] (#16 [#4 64] [#4]) (#16 [#2 67] [#4 08] (#16 [#2 66] [#4 01] [#4 02] [#4 03] ^7))) " ]; \
	  then echo "held back: of five tries in a search only the one that held wrote its line; a faulted path wrote nothing; a hundred held calls ran in the order held, and a fault past a closed choice kept what was held outside it, restored by the seed's leaf and by the leaf $(FLOOR) compiles in the run alike"; \
	  else echo "held back: '$$got', after $(FLOOR): '$$floor'"; exit 1; fi

	@test "$$(./ck run ground/circle-turns.ck)" = 'node #33'
	$(verify_exchange)

	$(verify_signed_controls)

	$(verify_derivative)

	$(verify_stewardship)

	$(verify_resume)

	$(verify_dispatch)

	$(verify_projection)

	$(verify_reconnection)

	$(verify_render)
	$(verify_translation_delivery)
	$(verify_participant_account)

	@test "$$(./ck run ground/translation-reference.ck)" = 'node #33'

	$(verify_key_recovery)

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
	  ./ck text lens/show-canonical.ck lens/all.ck < "$$work/observed"

check-bare: ck
	@n=$$(./ck text ground/reads-by-number.ck | head -1); \
	  if [ "$${n:-99999}" -le $(BARE) ]; then echo "parts by name: $$n reads by a bare number are left in the stored form, within the budget of $(BARE)"; \
	  else echo "parts by name: $$n reads by a bare number, over the budget of $(BARE); name the parts"; exit 1; fi

clean:
	rm -f ck fb kg seed-run .ck-header .ck-check.ice .ck-check.rec .ck-watch.rec .ck-hardened
	rm -rf .ck-images

.PHONY: all views metal-image check test-inventory $(CHECK_FLOWS) $(FULL_ONLY) check-full ground-isolated ground-together watch clean header

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
	  printf '%s' "$$code" | $(CANONICAL_WORDS) >"$$work/code"; printf '%s' "$$status" | $(CANONICAL_WORDS) >"$$work/status"; ./ck text translation/respond.ck 3<"$$work/request" 4<"$$work/status" 5<"$$work/code" 6<"$$work/diagnostic" <"$$work/answer" >"$$work/reply"; \
	  ./ck text translation/review.ck 3<"$$work/request" <"$$work/reply"
