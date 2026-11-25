/* ck.c - the bootstrap: framebuffer, node store, canonical reader and printer, walker, code pages.
 *
 *   make            builds ./ck and fails when the bootstrap is over its line budget
 *   ./ck run F [B..] read the canonical print in F, evaluate it, print the outcome; each B is
 *                   evaluated first and the bindings it makes stay in place for F
 *   ./ck text F [B..] the same, and an answer that is a leaf is written as its own bytes
 *
 * Everything else, the lens, the ice and the checks among it, is a program these two run.
 *
 * Authority: spec/node-encoding.md, spec/core-operations.md and spec/framebuffer.md. The header
 * fb.h is made from nodes (region/header.ck); it is not written by hand.
 * Everything this program reads and writes lives in one shared region laid out by fb.h,
 * named /ck.<pid> or by the variable CK_FB; it keeps no heap and its stack is in the region.
 * Host symbols used: shm_open shm_unlink ftruncate fstat mmap mprotect open read write close
 * getenv getpid atexit exit _exit clock_gettime dlsym sigaction sigaltstack sigemptyset sigsetjmp
 * siglongjmp snprintf memcpy memmove memcmp memset strlen strcmp
 * pthread_attr_init pthread_attr_setstack pthread_create pthread_join, and on Apple silicon
 * pthread_jit_write_protect_np and sys_icache_invalidate.
 */
#if !defined(__APPLE__)
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <dlfcn.h>
#include <signal.h>
#include <setjmp.h>
#include "fb.h"
#if defined(__APPLE__) && defined(__aarch64__)
#include <libkern/OSCacheControl.h>
#define NATIVE 1
#else
#define NATIVE 0
#endif

static void put(int fd, const char *s) {
  for (size_t n = strlen(s); n; ) { ssize_t w = write(fd, s, n); if (w <= 0) exit(2); s += w; n -= (size_t)w; }
}
static void die(const char *m) { put(2, "ck: "); put(2, m); put(2, "\n"); exit(2); }
static void say(const char *a, const char *b) { put(1, a); put(1, b); put(1, "\n"); }

/* ---- the host: every capability the floor takes from it, one function each ------- */
typedef struct { int64_t v, k; } Ret;              /* what code answers: v, and k saying what v is */
typedef struct {
  void *(*region)(const char *name, uint64_t n);   /* the one shared region, zeroed */
  void *(*code_map)(size_t);                       /* reserve the one executable region */
  void (*code_write)(int on);                      /* this thread may write code / may run it */
  void (*code_flush)(void *, size_t);              /* instruction cache */
  int64_t (*file_read)(const char *, uint8_t *to, uint64_t cap);
  Ret (*call)(void *fn, const int64_t *a);
  int64_t (*clock)(void);                          /* nanoseconds that only move forward */
  void *(*watch)(const char *name);                /* another process's region, mapped to be read only */
  void *(*symbol)(const char *name);               /* where the host keeps what it calls by that name */
  void (*guard)(void (*on)(int), void *stack, size_t n);   /* a fault of the processor is handed to on, on its own stack */
} Host;
static int64_t h_clock(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return (int64_t)t.tv_sec * 1000000000 + t.tv_nsec; }
static void *h_region(const char *name, uint64_t n) {
  shm_unlink(name);
  int fd = shm_open(name, O_CREAT | O_EXCL | O_RDWR, 0600);
  if (fd < 0 || ftruncate(fd, (off_t)n)) return NULL;
  void *p = mmap(NULL, n, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  close(fd); return p == MAP_FAILED ? NULL : p;
}
static void *h_code_map(size_t n) {
#if NATIVE
  void *p = mmap(NULL, n, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANON | MAP_JIT, -1, 0);
  return p == MAP_FAILED ? NULL : p;
#else
  (void)n; return NULL;
#endif
}
static void h_code_write(int on) {
#if NATIVE
  pthread_jit_write_protect_np(!on);
#else
  (void)on;
#endif
}
static void h_code_flush(void *p, size_t n) {
#if NATIVE
  sys_icache_invalidate(p, n);
#else
  (void)p; (void)n;
#endif
}
static int64_t h_file_read(const char *path, uint8_t *to, uint64_t cap) {
  int fd = open(path, O_RDONLY); int64_t n = 0; ssize_t r = 0;
  if (fd < 0) return -1;
  while ((uint64_t)n < cap && (r = read(fd, to + n, cap - (uint64_t)n)) > 0) n += r;
  close(fd); return r < 0 ? -1 : n;
}
static Ret h_call(void *fn, const int64_t *a) {
  return ((Ret (*)(int64_t, int64_t, int64_t, int64_t, int64_t, int64_t, int64_t, int64_t))fn)(a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7]);
}
static void *h_watch(const char *name) {
  int fd = shm_open(name, O_RDONLY, 0); struct stat st;
  if (fd < 0) return NULL;
  void *p = fstat(fd, &st) ? MAP_FAILED : mmap(NULL, (size_t)st.st_size, PROT_READ, MAP_SHARED, fd, 0);
  close(fd); return p == MAP_FAILED ? NULL : p;
}
static void *h_symbol(const char *name) { return dlsym(RTLD_DEFAULT, name); }
static void h_guard(void (*on)(int), void *stack, size_t n) {
  stack_t ss; struct sigaction sa; memset(&sa, 0, sizeof sa); memset(&ss, 0, sizeof ss);
  ss.ss_sp = stack; ss.ss_size = n; sa.sa_handler = on; sa.sa_flags = SA_ONSTACK; sigemptyset(&sa.sa_mask);
  sigaltstack(&ss, NULL); sigaction(SIGSEGV, &sa, NULL); sigaction(SIGBUS, &sa, NULL); sigaction(SIGILL, &sa, NULL); sigaction(SIGFPE, &sa, NULL);
}
static Host host = { h_region, h_code_map, h_code_write, h_code_flush, h_file_read, h_call, h_clock, h_watch, h_symbol, h_guard };

/* ---- the framebuffer: the one region, and where each table lies in it (fb.h) --------
 * These pointers are set once and never move. Every count is a field of the region. */
