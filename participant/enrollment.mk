export ROLE OWNER_ENDORSEMENT LISTENER_ENDORSEMENT
.PHONY: circle-propose circle-propose-return circle-inspect circle-endorse circle-establish

circle-propose: ck
	@$(PARTICIPANT_TMP); printf %s "$${OWNER:?set OWNER}" >"$$scratch/owner"; printf %s "$${LISTENER:?set LISTENER}" >"$$scratch/listener"; printf %s "$${CIRCLE:?set CIRCLE}" >"$$scratch/circle"; \
	  openssl rand -out "$$scratch/episode" 32; ./ck text participant/roster.ck 3<"$$scratch/owner" 4<"$$scratch/listener" 5<"$$scratch/circle" 6<"$${OWNER_PUBLIC:?set OWNER_PUBLIC}" 7<"$${LISTENER_PUBLIC:?set LISTENER_PUBLIC}" <"$$scratch/episode"

circle-propose-return: ck
	@./ck text participant/return.ck <"$${ROSTER:?set ROSTER to the original public roster}"

circle-inspect: ck
	@$(PARTICIPANT_TMP); printf inspect >"$$scratch/mode"; ./ck text participant/establish.ck 3<"$$scratch/mode" 4<"$${ROSTER:?set ROSTER to the proposed public roster}"

circle-endorse: ck
	@$(PARTICIPANT_TMP); $(PARTICIPANT_PUB); printf endorse >"$$scratch/mode"; printf %s "$${ROLE:?set ROLE to owner or listener}" >"$$scratch/role"; \
	  ./ck text participant/establish.ck 3<"$$scratch/mode" 4<"$${ROSTER:?set ROSTER}" 5<"$$scratch/role" 6<"$$scratch/public" >"$$scratch/statement" || { code=$$?; cat "$$scratch/statement" >&2; exit "$$code"; }; \
	  openssl pkeyutl -sign -rawin -inkey "$$KEY_DIR/signing.pem" -in "$$scratch/statement"

circle-establish: ck
	@$(PARTICIPANT_TMP); printf verify >"$$scratch/mode"; printf %s "$${BROKER:?set BROKER to an owner-controlled directory}" >"$$scratch/path"; \
	  handshake() { ./ck text participant/establish.ck 3<"$$scratch/mode" 4<"$${ROSTER:?set ROSTER}" 5<"$${OWNER_ENDORSEMENT:?set OWNER_ENDORSEMENT}" 6<"$${LISTENER_ENDORSEMENT:?set LISTENER_ENDORSEMENT}" 7<"$$scratch/path"; }; \
	  handshake >"$$scratch/agreement" || { code=$$?; cat "$$scratch/agreement" >&2; exit "$$code"; }; mkdir -p "$$BROKER/letters"; printf install >"$$scratch/mode"; handshake
