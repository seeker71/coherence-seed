# The cost of content identity, measured on this machine

Written 2026-11-24. Closes gap G9 of the critic's review ("No measurement stands behind the new design's cost
claims", `S/critic.md:147-154`) for one item: hashing cost per node and the cost of a digest-keyed store. It also
supplies act (5) of the critic's shortest path (`S/critic.md:436-439`) in synthetic form and the measurement
that not-ready item N2 asks for (`S/critic.md:413-415`).

## 0. Paths, status words, what was and was not done

- `S` = `scratch`
- `E` = `S/exp-hash` (everything I wrote and ran is here; nothing else was written anywhere)
- `R` = `coherence-kernel`
- **observed** = I ran it or read the line in this session. **inferred** = reasoning from observed things, the basis
  is named. **assumed** = not verified here.
- All timings are from 2026-11-24, 18:59 to 19:07 local time, on a machine that other agents were using at the
  same moment (`uptime` load average 3.3 to 5.1 throughout; `ps` at 19:01 showed two `fkwu` processes and one `git`
  at 100% CPU, none of them mine; observed). I did not run `fkwu`. I modified no repository file.
- Not done: no hash was timed over the nodes of a real unit of the old body (that needs a run of the old runtime,
  which the ground rules exclude). The nodes are synthetic 40-byte records. The official BLAKE3 implementation
  (with SIMD) was not available on this disk and nothing was downloaded, so BLAKE3 figures are for my scalar C.

### Machine (observed, `sysctl`, `sw_vers`, `uname -a`, `cc --version`, 2026-11-24)

| Item | Value |
|---|---|
| Chip | Apple M4 Max (`machdep.cpu.brand_string`), model `Mac16,5` |
| Cores | 16 logical: 12 "Performance" (`hw.perflevel0`), 4 "Efficiency" (`hw.perflevel1`) |
| Memory | 137,438,953,472 bytes (128 GiB) |
| Page / cache line | 16,384 bytes / 128 bytes |
| Caches | P core: L1d 131,072, L2 16,777,216. E core: L1d 65,536, L2 4,194,304 |
| CPU features | `FEAT_SHA256`=1, `FEAT_SHA512`=1, `FEAT_SHA3`=1, `FEAT_AES`=1, `FEAT_PMULL`=1 |
| OS | macOS 26.3.1 (a), build 25D771280a; Darwin 25.3.0, `xnu-12377.91.3~2/RELEASE_ARM64_T6041` |
| Compiler | Apple clang 17.0.0 (clang-1700.6.3.2), target arm64-apple-darwin25.3.0; every program built `cc -O2 -Wall` |
| Power | AC attached, `powermode 0` (`pmset -g`) |
| Clock while measuring | one dependent 64-bit add took 0.222 to 0.262 ns in undisturbed rows (3.8 to 4.5 GHz if an add is one cycle); some rows show 0.28 to 0.35 ns (section 6) |

## 1. Programs (all in `E`, all written in this session)

| File | What it is | Lines |
|---|---|---|
| `E/common.h` | timer (`clock_gettime_nsec_np(CLOCK_UPTIME_RAW)`), min/median, QoS request, optimisation barriers, core and clock probes | 60 |
| `E/sha256_arm.h` | SHA-256 written directly on the ARMv8 instructions (`vsha256hq_u32`, `vsha256h2q_u32`, `vsha256su0q_u32`, `vsha256su1q_u32`) | 68 |
| `E/mf128.h` | the 128-bit multiply-fold hash (section 2.2); the whole file is 48 lines including its description | 48 |
| `E/blake3_min.h` | BLAKE3, unkeyed, 32-byte output, scalar C, from the specification as I know it; any length (tree included) | 75 |
| `E/verify.c` | correctness of all three before timing | 125 |
| `E/noble_check.js` | prints BLAKE3 digests from an independent implementation already on this disk | 11 |
| `E/bench_insn.c` | latency of `add`, `mul`, `umulh`, `sha256h`, `sha256su1` on the core in use | 42 |
| `E/bench_base.c` | function call; bump allocation plus 40-byte `memcpy` | 94 |
| `E/bench_hash.c` | ns per digest for 16, 40, 64, 256, 4096 bytes; throughput and latency modes; 9 repetitions | 105 |
| `E/bench_intern.c` | open-addressing intern table keyed by the digest, 2^14 / 2^20 / 2^24 nodes; 5 repetitions | 238 |

Build and run, exactly as done:

```
cd E
cc -O2 -Wall -o verify verify.c && ./verify
cc -O2 -Wall -o bench_insn bench_insn.c && ./bench_insn
cc -O2 -Wall -o bench_base bench_base.c && ./bench_base
cc -O2 -Wall -o bench_hash bench_hash.c && ./bench_hash
for k in 0 1 2 3 4 5; do cc -O2 -Wall -DHASHSEL=$k -o bench_intern_$k bench_intern.c; done
for k in 0 5 2 1 3 4; do ./bench_intern_$k 20; done      # 1,048,576 nodes
for k in 0 5 2 1 3 4; do ./bench_intern_$k 24; done      # 16,777,216 nodes
for k in 0 5 2 1 3 4; do ./bench_intern_$k 14; done      # control: table inside the cache
node noble_check.js Coherence-Network/web/node_modules/@noble/hashes
```

Raw outputs are the `*_out*.txt` files in `E` and are reproduced whole in the appendix.

## 2. Correctness before speed (observed, `E/verify_out.txt`)

### 2.1 SHA-256 on the ARMv8 instructions

`sha256_arm` equals CommonCrypto `CC_SHA256` for every length 0 to 4200 over random bytes: 0 mismatches.
`sha256_arm("abc")` = `ba7816bf...f20015ad`, equal to `printf abc | shasum -a 256` (observed).

### 2.2 mf128: definition and quality checks

Definition (`E/mf128.h:1-12`, the code is lines 13-48):

- State: two 64-bit lanes `a`, `b`. Start `a = K0 xor length`, `b = K1`.
- Absorb: each 16 input bytes are two little-endian words XORed into the lanes, then one round.
- Round: `p = a * KA`, `q = b * KB` as full 64 x 64 to 128-bit products; `a' = lo(p) xor hi(q)`,
  `b' = lo(q) xor hi(p)`. `KA`, `KB` are odd, so the low half is a bijection of its own lane and the high half
  carries the lane's top bits into the other lane. The two multiplies do not depend on each other.
- Tail: the last 1 to 16 bytes are read as the final 16 bytes of the input (overlapping); inputs under 16 bytes
  use overlapping 8-, 4- or 1-byte reads; the length in the start state separates the cases.
- Finish: three more rounds.
- It is **not cryptographic**: an adversary can construct collisions. The zero state is a fixed point of the round.

Checks (observed):

- Avalanche, 40-byte inputs, 40,000 random inputs, each of 320 input bits flipped, each of 128 output bits
  counted: flip probability over the 40,960 bit pairs min 0.4900, mean 0.5000, max 0.5098 (one pair's sigma is
  0.0025, so the extremes sit at 4 sigma of 40,960 trials, which is what chance gives).
- 4,194,304 low-entropy nodes (five words, only one counter word varies): equal values in the four 32-bit windows
  of the digest: 2029, 2037, 1993, 1994, against 2048 expected of a random function (sigma 45). Zero collisions in
  the 64-bit word 0.
- This is a sanity check, not a test suite. No SMHasher-class battery was run (it would need a download).

### 2.3 BLAKE3

- `blake3_min("")` = `af1349b9f5f9a1a6a0404dea36dcc9499bcb25c9adc112b7cc9a93cae41f3262`. This is the official
  empty-input vector; I hard-coded it from memory, so the reference string itself is **assumed**; that a
  from-the-specification implementation reproduces all 256 bits of it is **observed** and cannot happen by chance.
- Another route (observed): `@noble/hashes` 1.8.0, a JavaScript implementation that was already installed at
  `Coherence-Network/web/node_modules/@noble/hashes/blake3.js`. `E/noble_check.js` prints
  the digests of `""`, `"abc"` and of the official test pattern (byte i = i mod 251) at lengths 0, 1, 2, 16, 40,
  63, 64, 65, 256, 1023, 1024, 1025, 2048, 2049, 3000, 4096, 8192. `diff E/noble_out.txt E/mine_b3.txt` is empty:
  all 19 digests are identical, including the multi-chunk tree cases. `"abc"` =
  `6437b3ac38465133ffb63b75273a8db548c558465d79db03fd359c6cd5bd9d85`.
