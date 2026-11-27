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
 * Everything this program reads and writes lives in one shared region laid out by fb.h.
 * CK_FB publishes it by name; an anonymous run's pages go back when its process ends.
 * Host symbols used: shm_open shm_unlink ftruncate fstat mmap mprotect open read write close
 * getenv getpid exit _exit clock_gettime dlsym sigaction sigaltstack sigemptyset sigsetjmp
 * siglongjmp snprintf memcpy memmove memcmp memset strlen strcmp sysconf
 * pthread_attr_init pthread_attr_setstack pthread_create pthread_join, and on Apple silicon
 * pthread_jit_write_protect_np and sys_icache_invalidate; the memory share uses getrlimit and,
 * on macOS, sysctlbyname and Mach VM statistics.
 */
#if !defined(__APPLE__)
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#ifdef __APPLE__
#include <mach/mach.h>
#include <dispatch/dispatch.h>
#include <sys/sysctl.h>
#endif
#include <sys/resource.h>
#ifdef __linux__
#include <sys/statvfs.h>
#endif
#include <sys/stat.h>
#include <time.h>
#include <dlfcn.h>
#include <signal.h>
#include <setjmp.h>
#include "fb.h"
/* Code leaves name the Seed calling convention and the processor that implements it. */
#if defined(__aarch64__) && defined(__APPLE__)
#define HOST_ABI TARGET_AAPCS64
#elif defined(__x86_64__) && defined(__LP64__) && (defined(__APPLE__) || defined(__linux__))
#define HOST_ABI TARGET_AMD64_SYSV
#else
#define HOST_ABI 0
#endif
#ifdef __APPLE__
#include <libkern/OSCacheControl.h>
#endif

static int send_fd(int fd, const uint8_t *p, uint64_t n) {   /* a write takes at most 2^31 - 1 bytes */
  while (n) { ssize_t w = write(fd, p, n < (1u << 30) ? (size_t)n : (1u << 30)); if (w < 0 && errno == EINTR) continue;
    if (w <= 0) return 0;
    p += w; n -= (uint64_t)w; }
  return 1;
}
static void put(int fd, const char *s) { if (!send_fd(fd, (const uint8_t *)s, strlen(s))) exit(2); }
static void die(const char *m) { put(2, "ck: "); put(2, m); put(2, "\n"); exit(2); }
#ifndef NUMERIC_PORT
static void say(const char *a, const char *b) { put(1, a); put(1, b); put(1, "\n"); }

#endif

/* ---- the host: every capability the floor takes from it, one function each ------- */
typedef struct { int64_t v, k; } Ret;              /* what code answers: v, and k saying what v is */
typedef struct {
  void *(*region)(const char *name, uint64_t n);   /* the one shared region, zeroed */
  void *(*code_map)(size_t);                       /* reserve the one executable region */
  int (*code_publish)(void *, const void *, size_t); /* copy and expose one completed function */
  int64_t (*file_read)(const char *, uint8_t *to, uint64_t cap, uint64_t *written);
  Ret (*call)(void *fn, const int64_t *a);
  int64_t (*clock)(void);                          /* nanoseconds that only move forward */
  void *(*watch)(const char *name);                /* another process's region, mapped to be read only */
  void *(*symbol)(const char *name);               /* where the host keeps what it calls by that name */
  void (*guard)(void (*on)(int), void *stack, size_t n);   /* a fault of the processor is handed to on, on its own stack */
  uint64_t (*memory)(void);                        /* available bytes on this host now */
  uint64_t (*line)(void);                          /* how many bytes a line of its data cache holds; 0 when it does not say */
} Host;
static int64_t h_clock(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return (int64_t)t.tv_sec * 1000000000 + t.tv_nsec; }
static void *h_region(const char *name, uint64_t n) {
  if (!name) {
    int flags = MAP_SHARED | MAP_ANON;
#ifdef __linux__
    flags |= MAP_NORESERVE;  /* the tables reserve addresses; room() admits their touched pages */
#endif
    void *p = mmap(NULL, n, PROT_READ | PROT_WRITE, flags, -1, 0);
    return p == MAP_FAILED ? NULL : p;
  }
  shm_unlink(name);
  int fd = shm_open(name, O_CREAT | O_EXCL | O_RDWR, 0600);
  if (fd < 0) return NULL;
  void *p = ftruncate(fd, (off_t)n) ? MAP_FAILED : mmap(NULL, n, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  close(fd);
  if (p == MAP_FAILED) { shm_unlink(name); return NULL; }
  return p;
}
static void *h_code_map(size_t n) {
#ifdef __APPLE__
  int flags = MAP_PRIVATE | MAP_ANON | MAP_JIT, protection = PROT_READ | PROT_WRITE | PROT_EXEC;
#else
  int flags = MAP_PRIVATE | MAP_ANON, protection = PROT_READ | PROT_EXEC;
#endif
  void *p = mmap(NULL, n, protection, flags, -1, 0);
  return p == MAP_FAILED ? NULL : p;
}
static int h_code_publish(void *p, const void *source, size_t n) {
#ifdef __APPLE__
#if defined(__aarch64__)
  pthread_jit_write_protect_np(0);
#endif
  memcpy(p, source, n);
#if defined(__aarch64__)
  pthread_jit_write_protect_np(1);
#endif
  sys_icache_invalidate(p, n);
#else
  long page = sysconf(_SC_PAGESIZE);
  if (page <= 0) return 0;
  uintptr_t start = (uintptr_t)p / page * page;
  size_t span = (uintptr_t)p - start + n;
  if (mprotect((void *)start, span, PROT_READ | PROT_WRITE)) return 0;
  memcpy(p, source, n);
  if (mprotect((void *)start, span, PROT_READ | PROT_EXEC)) die("cannot seal executable pages");
  __builtin___clear_cache((char *)p, (char *)p + n);
#endif
  return 1;
}
static Ret h_call(void *fn, const int64_t *a) {
  return ((Ret (*)(int64_t, int64_t, int64_t, int64_t, int64_t, int64_t, int64_t, int64_t))fn)(a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7]);
}
static void *h_watch(const char *name) {
  static void *previous; static size_t previous_size;
  int fd = shm_open(name, O_RDONLY, 0); struct stat st;
  if (fd < 0) return NULL;
  void *p = fstat(fd, &st) || st.st_size < (off_t)FB_PAGE ? MAP_FAILED : mmap(NULL, (size_t)st.st_size, PROT_READ, MAP_SHARED, fd, 0);
  close(fd);
  if (p == MAP_FAILED) return NULL;
  if (memcmp(p, FB_MAGIC, 8) || ((const Head *)p)->bytes != (uint64_t)st.st_size) { munmap(p, (size_t)st.st_size); return NULL; }
  if (previous) munmap(previous, previous_size);
  previous = p; previous_size = (size_t)st.st_size; return p;
}
static void *h_symbol(const char *name) { return dlsym(RTLD_DEFAULT, name); }
static void h_guard(void (*on)(int), void *stack, size_t n) {
  stack_t ss; struct sigaction sa; memset(&sa, 0, sizeof sa); memset(&ss, 0, sizeof ss);
  ss.ss_sp = stack; ss.ss_size = n; sa.sa_handler = on; sa.sa_flags = SA_ONSTACK; sigemptyset(&sa.sa_mask);
  sigaltstack(&ss, NULL); sigaction(SIGSEGV, &sa, NULL); sigaction(SIGBUS, &sa, NULL); sigaction(SIGILL, &sa, NULL); sigaction(SIGFPE, &sa, NULL);
}
static uint64_t h_memory(void) {
  long size = sysconf(_SC_PAGESIZE);
  if (size <= 0) return 0;
  uint64_t available;
#ifdef __APPLE__
  int pressure = 0; size_t len = sizeof pressure;
  if (sysctlbyname("kern.memorystatus_vm_pressure_level", &pressure, &len, NULL, 0)) pressure = 0;
  if (pressure & DISPATCH_MEMORYPRESSURE_CRITICAL) return 0;
  vm_statistics64_data_t vm; mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
  mach_port_t port = mach_host_self();
  kern_return_t rc = host_statistics64(port, HOST_VM_INFO64, (host_info64_t)&vm, &count);
  mach_port_deallocate(mach_task_self(), port);
  if (rc != KERN_SUCCESS) return 0;
  uint64_t pages = vm.free_count;
  if (!(pressure & DISPATCH_MEMORYPRESSURE_WARN)) pages += vm.inactive_count;
  available = pages * (uint64_t)size;
#else
  long pages = sysconf(_SC_AVPHYS_PAGES);
  available = pages > 0 ? (uint64_t)pages * (uint64_t)size : 0;
#endif
  long cpus = sysconf(_SC_NPROCESSORS_ONLN);
  available /= 1 + (uint64_t)(cpus > 0 ? cpus : 1);
#ifdef __linux__
  struct statvfs fs;
  if (getenv("CK_FB") && !statvfs("/dev/shm", &fs)) {
    uint64_t backing = (uint64_t)fs.f_bavail * fs.f_frsize / 2;
    if (backing < available) available = backing;
  }
#endif
  return available;
}
#define GROW_CHUNK (1u << 20)     /* resample host headroom as new memory is touched */
static int64_t h_file_read(const char *path, uint8_t *to, uint64_t cap, uint64_t *written) {
  int fd = open(path, O_RDONLY); uint64_t n = 0, admitted = 0; uint8_t extra;
  if (fd < 0) return -1;
  struct stat st;
  if (!fstat(fd, &st) && S_ISREG(st.st_mode) && st.st_size > 0 && (uint64_t)st.st_size > cap) {
    off_t offset = lseek(fd, 0, SEEK_CUR);
    if (offset >= 0 && offset < st.st_size && (uint64_t)(st.st_size - offset) > cap) { close(fd); return -2; }
  }
  int64_t result;
  for (;;) {
    if (n == admitted && n < cap) {
      uint64_t more = cap - n < GROW_CHUNK ? cap - n : GROW_CHUNK;
      if (!n || more <= h_memory()) admitted += more;
    }
    uint64_t chunk = admitted - n;
    ssize_t r = read(fd, chunk ? to + n : &extra, chunk ? (size_t)chunk : 1);
    if (r < 0 && errno == EINTR) continue;
    if (r <= 0 || !chunk) { result = r < 0 ? -1 : r == 0 ? (int64_t)n : -2; break; }
    n += (uint64_t)r;
  }
  *written = n; close(fd); return result;
}
static uint64_t h_line(void) {
#ifdef __APPLE__
  uint64_t l = 0; size_t len = sizeof l;
  if (!sysctlbyname("hw.cachelinesize", &l, &len, NULL, 0) && l) return l;
#endif
#ifdef _SC_LEVEL1_DCACHE_LINESIZE
  long l1 = sysconf(_SC_LEVEL1_DCACHE_LINESIZE); if (l1 > 0) return (uint64_t)l1;
#endif
  return 0;
}
static Host host = { h_region, h_code_map, h_code_publish, h_file_read, h_call, h_clock, h_watch, h_symbol, h_guard, h_memory, h_line };

/* ---- the framebuffer: the one region, and where each table lies in it (fb.h) --------
 * These pointers are set once and never move. Every count is a field of the region. */
static Head *fb;
static Node *nd; static uint8_t *bytes, *tb, *text; static char *ob;
static H *kids, *tab, *idtab, *argp, *opened, *tk, *aft, *noted, *hand; static uint32_t *pnum, *bykey; static H *held;
static Frame *fr; static Link *lk; static Activation *act; static ChoicePoint *cp; static Gathered *coll; static Code *born; static Binding *bd, *out; static Trail *trail; static uint8_t *rooms; static RoomTrail *rtrail; static H *gives; static uint8_t *ids;
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
#define nafter N(A_AFTER)
#define nnoted N(A_NOTED)
#define ntrail N(A_TRAIL)
#define nrtrail N(A_ROOM_TRAIL)
#define nopen N(A_READER_OPENED)
#define ntk N(A_GATHERING_CHILDREN)
#define ntb N(A_GATHERING_BYTES)
#define nob N(A_PRINT)
#define ntext N(A_TEXT)
#define nids N(A_IDS)
#define nhand N(A_VALUES)
#define IDREC 36                          /* a node's id: whose it is, and its 32 bytes */
#define NID(h) (ids + (uint64_t)nd[h].ida * IDREC + 4)
static const char *const AREA_NAME[A_COUNT] = { FB_AREA_NAMES };
/* An area's peak is the most it has held, and room goes further only past it. The areas together
 * take what the host can give: each time they have reached a further step of the host's, it is
 * asked how much it can spare, and a growth it has no room for, or one past an area's count, is
 * full. While a program runs that is its fault, full and the area's name, which it can
 * catch; the walker goes on from its loop. Before a run, or while a fault is being handled, it ends
 * the process. */
static sigjmp_buf in_code; static volatile sig_atomic_t coding, walking; static int full; static H full_at[A_COUNT];
static void area_full(int a) {
  if (walking && W->mode != M_FAULT) { full = a; siglongjmp(in_code, 2); }
  put(2, "ck: the framebuffer has no room left for "); put(2, AREA_NAME[a]); put(2, "\n"); exit(2);
}
static int host_has_growth(uint64_t old, uint64_t next) {
  if (old / GROW_CHUNK == next / GROW_CHUNK) return 1;
  uint64_t growth = next - old;
  return (growth < GROW_CHUNK ? GROW_CHUNK : growth) <= host.memory();
}
static uint64_t pages_for(uint64_t bytes_needed) { return (bytes_needed + FB_PAGE - 1) & ~(uint64_t)(FB_PAGE - 1); }
static uint64_t area_extent(const Area *a, uint64_t count) {
  return count ? a->off % FB_PAGE + count * a->size : 0;
}
/* Touched pages can remain resident after backtracking. The peaks already in the region
 * account for them. Charge the whole native stack up front since it grows without room(). */
static uint64_t remaining_memory(void) {
  uint64_t used = FB_PAGE + fb->a[A_STACK].cap + pages_for(W->ncode);
  for (int i = 0; i < A_COUNT; i++) used += pages_for(area_extent(&fb->a[i], fb->a[i].peak));
  return used < fb->a[A_ROOMS].cap ? fb->a[A_ROOMS].cap - used : 0;
}
static int fits_growth(uint64_t old, uint64_t next) {
  uint64_t extra = pages_for(next) - pages_for(old);
  return !extra || (extra <= remaining_memory() && host_has_growth(old, next));
}
static void further(int a, uint64_t need) {
  Area *area = &fb->a[a];
  if (need > area->cap || !fits_growth(area_extent(area, area->peak), area_extent(area, need))) area_full(a);
  area->peak = need;
}
static inline void room(int a, uint64_t need) { if (need > fb->a[a].peak) further(a, need); }
/* The limit word an activation runs under counts bindings: 23 bits for the program's own, 7 for
 * those made from outside (see bound, below). */
