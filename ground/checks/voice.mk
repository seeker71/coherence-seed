define verify_render
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
endef
