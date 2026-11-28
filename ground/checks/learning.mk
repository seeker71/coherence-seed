define verify_owned_model
@set -eu; \
  rounds=$$(mktemp -d); \
  trap 'rm -rf "$$rounds"' EXIT; \
  word() { printf %s "$$1" | $(CANONICAL_WORDS); }; \
  digest() { openssl dgst -sha256 -binary "$$1" | od -An -tx1 -v | tr -d ' \n'; }; \
  run() { printf %s "$$1" >"$$work/mode"; ./ck text ground/owned-study.ck 3<"$$work/mode" 4<"$$work/$${3:-none}" 5<"$$work/$${4:-none}" <"$$work/$$2"; }; \
  g() { printf %s "$$1" >"$$work/mode"; ./ck text ground/circle-exchange.ck 3<"$$work/mode" 4<"$$work/$${4:-none}" <"$$work/$$2" >"$$work/$$3"; }; \
  change() { g change "$$2" update "$$3"; ./ck text "$$1.ck" 3<"$$work/channel" <"$$work/update" >"$$work/$$4"; }; \
  begin() { \
   work="$$rounds/$$1"; mkdir -p "$$work/private" "$$work/letters"; \
   printf '#32' >"$$work/none"; for a in authority listening adapter-authority; do cp "$$work/none" "$$work/private/$$a"; done; \
   printf %s "$$work" >"$$work/location"; word "\"$$2\"" >"$$work/nonce"; g channel location channel; g identity none genesis; \
  }; \
  admit() { \
  protocol=$$(digest "$$work/protocol"); word "(teach.material [bytes $$(digest "$$work/data")] \"$$work/letters\" [bytes $$parent] [bytes $$parent] [bytes $$protocol])" >"$$work/material"; \
  ./ck text learning/source.ck 3<"$$work/channel" 4<"$$work/material" <"$$work/data" >"$$work/source"; g permit source granted nonce; g ticket source ticket nonce; \
  change circle/authorize granted genesis issued; change circle/listen ticket genesis listener; \
  ./ck text learning/teach.ck 3<"$$work/channel" <"$$work/ticket" >"$$work/round"; \
  set -- "$$work/letters/"*; test "$$#" = 1; cp "$$1" "$$work/received"; g bytes received admitted; cmp "$$work/data" "$$work/admitted"; \
  }; \
  activate_artifact() { \
  ./ck text learning/artifact-scope.ck 3<"$$work/plan" 4<"$$work/parent" 5<"$$work/fit" 6<"$$work/shuffled" <"$$work/round" >"$$work/derivative"; g permit derivative allowed nonce; \
  cp "$$2" "$$work/letters/$$1.artifact"; change learning/allow allowed genesis approved; \
  ./ck text learning/activate.ck 3<"$$work/channel" 4<"$$work/plan" 5<"$$work/parent" 6<"$$work/fit" 7<"$$work/shuffled" <"$$work/round" >"$$work/active"; \
  }; \
  begin intent_fit "synthetic specialist training"; \
  cp ground/intent-contract.ck "$$work/protocol"; \
  cp ground/intent-corpus.ck "$$work/data"; \
  parent=$$(digest learning/intent-network.ck); \
  admit; \
  word "[bytes $$protocol]" >"$$work/policy"; \
  expected_fit=b33f368a02160ccf0f69dcd387931c9aeabb639cb93134dac189adaa65959c1e; \
  fit_intent() { ./ck text learning/intent-admitted-fit.ck 3<"$$work/channel" 4<"$$work/policy" <"$$work/round"; }; \
  fit_intent >"$$work/model"; \
  test "$$(digest "$$work/model")" = "$$expected_fit"; \
  ./ck text learning/intent-fit.ck <"$$work/admitted" | cmp - "$$work/model"; \
  cp "$$work/model" "$$rounds/frozen"; \
  change circle/authorize none issued withdrawn; \
  if fit_intent >"$$work/refusal"; then exit 1; fi; \
  test "$$(cat "$$work/refusal")" = "fault $$(word share.ungranted)"; \
  begin intent_eval "synthetic specialist evaluation"; \
  cp ground/intent-contract.ck "$$work/protocol"; \
  ./ck text ground/intent-statements.ck <ground/intent-sealed.ck >"$$work/statements"; \
  word "[bytes $$(digest "$$work/protocol")]" >"$$work/policy"; \
  word "(teach.material [bytes $$(digest "$$work/statements")] \"$$work/letters\" [bytes $$expected_fit] [bytes $$expected_fit] [bytes $$(digest "$$work/protocol")])" >"$$work/material"; \
  ./ck text learning/intent-evaluation-source.ck 3<"$$work/channel" 4<"$$work/material" <"$$work/statements" >"$$work/source"; \
  g permit source granted nonce; \
  g ticket source ticket nonce; \
  change circle/authorize granted genesis issued; \
  change circle/listen ticket genesis listener; \
  if ./ck text learning/teach.ck 3<"$$work/channel" <"$$work/ticket" >"$$work/refusal"; then exit 1; fi; \
  test "$$(cat "$$work/refusal")" = "fault $$(word teach.wrong-purpose)"; \
  score_intent() { ./ck text learning/intent-evaluate.ck 3<"$$work/channel" 4<"$$work/policy" 5<"$$1" <"$$work/ticket"; }; \
  score_intent "$$rounds/frozen" >"$$work/measured"; \
  ./ck text learning/intent-read.ck 3<"$$rounds/frozen" <"$$work/statements" | cmp - "$$work/measured"; \
  if score_intent "$$work/statements" >"$$work/refusal"; then exit 1; fi; \
  test "$$(cat "$$work/refusal")" = "fault $$(word e.id)"; \
  change circle/listen none listener paused; \
  if score_intent "$$rounds/frozen" >"$$work/refusal"; then exit 1; fi; \
  test "$$(cat "$$work/refusal")" = "fault $$(word inbox.paused)"; \
  test "$$(./ck text ground/intent-study.ck)" = "$$(word yes)"; \
  begin grammar "owned grammar round"; \
  cp ground/language-corpus.ck "$$work/data"; run parent data >"$$work/parent-model"; parent=$$(digest "$$work/parent-model"); cases=$$(digest ground/language-cases.ck); \
  printf '%s\n' "Owned grammar v1; six finite completions in canonical request order; score every next gold token and EOS using top-1 mistakes; control rotates all final verbs by one; assess all cases as protected." "$$cases" "$$(digest ground/owned-study.ck)" "$$(digest learning/network.ck)" >"$$work/protocol"; \
  admit; \
  ./ck text learning/train.ck <"$$work/admitted" >"$$work/fit-model"; run control admitted >"$$work/perturbed"; ./ck text learning/train.ck <"$$work/perturbed" >"$$work/shuffled-model"; \
  fit=$$(digest "$$work/fit-model"); shuffled=$$(digest "$$work/shuffled-model"); word "(list [bytes $$parent] [bytes $$fit] [bytes $$shuffled] [bytes $$protocol])" >"$$work/identities"; \
  run design identities >"$$work/plan"; for arm in parent fit shuffled; do word "[bytes $$(digest "$$work/$$arm-model")]" >"$$work/model-id"; run run plan "$$arm-model" model-id >"$$work/$$arm"; done; \
  ./ck text learning/assess.ck 3<"$$work/parent" 4<"$$work/fit" 5<"$$work/shuffled" <"$$work/plan" >"$$work/decision"; \
  activate_artifact "$$fit" "$$work/fit-model"; \
  ./ck text learning/use.ck 3<"$$work/channel" <"$$work/active" >"$$work/received-model"; cmp "$$work/received-model" "$$work/fit-model"; \
  word '(lm.request (list 2 6) 3)' >"$$work/question"; ./ck text learning/generate.ck 3<"$$work/received-model" <"$$work/question" >"$$work/reply"; test "$$(cat "$$work/reply")" = "$$(word '(lm.completion (list 12) lm.eos)')"; \
  mv "$$work/letters/$$fit.artifact" "$$work/letters/$$fit.safetensors"; ./ck text learning/use.ck 3<"$$work/channel" <"$$work/active" | cmp - "$$work/fit-model"; \
  change learning/allow none approved stopped; rc=0; ./ck text learning/use.ck 3<"$$work/channel" <"$$work/active" >"$$work/refusal" || rc=$$?; test "$$rc" = 1; test "$$(cat "$$work/refusal")" = "fault $$(word share.ungranted)"; \
  ./ck text lens/show-canonical.ck lens/all.ck <"$$work/decision"; \
  begin rank "owned rank round"; \
  rank_study() { printf %s "$$1" >"$$work/mode"; ./ck text ground/rank-study.ck 3<"$$work/mode" 4<"$$work/$${3:-none}" 5<"$$work/$${4:-none}" 6<"$$work/base" <"$$work/$$2"; }; \
  fit() { ./ck text learning/rank-fit.ck 3<"$$work/base" <"$$work/$$1" >"$$work/$$2"; }; \
  ./ck text learning/train.ck <ground/language-corpus.ck >"$$work/base"; cp "$$work/base" "$$work/untouched"; \
  parent=$$(digest "$$work/base"); rank_study scope none >"$$work/scope"; \
  printf fit | ./ck text ground/rank-data.ck >"$$work/data"; \
  printf '%s\n' "Public finite grammar v1; rank 3, factor clip 12, margin 8, 12 passes, explicit adjective/noun slots; ordered teacher-forced top-1 with EOS; caller-origin fixed for the full continuation; all 22 cases protected." "$$(digest ground/rank-trial.ck)" "$$(digest ground/rank-study.ck)" "$$(digest learning/rank-network.ck)" "$$(digest "$$work/scope")" >"$$work/protocol"; \
  admit; \
  fit admitted raw-fit; rank_study control admitted >"$$work/shuffled"; fit shuffled raw-shuffled; \
  printf control | ./ck text ground/rank-data.ck | cmp - "$$work/shuffled"; \
  printf zero | ./ck text ground/rank-data.ck >"$$work/zero-data"; fit zero-data zero; \
  ./ck text ground/rank-compare.ck 3<"$$work/base" 4<"$$work/zero" 5<"$$work/raw-fit" 6<"$$work/raw-shuffled" >"$$work/held"; \
  test "$$(cat "$$work/held")" = "$$(word '(list (list) (list (list 2 7) (list 3 9) (list 3 11) (list 4 6) (list 4 10) (list 5 8)) (list))')"; \
  test "$$(./ck text ground/rank-invalid.ck 3<"$$work/base" 4<"$$work/raw-fit")" = "$$(word yes)"; \
  for arm in fit shuffled; do ./ck text learning/rank-compose.ck 3<"$$work/base" 4<"$$work/raw-$$arm" <"$$work/scope" >"$$work/$$arm-model"; done; \
  cp "$$work/base" "$$work/parent-model"; cmp "$$work/untouched" "$$work/base"; \
  study() { \
   candidate=$$(digest "$$work/$$1"); contrast=$$(digest "$$work/$$2"); word "(list [bytes $$parent] [bytes $$candidate] [bytes $$contrast] [bytes $$protocol])" >"$$work/identities"; \
   rank_study design identities >"$$work/plan"; \
   for arm in parent fit shuffled; do pick=parent-model; if test "$$arm" = fit; then pick=$$1; elif test "$$arm" = shuffled; then pick=$$2; fi; \
    word "[bytes $$(digest "$$work/$$pick")]" >"$$work/model-id"; rank_study run plan "$$pick" model-id >"$$work/$$arm"; \
   done; \
   ./ck text learning/assess.ck 3<"$$work/parent" 4<"$$work/fit" 5<"$$work/shuffled" <"$$work/plan" >"$$work/decision"; \
  }; \
  study raw-fit raw-shuffled; cp "$$work/decision" "$$work/raw-decision"; \
  test "$$(rank_study summary raw-decision)" = "$$(word '(list trial.hold (trial.score 16 6 0) (trial.score 13 14 9) (trial.score 9 15 7))')"; \
  study fit-model shuffled-model; \
  test "$$(rank_study summary decision)" = "$$(word '(list trial.better (trial.score 16 6 0) (trial.score 22 0 0) (trial.score 16 6 0))')"; \
  activate_artifact "$$candidate" "$$work/fit-model"; \
  use() { ./ck text learning/rank-use.ck 3<"$$work/channel" 4<"$$work/active" 5<"$$work/$$1" <"$$work/question" >"$$work/reply"; }; \
  word '(lm.request (list 4 6) 2)' >"$$work/question"; use base; test "$$(cat "$$work/reply")" = "$$(word '(lm.completion (list 13) lm.eos)')"; \
  word '(lm.request (list) 1)' >"$$work/question"; use base; test "$$(cat "$$work/reply")" = "$$(word '(lm.completion (list 5) lm.limited)')"; \
  word '(lm.request (list 3) 3)' >"$$work/question"; use base; ./ck text learning/generate.ck 3<"$$work/base" <"$$work/question" | cmp - "$$work/reply"; test "$$(cat "$$work/reply")" = "$$(word '(lm.completion (list 11 14) lm.eos)')"; \
  if use zero; then exit 1; fi; test "$$(cat "$$work/reply")" = "fault $$(word e.id)"; \
  change learning/allow none approved stopped; if use base; then exit 1; fi; test "$$(cat "$$work/reply")" = "fault $$(word share.ungranted)"; \
  cmp "$$work/base" "$$work/untouched"; \
  ./ck text lens/show-canonical.ck lens/all.ck <"$$work/raw-decision"; \
  ./ck text lens/show-canonical.ck lens/all.ck <"$$work/decision"