- So item (3) of the task **was measured**, with this limit: scalar code. For inputs of at most 64 bytes BLAKE3 is
  a single compression, where SIMD across chunks cannot help (inferred from the algorithm's structure); for 4096
  bytes (four chunks) a production BLAKE3 hashes the chunks in parallel lanes, so my 4096-byte figure is an upper
  bound, by an unmeasured factor.

## 3. Results

### 3.1 The yardsticks (observed, `E/base_out_run2.txt`, `E/base_out.txt`, `E/insn_out.txt`; 9 repetitions)

| Operation | min ns | median ns | Notes |
|---|---|---|---|
| one dependent 64-bit add | 0.222 | 0.246 | about one cycle; 0.250 / 0.252 in the first run |
| function call and return, direct, not inlined | 0.696 | 0.718 | 0.750 / 0.759 in the first run: 3 cycles |
| function call through a pointer | 0.696 | 0.733 | same as direct |
| build a 40-byte node in registers and spill it | 0.383 | 0.385 | the loop the next two rows contain |
| bump allocation + 40-byte `memcpy`, arena inside L1 | 1.893 (1.708 first run) | 1.956 | includes the 0.38 of building the node |
| bump allocation + 40-byte `memcpy`, streaming over 640 MiB | 2.284 | 2.357 | 2.274 / 2.287 first run |
| `mul` (low 64) | 0.759 | 0.763 | 2.99 add-times |
| `umulh` + `orr` | 1.006 | 1.018 | 3.96 add-times |
| one mf128 round as a chain | 1.194 | 1.200 | 4.70 add-times |
| `sha256h` chain | 1.006 | 1.010 | 3.96 add-times (4 SHA-256 rounds) |
| `sha256su1` chain | 0.754 | 0.756 | 2.97 add-times |

One row of `bench_insn` I do not trust: the `sha256h` + `sha256h2` pair measured 8.41 add-times, but the bulk
SHA-256 result below (22 ns per 64-byte block, 16 pairs) allows at most about 5.6 cycles per pair, so the barrier
I put between the two instructions added latency of its own (inferred).

A node that takes no identity therefore costs **1.2 to 2.3 ns** (bump + copy; the 1.2 figure is the `gen` phase
of the intern benchmark, which also computes two 64-bit mixes per node).

### 3.2 Digest cost per operation (observed, `E/hash_out_run3.txt`; 9 repetitions; min / median ns)

`tput` = independent digests back to back (the out-of-order core overlaps them). `lat` = the address of the next
input depends on the previous digest, so nothing overlaps; this includes one L1 load and the index arithmetic.
Length is not a compile-time constant except in the `fix40` rows. Inputs sit in about 64 KiB (inside L1).

| Function | mode | 16 B | 40 B | 64 B | 256 B | 4096 B |
|---|---|---|---|---|---|---|
| `CC_SHA256` (CommonCrypto one-shot) | tput | 145.5 / 147.3 | 97.2 / 101.8 | 179.5 / 184.5 | 213.6 / 224.4 | 1309 / 1445 |
| | lat | 149.5 / 151.2 | 101.0 / 107.1 | 172.7 / 179.0 | 204.9 / 219.2 | 1318 / 1460 |
| `sha256_arm` (direct instructions) | tput | 16.4 / 16.5 | 16.0 / 16.5 | 31.6 / 33.8 | 94.8 / 101.9 | 1445 / 1473 |
| | lat | 35.9 / 36.1 | 32.2 / 35.2 | 55.4 / 60.5 | 121.5 / 135.2 | 1457 / 1592 |
| `blake3_min` (scalar) | tput | 48.6 / 48.8 | 51.3 / 53.4 | 49.5 / 50.0 | 206.2 / 207.6 | 3485 / 3505 |
| | lat | 60.3 / 61.9 | 60.5 / 61.8 | 62.2 / 62.7 | 218.7 / 219.6 | 3505 / 3532 |
| `mf128` | tput | 1.55 / 1.56 | 2.75 / 2.77 | 3.44 / 3.49 | 14.1 / 14.7 | 326 / 345 |
| | lat | 6.27 / 6.57 | 8.52 / 8.97 | 9.77 / 10.25 | 26.1 / 27.1 | 341 / 362 |

With the length fixed at 40 at compile time (`fix40` rows): `sha256_arm` 15.0 / 16.0 tput and 28.2 / 29.2 lat;
`blake3_min` 48.3 / 48.9 and 59.3 / 60.5; `mf128` 2.61 / 2.66 and 8.85 / 9.44.

Read as rates at 4096 bytes: `sha256_arm` 0.353 ns/byte (2.83 GB/s), `CC_SHA256` 0.320 ns/byte (3.13 GB/s),
`blake3_min` 0.851 ns/byte (1.18 GB/s), `mf128` 0.080 ns/byte (12.6 GB/s).

What the table shows (observed unless marked):

1. At node size the hardware SHA-256 costs **16 ns** when digests can overlap and **28 to 36 ns** when the digest
   is needed before the next step. 16, 40 bytes are one block; 64 bytes is two blocks because of the padding.
2. CommonCrypto's one-shot call costs 97 to 150 ns for inputs of 16 to 40 bytes: **6 to 9 times** the direct
   instructions on the same input, and it equals them at 4096 bytes. So the call carries a fixed cost near 80 to
   130 ns. It was slower for 16 bytes than for 40 bytes in all three runs; I have no explanation.
3. Scalar BLAKE3 costs 48 to 51 ns for anything up to 64 bytes (one compression): **3.2 times** hardware SHA-256
   at 40 bytes, 2.4 times at 4096 bytes.
4. mf128 costs 2.6 to 2.8 ns at 40 bytes when overlapped (about 12 cycles: it is bound by the multiplier ports,
   20 multiply micro-operations) and 8.5 to 8.9 ns as a latency: **5.8 times cheaper** than hardware SHA-256 in
   throughput, 3.2 to 3.8 times cheaper in latency.
5. Per byte, SHA-256 costs about the same whether the bytes arrive as 40-byte nodes (0.40 ns/byte) or as one
   4096-byte block (0.353 ns/byte). Granularity does not change the hashing work per byte; it changes how many
   bytes are hashed at all and how many store probes happen (inferred from rows 1 and the 4096 column).

Two other runs exist. `E/hash_out.txt` (second run, probed) agrees within about 10% for most rows and is up to 40%
slower in the 64-byte rows, where its clock probe reads 0.324 ns instead of 0.25. `E/hash_out_run1_unprobed.txt`
(first run, before I added the probes) is 1.4 to 2.3 times slower in several rows (SHA-256 4096 bytes: 2902 ns
against 1445; mf128 4096: 808 against 326). I cannot say which core or clock that first run had; the disassembly
of the mf128 loop (`otool -tvV bench_hash`, 5 rounds, 20 multiplies, no calls) was already optimal, which is how I
knew the first numbers were the machine and not the code. Independent reading taken during that disturbed window:
`openssl speed -seconds 1 -bytes 4096 -evp sha256` (OpenSSL 3.5.9) gave 1,665,798.59k bytes/s (1.67 GB/s), and
218,255.20k at 40 bytes (183 ns per 40-byte digest through EVP).

### 3.3 Interning (observed, `E/intern20_out.txt`, `E/intern24_out.txt`, `E/intern14_out.txt`; 5 repetitions; min ns per node)

Table: open addressing, linear probing, 2N slots (load 0.5), slot = first 64 digest bits masked, a hit requires
the whole stored digest to be equal, handle = 32-bit arena index + 1. Nodes: N distinct 40-byte records. Timed
phases use prefaulted memory. `gen` = build the node and bump + copy it, no identity. `ins_pre` / `hit_pre` =
table cost alone (digests precomputed). `mint` = the whole act for a new node (build, digest, probe, bump + copy,
store). `hit` = the whole act for a node already present, visited in a permuted order.

N = 2^20 = 1,048,576:

| Variant | entry | gen | hash | ins_pre | hit_pre | mint | hit | bytes/node |
|---|---|---|---|---|---|---|---|---|
| mf128, 16-byte digest stored | 24 B | 1.16 | 2.06 | 7.48 | 12.93 | 18.54 | 19.57 | 88.1 |
| hash-consing: mf128 picks the slot, no digest stored, equality by node bytes | 8 B | 1.44 | 2.05 | 5.66 | 21.01 | 14.40 | 38.46 | 56.0 |
| `sha256_arm`, truncated to 16 bytes stored | 24 B | 1.17 | 15.73 | 7.77 | 13.16 | 35.41 | 35.47 | 88.1 |
| `sha256_arm`, 32 bytes stored | 40 B | 1.18 | 16.41 | 11.09 | 19.00 | 39.59 | 43.29 | 120.1 |
| `blake3_min`, 32 bytes stored | 40 B | 1.33 | 47.23 | 11.06 | 19.28 | 121.02 | 125.85 | 120.1 |
| `CC_SHA256`, 32 bytes stored | 40 B | 1.45 | 107.45 | 11.85 | 21.84 | 211.40 | 224.94 | 120.1 |

N = 2^24 = 16,777,216:

| Variant | table | gen | hash | ins_pre | hit_pre | mint | hit | resident growth |
|---|---|---|---|---|---|---|---|---|
| mf128 / 16 B | 768 MiB | 1.22 | 2.13 | 12.22 | 20.29 | 28.11 | 27.06 | 1408.0 MiB = 88.0 B/node |
| hash-consing, no digest | 256 MiB | 1.36 | 2.08 | 9.39 | 27.43 | 25.11 | 45.51 | 896.0 MiB = 56.0 B/node |
| `sha256_arm` / 16 B (disturbed: clock probe 0.349 ns at start) | 768 MiB | 1.52 | 18.96 | 13.53 | 22.56 | 50.93 | 48.23 | 1408.0 MiB = 88.0 B/node |
| `sha256_arm` / 32 B | 1280 MiB | 1.19 | 16.47 | 15.22 | 23.57 | 44.75 | 47.79 | 1920.0 MiB = 120.1 B/node |
| `blake3_min` / 32 B | 1280 MiB | 1.38 | 47.84 | 15.22 | 24.81 | 156.11 | 160.38 | 1920.0 MiB = 120.1 B/node |
| `CC_SHA256` / 32 B | 1280 MiB | 1.31 | 104.17 | 14.43 | 23.33 | 212.12 | 224.65 | 1920.0 MiB = 120.1 B/node |

Resident memory is the growth of `phys_footprint` (and of `resident_size`, equal within 1 MiB) across a mint into
fresh mappings; it equals the layout arithmetic (40-byte arena record + 2 slots x entry size). That cold pass,
page faults included, cost 26.2 / 29.7 ns per node for mf128 and 46.0 / 53.5 for `sha256_arm` 32 B (one run each).

Control, N = 2^14 (table 0.2 to 1.2 MiB, inside the cache), min ns: mint = 10.1 (mf128), 5.7 (hash-consing),
21.8 (`sha256_arm` 16 B), 21.1 (`sha256_arm` 32 B), 59.4 (`blake3_min`), 116.6 (`CC_SHA256`). In this control the
repetitions get faster one after another (for example 12.8, 12.1, 11.1, 10.6, 10.1), which I read as the branch
predictor learning a short fixed sequence, so the minimum is optimistic here (inferred).

What the tables show:

1. **An identity at node granularity costs at least 12 times the node itself (12 to 37 times with the two cheapest digests).** No identity: 1.2 ns (to 2.3 ns
   streaming). Minted with mf128: 18.5 ns at 1 M nodes, 28.1 ns at 16.8 M. With hardware SHA-256: 39.6 and 44.8 ns.
   With scalar BLAKE3: 121 and 156 ns. With the CommonCrypto call: 211 and 212 ns (observed).
2. **Most of that is the store, not the digest, once the digest is cheap.** mf128: 2.1 ns digest inside a 28.1 ns
   mint. Hardware SHA-256: 16.5 of 44.8 ns (observed; the split is by subtraction).
3. **A slow digest also un-overlaps the memory misses.** In the cache-resident control, mint is digest + 5 to 12 ns.
   At 2^20 and 2^24 the excess over the digest is 16 to 28 ns for mf128 and `sha256_arm` (32 in the one disturbed run), but 74 to 108 ns for
   `blake3_min` and `CC_SHA256`: with about 50 ns or more of work between probes, the core no longer has the next
   probe's load in flight, and each node pays one whole memory latency (inferred from the three table sizes; I did
   not confirm it with a prefetching variant).
4. **Stored digest width costs memory, hardly time.** Same function, 16 against 32 bytes stored: 88 against 120
   bytes per node (+36%), `ins_pre` 7.8 against 11.1 ns at 2^20 and 13.5 against 15.2 ns at 2^24 (observed).
5. **Hash-consing without a stored digest** (the slot holds a 4-byte handle and a 4-byte tag, equality is the 40
   node bytes) is the smallest and the cheapest to mint (56 B/node; 14.4 / 25.1 ns) and the most expensive to hit
   (38.5 / 45.5 ns), because a hit reads the arena record as well as the slot: two misses (observed; the reason
   is inferred).

## 4. What content addressing would cost a kernel

Arithmetic on the measured figures (inferred). One core-second per second = 100%.
`share of one core = rate x ns per operation / 10^9`.

### 4.1 At node granularity, 10 million nodes per second (budget: 100 ns per node for everything)

