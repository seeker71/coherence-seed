define verify_instrument
@set -eu; \
  work=$$(mktemp -d); \
  trap 'rm -rf "$$work"' EXIT; \
  word() { printf %s "$$1" | $(CANONICAL_WORDS); }; \
  station() { word "(meter.station (world.station \"$$work/sensor\" (list [bytes 4c] [bytes 48] [bytes 3f]) (list (world.rule \"stay\" (list 0 1 2)))) (meter.profile \"io-kit\" \"registry-tool\" \"mV\" 12090 $$1 [bytes 4c] [bytes 48] [bytes 3f]))"; }; \
  packet() { word "(meter.packet $$1 (meter.reading \"io-kit\" \"mV\" $$2 $$3 $$4) (meter.reading \"$$5\" \"mV\" $$6 $$7 $$8))" >"$$work/sensor"; }; \
  sample() { printf '%s' "$$1" | $(CANONICAL_WORDS) >"$$work/index"; ./ck text ground/instrument-input.ck <"$$work/index" >"$$work/sensor"; }; \
  refuses() { code=$$(word "$$1"); shift; rc=0; "$$@" >"$$work/cause" || rc=$$?; test "$$rc" = 1; test "$$(cat "$$work/cause")" = "fault $$code"; }; \
  open_station() { ./ck text world/open.ck 3<"$$work/station"; }; \
  predict() { ./ck text world/predict.ck 3<"$$work/station" <"$$work/history"; }; \
  observe() { ./ck text world/observe.ck 3<"$$work/station" 4<"$$work/prediction" <"$$work/history"; }; \
  for margin in 0 1; do \
   station "$$margin" >"$$work/station"; sample 0; open_station >"$$work/history"; \
   for i in 1 2 3 4 5 6 7; do predict >"$$work/prediction"; sample "$$i"; observe >"$$work/updated"; cp "$$work/updated" "$$work/history"; done; \
   predict >"$$work/prediction"; observe | cmp - "$$work/history"; printf '%s' "$$margin" >"$$work/mode"; \
   ./ck text ground/instrument-result.ck 3<"$$work/mode" 4<"$$work/station" <"$$work/history" >"$$work/checked"; test "$$(cat "$$work/checked")" = '#33'; \
  done; \
  packet 7 200 205 12090 registry-tool 201 204 12090; refuses world.conflict observe; \
  packet 8 200 205 12090 registry-tool 206 209 12090; refuses meter.outside-window observe; \
  packet 8 200 208 12090 io-kit 201 202 12090; refuses meter.wrong-reader observe; \
  packet 8 200 205 12088 registry-tool 201 204 12092; refuses world.stale observe; \
  packet 8 200000000000000 200000000000005 12088 registry-tool 200000000000001 200000000000004 12092; observe >"$$work/updated"; cp "$$work/updated" "$$work/history"; \
  predict >"$$work/prediction"; packet 9 210 215 9223372036854775807 registry-tool 211 214 9223372036854775807; refuses world.invalid observe; \
  echo "instrument: eight overlapping reader pairs replayed with the exact time brackets; a changed calibration preserved ambiguity and a changed duplicate was refused"
endef

define verify_forecast
@set -eu; \
  work=$$(mktemp -d); \
  trap 'rm -rf "$$work"' EXIT; \
  word() { printf %s "$$1" | $(CANONICAL_WORDS); }; \
  word "(world.station \"$$work/source\" (list [bytes 30] [bytes 31]) (list (world.rule \"hold\" (list 0 1)) (world.rule \"toggle\" (list 1 0))))" >"$$work/station"; \
  predict() { ./ck text world/predict.ck 3<"$$work/station" <"$$work/$$1"; }; \
  observe() { ./ck text world/observe.ck 3<"$$work/station" 4<"$$work/$$2" <"$$work/$$1"; }; \
  refuses() { expected=$$(word "$$1"); shift; rc=0; "$$@" >"$$work/reason" || rc=$$?; [ "$$rc" = 1 ] && [ "$$(cat "$$work/reason")" = "fault $$expected" ]; }; \
  [ "$$(./ck run ground/world-reference.ck)" = 'node #33' ]; \
  word '(world.sample 10 [bytes 30])' >"$$work/source"; \
  ./ck text world/open.ck 3<"$$work/station" >"$$work/initial"; \
  predict initial >"$$work/first"; \
  observe initial first | cmp - "$$work/initial"; \
  word '(world.sample 10 [bytes 31])' >"$$work/source"; refuses world.conflict observe initial first; \
  word '(world.sample 12 [bytes 31])' >"$$work/source"; refuses world.gap observe initial first; \
  word '(world.sample 11 [bytes 31])' >"$$work/source"; \
  observe initial first >"$$work/next"; observe initial first | cmp - "$$work/next"; \
  predict next >"$$work/second"; refuses world.stale observe next first; \
  word '(world.sample 12 [bytes 30])' >"$$work/source"; \
  observe next second >"$$work/last"; \
  ./ck text ground/world-result.ck 3<"$$work/first" 4<"$$work/second" <"$$work/last" >"$$work/result"; \
  test "$$(cat "$$work/result")" = '#33'; \
  rm "$$work/source"; refuses world.unobserved observe next second; \
  echo "world: two hypotheses followed a sequenced physical source; the rejected forecast, changed ranking and exact readings survived a restart"
endef