endef

define verify_fit_replay
@set -eu; work=$$(mktemp -d); trap 'rm -rf "$$work"' EXIT; \
  [ "$$(./ck run ground/supervision-reference.ck)" = 'node #33' ]; \
  [ "$$(./ck run ground/language-reference.ck)" = 'node #33' ]; \
  ./ck text learning/train.ck <ground/language-corpus.ck >"$$work/model"; \
  ./ck text lens/to-print.ck <ground/language-corpus.ck >"$$work/corpus-wire"; \
  ./ck text learning/train.ck <"$$work/corpus-wire" | cmp - "$$work/model"; \
  [ "$$(./ck run ground/language-replies.ck <"$$work/model")" = 'node #33' ]; \
  ./ck text form/from-print.ck <"$$work/model" >"$$work/model-records"; \
  [ "$$(./ck run ground/language-replies.ck <"$$work/model-records")" = 'node #33' ]; \
  echo "language: record and printed corpora reproduced identical weights; both checkpoint encodings generated every held-out completion"
endef

define verify_derivative
@set -eu; \
  work=$$(mktemp -d); \
  trap 'rm -rf "$$work"' EXIT; \
  mkdir "$$work/private" "$$work/letters" "$$work/elsewhere"; \
  printf '#32' > "$$work/none"; for register in authority listening adapter-authority; do cp "$$work/none" "$$work/private/$$register"; done; \
  printf %s "$$work" > "$$work/location"; printf '[#2 31]' > "$$work/nonce"; \
  case_value() { printf %s "$$2" > "$$work/mode"; ./ck text "ground/$$1.ck" 3<"$$work/mode" 4<"$$work/$${5:-none}" <"$$work/$$3" >"$$work/$$4"; }; \
  g() { case_value circle-exchange "$$@"; }; \
  t() { case_value teaching-case "$$@"; }; \
  change() { g change "$$2" update "$$3"; ./ck text "$$1.ck" 3<"$$work/channel" <"$$work/update" >"$$work/$$4"; }; \
  begin() { ./ck text learning/teach.ck 3<"$$work/$${2:-channel}" <"$$work/$$1"; }; \
  offer() { ./ck text learning/artifact-scope.ck 3<"$$work/plan" 4<"$$work/parent" 5<"$$work/$$2" 6<"$$work/control" <"$$work/$$1"; }; \
  activate() { ./ck text learning/activate.ck 3<"$$work/channel" 4<"$$work/plan" 5<"$$work/parent" 6<"$$work/$$2" 7<"$$work/control" <"$$work/$$1"; }; \
  use() { ./ck text learning/use.ck 3<"$$work/$${2:-channel}" <"$$work/$$1"; }; \
  denied() { reason=$$1; shift; rc=0; "$$@" >"$$work/denied" || rc=$$?; expected=$$(printf %s "$$reason" | $(CANONICAL_WORDS)); [ "$$rc" = 1 ] && [ "$$(cat "$$work/denied")" = "fault $$expected" ] || { echo "learning: expected $$reason, received $$(cat "$$work/denied")" >&2; return 1; }; }; \
  g channel location channel; g identity none genesis; t wrong-location channel elsewhere-channel; \
  for item in plan parent fit hold-fit other-fit control weights dataset; do t "$$item" none "$$item"; done; \
  t source channel source plan; t recall source recall-only; \
  g permit recall-only recall-permit nonce; g ticket recall-only recall-ticket nonce; \
  g permit source source-permit nonce; g ticket source source-ticket nonce; \
  change circle/authorize recall-permit genesis issued; change circle/listen recall-ticket genesis listened; \
  denied teach.wrong-purpose begin recall-ticket; test -z "$$(ls -A "$$work/letters")"; \
  change circle/authorize source-permit issued issued; change circle/listen source-ticket listened listened; \
  denied teach.wrong-location begin source-ticket elsewhere-channel; \
  t capture channel reconstructed source-ticket; offer reconstructed fit > "$$work/unadmitted-scope"; g permit unadmitted-scope unadmitted-permit nonce; change learning/allow unadmitted-permit genesis unadmitted-revision; \
  denied teach.not-admitted activate reconstructed fit; test ! -e "$$work/private/active-adapter"; test -z "$$(ls -A "$$work/letters")"; \
  begin source-ticket > "$$work/round"; set -- "$$work/letters/"*; test "$$#" = 1; cat "$$1" > "$$work/letter"; g bytes letter observed; cmp "$$work/dataset" "$$work/observed"; \
  offer round fit > "$$work/scope"; t bad-sha scope bad-scope; g permit bad-scope bad-permit nonce; g change bad-permit update unadmitted-revision; denied teach.mismatch ./ck text learning/allow.ck 3<"$$work/channel" <"$$work/update"; g permit scope artifact-permit nonce; change learning/allow artifact-permit unadmitted-revision approved; \
  t path round artifact-path plan; model_path=$$(cat "$$work/artifact-path"); cp "$$work/weights" "$$model_path"; \
  denied teach.not-eligible activate round hold-fit; test ! -e "$$work/private/active-adapter"; \
  denied share.different activate round other-fit; test ! -e "$$work/private/active-adapter"; \
  change circle/authorize none issued stopped; denied share.ungranted activate round fit; test ! -e "$$work/private/active-adapter"; \
  change circle/authorize source-permit stopped issued; denied teach.stale activate round fit; test ! -e "$$work/private/active-adapter"; \
  begin source-ticket > "$$work/listener-round"; offer listener-round fit > "$$work/scope"; g permit scope artifact-permit nonce; change learning/allow artifact-permit approved approved; \
  change circle/listen none listened silence; denied inbox.paused activate listener-round fit; test ! -e "$$work/private/active-adapter"; \
  change circle/listen source-ticket silence listened; denied teach.stale activate listener-round fit; test ! -e "$$work/private/active-adapter"; \
  begin source-ticket > "$$work/new-round"; offer new-round fit > "$$work/scope"; g permit scope artifact-permit nonce; change learning/allow artifact-permit approved approved; \
  rm "$$model_path"; denied teach.inactive activate new-round fit; test ! -e "$$work/private/active-adapter"; \
  provider=$$(./ck run ground/ed25519.ck); unavailable=$$(printf ed25519.unavailable | $(CANONICAL_WORDS)); \
  if [ "$$provider" = "node $$unavailable" ]; then cp "$$work/weights" "$$model_path"; denied teach.unavailable activate new-round fit; test ! -e "$$work/private/active-adapter"; echo "learning: missing verification provider kept the adapter inactive"; exit 0; fi; \
  printf 'changed' > "$$model_path"; denied teach.mismatch activate new-round fit; test ! -e "$$work/private/active-adapter"; \
  cp "$$work/weights" "$$model_path"; activate new-round fit > "$$work/selection"; use selection > "$$work/verified"; cmp "$$work/weights" "$$work/verified"; \
  denied teach.wrong-location use selection elsewhere-channel; \
  printf 'changed' > "$$model_path"; denied teach.mismatch use selection; cp "$$work/weights" "$$model_path"; \
  change learning/allow none approved held; denied share.ungranted use selection; \
  change learning/allow artifact-permit held approved; denied teach.stale use selection; \
  activate new-round fit > "$$work/selection"; change circle/authorize none issued stopped; denied share.ungranted use selection; \
  echo "learning: recall-only, held evidence, withdrawal, later grants and changed model bytes denied; selected fixture bytes reached only the explicit local consumer"
endef