| What is done per node | ns | Share of one core |
|---|---|---|
| nothing: bump + copy | 1.2 to 2.3 | 1.2% to 2.3% |
| mf128 digest only | 2.75 | 2.8% |
| hardware SHA-256 digest only, overlapped / as a latency | 16.0 / 32.2 | 16% / 32% |
| scalar BLAKE3 digest only | 51.3 | 51% |
| `CC_SHA256` digest only | 97.2 | 97% |
| mint into a store of 16.8 M nodes: hash-consing, no digest | 25.1 | 25% |
| mint: mf128, 16 B stored | 28.1 | 28% |
| mint: hardware SHA-256, 32 B stored | 44.8 | 45% |
| mint: scalar BLAKE3, 32 B stored | 156 | 156% (does not fit in one core) |
| mint: `CC_SHA256`, 32 B stored | 212 | 212% (does not fit in one core) |

Memory at that rate, if every minted node is kept: 400 MB/s for the 40-byte records alone, 880 MB/s with a
16-byte-digest table, 1.2 GB/s with a 32-byte-digest table. This machine's 128 GiB would last 344, 156 and 115
seconds. So at this rate the question is settled by memory before it is settled by time: nodes minted at 10 M/s
cannot all be kept, with or without identity (inferred from 40, 88, 120 bytes per node, observed).

### 4.2 At freeze granularity, 10 thousand per second

Two readings, since the task does not fix the size of what is frozen:

| Reading | Function | ns per freeze | Share of one core |
|---|---|---|---|
| a frozen recipe is one 4096-byte canonical encoding (about 100 nodes of 40 bytes; the size is **assumed**) | `sha256_arm` | 1445 + one intern (about 45) | 1.5% |
| | `CC_SHA256` | 1309 + about 212 | 1.5% |
| | `blake3_min` scalar | 3485 + about 156 | 3.6% |
| | mf128 | 326 + about 28 | 0.35% |
| the same recipe hashed node by node (a Merkle digest per node, 100 nodes) | `sha256_arm` mint | 100 x 44.8 | 4.5% |
| | `CC_SHA256` mint | 100 x 212 | 21% |
| only 10,000 forty-byte digests per second | `sha256_arm` mint | 44.8 | 0.045% |

Node granularity against freeze granularity with hardware SHA-256: 45% of a core against 1.5%, a factor of 30,
of which the factor 1000 in rate is offset by the factor 100 in bytes per freeze, and the rest is the probes.

### 4.3 Which digest widths are safe (birthday bound, arithmetic shown)

For n items and a b-bit digest that behaves like a random function, the probability of at least one accidental
collision is `p = 1 - exp(-n(n-1) / 2^(b+1))`, which is `n^2 / 2^(b+1)` when small.

| b | n = 2^32 (n^2 = 2^64) | n = 2^40 (n^2 = 2^80) |
|---|---|---|
| 64 | 2^64 / 2^65 = 0.5 expected pairs, p = 1 - e^-0.5 = 0.39 | 2^80 / 2^65 = 2^15 = 32,768 expected pairs, p = 1 |
| 96 | 2^64 / 2^97 = 2^-33 = 1.2e-10 | 2^80 / 2^97 = 2^-17 = 7.6e-6 |
| 128 | 2^64 / 2^129 = 2^-65 = 2.7e-20 | 2^80 / 2^129 = 2^-49 = 1.8e-15 |
| 160 | 2^-97 = 6.3e-30 | 2^-81 = 4.1e-25 |
| 192 | 2^-129 | 2^-113 |
| 256 | 2^-193 | 2^-177 |

- 64 bits is unsafe at both sizes as an identity; it is fine only as a slot index with a full comparison behind it
  (which is how the benchmark uses it). The old body's image keys are 64-bit FNV-1a (`S/runtime-jit.md:338`;
  `fk_bytes_fnv1a` is declared at `R/runtime/fkwu-uni.c:494`, observed; file last committed 2026-11-20).
- 96 bits: 1 in 8.4 billion at 2^32, 1 in 131,072 at 2^40. Not safe at 2^40.
- 128 bits: safe against accident at both sizes (2.7e-20 and 1.8e-15).
- Against a party who chooses inputs, n does not matter: finding a collision takes about 2^(b/2) digest
  evaluations for a cryptographic function, and almost none for mf128 or FNV. At 16 ns per SHA-256, 2^64
  evaluations are 2.95e11 core-seconds, about 9,400 core-years: within reach of a funded attacker. 2^80 (160 bits)
  is 6.1e8 core-years; 2^128 (256 bits) is out of reach. So a 128-bit digest is safe for accident and not for
  adversaries; an identity that crosses a trust boundary wants 256 bits (inferred; the 16 ns is observed).

## 5. Recommendation

Each item: the choice, the measurement under it, the condition that flips it. All four are inferred from the
observations above; none is a requirement the axioms state.

**1. Digest function: SHA-256 through the CPU's instructions, called directly, for the identity that is stored
and crosses a boundary; a multiply-fold hash for the in-process index.**
- Measurement: 16.0 ns per 40-byte node overlapped, 28 to 32 ns as a latency, 0.353 ns/byte in bulk
  (`sha256_arm`); scalar BLAKE3 is 3.2 times slower at 40 bytes and 2.4 times at 4096; the CommonCrypto one-shot is
  6 to 9 times slower at node size. mf128 is 2.75 ns and passes the avalanche and window checks, but has no
  collision resistance, so it may pick a slot and never decide equality between parties.
- Also for it: the old body already emits these instructions from its own grammar ("Form-emitted CPU images using
  the CPU's SHA-256 instructions hash a 1.39 MB file in 1 ms", `S/lessons.md:307`, citing
  `R/CURRENT_FLOOR.md:1361-1379`; not re-run by me), and the direct implementation is 68 lines of C.
- Do not route small digests through `CC_SHA256` or OpenSSL EVP: 97 to 183 ns per 40-byte call (observed).
- Flips to BLAKE3 if: a target without SHA-256 instructions has to be first-class (there SHA-256 would be scalar;
  not measured here, assumed slower than scalar BLAKE3); or leaves of megabytes dominate and a SIMD or
  multi-threaded BLAKE3 is admitted into the body (its parallel speed was not measured here; `S/web-semantics-machines.md:302`,
  R18, marks the function choice "an assumption until measured"); or verified streaming of part of a large leaf is
  a requirement. It does **not** flip on node-size speed: a single BLAKE3 compression measured 48 ns here.

**2. Digest width stored: 256 bits, once per frozen thing; nothing or 128 bits per in-process slot.**
- Measurement: the wider stored digest costs 32 bytes per entry (120 against 88 bytes per node) and 2 to 4 ns per
  probe; the truncation saves no digest time. Birthday table: 128 bits gives 2.7e-20 at 2^32 and 1.8e-15 at 2^40
  by accident, and only 2^64 work against an adversary; 256 bits gives 2^-177 and 2^128.
- Flips to 128 bits stored if: identities never cross a trust boundary (one owner, one machine) and the store is
  memory-bound; then 2^40 nodes are still safe against accident. Flips to more than 256 never, on this evidence.

**3. In-memory handle: 32 bits, an index into the store, never the digest.**
- Measurement: the benchmark's 32-bit handles carried 16.8 M nodes; the old body's live field holds 21.7 M nodes
  (`S/form-language.md:354`) and its node word already carries a 32-bit instance field
  (`R/runtime/fkwu-node-word.h:6-14` as checked by the critic, `S/critic.md:224`; I did not open that file). 2^32 is 198 times 21.7 M. An
  entry of 16-byte digest + 32-bit handle is 24 bytes; a slot of handle + tag is 8 bytes (56 bytes per node in
  all). Comparing two handles is one instruction; comparing two 256-bit digests is four words and a cache line.
- Flips to a wider handle (40 or 48 bits inside a 64-bit word) if: transient nodes draw handles from the same
  space (at 10 M/s, 2^32 handles are gone in 429 seconds), or the frozen store is expected to pass about 2^31
  nodes (at 10,000 new frozen nodes per second that takes 2.5 days of nothing but new content).

**4. Granularity: identity is computed when a thing is frozen or offered across a boundary, not when a node is
minted.** Transient nodes are bump-allocated and carry no identity.
- Measurement: 1.2 to 2.3 ns for a node without identity against 28 to 45 ns with one (12 to 37 times), plus 48
  to 80 bytes of table per node; 10 M nodes/s with identity is 28% to 45% of a core at best and 880 MB/s to
  1.2 GB/s of memory, against 1.5% of a core for 10,000 frozen 4096-byte recipes per second.
- This agrees with what the round already recommended without numbers: "identity computed only at the solid end"
  (`S/web-semantics-machines.md:300`, R16) and "A digest of those bytes only indexes the store. Equality is the
  bytes." (`R/runtime/membrane-boot.h:18-20`, observed, last committed 2026-11-05).
- Inside a frozen recipe, per-node digests (a Merkle form) remain affordable at freeze rate: 4.5% of a core for
  10,000 recipes of 100 nodes with direct SHA-256; so sub-recipe sharing can be content-addressed without moving
  identity to mint time.
- Flips to node granularity if: the new kernel's measured mint rate for nodes that live past one movement is at or
  below about 1 M/s (then 2.8% to 4.5% of a core), or structural sharing is shown to save more than it costs (a
  hit costs 20 to 48 ns; it pays only if it avoids more than that in recomputation or more than 40 bytes times
  the duplicates in memory), or constant-time equality of live values is required everywhere. In that case use
  hash-consing with a cheap hash and byte equality (25 ns, 56 bytes per node), not a cryptographic digest per node.

Design consequence to carry (inferred): if a cryptographic digest ever sits on a path that also probes a large
table, keep the digest under about 20 ns or batch the probes, because a 50 to 100 ns digest turned an 8 to 15 ns
overlapped probe into a 74 to 108 ns one (section 3.3, point 3).

## 6. Limits and what stayed uncomfortable

- The machine was shared. Rows carry a core number and a clock probe taken after the row; where the probe reads
  0.28 to 0.35 ns the row is pessimistic by up to 40%. I do not know the mapping from core number to core kind;
  one intern run started on `cpu1` with a 0.295 ns add. The first hash run (unprobed) was up to 2.3 times slower
  than the third and I cannot prove why.
- Minimum over 5 or 9 repetitions favours the best case. Medians are within 10% of minima in undisturbed rows.
- The nodes are synthetic and all distinct, 40 bytes, hashed as flat bytes. A real node's canonical encoding
  (kind, child identities, literal bytes; `S/web-semantics-machines.md:301`, R17) may be longer if children are
  named by digest rather than by handle: four 32-byte child digests make a 136-byte node, three SHA-256 blocks
  instead of one (about 50 ns overlapped, by the 64- and 256-byte columns). That cost belongs to the Merkle form
  and is one more reason to hash at freeze.
- The intern table never deletes and never grows; reclamation and resizing costs were not measured.
- One thread. Nothing about a store shared between processes, as the old field is, was measured.
- mf128 is mine, written today, checked by two statistics. It stands for "a good multiply-fold hash costs this
  much"; it is not a proposal to ship this function.

