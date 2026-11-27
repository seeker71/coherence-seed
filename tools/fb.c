/* fb.c - the observer: a second process that reads a framebuffer (bootstrap/fb.h).
 *
 *   fb show NAME [H]          one frame of a running or kept region; with H, that node and its writer
 *   fb record NAME FILE [MS]  copy a frame every MS milliseconds (default 5) until the writer is done
 *   fb play FILE              one line per recorded frame
 *   fb play FILE N [H]        frame N in detail (-1 is the last); with H, that node and its writer
 *   fb drop NAME              remove a kept region, or a named substrate
 *
 * It maps the region read-only and never writes to it, signals its writer or holds a lock. A member of a
 * named substrate keeps its nodes there: show maps that object too and reads them in place, settled or the
 * member's own. A recording holds the region only, so a member is not recorded yet.
 * A frame is a copy of the head page and of the used part of every area. A recording keeps
 * only the pages that differ from the frame before. The names it prints for catalogue
 * entries are for reading here; they are not part of any node.
 */
#ifdef __linux__
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "../bootstrap/fb.h"

#define STACK_TOP (1u << 18)              /* how much of the machine stack a frame keeps: its newest end */
static const char *const AREA_NAME[A_COUNT] = { FB_AREA_NAMES };
static const char *const NAMES[SEEDED + 1] = { FB_ENTRY_NAMES };     /* the words, as the lexicon has them */
static const char *const MODE[] = { FB_MODE_NAMES };
static const char *const FKIND[] = { FB_FRAME_NAMES };

static uint8_t *img; static uint64_t size;                /* a private copy of one frame, laid out as the region is */
static Head *hd; static Node *nd; static uint8_t *bytes; static H *kids, *argp;
static const uint8_t *far; static const Substrate *sub; static const Lane *lanes;   /* a named substrate, mapped beside the frame; the substrate; its lanes */
static Frame *fr; static Activation *act; static ChoicePoint *cp; static Binding *bd;
static void stop(const char *m, const char *n) { fprintf(stderr, "fb: %s%s\n", m, n); exit(2); }
static void require_layout(const Head *h) {
  if (size < FB_PAGE || memcmp(h->magic, FB_MAGIC, 8) || h->version != FB_VERSION || h->areas != A_COUNT)
    stop("incompatible framebuffer layout", "");
  if (h->bytes != size || h->me.share < FB_PAGE || h->me.share % FB_PAGE || (!h->me.name[0] && h->me.share > size - FB_PAGE))
    stop("invalid framebuffer extent", "");       /* a member's substrate lies beyond its region, in an object of its own */
}
static void require_substrate(const Head *h, const Substrate *s) {
  if (memcmp(s->magic, FB_SUBSTRATE_MAGIC, 8) || s->version != FB_VERSION || s->areas != S_COUNT ||
      s->share != h->me.share || (!far && s->bytes != size - h->me.share))
    stop("incompatible substrate layout", "");
}
static void *at(int i) {                  /* a table in a named substrate is read where it lies, not from the frame */
  return hd->a[i].off >= hd->me.share && far ? (void *)(far + (hd->a[i].off - hd->me.share)) : (void *)(img + hd->a[i].off);
}
static void bind(void) {
  hd = (Head *)img; sub = far ? (const Substrate *)far : (const Substrate *)(img + hd->me.share);
  lanes = (const Lane *)((const uint8_t *)sub + sub->a[S_LANES].off);
  nd = at(A_NODES); bytes = at(A_LEAF_BYTES); kids = at(A_CHILDREN); argp = at(A_ARGUMENTS);
  fr = at(A_FRAMES); act = at(A_ACTIVATIONS); cp = at(A_CHOICE_POINTS); bd = at(A_BINDINGS);
}
/* A handle names a node of this frame: one of its own lane below its count, or one another lane has settled. */
static int has(H h) {
  uint32_t shift = (uint32_t)__builtin_clz(sub->lanes) + 1; uint64_t lane = (uint64_t)h >> shift;
  return h && (lane == hd->me.lane ? h < hd->a[A_NODES].n : lane < sub->a[S_LANES].n && h < lanes[lane].sw);
}
static const uint8_t *substrate(const char *name, uint64_t *bytes_mapped) {     /* a named substrate, read only; NULL when none stands */
  int fd = shm_open(name, O_RDONLY, 0); struct stat s; const uint8_t *p = MAP_FAILED;
  if (fd >= 0 && !fstat(fd, &s) && s.st_size >= (off_t)FB_PAGE) p = mmap(NULL, (size_t)s.st_size, PROT_READ, MAP_SHARED, fd, 0);
  if (fd >= 0) close(fd);
  if (p == MAP_FAILED || memcmp(p, FB_SUBSTRATE_MAGIC, 8)) return NULL;
  *bytes_mapped = (uint64_t)s.st_size; return p;
}
static double now_ms(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec * 1e3 + t.tv_nsec / 1e6; }

