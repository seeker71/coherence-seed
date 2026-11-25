# Study: bootstrapping a small system to the hardware's full efficiency (state of the art through 2026)

Study date: 2026-11-24. All URLs below were fetched on 2026-11-24 unless a line says otherwise.
Scope: JIT and compilation techniques, small self-hosting systems, image persistence, tiering,
machine encodings, SIMD/GPU paths, memory, and energy, read for one question: how does a system whose
JIT is written in its own single grammar reach hardware speed while the stored bootstrap shrinks to
almost nothing?

## 0. How to read this report

Status words used on every claim:

- **observed**: I fetched the page or ran the command in this session and the claim is on it.
  WebFetch returns a small model's reading of the page, so each "observed" claim is one reading step
  away from the bytes; exact quotes are marked with quotation marks and were returned as quotes.
- **inferred**: my reasoning from observed things; the line names which.
- **assumed**: model memory, not verified in this session. Treat as a pointer to check, not a fact.

Limits of this study, stated once and plainly:

- I did not read full paper bodies, only abstract pages and project pages (PDF bodies arrive as
  bytes through the fetch tool; I read six pages of the STEPS report as page images).
- Lines-of-code numbers are only given where a primary page states them. I did not clone and count
  any repository (downloads were out of scope), so several well-known sizes (chibicc, QBE, tcc,
  LuaJIT) are "assumed" or absent.
- Fetches that failed: the Guix full-source-bootstrap post (HTTP 403), cproc on sr.ht (HTTP 502),
  lmdb.tech (TLS error), LuaJIT/DynASM pages returned little detail.
- Nothing in the repositories was modified.

### Source table