static Head *fb;
static Node *nd; static uint8_t *bytes, *tb, *text; static char *ob;
static H *kids, *tab, *idtab, *argp, *opened, *tk; static uint32_t *pnum, *bykey; static H *held;
static Frame *fr; static Link *lk; static Activation *act; static ChoicePoint *cp; static Gathered *coll; static Code *born; static Binding *bd, *out; static Trail *trail;
#define N(i) (fb->a[i].n)
#define W (&fb->m)
#define nn N(A_NODES)
#define nbytes N(A_LEAF_BYTES)
#define nkids N(A_CHILDREN)
#define tcap N(A_BY_CONTENT)
#define icap N(A_BY_ID)
#define nfr N(A_FRAMES)
#define nlink N(A_LINKS)
#define nact N(A_ACTIVATIONS)
#define nargp N(A_ARGUMENTS)
#define ncp N(A_CHOICE_POINTS)
#define ncoll N(A_GATHERED)
#define nborn N(A_BORN_CODE)
#define nbind N(A_BINDINGS)
#define nout N(A_OUTSIDE)
#define ntrail N(A_TRAIL)
#define nopen N(A_READER_OPENED)
#define ntk N(A_GATHERING_CHILDREN)
#define ntb N(A_GATHERING_BYTES)
#define nob N(A_PRINT)
#define ntext N(A_TEXT)
static const char *const AREA_NAME[A_COUNT] = { FB_AREA_NAMES };
static void room(int a, uint64_t need) {
  if (need <= fb->a[a].cap) return;
  put(2, "ck: the framebuffer has no room left for "); put(2, AREA_NAME[a]); put(2, "\n"); exit(2);
}
static char fbname[64]; static int fbkeep;
static void fb_close(void) { if (!fbkeep) shm_unlink(fbname); }
static void fb_open(void) {
  static const struct { uint32_t of, size, cap; } L[A_COUNT] = {
    [A_NODES] = { NODE, sizeof(Node), 1u << 22 },   [A_LEAF_BYTES] = { BYTE_TABLE, 1, 1u << 28 },
    [A_CHILDREN] = { WORD_TABLE, 4, 1u << 26 },             [A_BY_CONTENT] = { WORD_TABLE, 4, 1u << 23 },
    [A_BY_ID] = { WORD_TABLE, 4, 1u << 23 },            [A_FRAMES] = { FRAME, sizeof(Frame), 1u << 22 },
    [A_LINKS] = { LINK, sizeof(Link), 1u << 23 },   [A_ACTIVATIONS] = { ACTIVATION, sizeof(Activation), 1u << 23 },
    [A_ARGUMENTS] = { WORD_TABLE, 4, 1u << 25 },              [A_CHOICE_POINTS] = { CHOICE_POINT, sizeof(ChoicePoint), 1u << 20 },
    [A_GATHERED] = { GATHERED, sizeof(Gathered), 1u << 22 }, [A_BORN_CODE] = { CODE, sizeof(Code), 1u << 12 },
    [A_BINDINGS] = { BINDING, sizeof(Binding), 1u << 16 },   [A_BOUND_BY_NODE] = { WORD_TABLE, 4, 1u << 22 },
    [A_OUTSIDE] = { BINDING, sizeof(Binding), 127 },    /* OMOST, below */
    [A_CELLS] = { WORD_TABLE, 4, 1u << 22 },                [A_TRAIL] = { TRAIL, sizeof(Trail), 1u << 22 },
    [A_READER_OPENED] = { WORD_TABLE, 4, 1u << 22 },             [A_GATHERING_CHILDREN] = { WORD_TABLE, 4, 1u << 22 },
    [A_GATHERING_BYTES] = { BYTE_TABLE, 1, 1u << 26 },               [A_PRINT] = { BYTE_TABLE, 1, 1u << 26 },
    [A_PRINT_NUMBERS] = { WORD_TABLE, 4, 1u << 22 },
    [A_TEXT] = { BYTE_TABLE, 1, 1u << 28 },             [A_STACK] = { BYTE_TABLE, 1, 1u << 26 },
    [A_ASKED] = { BYTE_TABLE, 1, 1u << 16 },            [A_ANSWERED] = { BYTE_TABLE, 1, 1u << 20 } };
  uint64_t off[A_COUNT], at = FB_PAGE;
  for (int i = 0; i < A_COUNT; i++) {
    if (i == A_STACK) at += FB_PAGE;                      /* a page nothing may touch, under the stack */
    off[i] = at; at += ((uint64_t)L[i].size * L[i].cap + FB_PAGE - 1) / FB_PAGE * FB_PAGE;
  }
  const char *e = getenv("CK_FB");
  if (e) { fbkeep = 1; snprintf(fbname, sizeof fbname, "%s", e); } else snprintf(fbname, sizeof fbname, "/ck.%d", (int)getpid());
  if (!(fb = host.region(fbname, at))) die("cannot make the framebuffer");
  atexit(fb_close);
  for (int i = 0; i < A_COUNT; i++) fb->a[i] = (Area){ E(AREA), 0, E(L[i].of), L[i].size, L[i].cap, 0, off[i] };
  fb->bp = E(FRAMEBUFFER); fb->version = 9; fb->floor = (uint32_t)(off[A_STACK] / FB_PAGE) + 1; fb->areas = A_COUNT; fb->pid = (uint32_t)getpid(); fb->bytes = at; W->bp = E(MACHINE);
  mprotect((uint8_t *)fb + off[A_STACK] - FB_PAGE, FB_PAGE, PROT_NONE);
#define PLACE(i) ((void *)((uint8_t *)fb + off[i]))
  nd = PLACE(A_NODES); bytes = PLACE(A_LEAF_BYTES); kids = PLACE(A_CHILDREN); tab = PLACE(A_BY_CONTENT); idtab = PLACE(A_BY_ID); fr = PLACE(A_FRAMES);
  lk = PLACE(A_LINKS); act = PLACE(A_ACTIVATIONS); argp = PLACE(A_ARGUMENTS); cp = PLACE(A_CHOICE_POINTS); coll = PLACE(A_GATHERED); born = PLACE(A_BORN_CODE); bd = PLACE(A_BINDINGS); out = PLACE(A_OUTSIDE); bykey = PLACE(A_BOUND_BY_NODE); held = PLACE(A_CELLS); trail = PLACE(A_TRAIL);
  opened = PLACE(A_READER_OPENED); tk = PLACE(A_GATHERING_CHILDREN); tb = PLACE(A_GATHERING_BYTES); ob = PLACE(A_PRINT); pnum = PLACE(A_PRINT_NUMBERS); text = PLACE(A_TEXT);
  nn = 1;
}
/* Working bytes, taken from the top of the text area and given back by resetting ntext. */
static void *talloc(size_t n) {
  room(A_TEXT, (uint64_t)ntext + n + 16); void *p = text + ntext;
  memset(p, 0, n); ntext += (uint32_t)((n + 7) & ~(size_t)7); return p;
}
static uint8_t *load(const char *path, size_t *len) {      /* a file read straight into the region, a zero after it */
  room(A_TEXT, (uint64_t)ntext + 32); uint8_t *b = text + ntext; uint64_t cap = fb->a[A_TEXT].cap - ntext - 16;
  int64_t n = host.file_read(path, b, cap);
  if (n < 0 || (uint64_t)n >= cap) { put(2, "ck: "); put(2, path); put(2, ": cannot read\n"); exit(2); }
  b[n] = 0; ntext += (uint32_t)((n + 8) & ~7ll); *len = (size_t)n; return b;
}

/* ---- SHA-256 (FIPS 180-4) ------------------------------------------------------- */
static const uint32_t K[64] = {
  0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
  0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
  0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
  0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
  0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
  0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
  0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
  0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2};
#define ROR(x,r) (((x) >> (r)) | ((x) << (32 - (r))))
static void sha_block(uint32_t h[8], const uint8_t *p) {
  uint32_t w[64], a[8];
  for (int i = 0; i < 16; i++)
    w[i] = (uint32_t)p[4*i] << 24 | (uint32_t)p[4*i+1] << 16 | (uint32_t)p[4*i+2] << 8 | p[4*i+3];
  for (int i = 16; i < 64; i++) {
    uint32_t s0 = ROR(w[i-15],7) ^ ROR(w[i-15],18) ^ (w[i-15] >> 3);
    uint32_t s1 = ROR(w[i-2],17) ^ ROR(w[i-2],19) ^ (w[i-2] >> 10);
    w[i] = w[i-16] + s0 + w[i-7] + s1;
  }
  memcpy(a, h, sizeof a);
  for (int i = 0; i < 64; i++) {
    uint32_t s1 = ROR(a[4],6) ^ ROR(a[4],11) ^ ROR(a[4],25);
    uint32_t ch = (a[4] & a[5]) ^ (~a[4] & a[6]);
    uint32_t t1 = a[7] + s1 + ch + K[i] + w[i];
    uint32_t s0 = ROR(a[0],2) ^ ROR(a[0],13) ^ ROR(a[0],22);
    uint32_t mj = (a[0] & a[1]) ^ (a[0] & a[2]) ^ (a[1] & a[2]);
    uint32_t t2 = s0 + mj;
    a[7] = a[6]; a[6] = a[5]; a[5] = a[4]; a[4] = a[3] + t1;
    a[3] = a[2]; a[2] = a[1]; a[1] = a[0]; a[0] = t1 + t2;
  }
  for (int i = 0; i < 8; i++) h[i] += a[i];
}
static void sha256(const uint8_t *msg, size_t len, uint8_t out[32]) {
  uint32_t h[8] = {0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};
  size_t i = 0; uint8_t tail[128]; size_t r;
  for (; i + 64 <= len; i += 64) sha_block(h, msg + i);
  r = len - i; memcpy(tail, msg + i, r); tail[r++] = 0x80;
  size_t padded = r <= 56 ? 64 : 128;
  memset(tail + r, 0, padded - r);
  uint64_t bits = (uint64_t)len * 8;
  for (int b = 0; b < 8; b++) tail[padded - 1 - b] = (uint8_t)(bits >> (8 * b));
  sha_block(h, tail); if (padded == 128) sha_block(h, tail + 64);
  for (int j = 0; j < 8; j++) { out[4*j] = h[j] >> 24; out[4*j+1] = h[j] >> 16; out[4*j+2] = h[j] >> 8; out[4*j+3] = h[j]; }
}

/* ---- the store ---------------------------------------------------------------------
 * A node record is its blueprint's handle, its writer, and either bytes or child handles.
 * Equal nodes share one handle, found by a cheap hash and decided by comparing content.
 * The content id is computed the first time it is asked for. */