---

## Appendix: raw output

### `E/verify_out.txt`

```
sha256_arm vs CC_SHA256, lengths 0..4200: 0 mismatches
sha256_arm("abc") = ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad
blake3_min("")    = af1349b9f5f9a1a6a0404dea36dcc9499bcb25c9adc112b7cc9a93cae41f3262  matches official empty-input vector
blake3_min("abc") = 6437b3ac38465133ffb63b75273a8db548c558465d79db03fd359c6cd5bd9d85
blake3_min pattern len 0 = af1349b9f5f9a1a6a0404dea36dcc9499bcb25c9adc112b7cc9a93cae41f3262
blake3_min pattern len 1 = 2d3adedff11b61f14c886e35afa036736dcd87a74d27b5c1510225d0f592e213
blake3_min pattern len 2 = 7b7015bb92cf0b318037702a6cdd81dee41224f734684c2c122cd6359cb1ee63
blake3_min pattern len 16 = a6a492965517a830cb75fdb713465aa465f2f098233896fea44c1d98268bf9e3
blake3_min pattern len 40 = 9563a50f070b26eeaa1ee0a6ac95b8ba9168c32b1af71af2fe55a8b4e268ce5e
blake3_min pattern len 63 = e9bc37a594daad83be9470df7f7b3798297c3d834ce80ba85d6e207627b7db7b
blake3_min pattern len 64 = 4eed7141ea4a5cd4b788606bd23f46e212af9cacebacdc7d1f4c6dc7f2511b98
blake3_min pattern len 65 = de1e5fa0be70df6d2be8fffd0e99ceaa8eb6e8c93a63f2d8d1c30ecb6b263dee
blake3_min pattern len 256 = f462b63aae56ed9fb899ad8eb93aa35d3dd62773fda9c33bfe20f9dab5d3df5f
blake3_min pattern len 1023 = 10108970eeda3eb932baac1428c7a2163b0e924c9a9e25b35bba72b28f70bd11
blake3_min pattern len 1024 = 42214739f095a406f3fc83deb889744ac00df831c10daa55189b5d121c855af7
blake3_min pattern len 1025 = d00278ae47eb27b34faecf67b4fe263f82d5412916c1ffd97c8cb7fb814b8444
blake3_min pattern len 2048 = e776b6028c7cd22a4d0ba182a8bf62205d2ef576467e838ed6f2529b85fba24a
blake3_min pattern len 2049 = 5f4d72f40d7a5f82b15ca2b2e44b1de3c2ef86c426c95c1af0b6879522563030
blake3_min pattern len 3000 = 5fade288bf27444bee55ba2babb98c3c922c1e84c2e445e7d1f6da24756f5060
blake3_min pattern len 4096 = 015094013f57a5277b59d8475c0501042c0b642e531b0a1c8f58d2163229e969
blake3_min pattern len 8192 = aae792484c8efe4f19e2ca7d371d8c467ffb10748d8a5a1ae579948f718a2a63
mf128 avalanche (MF_FINAL=3, 40-byte input, 40000 samples): flip probability over 40960 bit pairs: min 0.4900 mean 0.5000 max 0.5098 (ideal 0.5; 1 sigma of one pair = 0.0025)
mf128 32-bit window 0 over 4194304 counter nodes: 2029 equal neighbours (random function: about 2048, sigma 45)
mf128 32-bit window 1 over 4194304 counter nodes: 2037 equal neighbours (random function: about 2048, sigma 45)
mf128 32-bit window 2 over 4194304 counter nodes: 1993 equal neighbours (random function: about 2048, sigma 45)
mf128 32-bit window 3 over 4194304 counter nodes: 1994 equal neighbours (random function: about 2048, sigma 45)
mf128 64-bit word 0 over 4194304 counter nodes: 0 collisions (random function: about 4.77e-07)
VERIFY OK (0 failures)
```

### `E/insn_out.txt`

```
19:02  up 27 days,  7:19, 3 users, load averages: 4.15 4.58 3.63
add x,x,#1                   min  0.254  med  0.255 ns
  implied clock 3.94 GHz
mul (low 64)                 min  0.759  med  0.763 ns   =  2.99 add-times
umulh + orr                  min  1.006  med  1.018 ns   =  3.96 add-times
mf128 round                  min  1.194  med  1.200 ns   =  4.70 add-times
sha256h (4 rounds)           min  1.006  med  1.010 ns   =  3.96 add-times
sha256h + sha256h2 pair      min  2.136  med  2.143 ns   =  8.41 add-times
sha256su1                    min  0.754  med  0.756 ns   =  2.97 add-times
# cpu number at start 8, at end 7; sink 55454d47deca8ec0 9e4d1f6b
```

### `E/base_out.txt`

```
18:59  up 27 days,  7:17, 3 users, load averages: 4.33 4.36 3.37
add chain              iters=200000000  min   0.250  med   0.252  max   0.254 ns/op
  -> one dependent add = 0.250 ns, so the core ran at about 4.00 GHz if an add is one cycle
call direct            iters=200000000  min   0.750  med   0.759  max   0.771 ns/op
call indirect          iters=200000000  min   0.751  med   0.760  max   0.778 ns/op
node build only        iters=16777216   min   0.382  med   0.397  max   0.428 ns/op
bump+memcpy hot        iters=16777216   min   1.708  med   1.846  max   1.920 ns/op
bump+memcpy stream     iters=16777216   min   2.274  med   2.287  max   2.359 ns/op
# sink 00048000076bc208
```

### `E/base_out_run2.txt`

```
19:07  up 27 days,  7:24, 3 users, load averages: 4.03 4.37 3.79
add chain              iters=200000000  min   0.222  med   0.246  max   0.251 ns/op
  -> one dependent add = 0.222 ns, so the core ran at about 4.50 GHz if an add is one cycle
call direct            iters=200000000  min   0.696  med   0.718  max   0.793 ns/op
call indirect          iters=200000000  min   0.696  med   0.733  max   0.770 ns/op
node build only        iters=16777216   min   0.383  med   0.385  max   0.388 ns/op
bump+memcpy hot        iters=16777216   min   1.893  med   1.956  max   2.020 ns/op
bump+memcpy stream     iters=16777216   min   2.284  med   2.357  max   2.440 ns/op
# sink 00048000076bc208
```

### `E/hash_out_run3.txt`

```
19:06  up 27 days,  7:24, 3 users, load averages: 4.42 4.47 3.81
# bench_hash: REPS=9, columns: function size mode iterations min med max ns/op, min ns/byte, MB/s at min
CC_SHA256             16 tput iters=262144     min    145.54  med    147.29  max    152.54 ns/op      9.097 ns/byte      109.9 MB/s  cpu8 add=0.252ns
CC_SHA256             16 lat  iters=262144     min    149.54  med    151.15  max    154.34 ns/op      9.346 ns/byte      107.0 MB/s  cpu4 add=0.251ns
sha256_arm            16 tput iters=2097152    min     16.36  med     16.54  max     23.44 ns/op      1.022 ns/byte      978.0 MB/s  cpu6 add=0.247ns
sha256_arm            16 lat  iters=1048576    min     35.92  med     36.13  max     36.31 ns/op      2.245 ns/byte      445.4 MB/s  cpu9 add=0.250ns
blake3_min            16 tput iters=1048576    min     48.59  med     48.82  max     49.09 ns/op      3.037 ns/byte      329.3 MB/s  cpu5 add=0.246ns
blake3_min            16 lat  iters=524288     min     60.32  med     61.93  max     62.09 ns/op      3.770 ns/byte      265.3 MB/s  cpu9 add=0.251ns
mf128                 16 tput iters=33554432   min      1.55  med      1.56  max      1.58 ns/op      0.097 ns/byte    10295.1 MB/s  cpu5 add=0.223ns
mf128                 16 lat  iters=8388608    min      6.27  med      6.57  max      6.94 ns/op      0.392 ns/byte     2550.9 MB/s  cpu11 add=0.223ns
CC_SHA256             40 tput iters=524288     min     97.20  med    101.80  max    106.03 ns/op      2.430 ns/byte      411.5 MB/s  cpu11 add=0.257ns
CC_SHA256             40 lat  iters=262144     min    101.02  med    107.11  max    113.42 ns/op      2.526 ns/byte      395.9 MB/s  cpu10 add=0.244ns
sha256_arm            40 tput iters=2097152    min     15.98  med     16.52  max     16.66 ns/op      0.400 ns/byte     2502.9 MB/s  cpu14 add=0.222ns
sha256_arm            40 lat  iters=1048576    min     32.20  med     35.20  max     36.94 ns/op      0.805 ns/byte     1242.1 MB/s  cpu15 add=0.252ns
blake3_min            40 tput iters=1048576    min     51.25  med     53.35  max     54.05 ns/op      1.281 ns/byte      780.5 MB/s  cpu10 add=0.250ns
blake3_min            40 lat  iters=524288     min     60.47  med     61.75  max     62.80 ns/op      1.512 ns/byte      661.5 MB/s  cpu12 add=0.230ns
mf128                 40 tput iters=16777216   min      2.75  med      2.77  max      2.80 ns/op      0.069 ns/byte    14559.4 MB/s  cpu11 add=0.241ns
mf128                 40 lat  iters=4194304    min      8.52  med      8.97  max      9.70 ns/op      0.213 ns/byte     4696.3 MB/s  cpu5 add=0.222ns
sha256_arm_fix40      40 tput iters=2097152    min     14.96  med     15.95  max     16.51 ns/op      0.374 ns/byte     2673.8 MB/s  cpu6 add=0.222ns
sha256_arm_fix40      40 lat  iters=1048576    min     28.17  med     29.24  max     31.56 ns/op      0.704 ns/byte     1420.1 MB/s  cpu4 add=0.222ns
blake3_min_fix40      40 tput iters=1048576    min     48.32  med     48.93  max     49.42 ns/op      1.208 ns/byte      827.8 MB/s  cpu8 add=0.237ns
blake3_min_fix40      40 lat  iters=524288     min     59.25  med     60.54  max     61.46 ns/op      1.481 ns/byte      675.1 MB/s  cpu8 add=0.238ns
mf128_fix40           40 tput iters=16777216   min      2.61  med      2.66  max      2.70 ns/op      0.065 ns/byte    15353.9 MB/s  cpu8 add=0.247ns
mf128_fix40           40 lat  iters=4194304    min      8.85  med      9.44  max      9.71 ns/op      0.221 ns/byte     4521.7 MB/s  cpu5 add=0.254ns
CC_SHA256             64 tput iters=262144     min    179.46  med    184.54  max    188.78 ns/op      2.804 ns/byte      356.6 MB/s  cpu11 add=0.222ns
CC_SHA256             64 lat  iters=262144     min    172.72  med    179.03  max    186.13 ns/op      2.699 ns/byte      370.5 MB/s  cpu13 add=0.222ns
sha256_arm            64 tput iters=1048576    min     31.59  med     33.84  max     34.76 ns/op      0.494 ns/byte     2025.7 MB/s  cpu15 add=0.245ns
sha256_arm            64 lat  iters=524288     min     55.39  med     60.49  max     63.58 ns/op      0.865 ns/byte     1155.5 MB/s  cpu12 add=0.222ns
blake3_min            64 tput iters=1048576    min     49.54  med     50.01  max     50.61 ns/op      0.774 ns/byte     1291.8 MB/s  cpu12 add=0.224ns
blake3_min            64 lat  iters=524288     min     62.16  med     62.71  max     63.46 ns/op      0.971 ns/byte     1029.5 MB/s  cpu13 add=0.254ns
mf128                 64 tput iters=16777216   min      3.44  med      3.49  max      3.53 ns/op      0.054 ns/byte    18604.0 MB/s  cpu15 add=0.224ns
mf128                 64 lat  iters=4194304    min      9.77  med     10.25  max     10.76 ns/op      0.153 ns/byte     6550.7 MB/s  cpu4 add=0.252ns
CC_SHA256            256 tput iters=262144     min    213.64  med    224.36  max    240.14 ns/op      0.835 ns/byte     1198.3 MB/s  cpu8 add=0.222ns
CC_SHA256            256 lat  iters=262144     min    204.88  med    219.23  max    229.66 ns/op      0.800 ns/byte     1249.5 MB/s  cpu8 add=0.222ns
sha256_arm           256 tput iters=524288     min     94.79  med    101.87  max    103.61 ns/op      0.370 ns/byte     2700.7 MB/s  cpu8 add=0.253ns
sha256_arm           256 lat  iters=262144     min    121.49  med    135.22  max    139.34 ns/op      0.475 ns/byte     2107.2 MB/s  cpu7 add=0.222ns
blake3_min           256 tput iters=262144     min    206.16  med    207.58  max    209.31 ns/op      0.805 ns/byte     1241.8 MB/s  cpu6 add=0.245ns
blake3_min           256 lat  iters=262144     min    218.65  med    219.61  max    221.83 ns/op      0.854 ns/byte     1170.8 MB/s  cpu10 add=0.252ns
mf128                256 tput iters=2097152    min     14.14  med     14.71  max     14.75 ns/op      0.055 ns/byte    18109.7 MB/s  cpu15 add=0.223ns
mf128                256 lat  iters=2097152    min     26.05  med     27.13  max     27.83 ns/op      0.102 ns/byte     9828.7 MB/s  cpu14 add=0.223ns
CC_SHA256           4096 tput iters=32768      min   1309.17  med   1444.51  max   1475.31 ns/op      0.320 ns/byte     3128.7 MB/s  cpu15 add=0.223ns
CC_SHA256           4096 lat  iters=32768      min   1318.40  med   1460.18  max   1500.32 ns/op      0.322 ns/byte     3106.8 MB/s  cpu10 add=0.223ns
sha256_arm          4096 tput iters=32768      min   1445.28  med   1473.08  max   1567.67 ns/op      0.353 ns/byte     2834.1 MB/s  cpu12 add=0.246ns
sha256_arm          4096 lat  iters=32768      min   1456.97  med   1592.45  max   1629.32 ns/op      0.356 ns/byte     2811.3 MB/s  cpu11 add=0.251ns
blake3_min          4096 tput iters=16384      min   3485.01  med   3504.92  max   3529.63 ns/op      0.851 ns/byte     1175.3 MB/s  cpu11 add=0.222ns
blake3_min          4096 lat  iters=16384      min   3504.56  med   3531.74  max   3578.53 ns/op      0.856 ns/byte     1168.8 MB/s  cpu7 add=0.247ns
mf128               4096 tput iters=131072     min    326.21  med    344.94  max    360.77 ns/op      0.080 ns/byte    12556.4 MB/s  cpu8 add=0.247ns
mf128               4096 lat  iters=131072     min    341.38  med    361.52  max    375.01 ns/op      0.083 ns/byte    11998.4 MB/s  cpu8 add=0.251ns
# sink 10a6269a32b13e0d
```

