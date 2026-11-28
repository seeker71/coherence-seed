# The component packages shown in every language. Each package nums/write.ck lists below is written in Python, C#,
# TypeScript, Java and BML by nums/write-text.ck, which links the libraries nums/libraries.ck lists for it. A render that
# ends otherwise than by answering text (a fault, a signal, nothing) is refused. The Java of each names its own package and
# class, no two the same, and every record, interface and function in it is preceded by its Javadoc line. Each rule is run
# against a planted file and must refuse it.
define verify_packages
@set -eu; \
  work=$$(mktemp -d); \
  trap 'rm -rf "$$work"' EXIT; \
  ulimit -s 65520; export CK_MEMORY_BYTES=134217728; \
  packages="nums/write.ck nums/arm.ck nums/compile.ck nums/part.ck nums/resolve.ck nums/classes.ck bmf/items.ck nums/bml-methods.ck nums/bml-link.ck nums/view-patterns.ck nums/bml-alphabet.ck lens/identifier-model.ck"; \
  render() { name="$$work/$$(basename "$$1" .ck).$$2"; if printf '%s\nnums/lang-%s.ck\n' "$$1" "$$2" | ./ck text nums/write-text.ck > "$$name" 2>&1; then echo 0 > "$$name.rc"; else echo 1 > "$$name.rc"; fi; }; \
  (for p in $$packages; do render $$p java; render $$p python; done) & \
  (for p in $$packages; do render $$p csharp; render $$p typescript; done) & \
  (for p in $$packages; do render $$p bml; done) & \
  wait; \
  answered() { for f in "$$@"; do case "$$f" in *.rc) continue;; esac; test "$$(cat "$$f.rc")" = 0 && test -s "$$f" && ! grep -q '^fault ' "$$f" || { echo "  $$f did not answer text"; return 1; }; done; }; \
  answered "$$work"/*.python "$$work"/*.csharp "$$work"/*.typescript "$$work"/*.java "$$work"/*.bml; \
  printf 'fault (#16)\n' > "$$work/planted"; echo 0 > "$$work/planted.rc"; \
  if answered "$$work/planted" > /dev/null; then echo "a fault was taken for an answer"; exit 1; fi; \
  : > "$$work/empty"; echo 0 > "$$work/empty.rc"; \
  if answered "$$work/empty" > /dev/null; then echo "an empty answer was taken for text"; exit 1; fi; \
  for f in "$$work"/*.java; do printf '%s %s\n' "$$(sed -n 's/^package \(.*\);$$/\1/p' "$$f")" "$$(sed -n 's/^public final class \([A-Za-z]*\) {$$/\1/p' "$$f")"; done > "$$work/names"; \
  test "$$(grep -c '^form\.[a-z]* [A-Z][A-Za-z]*$$' "$$work/names")" = "$$(wc -l < "$$work/names" | tr -d ' ')"; \
  test "$$(sort -u "$$work/names" | wc -l | tr -d ' ')" = "$$(wc -l < "$$work/names" | tr -d ' ')"; \
  documented() { awk '/^ +(private |public )?(static [^=;]*\(|record |interface |sealed interface )/ { if (prev !~ /\*\/ *$$/) { print FILENAME ": " $$0; bad = 1 } } { prev = $$0 } END { exit bad }' "$$@"; }; \
  documented "$$work"/*.java; \
  printf '    static long f(long x) {\n' > "$$work/planted.java"; \
  if documented "$$work/planted.java" > /dev/null; then echo "the Javadoc rule accepted a function without one"; exit 1; fi; \
  refers() { grep -q '^import static form\.bmf\.Items\.\*;$$' "$$1" && ! grep -q 'record Parser' "$$1"; }; \
  refers "$$work/part.java"; refers "$$work/bml-link.java"; refers "$$work/bml-methods.java"; \
  printf 'public record Parser() {}\n' > "$$work/copy.java"; \
  if refers "$$work/copy.java"; then echo "a copy of a linked package was taken for a reference to it"; exit 1; fi; \
  printf 'package form.a;\npublic final class A {\n' > "$$work/one.java"; printf 'package form.a;\npublic final class A {\n' > "$$work/two.java"; \
  test "$$(for f in "$$work/one.java" "$$work/two.java"; do printf '%s %s\n' "$$(sed -n 's/^package \(.*\);$$/\1/p' "$$f")" "$$(sed -n 's/^public final class \([A-Za-z]*\) {$$/\1/p' "$$f")"; done | sort -u | wc -l | tr -d ' ')" = 1; \
  echo "packages: each package written in five languages, none faulting; each Java names its own package and class and documents every part"
endef