static long long catn(H h) {              /* catalogue number of h, or -1 */
  if (h >= 1 && h <= SEEDED) return h - 1;
  if (!h || nd[h].comp || nd[h].bp != ROOT) return -1;
  long long v = 0; for (uint32_t i = nd[h].n; i > 0; i--) v = v << 8 | bytes[nd[h].off + i - 1];
  return v;
}
static int utf8_ok(const uint8_t *s, size_t n) {
  for (size_t i = 0; i < n; ) {
    uint8_t c = s[i]; uint32_t cp_; size_t k;
    if (c < 0x80) { i++; continue; }
    else if ((c & 0xe0) == 0xc0) { k = 1; cp_ = c & 0x1f; }
    else if ((c & 0xf0) == 0xe0) { k = 2; cp_ = c & 0x0f; }
    else if ((c & 0xf8) == 0xf0) { k = 3; cp_ = c & 0x07; }
    else return 0;
    if (i + k >= n) return 0;
    for (size_t j = 1; j <= k; j++) { if ((s[i+j] & 0xc0) != 0x80) return 0; cp_ = cp_ << 6 | (s[i+j] & 0x3f); }
    if ((k == 1 && cp_ < 0x80) || (k == 2 && cp_ < 0x800) || (k == 3 && cp_ < 0x10000)) return 0;
    if (cp_ > 0x10ffff || (cp_ >= 0xd800 && cp_ <= 0xdfff)) return 0;
    i += k + 1;
  }
  return 1;
}
/* spec/node-encoding.md sections 4 and 5: the rule a node with this blueprint breaks, or NULL. */
static const char *rule(int comp, H bp, const void *payload, uint32_t n) {
  const uint8_t *b = payload;
  if (!bp) return "nothing-blueprint";
  if (comp) for (uint32_t i = 0; i < n; i++) if (!((const H *)payload)[i]) return "nothing-child";
  long long c = catn(bp);
  if (c < 0 || c >= 32) return NULL;
  if (c == 15 || c >= 17) return "kept";
  if (c == 16) return comp ? NULL : "composite-only";
  if (comp) return "leaf-only";
  switch (c) {
  case 0: return (n >= 1 && n <= 4 && b[n-1] != 0) ? NULL : "ordinal";
  case 2: return utf8_ok(b, n) ? NULL : "utf8";
  case 3: return (n == 0 || b[n-1] != 0) ? NULL : "minimal";
  case 4:
    if (n == 0) return NULL;
    if (b[n-1] == 0x00 && (n == 1 || !(b[n-2] & 0x80))) return "minimal";
    if (b[n-1] == 0xff && n >= 2 && (b[n-2] & 0x80)) return "minimal";
    return NULL;
  case 5: case 9:  return n == 1 ? NULL : "width";
  case 6: case 10: return n == 2 ? NULL : "width";
  case 7: case 11: return n == 4 ? NULL : "width";
  case 8: case 12: return n == 8 ? NULL : "width";
  case 13: {
    if (n != 4) return "width";
    uint32_t u = (uint32_t)b[0] | (uint32_t)b[1] << 8 | (uint32_t)b[2] << 16 | (uint32_t)b[3] << 24;
    return ((u & 0x7f800000u) == 0x7f800000u && (u & 0x007fffffu) && u != 0x7fc00000u) ? "nan" : NULL; }
  case 14: {
    if (n != 8) return "width";
    uint64_t u = 0; for (int i = 7; i >= 0; i--) u = u << 8 | b[i];
    return ((u & 0x7ff0000000000000ull) == 0x7ff0000000000000ull && (u & 0x000fffffffffffffull)
            && u != 0x7ff8000000000000ull) ? "nan" : NULL; }
  }
  return NULL;
}
static uint32_t hash(int comp, H bp, const void *payload, uint32_t n) {
  uint32_t h = 2166136261u ^ (uint32_t)comp; const uint8_t *p = payload;
  h = (h ^ bp) * 16777619u;
  for (size_t i = 0, m = comp ? (size_t)n * sizeof(H) : n; i < m; i++) h = (h ^ p[i]) * 16777619u;
  return h;
}
static const void *payload_of(H h) { return nd[h].comp ? (const void *)(kids + nd[h].off) : (const void *)(bytes + nd[h].off); }
static void tab_put(H h) {
  uint32_t i = hash(nd[h].comp, nd[h].bp, payload_of(h), nd[h].n) & (tcap - 1);
  while (tab[i]) i = (i + 1) & (tcap - 1);
  tab[i] = h;
}
/* The one node with this content. Refused content sets *why and answers nothing.
 * A new record is whole before nn moves past it; its writer is the node now running. */
static H mk(int comp, H bp, const void *payload, uint32_t n, const char **why) {
  if (bp || nn > 1) { if ((*why = rule(comp, bp, payload, n))) return 0; }
  size_t sz = comp ? (size_t)n * sizeof(H) : n;
  if (!tcap || nn * 2 > tcap) {                 /* the table by content grows where it lies */
    uint32_t c = tcap ? tcap * 2 : 4096; room(A_BY_CONTENT, c); memset(tab, 0, (size_t)c * sizeof *tab); tcap = c;
    for (H h = 1; h < nn; h++) tab_put(h);
  }
  uint32_t i = hash(comp, bp, payload, n) & (tcap - 1);
  for (; tab[i]; i = (i + 1) & (tcap - 1)) {
    Node *x = &nd[tab[i]];
    if (x->comp == comp && x->bp == bp && x->n == n && !memcmp(payload_of(tab[i]), payload, sz)) return tab[i];
  }
  room(A_NODES, (uint64_t)nn + 1); Node *x = &nd[nn]; memset(x, 0, sizeof *x);
  x->bp = bp; x->by = W->at; x->comp = (uint8_t)comp; x->n = n;
  if (comp) { room(A_CHILDREN, (uint64_t)nkids + n); x->off = nkids; memcpy(kids + nkids, payload, sz); nkids += n; }
  else { room(A_LEAF_BYTES, (uint64_t)nbytes + n); x->off = nbytes; memcpy(bytes + nbytes, payload, sz); nbytes += n; }
  tab[i] = nn; return nn++;
}
#define KID(x, i) (kids[nd[x].off + (i)])
static uint32_t idslot(const uint8_t *id) {
  uint32_t i = ((uint32_t)id[0] | (uint32_t)id[1] << 8 | (uint32_t)id[2] << 16 | (uint32_t)id[3] << 24) & (icap - 1);
  while (idtab[i] && memcmp(nd[idtab[i]].id, id, 32)) i = (i + 1) & (icap - 1);
  return i;
}
/* Section 2: canonical bytes of h, in working bytes the caller gives back. */
static const uint8_t *idof(H h);
static uint8_t *cbytes(H h, size_t *len) {
  uint32_t n = nd[h].n; *len = 33 + (nd[h].comp ? (size_t)n * 32 : n);
  uint8_t *b = talloc(*len);
  b[0] = nd[h].comp;
  if (nd[h].bp) memcpy(b + 1, idof(nd[h].bp), 32);
  if (nd[h].comp) for (uint32_t i = 0; i < n; i++) memcpy(b + 33 + 32 * (size_t)i, idof(KID(h, i)), 32);
  else memcpy(b + 33, bytes + nd[h].off, n);
  return b;
}
static void setid(H h, const uint8_t *id) {
  if (nd[h].hasid) return;
  memcpy(nd[h].id, id, 32); nd[h].hasid = 1;
  if (!icap || W->ids * 2 >= icap) {
    uint32_t c = icap ? icap * 2 : 4096; room(A_BY_ID, c); memset(idtab, 0, (size_t)c * sizeof *idtab); icap = c;
    for (H x = 1; x < nn; x++) if (nd[x].hasid && x != h) idtab[idslot(nd[x].id)] = x;
  }
  idtab[idslot(id)] = h; W->ids++;
}
static const uint8_t *idof(H h) {         /* section 3 */
  if (!nd[h].hasid) { uint32_t m = ntext; size_t len; uint8_t *b = cbytes(h, &len), id[32]; sha256(b, len, id); ntext = m; setid(h, id); }
  return nd[h].id;
}
static H byid(const uint8_t *id) { return icap ? idtab[idslot(id)] : 0; }
static const uint8_t ZERO[32];
/* Section 2 read backwards: the node these canonical bytes are, every part already known. */
static H from_bytes(const uint8_t *b, size_t len, const char **why) {
  *why = NULL;
  if (len < 33) { *why = "short"; return 0; }
  if (b[0] > 1) { *why = "kind"; return 0; }
  size_t n = len - 33; int comp = b[0];
  if (!memcmp(b + 1, ZERO, 32)) { if (!comp && !n) return ROOT; *why = "nothing-blueprint"; return 0; }
  if (comp) {
    if (n % 32) { *why = "child-length"; return 0; }
    for (size_t i = 0; i < n; i += 32) if (!memcmp(b + 33 + i, ZERO, 32)) { *why = "nothing-child"; return 0; }
  }
  H bp = byid(b + 1), h; if (!bp) { *why = "unknown"; return 0; }
  if (comp) {
    uint32_t m = (uint32_t)(n / 32), base = ntk; room(A_GATHERING_CHILDREN, (uint64_t)ntk + m);
    for (uint32_t i = 0; i < m; i++) if (!(tk[base + i] = byid(b + 33 + 32 * (size_t)i))) { *why = "unknown"; return 0; }
    h = mk(1, bp, tk + base, m, why);
  } else h = mk(0, bp, b + 33, (uint32_t)n, why);
  if (h) { uint8_t id[32]; sha256(b, len, id); setid(h, id); }
  return h;
}
static void store_init(void) {
  const char *why; uint8_t b[4];
  mk(0, 0, "", 0, &why);                                  /* the root: blueprint nothing, no bytes */
  for (uint32_t n = 1; n < SEEDED; n++) { b[0] = (uint8_t)n; mk(0, ROOT, b, 1, &why); }
  for (H h = 1; h <= SEEDED; h++) idof(h);
  memcpy(fb->magic, FB_MAGIC, 8);                         /* from here a reader can make sense of the region */
}
static H txt(const char *s) { const char *why; return mk(0, E(TEXT), s, (uint32_t)strlen(s), &why); }
static int geti(H h, int64_t *out) {
  if (!h || nd[h].comp || nd[h].bp != E(INT) || nd[h].n > 8) return 0;
  uint32_t n = nd[h].n; const uint8_t *b = bytes + nd[h].off;
  uint64_t u = (n && (b[n-1] & 0x80)) ? ~0ull : 0;
  for (uint32_t i = n; i > 0; i--) u = u << 8 | b[i-1];
  *out = (int64_t)u; return 1;
}
static H mki(int64_t v) {
  uint8_t b[8]; uint32_t n = 8; const char *why;
  for (int i = 0; i < 8; i++) b[i] = (uint8_t)((uint64_t)v >> (8 * i));
  while (n > 1 && ((b[n-1] == 0x00 && !(b[n-2] & 0x80)) || (b[n-1] == 0xff && (b[n-2] & 0x80)))) n--;
  if (n == 1 && b[0] == 0) n = 0;
  return mk(0, E(INT), b, n, &why);
}