### `E/hash_out.txt`

```
19:02  up 27 days,  7:20, 3 users, load averages: 3.52 4.36 3.59
# bench_hash: REPS=9, columns: function size mode iterations min med max ns/op, min ns/byte, MB/s at min
CC_SHA256             16 tput iters=262144     min    148.13  med    154.50  max    155.74 ns/op      9.258 ns/byte      108.0 MB/s  cpu6 add=0.254ns
CC_SHA256             16 lat  iters=262144     min    151.85  med    154.35  max    157.40 ns/op      9.491 ns/byte      105.4 MB/s  cpu6 add=0.262ns
sha256_arm            16 tput iters=2097152    min     17.33  med     17.46  max     18.05 ns/op      1.083 ns/byte      923.4 MB/s  cpu4 add=0.277ns
sha256_arm            16 lat  iters=1048576    min     38.43  med     38.47  max     39.59 ns/op      2.402 ns/byte      416.3 MB/s  cpu5 add=0.267ns
blake3_min            16 tput iters=1048576    min     51.72  med     51.97  max     53.11 ns/op      3.233 ns/byte      309.3 MB/s  cpu5 add=0.277ns
blake3_min            16 lat  iters=524288     min     68.25  med     68.30  max     68.49 ns/op      4.266 ns/byte      234.4 MB/s  cpu7 add=0.280ns
mf128                 16 tput iters=33554432   min      1.66  med      1.71  max      1.74 ns/op      0.104 ns/byte     9610.4 MB/s  cpu9 add=0.267ns
mf128                 16 lat  iters=4194304    min      7.13  med      7.16  max      7.25 ns/op      0.446 ns/byte     2243.3 MB/s  cpu6 add=0.256ns
CC_SHA256             40 tput iters=524288     min    104.64  med    106.09  max    107.63 ns/op      2.616 ns/byte      382.3 MB/s  cpu15 add=0.254ns
CC_SHA256             40 lat  iters=524288     min    106.40  med    106.83  max    110.56 ns/op      2.660 ns/byte      375.9 MB/s  cpu10 add=0.252ns
sha256_arm            40 tput iters=2097152    min     16.75  med     16.87  max     17.06 ns/op      0.419 ns/byte     2388.5 MB/s  cpu11 add=0.257ns
sha256_arm            40 lat  iters=1048576    min     37.52  med     37.83  max     38.08 ns/op      0.938 ns/byte     1066.1 MB/s  cpu10 add=0.258ns
blake3_min            40 tput iters=1048576    min     54.94  med     55.13  max     55.66 ns/op      1.374 ns/byte      728.0 MB/s  cpu11 add=0.259ns
blake3_min            40 lat  iters=524288     min     63.57  med     63.83  max     64.06 ns/op      1.589 ns/byte      629.2 MB/s  cpu14 add=0.258ns
mf128                 40 tput iters=16777216   min      2.79  med      2.81  max      2.87 ns/op      0.070 ns/byte    14313.4 MB/s  cpu14 add=0.251ns
mf128                 40 lat  iters=4194304    min      9.63  med      9.64  max      9.70 ns/op      0.241 ns/byte     4155.1 MB/s  cpu6 add=0.251ns
sha256_arm_fix40      40 tput iters=2097152    min     16.15  med     16.29  max     16.34 ns/op      0.404 ns/byte     2476.8 MB/s  cpu4 add=0.252ns
sha256_arm_fix40      40 lat  iters=1048576    min     31.59  med     31.97  max     32.86 ns/op      0.790 ns/byte     1266.1 MB/s  cpu7 add=0.258ns
blake3_min_fix40      40 tput iters=1048576    min     50.70  med     52.54  max     54.05 ns/op      1.268 ns/byte      788.9 MB/s  cpu9 add=0.276ns
blake3_min_fix40      40 lat  iters=524288     min     69.54  med     69.74  max     69.93 ns/op      1.738 ns/byte      575.2 MB/s  cpu5 add=0.289ns
mf128_fix40           40 tput iters=16777216   min      3.19  med      3.22  max      3.42 ns/op      0.080 ns/byte    12528.8 MB/s  cpu8 add=0.324ns
mf128_fix40           40 lat  iters=4194304    min     12.41  med     12.46  max     12.49 ns/op      0.310 ns/byte     3222.3 MB/s  cpu7 add=0.324ns
CC_SHA256             64 tput iters=131072     min    238.60  med    239.36  max    242.16 ns/op      3.728 ns/byte      268.2 MB/s  cpu8 add=0.324ns
CC_SHA256             64 lat  iters=131072     min    238.19  med    240.36  max    243.85 ns/op      3.722 ns/byte      268.7 MB/s  cpu9 add=0.324ns
sha256_arm            64 tput iters=1048576    min     44.34  med     44.39  max     44.59 ns/op      0.693 ns/byte     1443.5 MB/s  cpu6 add=0.324ns
sha256_arm            64 lat  iters=524288     min     80.62  med     80.74  max     81.32 ns/op      1.260 ns/byte      793.9 MB/s  cpu4 add=0.324ns
blake3_min            64 tput iters=524288     min     64.83  med     65.17  max     67.71 ns/op      1.013 ns/byte      987.3 MB/s  cpu14 add=0.324ns
blake3_min            64 lat  iters=524288     min     87.18  med     87.51  max     87.87 ns/op      1.362 ns/byte      734.1 MB/s  cpu9 add=0.349ns
mf128                 64 tput iters=8388608    min      4.87  med      4.97  max      5.15 ns/op      0.076 ns/byte    13133.5 MB/s  cpu5 add=0.332ns
mf128                 64 lat  iters=4194304    min     13.88  med     13.90  max     13.95 ns/op      0.217 ns/byte     4612.3 MB/s  cpu9 add=0.324ns
CC_SHA256            256 tput iters=131072     min    259.42  med    272.96  max    279.27 ns/op      1.013 ns/byte      986.8 MB/s  cpu8 add=0.290ns
CC_SHA256            256 lat  iters=131072     min    239.73  med    249.90  max    252.76 ns/op      0.936 ns/byte     1067.8 MB/s  cpu5 add=0.259ns
sha256_arm           256 tput iters=524288     min    101.51  med    103.52  max    107.02 ns/op      0.397 ns/byte     2522.0 MB/s  cpu7 add=0.254ns
sha256_arm           256 lat  iters=262144     min    137.69  med    138.17  max    138.84 ns/op      0.538 ns/byte     1859.3 MB/s  cpu9 add=0.252ns
blake3_min           256 tput iters=262144     min    209.57  med    210.43  max    211.57 ns/op      0.819 ns/byte     1221.6 MB/s  cpu5 add=0.253ns
blake3_min           256 lat  iters=262144     min    222.12  med    222.88  max    223.75 ns/op      0.868 ns/byte     1152.5 MB/s  cpu10 add=0.252ns
mf128                256 tput iters=2097152    min     14.80  med     14.89  max     15.27 ns/op      0.058 ns/byte    17299.7 MB/s  cpu11 add=0.257ns
mf128                256 lat  iters=1048576    min     28.24  med     28.88  max     29.00 ns/op      0.110 ns/byte     9065.0 MB/s  cpu12 add=0.245ns
CC_SHA256           4096 tput iters=32768      min   1433.35  med   1460.16  max   1481.32 ns/op      0.350 ns/byte     2857.6 MB/s  cpu15 add=0.243ns
CC_SHA256           4096 lat  iters=32768      min   1445.31  med   1484.57  max   1496.83 ns/op      0.353 ns/byte     2834.0 MB/s  cpu11 add=0.235ns
sha256_arm          4096 tput iters=32768      min   1555.12  med   1576.50  max   1620.51 ns/op      0.380 ns/byte     2633.9 MB/s  cpu15 add=0.244ns
sha256_arm          4096 lat  iters=32768      min   1608.72  med   1638.37  max   1652.96 ns/op      0.393 ns/byte     2546.1 MB/s  cpu11 add=0.252ns
blake3_min          4096 tput iters=16384      min   3515.20  med   3547.57  max   3632.96 ns/op      0.858 ns/byte     1165.2 MB/s  cpu4 add=0.250ns
blake3_min          4096 lat  iters=16384      min   3546.20  med   3559.58  max   3625.37 ns/op      0.866 ns/byte     1155.0 MB/s  cpu8 add=0.259ns
mf128               4096 tput iters=131072     min    359.11  med    360.75  max    362.68 ns/op      0.088 ns/byte    11406.0 MB/s  cpu9 add=0.252ns
mf128               4096 lat  iters=131072     min    375.89  med    377.91  max    430.98 ns/op      0.092 ns/byte    10896.7 MB/s  cpu9 add=0.249ns
# sink e3f7746290ec3b99
```

