define verify_translation_delivery
@set -eu; \
  work=$$(mktemp -d); trap 'rm -rf "$$work"' EXIT; mkdir "$$work/private" "$$work/letters"; \
  word() { printf %s "$$1" | $(CANONICAL_WORDS); }; \
  word no >"$$work/none"; cp "$$work/none" "$$work/private/authority"; cp "$$work/none" "$$work/private/listening"; \
  g() { printf %s "$$1" >"$$work/mode"; ./ck text ground/circle-exchange.ck 3<"$$work/mode" 4<"$$work/$${4:-none}" <"$$work/$$2" >"$$work/$$3"; }; \
  change() { g change "$$2" update "$$3"; ./ck text "circle/$$1.ck" 3<"$$work/channel" <"$$work/update" >"$$work/$$4"; }; \
  word '"public translation reviewer"' >"$$work/actor"; word '"translation turn"' >"$$work/nonce"; \
  word "(inbox.channel \"public translation reviewer\" \"local review\" \"invited reader\" \"$$work/private\" \"$$work/letters\")" >"$$work/channel"; \
  ./ck text ground/translation-episode.ck >"$$work/study"; \
  test "$$(./ck text ground/translation-history.ck 3<"$$work/channel" <"$$work/study")" = "$$(word yes)"; \
  amend() { printf %s "$$1" >"$$work/action"; ./ck text translation/choice.ck 3<"$$work/$$2" 4<"$$work/action" <"$$work/$$4" >"$$work/choice"; ./ck text translation/amend.ck 3<"$$work/study" 4<"$$work/$$3" 5<"$$work/actor" <"$$work/choice" >"$$work/$$5"; }; \
  draft() { ./ck text translation/scope.ck 3<"$$work/channel" 4<"$$work/$$1" <"$$work/$$2"; }; \
  refused() { expected=$$1; shift; rc=0; "$$@" >"$$work/refusal" || rc=$$?; test "$$rc" = 1; test "$$(cat "$$work/refusal")" = "fault $$(word "$$expected")"; }; \
  receive() { ./ck text circle/receive.ck 3<"$$work/channel" <"$$work/$$1" >"$$work/receipt"; g status receipt status; test "$$(cat "$$work/status")" = "$$(word "$$2")"; }; \
  g identity none genesis; word 0 >"$$work/zero"; amend attest study none zero accepted; draft accepted accepted >"$$work/model-scope"; \
  g permit model-scope old-permit nonce; g ticket model-scope old-ticket nonce; change authorize old-permit genesis issued; \
  printf '%s' 'Kami boleh mendengarkan.' >"$$work/wording"; amend correct accepted accepted wording correction; refused lingua.stale draft correction accepted; \
  change authorize none issued withdrawn; change listen old-ticket genesis invited; receive old-ticket share.ungranted; \
  draft correction correction >"$$work/new-scope"; g permit new-scope new-permit nonce; g ticket new-scope new-ticket nonce; \
  test "$$(cat "$$work/old-ticket")" != "$$(cat "$$work/new-ticket")"; change authorize new-permit withdrawn granted; receive new-ticket inbox.paused; \
  set -- "$$work/letters/"*; test ! -e "$$1"; change listen new-ticket invited agreed; receive new-ticket inbox.accepted; receive new-ticket inbox.replayed; \
  set -- "$$work/letters/"*; test "$$#" = 1; cp "$$1" "$$work/published"; g bytes published received; cmp "$$work/wording" "$$work/received"; \
  amend decline correction correction none declined; refused share.ungranted draft declined declined; change authorize none granted stopped; receive new-ticket share.ungranted; \
  echo "translation: stale queued reading refused, original grant withdrawn, exact corrected wording reached only the newly invited recipient"
endef
