/* The Seed bootstrap keeps nodes in substrate lane 0 of the shared framebuffer.
 * ck run evaluates the input after its optional preludes. ck text emits a leaf's bytes.
 * seed-run opens a numeric image and accepts requests by content ID.
 * CK_FB publishes a region for observers. Anonymous regions end with their process.
 * A process becomes a member of a named substrate through one door, adopt; share/share.ck does the rest.
 * The host provides virtual memory, files, threads, timing and native dispatch.
 * The node encoding, core operations and framebuffer specs describe the interfaces.
 * region/header.ck derives fb.h from the numeric catalogue and layout.
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
#include <libkern/OSCacheControl.h>
#include <dispatch/dispatch.h>
#include <sys/sysctl.h>
#endif
#include <sys/resource.h>
#ifdef __linux__
#include <sys/statvfs.h>
#endif
#include <sys/stat.h>
#ifdef __APPLE__
#include <mach-o/loader.h>
#include <mach-o/dyld.h>
#endif
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
  void *(*symbol)(const char *name);               /* where the host keeps what it calls by that name */
  void (*guard)(void (*on)(int), void *stack, size_t n);   /* a fault of the processor is handed to on, on its own stack */
  uint64_t (*memory)(void);                        /* available bytes on this host now */
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
static Host host = { h_region, h_code_map, h_code_publish, h_file_read, h_call, h_clock, h_symbol, h_guard, h_memory };

/* ---- the framebuffer: the one region, and where each table lies in it (fb.h) --------
 * These pointers are set once and never move. Every count is a field of the region. */
static Head *fb; static Substrate *sub; static Lane *ln, *lanes;   /* the substrate, this process's lane in it, and every lane */
static int shared; static uint32_t LS = 32; static H lb, *ftab, none;   /* a member of a named substrate: lanes of 2^LS handles each, this one's from lb */
#define LD(x) __atomic_load_n(&(x), __ATOMIC_SEQ_CST)          /* what another member may write is read whole, */
#define ST(x, v) __atomic_store_n(&(x), v, __ATOMIC_RELEASE)   /* written after what it makes reachable, */
#define CAS(x, was, v) __atomic_compare_exchange_n(&(x), was, v, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)   /* and claimed by one */
#define TIDE __atomic_add_fetch(&sub->tide, 1, __ATOMIC_SEQ_CST)   /* the substrate's tide moves before any lane's settled mark rises, so a look at every lane that began */
static __attribute__((noinline)) uint32_t lsw(uint64_t j, uint64_t past) {   /* and ended at one tide saw one moment (opening). How far lane j is settled: what is in the index is */
  uint32_t s = LD(lanes[j].sw); if (s < past) TIDE; while (s < past && !CAS(lanes[j].sw, &s, (uint32_t)past)) {} return s; }   /* settled with all below it in its lane, so who takes it from there says so, past it */
static const uint8_t LANED[A_COUNT] = { [A_NODES] = S_NODES + 1, [A_LEAF_BYTES] = S_LEAF_BYTES + 1, [A_CHILDREN] = S_CHILDREN + 1, [A_IDS] = S_IDS + 1 };   /* in the lane: which area, plus one */
static Node *nd; static uint8_t *bytes, *tb, *text, *tbits;
static H *kids, *tab, *idtab, *stab, *sidt, *seen, *argp, *opened, *tk, *aft, *noted, *hand; static uint32_t *pnum, *bykey; static H *held;
static Frame *fr; static Link *lk; static Activation *act; static ChoicePoint *cp; static Gathered *coll; static Code *born; static Binding *bd, *out; static Trail *trail; static uint8_t *rooms; static RoomTrail *rtrail; static H *gives; static uint8_t *ids;
#define N(i) (fb->a[i].n)
#define SA(s) (A_COUNT + (s))                 /* a substrate area, numbered after the process's own */
#define AR(i) ((i) < A_COUNT ? &fb->a[i] : &sub->a[(i) - A_COUNT])
#define W (&fb->m)
#define sw (ln->sw)                           /* nodes below it are settled: no fail can reach them */
#define scap (sub->a[S_BY_CONTENT].n)
#define sicap (sub->a[S_BY_ID].n)
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
#define ntext N(A_TEXT)
#define nids N(A_IDS)
#define nhand N(A_VALUES)
#define IDREC 36                          /* a node's id: whose it is, and its 32 bytes */
#define NID(h) (ids + (uint64_t)nd[h].ida * IDREC + 4)
static const char *const AREA_NAME[A_COUNT + S_COUNT] = { FB_AREA_NAMES FB_SUBSTRATE_AREA_NAMES };
/* An area's peak is the most it has held, and room goes further only past it. The areas together
 * take what the host can give: each time they have reached a further step of the host's, it is
 * asked how much it can spare, and a growth it has no room for, or one past an area's count, is
 * full. While a program runs that is its fault, full and the area's name, which it can
 * catch; the walker goes on from its loop. Before a run, or while a fault is being handled, it ends
 * the process. */