### `E/hash_out_run1_unprobed.txt`

```
19:00  up 27 days,  7:17, 3 users, load averages: 4.47 4.39 3.39
# bench_hash: REPS=9, columns: function size mode iterations min med max ns/op, min ns/byte, MB/s at min
CC_SHA256             16 tput iters=262144     min    145.49  med    147.44  max    149.41 ns/op      9.093 ns/byte      110.0 MB/s
CC_SHA256             16 lat  iters=262144     min    148.55  med    151.18  max    152.17 ns/op      9.285 ns/byte      107.7 MB/s
sha256_arm            16 tput iters=2097152    min     16.39  med     16.79  max     19.34 ns/op      1.024 ns/byte      976.3 MB/s
sha256_arm            16 lat  iters=1048576    min     35.95  med     36.11  max     36.94 ns/op      2.247 ns/byte      445.0 MB/s
blake3_min            16 tput iters=1048576    min     49.36  med     50.54  max     52.47 ns/op      3.085 ns/byte      324.2 MB/s
blake3_min            16 lat  iters=524288     min     61.66  med     63.21  max     63.85 ns/op      3.854 ns/byte      259.5 MB/s
mf128                 16 tput iters=33554432   min      1.57  med      1.58  max      1.62 ns/op      0.098 ns/byte    10164.1 MB/s
mf128                 16 lat  iters=8388608    min      7.05  med      7.25  max      7.40 ns/op      0.441 ns/byte     2268.4 MB/s
CC_SHA256             40 tput iters=524288     min    117.05  med    120.25  max    126.60 ns/op      2.926 ns/byte      341.7 MB/s
CC_SHA256             40 lat  iters=262144     min    130.64  med    139.52  max    143.39 ns/op      3.266 ns/byte      306.2 MB/s
sha256_arm            40 tput iters=2097152    min     23.30  med     25.47  max     26.18 ns/op      0.583 ns/byte     1716.7 MB/s
sha256_arm            40 lat  iters=524288     min     55.13  med     55.25  max     57.36 ns/op      1.378 ns/byte      725.6 MB/s
blake3_min            40 tput iters=524288     min     90.81  med     91.56  max     92.04 ns/op      2.270 ns/byte      440.5 MB/s
blake3_min            40 lat  iters=524288     min    117.70  med    118.46  max    119.06 ns/op      2.942 ns/byte      339.8 MB/s
mf128                 40 tput iters=8388608    min      4.74  med      4.84  max      5.32 ns/op      0.118 ns/byte     8444.8 MB/s
mf128                 40 lat  iters=2097152    min     14.69  med     14.71  max     16.39 ns/op      0.367 ns/byte     2722.4 MB/s
sha256_arm_fix40      40 tput iters=2097152    min     24.75  med     26.14  max     27.63 ns/op      0.619 ns/byte     1616.1 MB/s
sha256_arm_fix40      40 lat  iters=1048576    min     43.97  med     44.15  max     48.38 ns/op      1.099 ns/byte      909.7 MB/s
blake3_min_fix40      40 tput iters=524288     min     63.72  med     66.32  max     68.79 ns/op      1.593 ns/byte      627.7 MB/s
blake3_min_fix40      40 lat  iters=524288     min     78.44  med     84.21  max     84.39 ns/op      1.961 ns/byte      509.9 MB/s
mf128_fix40           40 tput iters=16777216   min      3.41  med      3.70  max      4.06 ns/op      0.085 ns/byte    11715.0 MB/s
mf128_fix40           40 lat  iters=2097152    min     13.40  med     14.73  max     14.76 ns/op      0.335 ns/byte     2985.5 MB/s
CC_SHA256             64 tput iters=131072     min    255.15  med    258.28  max    266.59 ns/op      3.987 ns/byte      250.8 MB/s
CC_SHA256             64 lat  iters=131072     min    282.52  med    286.25  max    316.89 ns/op      4.414 ns/byte      226.5 MB/s
sha256_arm            64 tput iters=524288     min     65.74  med     66.69  max     76.37 ns/op      1.027 ns/byte      973.6 MB/s
sha256_arm            64 lat  iters=262144     min    137.31  med    138.23  max    150.82 ns/op      2.146 ns/byte      466.1 MB/s
blake3_min            64 tput iters=524288     min    111.59  med    112.14  max    130.42 ns/op      1.744 ns/byte      573.6 MB/s
blake3_min            64 lat  iters=262144     min    139.86  med    144.90  max    160.62 ns/op      2.185 ns/byte      457.6 MB/s
mf128                 64 tput iters=4194304    min      7.72  med      8.51  max     13.20 ns/op      0.121 ns/byte     8293.8 MB/s
mf128                 64 lat  iters=2097152    min     22.99  med     23.87  max     24.20 ns/op      0.359 ns/byte     2784.1 MB/s
CC_SHA256            256 tput iters=65536      min    480.40  med    499.71  max    509.43 ns/op      1.877 ns/byte      532.9 MB/s
CC_SHA256            256 lat  iters=65536      min    491.20  med    502.45  max    513.44 ns/op      1.919 ns/byte      521.2 MB/s
sha256_arm           256 tput iters=262144     min    225.57  med    227.95  max    235.35 ns/op      0.881 ns/byte     1134.9 MB/s
sha256_arm           256 lat  iters=131072     min    302.37  med    304.77  max    305.63 ns/op      1.181 ns/byte      846.6 MB/s
blake3_min           256 tput iters=65536      min    467.10  med    469.20  max    494.26 ns/op      1.825 ns/byte      548.1 MB/s
blake3_min           256 lat  iters=65536      min    491.86  med    513.41  max    533.73 ns/op      1.921 ns/byte      520.5 MB/s
mf128                256 tput iters=1048576    min     36.56  med     42.22  max     46.14 ns/op      0.143 ns/byte     7003.1 MB/s
mf128                256 lat  iters=524288     min     59.69  med     61.49  max     63.14 ns/op      0.233 ns/byte     4288.7 MB/s
CC_SHA256           4096 tput iters=16384      min   2925.10  med   3094.54  max   3294.93 ns/op      0.714 ns/byte     1400.3 MB/s
CC_SHA256           4096 lat  iters=16384      min   3081.63  med   3235.54  max   3353.47 ns/op      0.752 ns/byte     1329.2 MB/s
sha256_arm          4096 tput iters=16384      min   2901.85  med   3098.11  max   3699.15 ns/op      0.708 ns/byte     1411.5 MB/s
sha256_arm          4096 lat  iters=16384      min   2792.17  med   3132.78  max   3583.45 ns/op      0.682 ns/byte     1467.0 MB/s
blake3_min          4096 tput iters=4096       min   8334.98  med   8702.98  max  10034.77 ns/op      2.035 ns/byte      491.4 MB/s
blake3_min          4096 lat  iters=4096       min   8113.52  med   9087.28  max   9425.63 ns/op      1.981 ns/byte      504.8 MB/s
mf128               4096 tput iters=65536      min    807.95  med    907.00  max   1242.17 ns/op      0.197 ns/byte     5069.6 MB/s
mf128               4096 lat  iters=32768      min    766.29  med    808.23  max    942.92 ns/op      0.187 ns/byte     5345.2 MB/s
# sink 4e4da06f20606395
```

### `E/intern20_out.txt`