/* ---- section 6: the canonical print, read and written -------------------------------- */
typedef struct { const char *p, *err; } Rd;               /* where the reader stands, and its first refusal */
static H refuse(Rd *r, const char *w) { if (!r->err) r->err = w; return 0; }
static int number(Rd *r, uint32_t *out) {
  const char *p = r->p;
  if (*p < '0' || *p > '9' || (*p == '0' && p[1] >= '0' && p[1] <= '9')) return 0;
  uint64_t v = 0;
  for (; *p >= '0' && *p <= '9'; p++) { v = v * 10 + (unsigned)(*p - '0'); if (v > 4294967295ull) return 0; }
  r->p = p; *out = (uint32_t)v; return 1;
}
static int hexval(char c) { return c >= '0' && c <= '9' ? c - '0' : c >= 'a' && c <= 'f' ? c - 'a' + 10 : -1; }
static H read_node(Rd *r) {
  uint32_t n; const char *why;
  if (*r->p == '#') {
    r->p++; if (!number(r, &n)) return refuse(r, "syntax");
    if (n < SEEDED) return E(n);
    uint8_t b[4]; uint32_t len = 0; for (; n; n >>= 8) b[len++] = (uint8_t)n;
    return mk(0, ROOT, b, len, &why);
  }
  if (*r->p == '^') { r->p++; if (!number(r, &n)) return refuse(r, "syntax");
    return (n < nopen && opened[n]) ? opened[n] : refuse(r, "forward"); }
  if (*r->p != '[' && *r->p != '(') return refuse(r, "syntax");
  int comp = *r->p++ == '('; uint32_t me = nopen; H x;
  room(A_READER_OPENED, (uint64_t)nopen + 1); opened[nopen++] = 0;
  H bp = read_node(r); if (!bp) return 0;
  if (comp) {
    uint32_t base = ntk;
    while (*r->p == ' ') { r->p++; H k = read_node(r); if (!k) { ntk = base; return 0; }
      room(A_GATHERING_CHILDREN, (uint64_t)ntk + 1); tk[ntk++] = k; }
    if (*r->p++ != ')') { ntk = base; return refuse(r, "syntax"); }
    x = mk(1, bp, tk + base, ntk - base, &why); ntk = base;
  } else {
    uint32_t base = ntb;
    if (*r->p == ' ') { r->p++;
      if (hexval(*r->p) < 0) return refuse(r, "syntax");
      while (hexval(*r->p) >= 0) {
        if (hexval(r->p[1]) < 0) { ntb = base; return refuse(r, "syntax"); }
        room(A_GATHERING_BYTES, (uint64_t)ntb + 1); tb[ntb++] = (uint8_t)(hexval(r->p[0]) << 4 | hexval(r->p[1])); r->p += 2; } }
    if (*r->p++ != ']') { ntb = base; return refuse(r, "syntax"); }
    x = mk(0, bp, tb + base, ntb - base, &why); ntb = base;
  }
  if (!x) return refuse(r, why);
  if (catn(x) >= 0) return refuse(r, "not-canonical");                         /* rule 2 */
  for (uint32_t j = 0; j < nopen; j++) if (opened[j] == x) return refuse(r, "not-canonical");     /* rule 4 */
  opened[me] = x; return x;
}
/* A print ends with the text or at a line end. What a file holds after that line lies beside
 * the node and is no part of it: a lens keeps the names it gave there. */
static H read_print(const char *s, const char **why) {    /* s lies in the region */
  Rd r = { s, NULL }; nopen = 0;
  H x = read_node(&r);
  if (x && *r.p && *r.p != '\n') x = refuse(&r, "syntax");
  *why = r.err; return x;
}
static void oput(const char *s) { uint32_t l = (uint32_t)strlen(s); room(A_PRINT, (uint64_t)nob + l + 1); memcpy(ob + nob, s, l + 1); nob += l; }
static const char *dec(char t[24], uint64_t v) { char *e = t + 23; *e = 0; do *--e = (char)('0' + v % 10); while (v /= 10); return e; }
static const char HEX[] = "0123456789abcdef";
static void print_node(H x) {
  char t[24]; long long c = catn(x);
  if (c >= 0) { oput("#"); oput(dec(t, (uint64_t)c)); return; }
  if (pnum[x]) { oput("^"); oput(dec(t, pnum[x] - 1)); return; }
  pnum[x] = ++W->pcount; room(A_GATHERING_CHILDREN, (uint64_t)ntk + 1); tk[ntk++] = x;
  oput(nd[x].comp ? "(" : "["); print_node(nd[x].bp);
  if (nd[x].comp) for (uint32_t i = 0; i < nd[x].n; i++) { oput(" "); print_node(KID(x, i)); }
  else if (nd[x].n) { oput(" "); t[2] = 0;
    for (uint32_t i = 0; i < nd[x].n; i++) { uint8_t b = bytes[nd[x].off + i]; t[0] = HEX[b >> 4]; t[1] = HEX[b & 15]; oput(t); } }
  oput(nd[x].comp ? ")" : "]");
}
static const char *print(H x) {
  room(A_PRINT_NUMBERS, (uint64_t)nn + 1); N(A_PRINT_NUMBERS) = nn; W->pcount = 0; nob = 0; oput("");
  uint32_t base = ntk; print_node(x);
  while (ntk > base) pnum[tk[--ntk]] = 0;         /* a print costs what it prints: only its own numbers are taken back */
  return ob;
}

/* ---- code pages -----------------------------------------------------------------------
 * The host does not let executable pages be shared, so the runnable copy of a code leaf
 * lies outside the region. The leaf, where its copy starts and who asked for it are here. */
#define CODE_REGION (1u << 20)
static void *birth(H leaf) {              /* the leaf's instructions, runnable; NULL if this host cannot */
  uint8_t *code = (uint8_t *)(uintptr_t)W->code; uint32_t n = nd[leaf].n;
  for (uint32_t i = 0; i < nborn; i++) if (born[i].leaf == leaf) return code + born[i].off;
  if (!code) { if (!(code = host.code_map(CODE_REGION))) return NULL; W->code = (uint64_t)(uintptr_t)code; }
  if (!n || W->ncode + n > CODE_REGION) return NULL;
  void *fn = code + W->ncode;
  host.code_write(1); memcpy(fn, bytes + nd[leaf].off, n); host.code_write(0);
  host.code_flush(fn, n);
  room(A_BORN_CODE, (uint64_t)nborn + 1); born[nborn] = (Code){ E(CODE), W->at, leaf, W->ncode }; nborn++;
  W->ncode += (n + 15u) & ~15u;
  return fn;
}

/* ---- the walker ------------------------------------------------------------------------
 * A machine with no C recursion whose registers are the region's Machine record.
 * Continuation frames are never changed once made, so a choice point can keep one and
 * return to it; fail drops whatever was made after it. */
static uint32_t push(uint32_t t, H node, uint32_t i, uint32_t env, uint32_t cut, uint32_t vals, uint32_t k, uint32_t aux) {
  room(A_FRAMES, (uint64_t)nfr + 1); fr[nfr] = (Frame){ E(FRAME), W->at, t, node, i, env, cut, vals, k, aux, nact }; return nfr++;
}
/* Before a call or a let in the last position makes its activation: the activations made since the
 * frame it answers to are given back when no open choice and no user above (up) can return to
 * them. So a loop written as a recursion runs in the room of one round. */
static void last(uint32_t up) {
  uint32_t keep = W->k ? fr[W->k].mact : 1;
  if (ncp && cp[ncp-1].mact > keep) keep = cp[ncp-1].mact;
  if (up >= keep) keep = up + 1;
  if (keep < nact) { nargp = act[keep].off; nact = keep; }
}
static uint32_t again(uint32_t env, uint32_t n) {       /* a new activation like env, with room for n arguments */
  Activation o = act[env]; if (n < o.n) n = o.n;
  last(o.up); room(A_ACTIVATIONS, (uint64_t)nact + 1); room(A_ARGUMENTS, (uint64_t)nargp + n);
  memmove(argp + nargp, argp + o.off, sizeof(H) * o.n);            /* its own arguments may be the ones just given back */
  for (uint32_t i = o.n; i < n; i++) argp[nargp + i] = 0;
  o.by = W->at; o.off = nargp; o.n = n; act[nact] = o; nargp += n;
  return nact++;
}
static uint32_t enter(H recipe, const H *a, uint32_t n, uint32_t lim, uint32_t up, uint32_t cut) {
  room(A_ACTIVATIONS, (uint64_t)nact + 1); room(A_ARGUMENTS, (uint64_t)nargp + n);
  memcpy(argp + nargp, a, sizeof(H) * n); act[nact] = (Activation){ E(ACTIVATION), W->at, recipe, nargp, n, lim, up, cut }; nargp += n;
  return nact++;
}
/* What runs entry e. A program's own bindings are counted from 1 and a limit says how many of them
 * count: an implementation runs on the bindings beneath its own, so it can use the entry it
 * replaces and two entries can each be written with the other. Bindings made from outside the run
 * lie above all of those, are seen at every limit and are counted from OUT + 1. A limit holds
 * both counts, and OWN when what is bound under it is the program's own. 0: the bootstrap's code. */
#define PM 0xffffffu
#define OUT (PM + 1)
#define OMOST 127u
#define OWN 0x80000000u
#define B(b) ((b) > OUT ? &out[(b) - OUT - 1] : &bd[(b) - 1])
static uint32_t newest(H e, uint32_t lim) {             /* down the chain of e's own bindings, newest first */
  uint32_t i = e < N(A_BOUND_BY_NODE) ? bykey[e] : 0;
  while (i > lim) i = bd[i-1].prev;
  return i;
}
static uint32_t bound(H e, H any, uint32_t lim) {       /* any: a second entry that also counts, or nothing */
  uint32_t j = lim >> 24 & OMOST, i, k;
  for (j = j > nout ? nout : j; j > 0; j--) if (out[j-1].entry && (out[j-1].entry == e || out[j-1].entry == any)) return OUT + j;
  i = newest(e, lim & PM); k = any ? newest(any, lim & PM) : 0;
  return i > k ? i : k;
}
static uint32_t under(uint32_t b, uint32_t lim) {       /* the limit that leaves out b and all above it */
  return b > OUT ? (lim & ~(OMOST << 24)) | (b - OUT - 1) << 24 : (lim & ~PM) | (b - 1);
}
/* The c links ending at vals hold values already copied out. When they are the newest
 * links and no open choice could return to them, their room is given back. */
