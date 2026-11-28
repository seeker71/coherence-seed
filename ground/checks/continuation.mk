define verify_resume
@set -eu; \
  work=$$(mktemp -d); \
  trap 'rm -rf "$$work"' EXIT; \
  printf '#32' >"$$work/none"; printf '[#2 31]' >"$$work/one"; printf '[#2 32]' >"$$work/two"; printf '[#2 63616c6c6572]' >"$$work/caller"; printf '[#2 6775657374]' >"$$work/other"; printf %s "$$work" >"$$work/location"; \
  word() { printf %s "$$1" | ./ck text lens/to-nodes.ck lens/all.ck; }; \
  value() { printf %s "$$2" >"$$work/mode"; ./ck text "ground/$$1.ck" 3<"$$work/mode" 4<"$$work/$${5:-none}" <"$$work/$$3" >"$$work/$$4"; }; \
  g() { value circle-exchange "$$@"; }; \
  c() { value continuation-case "$$@"; }; \
  run() { ./ck text "continuation/$$1.ck" 3<"$$work/channel" 4<"$$work/$${3:-caller}" 5<"$$work/$${4:-none}" <"$$work/$${2:-job}"; }; \
  control() { g change "$$2" change "$$3"; ./ck text "circle/$$1.ck" 3<"$$work/channel" <"$$work/change" >"$$work/$$4"; }; \
  refused() { why=$$1; shift; rc=0; "$$@" >"$$work/failed" || rc=$$?; test "$$rc" = 1 && test "$$(cat "$$work/failed")" = "fault $$(word "$$why")" || { echo "care expected $$why, received $$(cat "$$work/failed")" >&2; return 1; }; }; \
  reference() { printf %s "$$1" >"$$work/mode"; actual=$$(./ck text ground/continuation-reference.ck 3<"$$work/mode" 4<"$$work/job" 5<"$$work/a-owner" 6<"$$work/a-listener" 7<"$$work/b-owner" 8<"$$work/b-listener" <"$$work/$$2"); test "$$actual" = "$$(word yes)" || { echo "care expected $$1 reference, received $$actual" >&2; return 1; }; }; \
  for scene in normal orphan repaired; do \
    mkdir "$$work/$$scene"; destination="$$work/$$scene"; mkdir "$$destination/private" "$$destination/letters"; printf %s "$$destination" >"$$work/location"; g channel location channel; \
    cp "$$work/none" "$$destination/private/authority"; cp "$$work/none" "$$destination/private/listening"; \
    g identity none genesis; c first channel first-scope; c second channel second-scope; \
    g permit first-scope first-permit one; g ticket first-scope first-ticket one; g permit second-scope second-permit two; g ticket second-scope second-ticket two; \
    refused share.different ./ck text continuation/action.ck 3<"$$work/second-ticket" <"$$work/first-scope"; \
    ./ck text continuation/action.ck 3<"$$work/first-ticket" <"$$work/first-scope" >"$$work/first-action"; ./ck text continuation/action.ck 3<"$$work/second-ticket" <"$$work/second-scope" >"$$work/second-action"; c pair first-action actions second-action; \
    control authorize first-permit genesis owner-a; control listen first-ticket genesis listener-a; \
    cp "$$destination/private/authority" "$$work/a-owner"; cp "$$destination/private/listening" "$$work/a-listener"; cp "$$work/none" "$$work/b-owner"; cp "$$work/none" "$$work/b-listener"; \
    run plan actions >"$$work/job"; c location job head-label; head_file="$$destination/private/$$(cat "$$work/head-label")"; \
    if test "$$scene" = repaired; then \
      c first-path job expected-path; mv "$$destination/letters" "$$destination/offline"; \
      run begin >"$$work/io-blocked"; c cause io-blocked io-cause; expected_path=$$(cat "$$work/expected-path"); \
      test "$$(cat "$$work/io-cause")" = "$$(word "(list inbox.io \"publish\" \"$$expected_path\")")"; refused care.unchanged run wake; \
      mv "$$destination/offline" "$$destination/letters"; run wake >"$$work/restored"; run resume job caller restored >"$$work/after-io"; \
      c cause after-io next-cause; test "$$(cat "$$work/next-cause")" = "$$(word share.different)"; set -- "$$destination/letters/"*; test "$$#" = 1; \
      g bytes "$$scene/letters/$${1##*/}" observed; printf 'first\0' | cmp - "$$work/observed"; continue; \
    fi; \
    if test "$$scene" = normal; then \
      c wrong-id first-action lied; c pair lied wrong-actions second-action; run plan wrong-actions >"$$work/wrong-job"; run begin wrong-job >"$$work/wrong-result"; c cause wrong-result wrong-cause; \
      test "$$(cat "$$work/wrong-cause")" = "$$(word care.mismatch)"; test -z "$$(ls -A "$$destination/letters")"; \
      run begin >"$$work/blocked"; test -f "$$head_file"; cmp "$$work/blocked" "$$head_file"; reference blocked blocked; \
      run resume | cmp - "$$work/blocked"; refused care.unchanged run wake; \
    else \
      c first-path job target; rc=0; ./ck text continuation/begin.ck ground/continuation-interrupt.ck 6<"$$work/target" 3<"$$work/channel" 4<"$$work/caller" <"$$work/job" >"$$work/lost" || rc=$$?; \
      test "$$rc" = 76; test ! -s "$$work/lost"; test -s "$$head_file"; \
    fi; \
    set -- "$$destination/letters/"*; test "$$#" = 1; first_path=$$1; ln "$$first_path" "$$destination/first-proof"; g bytes "$$scene/first-proof" observed; printf 'first\0' | cmp - "$$work/observed"; \
    control authorize second-permit owner-a owner-b; cp "$$destination/private/authority" "$$work/b-owner"; \
    if test "$$scene" = normal; then \
      run wake >"$$work/early"; refused care.wrong-owner run resume job other early; \
    fi; \
    control listen second-ticket listener-a listener-b; cp "$$destination/private/listening" "$$work/b-listener"; \
    if test "$$scene" = normal; then \
      run resume | cmp - "$$work/blocked"; refused care.stale run resume job caller early; \
      run wake >"$$work/event"; c other-owner channel other-channel; \
      refused care.wrong-owner ./ck text continuation/resume.ck 3<"$$work/other-channel" 4<"$$work/caller" 5<"$$work/event" <"$$work/job"; \
      mv "$$first_path" "$$destination/temporarily-absent"; refused care.absent run resume job caller event; cmp "$$work/blocked" "$$head_file"; test -z "$$(ls -A "$$destination/letters")"; mv "$$destination/temporarily-absent" "$$first_path"; \
      c second-path job target; rc=0; ./ck text continuation/resume.ck ground/continuation-interrupt.ck 3<"$$work/channel" 4<"$$work/caller" 5<"$$work/event" 6<"$$work/target" <"$$work/job" >"$$work/lost" || rc=$$?; \
      test "$$rc" = 76; test ! -s "$$work/lost"; set -- "$$destination/letters/"*; test "$$#" = 2; \
      refused care.stale run resume job caller event; run resume >"$$work/finished"; reference finished finished; \
      refused care.stale run resume job caller event; \
    else \
      refused care.wrong-owner run resume job other; run resume >"$$work/finished"; reference orphan finished; \
    fi; \
    run resume | cmp - "$$work/finished"; run begin | cmp - "$$work/finished"; \
    set -- "$$destination/letters/"*; test "$$#" = 2; if test "$$1" -ef "$$destination/first-proof"; then second=$$2; else test "$$2" -ef "$$destination/first-proof"; second=$$1; fi; \
    printf bytes >"$$work/mode"; ./ck text ground/circle-exchange.ck 3<"$$work/mode" <"$$second" >"$$work/observed"; printf 'second\n' | cmp - "$$work/observed"; \
  done; \
  mkdir "$$work/crossed" "$$work/crossed/private" "$$work/crossed/letters"; printf %s "$$work/crossed" >"$$work/location"; g channel location alpha-channel; c other-owner alpha-channel beta-channel; \
  cp "$$work/none" "$$work/crossed/private/authority"; cp "$$work/none" "$$work/crossed/private/listening"; cp "$$work/caller" "$$work/alpha-caller"; cp "$$work/other" "$$work/beta-caller"; \
  g identity none source-head; cp "$$work/source-head" "$$work/listen-head"; \
  pair_call() { ./ck text "continuation/$$2.ck" 3<"$$work/$$1-channel" 4<"$$work/$$1-caller" 5<"$$work/$${3:-none}" <"$$work/$$1-job"; }; \
  pair_rights() { cp "$$work/$$1-channel" "$$work/channel"; control authorize "$$1-$$2-permit" source-head source-head; control listen "$$1-$$2-ticket" listen-head listen-head; }; \
  pair_delivered() { c "$$2-path" "$$1-job" pair-path; printf bytes >"$$work/mode"; ./ck text ground/circle-exchange.ck 3<"$$work/mode" <"$$(cat "$$work/pair-path")" >"$$work/actual-bytes"; printf '%s' "$$1/$$2" | cmp - "$$work/actual-bytes"; }; \
  for who in alpha beta; do \
    for slot in first second; do \
      marker=$$(printf '%s' "$$who/$$slot" | od -An -v -tx1 | tr -d ' \n'); printf '[#1 %s]' "$$marker" >"$$work/marker"; c "$$slot" "$$who-channel" "$$who-$$slot-scope" marker; \
      nonce=one; if test "$$slot" = second; then nonce=two; fi; g permit "$$who-$$slot-scope" "$$who-$$slot-permit" "$$nonce"; g ticket "$$who-$$slot-scope" "$$who-$$slot-ticket" "$$nonce"; \
      ./ck text continuation/action.ck 3<"$$work/$$who-$$slot-ticket" <"$$work/$$who-$$slot-scope" >"$$work/$$who-$$slot-action"; \
    done; \
    c pair "$$who-first-action" "$$who-actions" "$$who-second-action"; ./ck text continuation/plan.ck 3<"$$work/$$who-channel" 4<"$$work/$$who-caller" <"$$work/$$who-actions" >"$$work/$$who-job"; \
    pair_call "$$who" begin >"$$work/$$who-wait"; c cause "$$who-wait" pair-cause; test "$$(cat "$$work/pair-cause")" = "$$(word share.ungranted)"; \
  done; \
  pair_rights alpha first; pair_call alpha wake >"$$work/wake-alpha"; refused care.unchanged pair_call beta wake; \
  pair_rights beta first; pair_call beta wake >"$$work/wake-beta"; refused care.unchanged pair_call alpha wake; \
  refused care.stale pair_call alpha resume wake-beta; refused care.stale pair_call beta resume wake-alpha; refused care.stale pair_call alpha resume wake-alpha; \
  for who in alpha beta; do pair_call "$$who" resume | cmp - "$$work/$$who-wait"; done; test -z "$$(ls -A "$$work/crossed/letters")"; \
  pair_call beta resume wake-beta >"$$work/beta-middle"; pair_delivered beta first; pair_call alpha resume | cmp - "$$work/alpha-wait"; \
  pair_rights beta second; pair_call beta wake >"$$work/updated"; pair_call beta resume updated >"$$work/beta-finished"; pair_delivered beta second; refused care.unchanged pair_call alpha wake; \
  for slot in first second; do pair_rights alpha "$$slot"; pair_call alpha wake >"$$work/updated"; pair_call alpha resume updated >"$$work/alpha-last"; pair_delivered alpha "$$slot"; done; \
  pair_call beta resume | cmp - "$$work/beta-finished"; pair_call alpha resume | cmp - "$$work/alpha-last"; set -- "$$work/crossed/letters/"*; test "$$#" = 4; \
  echo "continuation: beta finished before alpha in one store; crossed wakes left both checkpoints unchanged and all four owner-specific letters retained their exact bytes"; \
  clock_call() { word "$$2" >"$$work/now"; ./ck text "continuation/$$1.ck" ground/broker-clock.ck 3<"$$work/channel" 4<"$$work/caller" 5<"$$work/$${3:-none}" 7<"$$work/now" <"$$work/job"; }; \
  for scene in allowed expired; do \
    mkdir "$$work/$$scene" "$$work/$$scene/private" "$$work/$$scene/letters"; printf %s "$$work/$$scene" >"$$work/location"; g channel location channel; \
    cp "$$work/none" "$$work/$$scene/private/authority"; cp "$$work/none" "$$work/$$scene/private/listening"; g identity none genesis; \
    c first channel first-scope; c second channel second-scope; g permit first-scope first-permit one; g ticket first-scope first-ticket one; g ticket second-scope second-ticket two; \
    word '(list 100 110)' >"$$work/window"; c finite first-permit timed-permit window; control authorize timed-permit genesis owner-a; control listen first-ticket genesis listener-a; \
    for slot in first second; do ./ck text continuation/action.ck 3<"$$work/$$slot-ticket" <"$$work/$$slot-scope" >"$$work/$$slot-action"; done; \
    c pair first-action actions second-action; run plan actions >"$$work/job"; clock_call begin 99 >"$$work/early"; c cause early cause; test "$$(cat "$$work/cause")" = "$$(word share.early)"; \
    refused care.unchanged clock_call wake 99; clock_call wake 100 >"$$work/edge"; \
    if test "$$scene" = expired; then \
      refused care.stale clock_call resume 110 edge; run resume | cmp - "$$work/early"; test -z "$$(ls -A "$$work/$$scene/letters")"; \
    else \
      clock_call resume 100 edge >"$$work/middle"; c first-path job path; printf bytes >"$$work/mode"; ./ck text ground/circle-exchange.ck 3<"$$work/mode" <"$$(cat "$$work/path")" >"$$work/actual"; printf 'first\0' | cmp - "$$work/actual"; \
    fi; \
  done; \
  echo "continuation: changing only the time woke the exact finite grant at 100; the same signal at the exclusive cutoff 110 left the original checkpoint and destination untouched"; \
  echo "continuation: two intended letters, original consent and caller retained; dropped and stale wakes kept the checkpoint; another receiver reconciled an unacknowledged link"
endef