static sigjmp_buf in_code; static volatile sig_atomic_t coding, walking; static int full, parting, answering;   /* answering: the answer line may take SPARE past the allowance */ static H full_at[A_COUNT + S_COUNT]; static pthread_t walker;
#define SPARE ((uint64_t)16 << 20)
static int reading;                       /* a start is running: what it takes from the host or the process is counted in member.taken */
static void area_full(int a) {            /* how far memory goes follows the process, not the program: a start that met it is not saved */
  fb->me.taken += reading;
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
/* Touched pages can remain resident after backtracking. The peaks already in the region, the
 * substrate's among them, and the pages the tables keyed by handle charged (below) account for
 * them. Charge the whole native stack and both heads up front since they grow without room(). */
static uint64_t used_in(const Head *h, const Substrate *s) {   /* what a region with these heads charges: this process's stack */
  uint64_t used = 2 * FB_PAGE + fb->a[A_STACK].cap + pages_for(h->m.ncode) + h->me.charged * FB_PAGE;
  for (int i = 0; i < A_COUNT + S_COUNT; i++) if (i != A_STACK && (!shared || (i < A_COUNT && !LANED[i]))) {
    const Area *a = i < A_COUNT ? &h->a[i] : &s->a[i - A_COUNT]; used += pages_for(area_extent(a, a->peak)); }
  return used;
}
static uint64_t remaining_memory(void) { uint64_t used = used_in(fb, sub); return used < fb->me.allowance ? fb->me.allowance - used : 0; }
static int fits_growth(uint64_t old, uint64_t next) {
  uint64_t extra = pages_for(next) - pages_for(old);
  return !extra || (extra <= remaining_memory() + (answering ? SPARE : 0) && host_has_growth(old, next));
}
static void further(int a, uint64_t need) {
  Area *area = AR(a); uint64_t old = area_extent(area, area->peak), next = area_extent(area, need), extra = pages_for(next) - pages_for(old); int s = shared && a < A_COUNT && LANED[a];   /* a lane's growth is charged to the named substrate by whole pages: */
  if (s && extra && need <= area->cap) { uint64_t c = LD(sub->charged);    /* asked of what stands and taken by one swap, so a growth that has no room never counts against another's */
    do if ((c + extra > sub->allowance + (answering ? SPARE : 0) && !parting) || !host_has_growth(old, next)) { need = ~0ull; break; } while (!CAS(sub->charged, &c, c + extra)); }   /* but for a member that is leaving */
  if (need > area->cap || (!s && !fits_growth(old, next))) area_full(s ? SA(LANED[a] - 1) : a);
  area->peak = need; if (s) ln->charged += extra;
}
/* What every step runs is kept in the walker's loop and what seldom runs is kept out of it. Left
 * to the compiler's choice, the compiled sweep took 15% more cycles (spec/framebuffer.md). */
#define HOT static inline __attribute__((always_inline))
#define COLD static __attribute__((noinline))
HOT void room(int a, uint64_t need) { if (need > fb->a[a].peak) further(a, need); }      /* one of the process's own areas */
static void sroom(int a, uint64_t need) { if (need > AR(a)->peak) further(a, need); }   /* any area, the substrate's too */
/* Cells, bound-by-node and print-numbers are keyed by handle, so a peak would charge every page
 * below the highest key. A page of theirs is charged when first used instead, a bit for each in
 * touched; a page not yet used holds nothing and is not read, since reading it would take it. */
static uint64_t tbit[A_COUNT];             /* where an area's bits begin in touched */
static uint64_t dense[A_COUNT];            /* keys below it lie on used pages: the hot reads skip the bit */
static uint64_t page_bit(int a, uint64_t k) { return tbit[a] + (fb->a[a].off % FB_PAGE + 4 * k) / FB_PAGE; }
static int on_page(int a, uint64_t k) { uint64_t b = page_bit(a, k); return tbits[b >> 3] >> (b & 7) & 1; }
static void charge(int a, uint64_t k) {
  uint64_t b = page_bit(a, k); if (N(a) <= k) N(a) = k + 1;
  if (!(tbits[b >> 3] >> (b & 7) & 1)) { tbits[b >> 3] |= (uint8_t)(1u << (b & 7)); fb->me.charged++; }
  while (dense[a] < N(a) && on_page(a, dense[a])) dense[a]++;
}
COLD int later(int a, uint64_t k) { return k < N(a) && on_page(a, k); }
#define USED(a, k) ((k) < dense[a] || later(a, k))   /* a key whose page holds what it was given */
static int moved(int a, uint64_t k) {      /* a key that must be taken, in the middle of what cannot stop: whether its page had no room */
  int over = !on_page(a, k) && !fits_growth(fb->me.charged * FB_PAGE, (fb->me.charged + 1) * FB_PAGE); charge(a, k); return over;
}
static void touch(int a, uint64_t k) { if (moved(a, k)) area_full(a); }
/* The limit word an activation runs under counts bindings: 23 bits for the program's own, 7 for
 * those made from outside (see bound, below). */
#define PM 0x7fffffu
#define COOL 0x800000u
#define OUT 0x1000000u
#define OMOST 127u
#define OWN 0x80000000u
static void read_seen(const char *path, uint64_t at, uint64_t n, H name, H content); static int as_saved(uint64_t o, uint64_t n);
static void leaving(void); static void ending(int sig);
static const uint64_t SPAN = (uint64_t)1 << 32;   /* a named substrate: the handles its lanes share */
static Ret adopt(int64_t base, int64_t lane, int64_t name, int64_t leave);
static const struct { uint32_t of, size; uint64_t reserve; } L[A_COUNT + S_COUNT] = { FB_AREA_ROWS };   /* region/layout.ck */
/* Each table has stable addresses into which it can grow, reserved as region/layout.ck says, so the layout is the same
 * in every process and does not follow the memory it may use: that is the allowance, a word of its own. After the
 * process's own areas lies a substrate of one lane: its head, then its areas. The nodes, their parts and their ids lie
 * in its lane, and the process's entries for them point there; its indexes find what is settled, the process's own
 * by-content and by-id what is still working. Last, under a page nothing may touch, the stack, as large as the host's. */
static uint64_t cap[A_COUNT + S_COUNT], off[A_COUNT + S_COUNT], at, share;
static void fb_reset(void) {             /* the region's head as fb_open made it: the start of a cold run */
  for (int i = 0; i < A_COUNT + S_COUNT; i++) *AR(i) = (Area){ E(AREA), 0, E(L[i].of), L[i].size, 0, off[i] - (i < A_COUNT ? 0 : share), cap[i], 0 };
  fb->bp = E(FRAMEBUFFER); fb->version = FB_VERSION; fb->floor = (uint32_t)(off[A_STACK] / FB_PAGE) + 1; fb->areas = A_COUNT; fb->pid = (uint32_t)getpid(); fb->bytes = at; W->bp = E(MACHINE);
  fb->d.symbol = (uint64_t)(uintptr_t)dlsym; fb->d.adopt = (uint64_t)(uintptr_t)adopt; fb->d.region = (uint64_t)(uintptr_t)fb;
  sub->bp = E(SUBSTRATE); sub->version = FB_VERSION; sub->areas = S_COUNT; sub->lanes = 1; sub->pid = fb->pid; sub->share = fb->me.share = share; sub->bytes = at - share; sub->catalogue = FB_CATALOGUE;
  nn = 1; nids = IDREC;                   /* the first id lies after a record of none */
  *ln = (Lane){ E(LANE), 0, (uint64_t)fb->pid << 42 | 2, 0, 1, 1, 0, 0, {0} }; sroom(SA(S_LANES), 1); sub->a[S_LANES].n = 1;   /* lane 0 is this process's, nothing settled yet */
  room(A_SEEN, 1); N(A_SEEN) = 1; room(A_TOUCHED, cap[A_TOUCHED]); N(A_TOUCHED) = cap[A_TOUCHED];
}
static void fb_open(void) {
  uint64_t budget = host.memory(), bits = 0; struct rlimit limit;
  if (!getrlimit(RLIMIT_AS, &limit) && limit.rlim_cur < budget) budget = limit.rlim_cur;
  const char *setting = getenv("CK_MEMORY_BYTES");
  if (setting) {
    char *end; errno = 0; unsigned long long requested = strtoull(setting, &end, 10);
    if (*setting < '0' || *setting > '9' || *end || errno || !requested) die("CK_MEMORY_BYTES takes a positive byte count");
    if (requested < budget) budget = requested;
  }
  uint64_t mem = budget / FB_PAGE * FB_PAGE;
  if (mem < (1u << 17) + 4 * FB_PAGE) die("insufficient host memory for the framebuffer");
  for (int i = 0; i < A_COUNT + S_COUNT; i++)   /* what region/layout.ck reserves, whatever this host or its allowance: one layout on every host */
    cap[i] = L[i].reserve > FB_BITS ? L[i].reserve / L[i].size : 0;
  for (int i = 0; i < A_COUNT; i++) {
    if (LANED[i]) cap[i] = cap[SA(LANED[i] - 1)];
    if (L[i].reserve == FB_HANDLES) { cap[i] = getenv("CK_SHARE") ? SPAN : cap[SA(S_NODES)] + 8;   /* one for each handle: of the lane, or of any lane */
      tbit[i] = bits; bits += cap[i] * 4 / FB_PAGE + 2; } }                                           /* where the process may become a member */
  cap[A_BINDINGS] = cap[A_BINDINGS] < PM ? cap[A_BINDINGS] : PM;   /* numbered inside a limit word: 23 bits for the program's, 7 from outside */
  cap[A_TOUCHED] = (bits + 7) / 8;
  pthread_attr_t attr; size_t stack;
  if (pthread_attr_init(&attr) || pthread_attr_getstacksize(&attr, &stack)) die("cannot size the host stack");
  pthread_attr_destroy(&attr);
  if (!getrlimit(RLIMIT_STACK, &limit) && limit.rlim_cur != RLIM_INFINITY && limit.rlim_cur > stack) stack = limit.rlim_cur;
  cap[A_STACK] = pages_for(stack) < mem / 2 ? pages_for(stack) : mem / 2 / FB_PAGE * FB_PAGE;
  if (cap[A_STACK] < (uint64_t)PTHREAD_STACK_MIN) die("insufficient host memory for the stack");
  at = FB_PAGE;
  for (int i = 0; i <= A_COUNT + S_COUNT; i++) {
    int a = i == A_COUNT + S_COUNT ? A_STACK : i;
    if (i == A_STACK || (a < A_COUNT && LANED[a])) continue;   /* the stack comes last; a laned table lies in the lane, below */
    if (a == A_STACK) at += FB_PAGE;                      /* a page nothing may touch, under the stack */
    if (a == SA(0)) { share = at; at += FB_PAGE; }        /* the substrate's head */
    uint64_t hue = a == A_STACK ? 0 : FB_HUE * (uint64_t)a;   /* each area's first records on a */
    off[a] = at + hue; at += (L[a].size * cap[a] + hue + FB_PAGE - 1) / FB_PAGE * FB_PAGE;     /* cache set no other's first use */
  }
  for (int i = 0; i < A_COUNT; i++) if (LANED[i]) off[i] = off[SA(LANED[i] - 1)];
  if (!(fb = host.region(getenv("CK_FB"), at))) die("cannot make the framebuffer");
  sub = (Substrate *)((uint8_t *)fb + share);
#define AT(i) ((void *)((uint8_t *)fb + off[i]))
  nd = AT(A_NODES); bytes = AT(A_LEAF_BYTES); kids = AT(A_CHILDREN); tab = AT(A_BY_CONTENT); idtab = AT(A_BY_ID); fr = AT(A_FRAMES);
  lk = AT(A_LINKS); act = AT(A_ACTIVATIONS); argp = AT(A_ARGUMENTS); cp = AT(A_CHOICE_POINTS); coll = AT(A_GATHERED); born = AT(A_BORN_CODE); bd = AT(A_BINDINGS); out = AT(A_OUTSIDE); aft = AT(A_AFTER); noted = AT(A_NOTED); bykey = AT(A_BOUND_BY_NODE); held = AT(A_CELLS); trail = AT(A_TRAIL); rooms = AT(A_ROOMS); rtrail = AT(A_ROOM_TRAIL);
  opened = AT(A_READER_OPENED); tk = AT(A_GATHERING_CHILDREN); tb = AT(A_GATHERING_BYTES); pnum = AT(A_PRINT_NUMBERS); text = AT(A_TEXT); gives = AT(A_GIVES); ids = AT(A_IDS); hand = AT(A_VALUES);
  seen = AT(A_SEEN); tbits = AT(A_TOUCHED); stab = AT(SA(S_BY_CONTENT)); sidt = AT(SA(S_BY_ID)); ln = lanes = AT(SA(S_LANES));
  mprotect((uint8_t *)fb + off[A_STACK] - FB_PAGE, FB_PAGE, PROT_NONE);
  fb->me.allowance = sub->allowance = mem; fb_reset();
}
#define PLACE(i) ((void *)((uint8_t *)fb + fb->a[i].off))
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
  uint64_t extent = pages_for(area_extent(text_area, text_area->peak)) + remaining_memory() + (answering ? SPARE : 0);
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
#define NUMERIC_PORT_FLAG 1
#else
#define NUMERIC_PORT_FLAG 0
static int numeric_io;
#endif
static H notice(Status status); static H make_far(int comp, H bp, const void *payload, uint64_t n);
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
static void tab_put(H *t, uint64_t c, H h) {
  uint64_t i = hash(nd[h].comp, nd[h].bp, payload_of(h), nd[h].n) & (c - 1);
  while (t[i]) i = (i + 1) & (c - 1);
  t[i] = h;
}
static void regrow(int a, H *t, H from, H to, uint64_t want) {   /* a table by content made again for want, with [from, to) */
  uint64_t c = AR(a)->n ? AR(a)->n : 4096; while (want * 2 > c) c *= 2;
  sroom(a, c); memset(t, 0, (size_t)c * sizeof *t); AR(a)->n = c;
  for (H h = from; h < to; h++) tab_put(t, c, h);
}
static inline H found(const H *t, uint64_t c, uint64_t *i, int comp, H bp, const void *payload, uint64_t n, int far) {   /* or where it would go; */
  size_t sz = comp ? (size_t)n * sizeof(H) : n; H v;                                                  /* far: other members write this table */
  for (*i &= c - 1; (v = far ? LD(t[*i]) : t[*i]); *i = (*i + 1) & (c - 1)) { const Node *x = &nd[v];
    if (x->comp == comp && x->bp == bp && x->n == n && !memcmp(payload_of(v), payload, sz)) return v; }
  return 0;
}
/* A search sees of another lane what was settled when it began (opening, below), so going an option again meets the same nodes. now: none runs. */
COLD int seenp(H h, int now) { uint64_t j = (uint64_t)h >> LS; if (j == fb->me.lane) return 1; if (now) lsw(j, (uint64_t)h + 1); return now || h < seen[j]; }
static H known(int64_t v) { H h = (H)v; uint64_t j = (uint64_t)h >> LS;   /* the node a number names here: one of this lane's, or a settled one of another's this run may see */
  return v > 0 && v >> 32 == 0 && ((h >= lb && h < nn) || (shared && j != fb->me.lane && h < (ncp || nn > sw ? seen[j] : lsw(j, 0)))) ? h : 0;
}
COLD H met(uint64_t *s, int comp, H bp, const void *payload, uint64_t n) { return found(ftab, sub->a[S_BY_CONTENT].cap, s, comp, bp, payload, n, 1); }   /* in a named substrate's index */
COLD H publish(H h, uint64_t s) { Node *x = &nd[h]; H o, e = 0; x->settled = 1;   /* a settled record, whole, enters a named substrate's index by one swap; equal content met there answers */
  for (;; e = 0) { if ((o = met(&s, x->comp, x->bp, payload_of(h), x->n)) && o != h) { x->settled = 0; seenp(o, 1); return o; } if (o || CAS(ftab[s], &e, h)) return h; }
}
/* The one node with this content. Refused content sets *why and answers nothing. A node made while
 * no choice is open, and none made under one still stands, is settled at once: no fail can reach
 * it. Any other is working and found by the process's own index until a commit settles it (walk).
 * A settled node's parts are settled, so the settled index is asked only when this node's are.
 * A new record is whole before nn moves past it; its writer is the node now running. */
HOT H make(int comp, H bp, const void *payload, uint64_t n, Status *why, const int far) {     /* far: in a named substrate */
  if (!far && (bp || nn > 1)) { if ((*why = rule(comp, bp, payload, n))) return 0; }     /* a member comes here from the other, past its rule */
  size_t sz = comp ? (size_t)n * sizeof(H) : n; int settle = !ncp && sw == nn, parts = bp < sw || (far && bp >= nn); H h;
  if (settle && !far && (!scap || nn * 2 > scap)) { if (shared) return make_far(comp, bp, payload, n); regrow(SA(S_BY_CONTENT), stab, 1, sw, nn); }
  if (!settle && (!tcap || (nn - sw + 1) * 2 > tcap)) regrow(A_BY_CONTENT, tab, sw, nn, nn - sw + 1);
  uint64_t i = hash(comp, bp, payload, n), s = i;
  if (!settle && (h = found(tab, tcap, &i, comp, bp, payload, n, 0))) return h;
  for (uint64_t j = 0; !settle && comp && parts && j < n; j++) parts = ((const H *)payload)[j] < sw || (far && ((const H *)payload)[j] >= nn);
  if (parts && (far ? (h = met(&s, comp, bp, payload, n)) && seenp(h, settle) : (h = found(stab, scap, &s, comp, bp, payload, n, 0)) != 0)) return h;
  if (!far && !settle && scap == 1) return make_far(comp, bp, payload, n);    /* a member: the settled index a process alone asks is one empty place for it */
  room(A_NODES, nn + 1); Node *x = &nd[nn]; memset(x, 0, sizeof *x);
  x->bp = bp; x->by = W->at; x->comp = (uint8_t)comp; x->n = n; x->settled = (uint8_t)settle;
  if (comp) { room(A_CHILDREN, nkids + n); x->off = nkids; memcpy(kids + nkids, payload, sz); nkids += n; }
  else { room(A_LEAF_BYTES, nbytes + n); x->off = nbytes; memcpy(bytes + nbytes, payload, sz); nbytes += n; }
  if (!settle) tab[i] = (H)nn; else if (!far) stab[s] = (H)nn; else if ((h = publish((H)nn, s)) != nn) { if (comp) nkids -= n; else nbytes -= n; return h; }   /* its place is free again: none saw it */
  h = (H)nn++; if (settle && !far) sw = (uint32_t)nn; else if (far) { ST(ln->n, (uint32_t)nn); if (settle) { TIDE; ST(sw, (uint32_t)nn); } } return h;
}
COLD H make_far(int comp, H bp, const void *payload, uint64_t n) { return make(comp, bp, payload, n, NULL, 1); }   /* two makers, so that a process alone runs what */
static H mk(int comp, H bp, const void *payload, uint64_t n, Status *why) { return make(comp, bp, payload, n, why, 0); }   /* it ran; a member's leaves that one where it asks the settled index */
#define KID(x, i) (kids[nd[x].off + (i)])
#define TOMB 0xffffffffu                  /* a place in the table by id whose node was given back */
static uint64_t idstart(const uint8_t *id, uint64_t c) { uint64_t u; memcpy(&u, id, 8); return u & (c - 1); }
static uint64_t idslot(const H *t, uint64_t c, const uint8_t *id) {
  uint64_t i = idstart(id, c);
  while (t[i] && (t[i] == TOMB || memcmp(NID(t[i]), id, 32))) i = (i + 1) & (c - 1);
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
/* A table by id made again with only live places, larger while half would be taken: the settled one
 * over the settled nodes, or the process's own over its working nodes. */
static void idmake(int a, H *t, H from, H to, H skip) {
  uint64_t live = 0, c = AR(a)->n ? AR(a)->n : 4096;
  for (H x = from; x < to; x++) live += nd[x].ida && x != skip;
  while ((live + 1) * 2 >= c) c *= 2;
  sroom(a, c); memset(t, 0, (size_t)c * sizeof *t); AR(a)->n = c;
  if (a == A_BY_ID) W->ids = (uint32_t)live; else sub->ids = live;
  for (H x = from; x < to; x++) if (nd[x].ida && x != skip) t[idslot(t, c, NID(x))] = x;
}
/* A node's id lies beside it, in the area of ids, only once it is asked for. The table by id that
 * finds it is the settled one or the process's own, as the node is; a place is counted when taken. */
static void index_id(H h) {
  int s = nd[h].settled; H *t = s ? sidt : idtab, e = 0;
  if (s && shared) { for (uint64_t j; !LD(sidt[j = idslot(sidt, sicap, NID(h))]) && !CAS(sidt[j], &e, h); e = 0) {} return; }   /* made once: a place is swapped in */
  if (s ? !sicap || sub->ids * 2 >= sicap : !icap || (uint64_t)W->ids * 2 >= icap) s ? idmake(SA(S_BY_ID), sidt, 1, sw, h) : idmake(A_BY_ID, idtab, sw, nn, h);
  uint64_t j = idslot(t, s ? sicap : icap, NID(h));
  if (!t[j]) { if (s) sub->ids++; else W->ids++; }      /* ids counts the places taken, given back ones too */
  t[j] = h;
}
static void setid(H h, const uint8_t *id) {
  if (nd[h].ida) { if (shared && nd[h].settled) index_id(h); return; }     /* whoever gave it its id may have gone before it was found by it */
  room(A_IDS, nids + IDREC); memcpy(ids + nids, &h, 4); memcpy(ids + nids + 4, id, 32); uint32_t z = 0, r = (uint32_t)(nids / IDREC);
  if (shared ? CAS(nd[h].ida, &z, r) : (nd[h].ida = r, 1)) nids += IDREC; index_id(h);     /* the record is whole before one swap gives it to the node */
}
static const uint8_t *idof(H h) {         /* section 3 */
  if (!nd[h].ida) { uint64_t m = ntext; size_t len; uint8_t *b = cbytes(h, &len), id[32]; sha256(b, len, id); ntext = m; setid(h, id); }
  return NID(h);
}
static int catalogued;
static void catalogue_ids(void) { if (!catalogued) { catalogued = 1; for (H h = 1; h <= SEEDED; h++) idof(h); } }   /* where a node is found by its id */
static H byid(const uint8_t *id) {        /* a working node first, then a settled one */
  H h = icap ? idtab[idslot(idtab, icap, id)] : 0; if (!h && sicap) h = sidt[idslot(sidt, sicap, id)];
  return h && shared && !seenp(h, !ncp && nn == sw) ? 0 : h;          /* one a search may not see is not there for it */
}
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
  catalogue_ids(); H bp = byid(b + 1), h; if (!bp) { *why = S_UNKNOWN; return 0; }
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
  memcpy(sub->magic, FB_SUBSTRATE_MAGIC, 8); memcpy(fb->magic, FB_MAGIC, 8);   /* from here a reader can make sense of the region */
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

/* ---- section 6: stored nodes, read --------------------------------------------------------
 * spec/node-encoding.md, section 8: CKNODES and a line end, the format word 1, how many records and how
 * many bytes they take, then each record: its kind, its blueprint, how many parts, then its bytes or its parts. A part is
 * an entry below 128, or 128 more than an earlier record. Each node is one record, written once; the last is the node,
 * and what follows the records lies beside it. The canonical print is read and written in form/read.ck and form/print.ck. */
static const uint8_t NODES_MAGIC[8] = { 'C', 'K', 'N', 'O', 'D', 'E', 'S', '\n' };
#define LE4(p) ((uint32_t)(p)[0] | (uint32_t)(p)[1] << 8 | (uint32_t)(p)[2] << 16 | (uint32_t)(p)[3] << 24)
static H read_nodes(const uint8_t *b, uint64_t len, Status *why) {
  uint64_t count = len >= 20 ? LE4(b + 12) : 0, size = len >= 20 ? LE4(b + 16) : 0, at = 20; H x = 0; nopen = 0;
  if (len < 8 || memcmp(b, NODES_MAGIC, 8)) { *why = S_SYNTAX; return 0; }
  if (len < 20 || LE4(b + 8) != 1 || size > len - 20 || !count || count > size / 9) { *why = S_SHORT; return 0; }
  room(A_READER_OPENED, count); *why = S_OK;
  for (uint64_t i = 0; i < count && !*why; i++) {
    if (size + 20 - at < 9 || b[at] > 1) { *why = size + 20 - at < 9 ? S_SHORT : S_KIND; break; }
    uint32_t comp = b[at], r = LE4(b + at + 1), n = LE4(b + at + 5), base = ntk; at += 9;
    if (size + 20 - at < (uint64_t)n * (comp ? 4 : 1)) { *why = S_SHORT; break; }
    H refs[2] = { r, 0 }; const H *part = refs;
    if (comp) { room(A_GATHERING_CHILDREN, ntk + n); for (uint32_t j = 0; j < n; j++) tk[ntk + j] = LE4(b + at + 4 * j); part = tk + ntk; ntk += n; }
    for (uint32_t j = 0; j < (comp ? n : 0) + 1 && !*why; j++) {   /* the blueprint, then each part, from a number to a node */
      H *p = j ? &tk[base + j - 1] : &refs[0], v = *p;
      *p = v < SEEDED ? E(v) : v - SEEDED < nopen ? opened[v - SEEDED] : 0; if (!*p) *why = S_FORWARD; }
    if (!*why && (x = mk((int)comp, refs[0], comp ? (const void *)part : b + at, n, why))) {
      touch(A_PRINT_NUMBERS, x); if (x <= SEEDED || pnum[x]) { *why = S_NOT_CANONICAL; x = 0; } else { pnum[x] = 1; opened[nopen++] = x; } }
    ntk = base; at += (uint64_t)n * (comp ? 4 : 1);
  }
  for (uint32_t j = 0; j < nopen; j++) pnum[opened[j]] = 0;
  if (!*why && at != size + 20) *why = S_TRAILING;
  return *why ? 0 : x;
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
  uint32_t i = USED(A_BOUND_BY_NODE, e) ? bykey[e] : 0;
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
static int is_data(long long e) { return e < QUOTE || e == RECIPE || (e > NATIVE_OP && e < BLUEPRINT) || e == BINDING || e == LAZY || e == TRAIL || e == CLOSED || e == ROOM || e == ROOM_TRAIL || e == SUBSTRATE || e == LANE; }
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
#define FW(x) do { H f_ = (x); if (f_ >= fm && f_ < ftop && fw[f_ - fm]) (x) = fw[f_ - fm]; } while (0)   /* a commit's forwarding: only what met equal content moves */
static void root(H h) { if (h >= fm && h < ftop) fw[h - fm] = 1; }
static H fwd(H h) { return h >= fm && h < ftop ? fw[h - fm] : h; }
static H byfwd(H h) { return h >= fm && h < ftop ? (fw[h - fm] ? fw[h - fm] : fnode) : h; }
static uint64_t roomlen(uint64_t at) { return sizeof(Room) + ((((Room *)(rooms + at))->size + 7) & ~7ull); }
static void reclaim(ChoicePoint *q) {
  fm = q->mnode; ftop = nn; fnode = q->node;
  if (sw > fm) die("a fail would give back settled nodes");
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
    else if (icap && code_target(h)) for (uint64_t o = 0; o + 32 <= nd[h].n; o++) { H r = byid(bytes + nd[h].off + o); if (r && r != TOMB) root(r); }
  }
  for (uint32_t i = q->mtrail; i < ntrail; i++) if (trail[i].bp == E(NOTE) && trail[i].key >= fm && trail[i].key < ftop) fw[trail[i].key - fm] |= 2;
  H to = fm; uint64_t pages = 0, last = ~0ull;   /* where each kept node goes, and each page not yet used that */
  for (H h = fm; h < ftop; h++) if (fw[h - fm]) {   /* a cell moves onto, held or noted, counted once */
    if ((fw[h - fm] & 2 || (h < N(A_CELLS) && on_page(A_CELLS, h) && held[h])) && !on_page(A_CELLS, to) && page_bit(A_CELLS, to) != last) {
      pages++; last = page_bit(A_CELLS, to); }
    fw[h - fm] = to++; }
  if (pages && !fits_growth(fb->me.charged * FB_PAGE, (fb->me.charged + pages) * FB_PAGE)) { ntext = mark; return; }
  room(A_GIVES, 4);                       /* nothing past here can be full until every kept node is put back */
  for (H h = ftop; h-- > fm; ) {          /* the working index forgets them all, the newest first as linear probing needs */
    uint64_t i = hash(nd[h].comp, nd[h].bp, payload_of(h), nd[h].n) & (tcap - 1);
    while (tab[i] && tab[i] != h) i = (i + 1) & (tcap - 1);
    if (tab[i] == h) tab[i] = 0;
    if (nd[h].ida) {
      for (i = idstart(NID(h), icap); idtab[i] && idtab[i] != h; i = (i + 1) & (icap - 1)) ;
      if (idtab[i] == h) idtab[i] = TOMB; }
    if (h < N(A_CELLS) && !fw[h - fm] && on_page(A_CELLS, h)) held[h] = 0;
    if (h < N(A_BOUND_BY_NODE) && on_page(A_BOUND_BY_NODE, h)) bykey[h] = 0;
    if (h < N(A_PRINT_NUMBERS) && on_page(A_PRINT_NUMBERS, h)) pnum[h] = 0;
  }
  gives[0]++; gives[1] = q->node; gives[2] = fm; gives[3] = ftop; N(A_GIVES) = 4;
  nn = fm; if (shared) ST(ln->n, fm);     /* a watcher sees that places were given back, and none half moved */
  for (H h = fm; h < ftop; h++) {         /* each kept node moves down, its parts and cell with it */
    H n = fw[h - fm]; if (!n) continue;
    Node x = nd[h]; x.bp = fwd(x.bp); x.by = byfwd(x.by);
    if (x.comp) { for (uint64_t i = 0; i < x.n; i++) kids[k + i] = fwd(kids[x.off + i]); x.off = k; k += x.n; }
    else { memmove(bytes + b, bytes + x.off, x.n); x.off = b; b += x.n; }
    nd[n] = x;
    if (h < N(A_CELLS) && on_page(A_CELLS, h)) { H v = held[h]; held[h] = 0; if (v) { charge(A_CELLS, n); held[n] = fwd(v); } }
  }
  nn = to; nkids = k; nbytes = b; if (shared) ST(ln->n, to);
  uint64_t w = q->mids;                   /* ids made since stay for the nodes that stay, moved down with them */
  for (uint64_t r = q->mids; r < nids; r += IDREC) {
    H o; memcpy(&o, ids + r, 4); H n = fwd(o);
    if (!n || (shared && (o < fm || o >= ftop) && (w = r + IDREC))) continue;   /* a settled node's id stays where every member reads it */
    if (w != r) memmove(ids + w, ids + r, IDREC);
    memcpy(ids + w, &n, 4); nd[n].ida = (uint32_t)(w / IDREC); w += IDREC;
  }
  nids = w;
  for (H h = fm; h < nn; h++) { tab_put(tab, tcap, h);   /* back where they were found, in places they had */
    if (nd[h].ida) { uint64_t j = idslot(idtab, icap, NID(h)); W->ids += !idtab[j]; idtab[j] = h; } }
  if (icap && (uint64_t)W->ids * 2 >= icap) idmake(A_BY_ID, idtab, sw, nn, 0);
  if (q->all == 2) q->coll = fwd(q->coll);
  if (q->all == 3) { q->i = fwd(q->i); q->coll = fwd(q->coll); }
  W->v = fwd(W->v); W->reason = fwd(W->reason);
  for (uint32_t i = q->mcoll; i < ncoll; i++) { coll[i].v = fwd(coll[i].v); coll[i].by = byfwd(coll[i].by); }
  for (uint32_t i = q->mtrail; i < ntrail; i++) {
    Trail *e = &trail[i]; e->old = fwd(e->old); e->by = byfwd(e->by);
    if (e->bp == E(NOTE)) { e->key = fwd(e->key); charge(A_CELLS, e->key); }   /* its note is written there */
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
/* Everything the run holds is forwarded, as a fail's giving back forwards what it keeps: no choice is open, so no fail is left for the trail to serve. */
static void forwarded(void) {
  FW(W->x); FW(W->v); FW(W->reason); FW(W->at); ntrail = 0;
  for (uint32_t i = 1; i < nfr; i++) { FW(fr[i].node); FW(fr[i].by); if (fr[i].t == F_ASIDE) FW(fr[i].vals); }
  for (uint32_t i = 1; i < nlink; i++) { FW(lk[i].v); FW(lk[i].by); } for (uint32_t i = 0; i < nact; i++) { FW(act[i].recipe); FW(act[i].by); } for (uint32_t i = 0; i < nargp; i++) FW(argp[i]);
  for (uint32_t i = 0; i < ncoll; i++) { FW(coll[i].v); FW(coll[i].by); } for (uint32_t i = 0; i < nborn; i++) { FW(born[i].leaf); FW(born[i].by); }
  for (uint32_t i = 0; i < nbind + nout; i++) { Binding *b = i < nbind ? &bd[i] : &out[i - nbind]; FW(b->entry); FW(b->impl); FW(b->fast); FW(b->by); }
  for (uint64_t j = 0; j < nafter; j += (uint64_t)aft[j] + 1) for (uint32_t k = 1; k <= aft[j]; k++) FW(aft[j + k]);
  for (uint64_t at = 0; at < N(A_ROOMS); at += roomlen(at)) { FW(((Room *)(rooms + at))->bp); FW(((Room *)(rooms + at))->by); }
  for (uint64_t k = 0, o = fb->a[A_CELLS].off % FB_PAGE / 4; k < N(A_CELLS); k++) { if (!on_page(A_CELLS, k)) k += 4095 - (k + o) % 4096; else if (held[k]) FW(held[k]); }
}
/* No choice is open: no fail can reach what was made under one, and it is settled where it stands,
 * the newest first as the working index forgets it. In a lone process nothing settles while working
 * nodes stand but this, and mk found no equal settled content for any of them; finding one here would
 * need the working state forwarded onto it, which is not built, so it ends the process. */
COLD void commit(void) {
  H from = sw, top = (H)nn; uint64_t m = 0, mark = ntext; int any = 0, over = 0;
  if (shared) { fm = from; ftop = top; fw = talloc((size_t)(top - from) * 4); }   /* where each goes that meets equal content; taken, like the room */
  else if (nn * 2 > scap) regrow(SA(S_BY_CONTENT), stab, 1, sw, nn);   /* both settled indexes have for them, before any moves, so a full leaves */
  for (H h = from; !shared && h < top; h++) m += nd[h].ida != 0;       /* nothing half done */
  if (m && (!sicap || (sub->ids + m) * 2 >= sicap)) idmake(SA(S_BY_ID), sidt, 1, top, 0);
  for (H h = top; h-- > from; ) {
    uint64_t i = hash(nd[h].comp, nd[h].bp, payload_of(h), nd[h].n), s = i;
    for (i &= tcap - 1; tab[i] && tab[i] != h; i = (i + 1) & (tcap - 1)) ;
    if (tab[i] == h) tab[i] = 0;
    if (!shared) { if (found(stab, scap, &s, nd[h].comp, nd[h].bp, payload_of(h), nd[h].n, 0)) die("a commit met content already settled");
      nd[h].settled = 1; stab[s] = h; }
    if (nd[h].ida) { if (!shared) index_id(h); for (i = idstart(NID(h), icap); idtab[i] && idtab[i] != h; i = (i + 1) & (icap - 1)) ;
      if (idtab[i] == h) idtab[i] = TOMB; }
  }
  /* A member settles oldest first, since another may have settled equal content since the search began. A node that
   * meets it is forwarded to the settled one and gives its place back: those made after it move down, as a fail's
   * giving back moves what it keeps, with their parts, their cells, their bindings and their ids, and are settled
   * where they come to lie. k and b: where the next kept children and bytes go, once a node of each kind was met. */
  H to = from; uint64_t k = ~0ull, b = ~0ull;
  for (H h = from; shared && h < top; h++) {
    Node x = nd[h]; H o; uint32_t z = 0; uint64_t *c = x.comp ? &k : &b;
    if (*c == ~0ull) *c = x.off;
    if (any) {
      FW(x.bp); FW(x.by); for (uint64_t i = 0; x.comp && i < x.n; i++) FW(kids[x.off + i]);
      if (x.comp) memmove(kids + k, kids + x.off, (size_t)x.n * sizeof(H)); else memmove(bytes + b, bytes + x.off, x.n);
      x.off = *c; nd[to] = x;
    }
    if ((o = publish(to, hash(x.comp, x.bp, payload_of(to), x.n))) == to) *c += x.n; else ln->forwarded++;
    if (o != h) {
      if (!any) { any = 1; room(A_GIVES, 4); gives[0]++; gives[1] = 0; gives[2] = fm; gives[3] = ftop; N(A_GIVES) = 4; nn = to; ST(ln->n, to); }   /* a watcher sees places given back */
      fw[h - fm] = o;
      if (USED(A_CELLS, h) && held[h]) { over |= moved(A_CELLS, o); held[o] = held[h]; held[h] = 0; }
      if (USED(A_BOUND_BY_NODE, h) && bykey[h]) { over |= moved(A_BOUND_BY_NODE, o); bykey[o] = bykey[h]; bykey[h] = 0; }
      if (x.ida) memcpy(ids + (uint64_t)x.ida * IDREC, &o, 4);
    }
    if (x.ida) { if (o != to) CAS(nd[o].ida, &z, x.ida); index_id(o); }
    to += o == to;
  }
  if (any) { nn = to; if (k != ~0ull) nkids = k; if (b != ~0ull) nbytes = b; ST(ln->n, to); top = to; }
  if (any) forwarded();
  ntext = mark; if (shared) { TIDE; ST(sw, top); } else sw = top;
  if (over) area_full(A_CELLS);            /* the cells a forwarding moved took pages past the allowance: all is forwarded, and the run is told full */
}
/* The one door by which a process alone becomes a member of a named substrate; share/share.ck does everything else through the
 * host's rows. The substrate mapped at base becomes its substrate and lane its lane of it. Every node it had made is made
 * again there, oldest first, by the maker all a member's nodes come from, so content the substrate already holds answers with
 * the handle every member knows it by; then all the run holds is forwarded onto those. Lane 0 takes the catalogue with it, and
 * any other finds it there. Once begun it cannot stop half way: a fault or a full inside it ends the process. */
static Ret adopt(int64_t base, int64_t lane, int64_t name, int64_t leave) {
  Substrate *to = (Substrate *)(intptr_t)base; Ret no = { 0, 0 };
  if (shared || ncp || coding != 1 || !to || !name || to->version != FB_VERSION || lane < 0 || lane >= to->lanes || fb->a[A_CELLS].cap < SPAN) return no;   /* not from inside code, which holds handles */
  if (lane && (W->handeds > 1 || (W->handeds && W->handed != (H)leave))) return no;   /* a number given as a handle would name another node after: refused, not guessed */
  if (nn > sw) commit();
  const Node *ond = nd; const H *okids = kids; const uint8_t *obytes = bytes, *oids = ids; H top = (H)nn, at = W->at; uint64_t mark = ntext, base_tk = ntk; int was = coding, over = 0;
  coding = walking = 0; sub = to; shared = 1; fb->me.share = (uint64_t)base - (uint64_t)(uintptr_t)fb; fb->me.lane = (uint32_t)lane; strncpy((char *)fb->me.name, (const char *)(intptr_t)name, 31);
  LS = (uint32_t)__builtin_clz(sub->lanes) + 1; lb = (H)((uint64_t)lane << LS);
  for (int i = 0; i < A_COUNT; i++) if (LANED[i]) { Area *a = &sub->a[LANED[i] - 1]; uint64_t span = a->cap / sub->lanes;   /* the process's entry for a table in its lane: */
    fb->a[i] = *a; fb->a[i].off += fb->me.share; fb->a[i].n = fb->a[i].peak = span * (uint64_t)lane; fb->a[i].cap = span * (uint64_t)(lane + 1) - (i == A_NODES); }   /* the lane's part of it */
  nd = PLACE(A_NODES); bytes = PLACE(A_LEAF_BYTES); kids = PLACE(A_CHILDREN); ids = PLACE(A_IDS); stab = &none;
  ftab = (H *)((uint8_t *)sub + sub->a[S_BY_CONTENT].off); sidt = (H *)((uint8_t *)sub + sub->a[S_BY_ID].off); lanes = (Lane *)((uint8_t *)sub + sub->a[S_LANES].off); ln = lanes + lane;
  if (!lane) { nn = 1; nids = IDREC; }    /* handle 0 is nothing, and the first id lies after a record of none */
  ST(ln->n, (uint32_t)nn); ST(sw, (uint32_t)nn); room(A_SEEN, sub->lanes); N(A_SEEN) = sub->lanes;
  fm = lane ? SEEDED + 1 : 1; ftop = top; fw = talloc((size_t)(top - fm) * 4); H same = fm, first = lsw(0, 0);
  for (const Node *x, *y; lane && same < top && same < first && (x = &ond[same])->bp == (y = &nd[same])->bp && x->comp == y->comp && x->n == y->n   /* what it made as the */
         && !memcmp(x->comp ? (const void *)(okids + x->off) : (const void *)(obytes + x->off), payload_of(same), x->comp ? (size_t)x->n * 4 : x->n); same++) {}   /* maker did, in the maker's order, */
  for (H h = fm; h < top; h++) {          /* lies in the first lane at the handle it had. A node is newer than its parts, so each other is made from parts already forwarded */
    Node x = ond[h]; const void *p = obytes + x.off; H o = h;
    if (x.comp && h >= same) { room(A_GATHERING_CHILDREN, ntk + x.n); for (uint64_t i = 0; i < x.n; i++) tk[ntk + i] = fwd(okids[x.off + i]); p = tk + ntk; }
    if (h >= same) { W->at = fwd(x.by); o = make_far(x.comp, fwd(x.bp), p, x.n); } fw[h - fm] = o;
    if (!lane && o != h) die("the substrate's first lane did not take the catalogue where it lay");
    if (x.ida) setid(o, oids + (uint64_t)x.ida * IDREC + 4);
    for (int a = A_BOUND_BY_NODE; a <= A_CELLS; a += A_CELLS - A_BOUND_BY_NODE) {   /* its bindings and its cell go with it once every node is made: a new handle */
      H *t = a == A_CELLS ? held : (H *)bykey;                                    /* may be the old one of a node not yet reached */
      if (USED(a, h) && t[h]) { room(A_GATHERING_CHILDREN, ntk + 3); tk[ntk++] = (H)a; tk[ntk++] = o; tk[ntk++] = t[h]; t[h] = 0; } }
  }
  for (; ntk > base_tk; ntk -= 3) { int a = (int)tk[ntk - 3]; over |= moved(a, tk[ntk - 2]); (a == A_CELLS ? held : (H *)bykey)[tk[ntk - 2]] = tk[ntk - 1]; }
  W->at = at; forwarded(); for (int i = 0; i < A_COUNT + S_COUNT; i++) FW(full_at[i]);
  fb->me.leave = leave > 0 && leave < top ? fwd((H)leave) : 0; atexit(leaving);   /* what the member runs last, however its run ends: */
  signal(SIGTERM, ending); signal(SIGINT, ending); signal(SIGHUP, ending);        /* at a signal that would end it, too */
  ntext = mark; walking = 1; coding = was; if (over) area_full(A_CELLS);
  return (Ret){ 1, 0 };
}
/* A choice point is about to be made. The first one open keeps in seen how far each lane was settled
 * when the search began: one moment for all of them, so the lanes are looked at again when the tide moved meanwhile. */
static void opening(void) { uint64_t t; room(A_CHOICE_POINTS, (uint64_t)ncp + 1); if (ncp) return;
  if (!shared) seen[0] = sw; else do { t = LD(sub->tide); for (uint64_t j = LD(sub->a[S_LANES].n); j-- > 0; ) seen[j] = lsw(j, 0); } while (t != LD(sub->tide)); }
/* A cut closes the choices opened since c, but not a running outcome's: its body is failed out of,
 * and what the body did, a cut in it too, does not stay. So no choice ever opens again. */
static void cutto(uint32_t c) { uint32_t i = ncp; while (i > c && cp[i-1].all != 2) i--; ncp = i; if (!i && shared && nn > sw) commit(); }
/* A processor fault inside CPU code returns to the walker. Other processor faults end the process.
 * CPU returns carry a number (0), a truth value (1), a decline (3), or a node handle (4). */
static void crashed(int sig) {
  (void)sig; if (coding && pthread_equal(pthread_self(), walker)) siglongjmp(in_code, 1);   /* only on the walker's own thread */
  put(2, "ck: the processor faulted outside any code a program gave it\n"); _exit(3);
}
static uint32_t left;
static Ret called(void *fn, const int64_t *arg) { int was = coding; coding = was + 1; Ret r = host.call(fn, arg); coding = was; return r; }   /* coding: how deep in calls of code */
static int landed(void *fn, int64_t *arg) {
  arg[6] = (int64_t)(intptr_t)fb;         /* every call of code carries the region's address as its seventh argument */
  fb->me.taken += reading;                /* code can read anything: a start that runs it is not saved */
  uint64_t mark = nrtrail, rooms_n = N(A_ROOMS);                   /* code that declines is run again by the walker: its rooms */
  W->fresh = rooms_n < UINT32_MAX ? (uint32_t)rooms_n : UINT32_MAX;   /* those it takes itself are given back whole */
  left = ntk; Ret r = called(fn, arg); ntk = left;                 /* what code gathered and left is given back */
  if (r.k == 3 || (r.k == 4 && !r.v)) {                            /* are put back as they were, by the trail code always keeps, */
    while (nrtrail > mark) { nrtrail--; memcpy(rooms + rtrail[nrtrail].at, &rtrail[nrtrail].old, 4); }
    N(A_ROOMS) = rooms_n; nnoted = 0; return 0; }                  /* and the notes of code that declined are dropped: the walker */
  if (!ncp) nrtrail = 0;
  for (uint32_t j = 0; j < nnoted; j += 2) {                       /* writes them itself. Code that answered has them written now */
    touch(A_CELLS, noted[j]);
    held[noted[j]] = noted[j+1]; kept(E(NOTE), noted[j], noted[j+1]);
  }
  nnoted = 0;
  if (r.k == 0) W->v = mki(r.v); else if (r.k == 1) W->v = r.v ? E(YES) : E(NO);
  else if (r.k == 4 && known(r.v)) W->v = (H)r.v; else { W->v = 0; W->reason = notice(S_RANGE); }
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
/* Every invocation, direct or attached to a recipe, uses the ABI carried by the leaf. */
static Status execute_code(H leaf, const H *values, uint32_t count, H node, uint32_t from) {
  uint64_t abi = code_target(leaf);
  int64_t args[8] = {0};
  if (!abi) return S_KIND;
  if (node) { args[0] = node; args[1] = from; args[7] = count; }
  else if (!given(values, count, args)) return S_KIND;
  void *fn = HOST_ABI && abi == HOST_ABI ? birth(leaf) : NULL;
  return !fn ? S_NO_NATIVE : landed(fn, args) ? S_OK : S_DECLINED;
}
/* Something bound from outside sits on an operation of the bootstrap or on an entry the program defines:
 * while it does, a binding's faster form steps aside, so what watches sees every use. */
static int watched(void) {
  for (uint32_t j = 0; j < nout; j++) { H e = out[j].entry; long long n = e ? catn(e) : -1;
    if (e && ((n >= 0 && n < SEEDED) || (USED(A_BOUND_BY_NODE, e) && bykey[e]))) return 1; }
  return 0;
}
/* Run binding b: a recipe is entered on the bindings beneath b, a code leaf is called. With
 * node nothing they get values already in hand, code as integers. Otherwise they get the node
 * itself, unevaluated: a lazy keeps the cut scope, code gets the node's handle and the
 * activation it stands in. Answers 0 when code declined. */
static int apply(uint32_t b, uint32_t lim, const H *a, uint32_t c, uint32_t from, H node) {
  H impl = B(b)->impl;
  if (!nd[impl].comp && !code_target(impl)) {       /* another kind of implementation: what its blueprint */
    uint32_t r = bound(nd[impl].bp, 0, lim = under(b, lim));       /* is bound to runs it, given the leaf and the values */
    if (!r || !nd[B(r)->impl].comp) { fault(S_KIND); return 1; }
    H *t = in_hand((uint64_t)c + 1); t[0] = impl; if (c) memcpy(t + 1, a, sizeof(H) * c); a = t; c++; b = r; impl = B(r)->impl;
  }
  uint32_t cool = node ? 0 : act[from].lim & COOL;
  if (nd[impl].comp && !node && B(b)->fast && !cool && !watched()) {       /* the recipe has a faster form: tried first, */
    Status result = execute_code(B(b)->fast, a, c, 0, from);
    if (result == S_OK) { W->mode = M_RET; return 1; }
    if (result == S_DECLINED || (result == S_NO_NATIVE && local_code(B(b)->fast))) cool = COOL;
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
  if (execute_code(impl, a, c, node, from) != S_OK) return 0;
  W->mode = M_RET; return 1;
}
/* Entries 64 to 71: a node taken apart, a node built from parts, and an entry bound. A build
 * the encoding refuses answers nothing, and the reason is the refusal's word. */
static void parts(long long op, const H *a, uint32_t c, int own) {
  Status why = S_OK; int64_t k = 0; H x = a[0], l = c == 2 ? a[1] : 0;
  if (c != (op == CLOCK ? 0u : op == BLUEPRINT || op == COUNT || op == LEAF || op >= READ ? 1u : 2u)) { fault(S_ARITY); return; }
  if (!x || (c == 2 && !l)) { W->v = 0; W->reason = notice(S_NOTHING_IN); return; }
  if (op == PRINT) { fault(S_UNBOUND); return; }        /* the canonical print is form/print.ck's */
  if (op == ID) { W->v = mk(0, E(BYTES), idof(x), 32, &why); return; }
  if (op == READ || op == FILE_OP) {       /* a leaf's bytes as stored nodes to read, or as the path of a file */
    if (nd[x].comp) { fault(S_KIND); return; }
    uint64_t m = ntext; char *s = talloc((size_t)nd[x].n + 1); memcpy(s, bytes + nd[x].off, nd[x].n);
    if (op == READ) { if (!(W->v = read_nodes((uint8_t *)s, nd[x].n, &why))) W->reason = notice(why); }
    else { int64_t got = read_text(s);
      if (got == -2) area_full(A_TEXT);
      if (got < 0) { W->v = 0; W->reason = notice(S_NO_FILE); read_seen(0, 0, 0, 0, 0); } else { room(A_TEXT, ntext + (uint64_t)got); W->v = mk(0, E(BYTES), text + ntext, (uint64_t)got, &why); read_seen(0, 0, 0, x, W->v); } }
    ntext = m; return;
  }
  if (op == BIND) {                       /* x: an entry that is not data, or a composite such as a recipe or an */
    long long e = catn(x);                /* instance of one; l: a recipe, a lazy or a leaf */
    if ((e >= 0 && is_data(e)) || (e < 0 && !nd[x].comp) || (nd[l].comp && !is_recipe(l) && !is_lazy(l))) { fault(S_KIND); return; }
    if (!own) {                           /* bound from outside the run: seen at every limit, and no fail undoes it */
      if (nout == OMOST) { fault(S_FULL); return; }
      out[nout] = (Binding){ E(BINDING), W->at, x, l, 0, nout, 0 }; nout++; W->v = E(YES); return; }
    room(A_BINDINGS, (uint64_t)nbind + 1); touch(A_BOUND_BY_NODE, x);
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
/* w bytes at o in area a of the region, or past the areas its head (A_COUNT + S_COUNT) and the substrate's head, within
 * what the area holds at its peak or its count; far: a substrate's bytes, which another member reads and writes, so they are read
 * whole after what made them (acquire), written after what they make reachable (release) and swapped in one step. */
static uint8_t *spot(int64_t a, int64_t o, int64_t w, int *far) {
  uint64_t base, top; int s = a >= A_COUNT;
  if ((w != 1 && w != 2 && w != 4 && w != 8) || a < 0 || a > A_COUNT + S_COUNT + 1 || a == A_STACK || o < 0) return NULL;
  if (a >= A_COUNT + S_COUNT) { s = a > A_COUNT + S_COUNT; base = s ? fb->me.share : 0; top = s ? sizeof(Substrate) : sizeof(Head); }
  else { const Area *r = AR(a); base = r->off + (s ? fb->me.share : 0); top = (r->peak > r->n ? r->peak : r->n) * r->size; }   /* a substrate made in form may count its records in n alone */
  if ((uint64_t)o + (uint64_t)w > top || (uint64_t)o % (uint64_t)w) return NULL;
  *far = s; return (uint8_t *)fb + base + (uint64_t)o;
}
static uint64_t word_load(const uint8_t *p, int64_t w, int far) {
  if (far) return w == 1 ? LD(*(const uint8_t *)p) : w == 2 ? LD(*(const uint16_t *)p) : w == 4 ? LD(*(const uint32_t *)p) : LD(*(const uint64_t *)p);
  uint64_t u = 0; memcpy(&u, p, (size_t)w); return u;   /* both targets are little-endian */
}
static void word_store(uint8_t *p, int64_t w, int far, uint64_t v) {
  if (!far) { memcpy(p, &v, (size_t)w); return; }
  if (w == 1) ST(*(uint8_t *)p, (uint8_t)v); else if (w == 2) ST(*(uint16_t *)p, (uint16_t)v); else if (w == 4) ST(*(uint32_t *)p, (uint32_t)v); else ST(*(uint64_t *)p, v);
}
static uint64_t word_cas(uint8_t *p, int64_t w, uint64_t old, uint64_t v) {   /* what was there; it is v now when that was old */
  if (w == 1) { uint8_t e = (uint8_t)old; CAS(*(uint8_t *)p, &e, (uint8_t)v); return e; }
  if (w == 2) { uint16_t e = (uint16_t)old; CAS(*(uint16_t *)p, &e, (uint16_t)v); return e; }
  if (w == 4) { uint32_t e = (uint32_t)old; CAS(*(uint32_t *)p, &e, (uint32_t)v); return e; }
  uint64_t e = old; CAS(*(uint64_t *)p, &e, v); return e;
}
static void base(long long op, H *a, uint32_t c, uint32_t env, H node) {
  Status why; int64_t m, n, r = 0;
  if (op == HOST_TARGET) { if (c) fault(S_ARITY); else W->v = catalogue(HOST_ABI ? HOST_ABI : NO); return; }
  if (op == HAS_TARGET) {
    if (c != 1) fault(S_ARITY);
    else { long long target = catn(a[0]); W->v = target > 0 && (uint64_t)target == HOST_ABI ? E(YES) : E(NO); }
    return;
  }
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
  if ((op >= TAKE && op <= STORE) || op == CAS) {   /* a room: bytes in the region under a blueprint, read and changed in place. While */
    int64_t o = 0, w = 0, v = 0, old = 0, ar = 0; uint64_t at = 0, found = 0; Room *h; uint8_t *p; int far = 0;   /* a choice is open a */
    uint32_t want = op == CAS ? 5 : (uint32_t)(op - TAKE + 2), j;   /* store puts what it changes on the room trail, and a fail puts it back */
    if (c != want) { fault(S_ARITY); return; }
    for (j = 0; j < c; j++) if (!a[j]) { W->v = 0; W->reason = notice(S_NOTHING_IN); return; }
    if (!geti(a[1], &o) || (op != TAKE && !geti(a[2], &w)) || (op == STORE && !geti(a[3], &v)) || (op == CAS && (!geti(a[3], &old) || !geti(a[4], &v)))) { fault(S_KIND); return; }
    if (op == TAKE) {
      if (o < 0) { W->v = 0; W->reason = notice(S_RANGE); return; }
      at = N(A_ROOMS); uint64_t len = sizeof(Room) + (((uint64_t)o + 7) & ~7ull); room(A_ROOMS, at + len);
      memset(rooms + at, 0, len); *(Room *)(rooms + at) = (Room){ a[0], W->at, 0, {0}, (uint64_t)o }; N(A_ROOMS) = at + len;
      uint8_t b[8]; for (int i = 0; i < 8; i++) b[i] = (uint8_t)(at >> (8 * i));
      W->v = mk(0, E(ROOM), b, 8, &why); return;
    }
    uint64_t mask = w == 8 ? ~0ull : (1ull << (8 * w)) - 1;
    if (!nd[a[0]].comp && nd[a[0]].bp != E(ROOM) && geti(a[0], &ar)) {   /* an area of the region by its number, past them the head and the substrate's */
      if (!(p = spot(ar, o, w, &far)) || (op != LOAD && ar >= A_COUNT + S_COUNT)) { W->v = 0; W->reason = notice(S_RANGE); return; }   /* the heads, which place the areas, are only read; */
      if (reading && (op != LOAD || !as_saved((uint64_t)(p - (uint8_t *)fb), (uint64_t)w))) fb->me.taken++;   /* an area's bytes are written as they are, given back by their counts, not the trail */
      if (op == LOAD) { W->v = mki((int64_t)word_load(p, w, far)); return; }
      if (op == STORE) { word_store(p, w, far, (uint64_t)v); W->v = a[3]; return; }
      W->v = mki((int64_t)word_cas(p, w, (uint64_t)old & mask, (uint64_t)v & mask)); return;
    }
    if (nd[a[0]].comp || nd[a[0]].bp != E(ROOM) || nd[a[0]].n != 8) { fault(S_KIND); return; }
    memcpy(&at, bytes + nd[a[0]].off, 8); h = (Room *)(rooms + at);
    if (at % 8 || at + sizeof(Room) > N(A_ROOMS) || (w != 1 && w != 2 && w != 4 && w != 8) || o < 0 || (uint64_t)o + (uint64_t)w > h->size) {
      W->v = 0; W->reason = notice(S_RANGE); return; }
    p = rooms + at + sizeof(Room) + o; uint64_t from = (uint64_t)(p - rooms);
    found = word_load(p, w, 0);
    if (op == LOAD) { W->v = mki((int64_t)found); return; }
    if (op == CAS && found != ((uint64_t)old & mask)) { W->v = mki((int64_t)found); return; }
    if (!ncp && !coding) nrtrail = 0;
    else if (!coding || at < W->fresh) {  /* the bytes it changes, and the room's writer and count; in code always, so a decline */
      uint64_t g0 = from & ~3ull, n4 = (from + (uint64_t)w - g0 + 3) / 4; room(A_ROOM_TRAIL, nrtrail + n4 + 2);
      for (uint64_t g, k = 0; k < n4 + 2; k++) {
        g = k == 0 ? at + offsetof(Room, by) : k == 1 ? at + offsetof(Room, writes) : g0 + 4 * (k - 2);
        RoomTrail t = { E(ROOM_TRAIL), W->at, 0, {0}, g }; memcpy(&t.old, rooms + g, 4); rtrail[nrtrail++] = t; } }   /* undoes them, but in a room code took itself */
    word_store(p, w, 0, (uint64_t)v);
    h->by = W->at; h->writes++; W->v = op == CAS ? mki((int64_t)found) : a[3]; return;
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
    int64_t arg[9] = {0}; uint64_t mark = ntext; int raw = !numeric_io || (c && a[0] && geti(a[0], arg));   /* an integer first is the address itself; */
    if (c < 1 || (raw && c > 9)) { fault(S_ARITY); return; }      /* any other leaf goes as the address of its bytes with a zero after; eight registers */
    if (ncp) { fault(S_IRREVERSIBLE); return; }        /* what cannot be undone is refused while a choice is open */
    if (!raw) {
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
      if (geti(a[j], &arg[j])) continue;
      char *s = talloc((size_t)nd[a[j]].n + 1); memcpy(s, bytes + nd[a[j]].off, nd[a[j]].n);
      arg[j] = (int64_t)(intptr_t)(j ? (void *)s : host.symbol(s));
    }
    if (W->mode == M_FAULT) { ntext = mark; return; }
    fb->me.taken += reading;          /* a start that takes from the host is not saved */
    if (!arg[0]) { W->v = 0; W->reason = notice(S_NO_SYMBOL); ntext = mark; return; }
    Ret r;
    ntext = mark;                     /* given back before the call, since a crash does not return here */
    r = called((void *)(intptr_t)arg[0], arg + 1); W->v = mki(r.v);
    return;
  }
  if (op == PEEK || op == HANDLE || op == AT) {   /* the region read by a program in it: its bytes, and where a node is */
    int64_t o = 0, n = 0; uint64_t guard = fb->a[A_STACK].off;
    if (c != (op == PEEK ? 2u : 1u)) { fault(S_ARITY); return; }
    if (!a[0] || (op == PEEK && !a[1])) { W->v = 0; W->reason = notice(S_NOTHING_IN); return; }
    if (op == HANDLE) { if (a[0] > SEEDED && a[0] != W->handed) { W->handed = a[0]; W->handeds++; } W->v = mki(a[0]); return; }   /* adopt asks what was given: the last handle above the catalogue, and how many */
    if (!geti(a[0], &o) || (op == PEEK && !geti(a[1], &n))) { fault(S_KIND); return; }
    if (op == AT && !known(o) && o > 0 && o >> 32 == 0 && (H)o < lsw((uint64_t)o >> LS, 0)) { fault(S_IRREVERSIBLE); return; }   /* settled since this search began */
    if (op == AT ? !known(o) : (o < 0 || n < 0 || ((uint64_t)o + (uint64_t)n > fb->bytes && ((uint64_t)o < fb->me.share || (uint64_t)o + (uint64_t)n > fb->me.share + sub->bytes))
                                          || ((uint64_t)o < guard && (uint64_t)o + (uint64_t)n > guard - FB_PAGE))) { W->v = 0; W->reason = notice(S_RANGE); return; }
    if (op == AT) { W->v = (H)o; return; }
    if (reading && !as_saved((uint64_t)o, (uint64_t)n)) fb->me.taken++;   /* only what an image keeps as it was may a start read */
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
    if (!a[0] || !USED(A_BOUND_BY_NODE, a[0]) || !bykey[a[0]]) { W->v = E(NO); return; }
    room(A_BINDINGS, (uint64_t)nbind + 1); bd[nbind] = (Binding){ E(BINDING), W->at, a[0], 0, bykey[a[0]], nbind, 0 }; nbind++;
    bykey[a[0]] = bd[bykey[a[0]] - 1].prev; W->v = E(YES); return;
  }
  if (op == HOLD || op == HELD || op == NOTE) {   /* a cell is any node; what it holds is a node. While a choice is */
    H key = a[0];                     /* open, what it held before goes on the trail, and a fail puts it back */
    if (c != (op == HELD ? 1u : 2u)) { fault(S_ARITY); return; }
    if (!key || (op != HELD && !a[1])) { W->v = 0; W->reason = notice(S_NOTHING_IN); return; }
    if (op == HELD) { if (!(W->v = USED(A_CELLS, key) ? held[key] : 0)) W->reason = notice(S_EMPTY); return; }
    touch(A_CELLS, key);
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
    if (m + 1 < n) { H next = mki(m + 1); opening();
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
  if (op != ADD && op != SUB && op != MUL) { fault(S_KIND); return; }   /* a number no operation stands at */
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
    H g = cm >> 8 & 1 ? known(a0) : mki(a0);
    if (g) { room(A_GATHERING_CHILDREN, (uint64_t)ntk + 1); tk[ntk++] = g; }
    return (Ret){ 0, 4 };
  }
  if (op == LIST) {
    if (c != 1 || cm != 1 || a0 < 0 || (uint64_t)a0 > ntk) { fault(S_KIND); siglongjmp(in_code, 3); }
    x = mk(1, E(LIST), tk + a0, ntk - (uint32_t)a0, &why); ntk = (uint32_t)a0; return (Ret){ x, 4 };
  }
  if (op == NOTE || op == HELD) {
    H key = !c ? 0 : cm >> 8 & 1 ? known(a0) : mki(a0), v = c != 2 ? 0 : cm >> 9 & 1 ? known(a1) : mki(a1);
    if (c == (op == NOTE ? 2u : 1u) && key && (op == HELD || v)) {
      if (op == NOTE) { room(A_NOTED, (uint64_t)nnoted + 2); noted[nnoted] = key; noted[nnoted+1] = v; nnoted += 2; return (Ret){ v, 4 }; }
      for (uint32_t j = nnoted; j > 0; j -= 2) if (noted[j-2] == key) return num ? (geti(noted[j-1], &m) ? (Ret){ m, 0 } : (Ret){ 0, 3 }) : (Ret){ noted[j-1], 4 };
    }
  }
  if (op == NODE) { catalogue_ids(); x = c == 1 && cm == 1 ? byid((const uint8_t *)(uintptr_t)a0) : 0; if (x) return (Ret){ x, 4 }; fault(S_NO_NODE); siglongjmp(in_code, 3); }
  for (uint32_t j = 0; j < c; j++) a[j] = (cm >> (8 + j) & 1) ? known(v[j]) : 0;
  x = a[0];                               /* what code reads most is answered without a number being built for it */
  if (x && op == COUNT && num && c == 1) return (Ret){ nd[x].n, 0 };
  if (x && op == BYTE && num && c == 2 && !(cm >> 9 & 1) && !nd[x].comp && (uint64_t)v[1] < nd[x].n) return (Ret){ bytes[nd[x].off + v[1]], 0 };
  if (x && op == CHILD && c == 2 && !(cm >> 9 & 1) && nd[x].comp && (uint64_t)v[1] < nd[x].n) W->v = KID(x, v[1]);
  else {
    for (uint32_t j = 0; j < c; j++) if (!(cm >> (8 + j) & 1)) a[j] = mki(v[j]);
    if ((op == STORE || op == CAS) && c && geti(a[0], &m)) return (Ret){ 0, num ? 2 : 4 };   /* an area is written by code only inline, as its last act: */
    if (op == QUOTE) W->v = c == 1 ? a[0] : 0;   /* asked here, code declines unwritten and the walker writes once */
    else if (op == ALL) { for (uint32_t j = 0; j < c; j++) if (a[j]) a[n++] = a[j]; W->v = mk(1, E(LIST), a, n, &why); }
    else if (!basic(op)) fault(S_KIND); else base(op, a, c, W->env, 0);
    if (W->mode == M_FAULT) siglongjmp(in_code, 3);
  }
  if (!num) return (Ret){ W->v, 4 };
  return !W->v ? (Ret){ 0, 2 } : geti(W->v, &m) ? (Ret){ m, 0 } : (Ret){ 0, 3 };
}

static uint32_t top = ~COOL;            /* what the top of a run sees: every binding, or with 0 the bootstrap's code alone */
static void walk(H x0) {                 /* ends with mode M_DONE; outcome is 'n' node, '0' nothing, 'f' fault */
  Status why; H *a = hand;
  const uint64_t text_base = ntext;
  W->serve = (uint64_t)(uintptr_t)serve; W->x = x0; W->v = W->reason = 0; W->env = W->cut = W->k = 0; W->outcome = 0; W->mode = M_EVAL;
  nfr = nlink = nact = 1; nargp = ncp = ncoll = 0; act[0] = (Activation){ E(ACTIVATION), 0, 0, 0, 0, top, 0, 0 }; W->one = 0;
  switch (sigsetjmp(in_code, 1)) {        /* the walker goes on from here after what cannot go on where it happened: */
  case 1: coding = 0; ntk = left; nnoted = 0; fb->me.taken += reading; fault(S_CRASH); break;              /* the processor faulted inside code a program gave */
  case 2: if (answering == 2) answering = 0; while (ntk) { H h = tk[--ntk]; if (h < N(A_PRINT_NUMBERS) && on_page(A_PRINT_NUMBERS, h)) pnum[h] = 0; }
    for (uint32_t j = 0; j < nopen; j++) if (opened[j] < N(A_PRINT_NUMBERS) && on_page(A_PRINT_NUMBERS, opened[j])) pnum[opened[j]] = 0;
    coding = 0; ntb = 0; ntext = text_base; W->reason = full_at[full]; W->mode = M_FAULT; break;
  case 3: coding = 0; ntk = left; nnoted = 0; break;                              /* an operation code asked for faulted: the fault is the program's */
  }
  for (nhand = 0;; W->step++, nhand = 0) switch (W->mode) {
  case M_EVAL: {
    if (!ncp) { ntrail = 0; nrtrail = 0; if (nn > sw) commit(); }        /* every choice is closed: what was made under one settles before */
    if (nafter && !ncp) {                 /* anything runs, and what was held back for that is done now */
      H q = due(); W->k = push(F_ASIDE, W->x, W->one, W->env, W->cut, W->reason, W->k, 1); W->x = q; W->env = W->one = 0; W->cut = 0;
    }
    if (N(A_ASKED) && !N(A_ANSWERED)) {   /* another process asks: its node is evaluated here, between two steps of the */
      uint64_t n = N(A_ASKED), mark = ntext; H q, p, w = 0;  /* run, as from outside and leaving no other trace; in text, */
      if (n == ~0ull) { N(A_ASKED) = 0; exit(143); } if (n > fb->a[A_ASKED].cap) n = 0;
      room(A_ASKED, n);
      char *s = talloc((size_t)n + 1); memcpy(s, (uint8_t *)fb + fb->a[A_ASKED].off, n); s[n] = 0; N(A_ANSWERED) = 1;
      why = S_ID; q = numeric_io ? (n == 32 ? (catalogue_ids(), byid((uint8_t *)s)) : 0) : read_nodes((uint8_t *)s, n, &why);
      if (!numeric_io) { if (q && nd[q].comp && nd[q].bp == E(LIST) && nd[q].n == 2 && is_recipe(KID(q, 0))) { w = KID(q, 0); q = KID(q, 1); } else if (q) { why = S_KIND; q = 0; } }   /* a writer and a node */
      if (!q) { p = notice(why); q = mk(1, E(FAULT), &p, 1, &why); }
      ntext = mark; q = mk(1, E(OUTSIDE), &q, 1, &why); q = mk(1, E(OUTCOME), &q, 1, &why);
      W->k = push(F_ASIDE, W->x, W->one, W->env, W->cut, W->reason, W->k, 0); if (w) W->k = push(F_ASIDE, w, 0, 0, 0, 0, W->k, 3);
      W->x = q; W->env = W->one = 0; W->cut = ncp;
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
    if (!b && (is_data(op) || op > LAST)) { answer(x); break; }/* data, or past these an entry with no meaning yet */
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
      if (c > 1) { opening();
        cp[ncp] = (ChoicePoint){ E(CHOICE_POINT), x, 0, x, 1, W->env, W->k, nfr, nlink, nact, nargp, 0, nbind, ntrail, nafter, nrtrail, MARKS }; ncp++; }
      W->x = KID(x, 0); break;
    case FAIL:
      if (c > 1) { fault(S_ARITY); break; }
      if (!c) { W->mode = M_FAILED; break; }
      W->k = push(F_FAIL, x, 0, W->env, W->cut, 0, W->k, 0); W->x = KID(x, 0); break;
    case CUT:
      if (c > 1) { fault(S_ARITY); break; }
      if (!c) { cutto(W->cut); answer(E(YES)); break; }
      W->k = push(F_CUT, x, 0, W->env, W->cut, 0, W->k, 0); W->x = KID(x, 0); break;
    case ALL:
      if (c != 1) { fault(S_ARITY); break; }
      opening();
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
      opening(); W->reason = 0;
      cp[ncp] = (ChoicePoint){ E(CHOICE_POINT), x, 2, x, 0, W->env, W->k, nfr, nlink, nact, nargp, 0, nbind, ntrail, nafter, nrtrail, MARKS }; ncp++;
      W->k = push(F_OUTCOME, x, ncoll, W->env, W->cut, W->k, 0, ncp - 1); W->cut = ncp; W->x = KID(x, 0); break;
    default:                              /* call, arithmetic, eq, lt, native, parts: children first, in order */
      if (!c) {
        while (b && !apply(b, lim, hand, 0, W->env, 0)) b = bound(nd[x].bp, 0, lim = under(b, lim));
        if (!b) { if (op == CLOCK) { fb->me.taken += reading; answer(mki(host.clock())); } else if (op == HOST_TARGET) answer(catalogue(HOST_ABI ? HOST_ABI : NO)); else fault(S_ARITY); }
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
    if (!ncp || k >= cp[ncp-1].mfr) {     /* nothing can return to it, nor to the frames a commit left above it, */
      nfr = k; if (f.mlink < nlink && (!ncp || f.mlink >= cp[ncp-1].mlink)) nlink = f.mlink; }      /* nor to the values linked since */
    uint32_t keep = (ncp && cp[ncp-1].mact > f.mact) ? cp[ncp-1].mact : f.mact;
    if (keep < nact) { nargp = act[keep].off; nact = keep; }       /* calls made since the frame have returned */
    W->k = f.k;
    switch (f.t) {
    case F_SEQ:
      W->env = f.env; W->cut = f.cut; W->x = KID(f.node, f.i); W->mode = M_EVAL;
      if (f.i + 1 < nd[f.node].n) W->k = push(F_SEQ, f.node, f.i + 1, W->env, W->cut, 0, W->k, 0);
      break;
    case F_CUT: cutto(f.cut); break;
    case F_FAIL: W->reason = W->v; W->mode = M_FAILED; break;
    case F_ALL:
      if (W->v) { room(A_GATHERED, (uint64_t)ncoll + 1); coll[ncoll] = (Gathered){ E(GATHERED), W->at, f.aux, W->v }; ncoll++; }
      W->mode = M_FAILED; break;
    case F_NEED: if (!W->v || W->v == E(NO)) W->mode = M_FAILED; break;
    case F_RECOVER: break;
    case F_ASIDE: {                       /* something done between two steps is over: the run goes on where it was */
      if (f.aux == 2) { W->v = f.node; break; }            /* or ends with the answer it had */
      if (f.aux == 3) { H c[2] = { f.node, mk(1, E(QUOTE), &W->v, 1, &why) }; W->x = mk(1, E(CALL), c, 2, &why);   /* the outcome, */
        W->x = mk(1, E(OUTCOME), &W->x, 1, &why); W->env = enter(0, 0, 0, 0, 0, ncp); W->cut = ncp; W->mode = M_EVAL; answering = 2; break; }   /* with SPARE */   /* written beneath every binding */
      if (!f.aux) {                       /* it was asked from outside: the outcome goes back to who asked */
        H b = nd[W->v].comp && nd[W->v].n == 2 && KID(W->v, 0) == notice(S_NODE) && !nd[KID(W->v, 1)].comp ? KID(W->v, 1) : 0;   /* in text: the bytes */
        uint64_t n = numeric_io ? 32 : b && nd[b].n <= fb->a[A_ANSWERED].cap ? nd[b].n : 0;  /* the asked node answered, */
        room(A_ANSWERED, n); memcpy((uint8_t *)fb + fb->a[A_ANSWERED].off, numeric_io ? idof(W->v) : bytes + nd[b].off, n);  /* or none */
        N(A_ANSWERED) = n; N(A_ASKED) = 0; if (answering == 2) answering = 0;
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
      if (ncp <= f.aux) die("an outcome's choice was closed under it");
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
      if (op == NATIVE_OP) {              /* code for a target this processor does not run is run by what its blueprint */
        b = c && code_target(a[0]) && !local_code(a[0]) ? bound(nd[a[0]].bp, 0, f.aux) : 0;       /* is bound to, given the leaf */
        if (b && nd[B(b)->impl].comp) {                                                           /* and the list of the values */
          H two[2] = { a[0], mk(1, E(LIST), a + 1, c - 1, &why) };
          if (!two[1]) fault(why); else apply(b, f.aux, two, 2, f.env, 0);
          break; }
        Status result = execute_code(a[0], a + 1, c - 1, 0, f.env);
        if (result == S_KIND || result == S_ARITY) fault(result);
        else if (result != S_OK) { W->v = 0; W->reason = notice(result); }
        break;
      }
      if (op > LAST) { fault(S_KIND); break; }       /* an entry only code ran, and the code declined these values */
      base(op, a, c, f.env, f.node); break; }
    }
    break; }
  case M_FAILED: {
    if (!ncp) { if (!W->reason) W->reason = notice(S_NO_PATH); W->v = 0; W->outcome = '0'; W->mode = M_DONE; return; }
    ChoicePoint *q = &cp[ncp-1]; W->at = q->node; W->v = 0;      /* a fail carries no answer: what the failed path last answered is not kept */
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
      W->mode = M_RET; if (shared && !ncp && nn > sw) commit(); break;
    }
    if (q->all == 2) {                    /* an outcome: what was seen, or that no path was left */
      ncp--; W->v = q->coll ? q->coll : seen_as(S_NOTHING, W->reason ? W->reason : notice(S_NO_PATH));
      W->reason = 0; W->mode = M_RET; if (shared && !ncp && nn > sw) commit(); break;
    }
    if (q->all) {                         /* every answer has been gathered */
      uint32_t base = ntk, w = q->i, id = q->coll;       /* its answers lie above where it began: q->i */
      for (uint32_t i = q->i; i < ncoll; i++) {
        if (coll[i].id == id) { room(A_GATHERING_CHILDREN, (uint64_t)ntk + 1); tk[ntk++] = coll[i].v; } else coll[w++] = coll[i]; }
      ncoll = w; ncp--;
      W->v = mk(1, E(LIST), tk + base, ntk - base, &why); ntk = base; W->mode = M_RET; if (shared && !ncp && nn > sw) commit(); break;
    }
    W->x = KID(q->node, q->i); W->cut = ncp - 1; W->mode = M_EVAL;
    if (q->i + 1 >= nd[q->node].n) ncp--; else { q->i++; q->by = W->at; rise(q); }
    break; }
  case M_FAULT: {
    uint32_t j = W->k;                    /* an all's frame keeps the way out of it in vals; its own k is nothing */
    while (j && fr[j].t != F_RECOVER && fr[j].t != F_OUTCOME) j = fr[j].t == F_ALL ? fr[j].vals : fr[j].k;
    if (!j) { W->v = 0; W->outcome = 'f'; W->mode = M_DONE; return; }
    Frame f = fr[j];
    if (f.t == F_OUTCOME) { if (ncp <= f.aux) die("an outcome's choice was closed under it");
      cp[f.aux].coll = seen_as(S_FAULT, W->reason); ncoll = f.i; ncp = f.aux + 1; W->mode = M_FAILED; break; }
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
static H file_node(const char *path) {      /* a node file: its records */
  size_t len; Status why; uint8_t *b = load(path, &len);
  H x = read_nodes(b, len, &why);
  if (!x) { say("refused ", status_words[why]); exit(1); }
  read_seen(path, 0, 0, 0, x); return x;                  /* a start file is checked by the node it reads to */
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
/* A reply holds its value's closure, each part before what holds it. A part may lie in any lane, above its holder's
 * too, so the closure is found from the value: a node is listed once its blueprint and children are, in their order. */
static int send_answer(uint8_t status, H value) {
  uint8_t header[37]; uint64_t size = sizeof ice_header, base = ntk, first = nhand;
  room(A_GATHERING_CHILDREN, ntk + 1); tk[ntk++] = value;
  while (ntk > base) {                    /* print-numbers: 1 while a node's parts are being listed, 2 once it is */
    H h = tk[ntk - 1]; Node *x = nd + h; touch(A_PRINT_NUMBERS, h);
    if (pnum[h]) { ntk--; if (pnum[h] == 1) { pnum[h] = 2; *in_hand(1) = h; size += 4 + 33 + (x->comp ? (uint64_t)x->n * 32 : x->n); } continue; }
    pnum[h] = 1; room(A_GATHERING_CHILDREN, ntk + (x->comp ? x->n : 0) + 1);
    for (uint64_t j = x->comp ? x->n : 0; j-- > 0; ) if (KID(h, j) > ROOT && !(USED(A_PRINT_NUMBERS, KID(h, j)) && pnum[KID(h, j)])) tk[ntk++] = KID(h, j);
    if (x->bp > ROOT && !(USED(A_PRINT_NUMBERS, x->bp) && pnum[x->bp])) tk[ntk++] = x->bp;
  }
  int sent = size <= UINT32_MAX; header[0] = status; memcpy(header + 1, idof(value), 32); put4(header + 33, (uint32_t)size);
  sent = sent && send_bytes(header, sizeof header) && send_bytes(ice_header, sizeof ice_header);
  for (uint64_t i = first; i < nhand; i++) {
    uint64_t mark = ntext; size_t len; uint8_t *record = cbytes(hand[i], &len); put4(header, (uint32_t)len);
    sent = sent && send_bytes(header, 4) && send_bytes(record, len); ntext = mark; pnum[hand[i]] = 0;
  }
  nhand = first; return sent;
}
static int send_outcome(void) {
  return W->outcome == 'n' ? send_answer(1, W->v) : send_answer(W->outcome == 'f' ? 2 : 0, W->reason);
}

/* ---- the region at rest: saved once, entered by every later start ------------------------------
 * A start is the files run before the program, or the image seed-run opens with its first part. When it has run and
 * the region is at rest, nothing open, held back, asked, bound from outside or taken from the host or the process, the
 * region is a function of what was read. Its pages are then saved under a name made of the start and the place it ran
 * in, and a later start of that name maps them copy-on-write instead of running it. What was read, and whether it still
 * is, is region/rest.ck's: the builder hands it the files the start read, it answers the node the image carries, and a
 * later start runs that node in a fresh region, before anything is mapped, to learn whether the image may be entered.
 * The words of the process's own (FB_OWN) are saved as 0 and are this process's after the map. */
typedef struct { uint64_t off, n, at; } Extent;          /* n bytes of the region at off, in the file at at */
#define READS 64
static struct { const char *path; uint64_t at, n; H name, content; } reads[READS]; static uint32_t nreads;
static uint64_t text_base; static const char *rest_path; static char home[PATH_MAX], rest_buf[PATH_MAX];   /* the executable's folder */
static void read_seen(const char *path, uint64_t at, uint64_t n, H name, H content) {   /* a file the start read */
  if (!reading) return;
  if (nreads == READS || ncp || (!path && !content)) { fb->me.taken++; return; }   /* under a choice, what was read may be given back; */
                                                                                  /* a file not there is not a file the image can check */
  reads[nreads].path = path; reads[nreads].at = at; reads[nreads].n = n; reads[nreads].name = name; reads[nreads++].content = content;
}
#define OWN_SIZE(w) + sizeof(w)
#define OWN_KEEP(w) memcpy(k, &(w), sizeof(w)); k += sizeof(w);
#define OWN_PUT(w) memcpy(&(w), k, sizeof(w)); k += sizeof(w);
#define OWN_ZERO(w) memset(&(w), 0, sizeof(w));
#define OWN_HIT(w) hit |= o < (uint64_t)((uint8_t *)&(w) - (uint8_t *)fb) + sizeof(w) && (uint64_t)((uint8_t *)&(w) - (uint8_t *)fb) < o + n;
static uint8_t own_words[0 FB_OWN(OWN_SIZE)];
static void keep_own(void) { uint8_t *k = own_words; FB_OWN(OWN_KEEP) }
static void put_own(void) { uint8_t *k = own_words; FB_OWN(OWN_PUT) }
#define KEPT_IN(w) hit |= o >= (uint64_t)((uint8_t *)&(w) < (uint8_t *)sub ? 0 : share) + (w).off && o + n <= (uint64_t)((uint8_t *)&(w) < (uint8_t *)sub ? 0 : share) + (w).off + (w).cap * (w).size;
static int as_saved(uint64_t o, uint64_t n) {   /* bytes an image keeps as they were: the heads and the areas of fb.kept, but the process's own words */
  int hit = o + n <= sizeof(Head) || (o >= share && o + n <= share + sizeof(Substrate)); FB_KEPT(KEPT_IN)
  if (!hit) return 0;
  hit = 0; FB_OWN(OWN_HIT) return !hit;
}
static int self_uuid(uint8_t u[16]) {     /* the executable's identity, as the linker made it */
#ifdef __APPLE__
  extern const struct mach_header_64 _mh_execute_header; const struct load_command *c = (const void *)(&_mh_execute_header + 1);
  for (uint32_t i = 0; i < _mh_execute_header.ncmds; i++, c = (const void *)((const uint8_t *)c + c->cmdsize))
    if (c->cmd == LC_UUID) { memcpy(u, ((const struct uuid_command *)c)->uuid, 16); return 1; }
#endif
  (void)u; return 0;
}
/* Every page this process wrote, but the stack's and the text's, is zero again, and so are the heads. */
static void wipe(void) {
  for (int i = 0; i < A_COUNT + S_COUNT; i++) {
    Area *a = AR(i); uint8_t *b = (uint8_t *)fb + (i < A_COUNT ? 0 : share) + a->off; uint64_t m = a->peak > a->n ? a->peak : a->n, hue = a->off % FB_PAGE;
    if (i == A_STACK || i == A_TEXT) continue;
    if (i < A_COUNT && L[i].reserve == FB_HANDLES) { for (uint64_t j = 0; j <= (hue + 4 * a->n) / FB_PAGE; j++) if (tbits[(tbit[i] + j) >> 3] >> ((tbit[i] + j) & 7) & 1) memset(b - hue + j * FB_PAGE, 0, FB_PAGE); }
    else memset(b, 0, (size_t)(m * a->size));
  }
  memset(fb, 0, sizeof(Head)); memset(sub, 0, sizeof(Substrate)); memset(dense, 0, sizeof dense);
}
/* A start that will not use an image begins as fb_open left the region; what a look at an image did is gone. */
static void fresh(Area t) { keep_own(); wipe(); put_own(); fb_reset(); fb->a[A_TEXT] = t; store_init(); }
/* The image's pages over a region made zero. After its checks nothing stops it half way. */
static void image_load(int fd, const Extent *e, uint32_t n) {
  keep_own(); wipe();
  for (uint32_t i = 0; i < n; i++)
    if (getenv("CK_FB") ? pread(fd, (uint8_t *)fb + e[i].off, (size_t)e[i].n, (off_t)e[i].at) != (ssize_t)e[i].n     /* a region others see is filled */
        : mmap((uint8_t *)fb + e[i].off, (size_t)e[i].n, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_FIXED, fd, (off_t)e[i].at) == MAP_FAILED) die("an image could not be mapped");
  put_own();
}
/* What region/rest.ck is told of a start: the executable's identity, the port, the start's paths, the layout's facts and
 * the executable's folder, where the files it reads lie. */
static H told(int n, char **paths) {
  uint8_t u[16]; Status why; H g[5]; if (!n || !rest_path || !self_uuid(u)) return 0;
  const int64_t f[] = { offsetof(Head, d) + offsetof(Doors, symbol), offsetof(Head, d) + offsetof(Doors, region), (int64_t)(fb->a[A_ROOMS].off + sizeof(Room)),
    sizeof(Head), offsetof(Head, a), sizeof(Area), (int64_t)share, sizeof(Substrate), offsetof(Substrate, a), A_COUNT, S_COUNT, A_STACK, (int64_t)(off[A_STACK] - FB_PAGE),
    (int64_t)(off[A_TEXT] / FB_PAGE * FB_PAGE), (int64_t)pages_for(off[A_TEXT] + cap[A_TEXT]), offsetof(Head, m) + offsetof(Machine, ncode), offsetof(Head, me) + offsetof(Member, charged),
    (int64_t)(cap[A_TOUCHED] * 8), (int64_t)fb->me.allowance, (int64_t)fb->a[A_STACK].cap, FB_VERSION, FB_PAGE, offsetof(Area, of), offsetof(Area, size), offsetof(Area, peak),
    offsetof(Area, off), offsetof(Area, cap), offsetof(Area, n) };
  H *v = in_hand(sizeof f / sizeof *f + (uint64_t)n);
  for (size_t i = 0; i < sizeof f / sizeof *f; i++) v[i] = mki(f[i]);
  for (int i = 0; i < n; i++) v[sizeof f / sizeof *f + (size_t)i] = txt(paths[i]);
  g[0] = mk(0, E(BYTES), u, 16, &why); g[1] = mki(numeric_io + 2 * NUMERIC_PORT_FLAG);
  g[2] = mk(1, E(LIST), v + sizeof f / sizeof *f, (uint64_t)n, &why); g[3] = mk(1, E(LIST), v, sizeof f / sizeof *f, &why); g[4] = txt(home);
  return mk(1, E(LIST), g, 5, &why);
}
/* region/rest.ck, run: its recipes. The file and the bytes it gave are kept, for the saver's check. */
static H rest_read;
static H rest_load(void) {
  size_t len; Status why; uint8_t *b = load(rest_path, &len); H r = read_nodes(b, len, &why), p[2] = { txt(rest_path), mk(0, E(BYTES), b, len, &why) };
  rest_read = mk(1, E(LIST), p, 2, &why); if (r) run(r);
  return r && W->outcome == 'n' && nd[W->v].comp && nd[W->v].n == 2 ? W->v : 0;
}
static H rest_call(H v, int w, H *a, uint32_t n) {        /* recipe w of them, called with a: its answer, or nothing */
  Status why; H q[8]; if (!v || n > 7) return 0;
  q[0] = KID(v, w); for (uint32_t i = 0; i < n; i++) q[i + 1] = a[i];
  for (uint32_t i = 0; i <= n; i++) q[i] = mk(1, E(QUOTE), &q[i], 1, &why);
  run(mk(1, E(CALL), q, n + 1, &why)); return W->outcome == 'n' ? W->v : 0;
}
/* Whether an image of this start may be entered is region/rest.ck's, run in a region that has the catalogue alone: the
 * name, the head, the extents, the layout and the files the start read. What it answers is mapped. Answers how many of
 * the start's files the image stands for, 0 when none does, -1 when no image is kept. */
static int image_enter(int n, char **paths, char *entered) {
  Area t = fb->a[A_TEXT]; H g = told(n, paths), r = g ? rest_call(rest_load(), 0, &g, 1) : 0; int64_t k = 0, fd = -1, v[3];
  if (!g) return -1;
  if (r && nd[r].comp && nd[r].n == 4 && geti(KID(r, 0), &k) && k > 0 && geti(KID(r, 1), &fd) && !nd[KID(r, 2)].comp && nd[KID(r, 2)].n < PATH_MAX) {
    H es = KID(r, 3); uint64_t m = nd[es].n; Extent *e = talloc((size_t)m * sizeof *e);
    for (uint64_t i = 0; i < m; i++) { for (int j = 0; j < 3; j++) if (!geti(KID(KID(es, i), j), &v[j])) die("an image's extents are not numbers"); e[i] = (Extent){ (uint64_t)v[0], (uint64_t)v[1], (uint64_t)v[2] }; }
    memcpy(entered, bytes + nd[KID(r, 2)].off, nd[KID(r, 2)].n); entered[nd[KID(r, 2)].n] = 0;
    image_load((int)fd, e, (uint32_t)m); close((int)fd); return (int)k; }
  fresh(t); return r ? 0 : -1;                              /* as if no image had been looked at */
}
static int image_rest(int k, int n, char **paths, const char *prior, char *saved) {
  if (nn > sw) commit();
  ntrail = nrtrail = 0;                                     /* no choice is open: nothing on the trails can be undone */
  if (fb->me.taken || nafter || nout || nnoted || N(A_ASKED) || N(A_ANSWERED) || shared || !rest_path) return 0;
  nborn = 0; W->ncode = 0; tcap = icap = 0; W->ids = 0;   /* no code born, no working index: a later start makes them */
  uint64_t from[A_COUNT + S_COUNT + 2], to[A_COUNT + S_COUNT + 2], top = off[A_STACK] - FB_PAGE; int m = 2, got = 0; uint8_t *snap = MAP_FAILED;
  from[0] = 0; to[0] = FB_PAGE; from[1] = share; to[1] = share + FB_PAGE;   /* where the region holds anything: the heads and what each area uses */
  for (int i = 0; i < A_COUNT + S_COUNT; i++) { const Area *a = AR(i); uint64_t b = (i < A_COUNT ? 0 : share) + a->off;
    if (i != A_STACK && i != A_TEXT && a->n) { from[m] = b / FB_PAGE * FB_PAGE; to[m++] = pages_for(b + a->n * a->size); } }
  for (int i = 1; i < m; i++) for (int j = i; j > 0 && from[j] < from[j-1]; j--) { uint64_t f = from[j], t = to[j]; from[j] = from[j-1]; to[j] = to[j-1]; from[j-1] = f; to[j-1] = t; }
  Area t = fb->a[A_TEXT]; keep_own(); FB_OWN(OWN_ZERO) ntext = fb->a[A_TEXT].peak = text_base;   /* the region at rest, the process's own words */
#ifdef __APPLE__
  snap = mmap(NULL, (size_t)top, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);   /* copied on write: what the saver writes */
  got = snap != MAP_FAILED && vm_copy(mach_task_self(), (vm_address_t)(uintptr_t)fb, (vm_size_t)top, (vm_address_t)(uintptr_t)snap) == KERN_SUCCESS;
  if (!got && snap != MAP_FAILED) { munmap(snap, (size_t)top); snap = MAP_FAILED; }
#endif
  put_own(); fb->a[A_TEXT] = t;                            /* as 0 and the text as it was before the start, however far it reached */
  H list[READS + 1], pair[2], span[A_COUNT + S_COUNT + 2], q[6]; Status why; uint32_t c = 0;
  for (uint32_t i = 0; got && i < nreads; i++) {           /* what the start read, each path with the bytes it gave */
    pair[0] = reads[i].path ? txt(reads[i].path) : reads[i].name; pair[1] = reads[i].content ? reads[i].content : mk(0, E(BYTES), text + reads[i].at, reads[i].n, &why);
    list[c++] = mk(1, E(LIST), pair, 2, &why); }
  for (int i = 0; got && i < m; i++) { pair[0] = mki((int64_t)from[i]); pair[1] = mki((int64_t)to[i]); span[i] = mk(1, E(LIST), pair, 2, &why); }
  q[0] = mki((int64_t)(uintptr_t)snap); q[1] = mk(1, E(LIST), span, (uint64_t)m, &why); q[2] = mki(k); q[3] = told(n, paths); q[5] = prior ? txt(prior) : E(NO);
  signal(SIGXFSZ, SIG_IGN);                                 /* a file too large is refused, not a signal */
  H v = got && q[3] ? rest_load() : 0;                      /* region/rest.ck writes it */
  if (v) {
    list[c++] = rest_read; q[4] = mk(1, E(LIST), list, c, &why); H r = rest_call(v, 1, q, 6);
    if ((got = r && !nd[r].comp && nd[r].bp == E(TEXT) && nd[r].n < PATH_MAX)) {
      if (*saved) unlink(saved);
      memcpy(saved, bytes + nd[r].off, nd[r].n); saved[nd[r].n] = 0; } }
  else got = 0;
  if (snap != MAP_FAILED) {                                 /* the region goes on as a later start has it: the pages saved, as they were */
    keep_own(); wipe(); for (int i = 0; i < m; i++) memcpy((uint8_t *)fb + from[i], snap + from[i], (size_t)(to[i] - from[i])); put_own(); munmap(snap, (size_t)top); }
  return got;
}
/* What a start leaves in the cell under the entry after runs once it is over, in this process, whether the start ran or
 * was entered: so a package's opening, the door, the device, the join, follows its loading and is not saved with it. */
#ifndef NUMERIC_PORT
static H after_seen;                      /* what the start's files left under after so far: each new one runs after the last */
static void after_join(void) { H now = USED(A_CELLS, E(AFTER)) ? held[E(AFTER)] : 0, p[2] = { after_seen, now }; Status why;
  if (now && after_seen && now != after_seen) now = held[E(AFTER)] = mk(1, E(SEQ), p, 2, &why); after_seen = now; }
#endif
static int reborn(void) { H r = USED(A_CELLS, E(AFTER)) ? held[E(AFTER)] : 0; if (r) run(r); return !r || W->outcome == 'n'; }
static int execute_image(const char *path, int warm, int keyed, char **starts) {
  Status why; char saved[PATH_MAX] = "";
  if (!warm) {
    size_t len; uint8_t *raw = load(path, &len); reading = 1; read_seen(path, (uint64_t)(raw - text), len, 0, 0); H image = open_image(raw, len, &why);
    if (!image) { send_answer(2, notice(why)); return 1; }
    run(KID(image, 0)); reading = 0;
    if (W->outcome != 'n') { send_outcome(); return 1; }
    if (keyed) image_rest(1, 1, starts, 0, saved);
  }
  if (!reborn()) { send_outcome(); return 1; }
  for (;;) {
    uint8_t count, id[32]; ssize_t n = read(0, &count, 1);
    if (!n) return 0;
    if (n < 0 && errno == EINTR) continue;
    if (n < 0) { send_answer(2, notice(S_SHORT)); return 1; }
    nhand = 0; H *refs = in_hand((uint64_t)count + 1), *args = in_hand((uint64_t)count + 1);
    int missing = 0;
    for (unsigned j = 0; j <= count; ++j) {
      if (!recv_bytes(id, sizeof id)) { send_answer(2, notice(S_SHORT)); return 1; }
      catalogue_ids(); refs[j] = byid(id); if (!refs[j]) missing = 1;
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
#ifndef NUMERIC_PORT
/* The answer line: a word and the node's canonical print, made by the recipe form/print.ck beside the executable answers.
 * It runs after the program, in its region, on the bootstrap's code alone, so nothing the program bound is seen, and
 * nothing it held back for later runs; the machine then shows the program's end again. */
static void show(const char *word, H v) {
  answering = 1; char path[PATH_MAX]; size_t len; Status why; Machine m = *W; uint64_t later = nafter; H q[2] = { 0, v ? mk(1, E(QUOTE), &v, 1, &why) : 0 }, t;
  if (!*home) die("the host does not say where the executable lies, so form/print.ck is not found");
  snprintf(path, sizeof path, "%sform/print.ck", home); uint8_t *b = load(path, &len);
  nafter = 0; top = 0; if ((q[0] = read_nodes(b, len, &why))) run(q[0]);
  if (q[0] && q[1] && W->outcome == 'n') { q[0] = W->v; run(mk(1, E(CALL), q, 2, &why)); }
  t = q[0] && q[1] && W->outcome == 'n' && !nd[W->v].comp ? W->v : 0; *W = m; nafter = later; top = ~COOL; answering = 0;
  if (!t) die("form/print.ck beside the executable did not print the answer");
  put(1, word); if (!send_fd(1, bytes + nd[t].off, nd[t].n)) exit(2); put(1, "\n");
}
#endif
static int command(int argc, char **argv) {
#ifndef NUMERIC_PORT
  const char *cmd = argc > 1 ? argv[1] : "";
  numeric_io = argc == 3 && !strcmp(cmd, "execute");
#endif
  text_base = ntext; store_init();
  char entered[PATH_MAX] = "";                              /* the start: the image seed-run opens, or the files before the program */
#ifdef NUMERIC_PORT
  int start = 1, starts = argc == 2;
#else
  int start = numeric_io ? 2 : 3, starts = numeric_io ? 1 : argc > 3 && (!strcmp(cmd, "run") || !strcmp(cmd, "text")) ? argc - 3 : 0;
#endif
  int keyed = image_enter(starts, argv + start, entered), warm = keyed > 0 ? keyed : 0;   /* warm: how many of the start's files it stands for */
  keyed = keyed >= 0;
  for (int i = 0; i < A_COUNT + S_COUNT; i++) full_at[i] = seen_as(S_FULL, numeric_io ? mki(i) : txt(AREA_NAME[i]));
#ifdef NUMERIC_PORT
  return argc == 2 ? execute_image(argv[1], warm, keyed, argv + start) : 2;
#else
  if (numeric_io) return execute_image(argv[2], warm, keyed, argv + start);
  if (argc >= 3 && (!strcmp(cmd, "run") || !strcmp(cmd, "text"))) {
    char saved[PATH_MAX] = ""; reading = 1;               /* a start saves the longest part of it at rest, the part it entered with */
    after_seen = USED(A_CELLS, E(AFTER)) ? held[E(AFTER)] : 0;
    for (int i = 3 + warm; i < argc; i++) { run(file_node(argv[i])); if (W->outcome != 'n') { say("not bound: ", argv[i]); return 1; } after_join();
      if (keyed) image_rest(i - 2, starts, argv + 3, warm ? entered : 0, saved); }
    reading = 0; if (!reborn()) { say("not bound: ", "after"); return 1; }
    run(file_node(argv[2]));
    if (cmd[0] == 't' && W->outcome == 'n' && !nd[W->v].comp) {      /* a leaf answer leaves as its own bytes */
      return send_fd(1, bytes + nd[W->v].off, nd[W->v].n) ? 0 : 2;
    }
    char o = W->outcome; show(o == 'n' ? "node " : o == '0' ? "nothing " : "fault ", o == 'n' ? W->v : W->reason);
    return o == 'f' || cmd[0] == 't';
  }
  put(2, "usage: ck execute IMAGE | run F [B..] | text F [B..]\n");
  return 2;
#endif
}
/* A member leaves: the node adopt was given runs once, last, on the walker's thread, whether the run came to its end or the
 * bootstrap ends the process from inside it; the machine then shows the run's own end again. */
static void ending(int sig) {             /* outside a run nothing is half done, and the process ends here; inside one, the walker ends it between two steps */
  if (!pthread_equal(pthread_self(), walker)) { pthread_kill(walker, sig); return; }
  if (!walking) exit(128 + sig); N(A_ASKED) = ~0ull;
}
static void leaving(void) {
  H l = fb->me.leave; Machine m = *W; if (!l || !pthread_equal(pthread_self(), walker)) return;
  fb->me.leave = 0; parting = 1; ntk = ntb = 0; run(l); *W = m;
}
/* The process starts on the host's stack only long enough to move onto the region's. */
typedef struct { int argc; char **argv; int rc; } Start;
static void *on_region(void *p) {
  Start *s = p; walker = pthread_self(); room(A_TEXT, ntext + (1u << 17)); host.guard(crashed, text + ntext, 1u << 17); ntext += 1u << 17;   /* pages the host gives as zero */
  s->rc = command(s->argc, s->argv); leaving(); W->mode = M_DONE; return NULL;
}
int main(int argc, char **argv) {
  Start s = { argc, argv, 2 }; pthread_t t; pthread_attr_t at; char raw[PATH_MAX], real[PATH_MAX];   /* the executable's folder, as */
#ifdef __APPLE__                                                                                   /* the host has it, links followed */
  uint32_t size = sizeof raw; int got = !_NSGetExecutablePath(raw, &size);
#else
  ssize_t size = readlink("/proc/self/exe", raw, sizeof raw - 1); int got = size > 0; if (got) raw[size] = 0;
#endif
  const char *exe = (got || strchr(argv[0], '/')) && realpath(got ? raw : argv[0], real) ? real : argv[0], *slash = strrchr(exe, '/');
  if (slash && (size_t)(slash - exe) + 16 < sizeof home) { snprintf(home, sizeof home, "%.*s", (int)(slash - exe) + 1, exe);
    snprintf(rest_buf, sizeof rest_buf, "%sregion/rest.ck", home); rest_path = access(rest_buf, R_OK) ? 0 : rest_buf; }
  fb_open();
  pthread_attr_init(&at);
  if (pthread_attr_setstack(&at, (uint8_t *)fb + fb->a[A_STACK].off, fb->a[A_STACK].cap)
      || pthread_create(&t, &at, on_region, &s) || pthread_join(t, NULL)) die("cannot run on the framebuffer's stack");
  return s.rc;
}