```
19:03  up 27 days,  7:20, 3 users, load averages: 3.29 4.20 3.57
# bench_intern mf128/16B  N=2^20=1048576  slots=2097152  entry=24 B  table=48.0 MiB  arena=40.0 MiB
  probe at start: cpu6, dependent add 0.257 ns
  table: 1048576 of 2097152 slots used, longest occupied run 51
  gen      raw:    1.20    1.19    1.16    1.24    1.20   min    1.16  med    1.20 ns/node
  hash     raw:    2.09    2.08    2.10    2.20    2.06   min    2.06  med    2.09 ns/node
  ins_pre  raw:    7.78    7.48    7.89    8.43    7.87   min    7.48  med    7.87 ns/node
  hit_pre  raw:   13.44   12.93   12.97   14.34   14.29   min   12.93  med   13.44 ns/node
  mint     raw:   18.54   18.75   18.67   20.34   21.52   min   18.54  med   18.75 ns/node
  hit      raw:   19.72   19.57   22.95   22.88   21.15   min   19.57  med   21.15 ns/node
  probe after timed phases: cpu9, dependent add 0.251 ns
  cold mint (fresh mappings, page faults included, 1 run): 26.20 ns/node
  memory growth: resident +88.0 MiB, phys_footprint +88.1 MiB  = 88.1 bytes/node (arena 40 + table 48 = 88 by layout)
  check: bad=0 sink=50000500000 maxrun=51
# bench_intern cons-mf128/no-digest  N=2^20=1048576  slots=2097152  entry=8 B  table=16.0 MiB  arena=40.0 MiB
  probe at start: cpu1, dependent add 0.295 ns
  table: 1048576 of 2097152 slots used, longest occupied run 51
  gen      raw:    2.37    1.46    1.47    1.47    1.44   min    1.44  med    1.47 ns/node
  hash     raw:    2.23    2.32    2.13    2.07    2.05   min    2.05  med    2.13 ns/node
  ins_pre  raw:    6.15    6.55    6.17    5.66    5.70   min    5.66  med    6.15 ns/node
  hit_pre  raw:   24.49   24.56   21.36   21.01   23.23   min   21.01  med   23.23 ns/node
  mint     raw:   17.71   20.56   14.40   14.58   18.08   min   14.40  med   17.71 ns/node
  hit      raw:   43.07   43.27   47.85   38.46   38.68   min   38.46  med   43.07 ns/node
  probe after timed phases: cpu6, dependent add 0.256 ns
  cold mint (fresh mappings, page faults included, 1 run): 18.14 ns/node
  memory growth: resident +56.0 MiB, phys_footprint +56.0 MiB  = 56.0 bytes/node (arena 40 + table 16 = 56 by layout)
  check: bad=0 sink=50000500000 maxrun=51
# bench_intern sha256_arm/16B  N=2^20=1048576  slots=2097152  entry=24 B  table=48.0 MiB  arena=40.0 MiB
  probe at start: cpu8, dependent add 0.255 ns
  table: 1048576 of 2097152 slots used, longest occupied run 52
  gen      raw:    1.19    1.17    1.20    1.18    1.18   min    1.17  med    1.18 ns/node
  hash     raw:   15.78   15.95   15.91   15.92   15.73   min   15.73  med   15.91 ns/node
  ins_pre  raw:    8.44    7.77    8.01    8.33    9.16   min    7.77  med    8.33 ns/node
  hit_pre  raw:   14.22   13.69   13.16   15.09   15.80   min   13.16  med   14.22 ns/node
  mint     raw:   37.66   35.41   39.55   36.89   38.14   min   35.41  med   37.66 ns/node
  hit      raw:   37.02   35.47   39.44   39.50   39.17   min   35.47  med   39.17 ns/node
  probe after timed phases: cpu8, dependent add 0.252 ns
  cold mint (fresh mappings, page faults included, 1 run): 43.56 ns/node
  memory growth: resident +88.0 MiB, phys_footprint +88.1 MiB  = 88.1 bytes/node (arena 40 + table 48 = 88 by layout)
  check: bad=0 sink=50000500000 maxrun=52
# bench_intern sha256_arm/32B  N=2^20=1048576  slots=2097152  entry=40 B  table=80.0 MiB  arena=40.0 MiB
  probe at start: cpu9, dependent add 0.254 ns
  table: 1048576 of 2097152 slots used, longest occupied run 52
  gen      raw:    1.29    1.18    1.20    1.22    1.22   min    1.18  med    1.22 ns/node
  hash     raw:   16.80   16.41   16.56   16.60   16.68   min   16.41  med   16.60 ns/node
  ins_pre  raw:   13.59   11.83   11.09   11.39   11.10   min   11.09  med   11.39 ns/node
  hit_pre  raw:   25.24   20.02   19.85   19.05   19.00   min   19.00  med   19.85 ns/node
  mint     raw:   42.44   41.51   40.10   39.95   39.59   min   39.59  med   40.10 ns/node
  hit      raw:   45.71   43.48   43.55   43.29   43.57   min   43.29  med   43.55 ns/node
  probe after timed phases: cpu15, dependent add 0.250 ns
  cold mint (fresh mappings, page faults included, 1 run): 46.04 ns/node
  memory growth: resident +120.0 MiB, phys_footprint +120.1 MiB  = 120.1 bytes/node (arena 40 + table 80 = 120 by layout)
  check: bad=0 sink=50000500000 maxrun=52
# bench_intern blake3_min/32B  N=2^20=1048576  slots=2097152  entry=40 B  table=80.0 MiB  arena=40.0 MiB
  probe at start: cpu13, dependent add 0.252 ns
  table: 1048576 of 2097152 slots used, longest occupied run 50
  gen      raw:    1.33    1.35    1.37    1.34    1.37   min    1.33  med    1.35 ns/node
  hash     raw:   47.32   47.29   47.79   47.23   47.99   min   47.23  med   47.32 ns/node
  ins_pre  raw:   11.72   13.17   11.80   11.67   11.06   min   11.06  med   11.72 ns/node
  hit_pre  raw:   20.92   24.21   20.98   19.58   19.28   min   19.28  med   20.92 ns/node
  mint     raw:  130.64  135.01  137.47  121.02  127.70   min  121.02  med  130.64 ns/node
  hit      raw:  137.33  140.43  140.74  125.85  134.14   min  125.85  med  137.33 ns/node
  probe after timed phases: cpu12, dependent add 0.253 ns
  cold mint (fresh mappings, page faults included, 1 run): 138.26 ns/node
  memory growth: resident +120.0 MiB, phys_footprint +120.1 MiB  = 120.1 bytes/node (arena 40 + table 80 = 120 by layout)
  check: bad=0 sink=50000500000 maxrun=50
# bench_intern CC_SHA256/32B  N=2^20=1048576  slots=2097152  entry=40 B  table=80.0 MiB  arena=40.0 MiB
  probe at start: cpu10, dependent add 0.256 ns
  table: 1048576 of 2097152 slots used, longest occupied run 52
  gen      raw:    1.45    1.46    1.69    1.54    1.52   min    1.45  med    1.52 ns/node
  hash     raw:  107.45  107.89  117.67  122.32  117.30   min  107.45  med  117.30 ns/node
  ins_pre  raw:   12.47   11.85   13.50   12.97   12.25   min   11.85  med   12.47 ns/node
  hit_pre  raw:   21.84   21.89   23.75   23.26   22.55   min   21.84  med   22.55 ns/node
  mint     raw:  211.40  214.83  219.71  220.15  220.26   min  211.40  med  219.71 ns/node
  hit      raw:  227.54  238.06  225.38  230.11  224.94   min  224.94  med  227.54 ns/node
  probe after timed phases: cpu8, dependent add 0.260 ns
  cold mint (fresh mappings, page faults included, 1 run): 228.91 ns/node
  memory growth: resident +120.0 MiB, phys_footprint +120.1 MiB  = 120.1 bytes/node (arena 40 + table 80 = 120 by layout)
  check: bad=0 sink=50000500000 maxrun=52
```

### `E/intern24_out.txt`

```
19:04  up 27 days,  7:21, 3 users, load averages: 3.40 4.11 3.56
# bench_intern mf128/16B  N=2^24=16777216  slots=33554432  entry=24 B  table=768.0 MiB  arena=640.0 MiB
  probe at start: cpu6, dependent add 0.258 ns
  table: 16777216 of 33554432 slots used, longest occupied run 67
  gen      raw:    1.25    1.31    1.61    1.22    1.23   min    1.22  med    1.25 ns/node
  hash     raw:    2.35    2.24    2.86    2.13    2.13   min    2.13  med    2.24 ns/node
  ins_pre  raw:   13.21   13.41   13.88   12.22   12.48   min   12.22  med   13.21 ns/node
  hit_pre  raw:   20.85   22.56   23.42   20.29   20.43   min   20.29  med   20.85 ns/node
  mint     raw:   33.43   30.55   30.86   28.11   28.19   min   28.11  med   30.55 ns/node
  hit      raw:   34.11   32.48   29.07   28.31   27.06   min   27.06  med   29.07 ns/node
  probe after timed phases: cpu7, dependent add 0.254 ns
  cold mint (fresh mappings, page faults included, 1 run): 29.65 ns/node
  memory growth: resident +1408.0 MiB, phys_footprint +1408.7 MiB  = 88.0 bytes/node (arena 40 + table 48 = 88 by layout)
  check: bad=0 sink=5000005000000 maxrun=67
# bench_intern cons-mf128/no-digest  N=2^24=16777216  slots=33554432  entry=8 B  table=256.0 MiB  arena=640.0 MiB
  probe at start: cpu7, dependent add 0.254 ns
  table: 16777216 of 33554432 slots used, longest occupied run 67
  gen      raw:    1.36    1.36    1.37    1.42    1.96   min    1.36  med    1.37 ns/node
  hash     raw:    2.08    2.09    2.08    2.12    2.94   min    2.08  med    2.09 ns/node
  ins_pre  raw:    9.88    9.86    9.39   10.47   12.43   min    9.39  med    9.88 ns/node
  hit_pre  raw:   27.70   27.43   28.37   30.24   33.03   min   27.43  med   28.37 ns/node
  mint     raw:   25.11   27.46   25.47   29.39   30.78   min   25.11  med   27.46 ns/node
  hit      raw:   45.51   50.08   47.98   53.39   53.40   min   45.51  med   50.08 ns/node
  probe after timed phases: cpu5, dependent add 0.323 ns
  cold mint (fresh mappings, page faults included, 1 run): 32.93 ns/node
  memory growth: resident +896.0 MiB, phys_footprint +896.5 MiB  = 56.0 bytes/node (arena 40 + table 16 = 56 by layout)
  check: bad=0 sink=5000005000000 maxrun=67
# bench_intern sha256_arm/16B  N=2^24=16777216  slots=33554432  entry=24 B  table=768.0 MiB  arena=640.0 MiB
  probe at start: cpu9, dependent add 0.349 ns
  table: 16777216 of 33554432 slots used, longest occupied run 63
  gen      raw:    1.52    1.66    1.67    2.03    2.23   min    1.52  med    1.67 ns/node
  hash     raw:   18.96   22.57   21.61   27.94   28.41   min   18.96  med   22.57 ns/node
  ins_pre  raw:   13.53   15.84   14.82   20.98   16.49   min   13.53  med   15.84 ns/node
  hit_pre  raw:   22.56   25.57   25.17   31.86   25.97   min   22.56  med   25.57 ns/node
  mint     raw:   50.93   60.09   60.30   79.72   55.13   min   50.93  med   60.09 ns/node
  hit      raw:   54.19   55.33   62.23   72.82   48.23   min   48.23  med   55.33 ns/node
  probe after timed phases: cpu4, dependent add 0.276 ns
  cold mint (fresh mappings, page faults included, 1 run): 47.81 ns/node
  memory growth: resident +1408.0 MiB, phys_footprint +1408.7 MiB  = 88.0 bytes/node (arena 40 + table 48 = 88 by layout)
  check: bad=0 sink=5000005000000 maxrun=63
# bench_intern sha256_arm/32B  N=2^24=16777216  slots=33554432  entry=40 B  table=1280.0 MiB  arena=640.0 MiB
  probe at start: cpu14, dependent add 0.254 ns
  table: 16777216 of 33554432 slots used, longest occupied run 63
  gen      raw:    1.19    1.19    1.24    1.23    1.23   min    1.19  med    1.23 ns/node
  hash     raw:   16.47   16.47   16.63   16.72   16.75   min   16.47  med   16.63 ns/node
  ins_pre  raw:   15.22   15.23   15.85   15.40   15.93   min   15.22  med   15.40 ns/node
  hit_pre  raw:   24.20   23.57   25.22   24.38   24.41   min   23.57  med   24.38 ns/node
  mint     raw:   44.75   46.84   47.80   48.63   53.11   min   44.75  med   47.80 ns/node
  hit      raw:   47.79   50.11   49.74   49.83   52.48   min   47.79  med   49.83 ns/node
  probe after timed phases: cpu10, dependent add 0.262 ns
  cold mint (fresh mappings, page faults included, 1 run): 53.45 ns/node
  memory growth: resident +1920.0 MiB, phys_footprint +1921.0 MiB  = 120.1 bytes/node (arena 40 + table 80 = 120 by layout)
  check: bad=0 sink=5000005000000 maxrun=63
# bench_intern blake3_min/32B  N=2^24=16777216  slots=33554432  entry=40 B  table=1280.0 MiB  arena=640.0 MiB
  probe at start: cpu6, dependent add 0.254 ns
  table: 16777216 of 33554432 slots used, longest occupied run 68
  gen      raw:    1.38    1.48    1.68    1.72    1.42   min    1.38  med    1.48 ns/node
  hash     raw:   47.99   54.15   60.30   65.10   47.84   min   47.84  med   54.15 ns/node
  ins_pre  raw:   15.76   16.35   18.82   18.60   15.22   min   15.22  med   16.35 ns/node
  hit_pre  raw:   25.50   26.81   30.29   29.54   24.81   min   24.81  med   26.81 ns/node
  mint     raw:  167.89  165.09  176.17  176.64  156.11   min  156.11  med  167.89 ns/node
  hit      raw:  172.07  176.66  188.27  166.53  160.38   min  160.38  med  172.07 ns/node
  probe after timed phases: cpu9, dependent add 0.253 ns
  cold mint (fresh mappings, page faults included, 1 run): 155.91 ns/node
  memory growth: resident +1920.0 MiB, phys_footprint +1921.0 MiB  = 120.1 bytes/node (arena 40 + table 80 = 120 by layout)
  check: bad=0 sink=5000005000000 maxrun=68
# bench_intern CC_SHA256/32B  N=2^24=16777216  slots=33554432  entry=40 B  table=1280.0 MiB  arena=640.0 MiB
  probe at start: cpu12, dependent add 0.229 ns
  table: 16777216 of 33554432 slots used, longest occupied run 63
  gen      raw:    1.31    1.33    1.41    1.42    1.40   min    1.31  med    1.40 ns/node
  hash     raw:  107.04  104.17  107.14  106.63  106.39   min  104.17  med  106.63 ns/node
  ins_pre  raw:   14.81   14.43   15.60   15.41   15.55   min   14.43  med   15.41 ns/node
  hit_pre  raw:   24.40   23.33   24.95   24.83   25.37   min   23.33  med   24.83 ns/node
  mint     raw:  219.86  212.12  225.08  222.69  221.87   min  212.12  med  221.87 ns/node
  hit      raw:  224.65  229.21  230.77  230.25  230.73   min  224.65  med  230.25 ns/node
  probe after timed phases: cpu15, dependent add 0.257 ns
  cold mint (fresh mappings, page faults included, 1 run): 221.33 ns/node
  memory growth: resident +1920.0 MiB, phys_footprint +1921.0 MiB  = 120.1 bytes/node (arena 40 + table 80 = 120 by layout)
  check: bad=0 sink=5000005000000 maxrun=63
19:06  up 27 days,  7:23, 3 users, load averages: 5.08 4.59 3.83
```