| ID | Source | Source's own date | URL |
|---|---|---|---|
| S1 | Xu, Kjolstad, "Copy-and-Patch Compilation" (arXiv 2011.13127; DOI 10.1145/3485513) | v1 2020-11-26, v3 2021-09-15 | https://arxiv.org/abs/2011.13127 |
| S2 | PEP 744, "JIT Compilation" (CPython) | created 2024-04-11, status Draft | https://peps.python.org/pep-0744/ |
| S3 | What's New in Python 3.14 | release 2025-10-07 | https://docs.python.org/3.14/whatsnew/3.14.html |
| S4 | What's New in Python 3.15 (3.15.2 docs) | release 2026-10-09 | https://docs.python.org/3.15/whatsnew/3.15.html |
| S5 | PEP 836, "JIT Go Brrr: The Path to a Supported JIT Compiler for CPython" | created 2026-07-02, Withdrawn 2026-10-19 | https://peps.python.org/pep-0836/ |
| S6 | LWN, "A pause for the Python JIT" | 2026-09-01 | https://lwn.net/Articles/1090385/ |
| S7 | discuss.python.org PEP 836 thread, posts 41-80 | 2026-07-17 to 2026-10-30 | https://discuss.python.org/t/pep-836-jit-go-brrr-the-path-to-a-supported-jit-compiler-for-cpython/108010 |
| S8 | "Two JITs, One Problem" (personal blog; secondary) | 2026-04-05 | https://henry-the-frog.github.io/2026/04/05/jit-comparison/ |
| S9 | Xu, Kjolstad, "Deegen: A JIT-Capable VM Generator for Dynamic Languages" (arXiv 2411.11469) | 2024-11-18, v2 2024-11-24 | https://arxiv.org/abs/2411.11469 |
| S10 | Xu, "Building a baseline JIT for Lua automatically" | 2023-05-12 | https://sillycross.github.io/2023/05/12/2023-05-12/ |
| S11 | Schwarz, Kamm, Engelke, "TPDE: A Fast Adaptable Compiler Back-End Framework" (arXiv 2505.22610) | 2025-05-28 | https://arxiv.org/abs/2505.22610 |
| S12 | Titzer, "Whose baseline compiler is it anyway?" (arXiv 2305.13241) | 2023-05-22, v2 2023-11-05 | https://arxiv.org/abs/2305.13241 |
| S13 | Titzer, "A fast in-place interpreter for WebAssembly" (arXiv 2205.01183) | 2022-05-02 | https://arxiv.org/abs/2205.01183 |
| S14 | Palumbo et al., "Meta-compilation of Baseline JIT Compilers with Druid" (arXiv 2502.20543) | 2025-02-27 | https://arxiv.org/abs/2502.20543 |
| S15 | Chevalier-Boisvert, Feeley, "Lazy Basic Block Versioning" (arXiv 1411.0352) | 2014-11-03, v2 2015-05-29 | https://arxiv.org/abs/1411.0352 |
| S16 | Ruby 4.0.0 release note (ZJIT) | 2025-12-25 | https://www.ruby-lang.org/en/news/2025/12/25/ruby-4-0-0-released/ |
| S17 | V8 blog, "Sparkplug" | 2021-05-27 | https://v8.dev/blog/sparkplug |
| S18 | V8 blog, "Maglev - V8's Fastest Optimizing JIT" | 2023-12-05 | https://v8.dev/blog/maglev |
| S19 | V8 blog, "Land ahoy: leaving the Sea of Nodes" | 2025-03-25 | https://v8.dev/blog/leaving-the-sea-of-nodes |
| S20 | V8 blog index (latest post listed: 2025-08-04) | 2025-08-04 | https://v8.dev/blog |
| S21 | SeaOfNodes/Simple repository README | footer 2026, 877 commits | https://github.com/SeaOfNodes/Simple |
| S22 | Willsey et al., "egg: Fast and Extensible Equality Saturation" (arXiv 2004.03082, POPL 2021) | 2020-04-07, v3 2020-11-07 | https://arxiv.org/abs/2004.03082 |
| S23 | Zhang et al., "Better Together: Unifying Datalog and Equality Saturation" (arXiv 2304.04332, PLDI 2023) | 2023-04-10 | https://arxiv.org/abs/2304.04332 |
| S24 | egglog repository README (version 3.0.0 in example) | undated | https://github.com/egraphs-good/egglog |
| S25 | Cranelift e-graph RFC (aegraph) | undated on page; references 2022 roadmap | https://github.com/bytecodealliance/rfcs/blob/main/accepted/cranelift-egraph.md |
| S26 | cranelift.dev | undated | https://cranelift.dev/ |
| S27 | Schkufza, Sharma, Aiken, "Stochastic Superoptimization" (arXiv 1211.0557, ASPLOS 2013) | 2012-11-02 | https://arxiv.org/abs/1211.0557 |
| S28 | Sasnauskas et al., "Souper: A Synthesizing Superoptimizer" (arXiv 1711.04422) | 2017-11-13, v2 2018-04-06 | https://arxiv.org/abs/1711.04422 |
| S29 | QBE home and releases pages | 1.3 on 2026-06-01; 1.0 on 2022-06-22 | https://c9x.me/compile/ , https://c9x.me/compile/releases.html |
| S30 | chibicc README | new version uploaded 2020-09 | https://github.com/rui314/chibicc |
| S31 | TinyCC page (bellard.org) | copyright 2001-2018 | https://bellard.org/tcc/ |
| S32 | Maru page (Piumarta) | 2010-2012; v2.4 2012-08-24 | https://piumarta.com/software/maru/ |
| S33 | VPRI TR-2012-001, STEPS 2012 Final Report, pages 4-6 (read as page images) | 2012-10 | https://tinlizzie.org/VPRIPapers/tr2012001_steps.pdf |
| S34 | Ingalls et al., "Back to the Future: The Story of Squeak" | 1997 (OOPSLA) | http://files.squeak.org/docs/OOPSLA.Squeak.html |
| S35 | Project Oberon (2013 edition) download index | 2013 | https://people.inf.ethz.ch/wirth/ProjectOberon/ |
| S36 | sectorforth README | undated | https://github.com/cesarblum/sectorforth |
| S37 | Dusk OS home | undated | https://duskos.org/ |
| S38 | stage0-posix README | undated | https://github.com/oriansj/stage0-posix |
| S39 | RPython docs, JIT overview | describes the 2009 move to tracing | https://rpython.readthedocs.io/en/latest/jit/overview.html |
| S40 | pypy.org front page | site built 2026-11-12 | https://www.pypy.org/ |
| S41 | Marr, Ducasse, "Tracing vs. Partial Evaluation" (OOPSLA 2015) | 2015 | https://stefan-marr.de/papers/oopsla-marr-ducasse-meta-tracing-vs-partial-evaluation/ |
| S42 | GraalVM Truffle language implementation framework docs | "latest" | https://www.graalvm.org/latest/graalvm-as-a-platform/language-implementation-framework/ |
| S43 | Fallin, Bernstein, "Partial Evaluation, Whole-Program Compilation" (arXiv 2411.10559) | 2024-11-15 | https://arxiv.org/abs/2411.10559 |
| S44 | Wikipedia, "Partial evaluation" (secondary; cites Futamura 1971/1983/1999) | live page | https://en.wikipedia.org/wiki/Partial_evaluation |
| S45 | LuaJIT home | copyright 2005-2026 | https://luajit.org/luajit.html |
| S46 | V8 blog, "Custom startup snapshots" | 2015-09-25 | https://v8.dev/blog/custom-startup-snapshots |
| S47 | GraalVM Native Image reference | "latest" (selector shows 25.5) | https://www.graalvm.org/latest/reference-manual/native-image/ |
| S48 | CRIU main page | live wiki | https://criu.org/Main_Page |
| S49 | Apple, "Porting just-in-time compilers to Apple silicon" (JSON form of the doc page) | undated | https://developer.apple.com/tutorials/data/documentation/apple-silicon/porting-just-in-time-compilers-to-apple-silicon.json |
| S50 | Wikipedia, "AArch64" (secondary) | live page | https://en.wikipedia.org/wiki/AArch64 |
| S51 | Remke, Breuer, "Hello SME!" (arXiv 2409.18779) | 2024-09-27 | https://arxiv.org/abs/2409.18779 |
| S52 | Apple, MTLDevice makeLibrary(source:options:) (JSON form) | undated | https://developer.apple.com/tutorials/data/documentation/metal/mtldevice/makelibrary(source:options:).json |
| S53 | Khronos SPIR-V specification, version 1.6 revision 8 | copyright 2014-2026 | https://registry.khronos.org/SPIR-V/specs/unified1/SPIRV.html |
| S54 | W3C WebGPU, Candidate Recommendation Draft | 2026-11-16 | https://www.w3.org/TR/webgpu/ |
| S55 | IREE home | undated | https://iree.dev/ |
| S56 | tinygrad README | undated | https://github.com/tinygrad/tinygrad |
| S57 | Halide home | undated | https://halide-lang.org/ |
| S58 | MLX README | undated | https://github.com/ml-explore/mlx |
| S59 | Fleury, "Untangling Lifetimes: The Arena Allocator" | 2022-09-23 | https://www.dgtlgrove.com/p/untangling-lifetimes-the-arena-allocator |
| S60 | Wikipedia, "Hash consing" (secondary; cites Ershov 1958, Goto 1974, Filliatre and Conchon 2006) | live page | https://en.wikipedia.org/wiki/Hash_consing |
| S61 | Unison, "The big idea" | undated | https://www.unison-lang.org/docs/the-big-idea/ |
| S62 | Cap'n Proto home | undated | https://capnproto.org/ |
| S63 | py-lmdb documentation (describes LMDB) | undated | https://lmdb.readthedocs.io/en/release/ |
| S64 | van Kempen, Kwon, Nguyen, Berger, "It's Not Easy Being Green" (arXiv 2410.05460, ASE 2025) | 2024-10-07, v4 2025-10-03 | https://arxiv.org/abs/2410.05460 |
| L1 | Local command: `sysctl machdep.cpu.brand_string hw.optional.arm.FEAT_SME ...`, `sw_vers`, `cc --version`, `go version`, `rustc --version` | run 2026-11-24 | this machine |
| L2 | Local `man powermetrics` | macOS 26.3.1 | this machine |
| L3 | Existing kernel wording for node states | checked in | coherence-kernel/AGENTS.md:575, :585; .../axioms/host-kernel.form:211 |

