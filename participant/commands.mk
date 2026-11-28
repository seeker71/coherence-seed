# Participants keep their own signing key and private journal. Only the broker
# needs its protected register directory. The signing steps can run offline.
export KEY_DIR BROKER OWNER LISTENER CIRCLE OWNER_PUBLIC LISTENER_PUBLIC ROSTER EDITION UNTIL HEAD ACTION
PARTICIPANT_TMP = set -eu; umask 077; scratch=$$(mktemp -d); trap 'rm -rf "$$scratch"' EXIT
PARTICIPANT_RUN = ./ck text participant/operate.ck 3<"$$scratch/mode" 4<"$${ROSTER:?set ROSTER to the enrolled public roster}"
PARTICIPANT_PUB = openssl pkey -in "$${KEY_DIR:?set KEY_DIR to your own directory}/signing.pem" -pubout -outform DER -out "$$scratch/public.der"; test "$$(wc -c <"$$scratch/public.der")" -eq 44; tail -c 32 "$$scratch/public.der" >"$$scratch/public"

.PHONY: circle-key circle-join circle-retract circle-resume circle-source circle-record circle-correct circle-show circle-edition circle-select-return circle-select circle-preview circle-ticket circle-owner-head circle-listener-head circle-sign circle-verify circle-submit circle-read
.PHONY: circle-recovery-enroll circle-recovery-cancel circle-recovery-apply
.PHONY: circle-turn-open circle-turn-inspect circle-turn-sign circle-turn-advance
export TURN_HEAD SOURCE_LANGUAGE TARGET_LANGUAGE READING_ID
TURN_RUN = ./ck text participant/turn.ck 3<"$$scratch/mode" 4<"$${ROSTER:?set ROSTER to the agreed roster}"

circle-turn-open: ck
	@$(PARTICIPANT_TMP); printf open >"$$scratch/mode"; printf %s "$${SOURCE_LANGUAGE:?set SOURCE_LANGUAGE}" >"$$scratch/from"; printf %s "$${TARGET_LANGUAGE:?set TARGET_LANGUAGE}" >"$$scratch/into"; \
	  $(TURN_RUN) 6<"$$scratch/from" 7<"$$scratch/into"

circle-turn-inspect circle-turn-advance: ck
	@$(PARTICIPANT_TMP); printf %s '$(@:circle-turn-%=%)' >"$$scratch/mode"; $(TURN_RUN) 5<"$${TURN_HEAD:?set TURN_HEAD to the locally accepted turn file}"

circle-turn-sign: ck
	@$(PARTICIPANT_TMP); $(PARTICIPANT_PUB); cp "$${ROSTER:?set ROSTER}" "$$scratch/roster"; ROSTER="$$scratch/roster"; cp "$${TURN_HEAD:?set TURN_HEAD}" "$$scratch/head"; \
	  printf %s "$${ACTION:?set ACTION to propose, correct, refuse or silence}" >"$$scratch/action"; printf %s "$$READING_ID" >"$$scratch/reading"; \
	  turn_step() { printf %s "$$1" >"$$scratch/mode"; $(TURN_RUN) 5<"$$scratch/head" 6<"$$scratch/action" 7<"$$scratch/reading" 8<"$$scratch/signature" 9<"$$scratch/public" <"$$2" >"$$3" || { code=$$?; cat "$$3"; exit "$$code"; }; }; \
	  : >"$$scratch/signature"; turn_step draft /dev/null "$$scratch/choice"; turn_step digest "$$scratch/choice" "$$scratch/digest"; \
	  openssl pkeyutl -sign -rawin -inkey "$$KEY_DIR/signing.pem" -in "$$scratch/digest" -out "$$scratch/signature"; \
	  turn_step seal "$$scratch/choice" "$$scratch/packet"; cat "$$scratch/packet"

circle-key: ck
	@$(PARTICIPANT_TMP); mkdir "$${KEY_DIR:?set KEY_DIR to a new private directory}"; mkdir "$$KEY_DIR/journal"; \
	  openssl genpkey -algorithm ED25519 -out "$$KEY_DIR/signing.pem"; $(PARTICIPANT_PUB); cp "$$scratch/public" "$$KEY_DIR/public"

