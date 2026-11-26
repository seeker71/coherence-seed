# The bootstrap's build. The line budget is graph node 5.9.2; raising it is a change to that node.
# Nothing here needs more than a C compiler: no part of the old kernel builds or runs this repository.
BUDGET = 2000
# How many times the stored nodes may still read a part of a node by a bare number (graph node
# 7.22). The count is of the nodes, whatever names lie beside them. It is lowered, never raised.
BARE = 475
CC ?= cc
CFLAGS = -std=c99 -O2 -Wall -Wextra -Werror
BOOT = bootstrap/ck.c bootstrap/fb.h
LONG = ground/long-recursion.ck

all: ck fb kg seed-run

ck: $(BOOT)
	@n=$$(cat $(BOOT) | wc -l); if [ $$n -gt $(BUDGET) ]; then \
	  echo "the bootstrap has $$n lines; its budget is $(BUDGET)"; exit 1; fi
	$(CC) $(CFLAGS) -o ck bootstrap/ck.c

seed-run: ck $(BOOT)
	$(CC) $(CFLAGS) -DNUMERIC_PORT -o $@ bootstrap/ck.c

# The observer: a second process that shows, records and replays a framebuffer.
fb: tools/fb.c bootstrap/fb.h
	$(CC) $(CFLAGS) -o fb tools/fb.c

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
ifneq ($(word 4,$(CHECK_FLOWS)),)
$(error make check has room for three flows)
endif
check: $(CHECK_FLOWS)

check-bml: ck
	@got=$$(printf 'nums/bml.ck\n10 - 3 - 2\n' | ./ck text nums/read.ck | ./ck run /dev/stdin nums/run.ck) && \
	  [ "$$got" = 'node [#4 09]' ] || { echo "BML expression returned '$$got', expected 9"; exit 1; }; \
	  echo "BML: the source expression 10 - 3 - 2 ran as numeric nodes and returned 9"

check-image: seed-run
	@set -e; root=$$PWD; work=$$(mktemp -d); trap 'rm -rf "$$work"' EXIT; mkdir "$$work/run"; \
	  ./ck text nums/pack.ck < ground/nums-image.ck > "$$work/run/image"; \
	  ./ck text ground/nums-requests.ck > "$$work/requests"; test -s "$$work/requests"; \
	  if [ "$$(uname -sm)" = "Darwin arm64" ]; then \
	    ./ck text ground/nums-native-requests.ck > "$$work/native"; test -s "$$work/native"; \
	    cat "$$work/native" >> "$$work/requests"; fi; \
	  (cd "$$work/run" && "$$root/seed-run" image) < "$$work/requests" > "$$work/replies"; \
	  got=$$(./ck run ground/nums-replies.ck < "$$work/replies"); \
	  [ "$$got" = 'node #33' ] || { echo "image replies differed: $$got"; exit 1; }; \
	  echo "image: the standalone runtime answered by content id from a directory containing only the image"

check-live: ck fb
	@name=/ck.ask.$$$$; (CK_FB=$$name ./ck run $(LONG) > /dev/null &) ; $(up); \
	  ask() { printf '%s\n%s' "$$name" "$$1" | ./ck text region/ask.ck | tr -d '()\n'; }; \
	  on=$$(ask '(bind add (lazy (seq (note #1002 (add 1 (if (held #1002) (held #1002) 0))) (base $$0))))'); \
	  one=$$(ask '(held #1002)'); two=$$(ask '(held #1002)'); off=$$(ask '(unbind add)'); three=$$(ask '(held #1002)'); four=$$(ask '(held #1002)'); \
	  n=0; until ./fb show $$name | grep -q 'mode done'; do n=$$((n+1)); [ $$n -gt 1200 ] && break; sleep 0.05; done; \
	  last=$$(./fb show $$name | grep '^outcome'); ./fb drop $$name; n1=$${one##* }; n2=$${two##* }; \
	  case "$$last" in "outcome  node 2178309 "*) ok=1;; *) ok=0;; esac; \
	  if [ $$ok = 1 ] && [ "$$on" = 'list "node" yes' ] && [ "$$off" = "$$on" ] && [ "$${n1:-0}" -gt 0 ] && [ "$${n2:-0}" -gt "$$n1" ] && [ "$$three" = "$$four" ]; \
	  then echo "live: a second process installed an addition counter, read $$n1 and $$n2, removed it, and observed the final answer 2178309"; \
	  else echo "live: on '$$on', counts '$$one' '$$two', off '$$off', after '$$three' '$$four', final '$$last'"; exit 1; fi

