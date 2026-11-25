# Bootstrap language study: C, Go, Rust (with Zig and hex/assembly as outside options)

Study date: 2026-11-24. Machine: Apple M4 Max, macOS 26.3.1 (Darwin 25.3.0, arm64).
Every URL below was fetched on 2026-11-24 unless a line says otherwise.

Status words: **observed** = read or run in this session; **inferred** = reasoning from observed things (the
premises are named); **assumed** = not verified, source is model memory.

How to read the web citations: pages were fetched through a tool that answers a question over the page with a
small model. Sentences in quotation marks are as that tool relayed them. They are precise enough to act on and
should be re-read at the URL before being quoted outside this graph.

---

## 0. The answer in five lines

1. **Write the bootstrap in C** (a strict C99/C11 subset, one translation unit, no libc beyond a short list of
   declared OS symbols). Status: inferred from sections 2 to 7.
2. The reason is not speed of C. It is that **the platform C ABI is the one contract every other party already
   speaks** (libSystem, `objc_msgSend`/Metal, Vulkan, Win32, the kernel's JIT entitlement API), and a C bootstrap
   is *nothing but* that contract, so the system can later emit its own replacement function by function.
3. Go is the fastest to iterate in and already carries NUMS.Go, but its runtime (collector, scheduler, moving
   stacks, private ABI, signal ownership) is a second kernel that the self-emitted code cannot replace piecewise
   and that measurably holds a collection hostage while JIT code runs (627 ms here, section 3.3).
4. Rust is fully capable (naked functions, `no_std`, 50 KB binary measured) and is the choice if the bootstrap
   were going to grow and stay. For a few thousand lines whose core acts are all `unsafe` and which is meant to
   dissolve, it adds toolchain weight and gives little back.
5. Zig is the best *build tool* for a C bootstrap (cross-linking) but is still pre-1.0 with breaking changes every
   release (0.17.1, 2026-11-02). Hex/assembly is the destination, not the start: the system's own emitted image
   is the "assembly" bootstrap, and macOS forbids the static-syscall style that makes hex0 seeds practical.

Conditions that flip the answer are in section 9.

---

## 1. What the bootstrap must do, restated as contracts

| Duty | Contract it touches |
|---|---|
| Read the seed | file open/read or `mmap` (POSIX; `CreateFileMapping` on Windows) |
| Allocate executable memory, run emitted code | `mmap(MAP_JIT)` + `pthread_jit_write_protect_np` + `sys_icache_invalidate` (macOS arm64); `mmap`/`mprotect`/`__clear_cache` (Linux); `VirtualAlloc`/`VirtualProtect`/`FlushInstructionCache` (Windows) |
| Call OS and GPU | platform C ABI: `dlopen`/`dlsym`, `objc_msgSend` (Metal), Vulkan C headers, Win32 |
| Threads | `pthread_create` / `CreateThread`; the JIT write permission on macOS is per thread |
| Be replaced by its own output | emit Mach-O / ELF / PE that exports the same C-ABI entry points |

All five are C-ABI contracts. Status: inferred from the sources in sections 2 to 6.

---

## 2. Apple Silicon JIT rules (the hardest host constraint)

### 2.1 What Apple documents

Source: https://developer.apple.com/tutorials/data/documentation/apple-silicon/porting-just-in-time-compilers-to-apple-silicon.json
(the JSON behind "Porting just-in-time compilers to Apple silicon"; no publication date on the page). Status: observed.

- "Apple silicon enables memory protection for all apps, regardless of whether they adopt the Hardened Runtime."
  A thread cannot write to a region and execute it at the same time.
- "The `com.apple.security.cs.allow-jit` entitlement is required only when an app adopts the Hardened Runtime
  capability. If you don't adopt this capability, you don't need the entitlement to use the `MAP_JIT` flag."
- "When your app has the Hardened Runtime capability and the `com.apple.security.cs.allow-jit` entitlement, it can
  only create one memory region with the `MAP_JIT` flag set."
- "Always call `sys_icache_invalidate(_:_:)` before you run the machine instructions on a recently updated memory
  page. On Apple silicon, the instruction caches aren't coherent with data caches."
- The stricter path: with `com.apple.security.cs.jit-write-allowlist`, the app "can no longer call
  `pthread_jit_write_protect_np()`" and must write through `pthread_jit_write_with_callback_np()`, whose callbacks
  are declared with the C macro `PTHREAD_JIT_WRITE_ALLOW_CALLBACKS_NP(...)`, at most one allowlist per executable;
  the callback makes the region "writable, and not executable, for the current thread only."
- `pthread_jit_write_protect_np()` "isn't available on iOS."

Entitlement references (status: observed):
- https://developer.apple.com/tutorials/data/documentation/bundleresources/entitlements/com.apple.security.cs.allow-jit.json
  : "whether the app may create writable and executable memory using the `MAP_JIT` flag"; listed users include
  JavaScriptCore's fast path and PCRE.
- https://developer.apple.com/tutorials/data/documentation/bundleresources/entitlements/com.apple.security.cs.allow-unsigned-executable-memory.json
  : W+X memory "without the restrictions imposed by using the `MAP_JIT` flag", with the warning that it "exposes
  your app to common vulnerabilities in memory-unsafe code languages."

### 2.2 What this machine does (run today)

Program: `scratch/exp/jit.c`
Readings: `.../scratchpad/study/exp/out.txt` lines 7 to 34. Status: observed (one run each, 2026-11-24).

| Signing state | `MAP_JIT` + per-thread toggle | RW then `mprotect` to RX | plain RWX, no `MAP_JIT` |
|---|---|---|---|
| linker ad-hoc signature, no hardened runtime, no entitlement | works, returns 42 | works, returns 42 | `mmap` fails: Permission denied |
| hardened runtime (`codesign -s - -o runtime`), no entitlement | `mmap` fails: Invalid argument | process killed (signal 9) | not run |
| hardened runtime + `allow-jit` (ad-hoc) | works | process killed (signal 9) | not run |

Costs measured in the same program:
- one `pthread_jit_write_protect_np(0)`/`(1)` pair: **about 21 ns**;
- one `mprotect` RW/RX pair over a 16 KB page: **about 454 ns** (about 21 times dearer);
- indirect call into JIT code and return: about 0.79 ns from C.

Consequences for the design (status: inferred from 2.1 and 2.2):
- A locally built developer binary needs no entitlement and no Apple account. `MAP_JIT` simply works.
- A distributed (notarized) build needs the hardened runtime, therefore `MAP_JIT` plus `allow-jit`, and should
  **reserve one large JIT region once** so the documented one-region rule never bites.
- The "write RW, then `mprotect` RX" style, which is how Wasmtime publishes code today, dies under the hardened
  runtime. Cross-check: Wasmtime issue 11989 (open) reports "Code Signature Invalid" under the hardened runtime
  and a maintainer confirms that adding `MAP_JIT` fixes it; a workaround in use is the broader
  `allow-unsigned-executable-memory` entitlement.
  Source: https://bytecodealliance.github.io/zulip-archive/stream/225357-git-wasmtime/topic/wasmtime.20.2F.20issue.20.2311989.20Support.20mmap.20flag.20MAP_JIT.20for.20MACOS.html (observed).
- The write permission is per thread. A design with one emitting thread per region, or a toggle around each
  write, fits; a design where any thread patches any code does not.

Unverified report: a February 2026 third-party post says the one-region limit was not enforced in its tests on
macOS 26.2 (https://www.outflank.nl/blog/2026/02/19/macos-jit-memory/, HTTP 403 on fetch; known only from a search
snippet). Status: assumed. Treat Apple's documented limit as the rule.

### 2.3 What the present body already does

Status: observed.
- `coherence-kernel/runtime/membrane-boot.c:558-584`
  (last commit 2026-11-05): `map_executable` maps `PROT_READ|PROT_WRITE|PROT_EXEC` with
  `MAP_PRIVATE|MAP_ANON|MAP_JIT`, lifts `pthread_jit_write_protect_np(0)`, copies, restores, then
  `sys_icache_invalidate`.
- `.../runtime/fkwu-uni.c:4910-4929` (last commit 2026-11-20): `fk_page_alloc` uses the literal flags `0x1802`
  on Apple (private + anonymous + JIT), `fk_page_write` toggles the per-thread protection and calls
  `__builtin___clear_cache`.
- `.../runtime/membrane-boot.c:488` reaches Metal by `dlopen`; `.../runtime/fkwu-uni.c:4112-4127` reaches
  AudioToolbox by `dlopen`/`dlsym`.
- `.../form/form-stdlib/form-macho.fk:1-20`: a Form recipe already emits an arm64 Mach-O object; `ld` links it and
  supplies the ad-hoc signature that arm64 macOS requires.
- Sizes: `runtime/fkwu-uni.c` is **34,149 lines**, `runtime/membrane-boot.c` 1,199 lines (`wc -l`, today).

So the present kernel is a working C precedent for every duty in section 1, and also a warning: the language did
not keep the seed small. 34 thousand lines of C accreted. Status of the warning: inferred from the line count.

### 2.4 Other hosts

- Windows: "To execute dynamically generated code, use VirtualAlloc to allocate memory and the VirtualProtect
  function to grant PAGE_EXECUTE access"; the caller "bears responsibility for ensuring cache coherency via an
  appropriate call to FlushInstructionCache."
  Source: https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualalloc (page date
  2024-02-05, updated 2025-07-01). Status: observed.
- Linux: `PR_SET_MDWE` (Linux 6.3; `PR_MDWE_NO_INHERIT` in 6.6) lets a process, irreversibly, refuse mappings that
  are writable and executable and refuse non-executable mappings becoming executable.
  Source: https://man7.org/linux/man-pages/man2/PR_SET_MDWE.2const.html. Status: observed. A launcher or
  container that sets it would break both JIT styles; a dual mapping of one memory object is the known answer
  (status of the answer: assumed).
- V8's security team calls temporary permission flipping "inherently racy" and moves to per-thread permission
  views (x64 protection keys, Arm permission overlays).
  Source: https://v8.dev/blog/control-flow-integrity (2023-10-09). Status: observed. Apple's per-thread toggle is
  the same idea.

---

## 3. Go

### 3.1 What works (measured)

Program: `.../scratchpad/study/exp/gojit/jit.go` (pure Go, `CGO_ENABLED=0`, Go 1.27.2). Readings:
`.../scratchpad/study/exp/out2.txt` lines 1 to 17. Status: observed.

- Build: 0.10 s; rebuild after an edit 0.09 s.
- Binary: 2,447,186 bytes (1,604,402 with `-ldflags="-s -w"`).
- Even with cgo off, the binary links `/usr/lib/libSystem.B.dylib` and `libresolv` (`otool -L`).
- RW map, write arm64 words, `mprotect` RX, call through a func value on the goroutine stack: returns 42;
  0.79 ns per call, the same as a Go-to-Go noinline call (0.76 ns).
- The same binary under the hardened runtime: killed (signal 9). It has no route to `MAP_JIT`'s per-thread toggle
  without calling a libSystem function.
- Cross build of the pure program for linux/arm64 and linux/amd64: succeeds with no extra install
  (`out.txt` lines 64 to 66). Windows needs different calls (`syscall.Mmap` is undefined there), as expected.

### 3.2 cgo cost

- Go 1.26 (February 2026): "The baseline runtime overhead of cgo calls has been reduced by ~30%."
  Source: https://go.dev/doc/go1.26. Status: observed.
- Measured here on Go 1.27.2: **23.6 ns per cgo call** to a trivial C function
  (`.../exp/gocgo/cgo.go`, `out2.txt` line 28), against 0.79 ns for a direct call. Build with cgo: 0.65 s.
  Status: observed.
- A JIT author on golang-dev (2026-04-30) cites "about 43 ns for a CGO switch" against a 2 ns target and proposes
  a "self-describing foreign JIT stack-frame protocol"; no Go team reply is visible.
  Source: https://groups.google.com/g/Golang-dev/c/mABYStr5aYs. Status: observed.

### 3.3 The runtime versus JIT code

Measured (program `.../exp/gogc/gc.go`, `out2.txt` lines 18 to 23). Status: observed.

| Main goroutine is inside | Loop ran | `runtime.GC()` asked at +50 ms returned after |
|---|---|---|
| foreign arm64 loop on the goroutine stack | 679 ms | **627 ms** (the whole rest of the loop) |
| the same loop compiled by Go | 777 ms | 0.31 ms |

The collector's stop cannot reach a goroutine whose program counter is in code the runtime has no tables for.
The world waits until the foreign code returns. For a kernel whose normal state is "inside JIT code", every
collection is a stall of unbounded length, or the JIT code must poll the runtime, which makes the Go runtime part
of the emitted code's calling convention.

Documented constraints (status: observed unless marked):
- "We don't support JITed code." A JIT whose frames call back into Go fails in the collector's stack scan with
  "unexpected return pc" because the frames have no stack map; the issue was closed the day it opened.
  Source: https://github.com/golang/go/issues/20123 (2017-04-25).
- Goroutine stacks "start small (e.g., 2K) and grow or shrink dynamically"; system stacks "cannot grow" and are
  8K in a pure Go binary; code entered from C or a signal handler may have no valid goroutine and must be
  `nosplit`. Source: https://raw.githubusercontent.com/golang/go/master/src/runtime/HACKING.md.
  That growth relocates the stack (so emitted code cannot keep stack addresses) is from memory: assumed.
- Go's internal ABI "will change between Go versions"; on arm64 R28 holds the current goroutine, R26 the closure
  context, R27 is assembler scratch. Source: https://go.dev/src/cmd/compile/abi-internal. So emitted code that
  runs under Go must honour a register discipline that is neither the platform's nor stable.
- Signals: the runtime installs its handlers first; foreign handlers must set `SA_ONSTACK` ("Failing to do so is
  likely to cause the program to crash") and chain to Go's handler for SIGSEGV/SIGBUS/SIGFPE; foreign code "should
  not change the signal mask on any threads created by the Go runtime."
  Source: https://pkg.go.dev/os/signal. A JIT that wants fault-driven guards (null checks, stack guards, lazy
  page commit) is a guest in Go's signal house.
- Reaching runtime internals from outside got harder: since Go 1.23 the linker "disallows using a `//go:linkname`
  directive to refer to internal symbols in the standard library (including the runtime) that are not marked",
  with a grandfather clause for existing open-source uses. Source: https://go.dev/doc/go1.23.
- Go's own toolchain is a chain: "Go 1.26 now requires Go 1.24.6 or later for bootstrap."
  Source: https://go.dev/doc/go1.26.

### 3.4 Go precedents

- **wazero** (WebAssembly runtime, pure Go, with a compiler engine): zero dependencies to avoid cgo; generated
  code is entered through a Go assembler trampoline, never calls Go functions directly, and is not preemptible
  ("native code never preempts", so cancellation has to be polled); on darwin, Go "always uses 'CGO' mechanisms
  in the runtime layer." Source: https://raw.githubusercontent.com/tetratelabs/wazero/main/RATIONALE.md.
  Its unix platform file flips code to RX with `unix.Mprotect`
  (https://raw.githubusercontent.com/tetratelabs/wazero/main/internal/platform/mmap_unix.go), the style that
  section 2.2 shows killed under the hardened runtime. Status: observed.
- **purego** (calls C without cgo): beta; goes through `runtime_cgocall` with an assembly trampoline; "SyscallN
  does not properly call functions that have both integer and float parameters."
  Sources: https://github.com/ebitengine/purego and
  https://raw.githubusercontent.com/ebitengine/purego/main/syscall.go. Status: observed.
- **gojit** (2014 hack-day project): amd64 only. Source:
  https://raw.githubusercontent.com/nelhage/gojit/master/README.md. Status: observed.
- Go removed its own C: "The compiler and runtime are now implemented in Go and assembler, without C", moved "by
  automatic translation of the C code into Go." Source: https://go.dev/doc/go1.5. Status: observed. This is a
  precedent for *self-replacement by mechanical translation*, in the direction C to higher language.

### 3.5 Reading for Go

Go is a superb language for a *host program* (NUMS.Go at `NUMS.Go` is a Go workspace:
`go.work`, observed). As a *bootstrap that must vanish* it has a structural problem: the 1.6 to 2.4 MB of runtime
is not ours, cannot be freestanding, owns signals, stacks and the stop-the-world, and cannot be replaced one
function at a time by emitted code. Status: inferred from 3.1 to 3.4.

---

## 4. Rust

Measured (programs `.../exp/jit.rs`, `.../exp/jit_nostd.rs`; rustc 1.99.1 from Homebrew). Status: observed.
- `MAP_JIT` + toggle + icache flush through three `extern "C"` declarations, no crates: returns 42.
- Build: 0.95 s first, 0.09 s on rebuild. Binary with `std`: 455,384 bytes.
- `#![no_std] #![no_main]` version, linked with `-C link-arg=-lSystem -C panic=abort`: **50,128 bytes**, returns
  42. Without the link argument the link fails on `_mmap` and friends.
- The Homebrew toolchain carries only the host target (`lib/rustlib/aarch64-apple-darwin`); other targets need
  additional standard-library builds. Toolchain on disk: 406 MB.

Documented (status: observed):
- Naked functions are stable since Rust 1.88.0 (2025-06-26): "no compiler-generated epilogue and prologue",
  "full control over the generated assembly for a particular function."
  Source: https://blog.rust-lang.org/2025/06/26/Rust-1.88.0/.
- Current release: Rust 1.100.0 (2026-11-12); a parallel-frontend post on 2026-11-16.
  Source: https://blog.rust-lang.org/.
- Symbol and section control: `#[unsafe(no_mangle)]`, `export_name`, `link_section`.
  Source: https://doc.rust-lang.org/reference/abi.html.
- **Cranelift**: "production-ready", four backends (x86-64, aarch64, s390x, riscv64); a cited paper puts its code
  about 2% slower than V8 TurboFan and about 14% slower than an LLVM-based engine, with compile speed about an
  order of magnitude faster than LLVM; "APIs are not yet stable"; policy is to require the latest stable Rust.
  Source: https://github.com/bytecodealliance/wasmtime/blob/main/cranelift/README.md.
- **Wasmtime on `no_std`**: possible, but Cranelift and Winch are excluded, so no_std hosts run precompiled code
  only; JIT needs "a host operating system which supports creating executable memory pages on-the-fly."
  Source: https://docs.wasmtime.dev/stability-platform-support.html.
- **dynasm-rs**: a proc-macro assembler, x86/x64, AArch64 (to ARMv8.4, no SVE), RISC-V; needs rustc 1.77+.
  Source: https://github.com/CensoredUsername/dynasm-rs.

Reading for Rust (status: inferred):
- Runtime control is as good as C's: no collector, no scheduler, naked functions, `extern "C"`.
- The safety dividend is real for the *non-JIT* part (seed parsing, tables) and nil for the JIT part: writing
  bytes into a page, casting them to a function and calling it are `unsafe` in any language.
- The Rust JIT ecosystem (Cranelift, Wasmtime) assumes `std` and an OS and would be a large dependency that the
  system does not intend to keep, since the code generator is to be written in the one grammar.
- For self-replacement the system would target the `extern "C"` surface anyway. Rust's own ABI being unspecified
  is from memory (assumed) and does not matter if only `extern "C"` crosses.
- Six-week releases and a 406 MB toolchain are a heavier standing dependency than "any C compiler".

---

## 5. C

Measured (status: observed):
- `cc -O2` of the JIT test: 0.05 s; binary 33,816 bytes; links only libSystem.
- `cc -O2 -c runtime/membrane-boot.c` (1,199 lines): 0.19 s; at `-O0`: 0.06 s.
- `cc -O2 -c runtime/fkwu-uni.c` (34,149 lines): 5.52 s.
- Metal from plain C in about 20 lines (`.../exp/metal.c`): `dlopen` Metal, `MTLCreateSystemDefaultDevice`,
  `objc_msgSend` twice, prints "Apple M4 Max"; one `objc_msgSend` property read costs **about 5.9 ns**.
- Freestanding objects for x86_64/aarch64 Linux (ELF) and Windows (COFF) come out of Apple clang with
  `--target=... -ffreestanding -c`, no extra install (`out.txt` lines 90 to 94). Linking those needs a linker and
  system libraries this machine does not have.
- Apple clang 17.0.0 lives inside a 2.8 GB Xcode tree here; the smaller Command Line Tools were not measured.

Precedents (status: observed unless marked):
- **LuaJIT** (C + DynASM): its machine-code allocator covers POSIX `mmap`, Windows `VirtualAlloc`, W^X toggling
  ("Most of the time the memory pages holding machine code are executable, but NONE of them is writable"),
  and a `MAP_JIT` path with `pthread_jit_write_protect_np` guarded by `LUAJIT_ENABLE_OSX_HRT`; cache flush by
  `__clear_cache`, `sys_icache_invalidate` or `FlushInstructionCache`.
  Source: https://raw.githubusercontent.com/LuaJIT/LuaJIT/v2.1/src/lj_mcode.c. The fetch tool noted that the
  hardened-runtime branch has a `void` function returning a value, so that branch may be little exercised.
  DynASM is "a dynamic assembler for code generation engines" (https://luajit.org/dynasm.html).
- **CPython copy-and-patch JIT**: PEP 744 (created 2024-04-11): LLVM is a build-time dependency only; Clang is
  required because it is "the only C compiler with guaranteed tail calls"; no runtime dependency; "never both
  writable and executable." In 3.15 (released 2026-10-09) it is still "experimental": 11 to 12% over the
  tail-calling interpreter on AArch64 macOS, 7 to 8% on x86-64 Linux; builds with LLVM 21.
  Sources: https://peps.python.org/pep-0744/ , https://docs.python.org/3.15/whatsnew/3.15.html ,
  https://raw.githubusercontent.com/python/cpython/main/Tools/jit/README.md.
- **Copy-and-patch** itself: code generation "two orders of magnitude faster" than LLVM -O0, the result 14% faster
  than LLVM -O0 code. Source: https://arxiv.org/abs/2011.13127 (v1 2020-11-26, v3 2021-09-15).
- **MIR**: a JIT in C, generator about 23.4K lines against about 2,420K for GCC; c2m geomean 0.91 of `gcc -O2`
  on 15 small benchmarks; x86_64 and aarch64 on Linux and macOS. Source: https://github.com/vnmakarov/mir.
- **QBE**: "70% of the performance of industrial optimizing compilers in 10% of the code" (a goal), in C; amd64,
  arm64, riscv64. Source: https://c9x.me/compile/.
- **TinyCC**: "The Smallest ANSI C compiler", claims about 10 times faster than `gcc -O0`, runs C directly
  (`tcc -run`), targets i386, x86_64, arm, aarch64, riscv64, has a Mach-O writer.
  Source: https://github.com/TinyCC/tinycc. Not installed here; not tested.
- **Zig's own bootstrap is C**: since 2022-12-07 Zig commits a WebAssembly seed (`zig1.wasm.zst`, 637 KB
  compressed) and about **4,000 lines of portable C** (a wasm-to-C translator needing 22 WASI imports) that
  replaced about 80,000 lines of C++; the chain is reproducible ("`zig3` and `zig4` match byte-for-byte").
  Source: https://ziglang.org/news/goodbye-cpp/. This is the closest published precedent for the shape asked
  for here: *a small numeric seed plus a few thousand lines of C that the seed immediately outgrows*.
- **Bootstrappable builds** converge on C: a 256-byte hex0 seed builds hex1, hex2, M0, a per-architecture `cc_*`,
  then M2-Planet (a C subset compiler), then mescc-tools, for AMD64, x86, AArch64 and RISC-V
  (https://github.com/oriansj/stage0-posix); Mes/mescc builds TinyCC, which builds GCC 2.95, which builds GCC 4.7.4
  (https://bootstrappable.org/projects/mes.html); live-bootstrap automates hex0 to a Linux system
  (https://github.com/fosslinux/live-bootstrap); the stated goal is "to minimize the amount of bootstrap binaries"
  (https://bootstrappable.org/). A C bootstrap can therefore itself be reached from a few hundred auditable
  bytes on Linux. Status of that last sentence: inferred.
- V8 and JavaScriptCore are C++ (assumed; not fetched as a language fact). Their W^X practice is cited in 2.4.

Reading for C (status: inferred):
- Zero impedance with every contract in section 1. Apple's strictest JIT path is literally a C macro
  (`PTHREAD_JIT_WRITE_ALLOW_CALLBACKS_NP`).
- No runtime. What is in the binary is what was written; 34 KB here.
- The weakness is known: no memory safety, undefined behaviour, and nothing in the language stops accretion
  (section 2.3). Both are answered by the plan itself (the C shrinks as emitted code replaces it) and by a hard
  line budget that a gate reads, not by the language.

---

## 6. Outside options

### 6.1 Zig

Status: observed.
- Latest stable 0.17.1 (2026-11-02); master is 0.18.0-dev. macOS aarch64 tarball 51 MiB.
  Source: https://ziglang.org/download/.
- 0.17.0 notes: stabilising the language is a requirement for 1.0 and "some major decisions remain open"; this
  release removes `@cImport`, array multiplication, `void{}`, the `errdefer` capture, `i0`, and changes `@bitCast`
  in a way that "has the potential to break existing code without triggering a compile error"; the aarch64
  self-hosted backend is "blocked on Linker enhancements"; incremental compilation works for most projects on
  x86_64-linux only; aarch64-macos is Tier 2. Source: https://ziglang.org/download/0.17.0/release-notes.html.
- Cross-compilation is first class: "Zig builds for all supported targets independently of the host"; `zig cc`
  compiles C and ships libc for its targets; "no hidden control flow, no hidden memory allocations."
  Source: https://ziglang.org/learn/overview/.

Reading (inferred): as a *language* for a file that must sit in a repository and keep building for years, a
pre-1.0 language with silent semantic breaks is the wrong ground. As a *tool* (`zig cc` to cross-link the C
bootstrap for Linux and Windows from the Mac) it is attractive and optional. Not installed here; not tested.

### 6.2 Hand-written assembly or hex

- Possible and proven on Linux (256-byte hex0, section 5), where static binaries may issue system calls directly.
- On macOS, wazero's rationale quotes Apple: "Apple does not support statically linked binaries on Mac OS X"
  (source in 3.4). Every measured binary here, including pure Go with cgo off, links libSystem (observed), and
  the body's own Mach-O emitter leans on `ld` for linking and the ad-hoc signature (section 2.3). A hex seed on
  macOS would have to be a complete signed, dynamically linked Mach-O image: thousands of opaque bytes, per
  architecture, per OS.
- Forth and Lisp show how little host is needed once a language can extend itself: sectorlisp is "a 512-byte
  implementation of LISP" with a portable C reference (https://github.com/jart/sectorlisp, observed);
  jonesforth is a little assembly for primitives with "all the rest of the language and compiler ... written in
  FORTH itself" (search result for the jonesforth source, 2026-11-24; not opened: assumed). Project Oberon
  (New Edition 2013) ships OS, compiler and CPU as one small readable system
  (https://people.inf.ethz.ch/wirth/ProjectOberon/index.html, observed; its line counts were not on the page).

Reading (inferred): the assembly bootstrap is the *end state*. When the system emits a signed Mach-O/ELF/PE that
contains its own host doors, that image is the hand-free "hex seed", and the C file becomes a fallback that can
rebuild it from nothing but a C compiler.

---

## 7. Comparison table

Legend: (o) observed today, (d) documented in a fetched source, (i) inferred, (a) assumed.

| Criterion | C | Go | Rust | Zig | asm / hex |
|---|---|---|---|---|---|
| Hidden runtime | none (o: 34 KB binary, libSystem only) | collector, scheduler, signal handlers, moving stacks (d); held a GC 627 ms behind JIT code (o) | none in `no_std` (o: 50 KB) | none (d) | none |
| Stack and calling-convention control | platform ABI; inline asm for the rest (a) | private unstable ABI, R28 = goroutine (d); foreign frames unsupported (d) | naked functions stable since 1.88 (d); `extern "C"` (o) | naked/callconv (a) | total |
| `MAP_JIT` + per-thread toggle + icache | direct (o) | needs cgo or runtime-internal trampolines (i from o+d); pure-Go mprotect path killed under hardened runtime (o) | direct via `extern "C"` (o) | direct (a) | direct |
| Apple write-allowlist path (C macro) | native (d) | not reachable without C (i) | via `link_section` by hand (i) | by hand (a) | by hand |
| Call into C / Objective-C / Metal | 0.8 ns call, 5.9 ns `objc_msgSend` (o) | 23.6 ns per cgo call (o), 1.26 cut it ~30% (d) | same as C (i; 1.7 ns loop reading here is a harness artefact) | same as C (a) | same as C |
| Build of the tiny test | 0.05 s (o) | 0.10 s pure, 0.65 s with cgo (o) | 0.09 s warm, 0.95 s first (o) | not measured | n/a |
| Build at size | 1.2K lines 0.19 s; 34K lines 5.5 s at -O2 (o) | fast (a) | slower at scale (a) | fast claim (d) | n/a |
| Binary size of the tiny test | 33.8 KB (o) | 2.45 MB; 1.60 MB stripped (o) | 455 KB std; 50 KB no_std (o) | 9.9 KB static Linux hello claimed (d) | bytes |
| Toolchain on disk | Apple clang inside 2.8 GB Xcode (o); tcc is tiny (a) | 270 MB, self-contained (o) | 406 MB, host target only (o) | 51 MiB tarball, all targets (d) | none |
| Cross-compilation | freestanding objects for 4 foreign targets out of the box (o); linking needs sysroots (o) or `zig cc` (d) | Linux targets out of the box (o); needs cgo off | extra std per target (o) | first class (d) | manual per target |
| Language stability | decades; ISO standard (a) | Go 1 promise kept through 1.27 (d) | stable channel, editions; 1.100 (d) | pre-1.0, breaking each release (d) | ISA-stable |
| Memory safety | none | safe outside `unsafe`, but JIT interop is all `unsafe`/assembly (i) | safe outside `unsafe`; JIT core is `unsafe` (i) | checked in safe modes (d) | none |
| Ease of self-replacement | highest: the bootstrap *is* a set of C-ABI functions (i) | lowest: the runtime cannot be replaced piecewise (i) | high through `extern "C"` only (i) | high (i) | it is the replacement |
| Freestanding / no-libc | yes on Linux/Windows; macOS requires libSystem for everyone (d, o) | no (d: darwin always goes through libSystem; runtime mandatory) | yes (`no_std`) (o) | yes (d) | Linux yes; macOS no (d) |
| Precedent for "tiny host + self-growing system" | LuaJIT, CPython JIT, MIR, QBE, tcc, Zig's 4K-line C stage, the whole bootstrappable chain (d) | wazero (d); Go itself left C in 1.5 (d) | Cranelift/Wasmtime, dynasm-rs (d) | Zig bootstraps *through C* (d) | stage0, sectorlisp, Forth (d) |
| Already proven in this body | yes: `runtime/*.c` (o) | NUMS.Go is Go (o) | sibling kernels exist (a; not read in this study) | no | Mach-O emitter in Form (o) |

---

## 8. Recommendation

**C, as a strict small subset, with the shape of a door table.** Status: inferred.

What "C" means here:
1. One translation unit, target under about 3,000 lines, C99/C11 with no compiler extensions beyond inline
   assembly for the entry trampoline and `__builtin___clear_cache`. It should also build with TinyCC and with
   `zig cc`; that is the test that it stayed simple (both untested today).
2. No libc *use* beyond a declared list of OS symbols (the present seed already declares `mmap` and
   `pthread_jit_write_protect_np` by hand with literal flag values: `runtime/fkwu-uni.c:4891-4918`).
3. The bootstrap's whole public surface is a **table of host doors with C-ABI signatures**: map executable
   region, begin/end write for this thread, flush icache, map file, load library, find symbol, raw call with N
   words, create thread, clock, exit. The seed's emitted code receives the table pointer and calls nothing else.
4. A tiny evaluator or copy-and-patch tier for the seed, just enough to run the one grammar's own code generator
   once. After that, emitted machine code replaces door after door; finally the system writes a signed image
   that contains its own doors, and the C file remains as the cold-start path from "nothing but a C compiler".
5. Per-thread JIT write discipline and one reserved JIT region from the first line, so the same binary is correct
   with and without the hardened runtime (section 2.2).
6. A line-count gate on the C file. The present 34,149-line seed shows that only a budget keeps a bootstrap small.

Why this beats the others for *this* mission (each clause points at its evidence):
- runtime control: no second kernel inside the process (3.3 against 5);
- Apple JIT: all three API tiers are C interfaces (2.1); measured working in all signing states (2.2);
- FFI: Metal at 5.9 ns per message with no binding layer (5); cgo is 30 times a direct call (3.2);
- iteration: 0.05 to 0.2 s builds at the target size (5);
- self-replacement: the unit of replacement is one C-ABI function, and the body already emits Mach-O objects (2.3);
- precedent: Zig, a language with every reason to avoid C, chose "seed + about 4,000 lines of C" for exactly this
  problem (5).

## 9. When the recommendation flips

| If this becomes true | Then prefer | Why |
|---|---|---|
| The bootstrap is expected to stay above roughly 10K lines for years and parse untrusted input (network, foreign seeds) before the system's own boundary checks exist | Rust (`no_std`, `extern "C"` surface) | the safe part becomes the larger part; 50 KB floor measured; naked functions stable |
| A ready-made optimizing code generator is wanted *in the bootstrap* instead of one written in the one grammar | Rust + Cranelift | production-ready, aarch64 + x86-64; accepts std/OS dependence and API churn |
| The first milestone is a hosted prototype where emitted code is rare, short, and never calls back, and reuse of NUMS.Go code matters more than replaceability | Go | 0.1 s builds, trivial cross builds, direct calls work at 0.8 ns; accept the GC stall and no hardened runtime without cgo |
| The Go runtime gains supported foreign JIT frames (the 2026-04-30 pre-proposal or issue 78189 lands) | re-evaluate Go | removes the "unexpected return pc" and stall class |
| Zig reaches 1.0 with a native aarch64 backend and Mach-O incremental linking | re-evaluate Zig as the language | best cross story and freestanding control; today it breaks each release |
| Auditability from a few hundred bytes becomes the first criterion (supply-chain trust over convenience) | hex0-style seed on Linux first, C reached through the stage0 chain | already proven chain; macOS cannot host it natively |
| Apple removes `pthread_jit_write_protect_np` for non-allowlisted apps on macOS | stays C | the allowlist path is a C macro; cost: emission must run inside a registered callback |
| The target includes iOS | stays C, design changes | `pthread_jit_write_protect_np` "isn't available on iOS"; interpretation or ahead-of-time images only |

## 10. Limits of this study, stated as such

- One machine, one run per reading, tiny programs. The nanosecond figures are orders of magnitude, not benchmarks.
- The Rust-to-JIT call reading (1.7 ns) differs from C's (0.8 ns) only because of how the loop was guarded; no
  conclusion is drawn from it.
- Zig and TinyCC are not installed; everything about them is from their pages.
- Pages that refused the fetch: the Guix full-source-bootstrap post (403), gnu.org/software/mes (429), the
  sectorlisp v2 article (connection refused), the Outflank macOS JIT post (403). Their claims are either sourced
  elsewhere above or marked assumed.
- Windows and Linux hosts were read, not run.
- The hardened-runtime kills were observed as signal 9; the kernel's stated reason was not read from the log.
- The C++ engines (V8, JavaScriptCore) were not studied as a language option; C++ was not asked for.

## 11. Experiment files (all under the session scratchpad)

- `scratch/exp/jit.c`
- `scratch/exp/metal.c`
- `scratch/exp/gojit/jit.go`
- `scratch/exp/gogc/gc.go`
- `scratch/exp/gocgo/cgo.go`
- `scratch/exp/jit.rs`
- `scratch/exp/jit_nostd.rs`
- `scratch/exp/run.sh` and `run2.sh` (drivers)
- `scratch/exp/out.txt` and `out2.txt` (readings)

No file in any studied repository was modified; compiler outputs for `runtime/*.c` were written to the scratchpad.

## 12. Source list (fetched 2026-11-24)

Apple
- https://developer.apple.com/tutorials/data/documentation/apple-silicon/porting-just-in-time-compilers-to-apple-silicon.json (undated)
- https://developer.apple.com/tutorials/data/documentation/bundleresources/entitlements/com.apple.security.cs.allow-jit.json (undated)
- https://developer.apple.com/tutorials/data/documentation/bundleresources/entitlements/com.apple.security.cs.allow-unsigned-executable-memory.json (undated)

Go
- https://go.dev/doc/go1.26 (February 2026) · https://go.dev/doc/go1.27 (August 2026) · https://go.dev/doc/go1.23 · https://go.dev/doc/go1.5
- https://go.dev/src/cmd/compile/abi-internal · https://raw.githubusercontent.com/golang/go/master/src/runtime/HACKING.md · https://pkg.go.dev/os/signal
- https://github.com/golang/go/issues/20123 (2017-04-25) · https://groups.google.com/g/Golang-dev/c/mABYStr5aYs (2026-04-30)
- https://raw.githubusercontent.com/tetratelabs/wazero/main/RATIONALE.md · https://raw.githubusercontent.com/tetratelabs/wazero/main/internal/platform/mmap_unix.go · https://github.com/tetratelabs/wazero/tree/main/internal/platform
- https://github.com/ebitengine/purego · https://raw.githubusercontent.com/ebitengine/purego/main/syscall.go · https://raw.githubusercontent.com/nelhage/gojit/master/README.md

Rust
- https://blog.rust-lang.org/ (1.100.0 on 2026-11-12) · https://blog.rust-lang.org/2025/06/26/Rust-1.88.0/ · https://doc.rust-lang.org/reference/abi.html
- https://github.com/bytecodealliance/wasmtime/blob/main/cranelift/README.md · https://docs.wasmtime.dev/stability-platform-support.html · https://github.com/CensoredUsername/dynasm-rs
- https://bytecodealliance.github.io/zulip-archive/stream/225357-git-wasmtime/topic/wasmtime.20.2F.20issue.20.2311989.20Support.20mmap.20flag.20MAP_JIT.20for.20MACOS.html

C and JIT precedents
- https://raw.githubusercontent.com/LuaJIT/LuaJIT/v2.1/src/lj_mcode.c · https://luajit.org/dynasm.html · https://luajit.org/luajit.html
- https://peps.python.org/pep-0744/ (2024-04-11) · https://docs.python.org/3.14/whatsnew/3.14.html (2025-10-07) · https://docs.python.org/3.15/whatsnew/3.15.html (2026-10-09) · https://raw.githubusercontent.com/python/cpython/main/Tools/jit/README.md
- https://arxiv.org/abs/2011.13127 (2020-11-26; v3 2021-09-15) · https://github.com/vnmakarov/mir · https://c9x.me/compile/ · https://github.com/TinyCC/tinycc
- https://v8.dev/blog/control-flow-integrity (2023-10-09)

Zig
- https://ziglang.org/download/ (0.17.1 on 2026-11-02) · https://ziglang.org/download/0.17.0/release-notes.html · https://ziglang.org/learn/overview/ · https://ziglang.org/news/goodbye-cpp/ (2022-12-07)

Bootstrappable and minimal systems
- https://bootstrappable.org/ · https://bootstrappable.org/projects/mes.html · https://github.com/oriansj/stage0-posix · https://github.com/fosslinux/live-bootstrap
- https://github.com/jart/sectorlisp · https://people.inf.ethz.ch/wirth/ProjectOberon/index.html

Other hosts
- https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualalloc (2024-02-05, updated 2025-07-01) · https://man7.org/linux/man-pages/man2/PR_SET_MDWE.2const.html · https://github.com/KhronosGroup/Vulkan-Headers

Local files read
- `coherence-kernel/runtime/membrane-boot.c` (2026-11-05)
- `coherence-kernel/runtime/fkwu-uni.c` (2026-11-20)
- `coherence-kernel/form/form-stdlib/form-macho.fk`
- `NUMS.Go` (directory listing only)