static void unlink_vals(uint32_t vals, uint32_t c) {
  if (vals != nlink - 1 || nlink < c + 1 || (ncp && nlink - c < cp[ncp-1].mlink)) return;
  for (uint32_t j = 0, q = vals; j < c; j++, q = lk[q].next) if (q != vals - j) return;
  nlink -= c;
}
static long long argn(H x) {              /* which argument a node (arg k) names; -1 when it is not one */
  H i = x && nd[x].comp && nd[x].bp == E(ARG) && nd[x].n == 1 ? KID(x, 0) : 0; long long j = 0;
  if (!i || nd[i].comp || nd[i].bp != E(NAT) || nd[i].n > 3) return -1;
  for (uint32_t b = nd[i].n; b > 0; b--) j = j << 8 | bytes[nd[i].off + b - 1];
  return j;
}
static int is_recipe(H f) { return f && nd[f].comp && nd[f].bp == E(RECIPE) && nd[f].n == 1; }
static int is_lazy(H f) { return f && nd[f].comp && nd[f].bp == E(LAZY) && nd[f].n == 1; }
/* Entry e names data: a node under it answers itself and e cannot be bound. A leaf, e below 0, is data too. */
static int is_data(long long e) { return e < QUOTE || e == RECIPE || (e > NATIVE_OP && e < BLUEPRINT) || e == BINDING || e == LAZY || e == TRAIL; }
static void fault(const char *w) { W->reason = txt(w); W->mode = M_FAULT; }
static void answer(H v) { W->v = v; W->mode = M_RET; }
/* What an outcome answers: a word and a node, as 'ck run' prints them. */
static H seen_as(const char *word, H v) { const char *why; H p[2] = { txt(word), v }; return mk(1, E(LIST), p, 2, &why); }
/* Call code and take its answer: k 0 a number, k 1 yes or no, k 4 the node whose handle v is,
 * any other k out of range. With k 3 the code declines and 0 is answered: what lies beneath runs. */
/* Code a program gave is called under watch: when the processor faults inside it, the walker goes
 * on and the program has a fault, crash, like any other. A fault anywhere else ends the process. */
static sigjmp_buf in_code; static volatile sig_atomic_t coding;
static void crashed(int sig) {
  (void)sig; if (coding) siglongjmp(in_code, 1);
  put(2, "ck: the processor faulted outside any code a program gave it\n"); _exit(3);
}
static Ret called(void *fn, const int64_t *arg) { coding = 1; Ret r = host.call(fn, arg); coding = 0; return r; }
static int landed(void *fn, int64_t *arg) {
  if (!fn) { W->v = 0; W->reason = txt("no-native"); return 1; }
  arg[6] = (int64_t)(intptr_t)fb;         /* every call of code carries the region's address in its seventh register */
  Ret r = called(fn, arg);
  if (r.k == 3) return 0;
  if (r.k == 0) W->v = mki(r.v); else if (r.k == 1) W->v = r.v ? E(YES) : E(NO);
  else if (r.k == 4 && r.v > 0 && r.v < nn) W->v = (H)r.v; else { W->v = 0; W->reason = txt("range"); }
  return 1;
}
/* Run binding b: a recipe is entered on the bindings beneath b, a code leaf is called. With
 * node nothing they get values already in hand, code as integers. Otherwise they get the node
 * itself, unevaluated: a lazy keeps the cut scope, code gets the node's handle and the
 * activation it stands in. Answers 0 when code declined. */
static int apply(uint32_t b, uint32_t lim, const H *a, uint32_t c, uint32_t from, H node) {
  H impl = B(b)->impl, t[9]; int64_t arg[8] = {0};
  if (!nd[impl].comp && nd[impl].bp != E(CODE)) {       /* another kind of implementation: what its blueprint */
    uint32_t r = bound(nd[impl].bp, 0, lim = under(b, lim));       /* is bound to runs it, given the leaf and the values */
    if (!r || !nd[B(r)->impl].comp) { fault("kind"); return 1; }
    t[0] = impl; memcpy(t + 1, a, sizeof(H) * c); a = t; c++; b = r; impl = B(r)->impl;
  }
  if (nd[impl].comp) {
    uint32_t up = node ? from : act[from].up, cut = node ? W->cut : act[from].cut;      /* given answers, it has no user */
    if (!node) last(up);                                                                /* of its own: its caller's */
    uint32_t sees = b > OUT ? (lim & PM) | out[b-OUT-1].sees << 24                         /* bound from outside, it acts from there */
                            : (lim & OMOST << 24) | bd[b-1].sees | (act[from].lim & OWN);  /* the program's own acts as its user does */
    W->env = enter(impl, a, c, sees, up, cut); if (!node || !is_lazy(impl)) W->cut = ncp;
    W->x = KID(impl, 0); W->mode = M_EVAL; return 1;
  }
  if (node) { arg[0] = node; arg[1] = from; }
  else for (uint32_t j = 0; j < c; j++) if (j > 5 || !geti(a[j], &arg[j])) return 0;   /* code takes integers only */
  arg[7] = c;
  if (!landed(birth(impl), arg)) return 0;
  W->mode = M_RET; return 1;
}
/* Entries 64 to 71: a node taken apart, a node built from parts, and an entry bound. A build
 * the encoding refuses answers nothing, and the reason is the refusal's word. */
static void parts(long long op, const H *a, uint32_t c, int own) {
  const char *why = NULL; int64_t k = 0; H x = a[0], l = c == 2 ? a[1] : 0;
  if (c != (op == CLOCK ? 0u : op == BLUEPRINT || op == COUNT || op == LEAF || op >= READ ? 1u : 2u)) { fault("arity"); return; }
  if (!x || (c == 2 && !l)) { W->v = 0; W->reason = txt("nothing-in"); return; }
  if (op == PRINT) { print(x); W->v = mk(0, E(TEXT), ob, nob, &why); return; }
  if (op == ID) { W->v = mk(0, E(BYTES), idof(x), 32, &why); return; }
  if (op == READ || op == FILE_OP) {       /* a leaf's bytes as a print to read, or as the path of a file */
    if (nd[x].comp) { fault("kind"); return; }
    uint32_t m = ntext; char *s = talloc((size_t)nd[x].n + 1); memcpy(s, bytes + nd[x].off, nd[x].n);
    if (op == READ) { if (!(W->v = read_print(s, &why))) W->reason = txt(why); }
    else { int64_t got = host.file_read(s, text + ntext, fb->a[A_TEXT].cap - ntext - 16);
      if (got < 0) { W->v = 0; W->reason = txt("no-file"); } else W->v = mk(0, E(BYTES), text + ntext, (uint32_t)got, &why); }
    ntext = m; return;
  }
  if (op == BIND) {                       /* x: an entry that is not data, or a recipe; l: a recipe, a lazy or a leaf */
    long long e = catn(x);
    if ((e >= 0 && is_data(e)) || (e < 0 && !is_recipe(x)) || (nd[l].comp && !is_recipe(l) && !is_lazy(l))) { fault("kind"); return; }
    if (!own) {                           /* bound from outside the run: seen at every limit, and no fail undoes it */
      if (nout == OMOST) { fault("full"); return; }
      out[nout] = (Binding){ E(BINDING), W->at, x, l, 0, nout }; nout++; W->v = E(YES); return; }
    room(A_BINDINGS, (uint64_t)nbind + 1); room(A_BOUND_BY_NODE, (uint64_t)x + 1); if (N(A_BOUND_BY_NODE) <= x) N(A_BOUND_BY_NODE) = x + 1;
    bd[nbind] = (Binding){ E(BINDING), W->at, x, l, bykey[x], nbind }; bykey[x] = ++nbind; W->v = E(YES); return;
  }
  if (op == BLUEPRINT) { if (!(W->v = nd[x].bp)) W->reason = txt("root"); return; }
  if (op == COUNT) { W->v = mki(nd[x].n); return; }
  if (op == LEAF) { W->v = nd[x].comp ? E(NO) : E(YES); return; }
  if (op == CHILD || op == BYTE) {
    if (nd[x].comp != (op == CHILD) || !geti(l, &k)) { fault("kind"); return; }
    if (k < 0 || (uint64_t)k >= nd[x].n) { W->v = 0; W->reason = txt("range"); return; }
    W->v = op == CHILD ? KID(x, k) : mki(bytes[nd[x].off + k]); return;
  }
  if (!nd[l].comp || nd[l].bp != E(LIST)) { fault("kind"); return; }
  uint32_t n = nd[l].n;
  if (op == JOIN) W->v = mk(1, x, kids + nd[l].off, n, &why);
  else {
    room(A_GATHERING_BYTES, (uint64_t)ntb + n);
    for (uint32_t i = 0; i < n; i++) {
      if (!geti(KID(l, i), &k)) { fault("kind"); return; }
      if (k < 0 || k > 255) { W->v = 0; W->reason = txt("range"); return; }
      tb[ntb + i] = (uint8_t)k;
    }
    W->v = mk(0, x, tb + ntb, n, &why);
  }
  if (!W->v) W->reason = txt(why);
}