FAILED = .ck-failed
check-full: check
	@rm -f $(FAILED)
	@./ck text region/header.ck | cmp -s - bootstrap/fb.h && echo "header: bootstrap/fb.h is what the nodes generate" || \
	  { echo "header: bootstrap/fb.h is not what region/header.ck writes; run make header"; echo header >> $(FAILED); }
	@./ck text ground/paths.ck | xargs -n 1 -P 8 sh -c 'got=$$(printf %s "$$0" | ./ck run ground/check-one.ck 2>&1); [ "$$got" = "node #33" ] || echo "  $$0: $$got"' > .ck-ground; \
	  n=$$(./ck text ground/paths.ck | wc -l | tr -d ' '); \
	  if [ -s .ck-ground ]; then echo "ground: of $$n programs, each in a process of its own, these end otherwise:"; cat .ck-ground; echo ground >> $(FAILED); \
	  else echo "ground: each of $$n programs, in a process of its own, ends as expected, checked by a program in form"; fi; rm -f .ck-ground
	@got=$$(./ck run ground/check.ck 2>&1); \
	  if [ "$$got" = "node (#16)" ]; then echo "one run: all of them in one process end as expected, since a fail gives back what it returns past"; \
	  else echo "one run: path and outcome of each program that ends otherwise in one process: $$got"; echo one-run >> $(FAILED); fi
	@./ck text ice/freeze.ck lens/all.ck ice/ice.ck < ground/recursion.ck > .ck-check.ice && \
	  b=$$(./ck run ice/thaw.ck lens/all.ck ice/ice.ck < .ck-check.ice); rm -f .ck-check.ice; \
	  if [ "$$b" = "node $$(head -1 ground/recursion.ck)" ]; then echo "ice: frozen by a program in form, thawed by another in a second process to the same node"; \
	  else echo "ice: thawed '$$b'"; echo ice >> $(FAILED); fi
	@if [ "$$(uname -sm)" = "Darwin arm64" ]; then \
	  cp ck .ck-hardened && codesign -f -s - --options runtime --entitlements bootstrap/jit.entitlements .ck-hardened 2>/dev/null && \
	  got=$$(./.ck-hardened run ground/native-add.ck); rm -f .ck-hardened; \
	  if [ "$$got" = "node [#4 2a]" ]; then echo "native: a code leaf runs unsigned and under the hardened runtime"; \
	  else echo "native under the hardened runtime: got '$$got'"; echo native >> $(FAILED); fi; \
	else echo "native: not Apple silicon; the hardened-runtime witness did not run"; fi
	@name=/ck.check.$$$$; (CK_FB=$$name ./ck run $(LONG) > /dev/null &) ; $(up); ./fb record $$name .ck-check.rec 5 > /dev/null; \
	  frames=$$(./fb play .ck-check.rec | grep -c moving); last=$$(./fb play .ck-check.rec -1 | grep '^outcome'); \
	  ./fb drop $$name; rm -f .ck-check.rec; \
	  case "$$last" in "outcome  node 2178309 "*"written by (add (call (self)"*) ok=1;; *) ok=0;; esac; \
	  if [ $$ok = 1 ] && [ $$frames -ge 2 ]; then echo "framebuffer: a second process recorded $$frames frames of a run in motion; the last holds the answer and its writer"; \
	  else echo "framebuffer: $$frames frames in motion; last frame says '$$last'"; echo framebuffer >> $(FAILED); fi
	@got=$$(./ck run ground/held-back.ck | tr '\n' ' '); \
	  if [ "$$got" = "held node (#16 [#4 04] [#2 626f6f6d] [#2 697272657665727369626c65] [#1 340a]) " ]; \
	  then echo "held back: of five tries in a search only the one that held wrote its line; a faulted path wrote nothing"; \
	  else echo "held back: '$$got'"; echo held-back >> $(FAILED); fi
	@n=$$(./ck text ground/reads-by-number.ck | head -1); \
	  if [ "$${n:-99999}" -le $(BARE) ]; then echo "parts by name: $$n reads by a bare number are left in the stored form, within the budget of $(BARE)"; \
	  else echo "parts by name: $$n reads by a bare number, over the budget of $(BARE); name the parts"; echo parts-by-name >> $(FAILED); fi
	@if [ -s $(FAILED) ]; then echo "failed: $$(tr '\n' ' ' < $(FAILED))"; rm -f $(FAILED); exit 1; fi

clean:
	rm -f ck fb kg seed-run .ck-header .ck-check.ice .ck-check.rec .ck-watch.rec .ck-hardened .ck-failed .ck-ground

.PHONY: all check $(CHECK_FLOWS) check-full watch clean header