/* ---- reading a region ---------------------------------------------------------------- */
static const uint8_t *attach(const char *name, double wait_ms) {   /* waits for a region whose writer lives; takes a finished one last */
  const uint8_t *src = MAP_FAILED; double t0 = now_ms();
  for (;;) {
    int fd = shm_open(name, O_RDONLY, 0); struct stat s;
    if (fd >= 0 && !fstat(fd, &s) && s.st_size >= (off_t)FB_PAGE) {
      size = (uint64_t)s.st_size; src = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    }
    if (fd >= 0) close(fd);
    if (src != MAP_FAILED) {
      const Head *h = (const Head *)src; int live = h->pid && (!kill((pid_t)h->pid, 0) || errno == EPERM);
      if (!memcmp(h->magic, FB_MAGIC, 8) && (live || now_ms() - t0 >= wait_ms)) {
        uint64_t n; char name[33] = {0}; require_layout(h); memcpy(name, h->me.name, 32);
        if (name[0] && !(far = substrate(name, &n))) stop("its substrate is gone: ", name);
        if (far) require_substrate(h, (const Substrate *)far);
        return src;
      }
      munmap((void *)src, size); src = MAP_FAILED;
    }
    if (now_ms() - t0 >= wait_ms) stop("no framebuffer named ", name);
    usleep(200);
  }
}
/* Copies the pages of one frame that differ from the image; lists their offsets in *pg. */
static uint32_t grab(const uint8_t *src, uint64_t **pg, uint64_t *step_after) {
  static uint8_t head[FB_PAGE], shead[FB_PAGE]; static uint64_t *list; static uint32_t cap; uint32_t n = 0;
  memcpy(head, src, FB_PAGE);                             /* the counts are read once, first */
  const Head *h = (const Head *)head; const Substrate *s = (const Substrate *)shead;
  if (!far) { memcpy(shead, src + h->me.share, FB_PAGE); require_substrate(h, s); }   /* and the substrate's, which lies at share */
  for (int i = -1; i <= (far ? A_COUNT - 1 : A_COUNT + S_COUNT); i++) {
    uint64_t off = 0, len = FB_PAGE;
    if (far && i >= 0 && h->a[i].off >= h->me.share) continue;   /* a named substrate's tables are not in the frame */
    if (i == A_COUNT) off = h->me.share;                  /* the substrate's head; its tables in a lane are the process's own areas */
    else if (i > A_COUNT) { const Area *a = &s->a[i - A_COUNT - 1]; off = h->me.share + a->off; len = a->n * a->size;
      if (len) { uint64_t prefix = off % FB_PAGE; off -= prefix; len += prefix; } }
    else if (i >= 0) {
      const Area *a = &h->a[i]; uint64_t full = (uint64_t)a->cap * a->size;
      len = (uint64_t)a->n * a->size; off = a->off;
      if (i == A_STACK) { len = full < STACK_TOP ? full : STACK_TOP; off = a->off + full - len; }
      if (len > full) len = full;
      if (len) { uint64_t prefix = off % FB_PAGE; off -= prefix; len += prefix; }
    }
    for (uint64_t p = off; p < off + len; p += FB_PAGE) {
      const uint8_t *from = i < 0 ? head : i == A_COUNT ? shead : src + p;
      if (!memcmp(img + p, from, FB_PAGE)) continue;
      memcpy(img + p, from, FB_PAGE);
      if (n == cap) { cap = cap ? cap * 2 : 1024; if (!(list = realloc(list, cap * sizeof *list))) stop("out of memory", ""); }
      list[n++] = p;
    }
  }
  *step_after = ((const Head *)src)->m.step; *pg = list; return n;
}

