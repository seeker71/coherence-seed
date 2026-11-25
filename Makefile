# The bootstrap's build. The line budget is graph node 5.9.2; raising it is a change to that node.
# Nothing here needs more than a C compiler: no part of the old kernel builds or runs this repository.
BUDGET = 2000
EXPECT = ground/expect.tsv
CC ?= cc
CFLAGS = -std=c99 -O2 -Wall -Wextra -Werror
BOOT = bootstrap/ck.c bootstrap/fb.h
LONG = ground/long-recursion.ck

VARY = 200000

all: ck fb second

ck: $(BOOT)
	@n=$$(cat $(BOOT) | wc -l); if [ $$n -gt $(BUDGET) ]; then \
	  echo "the bootstrap has $$n lines; its budget is $(BUDGET)"; exit 1; fi
	$(CC) $(CFLAGS) -o ck bootstrap/ck.c

# The observer: a second process that shows, records and replays a framebuffer.
fb: tools/fb.c bootstrap/fb.h
	$(CC) $(CFLAGS) -o fb tools/fb.c

# The second hand: a reader of the node encoding written from the page, sharing no line with ck.c.
second: tools/second.c
	$(CC) $(CFLAGS) -o second tools/second.c

# Two hands, watched: the second reader writes $(VARY) variations of the vectors with its verdict
# on each; the bootstrap reads them while a second process records its memory every millisecond.
hands: ck fb second
	@./second vary spec/node-vectors.tsv $(VARY) 1 > .ck-vary.tsv
	@(CK_FB=/ck.hands ./ck vectors .ck-vary.tsv | tail -1 &) ; ./fb record /ck.hands .ck-hands.rec 1 > /dev/null
	@./fb play .ck-hands.rec; ./second vectors .ck-vary.tsv | tail -1
	@./fb drop /ck.hands; rm -f .ck-hands.rec .ck-vary.tsv

# One real run, watched live: the long recursion runs while a second process records its
# memory every 5 ms; then the recording is replayed and the answer is asked who wrote it.
watch: ck fb
	@CK_FB=/ck.watch ./ck run $(LONG) & ./fb record /ck.watch .ck-watch.rec 5; wait
	@./fb play .ck-watch.rec | head -3; ./fb play .ck-watch.rec | tail -1
	@./fb play .ck-watch.rec -1 | grep '^outcome'
	@./fb drop /ck.watch; rm -f .ck-watch.rec

# What the floor must show: the encoding's vectors, the ground programs, freeze and thaw in two
# processes, native code unsigned and under the hardened runtime, a live run recorded from a
# second process whose last frame holds the answer and names the node that wrote it, and the two
# hands: the vectors in the second reader, every ground program frozen by either hand and thawed
# by the other, and variations of the vectors that hold in both while the observer counts in the
# bootstrap's region the nodes the second reader holds.
check: ck fb second
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
	./second vectors spec/node-vectors.tsv
	@n=0; while IFS='	' read -r name want; do p=ground/$$name.ck; n=$$((n+1)); \
	  a=$$(./second freeze $$p .ck-second.ice) && b=$$(./ck thaw .ck-second.ice) && \
	  c=$$(./ck freeze $$p .ck-first.ice) && d=$$(./second thaw .ck-first.ice) && \
	  both=$$(printf '%s\n%s' "$$a" "$$(cat $$p)") && [ "$$a" = "$$c" ] && [ "$$b" = "$$both" ] && [ "$$d" = "$$both" ] && \
	  cmp -s .ck-second.ice .ck-first.ice || { echo "two hands: $$p frozen by one does not thaw the same in the other"; exit 1; }; \
	done < $(EXPECT); rm -f .ck-second.ice .ck-first.ice; \
	./second vary spec/node-vectors.tsv $(VARY) 1 > .ck-vary.tsv; lines=$$(grep -c . .ck-vary.tsv); \
	first=$$(CK_FB=/ck.hands ./ck vectors .ck-vary.tsv | tail -1); second=$$(./second vectors .ck-vary.tsv | tail -1); \
	seen=$$(./fb show /ck.hands | sed -n 's/^frame .* nodes \([0-9]*\) .*/\1/p'); ./fb drop /ck.hands; rm -f .ck-vary.tsv; \
	held=$${second##*; }; held=$${held%% *}; \
	if [ "$$first" = "ck: $$lines vectors hold, 0 do not" ] && [ "$${second%%;*}" = "second: $$lines vectors hold, 0 do not" ] && \
	   [ $$lines -ge 100000 ] && [ "$$seen" = "$$((held + 65))" ]; then \
	  echo "two hands: $$n programs frozen by either thaw in the other; $$lines varied lines hold in both; the observer counts the second reader's $$held nodes in the bootstrap's region"; \
	else echo "two hands: of $$lines variations, '$$first' and '$$second'; the observer counts $$seen node records"; exit 1; fi

clean:
	rm -f ck fb second kg .ck-check.ice .ck-check.rec .ck-watch.rec .ck-hardened .ck-second.ice .ck-first.ice .ck-vary.tsv .ck-hands.rec

.PHONY: all check watch clean
