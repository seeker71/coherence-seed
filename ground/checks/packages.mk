# The component packages shown in every language. Each of the twelve packages is written in Python, C#, TypeScript, Java and
# BML by nums/write-text.ck, which links the libraries nums/libraries.ck lists for it; none may fault. The Java of each
# names its own package and class (nums/packages.ck), no two the same, and every record, interface and function in it is
# preceded by its Javadoc line. The Javadoc rule is run against a planted file and must refuse it.
define verify_packages
@set -eu; \
  work=$$(mktemp -d); \
  trap 'rm -rf "$$work"' EXIT; \
  ulimit -s 65520; export CK_MEMORY_BYTES=134217728; \
  packages="nums/write.ck nums/arm.ck nums/compile.ck nums/part.ck nums/resolve.ck nums/classes.ck bmf/items.ck nums/bml-methods.ck nums/bml-link.ck nums/view-patterns.ck nums/bml-alphabet.ck lens/identifier-model.ck"; \
  render() { printf '%s\nnums/lang-%s.ck\n' "$$1" "$$2" | ./ck text nums/write-text.ck > "$$work/$$(basename "$$1" .ck).$$2" 2>&1 || true; }; \
  (for p in $$packages; do render $$p java; render $$p python; done) & \
  (for p in $$packages; do render $$p csharp; render $$p typescript; done) & \
  (for p in $$packages; do render $$p bml; done) & \
  wait; \
  count=0; for f in "$$work"/*; do count=$$((count + 1)); if head -c 5 "$$f" | grep -q fault; then echo "  $$f did not render: $$(head -c 200 "$$f")"; exit 1; fi; done; \
  test "$$count" = 60; \
  for f in "$$work"/*.java; do printf '%s %s\n' "$$(sed -n 's/^package \(.*\);$$/\1/p' "$$f")" "$$(sed -n 's/^public final class \([A-Za-z]*\) {$$/\1/p' "$$f")"; done > "$$work/names"; \
  test "$$(grep -c '^form\.[a-z]* [A-Z][A-Za-z]*$$' "$$work/names")" = 12; test "$$(sort -u "$$work/names" | wc -l | tr -d ' ')" = 12; \
  documented() { awk '/^ +(private |public )?(static [^=;]*\(|record |interface |sealed interface )/ { if (prev !~ /\*\/ *$$/) { print FILENAME ": " $$0; bad = 1 } } { prev = $$0 } END { exit bad }' "$$@"; }; \
  documented "$$work"/*.java; \
  printf '    static long f(long x) {\n' > "$$work/planted.java"; \
  if documented "$$work/planted.java" > /dev/null; then echo "the Javadoc rule accepted a function without one"; exit 1; fi; \
  echo "packages: twelve packages written in five languages, none faulting; each Java names its own package and class and documents every part"
endef
