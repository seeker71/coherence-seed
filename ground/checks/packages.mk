# The component packages shown in every language. Each package nums/write.ck lists below is written in Python, C#,
# TypeScript, Java and BML by nums/write-text.ck, which links the libraries nums/libraries.ck lists for it. A render that
# ends otherwise than by answering text (a fault, a signal, nothing) is refused. The Java of each names its own package and
# class, no two the same, and every record, interface and function in it is preceded by its Javadoc line. Each rule is run
# against a planted file and must refuse it. Where a JDK is found, the twelve are also compiled against a runtime that
# nums/runtime-text.ck writes from the nodes (the engine operations of form/operations.ck, the part names of the lexicon):
# the errors that remain are the Java's own, and they may not exceed JAVA_ERRORS, which is lowered and never raised.
# No word of lens/words.ck is written twice (nums/duplicate-words.ck lists them), and a package with no row in
# nums/packages.ck faults and does not write a header with nothing in it.
JAVA_ERRORS = 228
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
  test -z "$$(printf 'lens/words.ck' | ./ck text nums/duplicate-words.ck)" || { echo "a word of lens/words.ck is written twice"; exit 1; }; \
  printf '(bind lens.words {| (quote (list (list "a" root) (list "b" root) (list "a" root)))})' | ./ck text lens/to-nodes.ck lens/all.ck > "$$work/twice.ck"; \
  test "$$(printf '%s' "$$work/twice.ck" | ./ck text nums/duplicate-words.ck)" = a || { echo "a word written twice was not listed"; exit 1; }; \
  printf '%s\n' lens/participation.ck nums/lang-python.ck nums/view-patterns.ck bmf/items.ck | ./ck text nums/write-text.ck > "$$work/unlisted.python" 2>&1; \
  if grep -q '^fault ' "$$work/unlisted.python" || ! test -s "$$work/unlisted.python"; then echo "a language that names no package refused a package with no row in nums/packages.ck"; exit 1; fi; \
  printf '%s\n' lens/participation.ck nums/lang-java.ck nums/view-patterns.ck bmf/items.ck | ./ck text nums/write-text.ck 2>&1 | head -1 | grep -q '^fault ' || { echo "a package with no row in nums/packages.ck was written with a header of nothing"; exit 1; }; \
  jhome=$${JAVA_HOME:-}; javac=$${jhome:+$$jhome/bin/}javac; \
  if "$$javac" -version > /dev/null 2>&1; then \
    parses() { "$$javac" -XDshould-stop.ifError=PARSE -XDshould-stop.ifNoError=PARSE -proc:none -d "$$work/classes" "$$@" > "$$work/javac.out" 2>&1; }; \
    jfiles=; for p in $$packages; do jfiles="$$jfiles $$work/$$(basename $$p .ck).java"; done; \
    parses $$jfiles; \
    printf 'public final class P {\n    static void f() {\n        true;\n    }\n}\n' > "$$work/Planted.java"; \
    if parses "$$work/Planted.java"; then echo "javac took a statement that is only true; for Java"; exit 1; fi; \
    echo "packages: the Java of the twelve parses (JDK $$("$$javac" -version 2>&1))"; \
    tree="$$work/tree"; mkdir -p "$$tree/form"; \
    for p in $$packages; do f="$$work/$$(basename $$p .ck).java"; pkg=$$(sed -n 's/^package \(.*\);$$/\1/p' "$$f"); cls=$$(sed -n 's/^public final class \([A-Za-z]*\) {$$/\1/p' "$$f"); mkdir -p "$$tree/$$(echo "$$pkg" | tr . /)"; cp "$$f" "$$tree/$$(echo "$$pkg" | tr . /)/$$cls.java"; done; \
    for c in Entry Blueprinted Fault Recipe Node Runtime; do printf '%s' "$$c" | ./ck text nums/runtime-text.ck > "$$tree/form/$$c.java"; done; \
    if printf 'Nothing' | ./ck text nums/runtime-text.ck | grep -q '^fault '; then :; else echo "the runtime writer answered for a class it has no row for"; exit 1; fi; \
    errors_of() { status=0; out=$$("$$javac" -proc:none -Xmaxerrs 100000 -d "$$work/built" "$$@" 2>&1) || status=$$?; case $$status in 0) echo 0;; 1) n=$$(printf '%s\n' "$$out" | sed -n 's/^\([0-9][0-9]*\) errors\{0,1\}$$/\1/p' | tail -1); echo "$${n:-999999}";; *) echo 999999;; esac; }; \
    mkdir -p "$$work/probe/form"; cp "$$tree"/form/*.java "$$work/probe/form/"; \
    printf 'import static form.Runtime.*;\nclass Clean {\n    static long f(Object x) {\n        return count(x);\n    }\n}\n' > "$$work/probe/Clean.java"; \
    printf 'class Broken {\n    static int f() {\n        return "text";\n    }\n}\n' > "$$work/probe/Broken.java"; \
    test "$$(errors_of "$$work"/probe/form/*.java "$$work/probe/Clean.java")" = 0 || { echo "the runtime made from the nodes does not declare count"; exit 1; }; \
    test "$$(errors_of "$$work"/probe/form/*.java "$$work/probe/Broken.java")" -ge 1 || { echo "javac took a type error for Java"; exit 1; }; \
    errors=$$(errors_of $$(find "$$tree" -name '*.java')); \
    test "$$errors" -le $(JAVA_ERRORS) || { echo "packages: $$errors javac errors in the Java of the twelve, over the budget of $(JAVA_ERRORS)"; exit 1; }; \
    echo "packages: the Java of the twelve compiles against the runtime made from the nodes with $$errors errors of its own, within the budget of $(JAVA_ERRORS)"; \
  else echo "packages: the Java was not parsed: no JDK found (set JAVA_HOME)"; fi; \
  if python3 -I -c 'import ast' > /dev/null 2>&1; then \
    parses_python() { python3 -I -c 'import ast,sys; ast.parse(open(sys.argv[1]).read())' "$$1" > /dev/null 2>&1; }; \
    for f in "$$work"/*.python; do parses_python "$$f" || { echo "the Python of $$(basename "$$f") does not parse"; exit 1; }; done; \
    printf 'if x:\nelse:\n    y\n' > "$$work/planted.py"; \
    if parses_python "$$work/planted.py"; then echo "python took an empty block"; exit 1; fi; \
    echo "packages: the Python of the twelve parses"; \
  else echo "packages: the Python was not parsed: no python3 found"; fi; \
  echo "packages: each package written in five languages, none faulting; each Java names its own package and class and documents every part"
endef