### `E/intern14_out.txt`

```
19:06  up 27 days,  7:24, 3 users, load averages: 4.42 4.47 3.81
# bench_intern mf128/16B  N=2^14=16384  slots=32768  entry=24 B  table=0.8 MiB  arena=0.6 MiB
  gen      raw:    1.36    1.36    1.35    1.35    1.37   min    1.35  med    1.36 ns/node
  hash     raw:    2.58    2.43    2.52    2.54    2.59   min    2.43  med    2.54 ns/node
  ins_pre  raw:    5.86    4.73    3.80    3.26    3.00   min    3.00  med    3.80 ns/node
  hit_pre  raw:    7.38    6.53    5.83    5.63    5.13   min    5.13  med    5.83 ns/node
  mint     raw:   12.81   12.14   11.10   10.55   10.12   min   10.12  med   11.10 ns/node
  hit      raw:   14.72   11.78    9.99    9.34    9.07   min    9.07  med    9.99 ns/node
# bench_intern cons-mf128/no-digest  N=2^14=16384  slots=32768  entry=8 B  table=0.2 MiB  arena=0.6 MiB
  gen      raw:    1.43    1.39    1.40    1.37    1.38   min    1.37  med    1.39 ns/node
  hash     raw:    2.10    2.13    2.07    2.18    2.15   min    2.07  med    2.13 ns/node
  ins_pre  raw:    4.62    4.05    3.22    2.59    2.19   min    2.19  med    3.22 ns/node
  hit_pre  raw:    6.84    5.64    4.44    3.57    3.18   min    3.18  med    4.44 ns/node
  mint     raw:   10.22    9.13    7.49    6.38    5.67   min    5.67  med    7.49 ns/node
  hit      raw:   12.05   11.21    9.70    8.71    8.18   min    8.18  med    9.70 ns/node
# bench_intern sha256_arm/16B  N=2^14=16384  slots=32768  entry=24 B  table=0.8 MiB  arena=0.6 MiB
  gen      raw:    1.12    1.12    1.13    1.11    1.13   min    1.11  med    1.12 ns/node
  hash     raw:   15.57   15.53   15.70   15.82   15.53   min   15.53  med   15.57 ns/node
  ins_pre  raw:    4.69    3.40    2.51    2.14    1.96   min    1.96  med    2.51 ns/node
  hit_pre  raw:    6.01    5.43    4.61    4.09    3.81   min    3.81  med    4.61 ns/node
  mint     raw:   26.29   25.15   23.52   22.67   21.79   min   21.79  med   23.52 ns/node
  hit      raw:   26.98   25.04   23.24   22.02   21.40   min   21.40  med   23.24 ns/node
# bench_intern sha256_arm/32B  N=2^14=16384  slots=32768  entry=40 B  table=1.2 MiB  arena=0.6 MiB
  gen      raw:    1.13    1.14    1.14    1.14    1.13   min    1.13  med    1.14 ns/node
  hash     raw:   16.43   16.58   16.41   16.58   16.64   min   16.41  med   16.58 ns/node
  ins_pre  raw:    5.41    4.93    4.08    3.38    2.99   min    2.99  med    4.08 ns/node
  hit_pre  raw:    8.67    6.02    4.20    3.38    3.04   min    3.04  med    4.20 ns/node
  mint     raw:   27.01   26.16   24.37   22.35   21.07   min   21.07  med   24.37 ns/node
  hit      raw:   30.38   29.03   25.96   23.71   22.49   min   22.49  med   25.96 ns/node
# bench_intern blake3_min/32B  N=2^14=16384  slots=32768  entry=40 B  table=1.2 MiB  arena=0.6 MiB
  gen      raw:    1.36    1.36    1.37    1.39    1.37   min    1.36  med    1.37 ns/node
  hash     raw:   48.04   47.77   47.49   47.29   47.13   min   47.13  med   47.49 ns/node
  ins_pre  raw:    5.41    4.51    3.74    3.02    2.32   min    2.32  med    3.74 ns/node
  hit_pre  raw:    8.79    6.86    5.60    3.99    2.83   min    2.83  med    5.60 ns/node
  mint     raw:   61.63   60.81   60.82   59.54   59.41   min   59.41  med   60.81 ns/node
  hit      raw:   64.22   63.76   63.88   63.15   62.66   min   62.66  med   63.76 ns/node
# bench_intern CC_SHA256/32B  N=2^14=16384  slots=32768  entry=40 B  table=1.2 MiB  arena=0.6 MiB
  gen      raw:    1.36    1.41    1.37    1.35    1.34   min    1.34  med    1.36 ns/node
  hash     raw:  108.13  108.74  107.23  108.36  107.84   min  107.23  med  108.13 ns/node
  ins_pre  raw:    5.48    4.04    2.70    1.99    1.75   min    1.75  med    2.70 ns/node
  hit_pre  raw:    8.80    5.96    3.46    2.34    2.19   min    2.19  med    3.46 ns/node
  mint     raw:  118.80  116.62  117.66  117.86  117.46   min  116.62  med  117.66 ns/node
  hit      raw:  129.74  120.41  118.44  120.34  119.66   min  118.44  med  120.34 ns/node
```

### `E/noble_out.txt`

```
blake3_min("")    = af1349b9f5f9a1a6a0404dea36dcc9499bcb25c9adc112b7cc9a93cae41f3262
blake3_min("abc") = 6437b3ac38465133ffb63b75273a8db548c558465d79db03fd359c6cd5bd9d85
blake3_min pattern len 0 = af1349b9f5f9a1a6a0404dea36dcc9499bcb25c9adc112b7cc9a93cae41f3262
blake3_min pattern len 1 = 2d3adedff11b61f14c886e35afa036736dcd87a74d27b5c1510225d0f592e213
blake3_min pattern len 2 = 7b7015bb92cf0b318037702a6cdd81dee41224f734684c2c122cd6359cb1ee63
blake3_min pattern len 16 = a6a492965517a830cb75fdb713465aa465f2f098233896fea44c1d98268bf9e3
blake3_min pattern len 40 = 9563a50f070b26eeaa1ee0a6ac95b8ba9168c32b1af71af2fe55a8b4e268ce5e
blake3_min pattern len 63 = e9bc37a594daad83be9470df7f7b3798297c3d834ce80ba85d6e207627b7db7b
blake3_min pattern len 64 = 4eed7141ea4a5cd4b788606bd23f46e212af9cacebacdc7d1f4c6dc7f2511b98
blake3_min pattern len 65 = de1e5fa0be70df6d2be8fffd0e99ceaa8eb6e8c93a63f2d8d1c30ecb6b263dee
blake3_min pattern len 256 = f462b63aae56ed9fb899ad8eb93aa35d3dd62773fda9c33bfe20f9dab5d3df5f
blake3_min pattern len 1023 = 10108970eeda3eb932baac1428c7a2163b0e924c9a9e25b35bba72b28f70bd11
blake3_min pattern len 1024 = 42214739f095a406f3fc83deb889744ac00df831c10daa55189b5d121c855af7
blake3_min pattern len 1025 = d00278ae47eb27b34faecf67b4fe263f82d5412916c1ffd97c8cb7fb814b8444
blake3_min pattern len 2048 = e776b6028c7cd22a4d0ba182a8bf62205d2ef576467e838ed6f2529b85fba24a
blake3_min pattern len 2049 = 5f4d72f40d7a5f82b15ca2b2e44b1de3c2ef86c426c95c1af0b6879522563030
blake3_min pattern len 3000 = 5fade288bf27444bee55ba2babb98c3c922c1e84c2e445e7d1f6da24756f5060
blake3_min pattern len 4096 = 015094013f57a5277b59d8475c0501042c0b642e531b0a1c8f58d2163229e969
blake3_min pattern len 8192 = aae792484c8efe4f19e2ca7d371d8c467ffb10748d8a5a1ae579948f718a2a63
```

