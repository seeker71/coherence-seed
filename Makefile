# The bootstrap's build. The line budget is graph node 5.9.2; raising it is a change to that node.
# Nothing here needs more than a C compiler: no part of the old kernel builds or runs this repository.
BUDGET = 2000
EXPECT = ground/expect.tsv
CC ?= cc
CFLAGS = -std=c99 -O2 -Wall -Wextra -Werror
BOOT = bootstrap/ck.c bootstrap/fb.h
LONG = ground/long-recursion.ck

all: ck fb

ck: $(BOOT)
	@n=$$(cat $(BOOT) | wc -l); if [ $$n -gt $(BUDGET) ]; then \
	  echo "the bootstrap has $$n lines; its budget is $(BUDGET)"; exit 1; fi
	$(CC) $(CFLAGS) -o ck bootstrap/ck.c

# The observer: a second process that shows, records and replays a framebuffer.
fb: tools/fb.c bootstrap/fb.h
	$(CC) $(CFLAGS) -o fb tools/fb.c

# One real run, watched live: the long recursion runs while a second process records its
# memory every 5 ms; then the recording is replayed and the answer is asked who wrote it.
watch: ck fb
	@CK_FB=/ck.watch ./ck run $(LONG) & ./fb record /ck.watch .ck-watch.rec 5; wait
	@./fb play .ck-watch.rec | head -3; ./fb play .ck-watch.rec | tail -1
	@./fb play .ck-watch.rec -1 | grep '^outcome'
	@./fb drop /ck.watch; rm -f .ck-watch.rec

# What the floor must show: the encoding's vectors, the ground programs, freeze and thaw in two
# processes, native code unsigned and under the hardened runtime, a live run recorded from a
# second process whose last frame holds the answer and names the node that wrote it, and a
# program kept as nodes that takes leaves apart and builds new ones, read back from its region.
check: ck fb
	./ck vectors spec/node-vectors.tsv
	@bad=0; while IFS='	' read -r name want; do \
	  got=$$(./ck run ground/$$name.ck); \
	  if [ "$$got" != "$$want" ]; then echo "ground/$$name.ck: got '$$got', want '$$want'"; bad=1; fi; \
	done < $(EXPECT); [ $$bad = 0 ] && echo "ground: every program answers as expected"
	@a=$$(./ck freeze ground/recursion.ck .ck-check.ice) && b=$$(./ck thaw .ck-check.ice) && rm -f .ck-check.ice && \
	  if [ "$$b" = "$$(printf '%s\n%s' "$$a" "$$(cat ground/recursion.ck)")" ]; then echo "ice: thawed in a second process to the same id and print"; \
	  else echo "ice: froze $$a, thawed $$b"; exit 1; fi
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
	@CK_FB=/ck.parts ./ck run ground/nat-rule.ck > /dev/null; last=$$(./fb show /ck.parts | grep '^outcome'); \
	  h=$$(echo "$$last" | sed 's/.*(node \([0-9]*\), written by.*/\1/'); \
	  built=$$(./fb show /ck.parts $$((h - 1)) | tail -2 | tr -s ' \n' ' '); ./fb drop /ck.parts; \
	  case "$$last|$$built" in 'outcome  node (list 0n 42n "minimal" 256n "minimal") '*'written by (all (call (recipe'*'| leaf, 2 bytes written by (pack nat (all '*) \
	    echo "parts: a program kept as nodes reads bytes into nat leaves; the region holds each leaf it built and the pack node that wrote it";; \
	  *) echo "parts: the region says '$$last' and '$$built'"; exit 1;; esac

clean:
	rm -f ck fb kg .ck-check.ice .ck-check.rec .ck-watch.rec .ck-hardened

.PHONY: all check watch clean