/* ---- showing a frame ------------------------------------------------------------------- */
static char sb[4096]; static size_t sn, slim; static int cut;
static void sp(const char *s) {           /* appends to the line being shown; a line too long ends in three dots */
  for (; *s && !cut; s++) { if (sn + 4 >= slim) { memcpy(sb + sn, "...", 3); sn += 3; cut = 1; } else sb[sn++] = *s; }
  sb[sn] = 0;
}
static long long catn(H h) {
  if (h >= 1 && h <= SEEDED) return h - 1;
  if (!has(h) || nd[h].comp || nd[h].bp != ROOT) return -1;
  long long v = 0; for (uint64_t i = nd[h].n; i > 0; i--) v = v << 8 | bytes[nd[h].off + i - 1];
  return v;
}
static void shown(H h, int depth) {
  char t[40];
  if (!h) { sp("nothing"); return; }
  if (!has(h)) { sp("?"); return; }
  long long c = catn(h); const Node *x = &nd[h]; const uint8_t *b = bytes + x->off;
  if (c >= 0) { if (c <= SEEDED && NAMES[c]) sp(NAMES[c]); else { snprintf(t, sizeof t, "#%lld", c); sp(t); } return; }
  if (x->comp) {
    if (depth <= 0) { sp("(..)"); return; }
    sp("("); shown(x->bp, depth - 1);
    for (uint64_t i = 0; i < x->n && !cut; i++) { sp(" "); shown(kids[x->off + i], depth - 1); }
    sp(")"); return;
  }
  if ((x->bp == E(INT) || x->bp == E(NAT)) && x->n <= 8) {
    uint64_t u = (x->bp == E(INT) && x->n && (b[x->n-1] & 0x80)) ? ~0ull : 0;
    for (uint64_t i = x->n; i > 0; i--) u = u << 8 | b[i-1];
    if (x->bp == E(INT)) snprintf(t, sizeof t, "%lld", (long long)u); else snprintf(t, sizeof t, "%llun", (unsigned long long)u);
    sp(t); return;
  }
  if (x->bp == E(2)) { sp("\""); for (uint64_t i = 0; i < x->n && !cut; i++) { t[0] = (char)b[i]; t[1] = 0; sp(t); } sp("\""); return; }
  sp("["); shown(x->bp, depth - 1); sp(" ");
  for (uint64_t i = 0; i < x->n && !cut; i++) { snprintf(t, sizeof t, "%02x", b[i]); sp(t); }
  sp("]");
}
static const char *str(H h, size_t lim) { sn = 0; cut = 0; slim = lim < sizeof sb ? lim : sizeof sb; sb[0] = 0; shown(h, 8); return sb; }
static void writer(H by) { if (by) printf("%s", str(by, 90)); else printf("the bootstrap's own reader"); }

