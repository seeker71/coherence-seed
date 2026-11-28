define verify_stewardship
@set -eu; \
  stewardship_root=$$(mktemp -d); \
  trap 'rm -rf "$$stewardship_root"' EXIT; \
  for scene in normal before collected; do work="$$stewardship_root/$$scene"; mkdir "$$work"; \
  mkdir "$$work/private" "$$work/letters"; \
  printf '#32' > "$$work/none"; \
  cp "$$work/none" "$$work/private/authority"; \
  cp "$$work/none" "$$work/private/listening"; \
  printf %s "$$work" > "$$work/location"; \
  word() { printf %s "$$1" | $(CANONICAL_WORDS); }; \
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
endef