### The machine this would first run on (L1, observed)

Apple M4 Max, 12 performance + 4 efficiency cores, 137438953472 bytes RAM (128 GiB), page size 16384,
cache line 128 bytes, `hw.optional.arm.FEAT_SME: 1`, `hw.optional.arm.FEAT_SME2: 1`, macOS 26.3.1,
Apple clang 17.0.0 (arm64-apple-darwin25.3.0), go1.27.2 darwin/arm64, rustc 1.99.1 (2026-10-13).
All three candidate bootstrap languages are present on the host.

---

## 1. Baseline and template JITs

### 1.1 Copy-and-patch (Xu and Kjolstad)

- What it is (S1, observed): lower bytecode or a high-level language to machine code by "stitching
  together" prebuilt binary implementation variants called stencils, then patching holes
  (constants, addresses, branch targets).
- Measured results (S1 abstract, observed): TPC-H query compilation "two orders of magnitude faster
  than LLVM -O0" and "three orders of magnitude faster than higher optimization levels"; generated
  code "an order of magnitude faster than interpretation" and "14% faster than LLVM -O0"; the
  WebAssembly compiler generates code "4.9X-6.5X faster than Liftoff" and the code runs 39%-63%
  faster than Liftoff's on Coremark and PolyBenchC.
- Where the stencils come from (S10, observed): C++ bytecode semantics are compiled by LLVM at
  build time; the build parses the object files to extract unrelocated machine code and relocation
  records; operands are modelled as external symbol addresses so that relocations become the holes.
- Fit to the mission (inferred from S1, S2, S10): copy-and-patch makes the runtime emitter tiny,
  but it moves the compiler's intelligence into a build-time dependency on an industrial optimizer.
  In every deployment I read (CPython, Deegen), the stencils are LLVM output. A system that stores
  only a minimal seed and no external toolchain cannot take that route unchanged. See 1.6 for the
  form that does fit.

### 1.2 CPython: the largest public experiment with copy-and-patch, 2024 to 2026

- Size (S2, observed): "about 900 lines of build-time Python and 500 lines of runtime C". LLVM is a
  build-time dependency only; Clang is required because the design relies on guaranteed tail calls
  (`musttail`). Building adds 3 to 60 seconds.