static void line(int i, double ms, uint64_t after, uint32_t pages) {
  const Machine *m = &hd->m;
  printf("frame %4d  %9.2f ms  step %11llu%s  %-6s  nodes %-7llu frames %-5llu links %-5llu activations %-8llu choices %-3llu pages %-5u at %s\n",
         i, ms, (unsigned long long)m->step, after != m->step ? " moving" : "       ", MODE[m->mode <= M_DONE ? m->mode : 0],
         (unsigned long long)hd->a[A_NODES].n, (unsigned long long)hd->a[A_FRAMES].n, (unsigned long long)hd->a[A_LINKS].n,
         (unsigned long long)hd->a[A_ACTIVATIONS].n, (unsigned long long)hd->a[A_CHOICE_POINTS].n, pages, str(m->at, 70));
}
static void node(H h) {
  if (!has(h)) { printf("node %u: not in this frame\n", h); return; }
  printf("node %u\n  is         %s\n", h, str(h, 2000));
  printf("  blueprint  %s\n", str(nd[h].bp, 200));
  printf("  %s, %llu %s\n  written by ", nd[h].comp ? "composite" : "leaf", (unsigned long long)nd[h].n, nd[h].comp ? "children" : "bytes");
  writer(nd[h].by); printf("\n");
}
static void detail(void) {
  const Machine *m = &hd->m;
  printf("framebuffer version %u, writer pid %u, %llu bytes reserved\n", hd->version, hd->pid, (unsigned long long)hd->bytes);
  printf("memory allowance %llu bytes\n", (unsigned long long)hd->me.allowance);
  const Lane *l = &lanes[hd->me.lane]; char name[33] = {0}; memcpy(name, hd->me.name, 32);
  printf("substrate %s at byte %llu, %u lanes: lane %u of pid %u settled below %u\n", name[0] ? name : "of its own", (unsigned long long)hd->me.share,
         sub->lanes, hd->me.lane, (uint32_t)(l->who >> 42), l->sw);
  printf("machine  mode %s  step %llu  env %u  cut %u  k %u\n", MODE[m->mode <= M_DONE ? m->mode : 0], (unsigned long long)m->step, m->env, m->cut, m->k);
  printf("  at      %s\n", str(m->at, 160));
  printf("  x       %s\n", str(m->x, 160));
  printf("  v       %s\n", str(m->v, 160));
  printf("  reason  %s\n", str(m->reason, 160));
  if (m->mode == M_DONE && m->outcome) {
    printf("outcome  %s %s", m->outcome == 'n' ? "node" : m->outcome == '0' ? "nothing" : "fault", str(m->outcome == 'n' ? m->v : m->reason, 400));
    if (m->outcome == 'n') { printf("   (node %u, written by ", m->v); writer(nd[m->v].by); printf(")"); }
    printf("\n");
  }
  printf("areas\n");
  for (int i = 0; i < A_COUNT; i++) { const Area *a = &hd->a[i];
    printf("  %-20s %-13s %10llu in use %12llu bytes %12llu at most\n", AREA_NAME[i], str(a->of, 40), (unsigned long long)a->n,
           (unsigned long long)a->n * a->size, (unsigned long long)a->peak); }
  if (m->env && m->env < hd->a[A_ACTIVATIONS].n) {
    const Activation *e = &act[m->env];
    printf("activation %u  recipe %s\n  arguments", m->env, str(e->recipe, 120));
    for (uint32_t i = 0; i < e->n && i < 8; i++) printf("  %s", str(argp[e->off + i], 60));
    printf("\n");
  }
  printf("continuation, newest first\n");
  uint32_t k = m->k;
  for (int i = 0; k && k < hd->a[A_FRAMES].n && i < 12; i++, k = fr[k].k) {
    const Frame *f = &fr[k];
    printf("  frame %-6u %-8s child %-2u of %s\n", k, FKIND[f->t <= F_LAYER ? f->t : 0], f->i, str(f->node, 100));
    printf("  %14s written by ", ""); writer(f->by); printf("\n");
  }
  if (k) printf("  ... and older frames\n");
  for (uint32_t i = hd->a[A_CHOICE_POINTS].n, j = 0; i > 0 && j < 4; i--, j++)
    printf("choice point %u  next option %u of %s\n", i - 1, cp[i-1].i, str(cp[i-1].node, 100));
  const Binding *ob = at(A_OUTSIDE);
  for (uint32_t i = hd->a[A_OUTSIDE].n; i > 0; i--) if (ob[i-1].entry)          /* what was bound from outside the run */
    { printf("outside %u  %s ", i, str(ob[i-1].entry, 60)); printf("is run by %s\n", str(ob[i-1].impl, 100)); }
  for (uint32_t i = hd->a[A_BINDINGS].n, j = 0; i > 0 && j < 12; i--, j++) {      /* what runs, newest first */
    printf("binding %u  %s ", i, str(bd[i-1].entry, 60)); printf(bd[i-1].impl ? "is run by %s\n" : "is released%s\n", bd[i-1].impl ? str(bd[i-1].impl, 100) : ""); }
}