#define PM 0x7fffffu
#define COOL 0x800000u
#define OUT 0x1000000u
#define OMOST 127u
#define OWN 0x80000000u
static void fb_open(void) {
  /* Each table has stable addresses into which it can grow. Their pages share one allowance. */
  static const struct { uint32_t of, size; uint8_t wide; } L[A_COUNT] = {
    [A_NODES] = { NODE, sizeof(Node), 0 },          [A_LEAF_BYTES] = { BYTE_TABLE, 1, 1 },
    [A_CHILDREN] = { WORD_TABLE, 4, 1 },            [A_BY_CONTENT] = { WORD_TABLE, 4, 1 },
    [A_BY_ID] = { WORD_TABLE, 4, 1 },               [A_FRAMES] = { FRAME, sizeof(Frame), 0 },
    [A_LINKS] = { LINK, sizeof(Link), 0 },          [A_ACTIVATIONS] = { ACTIVATION, sizeof(Activation), 0 },
    [A_ARGUMENTS] = { WORD_TABLE, 4, 0 },           [A_CHOICE_POINTS] = { CHOICE_POINT, sizeof(ChoicePoint), 0 },
    [A_GATHERED] = { GATHERED, sizeof(Gathered), 0 }, [A_BORN_CODE] = { CODE, sizeof(Code), 0 },
    [A_BINDINGS] = { BINDING, sizeof(Binding), 0 }, [A_BOUND_BY_NODE] = { WORD_TABLE, 4, 0 },
    [A_OUTSIDE] = { BINDING, sizeof(Binding), 0 },  [A_CELLS] = { WORD_TABLE, 4, 0 },
    [A_TRAIL] = { TRAIL, sizeof(Trail), 0 },        [A_READER_OPENED] = { WORD_TABLE, 4, 0 },
    [A_GATHERING_CHILDREN] = { WORD_TABLE, 4, 0 },  [A_GATHERING_BYTES] = { BYTE_TABLE, 1, 1 },
    [A_PRINT] = { BYTE_TABLE, 1, 1 },               [A_PRINT_NUMBERS] = { WORD_TABLE, 4, 0 },
    [A_TEXT] = { BYTE_TABLE, 1, 1 },                [A_STACK] = { BYTE_TABLE, 1, 1 },
    [A_ASKED] = { BYTE_TABLE, 1, 1 },               [A_ANSWERED] = { BYTE_TABLE, 1, 1 },
    [A_AFTER] = { WORD_TABLE, 4, 0 },               [A_NOTED] = { WORD_TABLE, 4, 0 },
    [A_ROOMS] = { BYTE_TABLE, 1, 1 },               [A_ROOM_TRAIL] = { ROOM_TRAIL, sizeof(RoomTrail), 0 },
    [A_GIVES] = { WORD_TABLE, 4, 0 },               [A_IDS] = { BYTE_TABLE, 1, 1 },
    [A_VALUES] = { WORD_TABLE, 4, 0 } };
  uint64_t budget = host.memory(), line = host.line();
  struct rlimit limit;
  if (!getrlimit(RLIMIT_AS, &limit) && limit.rlim_cur / (A_COUNT + 3) < budget) budget = limit.rlim_cur / (A_COUNT + 3);
  const char *setting = getenv("CK_MEMORY_BYTES");
  if (setting) {
    char *end; errno = 0; unsigned long long requested = strtoull(setting, &end, 10);
    if (*setting < '0' || *setting > '9' || *end || errno || !requested) die("CK_MEMORY_BYTES takes a positive byte count");
    if (requested < budget) budget = requested;
  }
  uint64_t cap[A_COUNT], off[A_COUNT], at = FB_PAGE;
  uint64_t mem = budget / FB_PAGE * FB_PAGE;
  if (mem < (1u << 17) + 4 * FB_PAGE) die("insufficient host memory for the framebuffer");
  for (int i = 0; i < A_COUNT; i++) { cap[i] = mem / L[i].size; if (!L[i].wide && cap[i] > 0xffffffffu) cap[i] = 0xffffffffu; }
  if (cap[A_BY_ID] > (uint64_t)1 << 33) cap[A_BY_CONTENT] = cap[A_BY_ID] = (uint64_t)1 << 33;   /* twice the most nodes a handle names */
  cap[A_BOUND_BY_NODE] = cap[A_CELLS] = cap[A_PRINT_NUMBERS] = cap[A_NODES] + 8;   /* one for each node */
  cap[A_BINDINGS] = cap[A_BINDINGS] < PM ? cap[A_BINDINGS] : PM;   /* numbered inside a limit word: */
  cap[A_OUTSIDE] = OMOST;                                       /* 23 bits for the program's, 7 from outside */
  cap[A_GIVES] = 4;                                             /* a record, not a table */
  pthread_attr_t attr; size_t stack;
  if (pthread_attr_init(&attr) || pthread_attr_getstacksize(&attr, &stack)) die("cannot size the host stack");
  pthread_attr_destroy(&attr);
  if (!getrlimit(RLIMIT_STACK, &limit) && limit.rlim_cur != RLIM_INFINITY && limit.rlim_cur > stack) stack = limit.rlim_cur;
  cap[A_STACK] = pages_for(stack) < mem / 2 ? pages_for(stack) : mem / 2 / FB_PAGE * FB_PAGE;
  if (cap[A_STACK] < (uint64_t)PTHREAD_STACK_MIN) die("insufficient host memory for the stack");
  for (int i = 0; i < A_COUNT; i++) {
    if (i == A_STACK) at += FB_PAGE;                      /* a page nothing may touch, under the stack */
    uint64_t hue = i == A_STACK || line * A_COUNT > FB_PAGE ? 0 : line * (uint64_t)i;   /* each area's first records on a */
    off[i] = at + hue; at += (L[i].size * cap[i] + hue + FB_PAGE - 1) / FB_PAGE * FB_PAGE;  /* cache set no other's first use */
  }
  if (!(fb = host.region(getenv("CK_FB"), at))) die("cannot make the framebuffer");
  for (int i = 0; i < A_COUNT; i++) fb->a[i] = (Area){ E(AREA), 0, E(L[i].of), L[i].size, 0, off[i], cap[i], 0 };
  fb->bp = E(FRAMEBUFFER); fb->version = 19; fb->floor = (uint32_t)(off[A_STACK] / FB_PAGE) + 1; fb->areas = A_COUNT; fb->pid = (uint32_t)getpid(); fb->bytes = at; W->bp = E(MACHINE);
  mprotect((uint8_t *)fb + off[A_STACK] - FB_PAGE, FB_PAGE, PROT_NONE);
#define PLACE(i) ((void *)((uint8_t *)fb + off[i]))
  nd = PLACE(A_NODES); bytes = PLACE(A_LEAF_BYTES); kids = PLACE(A_CHILDREN); tab = PLACE(A_BY_CONTENT); idtab = PLACE(A_BY_ID); fr = PLACE(A_FRAMES);
  lk = PLACE(A_LINKS); act = PLACE(A_ACTIVATIONS); argp = PLACE(A_ARGUMENTS); cp = PLACE(A_CHOICE_POINTS); coll = PLACE(A_GATHERED); born = PLACE(A_BORN_CODE); bd = PLACE(A_BINDINGS); out = PLACE(A_OUTSIDE); aft = PLACE(A_AFTER); noted = PLACE(A_NOTED); bykey = PLACE(A_BOUND_BY_NODE); held = PLACE(A_CELLS); trail = PLACE(A_TRAIL); rooms = PLACE(A_ROOMS); rtrail = PLACE(A_ROOM_TRAIL);
  opened = PLACE(A_READER_OPENED); tk = PLACE(A_GATHERING_CHILDREN); tb = PLACE(A_GATHERING_BYTES); ob = PLACE(A_PRINT); pnum = PLACE(A_PRINT_NUMBERS); text = PLACE(A_TEXT); gives = PLACE(A_GIVES); ids = PLACE(A_IDS); hand = PLACE(A_VALUES);
  nn = 1; nids = IDREC;                   /* the first id lies after a record of none */
}
/* Working bytes, taken from the top of the text area and given back by resetting ntext. */
static void *talloc(size_t n) {
  room(A_TEXT, (uint64_t)ntext + n + 16); void *p = text + ntext;
  memset(p, 0, n); ntext += (n + 7) & ~(size_t)7; return p;
}
/* What no fail undoes, done while a choice is open, goes on the trail as kept: a note and its cell,
 * a faster form, code that was born. A fail back past it keeps what it holds (reclaim, below). */
