define verify_projection
@set -eu; \
  work=$$(mktemp -d); \
  trap 'rm -rf "$$work"' EXIT; \
  ./ck text ground/search-trace.ck > "$$work/reference"; \
  module() { printf '%s\n' ground/search-package.ck "nums/lang-$$1.ck" | ./ck text nums/write-text.ck > "$$2"; }; \
  adapter() { printf %s "$$1" | ./ck text ground/search-targets.ck; }; \
  compare() { test "$$(cat "$$work/actual")" = "$$(cat "$$work/reference")" || { diff -u "$$work/reference" "$$work/actual"; return 1; }; }; \
  if command -v python3 >/dev/null 2>&1; then \
  module python "$$work/search.py"; adapter python-runtime > "$$work/form.py"; adapter python-reader >> "$$work/search.py"; \
  python3 "$$work/search.py" > "$$work/actual"; compare; echo "projection: Python observed all five native answer and effect traces"; \
  fi; \
  if command -v deno >/dev/null 2>&1; then \
  module typescript "$$work/search.ts"; adapter typescript-runtime > "$$work/form.ts"; adapter typescript-reader >> "$$work/search.ts"; \
  deno run "$$work/search.ts" > "$$work/actual"; compare; echo "projection: TypeScript observed all five native answer and effect traces"; \
  fi; \
  jhome=$${JAVA_HOME:-}; javac=$${jhome:+$$jhome/bin/}javac; java=$${jhome:+$$jhome/bin/}java; \
  if "$$javac" -version >/dev/null 2>&1; then \
  mkdir -p "$$work/j/form/write"; module java "$$work/j/form/write/Writer.java"; \
  for type in Node Entry Blueprinted Fault Runtime; do adapter "java-$$type" > "$$work/j/form/$$type.java"; done; \
  adapter java-reader > "$$work/j/Check.java"; "$$javac" "$$work/j"/form/*.java "$$work/j/form/write/Writer.java" "$$work/j/Check.java"; \
  "$$java" -cp "$$work/j" Check > "$$work/actual"; compare; echo "projection: Java observed all five native answer and effect traces"; \
  fi; \
  if command -v dotnet >/dev/null 2>&1 && version=$$(dotnet --version 2>/dev/null); then \
  mkdir "$$work/cs"; module csharp "$$work/cs/Writer.cs"; adapter csharp-runtime > "$$work/cs/Program.cs"; \
  printf '<Project Sdk="Microsoft.NET.Sdk"><PropertyGroup><OutputType>Exe</OutputType><TargetFramework>net%s.0</TargetFramework></PropertyGroup></Project>\n' "$${version%%.*}" > "$$work/cs/Fixture.csproj"; \
  dotnet build "$$work/cs/Fixture.csproj" -o "$$work/net" --nologo --verbosity quiet; \
  dotnet "$$work/net/Fixture.dll" > "$$work/actual"; compare; echo "projection: C# observed all five native answer and effect traces"; \
  fi;
endef