/* ---- commands ---------------------------------------------------------------------------- */
static const char RMAGIC[8] = "CKREC1\n";
static void image(void) {
  int flags = MAP_PRIVATE | MAP_ANON;
#ifdef __linux__
  flags |= MAP_NORESERVE;  /* only the pages present in the recorded frame are copied */
#endif
  img = mmap(NULL, size, PROT_READ | PROT_WRITE, flags, -1, 0);
  if (img == MAP_FAILED) stop("cannot hold a frame", "");
}
static int record(const char *name, const char *path, double every) {
  const uint8_t *src = attach(name, 5000); image();
  if (far) stop("a member of a named substrate is not recorded yet: ", name);
  FILE *f = fopen(path, "wb"); if (!f) stop("cannot write ", path);
  fwrite(RMAGIC, 1, 8, f); fwrite(&size, 8, 1, f);
  double t0 = now_ms(); uint32_t frames = 0; uint64_t all = 0;
  for (int last = 0; !last; frames++) {
    const Head *live = (const Head *)src;
    last = live->m.mode == M_DONE || (kill((pid_t)live->pid, 0) && errno != EPERM);
    uint64_t *pg, after; uint32_t n = grab(src, &pg, &after); double ms = now_ms() - t0;
    fwrite(&ms, 8, 1, f); fwrite(&after, 8, 1, f); fwrite(&n, 4, 1, f);
    for (uint32_t i = 0; i < n; i++) { fwrite(&pg[i], 8, 1, f); fwrite(img + pg[i], 1, FB_PAGE, f); }
    all += n;
    if (!last) usleep((useconds_t)(every * 1000));
  }
  if (fclose(f)) stop("cannot write ", path);
  bind();
  printf("fb: %u frames, %llu pages, %.1f ms; the last frame is %s at step %llu\n", frames, (unsigned long long)all, now_ms() - t0,
         MODE[hd->m.mode <= M_DONE ? hd->m.mode : 0], (unsigned long long)hd->m.step);
  return 0;
}
static int play(const char *path, int want, int detailed, H h) {
  FILE *f = fopen(path, "rb"); char mg[8]; double ms = 0; uint64_t after = 0; uint32_t n = 0; int i = 0, any = 0;
  if (!f || fread(mg, 1, 8, f) != 8 || memcmp(mg, RMAGIC, 8) || fread(&size, 8, 1, f) != 1) stop("not a recording: ", path);
  image();
  for (;; i++) {
    double m2; uint64_t a2, off; uint32_t n2;
    if (fread(&m2, 8, 1, f) != 1 || fread(&a2, 8, 1, f) != 1 || fread(&n2, 4, 1, f) != 1) break;
    for (uint32_t j = 0; j < n2; j++) {
      if (fread(&off, 8, 1, f) != 1 || size < FB_PAGE || off % FB_PAGE || off > size - FB_PAGE || fread(img + off, 1, FB_PAGE, f) != FB_PAGE) stop("the recording is cut short: ", path);
      if (off == 0) require_layout((const Head *)img);
    }
    require_layout((const Head *)img);
    require_substrate((const Head *)img, (const Substrate *)(img + ((const Head *)img)->me.share));
    ms = m2; after = a2; n = n2; any = 1; bind();
    if (!detailed) line(i, ms, after, n);
    else if (i == want) break;
  }
  if (!any || (detailed && want >= 0 && i != want)) stop("no such frame in ", path);
  if (detailed) { line(want < 0 ? i - 1 : i, ms, after, n); if (h) node(h); else detail(); }
  return 0;
}
int main(int argc, char **argv) {
  const char *c = argc > 1 ? argv[1] : "";
  if ((argc == 3 || argc == 4) && !strcmp(c, "show")) {
    const uint8_t *src = attach(argv[2], 0); uint64_t *pg, after; image();
    uint32_t n = grab(src, &pg, &after); bind(); line(0, 0, after, n);
    if (argc == 4) node((H)strtoul(argv[3], NULL, 10)); else detail();
    return 0;
  }
  if ((argc == 4 || argc == 5) && !strcmp(c, "record")) return record(argv[2], argv[3], argc == 5 ? atof(argv[4]) : 5);
  if (argc == 3 && !strcmp(c, "play")) return play(argv[2], 0, 0, 0);
  if ((argc == 4 || argc == 5) && !strcmp(c, "play")) return play(argv[2], atoi(argv[3]), 1, argc == 5 ? (H)strtoul(argv[4], NULL, 10) : 0);
  if (argc == 3 && !strcmp(c, "drop")) { if (shm_unlink(argv[2])) stop("no framebuffer named ", argv[2]); return 0; }
  fprintf(stderr, "usage: fb show NAME [H] | record NAME FILE [MS] | play FILE [N [H]] | drop NAME\n");
  return 2;
}