static void kept(H kind, uint32_t key, H v) {
  if (!ncp) return;
  room(A_TRAIL, (uint64_t)ntrail + 1); trail[ntrail++] = (Trail){ kind, W->at, key, v };
}
static uint64_t text_limit(void) {
  const Area *text_area = &fb->a[A_TEXT];
  uint64_t extent = pages_for(area_extent(text_area, text_area->peak)) + remaining_memory();
  uint64_t skip = text_area->off % FB_PAGE, limit = extent > skip ? extent - skip : 0;
  return limit < fb->a[A_TEXT].cap ? limit : fb->a[A_TEXT].cap;
}
static int64_t read_text(const char *path) {
  room(A_TEXT, ntext + 16);
  uint64_t written = 0;
  int64_t result = host.file_read(path, text + ntext, text_limit() - ntext - 16, &written);
  if (ntext + written > fb->a[A_TEXT].peak) fb->a[A_TEXT].peak = ntext + written;
  return result;
}
static uint8_t *load(const char *path, size_t *len) {      /* a file read straight into the region, a zero after it */
  uint8_t *b = text + ntext;
  int64_t n = read_text(path);
  if (n == -2) area_full(A_TEXT);
  if (n < 0) { put(2, "ck: "); put(2, path); put(2, ": cannot read\n"); exit(2); }
  b[n] = 0; room(A_TEXT, ntext + (uint64_t)n + 1); ntext += (uint64_t)(n + 8) & ~7ull; *len = (size_t)n; return b;
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
#define STATUS_ROWS(X) \
  X(S_OK, 0, "") \
  X(S_ARG, 1, "arg") \
  X(S_ARITY, 2, "arity") \
  X(S_CHILD_LENGTH, 3, "child-length") \
  X(S_COMPOSITE_ONLY, 4, "composite-only") \
  X(S_CRASH, 5, "crash") \
  X(S_DECLINED, 6, "declined") \
  X(S_EMPTY, 7, "empty") \
  X(S_FAULT, 8, "fault") \
  X(S_FORWARD, 9, "forward") \
  X(S_FULL, 10, "full") \
  X(S_ID, 11, "id") \
  X(S_IMAGE, 12, "image") \
  X(S_IRREVERSIBLE, 13, "irreversible") \
  X(S_KEPT, 14, "kept") \
  X(S_KIND, 15, "kind") \
  X(S_LEAF_ONLY, 16, "leaf-only") \
  X(S_MINIMAL, 17, "minimal") \
  X(S_NAN, 18, "nan") \
  X(S_NO_ANSWER, 19, "no-answer") \
  X(S_NO_FILE, 20, "no-file") \
  X(S_NO_NATIVE, 21, "no-native") \
  X(S_NO_NODE, 22, "no-node") \
  X(S_NO_PATH, 23, "no-path") \
  X(S_NO_RECIPE, 24, "no-recipe") \
  X(S_NO_REGION, 25, "no-region") \
  X(S_NO_SYMBOL, 26, "no-symbol") \
  X(S_NODE, 27, "node") \
  X(S_NOT_A_RECIPE, 28, "not-a-recipe") \
  X(S_NOT_CANONICAL, 29, "not-canonical") \
  X(S_NOTHING, 30, "nothing") \
  X(S_NOTHING_BLUEPRINT, 31, "nothing-blueprint") \
  X(S_NOTHING_CHILD, 32, "nothing-child") \
  X(S_NOTHING_IN, 33, "nothing-in") \
  X(S_ORDINAL, 34, "ordinal") \
  X(S_RANGE, 35, "range") \
  X(S_ROOT, 36, "root") \
  X(S_SHORT, 37, "short") \
  X(S_SYNTAX, 38, "syntax") \
  X(S_TRAILING, 39, "trailing") \
  X(S_UNBOUND, 40, "unbound") \
  X(S_UNKNOWN, 41, "unknown") \
  X(S_UTF8, 42, "utf8") \
  X(S_WIDTH, 43, "width") \
  X(S_ZERO, 44, "zero")
#define STATUS_VALUE(key, value, label) key = value,
typedef enum { STATUS_ROWS(STATUS_VALUE) } Status;
#undef STATUS_VALUE
#ifdef NUMERIC_PORT
#define numeric_io 1
#else
static int numeric_io;
#endif
static H notice(Status status);
static Status rule(int comp, H bp, const void *payload, uint64_t n) {
  const uint8_t *b = payload;
  if (!bp) return S_NOTHING_BLUEPRINT;
  if (comp) for (uint64_t i = 0; i < n; i++) if (!((const H *)payload)[i]) return S_NOTHING_CHILD;
  long long c = catn(bp);
  if (c < 0 || c >= 32) return S_OK;
  if (c == 15 || c >= 17) return S_KEPT;
  if (c == 16) return comp ? S_OK : S_COMPOSITE_ONLY;
  if (comp) return S_LEAF_ONLY;
  switch (c) {
  case 0: return (n >= 1 && n <= 8 && b[n-1] != 0 && !(n == 8 && b[7] & 0x80)) ? S_OK : S_ORDINAL;
  case 2: return utf8_ok(b, n) ? S_OK : S_UTF8;
  case 3: return (n == 0 || b[n-1] != 0) ? S_OK : S_MINIMAL;
  case 4:
    if (n == 0) return S_OK;
    if (b[n-1] == 0x00 && (n == 1 || !(b[n-2] & 0x80))) return S_MINIMAL;
    if (b[n-1] == 0xff && n >= 2 && (b[n-2] & 0x80)) return S_MINIMAL;
    return S_OK;
  case 5: case 9:  return n == 1 ? S_OK : S_WIDTH;
  case 6: case 10: return n == 2 ? S_OK : S_WIDTH;
  case 7: case 11: return n == 4 ? S_OK : S_WIDTH;
  case 8: case 12: return n == 8 ? S_OK : S_WIDTH;
  case 13: {
    if (n != 4) return S_WIDTH;
    uint32_t u = (uint32_t)b[0] | (uint32_t)b[1] << 8 | (uint32_t)b[2] << 16 | (uint32_t)b[3] << 24;
    return ((u & 0x7f800000u) == 0x7f800000u && (u & 0x007fffffu) && u != 0x7fc00000u) ? S_NAN : S_OK; }
  case 14: {
    if (n != 8) return S_WIDTH;
    uint64_t u = 0; for (int i = 7; i >= 0; i--) u = u << 8 | b[i];
    return ((u & 0x7ff0000000000000ull) == 0x7ff0000000000000ull && (u & 0x000fffffffffffffull)
            && u != 0x7ff8000000000000ull) ? S_NAN : S_OK; }
  }
  return S_OK;
}
static uint64_t hash(int comp, H bp, const void *payload, uint64_t n) {   /* FNV-1a over 64 bits */
  uint64_t h = 14695981039346656037ull ^ (uint64_t)comp; const uint8_t *p = payload;
  h = (h ^ bp) * 1099511628211ull;
  for (uint64_t i = 0, m = comp ? n * sizeof(H) : n; i < m; i++) h = (h ^ p[i]) * 1099511628211ull;
  return h;
}
static const void *payload_of(H h) { return nd[h].comp ? (const void *)(kids + nd[h].off) : (const void *)(bytes + nd[h].off); }
static void tab_put(H h) {
  uint64_t i = hash(nd[h].comp, nd[h].bp, payload_of(h), nd[h].n) & (tcap - 1);
  while (tab[i]) i = (i + 1) & (tcap - 1);
  tab[i] = h;
}
/* The one node with this content. Refused content sets *why and answers nothing.
 * A new record is whole before nn moves past it; its writer is the node now running. */
static H mk(int comp, H bp, const void *payload, uint64_t n, Status *why) {
  if (bp || nn > 1) { if ((*why = rule(comp, bp, payload, n))) return 0; }
  size_t sz = comp ? (size_t)n * sizeof(H) : n;
  if (!tcap || nn * 2 > tcap) {                 /* the table by content grows where it lies */
    uint64_t c = tcap ? tcap * 2 : 4096; room(A_BY_CONTENT, c); memset(tab, 0, (size_t)c * sizeof *tab); tcap = c;
    for (H h = 1; h < nn; h++) tab_put(h);
  }
  uint64_t i = hash(comp, bp, payload, n) & (tcap - 1);
  for (; tab[i]; i = (i + 1) & (tcap - 1)) {
    Node *x = &nd[tab[i]];
    if (x->comp == comp && x->bp == bp && x->n == n && !memcmp(payload_of(tab[i]), payload, sz)) return tab[i];
  }
  room(A_NODES, nn + 1); Node *x = &nd[nn]; memset(x, 0, sizeof *x);
  x->bp = bp; x->by = W->at; x->comp = (uint8_t)comp; x->n = n;
  if (comp) { room(A_CHILDREN, nkids + n); x->off = nkids; memcpy(kids + nkids, payload, sz); nkids += n; }
  else { room(A_LEAF_BYTES, nbytes + n); x->off = nbytes; memcpy(bytes + nbytes, payload, sz); nbytes += n; }
  tab[i] = (H)nn; return (H)nn++;
}
#define KID(x, i) (kids[nd[x].off + (i)])
#define TOMB 0xffffffffu                  /* a place in the table by id whose node was given back */
static uint64_t idstart(const uint8_t *id) { uint64_t u; memcpy(&u, id, 8); return u & (icap - 1); }
static uint64_t idslot(const uint8_t *id) {
  uint64_t i = idstart(id);
  while (idtab[i] && (idtab[i] == TOMB || memcmp(NID(idtab[i]), id, 32))) i = (i + 1) & (icap - 1);
  return i;
}
/* Section 2: canonical bytes of h, in working bytes the caller gives back. */
static const uint8_t *idof(H h);
static uint8_t *cbytes(H h, size_t *len) {
  uint64_t n = nd[h].n; *len = 33 + (nd[h].comp ? (size_t)n * 32 : n);
  uint8_t *b = talloc(*len);
  b[0] = nd[h].comp;
  if (nd[h].bp) memcpy(b + 1, idof(nd[h].bp), 32);
  if (nd[h].comp) for (uint64_t i = 0; i < n; i++) memcpy(b + 33 + 32 * (size_t)i, idof(KID(h, i)), 32);
  else memcpy(b + 33, bytes + nd[h].off, n);
  return b;
}
static void idmake(H skip) {              /* made again with only live places, larger while half would be taken */
  uint64_t live = 0, c = icap ? icap : 4096;
  for (H x = 1; x < nn; x++) live += nd[x].ida && x != skip;
  while ((live + 1) * 2 >= c) c *= 2;
  room(A_BY_ID, c); memset(idtab, 0, (size_t)c * sizeof *idtab); icap = c; W->ids = (uint32_t)live;
  for (H x = 1; x < nn; x++) if (nd[x].ida && x != skip) idtab[idslot(NID(x))] = x;
}
/* A node's id lies beside it, in the area of ids, only once it is asked for. */
static void setid(H h, const uint8_t *id) {
  if (nd[h].ida) return;
  room(A_IDS, nids + IDREC); memcpy(ids + nids, &h, 4); memcpy(ids + nids + 4, id, 32);
  nd[h].ida = (uint32_t)(nids / IDREC); nids += IDREC;
  if (!icap || (uint64_t)W->ids * 2 >= icap) idmake(h);     /* ids counts the places taken, given back ones too */
  idtab[idslot(id)] = h; W->ids++;
}
static const uint8_t *idof(H h) {         /* section 3 */
  if (!nd[h].ida) { uint64_t m = ntext; size_t len; uint8_t *b = cbytes(h, &len), id[32]; sha256(b, len, id); ntext = m; setid(h, id); }
  return NID(h);
}
static H byid(const uint8_t *id) { return icap ? idtab[idslot(id)] : 0; }
static const uint8_t ZERO[32];
/* Section 2 read backwards: the node these canonical bytes are, every part already known. */
static H from_bytes(const uint8_t *b, size_t len, Status *why) {
  *why = S_OK;
  if (len < 33) { *why = S_SHORT; return 0; }
  if (b[0] > 1) { *why = S_KIND; return 0; }
  size_t n = len - 33; int comp = b[0];
  if (!memcmp(b + 1, ZERO, 32)) { if (!comp && !n) return ROOT; *why = S_NOTHING_BLUEPRINT; return 0; }
  if (comp) {
    if (n % 32) { *why = S_CHILD_LENGTH; return 0; }
    for (size_t i = 0; i < n; i += 32) if (!memcmp(b + 33 + i, ZERO, 32)) { *why = S_NOTHING_CHILD; return 0; }
  }
  H bp = byid(b + 1), h; if (!bp) { *why = S_UNKNOWN; return 0; }
  if (comp) {
    uint64_t m = n / 32, base = ntk; room(A_GATHERING_CHILDREN, ntk + m);
    for (uint64_t i = 0; i < m; i++) if (!(tk[base + i] = byid(b + 33 + 32 * (size_t)i))) { *why = S_UNKNOWN; return 0; }
    h = mk(1, bp, tk + base, m, why);
  } else h = mk(0, bp, b + 33, n, why);
  if (h) { uint8_t id[32]; sha256(b, len, id); setid(h, id); }
  return h;
}
static H catalogue(uint64_t n) {
  Status why;
  if (n < SEEDED) return E(n);
  uint8_t b[8]; uint32_t len = 0;
  for (; n; n >>= 8) b[len++] = (uint8_t)n;
  return mk(0, ROOT, b, len, &why);
}
static void store_init(void) {
  Status why; uint8_t b[4];
  mk(0, 0, "", 0, &why);                                  /* the root: blueprint nothing, no bytes */
  for (uint32_t n = 1; n < SEEDED; n++) { b[0] = (uint8_t)n; mk(0, ROOT, b, 1, &why); }
  for (H h = 1; h <= SEEDED; h++) idof(h);
  memcpy(fb->magic, FB_MAGIC, 8);                         /* from here a reader can make sense of the region */
}
static H txt(const char *s) { Status why; return mk(0, E(TEXT), s, strlen(s), &why); }
static int geti(H h, int64_t *out) {
  if (!h || nd[h].comp || nd[h].bp != E(INT) || nd[h].n > 8) return 0;
  uint32_t n = (uint32_t)nd[h].n; const uint8_t *b = bytes + nd[h].off;
  uint64_t u = (n && (b[n-1] & 0x80)) ? ~0ull : 0;
  for (uint32_t i = n; i > 0; i--) u = u << 8 | b[i-1];
  *out = (int64_t)u; return 1;
}
static H mki(int64_t v) {
  uint8_t b[8]; uint32_t n = 8; Status why;
  for (int i = 0; i < 8; i++) b[i] = (uint8_t)((uint64_t)v >> (8 * i));
  while (n > 1 && ((b[n-1] == 0x00 && !(b[n-2] & 0x80)) || (b[n-1] == 0xff && (b[n-2] & 0x80)))) n--;
  if (n == 1 && b[0] == 0) n = 0;
  return mk(0, E(INT), b, n, &why);
}

/* ---- section 6: the canonical print, read and written -------------------------------- */
typedef struct { const char *p; Status err; } Rd;               /* where the reader stands, and its first refusal */
static H refuse(Rd *r, Status w) { if (!r->err) r->err = w; return 0; }
static int number(Rd *r, uint64_t *out) {   /* at most 2^63 - 1, so one signed word holds any entry */
  const char *p = r->p;
  if (*p < '0' || *p > '9' || (*p == '0' && p[1] >= '0' && p[1] <= '9')) return 0;
  uint64_t v = 0;
  for (; *p >= '0' && *p <= '9'; p++) { unsigned d = (unsigned)(*p - '0'); if (v > (INT64_MAX - d) / 10) return 0; v = v * 10 + d; }
  r->p = p; *out = v; return 1;
}
static int hexval(char c) { return c >= '0' && c <= '9' ? c - '0' : c >= 'a' && c <= 'f' ? c - 'a' + 10 : -1; }
static H read_node(Rd *r) {
  uint64_t n; Status why;
  if (*r->p == '#') {
    r->p++; if (!number(r, &n)) return refuse(r, S_SYNTAX);
    return catalogue(n);
  }
  if (*r->p == '^') { r->p++; if (!number(r, &n)) return refuse(r, S_SYNTAX);
    return (n < nopen && opened[n]) ? opened[n] : refuse(r, S_FORWARD); }
  if (*r->p != '[' && *r->p != '(') return refuse(r, S_SYNTAX);
  int comp = *r->p++ == '('; uint32_t me = nopen; H x;
  room(A_READER_OPENED, (uint64_t)nopen + 1); opened[nopen++] = 0;
  H bp = read_node(r); if (!bp) return 0;
  if (comp) {
    uint32_t base = ntk;
    while (*r->p == ' ') { r->p++; H k = read_node(r); if (!k) { ntk = base; return 0; }
      room(A_GATHERING_CHILDREN, (uint64_t)ntk + 1); tk[ntk++] = k; }
    if (*r->p++ != ')') { ntk = base; return refuse(r, S_SYNTAX); }
    x = mk(1, bp, tk + base, ntk - base, &why); ntk = base;
  } else {
    uint64_t base = ntb;
    if (*r->p == ' ') { r->p++;
      if (hexval(*r->p) < 0) return refuse(r, S_SYNTAX);
      while (hexval(*r->p) >= 0) {
        if (hexval(r->p[1]) < 0) { ntb = base; return refuse(r, S_SYNTAX); }
        room(A_GATHERING_BYTES, (uint64_t)ntb + 1); tb[ntb++] = (uint8_t)(hexval(r->p[0]) << 4 | hexval(r->p[1])); r->p += 2; } }
    if (*r->p++ != ']') { ntb = base; return refuse(r, S_SYNTAX); }
    x = mk(0, bp, tb + base, ntb - base, &why); ntb = base;
  }
  if (!x) return refuse(r, why);
  if (catn(x) >= 0) return refuse(r, S_NOT_CANONICAL);                         /* rule 2 */
  for (uint32_t j = 0; j < nopen; j++) if (opened[j] == x) return refuse(r, S_NOT_CANONICAL);     /* rule 4 */
  opened[me] = x; return x;
}
/* A print ends with the text or at a line end. What a file holds after that line lies beside
 * the node and is no part of it: a lens keeps the names it gave there. */
static H read_print(const char *s, Status *why) {    /* s lies in the region */
  Rd r = { s, S_OK }; nopen = 0;
  H x = read_node(&r);
  if (x && *r.p && *r.p != '\n') x = refuse(&r, S_SYNTAX);
  *why = r.err; return x;
}
static void oput(const char *s) { size_t l = strlen(s); room(A_PRINT, nob + l + 1); memcpy(ob + nob, s, l + 1); nob += l; }
static const char *dec(char t[24], uint64_t v) { char *e = t + 23; *e = 0; do *--e = (char)('0' + v % 10); while (v /= 10); return e; }
static const char HEX[] = "0123456789abcdef";
static void print_node(H x) {
  char t[24]; long long c = catn(x);
  if (c >= 0) { oput("#"); oput(dec(t, (uint64_t)c)); return; }
  if (pnum[x]) { oput("^"); oput(dec(t, pnum[x] - 1)); return; }
  pnum[x] = ++W->pcount; room(A_GATHERING_CHILDREN, (uint64_t)ntk + 1); tk[ntk++] = x;
  oput(nd[x].comp ? "(" : "["); print_node(nd[x].bp);
  if (nd[x].comp) for (uint64_t i = 0; i < nd[x].n; i++) { oput(" "); print_node(KID(x, i)); }
  else if (nd[x].n) { oput(" "); t[2] = 0;
    for (uint64_t i = 0; i < nd[x].n; i++) { uint8_t b = bytes[nd[x].off + i]; t[0] = HEX[b >> 4]; t[1] = HEX[b & 15]; oput(t); } }
  oput(nd[x].comp ? ")" : "]");
}
static const char *print(H x) {
  room(A_PRINT_NUMBERS, nn + 1); N(A_PRINT_NUMBERS) = nn; W->pcount = 0; nob = 0; oput("");
  uint64_t base = ntk; print_node(x);
  while (ntk > base) pnum[tk[--ntk]] = 0;         /* a print costs what it prints: only its own numbers are taken back */
  return ob;
}

/* ---- code pages -----------------------------------------------------------------------
 * The host does not let executable pages be shared, so the runnable copy of a code leaf
 * lies outside the region. The leaf, where its copy starts and who asked for it are here. */
#define CODE_REGION 0xfffff000u          /* as far as a 32-bit offset into it reaches; pages are taken as written */
static uint64_t code_target(H leaf) {
  if (!leaf || nd[leaf].comp) return 0;
  H bp = nd[leaf].bp;
  if (bp == E(CODE)) return TARGET_AAPCS64;  /* the original AArch64 image format */
  long long abi = bp && nd[bp].comp && nd[bp].bp == E(CODE) && nd[bp].n == 1 ? catn(KID(bp, 0)) : -1;
  return abi > 0 ? (uint64_t)abi : 0;
}
static int local_code(H leaf) { return HOST_ABI && code_target(leaf) == HOST_ABI; }
static void *birth(H leaf) {              /* the leaf's instructions, runnable; NULL if this host cannot */
  uint8_t *code = (uint8_t *)(uintptr_t)W->code; uint64_t n = nd[leaf].n;
  for (uint32_t i = 0; i < nborn; i++) if (born[i].leaf == leaf) return code + born[i].off;
  uint64_t cap = fb->a[A_TEXT].cap < CODE_REGION ? fb->a[A_TEXT].cap : CODE_REGION;
  if (!n) return NULL;
  uint64_t next = W->ncode + ((n + 15) & ~15ull);
  if (next > cap) area_full(A_BORN_CODE);
  room(A_BORN_CODE, (uint64_t)nborn + 1);
  if (ncp) room(A_TRAIL, (uint64_t)ntrail + 1);
  if (!fits_growth(W->ncode, next)) area_full(A_BORN_CODE);
  if (!code) { if (!(code = host.code_map((size_t)cap))) return NULL; W->code = (uint64_t)(uintptr_t)code; }
  void *fn = code + W->ncode;
  if (!host.code_publish(fn, bytes + nd[leaf].off, n)) return NULL;
  born[nborn] = (Code){ E(CODE), W->at, leaf, W->ncode }; W->ncode = (uint32_t)next;
  kept(E(CODE), nborn, leaf); nborn++;
  return fn;
}

/* ---- the walker ------------------------------------------------------------------------
 * A machine with no C recursion whose registers are the region's Machine record.
 * Continuation frames are never changed once made, so a choice point can keep one and
 * return to it; fail drops whatever was made after it. */
static uint32_t push(uint32_t t, H node, uint32_t i, uint32_t env, uint32_t cut, uint32_t vals, uint32_t k, uint32_t aux) {
  room(A_FRAMES, (uint64_t)nfr + 1); fr[nfr] = (Frame){ E(FRAME), W->at, t, node, i, env, cut, vals, k, aux, nact, nlink }; return nfr++;
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
 * both counts, OWN when what is bound under it is the program's own, and COOL when a faster form
 * has declined above it: beneath that, faster forms are not tried again. 0: the bootstrap's code. */
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
/* Values in hand: the values of a node's children while its operation runs, and what a call is
 * given. They lie in the area values, taken from its top and all given back at the walker's next
 * step, so a call passes as many values as memory holds. */
static H *in_hand(uint64_t n) { room(A_VALUES, nhand + n); H *p = hand + nhand; nhand += n; return p; }
/* The calls held back by after, oldest first, as one node to run now that no choice is open. */
static H due(void) {
  uint32_t n = 0, at = 0; Status why;
  for (uint64_t j = 0; j < nafter; j += (uint64_t)aft[j] + 1, n++)      /* each: how many words follow, the recipe, its values */
    if ((uint64_t)aft[j] + 1 > nafter - j) die("a call held back runs past the calls held back");
  for (uint32_t k = W->k; k; k = fr[k].t == F_ALL ? fr[k].vals : fr[k].k)       /* a recover around this place marked */
    if (fr[k].t == F_RECOVER) fr[k].i = 0;                                       /* records this settling uses up */
  H *s = in_hand(n); n = 0;
  while (at < nafter) {
    uint32_t c = aft[at++]; H *p = in_hand(c); p[0] = aft[at];
    for (uint32_t j = 1; j < c; j++) p[j] = mk(1, E(QUOTE), &aft[at + j], 1, &why);
    s[n++] = mk(1, E(CALL), p, c, &why); at += c;
  }
  nafter = 0;
  return mk(1, E(SEQ), s, n, &why);
}
static int is_recipe(H f) { return f && nd[f].comp && nd[f].bp == E(RECIPE) && nd[f].n == 1; }
static int is_lazy(H f) { return f && nd[f].comp && nd[f].bp == E(LAZY) && nd[f].n == 1; }
/* Entry e names data: a node under it answers itself and e cannot be bound. A leaf, e below 0, is data too. */
static int is_data(long long e) { return e < QUOTE || e == RECIPE || (e > NATIVE_OP && e < BLUEPRINT) || e == BINDING || e == LAZY || e == TRAIL || e == CLOSED || e == ROOM || e == ROOM_TRAIL; }
/* What is called is a recipe, or a recipe closed over values: those come first among its arguments.
 * Answers the recipe or nothing, and in full the n values of a made ready for it, in hand. */
static int closed(H v) { return v && nd[v].comp && nd[v].bp == E(CLOSED) && nd[v].n; }
static H recipe_of(H v) { while (closed(v)) v = KID(v, 0); return is_recipe(v) ? v : 0; }
static H callee(H v, const H *a, uint32_t n, const H **full, uint32_t *nfull) {
  if (!closed(v)) { *full = a; *nfull = n; return is_recipe(v) ? v : 0; }     /* a plain call: its values where they lie */
  uint64_t m = 0, at;
  for (H u = v; closed(u); u = KID(u, 0)) m += nd[u].n - 1;
  H *p = in_hand(m + n); if (n) memcpy(p + m, a, sizeof(H) * n);      /* a, when in hand, lies below p */
  for (at = m; closed(v); v = KID(v, 0)) { uint32_t k = nd[v].n - 1; at -= k; memcpy(p + at, kids + nd[v].off + 1, sizeof(H) * k); }
  *full = p; *nfull = (uint32_t)(m + n);
  return is_recipe(v) ? v : 0;
}
static void fault(Status w) { W->reason = notice(w); W->mode = M_FAULT; }
static void answer(H v) { W->v = v; W->mode = M_RET; }
/* What an outcome answers: a word and a node, as 'ck run' prints them. */
static H seen_as(Status tag, H v) { Status why; H p[2] = { notice(tag), v }; return mk(1, E(LIST), p, 2, &why); }
/* ---- what a fail gives back ---------------------------------------------------------------------
 * A choice point marks where the nodes, their parts, what was gathered and the rooms stood when it
 * was made. A fail back to it gives back every node made since, as a backtracking machine gives
 * back its heap, except what no fail undoes and what that holds: what was gathered, the answer and
 * the reason in hand, an outcome's answer, a range's numbers, what the kept trail holds (notes and
 * their cells, faster forms, born code), bindings made from outside, and the rooms a kept node
 * names. Those move down in the order they were made. A node is always newer than its parts, so one
 * pass from the newest finds what is kept and one from the oldest moves it; a kept node whose
 * writer is given back is written by the choice that kept it. */
#define MARKS (uint32_t)nn, (uint32_t)ncoll, nkids, nbytes, N(A_ROOMS), nids
static uint32_t *fw; static H fm, ftop, fnode;
static void root(H h) { if (h >= fm && h < ftop) fw[h - fm] = 1; }
static H fwd(H h) { return h >= fm && h < ftop ? fw[h - fm] : h; }
static H byfwd(H h) { return h >= fm && h < ftop ? (fw[h - fm] ? fw[h - fm] : fnode) : h; }
static uint64_t roomlen(uint64_t at) { return sizeof(Room) + ((((Room *)(rooms + at))->size + 7) & ~7ull); }
static void reclaim(ChoicePoint *q) {
  fm = q->mnode; ftop = nn; fnode = q->node;
  if (ftop <= fm) return;
  uint64_t need = ntext + (uint64_t)(ftop - fm) * 4 + 64;
  if (need > fb->a[A_TEXT].peak && (need > fb->a[A_TEXT].cap || !fits_growth(area_extent(&fb->a[A_TEXT], fb->a[A_TEXT].peak), area_extent(&fb->a[A_TEXT], need)))) return;
  uint64_t mark = ntext, rtop = q->mtake, k = q->mkids, b = q->mbytes; fw = talloc((size_t)(ftop - fm) * 4);
  if (q->all == 2) root(q->coll);
  if (q->all == 3) { root(q->i); root(q->coll); }
  root(W->v); root(W->reason);
  for (uint32_t i = q->mcoll; i < ncoll; i++) root(coll[i].v);
  for (uint32_t i = q->mtrail; i < ntrail; i++) { root(trail[i].old); if (trail[i].bp == E(NOTE)) root(trail[i].key); }
  for (uint32_t i = 0; i < nout; i++) { root(out[i].entry); root(out[i].impl); root(out[i].fast); }
  for (H h = ftop; h-- > fm; ) {          /* what is kept holds its blueprint, its parts, the room it names */
    if (!fw[h - fm]) continue;            /* and, for code, the nodes its bytes name by id */
    root(nd[h].bp);
    if (nd[h].comp) for (uint64_t i = 0; i < nd[h].n; i++) root(KID(h, i));
    else if (nd[h].bp == E(ROOM) && nd[h].n == 8) {
      uint64_t at; memcpy(&at, bytes + nd[h].off, 8);
      if (at >= q->mtake && at < N(A_ROOMS)) { root(((Room *)(rooms + at))->bp); if (at + roomlen(at) > rtop) rtop = at + roomlen(at); }
    }
    else if (code_target(h) && icap) for (uint64_t o = 0; o + 32 <= nd[h].n; o++) { H r = byid(bytes + nd[h].off + o); if (r && r != TOMB) root(r); }
  }
  for (H h = ftop; h-- > fm; ) {          /* the tables forget them all, the newest first as linear probing needs */
    uint64_t i = hash(nd[h].comp, nd[h].bp, payload_of(h), nd[h].n) & (tcap - 1);
    while (tab[i] && tab[i] != h) i = (i + 1) & (tcap - 1);
    if (tab[i] == h) tab[i] = 0;
    if (nd[h].ida) {
      for (i = idstart(NID(h)); idtab[i] && idtab[i] != h; i = (i + 1) & (icap - 1)) ;
      if (idtab[i] == h) idtab[i] = TOMB; }
    if (h < N(A_CELLS) && !fw[h - fm]) held[h] = 0;
    if (h < N(A_BOUND_BY_NODE)) bykey[h] = 0;
    if (h < fb->a[A_PRINT_NUMBERS].peak) pnum[h] = 0;
  }
  H to = fm;
  for (H h = fm; h < ftop; h++) if (fw[h - fm]) fw[h - fm] = to++;
  room(A_GIVES, 4); gives[0]++; gives[1] = q->node; gives[2] = fm; gives[3] = ftop; N(A_GIVES) = 4;
  nn = fm;                                /* a watcher sees that places were given back, and none half moved */
  for (H h = fm; h < ftop; h++) {         /* each kept node moves down, its parts and cell with it */
    H n = fw[h - fm]; if (!n) continue;
    Node x = nd[h]; x.bp = fwd(x.bp); x.by = byfwd(x.by);
    if (x.comp) { for (uint64_t i = 0; i < x.n; i++) kids[k + i] = fwd(kids[x.off + i]); x.off = k; k += x.n; }
    else { memmove(bytes + b, bytes + x.off, x.n); x.off = b; b += x.n; }
    nd[n] = x;
    if (h < N(A_CELLS)) { H v = held[h]; held[h] = 0; held[n] = fwd(v); }
  }
  nn = to; nkids = k; nbytes = b;
  uint64_t w = q->mids;                   /* ids made since stay for the nodes that stay, moved down with them */
  for (uint64_t r = q->mids; r < nids; r += IDREC) {
    H o; memcpy(&o, ids + r, 4); H n = fwd(o);
    if (!n) continue;
    if (w != r) memmove(ids + w, ids + r, IDREC);
    memcpy(ids + w, &n, 4); nd[n].ida = (uint32_t)(w / IDREC); w += IDREC;
  }
  nids = w;
  for (H h = fm; h < nn; h++) { tab_put(h); if (nd[h].ida) { idtab[idslot(NID(h))] = h; W->ids++; } }
  if (icap && (uint64_t)W->ids * 2 >= icap) idmake(0);
  if (q->all == 2) q->coll = fwd(q->coll);
  if (q->all == 3) { q->i = fwd(q->i); q->coll = fwd(q->coll); }
  W->v = fwd(W->v); W->reason = fwd(W->reason);
  for (uint32_t i = q->mcoll; i < ncoll; i++) { coll[i].v = fwd(coll[i].v); coll[i].by = byfwd(coll[i].by); }
  for (uint32_t i = q->mtrail; i < ntrail; i++) {
    Trail *e = &trail[i]; e->old = fwd(e->old); e->by = byfwd(e->by);
    if (e->bp == E(NOTE)) e->key = fwd(e->key);
    else if (e->bp == E(FAST)) B(e->key)->fast = e->old;
    else { born[e->key].leaf = e->old; born[e->key].by = byfwd(born[e->key].by); }
  }
  for (uint32_t i = 0; i < nout; i++) {
    out[i].entry = fwd(out[i].entry); out[i].impl = fwd(out[i].impl); out[i].fast = fwd(out[i].fast); out[i].by = byfwd(out[i].by); }
  for (uint64_t at = q->mtake; at < rtop; at += roomlen(at)) {          /* rooms made since: the kept and those under them */
    Room *r = (Room *)(rooms + at); r->bp = fwd(r->bp); r->by = byfwd(r->by); }
  N(A_ROOMS) = rtop; ntext = mark;
}
/* A choice that stays open after a fail back to it begins again where things stand now. */
static void rise(ChoicePoint *q) {
  q->mnode = (uint32_t)nn; q->mkids = nkids; q->mbytes = nbytes; q->mcoll = (uint32_t)ncoll; q->mtrail = (uint32_t)ntrail;
  q->mtake = N(A_ROOMS); q->mids = nids; }
/* Call code and take its answer: k 0 a number, k 1 yes or no, k 4 the node whose handle v is,
 * any other k out of range. With k 3 the code declines and 0 is answered: what lies beneath runs.
 * So it does when code answers with no node: it met nothing, and the reason is for the walker to say. */
/* Code a program gave is called under watch: when the processor faults inside it, the walker goes
 * on and the program has a fault, crash, like any other. A fault anywhere else ends the process. */
static void crashed(int sig) {
  (void)sig; if (coding) siglongjmp(in_code, 1);
  put(2, "ck: the processor faulted outside any code a program gave it\n"); _exit(3);
}
static uint32_t left;
static Ret called(void *fn, const int64_t *arg) { int was = coding; coding = 1; Ret r = host.call(fn, arg); coding = was; return r; }
static int landed(void *fn, int64_t *arg) {
  if (!fn) { W->v = 0; W->reason = notice(S_NO_NATIVE); return 1; }
  arg[6] = (int64_t)(intptr_t)fb;         /* every call of code carries the region's address as its seventh argument */
  left = ntk; Ret r = called(fn, arg); ntk = left;                 /* what code gathered and left is given back */
  if (r.k == 3 || (r.k == 4 && !r.v)) { nnoted = 0; return 0; }    /* and the notes of code that declined are dropped: the walker */
  for (uint32_t j = 0; j < nnoted; j += 2) {                       /* writes them itself. Code that answered has them written now */
    room(A_CELLS, (uint64_t)noted[j] + 1); if (N(A_CELLS) <= noted[j]) N(A_CELLS) = noted[j] + 1;
    held[noted[j]] = noted[j+1]; kept(E(NOTE), noted[j], noted[j+1]);
  }
  nnoted = 0;
  if (r.k == 0) W->v = mki(r.v); else if (r.k == 1) W->v = r.v ? E(YES) : E(NO);
  else if (r.k == 4 && r.v > 0 && (uint64_t)r.v < nn) W->v = (H)r.v; else { W->v = 0; W->reason = notice(S_RANGE); }
  return 1;
}
/* The values code is given: an integer as the number it is, any other as its node's handle. The
 * eighth argument says how many there are and, above its low byte, has a bit for each that came
 * as a handle. Code that takes numbers only declines when a bit is set. */
static int given(const H *a, uint32_t c, int64_t *arg) {
  if (c > 6) return 0;
  arg[7] = c; for (uint32_t j = 0; j < c; j++) if (!geti(a[j], &arg[j])) { arg[j] = a[j]; arg[7] |= 256 << j; }
  return 1;
}
/* Something bound from outside sits on an operation of the bootstrap or on an entry the program defines:
 * while it does, a binding's faster form steps aside, so what watches sees every use. */
static int watched(void) {
  for (uint32_t j = 0; j < nout; j++) { H e = out[j].entry; long long n = e ? catn(e) : -1;
    if (e && ((n >= 0 && n < SEEDED) || (e < N(A_BOUND_BY_NODE) && bykey[e]))) return 1; }
  return 0;
}
/* Run binding b: a recipe is entered on the bindings beneath b, a code leaf is called. With
 * node nothing they get values already in hand, code as integers. Otherwise they get the node
 * itself, unevaluated: a lazy keeps the cut scope, code gets the node's handle and the
 * activation it stands in. Answers 0 when code declined. */
static int apply(uint32_t b, uint32_t lim, const H *a, uint32_t c, uint32_t from, H node) {
  H impl = B(b)->impl; int64_t arg[8] = {0};
  if (!nd[impl].comp && !code_target(impl)) {       /* another kind of implementation: what its blueprint */
    uint32_t r = bound(nd[impl].bp, 0, lim = under(b, lim));       /* is bound to runs it, given the leaf and the values */
    if (!r || !nd[B(r)->impl].comp) { fault(S_KIND); return 1; }
    H *t = in_hand((uint64_t)c + 1); t[0] = impl; if (c) memcpy(t + 1, a, sizeof(H) * c); a = t; c++; b = r; impl = B(r)->impl;
  }
  uint32_t cool = node ? 0 : act[from].lim & COOL;
  if (nd[impl].comp && !node && B(b)->fast && !cool && !watched()) {       /* the recipe has a faster form: tried first, */
    if (local_code(B(b)->fast) && given(a, c, arg)) {
      void *fn = birth(B(b)->fast);
      if (fn && landed(fn, arg)) { W->mode = M_RET; return 1; }
      cool = COOL;
    }      /* Where it declines, what the recipe */
    memset(arg, 0, sizeof arg);           /* calls does not try faster forms again: the reason to decline is still there */
  }
  if (nd[impl].comp) {
    uint32_t up = node ? from : act[from].up, cut = node ? W->cut : act[from].cut;      /* given answers, it has no user */
    if (!node) last(up);                                                                /* of its own: its caller's */
    uint32_t sees = b > OUT ? (lim & PM) | out[b-OUT-1].sees << 24                         /* bound from outside, it acts from there */
                            : (lim & OMOST << 24) | bd[b-1].sees | (act[from].lim & OWN);  /* the program's own acts as its user does */
    sees |= cool;
    W->env = enter(impl, a, c, sees, up, cut); if (!node || !is_lazy(impl)) W->cut = ncp;
    W->x = KID(impl, 0); W->mode = M_EVAL; return 1;
  }
  if (node) { arg[0] = node; arg[1] = from; arg[7] = c; }
  else if (!given(a, c, arg)) return 0;
  void *fn = local_code(impl) ? birth(impl) : NULL;
  if (!fn || !landed(fn, arg)) return 0;
  W->mode = M_RET; return 1;
}
/* Entries 64 to 71: a node taken apart, a node built from parts, and an entry bound. A build
 * the encoding refuses answers nothing, and the reason is the refusal's word. */
static void parts(long long op, const H *a, uint32_t c, int own) {
  Status why = S_OK; int64_t k = 0; H x = a[0], l = c == 2 ? a[1] : 0;
  if (c != (op == CLOCK ? 0u : op == BLUEPRINT || op == COUNT || op == LEAF || op >= READ ? 1u : 2u)) { fault(S_ARITY); return; }
  if (!x || (c == 2 && !l)) { W->v = 0; W->reason = notice(S_NOTHING_IN); return; }
  if (op == PRINT) { print(x); W->v = mk(0, E(TEXT), ob, nob, &why); return; }
  if (op == ID) { W->v = mk(0, E(BYTES), idof(x), 32, &why); return; }
  if (op == READ || op == FILE_OP) {       /* a leaf's bytes as a print to read, or as the path of a file */
    if (nd[x].comp) { fault(S_KIND); return; }
    uint64_t m = ntext; char *s = talloc((size_t)nd[x].n + 1); memcpy(s, bytes + nd[x].off, nd[x].n);
    if (op == READ) { if (!(W->v = read_print(s, &why))) W->reason = notice(why); }
    else { int64_t got = read_text(s);
      if (got == -2) area_full(A_TEXT);
      if (got < 0) { W->v = 0; W->reason = notice(S_NO_FILE); } else { room(A_TEXT, ntext + (uint64_t)got); W->v = mk(0, E(BYTES), text + ntext, (uint64_t)got, &why); } }
    ntext = m; return;
  }
  if (op == BIND) {                       /* x: an entry that is not data, or a composite such as a recipe or an */
    long long e = catn(x);                /* instance of one; l: a recipe, a lazy or a leaf */
    if ((e >= 0 && is_data(e)) || (e < 0 && !nd[x].comp) || (nd[l].comp && !is_recipe(l) && !is_lazy(l))) { fault(S_KIND); return; }
    if (!own) {                           /* bound from outside the run: seen at every limit, and no fail undoes it */
      if (nout == OMOST) { fault(S_FULL); return; }
      out[nout] = (Binding){ E(BINDING), W->at, x, l, 0, nout, 0 }; nout++; W->v = E(YES); return; }
    room(A_BINDINGS, (uint64_t)nbind + 1); room(A_BOUND_BY_NODE, (uint64_t)x + 1); if (N(A_BOUND_BY_NODE) <= x) N(A_BOUND_BY_NODE) = x + 1;
    bd[nbind] = (Binding){ E(BINDING), W->at, x, l, bykey[x], nbind, 0 }; bykey[x] = ++nbind; W->v = E(YES); return;
  }
  if (op == BLUEPRINT) { if (!(W->v = nd[x].bp)) W->reason = notice(S_ROOT); return; }
  if (op == COUNT) { W->v = mki(nd[x].n); return; }
  if (op == LEAF) { W->v = nd[x].comp ? E(NO) : E(YES); return; }
  if (op == CHILD || op == BYTE) {
    if (nd[x].comp != (op == CHILD) || !geti(l, &k)) { fault(S_KIND); return; }
    if (k < 0 || (uint64_t)k >= nd[x].n) { W->v = 0; W->reason = notice(S_RANGE); return; }
    W->v = op == CHILD ? KID(x, k) : mki(bytes[nd[x].off + k]); return;
  }
  if (!nd[l].comp || nd[l].bp != E(LIST)) { fault(S_KIND); return; }
  uint64_t n = nd[l].n;
  if (op == JOIN) W->v = mk(1, x, kids + nd[l].off, n, &why);
  else {
    room(A_GATHERING_BYTES, ntb + n);
    for (uint64_t i = 0; i < n; i++) {
      if (!geti(KID(l, i), &k)) { fault(S_KIND); return; }
      if (k < 0 || k > 255) { W->v = 0; W->reason = notice(S_RANGE); return; }
      tb[ntb + i] = (uint8_t)k;
    }
    W->v = mk(0, x, tb + ntb, n, &why);
  }
  if (!W->v) W->reason = notice(why);
}

/* The operations on values alone, the values in hand. env is the activation they are done in, and
 * node the node that asked, for a range. */
static void base(long long op, H *a, uint32_t c, uint32_t env, H node) {
  Status why; int64_t m, n, r = 0;
  if (op == HOST_TARGET) { if (c) fault(S_ARITY); else W->v = catalogue(HOST_ABI ? HOST_ABI : NO); return; }
  if (op == CLOSE) {                  /* a recipe and values: the recipe closed over them, a node that can be called */
    if (!recipe_of(a[0])) { fault(S_NOT_A_RECIPE); return; }
    for (uint32_t j = 1; j < c && a[0]; j++) if (!a[j]) a[0] = 0;
    if (!a[0]) { W->v = 0; W->reason = notice(S_NOTHING_IN); return; }
    W->v = mk(1, E(CLOSED), a, c, &why); return;
  }
  if (op == FAULT) {                  /* the program's own fault: its reason is whatever node it gives */
    if (c != 1) { fault(S_ARITY); return; }
    W->reason = a[0] ? a[0] : notice(S_NOTHING_IN); W->mode = M_FAULT; return;
  }
  if (op >= TAKE && op <= STORE) {    /* a room: bytes in the region under a blueprint, read and changed in place. While */
    int64_t o = 0, w = 0, v = 0; uint64_t at = 0; Room *h;   /* a choice is open a store puts what it changes on the */
    if (c != (uint32_t)(op - TAKE + 2)) { fault(S_ARITY); return; }   /* room trail, and a fail puts it back */
    if (!a[0] || !a[1] || (op == STORE && !a[3])) { W->v = 0; W->reason = notice(S_NOTHING_IN); return; }
    if (!geti(a[1], &o) || (op != TAKE && !geti(a[2], &w)) || (op == STORE && !geti(a[3], &v))) { fault(S_KIND); return; }
    if (op == TAKE) {
      if (o < 0) { W->v = 0; W->reason = notice(S_RANGE); return; }
      at = N(A_ROOMS); uint64_t len = sizeof(Room) + (((uint64_t)o + 7) & ~7ull); room(A_ROOMS, at + len);
      memset(rooms + at, 0, len); *(Room *)(rooms + at) = (Room){ a[0], W->at, 0, {0}, (uint64_t)o }; N(A_ROOMS) = at + len;
      uint8_t b[8]; for (int i = 0; i < 8; i++) b[i] = (uint8_t)(at >> (8 * i));
      W->v = mk(0, E(ROOM), b, 8, &why); return;
    }
    if (nd[a[0]].comp || nd[a[0]].bp != E(ROOM) || nd[a[0]].n != 8) { fault(S_KIND); return; }
    memcpy(&at, bytes + nd[a[0]].off, 8); h = (Room *)(rooms + at);
    if (at % 8 || at + sizeof(Room) > N(A_ROOMS) || (w != 1 && w != 2 && w != 4 && w != 8) || o < 0 || (uint64_t)o + (uint64_t)w > h->size) {
      W->v = 0; W->reason = notice(S_RANGE); return; }
    uint8_t *p = rooms + at + sizeof(Room) + o; uint64_t from = (uint64_t)(p - rooms);
    if (op == LOAD) { uint64_t u = 0; for (int64_t i = w; i > 0; i--) u = u << 8 | p[i-1]; W->v = mki((int64_t)u); return; }
    if (!ncp) nrtrail = 0;
    else {                                /* the bytes it changes, and the room's writer and count */
      uint64_t g0 = from & ~3ull, n4 = (from + (uint64_t)w - g0 + 3) / 4; room(A_ROOM_TRAIL, nrtrail + n4 + 2);
      for (uint64_t j = 0; j < n4 + 2; j++) {
        uint64_t g = j == 0 ? at + offsetof(Room, by) : j == 1 ? at + offsetof(Room, writes) : g0 + 4 * (j - 2);
        RoomTrail t = { E(ROOM_TRAIL), W->at, 0, {0}, g }; memcpy(&t.old, rooms + g, 4); rtrail[nrtrail++] = t; } }
    for (int64_t i = 0; i < w; i++) p[i] = (uint8_t)((uint64_t)v >> (8 * i));
    h->by = W->at; h->writes++; W->v = a[3]; return;
  }
  if (op >= BLUEPRINT && op < DIV) { parts(op, a, c, act[env].lim >> 31); return; }
  if (op == ENCODE || op == DECODE) {             /* the encoding's section 2, forwards and backwards */
    if (c != 1) { fault(S_ARITY); return; }
    if (!a[0]) { W->v = 0; W->reason = notice(S_NOTHING_IN); return; }
    if (op == ENCODE) { uint64_t mark = ntext; size_t len; uint8_t *cb = cbytes(a[0], &len); W->v = mk(0, E(BYTES), cb, len, &why); ntext = mark; return; }
    if (nd[a[0]].comp) { fault(S_KIND); return; }
    if (!(W->v = from_bytes(bytes + nd[a[0]].off, nd[a[0]].n, &why))) W->reason = notice(why);
    return;
  }
  if (op == HOST) {                   /* one raw call of what the host knows by a name: integers go as they are, */
    int64_t arg[8] = {0}; uint64_t mark = ntext; void *fn = 0;     /* any other leaf as the address of its bytes with a zero after */
    if (c < 1 || (!numeric_io && c > 9)) { fault(S_ARITY); return; }      /* the eight registers AArch64 passes values in */
    if (ncp) { fault(S_IRREVERSIBLE); return; }        /* what cannot be undone is refused while a choice is open */
    if (numeric_io) {
      H *quoted = in_hand(c - 1), target = a[0];
      if (!target || !bound(target, 0, act[env].lim)) { W->v = 0; W->reason = notice(S_UNBOUND); return; }
      for (uint32_t j = 1; j < c; ++j) {
        if (!a[j]) { W->v = 0; W->reason = notice(S_NOTHING_IN); return; }
        quoted[j-1] = mk(1, E(QUOTE), a + j, 1, &why);
      }
      W->x = mk(1, target, quoted, c - 1, &why); W->mode = M_EVAL; return;
    }
    for (uint32_t j = 0; j < c && W->mode != M_FAULT; j++) {
      if (!a[j] || nd[a[j]].comp) { fault(S_KIND); return; }
      if (j && geti(a[j], &arg[j-1])) continue;
      char *s = talloc((size_t)nd[a[j]].n + 1); memcpy(s, bytes + nd[a[j]].off, nd[a[j]].n);
      if (j) arg[j-1] = (int64_t)(intptr_t)s; else fn = host.symbol(s);
    }
    if (W->mode == M_FAULT) { ntext = mark; return; }
    if (!fn) { W->v = 0; W->reason = notice(S_NO_SYMBOL); ntext = mark; return; }
    Ret r;
    ntext = mark;                     /* given back before the call, since a crash does not return here */
    r = called(fn, arg); W->v = mki(r.v);
    return;
  }
  if (op == LOOK) {                   /* bytes of another process's region; the cell look holds the name of the one mapped */
    int64_t o = 0, n = 0; uint64_t mark = ntext;
    if (c != 3) { fault(S_ARITY); return; }
    if (!a[0] || !a[1] || !a[2]) { W->v = 0; W->reason = notice(S_NOTHING_IN); return; }
    if (nd[a[0]].comp || !geti(a[1], &o) || !geti(a[2], &n)) { fault(S_KIND); return; }
    room(A_CELLS, (uint64_t)E(LOOK) + 1); if (N(A_CELLS) <= E(LOOK)) N(A_CELLS) = E(LOOK) + 1;
    if (held[E(LOOK)] != a[0]) {
      char *s = talloc((size_t)nd[a[0]].n + 1); memcpy(s, bytes + nd[a[0]].off, nd[a[0]].n);
      void *p = host.watch(s); ntext = mark;
      if (!p) { W->v = 0; W->reason = notice(S_NO_REGION); return; }
      W->watch = (uint64_t)(uintptr_t)p; held[E(LOOK)] = a[0]; kept(E(NOTE), E(LOOK), a[0]);
    }
    const Head *far = (const Head *)(uintptr_t)W->watch;
    if (o < 0 || n < 0 || (uint64_t)o + (uint64_t)n > far->bytes) { W->v = 0; W->reason = notice(S_RANGE); return; }
    uint8_t *copy = talloc((size_t)n); memcpy(copy, (const uint8_t *)far + o, (size_t)n);
    W->v = mk(0, E(BYTES), copy, (uint64_t)n, &why); ntext = mark; return;
  }
  if (op == PEEK || op == HANDLE || op == AT) {   /* the region read by a program in it: its bytes, and where a node is */
    int64_t o = 0, n = 0; uint64_t guard = fb->a[A_STACK].off;
    if (c != (op == PEEK ? 2u : 1u)) { fault(S_ARITY); return; }
    if (!a[0] || (op == PEEK && !a[1])) { W->v = 0; W->reason = notice(S_NOTHING_IN); return; }
    if (op == HANDLE) { W->v = mki(a[0]); return; }
    if (!geti(a[0], &o) || (op == PEEK && !geti(a[1], &n))) { fault(S_KIND); return; }
    if (op == AT ? (o < 1 || (uint64_t)o >= nn) : (o < 0 || n < 0 || (uint64_t)o + (uint64_t)n > fb->bytes
                                          || ((uint64_t)o < guard && (uint64_t)o + (uint64_t)n > guard - FB_PAGE))) { W->v = 0; W->reason = notice(S_RANGE); return; }
    if (op == AT) { W->v = (H)o; return; }
    uint64_t mark = ntext; uint8_t *copy = talloc((size_t)n); memcpy(copy, (uint8_t *)fb + o, (size_t)n);
    W->v = mk(0, E(BYTES), copy, (uint64_t)n, &why); ntext = mark; return;
  }
  if (op == NOTHING) {                /* an answer of nothing, and the reason beside it */
    if (c != 1) { fault(S_ARITY); return; }
    W->v = 0; W->reason = a[0] ? a[0] : notice(S_NOTHING_IN); return;
  }
  if (op == FAST) {                   /* code as a faster form of the newest binding of an entry or a recipe: tried first */
    if (c != 2) { fault(S_ARITY); return; }      /* wherever that binding runs. A form of what is there, it is not undone */
    uint32_t r = a[0] ? bound(a[0], 0, ~0u) : 0;
    if (!r || !a[1]) { W->v = 0; W->reason = notice(r ? S_NOTHING_IN : S_UNBOUND); return; }
    if (!is_recipe(B(r)->impl) || !code_target(a[1])) { fault(S_KIND); return; }
    B(r)->fast = a[1]; kept(E(FAST), r, a[1]); W->v = E(YES); return;
  }
  if (op == BOUND) {                  /* what runs an entry or a recipe: its newest binding as a node, the entry, the */
    if (c < 1 || c > 2) { fault(S_ARITY); return; }     /* implementation and whether that runs on its own binding, as one made */
    uint32_t r = a[0] ? bound(a[0], 0, ~0u) : 0;       /* in a layer does. Seen from the top of the program, or, with a second */
    if (r && c == 2) {                /* entry, as that entry's own implementation sees it */
      uint32_t of = a[1] ? bound(a[1], 0, ~0u) : 0;
      r = of ? bound(a[0], 0, of > OUT ? (PM | out[of-OUT-1].sees << 24) : (OMOST << 24 | bd[of-1].sees)) : 0;
    }
    if (!r) { W->v = 0; W->reason = notice(S_UNBOUND); return; }
    H p[3] = { a[0], B(r)->impl, (r > OUT ? B(r)->sees + OUT : B(r)->sees) >= r ? E(YES) : E(NO) };
    W->v = mk(1, E(BINDING), p, 3, &why); return;
  }
  if (op == UNBIND) {                 /* the newest binding of an entry is taken out of its chain; the record */
    if (c != 1) { fault(S_ARITY); return; }      /* that says so points back at it, so a fail restores it */
    if (!(act[env].lim & OWN)) {    /* from outside: the newest one made there, which no fail brings back */
      uint32_t j = nout; while (j && out[j-1].entry != a[0]) j--;
      if (j && a[0]) { out[j-1].entry = 0; while (nout && !out[nout-1].entry) nout--; }
      W->v = j && a[0] ? E(YES) : E(NO); return;
    }
    if (!a[0] || a[0] >= N(A_BOUND_BY_NODE) || !bykey[a[0]]) { W->v = E(NO); return; }
    room(A_BINDINGS, (uint64_t)nbind + 1); bd[nbind] = (Binding){ E(BINDING), W->at, a[0], 0, bykey[a[0]], nbind, 0 }; nbind++;
    bykey[a[0]] = bd[bykey[a[0]] - 1].prev; W->v = E(YES); return;
  }
  if (op == HOLD || op == HELD || op == NOTE) {   /* a cell is any node; what it holds is a node. While a choice is */
    H key = a[0];                     /* open, what it held before goes on the trail, and a fail puts it back */
    if (c != (op == HELD ? 1u : 2u)) { fault(S_ARITY); return; }
    if (!key || (op != HELD && !a[1])) { W->v = 0; W->reason = notice(S_NOTHING_IN); return; }
    if (op == HELD) { if (!(W->v = key < N(A_CELLS) ? held[key] : 0)) W->reason = notice(S_EMPTY); return; }
    room(A_CELLS, (uint64_t)key + 1); if (N(A_CELLS) <= key) N(A_CELLS) = key + 1;
    if (op == NOTE) kept(E(NOTE), key, a[1]);      /* a note is what was seen of a run, not part of it: no fail undoes it */
    else if (!ncp) ntrail = 0; else { room(A_TRAIL, (uint64_t)ntrail + 1); trail[ntrail++] = (Trail){ E(TRAIL), W->at, key, held[key] }; }
    W->v = held[key] = a[1]; return;
  }
  if (c != 2) { fault(S_ARITY); return; }
  if (!a[0] || !a[1]) { W->v = 0; W->reason = notice(S_NOTHING_IN); return; }
  if (op == EQ) { W->v = a[0] == a[1] ? E(YES) : E(NO); return; }
  if (!geti(a[0], &m) || !geti(a[1], &n)) { fault(S_KIND); return; }
  if (op == LT) { W->v = m < n ? E(YES) : E(NO); return; }
  if (op == RANGE) {                  /* each integer from a up to b: a choice that counts instead of holding options */
    if (m >= n) { W->mode = M_FAILED; return; }
    if (m + 1 < n) { room(A_CHOICE_POINTS, (uint64_t)ncp + 1);
      H next = mki(m + 1);
      cp[ncp] = (ChoicePoint){ E(CHOICE_POINT), node, 3, node, next, env, W->k, nfr, nlink, nact, nargp, a[1], nbind, ntrail, nafter, nrtrail, MARKS }; ncp++; }
    W->v = a[0]; return;
  }
  if (op >= BAND && op <= SHR) {      /* the 64 bits of an integer: and, or, exclusive or, and shifts that fill with zero */
    if (op >= SHL && (n < 0 || n > 63)) { W->v = 0; W->reason = notice(S_RANGE); return; }
    uint64_t u = (uint64_t)m, w = (uint64_t)n;
    W->v = mki((int64_t)(op == BAND ? u & w : op == BOR ? u | w : op == BXOR ? u ^ w : op == SHL ? u << w : u >> w)); return;
  }
  if (op >= WADD && op <= UMOD) {     /* an integer as its 64 bits: sum, difference and product modulo 2^64, and */
    uint64_t u = (uint64_t)m, w = (uint64_t)n;        /* less-than, quotient and remainder read unsigned */
    if (op == ULT) { W->v = u < w ? E(YES) : E(NO); return; }
    if (op >= UDIV && !w) { W->v = 0; W->reason = notice(S_ZERO); return; }
    W->v = mki((int64_t)(op == WADD ? u + w : op == WSUB ? u - w : op == WMUL ? u * w : op == UDIV ? u / w : u % w)); return;
  }
  if (op == DIV || op == MOD) {       /* the quotient rounds toward zero; the remainder has the sign of a */
    if (!n) { W->v = 0; W->reason = notice(S_ZERO); return; }
    if (m == INT64_MIN && n == -1) { W->v = 0; W->reason = notice(S_RANGE); return; }
    W->v = mki(op == DIV ? m / n : m % n); return;
  }
  if (op == ADD ? __builtin_add_overflow(m, n, &r) : op == SUB ? __builtin_sub_overflow(m, n, &r)
                : __builtin_mul_overflow(m, n, &r)) { W->v = 0; W->reason = notice(S_RANGE); return; }
  W->v = mki(r);
}

/* Code asks the bootstrap for an operation on values in hand, as the bootstrap asks code: an integer
 * as the number it is, any other value as its handle, the eighth register saying how many and which.
 * The seventh is the operation: quote is the value itself, all the list of those that are something,
 * node the node whose id lies at the address given, so that code names a node by what it is and not
 * by where it is in one run, and any operation on values alone is itself. Answers found one at a
 * time are gathered here: gathered with no value says where the gathering stands, with one it adds
 * that value if it is something, and list with that place makes the list of what was added since.
 * A note code writes is kept back until the code answers, since code that declines is run again by
 * the walker; held reads what was kept back before it reads the cell. With 256 added the answer is
 * asked as a number: k 0, or k 2 for nothing, or k 3 for a node that is no number. Otherwise k 4 and a handle, none for
 * nothing. A fault goes to the walker and does not come back here. */
static int basic(long long op) {
  return op >= ADD && op <= LAST && !is_data(op) && op != NATIVE_OP && op != UP && op != BASE && op != STEP
      && op != OUTCOME && op != LAYER && !(op >= RANGE && op <= OUTSIDE) && op != AFTER;
}
static Ret serve(int64_t a0, int64_t a1, int64_t a2, int64_t a3, int64_t a4, int64_t a5, int64_t op, int64_t cm) {
  int64_t v[6] = { a0, a1, a2, a3, a4, a5 }, m; H a[8] = {0}, x; uint32_t c = cm & 255, n = 0; int num = op >> 8 & 1; Status why;
  op &= 255; if (c > 6) c = 6;
  if (op == GATHERED && c < 2) {
    if (!c) return (Ret){ ntk, num ? 0 : 3 };
    H g = cm >> 8 & 1 ? (a0 > 0 && (uint64_t)a0 < nn ? (H)a0 : 0) : mki(a0);
    if (g) { room(A_GATHERING_CHILDREN, (uint64_t)ntk + 1); tk[ntk++] = g; }
    return (Ret){ 0, 4 };
  }
  if (op == LIST) {
    if (c != 1 || cm != 1 || a0 < 0 || (uint64_t)a0 > ntk) { fault(S_KIND); siglongjmp(in_code, 3); }
    x = mk(1, E(LIST), tk + a0, ntk - (uint32_t)a0, &why); ntk = (uint32_t)a0; return (Ret){ x, 4 };
  }
  if (op == NOTE || op == HELD) {
    H key = !c ? 0 : cm >> 8 & 1 ? (a0 > 0 && (uint64_t)a0 < nn ? (H)a0 : 0) : mki(a0), v = c != 2 ? 0 : cm >> 9 & 1 ? (a1 > 0 && (uint64_t)a1 < nn ? (H)a1 : 0) : mki(a1);
    if (c == (op == NOTE ? 2u : 1u) && key && (op == HELD || v)) {
      if (op == NOTE) { room(A_NOTED, (uint64_t)nnoted + 2); noted[nnoted] = key; noted[nnoted+1] = v; nnoted += 2; return (Ret){ v, 4 }; }
      for (uint32_t j = nnoted; j > 0; j -= 2) if (noted[j-2] == key) return num ? (geti(noted[j-1], &m) ? (Ret){ m, 0 } : (Ret){ 0, 3 }) : (Ret){ noted[j-1], 4 };
    }
  }
  if (op == NODE) { x = c == 1 && cm == 1 ? byid((const uint8_t *)(uintptr_t)a0) : 0; if (x) return (Ret){ x, 4 }; fault(S_NO_NODE); siglongjmp(in_code, 3); }
  for (uint32_t j = 0; j < c; j++) a[j] = (cm >> (8 + j) & 1) && v[j] > 0 && (uint64_t)v[j] < nn ? (H)v[j] : 0;
  x = a[0];                               /* what code reads most is answered without a number being built for it */
  if (x && op == COUNT && num && c == 1) return (Ret){ nd[x].n, 0 };
  if (x && op == BYTE && num && c == 2 && !(cm >> 9 & 1) && !nd[x].comp && (uint64_t)v[1] < nd[x].n) return (Ret){ bytes[nd[x].off + v[1]], 0 };
  if (x && op == CHILD && c == 2 && !(cm >> 9 & 1) && nd[x].comp && (uint64_t)v[1] < nd[x].n) W->v = KID(x, v[1]);
  else {
    for (uint32_t j = 0; j < c; j++) if (!(cm >> (8 + j) & 1)) a[j] = mki(v[j]);
    if (op == QUOTE) W->v = c == 1 ? a[0] : 0;
    else if (op == ALL) { for (uint32_t j = 0; j < c; j++) if (a[j]) a[n++] = a[j]; W->v = mk(1, E(LIST), a, n, &why); }
    else if (!basic(op)) fault(S_KIND); else base(op, a, c, W->env, 0);
    if (W->mode == M_FAULT) siglongjmp(in_code, 3);
  }
  if (!num) return (Ret){ W->v, 4 };
  return !W->v ? (Ret){ 0, 2 } : geti(W->v, &m) ? (Ret){ m, 0 } : (Ret){ 0, 3 };
}

static void walk(H top) {                 /* ends with mode M_DONE; outcome is 'n' node, '0' nothing, 'f' fault */
  Status why; H *a = hand;
  const uint64_t text_base = ntext;
  W->serve = (uint64_t)(uintptr_t)serve; W->x = top; W->v = W->reason = 0; W->env = W->cut = W->k = 0; W->outcome = 0; W->mode = M_EVAL;
  nfr = nlink = nact = 1; nargp = ncp = ncoll = 0; act[0] = (Activation){ E(ACTIVATION), 0, 0, 0, 0, ~COOL, 0, 0 }; W->one = 0;
  switch (sigsetjmp(in_code, 1)) {        /* the walker goes on from here after what cannot go on where it happened: */
  case 1: coding = 0; ntk = left; nnoted = 0; fault(S_CRASH); break;              /* the processor faulted inside code a program gave */
  case 2: while (ntk) { H h = tk[--ntk]; if (h < fb->a[A_PRINT_NUMBERS].peak) pnum[h] = 0; }
    coding = 0; ntb = 0; ntext = text_base; W->reason = full_at[full]; W->mode = M_FAULT; break;
  case 3: coding = 0; ntk = left; nnoted = 0; break;                              /* an operation code asked for faulted: the fault is the program's */
  }
  for (nhand = 0;; W->step++, nhand = 0) switch (W->mode) {
  case M_EVAL: {
    if (nafter && !ncp) {                 /* every choice is closed: what was held back for that is done now */
      H q = due(); W->k = push(F_ASIDE, W->x, W->one, W->env, W->cut, W->reason, W->k, 1); W->x = q; W->env = W->one = 0; W->cut = 0;
    }
    if (N(A_ASKED) && !N(A_ANSWERED)) {   /* another process asks: its node is evaluated here, between two steps of the */
      uint64_t n = N(A_ASKED), mark = ntext; H q, p;        /* run, as from outside and leaving no other trace */
      if (n > fb->a[A_ASKED].cap) n = 0;
      room(A_ASKED, n);
      char *s = talloc((size_t)n + 1); memcpy(s, (uint8_t *)fb + fb->a[A_ASKED].off, n); s[n] = 0; N(A_ANSWERED) = 1;
      why = S_ID; q = numeric_io ? (n == 32 ? byid((uint8_t *)s) : 0) : read_print(s, &why);
      if (!q) { p = notice(why); q = mk(1, E(FAULT), &p, 1, &why); }
      ntext = mark; q = mk(1, E(OUTSIDE), &q, 1, &why); q = mk(1, E(OUTCOME), &q, 1, &why);
      W->k = push(F_ASIDE, W->x, W->one, W->env, W->cut, W->reason, W->k, 0); W->x = q; W->env = W->one = 0; W->cut = ncp;
    }
    H x = W->x; W->at = x;
    long long op = nd[x].comp ? catn(nd[x].bp) : -1; uint32_t c = nd[x].n, b = 0; int given = 0;
    uint32_t lim = W->one ? W->one - 1 : act[W->env].lim; W->one = 0;
    /* The newest binding that says what runs x: its blueprint's, which may be a composite such as
     * an instance of a recipe, or the step's, which is given every node. A step or a lazy gets x
     * itself; when its code declines, what lies beneath runs. */
    while ((nbind || nout) && (b = bound(nd[x].comp && (op >= QUOTE || op < 0) ? nd[x].bp : 0, E(STEP), lim)) && (B(b)->entry == E(STEP) || is_lazy(B(b)->impl))) {
      a = in_hand(1); a[0] = x; if ((given = apply(b, lim, a, 1, W->env, x))) break;
      lim = under(b, lim); b = 0;
    }
    if (given) break;
    if (!b && (is_data(op) || op > LAST)) { answer(x); break; }/* data, or an entry with no meaning yet */
    switch (b ? -1 : op) {
    case QUOTE: if (c != 1) fault(S_ARITY); else answer(KID(x, 0)); break;
    case ARG: {
      long long j = argn(x);
      if (j < 0) fault(S_ARITY); else if (j >= act[W->env].n) fault(S_ARG); else answer(argp[act[W->env].off + j]);
      break; }
    case SELF: if (!act[W->env].recipe) fault(S_NO_RECIPE); else answer(act[W->env].recipe); break;
    case LET:                             /* (let $k v body): body runs on the same arguments, with argument k being v */
      if (c != 3 || argn(KID(x, 0)) < 0) { fault(S_ARITY); break; }
      W->k = push(F_LET, x, 0, W->env, W->cut, 0, W->k, 0); W->x = KID(x, 1); break;
    case SEQ:
      if (!c) { answer(E(YES)); break; }
      if (c > 1) W->k = push(F_SEQ, x, 1, W->env, W->cut, 0, W->k, 0);
      W->x = KID(x, 0); break;
    case CHOICE:
      if (!c) { W->mode = M_FAILED; break; }
      W->cut = ncp;
      if (c > 1) { room(A_CHOICE_POINTS, (uint64_t)ncp + 1);
        cp[ncp] = (ChoicePoint){ E(CHOICE_POINT), x, 0, x, 1, W->env, W->k, nfr, nlink, nact, nargp, 0, nbind, ntrail, nafter, nrtrail, MARKS }; ncp++; }
      W->x = KID(x, 0); break;
    case FAIL:
      if (c > 1) { fault(S_ARITY); break; }
      if (!c) { W->mode = M_FAILED; break; }
      W->k = push(F_FAIL, x, 0, W->env, W->cut, 0, W->k, 0); W->x = KID(x, 0); break;
    case CUT:
      if (c > 1) { fault(S_ARITY); break; }
      if (!c) { if (ncp > W->cut) ncp = W->cut; answer(E(YES)); break; }
      W->k = push(F_CUT, x, 0, W->env, W->cut, 0, W->k, 0); W->x = KID(x, 0); break;
    case ALL:
      if (c != 1) { fault(S_ARITY); break; }
      room(A_CHOICE_POINTS, (uint64_t)ncp + 1);
      cp[ncp] = (ChoicePoint){ E(CHOICE_POINT), x, 1, x, ncoll, W->env, W->k, nfr, nlink, nact, nargp, ++W->gathers, nbind, ntrail, nafter, nrtrail, MARKS }; ncp++;
      W->k = push(F_ALL, x, 0, W->env, W->cut, W->k, 0, W->gathers); W->cut = ncp; W->x = KID(x, 0); break;
    case NEED: if (c != 1) { fault(S_ARITY); break; } W->k = push(F_NEED, x, 0, W->env, W->cut, 0, W->k, 0); W->x = KID(x, 0); break;
    case RECOVER: if (c != 2) { fault(S_ARITY); break; } W->k = push(F_RECOVER, x, nafter, W->env, W->cut, ncoll, W->k, ncp); W->x = KID(x, 0); break;
    case OUTSIDE:                         /* x runs as if asked from outside the run: what it binds is seen by every */
      if (c != 1) { fault(S_ARITY); break; }      /* layer of the program, and a fail does not take it away */
      W->env = again(W->env, 0); act[W->env].lim &= ~OWN; W->x = KID(x, 0); break;
    case LAYER: if (c != 1) { fault(S_ARITY); break; } W->k = push(F_LAYER, x, 0, W->env, W->cut, nout, W->k, nbind); W->x = KID(x, 0); break;
    case OUTCOME:                         /* x's first outcome is seen from behind a choice point, then failed into: */
      if (c != 1) { fault(S_ARITY); break; }      /* nothing x did stays, and the outcome is the answer */
      room(A_CHOICE_POINTS, (uint64_t)ncp + 1); W->reason = 0;
      cp[ncp] = (ChoicePoint){ E(CHOICE_POINT), x, 2, x, 0, W->env, W->k, nfr, nlink, nact, nargp, 0, nbind, ntrail, nafter, nrtrail, MARKS }; ncp++;
      W->k = push(F_OUTCOME, x, ncoll, W->env, W->cut, W->k, 0, ncp - 1); W->cut = ncp; W->x = KID(x, 0); break;
    default:                              /* call, arithmetic, eq, lt, native, parts: children first, in order */
      if (!c) {
        while (b && !apply(b, lim, hand, 0, W->env, 0)) b = bound(nd[x].bp, 0, lim = under(b, lim));
        if (!b) { if (op == CLOCK) answer(mki(host.clock())); else if (op == HOST_TARGET) answer(catalogue(HOST_ABI ? HOST_ABI : NO)); else fault(S_ARITY); }
        break; }
      W->k = push(F_ARGS, x, 1, W->env, W->cut, 0, W->k, lim); W->x = KID(x, 0); break;
    }
    break; }
  case M_RET: {
    if (!W->k) {
      if (W->v && nafter) {               /* the run ends with an answer: its choices are closed, and what waited is done */
        ncp = 0; H q = due(); W->k = push(F_ASIDE, W->v, 0, 0, 0, 0, 0, 2); W->x = q; W->env = W->cut = 0; W->mode = M_EVAL; break;
      }
      if (W->v) W->reason = 0; else if (!W->reason) W->reason = notice(S_NO_ANSWER);
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
    case F_ASIDE: {                       /* something done between two steps is over: the run goes on where it was */
      if (f.aux == 2) { W->v = f.node; break; }            /* or ends with the answer it had */
      if (!f.aux) {                       /* it was asked from outside: the outcome goes back to who asked */
        void *dst = (uint8_t *)fb + fb->a[A_ANSWERED].off;
        if (numeric_io) { room(A_ANSWERED, 32); memcpy(dst, idof(W->v), 32); N(A_ANSWERED) = 32; }
        else { print(W->v); uint64_t n = nob < fb->a[A_ANSWERED].cap ? nob : fb->a[A_ANSWERED].cap;
          room(A_ANSWERED, n); memcpy(dst, ob, n); N(A_ANSWERED) = n; } N(A_ASKED) = 0;
      }
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
      cp[f.aux].coll = W->v ? seen_as(S_NODE, W->v) : seen_as(S_NOTHING, W->reason ? W->reason : notice(S_NO_ANSWER));
      ncoll = f.i; ncp = f.aux + 1; W->mode = M_FAILED; break;
    case F_HANDLER: {
      const H *full; H node = f.node; uint32_t n; H to = callee(W->v, &node, 1, &full, &n);
      if (!to) { fault(S_NOT_A_RECIPE); break; }
      W->env = enter(W->v, full, n, act[f.env].lim, act[f.env].up, act[f.env].cut); W->cut = ncp; W->x = KID(to, 0); W->mode = M_EVAL; break; }
    case F_ARGS: {
      room(A_LINKS, (uint64_t)nlink + 1); lk[nlink] = (Link){ E(LINK), W->at, W->v, f.vals }; uint32_t vals = nlink++;
      uint32_t c = nd[f.node].n;
      if (f.i < c) { W->k = push(F_ARGS, f.node, f.i + 1, f.env, f.cut, vals, W->k, f.aux);
        W->env = f.env; W->cut = f.cut; W->x = KID(f.node, f.i); W->mode = M_EVAL; break; }
      a = in_hand(c); for (uint32_t j = c, q = vals; j > 0; j--, q = lk[q].next) a[j-1] = lk[q].v;
      unlink_vals(vals, c);
      long long op = catn(nd[f.node].bp);
      uint32_t b = 0, lim = f.aux;        /* code that declines leaves the values to what lies beneath it. So does a */
      W->cut = f.cut; W->env = f.env;     /* lazy bound after this node began: it takes nodes, and here are answers */
      while ((nbind || nout) && (b = bound(op == STEP ? 0 : nd[f.node].bp, 0, lim)) && (is_lazy(B(b)->impl) || !apply(b, lim, a, c, f.env, 0))) { lim = under(b, lim); b = 0; }
      if (b) break;
      if (op == UP || op == BASE || op == STEP) {       /* a node in hand is evaluated: by step where this node */
        if (c != 1) { fault(S_ARITY); break; }           /* stands; by the others where the implementation's user */
        if (!a[0]) { W->v = 0; W->reason = notice(S_NOTHING_IN); break; }
        W->one = op == BASE ? act[f.env].lim + 1 : 0;    /* stands, in that user's cut scope. base: the node's own */
        if (op != STEP) { W->cut = act[f.env].cut; W->env = act[f.env].up; }       /* entry means what it meant beneath */
        W->x = a[0]; W->mode = M_EVAL; break;
      }
      if (op == AFTER) {                  /* a call held back until every choice now open is closed; a fail that */
        if (!recipe_of(a[0])) { fault(S_NOT_A_RECIPE); break; }                /* returns past here first drops it */
        if (!ncp) op = CALL;
        else {
          for (uint32_t j = 1; j < c && a[0]; j++) if (!a[j]) a[0] = 0;
          if (!a[0]) { W->v = 0; W->reason = notice(S_NOTHING_IN); break; }
          room(A_AFTER, (uint64_t)nafter + c + 1); aft[nafter++] = c; memcpy(aft + nafter, a, sizeof(H) * c); nafter += c;
          W->v = E(YES); break;
        }
      }
      if (op == CALL) {
        const H *full; uint32_t n, on = act[f.env].lim;
        H to = callee(a[0], a + 1, c - 1, &full, &n);    /* a call its own code declined runs wholly on what lies beneath that code */
        if (!to) { fault(S_NOT_A_RECIPE); break; }
        for (lim = f.aux; (nbind || nout) && (b = bound(to, 0, lim)) && !apply(b, lim, full, n, f.env, 0); b = 0) { lim = under(b, lim); on = (lim & ~OWN) | (on & OWN); }
        if (b) break;
        uint32_t up = act[f.env].up, cut = act[f.env].cut; last(up);
        W->env = enter(a[0], full, n, on, up, cut); W->cut = ncp; W->x = KID(to, 0); W->mode = M_EVAL; break;
      }
      if (op == NATIVE_OP) {
        int64_t arg[8] = {0};
        if (!code_target(a[0]) || !given(a + 1, c - 1, arg)) { fault(S_KIND); break; }
        if (!landed(local_code(a[0]) ? birth(a[0]) : NULL, arg)) { W->v = 0; W->reason = notice(S_DECLINED); }
        break;
      }
      if (op > LAST) { fault(S_KIND); break; }       /* an entry only code ran, and the code declined these values */
      base(op, a, c, f.env, f.node); break; }
    }
    break; }
  case M_FAILED: {
    if (!ncp) { if (!W->reason) W->reason = notice(S_NO_PATH); W->v = 0; W->outcome = '0'; W->mode = M_DONE; return; }
    ChoicePoint *q = &cp[ncp-1]; W->at = q->node;
    nfr = q->mfr; nlink = q->mlink; nact = q->mact; nargp = q->marg; W->env = q->env; W->k = q->k; nafter = q->mafter;
    while (nbind > q->mbind) { nbind--; bykey[bd[nbind].entry] = bd[nbind].prev; }     /* bindings made since are undone */
    for (uint32_t t = ntrail; t > q->mtrail; t--)                      /* and cells hold what they held, */
      if (trail[t-1].bp == E(TRAIL)) held[trail[t-1].key] = trail[t-1].old;
    uint32_t kw = q->mtrail;                                            /* while what no fail undoes is kept, */
    for (uint32_t t = q->mtrail; t < ntrail; t++)                       /* a faster form as long as its binding */
      if (trail[t].bp != E(TRAIL) && (trail[t].bp != E(FAST) || trail[t].key > OUT || trail[t].key <= nbind)) trail[kw++] = trail[t];
    ntrail = kw;
    while (nrtrail > q->mroom) { nrtrail--; memcpy(rooms + rtrail[nrtrail].at, &rtrail[nrtrail].old, 4); }   /* rooms too; */
    reclaim(q);                                                         /* the nodes made since are given back, */
    for (uint32_t t = q->mtrail; t < ntrail; t++) if (trail[t].bp == E(NOTE)) held[trail[t].key] = trail[t].old;   /* notes stand */
    if (q->all == 3) {                    /* a range: its next integer, and the choice closes with the last */
      int64_t m = 0, n = 0; geti((H)q->i, &m); geti((H)q->coll, &n);
      W->v = (H)q->i; if (m + 1 < n) { q->i = mki(m + 1); rise(q); } else ncp--;
      W->mode = M_RET; break;
    }
    if (q->all == 2) {                    /* an outcome: what was seen, or that no path was left */
      ncp--; W->v = q->coll ? q->coll : seen_as(S_NOTHING, W->reason ? W->reason : notice(S_NO_PATH));
      W->reason = 0; W->mode = M_RET; break;
    }
    if (q->all) {                         /* every answer has been gathered */
      uint32_t base = ntk, w = q->i, id = q->coll;       /* its answers lie above where it began: q->i */
      for (uint32_t i = q->i; i < ncoll; i++) {
        if (coll[i].id == id) { room(A_GATHERING_CHILDREN, (uint64_t)ntk + 1); tk[ntk++] = coll[i].v; } else coll[w++] = coll[i]; }
      ncoll = w; ncp--;
      W->v = mk(1, E(LIST), tk + base, ntk - base, &why); ntk = base; W->mode = M_RET; break;
    }
    W->x = KID(q->node, q->i); W->cut = ncp - 1; W->mode = M_EVAL;
    if (q->i + 1 >= nd[q->node].n) ncp--; else { q->i++; q->by = W->at; rise(q); }
    break; }
  case M_FAULT: {
    uint32_t j = W->k;                    /* an all's frame keeps the way out of it in vals; its own k is nothing */
    while (j && fr[j].t != F_RECOVER && fr[j].t != F_OUTCOME) j = fr[j].t == F_ALL ? fr[j].vals : fr[j].k;
    if (!j) { W->v = 0; W->outcome = 'f'; W->mode = M_DONE; return; }
    Frame f = fr[j];
    if (f.t == F_OUTCOME) { cp[f.aux].coll = seen_as(S_FAULT, W->reason); ncoll = f.i; ncp = f.aux + 1; W->mode = M_FAILED; break; }
    while (ncp && cp[ncp-1].mfr > j) ncp--;   /* the choices the body opened close, though a cut in it closed older ones */
    nfr = j; if (f.mlink < nlink) nlink = f.mlink;     /* the faulted body's frames, values and calls are given back, */
    if (f.mact < nact) { nargp = act[f.mact].off; nact = f.mact; }
    nafter = f.i;                         /* what the faulted body held back is dropped with it */
    ncoll = f.vals;                       /* what the alls inside it had gathered goes with them */
    W->k = push(F_HANDLER, W->reason, 0, f.env, f.cut, 0, f.k, 0);      /* and the handler takes the recover's frame */
    W->env = f.env; W->cut = f.cut; W->x = KID(f.node, 1); W->mode = M_EVAL; break; }
  default: return;
  }
}

static void run(H top) { walking = 1; walk(top); walking = 0; }

/* ---- commands: a program is run, and everything else is a program -------------------------- */
#ifndef NUMERIC_PORT
#define STATUS_LABEL(key, value, label) [key] = label,
static const char *const status_words[] = { STATUS_ROWS(STATUS_LABEL) };
#undef STATUS_LABEL
#endif
static H notice(Status status) {
#ifndef NUMERIC_PORT
  if (!numeric_io) return txt(status_words[status]);
#endif
  uint8_t ordinal[8] = { (uint8_t)status, 0, 0, 0, 3, 0, 2, 16 }; Status why;
  return mk(0, ROOT, ordinal, sizeof ordinal, &why);
}
#ifndef NUMERIC_PORT
static H file_node(const char *path) {      /* a print file: one node on its first line */
  size_t len; Status why;
  H x = read_print((char *)load(path, &len), &why);
  if (!x) { say("refused ", status_words[why]); exit(1); }
  return x;
}
#endif
/* The numeric port accepts resolved content ids. A frozen graph already contains its links. */
static const uint8_t ice_header[8] = { 67, 75, 73, 67, 69, 49, 10, 0 };
static uint32_t little4(const uint8_t *p) {
  return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}
static void put4(uint8_t *p, uint32_t value) {
  for (int i = 0; i < 4; ++i) p[i] = (uint8_t)(value >> (8 * i));
}
static H open_image(const uint8_t *p, size_t n, Status *why) {
  *why = S_IMAGE;
  if (n < 8 || memcmp(p, ice_header, 8)) return 0;
  H last = 0;
  for (size_t i = 8; i < n;) {
    if (n - i < 4) { *why = S_SHORT; return 0; }
    size_t len = little4(p + i); i += 4;
    if (len > n - i) { *why = S_SHORT; return 0; }
    if (!(last = from_bytes(p + i, len, why))) return 0;
    i += len;
  }
  if (!last || !nd[last].comp || catn(nd[last].bp) != INT64_C(0x1002000400000001)
      || nd[last].n != 2 || nd[KID(last, 1)].bp != E(LIST)) { *why = S_IMAGE; return 0; }
  return last;
}
static int send_bytes(const uint8_t *p, size_t n) { return send_fd(1, p, n); }
static int recv_bytes(uint8_t *p, size_t n) {
  while (n) { ssize_t got = read(0, p, n); if (got < 0 && errno == EINTR) continue;
    if (got <= 0) return 0;
    p += got; n -= (size_t)got;
  }
  return 1;
}
/* Every part precedes its holder in the store. Mark its closure backwards, emit it forwards. */
static int send_answer(uint8_t status, H value) {
  uint8_t header[37]; uint64_t size = sizeof ice_header;
  room(A_PRINT_NUMBERS, (uint64_t)value + 1); N(A_PRINT_NUMBERS) = value + 1; pnum[value] = 1;
  for (H h = value; h; --h) if (pnum[h]) {
    Node *x = nd + h; size += 4 + 33 + (x->comp ? (uint64_t)x->n * 32 : x->n);
    if (x->bp > ROOT) pnum[x->bp] = 1;
    if (x->comp) for (uint32_t j = 0; j < x->n; ++j) if (KID(h,j) > ROOT) pnum[KID(h,j)] = 1;
  }
  if (size > UINT32_MAX) return 0;
  header[0] = status; memcpy(header + 1, idof(value), 32); put4(header + 33, (uint32_t)size);
  if (!send_bytes(header, sizeof header) || !send_bytes(ice_header, sizeof ice_header)) return 0;
  for (H h = 1; h <= value; ++h) if (pnum[h]) {
    uint64_t mark = ntext; size_t len; uint8_t *record = cbytes(h, &len); put4(header, (uint32_t)len);
    int sent = send_bytes(header, 4) && send_bytes(record, len); ntext = mark; pnum[h] = 0;
    if (!sent) return 0;
  }
  return 1;
}
static int send_outcome(void) {
  return W->outcome == 'n' ? send_answer(1, W->v) : send_answer(W->outcome == 'f' ? 2 : 0, W->reason);
}
static int execute_image(const char *path) {
  size_t len; Status why; uint8_t *raw = load(path, &len); H image = open_image(raw, len, &why);
  if (!image) { send_answer(2, notice(why)); return 1; }
  run(KID(image, 0));
  if (W->outcome != 'n') { send_outcome(); return 1; }
  for (;;) {
    uint8_t count, id[32]; ssize_t n = read(0, &count, 1);
    if (!n) return 0;
    if (n < 0 && errno == EINTR) continue;
    if (n < 0) { send_answer(2, notice(S_SHORT)); return 1; }
    nhand = 0; H *refs = in_hand((uint64_t)count + 1), *args = in_hand((uint64_t)count + 1);
    int missing = 0;
    for (unsigned j = 0; j <= count; ++j) {
      if (!recv_bytes(id, sizeof id)) { send_answer(2, notice(S_SHORT)); return 1; }
      refs[j] = byid(id); if (!refs[j]) missing = 1;
    }
    if (missing) { if (!send_answer(2, notice(S_ID))) return 2; continue; }
    H target = refs[0], top = target;
    int call = is_recipe(target) || (nd[target].comp && nd[target].bp == E(CLOSED));
    if (call || count || bound(target, 0, ~COOL)) {
      uint32_t argc = call ? 1 : 0;
      if (call) args[0] = mk(1, E(QUOTE), &target, 1, &why);
      for (unsigned j = 1; j <= count; ++j) args[argc++] = mk(1, E(QUOTE), refs + j, 1, &why);
      top = mk(1, call ? E(CALL) : target, args, argc, &why);
      if (!top) { if (!send_answer(2, notice(why))) return 2; continue; }
    }
    run(top); if (!send_outcome()) return 2;
  }
}
static int command(int argc, char **argv) {
#ifndef NUMERIC_PORT
  const char *cmd = argc > 1 ? argv[1] : "";
  numeric_io = argc == 3 && !strcmp(cmd, "execute");
#endif
  store_init();
  for (int i = 0; i < A_COUNT; i++) full_at[i] = seen_as(S_FULL, numeric_io ? mki(i) : txt(AREA_NAME[i]));
#ifdef NUMERIC_PORT
  return argc == 2 ? execute_image(argv[1]) : 2;
#else
  if (numeric_io) return execute_image(argv[2]);
  if (argc >= 3 && (!strcmp(cmd, "run") || !strcmp(cmd, "text"))) {
    for (int i = 3; i < argc; i++) { run(file_node(argv[i])); if (W->outcome != 'n') { say("not bound: ", argv[i]); return 1; } }
    run(file_node(argv[2]));
    if (cmd[0] == 't' && W->outcome == 'n' && !nd[W->v].comp) {      /* a leaf answer leaves as its own bytes */
      return send_fd(1, bytes + nd[W->v].off, nd[W->v].n) ? 0 : 2;
    }
    if (W->outcome == 'n') say("node ", print(W->v)); else say(W->outcome == '0' ? "nothing " : "fault ", print(W->reason));
    return W->outcome == 'f' || cmd[0] == 't';
  }
  put(2, "usage: ck execute IMAGE | run F [B..] | text F [B..]\n");
  return 2;
#endif
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
