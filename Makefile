# The bootstrap's build. The line budget is graph node 5.9.2; raising it is a change to that node.
# Nothing here needs more than a C compiler: no part of the old kernel builds or runs this repository.
BUDGET = 2000
# How many times the stored form may still read a part of a node by a bare number, where no shape
# names the part (graph node 7.22). It is lowered as code is written properly, and never raised.
BARE = 270
CC ?= cc
CFLAGS = -std=c99 -O2 -Wall -Wextra -Werror
BOOT = bootstrap/ck.c bootstrap/fb.h
LONG = ground/long-recursion.ck

all: ck fb kg

ck: $(BOOT)
	@n=$$(cat $(BOOT) | wc -l); if [ $$n -gt $(BUDGET) ]; then \
	  echo "the bootstrap has $$n lines; its budget is $(BUDGET)"; exit 1; fi
	$(CC) $(CFLAGS) -o ck bootstrap/ck.c

# The observer: a second process that shows, records and replays a framebuffer.
fb: tools/fb.c bootstrap/fb.h
	$(CC) $(CFLAGS) -o fb tools/fb.c

# The claim graph's checker.
kg: tools/kg.c
	$(CC) $(CFLAGS) -o kg tools/kg.c

# bootstrap/fb.h is made from nodes: the catalogue's words and the layout of the region's records.
header: ck
	./ck text region/header.ck > .ck-header && mv .ck-header bootstrap/fb.h

# One real run, watched live: the long recursion runs while a second process records its
# memory every 5 ms; then the recording is replayed and the answer is asked who wrote it.
watch: ck fb
	@CK_FB=/ck.watch ./ck run $(LONG) & ./fb record /ck.watch .ck-watch.rec 5; wait
	@./fb play .ck-watch.rec | head -3; ./fb play .ck-watch.rec | tail -1
	@./fb play .ck-watch.rec -1 | grep '^outcome'
	@./fb drop /ck.watch; rm -f .ck-watch.rec

# What the floor must show: the encoding's vectors, the ground programs, freeze and thaw in two
# processes, native code unsigned and under the hardened runtime, a live run recorded from a
# second process whose last frame holds the answer and names the node that wrote it, and a run
# asked from a second process to count its own additions while it goes on, and a search that
# writes a file only for the path that held.
check: ck fb
	@./ck text region/header.ck | cmp -s - bootstrap/fb.h && echo "header: bootstrap/fb.h is what the nodes generate" || \
	  { echo "header: bootstrap/fb.h is not what region/header.ck writes; run make header"; exit 1; }
	@got=$$(./ck run ground/check.ck); \
	  if [ "$$got" = "node (#16)" ]; then echo "ground: every program ends as expected, checked by a program in form"; \
	  else echo "ground: path and outcome of each program that ends otherwise: $$got"; exit 1; fi
	@./ck text ice/freeze.ck lens/all.ck ice/ice.ck < ground/recursion.ck > .ck-check.ice && \
	  b=$$(./ck run ice/thaw.ck lens/all.ck ice/ice.ck < .ck-check.ice); rm -f .ck-check.ice; \
	  if [ "$$b" = "node $$(head -1 ground/recursion.ck)" ]; then echo "ice: frozen by a program in form, thawed by another in a second process to the same node"; \
	  else echo "ice: thawed '$$b'"; exit 1; fi
	@if [ "$$(uname -sm)" = "Darwin arm64" ]; then \
	  cp ck .ck-hardened && codesign -f -s - --options runtime --entitlements bootstrap/jit.entitlements .ck-hardened 2>/dev/null && \
	  got=$$(./.ck-hardened run ground/native-add.ck); rm -f .ck-hardened; \
	  if [ "$$got" = "node [#4 2a]" ]; then echo "native: a code leaf runs unsigned and under the hardened runtime"; \
	  else echo "native under the hardened runtime: got '$$got'"; exit 1; fi; \
	else echo "native: not Apple silicon; the hardened-runtime witness did not run"; fi
	@(CK_FB=/ck.check ./ck run $(LONG) > /dev/null &) ; ./fb record /ck.check .ck-check.rec 5 > /dev/null; \
	  frames=$$(./fb play .ck-check.rec | grep -c moving); last=$$(./fb play .ck-check.rec -1 | grep '^outcome'); \
	  ./fb drop /ck.check; rm -f .ck-check.rec; \
	  case "$$last" in "outcome  node 2178309 "*"written by (add (call (self)"*) ok=1;; *) ok=0;; esac; \
	  if [ $$ok = 1 ] && [ $$frames -ge 10 ]; then echo "framebuffer: a second process recorded $$frames frames of a run in motion; the last holds the answer and its writer"; \
	  else echo "framebuffer: $$frames frames in motion; last frame says '$$last'"; exit 1; fi
	@(CK_FB=/ck.ask ./ck run $(LONG) > /dev/null &) ; sleep 0.2; \
	  ask() { printf '/ck.ask\n%s' "$$1" | ./ck text region/ask.ck | tr -d '()\n'; }; \
	  on=$$(ask '(bind add (lazy (seq (note #1002 (add 1 (if (held #1002) (held #1002) 0))) (base $$0))))'); \
	  one=$$(ask '(held #1002)'); two=$$(ask '(held #1002)'); off=$$(ask '(unbind add)'); three=$$(ask '(held #1002)'); four=$$(ask '(held #1002)'); \
	  for i in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20; do ./fb show /ck.ask | grep -q 'mode done' && break; sleep 0.5; done; \
	  last=$$(./fb show /ck.ask | grep '^outcome'); ./fb drop /ck.ask; n1=$${one##* }; n2=$${two##* }; \
	  case "$$last" in "outcome  node 2178309 "*) ok=1;; *) ok=0;; esac; \
	  if [ $$ok = 1 ] && [ "$$on" = 'list "node" yes' ] && [ "$$off" = "$$on" ] && [ "$${n1:-0}" -gt 0 ] && [ "$${n2:-0}" -gt "$$n1" ] && [ "$$three" = "$$four" ]; \
	  then echo "asked: a second process put a counter on add in a running program, read $$n1 and then $$n2, took it off, and the program answered as before"; \
	  else echo "asked: on '$$on', counts '$$one' '$$two', off '$$off', after '$$three' '$$four', the run's '$$last'"; exit 1; fi
	@got=$$(./ck run ground/held-back.ck | tr '\n' ' '); \
	  if [ "$$got" = "held node (#16 [#4 04] [#2 626f6f6d] [#2 697272657665727369626c65] [#1 340a]) " ]; \
	  then echo "held back: of five tries in a search only the one that held wrote its line; a faulted path wrote nothing"; \
	  else echo "held back: '$$got'"; exit 1; fi
	@n=$$(./ck text ground/reads-by-number.ck | head -1); \
	  if [ "$${n:-99999}" -le $(BARE) ]; then echo "parts by name: $$n reads by a bare number are left in the stored form, within the budget of $(BARE)"; \
	  else echo "parts by name: $$n reads by a bare number, over the budget of $(BARE); name the parts"; exit 1; fi

clean:
	rm -f ck fb kg .ck-header .ck-check.ice .ck-check.rec .ck-watch.rec .ck-hardened

.PHONY: all check watch clean header
