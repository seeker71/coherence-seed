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

include participant/commands.mk participant/enrollment.mk
include ground/checks/circle.mk ground/checks/components.mk ground/checks/continuation.mk \
        ground/checks/learning.mk ground/checks/mesh.mk ground/checks/participant.mk \
        ground/checks/provider.mk ground/checks/recovery.mk ground/checks/translation.mk \
        ground/checks/value.mk ground/checks/voice.mk ground/checks/world.mk

check-ice: ck
	@set -eu; work=$$(mktemp -d); trap 'rm -rf "$$work"' EXIT; \
	  word() { printf %s "$$1" | ./ck text lens/to-nodes.ck lens/all.ck; }; \
	  word '(trial.template "Read: " ".")' >"$$work/template"; word '"B"' >"$$work/other"; \
	  word '(trial.scene "room" "A" (list (trial.statement "A" "Both statements") (trial.statement "B" "Both statements")))' >"$$work/scene"; \
	  rc=0; ./ck text learning/pair.ck 3<"$$work/template" 4<"$$work/other" <"$$work/scene" >"$$work/out" || rc=$$?; test "$$rc" = 1; test "$$(cat "$$work/out")" = "fault $$(word trial.indistinguishable)"
	$(verify_owned_model)
	$(verify_fit_replay)
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
	@got=$$(./ck run ground/held-back.ck | tr '\n' ' '); \
	  if [ "$$got" = "held node (#16 [#4 04] [#2 626f6f6d] [#2 697272657665727369626c65] [#1 340a] (#16 [#4 64] [#4]) (#16 [#2 67] [#4 08] (#16 [#2 66] [#4 01] [#4 02] [#4 03] ^7))) " ]; \
	  then echo "held back: branch, fault and ordering checks passed. Scoped delivery matched the current grant, selected reading, receipt and bytes in the receiving stream."; \
	  else echo "held back: '$$got'"; exit 1; fi

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