static void run(H top) {                  /* ends with mode M_DONE; outcome is 'n' node, '0' nothing, 'f' fault */
  const char *why; H a[8];
  W->x = top; W->v = W->reason = 0; W->env = W->cut = W->k = 0; W->outcome = 0; W->mode = M_EVAL;
  nfr = nlink = nact = 1; nargp = ncp = ncoll = 0; act[0] = (Activation){ E(ACTIVATION), 0, 0, 0, 0, ~0u, 0, 0 }; W->one = 0;
  if (sigsetjmp(in_code, 1)) { coding = 0; fault("crash"); }      /* the processor faulted inside code: the walker goes on */
  for (;; W->step++) switch (W->mode) {
  case M_EVAL: {
    if (N(A_ASKED) && !N(A_ANSWERED)) {   /* another process asks: its node is evaluated here, between two steps of the */
      uint32_t n = N(A_ASKED), mark = ntext; H q, p;        /* run, as from outside and leaving no other trace */
      if (n > fb->a[A_ASKED].cap) n = 0;
      char *s = talloc((size_t)n + 1); memcpy(s, (uint8_t *)fb + fb->a[A_ASKED].off, n); s[n] = 0; N(A_ANSWERED) = 1;
      if (!(q = read_print(s, &why))) { p = txt(why); q = mk(1, E(FAULT), &p, 1, &why); }
      ntext = mark; q = mk(1, E(OUTSIDE), &q, 1, &why); q = mk(1, E(OUTCOME), &q, 1, &why);
      W->k = push(F_ASKED, W->x, W->one, W->env, W->cut, W->reason, W->k, 0); W->x = q; W->env = W->one = 0; W->cut = ncp;
    }
    H x = W->x; W->at = x;
    long long op = nd[x].comp ? catn(nd[x].bp) : -1; uint32_t c = nd[x].n, b = 0; int given = 0;
    uint32_t lim = W->one ? W->one - 1 : act[W->env].lim; W->one = 0;
    /* The newest binding that says what runs x: its blueprint's, or the step's, which is given
     * every node. A step or a lazy gets x itself; when its code declines, what lies beneath runs. */
    while ((nbind || nout) && (b = bound(op >= QUOTE ? nd[x].bp : 0, E(STEP), lim)) && (B(b)->entry == E(STEP) || is_lazy(B(b)->impl))) {
      a[0] = x; if ((given = apply(b, lim, a, 1, W->env, x))) break;
      lim = under(b, lim); b = 0;
    }
    if (given) break;
    if (!b && (is_data(op) || op > LAST)) { answer(x); break; }/* data, or an entry with no meaning yet */
    switch (b ? -1 : op) {
    case QUOTE: if (c != 1) fault("arity"); else answer(KID(x, 0)); break;
    case ARG: {
      long long j = argn(x);
      if (j < 0) fault("arity"); else if (j >= act[W->env].n) fault("arg"); else answer(argp[act[W->env].off + j]);
      break; }
    case SELF: if (!act[W->env].recipe) fault("no-recipe"); else answer(act[W->env].recipe); break;
    case LET:                             /* (let $k v body): body runs on the same arguments, with argument k being v */
      if (c != 3 || argn(KID(x, 0)) < 0) { fault("arity"); break; }
      W->k = push(F_LET, x, 0, W->env, W->cut, 0, W->k, 0); W->x = KID(x, 1); break;
    case SEQ:
      if (!c) { answer(E(YES)); break; }
      if (c > 1) W->k = push(F_SEQ, x, 1, W->env, W->cut, 0, W->k, 0);
      W->x = KID(x, 0); break;
    case CHOICE:
      if (!c) { W->mode = M_FAILED; break; }
      W->cut = ncp;
      if (c > 1) { room(A_CHOICE_POINTS, (uint64_t)ncp + 1);
        cp[ncp] = (ChoicePoint){ E(CHOICE_POINT), x, 0, x, 1, W->env, W->k, nfr, nlink, nact, nargp, 0, nbind, ntrail }; ncp++; }
      W->x = KID(x, 0); break;
    case FAIL:
      if (c > 1) { fault("arity"); break; }
      if (!c) { W->mode = M_FAILED; break; }
      W->k = push(F_FAIL, x, 0, W->env, W->cut, 0, W->k, 0); W->x = KID(x, 0); break;
    case CUT:
      if (c > 1) { fault("arity"); break; }
      if (!c) { if (ncp > W->cut) ncp = W->cut; answer(E(YES)); break; }
      W->k = push(F_CUT, x, 0, W->env, W->cut, 0, W->k, 0); W->x = KID(x, 0); break;
    case ALL:
      if (c != 1) { fault("arity"); break; }
      room(A_CHOICE_POINTS, (uint64_t)ncp + 1);
      cp[ncp] = (ChoicePoint){ E(CHOICE_POINT), x, 1, x, 0, W->env, W->k, nfr, nlink, nact, nargp, ++W->gathers, nbind, ntrail }; ncp++;
      W->k = push(F_ALL, x, 0, W->env, W->cut, W->k, 0, W->gathers); W->cut = ncp; W->x = KID(x, 0); break;
    case NEED: if (c != 1) { fault("arity"); break; } W->k = push(F_NEED, x, 0, W->env, W->cut, 0, W->k, 0); W->x = KID(x, 0); break;
    case RECOVER: if (c != 2) { fault("arity"); break; } W->k = push(F_RECOVER, x, 0, W->env, W->cut, ncoll, W->k, ncp); W->x = KID(x, 0); break;
    case OUTSIDE:                         /* x runs as if asked from outside the run: what it binds is seen by every */
      if (c != 1) { fault("arity"); break; }      /* layer of the program, and a fail does not take it away */
      W->env = again(W->env, 0); act[W->env].lim &= ~OWN; W->x = KID(x, 0); break;
    case LAYER: if (c != 1) { fault("arity"); break; } W->k = push(F_LAYER, x, 0, W->env, W->cut, nout, W->k, nbind); W->x = KID(x, 0); break;
    case OUTCOME:                         /* x's first outcome is seen from behind a choice point, then failed into: */
      if (c != 1) { fault("arity"); break; }      /* nothing x did stays, and the outcome is the answer */
      room(A_CHOICE_POINTS, (uint64_t)ncp + 1); W->reason = 0;
      cp[ncp] = (ChoicePoint){ E(CHOICE_POINT), x, 2, x, 0, W->env, W->k, nfr, nlink, nact, nargp, 0, nbind, ntrail }; ncp++;
      W->k = push(F_OUTCOME, x, ncoll, W->env, W->cut, W->k, 0, ncp - 1); W->cut = ncp; W->x = KID(x, 0); break;
    default:                              /* call, arithmetic, eq, lt, native, parts: children first, in order */
      if (c > 8) { fault("arity"); break; }
      if (!c) {
        while (b && !apply(b, lim, a, 0, W->env, 0)) b = bound(nd[x].bp, 0, lim = under(b, lim));
        if (!b) { if (op == CLOCK) answer(mki(host.clock())); else fault("arity"); }
        break; }
      W->k = push(F_ARGS, x, 1, W->env, W->cut, 0, W->k, lim); W->x = KID(x, 0); break;
    }
    break; }
  case M_RET: {
    if (!W->k) {
      if (W->v) W->reason = 0; else if (!W->reason) W->reason = txt("no-answer");
      W->outcome = W->v ? 'n' : '0'; W->mode = M_DONE; return;
    }
    uint32_t k = W->k; Frame f = fr[k]; W->at = f.node;
    if (k == nfr - 1 && (!ncp || k >= cp[ncp-1].mfr)) nfr--;       /* nothing can return to it */
    uint32_t keep = (ncp && cp[ncp-1].mact > f.mact) ? cp[ncp-1].mact : f.mact;
    if (keep < nact) { nargp = act[keep].off; nact = keep; }       /* calls made since the frame have returned */
    W->k = f.k;
    switch (f.t) {
    case F_SEQ:
      W->env = f.env; W->cut = f.cut; W->x = KID(f.node, f.i); W->mode = M_EVAL;
      if (f.i + 1 < nd[f.node].n) W->k = push(F_SEQ, f.node, f.i + 1, W->env, W->cut, 0, W->k, 0);
      break;
    case F_CUT: if (ncp > f.cut) ncp = f.cut; break;
    case F_FAIL: W->reason = W->v; W->mode = M_FAILED; break;
    case F_ALL:
      if (W->v) { room(A_GATHERED, (uint64_t)ncoll + 1); coll[ncoll] = (Gathered){ E(GATHERED), W->at, f.aux, W->v }; ncoll++; }
      W->mode = M_FAILED; break;
    case F_NEED: if (!W->v || W->v == E(NO)) W->mode = M_FAILED; break;
    case F_RECOVER: break;
    case F_ASKED: {                       /* the outcome goes back to who asked, and the run goes on where it was */
      print(W->v); uint32_t n = nob < fb->a[A_ANSWERED].cap ? nob : (uint32_t)fb->a[A_ANSWERED].cap;
      memcpy((uint8_t *)fb + fb->a[A_ANSWERED].off, ob, n); N(A_ANSWERED) = n; N(A_ASKED) = 0;
      W->env = f.env; W->cut = f.cut; W->x = f.node; W->one = f.i; W->reason = f.vals; W->mode = M_EVAL; break; }
    case F_LET: {                         /* a new activation: the one the let stands in, with one argument set */
      uint32_t j = (uint32_t)argn(KID(f.node, 0));
      W->env = again(f.env, j + 1); argp[act[W->env].off + j] = W->v;
      W->cut = f.cut; W->x = KID(f.node, 2); W->mode = M_EVAL; break; }
    case F_LAYER:                         /* made together, they see each other */
      for (uint32_t i = f.aux; i < nbind; i++) bd[i].sees = nbind;
      for (uint32_t i = f.vals; i < nout; i++) out[i].sees = nout;
      break;
    case F_OUTCOME:
      cp[f.aux].coll = W->v ? seen_as("node", W->v) : seen_as("nothing", W->reason ? W->reason : txt("no-answer"));
      ncoll = f.i; ncp = f.aux + 1; W->mode = M_FAILED; break;
    case F_HANDLER:
      if (!is_recipe(W->v)) { fault("not-a-recipe"); break; }
      a[0] = f.node; W->env = enter(W->v, a, 1, act[f.env].lim, act[f.env].up, act[f.env].cut); W->cut = ncp; W->x = KID(W->v, 0); W->mode = M_EVAL; break;
    case F_ARGS: {
      room(A_LINKS, (uint64_t)nlink + 1); lk[nlink] = (Link){ E(LINK), W->at, W->v, f.vals }; uint32_t vals = nlink++;
      uint32_t c = nd[f.node].n;
      if (f.i < c) { W->k = push(F_ARGS, f.node, f.i + 1, f.env, f.cut, vals, W->k, f.aux);
        W->env = f.env; W->cut = f.cut; W->x = KID(f.node, f.i); W->mode = M_EVAL; break; }
      for (uint32_t j = c, q = vals; j > 0; j--, q = lk[q].next) a[j-1] = lk[q].v;
      unlink_vals(vals, c);
      long long op = catn(nd[f.node].bp); int64_t m, n, r = 0;
      uint32_t b = 0, lim = f.aux;        /* code that declines leaves the values to what lies beneath it. So does a */
      W->cut = f.cut;                     /* lazy bound after this node began: it takes nodes, and here are answers */
      while ((nbind || nout) && (b = bound(op == STEP ? 0 : nd[f.node].bp, 0, lim)) && (is_lazy(B(b)->impl) || !apply(b, lim, a, c, f.env, 0))) { lim = under(b, lim); b = 0; }
      if (b) break;
      if (op == UP || op == BASE || op == STEP) {       /* a node in hand is evaluated: by step where this node */
        if (c != 1) { fault("arity"); break; }           /* stands; by the others where the implementation's user */
        if (!a[0]) { W->v = 0; W->reason = txt("nothing-in"); break; }
        W->one = op == BASE ? act[f.env].lim + 1 : 0;    /* stands, in that user's cut scope. base: the node's own */
        if (op != STEP) { W->cut = act[f.env].cut; W->env = act[f.env].up; }       /* entry means what it meant beneath */
        W->x = a[0]; W->mode = M_EVAL; break;
      }
      if (op == CALL) {
        uint32_t on = act[f.env].lim;     /* a call its own code declined runs wholly on what lies beneath that code */
        for (lim = f.aux; (nbind || nout) && (b = bound(a[0], 0, lim)) && !apply(b, lim, a + 1, c - 1, f.env, 0); b = 0) { lim = under(b, lim); on = (lim & ~OWN) | (on & OWN); }
        if (b) break;
        if (!is_recipe(a[0])) { fault("not-a-recipe"); break; }
        uint32_t up = act[f.env].up, cut = act[f.env].cut; last(up);
        W->env = enter(a[0], a + 1, c - 1, on, up, cut); W->cut = ncp; W->x = KID(a[0], 0); W->mode = M_EVAL; break;
      }
      if (op == NATIVE_OP) {
        int64_t arg[8] = {0}; uint32_t j = 1;
        if (c > 7 || !a[0] || nd[a[0]].comp || nd[a[0]].bp != E(CODE)) { fault("kind"); break; }
        while (j < c && geti(a[j], &arg[j-1])) j++;
        if (j < c) { fault("kind"); break; }
        arg[7] = c - 1;
        if (!landed(birth(a[0]), arg)) { W->v = 0; W->reason = txt("declined"); }
        break;
      }
      if (op > LAST) { fault("kind"); break; }       /* an entry only code ran, and the code declined these values */
      if (op == FAULT) {                  /* the program's own fault: its reason is whatever node it gives */
        if (c != 1) { fault("arity"); break; }
        W->reason = a[0] ? a[0] : txt("nothing-in"); W->mode = M_FAULT; break;
      }
      if (op >= BLUEPRINT && op < DIV) { parts(op, a, c, act[f.env].lim >> 31); break; }
      if (op == ENCODE || op == DECODE) {             /* the encoding's section 2, forwards and backwards */
        if (c != 1) { fault("arity"); break; }
        if (!a[0]) { W->v = 0; W->reason = txt("nothing-in"); break; }
        if (op == ENCODE) { uint32_t mark = ntext; size_t len; uint8_t *cb = cbytes(a[0], &len); W->v = mk(0, E(BYTES), cb, (uint32_t)len, &why); ntext = mark; break; }
        if (nd[a[0]].comp) { fault("kind"); break; }
        if (!(W->v = from_bytes(bytes + nd[a[0]].off, nd[a[0]].n, &why))) W->reason = txt(why);
        break;
      }
      if (op == HOST) {                   /* one raw call of what the host knows by a name: integers go as they are, */
        int64_t arg[8] = {0}; uint32_t mark = ntext;      /* any other leaf as the address of its bytes with a zero after */
        if (c < 1 || c > 7) { fault("arity"); break; }
        if (ncp) { fault("irreversible"); break; }        /* what cannot be undone is refused while a choice is open */
        for (uint32_t j = 0; j < c && W->mode != M_FAULT; j++) {
          if (!a[j] || nd[a[j]].comp) { fault("kind"); break; }
          if (j && geti(a[j], &arg[j-1])) continue;
          char *s = talloc((size_t)nd[a[j]].n + 1); memcpy(s, bytes + nd[a[j]].off, nd[a[j]].n);
          if (j) arg[j-1] = (int64_t)(intptr_t)s; else arg[7] = (int64_t)(intptr_t)host.symbol(s);
        }
        if (W->mode == M_FAULT) { ntext = mark; break; }
        if (!arg[7]) { W->v = 0; W->reason = txt("no-symbol"); ntext = mark; break; }
        void *fn = (void *)(intptr_t)arg[7]; Ret r; arg[7] = 0;
        ntext = mark;                     /* given back before the call, since a crash does not return here */
        r = called(fn, arg); W->v = mki(r.v);
        break;
      }
      if (op == LOOK) {                   /* bytes of another process's region; the cell look holds the name of the one mapped */
        int64_t o = 0, n = 0; uint32_t mark = ntext;
        if (c != 3) { fault("arity"); break; }
        if (!a[0] || !a[1] || !a[2]) { W->v = 0; W->reason = txt("nothing-in"); break; }
        if (nd[a[0]].comp || !geti(a[1], &o) || !geti(a[2], &n)) { fault("kind"); break; }
        room(A_CELLS, (uint64_t)E(LOOK) + 1); if (N(A_CELLS) <= E(LOOK)) N(A_CELLS) = E(LOOK) + 1;
        if (held[E(LOOK)] != a[0]) {
          char *s = talloc((size_t)nd[a[0]].n + 1); memcpy(s, bytes + nd[a[0]].off, nd[a[0]].n);
          void *p = host.watch(s); ntext = mark;
          if (!p || memcmp(p, FB_MAGIC, 8)) { W->v = 0; W->reason = txt("no-region"); break; }
          W->watch = (uint64_t)(uintptr_t)p; held[E(LOOK)] = a[0];
        }
        const Head *far = (const Head *)(uintptr_t)W->watch;
        if (o < 0 || n < 0 || n > (1 << 24) || (uint64_t)o + (uint64_t)n > far->bytes) { W->v = 0; W->reason = txt("range"); break; }
        uint8_t *copy = talloc((size_t)n); memcpy(copy, (const uint8_t *)far + o, (size_t)n);
        W->v = mk(0, E(BYTES), copy, (uint32_t)n, &why); ntext = mark; break;
      }
      if (op == PEEK || op == HANDLE || op == AT) {   /* the region read by a program in it: its bytes, and where a node is */
        int64_t o = 0, n = 0; uint64_t guard = fb->a[A_STACK].off;
        if (c != (op == PEEK ? 2u : 1u)) { fault("arity"); break; }
        if (!a[0] || (op == PEEK && !a[1])) { W->v = 0; W->reason = txt("nothing-in"); break; }
        if (op == HANDLE) { W->v = mki(a[0]); break; }
        if (!geti(a[0], &o) || (op == PEEK && !geti(a[1], &n))) { fault("kind"); break; }
        if (op == AT ? (o < 1 || o >= nn) : (o < 0 || n < 0 || n > (1 << 24) || (uint64_t)o + (uint64_t)n > fb->bytes
                                              || ((uint64_t)o < guard && (uint64_t)o + (uint64_t)n > guard - FB_PAGE))) { W->v = 0; W->reason = txt("range"); break; }
        if (op == AT) { W->v = (H)o; break; }
        uint32_t mark = ntext; uint8_t *copy = talloc((size_t)n); memcpy(copy, (uint8_t *)fb + o, (size_t)n);
        W->v = mk(0, E(BYTES), copy, (uint32_t)n, &why); ntext = mark; break;
      }
      if (op == BOUND) {                  /* what runs an entry or a recipe, as the top of the program sees it: the newest */
        if (c != 1) { fault("arity"); break; }      /* implementation, wherever this is asked from, or nothing */
        uint32_t r = a[0] ? bound(a[0], 0, ~0u) : 0;
        if (!(W->v = r ? B(r)->impl : 0)) W->reason = txt("unbound");
        break;
      }
      if (op == UNBIND) {                 /* the newest binding of an entry is taken out of its chain; the record */
        if (c != 1) { fault("arity"); break; }      /* that says so points back at it, so a fail restores it */
        if (!(act[f.env].lim & OWN)) {    /* from outside: the newest one made there, which no fail brings back */
          uint32_t j = nout; while (j && out[j-1].entry != a[0]) j--;
          if (j && a[0]) { out[j-1].entry = 0; while (nout && !out[nout-1].entry) nout--; }
          W->v = j && a[0] ? E(YES) : E(NO); break;
        }
        if (!a[0] || a[0] >= N(A_BOUND_BY_NODE) || !bykey[a[0]]) { W->v = E(NO); break; }
        room(A_BINDINGS, (uint64_t)nbind + 1); bd[nbind] = (Binding){ E(BINDING), W->at, a[0], 0, bykey[a[0]], nbind }; nbind++;
        bykey[a[0]] = bd[bykey[a[0]] - 1].prev; W->v = E(YES); break;
      }
      if (op == HOLD || op == HELD || op == NOTE) {   /* a cell is any node; what it holds is a node. While a choice is */
        H key = a[0];                     /* open, what it held before goes on the trail, and a fail puts it back */
        if (c != (op == HELD ? 1u : 2u)) { fault("arity"); break; }
        if (!key || (op != HELD && !a[1])) { W->v = 0; W->reason = txt("nothing-in"); break; }
        if (op == HELD) { if (!(W->v = key < N(A_CELLS) ? held[key] : 0)) W->reason = txt("empty"); break; }
        room(A_CELLS, (uint64_t)key + 1); if (N(A_CELLS) <= key) N(A_CELLS) = key + 1;
        if (op == NOTE) ;                 /* a note is what was seen of a run, not part of it: no fail undoes it */
        else if (!ncp) ntrail = 0; else { room(A_TRAIL, (uint64_t)ntrail + 1); trail[ntrail++] = (Trail){ E(TRAIL), W->at, key, held[key] }; }
        W->v = held[key] = a[1]; break;
      }
      if (c != 2) { fault("arity"); break; }
      if (!a[0] || !a[1]) { W->v = 0; W->reason = txt("nothing-in"); break; }
      if (op == EQ) { W->v = a[0] == a[1] ? E(YES) : E(NO); break; }
      if (!geti(a[0], &m) || !geti(a[1], &n)) { fault("kind"); break; }
      if (op == LT) { W->v = m < n ? E(YES) : E(NO); break; }
      if (op == RANGE) {                  /* each integer from a up to b: a choice that counts instead of holding options */
        if (m >= n) { W->mode = M_FAILED; break; }
        if (m + 1 < n) { room(A_CHOICE_POINTS, (uint64_t)ncp + 1);
          cp[ncp] = (ChoicePoint){ E(CHOICE_POINT), f.node, 3, f.node, mki(m + 1), f.env, W->k, nfr, nlink, nact, nargp, a[1], nbind, ntrail }; ncp++; }
        W->v = a[0]; break;
      }
      if (op == DIV || op == MOD) {       /* the quotient rounds toward zero; the remainder has the sign of a */
        if (!n) { W->v = 0; W->reason = txt("zero"); break; }
        if (m == INT64_MIN && n == -1) { W->v = 0; W->reason = txt("range"); break; }
        W->v = mki(op == DIV ? m / n : m % n); break;
      }
      if (op == ADD ? __builtin_add_overflow(m, n, &r) : op == SUB ? __builtin_sub_overflow(m, n, &r)
                    : __builtin_mul_overflow(m, n, &r)) { W->v = 0; W->reason = txt("range"); break; }
      W->v = mki(r); break; }
    }
    break; }
  case M_FAILED: {
    if (!ncp) { if (!W->reason) W->reason = txt("no-path"); W->v = 0; W->outcome = '0'; W->mode = M_DONE; return; }
    ChoicePoint *q = &cp[ncp-1]; W->at = q->node;
    nfr = q->mfr; nlink = q->mlink; nact = q->mact; nargp = q->marg; W->env = q->env; W->k = q->k;
    while (nbind > q->mbind) { nbind--; bykey[bd[nbind].entry] = bd[nbind].prev; }     /* bindings made since are undone */
    while (ntrail > q->mtrail) { ntrail--; held[trail[ntrail].key] = trail[ntrail].old; } /* and cells hold what they held */
    if (q->all == 3) {                    /* a range: its next integer, and the choice closes with the last */
      int64_t m = 0, n = 0; geti((H)q->i, &m); geti((H)q->coll, &n);
      W->v = (H)q->i; if (m + 1 < n) q->i = mki(m + 1); else ncp--;
      W->mode = M_RET; break;
    }
    if (q->all == 2) {                    /* an outcome: what was seen, or that no path was left */
      ncp--; W->v = q->coll ? q->coll : seen_as("nothing", W->reason ? W->reason : txt("no-path"));
      W->reason = 0; W->mode = M_RET; break;
    }
    if (q->all) {                         /* every answer has been gathered */
      uint32_t base = ntk, w = 0, id = q->coll;
      for (uint32_t i = 0; i < ncoll; i++) {
        if (coll[i].id == id) { room(A_GATHERING_CHILDREN, (uint64_t)ntk + 1); tk[ntk++] = coll[i].v; } else coll[w++] = coll[i]; }
      ncoll = w; ncp--;
      W->v = mk(1, E(LIST), tk + base, ntk - base, &why); ntk = base; W->mode = M_RET; break;
    }
    W->x = KID(q->node, q->i); W->cut = ncp - 1; W->mode = M_EVAL;
    if (q->i + 1 >= nd[q->node].n) ncp--; else { q->i++; q->by = W->at; }
    break; }
  case M_FAULT: {
    uint32_t j = W->k;                    /* an all's frame keeps the way out of it in vals; its own k is nothing */
    while (j && fr[j].t != F_RECOVER && fr[j].t != F_OUTCOME) j = fr[j].t == F_ALL ? fr[j].vals : fr[j].k;
    if (!j) { W->v = 0; W->outcome = 'f'; W->mode = M_DONE; return; }
    Frame f = fr[j];
    if (f.t == F_OUTCOME) { cp[f.aux].coll = seen_as("fault", W->reason); ncoll = f.i; ncp = f.aux + 1; W->mode = M_FAILED; break; }
    if (ncp > f.aux) ncp = f.aux;
    ncoll = f.vals;                       /* what the alls inside it had gathered goes with them */
    W->k = push(F_HANDLER, W->reason, 0, f.env, f.cut, 0, f.k, 0);
    W->env = f.env; W->cut = f.cut; W->x = KID(f.node, 1); W->mode = M_EVAL; break; }
  default: return;
  }
}