circle-join: ck
	@$(PARTICIPANT_TMP); $(PARTICIPANT_PUB); printf folder >"$$scratch/mode"; folder=$$($(PARTICIPANT_RUN) 9<"$$scratch/public"); \
	  printf join >"$$scratch/mode"; $(PARTICIPANT_RUN) 9<"$$scratch/public" >"$$scratch/head"; mkdir "$$KEY_DIR/journal/$$folder"; cp "$$scratch/head" "$$KEY_DIR/journal/$$folder/head"

circle-source circle-retract circle-resume circle-record circle-correct circle-show circle-edition circle-select-return circle-select: ck
	@$(PARTICIPANT_TMP); $(PARTICIPANT_PUB); mode='$(@:circle-%=%)'; printf folder >"$$scratch/mode"; folder=$$($(PARTICIPANT_RUN) 9<"$$scratch/public"); printf %s "$$mode" >"$$scratch/mode"; printf %s "$$KEY_DIR/journal/$$folder" >"$$scratch/path"; \
	  case "$$mode" in correct|select|select-return|resume|retract) test -s "$${EDITION:?set EDITION to the reviewed private revision file}";; *) EDITION=/dev/null;; esac; \
	  if test "$$mode" = select || test "$$mode" = select-return; then openssl rand -out "$$scratch/nonce" 32; printf %s "$${UNTIL:?set UNTIL to the exclusive Unix expiry second}" >"$$scratch/until"; else : >"$$scratch/nonce"; : >"$$scratch/until"; fi; \
	  $(PARTICIPANT_RUN) 5<"$$scratch/path" 6<"$$EDITION" 7<"$$scratch/nonce" 8<"$$scratch/until" 9<"$$scratch/public"

circle-preview circle-ticket circle-verify: ck
	@$(PARTICIPANT_TMP); printf %s '$(@:circle-%=%)' >"$$scratch/mode"; $(PARTICIPANT_RUN)

circle-owner-head circle-listener-head circle-submit circle-read: ck
	@$(PARTICIPANT_TMP); printf %s '$(@:circle-%=%)' >"$$scratch/mode"; printf %s "$${BROKER:?set BROKER to the protected broker directory}" >"$$scratch/path"; $(PARTICIPANT_RUN) 5<"$$scratch/path"

circle-recovery-enroll circle-recovery-cancel circle-recovery-apply: ck
	@$(PARTICIPANT_TMP); printf channel >"$$scratch/mode"; printf %s "$${BROKER:?set BROKER to the protected broker directory}" >"$$scratch/path"; \
	  ROSTER="$$BROKER/private/peers"; $(PARTICIPANT_RUN) 5<"$$scratch/path" </dev/null >"$$scratch/channel"; \
	  ./ck text recovery/$(@:circle-recovery-%=%).ck 3<"$$scratch/channel"

circle-sign: ck
	@$(PARTICIPANT_TMP); case "$${ACTION:?set ACTION to share, listen, withdraw or pause}" in share|listen|withdraw|pause);; *) exit 2;; esac; \
	  printf %s "$$ACTION" >"$$scratch/mode"; $(PARTICIPANT_RUN) 5<"$${HEAD:?set HEAD to the matching broker revision file}" >"$$scratch/intent" || { code=$$?; cat "$$scratch/intent"; exit "$$code"; }; \
	  ./ck text circle/statement.ck 3<"$$ROSTER" <"$$scratch/intent" >"$$scratch/control"; ./ck text circle/signing-bytes.ck <"$$scratch/control" >"$$scratch/digest"; \
	  openssl pkeyutl -sign -rawin -inkey "$${KEY_DIR:?set KEY_DIR to your own directory}/signing.pem" -in "$$scratch/digest" -out "$$scratch/signature"; \
	  ./ck text circle/seal.ck 4<"$$scratch/signature" <"$$scratch/control" >"$$scratch/proof"; printf packet >"$$scratch/mode"; $(PARTICIPANT_RUN) 5<"$$scratch/intent" <"$$scratch/proof"
