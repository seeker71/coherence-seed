define verify_reconnection
@set -eu; \
  work=$$(mktemp -d); \
  trap 'rm -rf "$$work"' EXIT; \
  word() { printf %s "$$1" | $(CANONICAL_WORDS); }; \
  pc() { printf %s "$$1" >"$$work/mode"; ./ck text ground/mesh-case.ck 3<"$$work/mode" <"$${2:-$$work/no}" ; }; \
  word no >"$$work/no"; \
  for name in roster foreign-roster episode withdraw-change withdraw-proof wrong-role altered-time competing-receipt; do pc "$$name" >"$$work/$$name"; done; \
  for tag in A B; do for kind in ticket expectation arrival receipt letter owner-change owner-proof listener-change listener-proof; do pc "$$tag-$$kind" >"$$work/$$tag-$$kind"; done; done; \
  setup() { area="$$work/$$1"; mkdir -p "$$area/private" "$$area/letters"; word "\"$$area\"" >"$$area/path"; pc channel "$$area/path" >"$$area/channel"; }; \
  for place in sender receiver; do setup "$$place"; cp "$$work/roster" "$$area/private/peers"; ./ck text circle/pin.ck <"$$work/roster" >"$$area/private/authority"; cp "$$area/private/authority" "$$area/private/listening"; done; \
  setup witness; \
  ./ck text mesh/pin.ck <"$$work/roster" >"$$area/private/mesh-time"; \
  apply() { for actor in owner listener; do ./ck text circle/apply.ck 3<"$$work/$$1/channel" 4<"$$work/$$2-$$actor-change" <"$$work/$$2-$$actor-proof" >"$$work/last-change"; done; }; \
  expecting() { ./ck text mesh/expect.ck 3<"$$work/sender/channel" <"$$work/$$1-ticket" >"$$work/expectation"; cmp "$$work/expectation" "$$work/$$1-expectation"; }; \
  apply sender A; apply receiver A; expecting A; \
  cp -R "$$work/receiver" "$$work/early-receiver"; \
  mkfifo "$$work/to-clock" "$$work/from-clock"; \
  cross() { \
   sample=$$1; shift; \
   word "$$sample" >"$$work/tick"; \
   ./ck text "$${CLOCK_SERVICE:-ground/mesh-clock.ck}" 3<"$$work/witness/channel" 4<"$$work/tick" <"$$work/to-clock" 5>"$$work/from-clock" >"$$work/service-log" & clock_pid=$$!; \
   code=0; "$$@" 5>"$$work/to-clock" <"$$work/from-clock" >"$$work/delivered" || code=$$?; \
   wait "$$clock_pid" || return 1; \
   return "$$code"; \
  }; \
  deliver() { cross "$$2" ./ck text mesh/deliver.ck 3<"$$work/receiver/channel" 4<"$$work/$$1-expectation"; }; \
  check_error() { test "$$(cat "$$work/$$1")" = "fault $$(word "$$2")"; }; \
  for sample in mesh.episode mesh.challenge no; do \
   if deliver A "$$sample"; then exit 1; fi; \
   reason=e.id; if test "$$sample" = no; then reason=e.short; fi; check_error delivered "$$reason"; \
   test -z "$$(ls -A "$$work/receiver/letters")"; \
  done; \
  deliver A 100; \
  cmp "$$work/delivered" "$$work/A-arrival"; set -- "$$work/receiver/letters/"*; test "$$#" = 1; cmp "$$1" "$$work/A-letter"; \
  test -z "$$(ls -A "$$work/sender/letters")"; \
  if ./ck text mesh/confirm.ck 3<"$$work/sender/channel" <"$$work/B-receipt" >"$$work/confirmed"; then exit 1; fi; check_error confirmed e.unbound; \
  for p in sender receiver; do apply "$$p" B; done; expecting B; \
  deliver B 120; cmp "$$work/delivered" "$$work/B-arrival"; \
  confirm() { ./ck text mesh/confirm.ck 3<"$$work/sender/channel" <"$$work/$$1" >"$$work/confirmed"; }; \
  confirm B-receipt; cmp "$$work/confirmed" "$$work/B-receipt"; \
  if test -n "$${MESH_WITNESS:-}"; then cp "$$MESH_WITNESS" "$$work/guest-delivery"; confirm guest-delivery; cmp "$$work/confirmed" "$$work/A-receipt"; fi; \
  confirm A-receipt; cmp "$$work/confirmed" "$$work/A-receipt"; \
  confirm B-receipt; cmp "$$work/confirmed" "$$work/B-receipt"; \
  for event in altered-time wrong-role competing-receipt; do \
   if confirm "$$event"; then exit 1; fi; \
   reason=ed25519.invalid-signature; if test "$$event" = competing-receipt; then reason=inbox.conflict; fi; \
   test "$$(cat "$$work/confirmed")" = "fault $$(word "$$reason")"; \
  done; \
  confirm A-receipt; cmp "$$work/confirmed" "$$work/A-receipt"; \
  cp "$$work/foreign-roster" "$$work/sender/private/peers"; \
  if confirm A-receipt; then exit 1; fi; check_error confirmed e.id; \
  cp "$$work/roster" "$$work/sender/private/peers"; \
  mv "$$work/receiver" "$$work/later-receiver"; cp -R "$$work/early-receiver" "$$work/receiver"; \
  if deliver A 110; then exit 1; fi; \
  test "$$(cat "$$work/delivered")" = "fault $$(word share.clock-regressed)"; \
  test -z "$$(ls -A "$$work/receiver/letters")"; \
  pc tick "$$work/witness/private/mesh-time" >"$$work/watermark"; word 120 >"$$work/want"; test "$$(cat "$$work/watermark")" = "$$(cat "$$work/want")"; \
  if deliver A 125; then exit 1; fi; \
  test "$$(cat "$$work/delivered")" = "fault $$(word inbox.stale)"; test -z "$$(ls -A "$$work/receiver/letters")"; \
  if deliver B 125; then exit 1; fi; \
  test "$$(cat "$$work/delivered")" = "fault $$(word inbox.stale)" || test "$$(cat "$$work/delivered")" = "fault $$(word inbox.paused)"; \
  test -z "$$(ls -A "$$work/receiver/letters")"; \
  cp "$$work/later-receiver/private/authority" "$$work/later-receiver/private/listening" "$$work/receiver/private"; \
  ./ck text circle/apply.ck 3<"$$work/sender/channel" 4<"$$work/withdraw-change" <"$$work/withdraw-proof" >"$$work/stopped"; \
  cross 130 ./ck text mesh/checkpoint.ck 3<"$$work/sender/channel"; \
  if deliver B 135; then exit 1; fi; check_error delivered inbox.stale; \
  test -z "$$(ls -A "$$work/receiver/letters")"; \
  cross 201 ./ck text mesh/now.ck 3<"$$work/episode"; \
  pc tick "$$work/witness/private/mesh-time" >"$$work/watermark"; test "$$(cat "$$work/watermark")" = "$$(word 201)"; \
  word 205 >"$$work/tick"; \
  ./ck text ground/mesh-clock.ck 3<"$$work/witness/channel" 4<"$$work/tick" <"$$work/to-clock" 5>"$$work/from-clock" >"$$work/service-log" & session_pid=$$!; \
  exec 7>"$$work/to-clock" 8<"$$work/from-clock"; \
  ./ck text mesh/now.ck 3<"$$work/episode" 5>&7 <&8 >"$$work/clock-observation"; \
  pc witness-busy "$$work/witness/channel" >"$$work/busy"; test "$$(cat "$$work/busy")" = "$$(word yes)"; \
  exec 7>&- 8<&-; wait "$$session_pid"; \
  pc witness-busy "$$work/witness/channel" >"$$work/busy"; test "$$(cat "$$work/busy")" = "$$(word no)"; \
  CLOCK_SERVICE=mesh/clock.ck cross 0 ./ck text mesh/now.ck 3<"$$work/episode"; \
  pc tick "$$work/witness/private/mesh-time" >"$$work/watermark"; cmp "$$work/watermark" "$$work/delivered"; \
  echo "mesh: signed arrival, reordered receipts, changed attestations and rollback agreed"
endef