- What it compiles (S2, observed): optimized micro-op traces, not bytecode directly.
- 2024 result (S2, observed): "about as fast as the existing specializing interpreter"; memory
  "about 10-20% more" than the interpreter, with the upper end on `aarch64-apple-darwin` because of
  its larger pages (this machine's 16 KiB pages, L1).
- 2025 (S3, observed): the tail-call interpreter alone gave "3-5% faster" geometric mean on
  pyperformance, Clang 19+ on x86-64 and AArch64 only.
- 2026, Python 3.15 released 2026-10-09 (S4, observed): JIT still labelled experimental; "7-8%"
  geometric mean over the fully optimized interpreter on x86-64 Linux; "11-12%" over the tail-calling
  interpreter on AArch64 macOS; per-benchmark range "roughly 15% slowdown to over 100% speedup".
  The gains came with a new tracing frontend, basic register allocation, more constant propagation,
  reference-count elimination, and LLVM 21 for stencil generation.
- PEP 836 (S5, observed): created 2026-07-02, proposed a time-bounded path (at least 5% with the
  GIL by 3.16 beta, at least 20% with free threading by 3.17 beta), and a shift from a trace-based
  to a method-based frontend. Per-machine June 2026 averages: M3 Pro macOS 12.6%, AmpereOne Linux
  7.3%, i5-8400 Linux 6.9%, Ryzen Windows 4.7%. The method-based prototype was 5-6% versus 7-8% for
  the tracing design on the same build.
- Governance (S6, S7, observed): in June 2026 the Steering Council paused new JIT development on
  main pending an accepted PEP, with a six-month window. On 2026-10-19 the authors withdrew PEP 836:
  "we don't feel we have the personal bandwidth to carry the current version forward." A council
  member wrote on 2026-10-29: "The SC has not killed the JIT. The SC has not rejected PEP 836." As of
  the last post I read (2026-10-30) the fate of the JIT code in main was not stated.
- Reading (inferred from S2, S4, S5, S8): three release cycles of work moved copy-and-patch from
  parity to roughly 5-12%. The emitter was never the bottleneck; the gains tracked the trace
  optimizer (what work is removed before emission). S8, a secondary source, puts it as "the biggest
  wins come from eliminating work, not doing work faster." The second lesson is organisational: a
  JIT bolted onto a mature interpreter with reference counting, a C API and a large semantic
  surface pays for every one of those; a new system that designs its node form for compilation from
  the first day does not inherit that cost. This is an inference, not a measurement.

### 1.3 Deegen and LuaJIT Remake: generate the tiers from one semantic description

- What (S9, observed): from bytecode semantics written as C++ functions, Deegen generates "a
  two-tier VM execution engine" (interpreter, baseline JIT, the linking logic, inline caches).
- Measured (S9, observed, 44 benchmarks): interpreter "179% faster than the official PUC Lua
  interpreter" and "31% faster than LuaJIT's interpreter"; baseline JIT "360% faster than PUC Lua"
  and "only 33% slower" than LuaJIT's optimizing JIT.
- Compile throughput (S10, observed): 19.1 million Lua bytecodes per second, 1.62 GB of machine
  code per second; 17,197 bytecodes compiled "in less than one millisecond total"; cost: 91 bytes of
  machine code per bytecode on average.
- Fit (inferred): this is the strongest evidence for "write the meaning once, derive the tiers".
  The mission's version is the same shape with the generator and the semantics both in the one
  grammar.

### 1.4 Other single-pass baselines

- Sparkplug (S17, observed, 2021): compiles from bytecode with no IR in one pass (plus a loop-finding
  pre-pass), keeps the interpreter's frame layout so the debugger, unwinder and on-stack replacement
  work unchanged, calls the same builtins as the interpreter. Speedometer +5-10%, real-site main
  thread time 5-15% better. "a fast compiler is a simple compiler".
- TPDE (S11, observed, 2025): one analysis pass, then instruction selection, register allocation
  and encoding in a single pass; "8--24x faster than LLVM -O0" on SPECint 2017 with on-par run time.
- WebAssembly baselines (S12, observed, 2023): empirical survey of six production single-pass
  compilers; the design integrates with an in-place interpreter using value tags, whose overhead
  "can be reduced to near zero".
- Druid (S14, observed, 2025): generates a baseline JIT frontend ahead of time from the interpreter
  for Pharo; about 2x the interpreter and about 0.7x of the hand-written JIT maintained for over ten
  years, with changes at 60 interpreter call sites.
- Lazy basic block versioning (S15, observed, 2014): "On average, 71% of type tests are eliminated,
  yielding speedups of up to 50%" with no separate type analysis. Ruby's YJIT descends from it; Ruby
  4.0 (S16, observed, 2025-12-25) introduced ZJIT, method-based with an SSA IR, "faster than the
  interpreter, but not yet as fast as YJIT".

### 1.5 Interpreter tier

- In-place interpretation (S13, observed, 2022): interpreting the stored code format directly, "no
  rewrite and no separate format is necessary", with "performance on-par with interpreting a
  custom-designed internal format".
- Fit (inferred from S13 and the mission's content-addressing axiom): if the stored node form is
  designed to be executed in place, the frozen form and the interpreter's working form are the same
  bytes. That removes a whole translation layer and a cache-invalidation problem.

### 1.6 What form of template JIT fits a self-written JIT (inferred)

Three options, with their evidence:

1. Hand-written per-node emitters in the grammar (the Sparkplug and Maru shape). Evidence: S17, S32.
   Cost: one emitter per node kind per architecture. On A64 every instruction is one 32-bit word
   (S50), so an emitter is "OR fields into a constant word".
2. Self-produced stencils: the system's own optimizing tier compiles each primitive's recipe to a
   relocatable blob once, freezes it content-addressed, and the baseline tier becomes copy-and-patch
   over those blobs. This keeps S1's speed without LLVM, but only exists after stage 3. Evidence
   that derivation works: S9, S14. No source I read does this without an external optimizer:
   **assumed feasible, unproven**.
3. Borrow a host C compiler once to make stencils, treat them as bootstrap ice. Fast to reach, but
   it stores opaque machine code whose source is another toolchain, which works against the
   "only a minimal bootstrap is stored" rule.

---

## 2. Tracing and meta-tracing

- LuaJIT (S45, observed): "high-speed interpreter, written in assembler", a trace compiler with
  SSA-based optimizations, backends for x86, x64, ARM, ARM64, PPC, MIPS32, MIPS64; copyright years
  2005-2026. Sizes and benchmark numbers were not on the page I fetched: any LuaJIT size figure is
  **assumed** (model memory: the whole VM is on the order of a few hundred kilobytes).
- RPython/PyPy (S39, observed): the JIT is not written for Python; the interpreter is written in
  RPython and "we turn it *automatically* into a JIT compiler"; "starting from 2009, our prototype
  is no longer using partial evaluation"; only "a small number of hints" are needed in the
  interpreter. PyPy front page (S40, observed, site built 2026-11-12): "On average, PyPy 3.12 is
  about 4 times faster than CPython 3.11."
- Meta-tracing versus partial evaluation on the same language (S41, observed, 2015): peak
  performance similar (meta-tracing "on average 3x slower than Java", partial evaluation "2.3x
  slower"); baseline implementations 5414 lines (meta-tracing) versus 11037 (partial evaluation);
  meta-tracing warms up far faster (1.5x slower at first iteration versus 16.3x); the authors
  attribute the remaining peak gap to toolchain engineering, not to the approach.
- Fit (inferred from S39, S41, S5): meta-tracing gives the most speed per line written, which
  matters when the JIT must be small and self-hosted. Its known weakness is branchy code and trace
  explosion; CPython's authors proposed leaving tracing for a method-based frontend in 2026 (S5)
  even though the tracing design measured faster at that moment, which says the choice is not
  settled by one benchmark suite.

## 3. Partial evaluation and the Futamura projections

- Definitions (S44, secondary, observed): (1) specializing an interpreter for given source yields an
  executable; (2) specializing the specializer for the interpreter yields a compiler; (3)
  specializing the specializer for itself yields a tool that converts any interpreter to a compiler.
  Original: Futamura 1971, English 1983, reprinted 1999.
- Truffle (S42, observed): "partial evaluation is central to how Truffle derives compiled code from
  interpreters"; interpreters are "self-modifying Abstract Syntax Trees". The PLDI 2017 paper was
  not fetched: its numbers are absent here.
- weval (S43, observed, 2024): a partial evaluator that derives compiled code "for free" by
  specializing an interpreter with its bytecode: 2.17x on SpiderMonkey's interpreter tier and 1.84x
  on PUC-Rio Lua, the latter in "only three hours' effort".
- Fit (inferred): the first projection is the cheapest honest path from "interpreter written in the
  grammar" to "compiled code", and it composes with content addressing: a specialization is a pure
  function of (interpreter hash, program hash), so its result can be frozen and never recomputed.
  S61 gives the same argument for parse and typecheck results. The cost is the one S41 measured:
  slow warm-up and a bigger implementation than tracing.

## 4. Intermediate representations small enough to self-host

- Sea of nodes, teaching scale (S21, observed): the Simple repository builds a compiler chapter by
  chapter (31 numbered chapters); instruction selection in chapter 19, graph-colouring register
  allocation in chapter 20, "Instruction Encodings & ELF" in chapter 21; targets x86-64, RISC-V and
  ARM64 ahead of time. Click's 1995 paper was not fetched (**assumed** as the origin).
- Sea of nodes, production verdict (S19, observed, 2025-03-25): V8 left it for a CFG-based IR.
  Reasons: effect and control chains "hard to understand, leading to many subtle bugs"; most
  JavaScript nodes sit on the effect chain so they cannot float; poor cache locality. Numbers:
  "compile time got divided by 2", load elimination "up to 190 times faster", "about 3 times more L1
  dcache misses" under sea of nodes, peephole nodes "changed only once every 20 visits".
- Mid tier (S18, observed, 2023): Maglev is SSA over a CFG, "roughly 10x slower than Sparkplug, and
  10x faster than TurboFan"; energy -3.5% on JetStream and -10% on Speedometer.
- QBE (S29, observed): "aims to provide 70% of the performance of industrial optimizing compilers in
  10% of the code"; SSA IL; targets amd64 (linux, osx), arm64, riscv64; release 1.3 on 2026-06-01
  added the Windows ABI; 1.1 (2023-02-01) added `arm64_apple`. No line count on the page.
- Cranelift (S26, observed): "about 200 thousand lines of code" versus "over 20 million" for LLVM;
  compiles "about an order of magnitude faster than an equivalent LLVM-based system"; code "~2%
  slower than V8 (TurboFan)" and "~14% slower than an LLVM-based system" (the page attributes these
  to arXiv 2011.13127, which is S1; I did not verify that attribution inside S1's body).
- Fit (inferred from S19, S18, S29): for the optimizing tier, a CFG with SSA values is the
  evidence-backed choice in 2026. Sea of nodes is attractive on paper for a graph-native system
  (everything is already a node), and the V8 post is a specific warning: the benefit depends on how
  many nodes are pure. A system whose nodes are content-addressed and mostly pure may sit on the
  favourable side of that trade, unlike JavaScript. That is a hypothesis to test, not a result.

## 5. E-graphs and equality saturation

- egg (S22, observed, POPL 2021): "rebuilding", an amortized invariant restoration technique with
  asymptotic speedups; "e-class analyses" to fold domain analyses into the e-graph.
- egglog (S23, observed, PLDI 2023): "a fixpoint reasoning system that unifies Datalog and equality
  saturation"; reimplementations of two prior systems came out "faster, simpler, and fix bugs". The
  repository (S24, observed) shows version 3.0.0 and notes parallel execution is new and most users
  run single-threaded.
- Production use (S25, S26, observed): Cranelift's acyclic e-graph restricts merging to node
  creation time so no parent pointers or full recanonicalization are needed; the prototype replaced
  GVN, LICM, simple preopt and alias analysis and measured SpiderMonkey.wasm at +1% compile time and
  13% faster run time, bz2.wasm at 15% faster compile and 3% faster run. "the first production
  compiler to use e-graphs to build a unified optimization framework."
- Fit (inferred from S22, S25, S60): an e-graph is a hash-consed node table plus a union-find. The
  mission's content-addressed node store is already the hash-cons half. Optimization then becomes
  "rewrite rules are recipes; equivalence is a relation between content addresses", which is the
  single-grammar form of an optimizer. The acyclic variant is the one with production evidence for
  compile-time cost; full saturation is better kept for offline search (see 6).

## 6. Superoptimization

- STOKE (S27, observed, ASPLOS 2013): loop-free x86-64 superoptimization as MCMC search; from
  `llvm -O0` output it produces programs that "either match or outperform the code sequences
  produced by gcc with full optimizations enabled".
- Souper (S28, observed, 2017-2018): synthesizing superoptimizer for LLVM IR; a Clang built with it
  is "about 3 MB (4.4%) smaller"; "Shipping, or about-to-ship, versions of both compilers contain
  optimizations suggested by Souper but implemented by hand".
- Massalin 1987 was not fetched (**assumed** as the origin of the term).
- Fit (inferred): superoptimization is an idle-time producer of rewrite rules and stencils, not a
  runtime tier. Its outputs are small, verifiable, and content-addressable, so they freeze well.

## 7. Small self-hosting systems

| System | What the primary page states | Source, status |
|---|---|---|
| Maru | Evaluator and compiler written in its own s-expression language; only `boot-eval.c` is C; "The whole thing is 1750 lines of code (about twice what it should be)"; the self-compiled evaluator regenerates an identical `eval.s`, so C is out of the loop; runs at "about 30% the speed of aggressively-optimised C" | S32, observed |
| STEPS | Aim: "about 20,000 lines of meaning-code for everything 'down to the metal'"; optimizations counted separately, "on the side". Nile graphics in "just 495 lines of code"; Nile defined in about 130 lines of form plus about 750 of meanings and optimizations; the metatranslator makes itself from "about 100 lines of definition code" | S33 pages 4-6, observed |
| Squeak | VM "written entirely in Smalltalk", a subset translated to C by 42 rules; interpreter 3951 lines, object memory 1283, BitBlt 1313; image 968K bytes; inlining gave a factor of 3.4 for 13% more code; the Smalltalk-hosted VM was about 450 times slower than the translated one; self-supporting about sixteen weeks after the December 1995 start | S34, observed |
| chibicc | C11 compiler that compiles itself, Git, SQLite, libpng; no optimization pass, output "twice or more slower than GCC's" | S30, observed; line count **assumed** absent |
| TinyCC | "about 100KB for x86 TCC executable, including C preprocessor, C compiler, assembler and linker"; 859,000 lines per second versus 98,000 for GCC 3.2 -O0 on a 2.4 GHz Pentium 4; compiles itself | S31, observed |
| QBE | See section 4 | S29, observed |
| cproc | Fetch failed (HTTP 502) | **assumed**: a C11 compiler on QBE |
| Project Oberon | 2013 edition: compiler modules ORB, ORG, ORP, ORS; RISC5 processor in Verilog; system sources | S35, observed; sizes not on the page |
| sectorforth | A 16-bit x86 Forth in a 512-byte boot sector: eight primitives (`@ ! sp@ rp@ 0= + nand exit`) plus `key` and `emit`; everything else written in Forth | S36, observed |
| Dusk OS | Forth with an "almost C" compiler; a fully booted i386 system with C compiler, assembler and editor in "less than 6000" lines; bootstrapped from a POSIX VM written in C; "completely self-hosts on all its targets" | S37, observed |
| stage0-posix | Bootstraps "from a single 256 byte seed" through hex0, hex1, hex2, M0, cc_*, M2-Planet, toward GCC; AMD64, x86, AArch64, RISC-V | S38, observed (the Guix post that I remember giving 357 bytes returned HTTP 403: that older figure is **assumed**) |

Reading (inferred from the table):

- The seed language in every one of these is C or raw machine code. None uses Go or Rust as the
  seed. That is evidence about precedent, not a proof that Go or Rust cannot serve.
- Maru is the closest existing proof of the mission's shape: a C evaluator that exists only to run
  the self-written compiler once, after which the system reproduces itself bit for bit.
- STEPS is the closest proof of the size claim, with its own honest caveat: the 20,000-line figure
  counts meaning, and the speed needed "optimization code" counted on the side. The new design
  should keep the same two ledgers: lines of meaning, lines of speed.
- Squeak measures the price of skipping the self-compile step: 450 times slower when the VM runs on
  itself uncompiled.

## 8. Image persistence as the analogue of frozen state

- Squeak (S34, observed): the system is "an image, or object memory"; 968K bytes held the class
  library and development environment.
- V8 startup snapshots (S46, observed, 2015): deserializing a prepared heap cut context creation
  from 40 ms to under 2 ms on desktop and 270 ms to 10 ms on a phone. Limits: only the engine's heap
  is captured, so nothing that touches the outside world can be inside; `Math.random` and `Date.now`
  values are fixed at snapshot time.
- GraalVM Native Image (S47, observed): ahead-of-time under a "closed world assumption"; "Starts in
  milliseconds"; "Delivers peak performance immediately, with no warmup". The page I read did not
  describe the image heap (**assumed**: build-time initialized objects are stored in the binary).
- CRIU (S48, observed): freezes a running Linux process or container to disk and restores it
  "exactly as it was"; Linux only.
- Lisp images (`save-lisp-and-die`) were not fetched: **assumed**.
- Fit (inferred from S46, S61, L3): the existing kernel already calls a compiled artifact kept
  beside its source "ice" (L3). The snapshot literature adds two rules worth carrying: (a) a frozen
  image may contain only what is a pure function of content; anything that crossed the boundary (a
  handle, a clock reading, a random draw) must be re-established on thaw, which is the boundary
  axiom stated operationally; (b) the win is large and immediate (20x to 27x faster start in S46).
  Unlike a Smalltalk image, a content-addressed store does not need to be one monolithic blob: each
  node can be frozen or thawed alone, and sharing between images is free.

## 9. Tiered execution as the analogue of ice, water and gas

Observed tier ladder in V8 (S17, S18, S19): Ignition interpreter, Sparkplug (no IR), Maglev (SSA
CFG, 10x slower to compile than Sparkplug), TurboFan/Turboshaft (10x slower again). Each tier
trades compile time for code quality by about an order of magnitude.

A proposed mapping, **assumed**, because I did not find a normative definition of the three states
in the files I searched (L3 shows the words in use, not a definition):

| State | Proposed meaning for code | Evidence that the analogue works |
|---|---|---|
| ice | content-addressed, position-independent, immutable: the stored node form, relocatable stencils, verified rewrite rules, a frozen image | S46, S47, S61, S1 (stencils are relocatable by construction) |
| water | live, patched, executable but still general: interpreter state and baseline machine code in a JIT region | S17, S9 |
| gas | speculative and discardable: traces and optimized code that hold guards and can evaporate back to water on a failed guard | S39, S15, S20 ("Speculative Optimizations for WebAssembly using Deopts and Inlining", 2025-06-24, title only) |

The useful property (inferred): a transition downward (gas to water to ice) is always safe because
the lower state is more general; a transition upward is an optimization that must carry its guard.

## 10. Machine encodings and the host's rules for generated code

- A64 (S50, secondary, observed): instructions "are still 32 bits long"; 31 general-purpose 64-bit
  registers. SME is listed under Armv9.2-A, SME2 under Armv9.4-A.
- Apple silicon (S49, observed): "Apple silicon enables memory protection for all apps"; "a thread
  cannot write to a memory region and run instructions in that region at the same time"; `mmap`
  with `MAP_JIT`; the entitlement `com.apple.security.cs.allow-jit` "is required only when an app
  adopts the Hardened Runtime capability"; with the hardened runtime an app "can only create one
  memory region with the `MAP_JIT` flag set"; the current guidance is
  `pthread_jit_write_with_callback_np()` with an allowlist of callbacks, and with that entitlement
  the older `pthread_jit_write_protect_np()` can no longer be called; "Always call
  `sys_icache_invalidate`" because "the instruction caches aren't coherent with data caches".
- DynASM (S45-adjacent page, observed): "a Dynamic Assembler for code generation engines"; the page
  gave no technical detail. The preprocessor-plus-tiny-runtime design is **assumed** from memory.
- x86-64 encoding strategy: nothing fetched; **assumed**: variable-length encoding (prefixes,
  ModRM, SIB) makes a table-driven or stencil approach more attractive there than on A64.
- Fit (inferred from S49, S50, L1): the irreducible host contact for a JIT on this machine is four
  calls: `mmap(MAP_JIT)`, the write toggle, `sys_icache_invalidate`, and a jump. That is a
  lower bound on what the stored bootstrap must be able to reach. All four are C-callable libSystem
  entry points; reaching them from Go or Rust needs their foreign-call layer (**assumed** for Go:
  cgo or assembly trampolines).

## 11. SIMD, matrix and GPU paths

- SME on this hardware (S51, observed, 2024; L1, observed): M4 is "the first chip to support SME";
  support is FP32-centric with "throughput above 2.3 FP32 TFLOPS"; just-in-time generated small
  matrix kernels "outperform the vendor-optimized BLAS implementation in almost all tested
  configurations". This machine reports FEAT_SME and FEAT_SME2 present.
- Metal (S52, observed): `makeLibrary(source:options:)` "Synchronously creates a Metal library
  instance by compiling the functions in a source string". A system can therefore emit shader text
  at run time with no offline toolchain; the page does not discuss the compile cost.
- SPIR-V (S53, observed): "a simple binary intermediate language for graphical shaders and compute
  kernels"; each instruction begins with a word count and an opcode; results are single-assignment;
  version 1.6 revision 8. A direct binary emitter for it is the GPU twin of an A64 emitter
  (inferred).
- WebGPU (S54, observed): W3C Candidate Recommendation Draft dated 2026-11-16, with compute shaders.
- IREE (S55, observed): MLIR-based, "lowers Machine Learning (ML) models to a unified IR"; targets
  Vulkan/SPIR-V, Metal, CPU, experimental WebGPU; "Binary size as low as 30KB on embedded systems".
- tinygrad (S56, observed): tensor library, "IR and compiler that fuse and lower kernels", JIT;
  backends OpenCL, CPU, METAL, CUDA, AMD, NV, QCOM, WEBGPU; a new accelerator "only needs to support
  a total of ~25 low level ops".
- Halide (S57, observed): the algorithm has "no storage or order"; the schedule "defines order,
  locality; implies storage"; backends include Apple Metal, CUDA, OpenCL, DirectX 12.
- MLX (S58, observed): "Arrays in MLX live in shared memory. Operations on MLX arrays can be
  performed on any of the supported device types without transferring data."
- Fit (inferred from S56, S57, S58, S51): the GPU tier is not a new compiler. It is a second backend
  for the same lowered kernels: the recipe states the algorithm; a schedule is a separate,
  replaceable node (Halide); the backend surface is a few dozen primitive ops (tinygrad); on Apple
  silicon the data never moves (MLX). SME is a CPU-side matrix tier that should come before the GPU
  because it reuses the A64 emitter and has no shader compile step.

## 12. Memory

- Arenas (S59, observed, 2022): an arena is "a handle to which allocations are bound"; lifetimes
  nest; releasing is resetting a position; reserve a large virtual range and commit pages as used.
- Hash consing (S60, secondary, observed): share structurally equal values; equality becomes a
  pointer comparison; origin Ershov 1958, term from 1970s Lisp, typed formulation Filliatre and
  Conchon 2006 (the paper itself not fetched).
- Content-addressed code (S61, observed): "Each Unison definition is identified by a hash of its
  syntax tree"; names are "just separately stored metadata that don't affect the function's hash";
  512-bit SHA3; results of parse and typecheck go to "a cache which is never invalidated"; renames
  are "instant non-breaking".
- Zero copy (S62, S63, observed): Cap'n Proto: "there is no encoding/decoding step", the encoding is
  "appropriate both as a data interchange format and an in-memory representation", and a large file
  can be memory-mapped so that "The OS won't even read in the parts that you don't access". LMDB:
  "Memory mapped, allowing for zero copy lookup and iteration"; "readers don't block writers".
- Region inference (Tofte and Talpin) was not fetched: **assumed**.
- Fit (inferred): S61 is direct outside evidence for the mission's central idea that symbols are a
  replaceable lens over a numeric, content-addressed space. The memory design that falls out is:
  ice is a memory-mapped, position-independent, content-addressed file read in place (S62, S63);
  water is arena-allocated per movement and released in one step (S59); gas never needs individual
  freeing. A hash of a node also serves as the cache key for every derived artifact (parse, type,
  stencil, optimized code), which is what makes "compile once, ever" true (S61).

## 13. Energy

- Language choice and energy (S64, observed, ASE 2025): once the number of active cores and memory
  activity are controlled, "the choice of programming language implementation has no significant
  impact on energy consumption beyond execution time". (The abstract page did not name the
  measurement tool; RAPL is **assumed**.)
- Tiering and energy (S18, observed): adding a cheaper optimizing tier cut energy 3.5% on JetStream
  and 10% on Speedometer by reducing compile work off the main thread.
- Measuring on this machine (L2, observed): `powermetrics` "may also display estimated power
  consumed by various SoC subsystems, such as CPU, GPU, ANE"; the manual warns the average power
  values "are estimated and may be inaccurate"; it needs elevated privileges to run (**assumed**; I
  read the manual, I did not run the tool).
- Fit (inferred from S64, S18): for this design, energy is a derived reading of three things the
  system can count itself: time, cores awake, and bytes of memory touched. Compile work is part of
  the bill, so a tier that compiles less (or thaws frozen code instead of compiling) saves energy
  directly. Wakeups matter too (L2's guidance section), which argues against polling loops.

---

## 14. Which bootstrap language: what this study's evidence says (inferred, partial)

This study was not the full C/Go/Rust analysis; it contributes these grounded points:

- Precedent: every small self-hosting system in section 7 seeds from C or raw bytes (S30, S31, S32,
  S34, S37, S38). Production JIT infrastructure in Rust exists (Cranelift, 200k lines, S26; ZJIT
  needs Rust 1.85+, S16). I found no small self-replacing seed written in Go in this session
  (absence of evidence within about 60 fetches, not evidence of absence).
- Host contact: the four JIT calls on Apple silicon are libSystem C entry points (S49).
- Shrinking: a seed that exists to be replaced should have no runtime of its own to coexist with
  the code it emits. C has none; Go brings a garbage collector and scheduler; Rust brings none at
  run time but a heavy build toolchain. (**assumed** for the Go and Rust halves; common knowledge,
  not fetched.)
- All three toolchains are installed on this machine (L1), so the choice is not constrained by
  availability.

On this evidence C is the lowest-risk seed for the specific job "evaluate the grammar once so the
self-written compiler can compile itself" (the Maru pattern), and the seed's size target is given by
Maru: an evaluator in a few hundred to a couple of thousand lines.

## 15. Recommended staged path, with the evidence for each stage

Each stage ends when the next stage's code, written in the one grammar, can replace it. The seed is
kept in the repository for audit and for a cold re-bootstrap (S38's reason: nothing opaque needs to
be trusted), but it stops running once ice exists (S32: "the C compiler is out of the loop").

**Stage 0. Seed (stored, in C or equivalent).** A reader for the numeric node form, a plain
evaluator, the four JIT host calls, file mapping. No optimization at all.
- Evidence it can be tiny: S32 (1750 lines for evaluator plus compiler plus boot), S36 (ten
  primitives in 512 bytes), S38 (256-byte seed to a C compiler), S37 (under 6000 lines to a booted
  system with a C compiler).
- Evidence of the cost of stopping here: S34 (450x slower uncompiled).
- Exit test: the seed evaluates the stage-2 compiler, the compiler compiles itself, and the result
  compiles itself again to identical bytes (S32's fixed point).

**Stage 1. In-place interpreter, written in the grammar.** Executes the stored node form directly.
- Evidence: S13 (in-place on par with a custom internal format), S3 (tail-call dispatch worth
  3-5%), S9 (a generated interpreter 31% faster than LuaJIT's hand-written assembly interpreter).
- Design consequence: the stored form must be designed for execution, not only for meaning.

**Stage 2. Self-compiled baseline JIT.** One pass, no IR, one emitter per node kind, frames
compatible with the interpreter so that falling back is always possible.
- Evidence for speed of compilation: S10 (19.1 million bytecodes per second), S11 (8-24x faster
  than LLVM -O0), S31 (859,000 lines per second in 2004-era hardware).
- Evidence for code quality: S32 (30% of optimized C from a 1750-line system), S9 (within 33% of
  LuaJIT's optimizing JIT), S17 (5-15% over an already fast interpreter).
- Evidence for the caution: S2 and S4 (a baseline emitter alone gave parity; gains came from the
  optimizer in front of it).
- A64 first: fixed 32-bit instructions (S50) and the measured machine (L1).

**Stage 3. Optimizing tier.** SSA values over a CFG; rewrite rules as content-addressed recipes
applied through an acyclic e-graph; speculation with guards that fall back to stage 2; frozen
results keyed by content hash.
- Evidence for CFG over sea of nodes: S19 (compile time halved, 3x fewer L1 misses).
- Evidence for e-graph rewriting at production cost: S25 (13% faster code for 1% compile time).
- Evidence for the size target: S29 ("70% ... in 10% of the code"), S26 (200k versus 20 million).
- Evidence for how to get the speculative part cheaply: S39 and S41 (meta-tracing: half the code of
  partial evaluation for similar peak), S43 (first projection: 1.84x to 2.17x for hours of work),
  S15 (71% of type tests removed without an analysis).
- Idle-time enrichment: S27, S28 (superoptimization finds rules; freeze what is verified).
- Stencil closure: once this tier exists it can produce the stage-2 stencils itself (option 2 in
  1.6), which is the point where copy-and-patch becomes available without LLVM. Unproven anywhere I
  read.

**Stage 4. Matrix and GPU.** The same lowered kernels, new backends.
- SME first on this machine: S51 (JIT kernels beat vendor BLAS on M4; over 2.3 FP32 TFLOPS), L1.
- Metal by emitting shader source at run time (S52), or SPIR-V words directly for Vulkan and
  WebGPU portability (S53, S54).
- Algorithm and schedule as separate nodes (S57); a backend surface of a few dozen ops (S56);
  no copies between CPU and GPU on unified memory (S58).

**Across all stages.** Freeze every derived artifact by content hash and map it in place (S61, S62,
S63, S46). Count time, cores awake and memory touched as the energy reading (S64, L2).

## 16. Open points this study could not settle

1. Whether self-produced stencils (stage 3 feeding stage 2) match LLVM-produced stencils closely
   enough to be worth the loop. No source does this.
2. Whether a mostly pure, content-addressed node space puts sea of nodes back on the favourable
   side of V8's trade (S19 is about JavaScript's effect-heavy nodes).
3. Trace-based versus method-based for the speculative tier: CPython's own authors proposed
   switching in 2026 while their numbers favoured tracing (S5).
4. What happened to the CPython JIT in main after 2026-10-30 (S7 ends unresolved).
5. Real line counts for chibicc, QBE, tcc, LuaJIT and the Oberon compiler (not on the pages read).
6. How Apple's callback allowlist model (S49) interacts with a JIT whose emitter is itself generated
   code: the allowlisted callback must be static code that validates what it copies. This may fix a
   small permanent piece of the seed.
7. The run-time cost of Metal source compilation versus a direct binary path (S52 is silent).
8. The normative definition of ice, water and gas for the new design; section 9's mapping is a
   proposal.