/* ---- commands: a program is run, and everything else is a program -------------------------- */
static H file_node(const char *path) {      /* a print file: one node on its first line */
  size_t len; const char *why;
  H x = read_print((char *)load(path, &len), &why);
  if (!x) { say("refused ", why); exit(1); }
  return x;
}
static int command(int argc, char **argv) {
  const char *cmd = argc > 1 ? argv[1] : "";
  store_init();
  if (argc >= 3 && (!strcmp(cmd, "run") || !strcmp(cmd, "text"))) {
    for (int i = 3; i < argc; i++) { run(file_node(argv[i])); if (W->outcome != 'n') { say("not bound: ", argv[i]); return 1; } }
    run(file_node(argv[2]));
    if (cmd[0] == 't' && W->outcome == 'n' && !nd[W->v].comp) {      /* a leaf answer leaves as its own bytes */
      for (uint32_t at = 0, n = nd[W->v].n; at < n; ) { ssize_t w = write(1, bytes + nd[W->v].off + at, n - at); if (w <= 0) return 2; at += (uint32_t)w; }
      return 0;
    }
    if (W->outcome == 'n') say("node ", print(W->v)); else say(W->outcome == '0' ? "nothing " : "fault ", print(W->reason));
    return W->outcome == 'f' || cmd[0] == 't';
  }
  put(2, "usage: ck run F [B..] | text F [B..]\n");
  return 2;
}
/* The process starts on the host's stack only long enough to move onto the region's. */
typedef struct { int argc; char **argv; int rc; } Start;
static void *on_region(void *p) {
  Start *s = p; host.guard(crashed, talloc(1u << 17), 1u << 17);
  s->rc = command(s->argc, s->argv); W->mode = M_DONE; return NULL;
}
int main(int argc, char **argv) {
  Start s = { argc, argv, 2 }; pthread_t t; pthread_attr_t at;
  fb_open();
  pthread_attr_init(&at);
  if (pthread_attr_setstack(&at, (uint8_t *)fb + fb->a[A_STACK].off, fb->a[A_STACK].cap)
      || pthread_create(&t, &at, on_region, &s) || pthread_join(t, NULL)) die("cannot run on the framebuffer's stack");
  return s.rc;
}
