define verify_dispatch
@set -eu; \
  area=$$(mktemp -d); \
  trap 'rm -rf "$$area"' EXIT; \
  printf '#32' > "$$area/none"; \
  pc() { printf %s "$$1" > "$$area/mode"; ./ck text ground/provider-case.ck 3<"$$area/mode" 4<"$${4:-$$area/none}" <"$$2" >"$$3"; }; \
  word() { printf %s "$$1" | $(CANONICAL_WORDS); }; \
  testword() { word "$$2" >"$$area/want"; [ "$$(cat "$$1")" = "$$(cat "$$area/want")" ]; }; \
  ./ck text learning/train.ck <ground/language-corpus.ck > "$$area/model"; \
  pc job "$$area/model" "$$area/job"; pc policy "$$area/none" "$$area/local-policy"; \
  pc changed-source "$$area/job" "$$area/other"; \
  pc roster "$$area/none" "$$area/roster"; pc alt-roster "$$area/none" "$$area/alt-roster"; \
  ./ck text circle/pin.ck <"$$area/roster" >"$$area/pin"; \
  ./ck text circle/pin.ck <"$$area/alt-roster" >"$$area/alt-pin"; \
  for role in owner listener withdraw alt-owner alt-listener; do pc "$$role-proof" "$$area/none" "$$area/$$role-proof"; done; \
  setup() { \
   dir="$$area/$$1"; mkdir -p "$$dir/private" "$$dir/letters"; \
   prefix=$${2:-}; cp "$$area/$${prefix}roster" "$$dir/private/peers"; \
   cp "$$area/$${prefix}pin" "$$dir/private/listening"; cp "$$area/$${prefix}pin" "$$dir/private/authority"; \
   word "\"$$dir\"" > "$$dir/path"; pc channel "$$dir/path" "$$dir/channel"; \
  }; \
  apply_role() { ./ck text circle/apply.ck 3<"$$dir/channel" 4<"$$area/$${prefix}$$1-change" <"$$area/$${prefix}$$1-proof" >"$$area/revision"; }; \
  job_run() { ./ck text provider/dispatch.ck 3<"$$dir/channel" 4<"$$1" 7<"$$2" < /dev/null > "$$3"; }; \
  check_route() { pc decision "$$1" "$$area/decision"; pc selected "$$area/decision" "$$area/kind"; testword "$$area/kind" "$$2"; pc cause "$$area/decision" "$$area/cause"; testword "$$area/cause" "$$3"; verify_trace; }; \
  verify_trace() { for record in "$$dir/private"/route-*; do if cmp -s "$$record" "$$area/decision"; then return 0; fi; done; return 1; }; \
  empty() { test -z "$$(ls -A "$$dir/letters")"; }; \
  setup refused; \
  pc revision "$$area/none" "$$area/genesis"; pc permit "$$area/job" "$$area/permit" "$$dir/channel"; \
  pc ticket "$$area/job" "$$area/ticket" "$$dir/channel"; \
  pc change "$$area/permit" "$$area/owner-change" "$$area/genesis"; \
  pc change "$$area/ticket" "$$area/listener-change" "$$area/genesis"; \
  apply_role owner; \
  pc change "$$area/none" "$$area/withdraw-change" "$$area/revision"; \
  job_run "$$area/none" "$$area/job" "$$area/no-listener"; check_route "$$area/no-listener" provider.unserved inbox.paused; empty; \
  apply_role listener; \
  ./ck text provider/offer.ck 3<"$$dir/channel" <"$$area/job" >"$$area/prepared"; \
  apply_role withdraw; \
  job_run "$$area/none" "$$area/job" "$$area/cancelled"; check_route "$$area/cancelled" provider.unserved share.ungranted; empty; \
  setup active; \
  apply_role owner; apply_role listener; \
  job_run "$$area/none" "$$area/other" "$$area/wrong"; check_route "$$area/wrong" provider.unserved share.different; empty; \
  job_run "$$area/none" "$$area/job" "$$area/sent"; check_route "$$area/sent" provider.external inbox.accepted; \
  cp "$$area/decision" "$$area/dispatch"; set -- "$$dir/letters/"*; test "$$#" = 1; cp "$$1" "$$area/letter"; \
  pc bytes-exact "$$area/letter" "$$area/exact" "$$area/job"; testword "$$area/exact" yes; \
  ./ck text provider/answer.ck 3<"$$area/model" 4<"$$area/roster" <"$$area/letter" >"$$area/answer"; \
  if ./ck text provider/answer.ck 3<"$$area/model" 4<"$$area/alt-roster" <"$$area/letter" >"$$area/wrong-episode" 2>"$$area/recipient-error"; then exit 1; fi; \
  test "$$(cat "$$area/wrong-episode")" = "fault $$(word e.id)"; \
  pc generated "$$area/answer" "$$area/match"; testword "$$area/match" yes; \
  seal() { pc "$$1" "$$area/none" "$$area/raw-sig"; ./ck text provider/seal.ck 4<"$$area/raw-sig" <"$$2" >"$$3"; }; \
  seal answer-signature "$$area/answer" "$$area/attested"; \
  seal wrong-signature "$$area/answer" "$$area/wrong-role"; \
  pc changed-answer "$$area/answer" "$$area/changed-answer"; \
  seal answer-signature "$$area/changed-answer" "$$area/altered"; \
  pc ledger "$$area/job" "$$area/ledger"; \
  unchanged() { pc was-untouched "$$area/settlement" "$$area/unchanged" "$$area/ledger"; testword "$$area/unchanged" yes; }; \
  consume() { ./ck text provider/resume.ck 3<"$$dir/channel" 4<"$$1" 7<"$$area/dispatch" 8<"$$area/ledger" <"$${2:-$$area/attested}" >"$$area/settlement"; pc settled "$$area/settlement" "$$area/status"; }; \
  consume "$$area/job" "$${PROVIDER_WITNESS:-$$area/attested}"; testword "$$area/status" share.allowed; pc completed "$$area/settlement" "$$area/completed" "$$area/answer"; testword "$$area/completed" yes; \
  for envelope in altered wrong-role; do consume "$$area/job" "$$area/$$envelope"; testword "$$area/status" ed25519.invalid-signature; unchanged; done; \
  consume "$$area/other"; testword "$$area/status" e.id; unchanged; \
  job_run "$$area/none" "$$area/job" "$$area/repeated"; check_route "$$area/repeated" provider.external inbox.replayed; cmp "$$1" "$$area/letter"; \
  apply_role withdraw; \
  consume "$$area/job"; testword "$$area/status" share.ungranted; unchanged; \
  setup renewed alt-; \
  pc permit "$$area/job" "$$area/alt-permit" "$$dir/channel"; pc ticket "$$area/job" "$$area/alt-ticket" "$$dir/channel"; \
  pc change "$$area/alt-permit" "$$area/alt-owner-change" "$$area/genesis"; \
  pc change "$$area/alt-ticket" "$$area/alt-listener-change" "$$area/genesis"; \
  apply_role owner; apply_role listener; \
  job_run "$$area/none" "$$area/job" "$$area/renewed-turn"; check_route "$$area/renewed-turn" provider.external inbox.accepted; \
  cp "$$area/dispatch" "$$area/first-decision"; cp "$$area/decision" "$$area/dispatch"; \
  consume "$$area/job"; testword "$$area/status" e.id; unchanged; \
  ./ck text provider/answer.ck 3<"$$area/model" 4<"$$area/alt-roster" <"$$(find "$$dir/letters" -type f)" >"$$area/renewed-answer"; \
  if cmp -s "$$area/answer" "$$area/renewed-answer"; then exit 1; fi; \
  for record in "$$area/active/private"/route-*; do if cmp -s "$$record" "$$area/first-decision"; then cp "$$record" "$$dir/private/"; fi; done; \
  cp "$$area/first-decision" "$$area/dispatch"; consume "$$area/job"; testword "$$area/status" e.id; unchanged; \
  setup local; \
  apply_role owner; apply_role listener; \
  ./ck text nums/pack.ck <ground/nums-image.ck >"$$area/image"; \
  mkfifo "$$area/requests" "$$area/returns"; ./seed-run "$$area/image" <"$$area/requests" >"$$area/returns" & pid=$$!; \
  ./ck text provider/dispatch.ck 5>"$$area/requests" <"$$area/returns" 3<"$$dir/channel" 4<"$$area/local-policy" 7<"$$area/job" >"$$area/served"; \
  wait "$$pid"; pc decision "$$area/served" "$$area/decision"; pc selected "$$area/decision" "$$area/kind"; testword "$$area/kind" provider.local; verify_trace; empty; \
  pc local-answer "$$area/served" "$$area/local-answer"; pc local-value "$$area/local-answer" "$$area/value"; testword "$$area/value" 42; \
  ./ck text provider/dispatch.ck 5>/dev/null </dev/null 3<"$$dir/channel" 4<"$$area/local-policy" 7<"$$area/job" >"$$area/uncertain-local"; \
  check_route "$$area/uncertain-local" provider.unserved '(list provider.local e.short)'; empty; \
  printf 'received canonical letter bytes: '; wc -c <"$$area/letter"; \
  printf 'refused provider inbox bytes: '; find "$$area/refused/letters" -type f -exec cat {} + | wc -c; \
  echo "provider: exact dispatch, signed answer, epoch change, late refusal and local preference agreed"
endef
